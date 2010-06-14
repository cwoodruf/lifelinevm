
/* ansrscrn.h - part of ansr3 program - see also util.h, ansr3.h, defs.h */
#include "userstr.h"
        
/* keep file positions for customer data and pin numbers */
// constants - see userstr.h (after US typedef)
// data structure
typedef struct useridxstruct {
    int avail :  5; // INUSE, ACTIVE, INACTIVE
    int admh  :  6; // if this is an admin box which port has reminder handle
	int type  :  5; // store box type here
} UIDX;
UIDX useridx[MAXBOX] = {0}; 

int userpos[MAXBOX] = {0};

//typedef struct admnstruct {
//	was: int admh, type, admn;
//} AS;
//AS admn[MAXBOX] = {0};

// pointers
int curruser; /* pointer to useridx */
int curractive;
int currinactive;

/* file for customer records */
int fuser; /* file handle for customer file */
int newuserfile = FALSE; /* indicate if file was newly created */

/* the customer data userstruct in userstr.h */
US user = {0},
   blankuser = {0},
   olduser = {0},
   finduser = {0},
   temp = {0},
   defuser = {0};
long defpos = NOTFOUND; //where defuser is in file
   
char password[DOSPATHLEN] = ""; /* password for edit functions */
char checkpass[DOSPATHLEN] = ""; /* buffer to check password */
char newtimestr[TIMELEN + 1]; //for changing system time

typedef struct renamefiles
	{
		char file[DOSPATHLEN];
		char name[DOSPATHLEN];
	} RF;
RF ren = {0};
	
//keyboard states, messages, titles and field limits
typedef enum keystates
	{
	ksVIEW, 			//scroll through boxes
	ksEDIT,             //change a box
	ksPASSWORD,         //use system password
	ksEDITPASSWORD,     //change system password
	ksTIME,				//change system time
	ksRENOLDN,			//rename file: old name
	ksRENNEWN,			//rename file: new name
	ksMOVEMSG,			//enter box to move a message to
	ksBLOCKACTIVATE,    //activate or deactivate one or more boxes (sequentially)
	ksCOPYTEMPLATE,     //copy a box record ("template") to one or more boxes
	ksRANDOMPIN,		//randomize sec code for boxes
    ksRESET,
    ksSHOWUSER,			//dummy keystate that acts as a sort of border
	ksMSGINFO,			//view msg file info
	ksBACKUP,           //enter system backup parameters
	ksFIND,				//find user record
	ksMAX
	} KS; 
//max keystate
KS keystate = ksVIEW; 

//help data structures
typedef struct helpstruct 
	{
	char info[60];
	char kstr[30];
	unsigned char key;
	char zero;
	} HS;

enum helpwindows  
	{
	hwCURSOR, 
	hwEDITREC, 
	hwEDITFIELD, 
	hwEDITSPECIAL1,
	hwEDITSPECIAL2,
	hwEDITSPECIAL3,
	hwFIND,
	hwVIEWACTIVE,
	hwVIEWALL,
	hwVIEWMODES,
	hwVIEWSPECIAL,
	hwEND
	};

char hwins[] = 
	{ 
//find	
	hwCURSOR, 
	hwEDITREC, 
	hwEDITFIELD, 
	hwFIND, 
	hwEND,
//edit	
	hwCURSOR, 
	hwEDITREC, 
	hwEDITFIELD, 
	hwEDITSPECIAL1, 
	hwEDITSPECIAL2, 
	hwEDITSPECIAL3, 
	hwEND,
//view
	hwVIEWMODES,
	hwVIEWSPECIAL,	
	hwVIEWACTIVE,
	hwVIEWALL,
	hwEND
	};

#define FINDHSTART 	0
#define EDITHSTART 	5
#define VIEWHSTART  12

#define MOREHELP 127 //see next screen key
    	
