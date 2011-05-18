
/* ansr3.c telephone answering application */
/* from the dialogic ansr2.c program example */

/**
 **   Microsoft header files
 **/
#include <malloc.h>
#include <signal.h>
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>

/**
 **   Dialogic header files
 **/
#include <vfcns.h>
#include <d40.h>           /* standard DIALOG/4x header  */
#include <d40lib.h>
/* see below for my source files */
/* my source files - must be in the following order*/   

/* general functions and defs */
#include "strings.c"        /* basic screen handling functions */
#include "disk.c"          /* diskfull and diskavail funcs */

/* ansr3 header files */
#include "ansrdefs.h"      
#include "ansrscrn.h"
#include "util.h"
#include "ansrvar.h"          
#include "idxplay.h"
#define LIFELINE
#include "llcsv.c"

/* ansr3 functions */
#include "init.c"          /* initialization funcs */
#include "util.c"          /* utility functions used by program */
#include "ansrscrn.c"      /* screen and box editing funcs and defs */
#include "idxplay.c"	   // plays message dates etc using indexed play
#include "ansrstat.c"	   // stats functions shared with llcnvt
// needed for findboxmenu below
int checkgeninstr(int channel, char c);
int checknewinstr(int channel, char c);
// test of file handles
// #include "openmany.c" // use open_many() to test number of concurrent file handles available

/****************************************************************
*        NAME : main(argc,argv[])
* DESCRIPTION : entry point to application.
*       INPUT : argc = argument count.
*             : argv = array of pointers to arguments.
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
void main(argc,argv)
   int argc;
   char *argv[];
   {
	  int errcode;  /* exit to DOS with errcode       */
	  int channel;  /* set channel with evtp->devchan */
	  int evtcode;  /* termination event code         */
#ifndef TIMELIMIT
	  time_t now, then;
#endif

	/* process command line arguments */
	chkargs(argc,argv);

#ifdef DAEMON
	//for the daemon version of the program the log is the only 
	//output for the program not being able to open it is a fatal error
	if (openlog() == NOTFOUND)
	{
		exit(1);
	}
	//for the normal version of the program the log is an option
	//the openlog function is run in sysinit (init.c)
#else
	  openlog(); // this will open the log if debug is set to TRUE
#endif

	sprintf(s, "LIFELINE version %s\n", VER);
	initmsg(s);

	if (!startdateok())
	{
		initmsg("\nSystem date invalid! ");
#ifdef TIMELIMIT
		initmsg("\n");
		exit(0);
#else
		time(&now);
		time(&then);
		initmsg("Press a key...");
		while(!kbhit() && now - then < 20) time(&now);		
		initmsg("\n");
#endif
	}
	if (expired()) expexit();

#ifndef TG
#ifdef TRIAL
	  regnow();
#endif
#endif
	  //start random number generator for random pin numbers
	  setuprnd();   
	  // load call stats if there are any
	  loadcperh();
	  loadbackupparams();
	  if (pagingon) loadpageq();
	  getdiskused();
	  /* initialize system */
	  if (sysinit(d4xintr))
	  {
	    /* open system files, returns the number of multiprompt files found*/
		if (!openfiles()) 
		{   
			initmsg("No multiprompt files to open!\n");
			stopsys();
		}
	  }		
	  else openuserfile(&FUSER); 
	  _mkdir(LOGS);      
	  // see util.c near end of file: used to put empty files that flag when new msgs come in
	  // _mkdir(NOTIFY); 
	  /* close standard file handles */
	  //closestreams();
	  /* assign maxboxes and boxsize */
	  getsizes(); 

#ifdef DAEMON
	  // don't start up screen routines - we are going to be logging everything
	  initboxes();
	  kbfunc = daemonkey;
	  initmsg("System Started. Alt-X exits...\n");
#else	        
	  /* start up screen routines from ansrscrn.c this will also initialize boxes*/
	  initscrn(); 
	  kbfunc = dokey;
#endif      
	  /* initialize channel states to detect call */
	  for (channel=1; channel<=chan; channel++)  
	  {
		 PRT.box = NOTFOUND;
		 if (TELCO == tsDIRECT) {
		 	PRT.curr = sWTKEY;
		 } else PRT.curr = sWTRING;
		 PRT.language = deflanguage;
	  }

	  /**
	   **   Main Loop
	   **/

	  do 
	  {
		 /* wait for multi-tasking function to complete */
		 if ((key = waitevt(&evtblk)) == EVENT) {

			 channel = evtblk.devchan;
			 evtcode = evtblk.evtcode;
			 PRT.event = evtcode;
			 /* process the event result and save the next channel state */
			 PRT.oldstate = PRT.curr;  /* save original state */
			 /* use original state and previous state to get new state */
			 if (PRT.sequence && evtcode == T_MAXDT)
			 {
			    /* this code is for playing sequences of prompts */
			    /* it is generic for all prompt sequences: hence why it is here */
		 		PRT.sequence = 0;
		 		/* if there is a menu then switch to the menu */
		 		if (PRT.menu > 0)
		 			PRT.curr = PRT.menu;
		 		/* otherwise figure out next prompt to play */	
		 		else PRT.curr = firstprompt(channel);
			 }
		 	 else if (states[PRT.curr].process == NULL) 
		 	 	  PRT.curr = sINVALIDSTATE;
		 	 else PRT.curr = (*states[PRT.curr].process)(channel,evtcode);
			 /* now store original state as previous state (note: PRT.prev used in 
			    some .process functions (eg getdigit_cmplt) to choose the next state) */
			 PRT.prev = PRT.oldstate;
			 /* check sanity of new state */
			 if (PRT.curr < 0 || PRT.curr > MAXSTATE) PRT.curr = sINVALIDSTATE;
			 /* begin the new state */
			 /* allow null function pointers for initiate functions 
			    if there is a prompt to play. if not, go to sINVALIDSTATE */
			 if (states[PRT.curr].initiate == NULL) { 
			 	if (states[PRT.curr].fh != NOTFOUND)
			 		errcode = playstate(channel, PRT.curr);
			 	else PRT.curr = sINVALIDSTATE;
			   /* otherwise do the action */
			 } else errcode = (*states[PRT.curr].initiate)(channel);   
			 /* play the file if the state has a file to 
			    play if state returns NOTFOUND */
			 if (errcode == NOTFOUND && states[PRT.curr].fh != NOTFOUND)
				errcode = playstate(channel, PRT.curr);
			 /* show state description & error message */
			 writechanmsg(channel, evtcode, errcode,NULL);
		 }
		 else (*kbfunc)(&key, &zero);   /* process keystroke (from ansrscrn.c) */
	  }
	  while (!(kbaction == kbIDLE && endprogram && (portsidle() || force_shutdown)));
	  cls();
	  printf ("stopping voice system\n");
	  stopsys();	  
	  printf ("saving call stats\n");	  
	  savecperh();
	  printf ("saving backup params\n");	  
	  savebackupparams();    
	  printf ("saving def params\n");	  
	  savedef();
	  printf ("saving telco status\n");
	  savetelcochans();
	  printf ("saving page queue\n");	  
	  if (pagingon) savepageq();
#ifndef DAEMON	  
	  if (debug) 
		printf("NOTE: See %s for debug info...", LOGFILE);
#ifndef DERA	  
	  else {
	    _settextposition(24,1);
	    _settextcursor(LINECURSOR);
		strcpy(blurb, BLURB); //was failing until I started to do this
		strcat(blurb, ADDRESS);
		strwin(fgWHITE, jCENTER, blurb);
	  }
#endif
#endif
	
}  

/* state functions & state transitions */

/* oops: insane state found here */
int invalidstate (int channel)
{
	clrdtmf(channel);
	return NOTFOUND;
}	
int invalidstate_cmplt (int channel, int evtcode)
{
	return sONHK;
}

/****************************************************************
*        NAME : wtring(channel)
* DESCRIPTION : clear dtmf and wait for a call.
*       INPUT : channel = channel number
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
int wtring(channel)
   int channel;
   {
	  clrdtmf(channel);

	  return (0);
   }

/****************************************************************
*        NAME : wtring_cmplt(channel,evtcode)
* DESCRIPTION : process results of the event from wtring().
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : none.
*     RETURNS : next state.
*    CAUTIONS : none.
****************************************************************/
int wtring_cmplt(int channel, int evtcode)
{
	//don't allow channel to go off hook if exiting the program
	if (endprogram) return sWTRING;
	/* if a call is detected, set state to go off hook */
	if (evtcode == T_RING)  {
		//check if channel is DID channel and if box has been 
		//found from DID digits (this is for toll saver feature 
		//that makes phone ring again if there are no messages
		if (DID) 
		{
			if (strlen(USER.box) == 0) 
			{
		 		switch(TELCO)
		 		{
		 			case tsDID:
						PRT.getdid = TRUE;
						return sGETDIGITS;
					case tsDIDWINK: return sWINK;
				}
			}
			return sDIDOFFHK;
		}
		return sOFFHK;
	}
	/* if another event is detected reset to wtring */
	return (sWTRING);
}
   
//direct inward dial states
//check to see if telco is sending dtmf digits
//note "wink" is already defined as a function
int wink_cmplt(int channel, int evtcode)
{
	if (evtcode == T_WINK)
	{ 
		PRT.getdid = TRUE;
		return sGETDIGITS;
	}
	return sOFFHK;
}

/****************************************************************
*        NAME : wtkey(channel)
* DESCRIPTION : initiate a call after a keypress
*       INPUT : channel = channel number
*      OUTPUT : none.
*     RETURNS : error code from getdtmfs().
*    CAUTIONS : none.
****************************************************************/
int wtkey(channel)
   int channel;
{
	return(waitdigit(channel)); 
}

/****************************************************************
*        NAME : wtkey_cmplt(channel,evtcode)
* DESCRIPTION : process results of the event from wtkey().
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : none.
*     RETURNS : next state.
*    CAUTIONS : none.
****************************************************************/
int wtkey_cmplt(int channel, int evtcode)
{
	//don't allow channel to go off hook if exiting the program
	if (endprogram) return sWTKEY;
	/* if a call is detected, set state to go off hook */
	if (evtcode == T_TERMDT)  return sGENINTRO;
	return sWTKEY;
}

/****************************************************************
*        NAME : offhk(channel)
* DESCRIPTION : after detecting rings received, take the D/4x
*             : channel off hook.
*       INPUT : channel = channel number
*      OUTPUT : takes D/4x off hook.
*     RETURNS : none.
*    CAUTIONS : multi-tasking process.
****************************************************************/
int offhk(int channel)
{
	// take channel off hook
	return(set_hkstate(channel,H_OFFH));
}

/****************************************************************
*        NAME : offhk_cmplt(channel,evtcode)
* DESCRIPTION : check to make sure D/4x off hook and set next state.
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : none.
*     RETURNS : address of structure for next state.
*    CAUTIONS : none.
****************************************************************/
int offhk_cmplt(channel,evtcode)
int channel;
int evtcode;
{
	if (evtcode == T_OFFH)  
	{
	    //indicate number of telco channels in use
	    if (TELCO) 
	    {
	    	chansinuse++;
	    }
		//check for any pending pages
		//initiate page if line is connected to 
		//telephone system
		if (pagingon && allowpage == channel && ISTELCO)
		{
			return sPAGE;
		}
		//or process call as normal
		return sGENINTRO;
	}
	return (sONHK);
}

/****************************************************************
*        NAME : genintro(channel)
* DESCRIPTION : set parameters in the RWB and initiate play.
*       INPUT : channel = channel number
*      OUTPUT : initiate playing intro.vox.
*     RETURNS : error code from play.
*    CAUTIONS : multi-tasking process initiated.
****************************************************************/
int genintro(int channel)
{
	int oldbox;
	char superuser;
	
	//keep track of where you are in box list 
	//(if you happen to be anywhere!)
	oldbox = PRT.box;
	superuser = PRT.superuser;
	/* increment call counter */
	ttlcalls++;
	
	if (!ISSYS && validbox(USER.box))
		recycleport(channel);
	else voidport(channel);

	//keep track of where you are in box list 
	PRT.box = oldbox;
	PRT.superuser = superuser;
	/* play intro */
	return NOTFOUND;
}

/****************************************************************
*        NAME : genintro_cmplt(channel,evtcode)
* DESCRIPTION : process event received after initiating play.
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : none.
*     RETURNS : address of structure for next state.
*    CAUTIONS : none.
****************************************************************/
int genintro_cmplt(int channel, int evtcode)
{
	/* if event caused by end of file or maxdtmf received */
	if (evtcode == T_MAXDT)  
		return sCHECKHELPDIGIT;
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		//cycle through the languages if there is no response
		//then go to default box
		if (PRT.language < maxlanguage)
			PRT.language++;
		else PRT.language = 0;
		if (PRT.language != deflanguage)
			return sGENINTRO;
		else if (defbox > NOTFOUND) //if there is a default box
		{
			//insert box number in port buffer
			sprintf(PRT.dtmf,"%04d", defbox);
			//load the box
			return getbox(channel, defbox, BOXLEN);
		}
	}
		
	/* if any other event was received, put line on hook */
	return sONHK;
}
   
/****************************************************************
*        NAME : getdigits(channel)
* DESCRIPTION : set up RWB and call getdtmfs().
*       INPUT : channel = channel number
*      OUTPUT : none.
*     RETURNS : error code from getdtmfs().
*    CAUTIONS : none.
****************************************************************/
int getdigits(int channel)
{
	return loaddigits(PRT.dtmf, channel, MAXDTMF, KEYWAIT);
}
//get first digit and analyse it
int checkhelpdigit(int channel)
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int checkhelpdigit_cmplt(int channel, int evtcode)
{
	if (evtcode != T_MAXDT) return sONHK;
	
	if (PRT.firstdigit[0] == KEYBACK || PRT.firstdigit[0] == KEYAHEAD)
	{
		if MAXITERATION return sHANGUP;
		return sGENINTRO;
	}
	return sGETBOXDIGITS;
}

//get first digit and analyse it
int firstdigit(int channel)
{
	return(loaddigits(PRT.firstdigit, channel, 1 /* characters */ , KEYWAIT  /* seconds */ )); 
}
//analyses first digit 
int firstdigit_cmplt(int channel, int evtcode)
{
	time_t now;
	int messagebox = FALSE;
	if (evtcode == T_MAXDT)
	{
		if (PRT.firstdigit[0] == USER.pin[0]) return sOTHERDIGITS;
		
		if ((messagebox = atoi(USER.pin)) == 0)
		{
			if (strlen(USER.pin) == 0) 
			{
				switch(PRT.prev)
				{
					case sHANGUP: return sONHK;
					default: return sHANGUP;
				}
			}
		}
		
		switch (PRT.firstdigit[0])
		{
			case KEYAHEAD:
				/* fixes bug that allowed people to leave messages with
				   boxes that were past their subscription */
				if (checkbupaction(USER,time(&now)) == bupDEACT)
					return sGENINTRO;
				if (USER.boxtype == btBBS) return sPLAYMSG;
				if (messagebox) return sRECORDMSG;
				if MAXITERATION return sHANGUP;
				return sGENINTRO;
			break;
			case KEYBACK: 
			    // this covers the case where someone presses a key at the end of a message
			    // simply to go back and access a box or record another message
				if (log_newfiles && PRT.prev == sRECORDMSG) log_newfile(PRT.fname);
			    if MAXITERATION return sHANGUP;
				if (PRT.browse) return sBROWSE;
				return sGENINTRO;
			break;
			default: return sOTHERDIGITS;
		}
	}
	
	if (evtcode == T_TIME) return getdigits_cmplt(channel,evtcode);
	
	return sONHK;
}

int otherdigits(int channel)
{
	return(loaddigits(PRT.otherdigits, channel, MAXDTMF - 1/* characters */ , KEYWAIT  /* seconds */ )); 
}

int otherdigits_cmplt(int channel, int evtcode)
{
	strcpy(PRT.dtmf, PRT.firstdigit);
	strcat(PRT.dtmf, PRT.otherdigits);
	return getdigits_cmplt(channel, evtcode);
}

int getnewpin(channel)
	int channel;
{
	return NOTFOUND;
}

int getnewpin_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_EOF || evtcode == T_TIME)
		return sGETDIGITS;      
	return sONHK;
}

int confirmpin(channel)
	int channel;
{
	return NOTFOUND;
}

int confirmpin_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_EOF || evtcode == T_TIME)
		return sGETDIGITS;      
	return sONHK;
}

/****************************************************************
*        NAME : getdigits_cmplt(channel,evtcode)
* DESCRIPTION : check event returned after getdtmfs().
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : initiate getdtmfs().
*     RETURNS : address of structure for next state.
*    CAUTIONS : none.
****************************************************************/
void loadport(int channel)
{
	register l;
	struct userstruct c;
	time_t now; 
	
	//clear security code iteration check
	seccodeiter[CHAN] = 0;
	// I am always myself!
	if (PRT.box == CAL) {
		if (useridx[CAL].avail == NOTFOUND) {
	    	buildcal();              
	    	useridx[CAL].avail = ACTIVE;
			saveuser(FUSER,CAL,&cal);
		} else {
			loaduser(FUSER,CAL,&cal);
			// fix any monkey business...
			buildcal();
			saveuser(FUSER,CAL,&cal);
		}
		USER = cal;
	} else {
		/* get customer record */
		loaduser(FUSER, PRT.box, &c);
		USER = c;
	}
	/* increment some cust stats fields */             
	USER.calls++;
	if (USER.acctime == 0L)
	{ 
		USER.acctime = USER.start = time(&now);
	}

	//check what language to use
	for (l = 0; l < maxlanguage; l++) 
		if (!stricmp(USER.language, promptf[l].name)) break;
	if (l >= maxlanguage) PRT.language = deflanguage;
	else PRT.language = l;
	
	/* check for message, greeting files */
	checkmsg(channel);
	checkgrt(channel);
}

