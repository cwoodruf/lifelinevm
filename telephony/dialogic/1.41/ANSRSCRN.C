
/* ansrscrn.c: part of the ansr3.c program that makes the video display */

void writemsg(char * s) //write user help message
{
#ifdef DAEMON
	writedebug(s);
#else
	//check to see if message is default message for state or not
	if (strcmp(s, CURRSTR) == 0) isdefmsg = TRUE;
	else isdefmsg = FALSE;
	//write the message
	writestr(2, /* x pos */
			 HELP_TOP,  /* y pos */
			 0,   /* start in string at */
			 MSGLEN, /* size of window */
			 scrtxt[HELP].attr + fgWHITE, /* color */
			 ((s[0] <= 32 && s[0] > 0) ? s + 1: s)); 
			 /* string: increment if first char is whitespace */
#endif
			 
}

//use below for debug purposes only!!!!
void writech(char * s)
{
	writemsg(s);
	if (getch() == ESC) sysexit(0);
}

void writeimsg(char * s, int i) //write message on specific line of user info
{
	int attr;
	
	if (i == 1) attr = bgWHITE + fgBLUE;
	else attr = FLDATTR;
	
	//write the message
	writestr(2, /* x pos */
			 INFO_TOP + i + 1,  /* y pos */
			 0,   /* start in string at */
			 MSGLEN, /* size of window */
			 attr, /* color */
			 s); 
}

void trialch(char *s)
{
#ifdef TG
	return;
#endif
#ifdef TRIAL
	writemsg(s);
	if (getch() == ESC) sysexit(0);
#endif	
}

void writettl(char * s)
{
	//write the title
	writestr(2, /* x pos */
			 INFO_TOP,  /* y pos */
			 0,   /* start in string at */
			 MSGLEN, /* size of window */
			 scrtxt[HELP].attr + fgWHITE, /* color */
			 (s[0] <= 32? s + 1: s)); 
			 /* string: increment if first char is whitespace*/
}

void showttls( void )
{
	enum fields i;       
	int min, max;

	cleardisplay();

	switch (keystate)
	{
		case ksBACKUP:
			min = fBUPPATH; 
			max = fBUPTTL1;
		break;
		case ksMSGINFO:
			return;
		default:
			min = fBOX; 
			max = fACTIVE;
	}
    
	/* editable fields */
	for (i = min; i <= max; i++)
    {
		if (field[i].ttlx != NOTTL)
		{
			if (strlen(field[i].name) > 0) 
				writestr( 
						 field[i].ttlx,
						 field[i].y,
						 0,         
						 strlen(field[i].name),
						 FLDATTR,
						 field[i].name
						);
		}
	}
}

void makeemptyscrn( void )
{
	register i;
    int size = sizeof(emptyscrn)/sizeof(CELL);
    CELL blankcell = {SPACE, FLDATTR};

    for (i = 0; i < size; i++)
    {
    	emptyscrn[i] = blankcell;
    	//emptyscrn[i].attr = blankcell.attr;
    }
}

void cleardisplay( void )
{
    unsigned char far *c;
    c = scrnaddress(1,16);
    _fmemmove(c, emptyscrn, sizeof(emptyscrn));
}

void writescreentext(struct screentext scr)
{
	int k, j, i, pos;
	char n[20], s[max(SCRWID,STATSWID)];
	
	if (scr.attr != bgWHITE)
		scr.attr += fgWHITE;
	
	if (scr.num) /* if you are supposed to put a number in the string */ 
	{            
		i = 0;                         
		
		strcpy(s, scr.s);                                  
		
		while (s[i] != '\0' && s[i] != '^') i++;
		if (s[i] == '^')  /* find the location */
		{ 
			pos = i;
			for (j = 0; j < scr.reps; j++) /* for the number of vertical reps */            
			{				
				_itoa(j + scr.num, n, 10); /* convert the number to a string */
				
				i = pos;
				k = 0;
				while (n[k] != '\0' && s[i] != '\0')
				{    /* put the number in the output string at the location */
					s[i] = n[k];
					k++;            
					i++;
				}
				
				/* output the string to screen */
				writestr(scr.x, scr.y + j, 0, scr.l, scr.attr, s); 
			}  
			return;
		}
	}                                                             

	for (j = 0; j < scr.reps; j++) /* for the number of vertical reps */
	{
		/* output the string to screen */
		writestr(scr.x, scr.y + j, 0, scr.l, scr.attr, scr.s); 
	}
}

void makestatstr(void)
{	
    //set up bottom line of screen with data that is constant
    /* maximum # of messages before box full msg occurs */
    statlen[MSGS] = sprintf(statstr[MSGS], "%ux%us msgs/box", 
                         (unsigned) maxmsg, (unsigned) maxtime);
    /* number of rings before line is set off hook */
    if (maxring == 1) 
    	statlen[RINGS] = sprintf(statstr[RINGS], "%i ring", maxring);
    else statlen[RINGS] = sprintf(statstr[RINGS], "%i rings", maxring);
    // number of times user can go back to start        
    if (maxiteration == 1) 
    	statlen[ITERS] = sprintf(statstr[ITERS], "%i repeat", maxiteration);
    else if (maxiteration > 1) 
    		statlen[ITERS] = sprintf(statstr[ITERS], "%i repeats", maxiteration);
	if (pagingon) statlen[PGON] = sprintf(statstr[PGON], "Page on");
	else statlen[PGON] = sprintf(statstr[PGON], "Page off");
}

void makescreen(void) {
	register i = 0;
	
	//set up user info line if there is an expiry date
#ifdef TIMELIMIT
	sprintf(scrtxt[0].s, scrtxt[0].f, LDAY, months[LMON - 1], LYEAR % 100, cwd);
#else	
	sprintf(scrtxt[0].s, scrtxt[0].f, cwd);
#endif

	//build rest of screen
	while (scrtxt[i].x) /* a null struct is used to indicate end of array */
	{
		writescreentext(scrtxt[i]);
		i++;
	}
	
}

void writedebug(char *msg)
{
	_write(logfh, msg, strlen(msg));
	_write(logfh, "\n", strlen("\n"));
	_commit(logfh);
}

void writechan(int channel)
{       
#ifndef DAEMON
	writestr(CHANX,
			 LINES_TOP + channel - 1,
			 0,
			 SCRWID - CHANX + 1,
			 fgWHITE + bgBLACK,
			 CHMSG);
	if (debug)
#endif	 
		writedebug(CHMSG);
}

int loaduser(int fuser, int idx, struct userstruct *user)
{
	if (userpos[idx] != NOTFOUND) {
		_lseek(fuser, USERPOS(idx), SEEK_SET);
		_read(fuser, user, sizeof(struct userstruct)); 
		_lseek(fuser, 0L, SEEK_SET);
		//put box number in in case record is corrupted
		putbox(user, idx);
		return 0;
	}            
	return NOTFOUND;
}
 
void getuserrec(int fuser, int curruser, struct userstruct *user)
{
	if (userpos[curruser] >= 0L) {
		_lseek(fuser, USERPOS(curruser), SEEK_SET);
		_read(fuser, user, sizeof(struct userstruct)); 
		_lseek(fuser, 0, SEEK_SET);
		putbox(user, curruser);    
		//make sure that defbox parameter is correct
		//if (curruser == defbox && user->boxtype != btDEF)
		//	defbox = NOTFOUND;
	}
	else 
	{
		//create new record
		*user = blankuser;
		putbox(user, curruser);
		//saveuser(fuser, curruser, user);
	}
}

void saveuser(int fuser, int box, struct userstruct *user)
{
	//int admin;
	if (userpos[box] >= 0L) {
		/* go to saved file position for record */
		_lseek(fuser, USERPOS(box), SEEK_SET);
	}
	else {                                        
		/* go to end of file */                   
		_lseek(fuser, 0L, SEEK_END); 
		userpos[box] = (int) (_filelength(fuser)/sizeof(US)); 
	}
	_write(fuser, user, sizeof(struct userstruct));
	_lseek(fuser, 0L, SEEK_SET);
	/* find out how many new boxes can be added */
	numavailboxes();
	_commit(fuser);
}

void setuprnd( void )
{ 
	//start randomization for random pin
	srand((unsigned)time(NULL));
	#undef RAND_MAX
	#define RAND_MAX (MAXBOX - 1)
}
	
void putbox(struct userstruct *c, int box)
{
	//create box number 
	sprintf(c->box, "%04i", box);
	//if box is not saved in users.dat then insert default pin
	if (userpos[atoi(c->box)] == NOTFOUND)
		strcpy(c->pin, defpin);
	//if box is in users.dat and pin is not blank clean it up
//	else if (!pinok(c->pin))
//			sprintf(c->pin, "%04i", atoi(c->pin));
}               

int userempty(struct userstruct *user)
{
	int i;
	
	for (i = poslimit[ksFIND].min; i <= poslimit[ksFIND].max; i++) 
	{
		if (strlen(selectfield(i, user)) > 0) 
			return FALSE;
	}
	return TRUE;
}

int userequal(struct userstruct *user1, struct userstruct *user2)
{
	enum fields i;
	for (i = poslimit[ksFIND].min; i <= poslimit[ksFIND].max; i++) 
	{
		if (strcmp(selectfield(i, user1), 
				   selectfield(i, user2)) != 0)  return FALSE; 
	}
	
	if (user1->active != user2->active) return FALSE;
	
	return TRUE;
}

void updatecalls(struct userstruct *user)
{                   
	int i;
	
	/* don't change screen if edit help window visible or 
	keystate does not support update */
	if (keystate > ksSHOWUSER || 
		helpon || showtelco || showingcperh) return; 
	
	for (i = fBOX; i <= fMSGS; i++)
	{
		switch(i)
		{
		case fREMIND:
		case fPAIDTO:
		case fMISC:
		case fLANGUAGE:
		case fPIN:
		case fPHONE:
		case fLASTCALL:
		case fMSGS:
		case fCALLS:
		writestr( 
				 field[i].x,
				 field[i].y,
				 0,         /* start from start of string */
				 field[i].l,
				 field[i].attr,
				 selectfield(i, user) /* string to display */
				); 
		}
	}
}

void showuser(struct userstruct *user)
{
#ifdef DAEMON
	return;
#else
	enum fields i;       
	int min,max,loffs = 0;
	switch(keystate)
	{
		case ksBACKUP:
			min = fBUPPATH; max = fLASTBUP;
		break;
		case ksMSGINFO:
			return;
		default:
			min = fBOX, max = fACTIVE;
	}
	/* editable fields */
	for (i = min; i <= max; i++)
		if (field[i].l > 0)
		{
			if (keystate == ksFIND)
			switch(i)
			{
				case fTTLPAID:
				case fPAIDTO:
				case fREMIND:
				case fCALLS:
				case fMSGS:
					loffs = 2;
				break;
				default: loffs = 0;
			}
			writefield(i, loffs, user);
		}
#endif
}

void writefield(int i, int loffs, US *user)
{
	writestr( 
			 field[i].x,
			 field[i].y,
			 0,         /* start from start of string */
			 field[i].l + loffs,
			 field[i].attr,
			 selectfield(i, user) /* string to display */
			);
}

void activateuser(void)
{                               
	int box;
	/* check that there is a pin number before saving */
	if (useridx[curruser].avail < ACTIVE) 
	{
		box = atoi(user.box);
		phoneactivate(box, &user);
		saveuser(FUSER, box, &user);
	}    
}   

void deactivateuser(void)
{
	int box;
	/* only deactivate active users not currently on line */
	if (useridx[curruser].avail > INACTIVE)
	{
		box = atoi(user.box);
		phonedeactivate(box, &user);
		saveuser(FUSER, box, &user);
	}
}

