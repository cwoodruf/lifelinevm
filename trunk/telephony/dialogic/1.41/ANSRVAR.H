
/* ansrvar.h -- part of ansr3.c file - see also util.h, ansrscrn.h, ansr3.h */

/* data structures */
/* struct to hold channel information */
struct portstruct{
   char dtmf[BUFFSIZE];  /* buffer for dtmf digits */
   char firstdigit[2];
   char otherdigits[BUFFSIZE];
   char pin[PINLEN + 1];
   int event; // what was the last event?
   int box;   /* keep box number here */
   int newbox; //used by autoassign feature ...
   int newpin; //to announce new box and pin.
   int admin;   //number of admin box used as basis for browsing
   int numboxes; // number of boxes associated with admin box
   int accessed; // did user enter a correct pin number?
   time_t newpaidto; //and paid to date
   time_t lastaccess; //when was the port last used?
   // state transition info
   char language; //language to use
   int prev;  /* for selecting between states after state has changed */
   int curr;  /* keep track of state of each channel */
   int oldstate; // in case you want to rewrite history
   int greetstate; //indicates whether a personalized or generic greeting will be used
   int menu; //menu that user is currently in
   char sequence; //is user in a sequence (eg playing box properties)?
   //messages
   char modified; //did user go into box to listen/delete messages or greeting?
   long boxsize; //actual file space taken up by box
   char currmsg; /* current message */
   char lastmsg; /* to store nextmsg variable from custstruct */
   char deleted; /* indicates if messages are to be written over */
   // flags
   char getdid; //get direct inward dial digits if DID active
   char browse;   // is admin box browse function turned on?
   char paycodeonly; // payment method for admin box
   char superuser; // did user log in with skeletonpin?
   char god;	   // did user log in as?
   // file handles
   int mh;      /* file handle for message file */
   char fname[FNAME_SIZE+1]; // the current message being saved
   int gh;      /* file handle for greeting file */
   int rh;      // reminder handle for admin boxes

} blankport = {0}, port[MAXCHAN + 1] = {0};
typedef struct portstruct PS;
//line status message
char chmsg[MAXCHAN][80];  //line status message
// customer info
struct userstruct prtuser[MAXCHAN + 1] = {0};
//whether a message "exists" or not T or F or box number plus offset
int msgs[MAXCHAN + 1][MAXMSG] = {0}; 
//how many times has a box been accessed?
unsigned char iteration[MAXCHAN] = {0}; 
//how many times has a security code been entered?
unsigned char seccodeiter[MAXCHAN] = {0}; 
//file handle for multi prompt files see also ANSRDEFS.H
int announceh = 0; //for general system announcement file
// Paging variables
//array for paging queue
typedef struct pagestruct
{
	int next;
	char tries;
} PGS;
PGS page[MAXBOX] = {0};
PGS blankpgs = {0};
//pointer to start of paging queue
int pgstart = NOTFOUND;
int currpg;
//used to determine if it is ok to try page
enum pagestates {NOPAGE, PAGENEW, PAGEALL};
int pagestate = NOPAGE;

//variables for changing properties of "new" boxtype
typedef struct newpropstruct {
	char rangestart[BOXLEN + 1];
	char rangeend[BOXLEN + 1];
	char freetime[BOXLEN + 1];
} NP;
NP newprops[MAXCHAN] = {0};
NP emptynewprops = {0};
//variables for changing regular box properties
US boxprops[MAXCHAN] = {0}; //BOXPROPS and BPP (pointer to boxprops record) used
char keep[MAXCHAN] = {0}; //KEEP used. Monitors if box prop record needs to be updated

//variables for telcoscrn (util.c) 
//select which channels are used by telephone company
//used also to indicate DID channels
char telcochans[MAXCHAN] = {0};
char telcomax = 0; //keeps track of number of telco lines
char showtelco = FALSE; //show telco select screen
int celleft; //left pos of array
int celtop;  //top of array
int celwid;  //width of each array element
char tstatestr[6][20] = {"        ", "Telco   ", "DID     ", "Wink DID", "DIRECT  ", "n/a     "};
enum tstates {tsAVAIL, tsTELCO, tsDID, tsDIDWINK, tsDIRECT, tsNOTAVAIL};
//variables for moving a message
char targetbox[20] = "";

/**
 **   Function prototypes
 **/
 
//general function prototypes
int renannounce( void ); //tries to rename temp announcement file 
int getbox(int channel, int dtmf, int len); //checks box number and loads box info
/*

STATE MACHINE PROTOTYPES:

initiating functions;   processing functions 
 => does something       => goes to a state based on result of 
                            initiating function
*/
int invalidstate(int);	int  invalidstate_cmplt(int,int);
/* start up functions */
int wtring(int);        int  wtring_cmplt(int, int);
						int  wink_cmplt(int, int);