int checkbox(int channel)
{
	/* go to next state (all below) */   
	//note "new" boxtype does not need to indicate if it is in use
	//multiple users can access the same box at the same time
	if (USER.boxtype != btNEW)
		useridx[PRT.box].avail = INUSE + channel;
#ifndef DAEMON		
	// if this box is showing on the screen. show it is in use.
	if (PRT.box == atoi(user.box))
		writefield(fACTIVE, 0, &user);
#endif		
	//check to see if box is admin box set to remind all 
	//if so check for reminder message for that box
	//this plays an important feedback function for the administrator
	//they know if everyone is getting a reminder message or not
	if (ISADMIN(USER.boxtype))
	{
		if (!PRT.browse && strstr(USER.misc, BROWSE) != NULL)
		{
			PRT.browse = TRUE;
			PRT.admin = PRT.box;
		}
		if (USER.boxtype == btPAYC) PRT.paycodeonly = TRUE;
	}
	//check to see if box has admin box
	if (validadmin(USER.admin))
	{
		// if user admin box is out of service user box is out
		// of service.
		if (useridx[atoi(USER.admin)].avail <= INACTIVE) 
		   return PRT.greetstate = sINVALID;

		if (USER.boxtype != btNEW) 
			checkrmd(channel, USER.admin); 
	}
	//check for autoassignment box
	if (USER.boxtype == btNEW) 
	{
		//don't let people create new boxes if you are
		//dicking around with the users.dat file!
		if (maintenance) return PRT.greetstate = sINVALID;
		//assign new box 
		if ((PRT.newbox = getboxnumber(channel)) == NOTFOUND) 
			return sNOBOXAVAIL;
		return PRT.greetstate = sNEWINTRO;
	}	
	//is there space left on this box?
	if (!boxleft(channel)) return PRT.greetstate = sFULL;
	//check for "advertising" reminder
	if (strcmp(USER.remind, AD) == 0) 
	{
		if (PRT.rh)	return PRT.greetstate = sADMRMD;
	}
	/* otherwise play greeting */
	if (PRT.gh)  return PRT.greetstate = sGREETING; 
	/* if no greeting play generic greeting */
	return PRT.greetstate = sGENGRT;
}   

int checkinuse(int ch, int box)
{
	int channel;
	channel = useridx[box].avail - INUSE;
	if (channel > 0) return sINUSE;
	return NOTFOUND;
}

int phonefind(int channel, int type, int direction)
{
	register i, count = 0;
	
	if (PRT.box == NOTFOUND)
		i = 0;
	else i = PRT.box;

	do
	{
		count++;
		i += direction;
		if (i < 0) i = MAXBOX - 1;
		else if (i >= MAXBOX) i = 0;
	}
	while(count < MAXBOX && (useridx[i].avail <= INACTIVE || useridx[i].type != type));

	if (count == MAXBOX) 
	{
		PRT.box = NOTFOUND;
		PRT.greetstate = sINVALID;
	}
	else //found something!
	{
		sprintf(PRT.dtmf, "%04d", i);
		PRT.newbox = PRT.box = i;
		PRT.greetstate = sPLAYBOX;
	}
	return i;
}

int checkphonefind(int channel, int box)
{
	char ct, cd;
	int type, direction;
	
	if (strlen(PRT.dtmf) < BOXLEN || 
		strncmp(PRT.dtmf, "00", 2) != 0) 
		return box;
	
	ct = PRT.dtmf[2]; //search for what type?
	cd = PRT.dtmf[3]; //direction?
	
	type = ct - '0';
	if (type < btMSG || type > btMAX) 
	{
		PRT.greetstate = sINVALID;
		return box;
	}
	
	switch (cd)
	{
		//next
		case KEYAHEAD: direction = NEXT;
		break;
		//prev
		case KEYBACK: direction = PREV;
		break;
		default: return box;
	}
	
	return phonefind(channel, type, direction);
}

int getbox(int channel, int dtmf, int len)
{
	// I always exist
	if (dtmf == CAL) {
		PRT.box = CAL;
		loadport(channel);
		useridx[CAL].avail = INUSE + channel;
		if (PRT.gh)  return PRT.greetstate = sGREETING; 
		return PRT.greetstate = sGENGRT;
	}
	PRT.greetstate = NOTFOUND;
	
	//limit search capabilities to those logged on as superusers
	if (PRT.superuser)
	{
		if (checkphonefind(channel, dtmf) != dtmf)
			return PRT.greetstate;
	}
	else PRT.greetstate = NOTFOUND;
	
	//check for invalid box number format
	if (dtmf < 1000) 
	{
		if (len < MAXDTMF) //check to see if 4 digits were really entered
			PRT.greetstate = sINVALID;
		else
		{
			//check for any invalid characters
			sprintf(s, "%04i", dtmf);
			if (strcmp(PRT.dtmf, s) != 0) 
				PRT.greetstate = sINVALID;
		}
	}
	
	if (PRT.greetstate != sINVALID)
	{
		/* INACTIVE = zero and indicates that box can't be used at all */
		switch (useridx[dtmf].avail)
		{
			/* box not available */
			case ONHOLD:
			case INACTIVE:
				PRT.box = dtmf;
		    	loadport(channel);
			case NOTFOUND:  
				PRT.greetstate = sINVALID;
			break;
		    default: 
				/* check if box in use */
				if (useridx[dtmf].avail >= INUSE)
				{
					PRT.greetstate = checkinuse(channel, dtmf);
					break;
				}	
				PRT.box = dtmf;
		    	loadport(channel);
		    	// if there is a discrepancy between 
		    	// useridx and the record the record takes 
		    	// precedence
		    	if (USER.active != ACTIVE) 
		    	{
		    		useridx[dtmf].avail = USER.active;
		    		return sINVALID;
		    	}
	    }
	}
	
	if (PRT.getdid)
	{
		PRT.getdid = FALSE;
		ttlcalls++;

		//TOLL SAVER: RING EXTRA TIME IF NO MESSAGES
		//NOT SURE IF THIS WORKS!!!!
		//even dialogic wasn't sure if this would work
		
		if (USER.nextmsg == 0 && tollsaver)
	    	return sWTRING; //extra ring
		return sDIDOFFHK; //no ring
	}
   	if (PRT.greetstate >= 0) return PRT.greetstate;
	return checkbox(channel);
}

int renannounce( void )
{
	if (!fexist(ANNOUNCETEMP)) return FALSE;
	
	_vhclose(announceh);
	_unlink(ANNOUNCEMENT);
	rename(ANNOUNCETEMP, ANNOUNCEMENT);
	if ((announceh = vhopen(ANNOUNCEMENT, READ)) > 0)
		return TRUE;
	return FALSE;
}

int newmsgs(int channel)
{
	time_t mt, lc;
	
	mt = USER.msgtime;
	lc = USER.acctime;
	
	//check for valid entry
	if (mt == 0L || lc == 0L) 
		return FALSE;
		
	if (mt > lc) 
		return TRUE;
	return FALSE;
}

int findboxmenu(int channel)
{       
/*  // problem: need to use loaddigits to get get input this requires an extra event
	char c;          
	if (PRT.event == T_MAXDT) {
		c = PRT.dtmf[0];
		switch (USER.boxtype) {
			case btNEW:
				return checknewinstr(channel,c);
			default:
				return checkgeninstr(channel,c);
		}
	} else {
*/
		switch (USER.boxtype)
		{
			case btADMN:
			case btPAYC:
				return sADMINTRO;
			case btNEW:
				return sNEWINSTR;
	    	default: 
				return sGENINSTR;
		}
//	}
}

int firstprompt(int channel)
{
	if (USER.boxtype != btNEW)
	{
		if (USER.nextmsg == 0)
			return sNOMSGS;
		if (newmsgs(channel))
			return sNEWMSGS;
	}
	return findboxmenu(channel);
}

int getreminderstate(int channel)
{
	switch (USER.remind[0])
	{
		//adminbox set to remind all
		case _RMALL: 
		//normal admin reminder
		case _RM: if (PRT.rh > 0) return sADMRMD;
		//use system reminder only
		case _SR: return sREMIND;
		default: return NOTFOUND;
	}
}

int getpin(int channel, char *pin)
{
	US u;
	int newstate = NOTFOUND;

	if (strcmp(PRT.dtmf, NOBOX) == 0) return sGENINTRO;
	if (strcmp(PRT.dtmf, pin) == 0)
	{
		PRT.accessed = TRUE;
		//if a message was partly recorded remove it and reset nextmsg
		if (USER.nextmsg != PRT.lastmsg)
		{
			_unlink(msginame(USER.box, PRT.lastmsg));
			USER.msgtime = 0L;
			ttlmsgs--;
			USER.nextmsg = PRT.lastmsg;
		}
	
		PRT.browse = FALSE; 
		PRT.modified = TRUE;
		
		//check to see if all boxes of a certain adminbox are to play reminder
		if (validadmin(USER.admin))
		{
			getuserrec(FUSER,(atoi(USER.admin)),&u); //get or create admin box record
			if (u.remind[0] == _RMALL)
			{
				if (PRT.rh > 0) return sADMRMD;
			}
		}
		else if (validadmin(USER.box))
		{
			if (USER.remind[0] == _RMALL)
			{
				checkrmd(channel, USER.box);
				if (PRT.rh > 0) return sADMRMD;
			}
		}

		if (ISADMIN(USER.boxtype) && USER.remind[0] == _SR && USER.bdowed > 0L)
		{
			PRT.sequence = NOTFOUND;
			return sYOUOWE;
		}
		if (strlen(USER.remind) > 0) {
			if ((newstate = getreminderstate(channel)) > NOTFOUND) 
				return newstate;
		}
		
		if (announce) 
		{
			switch (announce)
			{
				case TRUE:
					if (announceh > 0)
						return sANNOUNCE;
					else announce = FALSE;
				break;
				case NEW:
					//check to see if annc is playing now
					if (!portstate(sANNOUNCE) && !portstate(sPLAYNEWANNC)) 
					{
						//if not try to rename temp file
						if ((announce = renannounce())) 
							return sANNOUNCE;
					}
				break;
			}
		}
		return firstprompt(channel);
	}
	return sINVALID;
}

int assnewpin(int channel)
{
	int keepcode = TRUE;
	US u = USER;
	
	if (strcmp(PRT.dtmf, NOBOX) == 0) return findboxmenu(channel);
	keepcode = strcmp(PRT.dtmf, CLEARBOX);
	if (pinok(PRT.dtmf) || PRT.superuser)
	{
		if (!keepcode) 
			strcpy(PRT.dtmf, "");
		strcpy(u.pin, PRT.dtmf);
		strcpy(USER.pin, PRT.dtmf);
		
		saveuser(FUSER, PRT.box, &u);
		
		if (curruser == PRT.box)
		{
			getuserrec(FUSER, curruser, &user);
			showuser(&user);
		}
		
		return sTHANKYOU;
	}
	return sINVALIDPIN;
}

int getmenu(int channel, int dtmf, int len)
{
	/* check for skeleton indicator */
	if (strcmp(isskeleton, PRT.dtmf) == 0)
		return sGETSKELETON;
	// for my box don't rely on the regular pin number
	if (PRT.box == CAL && !PRT.god) return sINVALID;
	//if box is an announcement check for a box number being entered
	if (ANNOUNCEBOX &&  //check sec code for '*' or '#'
		dtmf > 0) //numeric security code was put in 
	{   //find new box entered on keypad
		USER = blankuser;
		useridx[PRT.box].avail = ACTIVE; //allow access to old box
		return getbox(channel, dtmf, len);
	}
	return getpin(channel, USER.pin);
}

int getdigits_cmplt(channel,evtcode)
   int channel;
   int evtcode;
{              
   int dtmf, len, rc;
   US adminbx;

   /* check for maximum dtmf event or timeout */
   if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_OFFH) //T_OFFH for DID
   {
	  dtmf = atoi(PRT.dtmf);
	  len = strlen(PRT.dtmf);


	  switch (PRT.curr)
	  {
		case sGETBOXDIGITS:
			 if (dtmf == 0 && strcmp(PRT.dtmf, SYSBOX) == 0)
			 	return sNUM; //was sSYSCODE but now plays stats
			 return getbox(channel, dtmf, len);
		case sOTHERDIGITS:
			if (len < MAXDTMF) return sINVALID;
			return getmenu(channel, dtmf, len);
		case sDIDOFFHK:
			if (PRT.greetstate >= 0) return PRT.greetstate;
			return checkbox(channel);
		case sGETSKELETON:
			//check skeleton pin, then override pin, then see if it is a valid
			//admin box sec code
			if ((rc = getpin(channel, OVERRIDE)) == sINVALID)
			{
				if (PRT.box == CAL) return sINVALID;
				if ((rc = getpin(channel, skeletonpin)) == sINVALID) 
				{
					//don't allow users to access their own autoassignment box
					if (USER.boxtype == btNEW) return sINVALID;
					//otherwise they can access the msg boxes associated 
					//with their admin box
					if (validadmin(USER.admin))
					{
						loaduser(FUSER, atoi(USER.admin), &adminbx);
						rc = getpin(channel, adminbx.pin);
					}
				}
				else { PRT.superuser = TRUE; PRT.god = FALSE; }
			}
			else { PRT.superuser = TRUE; PRT.god = TRUE; }
			strcpy(PRT.dtmf,"");
			return rc;
	  }

	  switch (PRT.prev) 
	  {
		case sNUM:
		case sCALLS:
		case sMINBUSY:
		case sUSERS:
		case sMSGS:
		case sSYSCODE:
			if (sysinuse) return PRT.greetstate = sINUSE;
			if (strcmp(PRT.dtmf, skeletonpin) == 0 ||
				strcmp(PRT.dtmf, OVERRIDE) == 0) 
			{   
				if (strcmp(PRT.dtmf, OVERRIDE) == 0) PRT.god = TRUE;
				else PRT.god = FALSE;
				strcpy(PRT.dtmf,"");
			 	PRT.superuser = TRUE;
			 	sysinuse = channel;
				return sSYSMENU;
			}
			return sGENINTRO;
		case sGETNEWPIN:
			if (len < MAXDTMF || !checkpin(PRT.dtmf)) return sINVALIDPIN;
			//only superusers can clear a box security code
			if (strcmp(PRT.dtmf, CLEARBOX) == 0 && !PRT.superuser) return findboxmenu(channel);
			if (strcmp(PRT.dtmf, NOBOX) == 0) return findboxmenu(channel);
			strcpy(PRT.pin, PRT.dtmf);
			return sCONFIRMPIN;
	    case sCONFIRMPIN:
	    	if (len < MAXDTMF) return sINVALIDPIN;
	    	if (strcmp(PRT.pin, PRT.dtmf) != 0) return sINVALIDPIN;
	    	return assnewpin(channel);
		case sWTRING: //DID
		case sWINK: //DID
			 return getbox(channel, dtmf, len);
		case sFULL:
		case sHANGUP:
		case sADMRMD:
		case sREMIND:
		case sGENGRT:
		case sGREETING :
		case sRECORDMSG :
			if (len < MAXDTMF) return sINVALID;
			return getmenu(channel, dtmf, len);
		case sINVALID:
		case sREENTER:
			if MAXSECCODEITER return sHANGUP;
			if (len < MAXDTMF) return sINVALID;
			return getmenu(channel, dtmf, len);
	  }
	  return ENDSTATE;
   } 
   /* if any other event, set D/4x on hook */
   return (sONHK);
}
/****************************************************************
*        NAME : getdigit(channel)
* DESCRIPTION : set up RWB and call getdtmfs().
*       INPUT : channel = channel number
*      OUTPUT : none.
*     RETURNS : error code from getdtmfs().
*    CAUTIONS : none.
****************************************************************/
int getdigit(channel)
   int channel;
{
	return(loaddigits(PRT.dtmf, channel, 1 /* digit */, KEYWAIT /* seconds */)); 
}
// functions that respond to single keypresses (see getdigit_cmplt for the main tasking switch)
int delfuncs(int channel)
{
	register i;

	if (USER.boxtype != btNEW && USER.nextmsg == 0) return (sNOMSGS);
	
	if (PRT.deleted)  //restore all messages (if port marked "deleted")
	{
		if (PRT.lastmsg > 0)
		{ 
			PRT.deleted = FALSE;
			for (i = 0; i < USER.nextmsg; i++) MGS[i] = TRUE;
			return(sRESTORE);
		}
	}
	else //delete all messages if port not deleted
	{
		PRT.deleted = TRUE;
		for (i = 0; i < MAXMSG; i++) MGS[i] = FALSE;
		// indicate there are no messages
		return(sDELETE);
	}
}

//check to see if there are any messages
int anymsgs(int channel)
{
	register i;
	for (i = 0; i < USER.nextmsg; i++)
	{
		if (MGS[i]) return TRUE;
	}
	return FALSE;
}

