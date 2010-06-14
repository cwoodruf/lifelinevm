
/* util.c -- part of ansr3.c (lifeline.exe program) */

// find out if a box is being used by another application or user
int
box_in_use (char *box)
{
  char lock[BOXLEN + 10] = { 0 };
  struct _find_t f;
  // check for the existence of a lock file
  sprintf (lock, "%s.LCK", box);
  // findfirst returns 0 if the file is found
  if (!_dos_findfirst (lock, _A_NORMAL, &f))
    return FALSE;
  return TRUE;
}

// check a file to see if its from a box that is in use
int
check_box_file (char *fname)
{
  int i;
  char box[BOXLEN + 1] = { 0 };
  for (i = 0; i < BOXLEN; i++)
    {
      switch (fname[i])
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  continue;
	default:
	  return TRUE;		// not a box file so we don't care
	}
    }
  strncpy (box, fname, 4);
  return box_in_use (box);
}

// check if a file is from a box that is in use before deleting it
void
safe_unlink (char *fname)
{
  if (check_box_file (fname))
    {
      _unlink (fname);
    }
}

long
getsecs (long size)
{
  // get number of seconds : if over even second round up
  long secs;
  secs = size / SMPLRATE;
  if (secs * SMPLRATE < size)
    secs++;
  return secs;
}

char *
newmsgname (int channel)
{
  sprintf (PRT.fname, MSGFSTR, USER.box, USER.nextmsg, MSG);
  return PRT.fname;
}

char *
msgname (int channel)
{
  sprintf (PRT.fname, MSGFSTR, USER.box, PRT.currmsg, MSG);
  return PRT.fname;
}

char *
msginame (char *box, int m)
{
  sprintf (s, MSGFSTR, box, m, MSG);
  return s;
}

int
lookforfiles (char *box, char *ext, struct _find_t *mf)
{
  char mask[DOSPATHLEN];

  strcpy (mask, box);
  strcat (mask, ext);
  return _dos_findfirst (mask, _A_NORMAL, mf);
}

void
fillmsginfo (MI * msginfo, struct _find_t *mf)
{
  msginfo->secs = mf->size / SMPLRATE;
  boxttl += msginfo->secs;
  msginfo->Day = mf->wr_date & 0x1f;
  msginfo->Month = (mf->wr_date >> 5) & 0x0f;
  msginfo->Year = (mf->wr_date >> 9) + 1980;
  msginfo->Hour = (mf->wr_time >> 11);
  msginfo->Minute = (mf->wr_time >> 5) & 0x3f;
}

time_t
getftime (struct _find_t *mf)
{
  int Day, Month, Year, Hour, Minute, Sec;

  Day = mf->wr_date & 0x1f;
  Month = (mf->wr_date >> 5) & 0x0f;
  Year = (mf->wr_date >> 9) + 1980;
  Hour = (mf->wr_time >> 11);
  Minute = (mf->wr_time >> 5) & 0x3f;
  Sec = 0;

  return gettimesecs (Sec, Minute, Hour, Day, Month, Year);
}

void
getmsginfo (void)
{
  struct _find_t mf;
  int err, i = 0, m;
  MI mi;

  boxttl = 0;

  for (i = 0; i < MAXFILES; i++)
    minfo[i] = nominfo;
  //look for greeting
  if (!lookforfiles (user.box, GRT, &mf))
    {
      fillmsginfo (&mi, &mf);
      minfo[0] = mi;
    }
  //look for reminder
  if (!lookforfiles (user.box, RMD, &mf))
    {
      fillmsginfo (&mi, &mf);
      minfo[1] = mi;
    }
  //start looking for files
  err = lookforfiles (user.box, MMASK, &mf);
  i = 0;
  while (i < MAXMSG && !err)
    {
      m = atoi (mf.name + strlen (user.box));
      if (m < MAXMSG)
	{
	  fillmsginfo (&mi, &mf);
	  minfo[m + 2] = mi;
	  i++;
	}
      sched ();
      err = _dos_findnext (&mf);
    }
}

void
fillinfostr (char *s, int i, MI msginfo)
{
  if (msginfo.secs <= 0)
    sprintf (s, " Message: %2d  Not in use.", i - 2);
  else if (msginfo.secs == 1)
    sprintf (s,
	     " Message: %2d  Date: %4d/%02d/%02d.  Time: %2d:%02d.  Length: 1 second.",
	     i - 2, msginfo.Year, msginfo.Month, msginfo.Day, msginfo.Hour,
	     msginfo.Minute);
  else
    sprintf (s,
	     " Message: %2d  Date: %4d/%02d/%02d.  Time: %2d:%02d.  Length: %ld seconds.",
	     i - 2, msginfo.Year, msginfo.Month, msginfo.Day, msginfo.Hour,
	     msginfo.Minute, msginfo.secs);
}

void
blinker (int i)
{
  //write the message
  writestr (1,			/* x pos */
	    INFO_TOP + i + 1,	/* y pos */
	    0,			/* start in string at */
	    1,			/* size of window */
	    FLDATTR + BLINK,	/* color */
	    ">");
}

void
showmsginfo (int start)
{
  int i;
  long percentfull = (boxttl * 100) / ((maxmsg + 1) * maxtime);	//include greeting size in message
  //round up
  if ((boxttl * 100) % ((maxmsg + 1) * maxtime) >
      ((maxmsg + 1) * maxtime) / 2)
    percentfull++;
  //check start value
  if (start < 0)
    start = 0;

  sprintf (s, " Files for Box %s. Message time = %ld seconds. %d%% full.",
	   user.box, boxttl, percentfull);
  writeimsg (s, 0);
  blinker (1);
  for (i = start + 1; i < start + INFOHT; i++)
    {
      strcpy (s, "");
      if (i <= MAXFILES)
	{
	  if (start <= INFOHT)
	    {
	      switch (i)
		{
		case 1:
		  if (minfo[i - 1].secs <= 0)
		    strcpy (s, " Using default greeting.");
		  else
		    sprintf (s,
			     " Personal greeting recorded. Length: %ld seconds.",
			     minfo[i - 1].secs);
		  break;
		case 2:
		  if (minfo[i - 1].secs <= 0)
		    strcpy (s, " Custom reminder not recorded.");
		  else
		    sprintf (s,
			     " Custom reminder recorded. Length: %ld seconds.",
			     minfo[i - 1].secs);
		  break;
		default:
		  fillinfostr (s, i, minfo[i - 1]);
		}
	    }
	  else
	    fillinfostr (s, i, minfo[i - 1]);
	}
      writeimsg (s, i - start);
    }
}

void
movemsg (int msg, char *srcbox, char *targbox)
{
  int i, tbox, sbox, sold, told;
  char srcmsg[40];
  char targmsg[40];

  if (strlen (targbox) == 0)
    return;
  tbox = atoi (targbox);
  sbox = atoi (srcbox);

  if (useridx[sbox].avail >= INUSE)
    {
      writemsg ("Cannot move file source box is in use!");
      return;
    }

  switch (useridx[tbox].avail)
    {
    case INACTIVE:
    case ACTIVE:
    case ONHOLD:
      sold = useridx[sbox].avail;
      told = useridx[tbox].avail;
      useridx[tbox].avail = INUSE;
      useridx[sbox].avail = INUSE;
      //find last message
      for (i = 0; i < MAXMSG; i++)
	{
	  sched ();		//do some phone processing
	  if (!fexist (msginame (targbox, i)))
	    break;
	}
      //if it is less than MAXMSG rename message
      if (i >= MAXMSG)
	{
	  sprintf (s, "Could not move message. Box %s full!", targbox);
	  writemsg (s);
	}
      else
	{
	  switch (msg)
	    {
	    case 0:		//greeting
	      strcpy (srcmsg, fname (srcbox, GRT));
	      strcpy (targmsg, fname (targbox, GRT));
	      break;
	    case 1:		//reminder
	      strcpy (srcmsg, fname (srcbox, RMD));
	      strcpy (targmsg, fname (targbox, RMD));
	      break;
	    default:
	      strcpy (srcmsg, msginame (srcbox, msg - 2));	//messages start from zero and not 2
	      strcpy (targmsg, msginame (targbox, i));
	    }
	  if (rename (srcmsg, targmsg))
	    sprintf (s, "Unable to rename %s as %s.", srcmsg, targmsg);
	  else
	    sprintf (s, "MOVE: %s successfully renamed %s.", srcmsg, targmsg);
	  writemsg (s);
	  useridx[tbox].avail = told;
	  useridx[sbox].avail = sold;
	}
      break;
    default:
      if (useridx[tbox].avail >= INUSE)
	writemsg ("Target box is in use please try again later!");
      break;
    }
}

void
updatebox (char *box)
{
  int channel = EDITCHAN;
  US c;

  PRT = blankport;
  USER = blankuser;
  strcpy (PRT.dtmf, box);
  PRT.box = atoi (box);
  loaduser (FUSER, PRT.box, &c);
  USER = c;
  checkmsg (channel);
  c = USER;
  if (strcmp (user.box, c.box) == 0)
    user = c;
  saveuser (FUSER, PRT.box, &c);
  PRT = blankport;
  USER = blankuser;
}

void
delsinglemsg (char *userbox, int start)
{
  char name[20];
  int box;
  box = atoi (userbox);
  // this was breaking some of the console file management features
  // if (box_in_use(userbox) || useridx[box].avail >= INUSE)
  if (useridx[box].avail >= INUSE)
    {
      writemsg ("Cannot delete message: box in use!");
      return;
    }

  {
    switch (start)
      {
      case 0:
	strcpy (name, fname (userbox, GRT));
	break;
      case 1:
	strcpy (name, fname (userbox, RMD));
	break;
      default:
	strcpy (name, msginame (userbox, start - 2));
      }
    sprintf (s, "Deleted %s...", name);
    writemsg (s);
    _unlink (name);
    updatebox (userbox);
  }
}

void
viewmsginfo (char key, char zero, int *i, int *pos, int *startpos)
{
  static int start = 0;
  int oldks;

  if (keystate != ksMSGINFO)
    {
      oldks = keystate;
      keystatesetup (ksMSGINFO, i, pos, startpos);
      switch (oldks)
	{
	case ksEDIT:
	  start = 0;
	  break;
	case ksMOVEMSG:
	  if (action == aMOVEMSG)
	    {
	      movemsg (start, user.box, targetbox);
	      updatebox (user.box);
	      updatebox (targetbox);
	    }
	  break;
	}
      getmsginfo ();
    }
  else
    {
      if (action == aDELSINGLEMSG)
	{
	  delsinglemsg (user.box, start);
	  updatebox (user.box);
	  getmsginfo ();
	}

      if (zero)
	switch (key)
	  {
	  case DEL:
	    action = aDELSINGLEMSG;
	    writemsg ("Delete this file? Y/N (Enter = Y)");
	    break;
	  case UP:
	    if (start > 0)
	      start--;
	    break;
	  case DOWN:
	    if (start < MAXFILES - 1)
	      start++;
	    break;
	  case HOME:
	    start = 0;
	    break;
	  case END:
	    start = MAXFILES - 1;
	    break;
	  case PGDN:
	    if (start <= MAXFILES - INFOHT)
	      start += INFOHT - 1;
	    else
	      start = MAXFILES - 1;
	    break;
	  case PGUP:
	    if (start >= INFOHT - 1)
	      start -= INFOHT - 1;
	    else
	      start = 0;
	    break;
	  case AltM:
	    if (minfo[start].secs > 0)
	      keystatesetup (ksMOVEMSG, i, pos, startpos);
	    break;
	  }
    }
  showmsginfo (start);
}

int
findnewmsg (char *box)
{
  long maxbs = ((maxmsg - 1L) * maxtime) - 1L;
  long bs = getboxsize (box);
  register i;

  if (maxbs - bs < MINTIME)
    return NOTFOUND;
  for (i = 0; i < MAXMSG; i++)
    if (!fexist (msginame (box, i)))
      return i;
  return NOTFOUND;
}