int wtkey(int);			int  wtkey_cmplt(int, int);
int offhk(int);         int  offhk_cmplt(int, int);
/*introductory message and key capture functions */
int genintro(int);      int  genintro_cmplt(int, int);
int getdigits(int);     int  getdigits_cmplt(int, int);
int checkhelpdigit(int);int  checkhelpdigit_cmplt(int, int);
int firstdigit(int);    int  firstdigit_cmplt(int, int);
int otherdigits(int);   int  otherdigits_cmplt(int, int);
int reenter(int);		int  reenter_cmplt(int, int);
int getnewpin(int);     int  getnewpin_cmplt(int, int);
int confirmpin(int); 	int  confirmpin_cmplt(int, int);
int getdigit(int);      int  getdigit_cmplt(int, int);
int inuse(int);         int  inuse_cmplt(int, int);
//funcs for btNEW 
int newintro(int);		int  newintro_cmplt(int,int);
int playphintro(int);	int playphintro_cmplt(int, int);
int playboxintro(int);	int  playboxintro_cmplt(int, int);
int playbox(int);		int  playbox_cmplt(int, int);
int noboxavail(int);	int  noboxavail_cmplt(int, int);
int playpinintro(int);	int  playpinintro_cmplt(int, int);
int playpin(int);		int  playpin_cmplt(int, int);
int newinfo(int);		int  newinfo_cmplt(int,int);
//funcs to change btNEW properties
void savenewprops(int);
void loadnewprops(int);
void clearnewprops(int);
int newmenu(int);		int  newmenu_cmplt(int,int);
int rangedigits(int);	int rangedigits_cmplt(int, int);
//end btNEW funcs
int invalidpin(int);    int  invalidpin_cmplt(int, int);
int thankyou(int);      int  thankyou_cmplt(int, int);
int remind(int);        int  remind_cmplt(int, int);
int admrmd(int);		int  admrmd_cmplt(int, int);
int announcement(int);  int  announcement_cmplt(int, int);
int pressany(int);		int  pressany_cmplt(int,int);
/*record functions */
int greeting(int);      int  greeting_cmplt(int, int); 
int gengrt(int);
int recordmsg(int);     int  recordmsg_cmplt(int, int);
//paging functions
int addpage(US *u);
char * rmfromq(int box);
void clearpgq( void );
int pgcall(int);		int  pgcall_cmplt(int, int);
int pgresponse(int);	int  pgresponse_cmplt(int, int);
int pgdial(int);		int pgdial_cmplt(int, int);
int askpgnum(int);		int  askpgnum_cmplt(int, int);
int pgdigit(int);		int  pgdigit_cmplt(int, int);
int pgnum(int);			int  pgnum_cmplt(int, int);
int playpgnum(int);		int  playpgnum_cmplt(int, int);
int confirmpgnum(int);	int  confirmpgnum_cmplt(int, int);
int pgactive(int);		int  pgactive_cmplt(int,int);
int pgid(int);			int pgid_cmplt(int, int);
int playsysphone(int);  int playsysphone_cmplt(int, int);
/*playback functions */
int geninstr(int);      int  geninstr_cmplt(int, int);
int listenmnu(int);     int  listenmnu_cmplt(int, int);
int playmsg(int);       int  playmsg_cmplt(int, int);
int lang(int);			int lang_cmplt(int, int);
int nolang(int);		int nolang_cmplt(int, int);
//forwarding
int forward(int);		int forward_cmplt(int, int);
int getfwddigit(int);   int getfwddigit_cmplt(int, int);
int getfwdbox(int);   	int getfwdbox_cmplt(int, int);
int conffwd(int);		int conffwd_cmplt(int, int);
int willfwd(int);		int willfwd_cmplt(int, int);
int wontfwd(int);		int wontfwd_cmplt(int, int);
//greeting
int playgrtinfo(int);   int  playgrtinfo_cmplt(int, int);
int recgrtinfo(int);    int  recgrtinfo_cmplt(int, int);
int recgrt(int);        int  recgrt_cmplt(int, int);
//paycode stuff
int numboxes(int);		int numboxes_cmplt(int, int);
int paidtomsg(int); 	int  paidtomsg_cmplt(int, int);
int getpaymsg(int); 	int  getpaymsg_cmplt(int, int);
int newpaymsg(int);  	int  newpaymsg_cmplt(int, int);
int getpaydigit(int);	int  getpaydigit_cmplt(int, int);
int getpaystr(int);		int  getpaystr_cmplt(int, int);
int paidtodate(int); 	int  paidtodate_cmplt(int, int);
int payerrintro(int);	int  payerrintro_cmplt(int,int);
int payerrnum(int);		int  payerrnum_cmplt(int,int);
int bdnum(int);			int bdnum_cmplt(int, int); 
int bdupdate(int);		int bdupdate_cmplt(int, int);
int getbdval(int);		int getbdval_cmplt(int, int);
int bddigit(int);		int bddigit_cmplt(int, int);
int pdtodigit(int);		int pdtodigit_cmplt(int, int); 
int pdtostr(int);		int pdtostr_cmplt(int, int); 
int nomsgs(int);        int  nomsgs_cmplt(int, int);
int lastmsg(int);		int  lastmsg_cmplt(int, int);
                        int  deletedmsg_cmplt(int, int);
                        int  wasdeleted_cmplt(int, int);
int delall(int);        int  delall_cmplt(int, int);
int dateintro(int);		int  dateintro_cmplt(int, int);
int date(int);			int  date_cmplt(int, int);
//admin functions
int admin(int); 		int admin_cmplt(int, int);
int admindigits(int);   int admindigits_cmplt(int, int);
int adminmsg(int);		int adminmsg_cmplt(int, int);
int newrmd(int);		int newrmd_cmplt(int, int);
int recrmd(int);		int recrmd_cmplt(int,int);
int browse(int); 		int browse_cmplt(int,int);
//sys functions
int syscode(int);		int syscode_cmplt(int, int);
int sysmenu(int);		int sysmenu_cmplt(int, int);
int act(int);			int act_cmplt(int, int);
int bup(int);			int bup_cmplt(int, int);
int recannc(int);		int recannc_cmplt(int, int);
int newannc(int);		int newannc_cmplt(int, int);
int hearannc(int);		int hearannc_cmplt(int, int);
int playnewannc(int);	int playnewannc_cmplt(int, int);
int rmord(char *rmd);
void incrm(int channel, char *rmd);
void phoneactivate(int box, US *u);
void phonedeactivate(int box, US *u);
int playboxprops(int);	int playboxprops_cmplt(int, int);
int actresult(int);		int actresult_cmplt(int, int);
int togprop(int);		int togprop_cmplt(int, int);
int type(int);			int type_cmplt(int, int);
int rmd(int);			int rmd_cmplt(int, int);
int no(int);			int no_cmplt(int, int);
int playadmn(int);		int playadmn_cmplt(int, int);
int boxstat(int);		int boxstat_cmplt(int,int);
int boxstatmsg(int);	int boxstatmsg_cmplt(int,int);
int youhave(int);		int youhave_cmplt(int,int);
int num(int);			int num_cmplt(int,int);
int statmsg(int);		int statmsg_cmplt(int,int);
int systime(int);		int systime_cmplt(int,int);
int systimedigit(int);	int systimedigit_cmplt(int,int);
int systimestr(int);	int systimestr_cmplt(int,int);
//not used
/*
int newsyscode(int);	int newsyscode_cmplt(int, int);
int getsyscode(int);	int getsyscode_cmplt(int, int);
int getdestroy(int);	int getdestroy_cmplt(int, int);
int _continue(int);		int _continue_cmplt(int, int);
*/
/*exit functions */
int reset(int);			int reset_cmplt(int, int); 
int hangup(int);		int hangup_cmplt(int,int);
int onhk(int);          int onhk_cmplt(int, int);

/***************************************************************************/
/**
 **   Data structure which stores the pointers to functions for each state
 **/