void openuserfile(int *fuser)
{                          
    //for auto pay function to work
    openpaycodes();

	if ((*fuser = _open(USERFILE, _O_BINARY | _O_RDWR )) == -1) 
	{
		newuserfile = TRUE;
		if ((*fuser = _open(USERFILE, _O_BINARY | _O_CREAT | _O_RDWR, _S_IWRITE )) == -1) 
		{
			sprintf(s, "Unable to open or create file %s.", USERFILE);
			initmsg(s);
			exit (1);
		}
		//put a blank record in first position.
		_lseek(*fuser, 0L, SEEK_SET);
		_write(*fuser, &blankuser, sizeof(struct userstruct));
		_commit(*fuser);
	}    
}

int invalidbox(US *u)
{
	unsigned register i;
	
	if (strlen(u->box) != field[fBOX].l) return TRUE;
	
	for (i = 0; i < field[fBOX].l; i++)
		if (u->box[i] < '0' || u->box[i] > '9') return TRUE;

	return FALSE;
}

int isdef(US u)
{
	if (u.active == ISDEF)
	{
		return TRUE;
	}
	return FALSE;
}

void loaddef( long pos, US u )
{
#ifdef TIMELIMIT
	time_t now;

	//get default info and check last date program used
	time(&now);
	if (now < user.acctime - DAYSECS)
	{
		sprintf(s,"Invalid system time!");
		initmsg(s);
		stopsys();
		exit(0);
	}
#endif
	defpos = pos; //save record position
	defuser = u;
	announce = ANNCSTATE; //load announce variable
	if (strlen(defuser.misc) > 0)
		strcpy(password, defuser.misc);
	if (strlen(defuser.pin) > 0 && strlen(defpin) == 0)
		strcpy(defpin, defuser.pin);
	if (strlen(defuser.admin) > 0)
		strcpy(skeletonpin, defuser.admin);
	
}

void savedef( void )
{
#ifdef TIMELIMIT
	time_t now;
    
	time(&now);
	defuser.acctime = now;
#endif
#ifdef SN
	strcpy(SERIALNUMBER, SN); // save serial number
#endif
	sprintf(defuser.box, "%04d", ISDEF); //indicate that record is defaults
	strcpy(defuser.misc, password); 
	strcpy(defuser.pin, defpin); 
	strcpy(defuser.admin, skeletonpin); 
    defuser.active = ISDEF;
	ANNCSTATE = announce; //save announce variable

	if (defpos == NOTFOUND)
	{
		_lseek(FUSER, 0L, SEEK_END);
		defpos = _tell(FUSER);
	}
	else _lseek(FUSER, defpos, SEEK_SET);
	
	_write(FUSER, &defuser, sizeof(US));
	_commit(FUSER);
	_lseek(FUSER, 0L, SEEK_SET);

}

void inituser(int *fuser)
{
	long pos;
	int numrec, i;
	int box;
	
	/* initialize useridx */
	for (i = 0; i < MAXBOX; i++) {
		userpos[i] = NOTFOUND;
		useridx[i].avail = NOTFOUND;
	//	admn[i].admn = NOTFOUND;
	}                             
	
	/* initialize struct userstruct variables */
	blankuser.active = INACTIVE;
	finduser.start = 
	finduser.calls = 
	finduser.msgtime = 
	finduser.acctime =
	finduser.nextmsg = 
	finduser.active = NOTFOUND;
	finduser.boxtype = btANY;

	curruser = 0;
	curractive = 0;
	currinactive = 0;
	
	if (!newuserfile)
	{                     
		/* go to start of file */
		_lseek(*fuser,0L, SEEK_SET);  
		numrec = (int)(_filelength(*fuser)/sizeof(US));
#ifdef MAXUSERS
		if (numrec > MAXUSERS) numrec = MAXUSERS;
#endif		
		for (i = 0; i < numrec; i++) 
		{
			pos = _tell(*fuser);
			_read(*fuser, &user, sizeof(US));
			
			box = atoi(user.box);
			//check for default settings box
			if (isdef(user)) 
			{
				loaddef(pos, user);
				continue; 
			}
			//if box is not four digits go to next iteration
			if (strlen(user.box) > 0 && invalidbox(&user))
			{
				if (strlen(user.box) == 0) continue;
#ifndef DAEMON				
				printf("ERROR: invalid box number %s found at %ld. ", user.box, i);
				wait(5);
#else
				sprintf(s, "ERROR: invalid box number %s found at %ld.\n", user.box, i);				
				initmsg(s);
#endif				
				continue;
			}
			//enter position of record in index
			userpos[box] = i;
			//enter whether user is ACTIVE, INACTIVE or ONHOLD
			useridx[box].avail = user.active;
			//increment number of active boxes if active
			if (user.active == ACTIVE)
			{
				//define default box if found (goes to this box if no entry)
				if (user.boxtype == btDEF)
				{
					if (defbox == NOTFOUND)
						defbox = box;
				}
	/*			if (boxnumeric(user.admin)) 
				{
					admn[box].admn = atoi(user.admin);
				}
	*/			
				useridx[box].type = user.boxtype;
				numusers++;
			}

		}     
		_lseek(*fuser, 0L, SEEK_SET);
		/* find first active record */  
		while (useridx[curruser].avail <= INACTIVE && curruser < MAXBOX) 
			curruser++;
		if (curruser == MAXBOX) curruser = 0;    
		if (useridx[curruser].avail <= INACTIVE) 
			curractive = curruser;
	}
}

int checkpassword(int *i, int *pos, int *startpos)
{
	char cp[DOSPATHLEN];
	
	strcpy(cp, checkpass);
	checkpass[0] = '\0';
	keystatesetup(ksVIEW, i, pos, startpos);
	if (strlen(password) == 0 || strcmp(password, cp) == 0) 
		return TRUE;
	writemsg("Invalid password!");
	return FALSE;
}

int finduserrec(void)
{    
	int box2find;   
	//this struct keeps pointers to finduser and string lengths of each field
	//use this struct to avoid repetition of work
	struct findinfo
	{
		unsigned l;
		char *s;
	}
	finfo[fMAX] = {0};
	int totallen = 0; 	//total number of characters in finduser
	int i;				//count variables
	register j;
	US cs; 				//user structure and string pointer
	char *userstr;
	
	writemsg("Searching..."); //default message
	//keystate has to be changed to uncover PIN and ADMIN fields in user struct
	keystate = ksFIND;
	//load up pointers to finduser and string lengths
	for (i = poslimit[ksFIND].min; i <= poslimit[ksFIND].max; i++)
	{
		finfo[i].s = selectfield(i, &finduser);
		finfo[i].l = strlen(finfo[i].s);
		if (i == fBOXTYPE && finduser.boxtype == btANY ||
			i == fLANGUAGE && findlanguage == maxlanguage) continue;
		totallen += finfo[i].l;
	}
	
	/* if the finduser variable is blank exit func */
	if (totallen == 0 && finduser.active != ONHOLD) 
	{
		writemsg("Nothing to find! Use Alt-F to edit find record. Press a key...");
		return FALSE;
	} 
	
	/* if there is a full box number find that box */
	else if ( finfo[fBOX].l && boxnumeric(finduser.box) ) 
	{
		box2find = atoi(finduser.box);
		if (box2find != CAL) {
			sprintf(finduser.box, "%04d", box2find);
			curruser = box2find;
			writemsg("Box found. Press a key...");
			return TRUE;
		}
	}
	else 
	{
	/* if not, go through the file and find the record containing the other
	   info */          
	   i = curruser;
	   do
	   {    
			if (i < MAXBOX - 1) i++;
			else i = 0;       
			// check to see if record is on file
			if ( userpos[i] != NOTFOUND )
			{
				// let the phone board process calls
				sched(); 
				// get record from file
				_lseek(FUSER, USERPOS(i), SEEK_SET);
				_read(FUSER, &cs, sizeof(struct userstruct));
				_lseek(FUSER, 0, SEEK_SET);
				
				if (finduser.active != NOTFOUND)
				{
					if (finduser.active != cs.active) continue;
				}
				
				//check for box type go to next record if they are not equal
				if (finduser.boxtype != btANY && 
					finduser.boxtype != cs.boxtype) continue;
				
				// compare fields 
				for (j = poslimit[ksFIND].min; j <= poslimit[ksFIND].max; j++) 
				{
					//ignore if you are not comparing language
					if (j == fLANGUAGE)
					{
						if (findlanguage == maxlanguage) continue;
					}
					//ignore box type
					if (j == fBOXTYPE) continue;
					//get find field and check for length
					if (finfo[j].l == 0) continue;
					//get user string
					userstr = selectfield(j, &cs);
					//compare fields, if text not found go to next record
					if (fldcmp(finfo[j].s, userstr, &cs, j) == NOTFOUND) break;
				}   
				//check for last field
				if (j > poslimit[ksFIND].max && i != CAL) 
				{
					//box found
					writemsg("Box found. Press a key...");
					curruser = i;
					return TRUE;
				}
			}
	   }
	   while (i != curruser);
   }   
   writemsg("No records matched your search criteria...");
   return FALSE;
}

int checkfld(int fld, US *u)
{
	switch (fld)
	{
		case fACTIVE:
			if (finduser.active == NOTFOUND || u->active == finduser.active) 
				return TRUE;
			return NOTFOUND;
		break;
		case fBOXTYPE:
			if (finduser.boxtype == btANY || u->boxtype == finduser.boxtype) 
				return TRUE;
			return NOTFOUND;
		break;
		default: return ERR;
	}
}

int fldcmp(char *find, char *user, US *u, int fld)
{
	int rc; //result of field comparison 
	char rcstr[20] = "";
	char *fstr, ftoken; //find string and token (if any) for search. see below
	static int istag = FALSE; //indicates if you are getting find comparison field from other field
	int tag; //which field it is (alphabetical A-T)
	
	//check for specific fields
	if ((rc = checkfld(fld, u)) != ERR) return rc;
	//get token
	ftoken = find[0];

	//check for tag
	if (istag) 
	{
		istag = FALSE;
		if ((rc = strlen(find)) == 0) return NOTFOUND;
		tag = toupper(find[rc - 1]) - 'A';
		if (tag > fMAX) return NOTFOUND;
		fstr = selectfield(tag, u);
	}
	else fstr = find + 1; //skip over tag when assigning string
	
	switch (ftoken)   //TOKENS: special characters that instruct comparison
	{
		case WILDCARD://?
			if (strlen(user) > 0) return TRUE;
			return NOTFOUND;
		case EMPTY:   //~
			if (strlen(user) == 0) return TRUE;
			return NOTFOUND;
		case GREATER: //>
		case LESSER:  //<
			rc = cmpfld(user, fstr, fld);
			if (rc == ERR) return NOTFOUND;
			if (rc > 0 && ftoken == GREATER) return TRUE;
			else if (rc < 0 && ftoken == LESSER) return TRUE;
			return NOTFOUND; 
		case TAG:     //@ (do recursion here)
			istag = TRUE;
			return fldcmp(find + 1, user, u, fld);
		default: 
			if (field[fld].type == ALPHANUM) return pos(find, user);
			if (cmpfld(user, find, fld) == 0) return TRUE;
			return NOTFOUND;
		}
}

int isnumeric(char *fstr)
{
	register i;
	int l = strlen(fstr);
	
	for (i = 0; i < l; i++)
		if (fstr[i] < '0' || fstr[i] > '9') return FALSE;
	return TRUE;
}

int cmpnumbers(char *user, char *fstr)
{
	long u, f;
	if (!isnumeric(user)) return ERR;
	if (!isnumeric(fstr)) return ERR;
	u = atol(user);
	f = atol(fstr);
	if (u > f) return 1;
	if (u == f) return 0;
	return -1;
}