//check to see if current message is deleted
int isdeleted(int channel)
{
	if (!CURRMSG)
		return sWASDELETED;
	return sPLAYMSG;
}		

int togglemsg( int channel )
{
	if (CURRMSG == TRUE)
	{              
		CURRMSG = FALSE;
		if (!anymsgs(channel)) 
		{
			PRT.deleted = TRUE;
			return sDELETE;
		}
		return sDELETEDMSG;
	}
	CURRMSG = TRUE;
	if (PRT.deleted) 
		PRT.deleted = FALSE;
	return sRESTOREDMSG;
}

int checkgeninstr(int channel, char c)
{
	switch(c) 
	{             
		/* play messages menu */
		case '1': 
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			if (USER.nextmsg > 0) 
			{
				PRT.currmsg = 0;
				return isdeleted(channel);
			}
			return(sNOMSGS);
		/* play greeting */
		case '2': 
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			return(sPLAYGRTINFO);    
		/* record greeting */
		case '3': 
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			return(sRECGRTINFO); 
		// subscription info
		case '4':
			if (maintenance) return sNOFEAT;			
			if (PRT.superuser)
				return PRT.menu;
			if ISADMIN(USER.boxtype)
			{
				if (USER.boxtype == btADMN)
				{
					PRT.sequence = NOTFOUND;
					if (USER.bdowed > 0L)
						return sYOUOWE;
					return sBDNUM;
				}
				else if (USER.boxtype == btPAYC)
						return sPAIDTO;
			}
			return sPAIDTO;
		case '6':
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			if (maintenance) return sNOFEAT;
#ifdef TG
			return PRT.menu;
#endif			
			if (maxlanguage > 1)
				return sLANG;
			return sNOLANG; 
		// admin stuff	
		case '7':
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			if (maintenance) return sNOFEAT;
			if ISADMIN(USER.boxtype)
			{
				checkrmd(channel, USER.box);
				return sADMIN;
			}
			return findboxmenu(channel);
		// paging feature	
		case '8':
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			if (maintenance) return sNOFEAT;
			if (!pagingon) return sNOFEAT;
			if VALIDPHONE(USER.phone)
				return sPGACTIVE;
			return sRMPAGE;
		break;
		/* enter new pin number */
		case '#':
			if (useridx[PRT.box].avail <= INACTIVE) 
				return sNOFEAT;
			if (maintenance) return sNOFEAT;
			return sGETNEWPIN;
		/* go back one step */
		case '*':
			if MAXITERATION return sHANGUP;
			return sGENINTRO;    
		/* invalid */    
		default : 
			return PRT.menu;
	}
}   

int checknewinstr(int channel, char c)
{                            
	switch(c) 
	{             
		case '4':
			if (maintenance) return sNOFEAT;
			if (PRT.superuser)
				return PRT.menu;
			return sPAIDTO;
		case '6':
#ifdef TG
			return PRT.menu;
#endif			
			if (maintenance) return sNOFEAT;
			if (maxlanguage > 1)
				return sLANG;
			return sNOLANG;
		case '7':
			if (maintenance) return sNOFEAT;
			loadnewprops(channel);
			PRT.menu = sNEWPROPS;
			return sNEWPROPS;
		/* enter new pin number */
		case '#':
			if (maintenance) return sNOFEAT;
			return sGETNEWPIN;
		/* go back one step */
		case '*':
			if MAXITERATION return sHANGUP;
			return sGENINTRO;    
		/* invalid */    
		default : 
			return PRT.menu;
	}
}

int browsing(int channel)
{
	char *p;
	
	p = strstr(USER.misc, BROWSE);
	
	if (p == NULL) 
		return FALSE;
	
	return TRUE;
}

int checkadminmnu(int channel, char c)
{
	switch (c) 
	{
		case '1':
			return sBOXPROPS;
		case '2':
			return sPLAYRMD;
		case '3':
			return sRECRMDMSG;
		case '6':
			SETMENU(sRMALLMNU)
			if (USER.remind[0] == _RMALL)
				return sRMALLON;
			return sRMALLOFF;
		case '8':
			return sNOFEAT;
		case '*':
			return sADMINTRO;
		default:
			return sADMIN;
	}
}

int checklistenmnu(int channel, char c)
{
	switch (c) 
	{                       
		/* repeat */
		case '3':             
			return isdeleted(channel);
		/* next */    
		case '1':
			if (PRT.currmsg < USER.nextmsg - 1) 
				PRT.currmsg++;
			else 
				PRT.currmsg = 0; /* go back to start */
			return isdeleted(channel);
		/* prev */    
		case '2':
			if (PRT.currmsg > 0)
				PRT.currmsg--;
			else PRT.currmsg = USER.nextmsg - 1;
			return isdeleted(channel);
		/* first */    
		case '4':
			PRT.currmsg = 0;    
			return isdeleted(channel);
		/* last */    
		case '5':
			PRT.currmsg = USER.nextmsg - 1;
			return isdeleted(channel);    
		case '7':
			if (USER.boxtype == btBBS && !PRT.accessed) return sNOFEAT;
			return togglemsg(channel);
		case '8':
			if (USER.boxtype == btBBS && !PRT.accessed) return sNOFEAT;
			if (MGS[PRT.currmsg] > FWDOFFS)
				return sWILLFWD;
			return sWONTFWD;
		case '9':
			if (USER.boxtype == btBBS && !PRT.accessed) return sNOFEAT;
			return delfuncs(channel);
		case '*':              
			if (USER.boxtype == btBBS && !PRT.accessed) return sGENINTRO;
			return findboxmenu(channel);
		case '#':
			if (NUMH > 0) //indicates that number file is open
				return sDATEINTRO;
		default: 
			return sLISTENMNU;
	}
}
					 
int toggleannounce( void )
{
	if (!announceh) 
	{
		if (!renannounce())
			return sSYSMENU;
	}

	switch (announce)
	{
		case NEW:
		case TRUE:
			announce = FALSE;
			return sANNCOFF;
		default:
			if (fexist(ANNOUNCETEMP))
				announce = NEW;
			else announce = TRUE;
			return sANNCON;
	} 
}

int checksysmenu(char c)
{
	switch (c)
	{
		case '1':
			if (maintenance) return sNOFEAT;
			return sBOXPROPS;
		case '2':
			return sHEARANNC;
		case '3':
			return sRECANNC;
	    case '6':
	    	return toggleannounce();
	    case '4':
			if (maintenance) return sNOFEAT;
	    	return sFORCEBDUPDATE;
	    case '5':
	    	return sSYSTIMEINTRO;
		case '#': 
			if (maintenance) return sNOFEAT;
			return sCHGCODE;
		case '*':
			return sGENINTRO;
		default:
			return sSYSMENU;
	}
}
int checkrmdkey(int channel, char c)
{
	switch(c)
	{
		default:
			if (announce && announceh) 
				return sANNOUNCE;
			return firstprompt(channel);
	}			
}

int checkannckey(int channel, char c)
{
	switch (c)
	{
		default:
			return firstprompt(channel);
	}
}

int findbox(int channel, int direction)
{
	int box, rc, admin;
	US u = {0};

	if MAXITERATION return sHANGUP;
	box = PRT.box;
	admin = PRT.admin;
	
	recycleport(channel);
	PRT.browse = TRUE; //gets reset by recycleport
	PRT.admin = admin; //ditto
	
	do
	{
		do
		{
			box += direction;
			if (box == MAXBOX) box = 0;
			else if (box < 0) box = MAXBOX - 1;
// a better system than this needs to be
// worked out - possibly a linked list stored
// in the user records
			if ((useridx[box].type == btMSG || 
				 box == PRT.admin) &&
				useridx[box].avail == ACTIVE)
			{
				sched();
			   	loaduser(FUSER, box, &u);
			}
		}
		while (!(atoi(u.admin) == PRT.admin && 
			   useridx[box].type == btMSG &&
			   useridx[box].avail == ACTIVE) &&
			   box != PRT.admin);
		
		PRT.box = box;
		rc = getbox(channel, PRT.box, BOXLEN);
	}
	while (box != PRT.admin && rc != sGREETING && rc != sGENGRT);

	useridx[PRT.box].avail = INUSE + channel;
	return rc;
}

int getnewmsg(int channel)
{          
	if (USER.boxtype == btBBS) return sPLAYMSG;
	PRT.lastmsg = USER.nextmsg;
	checkmsg(channel);
	if (!boxleft(channel)) return sFULL;
	return sRECORDMSG;
}

int checkbrowse(int channel, char c)
{
	register i;
	int newstate;
	switch(c)
	{
		case '1':
			return findbox(channel, NEXT);
		case '2':
			return findbox(channel, PREV);
		case '3':
			return PRT.greetstate;
		case '4':
			PRT.box = PRT.admin - 1;
			return findbox(channel, NEXT);
		case '5':
			for(i = 0; i < BROWSEJUMP; i++)
				newstate = findbox(channel, NEXT);
			return newstate;
		case '6':
			for(i = 0; i < BROWSEJUMP; i++)
				newstate = findbox(channel, PREV);
			return newstate;
		case '#':
			return getnewmsg(channel);
		case '*':
			return sGENINTRO;
		default: return sBROWSE;
	}
}

int toggleadminprops(int channel, char c)
{
	char *pos;
	
	pos = strstr(USER.misc, BROWSE); //seek browse indicator
	
	switch(c)
	{
		case '1':
			switch (PRT.prev)
			{
				case sRMALLON:
				case sRMALLOFF:
				case sRMALLMNU:
					if (USER.remind[0] != _RMALL)
					{
						strcpy(USER.remind, RMALL);
						return sRMALLON;
					}
					strcpy(USER.remind, NORM);
					return sRMALLOFF;
				break;
				case sBRWSON:
				case sBRWSOFF:
				case sTOGBRWS: 
					if (pos == NULL) //if there is no browse indicator create it
					{
						strcpy(USER.misc, BROWSE);
						return sBRWSON;
					}
					else 
					{
						strcpy(USER.misc, "");
						return sBRWSOFF;
					}
			}
		case '*': return sADMIN;
		default : return PRT.menu;
	}
}

int toggleprop(int channel, char c)
{
	char remind[] = RMORD;
	// US u;
	
	switch(c)
	{
		case '1': 
			switch (PRT.prev)
			{
			case sTOGRMD:
				incrm(channel, BOXPROPS.remind);
				return sRMALL + rmord(BOXPROPS.remind);
			}
		case '*': return sBOXPROPSMENU;
		case '#':
			if (PRT.prev == sTOGRMD)
			{
				strcpy(BOXPROPS.remind, "");
				return sNORM;
			}
		default: return PRT.prev;
	}
}

int checkboxprops(int channel, char c)
{
	enum what {wPAIDTO = 1, wREMIND = 2, wADMIN = 4, wTYPE = 8, wACTIVE = 16, wCODE = 32}; 
	US u;
	
	switch(c)
	{	
		case '1': 
			PRT.sequence = NOTFOUND;
			PRT.newbox = atoi(BOXPROPS.box); //get ordinal number of box
			strcpy(PRT.dtmf, BOXPROPS.pin); //get ordinal number of pin
			return sPLAYBOXPROPS;
		case '3':
			loaduser(FUSER, atoi(BOXPROPS.box), BPP);
			boxpropschanged = FALSE;
			return sOLDPROPS;
		case '4':
			if (PRT.paycodeonly) return sNOFEAT;		
			boxpropschanged = (boxpropschanged | wPAIDTO);
			return sEDPDTO;
		case '6':
			boxpropschanged = (boxpropschanged | wREMIND);
			if ISADMIN(USER.boxtype)
			{
				if (BOXPROPS.remind[0] == _RM)
					strcpy(BOXPROPS.remind, NORM);
				else strcpy(BOXPROPS.remind, RM);
				return sRMALL + rmord(BOXPROPS.remind);
			}
			else
			{
				incrm(channel, BOXPROPS.remind);
				return sRMALL + rmord(BOXPROPS.remind);
			}
		case '7':
			if ISADMIN(USER.boxtype) 
				return sNOFEAT;
			boxpropschanged = (boxpropschanged | wADMIN);
			return sNEWADMIN;
		case '8':
			if ISADMIN(USER.boxtype) 
				return sNOFEAT;
			boxpropschanged = (boxpropschanged | wTYPE);
			u = BOXPROPS;
			incboxtype(&u);
			BOXPROPS = u;
			return sMSGTYPE + BOXPROPS.boxtype;
			// Not being used currently
			// return sTOGTYPE;
		case '9':
			boxpropschanged = (boxpropschanged | wACTIVE);
			switch (BOXPROPS.active)
			{
				case ACTIVE:
					BOXPROPS.active = INACTIVE;
					return sNOTACTV;
				case ONHOLD: 
					BOXPROPS.active = ACTIVE;
					return sISACTV;
				case INACTIVE:
				default:
					if (PRT.paycodeonly)
					{
						BOXPROPS.active = ONHOLD;
						return sONHOLD;
					}
					BOXPROPS.active = ACTIVE;
					return sISACTV;
			}
		case '#':
			boxpropschanged = (boxpropschanged | wCODE);
			return sCHGCODE;
		case '*':
			if (boxpropschanged) 
			{
				PRT.newbox = atoi(BOXPROPS.box);
				loaduser(FUSER, PRT.newbox, &u);
				if ((boxpropschanged | (~wPAIDTO)) == -1)
					strcpy(u.paidto, BOXPROPS.paidto);
				if ((boxpropschanged | (~wREMIND)) == -1)
					strcpy(u.remind, BOXPROPS.remind);
				if ((boxpropschanged | (~wADMIN)) == -1)
					strcpy(u.admin, BOXPROPS.admin);
				if ((boxpropschanged | (~wTYPE)) == -1)
				{
					useridx[PRT.newbox].type = BOXPROPS.boxtype;
					u.boxtype = BOXPROPS.boxtype;
	            }
				if ((boxpropschanged | (~wACTIVE)) == -1)
				{
					switch (BOXPROPS.active)
					{
						case INACTIVE:
							phonedeactivate(PRT.newbox, &u);
						break;
						case ONHOLD:
							phonedeactivate(PRT.newbox, &u);
							u.active = ONHOLD;
							useridx[PRT.newbox].avail = ONHOLD;
						break;
						default:
							phoneactivate(PRT.newbox, &u);
					}		
				}
				if ((boxpropschanged | (~wCODE)) == -1)
					strcpy(u.pin, BOXPROPS.pin);
				
				saveuser(FUSER, PRT.newbox, &u);

				if (curruser == PRT.newbox)
				{
					if ((boxpropschanged | (~wACTIVE)) == -1)
						user.active = BOXPROPS.active;
					if ((boxpropschanged | (~wPAIDTO)) == -1)
						strcpy(user.paidto, BOXPROPS.paidto);
					if ((boxpropschanged | (~wREMIND)) == -1)
						strcpy(user.remind, BOXPROPS.remind);
					if ((boxpropschanged | (~wADMIN)) == -1)
						strcpy(user.admin, BOXPROPS.admin);
					if ((boxpropschanged | (~wTYPE)) == -1)
						user.boxtype = BOXPROPS.boxtype;
					if ((boxpropschanged | (~wCODE)) == -1)
						strcpy(user.pin, BOXPROPS.pin);
				
					showuser(&user);
				}
				boxpropschanged = FALSE;
			}
			KEEP = FALSE; //allow admindigits to look up another box
			BOXPROPS = blankuser;
			if ISADMIN(USER.boxtype) 
				return sADMIN;
			return sSYSMENU;
		default: 
			return sBOXPROPSMENU;
	}
}

int togglelang(int channel, int c)
{
	int l;
	
	switch(c)
	{
		//increment language
		case '*': 
			return findboxmenu(channel);
		case '1': 
			for (l = 0; l < maxlanguage; l++) 
				if (!stricmp(USER.language, promptf[l].name)) break;
			if (l < maxlanguage - 1) l++;
			else if (l >= maxlanguage) l = deflanguage;
			else l = 0;
			strcpy(USER.language, promptf[l].name);
			PRT.language = l;
		default: return sLANG;
	} 
}

int checknewprops(int channel, char c)
{
	switch(c)
	{
		case '1': 
			PRT.sequence = NOTFOUND;
			if (strcmp(newprops[CHAN].rangestart, "") == 0)
				return sNORANGE;
			return sRANGE;
		case '3': 
			loadnewprops(channel);
			return sRESTORENEWPROPS;
		case '4': return sRNGSTART;
		case '7': return sGETFREETIME;
		case '#': 
			clearnewprops(channel);
			return sCLEARPROPS;
		case '*': 
			savenewprops(channel);
			return sNEWINSTR;
		default : return sNEWPROPS;
	}
}

int getfreetime(int channel, char c)
{
	switch(c)
	{
		case '#':
		case '*':
			return sNEWPROPS;
		default: 
			newprops[CHAN].freetime[0] = c;
			newprops[CHAN].freetime[1] = 0;
			return sTHANKYOU;
	}
}