/***************************************************************************/
/* state constants */
enum statenames {
/* start up states */
sWTRING, sWTKEY, sWINK, sOFFHK, sDIDOFFHK,
/* introductory message and key capture states */
sGENINTRO, sGETDIGITS, sCHECKHELPDIGIT, sGETBOXDIGITS, 
sFIRSTDIGIT, sOTHERDIGITS, 
sNOTHING, /* really doesn't do anything except take up space */
sREENTER, sGETNEWPIN, sCONFIRMPIN, sGETSKELETON, sGETDIGIT, 
//response states
sINVALID, sINVALIDPIN, sTHANKYOU, sINUSE, sREMIND, sADMRMD, 
sANNOUNCE, sPRESSANY, sFULL,
//autoassign states
sNEWINSTR, 
sNEWINTRO, sPHINTRO, sEXTENSION, 
sPLAYBOX, sNOBOXAVAIL, 
sPLAYPININTRO, sPLAYPIN, sNEWINFO, 
sNEWPROPS, sRANGE, sTO, sNORANGE, sFREETIME, sMONTHS, 
sRESTORENEWPROPS, sRNGSTART, sRNGEND, sRANGEDIGITS, sGETFREETIME, sCLEARPROPS,
/* record states */
sGREETING, sGENGRT, sRECORDMSG,
// paging states
sPAGE, sPGRESPONSE, sPGDIAL, sASKPGNUM, sPGDIGIT, 
sPGNUM, sPLAYPGNUM, sCONFPAGE, sPGACTIVE, sRMPAGE,
sPGID, sPLAYSYSPHONE, sPGRPT,
/* playback states */
sGENINSTR, sLISTENMNU, sPLAYMSG, sLANG, sNOLANG, 
// forward message states
sFORWARD, sGETFWDDIGIT, sGETFWDBOX, sCONFFWD, 
sWILLFWD, sCONFWILLFWD, sWONTFWD, sFWDFULL,
// greeting modification states
sPLAYGRTINFO, sRECGRTINFO, sRECGRT,
// paycode states 
sPAIDTO, sGETPAY, sNEWPAY, 
sPAYDIGIT, sPAYSTR, sPAIDTODATE,
sPAYERRINTRO, sPAYERRNUM, 
//superuser payment menu
sBDMENU, sYOUOWE, sDOLLARSNUM, sDOLLARS, sAND, sCENTSNUM, sCENTS, sFOR,
sBDOWED, sBBOXDAYS, sAT, sPLAYBDVAL, sCENTSEACH, 
sBDNUM, sNBOXDAYS, sNUMBOXES, sBOX, sBOXES, sBDUPDATEINTRO, sBDUPDATE, 
sGETBDVAL, sGETBDVALDIGIT, sGETBDVALSTR, 
sBBDREVERT, 
sBBDEDIT, sBBDSIL, sBBDCLEAR, 
sBDEDIT, sBDSIL, sBDCLEAR, 
sBDDIGIT, 
sBDRESTORE, 
sEDPDTO, sPDTODIGIT, sPDTOSTR, 
// message status states
sNOMSGS, sNEWMSGS, sLASTMSG, 
sDELETEDMSG, sWASDELETED, sRESTOREDMSG, sDELETE, sRESTORE,
sDATEINTRO, sDATE,
//admin states
sADMINTRO, sADMIN, 
sSETRMD, sCLRRMD, sPLAYRMD, sNEWRMD, sRECRMDMSG, 
sRMALLMNU, sRMALLON, sRMALLOFF,
sCHECKDEL, sDELBOX, 
sADMINDIGITS, sRECRMD, 
sBOXCODE, sCHGCODE, sCONFCHGCODE,
sBROWSE, sTOGBRWS, sBRWSON, sBRWSOFF,
//sys states
sNUM, sUSERS, sMINBUSY, sMAXWAIT, sCALLS, sMSGS, sPGCALLS, sPPCUSED, sSYSCODE, 
sSYSMENU,
sBOXPROPS,
sRECANNC, sANNCON, sANNCOFF, sACTANNC, sNEWANNC, 
sNOANNC, sHEARANNC, sPLAYNEWANNC, sFORCEBDUPDATE,
sSYSTIMEINTRO, sPLAYSYSDATE, sPLAYSYSTIME, 
sEDSYSTIME, sSYSTIMEDIGIT, sSYSTIMESTR, sSYSTIMEERR,
sBOXPROPSMENU, sNOFEAT, sPLAYBOXPROPS, sPLAYADMN, sNO, 
sBOXSTAT, sBOXLASTCALL, sBOXPAIDTO, sBOXCALLS, sBOXMSGS,
sOLDPROPS,
sTOGRMD, sRMALL, sSYSRM, sADRM, sNEVRM, sADMRM, sNORM,
sNEWADMIN, sCONFNEWADMIN, 
sTOGTYPE, sMSGTYPE, sNEWTYPE, sDEFTYPE, sADMNTYPE, sPAYCTYPE, sBBSTYPE,
sISACTV, sNOTACTV, sONHOLD,
//UNUSED SYS STATES
/*
sPARTBUP, sFULLBUP, 
sNEWSYSCODE, sGETSYSCODE,
sEXITMENU, sGETDEST, sDESTROY, sCONTINUE,
*/
/* exit states */
sRESET, sHANGUP, sONHK,
/* OOPS! */
sINVALIDSTATE
};
#define MAXSTATE sINVALIDSTATE
/***************************************************************************/
//state info
struct state {                
   int ord;				//placeholder for the name of the state
   char *desc;          /* state descriptions */
   int (*initiate)();   /* pointer to initiating functions */
   int (*process)();    /* pointer to processing functions */
   char fname[20];      /* any message file name associated with state */
   int fh;              /* file handle associated with file name */
   char termdt;		// if true function is not available
} states[] = {
       /* start up states */
       {sWTRING, "Waiting for a call.", wtring, wtring_cmplt, "", 0, 0},
       {sWTKEY, "Waiting for a keypress.", wtkey, wtkey_cmplt, "", 0, 0},
       {sWINK, "Sending wink to get DID digits.", wink, wink_cmplt, "", 0, 0},
       {sOFFHK, "Off hook.", offhk, offhk_cmplt, "", 0, 0},
       {sDIDOFFHK, "Going off hook after getting DID digits.", offhk, getdigits_cmplt, "", 0, 0},
       /* introductory message and key capture states */
       /* "enter your four digit box# now" */
       {sGENINTRO, "Asking for box number.", genintro, genintro_cmplt, "GETBOX.VOX", 0, 0}, //1
       {sGETDIGITS, "Analysing multiple digits.", getdigits, getdigits_cmplt, "", 0, 0},     
       {sCHECKHELPDIGIT, "Analysing first box digit.", checkhelpdigit, checkhelpdigit_cmplt, "", 0, 0},     
       {sGETBOXDIGITS, "Analysing rest of box digits.", otherdigits, otherdigits_cmplt, "", 0, 0},     
       //basic sec code check to access box
       {sFIRSTDIGIT, "Analysing first sec code digit.", firstdigit, firstdigit_cmplt, "", 0, 0},     
       {sOTHERDIGITS, "Analysing rest of sec code digits.", otherdigits, otherdigits_cmplt, "", 0, 0},     
       {sNOTHING, "Placeholder state",NULL,NULL,"",0}, /* put here because we are losing R.V in fname below */
       {sREENTER, "Re-entering security code.", reenter, reenter_cmplt, "REENTER.VOX", 0, 0},
		//edit security code
       {sGETNEWPIN, "Getting new security code.", getnewpin, getnewpin_cmplt, "NEWPIN.VOX", 0, 0},     //2
       {sCONFIRMPIN, "Confirming new security code.", confirmpin, confirmpin_cmplt, "CONFRMPN.VOX", 0},
       //generic sec code
       {sGETSKELETON, "Getting skeleton security code.", getdigits, getdigits_cmplt, "", 0, 0},
       //multiuse state for validating menu choices
       {sGETDIGIT, "Analysing one digit.", getdigit, getdigit_cmplt, "", 0, 0},
       /* " that box in use right now please call again " */
       {sINVALID, "Playing \"invalid number\" message.", inuse, inuse_cmplt, "INVALID.VOX", 0, 0}, //3
       {sINVALIDPIN, "Playing \"invalid security code\" message.", invalidpin, invalidpin_cmplt, "", 0, 0}, 
       {sTHANKYOU, "Playing \"thank you\" message.", thankyou, thankyou_cmplt, "THANKS.VOX", 0, 0}, //4
       {sINUSE, "Playing \"box in use\" message.", inuse, inuse_cmplt, "INUSE.VOX", 0, 0},//5
       {sREMIND, "Playing generic reminder message.", remind, remind_cmplt, "REMIND.VOX", 0, 0},//6
       {sADMRMD, "Playing custom reminder message.", admrmd, admrmd_cmplt, "", 0, 0}, 
       {sANNOUNCE, "Playing announcement.", announcement, announcement_cmplt, "", 0, 0}, 
       {sPRESSANY, "Playing \"press any key\" message.", pressany, pressany_cmplt, "PRESSANY.VOX", 0, 0}, 
       {sFULL, "Playing \"box full\" message.", inuse, inuse_cmplt, "FULL.VOX", 0, 0},//7
       //autoassign states
       {sNEWINSTR, "(New) Main autoassignment box menu.", geninstr, geninstr_cmplt, "NEWINSTR.VOX", 0, 0},//7
       {sNEWINTRO, "(New) Playing new user welcome message.", newintro, newintro_cmplt, "NEWINTRO.VOX", 0, 0},//7
       {sPHINTRO, "(New) Playing phone intro message.", playphintro, playphintro_cmplt, "PHINTRO.VOX", 0, 0},
       {sEXTENSION, "(New) Playing extension message.", playboxintro, playboxintro_cmplt, "EXT.VOX", 0, 0},
       {sPLAYBOX, "(New) Playing box number.", playbox, playbox_cmplt, "INVALID.VOX", 0, 0},
       {sNOBOXAVAIL, "(New) Playing new box not available message.", noboxavail, noboxavail_cmplt, "NOBOX.VOX", 0, 0},
       {sPLAYPININTRO, "(New) Playing sec code intro message.", playpinintro, playpinintro_cmplt, "PININTRO.VOX", 0, 0},
       {sPLAYPIN, "(New) Playing security code.", playpin, playpin_cmplt, "INVALID.VOX", 0, 0},
       {sNEWINFO, "(New) Playing new user info.", newinfo, newinfo_cmplt, "NEWINFO.VOX", 0, 0},
       {sNEWPROPS, "(New) Playing properties menu.", newmenu, newmenu_cmplt, "NEWPROPS.VOX", 0, 0},
       {sRANGE, "(New) Playing range start.", newmenu, newmenu_cmplt, "RANGE.VOX", 0, 0},
       {sTO, "(New) Playing range end.", newmenu, newmenu_cmplt, "TO.VOX", 0, 0},
       {sNORANGE, "(New) Playing no range message.", newmenu, newmenu_cmplt, "NORANGE.VOX", 0, 0},
       {sFREETIME, "(New) Playing free time message.", newmenu, newmenu_cmplt, "ACTIVE.VOX", 0, 0},
       {sMONTHS, "(New) Playing number of free months.", newmenu, newmenu_cmplt, "NOMON.VOX", 0, 0},
       {sRESTORENEWPROPS, "(New) Restoring autoassignment properties.", newmenu, newmenu_cmplt, "RSTNEW.VOX", 0, 0},
       {sRNGSTART, "(New) Getting range start.", newmenu, newmenu_cmplt, "RNGSTART.VOX", 0, 0},
       {sRNGEND, "(New) Getting range end.", newmenu, newmenu_cmplt, "RNGEND.VOX", 0, 0},
       {sRANGEDIGITS, "(New) Getting range digits.", rangedigits, rangedigits_cmplt, "", 0, 0},
       {sGETFREETIME, "(New) Getting free time.", newmenu, newmenu_cmplt, "FREETIME.VOX", 0, 0},
       {sCLEARPROPS, "(New) Clearing btNEW properties.", newmenu, newmenu_cmplt, "CLRPROPS.VOX", 0, 0},
       /* record states */
       /* play customer greeting */
       {sGREETING, "Playing user greeting.", greeting, greeting_cmplt, "", 0, 0},
       {sGENGRT, "Playing generic greeting.", gengrt, greeting_cmplt, "GREETING.VOX",0, 0},//8
       {sRECORDMSG, "Recording a message.", recordmsg, recordmsg_cmplt, "", 0, 0},
       // paging
       {sPAGE, "Initiating page call.", pgcall, pgcall_cmplt, "INVALID.VOX", 0, 0},
       {sPGRESPONSE, "Page connected: asking for response.", pgresponse, pgresponse_cmplt, "PAGE.VOX", 0, 0},
       {sPGDIAL, "Page connected: dialing call back number.", pgdial, pgdial_cmplt, "", 0, 0},
	   {sASKPGNUM, "Paging setup prompt.", askpgnum, askpgnum_cmplt, "ASKPAGE.VOX", 0, 0},
	   {sPGDIGIT, "Getting first digit of paging number.", pgdigit, pgdigit_cmplt, "", 0, 0},
	   {sPGNUM, "Getting rest of paging number.", pgnum, pgnum_cmplt, "", 0, 0},
	   {sPLAYPGNUM, "Play paging number.", playpgnum, playpgnum_cmplt, "PRESSANY.VOX", 0, 0},
	   {sCONFPAGE, "Confirm paging number.", confirmpgnum, confirmpgnum_cmplt, "CONFPAGE.VOX", 0, 0},
	   {sPGACTIVE, "Paging active message.", pgactive, pgactive_cmplt, "PAGEON.VOX", 0, 0},
	   {sRMPAGE, "Paging inactive message.", pgactive, pgactive_cmplt, "RMPAGE.VOX", 0, 0},
 	   {sPGID, "Playing call back number intro.", pgid, pgid_cmplt, "PGID.VOX", 0, 0},
 	   {sPLAYSYSPHONE, "Playing call back number.", playsysphone, playsysphone_cmplt, "INVALID.VOX", 0, 0},
 	   {sPGRPT, "Repeat call back number intro.", pgid, pgid_cmplt, "PGRPT.VOX", 0, 0},
       /* playback states */
       /* menu:
          1) listen to your messages
          2) listen to your greeting
          3) record your greeting
          7) administrator functions (ADMIN BOXES ONLY)
          8) set up paging
          #) change PIN
          0) repeat these instructions
          *) go back to start
        */
       {sGENINSTR, "Main user menu.", geninstr, geninstr_cmplt, "GENINSTR.VOX", 0, 0},//9
       /* menu:
          1) next message
          2) prev message
          3) repeat message
          4) first message
          5) last message
		  7) delete this message
		  8) forward this message
          9) delete all messages
          *) back to previous menu
          0) listen to these instructions again  
        */
       {sLISTENMNU, "Listen to message menu.", listenmnu, listenmnu_cmplt, "LISTEN.VOX", 0, 0},//10
       {sPLAYMSG, "Playing a message.", playmsg, playmsg_cmplt, "NOMSGS.VOX", 0, 0},
       {sLANG, "Changing box language.", lang, lang_cmplt, "LANG.VOX", 0, 0},
       {sNOLANG, "No additional languages.", nolang, nolang_cmplt, "NOLANG.VOX", 0, 0},
       //forwarding
       {sFORWARD, "Playing forwarding intro message.", forward, forward_cmplt, "FORWARD.VOX", 0, 0},
       {sGETFWDDIGIT, "Get first forwarding box digit.", getfwddigit, getfwddigit_cmplt, "", 0, 0},
       {sGETFWDBOX, "Get rest of forwarding box digits.", getfwdbox, getfwdbox_cmplt, "", 0, 0},
       {sCONFFWD, "Playing forwarding confirmation message.", conffwd, conffwd_cmplt, "CONFFWD.VOX", 0, 0},
       {sWILLFWD, "Playing forwarding on message.", willfwd, willfwd_cmplt, "WILLFWD.VOX", 0, 0},
       {sCONFWILLFWD, "Playing confirm fwd on message.", willfwd, willfwd_cmplt, "WILLFWD.VOX", 0, 0},
       {sWONTFWD, "Playing forwarding off message.", wontfwd, wontfwd_cmplt, "WONTFWD.VOX", 0, 0},
       {sFWDFULL, "Playing fwd box full message.", inuse, inuse_cmplt, "FWDFULL.VOX", 0, 0},
       //change greeting
       {sPLAYGRTINFO, "Play greeting intro message.", playgrtinfo, playgrtinfo_cmplt, "GRTINTRO.VOX", 0, 0},//11
       {sRECGRTINFO, "Record greeting intro message.", recgrtinfo, recgrtinfo_cmplt, "RECGRT.VOX", 0, 0},//12
       {sRECGRT, "Recording greeting.", recgrt, recgrt_cmplt, "", 0, 0},
		//paycodes
       {sPAIDTO, "Playing \"You are paid to\" message.", paidtomsg, paidtomsg_cmplt, "PAIDTO.VOX", 0, 0},
       {sGETPAY, "Playing payment message.", getpaymsg, getpaymsg_cmplt, "GETPAY.VOX", 0, 0},
       {sNEWPAY, "Playing \"You are now paid to\" message.", newpaymsg, newpaymsg_cmplt, "NEWPAY.VOX", 0, 0},
       {sPAYDIGIT, "Getting first pay code digit.", getpaydigit, getpaydigit_cmplt, "", 0, 0},
       {sPAYSTR, "Getting pay code.", getpaystr, getpaystr_cmplt, "", 0, 0},
       {sPAIDTODATE, "Playing paid to date.", paidtodate, paidtodate_cmplt, "NEWSUB.VOX", 0, 0}, 
       {sPAYERRINTRO, "Pay error intro.", payerrintro, payerrintro_cmplt, "ERRINTRO.VOX", 0, 0}, 
       {sPAYERRNUM, "Playing pay code.", payerrnum, payerrnum_cmplt, "INVALID.VOX", 0, 0}, 
       //superuser payment menu
       {sBDMENU, "(Admin) Playing pay menu.", adminmsg, adminmsg_cmplt, "BDMENU.VOX", 0, 0}, 
       {sYOUOWE, "(Admin) Playing \"you owe\".", adminmsg, adminmsg_cmplt, "YOUOWE.VOX", 0, 0}, 
       {sDOLLARSNUM, "(Admin) Playing dollars owed.", bdnum, bdnum_cmplt, "ZERO.VOX", 0, 0}, 
       {sDOLLARS, "(Admin) Playing \"dollars\".", adminmsg, adminmsg_cmplt, "DOLLARS.VOX", 0, 0}, 
       {sAND, "(Admin) Playing \"and\".", adminmsg, adminmsg_cmplt, "AND.VOX", 0, 0}, 
       {sCENTSNUM, "(Admin) Playing cents.", bdnum, bdnum_cmplt, "ZERO.VOX", 0, 0}, 
       {sCENTS, "(Admin) Playing \"cents\".", adminmsg, adminmsg_cmplt, "CENTS.VOX", 0, 0}, 
       {sFOR, "(Admin) Playing \"for\".", adminmsg, adminmsg_cmplt, "FOR.VOX", 0, 0}, 
       {sBDOWED, "(Admin) Playing number of billed box days.", bdnum, bdnum_cmplt, "ZERO.VOX", 0, 0}, 
       {sBBOXDAYS, "(Admin) Playing \"billed box days\".", adminmsg, adminmsg_cmplt, "BBOXDAYS.VOX", 0, 0}, 
       {sAT, "(Admin) Playing \"at\".", adminmsg, adminmsg_cmplt, "AT.VOX", 0, 0}, 
       {sPLAYBDVAL, "(Admin) Playing box day value.", bdnum, bdnum_cmplt, "ZERO.VOX", 0, 0}, 
       {sCENTSEACH, "(Admin) Playing \"cents each\".", adminmsg, adminmsg_cmplt, "CENTSEA.VOX", 0, 0}, 
       {sBDNUM, "(Admin) Playing number of unbilled box days.", bdnum, bdnum_cmplt, "NO.VOX", 0, 0}, 
       {sNBOXDAYS, "(Admin) Playing \"unbilled box days\".", adminmsg, adminmsg_cmplt, "NBOXDAYS.VOX", 0, 0}, 
       {sNUMBOXES, "(Admin) Playing number of boxes.", numboxes, numboxes_cmplt, "ZERO.VOX", 0, 0},
       {sBOX, "(Admin) Playing \"box\".", adminmsg, adminmsg_cmplt, "BOX.VOX", 0, 0},
       {sBOXES, "(Admin) Playing \"boxes\".", adminmsg, adminmsg_cmplt, "BOXES.VOX", 0, 0},
       {sBDUPDATEINTRO, "(Admin) Playing \"last updated\".", adminmsg, adminmsg_cmplt, "BDUPDATE.VOX", 0, 0}, 
       {sBDUPDATE, "(Admin) Playing date of last update.", bdupdate, bdupdate_cmplt, "NEWSUB.VOX", 0, 0}, 
       {sGETBDVAL, "(Admin) Get boxday value prompt.", adminmsg, adminmsg_cmplt, "GETBDVAL.VOX", 0, 0}, 
       {sGETBDVALDIGIT, "(Admin) Getting boxday value.", pdtodigit, pdtodigit_cmplt, "", 0, 0}, 
       {sGETBDVALSTR, "(Admin) Getting boxday value.", getbdval, getbdval_cmplt, "", 0, 0}, 
       {sBBDREVERT, "(Admin) Billed box days reverted to unbilled box days.", adminmsg, adminmsg_cmplt, "BDREVERT.VOX", 0, 0}, 
       {sBBDEDIT, "(Admin) Edit billed box days.", adminmsg, adminmsg_cmplt, "BBDEDIT.VOX", 0, 0}, 
       {sBBDSIL, "(Admin) Edit billed box days.", adminmsg, adminmsg_cmplt, "SILENCE.VOX", 0, 0}, 
       {sBBDCLEAR, "(Admin) Billed box days cleared.", adminmsg, adminmsg_cmplt, "BBDCLEAR.VOX", 0, 0}, 
       {sBDEDIT, "(Admin) Edit unbilled box days.", adminmsg, adminmsg_cmplt, "BDEDIT.VOX", 0, 0}, 
       {sBDSIL, "(Admin) Edit unbilled box days.", adminmsg, adminmsg_cmplt, "SILENCE.VOX", 0, 0}, 
       {sBDCLEAR, "(Admin) Unbilled box days cleared.", adminmsg, adminmsg_cmplt, "BDCLEAR.VOX", 0, 0}, 
       {sBDDIGIT, "(Admin) Get box day digit.", bddigit, bddigit_cmplt, "", 0, 0}, 
       {sBDRESTORE, "(Admin) Billing parameters restored.", adminmsg, adminmsg_cmplt, "BDRESTOR.VOX", 0, 0}, 
       {sEDPDTO, "(Admin) Edit paid to date intro.", adminmsg, adminmsg_cmplt, "EDPDTO.VOX", 0, 0}, 
       {sPDTODIGIT, "(Admin) First digit of paid to string.", pdtodigit, pdtodigit_cmplt, "", 0, 0}, 
       {sPDTOSTR, "(Admin) Rest of paid to string.", pdtostr, pdtostr_cmplt, "", 0, 0}, 
       //messages 
       {sNOMSGS, "No messages message.", nomsgs, nomsgs_cmplt, "NOMSGS.VOX", 0, 0},//13
       {sNEWMSGS, "New messages message.", nomsgs, nomsgs_cmplt, "NEWMSGS.VOX", 0, 0},//13
       {sLASTMSG, "Last message message.", lastmsg, lastmsg_cmplt, "LASTMSG.VOX", 0, 0},//14
       {sDELETEDMSG, "Message deleted message.", lastmsg, deletedmsg_cmplt, "DELMSG.VOX", 0, 0},//15
       {sWASDELETED, "Message had been deleted message.", lastmsg, wasdeleted_cmplt, "WASDEL.VOX", 0, 0},//15
       {sRESTOREDMSG, "Message restored message.", lastmsg, lastmsg_cmplt, "RESTMSG.VOX", 0, 0},//16
       {sDELETE, "Deleting messages.", delall, delall_cmplt, "DELETE.VOX", 0, 0},//17
       {sRESTORE, "Restoring messages.", delall, delall_cmplt, "RESTORE.VOX", 0, 0},//18
       {sDATEINTRO, "Message recorded on...", dateintro, dateintro_cmplt, "MSGREC.VOX", 0, 0},//19
       {sDATE, "Playing message date.", date, date_cmplt, "", 0, 0},//20
       // admin states
       // as above for geninstr but 7) starts admin menu below
       {sADMINTRO, "(Admin) Main user menu.", geninstr, geninstr_cmplt, "ADMINTRO.VOX", 0, 0},//19
       // 1) set reminder
       // 2) play reminder
       // 3) record reminder
       // 4) remove reminder
       // 9) delete box
       // #) change msg box security code
       {sADMIN, "(Admin) Administrator functions menu.", admin, admin_cmplt, "ADMIN.VOX", 0, 0},//20
       {sSETRMD, "(Admin) Adding box reminder.", adminmsg, adminmsg_cmplt, "REMBOX.VOX", 0, 0},//21
       {sCLRRMD, "(Admin) Removing box reminder.", adminmsg, adminmsg_cmplt, "CLRBOX.VOX", 0, 0},//22
       {sPLAYRMD, "(Admin) Play new reminder intro.", adminmsg, adminmsg_cmplt,  "PLAYRMD.VOX", 0, 0},//23
       {sNEWRMD, "(Admin) Play new reminder.", newrmd, newrmd_cmplt,  "", 0, 0},//24
       {sRECRMDMSG, "(Admin) Record new reminder intro", adminmsg, adminmsg_cmplt, "RECRMD.VOX", 0, 0},//24
       {sRMALLMNU, "(Admin) Remind all menu.", adminmsg, adminmsg_cmplt, "RMALLMNU.VOX", 0},
       {sRMALLON, "(Admin) Remind all feature on.", adminmsg, adminmsg_cmplt, "RMALLON.VOX", 0},
       {sRMALLOFF, "(Admin) Remind all feature off.", adminmsg, adminmsg_cmplt, "RMALLOFF.VOX", 0},
       {sCHECKDEL, "(Admin) Getting message box to delete.", adminmsg, adminmsg_cmplt, "DELBOX.VOX", 0, 0},//25
       {sDELBOX, "(Admin) Confirming message box to delete.", adminmsg, adminmsg_cmplt, "CONFIRM.VOX", 0, 0},//25
       {sADMINDIGITS, "(Admin) Analysing multiple digits.", admindigits, admindigits_cmplt, "", 0, 0},
       {sRECRMD, "(Admin) Recording new reminder message.", recrmd, recrmd_cmplt, "", 0, 0},
       {sBOXCODE, "(Admin) Getting box to change code.", adminmsg, adminmsg_cmplt, "BOXCODE.VOX", 0, 0},//26
       {sCHGCODE, "(Admin) Getting new code for box.", adminmsg, adminmsg_cmplt, "CHGCODE.VOX", 0, 0},//27
       {sCONFCHGCODE, "(Admin) Confirming new code.", adminmsg, adminmsg_cmplt, "CONFRMPN.VOX", 0},
       {sBROWSE, "(Admin) Browse menu.", browse, browse_cmplt, "BROWSE.VOX", 0},
       {sTOGBRWS, "(Admin) Toggle browse feature.", adminmsg, adminmsg_cmplt, "TOGBRWS.VOX", 0},
       {sBRWSON, "(Admin) Browsing on message.", adminmsg, adminmsg_cmplt, "BRWSON.VOX", 0},
       {sBRWSOFF,"(Admin) Browsing off message.", adminmsg, adminmsg_cmplt, "BRWSOFF.VOX", 0},
       //sys box
       // get code
       // sys main menu:
       // 1) box properties (asks for box number)
       // 		1) Play properties
       //		4) toggle reminder
       //		7) new adminbox
       //		8) new boxtype
       //		9) toggle active or inactive
       //		#) new security code
       // 2) listen to announcement 
       // 3) record announcement
       // 6) toggle announcement on / off
       // *) back to start	
	   //sys intro - stats
	   {sNUM, "(Sys) Stats message - number of...", num, num_cmplt, "ZERO.VOX", 0, 0},
	   {sUSERS, "(Sys) Stats message - users.", statmsg, statmsg_cmplt, "USERS.VOX", 0, 0},
	   {sMINBUSY, "(Sys) Stats message - minutes busy.", statmsg, statmsg_cmplt, "MINBUSY.VOX", 0, 0},
	   {sMAXWAIT, "(Sys) Stats message - max wait.", statmsg, statmsg_cmplt, "MAXWAIT.VOX", 0, 0},
	   {sCALLS, "(Sys) Stats message - calls.", statmsg, statmsg_cmplt, "CALLS.VOX", 0, 0},
	   {sMSGS, "(Sys) Stats message - messages.", statmsg, statmsg_cmplt, "MSGS.VOX", 0, 0},
	   {sPGCALLS, "(Sys) Stats message - page calls.", statmsg, statmsg_cmplt, "PGCALLS.VOX", 0, 0},
	   {sPPCUSED, "(Sys) Stats message - paycodes.", statmsg, statmsg_cmplt, "PPCUSED.VOX", 0, 0},
       {sSYSCODE, "(Sys) Get sys box sec code.", syscode, syscode_cmplt, "SYSCODE.VOX", 0, 0},//28
       //sysmenu
       {sSYSMENU, "(Sys) System functions menu.", sysmenu, sysmenu_cmplt, "SYSMENU.VOX", 0, 0},//28
       {sBOXPROPS, "(Sys) Get box number for box properties.", adminmsg, adminmsg_cmplt, "BOXPROPS.VOX", 0, 0},//28
       {sRECANNC, "(Sys) Record announcement prompt.", recannc, recannc_cmplt, "RECANNC.VOX", 0, 0},//31
       {sANNCON, "(Sys) Announcement turned on.", bup, bup_cmplt, "ANNCON.VOX", 0, 0},//31
       {sANNCOFF, "(Sys) Announcement turned off.", bup, bup_cmplt, "ANNCOFF.VOX", 0, 0},//31
       {sACTANNC, "(Sys) Activate announcement.", act, act_cmplt, "ACTANNC.VOX", 0, 0},//31
       {sNEWANNC, "(Sys) Recording announcement.", newannc, newannc_cmplt, "", 0, 0},//31
       {sNOANNC, "(Sys) No announcement prompt.", bup, bup_cmplt, "NOANNC.VOX", 0, 0},//32
       {sHEARANNC, "(Sys) Hear announcement prompt.", hearannc, hearannc_cmplt, "HEARANNC.VOX", 0, 0},//32
       {sPLAYNEWANNC, "(Sys) Playing new announcement.", playnewannc, playnewannc_cmplt, "", 0, 0},//32
       {sFORCEBDUPDATE, "(Sys) Force boxday update of admin boxes.", adminmsg, adminmsg_cmplt, "FRCBDUP.VOX", 0, 0},//32
       {sSYSTIMEINTRO, "(Sys) Play \"system date & time\".", adminmsg, adminmsg_cmplt, "SYSTIME.VOX", 0, 0},//32
       {sPLAYSYSDATE, "(Sys) Play system date.", systime, systime_cmplt, "", 0, 0},//32
       {sPLAYSYSTIME, "(Sys) Play system time.", systime, systime_cmplt, "", 0, 0},//32
       {sEDSYSTIME, "(Sys) Edit system time prompt.", adminmsg, adminmsg_cmplt, "EDTIME.VOX", 0, 0},//32
       {sSYSTIMEDIGIT, "(Sys) Get 1st digit of system time.", systimedigit, systimedigit_cmplt, "", 0, 0},//32
       {sSYSTIMESTR, "(Sys) Get system time string.", systimestr, systimestr_cmplt, "", 0, 0},//32
       {sSYSTIMEERR, "(Sys) Play system time error message.", adminmsg, adminmsg_cmplt, "NOTIME.VOX", 0, 0},//32
       //box properties menu
       {sBOXPROPSMENU, "(Sys) Box properties menu.", adminmsg, adminmsg_cmplt, "BOXPRMNU.VOX", 0, 0},//28
       {sNOFEAT, "(Admin) Feature not available.", adminmsg, adminmsg_cmplt, "NOFEAT.VOX", 0, 0},//21
       {sPLAYBOXPROPS, "(Sys) Play box properties.", adminmsg, adminmsg_cmplt, "PLAYPROP.VOX", 0, 0},//28
       {sPLAYADMN, "(Sys) Play box admin box.", playadmn, playadmn_cmplt, "BOXADMN.VOX", 0, 0},//28
       {sNO, "(Sys) No...", no, no_cmplt, "NO.VOX", 0, 0},//28
	   {sBOXSTAT, "(Sys) Play box statistic.", boxstat, boxstat_cmplt, "ZERO.VOX", 0, 0},
	   {sBOXLASTCALL, "(Sys) Date of last call.", boxstatmsg, boxstatmsg_cmplt, "LASTCALL.VOX", 0, 0},
	   {sBOXPAIDTO, "(Sys) Paid to date.", boxstatmsg, boxstatmsg_cmplt, "PAIDTO.VOX", 0, 0},
	   {sBOXCALLS, "(Sys) Number of calls.", boxstatmsg, boxstatmsg_cmplt, "BOXCALLS.VOX", 0, 0},
	   {sBOXMSGS, "(Sys) Number of messages.", boxstatmsg, boxstatmsg_cmplt, "BOXMSGS.VOX", 0, 0},
       {sOLDPROPS, "(Sys) Properties set to original values.", adminmsg, adminmsg_cmplt, "OLDPROPS.VOX", 0, 0},//28
       //RMORD = "CSAXY "
       {sTOGRMD, "(Sys) Toggle reminder.", togprop, togprop_cmplt, "TOGRMD.VOX", 0, 0},
       {sRMALL, "(Sys) New box reminder: C.", rmd, rmd_cmplt, "RMALL.VOX", 0, 0},
       {sSYSRM, "(Sys) New box reminder: S.", rmd, rmd_cmplt, "SYSRM.VOX", 0, 0},
       {sADRM, "(Sys) New box reminder: A.", rmd, rmd_cmplt, "ADRM.VOX", 0, 0},
       {sNEVRM, "(Sys) New box reminder: X.", rmd, rmd_cmplt, "NEVRM.VOX", 0, 0},
       {sADMRM, "(Sys) New box reminder: Y.", rmd, rmd_cmplt, "ADMRM.VOX", 0, 0},
       {sNORM, "(Sys) New box reminder: .", rmd, rmd_cmplt, "NORM.VOX", 0, 0},

       {sNEWADMIN, "(Sys) Getting new admin box for box.", adminmsg, adminmsg_cmplt, "NEWADMN.VOX", 0, 0},//27
       {sCONFNEWADMIN, "(Sys) Confirming new admin box.", adminmsg, adminmsg_cmplt, "CONFADMN.VOX", 0},

       {sTOGTYPE, "(Sys) Toggle box type.", togprop, togprop_cmplt, "TOGTYPE.VOX", 0, 0},
       {sMSGTYPE, "(Sys) New box type: btMSG.", type, type_cmplt, "MSGTYPE.VOX", 0, 0},
       {sNEWTYPE, "(Sys) New box type: btNEW.", type, type_cmplt, "NEWTYPE.VOX", 0, 0},
       {sDEFTYPE, "(Sys) New box type: btDEF.", type, type_cmplt, "DEFTYPE.VOX", 0, 0},
       {sADMNTYPE, "(Sys) New box type: btADMN.", type, type_cmplt, "ADMNTYPE.VOX", 0, 0},
       {sPAYCTYPE, "(Sys) New box type: btPAYC.", type, type_cmplt, "PAYCTYPE.VOX", 0, 0},
       {sBBSTYPE, "(Sys) New box type: btBBS.", type, type_cmplt, "BBSTYPE.VOX", 0, 0},
       {sISACTV, "(Sys) Box activated.", actresult, actresult_cmplt, "ISACTV.VOX", 0, 0},
       {sNOTACTV, "(Sys) Box deactivated.", actresult, actresult_cmplt, "NOTACTV.VOX", 0, 0},
       {sONHOLD, "(Sys) Box put on hold.", actresult, actresult_cmplt, "ONHOLD.VOX", 0, 0},
       /* exit states */
       {sRESET, "Reseting inactive lines.", reset, reset_cmplt, "", 0, 0},
       {sHANGUP, "Playing hang up message.", hangup, hangup_cmplt, "HANGUP.VOX", 0, 0},
       {sONHK, "Hanging up.", onhk, onhk_cmplt, "", 0, 0},
       {sINVALIDSTATE, "ERROR: INVALID STATE FOUND!",invalidstate,invalidstate_cmplt,"NOPROMPT.VOX",0,0},
};