int cmpdates(char *user, char *fstr)
{
	int mu, du, yu, mf, df, yf;
	time_t us, fs;
	
	if (!getdate(user, &du, &mu, &yu)) return ERR;
	if (!getdate(fstr, &df, &mf, &yf)) return ERR;
	
	us = getdatesecs(du, mu, yu);
	fs = getdatesecs(df, mf, yf);

	if (us > fs) return 1;
	else if (us < fs) return -1;
	else return 0;
}

int cmpfld(char *user, char *fstr, int fld)
{
	int rc, d, m, y;
	switch (field[fld].type)
	{
		case NUMERIC:
			return cmpnumbers(user, fstr);
		case DATE:
			return cmpdates(user, fstr);
		default: 
			//ignore blank fields
			if (strlen(user) == 0) return ERR;
			//check for number
			if (isnumeric(fstr))
				return cmpnumbers(user, fstr);
			//check for date
			if (getdate(fstr, &d, &m, &y))
				return cmpdates(user, fstr);
			//otherwise compare as strings
			rc = stricmp(user, fstr);
			if (rc > 0) return 1;
			if (rc < 0) return -1;
	}
}

void editkeystate( KS keystate, 
				   enum fields i,
				   int *startpos, 
				   int *pos, 
				   char key, 
				   char zero)
{
	int len, x, type;
	char *str;
	
	type = field[i].type;
	len = field[i].l;
	x = field[i].x;
	
	switch(keystate)       
	{
		case ksMSGINFO:
		case ksVIEW:
		return;
		
		case ksRANDOMPIN:
		case ksCOPYTEMPLATE:
		case ksBLOCKACTIVATE:
			len = 4;
			x = editstart;
			str = numactstr;
			type = NUMERIC;
		break;
		
		case ksRESET:
			len = 2;
			x = editstart;
			str = channelstr;
			type = NUMERIC;
		break;
				
		case ksEDITPASSWORD:
		case ksPASSWORD:
			coverchar = COVERCHAR;
			x = editstart;
			str = checkpass;
		break;        
		
		case ksTIME:
			len = 5;
			x = editstart;
			str = newtimestr;
		break;
		
		case ksRENOLDN:
			len = DOSPATHLEN;
			x = editstart;
			str = ren.file;
		break;
		
		case ksRENNEWN:
			len = DOSPATHLEN;
			x = editstart;
			str = ren.name;
		break;
		
		case ksMOVEMSG:
			len = 4;
			x = editstart;
			str = targetbox;
			type = NUMERIC;
		break;
		
		case ksFIND:            
			usewildcard = TRUE; /* lets you put "?" or "~" in a numeric field */
			if (i >= fPAIDTO) len += 2;
			str = selectfield(i, &finduser);
			type = ALPHANUM;
		break;
		
		case ksBACKUP:
		case ksEDIT:
			str = selectfield(i, &user);
		break;
	}
	//do edit
	edit(x, 
		 field[i].y, 
		 len,
		 startpos,
		 pos,
		 len,
		 field[i].attr,
		 key,
		 zero,            
		 type,              
		 str);
		 
	//reset values to default;
	coverchar = 0;     
	usewildcard = FALSE;  
}

void godown(enum fields *j)
{                          
	enum fields i = *j;
	if (i >= poslimit[keystate].max) i = poslimit[keystate].min;
	else do i++;
		 while (i < poslimit[keystate].max && field[i].type == NOTHING);

	_settextposition(field[i].y, field[i].x);
	*j = i;
}

int newuser( void )
{
	if (user.active == INACTIVE && newboxes <= 0)
	{
		writemsg("Not enough disk space to store new user record & messages. Press a key...");
		return FALSE;
	}
	return TRUE;
}

void keystatesetup(KS ks, int *i, int *pos, int *startpos)
{
	time_t now;
	static int findfld = fBOX; //both of these should be poslimit[ks].min val
	static int editfld = fPIN;
    static int findpos = 0;
	switch (keystate)
	{
		case ksFIND:
			findfld = *i;
			findpos = *pos;
		break;
		case ksEDIT:
			editfld = *i;
		break;
	}
	
	/* zero pos and startpos */
	*pos = *startpos = 0;       
	/* set current field */
	*i = poslimit[ks].min;  
	/* set cursor etc */               
	switch (ks)
	{
		case ksMSGINFO:
			editstart = NOTFOUND;
		break;
		case ksRENOLDN:
			strcpy(ren.file, "");
			strcpy(ren.name, "");
		case ksRENNEWN:  
		case ksMOVEMSG:
		case ksPASSWORD: 
		case ksEDITPASSWORD:
			editstart = strlen(msg[ks]) + 2;
		break;
		case ksTIME:
			time(&now);
			//create time string
			strncpy(newtimestr, timestr(&now) + DATELEN + 1, NEWTIMELEN);
			editstart = strlen(msg[ks]) - NEWTIMELEN + 2;
			//copy time string to end of prompt
			strncpy(msg[ks] + strlen(msg[ks]) - NEWTIMELEN, newtimestr, NEWTIMELEN);
		break;
		case ksEDIT:
			*i = editfld;
		case ksVIEW:
			if (keystate == ksEDIT) updateUSER();
		case ksBACKUP:
			editstart = field[*i].x;
		break;
		case ksFIND:
			*i = findfld;
			editstart = field[*i].x;
			/*
			switch (*i)
			{
				case fBOX:
				case fPIN:
				case fADMIN:
					editstart = field[*i].x;
				break;
				default: 
					*pos = findpos;
					editstart = findpos + field[*i].x;
			}
			*/
		break;
		default:
			strcpy(numactstr, "1");
			numact = 1;
			switch (ks)
			{
				case ksRESET:
				case ksRANDOMPIN:
				case ksCOPYTEMPLATE:
					editstart = strlen(msg[ks]) + 1;
				break;
				case ksBLOCKACTIVATE: 
					if (user.active == ACTIVE)
						editstart = strlen(msg[ks]) + 1;
					else editstart = strlen(msg[ks]) - 1;
			}
	}        
	
	//sprintf(s, "field: %s, x = %d, y = %d.", field[*i].name, field[*i].x, field[*i].y);
	
	if (editstart == NOTFOUND)
		_settextcursor(NOCURSOR);
	else 
	{
		_settextcursor(cursortype);
		_settextposition(field[*i].y, editstart);
	}
	
	/* write the default message for keystate */
	//trim first two characters of block activate message if you are activating
	if (ks == ksBLOCKACTIVATE && user.active != ACTIVE)
		offs = 2;
	else offs = 0;
	
    tagsvisible( FALSE );

	switch(keystate)
	{
		//force keystate to show titles
		case ksEDIT:
			if (ks != ksBACKUP && ks != ksMSGINFO) 
			{ keystate = ks; break;}
		case ksFIND:
		case ksBACKUP:
		case ksMSGINFO:
			keystate = ks;
			showttls();
		break;
		default:
			keystate = ks;
	}
	writemsg( CURRSTR );
	if (title[ks] != NULL) 
		writettl( title[ks] );
	if (keystate == ksMOVEMSG) 
		writestr(editstart, HELP_TOP, 0, BOXLEN, HELPATTR + fgWHITE, targetbox);
}

void tagsvisible( int st )
{
	static showtags = TRUE;
	int j;
	char str[3];
    
    if ( st != NOTFOUND ) showtags = st;
	else if (keystate == ksFIND) showtags = !showtags;
	else showtags = FALSE;
	
	for (j = fBOX; j <= fACTIVE; j++)
	{
		if (keystate == ksFIND && showtags) 
		{
			str[0] = j + 'A';
			str[1] = '\0';
		}
		else strcpy(str, "");
		writestr( 
				 field[j].x - 1,
				 field[j].y,
				 0,         /* start from start of string */
				 1,
				 FLDATTR - 8,
				 str /* string to display */
				);
	}
}	
	
int changekeystate(KS ks, int *i, int *pos, int *startpos)
{                             
	/* change keystate only if you are going back to ksVIEW or
	   starting from ksVIEW or if you are starting from */
	if (keystate == ksVIEW || ks == ksVIEW) {
		/* make keystate = new keystate */
		keystatesetup( ks, i, pos, startpos);
		/* indicate that change was successful */
		return TRUE;
	}
	else return FALSE;
}

int checkactive(int active, int curruser)
{
	switch(active) {
		case ONHOLD:
		case INACTIVE:
			if (useridx[curruser].avail <= INACTIVE) return FALSE;
			else return FALSE;
		break;
		case ACTIVE:
			if (useridx[curruser].avail <= INACTIVE) return TRUE;
			else return FALSE;
		break;
	}
}

int up(int active)
{
	int olduser = curruser;
	do
	{
		if (curruser > 0)
			curruser--;
		else curruser = MAXBOX - 1; 
	}
	while (curruser == CAL ||
		   (curruser != olduser &&
		    checkactive(active, curruser)));
	return curruser;       
}

int down(int active)
{
	int olduser = curruser;
	do
		if (curruser < MAXBOX - 1)
			curruser++;
		else curruser = 0;    
	while (curruser == CAL ||
		   (curruser != olduser && 
		    checkactive(active, curruser)));
	return curruser;       
}

void copytemplate(US *user)
{
	int c;
	for (c = poslimit[ksEDIT].min; c <= poslimit[ksEDIT].max; c++)
	{
		if (c == fBOXTYPE) user->boxtype = temp.boxtype;
		else strcpy(selectfield(c, user), selectfield(c, &temp));
	}
	showuser(user);
	writemsg("Template copied...");
}

int backupchanged( void )
{
	if (strcmp(backup.hour, savebup.hour) != 0) return TRUE;
	if (strcmp(backup.min, savebup.min) != 0) return TRUE;
	if (strcmp(backup.path, savebup.path) != 0) return TRUE;
	if (strcmp(backup.frequency, savebup.frequency) != 0) return TRUE;
	if (strcmp(backup.purgedays, savebup.purgedays) != 0) return TRUE;
	if (strcmp(backup.remind, savebup.remind) != 0) return TRUE;
	if (strcmp(backup.deactivate, savebup.deactivate) != 0) return TRUE;
	return FALSE;
}

int checkbackup( void )
{
	int h, m;
	char bup[DOSPATHLEN + 1], cwd[DOSPATHLEN + 1];
	
	//fix number of days to wait before purging a file
	sprintf(backup.purgedays,"%2d", (purgedays = atol(backup.purgedays)));
	purgedays = labs(purgedays);
	purgedays *= DAYSECS; //this is actually measured in seconds 
    //and number of days before the cutoff to remind user
	sprintf(backup.remind,"%2d", (bupremind = atol(backup.remind)));
	bupremind = labs(bupremind);
	bupremind *= DAYSECS; //this is actually measured in seconds 
	//and number of days after cutoff to deactivate user
	sprintf(backup.deactivate,"%2d", (bupdeactivate = atol(backup.deactivate)));
	bupdeactivate = labs(bupdeactivate);
	bupdeactivate *= DAYSECS; //this is actually measured in seconds 
    //fix back up frequency (# of hours between back ups)
	bupfreq = atoi(backup.frequency);
	bupfreq = abs(bupfreq);
	if (bupfreq < 1) 
		bupfreq = 24;
	sprintf(backup.frequency, "%2d", bupfreq);
	//fix hour and minute strings
	if (strlen(backup.hour) > 0)
		sprintf(backup.hour, "%2d", abs(atoi(backup.hour)));
	else if (strlen(backup.min) > 0) strcpy(backup.hour, " 0");
	
	if (strlen(backup.min) > 0) 
		sprintf(backup.min, "%02d", abs(atoi(backup.min)));
    else if (strlen(backup.hour) > 0) strcpy(backup.min, "00");
    
	//check time to see if it is valid
	h = atoi(backup.hour);
	m = atoi(backup.min);
	if (h > 23 || m > 59)
	{
		writemsg("Invalid back up time! Press a key...");
		return FALSE;
	}
	
	//check back up path
	removeslash(backup.path);
	if (strlen(backup.path) > 0) 
	{   
		_getcwd(cwd, DOSPATHLEN);
		if (_fullpath(bup, backup.path, DOSPATHLEN) == NULL)
		{
			if (mkdir(backup.path) == -1 && errno == ENOENT)
			{
				writemsg("Back up directory not valid! Press a key...");
			}
			else 
			{
				_fullpath(bup, backup.path, DOSPATHLEN);
				writemsg("New back up directory created. Press a key...");
			}
			strcpy(backup.path, bup);
			chdir(cwd);
			return FALSE;
		}
		if (strcmp(cwd, backup.path) == 0)
		{
			writemsg("Error: source and target are the same! Press a key...");
			return FALSE;
		}
		strcpy(backup.path, bup);
		chdir(cwd);
	}

	return TRUE;
}

