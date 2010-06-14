
/* init.c -- part of ansr3.c */

/* function definitions for this module */

void initmsg(char *out); 				/* print logging messages on startup status */
void ctrlcdiscard(int sig); 			/* used to prevent "ctrl-break" from ending program */
void sysexit(int errno); 				/* cleanup routines for exit */
void showhelp(void); 					/* command line syntax */
int  pinok(char *pin); 					/* check for illegal characters in pin number */
void chkargs(int argc, char *argv[]); 	/* get command line options */
int  sysinit(int d4xint); 				/* start voice board and initialize channels */
void closestreams(void); 				/* free up file handles for program use */
int  openfiles(void); 					/* open common prompt files and user data file */

/* functions */
void initmsg(char *out)
{
	if (debug) _write(logfh, out, strlen(out));
#ifndef DAEMON	
	printf(out);		
#endif
}

void ctrlcdiscard(int sig)
{                                
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, ctrlcdiscard);
}

void warmboot( void )
{
	union _REGS inregs, outregs;

    /* Do print screen. */
    inregs.h.ah = 0x19;
    _int86( 0x19, &inregs, &outregs );
}

/****************************************************************
*        NAME : sysexit(errno)
* DESCRIPTION : display exit message and exit to DOS.
*       INPUT : errno = exit number.
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
void sysexit(int errno)
   {
      switch (errno)  {
      /* exit states from sysinit */
      case -10:
      case -9:
      case -8:
            stopsys();
      case -7:
      case -6:
      case -5:
      case -4:
      case -3:
      case -2:
      case -1:
            showhelp();
         break;
      /* all other exit states */
      default:         
            stopsys();
            showhelp();
      }

      exit (errno);
   }