void main(int,char **);
void (*kbfunc)();		      //to allow for alternate key processing
int firstprompt(int channel); //selects intro prompt when accessing a vm box
int addpage(US *u);           //add page to paging queue
int getboxnumber(int channel); //find a new box within a defined range
int validphone(char * p);     //is this a valid number for paging?

/**
 **   Global data
 **/

RWB d4xrwb = {0};             /* read/write block for the D/4x */
CSB csb = {0};                /* call status block */
EVTBLK evtblk;                /* event data block               */
CPB cpb; 					  // channel parameter block
char helloedge = TRUE;        // determines when connect is detected

char *progname;  /* program name */
char fullprogname[80] = "";
char sysphone[PHONELEN + 1] = SYSPHONE;
char allowpage = NOTFOUND;    //determines the paging channel
char pagingon = TRUE;		  //allow paging (set to FALSE to turn paging feature off)

#define PATHP "DIR="
char cwd[PATHLEN + 10]; //working directory (includes length of PATHP)

//statistical variables
enum statviews {svCALLS, svMSGS, svPGCALLS, svFFREQ, svWAIT, svFULL, svPAYCODE};
enum svformats {fNUMBER, fMINSEC};
#define svFIRST svCALLS
#define svLAST  svPAYCODE
 
int chansinuse = 0; //number of channels in use at any given time
int pgcalls[24] = {0}; //cumulative pages
int pagecalls = 0; //number of pages
int chanwait[24] = {0}; //maximum ammount of time to wait for free channel
int chansfull[24] = {0}; //seconds all channels have been in use per hour
int chansffreq[24] = {0};