void
boxclear (int channel)
{
  US u;
  time_t now;
  register i, j = 0;
  char oldname[40], newname[40], newbox[10];
  int firstmsg, lastmsg, nummsg = 0, newmsg, nb;

  firstmsg = lastmsg = -1;

  //check for messages
  for (i = 0; i < USER.nextmsg; i++)
    {
      if (MGS[i] == FALSE)	//remove deleted messages
	{
	  _unlink (msginame (USER.box, i));
	}
      else if (MGS[i] == TRUE)	//count messages 
	{
	  nummsg++;
	  if (firstmsg == -1)
	    firstmsg = i;
	  if (i > lastmsg)
	    lastmsg = i;
	}
      else			//forward message to another box
	{
	  nb = MGS[i] - FWDOFFS;
	  sprintf (newbox, "%04d", nb);
	  if ((newmsg = findnewmsg (newbox)) >= 0)
	    {
	      strcpy (newname, msginame (newbox, newmsg));
	      strcpy (oldname, msginame (USER.box, i));
	      rename (oldname, newname);
	      loaduser (FUSER, nb, &u);
	      u.nextmsg = newmsg + 1;
	      u.msgtime = time (&now);
	      saveuser (FUSER, nb, &u);
	      addpage (&u);
	      if (nb == curruser)
		{
		  user.nextmsg = newmsg + 1;
		  user.msgtime = time (&now);
		  showuser (&user);
		}
	    }
	}
    }

  //check for no messages
  if (nummsg == 0)
    {
      USER.nextmsg = PRT.lastmsg = 0;
      PRT.deleted = TRUE;
      return;
    }

  //if there are messages rename them in consecutive order
  for (i = firstmsg; i <= lastmsg; i++)
    {
      if (MGS[i] == TRUE)
	{
	  if (i != j)
	    {
	      strcpy (oldname, msginame (USER.box, i));
	      strcpy (newname, msginame (USER.box, j));
	      //modify MGS array if rename is successful
	      if (!rename (oldname, newname))
		{
		  MGS[i] = FALSE;
		  MGS[j] = TRUE;
		}
	    }
	  j++;
	}
    }
  //set variables
  PRT.lastmsg = USER.nextmsg = nummsg;
}

long
boxload (int channel)
{
  char nummsg = 0, lastmsg = 0;
  int err, i, len;
  struct _find_t f;
  long size = 0L;
  time_t ftime = 0L;
  register j;

  for (j = 0; j < MAXMSG; j++)
    MGS[j] = 0;			//clear message array

  err = _dos_findfirst (fname (USER.box, "*.*"), _A_NORMAL, &f);
  while (!err)
    {
      len = strlen (f.name);
      //message must follow correct format
      if ((len == 8 && strcmp (f.name + 4, MSG) == 0) ||
	  (len == 10 && strcmp (f.name + 6, MSG) == 0))
	{
	  nummsg++;
	  if (len == 8)		//message is old style message file with no number
	    {
	      i = 0;
	      rename (f.name, msginame (USER.box, i));
	    }
	  else
	    i = atoi (f.name + 4);	//get number of message
	  //mark message as TRUE in MGS array and 
	  //modify lastmsg to indicate number of messages 
	  if (i >= 0 && i < MAXMSG)
	    {
	      MGS[i] = TRUE;
	      size += f.size;
	      if (i + 1 > lastmsg)
		lastmsg = i + 1;
	    }
	}
      else
	size += f.size;
      err = _dos_findnext (&f);
    }

  USER.nextmsg = lastmsg;

  if (nummsg != USER.nextmsg)
    {
      if (nummsg == 0)
	{
	  PRT.deleted = TRUE;
	  USER.nextmsg = 0;
	}
      else
	boxclear (channel);
    }

  PRT.lastmsg = USER.nextmsg;

  return getsecs (size);
}

long
getboxsize (char *box)
{
  struct _find_t f;
  long size = 0L;
  int err;

  err = _dos_findfirst (fname (box, "*.*"), _A_NORMAL, &f);

  while (!err)
    {
      size += f.size;
      err = _dos_findnext (&f);
    }

  return getsecs (size);
}

//deletes files after getting size
long
delboxsize (char *box)
{
  struct _find_t f;
  long size = 0L;
  int err;

  err = _dos_findfirst (fname (box, "*.*"), _A_NORMAL, &f);

  while (!err)
    {
      size += f.size;
      _unlink (f.name);
      sched ();
      err = _dos_findnext (&f);
    }

  return getsecs (size);
}

long
getdiskused (void)
{
  int err;
  struct _find_t f;

  sprintf (s, "Calculating disk space used. Please wait...\n");
  initmsg (s);
  sprintf (s, "Adding greetings...\n");
  initmsg (s);
  err = _dos_findfirst (GMASK, _A_NORMAL, &f);
  while (!err)
    {
      diskused += f.size;
      err = _dos_findnext (&f);
    }

  sprintf (s, "Adding reminders...\n");
  initmsg (s);
  err = _dos_findfirst (RMASK, _A_NORMAL, &f);
  while (!err)
    {
      diskused += f.size;
      err = _dos_findnext (&f);
    }

  sprintf (s, "Adding messages...\n");
  initmsg (s);
  err = _dos_findfirst (MMASK, _A_NORMAL, &f);
  while (!err)
    {
      diskused += f.size;
      err = _dos_findnext (&f);
    }

  return diskused = getsecs (diskused);
}

void
loadinfo (int channel, US * user)
{
  US buff;

  buff = *user;

  strcpy (buff.remind, USER.remind);
  strcpy (buff.pin, USER.pin);
  strcpy (buff.language, USER.language);
  strcpy (buff.phone, USER.phone);
  strcpy (buff.misc, USER.misc);
  strcpy (buff.paidto, USER.paidto);
  buff.nextmsg = USER.nextmsg;
  buff.calls = USER.calls;
  buff.acctime = USER.acctime;
  buff.start = USER.start;
  buff.msgtime = USER.msgtime;

  *user = buff;
}

void
voidport (int channel)
{
  time_t now;
  PS s;				//save port info

  if (sysinuse == channel)
    sysinuse = FALSE;

  /* clear the port variable on hangup OR restart but save state info */
  s.curr = PRT.curr;
  s.prev = PRT.prev;
  s.language = PRT.language;
  s.oldstate = PRT.oldstate;
  PRT = blankport;
  PRT.language = s.language;
  PRT.prev = s.prev;
  PRT.curr = s.curr;
  PRT.oldstate = s.oldstate;
  PRT.box = NOTFOUND;
  PRT.lastaccess = time (&now);
  USER = blankuser;
}

void
recycleport (int channel)
{
  US saveduser;
  int b;

  b = atoi (USER.box);

  if (PRT.prev != sOFFHK)
    {
      /* find out what is already saved at that position */
      loaduser (FUSER, b, &saveduser);
      if (useridx[b].avail >= ACTIVE)
	{
	  //change disk usage total and clear deleted messages
	  closermd (channel);
	  if (PRT.modified)	//only delete messages if authorized user accessed box
	    boxclear (channel);
	  diskused -= PRT.boxsize;
	  diskused += getboxsize (USER.box);
	  /* update box info that has changed */
	  loadinfo (channel, &saveduser);
	  // reset box status
	  useridx[b].avail = saveduser.active;
	  /* if box is on the screen show new values */
	  if (b == curruser)
	    {
	      /* make sure current box on screen is up to date */
	      user = USER;
	      loadinfo (channel, &user);
	      /* refresh if user is actually showing on screen */
	      updatecalls (&user);
	      writefield (fACTIVE, 0, &user);
	    }
	  saveuser (FUSER, b, &saveduser);
	}
      else
	useridx[b].avail = saveduser.active;
      voidport (channel);
    }
}

int
trecord (int channel, int fh, int rtime)
{
  vhseek (fh, 0L, 0);		//go back to start
  clrxrwb (&d4xrwb);		//clear any saved digits in queue
  d4xrwb.filehndl = fh;
  d4xrwb.maxsec = rtime;	/* maximum MAXMSGTIME seconds for the message   */
  d4xrwb.maxdtmf = 1;		/* terminate if any dtmf                */
  d4xrwb.termdtmf = '@';	/* terminate if any dtmf                */
  d4xrwb.maxsil = MAXSIL;	/* terminate after MAXSIL seconds of silence */
  d4xrwb.loopsig = 1;		/* terminate on loop signal             */
  d4xrwb.rwbflags = 0x02;	/* enable beep before record            */
  d4xrwb.rwbdata1 = 3;		/* 0.6 second beep                       */

  return (recfile (channel, &d4xrwb, RM_NORM + scomp));
}

int
record (int channel, int fh)
{
  return trecord (channel, fh, (int) maxtime);
}

int
recordgreeting (int channel)
{
  struct _find_t f;
  long rtime = 0L;
  long gsize = 0L;
  //leave enough time for a message along with greeting
  long maxbs = ((maxmsg - 1L) * maxtime) - 1L;
  long bs = getboxsize (USER.box);
  char mask[DOSPATHLEN];
  //build greeting file name
  strcpy (mask, USER.box);
  strcat (mask, GRT);
  //find greeting size
  if (!_dos_findfirst (mask, _A_NORMAL, &f))
    gsize = f.size;
  //find actual amount of time you have to record
  if (maxbs > bs)
    rtime = maxbs - bs + gsize;
  //if actual amount is too low reset to default value
  if (rtime < maxtime)
    rtime = maxtime;
  //record greeting
  return trecord (channel, PRT.gh, (int) rtime);
}

int
loaddigits (char *dtmf, int channel, int maxdt, int waitsec)
{
  /* set up the read/write block for call to getdtmfs() */
  clrxrwb (&d4xrwb);
  /* minimum time of 2 seconds for function to terminate */
  if (waitsec < 2)
    waitsec = 2;
  /* give rwb the segment and offset of our dtmf buffer */
  d4xrwb.xferoff = d4getoff (dtmf);
  d4xrwb.xferseg = d4getseg (dtmf);
  d4xrwb.maxdtmf = maxdt;
  d4xrwb.maxsec = waitsec;	/* wait waitsec seconds */
  d4xrwb.loopsig = 1;		/* terminate on loop signal */

  return (getdtmfs (channel, &d4xrwb));
}

// wait for any digit to be pressed: hopefully forever.
// this should only be used by a channel that is set
// up for a direct connection (ie no on hook state)
int
waitdigit (int channel)
{
  static char tmp[2] = { 0 };
  // wait for a key forever
  clrxrwb (&d4xrwb);
  d4xrwb.xferoff = d4getoff (tmp);
  d4xrwb.xferseg = d4getseg (tmp);
  d4xrwb.termdtmf = '@';
  return (getdtmfs (channel, &d4xrwb));
}

/****************************************************************
*        NAME : waitevt(evtp)
* DESCRIPTION : wait for an event from the event block.
*       INPUT : evtp = pointer to EVTBLK.
*      OUTPUT : none.
*     RETURNS : key pressed, 0 if event received.
*    CAUTIONS : none.
****************************************************************/
unsigned char
waitevt (EVTBLK * evtp)
{
  while (1)
    {
      //check for voice processing event
      if (gtevtblk (evtp) == -1) break;

      /* update stats line on screen */
      writestats ();

      /* check to see if a key has been pressed */
      keypressed = FALSE;

      if (kbhit ())
	{
	  keypressed = TRUE;
	  zero = FALSE;
	  while ((key = getch ()) == 0)
	    zero = TRUE;

	  if (kbaction > kbIDLE)
	    {
	      if (zero == FALSE && key == ESC)
		{
		  switch (kbaction)
		    {
		    case kbPARTBUP:
		    case kbBACKUP:
		    case kbCSVABUP:
		    case kbCSVBUP:
		      if (bupstage == bWAITDISK)
			bupstage = bAUTOABORT;
		      else
			{
			  bupstage = bERR;
			  buperr = TRUE;
			}
		      writemsg ("Back up aborted. Complete back up ASAP!");
		    }
		}
	    }
	  else
	    {
	      return key;
	    }
	}
      if (kbaction > kbIDLE)
	{
	  // note 0 and 1 arguments determine what mask 
	  // to start using when copying (ie what data files will get picked up)
	 	if (kbaction == kbPARTBUP || kbaction == kbBACKUP) {
	      dobackup (0, backup.path);
	    } else if (kbaction == kbCSVBUP) {
	      dobackup (1, backup.path);
	    } else if (kbaction == kbCSVABUP) {
	      dobackup (1, "A:\\");
	    }
	}
    }

  return (EVENT);
}

