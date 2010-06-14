
/* ansrdefs.h - part of ansr3 program - see also defs.h, ansrscrn.h and util.h */


/**
 **   Definitions
 **/

//definitions relating to function
// #define DAEMON //only log info, no internal backup, AltX to exit

// Site defines - determines which set of site specific parameters to use
// Only one of the following should be defined at one time
// #define COOLAID // coolaid specific stuff
#define DERA //dera specific stuff
// end site defines

//definitions relating to version
#define DIALOGIC
#define VERNUM "3.45"
#define RCSID	"$Id: ANSRDEFS.H,v 1.41 2007/03/08 19:10:22 cwoodruf Exp cwoodruf $"

#ifdef DAEMON
#define VER VERNUM "D"
#else
#define VER VERNUM
#endif

#define ENDSTATE sGENINTRO

#ifdef COOLAID
#define DEFPIN      "2665"
#define RELEASE     "COOLAID"
#define SYSPHONE    "2503835144" //for numeric pagers
#endif
// note: if they are both defined this will give an error
#ifdef DERA
#define DEFPIN      "3372"
#define RELEASE     "DERA"
#define SYSPHONE    "6046823269" //for numeric pagers
#endif

#define SKELETONPIN "1234"
#define OVERRIDE    "1962" //to override system code    

#ifdef   TRIAL              //one line trial version
#ifdef   TG                 //Never miss a call demo system
#define  TITLEMSG " NEVER MISS A CALL (Lifeline v " VER ")"
#define  MAXCHAN       2    //allocate memory for this many channels 
#define  MAXCHANSTR    "2"  // maximum number of channels available
#else
#undef   RELEASE
#define  RELEASE        "TRIAL VERSION"
#define  MAXCHAN       1    //allocate memory for this many channels 
#define  MAXCHANSTR    "1"  // maximum number of channels available
#define  MAXUSERS  50 //max # boxes on system
#endif
#else
#define  MAXCHAN       12    //allocate memory for this many channels 
#define  MAXCHANSTR    "12"  // maximum number of channels available
#endif

#ifndef TITLEMSG
#define  TITLEMSG " LIFE LINE " VER " VOICE MAIL SYSTEM.(" RELEASE ")"
#endif

#define  EDITCHAN MAXCHAN //use last "channel" for editing functions