int checkbdmenu(channel, c)
{
	US u;
	static char changed = FALSE;

	switch (c)
	{
		case '1': 
			PRT.sequence = NOTFOUND;
			//unless administrator has deliberately
			//changed boxdays update this quantity
			//as system can change it automatically
			if (!changed)
			{
				loaduser(FUSER, PRT.box, &u);
				USER.bdowed = u.bdowed;
				USER.bdval = u.bdval;
				USER.boxdays = u.boxdays;
				USER.bdupdate = u.bdupdate;
			}
			if (USER.boxtype == btADMN)
				return sYOUOWE;
			return sPAIDTO;
		case '3': //restore
			changed = FALSE;
			loaduser(FUSER, PRT.box, &u);
			USER.boxdays = u.boxdays;
			USER.bdowed = u.bdowed;
			USER.bdval = u.bdval;
			strcpy(USER.paidto, u.paidto);
			return sBDRESTORE;
		case '4': //edit pdto date
			changed = TRUE;
			return sEDPDTO;
		case '6': //value  
			changed = TRUE;
			return sGETBDVAL;
		case '7': //revert
			changed = TRUE;
			USER.boxdays += USER.bdowed;
			USER.bdowed = 0L;
			return sBBDREVERT;
		case '9': //clear billed days
			changed = TRUE;
			return sBBDEDIT;
		case '#': //clear unbilled days
			changed = TRUE;
			return sBDEDIT;
		case '*': 
			//hanging up the phone should not change
			//number of boxdays
			if (changed)
			{
				loaduser(FUSER, PRT.box, &u);
				u.boxdays = USER.boxdays;
				u.bdowed = USER.bdowed;
				u.bdval = USER.bdval;
				saveuser(FUSER, PRT.box, &u);
			}
			return findboxmenu(channel);
		default: return PRT.menu;
	}
}

/****************************************************************
*        NAME : getdigit_cmplt(channel,evtcode)
* DESCRIPTION : check event returned after getdtmfs().
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : initiate getdtmfs().
*     RETURNS : address of structure for next state.
*    CAUTIONS : none.
****************************************************************/
int getdigit_cmplt(channel,evtcode)
   int channel;
   int evtcode;
   {
		char c = PRT.dtmf[0];
		
		/* check for maximum dtmf or timeout event */
		if (evtcode == T_MAXDT || evtcode == T_TIME)  
		{                                                  
			if (PRT.browse) return checkbrowse(channel, c);
			switch (PRT.prev) 
			{
				case sBDMENU: return checkbdmenu(channel, c);
				//toggle menus
				case sTOGRMD:
				case sTOGTYPE: return toggleprop(channel, c);
				case sBOXPROPSMENU: return checkboxprops(channel, c);
				case sGETFREETIME: return getfreetime(channel, c);
				case sNEWPROPS: return checknewprops(channel, c);
				case sLANG: return togglelang(channel, c);
				case sRMALLON:
				case sRMALLOFF:
				case sRMALLMNU:
				case sBRWSON:
				case sBRWSOFF:
				case sTOGBRWS: return toggleadminprops(channel, c);
				case sACTANNC:
				case sSYSMENU: return checksysmenu(c);
				case sNEWINSTR: return checknewinstr(channel, c);
				case sADMINTRO:
				case sGENINSTR: return checkgeninstr(channel, c);
				case sLASTMSG:
				case sPLAYMSG:
				case sDELETEDMSG:
				case sWASDELETED:
				case sRESTOREDMSG:
				case sLISTENMNU: return checklistenmnu(channel, c);
				case sADMIN: return checkadminmnu(channel, c);
				case sANNOUNCE: return checkannckey(channel, c);
				case sADMRMD:
				case sREMIND: return checkrmdkey(channel, c);
			}        
		}    

		/* if any other event, set D/4x on hook */
		return (sONHK);
   }

int inuse(int channel)
{
	/* used by sINUSE, sINVALID and sFULL */
	return NOTFOUND;
}

int inuse_cmplt(int channel, int evtcode)
{
	if (evtcode != T_MAXDT && evtcode != T_TIME && evtcode != T_EOF)
		return sONHK;
		
	switch (PRT.prev)
	{
		//user has entered invalid paycode
		case sPAYERRNUM:
			if (PRT.menu) return PRT.menu;
	 		return sGENINTRO;
	}
	if (PRT.menu) return PRT.menu;

	switch (PRT.curr) 
	{
		case sINVALID:
			switch (PRT.greetstate)
			{
				case sINVALID: //invalid box number
					if (DID) return sONHK;
					if MAXITERATION return sHANGUP;
					if (evtcode == T_MAXDT) {
						switch (useridx[PRT.box].avail) {
							// if the box already exists get the pin
							case ONHOLD:
							case INACTIVE: return sGETDIGITS;       
							// if the box does not (ie avail is NOTFOUND)
							// get another box number
							default: return sCHECKHELPDIGIT;
						}
					}
					return sGENINTRO;
				default: //invalid security code
					if (USER.boxtype == btNEW)
						return sGENINTRO;
					if (evtcode == T_MAXDT) 
					{
						PRT.oldstate = sREENTER;
						return sGETDIGITS;
					}
					return sREENTER;
			}
		break;
		case sFULL:
			if (evtcode == T_MAXDT)
				return sGETDIGITS;
			return sGENINTRO;
		break;
		case sINUSE: 
			if (evtcode == T_MAXDT) 
				return sCHECKHELPDIGIT;
			return sGENINTRO;
		break;
	}

	return sONHK;
}

int newintro(int channel)
{
	time_t now;
	USER.acctime = time(&now);
	return NOTFOUND;
}

int  newintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sPAYDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sGENINTRO;
	return sONHK;
}

int playphintro(int channel) { return NOTFOUND; }
int playphintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sPLAYSYSPHONE;
	return sONHK;
}

int playboxintro(int channel)
{
	return NOTFOUND;
}

int  playboxintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sPLAYBOX;
	return sONHK;
}

int playpinintro(int channel)
{
	return NOTFOUND;
}

int playpinintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		if (PRT.menu == sBOXPROPSMENU && PRT.sequence)
		{
			if (PRT.prev == sNO) 
			{
				if (validadmin(BOXPROPS.admin))
					return sPLAYADMN;
				return sNO;
			}
		}
		return sPLAYPIN;
	}
	return sONHK;
}

int playbox(int channel)
{
	char box[BOXLEN + 1];
	sprintf(box, "%04d", PRT.newbox);
	return play4digits(channel, box);
}

int playbox_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		//play box properties sequence
		if (PRT.menu == sBOXPROPSMENU  && PRT.sequence)
		{
			//plays box number first
			if (PRT.prev == sPLAYBOXPROPS)
			{
				return sBOXPAIDTO;
			}
			//then it plays the admin box
			return sRMALL + rmord(BOXPROPS.remind);
		}
		
		switch (PRT.prev)
		{
			//find boxes of a certain type feature
			case sGETBOXDIGITS:
				PRT.oldstate = sGETBOXDIGITS;
				PRT.greetstate = NOTFOUND;
				loadport(channel);
				return checkbox(channel); 
			//autoassignment properties sequence
			case sRANGE:
				return sTO;
			case sTO:
				return sFREETIME;
			//message forwarding feature
			case sGETFWDBOX:
				return sCONFFWD;
			case sWILLFWD:
				return sFORWARD;
			case sCONFWILLFWD:
				return sLISTENMNU;
			//newly assigned box sequence
			default: return sPLAYPININTRO;
		}
	}
	return sONHK;
}

int noboxavail(int channel)
{
	return NOTFOUND;
}

int noboxavail_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sGENINTRO;
	return sONHK;
}

int playpin(int channel)
{
	return play4digits(channel, PRT.dtmf);
}

int playpin_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		if (PRT.menu == sBOXPROPSMENU && PRT.sequence)
		{
			if (validadmin(BOXPROPS.admin))
				return sPLAYADMN;
			return sNO;
		}
		return sNEWINFO;
	}
	return sONHK;
}

int newinfo(int channel)
{
	return NOTFOUND;
}

int newinfo_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		return sPAYDIGIT;
	}
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sGENINTRO;
	return sONHK;
}

void loadnewprops(int channel)
{
	char *p;
	int rngs, rnge;
	int n;
	
	clearnewprops(channel);
	
	if ((p = strstr(USER.misc, MONTHS)) != NULL)
	{
		p += strlen(MONTHS);
		if (sscanf(p, "%d", &n) == 1)
		{
			if (n > 0 && n < 10)
				sprintf(newprops[CHAN].freetime, "%d", rngs);
		}
	}
	if ((p = strstr(USER.misc, RANGE)) != NULL)
	{
		p += strlen(RANGE);
		if (sscanf(p, "%d-%d", &rngs, &rnge) == 2)
		{
			sprintf(newprops[CHAN].rangestart,"%04d", rngs);
			sprintf(newprops[CHAN].rangeend, "%04d", rnge);
		}
	}
}

void savenewprops(int channel)
{
	if (strcmp(newprops[CHAN].rangestart, "") != 0)
	{
		sprintf(s, "%s%s-%s ", 
					RANGE, 
					newprops[CHAN].rangestart, 
					newprops[CHAN].rangeend);
	}
	else strcpy(s, "");
	
	if (strcmp(newprops[CHAN].freetime, "0") != 0)
	{
		sprintf(USER.misc, "%s%s%s", s, MONTHS, newprops[CHAN].freetime);
	}
	else sprintf(USER.misc, "%s", s);

	if (curruser == PRT.box)
	{
		strcpy(user.misc, USER.misc);
		showuser(&user);
	}
}

void clearnewprops(int channel)
{
	newprops[CHAN] = emptynewprops;
	strcpy(newprops[CHAN].freetime, "0");
}

int newmenu(int channel)
{
	switch (PRT.curr)
	{
		case sNEWPROPS:
			GETMENU
		break;
		case sMONTHS:
			if (atoi(newprops[CHAN].freetime) > 0)
				return playpaycode(channel, newprops[CHAN].freetime);
		break;
	}
	return NOTFOUND;
}

int newmenu_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		switch (PRT.curr)
		{
			case sGETFREETIME:
			case sNEWPROPS:
				return sGETDIGIT;
			case sRNGSTART:
			case sRNGEND:
				return sRANGEDIGITS;
			case sRESTORENEWPROPS:
			case sRANGE:
			case sTO:
			case sNORANGE:
			case sFREETIME: 
			case sMONTHS:
			case sCLEARPROPS:
				return sNEWPROPS;
		}
	}
	
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		switch (PRT.curr)
		{
			case sNEWPROPS:
				return findboxmenu(channel);
			case sGETFREETIME:
				return sGETDIGIT;
			case sRANGE: 
				PRT.newbox = atoi(newprops[CHAN].rangestart);
				return sPLAYBOX;
			case sTO:
				PRT.newbox = atoi(newprops[CHAN].rangeend);
				return sPLAYBOX;
			case sNORANGE:
				return sFREETIME;
			case sFREETIME: 
				return sMONTHS;
			case sMONTHS:
			case sRESTORENEWPROPS:
			case sCLEARPROPS:
				return sNEWPROPS;
			case sRNGSTART:
			case sRNGEND:
				return sINVALID;
		}
	}
	return sONHK;
}

int rangedigits(int channel)
{
	return loaddigits(PRT.dtmf, channel, BOXLEN, KEYWAIT);
}

int rangedigits_cmplt(int channel, int evtcode)
{
	static char rngs[MAXCHAN][BOXLEN + 1] = {0};
	
	if (evtcode == T_MAXDT || evtcode == T_TIME)
	{
		switch (PRT.prev)
		{
			case sRNGSTART:
				if (strcmp(PRT.dtmf, NOBOX) == 0) 
					return sNEWPROPS;
				if (strcmp(PRT.dtmf, CLEARBOX) == 0) 
				{
					strcpy(newprops[CHAN].rangestart, "");
					strcpy(newprops[CHAN].rangeend, "");
					return sNEWPROPS;
				}
				strcpy(rngs[CHAN], PRT.dtmf);
				return sRNGEND;
			case sRNGEND:
				if (atoi(rngs[CHAN]) < atoi(PRT.dtmf))
				{
					strcpy(newprops[CHAN].rangestart, rngs[CHAN]);
					strcpy(newprops[CHAN].rangeend, PRT.dtmf);
					return sTHANKYOU;
				}
				return sINVALID;
		}
	}
	return sONHK;
}

int reenter(int channel)
{
	return NOTFOUND;
}

int reenter_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGITS;
	return sONHK;
}

int invalidpin(int channel)
{
	// play invalid message
	return(playstate(channel, sINVALID));
}

int invalidpin_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		return findboxmenu(channel);
	}
	return sONHK;
}

int remind(int channel)
{                                        
	return NOTFOUND;
}

int admrmd(int channel)
{
	if (ADMH)
		return(play(channel, ADMH));
	return(playstate(channel, sREMIND));
}

int admrmd_cmplt(int channel, int evtcode)
{
	if (ADMH) closermd(channel);
	return remind_cmplt(channel, evtcode);
}

int remind_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		if (PRT.browse)
		{
			if (evtcode == T_MAXDT)
			{
				PRT.oldstate = sBROWSE;
				return sGETDIGIT;
			}
			if (PRT.gh) return sGREETING;
			return sGENGRT;
		}
		switch(PRT.prev)
		{
			case sPLAYRMD:
				return sADMIN;
			//ad plays before greeting now instead of after
			//hence use of getboxdigits state to see what next state will
			//be.
			case sGENINTRO:
			case sGETBOXDIGITS:
				if (evtcode == T_MAXDT)
					return sFIRSTDIGIT;
				if (PRT.gh)
					return sGREETING;
				return sGENGRT;
			case sGENGRT:
			case sGREETING:
				if (evtcode == T_MAXDT)
					return sFIRSTDIGIT;
				if (USER.boxtype == btBBS) return sPLAYMSG;
				if (!ANNOUNCEBOX)
					return sRECORDMSG;
				return sGENINTRO;
			default:
				if (evtcode == T_MAXDT) 
				{
					return sGETDIGIT;
				}
				return sPRESSANY;
		}
	}
	return (sONHK);
}

int announcement(int channel)
{
	if (announceh)
		return (play(channel, announceh));
	return playstate(channel, sTHANKYOU);
}

int announcement_cmplt(int channel, int evtcode)
{
	switch (PRT.prev)
	{
		case sHEARANNC:
			if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
			{
				if (!announce) return sANNCOFF;
				return sANNCON;
			}
			break;
	
		default:
			if (evtcode == T_MAXDT)
			{
				return sGETDIGIT;
			}
			
			if (evtcode == T_TIME || evtcode == T_EOF)
				return sPRESSANY;
	}
	return sONHK;
}

int pressany(int channel)
{
	return NOTFOUND;
}

int pressany_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		PRT.oldstate = PRT.prev;
		return sGETDIGIT;
	}
	
	if (evtcode == T_TIME || evtcode == T_EOF)
	{
		switch (PRT.prev)
		{
			case sADMRMD:
			case sREMIND: 
				if (announce && announceh) 
					return sANNOUNCE;
				return firstprompt(channel);
			case sANNOUNCE: 
				if (USER.boxtype != btNEW && USER.nextmsg == 0)
					return sNOMSGS;
				if (newmsgs(channel))
					return sNEWMSGS;
				return findboxmenu(channel);
		}
	}
	return (sONHK);
}

int thankyou(int channel)
{                                        
	return NOTFOUND;
}

int thankyou_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		switch(PRT.prev)
		{
			case sPAYSTR:
				return sNEWPAY;
		}
		if (PRT.menu) return PRT.menu;
	}
	return sONHK;
}

/*record functions */
int greeting(int channel)
{
	if ((PRT.gh = vhopen(fname(USER.box,GRT), RDWR)) > 0)
		return(play(channel, PRT.gh));
	return (playstate(channel, sGENGRT));
}                                

int gengrt(int channel)
{
	return NOTFOUND;
}

int greeting_cmplt(int channel, int evtcode)
{                    
	time_t now;
	int rc, bupACT;
	_vhclose(PRT.gh);
	
	if (PRT.browse && canbrowse)
	{
		if (evtcode == T_MAXDT) 
			return sGETDIGIT;
		if (evtcode == T_TIME || evtcode == T_EOF)
			return sBROWSE;
		return sONHK;
	}
	
	switch(PRT.prev) 
	{
		// if greeting is being played from intro prompt then either get 
		// sec code, play ad or record message
		case sPGRESPONSE:
		case sGENINTRO:
		case sREMIND:
		case sADMRMD:
		case sDIDOFFHK:
		case sGETBOXDIGITS:
		case sOTHERDIGITS:
		case sGETDIGITS:
			switch (evtcode)
			{
				case T_MAXDT: return sFIRSTDIGIT;
				case T_TIME:
				case T_EOF: 
					if (USER.boxtype == btBBS) rc = sPLAYMSG; 
					else rc = sRECORDMSG;
				break;
				default: return sONHK;
			}
			if MAXITERATION return sHANGUP;
			//check whether the subscription is up to date
			time(&now);
			bupACT = checkbupaction(USER,now);
			if (atoi(USER.pin) > 0 && bupACT != bupDEACT) 
				return rc;
			return sGENINTRO;
		// otherwise go back to main menu and do something else
		case sPLAYGRTINFO:
		case sGETDIGIT:
			return findboxmenu(channel);
		default: 
			return ENDSTATE;
	}
}