/****************************************************************
*        NAME : play(channel,h)
* DESCRIPTION : set R/W block and initiate playing a file.
*       INPUT : channel = channel number
*             : h = file handle
*      OUTPUT : calls function that initiates play.
*     RETURNS : error code from xplayf().
*    CAUTIONS : multi-tasking process.
****************************************************************/
int
play (int channel, int h)
{
  /* rewind to top of file */
  vhseek (h, 0L, 0);

  clrdtmf (channel);		/* empty out any saved digits */
  clrxrwb (&d4xrwb);		/* clear the D/4x read/write block */
  d4xrwb.filehndl = h;		/* handle of file to play */
  d4xrwb.maxdtmf = 1;		/* cause an event if max digits */
  d4xrwb.loopsig = 1;		/* terminate on loop signal drop */

  /* play vox file on D/4x channel, normal play back */
  return (xplayf (channel, PM_NORM, &d4xrwb));
}

int
playgetnum (int channel, int h)
{
  /* rewind to top of file */
  vhseek (h, 0L, 0);

  clrdtmf (channel);		/* empty out any saved digits */
  clrxrwb (&d4xrwb);		/* clear the D/4x read/write block */
  d4xrwb.filehndl = h;		/* handle of file to play */
  d4xrwb.loopsig = 1;		/* terminate on loop signal drop */

  /* play vox file on D/4x channel, normal play back */
  return (xplayf (channel, PM_NORM, &d4xrwb));

}

int
indexplay (int channel, int state)
{
  static MS ms[2] = { 0 };

  ms[0] = PROMPT (PRT.language, state);
  ms[1].offset = DATASTOP;

  clrdtmf (channel);		/* empty out any saved digits */
  clrxrwb (&d4xrwb);		/* clear the D/4x read/write block */
  d4xrwb.filehndl = promptf[PRT.language].ph;	/* handle of file to play */
  if (states[state].termdt)
    d4xrwb.termdtmf = states[state].termdt;	/* cause an event if number terminator is played */
  else
    d4xrwb.maxdtmf = 1;		/* cause an event if max digits */
  d4xrwb.loopsig = 1;		/* terminate on loop signal drop */
  d4xrwb.indexseg = d4getseg ((char *) ms);
  d4xrwb.indexoff = d4getoff ((char *) ms);

  return (xplayf (channel, PM_NDX, &d4xrwb));
}

int
playstate (int channel, int state)
{
  if (maxlanguage > 0)
    return indexplay (channel, state);
  return play (channel, states[state].fh);
}

int
playsingletone (int channel, int freq, int msec)
{
  TN_GEN tone;
  RWB rwb;

  tone.tg_dflag = TN_SINGLE;
  // single tone at 1000 hz played for 1/2 sec at -10db
  dl_bldtngen (&tone, freq, freq, -10, -10, msec);

  clrdtmf (channel);
  clrxrwb (&rwb);
  rwb.indexseg = d4getseg ((char *) &tone);
  rwb.indexoff = d4getoff ((char *) &tone);

  return (dl_playtone (channel, &rwb));
}

/***************************************************************************
 *        NAME: int set_hkstate( channel, state )
 * DESCRIPTION: Set the channel to the appropriate hook status
 *       INPUT: int channel;            - Index into dxinfo structure
 *              int state;              - State to set channel to
 *      OUTPUT: None.
 *     RETURNS: -1 = Error
 *               0 = Success
 *    CAUTIONS: None.
 ***************************************************************************/
int
set_hkstate (int channel, int state)
{
  int rc = 0;

  if ((rc = sethook (channel, (state == H_ONH) ? H_ONH : H_OFFH)) != E_SUCC)
    {
      sprintf (CHMSG, "Failed to change hook state - %d", rc);
      writechan (channel);
    }

  return (rc);
}

void
_vhclose (int handle)
{
  //close file only if it has a valid handle
  if (handle > 0)
    vhclose (handle);
}

void
renrmd (int channel, char *box)
{
  char rmdr[20];
  char tmpf[20];

  strcpy (rmdr, fname (box, RMD));
  strcpy (tmpf, fname (box, TMP));

  _unlink (rmdr);
  rename (tmpf, rmdr);
//      checkrmd(channel); //why am I doing this?
}

void
closermd (int channel)
{
  struct _find_t f;
  register i;

  if (PRT.rh == 0)
    return;			//ignore if no reminder handle assigned

  PRT.rh = 0;			//clear handle
  //find out if reminder is still in use by another port
  for (i = 0; i < chan; i++)
    {
      if (port[i].rh == ADMH)
	return;
    }
  //if not close the file
  _vhclose (ADMH);
  ADMH = 0;			//clear ADMH
  //see if a new reminder message was recorded for this box
  if (_dos_findfirst (fname (USER.box, TMP), _A_NORMAL, &f) == 0)
    renrmd (channel, USER.box);
  if (_dos_findfirst (fname (USER.admin, TMP), _A_NORMAL, &f) == 0)
    renrmd (channel, USER.admin);
}

int
checkgrt (int channel)
{
  long flen;
  //check for file, exit if not found
  if ((PRT.gh = vhopen (fname (USER.box, GRT), RDWR)) == 0)
    return 0;
  flen = filelength (PRT.gh);
  _vhclose (PRT.gh);
  // if the length of the greeting is less than 1 second
  // then play the default greeting
  if (flen < SMPLRATE)
    PRT.gh = 0;
  return PRT.gh;
}

int
checkrmd (int channel, char *admin)	//note this func does not close PRT.rh if found
{
  long flen;
  int h;
  char rname[20], tname[20];

  //default to no handle
  PRT.rh = 0;

//writech("check for admin box");
  if (strlen (admin) != field[fADMIN].l)
    return 0;

  if (ADMH > 0)
    return PRT.rh = ADMH;

  strcpy (rname, fname (admin, RMD));
  strcpy (tname, fname (admin, TMP));

  if (fexist (tname))		//check for new reminder
    {
      _unlink (rname);		//remove old reminder
      rename (tname, rname);	//rename temporary file as reminder
    }

  if ((h = vhopen (rname, READ)) > 0)	//open reminder
    {
      flen = filelength (h);
//writech(" if the length of the reminder is zero then play the default reminder");
      if (flen < SMPLRATE)
	{
	  _vhclose (h);
	  _unlink (rname);
	  return 0;
	}
//writech("indicate that file is now in use");
      return (PRT.rh = ADMH = h);
    }

  return 0;
}

void
checkmsg (int channel)
{
  PRT.boxsize = boxload (channel);
}

void
removesilence (long *size, int h)
{
  //use this function to remove silence after someone hangs up while
  //recording a message
  //NOTE: must have valid file handle to work!!!
  if (scomp != 0)
    return;			// ignore if using SILENCE compression
  *size -= SILENCE;
  if (*size > 0L)
    chsize (h, *size);
}

void
writechanmsg (int channel, unsigned int evtcode, int error, char *extramsg)
{
  char err[10] = "ERR: ";

  if (DID)
    sprintf (CHMSG, "#LINE %-2d: ", channel);
  else if (DIRECT)
    sprintf (CHMSG, "*LINE %-2d: ", channel);
  else
    sprintf (CHMSG, " LINE %-2d: ", channel);

  if (useridx[PRT.box].avail >= ACTIVE && strlen (USER.box) > 0)
    {
      strcat (CHMSG, "(Box ");
      strcat (CHMSG, USER.box);
      strcat (CHMSG, ") ");
    }
  else if (PRT.curr == sPAGE)
    {
      sprintf (s, "(Box %04d) ", currpg);
      strcat (CHMSG, s);
    }

  strcat (CHMSG, states[PRT.curr].desc);	/* describe state */

  if (extramsg != NULL)
    strcat (CHMSG, extramsg);

  if (error)
    {
      strcat (CHMSG, " ");
      strcat (CHMSG, err);
      strcat (CHMSG, d4xerr (error));
    }

  if (debug)
    {
      if (strlen (PRT.dtmf) > 0)
	{
	  strcat (CHMSG, " Digits: ");
	  strcat (CHMSG, PRT.dtmf);
	  strcat (CHMSG, ".");
	}
      strcat (CHMSG, " ");
      strcat (CHMSG, d4xevtmacro (evtcode));	/* what is the macro for this event? */
      strcat (CHMSG, ": ");
      strcat (CHMSG, d4xevtstr (evtcode));	/* event code description */
    }

  writechan (channel);
}

//stats functions
long
numavailboxes (void)
{
  long diskspace;

  // available disk space
  if (diskmb)
    diskspace = (diskmb * MBSECS);
  else
    diskspace = (diskavail (0) / SMPLRATE);
  diskspace -= (((long) numusers * boxsize) - diskused);
  //integer number of boxes that can fit in disk space
  if (!boxsize)
    {
      getsizes ();
    }
  newboxes = diskspace / boxsize;

#ifdef MAXUSERS
  if (newboxes + numusers > MAXUSERS)
    newboxes = MAXUSERS - numusers;
#endif
  //if result is negative make sure to include modulus in result
  if (diskspace % boxsize != 0 && diskspace < 0)
    newboxes--;
  //only 10000 boxes available at any one time
  if (newboxes > MAXBOX)
    newboxes = MAXBOX;
  else if (newboxes < -MAXBOX)
    newboxes = -MAXBOX;
  //newboxes is global variable 
  return newboxes;
}

unsigned long
wastage (void)			//not used right now
{
  //how much space is wasted by DOS in storing files 
  //(see p483 PC-BASED VOICE PROCESSING by Bob Edgar)
  struct diskfree_t d;
  unsigned long size;
  _dos_getdiskfree (0, &d);	//"0" means current disk
  size = ((unsigned long) d.sectors_per_cluster *
	  (unsigned long) d.bytes_per_sector);
  return (size);
}

void
getsizes (void)
{
  boxsize = ((maxmsg + 1) * maxtime);
  /* maxboxes = total boxes that could fit on disk; 
     newboxes = number of boxes that could fit on avail space */
  maxboxes = (disksize (0) / SMPLRATE) / boxsize;
}

long
boxleft (int channel)
{
  long bl, maxsize;
  //check to see if you have surpassed maximum # of messages
  //this is set to the absolute maximum for the system rather than
  //the maxmsg variable which can be less. the maxmsg variable 
  //should be treated more like a dimension than an absolute maximum
  if (USER.nextmsg >= MAXMSG)
    return 0L;
  //find maximum size of a box
  maxsize = (maxmsg * maxtime) - 10L;
  //exit if the actual size of the box has exceeded this
  if (PRT.boxsize < 0 || PRT.boxsize >= maxsize)
    return 0L;
  //get time left for box
  bl = maxsize - PRT.boxsize;
  //if it exceeds maximum amount of time return maxtime 
  if (bl >= maxtime)
    return maxtime;
  //allow for at least a 10 second message
  if (bl < 10L)
    return 0L;
  //otherwise return what is left of box time
  return bl;
}

int
needsbup (int hr)		//check to see if it is time for a backup
{
  int buphr = atoi (backup.hour);

  if (hr == buphr)
    return TRUE;
  else if (bupfreq < MAXHR && bupfreq > 0)
    {
      if (bupfreq < buphr)
	{
	  if (0 == hr % bupfreq)
	    return TRUE;
	}
      else if (buphr == hr % bupfreq)
	return TRUE;
    }
  return FALSE;
}

void
checkautobackup (void)
{
#define MAXTIMER 10
  static int timer = 0;

  if (action != aAUTOBACKUP)
    return;

#ifndef DAEMON
  sprintf (s, "Do back up? Y/N (Enter = Y) (Auto start in %ds.)",
	   MAXTIMER - timer++);
  writemsg (s);
#endif

  if (MAXTIMER < timer)
    {
      action = aNOACTION;
      kbaction = kbBACKUP;
      timer = 0;
    }
#undef MAXTIMER
}