int _gotouser(unsigned char *key, int active)
{
	int olduser;
	olduser = curruser;
    
	if (keystate != ksVIEW) return FALSE;

	switch (*key)
	{
		case PGUP:
		case CtrlPGUP:
			up(active);
		break;    
		case PGDN:
		case CtrlPGDN:
			down(active);
		break;
		case HOME:
		case CtrlHOME:
			curruser = 0;
			if (checkactive(active, curruser)) down(active);            
		break;
		case END:
		case CtrlEND:    
			curruser = MAXBOX - 1;
			if (checkactive(active, curruser)) up(active);
		break;
	}    
	 
	if (curruser != olduser) 
	{
		getuserrec(FUSER, curruser, &user);
		showuser(&user);
	}    
	
	return TRUE;
}

int checkpin(char *pin)
{           
	if (keystate == ksEDIT) 
	{
	    //always allow people to go back to the start
	    if (pin[0] == '*') return FALSE; 
	    /* you can't have one that is the skeleton indicator either */
	    if (strcmp(pin, isskeleton) == 0) return FALSE; 
		return TRUE;
	}
	return TRUE;
}

void deluser( int curr, US *u )
{
	//modify message and disk space variables
	u->nextmsg = 0;
	//delete boxes while getting old size
	diskused -= delboxsize(u->box);
	//update data
	saveuser(FUSER, curr, u);
	//update screen if user is the same as 
	if (u == &user) 
		showuser(&user);
}

char *inccs(char *cs)
{
	char *rs = cs;
	
	switch(cs[0])
	{
		case '<':
		case '>':
			rs++;
		break;
		case '@':
			rs++;
			if (cs[1] == '>' || cs[1] == '<') 
				rs++;
		break;
	}
	
	return rs;
}

void pastefld(US *user, char *buff, int pos, int fld)
{
	char s[DOSPATHLEN] = "";
	char *fs = selectfield(fld, user);
	// put new info in field
	strncpy(s, fs, pos); // copy first fragment from field
	strcat(s, buff); // add buffer
	if (strlen(s) < field[fld].l) //if not too long add rest of field
		strcat(s, fs + pos);
	if (strlen(s) > field[fld].l)  // if it is too long truncate it
		strncpy(fs, s, field[fld].l);
	else strcpy(fs, s); //copy result to field
	// show results
	showuser(user);
}

void gouser(int i)
{
	curruser = i;
	getuserrec(FUSER, curruser, &user);
	showuser(&user);
}

void blockactivate( void )
{
	int i, state, error = FALSE, action = aBLOCKACTIVATE;
	
	saveuser(FUSER, curruser, &user);
	state = user.active;
	if (numact > 1)
	{
		if (user.active <= INACTIVE)
			writemsg("Activating boxes now. Please wait...");
		else 
			writemsg("Deactivating boxes now. Please wait...");
    }
//	else writemsg("Single box.");

	i = curruser;
	for (; curruser < MAXBOX, curruser < i + numact; curruser++)
	{
		sched();
		getuserrec(FUSER, curruser, &user);
		switch (state)
		{
			case ONHOLD: break; //ignore any boxes that are on hold
			case INACTIVE: 
			case NOTFOUND:
				if (exitedit(&action)) 
					activateuser(); 
				else error = TRUE;
				break;
			default: deactivateuser(); 
		}
		if (error) break;
	} 
	// go back to start
	gouser(i);
	numact = 1;
	offs = 0;
	if (!error) writemsg("Finished.");
}

void blockcopytemplate( void )
{
	int i;
	i = curruser;
	writemsg("Copying template...");
	for (; curruser < i + numact; curruser++)
	{
		sched();
		getuserrec(FUSER, curruser, &user);
		copytemplate(&user);
		saveuser(FUSER, curruser, &user);
	}
	// go back to start
	gouser(i);
	writemsg("Finished copying template.");
}

int makerandpin (US * u)
{
	int pin;
	// NOTE, this function is not limited by RAND_MAX 
	// as the documentation says it should be
	pin = abs(((rand()) % RAND_MAX)); 
	if (pin < RAND_MAX && pin >= 0)
	{
		sprintf(u->pin, "%04d", pin);
		u->pin[PINLEN] = '\0';
		showuser(u);
		return TRUE;
	}
	else writemsg("ERROR: generated pin invalid!"); 
	return FALSE;
}

void randompin( void )
{
	register i;
	i = curruser;
	writemsg("Creating random security codes...");
	for (; curruser < i + numact; curruser++)
	{
		sched();
		getuserrec(FUSER, curruser, &user);
		if (makerandpin(&user))	saveuser(FUSER, curruser, &user);
	}
	// go back to start
	gouser(i);
	writemsg("Finished creating random security codes.");
}
/*
void remindall( char *c )
{
	register i;
	US u;
	writemsg("Changing reminders. Please Wait...");
	for (i = 0; i < MAXBOX; i++)
	{
		if (useridx[i].avail != INACTIVE)
		{
			sched();
			loaduser(FUSER, i, &u);
			strcpy(u.remind, c);
			saveuser(FUSER, i, &u);
		}
	}
	getuserrec(FUSER, curruser, &user);
	showuser(&user);
	writemsg(CURRSTR);
}
*/
int renfiles( void )
{
	if (strlen(ren.file) == 0 || strlen(ren.name) == 0)
	{
		writemsg("Error: missing file name!");
		return action;
	}
	if (strcmp(ren.file, ren.name) == 0)
	{
		writemsg("Error: files have the same name!");
		return action;
	}
	
	if (!rename(ren.file, ren.name)) //return of zero = OK
	{
		if (stricmp(ren.name, states[sANNOUNCE].fname) == 0)
		{
			sprintf(s, "Rename ok. Create new announcement? Y/N (Enter = Y)");
			action = aANNOUNCE;
		}
		else if (stricmp(ren.file, states[sANNOUNCE].fname) == 0)
		{
			sprintf(s, "Rename ok. Announcement deleted.");
			announce = FALSE;
			_vhclose(states[sANNOUNCE].fh);
		}
		else sprintf(s, "Renamed %s %s. Press a key...", ren.file, ren.name);
	}
    else

	switch(errno)
	{
		case EACCES: sprintf(s, "ERROR: %s already exists!", ren.name);
		break;
		case ENOENT: sprintf(s, "ERROR: %s does not exist!", ren.file);
		break;
		case EXDEV: sprintf(s, "ERROR: attempting to move file to different drive!");
		break;
	};
	writemsg(s);
	return action;
}

void initannounce( void )
{
	if ((states[sANNOUNCE].fh = vhopen(states[sANNOUNCE].fname,READ)) > 0) 
	{
		announce = TRUE;
		writemsg("New announcement initialized.");
	}
	else writemsg("Failed to create new announcement - unable to open file.");
}

#define YES (*zero == FALSE && (toupper(*key) == 'Y' || *key == ENTER))
#define NO  (*zero == FALSE && toupper(*key) == 'N')
#define REMIND (*zero == FALSE && (toupper(*key) == 'R'))
#define CLEAR (*zero == FALSE && (toupper(*key) == 'C'))

void shutdown (unsigned char *key, char *zero)
{                    
	writemsg("System shutdown: please wait (press Ctrl-F to force shutdown) ... ");
	pagestate = NOPAGE;
	endprogram = TRUE;
}

void boxdaymsg( US u )
{
	char bdud[TIMELEN + 1];
	strncpy(bdud,selectfield(fBDUPDATE, &u),9);
	bdud[9] = 0;
	sprintf(s, "Boxdays: %7ld @ $.%02d %7ld new %s. Edit Bill Clear Update all",
				u.bdowed,
				BDVAL(u),
				u.boxdays,
				bdud);
	writemsg(s);
}

int doaction(int *action, unsigned char *key, char *zero, 
			 int *i, int *pos, int *startpos)
{
	struct _dostime_t t; //for changing system time;
	US u;
	
	/* section deals with messages that need a keystroke response */
	switch (*action)
	{
		case aNOACTION: 
			break;
		case aVIEW:
			if YES 
			{
				saveuser(FUSER, curruser, &user);
				changekeystate(ksVIEW, i, pos, startpos);
			}    
			else if NO 
			{
				getuserrec(FUSER, curruser, &user);
				changekeystate(ksVIEW, i, pos, startpos);
			}
			showuser(&user);
			break;
		case aEDITFINDNEXT:
			if YES
				saveuser(FUSER, curruser, &user);

			if (YES || NO)
			{
				changekeystate(ksVIEW, i, pos, startpos);
				finduserrec();
				getuserrec(FUSER, curruser, &user);
				showuser(&user);
			}
			break;
		case aSAVEUSER:
			if YES 
			{
				saveuser(FUSER, curruser, &user);
			}    
			else getuserrec(FUSER, curruser, &user);
			showuser(&user);
			break;
		case aEDSAVENOEXIT:
			if YES
			{
				saveuser(FUSER, curruser, &user);
				showuser(&user);
				writemsg("Edit saved.");
			}
			else showuser(&user);
			break;
		case aDELUSER:
			if YES
				deluser(curruser, &user);
			break;
		case aDEFPIN:
			if YES
			{
				strcpy(defpin, selectfield(fPIN, &user));
				sprintf(s,"Default PIN is now %s. Press a key...", defpin);
				writemsg(s);
			}
		case aBLOCKACTIVATE:
			if YES 
			{
				blockactivate();
			}
			break;
		case aCOPYTEMPLATE:
			if YES
			{
				blockcopytemplate();
			}
			break;
		case aRANDOMPIN:
			if YES
			{
				randompin();
			}
			break;
		case aEDITUSER:
			if (checkpassword(i,pos,startpos))   
			{
				keystate = ksVIEW;
				if (changekeystate(ksEDIT, i, pos, startpos)) 
					showuser(&user);
			}
			break;
/*		case aDOREMIND:
			if REMIND 
			{
				writemsg("Change all \"REMIND?\" fields to \"Y\". Continue? Y/N (Enter = Y)");
				return aREMINDALL;
			}   
			else if CLEAR 
			{
				writemsg("Clear all \"REMIND?\" fields. Continue? Y/N (Enter = Y)");
				return aCLEARALL;
			}
			break;
		case aREMINDALL:
			if YES remindall(RM);
			break;			
		case aCLEARALL:  
			if YES remindall(NORM);
			break;
*/			
		case aAUTOBACKUP:
			if YES kbaction = kbBACKUP;
			writemsg(CURRSTR);
			break;
		case aBACKUP: 
			if YES kbaction = kbBACKUP;
		break;
		case aPARTBUP:
			if YES
			{	
				writemsg("REMIND, DEACTIVATE or PURGE boxes as well? Y/N (Enter = Y)");
				return (*action = aCHKBOXES);
			}
		break;
		case aCSVBUP: 
			if YES kbaction = kbCSVBUP;
		break;
		case aCSVABUP:
			if YES kbaction = kbCSVABUP;
		break;
		case aCHKBOXES:
			if YES chkboxes = TRUE;
			else chkboxes = FALSE;
			kbaction = kbPARTBUP;
		break;
		case aKEEPBACKUP:
			if YES 	
			{
				savebup = backup;
				savebackupparams();
				showuser(&user);
				writemsg("New back up parameters saved.");
			}
			else 
			{
				backup = savebup;
				showuser(&user);
				writemsg("Old back up parameters restored.");
			}
        break;
		case aRESET:
			if YES resetports();
			else if NO 
			{
				*action = aRESETSINGLE;
				writemsg("Reset a single channel? Y/N (Enter = Y)");
				return aRESETSINGLE;
			}
		break;
		case aRESETSINGLE:
			if YES keystatesetup(ksRESET, i, pos, startpos);
		break;
		case aRESETCONFIRM:
			if YES resetchannel(atoi(channelstr));
		break;
		case aRENAME:
			if YES
			{
				if ((*action = renfiles()) == aANNOUNCE) 
					return *action;
			}
		break;  
		case aDELSINGLEMSG:
		case aMOVEMSG:
			if (!YES)
			{
				*action = aNOACTION;
			}
			viewmsginfo(*key, *zero, i, pos, startpos);
		break;
		case aANNOUNCE:
			if YES
				initannounce();
		break;
		case aTIME:
			if YES
			{
				_dos_gettime(&t);
				if (((t.hour = atoi(newtimestr)) <= 23) &&
					((t.minute = atoi(newtimestr + 3)) <= 59))
					_dos_settime(&t);
			}
		break;
		case  aCLEARBOXDAYS:
			if YES
			{
				loaduser(FUSER, curruser, &u);
				u.bdowed = user.bdowed = 0L;
				saveuser(FUSER, curruser, &u);
				writemsg("Billed box days cleared.");
			}
		break;
		case aCLEARPAGEQ:
			if YES
			{
				writemsg("Wait...");
				clearpgq();
				writemsg("Cleared paging queue.");
			}
		break;
		case aMAINTENANCE:
			if YES
			{
				if (maintenance) maintenance = FALSE;
				else maintenance = TRUE;
			}
		break;
		case aEXITPROGRAM:
			if (keystate == ksPASSWORD)
			{
				if (checkpassword(i,pos,startpos))
					shutdown(key, zero);
				break;    
			}
			else 
			{
				if YES
					shutdown(key, zero);
				else *key = 0;
			}
			break;
	}
	return aNOACTION;
}