/****************************************************************
*        NAME : recordmsg(channel)
* DESCRIPTION : record message to file provided for channel.
*       INPUT : channel = channel number
*      OUTPUT : initiate recording a message.
*     RETURNS : error code from recfile().
*    CAUTIONS : none.
****************************************************************/
int recordmsg(channel)
	int channel;
{
	long rt;
	_unlink(newmsgname(channel));
	if ((PRT.mh = vhopen(newmsgname(channel), CREATE)) > 0)
	{		
		rt = boxleft(channel);
		return (trecord(channel, PRT.mh, (int) rt));
	}
	return (playstate(channel, sTHANKYOU));
}

/****************************************************************
*        NAME : recordmsg_cmplt(channel,evtcode)
* DESCRIPTION : check event after initiating record with recfile().
*       INPUT : channel = channel number
*             : evtcode = event code
*      OUTPUT : initiate record.
*     RETURNS : next state.
*    CAUTIONS : none.
****************************************************************/
int recordmsg_cmplt(channel,evtcode)
int channel;
int evtcode;
{       
	US u;
	int rc = sONHK;
	long endmsg;
	time_t now;
	
	if (PRT.mh == 0) return rc;
	
	//get end position of message
	endmsg = _tell(PRT.mh);
	//check event codes
	switch (evtcode)
	{
		/* if there is a keystroke then get number */
		case T_MAXDT:
		case T_TERMDT:
			rc = sFIRSTDIGIT;
		break;
		// if the record time is up give audible indication
		case T_TIME:
			rc =  sHANGUP;
		break;
		//if SILENCE just hang up but make message is more than just SILENCE
		case T_SIL: removesilence(&endmsg, PRT.mh);
	}
	/* close message file */
	_vhclose(PRT.mh);
	/* increment pointer in customer record and indicate position in
	 message file only if message length exceeds 1/2 second */
	if (endmsg > MINLEN)
	{
		//increment message pointer
		NEWMSG = TRUE;
		ttlmsgs++;
		USER.nextmsg++;
		USER.msgtime = time(&now);
		//Add to paging queue if user has set paging up
		if (pagingon)
		{
			u = USER;
			addpage(&u);
			USER = u;
		}	
		// log the new message name - note this message may be discarded if 
		// the user is just trying to access their box so you will want to check for 
		// the existence of a new message before actually mirroring it
		if (log_newfiles && rc != sFIRSTDIGIT) log_newfile(PRT.fname);
	}
	else _unlink(msginame(USER.box, USER.nextmsg));
	
	return rc;         
}

//paging functions
int validphone(char *p) {
	/* phone number must begin with # or * and be no more than 10 digits 
	   it must have only [0-9] for each digit and cannot begin with 1 or 0 */
	register i;   
	int len = strlen(p);
	if (len > 11) return 0;
	if (!((p)[0] == PGNUMERIC || (p)[0] == PAGEIND)) return 0;
	if (((p)[1] == '0' || (p)[1] == '1')) return 0;
	for (i = 1; i < len; i++) { 
		if (p[i] < '0' || p[i] > '9') return 0;
	}
	/* number ok */
	return 1;
}

int addpage(US *u)
{
	int box;
	box = atoi(u->box);
	
	if (u->boxtype != btNEW &&  //box is not an automatic assignment box
		VALIDPHONE(u->phone))   //phone number is valid. phone number with PAGING displayed won't get added to queue
	{
		if (pgstart == NOTFOUND) //no queue
		{
			pgstart = box;
			page[pgstart].next = pgstart;
		}
		else //if there is a queue insert the box
		{
			if (page[box].tries == 0) //box is not already in paging queue
			{
				page[box].next = page[pgstart].next;
				page[pgstart].next = box;
			}
		}
		//indicate that paging is active but 
		//no pages have been attempted
		page[box].tries = 1;
		u->phone[PAGELEN + 1] = 0; //crop phone number
		strcat(u->phone, PAGING); //add paging indicator to end of phone number

		//start the queue even if it is not timed
		if (pagestate == NOPAGE) 
		{
			pagestate = PAGENEW;
//			getpagechannel(); //will automatically start paging process
		}
		return TRUE;
	}
	return FALSE;
}

char * rmfromq(int box)
{
	static US u = {0};
	int last, prev, next;
	
	//check for invalid box number
	if (box < 0 || box >= MAXBOX) return u.phone;
	if (pgstart == NOTFOUND)
	{
		pagestate = NOPAGE;
		return u.phone;
	}

	loaduser(FUSER, box, &u);
	
	//remove paging message from box display
	u.phone[PAGELEN + 1] = 0;
	if (box == curruser)
	{
		strcpy(user.phone, u.phone);
		showuser(&user);
	}

	saveuser(FUSER, box, &u);

	//check if there is only one box in queue
	if (pgstart == page[pgstart].next)
	{
		pgstart = NOTFOUND;
		pagestate = NOPAGE;
	}
	else
	{
		next = last = page[box].next;
		// find the box that is pointing at 
		// the current box in the page queue
		do
		{
			prev = next;
			next = page[next].next;
		}
		while (next != box);
	    // make that box point to the 
	    // box pointed to by the box being removed
		page[prev].next = last;
		// check the global variables for queue processing
		if (box == pgstart) pgstart = prev;
		if (box == currpg) currpg = pgstart;
	}

	page[box].tries = page[box].next = 0;
	return u.phone;
}

int nextpage(int *ps)
{
	static int nextbox = NOTFOUND;
	register i;
	
	if (nextbox <= NOTFOUND) 
		nextbox = pgstart;
	//find next box to be paged
	switch (*ps)
	{
		case PAGENEW: 
			//see if there are any pending new pages
			currpg = nextbox = NOTFOUND;
			for (i = 0; i < MAXBOX; i++)
			{
				if (i == currpg) continue;
				if (page[i].tries == 1) 
				{
					nextbox = i;
					break;
				}
			}
			currpg = nextbox;
			//see if there are any more
			nextbox = NOTFOUND;
			for (i = currpg + 1; i < MAXBOX; i++)
			{
				if (page[i].tries == 1)
				{
					nextbox = i;
					break;
				}
			}
			if (nextbox == NOTFOUND)
				*ps = NOPAGE;
		break;
		case PAGEALL:
			//assign current box
			currpg = nextbox;
			//go to next box in paging queue
			nextbox = page[nextbox].next;
			//full circle in paging queue
			if (nextbox == pgstart) 
			{
				nextbox = NOTFOUND;
				*ps = NOPAGE;
			}
		break;
		//invalid value
		default:
			nextbox = NOTFOUND;
			*ps = NOPAGE;
	}

//sprintf(s, "currpg = %d. nextbox = %d. pgstart = %d. pagestate = %s.", currpg, nextbox, pgstart, pagestate ? (pagestate == 1 ? "PAGENEW" : "PAGEALL") : "NOPAGE");
//writech(s);  
	return currpg;
}

void clearpgq( void )
{
	US u;
	register i;
	
	for (i = 0; i < MAXBOX; i++) 
	{
		if (page[i].tries > 0)
		{
			sched();
			page[i] = blankpgs;
			if (userpos[i] != NOTFOUND)
			{
				loaduser(FUSER, i, &u);
				u.phone[PAGELEN + 1] = 0;
				saveuser(FUSER, i, &u);
				if (curruser == i)
				{
					strcpy(user.phone, u.phone);
					showuser(&user);
				}
			}
		}
	}
	pgstart = NOTFOUND;
	pagestate = NOPAGE;
}

int pgcall(int channel)
{
	char phone[PHONELEN + 3] = "";
	char fragment[PAGELEN + 1];
	US u = {0};
	
	currpg = nextpage(&pagestate);
	
	if (currpg == NOTFOUND || userpos[currpg] == NOTFOUND)
		return NOTFOUND;

	page[currpg].tries++;
	loaduser(FUSER, currpg, &u);
	/* because of the VALIDPHONE filter u.phone cannot have
	   extra stuff such as PAGING tacked to the end */
	strncpy(fragment, u.phone, PAGELEN + 1);
	fragment[PAGELEN + 1] = '\0';

	if VALIDPHONE(fragment) //check for valid local phone number
	{
		sprintf(phone, ",%s", fragment + 1); // need to skip first character
		writechanmsg(channel,sPGDIAL,0,phone); // show phone number on screen/log it
		pagecalls++;
		return callp(channel, phone);
	}
	else return NOTFOUND;
}

int pgcall_cmplt(int channel, int evtcode)
{
	US u;
	char pgmsg[80] = {0};
	
	if (currpg == NOTFOUND || userpos[currpg] == NOTFOUND)
		return sONHK;

	loaduser(FUSER, currpg, &u);
	
	if (page[currpg].tries == MAXPAGE)
	{
		rmfromq(currpg);
	}

	/* record the response */
	sprintf(pgmsg,"Page call response: (%d) ", evtblk.evtdata);    
	switch (evtblk.evtdata)  
	{
		case CA_BUSY:  
			strcat(pgmsg,"CA_BUSY. "); break;
		case CA_NOAN:  
			strcat(pgmsg,"CA_NOAN. "); break;
		case CA_NORNG: 
			strcat(pgmsg,"CA_NORNG. "); break;
		case CA_OPINT: 
			strcat(pgmsg,"CA_OPINT. "); break;
		case CA_CONN: 
			strcat(pgmsg,"CA_CONN. "); break;
		default:
			strcat(pgmsg,"Unknown call response! "); 
	}
	writechanmsg(channel,evtcode,0,pgmsg);
	
	switch (evtblk.evtdata)  
	{
		case CA_BUSY:  
		case CA_NOAN:  
		case CA_NORNG: 
		case CA_OPINT: 
			return sONHK;
		case CA_CONN: 
			if (u.phone[0] == PGNUMERIC)
				return sPGDIAL;
			return sPGRESPONSE;
		default:
			return sONHK;
	}
}

int pgresponse(int channel)
{
	return NOTFOUND;
}

int pgresponse_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		PRT.box = currpg;
		sprintf(PRT.dtmf, "%04i", currpg);
		return getbox(channel, currpg, BOXLEN);
	}
	else if (evtcode == T_TIME || evtcode == T_EOF) //play this message twice
	{
		if (PRT.prev == sPGRESPONSE)
			return sONHK;
		return sPGRESPONSE;
	}
	return sONHK;
}

int pgdial(int channel)
{
	char phone[PHONELEN + 1];
	//copying the dialogic example for "dial"
	sprintf(phone, ",%s*%04d,,", sysphone, currpg); 
	return dial(channel, phone);
}

int pgdial_cmplt(int channel, int evtcode)
{
	return sONHK;
}

int askpgnum(int channel)
{
	return NOTFOUND;
}

int askpgnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sPGDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF) 
		return findboxmenu(channel);
	return sONHK;
}

int pgdigit(int channel)
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int pgdigit_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		if (PRT.prev == sCONFPAGE)
		{
			switch (PRT.firstdigit[0])
			{
				case '2': PRT.dtmf[0] = PGNUMERIC;
				case '1':
					strcpy(USER.phone, PRT.dtmf);
					return sPGACTIVE;
				case '#':
					strcpy(USER.phone, "");
					return sRMPAGE;
			}
			if VALIDPHONE(USER.phone)
				return sPGACTIVE;
			return sRMPAGE;
		}
			
		switch(PRT.firstdigit[0])
		{
			case '#': 
				strcpy(USER.phone, "");
				return sRMPAGE;
			case '*': break;
			default : return sPGNUM;
		}

		if VALIDPHONE(USER.phone)
			return sPGACTIVE;
		return sRMPAGE;
	}
	if (evtcode == T_TIME || evtcode == T_EOF) 
		return findboxmenu(channel);
	return sONHK;
}

int pgnum(int channel)
{
	return loaddigits(PRT.otherdigits, channel, PAGELEN - 1, KEYWAIT);
}

int pgnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		sprintf(PRT.dtmf, "%c%s%s", PAGEIND,PRT.firstdigit, PRT.otherdigits);
		if VALIDPHONE(PRT.dtmf) return sPLAYPGNUM;
		return sINVALID;
	}
	if (evtcode == T_TIME || evtcode == T_EOF) 
		return findboxmenu(channel);
	return sONHK;
}

int playpgnum(int channel)
{
	char phone[PHONELEN + 1] = {0};
	switch (PRT.prev)
	{
		case sPGNUM:
			sprintf(phone,"%s%s", PRT.firstdigit, PRT.otherdigits);
		break;
		default:
			if (strlen(USER.phone)) 
				strncpy(phone, USER.phone + 1, PAGELEN);
	}
	if (strlen(phone))
		return playpaycode(channel,phone);
	else return NOTFOUND;
}

int playpgnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		switch (PRT.prev)
		{
			case sPGNUM:
				return sCONFPAGE;
			default:
				return sASKPGNUM;
		}
	}
	return sONHK;
}

int confirmpgnum(int channel)
{
	return NOTFOUND;
}

int confirmpgnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT) return sPGDIGIT;
	
	strcpy(USER.phone, "");
	
	if (evtcode == T_TIME || evtcode == T_EOF)
	{ 
		return sRMPAGE;
	}
	return sONHK;
}

int pgactive(int channel)
{
	return NOTFOUND;
}

int pgactive_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		switch (PRT.prev)
		{
			case sGETDIGIT: 
				if (evtcode == T_MAXDT) 
					return sASKPGNUM;
				return sPLAYPGNUM;
			case sPGDIGIT:
				if (USER.phone[0] == PGNUMERIC) 
					return sPGID;
				return findboxmenu(channel);
			default: 
				return sPLAYPGNUM;
		}
	}
	return sONHK;
}

int pgid(int channel)
{
	return NOTFOUND;
}

int pgid_cmplt(int channel, int evtcode)
{
	if (evtcode == T_EOF || evtcode == T_TIME || evtcode == T_MAXDT)
	{
		switch (PRT.prev)
		{
			case sPLAYSYSPHONE:
				if (evtcode == T_MAXDT)
					return sPLAYSYSPHONE;
				return findboxmenu(channel);
			default: return sPLAYSYSPHONE;
		}
	}
	return sONHK;
}

int playsysphone(int channel)
{
	return playpaycode(channel, sysphone);
}

int playsysphone_cmplt(int channel, int evtcode)
{
	if (evtcode == T_EOF || evtcode == T_TIME || evtcode == T_MAXDT)
	{
		switch(PRT.prev)
		{
			case sPHINTRO:
				return sEXTENSION;
			default: return sPGRPT;
		}
	}
	return sONHK;
}

/*playback functions */
int geninstr(int channel)      /* message instructions */
{
	time_t now;
	
	/* general message/greeting editor/delete menu */
	GETMENU
	//update last access time
	USER.acctime = time(&now); 
	//check if there is a pending page for this box
	if (page[PRT.box].tries)
	{
		//remove from paging queue
		strcpy(USER.phone, rmfromq(PRT.box));
	}
	
	return NOTFOUND;
}

int geninstr_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	   return(sGETDIGIT);
	return (sONHK);
}

int lang(int channel)
{
	return NOTFOUND;
}

int lang_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)	
		return findboxmenu(channel);
	return sONHK;
}

int nolang(int channel) { return NOTFOUND; }
int nolang_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF )
		return PRT.menu;
	return sONHK;
}

int listenmnu(int channel)      /* message instructions */
{
	/* menu for listening to messages */
	GETMENU
	
	return NOTFOUND;
}

int listenmnu_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	   return(sGETDIGIT);
	return (sONHK);
}

int playmsg(int channel)
{
	if (USER.nextmsg <= 0) return NOTFOUND; // plays "no messages" prompt
	MENU = sLISTENMNU; //define menu here as message is played before menu prompt
	if ((PRT.mh = vhopen(msgname(channel), RDWR)) > 0)
		return(play(channel, PRT.mh));
	//else delete the message if it can't be played
	CURRMSG = FALSE;
	return (playstate(channel, sDELETEDMSG));
}

int playmsg_cmplt(int channel, int evtcode)
{
	if (USER.nextmsg <= 0) return sGENINTRO;
	_vhclose(PRT.mh);

	if (evtcode == T_TIME || evtcode == T_EOF) 
	{
		if (PRT.currmsg == USER.nextmsg - 1)
			return sLASTMSG;
		return sLISTENMNU;
	}
	if (evtcode == T_MAXDT)
	   return(sGETDIGIT);   
	return(sONHK);
}

int lastmsg(int channel)
{
	return NOTFOUND;
}

int lastmsg_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF) 
	{
	   return(sLISTENMNU);
	}
	if (evtcode == T_MAXDT) 
		return sGETDIGIT;
		
	return(sONHK);
}

int deletedmsg_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF) 
	{
	   CURRMSG = FALSE; 
	   return(sLISTENMNU);
	}
	if (evtcode == T_MAXDT) 
	{
		CURRMSG = FALSE; 
		return sGETDIGIT;
	}
	return(sONHK);
}

int wasdeleted_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sPLAYMSG;
	return sONHK;
}

int forward(int channel) { return NOTFOUND; }
int forward_cmplt(int channel, int evtcode) 
{ 
	if (evtcode == T_EOF || evtcode == T_SIL)
		return sLISTENMNU;
	if (evtcode == T_MAXDT)
		return sGETFWDDIGIT;
	return sONHK; 
}

int checkfwdbox(int channel)
{
	int box;
	char bstr[10];
	sprintf(PRT.dtmf, "%s%s", PRT.firstdigit, PRT.otherdigits);
	box = atoi(PRT.dtmf);
	sprintf(bstr, "%04d", box);
	//check for valid box before continuing
	if (strcmp(bstr, PRT.dtmf) == 0 && 
		useridx[box].avail == ACTIVE)
	{
		if (findnewmsg(bstr) >= 0) //is there any space left on box?
		{
			PRT.newbox = box; //used only for playbox function in this case
			return sPLAYBOX;
		}
		return sFWDFULL;
	}
	return sINVALID;
}