void
checkpage (void)
{
  if (pagingon && allowpage == NOTFOUND && pagestate != NOPAGE)
    {
      getpagechannel ();
    }
}

void
countactiveboxes (char manual)
{
  register int i;
  int maxbox, admin;
  time_t now, offset;
  US u, a;

  time (&now);
  //why is offset wrong by 7 hours?
  offset = (now % DAYSECS) - (7 * 3600);

  //first tally the number of message boxes for admin box
  _lseek (FUSER, 0L, SEEK_SET);
  maxbox = (int) (filelength (FUSER) / sizeof (US));
  for (i = 0; i < maxbox; i++)
    {
      sched ();
      _lseek (FUSER, (long) i * sizeof (US), SEEK_SET);
      _read (FUSER, &u, sizeof (US));
      if (validadmin (u.admin) == TRUE)
	{
	  admin = atoi (u.admin);
	  loaduser (FUSER, admin, &a);
	  if (a.bdupdate < now - offset)
	    {
	      a.boxdays++;
	      saveuser (FUSER, admin, &a);
	    }
	}
    }
  //find all the admin boxes and update the bdupdate timestamp
  //update the screen if the admin box is showing
  for (i = 0; i < MAXBOX; i++)
    {
      if (useridx[i].type == btADMN)
	{
	  sched ();
	  loaduser (FUSER, i, &a);
	  a.bdupdate = now - offset;
	  saveuser (FUSER, i, &a);
	  if (i == curruser)
	    {
	      user.bdupdate = a.bdupdate;
	      user.boxdays = a.boxdays;
	    }
	}
    }

  _lseek (FUSER, 0L, SEEK_SET);
}

void
writestats (void)
{
  char *bx[] = { "box", "boxes" };
  static char allbusy = FALSE;	//are all lines currently busy?

#ifndef DAEMON
  int i, spacing, totallen, xpos, color;
#endif

  struct tm *t;
  time_t now;
  char ts[40];
  int tlen = 0;

  static int second = 60;
  static int minute = 60;

#ifndef DAEMON
  if (scrtxt[STATS].attr == bgWHITE)
    color = bgWHITE;
  else
    color = scrtxt[STATS].attr + fgWHITE;
#endif

  time (&now);
  t = localtime (&now);

  if (hour == NOTFOUND)
    {
      hour = t->tm_hour;
      ttlcalls = cperh[hour];
      ttlmsgs = mperh[hour];
      cwait = chanwait[hour];
      cfull = chansfull[hour];
      cffreq = chansffreq[hour];
      maxwait = 0;
    }

  if (t->tm_sec != second)
    {
      //check back up time if one has been set
      if (!DOINGBACKUP &&
	  strlen (backup.path) > 0 &&
	  strlen (backup.hour) > 0 && strlen (backup.min) > 0)
	{
	  if (t->tm_min != minute)	// check this every minute
	    {
	      minute = t->tm_min;
	      if (needsbup (t->tm_hour))
		{
		  if (t->tm_min == atoi (backup.min))
		    {
		      if (keystate == ksVIEW)
			kbaction = kbBACKUP;
		      else
			action = aAUTOBACKUP;
		    }
		}
	    }
	}

      checkpage ();

      checkautobackup ();

      if (ttlcalls > maxcallshr)	//check for maximum # calls /hour
	{
	  maxcallshr = ttlcalls;
	  maxhr = hour;
	}

      if (ttlmsgs > maxmsgshr)	//check for maximum messages /hour
	{
	  maxmsgshr = ttlmsgs;
	  maxmhr = hour;
	}

      if (telcomax && chansinuse >= telcomax)	//if all telco chans are busy ...
	{
	  if (!allbusy)
	    {
	      cffreq++;
	      allbusy = TRUE;
	    }
	  cwait++;
	  if (cwait > maxwait)
	    maxwait = cwait;
	  cfull++;
	}
      else if (chan)
	{
	  allbusy = FALSE;
	  cwait = 0;
	}

#ifdef TIMELIMIT
      if (now < defuser.acctime - DAYSECS)
	{
	  stopsys ();
	  cls ();
	  strwin (fgWHITE, jCENTER,
		  "System date tampered with during program execution!");
	  _settextposition (24, 1);
	  _settextcursor (LINECURSOR);
	  exit (0);
	}
      else
	defuser.acctime = now;
#endif

      second = t->tm_sec;

      if (t->tm_hour != hour)
	{
	  //check for any pages and initiate paging
	  if (pgstart != NOTFOUND && pagestate != PAGEALL)
	    {
	      pagestate = PAGEALL;
	    }
	  //load stats arrays
	  cperh[hour] = ttlcalls;
	  mperh[hour] = ttlmsgs;
	  chanwait[hour] = maxwait;
	  chansfull[hour] = cfull;
	  chansffreq[hour] = cffreq;
	  pgcalls[hour] = pagecalls;
	  //reset channels once per hour 
	  //if they were not busy (to prevent channel from "sticking open")
	  if (cffreq == 0)
	    resetports ();
	  //check for any errant inuse boxes
	  checkeveryboxinuse ();
	  //reset variables
	  hour = t->tm_hour;
	  if (hour == countactivehour)
	    countactiveboxes (FALSE);
	  pagecalls = ttlmsgs = ttlcalls = 0;
	  maxwait = cwait = cfull = cffreq = 0;
	  allbusy = FALSE;
	  //check to see if expiry date has been exceeded if set
	  if (expired ())
	    expexit ();
	}

#ifndef DAEMON
      /* actual number of boxes currently in use */
      statlen[USERS] = sprintf (statstr[USERS], "%d %s (%i new avail)",
				(int) numusers,
				bx[(numusers == 1 ? 0 : 1)], (int) newboxes);

      totallen = 0;
      for (i = 0; i <= MAXSTAT; i++)
	totallen += statlen[i];

      if (maxiteration)
	spacing = (79 - totallen) / MAXSTAT;
      else
	spacing = (79 - totallen) / (MAXSTAT - 1);

      xpos = 2;

      /* write stats */
      for (i = 0; i <= MAXSTAT; i++)
	{
	  //skip if there is no "repeats"
	  if (!maxiteration && i == ITERS)
	    continue;
	  writestr (xpos,	/* x pos */
		    STATS_TOP,	/* y pos */
		    0,		/* start at this char in string */
		    statlen[i] + spacing,	/* length */
		    color, statstr[i]);	/* string */
	  if (i == MAXSTAT - 1)
	    xpos = 80 - statlen[MAXSTAT];
	  else
	    xpos += (statlen[i] + spacing);
	}

      /* write time */
      if (maintenance)
	tlen = sprintf (ts, "MAINTENANCE %s", timestr (&now));
      else
	tlen = sprintf (ts, "            %s", timestr (&now));
      writestr (80 - tlen,	/* x pos */
		TITLE_TOP,	/* y pos */
		0,		/* start at this char in string */
		tlen,		/* length */
		0, ts);		/* string */
#endif
    }
}

void
loadpageq (void)
{
  int h;

  if ((h = _open (PGQFILE, _O_BINARY | _O_RDWR)) >= 0)
    {
      if (filelength (h) < sizeof (page) + (3 * sizeof (int)))
	{
	  _close (h);
	  _unlink (PGQFILE);
	  return;
	}
      _read (h, page, sizeof (page));
      _read (h, &pagestate, sizeof (int));
      _read (h, &pgstart, sizeof (int));
      _read (h, &currpg, sizeof (int));
      _close (h);
    }
}

void
savepageq (void)
{
  int h;

  if ((h = _open (PGQFILE, _O_BINARY | _O_RDWR)) < 0)
    {
      if ((h =
	   _open (PGQFILE, _O_RDWR | _O_BINARY | _O_CREAT,
		  _S_IWRITE | _S_IREAD)) < 0)
	return;
    }
  _write (h, page, sizeof (page));
  _write (h, &pagestate, sizeof (int));
  _write (h, &pgstart, sizeof (int));
  _write (h, &currpg, sizeof (int));
  _close (h);
}

char *
cperhstr (int sv, int cperh[], int ttlcalls, char *formatstr, char format)
{
  int i, min, minh, max, maxh, total, av;
  static char s[800];
  char a[21], cph[81];
  struct tm *t;
  time_t now;

  time (&now);
  t = localtime (&now);

  av = minh = maxh = total = max = 0;
  min = 10000;

  if (cperh[hour] < ttlcalls)
    cperh[hour] = ttlcalls;
  if (format == fMINSEC)
    sprintf (s, formatstr, ttlcalls / 60, ttlcalls % 60, t->tm_hour,
	     t->tm_min);
  else
    sprintf (s, formatstr, ttlcalls, t->tm_hour, t->tm_min);
  for (i = 0; i < MAXHR; i++)
    {
      if (sv >= svFFREQ)
	{
	  if (min > cperh[i] && cperh[i] > 0)
	    {
	      min = cperh[i];
	      minh = i;
	    }
	}
      else
	{
	  if (min > cperh[i])
	    {
	      min = cperh[i];
	      minh = i;
	    }
	}

      if (max < cperh[i])
	{
	  max = cperh[i];
	  maxh = i;
	}
      total += cperh[i];

      if (i % 4 == 0)
	strcat (s, "|");
      strcpy (cph, "");
      if (cperh[i] == 0)
	{
	  sprintf (a, " %2dh:       ", i);
	  strcat (cph, a);
	}
      else if (format == fMINSEC)
	{
	  sprintf (a, " %2dh: %2d:%02d ", i, cperh[i] / 60, cperh[i] % 60);
	  strcat (cph, a);
	}
      else
	{
	  sprintf (a, " %2dh: %-5d ", i, cperh[i]);
	  strcat (cph, a);
	}

      strcat (s, cph);
    }

  if (min > max)
    min = max;

  switch (sv)
    {
    case svFULL:
      sprintf (cph,
	       " | Min = (%2dh)%2d:%02d Max = (%2dh)%2d:%02d Ttl = %3d:%02d ",
	       minh, min / 60, min % 60, maxh, max / 60, max % 60, total / 60,
	       total % 60);
      break;
    case svWAIT:
      total = (ttlffreq > 0 ? ttlffreq : MAXHR);	//avoid divide by zero error
      av = ttlfull / total;	//get actual average
      if (((ttlfull * 10) / total) % 10 >= 5)
	av++;			//round up
      sprintf (cph,
	       " | Min = (%2dh)%2d:%02d Max = (%2dh)%2d:%02d Avg = %3d:%02d ",
	       minh, min / 60, min % 60, maxh, max / 60, max % 60, av / 60,
	       av % 60);
      break;
    default:
      sprintf (cph, " | Min = (%2dh)%-5d Max = (%2dh)%-5d Total = %-6d ",
	       minh, min, maxh, max, total);
    }

  strcat (s, cph);

  return s;
}

void
showcperh (int sv, int cperh[], int ttlcalls, char *formatstr, char format)
{
  char *str;
  int nl, ml;

  showingcperh = TRUE;
  str = cperhstr (sv, cperh, ttlcalls, formatstr, format);
  _settextcursor (NOCURSOR);
  getlinestats (str, &nl, &ml);
  create_win (middle (80, ml),	/* x position */
	      EDITHELP_TOP + 1,	/* y position */
	      ml,		/* width */
	      nl,		/* height */
	      HWINATTR,		/* color */
	      bsSINGLE,		/* style */
	      str);		/* what to put there */
  writemsg
    ("STATS:  F2/PGDN/PGUP/HOME/END: Select screen. Alt-T: Telco ports. ESC: exit.");
}

long
countpaycodes (long *usedpc, long *unusedpc)
{
  long i, ttlpc, uupc, upc;
  PAY pc;

  uupc = upc = 0L;

  ttlpc = filelength (pfh) / sizeof (PAY);
  _lseek (pfh, 0L, SEEK_SET);
  for (i = 0L; i < ttlpc; i++)
    {
      if (i % 100 == 0)
	sched ();
      if (_read (pfh, &pc, sizeof (PAY)) == -1)
	{
	  break;
	}
      if (!pc.used)
	uupc++;
      else
	upc++;
    }
  _lseek (pfh, 0L, SEEK_SET);

  *unusedpc = uupc;
  *usedpc = upc;
  if (i < ttlpc)
    return NOTFOUND;
  return ttlpc;
}