HS help[hwEND][HELPHT] = 
{
  {
    {"CURSOR COMMANDS ", " ", 0, 0},
    {"Go to first field. ", "Ctrl-HOME", CtrlHOME, TRUE},
    {"Go to last field. ", "Ctrl-END", CtrlEND, TRUE},
    {"Toggle insert/overwrite. ", "Insert", INS, TRUE},
    {"Go to next field. ", "TAB/ENTER/DOWN", TAB, FALSE},
    {"Go to previous field. ", "Shift-TAB/UP", UP, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },
    
  {
    {"EDIT COMMANDS: FIELDS ", " ", 0, 0},
    {"Copy a field to clipboard. ", "Ctrl-K", CtrlK, FALSE},
    {"Cut a field and copy to clipboard. ", "Ctrl-X", CtrlX, FALSE},
    {"Paste to field from clipboard. ", "Ctrl-V", CtrlV, FALSE},
    {"Insert date. ", "F2", F2, TRUE},
    {"Clear a field. ", "Ctrl-Y", CtrlY, FALSE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },
	
  {    
    {"EDIT COMMANDS: RECORDS ", " ", 0, 0},
    {"Copy a whole record (template) to clipboard. ", "Alt-T", AltT, TRUE},
    {"Paste whole records (templates) from clipboard. ", "Alt-Z", AltZ, TRUE},
    {"Clear a whole record. ", "Alt-Y", AltY, TRUE},
    {"Save without exiting EDIT mode. ","Alt-S", AltS, TRUE},
    {"Force box to be ACTIVE if INUSE.","Alt-F", AltF, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },
    
  {   
    {"SPECIAL EDIT COMMANDS ", " ", 0, 0},
    {"Set reminder message for box (four choices). ", "Ctrl-R", CtrlR, FALSE},
    {"Activate or deactivate box(es). ", "Alt-A", AltA, TRUE},
    {"Toggle box on hold. ","Alt-H", AltH, TRUE},
    {"Delete files for this box. ", "Alt-D", AltD, TRUE},
    {"Change box type. ", "Ctrl-T", CtrlT, FALSE},
	{"Undo edit. ", "Alt-U", AltU, TRUE},
    {"","", 0, NOTFOUND}
  },
    //{"Delete files of all inactive boxes. ", "Alt-R", AltR, TRUE},

  {   
    {"MORE SPECIAL EDIT COMMANDS ", " ", 0, 0},
    {"Rename a file. ", "Ctrl-N", CtrlN, FALSE},
    {"Create/edit password. ", "Alt-P", AltP, TRUE},
    {"Set default security code for new boxes. ", "Ctrl-D", CtrlD, FALSE},
    {"View message info. ","PGDN", PGDN, TRUE},
    {"Boxday count (admin boxes only). ","F4", F4, TRUE},
    {"Change language. ","Ctrl-L", CtrlL, FALSE},
    {"","", 0, NOTFOUND}
  },

  {   
    {"MORE SPECIAL EDIT COMMANDS ", " ", 0, 0},
    {"Create a series of random security codes. ", "Ctrl-A", CtrlA, FALSE},
    {"Do a system backup. ","Alt-B", AltB, TRUE},
    {"Activate system announcement.","Alt-M", AltM, TRUE},
    {"Put system in maintenance mode.","Alt-O", AltO, TRUE},
    {"Reset channels.","Alt-R", AltR, TRUE},
    {"Paging management.","Alt-Q", AltQ, TRUE},
    {"","", 0, NOTFOUND}
  },

  {
    {"FIND HELP ", " ", 0, 0},
    {"Field syntax: [@][</>][info/tag]. ", " ", 0, 0},
    {"(@: info is tag. F4: show tags.)", " ", 0, 0},
    {"~ = empty field. ? = non empty field.", "~/?", 0, 0},
    {"Toggle language. ", "Ctrl-L", CtrlL, FALSE},
    {"Toggle active/inactive/both. ", "Alt-A", AltA, TRUE},
    {"Toggle box type. ","Ctrl-T", CtrlT, FALSE},
    {"","", 0, NOTFOUND}
  },
  
  {
  	{"VIEWING ACTIVE BOXES ", " ", 0, 0},
  	{"First active box. ", "HOME", HOME, TRUE},
  	{"Last active box. ", "END", END, TRUE},
  	{"Next active box. ", "PGDN", PGDN, TRUE},
  	{"Previous active box. ", "PGUP", PGUP, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },
  
  {
  	{"VIEW BOXES NUMERICALLY", " ", 0, 0},
  	{"Box \"0000\". ", "Ctrl-HOME", CtrlHOME, TRUE},
  	{"Box \"9999\". ", "Ctrl-END", CtrlEND, TRUE},
  	{"Next numerical box. ", "Ctrl-PGDN", CtrlPGDN, TRUE},
  	{"Previous numerical box. ", "Ctrl-PGUP", CtrlPGUP, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },
  
  {
    {"HELP: "," ", 0, 0},
    {"Edit a box. ","TAB", TAB, FALSE},
    {"Find a specific box. ","Alt-F", AltF, TRUE},
    {"Find next box (of a specified type). ","F3", F3, TRUE},
    {"Call statistics. ","F2", F2, TRUE},
    {"Exit program. ","Alt-X", AltX, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  },

  {
    {"SPECIAL COMMANDS: "," ", 0, 0},
    {"Change system time. ","Alt-T", AltT, TRUE},
    {"Refresh user data. ","Alt-R", AltR, TRUE},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND},
    {"","", 0, NOTFOUND}
  }
};

//message defines
#define CURRSTR (msg[keystate] + offs)
#define STDSTR " VIEW:  F1: Help.  Alt-N: Create New Box.  Alt-X: Exit."

//MESSAGES
int editstart = 0; //where to start editing on msg line
int offs = 0; //for deactivate message to lose  first two characters
int isdefmsg = TRUE; //indicates if default message for keystate has been overwritten

char * msg[ksMAX] = 
    {
        STDSTR,
        "EDIT:  F1: help. Alt-A: Activate. Ctrl-R: Reminder. Ctrl-L: Language.",
        "PASSWORD : ",
        "NEW PASSWORD : ",
		"ENTER SYSTEM TIME : HH:MM",
        "FILE TO RENAME : ",
        "NEW NAME : ",
		"MOVE: enter target box (ESC to exit): ",
        "DEACTIVATE: Enter number of boxes: 1",
        "COPY TEMPLATE: Enter number of boxes to copy template to: 1",
        "RANDOM SEC CODE: Enter number of boxes to create random sec codes: 1",
        "RESET CHANNEL: Enter channel number: 1",
        "",
        "UP/DOWN/HOME/END/PGUP/PGDN select. Alt-M: move file. DEL: delete. ESC: exit.",
        "BACK UP FILES: Alt-F: all. Alt-P: data. Alt-C: text. Alt-A: text to A:",
        "FIND:  F1: help.  ? = non-empty field, ~ = empty field. Alt-A: toggle active."
    };
    
#define STDTTL " Tab: EDIT Box.  Alt-F: FIND Box.  F3: Find Next.  F2: Stats.   Alt-X: EXIT."
#define EDTTL "Esc: VIEW Boxes.  Alt-B: System Back up.  PGDN: Message info. F3: Find Next."
#define SUBTTL "Esc: EDIT Boxes."
char * title[ksMAX] =
    {
		STDTTL,
		EDTTL,
		NULL,
		EDTTL,
		EDTTL,
		EDTTL,
		EDTTL,
		EDTTL,
		EDTTL,
		EDTTL,
		EDTTL,
		"",
		EDTTL,
		EDTTL,
		SUBTTL,
        "Esc: VIEW Boxes.  F3: Find Next."
    };
    
/* first and last field accessible for a given keystate */
struct posstruct
    {
        enum fields min, max;   
    } 
    poslimit[ksMAX] =
    {
    {fPIN, fPIN},    /* ksVIEW */
    {fPIN,fPAIDTO},  /* ksEDIT */
    {fMSG,fMSG},     /* ksPASSWORD */
    {fMSG,fMSG},     /* ksEDITPASSWORD */
    {fMSG,fMSG},     /* ksTIME */
    {fMSG,fMSG},     /* ksRENOLDN */
    {fMSG,fMSG},     /* ksRENNEWN */
    {fMSG,fMSG},     /* ksMOVEMSG */
    {fMSG,fMSG},     /* ksBLOCKACTIVATE */
    {fMSG,fMSG},     /* ksCOPYTEMPLATE */
    {fMSG,fMSG},     /* ksRANDOMPIN */
    {fMSG,fMSG},	 // ksRESET
    {0,0}, 			 // ksSHOWUSER
    {fMSG,fMSG},     /* ksMSGINFO */
    {fBUPPATH, fPURGE},/* ksBACKUP */
    {fBOX,fMSGS}   /* ksFIND */
    };
                                             
#undef min
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )

//for clearing screen
CELL far *scrn;
CELL far emptyscrn[8 * 80];

//screen definition array
struct screentext 
{
    int x, y, l; /* initial x,y location on screen and length of string to show */
    int reps;  /* how many times to repeat it down */
    int num;   /* whether or not to add numbers at "^" in string */
    int attr;  /* color attribute of string */
    char s[81];   /* string to be written */
    char f[81];   //format specifier for string if any
} scrtxt[] = {
    /* customer info */
#ifdef TIMELIMIT
    {MSGX,USER_TOP,SCRWID,1,FALSE,STATSATTR,""," USER INFO:                    EXPIRY: %02d-%s-%02d      %s"},
#else                                                                                                  
    {MSGX,USER_TOP,SCRWID,1,FALSE,STATSATTR,""," USER INFO:                                           %s"}, 
#endif    
    /* title */
    {MSGX,TITLE_TOP,SCRWID,1,FALSE,STATSATTR, TITLEMSG, ""},
    {MSGX,STATS_TOP,SCRWID,1,FALSE,STATSATTR, "", ""},
    
    /* line info table */
    {MSGX,LINES_TOP,          SCRWID,     MAXCHAN,       TRUE,bgBLACK," LINE ^ : Inactive.", ""},
    {MSGX, LINES_TOP + min(MAXCHAN, 12), SCRWID, 12 - min(MAXCHAN,12),min(MAXCHAN, 12) + 1,bgBLACK," LINE ^ : Unavailable.", ""},
    
    /* field area */
    {MSGX,BOXY,SCRWID,8,FALSE,bgBLUE,"", ""},
    /* sub title line */
    {MSGX,INFO_TOP,SCRWID,1,FALSE,HELPATTR,   STDTTL, ""},
    /* help line */
    {MSGX,HELP_TOP,SCRWID,1,FALSE,HELPATTR,   STDSTR, ""},
    /* end of list marker */
    {0} 
};      

enum screen 
{TITLE, STATS, LINES, LINESNA, USERINFO, FIELDS, INFO, HELP, EOL};
 
/* used to accept y/n keyboard input in dokey */
enum actions {aNOACTION, aEDITFINDNEXT, aEDSAVENOEXIT,
			  aDELUSER, aDEFPIN, aVIEW, aSAVEUSER, 
			  aBLOCKACTIVATE,
              aCOPYTEMPLATE, aRANDOMPIN, aEDITUSER, 
              aDOREMIND, aREMINDALL, aCLEARALL, 
              aAUTOBACKUP, aBACKUP, aPARTBUP, aCSVBUP, aCSVABUP,
              aCHKBOXES, aKEEPBACKUP, 
              aRESET, aRESETSINGLE, aRESETCONFIRM,
              aPURGE, aRENAME, aMOVEMSG, aDELSINGLEMSG,
              aANNOUNCE, aDELRMD, aTIME, aCLEARBOXDAYS, aCLEARPAGEQ,
              aMAKETEMPLATE, aMAINTENANCE, aEXITPROGRAM, aWAITEXIT};

/* function definitions */    
/* this file...*/
void writedebug(char *msg);
void writemsg(char * s);
void writeimsg(char * s, int i);
void writech(char * s);
void writescreentext(struct screentext scr);
void makescreen(void);
void remakescrn(char *hwin, char *currwin);
void showttls( void );
void cleardisplay( void );
void writechan(int channel);
void makestats( int channel, int state, int error);
int loaduser(int fuser, int idx, struct userstruct *user);
void getuserrec(int fuser, int curruser, struct userstruct *user);
void savepin( void );
void saveuser(int fuser, int box, struct userstruct *user);
int getboxtype(US *user);
void makeboxtype( US *user, int bt );
void setuprnd( void );
void copypin(US *u);
void putbox(struct userstruct *c, int box);
char *selectfield(enum fields i, struct userstruct *user);
int userequal(struct userstruct *user1, struct userstruct *user2);
void updatecalls(struct userstruct *user);
void writefield(int i, int loffs, US *user);
void showuser(struct userstruct *user);
void douseractivate(struct userstruct *user);
void activateuser( void );
void deactivateuser( void );
void openuserfile(int *fuser);
void inituser(int *fuser);
int checkpassword(int *i, int *pos, int *startpos);
int finduserrec(void);
int fldcmp(char *find, char *user, US *u, int fld);
int cmpfld(char *user, char *fstr, int fld);
int isnumeric(char *fstr);
int cmpnumbers(char *user, char *fstr);
int cmpdates(char *user, char *fstr);
void editkeystate(enum keystates keystate, enum fields i, int *startpos, int *pos, char key, char zero);
void godown(enum fields *j);
int newuser( void );
void keystatesetup(KS ks, int *i, int *pos, int *startpos);
int changekeystate(enum keystates c, enum fields *i, int *pos, int *startpos);
void tagsvisible ( int st );
int checkactive(int active, int curruser);
int up(int active);
int down(int active);
void copytemplate(US *user);
int _gotouser(unsigned char *key, int active);
int checkpin(char *pin);
int saveedit(void);
void deluser( int curr, US *u );
void gouser(int i);
void blockactivate( void );
void blockcopytemplate( void );
void randompin( void );
void copytemplate(US *user);
int backupchanged( void );
int checkbackup( void );
int doaction(int* action, unsigned char *key, char *zero, int *i, int *pos, int *startpos);
void clearfind( int *i, int *pos, int *startpos );
void clearuser(int *i, int *pos, int *startpos);
void editactive(int *action);
void writefieldmsg(int i);
int exitedit(int *action);
void renamegrt( void );
void updateUSER( void );
void telcoscrn(unsigned char key, char zero);
void daemonkey(unsigned char *key, char *zero);
void dokey(unsigned char *key, char *zero);
void initboxes (void);
void initscrn (void);                                 

/* for debug: show custinfo using printf statements */
void printuserrec(struct userstruct user);
void printuser (void); 
/* end ansrscrn.h - see also util.h, ansr3.h, defs.h */