/* general program constants */
#define  EVENT        0                /* means event occured instead of keystroke */
#define  MINCHAN      1                //minimum # of channels you can select
#define  CHAN         (channel - 1)    /* so you can fit 1..4 channels in 0..3 array indices */
#define  PRT          port[CHAN]       /* data structure for the channel information */
#define  CHMSG        chmsg[CHAN]       //line status message pointer
#define  USER         prtuser[CHAN]    /* points to customer record held by port data struct*/
#define  MGS          msgs[CHAN]       //pointer to current message
#define  CURRMSG      MGS[PRT.currmsg]
#define  NEWMSG       MGS[USER.nextmsg]
#define  SETMENU(m)   {PRT.menu = (m); PRT.sequence = 0;} // */ sprintf(s, "menu = %s", states[PRT.menu].desc); writemsg(s);}
#define  GETMENU      {PRT.menu = PRT.curr; PRT.sequence = 0;} // */ sprintf(s, "menu = %s", states[PRT.menu].desc); writemsg(s);}
#define  MENU         PRT.menu
#define  TELCO        telcochans[CHAN]
#define  DID        (TELCO == tsDID || TELCO == tsDIDWINK)
#define  DIRECT     (TELCO == tsDIRECT)
#define  ISADMIN(bt)  (bt == btADMN || bt == btPAYC)
#define  ANNOUNCEBOX (atoi(USER.pin) == 0)
#define  ADMH         useridx[PRT.menu == sADMIN ? atoi(USER.box) : atoi(USER.admin)].admh
#define  USERPOS(p)  ((long)((long) userpos[(p)] * sizeof(US)))
#define  AUTORMD     ((kbaction == kbBACKUP || chkboxes) && (bupremind || bupdeactivate)) //remind and deactivate must be > 0
#define  RMORD      "CSAXY" //order of reminder states NORM is represented by \0 at end of string
#define  RM         "Y" //play reminder before main menu
#define  SR         "S" //system reminder (if time limit is put on ad them reminder defaults to system reminder)
#define  AD         "A" //play reminder after greeting
#define  NEVER      "X" //never play reminder
#define  NORM       ""  //don't play reminder
#define  RMALL      "C" //use adminbox reminder as an announcement
#define  _RM        'Y' //play reminder before main menu
#define  _SR        'S' //system reminder (if time limit is put on ad them reminder defaults to system reminder)
#define  _AD        'A' //play reminder after greeting
#define  _NEVER     'X' //never play reminder
#define  _NORM      ''  //don't play reminder
#define  _RMALL     'C' //use adminbox reminder as an announcement (ie heard by all associated message boxes)
#define MAXBDVAL    127L //max number of cents per day
#define BDV         (USER.bdval?(long) USER.bdval: bdval) // figure out the boxday value for that user
#define BDVAL(u)    ((u).bdval ? (u).bdval : ((char)bdval))
#define BDVALSTR    "NEW BOXDAY VALUE: "
#define  ALLBOXES   "####"    //indicates that adminbox reminder is to be played as an announcement
#define  NOBOX      "****"    //go back to previous menu without changing
#define  CLEARBOX   "####"    //clear entry
#define  KEEP       keep[CHAN]//for admindigits_cmplt: keep user record
#define  BOXPROPS   boxprops[CHAN] //for changing box properties from sys or admin box
#define  LASTCALL(u)   ((u).msgtime > (u).acctime ? (u).msgtime : (u).acctime)
#define  BPP         (boxprops + CHAN) //pointer to boxprops element
#define  MONTHS      "MONTHS=" //btNEW - how many months free offer gives
#define  DAYS        "DAYS="   //btNEW - how many days free offer gives
#define  RANGE       "RANGE="  //btNEW - box range for box assignment
#define  NAME        "NAME="   //btNEW - use to paste name in name field
#define  BROWSE      "BROWSE"  //btADMN - permit browsing of message boxes
//using new box type btPAYC instead
//#define  PAYCODEONLY "PAYCODE" //btADMN - don't allow changes to paid to date by administrators
//#define  ISPCONLY(UM)    (strstr((UM), PAYCODEONLY) != NULL)
#define  PCONLY  2         //return code for validadmin in init.c
#define  NEXT    1         //direction to go in for browse function
#define  PREV   -1         //direction to go in for browse function
#define  DATASTOP      (unsigned long) -1 //indicates end of data array for indexed play
#define  CHECKADMIN(user)    (strictadmin && user.boxtype == btMSG && strlen(user.paidto) == 0 && (strlen(user.admin) == 0 || validadmin(user.admin) != TRUE))
#define  FWDOFFS       10 //offset for number being forwarded in msgs array (to allow T,F and other states for msgs array)
#define  ISTELCO       (telcochans[CHAN] != tsAVAIL && telcochans[CHAN] != tsNOTAVAIL)
#define  MAXPAGE       10  //maximum number of times to attempt page 
#define  PAGEIND       '#' //used in phone field to indicate that this is paging number
#define  PGNUMERIC     '*' //indicate that page is going to numeric pager
#define  PAGING        " PAGING"    
#define  PAGELEN       10   //needed length for paging phone number to be valid
// check the page number for length and a valid start character (*#) phone number should not have a 1 in front
#define  VALIDPHONE(p) (validphone((p)))
#define  MAXITERATION (!PRT.superuser && (maxiteration > 0 && iteration[CHAN]++ >= maxiteration - 1)) //how many times has a box been selected?
#define  MAXSECCODEITER (maxiteration > 0 && seccodeiter[CHAN]++ >= maxiteration - 1) //how many times has a box been selected?
#define  GOBACK       (PRT.dtmf[0] == '*') //test for return to start
#define  KEYAHEAD   '#' //key to skip to record a message
#define  KEYBACK    '*' //key to skip back to start
#define  BTNEXT(u,m)  if ((u)->boxtype < (m)) (u)->boxtype++; else (u)->boxtype = 0;

#define  ERR          2     //used when checking values in cmpdate in ansrscrn.c
#define  BROWSEJUMP   10    //number used when jumping ahead using browse feature
#define  BXRNGSTART   1000  //starting range for autoassignment
#define  BXRNG        (MAXBOX - BXRNGSTART)  //range length for autoassignment
#define  DEFINTR      5     /* default d4x interupt */
#define  KEYWAIT     10     //how many seconds to wait for a key stroke
#define  PAYWAIT     30     //how long to wait for the paycode
#define  MAXTIME     90L    /* max msg length in seconds */
#define  MINTIME      5L    /* minimum msg length in seconds */
#define  DEFTIME     60L    //default message length in secs
#define  MAXHR       24     //used for stats functions in util.c
#define  MAXSIL      10L    /* max time to wait before terminating message */   
#define  MBSECS     349L     //approximate number of seconds per MB 
#define  SMPLRATE  3000L    /* bytes per second of message (6000 * 1/2 byte) */
#define  SILENCE   ((MAXSIL - 1) * SMPLRATE) //approx length of recorded silence
//note: system is inaccurate up to 1 second
#define  MINLEN    5000L //byte threshold for keeping message
#define  MAXMSG      99L    /* maximum # of messages per box */
#define  MAXFILES    (MAXMSG + 2L)    //total maximum files per box (includes greeting and reminder)
#define  MINMSG       5L    /* minimum # of messages per box */
#define  DEFMSG      10L    //default # msgs
#define  MAXBOX    10000    /* maximum number of boxes */
#define  MAXSTRING  3072    //4k buffer for logging user activation changes
#define  MAXDTMF       4    /* max # keys you can input */
#define  DEFRING       1    /* default: pick up after # rings */
#define  MAXRING      10    /* max number of rings before picking up      */
//for DID variable 'didtype' - determines sequence of states
#define  INSTANT       0   //digits sent after first ring
#define  WINK          1   //digits sent when application sends wink to CO