/****************************************************************
*        NAME : chkargs(argc,argv)
* DESCRIPTION : check command line arguments, display help or set
*             : interrupt level("-?" or "-In").
*       INPUT : argc = argument count.
*             : *argv[] = array of pointers to command line arguments.
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
void showhelp(void)
{
    printf("%s -A -Cn -Dnnnn -E -F -Gn -H -In -Nn -Pn.. -Q -Rn -Tn -Vn -X\n", progname);
    printf("-A     Always check for admin box when editing records. Default: %s.\n", strictadmin? "CHECK" : "DON'T CHECK");
    printf("-B     Permit the browse feature. (default: browse off).\n");
    printf("-Cn    Number of D/4x channels to use, max: %d.\n",maxchan);
    printf("-Dnnnn Default security code for new boxes. Currently %s.\n", defpin);
    printf("-E     Show event codes on console. Log file: %s.\n", LOGFILE);
    printf("-F     Log new messages. Log dir: %s.\n", LOGS);
    printf("-Gn    Number of times user can go back to start in one call. Default: No limit.");
    printf("-H     Enable answering machine detection.\n");
    printf("-In    D/4x interrupt level (2-15), default: %d.\n",DEFINTR); 
    printf("-Mn    Disk size in MB. Use if size exceeds 2GB.\n");
    printf("-Nn    Number of messages per box (min: %ld, max: %ld, default: %ld).\n", MINMSG, MAXMSG, DEFMSG);
    printf("-Pn... Call back number for numeric paging. Default: %s.\n", sysphone);
    printf("-Q     Remove silence when recording messages.\n");
    printf("-Rn    Number of rings to pick up on, max: %d, default: %d.\n", MAXRING, DEFRING);
    printf("-Tn    Message length in seconds, min: %d, max: %d, default: %d.\n", (int) MINTIME, (int) MAXTIME, (int) DEFTIME);
	printf("-Vn    Default value in cents of one box day (currently = %ld).\n", bdval);
	printf("-X     Disable paging feature.\n");
	printf("\n");
	printf("Use LLSETUP.EXE to configure command line parameters.\n");
}

int boxnumeric(char *boxstr)
{
    register i;
    int bl;
    bl = strlen(boxstr);
    if (bl < BOXLEN) return FALSE;
    for (i = 0; i < bl; i++)
        switch (boxstr[i])
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
	         	break;
         	default: return FALSE;
         }
         
	return TRUE;
}	

int validbox(char *box)
{
	int b;
	if (!boxnumeric(box)) return FALSE;
	b = atoi(box);
	if (userpos[b] == NOTFOUND)
	{
		//seems to be the best place to do this
		//voidport does not do anything to useridx and this
		//index may get out of sync with the status of the box
		//boxes that do not already exist cannot be put "ONHOLD"
		useridx[b].avail = NOTFOUND;
		return FALSE;
	}
	return TRUE;
}

int validadmin(char *box)
{
	int b;
	if (strlen(box) != BOXLEN) return FALSE;
	if (!boxnumeric(box)) return FALSE;
	b = atoi(box);
	if (useridx[b].type == btADMN) return TRUE;
	if (useridx[b].type == btPAYC) return PCONLY;
	return FALSE;
}

int pinok(char *pin)
{                   
    //pin must be 4 digits - phone system expects 4 digits                            
    //any comparision with a shorter pin would never be true
    if (strlen(pin) != field[fPIN].l) return FALSE;
    // pins that evaluate to zero make the vm box not accept messages
    if (atoi(pin)) return TRUE;
    return FALSE;
}

int pintoi( char * pin )
{
	register i;
	int len;
	char pbuff[PINLEN + 1];
	len = strlen(pin);
	strcpy(pbuff, pin);
	for (i = 0; i < len; i++) 
		if (pin[i] == '*' || pin[i] == '#') pbuff[i] = 0;
	return atoi(pbuff);
}

void chkargs(int argc,char *argv[])
   {
      int argno,data, cwdlen;
      char spaces[] = "                                            ";
      char *cp = *argv;                        
      char s[DOSPATHLEN] = "";
      
      /* get program name for error messages */
      for (progname = cp; *cp; ++cp)
        if (*cp == '\\') progname = cp + 1;  
      
      strcpy(fullprogname, argv[0]); //get full path name for program
      
      // create current working directory string
      getcwd(s, PATHLEN);      
      cwdlen = strlen(s);
      if (cwdlen < PATHLEN)
      {
	   	strncpy(cwd, spaces, PATHLEN - cwdlen);
	   	strcat(cwd, PATHP);
	   	strcat(cwd, s);
      }
      else 
      {
      	strcpy(cwd, PATHP);
      	strcat(cwd, s);
      }
	  if (cwd[strlen(cwd)] != '\\') strcat(cwd, "\\");

      /* check each argument */
      for (argno=1; argno < argc; argno++)  {

         /* check for dash or slant bar */
         if (*argv[argno] != '-')  {
            sprintf(s, "%s: Must begin option with '-'\n", progname);
            initmsg(s);
            sysexit(1);
         }
         switch (tolower(*(argv[argno]+1)))  {
         case 'a':
         	strictadmin = TRUE;
         break; 
    	 case 'b':
    	 	canbrowse = TRUE;
    	 break;     
         case 'c':
            /* set maximum number of channels to use */
            sscanf(argv[argno]+2,"%d",&data);
            if (data<=0)  {
               sprintf(s, "%s: Need to use at least one channel.\n", progname);
               initmsg(s);
               sysexit(1);
            }
            chan = data;
            break;

         case 'd':
            // ignore if there is no pin there
            if (strlen(argv[argno]) <= 2) break;
            sscanf(argv[argno]+2,"%s", &s);            
            if (!pinok(s)) 
            {
                initmsg("Invalid default pin (-D option)\n");
                sysexit(2);      
            }
            else strcpy(defpin, s);
            break;

         case 'e': 
            //write event codes and log events in file
            debug = TRUE;
            break;

         case 'f': 
            //log any new messages created so they can be mirrored to a web server
            log_newfiles = TRUE;
            break;

         case 'g': //maximum # of times user can go back to start in one call
         	sscanf(argv[argno]+2, "%d", &maxiteration);
         	break;
         	
         case 'h': helloedge = FALSE; break;
         
         case 'i':
            /* set interrupt level between 2 and 7 if given */
            sscanf(argv[argno]+2,"%d",&data);
            /* interrupt level should range between 2 and 7 */
            if ((data<2 || data>15) || (data == 8) || (data == 13))  {
               sprintf(s, "%s: Invalid interrupt level (2-15 only).\n", progname);
               initmsg(s);
               sysexit(3);
            }
            d4xintr = data;
         break;
         
         //get disk size in MB
         case 'm':
         	sscanf(argv[argno]+2,"%ld",&diskmb);
         break;
         
         case 'n':
            /* set maximum number of messages */
            sscanf(argv[argno]+2,"%d",&maxmsg);
            if (maxmsg > MAXMSG)
            {
                sprintf(s, "%s: %d is above maximum number of messages.\n", progname, maxmsg);
                initmsg(s);
                sysexit(5);
            }
            else if (maxmsg < MINMSG)
            {
                sprintf(s, "%s: %d is below minimum number of messages.\n", progname, maxmsg);
                initmsg(s);
                sysexit(5);
            }    
            break;
         
         case 'p':
            /* paging number */
            sscanf(argv[argno]+2,"%s", &sysphone);
            break;   
         case 'q':
         	//sprintf(s, "%s: Silence compression on.", progname);
         	//initmsg(s);
         	scomp = RM_SCOMP;
         break;
         case 'r':
            /* set number of rings to use */
            sscanf(argv[argno]+2,"%d", &maxring);
            if (maxring > MAXRING)
            {
                sprintf(s,"%s: too many rings. Should be no more than %d.\n",
                        progname,
                        MAXRING);
                initmsg(s);        
                sysexit(6);        
            }            
            else if (maxring <= 0)
            {
                sprintf(s,"%s: too few rings.\n", progname);
                initmsg(s);
                sysexit(6);
            }
            break;
         
/*         case 's':
            sscanf(argv[argno]+2,"%s", &s);
            if (strlen(s) == 0 || pinok(skeletonpin)) 
                strcpy(skeletonpin, s);
            else
            {
                pinwrong(s);
                sysexit(7);
            }
            break;   
*/         
         case 't':
            /* message time */
            sscanf(argv[argno]+2, "%d", &maxtime);
            if (maxtime > MAXTIME)
            {
                sprintf(s,"%s: message length should not exceed %d seconds.\n", progname, MAXTIME);
                initmsg(s);
                sysexit(8);   
            }
            else if (maxtime < MINTIME)
            {
                sprintf(s,"%s: message length should not go below %d seconds.\n", progname, MINTIME);
                initmsg(s);
                sysexit(8);
            }    
            break;
         case 'v':
            /* box day value */
            sscanf(argv[argno]+2, "%ld", &bdval);
            if (bdval > MAXBDVAL)
            {
                sprintf(s,"%s: box day value cannot exceed %ld cents.\n", progname, MAXBDVAL);
                initmsg(s);
                sysexit(8);   
            }
            else if (bdval < 0L)
            {
                sprintf(s,"%s: box day value cannot go below %d cents.\n", progname, 0);
                initmsg(s);
                sysexit(8);
            }    
            break;
            
         case 'x': 
         	//tollsaver: not documented but needed in case feature does not work.
         	tollsaver = FALSE;
         	//may want to turn paging off if system use is too heavy
         	pagingon = FALSE;
         break;
         
         default:
            /* display the help message */
            sysexit(23);

         }
      }
   }