void clearfind( int *i, int *pos, int *startpos )
{
	finduser = blankuser;
	// below indicates that this is finduser and not user 
	finduser.start = 
	finduser.calls = 
	finduser.acctime = 
	finduser.msgtime =
	finduser.nextmsg = 
	finduser.active = NOTFOUND;
	finduser.boxtype = btANY;
	findlanguage = maxlanguage;
	//strings for selectfield
	strcpy(fnextmsgstr, "");
	strcpy(fcallstr, "");
	strcpy(flastcallstr, "");
	strcpy(fstartstr, "");
	strcpy(fpaidtostr, "");
	strcpy(fremindstr, "");
	
	*i = poslimit[ksFIND].min;
	*startpos = 0;
	*pos = 0;
	_settextposition(field[*i].y, field[*i].x);
}   

void clearuser(int *i, int *pos, int *startpos)
{        
	char *f; //field string
	int c; //field identifier
	//reset cursor parameters
	*i = poslimit[keystate].min;
	*startpos = 0;
	*pos = 0;
	//change all fields to null string
	for (c = poslimit[keystate].min; c <= poslimit[keystate].max; c++)
	{
		f = selectfield(c, &user);
		if (c != fBOXTYPE && c != fPIN && f != NULL) strcpy(f, "");
	}
	//place cursor back to first position of first field
	_settextposition(field[*i].y, field[*i].x);
}

void writefieldmsg(int i)
{
	char tip[DOSPATHLEN], str[DOSPATHLEN];
	strcpy(tip, "TIP: ");
	strcpy(str, "");
	switch (i)
	{
		case fPAIDTO: 
			strcpy(str, "F2 to enter today's date/increment month.");
		break;
		case fBOX:
			if (keystate == ksFIND)
			strcpy(str, "Enter 4 digit box number and press RETURN to go directly to box.");
		break;
		default: return;
	}
	strcat(tip, str);
	writemsg(tip);
}

void writeerrmsg(int i)
{
	char errstr[DOSPATHLEN], str[DOSPATHLEN];
	strcpy(errstr, "ERROR: ");
	strcpy(str, "");
	switch (i)
	{
		case fPIN: 
			if (keystate == ksEDIT &&
				!checkpin(user.pin)) 
			{
				strcpy(str, user.pin);
				strcat(str, " is an invalid security code!");
			}
			else return;
		break;
		default: return;
	}
	strcat(errstr, str);
	writemsg(errstr);
}

//used to check paid to dates with a DD-Mmm-YY format
int validdate(char *date)
{
	char monthlist[] = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";
	char *mstrpos;
	int mpos;
	char month[4];
	int day, year, lastday;
	
	if (strlen(date) == 0) return TRUE;
		
	sscanf(date, "%02d-%3s-%02d", &day, &month, &year);
	month[3] = '\0';
	
	if ((mstrpos = strstr(monthlist, month)) == NULL) return FALSE; 
	
	mpos = (int)((mstrpos - monthlist)/4);
	if (!(year % 4)) lastday = llymonth[mpos];
	else lastday = lmonth[mpos];
	
	if (day < 1 || day > lastday) return FALSE;
	
	return TRUE;
}

int exitedit(int *action)
{
	if (!checkpin(user.pin)) 
	{
		sprintf(s, "ERROR: %s is not a valid security code!", user.pin);
		writemsg(s);
	}
	else if (!validdate(user.paidto)) 
	{
		sprintf(s, "ERROR: %s is not a valid date!", user.paidto);
		writemsg(s);
	}
	else if CHECKADMIN(user)
	{
		sprintf(s, "ERROR: %s paidto blank but no valid nonPAYCODE admin box.", user.box);
		writemsg(s);
	}
	else 
	{
		getuserrec(FUSER, curruser, &olduser);
		if (*action == aVIEW)
		{
			if (!userequal(&user, &olduser)) 
			{
				writemsg("Save current edit? Y/N (Enter = Y)");
				return FALSE;
			}	
		}
		if (*action != aBLOCKACTIVATE) *action = aNOACTION;
		return TRUE;
	}
	return FALSE;
}

void updateUSER( void )
{
	register channel;
	for (channel=0; channel<chan; channel++)  
		if (strcmp(USER.box, user.box) == 0)
		{
			strcpy(USER.pin,user.pin);
			//strcpy(USER.admin,user.admin);
		}
}

void changereminder(char rs)
{
	switch (keystate)
	{
		case ksEDIT: 
			switch(rs)
			{   //SEQUENCE: 0clear Ssystem Aad Xnone Yadmin (Cplay admin to all)
				case 'Y': //remind
				if (user.boxtype == btADMN)
				{
					strcpy(selectfield(fREMIND, &user), "C");
					writemsg("Play admin reminder to all boxes controlled by admin box.");
				}
				else
				{
					strcpy(selectfield(fREMIND, &user), "");
					writemsg("Cleared reminder.");
				}
				break;
				case 'C':
				strcpy(selectfield(fREMIND, &user), "");
				writemsg("Cleared reminder.");
				break;
				case 0: //not reminded
				strcpy(selectfield(fREMIND, &user), "S");
				writemsg("Added default system reminder.");
				break;
				case 'S': //default system reminder
				strcpy(selectfield(fREMIND, &user), "A");
				writemsg("Added \"advertising\" reminder.");
				break;
				case 'A': //ad
				strcpy(selectfield(fREMIND, &user), "X");
				writemsg("Never remind box.");
				break;
				case 'X': //never remind
				strcpy(selectfield(fREMIND, &user), "Y");
				writemsg("Added admin reminder. (Defaults to system reminder if no admin box.)");
				break;
				default: 
				strcpy(selectfield(fREMIND, &user), "");
			}
			if (keystate == ksVIEW)
				saveuser(FUSER, curruser, &user);
			showuser(&user);
	 }
}

int incboxtype( US *puser )
{
	int rc = NOTFOUND;
	// no default box assigned
	if (defbox == NOTFOUND)
	{
		// note: btDEF can never be the first box type (0)
		if (puser->boxtype == (btDEF - 1))
		{
			// assign default box
			puser->boxtype = btDEF;
			defbox = atoi(puser->box);
			rc = TRUE;
		}
		else BTNEXT(puser,btMAX)
	}
	else 
	{   
		if (puser->boxtype == btDEF)
		{
			BTNEXT(puser,btMAX)
			// unassign default box
			defbox = NOTFOUND;
			rc = FALSE;
		}
		else 
		{
			BTNEXT(puser,btMAX)
			// skip default box if already defined
			if (puser->boxtype == btDEF) BTNEXT(puser,btMAX)
		}
	}
	useridx[atoi(puser->box)].type = puser->boxtype;
	return rc;
}

void changeboxtype( US *puser )
{
	int rc;
	switch (keystate)
	{
		case ksFIND: BTNEXT(puser,btANY)
			showuser(puser);
		break;
		case ksEDIT:
			rc = incboxtype(puser);
			if (rc == TRUE)
			{
					sprintf(s, "Box %04d is now the default box.", defbox);
					writemsg(s);
			}
			else if (rc == FALSE) writemsg("No default box assigned.");
			showuser(puser);
	}
	
}

void incmonth( void )
{
	time_t now;
	char *ts, *cs;
	US *c;
	int i, j;
	
	time(&now);
	switch(keystate)
	{
		case ksEDIT:
			c = &user;
			cs = c->paidto;
		break;
		case ksFIND:
			c = &finduser;
			cs = inccs(c->paidto);
		break;
	}
	
	if (strlen(cs) < 6)
		strncpy(cs, timestr(&now), field[fPAIDTO].l);
	else 
	{
		ts = cs;
		// try and find out what month is in the string
		for (i = 0; i < 12; i++)
		{
			// if you found a real month put the next month in that place
			if (ismonth(i, ts + 3))
			{
				if (i < 11) i++;
				else i = 0;
				for (j = 0; (unsigned) j < strlen(months[i]); j++)
					ts[j + 3] = months[i][j];
				break;
			}
		}
		if (i == 12)
			strncpy(cs, timestr(&now), field[fPAIDTO].l);
	}
	showuser(c);
}

void writehstr(char hwin, int line, int width, char attr)
{
	int kl, il, ix, kx, t;
	
	il = strlen(help[hwin][line].info);
	kl = strlen(help[hwin][line].kstr);
	ix = middle(80,width);
	kx = ix + width - kl;;
	
	switch (attr)
	{
		case HILITE:
			attr = HWINHILITE;
		break;
		default:
			attr = HWINATTR;
	}
	
	t = EDITHELP_TOP + line + 1;
	writestr(ix, t, 0, width - kl, attr, help[hwin][line].info);
	writestr(kx, t, 0, kl, attr, help[hwin][line].kstr);
}