int cwait = 0;	//current wait time when all channels are full
int maxwait = 0; //maximum wait time during this hour
int cfull = 0; //accumulated time when all channels are full
int cffreq = 0; //how often all channels are full
int ttlfull = 0;
int ttlffreq = 0;

int maxcallshr = 0; // largest number of calls in an hour
int maxmsgshr = 0; // largest # msgs per hour

int maxhr = 0; // hour which had largest number of calls
int maxmhr = 0; // hour with largest # of messages
int hour = NOTFOUND; //current hour

int ttlcalls = 0; /* number of calls per hour */
int ttlmsgs = 0; // number of messages per hour

int cperh[24] = {0}; /* keep track of calls for the last 24 hours */
int mperh[24] = {0}; //messages left per hour

char showingcperh = FALSE; //calls per hour window showing?

int scomp = 0; // turns record silence compression on
int maxchan = MAXCHAN, /* max possible channels in system */
    chan = 0;                 /* number of D/4x channels in system */
int d4xintr = DEFINTR;        /* default d/4x hardware interrupt */
int numusers = 0;             /* number of boxes in system */
int maxring = DEFRING;        /* number of rings to wait before picking up */
unsigned char maxiteration = 0;
long newboxes,         /* how many new boxes could fit into avail disk space */
     maxboxes,         /* # boxes that could fit on empty disk */
     boxsize,          /* cust record and message size */
     maxtime = DEFTIME,/* message length in seconds */ 
	 maxmsg = DEFMSG,  /* maximum number of messages per box */
     diskmb = 0L, 	   //total space on disk in mb (taken from command line)
     diskused;         /* amount of disk space taken up by user files */
     