/****************************************************************
*        NAME : sysinit(d4xint)
* DESCRIPTION : start d/4x system, set cst parameters, put line on
*             : hook and open vox files.
*       INPUT : d4xint = hardware interrupt level.
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
int openlog( void )
{
	time_t now;
	
	logfh = 0;
#ifndef DAEMON	
    if (debug) 
    {
#endif    
    	if ((logfh = _open(LOGFILE, _O_WRONLY|_O_APPEND)) < 0)
    	{
    		if ((logfh = _open(LOGFILE, _O_WRONLY|_O_CREAT, _S_IWRITE)) < 0)
    		{
    			printf("%s: Unable to create log file for debug!\n", progname);
    			sysexit(0);
    		}
    	}
      
    	time(&now);
    	sprintf(s, "\nDEBUG SESSION: %sVersion: %s\n%s\n\n", ctime(&now),VER,RCSID);
    	_write(logfh, s, strlen(s));
#ifndef DAEMON
    }
#endif    
    return logfh;
}    

int sysinit(int d4xint)
{
      int rc;  /* return code from startsys() */
      int channel = 2;
      /* load control-break handler */
      if (signal(SIGINT, ctrlcdiscard) == SIG_ERR)
      {
        initmsg("Unable to redirect ctrl-c.\n");
        abort();
      }
      else initmsg("Ctrl-C handler installed.\n");
            
      /* check for the D/4x driver */
      if (!getvctr())  {
         initmsg("DIALOG/4x voice driver not installed.\n");
         return FALSE;
      }
      else initmsg("Found Dialog/4x voice driver.\n");
      
      //make sure system is shut down before initializing
      stopsys();
      //initialize system (all following commands)
      rc = startsys(d4xint,SM_EVENT,0,0,&channel);
      if (rc != E_SUCC) {
         sprintf(s, "Unable to start voice system, Return code %d.\nCheck that you are using the -i option correctly.\n",rc);
      //   initmsg(s);
         sysexit(-7);
      }
      else initmsg("Voice system started.\n");

      /* if maximum chans set from the command line */
      if (chan==0 || chan>channel)  {
         chan = channel;
      }

      /* chan can't be greater than maxchan */
      if (chan>maxchan) {
         chan = maxchan;
      }
      
      sprintf(s, "Number of active channels = %d.\n", channel); initmsg(s);
      
      /* set all channels to detect call */
      initmsg("Clearing ports.\n");
      if (helloedge) 
      {
      	 clrcpb(&cpb);
      	 cpb.hedge = 1; //detect connect immediately, disable ans mach detect
      }
      
	  //check to see which channels are connected to telco
      gettelcochans(); 
      
      for (channel=1; channel<=chan; channel++)  {
      	 if (telcochans[channel-1] == tsDIRECT) {
         	setcst(channel, (C_OFFH), 0); // don't respond to events normally
         	set_hkstate(channel,H_OFFH); /* put line OFF hook */
      	 } else {
         	setcst(channel, (C_RING+C_OFFH+C_ONH+C_LC), maxring);
         	if (helloedge) setcparm(channel, &cpb);
         	set_hkstate(channel,H_ONH); /* put line on hook */
         }
         PRT = blankport; /* clear port array */
      }
      return TRUE;
}