void buildwin(char hwin, char *top, char *bottom, int *w)
{
	register i;
	int t, l, h, infol, kstrl, imax, kmax;
	
	*top = *bottom = *w = h = l = imax = kmax = NOTFOUND;
	
	for (i = 0; i < HELPHT; i++)
	{
		if (imax < (infol = strlen(help[hwin][i].info))) 
			imax = infol;
		if (kmax < (kstrl = strlen(help[hwin][i].kstr)))
			kmax = kstrl;
		if (h == NOTFOUND && help[hwin][i].zero == NOTFOUND) 
			h = i;
		if (i > 0 && 
			(help[hwin][i].zero == NOTFOUND) &&
			*bottom == NOTFOUND)
		{
			*bottom = i - 1;
		}
		else if (*top == NOTFOUND && help[hwin][i].key > 0) *top = i;
	}

    *w = imax + kmax + 2;
	t = EDITHELP_TOP + 1;
	l = middle(80,(*w));
	create_win(l, t, *w, h, HWINATTR, bsSINGLE, NULL);
	for (i = 0; i < h; i++) writehstr(hwin, i, *w, NOHILITE);
	writemsg(" HELP:  F1/PGDN/PGUP/Ctrl-HOME/Ctrl-END: Select screen. ESC: Exit. ");
}

void remakescrn(char *hwin, char *currwin)
{
	loadudisplay(scrnbuff);

	if (keystate == ksEDIT) showuser(&user);
	if (hwin != NULL) 
	{
		*hwin = 0;
		*currwin = NOTFOUND;
		_settextcursor(cursortype);
	}
	else if (showingcperh) 
	{
		_settextcursor(cursortype);
		showingcperh = FALSE;
	}
}

void helpwindow(char hwin, 
				char key, char zero, 
				char *currline, 
				char topline, char bottomline, 
				int width)
{
	char cl = *currline;
	
	if (topline == NOTFOUND || bottomline == NOTFOUND || width == NOTFOUND) 
		return;
		
	writehstr(hwin, cl, width, NOHILITE);
	
	if (zero)
	switch (key)
	{
		case UP:
			if (cl > topline) cl--; 
		break;
		case DOWN:
			if (cl < bottomline) cl++; 
		break;
		case HOME:
			cl = topline; break;
		case END:
			cl = bottomline; break;
	}
	
	writehstr(hwin, cl, width, HILITE);
	
	*currline = cl;
}

char showhwins(char minwin, char *key, char *zero)
{
	static char helpvisible = FALSE;
	static char hwin;
    static char currline = NOTFOUND;
	static char currwin = NOTFOUND;
	static char topline, bottomline;
	static int width;
	
	if (helpvisible)
	{
		if (*zero)
		switch (*key)
		{
			case HOME:
			case END:
			case UP:
			case DOWN: break;
			case CtrlHOME:
				hwin = minwin;
			break;
			case CtrlEND:
				while (hwins[hwin + 1] < hwEND) hwin++;
			break;
			case F1:
			case PGDN:
				if (hwins[hwin + 1] < hwEND) 
					hwin++;
			break;
			case PGUP:
				if (hwin > minwin) hwin--; //if hwin > 0 show previous
			break;
			default: helpvisible = FALSE;
		}
		else
		switch (*key)
		{
			case ESC:
				*key = 0;
				*zero = FALSE;
				helpvisible = FALSE;
			break;
			case ENTER:
				if (help[hwins[hwin]][currline].key == MOREHELP)
				{
					if (hwins[hwin + 1] < hwEND) 
						hwin++;
				}
				else 
				{
					*key = help[hwins[hwin]][currline].key;
					*zero = help[hwins[hwin]][currline].zero;
					helpvisible = FALSE;
				}
			break;
			default: helpvisible = FALSE;
		}
		if (!helpvisible) remakescrn(&hwin, &currwin);
	}
	else if (*zero)
	switch (*key)
	{
		case F1:
			hwin = minwin;
			helpvisible = TRUE;
			_settextcursor(NOCURSOR);
			saveudisplay(scrnbuff);
		break;
	}
		
	if (helpvisible)
	{
		if (hwin != currwin)
		{
			loadudisplay(scrnbuff);
			currwin = hwin;
			buildwin(hwins[hwin], &topline, &bottomline, &width);
			currline = topline;
		}
		
		helpwindow(hwins[hwin], *key, *zero, &currline, topline, bottomline, width);
	}
	
	return helpvisible;
}

void bdedit(unsigned char *key, 
		   		char *zero)
{
	static char bdstr[3][8] = {0};
	static int init = TRUE;
	int bdpos[3] = {11, 23, 26};
	static int sp = 0, p = 0, bdp = 0, l = 7;
	static struct _rccoord xy;
	int message = FALSE, bdv;
	
	if (init)
	{
		init = FALSE;
		sprintf(bdstr[0],"%7ld", user.bdowed);
		sprintf(bdstr[1],"%02d", BDVAL(user));
		sprintf(bdstr[2],"%7ld", user.boxdays);
	}

	switch(*key)
	{
		case ENTER:
			bdv = atoi(bdstr[1]);
			if ((user.bdval  && user.bdval != bdv) || 
				(!user.bdval && bdv != bdval) ||
				(atol(bdstr[0]) != user.bdowed) ||
				(atol(bdstr[2]) != user.boxdays))
			{
				message = TRUE;
				user.bdval = bdv;
				user.bdowed = atol(bdstr[0]);
				user.boxdays = atol(bdstr[2]);
				saveuser(FUSER,curruser,&user);
				writemsg("Box day values saved.");
			}
		case ESC:
			kbfunc = dokey;
			strcpy(bdstr[0], "");
			p = 0;
			l = 7;
			sp = 0;
			bdp = 0;
			init = TRUE;
			if (!message) writemsg(EDTTL);
			_settextposition(field[currfield].y, field[currfield].x);
		break;
		case TAB:
			if (bdp < 2) bdp++;
			else bdp = 0;
			if (bdp == 1) l = 2;
			else l = 7;
			p = 0;
			_settextposition(HELP_TOP, bdpos[bdp]);
		break;
		case ShiftTAB:
			if (bdp > 0) bdp--;
			else bdp = 2;
			if (bdp == 2) l = 2;
			else l = 7;
			p = 0;
			_settextposition(HELP_TOP, bdpos[bdp]);
		break;
		default:
			//edit text window (single line)
			edit(bdpos[bdp],		//xcoord of window
				 HELP_TOP, 			//ycoord of window
				 l,					//window width 
				 &sp,				//pos of first char in string in window
				 &p,				//pos of cursor in string
				 l,					//actual length of string
				 fgWHITE + HELPATTR,//color
				 *key,
				 *zero,            
				 NUMERIC,           //edit char type
				 bdstr[bdp]);			//string being edited
	}
}

void daemonkey(unsigned char *key, char *zero)
{   
	if (*zero == TRUE)
	switch(*key)
	{                       
		case AltX: 
			writedebug("Shutting down system...");
			shutdown(key, zero); 
			return;
	}                                          
	printf ("Alt-X exits...\n");
}