void
showpaycodestats (void)
{
  register j;
  int nl, ml, avail = 0;
  long usedpc = 0, unusedpc = 0, ttlpc = 0;

  if (pfh == NOTFOUND)
    {
      sprintf (s, "Pay code file %s not found.", PAYCODEFILE);
    }
  else
    {
      if (!countpaycodes (&usedpc, &unusedpc))
	{
	  sprintf (s, "Error reading %s! Pay code stats aborted.",
		   PAYCODEFILE);
	  writemsg (s);
	  return;
	}

      for (j = 0; j < boxrange; j++)
	{
	  if (useridx[boxrangestart + j].avail == INACTIVE)
	    avail++;
	}

      ttlpc = usedpc + unusedpc;
      sprintf (s,
	       " PAYCODE STATISTICS: | Total number of paycodes: %ld. Unused: %ld (%ld%%).| Alt-C: add new paycodes.",
	       ttlpc, unusedpc, ttlpc ? (unusedpc * 100L) / ttlpc : 0L,
	       PAYCODECSV);
    }

  showingcperh = TRUE;
  _settextcursor (NOCURSOR);
  getlinestats (s, &nl, &ml);
  create_win (middle (80, ml),	/* x position */
	      EDITHELP_TOP + 1,	/* y position */
	      ml,		/* width */
	      nl,		/* height */
	      HWINATTR,		/* color */
	      bsSINGLE,		/* style */
	      s);		/* what to put there */
  writemsg (PCSTATSTR);
}

int
openpcfile (void)
{
  //back up the old file
  if ((pcsvh = _open (PAYCODECSV, _O_RDWR | _O_TEXT)) != -1)
    {
      sprintf (s, "Copying %s to %s.", PAYCODECSV, PAYCODEBAK);
      writemsg (s);
      _close (pcsvh);
      _unlink (PAYCODEBAK);
      rename (PAYCODECSV, PAYCODEBAK);
    }

  if ((pcsvh =
       _open (PAYCODECSV, _O_RDWR | _O_TEXT | _O_CREAT,
	      _S_IREAD | _S_IWRITE)) == -1)
    {
      sprintf (s, "Unable to open %s. Press a key...", PAYCODECSV);
      writemsg (s);
      return FALSE;
    }

  return TRUE;
}

int
loadpcfile (void)
{
  char csvstr[sizeof (PAY) + 6 + (2 * DATESORTLEN)];
  PAY pc = { 0 };
  int len;
  long maxrec, i;

  _lseek (pfh, 0L, SEEK_SET);
  _lseek (pcsvh, 0L, SEEK_SET);

  maxrec = filelength (pfh) / sizeof (PAY);
  for (i = 0L; i <= maxrec; i++)
    {
      len = paycsvstr (csvstr, pc);
      if (_write (pcsvh, csvstr, len) == -1)
	{
	  sprintf (s, "Error (%s) writing to %s. Press a key...",
		   strerror (errno), PAYCODECSV);
	  _close (pcsvh);
	  writemsg (s);
	  return FALSE;
	}
      if (_read (pfh, &pc, sizeof (PAY)) == -1)
	{
	  sprintf (s, "Error (%s) reading from %s. Press a key...",
		   strerror (errno), PAYCODECSV);
	  _close (pcsvh);
	  writemsg (s);
	  return FALSE;
	}
    }
  _close (pcsvh);
  sprintf (s, "Copied %ld paycodes to %s.", maxrec, PAYCODECSV);
  writemsg (s);
  return TRUE;
}

int
writepaycodes (void)
{
  sprintf (s, "Writing paycodes to %s.", PAYCODECSV);
  writemsg (s);
  if (openpcfile ())
    {
      return loadpcfile ();
    }
  else
    return FALSE;
}

/* put current screen contents into buffer */
void
saveudisplay (unsigned char far * scrnbuff)
{
  unsigned char far *scrn;
  scrn = scrnaddress (1, USER_TOP);

  _fmemmove (scrnbuff, scrn,
	     (sizeof (CELL) * (MAXHEIGHT - USER_TOP) * MAXWIDTH) - 1);
}

/* write to screen from buffer */
void
loadudisplay (unsigned char far * scrnbuff)
{
  unsigned char far *scrn;
  scrn = scrnaddress (1, USER_TOP);

  _fmemmove (scrn, scrnbuff,
	     (sizeof (CELL) * (MAXHEIGHT - USER_TOP) * MAXWIDTH) - 1);
}

void
getpcstr (void)
{
  static int areapos[] = { 7, 32, 47, 60 };

  sprintf (pcstr,
	   "Tag:             Start value:      Quantity:      Months:      ");
  sprintf (pcscan,
	   "Tag: %%8d Start value: %%4d Quantity: %%4d Months: %%4d ");
  pcpos = areapos;
}

int
checkpcparameters (PAYP * pp)
{
  register i;
  long val;
  int len;

  for (i = 0; i < PCMAX; i++)
    {
      len = (i == 0 ? (2 * BOXLEN) : BOXLEN);
      if (strncmp (pcstr + pcpos[i] - 2, "    ", len) == 0)
	{
	  writemsg ("One of the fields is empty.");
	  return FALSE;
	}
      val = atol (pcstr + pcpos[i] - 2);
      switch (i)
	{
	case pcTAG:
	  pp->tag = val;
	  break;
	case pcSTART:
	  pp->start = (int) val;
	  break;
	case pcNUMTOADD:
	  pp->numtoadd = (int) val;
	  break;
	case pcMONVAL:
	  pp->monval = (int) val;
	  break;
	}
    }

  if (pp->numtoadd <= 0 || pp->monval <= 0)
    {
      writemsg ("You are missing some parameters.");
      return FALSE;
    }

  if (pp->numtoadd + pp->start > MAXBOX)
    {
      writemsg ("You are trying to add too many paycodes.");
      return FALSE;
    }
}

int
loadnewpaycodes (PAYP * pp)
{
  register i;
  PAY pc = { 0 }, nullpc =
  {
  0};
  int rn;
  time_t now;

  //go to end of file before starting
  _lseek (pfh, 0, SEEK_END);

  for (i = pp->start; i < pp->start + pp->numtoadd; i++)
    {
      pc = nullpc;
      rn = rand ();
      rn %= 10000;		//lop of 1st digit of 5 digit number
      sprintf (pc.paycode, "%08ld%04d%04d", pp->tag, i, rn);
      pc.paycode[PAYCODELEN] = '\0';
      pc.value = pp->monval;
      pc.created = time (&now);
      if (_write (pfh, &pc, sizeof (PAY)) == -1)
	{
	  sprintf (s, "Error writing to %s!", PAYCODEFILE);
	  writemsg (s);
	  return FALSE;
	}
      _commit (pfh);
    }

}

int
addpaycodes (void)
{
  PAYP pp;
  if (checkpcparameters (&pp))
    return loadnewpaycodes (&pp);
  else
    return FALSE;
}

void
editpaycode (unsigned char *key, char *zero)
{
  static int nochange = FALSE, startpos = 0, pos = 0, spos = 0, keep =
    FALSE, bort = FALSE;
  register i;
  int len;
  //create default text position
  if (!pos)
    {
      spos = 0;
      pos = pcpos[spos];
    }
  len = (spos == 0 ? (2 * BOXLEN) : BOXLEN);
  //save or discard string
  if (keep || bort)
    {
      if (!nochange)
	{
	  if (!*zero)
	    switch (tolower (*key))
	      {
	      case ENTER:
	      case 'y':
		if (keep)
		  {
		    if (addpaycodes ())
		      writemsg ("Created paycodes. Press a key...");
		    else
		      {
			nochange = TRUE;
			return;
		      }
		  }
		else if (bort)
		  writemsg ("Aborted paycode create. Press a key...");
		break;
	      case 'n':
		if (keep)
		  {
		    writemsg ("Aborted paycode create. Press a key...");
		    break;
		  }
	      default:
		nochange = TRUE;
	      }
	  else
	    nochange = TRUE;

	  bort = keep = FALSE;
	}

      if (nochange)
	{
	  nochange = FALSE;
	  _settextcursor (cursortype);
	  writemsg (pcstr);
	}
      else
	kbfunc = dokey;
      return;
    }
  //edit string   
  if (*zero)
    switch (*key)
      {
      case LEFT:
	if (pos > pcpos[spos])
	  pos--;
	else
	  {
	    if (spos > 0)
	      spos--;
	    else
	      spos = EPCMAX;
	    pos = pcpos[spos];
	  }
	break;
      case RIGHT:;
	if (pos < pcpos[spos] + len - 1)
	  pos++;
	else
	  {
	    if (spos < EPCMAX)
	      spos++;
	    else
	      spos = 0;
	    pos = pcpos[spos];
	  }
	break;
      case ShiftTAB:
	if (spos > 0)
	  {
	    spos--;
	  }
	else
	  spos = EPCMAX;
	pos = pcpos[spos];
	break;
      case HOME:
	if (spos != 0)
	  {
	    spos = 0;
	    pos = pcpos[spos];
	  }
	break;
      case END:
	if (spos != EPCMAX)
	  {
	    spos = EPCMAX;
	    pos = pcpos[spos];
	  }
	break;
      case AltY:
	getpcstr ();
	spos = 0;
	pos = pcpos[spos];
	writemsg (pcstr);
	break;
      case INS:;
      case DEL:
	for (i = pos - 2; i < pcpos[spos] + len - 3; i++)
	  {
	    pcstr[i] = pcstr[i + 1];
	  }
	pcstr[i] = SPACE;
	writemsg (pcstr);
	break;
      case F1:
      case AltH:
	break;
      }
  else
    switch (*key)
      {
      case CtrlY:
	for (i = pcpos[spos] - 2; i < pcpos[spos] + len - 2; i++)
	  pcstr[i] = SPACE;
	pos = pcpos[spos];
	writemsg (pcstr);
	break;
      case BKSPC:
	if (pos > pcpos[spos])
	  pos--;
	for (i = pos - 2; i < pcpos[spos] + len - 3; i++)
	  pcstr[i] = pcstr[i + 1];
	pcstr[i] = SPACE;
	writemsg (pcstr);
	break;
      case TAB:
	if (spos < EPCMAX)
	  {
	    spos++;
	  }
	else
	  spos = 0;
	pos = pcpos[spos];
	break;
      case ENTER:
	sprintf (s, "Create paycodes now? Y/N (Enter = Y)");
	writemsg (s);
	_settextcursor (NOCURSOR);
	keep = TRUE;
	break;
      case ESC:
	writemsg ("Abort paycode create? Y/N (Enter = Y)");
	_settextcursor (NOCURSOR);
	bort = TRUE;
	break;
      default:
	pos -= 2;
	edit (2,		//xpos of window
	      HELP_TOP,		//ypos of window
	      MSGLEN,		//size of window 
	      &startpos,	//first character of the string you want to show
	      &pos,		//your position in the string
	      pclen,		//actual length of string
	      fgWHITE + scrtxt[HELP].attr,	//color
	      *key, *zero,	//leading zero?
	      NUMERIC,		//keyboard filter
	      pcstr);		//pointer to string
	pos += 2;
      }
  if (pos > pcpos[spos] + len - 1)
    pos--;
  _settextposition (HELP_TOP, pos);
}

int
loadmkpc (void)
{
  if (strlen (pcstr) == 0)
    getpcstr ();
  writemsg (pcstr);
  pclen = strlen (pcstr);
  typemode = OVERWRITE;
  cursortype = BLOCKCURSOR;
  _settextcursor (cursortype);
  _settextposition (HELP_TOP, pcpos[0]);
  kbfunc = editpaycode;
  return TRUE;
}