char endprogram = FALSE; /* end program now? */
char force_shutdown = FALSE; /* end program immediately */
char debug = FALSE; /* show event codes and log state transitions */
char isskeleton[BOXLEN + 1] = "0###"; /* used before entering skeletonpin to identify pin as skeletonpin */
char skeletonpin[BOXLEN + 1] = SKELETONPIN; /* "new0" for remote box activation etc */
char defpin[BOXLEN + 1] = ""; /* used for remote activation of box */
char numactstr[10] = ""; /* for block activate: get number to activate */
int numact = 0;          /* actual number used in block activate */
char channelstr[5] = "1"; //for hanging up a single channel
#define BLURB  " Thank you for using LIFELINE.  For further information contact || "
char blurb[] = BLURB;

char fieldbuff[80] = ""; // field clipboard copy should be able to hold 2 max length fields
unsigned int fieldtype = ALPHANUM;

enum actions action = aNOACTION;

char s[400]; //general string for messages
int logfh; //file handle for debug log.

//used with DID can be turned off from command line with "-x"
char tollsaver = TRUE; //wait extra ring if there are no messages

char remotestop = FALSE; //remotely stop program and remove lifeline and user file?
char announce = FALSE; //is there a user announcement? (played after reminder)

//backup variables
typedef struct backupstruct
{
	char hour[3]; //time to initiate backup
	char min[3]; 
	char path[DOSPATHLEN]; //where to back it up to
	char frequency[3]; //time to initiate backup
	char purgedays[3]; 
	time_t lastbackup; //save last back up time
	char  partbackup;
	char remind[3]; //threshold for auto reminder
	char deactivate[3]; //threshold for auto deactivation
}BS;
BS savebup = {0}, backup = {0};
#define LASTBUPSIZE  (sizeof(time_t) + sizeof(char))