void closestreams(void)
{
    /* close standard c streams */
    /* fclose(stdin); */ /* keyboard input */
    fclose(stdout); /* console output */
    fclose(stderr);
    fclose(_stdaux); /* serial port */
    fclose(_stdprn); /* parallel port */
}

void openpaycodes( void )
{
	if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY)) == -1)
	{
#ifdef TRIAL
		return;
#endif
		sprintf(s,"File %s not found. Creating pay code file.\n", PAYCODEFILE);
		initmsg(s);
		if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY|_O_CREAT,_S_IREAD|_S_IWRITE)) == -1)
		{
			sprintf(s,"Could not create %s.\n", PAYCODEFILE);
			initmsg(s);
		}
	}
}

void missingfile(int i)
{
    char yn;

    sprintf(s, "%s: Cannot open %s.\nStart %s without voice system? Y/N (Enter = Y)", progname, states[i].fname, progname);
    initmsg(s);
    yn = getch();
    initmsg("\n");
    yn = toupper(yn);
    if (yn == 'Y' || yn == ENTER) 
    {
    	stopsys();
    	chan = 0;
    	return;
    }
    else sysexit(i);
}

int loadnumidx( int l )
{
	register i;
	MS n;
	
	for (i = 0; i < MAXNUM; i++)
	{
		if (_eof(promptf[l].nih)) //if closed before all needed numbers are indexed...
		{
			initmsg("Error loading date and time prompts. \nFile date and time disabled. \nCheck that you are using the correct prompt files!\n");
			_vhclose(promptf[l].nh); //close number voice file
			promptf[l].nh = 0;		//0 here prevents date from being played
			return FALSE;
		}
		_read(promptf[l].nih, &n, sizeof(MS));
		promptf[l].nidx[i] = n;
	}
		
	_close(promptf[l].nih);
	return TRUE;
}