void
getpass (unsigned char *key, char *zero)
{
  static int startpos = 0, pos = 0;

  if (!*zero)
    switch (*key)
      {
      case ESC:
	writemsg ("Action aborted. Press a key...");
	kbfunc = dokey;
	break;
      case ENTER:
	if (strcmp (password, checkpass) == 0)
	  loadmkpc ();
	else
	  {
	    writemsg ("Invalid password! Press a key...");
	    kbfunc = dokey;
	  }
	break;
      default:
	editkeystate (ksPASSWORD, fMSG, &startpos, &pos, *key, *zero);

      }

  if (kbfunc != getpass)
    {
      startpos = pos = 0;
      strcpy (checkpass, "");
      if (kbfunc != editpaycode)
	_settextcursor (NOCURSOR);
    }
}

int
loadgetpass (void)
{
  writemsg (msg[ksPASSWORD]);
  typemode = OVERWRITE;
  cursortype = BLOCKCURSOR;
  _settextcursor (cursortype);
  editstart = strlen (msg[ksPASSWORD]) + 2;
  _settextposition (HELP_TOP, editstart);
  kbfunc = getpass;
  return TRUE;
}

int
selectstatscrn (unsigned char key)
{
  static int prevstatscr, statscr = NOTFOUND, getpc = FALSE;
  int nmperh[MAXHR] = { 0 };

  prevstatscr = statscr;

  if (statscr == NOTFOUND)
    {
      saveudisplay (scrnbuff);
      statscr = svFIRST;
    }
  else
    switch (key)
      {
      case 0:
	break;			//check if key has been cleared
      case HOME:
	statscr = svFIRST;
	break;
      case END:
	statscr = svLAST;
	break;
      case F2:
      case PGDN:
	if (statscr < svLAST)
	  statscr++;
	break;
      case PGUP:
	if (statscr > svFIRST)
	  statscr--;
	break;
      case AltC:
	if (statscr == svPAYCODE)
	  {
	    if (strlen (password))
	      getpc = loadgetpass ();
	    else
	      getpc = loadmkpc ();
	  }
	return TRUE;
      case AltT:
	if (statscr != svPAYCODE)
	  {
	    showtelco = TRUE;
	    telcoscrn (AltT, TRUE);
	  }
	return TRUE;
/*			case AltP:
	if (statscr == svPAYCODE)
		writepaycodes();
	return TRUE;	
*/
      case ESC:
	if (getpc)
	  getpc = FALSE;
	else
	  {
	    statscr = NOTFOUND;
	    return FALSE;
	  }
      }

  loadudisplay (scrnbuff);
  switch (statscr)
    {
    case svCALLS:
      showcperh (statscr, cperh, ttlcalls,
		 " Calls/hr in last 24hrs (%d calls at %d:%02d):", fNUMBER);
      break;
    case svMSGS:
      showcperh (statscr, mperh, ttlmsgs,
		 " Msgs/hr in last 24hrs (%d msgs at %d:%02d):", fNUMBER);
      break;
    case svPGCALLS:
      showcperh (statscr, pgcalls, pagecalls,
		 " Page calls/hr (%d calls at %d:%02d):", fNUMBER);
      break;
    case svFFREQ:
      showcperh (statscr, chansffreq, cffreq,
		 " Freq all telco lines full/hr (%d at %d:%02d):", fNUMBER);
      break;
    case svFULL:
      showcperh (statscr, chansfull, cfull,
		 " Time all telco lines full/hr (%2d:%02d at %d:%02d):",
		 fMINSEC);
      break;
    case svWAIT:
      ttlfull = sumhrarray (chansfull);
      ttlffreq = sumhrarray (chansffreq);
      showcperh (statscr, chanwait, maxwait,
		 " Max free telco line wait/hr (%2d:%02d at %d:%02d):",
		 fMINSEC);
      break;
    case svPAYCODE:
      showpaycodestats ();
      break;
    }

  return TRUE;
}

//save retrieve telco setup
void
gettelcomax (void)
{
  register i;

  telcomax = 0;

  for (i = 0; i < chan; i++)
    if (telcochans[i])
      telcomax++;
}

void
gettelcochans (void)
{
  int h;

  if ((h = _open (TELCOFILE, _O_RDONLY)) != NOTFOUND)
    {
      _read (h, telcochans, MAXCHAN * sizeof (char));
      _close (h);
    }

  gettelcomax ();
}

void
savetelcochans (void)
{
  int h;

  if ((h = _open (TELCOFILE, _O_WRONLY)) != NOTFOUND)
    {
      _write (h, telcochans, MAXCHAN * sizeof (char));
      _close (h);
    }
  else
    if ((h =
	 _open (TELCOFILE, _O_RDWR | _O_BINARY | _O_CREAT,
		_S_IWRITE | _S_IREAD)) != NOTFOUND)
    {
      _write (h, telcochans, MAXCHAN * sizeof (char));
      _close (h);
    }
}

//show telco lines
char *
telcostr (void)
{
  register i;
  static char s[800];
  char cel[40];
  char tstate;

  strcpy (s, " Select port. Use SPACE to flag telco ports. ");
  for (i = 0; i < MAXCHAN; i++)
    {
      if (i % CELMOD == 0)
	strcat (s, "|");
      if (i < chan)
	tstate = telcochans[i];
      else
	tstate = tsNOTAVAIL;

      sprintf (cel, " %2d: %s ", i + 1, tstatestr[tstate]);

      strcat (s, cel);
    }

  celwid = strlen (cel);

  return s;
}

void
writetelcomsg (void)
{
  writemsg
    ("TELCO PORTS:  UP/DOWN/HOME/END: Select port. SPACE: Toggle port. ESC: exit.");
}

// only use this function when you are initializing
// the telco screen as it stores the previous screen
// use writetelcomsg() to clear bottom line of messages
// instead.
void
maketelcoscrn (void)
{
  int w, h;
  char *tstr;

  remakescrn (NULL, NULL);
  _settextcursor (NOCURSOR);

  tstr = telcostr ();

  getlinestats (tstr, &h, &w);
  celleft = middle (80, w);
  celtop = EDITHELP_TOP + 2;
  create_win (celleft,		/* x position */
	      celtop - 1,	/* y position */
	      w,		/* width */
	      h,		/* height */
	      CELATTR,		/* color */
	      bsSINGLE,		/* style */
	      tstr);		/* what to put there */
  writetelcomsg ();
}

int
celx (int c)
{
  return CELOFFS + celleft + ((c % CELMOD) * celwid);	//get x position of cel
}

int
cely (int c)
{
  return celtop + (c / CELMOD);	//get y position of cel
}

void
showcel (int c, int attr)
{
  if (c >= chan)
    return;			//ignore if invalid value

  writestr (celx (c),		//x pos
	    cely (c),		//y pos
	    0,			//start string at start of string
	    celwid - CELOFFS,	//size of window
	    attr,		//color
	    tstatestr[telcochans[c]]	//string
    );
}

void
telcoscrn (unsigned char key, char zero)
{
  static int c = 0, refresh = 0;
  char directmsg[] =
    "You must restart the system for this change to take effect!";

  if (key == AltT && zero == TRUE)
    maketelcoscrn ();
  else
    {
      if (refresh)
	{
	  writetelcomsg ();
	  refresh = 0;
	}
      showcel (c, CELATTR);
      if (zero)
	switch (key)
	  {
	  case UP:
	    if (c >= CELMOD)
	      c -= CELMOD;
	    break;
	  case DOWN:
	    if (c <= (chan - CELMOD - 1))
	      c += CELMOD;
	    break;
	  case LEFT:
	    if (c > 0)
	      c--;
	    break;
	  case RIGHT:
	    if (c < chan - 1)
	      c++;
	    break;
	  case HOME:
	    c = 0;
	    break;
	  case END:
	    c = chan - 1;
	    break;
	  }
      else
	switch (key)
	  {
	  case SPACE:
	    if (telcochans[c] == tsDIRECT)
	      {
		writemsg (directmsg);
		refresh = 1;
	      }
	    if (telcochans[c] < tsDIRECT)
	      telcochans[c]++;
	    else
	      telcochans[c] = 0;
	    if (telcochans[c] == tsDID || telcochans[c] == tsDIDWINK)
	      chmsg[c][0] = '#';
	    else if (telcochans[c] == tsDIRECT)
	      {
		writemsg (directmsg);
		refresh = 1;
		chmsg[c][0] = '*';
	      }
	    else
	      chmsg[c][0] = SPACE;
	    writechan (c + 1);
	    break;
	  case ESC:
	    showtelco = FALSE;
	    gettelcomax ();
	    savetelcochans ();
	    break;
	  }
    }

  showcel (c, CELHILITE);
}

//various port utilities
void
updatechanmsgs (void)
{
  int channel;
  for (channel = 1; channel <= chan; channel++)
    writechan (channel);
}

void
simulatecalls (void)
{

}

int
portstate (int state)
{
  register channel;
  for (channel = 1; channel <= chan; channel++)
    if (PRT.curr == state)
      return TRUE;
  return FALSE;
}

int
portsidle (void)
{
  register channel;
  for (channel = 1; channel <= chan; channel++)
    if (PRT.curr > sWTKEY && PRT.curr != sONHK)
      return FALSE;
  return TRUE;
}

void
resetports (void)
{
  register channel;
  for (channel = 1; channel <= chan; channel++)
    if (PRT.curr == sWTRING)
      {
	reset (channel);
	PRT.curr = sRESET;
      }
}

void
getpagechannel (void)
{
  time_t now;
  register channel;

  for (channel = 1; channel <= chan; channel++)
    {
      time (&now);
      if (pagestate != NOPAGE &&
	  now - PRT.lastaccess > 30 && ISTELCO && PRT.curr == sWTRING)
	{
	  allowpage = channel;
	  PRT.curr = sOFFHK;
	  offhk (channel);
	  return;
	}
    }
}

void
resetchannel (int channel)
{
  //clear channel string
  strcpy (channelstr, "1");
  //reset channel without waiting for it to hang up
  if (PRT.curr == sWTRING)
    {
      reset (channel);
      PRT.curr = sRESET;
    }
  else
    {
      closermd (channel);
      _vhclose (PRT.gh);
      _vhclose (PRT.mh);
      PRT.curr = sONHK;
    }
}

int
resetinuse (int box)
{
  register channel;

  switch (useridx[box].avail)
    {
    case ACTIVE:
      return ACTIVE;
    case NOTFOUND:
    case INACTIVE:
      return INACTIVE;
    case ONHOLD:
      return ONHOLD;
    default:
      if (useridx[box].avail >= INUSE)
	{
	  for (channel = 1; channel <= chan; channel++)
	    if (PRT.box == box)
	      return INUSE;
	  useridx[box].avail = ACTIVE;
	  return ACTIVE;
	}
      else
	return INACTIVE;
    }
}

void
regnow (void)
{
  char s[1000];
  _settextcursor (NOCURSOR);
  cls ();

  strcpy (s, REGNOW);
  strcat (s, ADDRESS);
  strcat (s, PRESSANY);
  strwin (fgWHITE, jCENTER, s);
  getch ();

  cls ();
  _settextcursor (LINECURSOR);
}

//backup and purge (purge not currently used because of data problems)
void
loadbackupparams ()
{
  int h;
  size_t count;

  if ((h = vhopen (BACKUPFILE, RDWR)) == 0)
    return;

  if ((count = (size_t) filelength (h)) >= sizeof (BS))
    count = sizeof (BS);

  _read (h, &backup, count);

  _vhclose (h);

  purgedays = atol (backup.purgedays);
  purgedays *= DAYSECS;
  bupfreq = atoi (backup.frequency);
  bupremind = atol (backup.remind);
  bupremind *= DAYSECS;
  bupdeactivate = atol (backup.deactivate);
  bupdeactivate *= DAYSECS;
  savebup = backup;
}

void
savebackupparams ()
{
  int h;

  if ((h = openfile (BACKUPFILE)) == -1)
    return;

  _write (h, &savebup, sizeof (BS));

  _vhclose (h);
}