#define  BXRNGSTR      ""      //range length for autoassignment
#define  DEFINTRSTR    "5"
#define  MAXTIMESTR    "90"    /* max msg length in seconds */
#define  DEFTIMESTR    "60"    /* default msg length */
#define  MAXMSGSTR     "30"    /* maximum # of messages per box */
#define  DEFMSGSTR     "10"    // default number of messages
#define  DEFRINGSTR    "1"    /* default: pick up after # rings */
#define  MAXRINGSTR    "10"    /* max number of rings before picking up      */

//sys box constants
#define  BUFFSIZE 21 //MUST BE BIGGER THAN BIGGEST ENTRY (PAYCODELEN = 16)
#define  SYSBOX "0***"
#define  ISSYS (sysinuse == channel) 
//defuser constants (see savedef and loaddef in ansrscrn.c
#define  ANNCSTATE defuser.boxtype
#define  SERIALNUMBER defuser.name

#define  FALSE         0
#define  TRUE          1
#define  NEW           2

//for help menu selections 
#define  HILITE        TRUE
#define  NOHILITE      FALSE
//for showing telco lines (util.c telcoscrn() etc)
#define  CELMOD     3   //number of lines across
#define  CELOFFS    5   //offset for showing state of channel
#define  CELHILITE  bgWHITE + fgWHITE
#define  CELATTR    HWINATTR

#define TMP ".TMP"
#define GRT ".GRT"             /* greeting extension */
#define GMASK "*.GRT"
#define MSG ".MSG"             /* message extension */
#define MMASK "*.MSG"          //mask for finding message files
#define MSGFSTR "%s%02d%s"     //format string for writing box 
#define RMD ".RMD"             // reminder message extension
#define RMASK "*.RMD"    
#define DATMASK "*.DAT"         //for doing backups
#define CSVMASK "*.CSV"         //for doing backups
#define MASKLEN 12

//user data file handle
#define FUSER fuser

#define DEFGRT "GREETING.VOX"  /* default greeting file -- not used */
#define LOGFILE "LIFELINE.LOG" /* log file for debug */
#define CPERH "CALLS.DAT"      //file to keep last 24 hours of calls 
#define USERFILE "USERS.DAT"   //user data file
#define PGQFILE  "PAGE.DAT"     //image of paging queue
// ??? #define USERIDX  "USERS.IDX"   //user index file
#define BACKUPFILE "BACKUP.DAT" //back up parameters file
#define DISKUSED "DISKUSED.DAT" //keep track of space used by lifeline
#define ANNOUNCEMENT "ANNOUNCE.VOX"
#define ANNOUNCETEMP "ANNOUNCE.TMP"
#define TELCOFILE "TELCO.DAT" //Keeps info on which lines are connected to telco
#define PAYCODEFILE "PAYCODE.DAT" //Keeps payment codes for autopay system
#define PAYCODECSV "PAYCODE.CSV" //Keeps payment codes in csv format
#define PAYCODEBAK "PAYCODE.BAK" //for writing new csv file
#define PAYCODEOLD "PAYCODE.OLD" //backup for removing used paycodes

//user log is a bit different from the other 
//files it is designed to be used by programs outside of 
//lifeline (mainly to copy it elsewhere)
#define ULOGERR  "User log buffer overflow!!!"
#define ULOGFILE "USERLOG.DAT" //logs activation and deactivation of boxes
#define CSVFILE "USERS.CSV" //ascii delimited format (csv) file of user data
#define STATEMENT "STATEMNT.CSV" //for printing statements of receipts
#define LOGS "newfiles" // where logs of new files are kept for mirroring messages to a website (see util.c,ansr3.c also)
#define NOTIFY "notify" // where zero length file names are kept for message notification (see util.c,ansr3.c also)
#define FNAME_SIZE 13 // standard 8.3 format name size

#define REGNOW " This is a trial version of the LIFELINE voice mail system. | Register now and get the full 12 line version of LIFELINE, | a printed manual and user support documentation. || For registration info contact || "
#define ADDRESS " Cal Woodruff | | vmail: 604-682-3269 box 4000 | cel: 604-928-0730 | email: calvert@vcn.bc.ca | "
#define PRESSANY "| Press any key to continue... | "

/* end ansrdefs.h - see also defs.h, util.h and ansrscrn.h*/