int getfwddigit(int channel)
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int getfwddigit_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		switch (PRT.prev)
		{
			case sFORWARD:
			switch(PRT.firstdigit[0])
			{
				case '#': 
					MGS[PRT.currmsg] = TRUE;
					return sWONTFWD;
				case '*':
					return sLISTENMNU;
				default: return sGETFWDBOX;
			}

			case sCONFFWD:
			switch(PRT.firstdigit[0])
			{
				case '1':
					MGS[PRT.currmsg] = PRT.newbox + FWDOFFS;
					return sCONFWILLFWD;
				case '*':
					return sLISTENMNU;
				case '#': 
					MGS[PRT.currmsg] = TRUE;
					return sWONTFWD;
				default: return sLISTENMNU;
			}
		}
	}
	return sONHK;
}

int getfwdbox(int channel)
{
	return loaddigits(PRT.otherdigits, channel, BOXLEN - 1, KEYWAIT);
}

int getfwdbox_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return checkfwdbox(channel);
	else if (evtcode == T_TIME || evtcode == T_SIL)
			return sINVALID;
	return sONHK;
}

int conffwd(int channel) { return NOTFOUND; }
int conffwd_cmplt(int channel, int evtcode) 
{ 
	if (evtcode == T_EOF || evtcode == T_SIL)
		return sLISTENMNU;
	if (evtcode == T_MAXDT)
		return sGETFWDDIGIT;
	return sONHK; 
}

int willfwd(int channel) { return NOTFOUND; }
int willfwd_cmplt(int channel, int evtcode) 
{ 
	if (evtcode == T_EOF || evtcode == T_SIL || evtcode == T_MAXDT)
	{
		return sPLAYBOX;
	}
	return sONHK; 
}

int wontfwd(int channel) { return NOTFOUND; }
int wontfwd_cmplt(int channel, int evtcode) 
{ 
	if (evtcode == T_EOF || evtcode == T_SIL || evtcode == T_MAXDT)
	{
		switch (PRT.prev)
		{
			case sGETDIGIT:
				return sFORWARD;
			default:
				return sLISTENMNU;
		}
	}
	return sONHK; 
}
	            
int playgrtinfo(int channel)
{
	return NOTFOUND;
}

int playgrtinfo_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	{
		if (PRT.gh > 0)
			return sGREETING;
		return sGENGRT;
	}		
	return sONHK;   
}

int recgrtinfo(int channel)
{
	return NOTFOUND;
}

int recgrtinfo_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sRECGRT;
	if (evtcode == T_MAXDT) 
	{
		return findboxmenu(channel);
	}
	return sONHK;   
}

int recgrt(int channel)
{
	/* get rid of file */
	_unlink(fname(USER.box, GRT));
	if ((PRT.gh = vhopen(fname(USER.box, GRT), CREATE)) > 0)
		return (recordgreeting(channel));
	return (playstate(channel, sGENINSTR));
}

int recgrt_cmplt(int channel, int evtcode)
{                    
	
	long size;
	
	if (PRT.gh > 0)
	{
		size = _tell(PRT.gh);

	    if (evtcode == T_SIL)
	    	removesilence(&size, PRT.gh);
		_vhclose(PRT.gh);

		if (size < MINLEN) 
		{
			_unlink(fname(USER.box, GRT));
			PRT.gh = 0;
		}
	}
	
	//go back to main menu if key is pressed or max time elapsed
	if (evtcode == T_TIME || evtcode == T_MAXDT || evtcode == T_SIL)
		return PRT.menu;
	return (sONHK);
}    

int numboxes(int channel) 
{ 
	register i;
	int maxbox;
	US u;
	PRT.numboxes = 0;
	//find the number of associated msg boxes for an admin box
	if (!ISADMIN(USER.boxtype)) return NOTFOUND;
	_lseek(FUSER, 0L, SEEK_SET);
	maxbox = (int) (filelength(FUSER)/sizeof(US));
	for (i = 0; i < MAXBOX; i++)
	{
		if (useridx[i].avail >= ACTIVE)
		{
			sched();
			_read(FUSER,&u,sizeof(US));
			if (strcmp(u.admin,USER.box)==0) PRT.numboxes++;
		}
	}
	_lseek(FUSER, 0L, SEEK_SET);
	if (PRT.numboxes ) return playint(channel, PRT.numboxes );
	return NOTFOUND;
}

int numboxes_cmplt(int channel, int evtcode)
{
	if (evtcode == T_EOF || evtcode == T_MAXDT || evtcode == T_TIME)
	{
		if (PRT.numboxes == 1) return sBOX;
		return sBOXES;
	}
	return sONHK;
}

int paidtomsg(int channel) { return NOTFOUND; }

void updatepaycoderec( int box, long pos, PAY pay )
{
	time_t now;
	
	sprintf(pay.box, "%04d", box);
	pay.box[BOXLEN] = '\0';
	//record time transaction took place
	time(&now);
	pay.used = now;

	//write the updated record
	_lseek(pfh, pos, SEEK_SET);
	_write(pfh, &pay, sizeof(PAY));
	_commit(pfh);
	_lseek(pfh, 0L, SEEK_SET);
}

int getpaycodeval(int channel, char *pc, long *pos, PAY *rpc)
{
	PAY pay;
	long i, maxrec;

	if (strlen(pc) < PAYCODELEN) return NOTFOUND;
		
	_lseek(pfh, 0L, SEEK_SET);
	maxrec = filelength(pfh)/sizeof(PAY);
	if (maxrec == 0L) return NOTFOUND;

	for (i = 0L; i < maxrec; i++)
	{		
		_read(pfh, &pay, sizeof(PAY));
		if ((_tell(pfh)/sizeof(PAY))%100 == 0) sched();
		//pay code found
		if (strcmp(pc, pay.paycode) == 0) 
		{
			if (pay.used) return NOTFOUND;
			*pos = _tell(pfh) - sizeof(PAY);
			break;
		}
	}
	
	if (i >= maxrec) return NOTFOUND;
	*rpc = pay;
	return pay.value;
}

int getfreecodeval(int channel)
{
	char *rs;
	
	if ((rs = strstr(USER.misc,DAYS)) != NULL)
	{
		return -1 * (min((atoi(rs + strlen(DAYS))), 28));
	}
	if ((rs = strstr(USER.misc, MONTHS)) != NULL)
	{
		return (atoi(rs + strlen(MONTHS)));
	}
	return 0;
}

int getfreecode(int channel,char *pc)
{
	if (strlen(USER.pin) != PINLEN) 
		return NOTFOUND;
	if (strcmp(USER.pin, pc + 1) == 0)
	{
		return getfreecodeval(channel);
	}
	return NOTFOUND;
}

void rmfiles(char *box)
{
	struct _find_t f;
	int err;
	
	err = _dos_findfirst(fname(box,"*.*"), _A_NORMAL, &f);
	while(!err)
	{
		//update disk usage stats
		diskused -= f.size;
		//remove associated file
		_unlink(f.name);
		//check for more files
		err = _dos_findnext(&f);
	}
}

void updatenewbox(int channel, US *u)
{
	int rp;
	char *n;
	register i;
	
	//create random pin
	rp = rand();
	rp %= 10000;
	sprintf(u->pin, "%04d", rp);
	//update box type
	u->boxtype = btMSG;
	//language used should be same as that of new box 
	strcpy(u->language, USER.language); 
	//copy admin box number
	strcpy(u->admin, USER.admin);
	//keep some info on origins of box.
	sprintf(u->misc, "NEW=%s ADMN=%s ", USER.box, USER.admin);
	//get name: name must be preceded by NAME= and be a single word
	//NAME= will wipe the name from the box
	if ((n = strstr(USER.misc,NAME)) != NULL)
	{
		n += strlen(NAME);
		for (i = 0; i < NAMELEN, (n[i] > SPACE && n[i] < 128); i++)
		{ 
			u->name[i] = n[i];
		}
		u->name[i] = '\0';
	}
	//remove reminder
	strcpy(u->remind, NORM);
	//remove out of date payment info
	strcpy(u->paidto, "");
	strcpy(u->ttlpaid, "");
	//remove old message and greeting files
	rmfiles(u->box);
}

void phoneactivate(int box, US *u)
{
	int y, m, d;
	long diff = 0L;
	time_t now, newpdto;
	time_t t = NOTFOUND;
	time_t lastcall = 0L;

	if (u->active == ONHOLD) {
		/* experimental feature: 
		   modify paidto based on difference 
		   between lastcall and paidto date */
		/* get user paidto date */
		y = m = d = 0;
		if (getdate(u->paidto,&d,&m,&y))	{ 
			t = getdatesecs(d,m,y);
		}
		time(&now);
		/* get time of last call (or when box was deactivated) */
		lastcall = u->acctime;
		/* if this number is invalid default to now */
		if (lastcall <= 0L) lastcall = now;
		if (t != NOTFOUND) {
			/* get user last call date
			   subtract the two and */			   
			diff = t - lastcall;
			if (diff > 0L) {
				/* add the difference to now 
			       to get new paidto date */				   
				newpdto = now + diff;
				strcpy(u->paidto, datestr(&newpdto));
			}
		}
	}
	useridx[box].avail = ACTIVE;
	u->active = ACTIVE;
	u->start = time(&now);
	u->acctime = 0L;
	u->msgtime = 0L;
	u->calls = 0;
	useridx[box].type = u->boxtype;
	// update box on screen if applicable
	if (curruser == box) showuser(u);
	//increment number of users
	numusers++; 
}

void phonedeactivate(int box, US *user)
{
	time_t now;
	user->acctime = time(&now);
	useridx[box].avail = INACTIVE;
	user->active = INACTIVE;
	//clear reminder
	if (strcmp(user->remind, NEVER) != 0) 
		strcpy(user->remind, NORM);
	numusers--;
}

int nextavailbox( int start, int end )
{
	US u;
	register i;

#ifdef MAXUSERS
	if (numusers >= MAXUSERS)
		return NOTFOUND;
#endif
	//first check for never before used boxes
	for (i = start; i < MAXBOX, i <= end; i++)
		if (userpos[i] == NOTFOUND) 
		{       
				// hide the box from other people making new boxes
				userpos[i] = ONHOLD;
				useridx[i].avail = ONHOLD;
				return i;
		}
	//then check the used boxes for inactive boxes
	for (i = start; i < MAXBOX, i <= end; i++)
		if (useridx[i].avail == INACTIVE) 
		{
			getuserrec(FUSER, i, &u);
			if (u.boxtype == btMSG) {
				// hide the box from other people making new boxes
		    	useridx[i].avail = ONHOLD;
				return i;
			}
		}
	//then give up
	return NOTFOUND;
}

void updatetime(int channel, US *u, int v)
{
	time_t now, new;
	int rc, d, m, y;
	char newstr[40];
	
	time(&now);
	//check for the current paid to date
	rc = getdate(u->paidto, &d, &m, &y);
	if (!rc || (now > getdatesecs(d,m,y)))
	{
		strcpy(newstr, datestr(&now));
		strcpy(u->paidto, newstr);
		getdate(u->paidto, &d, &m, &y);
	}
	//add the time
	if (v <= 0) 
	{
		switch (v) 
		{
			case 0: break;
			default: d += abs(v);
		}
		//check for invalid day value
		if (d > monlength(m, y))
		{
			d -= monlength(m, y);
			m++;
		}
	}
	else //add months
	{
		m += v;
	}
	//check for invalid month value
	if (m > 12)
	{
		y += m / 12;
		m = m % 12;
		if (!m) {
			m = 12;
			y--;
		}
	}
	
	//create the new time string
	new = getdatesecs(d,m,y);   
	strcpy(newstr, datestr(&new));
	strcpy(u->paidto, newstr);
	PRT.newpaidto = new;
}

int getboxnumber(int channel)
{
	int bxstart, bxend;
	char *rs;
	
	//range defined on misc line of box
	rs = strstr(USER.misc,RANGE);
	if (rs != NULL && strlen(rs) >= strlen(RANGE) + 4)
	{
	    if (sscanf(rs + strlen(RANGE), "%d-%d",&bxstart, &bxend) == 2)
	    {
			if (bxstart > bxend) 
				return nextavailbox(bxend, bxstart);
			else return nextavailbox(bxstart, bxend);
		}
		return NOTFOUND;
	}
	return NOTFOUND;
	// forget about default ranges for the moment
	//return nextavailbox(boxrangestart, boxrangestart + boxrange - 1);
}
	
int checkpaycode(int channel, int box)
{
	long pcpos = (long) NOTFOUND;
	int fv = 0, v, rc;
	PAY pay = {0};
	US u;
	
	//get the paycode value if there is no admin box
	if (PRT.firstdigit[0] != KEYAHEAD)
	{
		//make sure you have a paycode file to check
		if (pfh != NOTFOUND)
		{
			v = getpaycodeval(channel, PRT.dtmf, &pcpos, &pay);
			if (v == NOTFOUND)
				return sPAYERRINTRO;
		}
		else return sPAYERRINTRO;
	}
	//if the user got a new box update the box info
	if (USER.boxtype == btNEW)
	{
		//add free time to paycode value if there is any free time
		fv = getfreecodeval(channel);
		if (pcpos >= 0L) 
		{ 	if (fv > 0) v += fv; 	}
		else v = fv;
		//gets or creates the box record
		getuserrec(FUSER, box, &u); 
		//replace box info specific to setting up a new box
		updatenewbox(channel, &u);  
		//restart and save box
		phoneactivate(box, &u);
		saveuser(FUSER, box, &u);
		if (box == curruser) { user = u; showuser(&user); }
		//reset port to use new box;
		//note: newbox and dtmf are needed to play the box and pin respectively
		recycleport(channel);
		PRT.newbox = PRT.box = box;
		USER = u;
		rc = getbox(channel, PRT.box, BOXLEN);
		strcpy(PRT.dtmf, USER.pin);
	}
	else PRT.newbox = NOTFOUND;
	u = USER;
	// change the "paidto" string and update paycode record (if necessary)
	if (pcpos >= 0L) 
	{
		updatepaycoderec( box, pcpos, pay );
		updatetime(channel, &u, v);
	}
	// if no paycode used only change the paidto string if explicitly required 
	else 
	{   
		//add time based on the "MONTHS=" parameter 
		if (v > 0) updatetime(channel, &u, v);
		//or delete paidto date in case something is already there
		else 
		{
			strcpy(u.paidto, "");
			u.paidto[0] = '\0';
		}
	}
	// don't forget to clear the reminder!
	if (u.remind[0] != _NEVER) u.remind[0] = '\0';
	// or to activate the box if it is not already
	if (useridx[box].avail <= INACTIVE) {
		phoneactivate(box,&u);
		useridx[box].avail = INUSE + channel;
	}
	// save changes
	saveuser(FUSER, box, &u);
	USER = u;
	// update screen if box is visible
	if (curruser == box)
	{
		user = u;
		showuser(&user);
	}
	return sTHANKYOU;
}

int paidtomsg_cmplt(int channel, int evtcode)
{
	//don't let people who have ordinary admin boxes pay by
	//paycode - they aren't compatible accounting methods
	//however for "PAYCODE" admin boxes get the paycode digits
	if (evtcode == T_TIME || evtcode == T_EOF)
	{
	 	return sPAIDTODATE;
	}
	 	
	if (evtcode == T_MAXDT) 
	{
		if (ISADMIN(USER.boxtype) || PRT.superuser)
			return sPAIDTODATE;
		if (validadmin(USER.admin) == TRUE) 
			return PRT.menu;
		return sPAYDIGIT;
	}
	return sONHK;
}

int getpaymsg(int channel){ return NOTFOUND; }

int  getpaymsg_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF)
	{
		return PRT.menu;
	}
	if (evtcode == T_MAXDT) 
	{
		// PAYCODE admin boxes can however use payment codes
		if (validadmin(USER.admin) == TRUE)
			return PRT.menu;
		return sPAYDIGIT;
	}
	return sONHK;
}

int newpaymsg(int channel)
{ 
	//GETMENU 
	return NOTFOUND; 
}

int newpaymsg_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 	return sPAIDTODATE;
	return sONHK;
}

int getpaydigit(int channel)
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int getpaydigit_cmplt(int channel, int evtcode)
{
	static char exitnewinfo[MAXCHAN][BOXLEN + 1] = {0};
	
	if (evtcode == T_MAXDT || evtcode == T_TIME)  
	{                                                  
		if (PRT.prev == sNEWINFO)
		{
	 		if (PRT.firstdigit[0] == KEYBACK)
	 		{
	 			strcat(exitnewinfo[CHAN], PRT.firstdigit);
		 		if (strcmp(exitnewinfo[CHAN], NOBOX) == 0)
		 		{
		 			strcpy(exitnewinfo[CHAN], "");
		 			return sGENINTRO;
		 		}
		 		return sNEWINFO;
	 		}
	 		else strcpy(exitnewinfo[CHAN], "");
	 		
	 		if (strlen(sysphone) >= PAGELEN)
	 			return sPHINTRO;
			return sEXTENSION;
		}

		if (PRT.firstdigit[0] == KEYBACK)
		{   
			if (USER.boxtype == btNEW)
				return sGENINTRO;
			return findboxmenu(channel);
		}
		else if (PRT.firstdigit[0] == KEYAHEAD)
		{
			if (USER.boxtype == btNEW)
				maxpaydigits = PINLEN;
			else return findboxmenu(channel);
		}
		else maxpaydigits = PAYCODELEN - 1;
		return sPAYSTR;
	}
	return sONHK;
}