int
purgefile (struct _find_t ft)
{
  int len;
  int nDay = ft.wr_date & 0x1f;
  int nMonth = ((ft.wr_date >> 5) & 0x0f) - 1;
  int nYear = (ft.wr_date >> 9) + 80;
  time_t now, then;
  struct tm *t;

  //must be doing a full backup
  if (kbaction != kbBACKUP)
    return FALSE;

  //purgedays must be a whole number value
  if (purgedays <= 0)
    return FALSE;

  //must be a message file
  len = strlen (ft.name);
  if (!(len == 8 && strcmp (ft.name + 4, MSG) == 0) &&
      !(len == 10 && strcmp (ft.name + 6, MSG) == 0))
    return FALSE;

  time (&now);

  then = now - purgedays;
  t = localtime (&then);
  if (t->tm_year > nYear)
    return TRUE;
  else if (t->tm_year == nYear && t->tm_mon > nMonth)
    return TRUE;
  else if (t->tm_year == nYear && t->tm_mon == nMonth && t->tm_mday >= nDay)
    return TRUE;

  return FALSE;

}

int
fnameok (US u)
{
  char *fn;
  if ((fn = fname (u.box, "*.*")) == NULL)
    {
      sprintf (s, "ERROR: %s invalid.", u.box);
      writemsg (s);
      bupstage = bINIT;
      return FALSE;
    }

  strcpy (s, fn);
  return TRUE;
}

int
checkdrvltr (char *path)
{
  unsigned char drvltr;

  if (path[1] == ':')		//check for drive letter
    {
      drvltr = toupper (path[0]);
      switch (drvltr)
	{
	case 'A':
	case 'B':
	  if (kbaction == kbBACKUP)
	    {
	      writemsg ("ERROR: attempting full back up to floppy drive!");
	      return bAUTOABORT;
	    }
	  else
	    {
	      sprintf (s,
		       "Please insert disk in %c: and press a key. Press ESC to abort...",
		       drvltr);
	      writemsg (s);
	      return bWAITDISK;
	    }
	}
    }
  return bSTART;
}

int
checkboxinuse (int box)
{
  int channel;
  US u;

  channel = useridx[box].avail - INUSE;
  // check for errors here - it does happen!
  // PRT.box should equal the box if that port is being used by the box
  if (PRT.box != box || box == 0)	// having problems with 0000 being randomly in use!
    {
      // if it isn't really in use find out whether it is active or not
      if (loaduser (FUSER, box, &u) == NOTFOUND)
	useridx[box].avail = NOTFOUND;
      else
	useridx[box].avail = u.active;
      return TRUE;
    }
  else
    return FALSE;
}

// check every box to see if it is in use 
// change it if it isn't
void
checkeveryboxinuse (void)
{
  register i;
  for (i = 0; i < MAXBOX; i++)
    {
      if (useridx[i].avail > INUSE)
	checkboxinuse (i);
    }
}

int
boxavail (int box, char *boxstr)
{

  if (useridx[box].avail > INUSE)
    {
      return checkboxinuse (box);
    }
  else if (useridx[box].avail == INUSE)
    return FALSE;

  if (useridx[box].avail == ACTIVE)
    useridx[box].avail = INUSE;
  return TRUE;
}

void
dobackup (char firstmask, char *bpath)
{
#define BUPDIV 8
#define U uuserptr
#define UBUFF ubuffptr
#define MAXMASK 1

  typedef union utype
  {
    US user;
    char ubuff[sizeof (US)];
  }
  UT;

  static time_t now;
  static struct _find_t m;
  static UT u;
  static UT *uptr;
  static US *uuserptr;
  static char *ubuffptr;
  static char boxmask[20];
  static int utotal;
  static int bupchunk = sizeof (US) / BUPDIV;
  int ucount;			// for copying user record
  static char bupcsvstr[sizeof (US) + 1];
  static int doserror, huser, hsrc, htarg, csvlen;
  static int hscsv, htcsv;
  static long count;
  static int box = NOTFOUND, bmsg = 0;
  static char nextmsg;
  static char targmask[DOSPATHLEN];
  static char maskcount = 0;
  static char mask[][MASKLEN + 1] = { DATMASK, CSVMASK };
  static char prevavail;	//keep track of previous status of box
  //static char srcmask[DOSPATHLEN];
  static int targlen;
  unsigned long flen;
  //paycode csv variables
  static long maxpayrec, currpayrec;
  static PAY pc = { 0 };
  PAY nullpc = { 0 };
  char pcsvstr[sizeof (PAY) + 6 + (2 * DATESORTLEN)];
  int paycsvlen;

  switch (bupstage)
    {
    case bIDLE:
      if (DOINGBACKUP)
	{
	  maskcount = firstmask;
	  bupstage = checkdrvltr (bpath);
	}
      else
	return;
      break;
      //WAIT FOR FLOPPY TO BE PUT IN SLOT
    case bWAITDISK:
      if (keypressed)
	{
	  bupstage = bSTART;	//bSTART is defined in util.h
	}
      break;
      //section for copying all data files except users.dat
      //now modified to copy csv files as well
      //this section update the paycode csv file first before updating it
    case bPAYCSVOPEN:
      if (!openpcfile ())
	{
	  bupstage = bDATINIT;
	  break;
	}
      sprintf (s, "Writing paycodes to %s.", PAYCODECSV);
      writemsg (s);
      strcpy (pcsvstr, "");
      pc = nullpc;
      _lseek (pcsvh, 0L, SEEK_SET);
      maxpayrec = filelength (pfh) / sizeof (PAY);
      currpayrec = 0;
      bupstage = bPAYCSVCOPY;
      break;
    case bPAYCSVCOPY:
      if (currpayrec <= maxpayrec)
	{
	  // never assume you know where you are in the paycode file!
	  paycsvlen = paycsvstr (pcsvstr, pc);
	  if (paycsvlen < 0)
	    {
	      sprintf (pcsvstr,
		       "ERROR: damaged record. Fix %s with LLPAYFIX.EXE.\n",
		       PAYCODEFILE);
	      paycsvlen = strlen (pcsvstr);
	    }
	  _lseek (pfh, currpayrec * sizeof (PAY), SEEK_SET);
	  currpayrec++;
	  if (_write (pcsvh, pcsvstr, paycsvlen) == -1)
	    {
	      sprintf (s, "Error (%s) writing to %s.", strerror (errno),
		       PAYCODECSV);
	      _close (pcsvh);
	      writemsg (s);
	      bupstage = bINITUSER;
	      break;
	    }
	  if (_read (pfh, &pc, sizeof (PAY)) == -1)
	    {
	      sprintf (s, "Error (%s) reading from %s.", strerror (errno),
		       PAYCODECSV);
	      _close (pcsvh);
	      writemsg (s);
	      bupstage = bINITUSER;
	      break;
	    }
	}
      else
	bupstage = bPAYCSVCLOSE;
      break;
    case bPAYCSVCLOSE:
      _close (pcsvh);
      sprintf (s, "Copied %ld paycodes to %s.", maxpayrec, PAYCODECSV);
      writemsg (s);
      bupstage = bDATINIT;
      break;

    case bDATINIT:
      if (strlen (bpath) == 0)
	{
	  writemsg
	    ("Could not start system backup! Back up target invalid. Press a key...");
	  bupstage = bAUTOABORT;
	  break;
	}
      mkdir (removeslash (bpath));
      strcpy (targmask, addslash (bpath));
      targlen = strlen (bpath);
      // figure out what mask you want to use
      strcpy (targmask + targlen, mask[maskcount]);
      if (!(doserror = _dos_findfirst (targmask, _A_NORMAL, &m)))
	bupstage = bDATDEL;
      else
	bupstage = bDATINITOPEN;
      break;
    case bDATDEL:
      strcpy (targmask + targlen, m.name);
      _unlink (targmask);
      if (!(doserror = _dos_findnext (&m)))
	bupstage = bDATDEL;
      else
	bupstage = bDATINITOPEN;
      break;
    case bDATINITOPEN:
      //find first dat file if there is one
      if (!(doserror = _dos_findfirst (mask[maskcount], _A_NORMAL, &m)))
	bupstage = bDATOPEN;
      else
	bupstage = bINITUSER;
      break;
    case bDATOPEN:		//open user message, greeting etc for copy
      //skip users.dat, users.csv and paycode.csv
      // they will be created below
      if (strcmp (m.name, USERFILE) == 0 || strcmp (m.name, CSVFILE) == 0)
	{
	  bupstage = bDATNEXT;
	  break;
	}
      if (bupopen (targmask, targlen, m.name, &hsrc, &htarg))
	{
	  sprintf (s, "Backing up %s...", m.name);
	  writemsg (s);
	  bupstage = bDATCOPY;
	}
      else
	bupstage = bDATNEXT;
      break;
    case bDATCOPY:		//copy message, greeting etc
      bupcopy (hsrc, htarg);
      break;
    case bDATCLOSE:		//close source and target files, go to next file
      bupclose (hsrc, htarg);
    case bDATNEXT:
      if (!(doserror = _dos_findnext (&m)))
	{
	  bupstage = bDATOPEN;
	}
      else
	{
	  if (++maskcount > MAXMASK)
	    bupstage = bINITUSER;
	  else
	    bupstage = bDATINIT;
	}
      break;

    case bINITUSER:		//open user file
      uptr = &u;
      U = (US *) uptr;
      UBUFF = (char *) uptr;	//set up pointer to buffer
      mkdir (removeslash (bpath));
      strcpy (targmask, addslash (bpath));
      targlen = strlen (bpath);
      strcpy (targmask + targlen, USERFILE);
      if (kbaction != kbCSVBUP && kbaction != kbCSVABUP)
	{
	  if ((huser = openfile (targmask)) == -1)
	    {
	      writemsg
		("Could not start system backup! Back up target invalid. Press a key...");
	      bupstage = bAUTOABORT;
	      break;
	    }
	}
      count = 0L;		//keep track of number of records read
      flen = _filelength (FUSER);
      if (flen == 0L)
	{
	  writemsg ("Nothing to back up! Press a key...");
	  bupstage = bERR;
	  break;
	}
      writemsg ("Initializing system backup.");

      if ((hscsv = openfile (CSVFILE)) == -1)
	{
	  writemsg
	    ("Could not start system backup! Unable to open csv source. Press a key...");
	  bupstage = bAUTOABORT;
	  break;
	}
      strcpy (targmask + targlen, CSVFILE);	//create target path
      if ((htcsv = openfile (targmask)) == -1)	//open csv file on target path
	{
	  writemsg
	    ("Could not start system backup! Unable to open csv target. Press a key...");
	  bupstage = bAUTOABORT;
	  break;
	}
      //write field names to csv files
      if (0 > writecsv (hscsv, htcsv, bupcsvstr, NULL))
	{
	  writemsg
	    ("Back up disk full (writing field names)! Back up aborted...");
	  bupstage = bERR;
	  break;
	}
      //get time for doing auto remind and deactivate
      time (&now);
      now += (DAYSECS - (now % DAYSECS));
      //go to next stage: initialize a user record for back up
      bupstage = bINIT;
      break;

    case bINIT:
      buperr = FALSE;
      //make box available to phone system again
      if (box != NOTFOUND)
	{
	  //note: only active boxes are marked as being inuse
	  if (useridx[box].avail >= INUSE)
	    useridx[box].avail = prevavail;
	}
      //check to see if there are more records to back up
      if ((count * sizeof (US)) >= _filelength (FUSER))	//check to see if you are at the end of the user file
	{
	  bupstage = bCLOSEUSER;
	  break;
	}
      //get user record
      _lseek (FUSER, (count * sizeof (US)), SEEK_SET);	//find next record
      _read (FUSER, U, sizeof (US));
      _lseek (FUSER, 0L, SEEK_SET);	//go back to start of file
      box = atoi (U->box);
      prevavail = useridx[box].avail;
      //wait until user is not in use only if you are doing a full backup 
      if (!boxavail (box, U->box) && AUTORMD)
	{
	  sprintf (s, "Box %04d in use. Please wait. ESC to abort...", box);
	  writemsg (s);
	  bupstage = bUPDATECOUNT;
	}
      else
	bupstage = bSTARTUSERCOPY;
      break;

    case bWAIT:
      if (boxavail (box, U->box))
	bupstage = bSTARTUSERCOPY;
      break;
    case bSTARTUSERCOPY:
      //remind and deactivate users, purge files from inactive boxes
      //must be doing full back up or be explicitly stated
      purgebox = FALSE;
      if AUTORMD
	autormd (U, now);
      bupstage = bCOPYUSER;
      break;
    case bCOPYUSER:		//copy user record
      _lseek (huser, (count * sizeof (US)), SEEK_SET);	//find next record
      utotal = 0;
      if (kbaction == kbCSVBUP || kbaction == kbCSVABUP)
	bupstage = bUPDATECOUNT;
      else
	bupstage = bWRITEREC;
      break;

    case bWRITEREC:
      if ((ucount = sizeof (US) - utotal) > bupchunk)
	ucount = bupchunk;
      ucount = _write (huser, UBUFF + utotal, ucount);
      if (ucount >= 0)
	{
	  utotal += ucount;
	  if (utotal >= sizeof (US))
	    bupstage = bUPDATECOUNT;
	}
      else			//error copying to disk
	{
	  writemsg ("Backup disk full! Back up aborted...");
	  bupstage = bERR;
	}
      break;

    case bUPDATECOUNT:
      _lseek (FUSER, 0L, SEEK_SET);	//go back to head of file
      _lseek (huser, 0L, SEEK_SET);	//go back to head of file
      count++;
      bupstage = bCHECKRECTYPE;
      break;

    case bCHECKRECTYPE:
      if (!isdef (u.user) && strlen (U->box) == field[fBOX].l)	//ignore default settings box
	bupstage = bMAKECSV;
      else
	bupstage = bINIT;
      break;

    case bMAKECSV:
      makecsvstr (U, bupcsvstr);
      csvlen = strlen (bupcsvstr);
      ucount = utotal = 0;
      bupstage = bWRITECSV;
      break;

    case bWRITECSV:
      //update where you are in the string
      if (csvlen <= bupchunk)
	ucount = csvlen;
      else if ((ucount = csvlen - utotal) > bupchunk)
	ucount = bupchunk;

      //copy to both source and back up disks 
      ucount = _write (hscsv, bupcsvstr + utotal, ucount);
      if (ucount < 0)		//disk full error
	{
	  writemsg ("Source disk full! Back up aborted...");
	  bupstage = bERR;
	  break;
	}

      ucount = _write (htcsv, bupcsvstr + utotal, ucount);
      if (ucount < 0)
	{
	  writemsg ("Back up disk full! Back up aborted...");
	  bupstage = bERR;
	  break;
	}
      //check where you are in the string
      if (ucount >= 0)
	{
	  utotal += ucount;
	  if (utotal >= csvlen)
	    bupstage = bINITCOPY;
	}
      break;

    case bINITCOPY:
      nextmsg = U->nextmsg;
      sprintf (s, "%s back up of box %s. ESC to abort...",
	       (kbaction == kbBACKUP ? "Full" : "Partial"), U->box);
      writemsg (s);
      sprintf (boxmask, "%s*.*", u.user.box);
      switch (kbaction)
	{
	case kbBACKUP:
	  bupstage = bINITDEL;
	  break;
	case kbPARTBUP:
	  if (chkboxes && purgebox)
	    bupstage = bINITOPEN;
	  else
	    bupstage = bINIT;
	  break;
	case kbCSVABUP:
	case kbCSVBUP:
	  bupstage = bINIT;
	  break;
	default:
	  bupstage = bERR;
	}
      break;

    case bINITDEL:
      //remove files from target
      if (!fnameok (u.user))
	break;
      strcpy (targmask + targlen, boxmask);
      if (!(doserror = _dos_findfirst (targmask, _A_NORMAL, &m)))
	bupstage = bDEL;
      else
	bupstage = bINITOPEN;
      break;
    case bDEL:
      strcpy (s, targmask);
      strcpy (s + targlen, m.name);
      _unlink (s);
      if (!(doserror = _dos_findnext (&m)))
	bupstage = bDEL;
      else
	bupstage = bINITOPEN;
      break;
    case bINITOPEN:
      //find first message file if there is one
      if (!(doserror = _dos_findfirst (boxmask, _A_NORMAL, &m)))
	bupstage = bOPEN;
      else
	bupstage = bINIT;
      break;
    case bOPEN:		//open user message, greeting etc for copy
      //check to see if files need to be purged
      if (purgebox)
	{
	  _unlink (m.name);
	  bupstage = bNEXT;
	}
      //otherwise copy the file to backup path ONLY IF IT IS A FULL BACKUP!
      else if (kbBACKUP && bupopen (targmask, targlen, m.name, &hsrc, &htarg))
	{
	  bupstage = bCOPY;
	}
      else
	bupstage = bNEXT;
      break;
    case bCOPY:		//copy message, greeting etc
      bupcopy (hsrc, htarg);
      break;
    case bCLOSE:		//close source and target files, go to next file
      bupclose (hsrc, htarg);
    case bNEXT:
      if (!(doserror = _dos_findnext (&m)))
	bupstage = bOPEN;
      else			//no more files
	{
	  if (U->nextmsg != nextmsg)
	    {
	      _lseek (FUSER, (count * sizeof (US)), SEEK_SET);	//find next record
	      _write (FUSER, U, sizeof (US));
	      _commit (FUSER);
	      _lseek (huser, (count * sizeof (US)), SEEK_SET);	//find next record
	      _write (huser, U, sizeof (US));
	      _lseek (FUSER, 0L, SEEK_SET);	//go back to start
	      _lseek (huser, 0L, SEEK_SET);	//go back to start
	    }
	  bupstage = bINIT;
	}
      break;
    case bCLOSEUSER:		//close user data backup file and end backup
      //write system stats to csv files
      if (0 > writestatsstr (hscsv, htcsv, bupcsvstr))
	{
	  writemsg
	    ("Back up disk full (writing system stats)! Back up aborted...");
	  bupstage = bERR;
	  break;
	}
      time (&now);
      savebup.lastbackup = backup.lastbackup = now;
      if (kbaction == kbBACKUP)
	savebup.partbackup = FALSE;
      else
	savebup.partbackup = TRUE;
      backup.partbackup = savebup.partbackup;
      savebackupparams ();
      sprintf (s, "Back up finished %s. Press a key...", timestr (&now));
      writemsg (s);
    case bERR:			//if back up is interrupted...
      _lseek (FUSER, 0L, SEEK_SET);	//go back to head of file
      if (huser)
	_close (huser);
      if (hscsv)
	_close (hscsv);
      if (htcsv)
	_close (htcsv);
      if (buperr && kbaction == kbBACKUP)
	bupclose (hsrc, htarg);
    case bAUTOABORT:
      buperr = FALSE;
      bupstage = bIDLE;
      kbaction = kbIDLE;
      chkboxes = FALSE;
      purgebox = FALSE;
    }
#undef BUPDIV
#undef U
#undef UBUFF
}