int loadpromptidx( int l )
{
	register i, j;
	IS idx;
	long maxprompt;
	char prompterr = 0;
	static char statesfhfilled = FALSE;
	
	if (!statesfhfilled)
	for (j = 0; j < MAXSTATE; j++)
		states[i].fh = NOTFOUND;

	maxprompt = _filelength(promptf[l].pih) / sizeof(IS);

	for (i = 0; i < maxprompt; i++)
	{
		_read(promptf[l].pih, &idx, sizeof(IS));
		promptf[l].pidx[i].offset = idx.pos;
		promptf[l].pidx[i].length = idx.len;

		if (statesfhfilled) continue;
		//check all the prompts for matches
		for (j = 0; j <= MAXSTATE; j++)
		{
			//skip states with blank file names
			if (strlen(states[j].fname) == 0) 
				continue;
			//if you find a prompt that matches the state then
			//mark that state with the location of that prompt
			if (stricmp(idx.name, states[j].fname) == 0)
			{
				states[j].fh = i;
				continue;
			}
		}
	}
	
		
	_close(promptf[l].pih);
		
	for (j = 0; j <= MAXSTATE; j++)
		if (strlen(states[j].fname) > 0 && states[j].fh < 0) // a prompt was not found!
		{
			sprintf(s, "Prompt %s was not found. Handle: %d.\n", states[j].fname, states[j].fh);
			initmsg(s);
			prompterr = TRUE;
		}	
	if (prompterr) return FALSE;
	statesfhfilled = TRUE;
	return TRUE;
}

int compare(const void * e1, const void * e2)
{
	return stricmp( ((PT *)e1)->name, ((PT *)e2)->name );
}

int openulfh( void )
{
	int ulfh;
	if ((ulfh = _open(ULOGFILE, _O_RDWR|_O_TEXT)) < 0)
	{
		return ulfh = _open(ULOGFILE, _O_RDWR|_O_TEXT|_O_CREAT, _S_IREAD|_S_IWRITE);
	}
	return ulfh;
}

int openfiles(void) 
{          
    int err;
    register j = 0;
    struct _find_t p;
    int pos;
                      
    /* open program files */                 
    initmsg("Opening files...\n");
    
    //user data and some program flags (eg announce)
    openuserfile(&FUSER);
	//check for announcement
	if (announce = TRUE && 
		(announceh = vhopen(ANNOUNCEMENT,READ)) == 0)
		announce = FALSE;
    
    //look for multi prompt files
    err = _dos_findfirst(PMASK, _A_NORMAL, &p);
    while(!err && j < MAXPROMPTF)
    {
	    if (promptf[j].ph = vhopen(p.name, READ))
	    {
			//get language file name         
			pos = (int)(strchr(p.name,'.') - p.name);
			strncpy(promptf[j].name, p.name, pos);
			promptf[j].name[pos] = '\0';
			if ((promptf[j].pih = _open(fname(promptf[j].name, IXP), _O_RDWR|_O_BINARY)) >= 0)
			{
				sprintf(s, "Loading %s system prompts.\n", promptf[j].name);
				initmsg(s);				
		    	if (loadpromptidx(j))
		    		maxlanguage++;
		    	else sysexit(0);
			    //open file that has file date and time number prompts
			    if (promptf[j].nh = vhopen(fname(promptf[j].name, VXN), READ)) 
			    {
					if ((promptf[j].nih = _open(fname(promptf[j].name, IXN), _O_RDWR|_O_BINARY)) >= 0)
					{
						sprintf(s, "Loading %s date and time prompts.\n", promptf[j].name);
						initmsg(s);
				    	if (!loadnumidx(j)) sysexit(0);
				    }
			    }
		    	j++;
		    }
		    else
		    {
		    	sprintf(s, "Could not load %s system prompts!\n", promptf[j].name);
		    	initmsg(s);
		    }	
	    }
	    err = _dos_findnext(&p);
	}
	if (maxlanguage) 
	{
		if (maxlanguage > 1)
		{
			qsort((void *)promptf, (size_t)maxlanguage, sizeof(PT), compare);
			//check to see if this this is the default language file
			for (j = 0; j < maxlanguage; j++)
				if (stricmp(promptf[j].name, DEFLANG) == 0) 
				{
					deflanguage = j;
					break;
				}
		}
		//using this value brings up "any" on finduser record
		findlanguage = maxlanguage;
		return maxlanguage;
	}
	else return 0;
}