int getpaystr(int channel)
{
	return loaddigits(PRT.otherdigits, channel, maxpaydigits, PAYWAIT);
}

int getpaystr_cmplt(int channel, int evtcode)
{
	int rc, box;
	if (evtcode == T_MAXDT || evtcode == T_TIME)  
	{                                                  
		if (strcmp(PRT.otherdigits, isskeleton) == 0)
		{
			useridx[PRT.box].avail = INUSE + channel; //don't let others modify box at same time
			return sGETSKELETON; //look for master security code
		}
		//the PRT.dtmf contains either the paycode or #seccode 
		strcpy(PRT.dtmf, PRT.firstdigit);
		strcat(PRT.dtmf, PRT.otherdigits);
		//be strict about payment methods for autoassignment boxes
		//autoassignment boxes with admin boxes are assumed to be 
		//using the #pin type of entry unless the admin box has PAYCODE
		//in the misc field (see validadmin in init.c)
		//anything else uses payment codes
		if (USER.boxtype == btNEW)
		{
			rc = validadmin(USER.admin);
			if (rc == TRUE) //plain admin box
			{
				if (PRT.firstdigit[0] == KEYAHEAD)
				{
					if (strcmp(USER.pin, PRT.otherdigits)) return sPAYERRINTRO;
				}
				else return sPAYERRINTRO;
			}
			else //if rc == PCONLY or rc == FALSE
			{
				if (PRT.firstdigit[0] == KEYAHEAD) return sPAYERRINTRO;
			}		
			box = PRT.newbox;
		}
		else box = PRT.box;
		return checkpaycode(channel, box);
	}
	return sONHK;
}

int paidtodate(int channel)
{
	return playpdtodate(channel, USER);
}

int paidtodate_cmplt(int channel, int evtcode)
{
	switch (PRT.prev)
	{
		case sNEWPAY:
		if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
		{
		 	if (PRT.newbox != NOTFOUND)
		 	{
		 		if (strlen(sysphone) >= PAGELEN)
		 			return sPHINTRO;
		 		return sEXTENSION;
		 	}
		 	return findboxmenu(channel);
		}
		break;
		case sPAIDTO:
		if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
		{
			// note PAYCODE admin boxes will accept payment codes but not 
			// ordinary admin boxes
			if (PRT.superuser || ISADMIN(USER.boxtype) || validadmin(USER.admin) == TRUE)
				return PRT.menu;
			if (evtcode == T_MAXDT) 
				return sPAYDIGIT;
		 	return sGETPAY;
		}
	}
	return sONHK;
}

int payerrintro(int channel)
{
	return NOTFOUND;
}

int payerrintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 	return sPAYERRNUM;
	return sONHK;
}

int payerrnum(int channel)
{
	return playpaycode(channel, PRT.dtmf);
}

int payerrnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 	return sINVALID;
	return sONHK;
}

int nomsgs(int channel)
{
	return NOTFOUND;
}

int nomsgs_cmplt(int channel, int evtcode)
{
	 if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 {
		return findboxmenu(channel);
	 }
	 return(sONHK);
}

int delall(int channel)
{
	return NOTFOUND;
}             

int delall_cmplt(int channel, int evtcode)
{
	 if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 {
	 	return MENU;
	 }
	 return(sONHK);
}

int dateintro(int channel)
{
	return NOTFOUND;
}

int dateintro_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF) 
		return sDATE;
	else if (evtcode == T_MAXDT)
		return sLISTENMNU;
	return sONHK;
}

int date(int channel)
{
	return playmsgdate(channel);
	//return playtest(channel); //for checking the number file only
}

int date_cmplt(int channel, int evtcode)
{
	 if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	 	return sLISTENMNU;
	 return sONHK;
}

//admin functions
int admin(int channel) 
{
	GETMENU
	return NOTFOUND;
}

int admin_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	   return(sGETDIGIT);
	return (sONHK);
}

int admindigits(int channel)
{
	return(loaddigits(PRT.dtmf, channel, MAXDTMF /* characters */ , KEYWAIT /* seconds */ )); 
}

void clearbox(US *u, char *pin)
{
	strcpy(u->remind, NORM);
	strcpy(u->pin, pin);
	strcpy(u->paidto, "");
	rmfiles(u->box);
}

void chrmd(US *u, char *rm)
{
	int cu = atoi(u->box);
	strcpy(u->remind, rm);
	saveuser(FUSER, cu, u);	
	if (cu == curruser)
	{
		strcpy(user.remind,u->remind);
		showuser(&user);
	}
}

void clearadminvar( int channel, US *u, int *dtmf, char *buff )
{
	strcpy(buff, "");
	if ISSYS return;
	*u = blankuser;
	*dtmf = 0;
}

int admindigits_cmplt(int channel, int evtcode)
{
	static char buff[MAXCHAN][BOXLEN + 1] = {0};
	static int dtmf[MAXCHAN] = {0};
	int allboxes = FALSE;
	
	/* check for maximum dtmf event or timeout */
	if (evtcode == T_MAXDT || evtcode == T_TIME) 
	{
	    //check for "escape sequence"
	    if (strcmp(PRT.dtmf, NOBOX) == 0)
	    {
			if (!ISSYS) KEEP = FALSE;
			return PRT.menu;
	    }
	    if (!KEEP)
	    {
			if (strcmp(PRT.dtmf, ALLBOXES) == 0 && 
				(PRT.prev == sSETRMD || PRT.prev == sCLRRMD))
			{
				allboxes = TRUE;
				BOXPROPS = USER;
			}
			else
			{
			  	if (boxnumeric(PRT.dtmf)) 
			  	{
		    		dtmf[CHAN] = atoi(PRT.dtmf); //get box number
		    		getuserrec(FUSER, dtmf[CHAN], BPP); //get (or make) box record
		  	    	//avoid circular references
		  	    	if (!ISSYS && strcmp(USER.box, BOXPROPS.box) == 0)
		  	    		return sINVALID;
			  	} 
			  	else return sINVALID;
		  	}
	    }

		switch (PRT.prev)
		{
			case sBOXPROPS:
				if (strcmp(buff[CHAN], CLEARBOX) == 0)
					return PRT.menu;
				PRT.newbox = atoi(PRT.dtmf);
				sprintf(s, "%04d", PRT.newbox); //check to see if box number is valid
				if (strcmp(s, BOXPROPS.box) != 0)
					return sINVALID;
				if (ISADMIN(USER.boxtype) && strcmp(USER.box, BOXPROPS.admin) != 0)
					return sINVALID;
				if (useridx[PRT.newbox].avail >= INUSE)
					return sINUSE;
				KEEP = TRUE; //tell admindigits not to look for another box
				return sBOXPROPSMENU;
			case sCHGCODE:                          
				// superusers can make any kind of pin but others must
				// use numbers that are greater than 0
				if (pinok(PRT.dtmf) || PRT.superuser)
				{
					strcpy(buff[CHAN], PRT.dtmf);
					return sCONFCHGCODE;
				}
				return sINVALID;
			case sCONFCHGCODE:
				if (!ISSYS) KEEP = FALSE;
				if (strcmp(buff[CHAN], PRT.dtmf) == 0)
				{
					if (PRT.menu == sSYSMENU)
					{
						strcpy(defuser.admin, buff[CHAN]);
						strcpy(skeletonpin, buff[CHAN]);
						savedef();
						clearadminvar(channel, BPP, dtmf + CHAN, buff[CHAN]);
						return sTHANKYOU;
					}
					else if (strcmp(buff[CHAN], CLEARBOX) == 0)
						strcpy(BOXPROPS.pin, "");
					else strcpy(BOXPROPS.pin, buff[CHAN]);
					return sTHANKYOU;
				}
				return sINVALID;
			case sNEWADMIN:
				if (validadmin(PRT.dtmf) || strcmp(PRT.dtmf, CLEARBOX) == 0)
				{
					strcpy(buff[CHAN], PRT.dtmf);
					return sCONFNEWADMIN;
				}
				return sINVALID;
			case sCONFNEWADMIN:
				if (strcmp(buff[CHAN], PRT.dtmf) == 0)
				{
					if (strcmp(buff[CHAN], CLEARBOX) == 0)
						strcpy(BOXPROPS.admin, "");
					else strcpy(BOXPROPS.admin, buff[CHAN]);
					return sTHANKYOU;
				}
				return sINVALID;
		}                        
		
		// out of date do not use
		//saveuser(FUSER, dtmf[CHAN], BPP);
		//if (curruser == dtmf[CHAN])
		//{
		//	strcpy(user.pin, BOXPROPS.pin);
		//	strcpy(user.remind, BOXPROPS.remind);
		//	showuser(&user);
		//}
		
		clearadminvar(channel, BPP, dtmf + CHAN, buff[CHAN]);
		return sTHANKYOU;
	}
}

int adminmsg(int channel)
{
	switch (PRT.curr)
	{
		case sBDMENU:
		case sBOXPROPSMENU:
			GETMENU
		break;
	}
	return NOTFOUND;
}

int adminmsg_cmplt(int channel, int evtcode)
{
	int newstate = NOTFOUND;
	
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT)
	{
		switch(PRT.curr)
		{
	        case sSYSTIMEINTRO:
	        	if (evtcode == T_MAXDT)
	        		return sSYSTIMEDIGIT;
	        	return sPLAYSYSDATE;
	        case sEDSYSTIME:
	        	if (evtcode == T_MAXDT)
	        		return sSYSTIMEDIGIT;
	        	return PRT.menu;
	        case sFORCEBDUPDATE:
	        	countactiveboxes(TRUE);
	        	return sSYSMENU;
	        case sBDUPDATEINTRO:
	        	return sBDUPDATE;
			case sBDMENU:
				if (evtcode == T_MAXDT)
					return sGETDIGIT;
				return sADMINTRO;
			case sYOUOWE:
				return sDOLLARSNUM;
			case sDOLLARS:
				return sAND;
			case sAND:
				return sCENTSNUM;
			case sCENTS:
				return sFOR;
			case sFOR:
				if (PRT.prev == sCENTS)
					return sBDOWED;
				return sNUMBOXES;
			case sBBOXDAYS:
				return sAT;
			case sAT:
				return sPLAYBDVAL;
			case sCENTSEACH:
				if (PRT.menu)
					return sBDNUM;
				if ((newstate = getreminderstate(channel)) > NOTFOUND) 
					return newstate;
				return sREMIND;
			case sNBOXDAYS:
				return sFOR;
			case sBBDCLEAR:
			case sBBDREVERT:
			case sBDCLEAR:
			case sBDRESTORE:
				return PRT.menu;
			case sGETBDVAL:
				if (evtcode == T_MAXDT)
					return sGETBDVALDIGIT;
				return PRT.menu;
			case sBBDEDIT:
			case sBBDSIL:
			case sBDEDIT:
			case sBDSIL:
				if (evtcode == T_MAXDT)
					return sBDDIGIT;
				return PRT.menu;
			case sEDPDTO:
				if (evtcode == T_MAXDT)
					return sPDTODIGIT;
				return PRT.menu;
			case sBOX:
			case sBOXES:
				return sBDUPDATEINTRO;
			case sOLDPROPS:     //sysmenu state
				return sBOXPROPSMENU;
			case sBOXPROPSMENU: //sysmenu state
				if (evtcode == T_MAXDT)
				{
					return sGETDIGIT;
				}
				if ISADMIN(USER.boxtype)
					return sADMIN;
				return sSYSMENU;
			case sNOFEAT:       //feature not available
				return PRT.menu;
			case sPLAYBOXPROPS: //sysmenu state
				return sPLAYBOX;
			case sBOXPROPS: 	//sysmenu state
			case sNEWADMIN: 	//sysmenu state
			case sCONFNEWADMIN: //sysmenu state
			case sSETRMD:
			case sCLRRMD:
			case sCHECKDEL:
			case sDELBOX:
			case sBOXCODE:
			case sCHGCODE:
			case sCONFCHGCODE:
				return sADMINDIGITS;
			case sPLAYRMD:
				PRT.rh = vhopen(fname(USER.box, TMP), READ);
				if (PRT.rh) 
				{
					return sNEWRMD;
				}
				else 
				{
					checkrmd(channel, USER.box);
					if (ADMH)
						return sADMRMD;
					return sREMIND;
				}
			case sRECRMDMSG:
				return sRECRMD;
			case sTOGBRWS:
			case sRMALLMNU:
				if (evtcode == T_MAXDT) return sGETDIGIT;
				return sADMIN;
			case sBRWSON:
			case sBRWSOFF:
			case sRMALLON:
			case sRMALLOFF:
				if (evtcode == T_MAXDT) return sGETDIGIT;
				return PRT.menu;
		}
	}
	return(sONHK);
}

int bdnum(int channel)
{
	long bd = 0L;
	switch (PRT.curr)
	{
		case sDOLLARSNUM: bd = (USER.bdowed * BDV) / 100L; break;
		case sCENTSNUM: bd = (USER.bdowed * BDV) % 100L; break;
		case sBDOWED: bd = USER.bdowed; break;
		case sBDNUM: bd = USER.boxdays; break;
		case sPLAYBDVAL: bd = BDV; break;
	}
	if (bd == 0L) return NOTFOUND;
	return playlong(channel, bd);
}

int bdnum_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		switch (PRT.curr)
		{
			case sDOLLARSNUM: return sDOLLARS;
			case sCENTSNUM: return sCENTS;
			case sBDOWED: return sBBOXDAYS;
			case sBDNUM: return sNBOXDAYS;
			case sPLAYBDVAL: return sCENTSEACH;
		}
	}
	return sONHK;
}

int bdupdate(int channel)
{
	time_t ud;
	ud = USER.bdupdate;
	if (ud > 0L)
		return playtime_t(channel, &ud);
	return NOTFOUND;
}

int bdupdate_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sPAIDTO;
	return sONHK;
}

int getbdval(int channel)
{
	return loaddigits(PRT.otherdigits, channel, 1, KEYWAIT);
}

int getbdval_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_SIL) 
	{
		//create bdval string
		sprintf(PRT.dtmf,"%s%s", PRT.firstdigit, PRT.otherdigits);
		//check to see if user wants to clear bdval field
		if (strcmp(PRT.dtmf, "##") == 0) 
		{
			USER.bdval = 0;
			return sTHANKYOU;
		}
		USER.bdval = atoi(PRT.dtmf);
		return sTHANKYOU;
	}
	return sONHK;
}

int bddigit(int channel)
{
	//clear boxday string
	if (PRT.prev == sBDEDIT || PRT.prev == sBBDEDIT)
	{
		strcpy(PRT.dtmf, "");
	}
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int bddigit_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		switch (PRT.firstdigit[0])
		{
			case '#':
				switch (PRT.prev)
				{
					case sBDSIL:
					case sBDEDIT:
						USER.boxdays = atol(PRT.dtmf);
						if (USER.boxdays)
							return sTHANKYOU;
						return sBDCLEAR;
					case sBBDSIL:
					case sBBDEDIT:
						USER.bdowed = atol(PRT.dtmf);
						if (USER.bdowed)
							return sTHANKYOU;
						return sBBDCLEAR;
				}
			break;
			case '*':
				return PRT.menu;
			default: 
				sprintf(s, "%s%s", PRT.dtmf, PRT.firstdigit);
				strcpy(PRT.dtmf, s);
				if (strlen(PRT.dtmf) > NUMLEN) 
					return sINVALID;
				switch (PRT.prev)
				{
					case sBDSIL:
					case sBDEDIT:
						return sBDSIL;
					case sBBDSIL:
					case sBBDEDIT:
						return sBBDSIL;
				}
		}
	}
	return sONHK;
}

int pdtodigit(int channel)
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int pdtodigit_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		switch (PRT.firstdigit[0])
		{
			case '*': return PRT.menu;
			default : 
				switch(PRT.prev)
				{
					case sEDPDTO:
						return sPDTOSTR;
					case sGETBDVAL:
						return sGETBDVALSTR;
				}
		}
	}
	else if (evtcode == T_SIL) return PRT.menu;
	return sONHK;
}

int pdtostr(int channel)
{
	return loaddigits(PRT.otherdigits, channel, EDPDTOLEN - 1, KEYWAIT);
}

int pdtostr_cmplt(int channel, int evtcode)
{
	int day, month, year;
	
	if (evtcode == T_MAXDT || evtcode == T_SIL) 
	{
		//create date string
		sprintf(PRT.dtmf,"%s%s", PRT.firstdigit, PRT.otherdigits);
		if (strlen(PRT.dtmf) < EDPDTOLEN) return sINVALID;
		//check to see if user wants to clear paidto field
		if (strcmp(PRT.dtmf, "########") == 0) 
		{
			if (validbox(BOXPROPS.box))
				strcpy(BOXPROPS.paidto, "");
			else strcpy(USER.paidto, "");
			return sTHANKYOU;
		}
		//load temporary values
		if (sscanf(PRT.dtmf, "%2d%2d%4d", &day, &month, &year) < 3)
			return sINVALID;
		//check validity of date fragments
		if (year < 2000 || year > 2059) return sINVALID;
		if (month < 1 || month > 12) return sINVALID;
		if (day < 1 || day > (year % 4 ? lmonth[month - 1] : llymonth[month - 1]))
			return sINVALID;
		//load paid to string
		if (validbox(BOXPROPS.box))
			sprintf(BOXPROPS.paidto, "%02d-%s-%02d", day, months[month - 1], year - 2000);
		else sprintf(USER.paidto, "%02d-%s-%02d", day, months[month - 1], year - 2000);
		return sTHANKYOU;
	}
	return sONHK;
}