long purgedays = 0L; //days to wait before automatically purging messages.
int bupfreq = 0; //back up after how many hours?
long bupremind = 0L;
long bupdeactivate = 0L;
char chkboxes = FALSE; //do automated purge, remind and delete
char purgebox = FALSE; //box needs to be purged?
//when automatically changing state of boxes what will happen
enum bupactions {bupNOACTION, bupDEACT, bupREMIND, bupPURGE};

//data for writestats (util.c)
enum types {RINGS, PGON, ITERS, USERS, MSGS, MAXS};
int MAXSTAT = MAXS - 1;
char statstr[MAXS][80] = {0};
statlen[MAXS] = {0};

//data for showing message info
typedef struct msginfo 
{
long secs;
int Day;
int Month;
int Year;
int Hour;
int Minute;
} MI;
MI nominfo = {0}, minfo[MAXFILES] = {0};
long boxttl; //total amount of time used by box messages

//for system menu
enum sysactions {sysDONOTHING, sysREBOOT, sysSHUTDOWN, sysDESTROY};
int sysaction = sysDONOTHING;
char sysinuse = FALSE;

//for automatic box assignment
int boxrangestart = BXRNGSTART;
int boxrange = BXRNG;
//for boxprops funcs
unsigned char boxpropschanged = FALSE;
//time when active boxes are tallied
int countactivehour = 0;
//box day value in cents
long bdval = 7L;
//buffer for current field
int currfield = 0;
//be strict about whether you have an admin box or not
//won't let records be created without a paidto date or an admin box
char strictadmin = FALSE;
//template created and can be used
char templateavail = FALSE;
//maintenance mode: for when you need to edit the users.dat file manually
//but don't want to shut the system down
char maintenance = FALSE;
// allow browsing?
char canbrowse = FALSE;
// log messages?
char log_newfiles = 0;
/* end of ansrvar.h */