//for expiry date functions
int startdateok( void )
{
    int nDay;
    int nMonth;
    int nYear;
    struct find_t ft;
    struct tm *t;
    time_t now;
    
    if (_dos_findfirst(USERFILE, _A_NORMAL, &ft))
    {
    	//return true if no user data file
    	return TRUE;
    }
    
    nDay = ft.wr_date & 0x1f;
    nMonth = ((ft.wr_date >> 5) & 0x0f) - 1;
    nYear = (ft.wr_date >> 9) + 80;
    
    time(&now);
    t = localtime(&now);

   	if (t->tm_year > nYear)
		return TRUE;
	else if(t->tm_year == nYear && 
			t->tm_mon > nMonth)
			return TRUE;
	else if(t->tm_year == nYear && 
			t->tm_mon == nMonth && 
			t->tm_mday >= nDay)
			return TRUE;
			
	return FALSE;
}

int expired( void )
{
#ifdef TIMELIMIT
	time_t now;
	struct tm *t;

	time(&now);
	t = localtime(&now);
	if (t->tm_year > LYEAR - 1900) return TRUE;
	else if(t->tm_year == LYEAR - 1900 && 
			t->tm_mon > LMON - 1 ) return TRUE;
	else if(t->tm_year == LYEAR - 1900 && 
		    t->tm_mon == LMON - 1 && 
		    t->tm_mday > LDAY) 
			return TRUE;
	return FALSE;
#else
	return FALSE;
#endif
}

void expexit( void )
{
#ifdef TIMELIMIT
	stopsys();
	sprintf(s, "Program expired %d-%d-%d! || For upgrade information please contact: || %s ", 
	            LDAY, LMON, LYEAR, ADDRESS);
  	cls();
	_settextposition(24,1);
	_settextcursor(LINECURSOR);
  	strwin(fgWHITE, jCENTER, s);
  	exit(0);
#endif
}

void getsn(char *sn)
{
#ifdef SNLEN
	int i = 0;
	unsigned char key;
	while ((key = getch()) != ENTER && i < SNLEN)
	{
		sn[i++] = key;
		printf("*");
	}
	sn[i] = '\0';
#endif
}

void checksn( void )
{

#ifdef TRIAL
	return;
#endif

#ifdef SN
	char sn[SNLEN + 1];
	
	if (strlen(SN) == 0) return;
	if (strncmp(SERIALNUMBER, SN, strlen(SN)) == 0) return;
		
	printf("Enter serial number: ");
	getsn(sn);
	if (strcmp(sn, SN) != 0)
	{
		printf("\nSerial number wrong. Please reenter: ");
	    getsn(sn);
		if (strcmp(sn, SN) != 0)
		{
			printf("\nInvalid serial number!\n");
			exit(0);
		}
	}
	else printf("\n");
	
#endif

}
/* end of init.c */




