void dokey(unsigned char *key, 
		   char *zero)
{            
	time_t now;
	register l;
	enum keycombos {kcNONE, kcBOXDAYS, kcPAGE};
	static int keycombo = kcNONE;
	static enum fields i = 0;
	static int pos = 0;      
	static int startpos = 0;
	static enum actions passwordaction = aNOACTION;
	int prevkeystate;
	US *puser = &user; // for cut/paste functions
	char msgstr[DOSPATHLEN]; /* for messages */
	int c, channel, box2find;
	/* do not process keystrokes if system is shutting down 
	   with the exception of the immediate shutdown key */    
	if (endprogram && !(*key == CtrlF && *zero == FALSE)) return; 
	/* section deals with initial screen processing */
	if (i == 0) i = poslimit[keystate].min;
	
	//key combos
	switch(keycombo)
	{
		case kcBOXDAYS:
			keycombo = kcNONE;
			if (!*zero)
			switch (tolower(*key))
			{
				case 'u': //update
					countactiveboxes(TRUE);
					writemsg("Boxday counts updated for all admin boxes.");
				return;
				case 'e': //edit value
					currfield = i;
					kbfunc = bdedit;
					_settextposition(HELP_TOP, 11);
					writemsg(s);
				return;
				default: 
					*key = 0;
				break;
			}
		break;
		case kcPAGE:
			if (!*zero)
			switch(tolower(*key))
			{
				case 'c': 
					writemsg("Are you sure you want clear all pending pages? Y/N (Enter = Y)");
					action = aCLEARPAGEQ;
				break;
				case 's':
					if (pgstart == NOTFOUND)
					{
						writemsg("Nothing to page! Cannot restart paging queue.");
						break;
					}
					if (pagestate == NOPAGE)
					{
						writemsg("Page queue started.");
						pagestate = PAGEALL;
						currpg = pgstart;
						getpagechannel();
					}
					else
					{
						writemsg("Page queue stopped.");
						pagestate = NOPAGE;
					}
				break;
				case 'a':
					if (addpage(&user))
					{
						pagestate = NOPAGE; //don't start paging yet
						saveuser(FUSER, curruser, &user);
						showuser(&user);
						writemsg("Added box to paging queue.");
					}
					else writemsg("Could not add box to paging queue. Check phone & box type.");
				break;
				case 'r':
					if (page[curruser].tries == 0)
					{
						writemsg("Box is not in paging queue.");
					}
					else
					{
						rmfromq(atoi(user.box));
						showuser(&user);
						writemsg("Removed box from queue.");
					}
			}
			keycombo = kcNONE;
		return;
		default: keycombo = kcNONE;
	}
	
	//help and stat screens
	switch(keystate)
	{
		case ksEDIT:
			if (showhwins(EDITHSTART, key, zero)) return; //see if user wants to display help screen
		break;
		case ksFIND:
			if (showhwins(FINDHSTART, key, zero)) return; //see if user wants to display help screen
		break;
		case ksVIEW:
			if (showtelco) 
			{
				telcoscrn(*key, *zero); //select telco lines
				if (!showtelco)
					*key = 0; //remove key information
				else return;
			}
			if (!showtelco && showingcperh) //show stat scrn
			{
				showingcperh = selectstatscrn(*key);
				if (!showingcperh)
					remakescrn(NULL, NULL);
				else return;
			}
			else if (showhwins(VIEWHSTART, key, zero)) return; //see if user wants to display help screen
		break;
	}

	writemsg( CURRSTR );
	
	if (keystate == ksVIEW) showuser(&user);
	if (keystate == ksFIND) puser = &finduser;
	else puser = &user;    

	/* check for unfinished Y/N (Enter = Y) keyboard prompts */
	if (action != aNOACTION) 
	{
		action = doaction(&action, key, zero, &i, &pos, &startpos);
		return;
	}    
	
	/* normal key functions */    
	if (*zero == TRUE)
	switch(*key)
	{                       
		case 0: break;
		case CtrlHOME: /* find first inactive box */
		case CtrlEND:  /* find last inactive box */
			/* or if you are in ksEDIT or ksFIND go to first or last field */
			if (keystate == ksEDIT || 
				keystate == ksFIND || 
				keystate == ksBACKUP)
			{
				pos = 0;
				if (*key == CtrlHOME)
					i = poslimit[keystate].min;
				else i = poslimit[keystate].max;    
				_settextposition(field[i].y, field[i].x);
				break;
			}
		case CtrlPGUP: /* find previous inactive box */
		case CtrlPGDN: /* find next inactive box */
			if (!_gotouser(key, INACTIVE))
				editkeystate(keystate, i, &startpos, &pos, *key, *zero); 
		break;
		case PGUP: /* find previous active box */
		case PGDN: /* find next active box */
			if (keystate == ksMSGINFO || keystate == ksEDIT)
			{ 
				viewmsginfo(*key,*zero,&i,&pos,&startpos);
				break;
			}
		case HOME: /* find first active box */
		case END:  /* find last active box */
			if (keystate == ksMSGINFO)
			{ 
				viewmsginfo(*key,*zero,&i,&pos,&startpos);
				break;
			}
			if (!_gotouser(key, ACTIVE))
				editkeystate(keystate, i, &startpos, &pos, *key, *zero); 
		break;                                  
		//disable regular help keys
		case F1: break;
		//put a box "on hold" that is reserved for future use 
		case AltH:
			if (keystate == ksEDIT)
			{
				switch (puser->active)
				{
				case INACTIVE:
					puser->active = ONHOLD;
					useridx[atoi(puser->box)].avail = ONHOLD;
					saveuser(fuser, curruser, puser);
					writemsg("Box now on hold (no undo).");
					break;
				case ONHOLD:	
					activateuser();
					writemsg("Box now active.");
					break;
				default: writemsg("Deactivate box before putting on hold!"); 
				}
				showuser(puser);
			}
			break;
		
		/* put todays date in paidto field */
		case F2:
			switch (keystate)
			{
				case ksFIND:
				case ksEDIT:
					if (i == fPAIDTO) incmonth();
					else if (field[i].type != NUMERIC)
					{
						time(&now);
						pastefld(puser, datestr(&now), pos, i);
					}
				break;
				case ksVIEW:
					showingcperh = selectstatscrn(*key);
				break;
			}
		break;
		/* repeat find */
		case F3:
			if (keystate != ksEDIT && keystate != ksVIEW && keystate != ksFIND) break;
			prevkeystate = keystate;
			if (finduserrec())
			{
				if (prevkeystate == ksFIND)
					keystatesetup(ksVIEW, &i, &pos, &startpos);
				else keystate = prevkeystate;
				getuserrec(FUSER, curruser, &user);
				showuser(&user);
			}
			else keystate = prevkeystate;
		break;  
        // show tags for find routine
		case F4:
			if (keystate == ksFIND) 
				tagsvisible( NOTFOUND );
		// show boxday count
			else if (keystate == ksEDIT)
			{
				if (user.boxtype == btADMN)
				{
					boxdaymsg(user);
					keycombo = kcBOXDAYS;
				}
				else writemsg("No boxday count available. This box is not an administrative box!");
			}
		break;
		/* activate/deactivate box */
		case AltA: 
			if (keystate == ksEDIT)
			{
				if (puser->active == ONHOLD)
				{
					activateuser();
					showuser(puser);
					writemsg("Box now active (no undo).");
				}
				else keystatesetup(ksBLOCKACTIVATE,&i,&pos,&startpos);
			}
			else if (keystate == ksFIND)
			{
				switch (finduser.active)
				{
					case ACTIVE: finduser.active = INACTIVE; 
						break;
					case INACTIVE: finduser.active = ONHOLD; 
						break;
					case ONHOLD: finduser.active = NOTFOUND;
						break;
					case NOTFOUND: finduser.active = ACTIVE; 
						break;
					default: finduser.active = NOTFOUND;
				}
				showuser(&finduser);
			}
			else if (keystate == ksBACKUP) {
					action = aCSVABUP;
					writemsg("Start csv backup to the A: drive now? Y/N (Enter = Y)");
			}
		break;
		//do system backup
		case AltB:
			if (keystate == ksEDIT) // || keystate == ksVIEW)
			{
				keystatesetup(ksBACKUP,&i,&pos,&startpos);
				showuser(&user);
			}
		break;
		// back up only users.csv
		case AltC:
			if (keystate == ksBACKUP)
			{
				if (checkbackup())
				{
					action = aCSVBUP;
					writemsg("Initiate csv back up now? Y/N (Enter = Y)");
				}
			}		
		break;	
        //toggle system announcement.
		case AltM:
			if (keystate == ksEDIT)
			{
				switch (announce)
				{
					case NEW:
					case TRUE:
						announce = FALSE;
						writemsg("System announcement turned off.");
					break;
					case FALSE:
						if (fexist(ANNOUNCETEMP))
							announce = NEW;
						else if (fexist(ANNOUNCEMENT))
							announce = TRUE;
						else 
						{
							writemsg("You do not have a system announcement recorded.");
							break;
						}
						writemsg("System announcement turned on.");
				}
			}
			else if (keystate == ksMSGINFO)
					viewmsginfo(*key, *zero, &i, &pos, &startpos);
			
		break;  
		case AltN:
			if (keystate == ksVIEW)
			writemsg("To create box: Find box (Alt-F). Open box for edit (Tab): Alt-A activates.");
		break;
		//go into mainenance mode
		case AltO:
			if (keystate != ksEDIT) break;
			if (maintenance)
				writemsg("Return to normal mode now? Y/N (Enter = Y)");
			else
				writemsg("Go into maintenance mode now? Y/N (Enter = Y)");
			action = aMAINTENANCE;
		break;		
		//reset idle channels by setting them offhook and then onhook again
		case AltR:
			if (keystate == ksEDIT) {
				writemsg("Reset all inactive channels now? Y/N (Enter = Y, Single Channel = N)");
				action = aRESET;
			} else if (keystate = ksVIEW) {
					// refresh the screen
					makestatstr();
					showttls();
					getuserrec(FUSER, curruser, &user);
					showuser(&user);
            }
			break;
		break;  
		/* clear finduser */
		case AltY:
			if (keystate == ksFIND) 
			{
				clearfind(&i,&pos,&startpos);
				showuser(&finduser);
			}
			else if (keystate == ksEDIT || keystate == ksBACKUP)
			{
				clearuser(&i,&pos,&startpos);
				showuser(&user);
			}
		break;        
		/* clear message/greeting files */
		case AltD:
			if (keystate == ksEDIT)
			{
				if (user.active == INACTIVE)
				{
					writemsg("Do you want to erase user message and greeting files? Y/N (Enter = Y)");
					action = aDELUSER;
				}     
				else writemsg("Deactivate box before deleting messages. Press a key...");
			}    
		break;    
		/* find box */
		case AltF: 
			if (keystate == ksEDIT)
			{
				switch(resetinuse(atoi(user.box)))
				{
					case INUSE: 
						writemsg("Box is in use right now. Wait for channel to go on hook.");
					break;
					case ACTIVE: 
						showuser(puser);
						writemsg("Box now set to ACTIVE.");
					break;
					case INACTIVE:
						writemsg("Box is currently INACTIVE.");
					break;
				}
				break;
			}
						
			if (keystate == ksBACKUP)
			{
				if (checkbackup())
				{
					action = aBACKUP;
					writemsg("Initiate full back up now? Y/N (Enter = Y)");
				}
			}
			else if (changekeystate(ksFIND, &i, &pos, &startpos))
					showuser(&finduser);
		break;
		/* undo edit */        
		case AltU: 
			switch (keystate)
			{
			case ksEDIT:
				getuserrec(FUSER, curruser, &user);                
				showuser(&user);
			break;
			case ksBACKUP:
				loadbackupparams();
				showuser(&user);
			break;
			}
		break;    
		//force edit to save record
		case AltS:
			if (keystate == ksEDIT)
			{
				writemsg("Save the current edit (no undo)? Y/N (Enter = Y)");
				action = aEDSAVENOEXIT;
			}
		break;
		/* make template or change system time (ksVIEW) */
		case AltT:
			if(keystate == ksVIEW)
				keystatesetup(ksTIME, &i, &pos, &startpos);
			else if(keystate == ksEDIT || keystate == ksFIND)
			{
				action = aMAKETEMPLATE;
				if (exitedit(&action))
				{
					for (c = poslimit[ksEDIT].min; c <= poslimit[ksEDIT].max; c++)
					{
						if (c == fBOXTYPE) temp.boxtype = puser->boxtype;
						else strcpy(selectfield(c, &temp), selectfield(c, puser));
					}
					templateavail = TRUE;
					writemsg("Template created. Press a key...");
				}
			}
		break;  
		/* copy template */        
		case AltZ:
			if (keystate == ksEDIT) 
			{
				if (templateavail) keystatesetup(ksCOPYTEMPLATE,&i,&pos,&startpos);
				else writemsg("No template available!");
			}
			else if (keystate == ksFIND) 
			{
				if (templateavail) copytemplate(puser);
				else writemsg("No template available!");
			}
		break;
		/* change password */
		case AltP:
			if (keystate == ksEDIT)
			{
				keystatesetup(ksEDITPASSWORD,&i,&pos,&startpos);
			}
			else if (keystate == ksBACKUP)
			{
				if (checkbackup())
				{
					action = aPARTBUP;
					writemsg("Initiate partial back up now? Y/N (Enter = Y)");
				}
			}

		break;
		//page controls
		case AltQ:
			if (keystate == ksEDIT) 
			{
				writemsg("PAGING FUNCTIONS: C=clear queue. S=toggle paging. A=add page. R=remove page."); 
				keycombo = kcPAGE;
			}
		break;
		/* exit program */
		case AltX: 
			*key = '\0';        
			switch(keystate)
			{
				case ksVIEW:
					if (strlen(password) > 0)    
					{
						changekeystate(ksPASSWORD, &i, &pos, &startpos);
						passwordaction = aEXITPROGRAM;
					}    
					else
					{
						writemsg("Exit program now? Y/N (Enter = Y)");
						action = aEXITPROGRAM;
					}    
				break;
			}
		break;   

		case DOWN:
			if (keystate == ksMSGINFO)
			{ 
				viewmsginfo(*key,*zero,&i,&pos,&startpos);
				break;
			}
			if (poslimit[keystate].min == poslimit[keystate].max)
				break;
			pos = 0;
			godown(&i);       
		break;
		
		case ShiftTAB:
		case UP:
			if (keystate == ksMSGINFO)
			{ 
				viewmsginfo(*key,*zero,&i,&pos,&startpos);
				break;
			}
			if (poslimit[keystate].min == poslimit[keystate].max)
				break;
			pos = 0;
			if (i <= poslimit[keystate].min) i = poslimit[keystate].max;
			else do i--;
				 while (i > poslimit[keystate].min && field[i].type == NOTHING);
			_settextposition(field[i].y, field[i].x);
		break;
		//delete a message from keyboard
		case DEL:
			if (keystate == ksMSGINFO)
			{ 
				viewmsginfo(*key,*zero,&i,&pos,&startpos);
				break;
			}
		default:                                                            
			editkeystate(keystate, i, &startpos, &pos, *key, *zero); 
	}
	else
	switch(*key)
	{
		case 0: break;  
		// copy field
		case CtrlC: break;
		case CtrlK:
			if (keystate == ksEDIT  || keystate == ksFIND)
			{
				strcpy(fieldbuff, selectfield(i, puser));
				fieldtype = field[i].type;
				showuser(puser);
				writemsg("Field copied...");
			}
		break;
		// cut field
		case CtrlX:
			if (keystate == ksEDIT  || keystate == ksFIND)
			{
				strcpy(fieldbuff, selectfield(i, puser));
				fieldtype = field[i].type;
				if (i <= poslimit[ksEDIT].max && i >= poslimit[ksEDIT].min)
					strcpy(selectfield(i, puser), "");
				showuser(puser);
				writemsg("Field cut...");
			}
		break;
		// paste field
		case CtrlV:
			if ((keystate == ksEDIT  || keystate == ksFIND) && 
				(field[i].type == fieldtype || field[i].type == ALPHANUM))
				pastefld(puser, fieldbuff, pos, i);
		break;    
		/* new defpin */
		case CtrlD:
			if(keystate == ksEDIT && strlen(selectfield(fPIN,&user)) == (unsigned) field[fPIN].l)
			{
				action = aDEFPIN;
				sprintf(s, "Change default security code from %s to %s? Y/N (Enter = Y)", defpin, selectfield(fPIN, &user));
				writemsg(s);
			}
		break;
		case CtrlF:
			if (endprogram) {
				if (debug) writedebug("Forcing shutdown ... ");
				force_shutdown = TRUE;
			}
		break;
		//change language
		case CtrlL:
			switch (keystate)
			{
				case ksFIND:
					if (findlanguage < maxlanguage) findlanguage++;
					else findlanguage = 0;
					strcpy(finduser.language, promptf[findlanguage].name);
					showuser(&finduser);
				break;
				case ksEDIT:
					for (l = 0; l < maxlanguage; l++) 
						if (!stricmp(user.language, promptf[l].name)) break;
					if (l < maxlanguage - 1) l++;
					else if (l >= maxlanguage) l = deflanguage;
					else l = 0;
					strcpy(user.language, promptf[l].name);
					showuser(&user);
			}
		break;
		//rename a file
		case CtrlN:
			if(keystate == ksEDIT) keystatesetup(ksRENOLDN,&i,&pos,&startpos);
		break;
		/* toggle reminder */
		case CtrlR: 
			if (keystate == ksVIEW || keystate == ksEDIT)
			changereminder(user.remind[0]); 
		break;
		//randomize security code
		case CtrlA: 
			if (keystate == ksEDIT) 
				keystatesetup(ksRANDOMPIN,&i,&pos,&startpos);
		break;
		//toggle box type
		case CtrlT:
			if (keystate == ksEDIT || keystate == ksFIND)
				changeboxtype(puser);
		break;
		// disable these keys (incompatibility with CSV format)
		case ',':
			if (keystate == ksEDIT)
				writemsg("Invalid key! (Incompatible with comma delimited format)");
			else editkeystate(keystate, i, &startpos, &pos, *key, *zero); 
		break;
		// edit box
		case TAB: 
			switch(keystate)
			{
				case ksVIEW:
					if (curruser == CAL) break;
					if (strlen(password) > 0)
					{
					   changekeystate(ksPASSWORD, &i, &pos, &startpos);
					   passwordaction = aEDITUSER;
					}   
					else
					{
						if (changekeystate(ksEDIT, &i, &pos, &startpos)) 
							showuser(&user);
					}
				break;
				case ksBACKUP:
				case ksEDIT:
				case ksFIND:
					pos = 0;
					godown(&i);
				break;
			}
		break;
		
		case ESC:
			switch (keystate) {
				case ksEDIT:
					action = aVIEW;
					if (exitedit(&action))
						changekeystate(ksVIEW, &i, &pos, &startpos);
					else return;	
				break;
				case ksMOVEMSG:
					viewmsginfo(*key, *zero, &i, &pos, &startpos);
				break;
				case ksTIME:
				    keystatesetup(ksVIEW, &i, &pos, &startpos);
				break;
				case ksMSGINFO:
				case ksCOPYTEMPLATE:
				case ksRANDOMPIN:
				case ksRESET:
				case ksBLOCKACTIVATE:
				case ksEDITPASSWORD:
				case ksRENOLDN:
				case ksRENNEWN:
					keystatesetup(ksEDIT, &i, &pos, &startpos);
				break;
				case ksBACKUP:
					if (checkbackup())
					{
						if (backupchanged()) 
						{
							action = aKEEPBACKUP;
							writemsg("Keep new back up settings? Y/N (Enter = Y)");
						}
						else
						{
							keystatesetup(ksEDIT, &i, &pos, &startpos);
							showuser(&user);
						}
					}
				break;
				default:
					changekeystate(ksVIEW, &i, &pos, &startpos);
			}
			showuser(&user);
		break;             
		
		case ENTER:
			switch(keystate)
			{
				case ksMSGINFO:
					viewmsginfo(*key,*zero,&i,&pos,&startpos);
					break;
				case ksEDITPASSWORD:
					if(strlen(checkpass) > 0)
						strcpy(password, checkpass);
					else password[0] = '\0';    
					checkpass[0] = '\0';
					keystatesetup(ksEDIT,&i,&pos,&startpos);
					writemsg("Password changed.");
					break;
				case ksTIME:
					keystatesetup(ksVIEW, &i, &pos, &startpos);
					sprintf(s, "Change time to %s? Y/N (Enter = Y)", newtimestr);
					writemsg(s);
					action = aTIME;
				break;
				case ksPASSWORD:
					doaction(&passwordaction, key, zero, &i,&pos,&startpos);
				break;
				case ksRENOLDN: //if you are renaming
					keystatesetup(ksRENNEWN,&i,&pos,&startpos); //go to next name if you've got file
				break;
				case ksRENNEWN:
				{
					keystatesetup(ksEDIT,&i,&pos,&startpos);
					action = aRENAME;
					sprintf(s, "Rename %s to %s? Y/N (Enter = Y)", ren.file, ren.name);
					writemsg(s);
				}
				break;
				case ksRESET:
				{
					keystatesetup(ksEDIT,&i,&pos,&startpos);
					channel = atoi(channelstr);  
					if (channel <= 0 || channel > chan)
					{
						writemsg("Invalid channel number selected!");
						strcpy(channelstr, "1");
						break;
					}
					action = aRESETCONFIRM;
					sprintf(s, "Reset channel %s ? Y/N (Enter = Y)", channelstr);
					writemsg(s);
				}
				break;
				case ksMOVEMSG:
				{
					if (strlen(targetbox) == 0) 
					{
						viewmsginfo(*key, *zero, &i, &pos, &startpos);
						break;
					}
					action = aMOVEMSG;
					_settextcursor(NOCURSOR);
					sprintf(s, "Move selected message to %s? Y/N (Enter = Y)", targetbox);
					writemsg(s);
				}
				break;
				case ksBLOCKACTIVATE:
				{
					//note: this routine has been changed because 
					//size of HDs over 2.2 gigabytes is misreported
					//by the dos service to this effect
					keystatesetup(ksEDIT, &i, &pos, &startpos);
					numact = atoi(numactstr);
					//check size of numact
					if (numact + curruser - 1 >= MAXBOX)
						numact = MAXBOX - curruser - 1;
					
					if (numact > 0)
					/*  && 
					    (user.active == ACTIVE || 
					    (user.active == INACTIVE && numact <= newboxes))
					    ) */
					{
						if (numact > 1)
						{
							if (user.active == ACTIVE)
								sprintf(msgstr, "DEACTIVATE boxes %04d to %04d (no undo) ? Y/N (Enter = Y)", curruser, curruser + numact - 1);
							else sprintf(msgstr, "ACTIVATE boxes %04d to %04d (no undo) ? Y/N (Enter = Y)", curruser, curruser + numact - 1);
							writemsg(msgstr);
							action = aBLOCKACTIVATE;
						}
						else blockactivate();
					}
//					else if (numact > newboxes || numact + curruser - 1 >= MAXBOX) 
//							writemsg("ERROR: Not enough available boxes to activate this number! Press a key...");
				}
				break;
				case ksCOPYTEMPLATE:
				{
					keystatesetup(ksEDIT, &i, &pos, &startpos);
					numact = atoi(numactstr);
					if (numact > 0 && numact + curruser - 1 < MAXBOX)
					{
						if (numact > 1)
						{
						sprintf(msgstr, "COPY TEMPLATE to boxes %04d to %04d (no undo) ? Y/N (Enter = Y)", curruser, curruser + numact - 1);
						writemsg(msgstr);
						action = aCOPYTEMPLATE;
						}
						else copytemplate(&user);
					}
					else if (numact + curruser - 1 >= MAXBOX)
							writemsg("ERROR: Number of boxes to copy to is too big! Press a key...");
				}
				break;
				case ksRANDOMPIN:
				{
					keystatesetup(ksEDIT, &i, &pos, &startpos);
					numact = atoi(numactstr);
					if (numact > 0 && numact + curruser - 1 < MAXBOX)
					{
						if (numact > 1)
						{
						sprintf(msgstr, "RANDOMIZE SEC CODE for boxes %04d to %04d (no undo) ? Y/N (Enter = Y)", curruser, curruser + numact - 1);
						writemsg(msgstr);
						action = aRANDOMPIN;
						}
						else 
						{
							makerandpin(&user);
						}
					}
					else if (numact + curruser - 1 >= MAXBOX)
							writemsg("ERROR: Number of boxes selected is too big! Press a key...");
					else writemsg("ERROR: Number of boxes selected is invalid! Press a key...");
				}
				break;
				case ksFIND: 
				if (i == fBOX && strlen(finduser.box) == field[fBOX].l)
				{                                           
					/* find the box number */
				    box2find = atoi(finduser.box);
				    if (box2find != CAL) {
						curruser = atoi(finduser.box);
						changekeystate(ksVIEW, &i, &pos, &startpos);
						getuserrec(FUSER, curruser, &user);
						showuser(&user);
					}
				}
				default:
				{    
					godown(&i);
					pos = 0;
				}
			}
		break;
		
		default:          
			editkeystate(keystate, i, &startpos, &pos, *key, *zero); 
	}
	if (keystate != ksVIEW && isdefmsg && !helpon)
	{
		if (pos == 0) writefieldmsg(i);
		writeerrmsg(i);
	}
}
#undef YES
#undef RET