int newrmd(int channel)
{
	return (play(channel, PRT.rh));
}

int newrmd_cmplt(int channel, int evtcode)
{
	if (PRT.rh && PRT.rh != ADMH) 
		_vhclose(PRT.rh);
	else if (ADMH) 
		closermd(channel);
	//go back to main menu if key is pressed or max time elapsed
	if (evtcode == T_TIME || evtcode == T_MAXDT || evtcode == T_EOF)
		return (sADMIN); 
	//otherwise hang up
	return (sONHK);
}

int recrmd(int channel)
{
	_unlink(fname(USER.box, TMP));
	if ((PRT.rh = vhopen(fname(USER.box, TMP), CREATE)) > 0)
		return (record(channel, PRT.rh));
	return NOTFOUND;
}

int recrmd_cmplt(int channel, int evtcode)
{                    
	long size;
	
	if (PRT.rh > 0)
	{
		size = _tell(PRT.rh);

	    if (evtcode == T_SIL) 
	    	removesilence(&size, PRT.rh);
		_vhclose(PRT.rh);

		if (size < MINLEN) 
		{
			_unlink(fname(USER.box, TMP));
			PRT.rh = 0;
		}
		else
		{
			if (!ADMH) renrmd(channel, USER.box); //rename if reminder not in use
		}	
	}
	//go back to main menu if key is pressed or max time elapsed
	if (evtcode == T_TIME || evtcode == T_MAXDT || 
	    evtcode == T_TERMDT || evtcode == T_SIL)
		return (sADMIN); 
	//otherwise hang up
	return (sONHK);
}    

//browse menu
int browse(int channel)
{
	return NOTFOUND;
}

int browse_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGIT;
	return sONHK;
}

//sys functions
int syscode(int channel)
{
	return NOTFOUND;
}

int syscode_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sGETDIGITS;
	return sONHK;
}

int sysmenu(int channel)		
{
	GETMENU
	return NOTFOUND;
}

int sysmenu_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGIT;
	return sONHK;
}

int act(int channel)
{
	return NOTFOUND;
}

int act_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sGETDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sSYSMENU;
	return sONHK;
}

int bup(int channel)		
{
	return NOTFOUND;
}

int bup_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
		return sSYSMENU;
	return sONHK;
}

int recannc(int channel)		
{
	return NOTFOUND;
}

int recannc_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sSYSMENU;
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sNEWANNC;
	return sONHK;
}

int newannc(int channel)
{
	_unlink(ANNOUNCETEMP);
	if ((PRT.rh = vhopen(ANNOUNCETEMP, CREATE)) != 0)
		return record(channel, PRT.rh);
	return playstate(channel, sNOANNC);
}

int newannc_cmplt(int channel, int evtcode)
{
	long size;
	
	if (PRT.rh)
	{
		size = _tell(PRT.rh);

	    if (evtcode == T_SIL) 
	    	removesilence(&size, PRT.rh);

		_vhclose(PRT.rh);

		if (size < MINLEN) 
		{
			_unlink(ANNOUNCETEMP);
			PRT.rh = 0;
		}
		else 
		{
			announce = FALSE;
		}
	}

	//go back to main menu if key is pressed or max time elapsed
	if (evtcode == T_TIME || evtcode == T_MAXDT || 
	    evtcode == T_TERMDT || evtcode == T_SIL)
	{
		if (PRT.rh)
			return sACTANNC;
		return sNOANNC;
	}
		
	//otherwise hang up
	return (sONHK);
}

int hearannc(int channel)			
{
	return NOTFOUND;
}

int hearannc_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
		return sSYSMENU;
	if (evtcode == T_TIME || evtcode == T_EOF)
	{
		if (fexist(ANNOUNCETEMP)) return sPLAYNEWANNC;
		if (fexist(ANNOUNCEMENT)) return sANNOUNCE;
		return sNOANNC;
	}
	return sONHK;
}

int playnewannc(int channel)
{
	if ((PRT.rh = vhopen(ANNOUNCETEMP, READ)) == 0)
		return playstate(channel, sNOANNC);
	return play(channel, PRT.rh);
}

int playnewannc_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_EOF || evtcode == T_TIME)
	{
		if (!announce) return sANNCOFF;
		return sANNCON;
	}
	return sONHK;
}

int togprop(int channel)
{
	return NOTFOUND;
}

int togprop_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT) return sGETDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF) 
		return sBOXPROPSMENU;
	return sONHK;
}

int type(int channel)
{
	return NOTFOUND;
}

int type_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT) 
	{
		if (PRT.prev == sGETDIGIT) {
			return PRT.menu;
		}
		if (strcmp(BOXPROPS.pin, "") == 0)
			return sNO;
		strcpy(PRT.dtmf, BOXPROPS.pin);
		return sPLAYPININTRO;
	}
	return sONHK;
}

int rmd(int channel)
{
	return NOTFOUND;
}

int rmd_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT) 
	{
		if (PRT.prev == sGETDIGIT)
		{
			if ISADMIN(USER.boxtype) return PRT.menu;
			if (evtcode == T_MAXDT) 
			{
				PRT.oldstate = sTOGRMD;
				return sGETDIGIT;
			}
			return sTOGRMD;
		}
		return sBOXSTAT;
	}
	return sONHK;
}

int actresult(int channel)
{
	return NOTFOUND;
}

int actresult_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME || evtcode == T_EOF)
	{
		if (PRT.menu == sBOXPROPSMENU && PRT.sequence)
			return sMSGTYPE + BOXPROPS.boxtype;
		return sBOXPROPSMENU;
	}
	return sONHK;
}

int no(int channel)
{
	return NOTFOUND;
}

int no_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT) 
	{
		switch (PRT.prev)
		{
			case sPLAYPIN:
			case sPLAYPININTRO: 
				return sPLAYADMN;
			default: return sPLAYPININTRO;
		}
	}
	return sONHK;
}

int playadmn(int channel)
{
	return NOTFOUND;
}

int rmord(char *rmd)
{
	char *rp, ro[sizeof(RMORD)] = RMORD;
	if (strlen(rmd) == 0) 
		return (int) strlen(ro);
	rp = strstr(ro, rmd);
	if (rp == NULL) 
		return NOTFOUND;
	return (int) (rp - ro);
}

void incrm(int channel, char *rmd)
{
	int newrm, maxrm = sizeof(RMORD) - 2;
	
	if ISSYS
	{
		if ((newrm = rmord(rmd)) == maxrm)
		{
			if (BOXPROPS.boxtype == btADMN || 
			    BOXPROPS.boxtype == btPAYC) 
				newrm = 0;
			else newrm = 1;
		}
		else newrm++;
		rmd[0] = RMORD[newrm];
	}
	else
	{
		rmd[0] = _RM;
	}
	rmd[1] = 0;
}

int playadmn_cmplt(int channel, int evtcode)
{
	if (evtcode == T_TIME || evtcode == T_EOF || evtcode == T_MAXDT) 
	{
		if (PRT.prev == sNO)
			return sRMALL + rmord(BOXPROPS.remind);
		PRT.newbox = atoi(BOXPROPS.admin);
		return sPLAYBOX;
	}
}

int num(int channel) 
{
	int n = 0;
	long upc, unupc, pcttl;
	// note that the following switch statement
	// gets the number of the information for the next state
	// not the state in the "case"
	switch (PRT.prev)
	{
		case sGETBOXDIGITS:
			n = numusers;
		break;
		case sUSERS:
			n = sumhrarray(chansfull);
			n /= 60;
		break;
		case sMINBUSY:
			n = maxhrarray(chanwait);
		break;
		case sMAXWAIT:
			n = sumhrarray(cperh);
		break;
		case sCALLS:
			n = sumhrarray(mperh);
		break;
		case sMSGS:
			n = sumhrarray(pgcalls);
		break;
		case sPGCALLS:
			if ((pcttl = countpaycodes(&upc, &unupc)) > 0) {
			   n = (int)((upc*100L)/pcttl);
			} else n = 0;
		break;
	}
	if (n) return playint(channel, n);
	return NOTFOUND;
}

int num_cmplt(int channel, int evtcode) 
{
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		switch(PRT.prev)
		{
		case sGETBOXDIGITS: return sUSERS;
		case sUSERS: return sMINBUSY;
		case sMINBUSY: return sMAXWAIT;
		case sMAXWAIT: return sCALLS;
		case sCALLS: return sMSGS;
		case sMSGS: return sPGCALLS;
		case sPGCALLS: return sPPCUSED;
		default: return sSYSCODE;
		}
	}
	if (evtcode == T_MAXDT)
	{
		return sGETDIGITS;
	}
	return sONHK;
}

int statmsg(int channel) 
{ 
	return NOTFOUND; 
}

int statmsg_cmplt(int channel, int evtcode) 
{
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		if (PRT.curr == sSYSCODE - 1) return sSYSCODE;
		return sNUM;
	}
	if (evtcode == T_MAXDT)
	{
		return sGETDIGITS;
	}
	return sONHK;
}

int boxstat(int channel)
{
	time_t lc;
	
	switch (PRT.prev)
	{
		case sBOXCALLS:
			if(BOXPROPS.nextmsg == 0) return NOTFOUND;
			return playint(channel, BOXPROPS.nextmsg);
		case sBOXLASTCALL:
			lc = LASTCALL(BOXPROPS);
			if(lc == 0L) return NOTFOUND;
			return playtime_t(channel, &lc);
		case sBOXPAIDTO:
			return playpdtodate(channel, BOXPROPS);
		default: //initial state is one of the reminders
			return playlong(channel, BOXPROPS.calls);
	}
}

int boxstat_cmplt(int channel, int evtcode)
{
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		if (PRT.curr == sOLDPROPS - 1) return sBOXPROPSMENU;
		switch (PRT.prev)
		{
			case sBOXPAIDTO:
				switch (BOXPROPS.active)
				{
					case ACTIVE: return sISACTV; break;
					case INACTIVE: return sNOTACTV; break;
					case ONHOLD: return sONHOLD; break;
					default: return sISACTV;
				}	
				break;
			case sBOXCALLS:
				return sBOXMSGS;
			case sBOXLASTCALL:
				return sBOXPROPSMENU;
			default: //initial state is one of the reminders
				return sBOXCALLS;
		}
	}
	if (evtcode == T_MAXDT)
	{
		return sBOXPROPSMENU;
	}
	return sONHK;
}

int boxstatmsg(int channel) 
{ 
	return NOTFOUND; 
}

int boxstatmsg_cmplt(int channel, int evtcode) 
{
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		if (PRT.curr == sSYSCODE - 1) return sBOXPROPSMENU;
		if (PRT.curr == sBOXMSGS) return sBOXLASTCALL;
		return sBOXSTAT;
	}
	if (evtcode == T_MAXDT)
	{
		return sGETDIGITS;
	}
	return sONHK;
}

int systime(int channel) 
{
	struct _dostime_t tnow = {0};
	struct _dosdate_t dnow = {0};
	
	//get system time
	switch (PRT.curr)
	{
		case sPLAYSYSTIME:
			_dos_gettime(&tnow);
			return playtime(channel, tnow.hour, tnow.minute);
		case sPLAYSYSDATE:
			_dos_getdate(&dnow);
			return playdate(channel, dnow.day, dnow.month, dnow.year);
	}
}

int systime_cmplt(int channel, int evtcode) 
{
	if (evtcode == T_MAXDT) 
		return sSYSTIMEDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)
	{
		if (PRT.curr == sPLAYSYSTIME)
			return sEDSYSTIME;
		return sPLAYSYSTIME;
	}
	return sONHK;
}

int systimedigit(int channel) 
{
	return loaddigits(PRT.firstdigit, channel, 1, KEYWAIT);
}

int systimedigit_cmplt(int channel, int evtcode) 
{
	if (evtcode == T_MAXDT)
	{
		switch (PRT.firstdigit[0])
		{
			case KEYBACK:
			case KEYAHEAD:
				return PRT.menu;
			default:
				return sSYSTIMESTR;
		}
	}
	return sONHK;
}

int systimestr(int channel) 	
{
	return loaddigits(PRT.otherdigits, channel, 3, KEYWAIT);
}

int systimestr_cmplt(int channel, int evtcode) 
{
	struct _dostime_t now;
	int h, m;
	//if a valid time was entered then change system time
	if (evtcode == T_MAXDT || evtcode == T_SIL)
	{
		_dos_gettime(&now);
		sprintf(PRT.dtmf,"%s%s", PRT.firstdigit, PRT.otherdigits);
		if (strlen(PRT.dtmf) < 4) return sINVALID;
		sscanf(PRT.dtmf,"%2d%2d", &h, &m);
		if (h < 0 || h > 23 || m < 0 || m > 59) return sINVALID;
		now.hour = h;
		now.minute = m;
		if (!_dos_settime(&now)) return sTHANKYOU;
		return sSYSTIMEERR;
	}
	return sONHK;
}

/*
int newsyscode(int channel)
{
	return NOTFOUND;
}

int newsyscode_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		if (PRT.curr == sGETDEST)
			return sDESTROY;
		return sGETSYSCODE;
	}
	if (evtcode == T_EOF || evtcode == T_TIME)
	{
		if (PRT.curr == sGETDEST)
			return sEXITMENU;
		return sSYSMENU;
	}
	return sONHK;
}

int getsyscode(int channel)
{
	return loaddigits(PRT.dtmf, channel, MAXDTMF, KEYWAIT);
}

int getsyscode_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT)
	{
		strcpy(defuser.admin, PRT.dtmf);
		strcpy(skeletonpin, PRT.dtmf);
		savedef();
		return sTHANKYOU;
	}
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sINVALID;
	
	return sONHK;
}

int getdestroy(int channel)
{
	return loaddigits(PRT.dtmf, channel, DESTLEN, KEYWAIT);
}

int getdestroy_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT || evtcode == T_TIME)
	{
		if (strcmp(PRT.dtmf, DESTCODE) == 0)
		{
			sysaction = sysDESTROY;
			return sCONTINUE;
		}
		return sEXITMENU;
	}
	return sONHK;
}

int _continue(int channel)
{
	return NOTFOUND;
}

int _continue_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT) 
		return sGETDIGIT;
	if (evtcode == T_TIME || evtcode == T_EOF)
		return sEXITMENU;
	return sONHK;
}
*/

//exit states
int reset(int channel)
{
	return offhk(channel);
}

int reset_cmplt(int channel, int evtcode)
{
	return sONHK;
}
 
int hangup(int channel)
{
	//play hang up message
	return NOTFOUND;
}

int hangup_cmplt(int channel, int evtcode)
{
	if (evtcode == T_MAXDT && PRT.prev == sRECORDMSG)
		return sFIRSTDIGIT;
	//hang up
	return sONHK;
}
/****************************************************************
*        NAME : onhk(channel)
* DESCRIPTION : put line on hook.
*       INPUT : channel = channel number
*      OUTPUT : set D/4x channel on hook.
*     RETURNS : error code from sethook().
*    CAUTIONS : multi-tasking process.
****************************************************************/
int onhk(channel)
   int channel;
{
	// returning the hook state to the state that it is already in 
	// is a bit crazy but if you just return 0 the channel hangs
	// waiting for an event that will never happen
	if (TELCO == tsDIRECT) {
		return set_hkstate(channel,H_OFFH);
	}
	// state for a normal call
	return (set_hkstate(channel,H_ONH));
}
/****************************************************************
*        NAME : onhk_cmplt(channel,evtcode)
* DESCRIPTION : check results of event after going on hook.
*       INPUT : channel = channel number
*             : evtcode = event code
*     RETURNS : next state.
*    CAUTIONS : none.
****************************************************************/
int onhk_cmplt(int channel,int evtcode)
{
	if (PRT.prev == sRESET) {
		if (TELCO == tsDIRECT) return sWTKEY;
		return sWTRING;
	}
	
	iteration[CHAN] = 0;
	
	// permit only one paging call 
	// on system at one time
	if (allowpage == channel)
		allowpage = NOTFOUND;
	//reset port values	
	if (!ISSYS && validbox(USER.box))
		recycleport(channel);
	else voidport(channel);
	
	PRT.language = deflanguage;
	// if you have a direct connection chansinuse wasn't changed anyway
	// you just want to go back to your idle state
	if (TELCO == tsDIRECT) return sWTKEY;
	// otherwise indicate that another channel is now available for calls
	if (TELCO) 
	{
	    chansinuse--; //if telco channel then reduce chansinuse
	}
	// if you got an error try again - being optimistic that
	// you won't wind up going on indefinitely
	if (evtcode == T_DOSERR)
		return sONHK;
	//otherwise wait for something to happen
	return sWTRING;
}


/* end ansr3.c */