int
checkbupaction (US u, time_t now)
{
  int y, m, d;
  time_t t = NOTFOUND;
  y = m = d = 0;

  if (getdate (u.paidto, &d, &m, &y))
    {
      t = getdatesecs (d, m, y);
    }
  else
    return bupNOACTION;
  if (t == NOTFOUND)
    return bupNOACTION;
  if (bupdeactivate && u.active == ACTIVE && t < now - bupdeactivate)
    return bupDEACT;
  if (bupremind && u.active == ACTIVE && t < now + bupremind)
    return bupREMIND;
  if (purgedays && u.active == INACTIVE && t < now - purgedays)
    return bupPURGE;
  return bupNOACTION;
}

void
autormd (US * u, time_t now)
{
  int box;
  box = atoi (u->box);

  switch (checkbupaction (*u, now))
    {
    case bupNOACTION:
      return;
      //check deactivation first
    case bupDEACT:
      phonedeactivate (box, u);
      break;
      //then check reminder                   
    case bupREMIND:
      if (!(u->remind[0] == _SR || u->remind[0] == _RM))
	{
	  if (validadmin (u->admin))
	    strcpy (u->remind, RM);
	  else
	    strcpy (u->remind, SR);

	  if (u->boxtype == btADMN)
	    {
	      //bdowed must be deliberately cleared
	      u->bdowed += u->boxdays;
	      u->boxdays = 0L;
	      //page box if there is a valid paging number
	      addpage (u);
	    }
	}
      else
	return;
      break;
      //then check if messages need to be purged
    case bupPURGE:
      purgebox = TRUE;
      u->nextmsg = 0;
      break;
    default:
      return;
    }

  saveuser (FUSER, box, u);
  if (box == curruser)
    {
      user.bdowed = u->bdowed;
      user.boxdays = u->boxdays;
      user.active = u->active;
      strcpy (user.remind, u->remind);
    }
}

int
openfile (char *fname)
{
  int h;

  if ((h = _open (fname, _O_BINARY | _O_TRUNC | _O_RDWR)) == -1)
    {
      if ((h =
	   _open (fname, _O_BINARY | _O_CREAT | _O_RDWR,
		  _S_IREAD | _S_IWRITE)) == -1)
	{
	  sprintf (s, "Unable to open file %s!", fname);
	  writemsg (s);
	}
    }
  return h;
}

int
bupopen (char *targmask, int targlen, char *name, int *hsrc, int *htarg)
{
  if (strlen (name) == 0)
    return FALSE;

  if ((*hsrc = _open (name, _O_BINARY | _O_RDWR)) != -1)
    {
      strcpy (targmask + targlen, name);
      if ((*htarg = openfile (targmask)) != -1)
	{
	  return TRUE;
	}
      _close (*hsrc);
    }

  return FALSE;
}

void
bupclose (int hsrc, int htarg)
{
  unsigned date, time;

  //make sure file dates and times are the same
  if (hsrc)
    {
      _dos_getftime (hsrc, &date, &time);
      _close (hsrc);
    }

  if (htarg)
    {
      _dos_setftime (htarg, date, time);
      _close (htarg);
    }
}

int
bupcopy (int hsource, int htarget)
{
  static char buf[BUFSIZE] = "";
  unsigned count = BUFSIZE;
  long diff;

  //check size of count variable
  if ((diff = _filelength (hsource) - _tell (hsource)) < (long) count)
    count = (unsigned) diff;

  /* Read and write input. */
  if ((count = _read (hsource, buf, count)) == -1)
    {
      writemsg ("ERROR: back up copy read!");
      buperr = TRUE;
      if (bupstage == bDATCOPY)
	return bupstage = bDATCLOSE;
      return bupstage = bCLOSE;
    }

  if ((count = _write (htarget, buf, count)) == -1)
    {
      buperr = TRUE;
      if (errno = ENOSPC)	//disk full
	{
	  writemsg ("ERROR: back up disk full!");
	  bupstage = bERR;
	}
      else
	{
	  writemsg ("ERROR: wrote to bad file handle!");
	  if (bupstage == bDATCOPY)
	    return bupstage = bDATCLOSE;
	  bupstage = bCLOSE;
	}
      return bupstage;
    }

  if (_tell (hsource) >= _filelength (hsource))
    {
      if (bupstage == bDATCOPY)
	return bupstage = bDATCLOSE;
      return bupstage = bCLOSE;
    }
  return bupstage;
}

// file name logging functions for mirroring messages to a website
// make a log file name based on the current date 
// put everything in a separate subdirectory to make finding the files easier
char *
get_newfile_log (void)
{
  static char logname[20] = { 0 };
  time_t t_time;
  struct tm *t;
  time (&t_time);
  t = localtime (&t_time);
  sprintf (logname, "%s/%04d%02d%02d.txt", LOGS, t->tm_year + 1900,
	   t->tm_mon + 1, t->tm_mday);
  return (logname);
}

// save the name of a file in the current log
void
log_newfile (char *curr_fname)
{
  FILE *f;
  if (strlen (curr_fname))
    {
      if ((f = fopen (get_newfile_log (), "a")) != NULL)
	{
	  fprintf (f, "%s\n", curr_fname);
	  fclose (f);
	}
      // sprintf(s,"%s/%s",NOTIFY,curr_fname);
      // if ((f = fopen(s,"w")) != NULL) {
      //      fclose(f);
      // }
    }
}

/* end of util.c */