void initboxes(void)
{
	//load up user data index
	inituser(&FUSER);
	// make sure the pin got loaded properly
	if (strlen(defpin) != field[fPIN].l) strcpy(defpin, DEFPIN);
	checksn(); //check for serial number
	
	/* get disk area already used by user files */
	numavailboxes();
	
}

void initscrn(void)
{
	int i;            

	makeemptyscrn(); //create blank array that can be used to clear user info area

	keystate = ksVIEW;
	cursortype = BLOCKCURSOR;
	typemode = OVERWRITE;
	shadow = FALSE;
	
	initboxes();	
	
	strcpy(fieldbuff, "");
	_settextcursor(cursortype);  
	
	i = poslimit[keystate].min;
	_settextposition(field[i].y, field[i].x);
	/* change to black & white color scheme if mono screen detected */
	if (getscrtype() == MONO_SCR)
	{
		scrtxt[USERINFO].attr = bgWHITE;
		scrtxt[STATS].attr = bgWHITE;
		scrtxt[TITLE].attr = bgWHITE;
		scrtxt[INFO].attr = bgBLACK;
		scrtxt[HELP].attr = bgBLACK;
		scrtxt[FIELDS].attr = bgWHITE;
		for (i = 0; i <= fMAX; i++)
			field[i].attr = fgWHITE;
		edithelpattr = bgWHITE;
	}        
	else edithelpattr = bgMAGENTA + fgWHITE;

	makescreen();
	makestatstr();
	showttls();
	getuserrec(FUSER, curruser, &user);
	showuser(&user);
}

void printuserrec(struct userstruct user)
{
	/* print out customer records using prinf statements */
	int i;           
	printf("\n");
	for (i = fBOX; i <= fPAIDTO; i++)
		printf("%s\n", selectfield(i, &user));
	printf("Msg pos: ");
/*	for (i = 0; i < maxmsg; i++)
		printf("%ld|", user.msgs[i]); */
	printf("\n");
	printf("Nextmsg = %d\n", user.nextmsg);
	printf("Active = ");
	if (user.active == ACTIVE)
		printf("active.\n");
	else printf("inactive.\n");
}        
		
void printuser(void) /*note you need to open user file to use this*/
{
	int i;
	for (i = 0; i < MAXBOX; i++)
		if (userpos[i] != NOTFOUND)
		{
			loaduser(FUSER, i, &user);
			printuserrec(user);                  
			getch(); /* wait for keystroke to continue */
		}    
}
/* end of ansrscrn.c */















