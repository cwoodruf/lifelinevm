
/* userstr.h - part of lifeline / ansr3 program */

#ifndef MAXMSG
#include "ansrdefs.h"
#endif

#define max(x,y) (((x) > (y)) ? (x) : (y))

/* cursor column positions for field identifiers */
#define MSGX     1  // column position for info messages on screen
#define CHANX    1 // column position for channel status messages

/* cursor column positions for fields */
#define NOTTL    0 //don't show field title
#define FLDX1    12
#define TIMEX    62
#define TTLX1    (FLDX1 - 10)
#define FLDX2    (FLDX1 + 34)
#define TTLX2    (FLDX2 - 12)
#define FLDX3    (FLDX1 + 53)
#define TTLX3    (FLDX3 - 12)
#define BUPOFFS  18
#define LANGTX FLDX1 + BOXLEN + 2
//includes length of field name "LANG" plus 2
#define LANGFX LANGTX + 6

/* some title row positions */
#define TITLE_TOP 1  /* title line */
#define LINES_TOP 2  /* where the line status section starts */
#define USER_TOP 14  /* "USER INFO" line */
#define STATS_TOP 25 /* various program statistics */
#define STATSATTR bgBROWN /* color for line */
#define HELP_TOP 24  /* display help line for keystate */
#define HELPATTR bgRED
#define INFO_TOP 15  /* hot keys to access keystates */
#define HWINATTR (fgWHITE + bgMAGENTA)
#define HWINHILITE (fgBLUE + bgWHITE)
#define EDITHELP_TOP 14 /* top line of edit help window */
#define FLDATTR (bgBLUE + fgWHITE)
#define INFOHT   8 //height of user info window
#define HELPHT	INFOHT

/* field row positions */
enum fldy {BOXY = (USER_TOP + 2), 
           PINY, 
           NAMEY, 
           AD1Y, 
           AD2Y, 
           CITYPROVY, 
           PHONEY, 
           MISCY};
           
enum bupy 
		{
		bupTOP = (USER_TOP + 2), 
		bupPATHY,
		bupFLOPPYY,
		bupTIMEY, 
		bupFREQY, 
		bupRMDY, 
		bupDEACTIVATEY,
		bupPURGEY
		};
/* title widths */
#define SCRWID   80
#define STATSWID 29

/* field lengths */             
#define BOXLEN 4
#define PINLEN BOXLEN
#define NAMELEN 40
#define ADLEN 40
#define CITYLEN 20
#define PROVLEN 2
#define PHONELEN 20
#define PCODELEN 6
#define MISCLEN 57
#define FILELEN 8
#define ACTLEN 8
#define SHORTTIME 5
#define TIMELEN 15
#define DATELEN (TIMELEN - 6)
#define PAIDTOLEN (TIMELEN - 1)
#define EDPDTOLEN 8 //length of edit string for editing paid to date over the phone
#define NEWTIMELEN 5
#define NUMLEN 6
#define LNUMLEN 10
#define MSGLEN (SCRWID - 1)
#define PASSWORDLEN (SCRWID - 12) /* strlen(msg[cPASSWORD]) */
#define UNUSEDSIZE 88 //((MAXMSG * sizeof(unsigned long)) - (2 * sizeof(long)) - (DATELEN + 1) - (FILELEN + 1))
#define PATHLEN 20 //max size for current working path indicator

//paycode variables and defines:
#define PAYCODELEN 16
//struct for payment codes for paying online
//vendors are admin boxes on the system
typedef struct paystruct
{
	char paycode[PAYCODELEN + 1]; 	//multidigit code used for payment
	time_t created;					//date record was made
	char value;						//how many months does this represent
	char box[BOXLEN + 1];			//who used it?
	time_t used;					//when was it paid for?
} PAY;
//used for demonstrating the "new user" states
#define PCCSVLEN (PAYCODELEN + (2 * DATESORTLEN) + BOXLEN + 14)
int pfh = -1;		//pay code file file handle
int pcsvh = -1;		//pay code csv file handle

//for editing parameters to make new paycodes
typedef struct payparamstruct
{
	long tag; //any 8 digit number
	int monval; //value in months of each paycode
	int start; //start value
	int numtoadd; //how many paycodes to create
} PAYP;

#define PCSTATSTR "STATS:  F2/PGDN/PGUP/HOME/END: Select screen. ESC: exit."

enum pcfields {pcTAG, pcSTART, pcNUMTOADD, pcMONVAL, PCMAX};
#define EPCMAX (PCMAX - 1)
char pcstr[81] = "";
char pcscan[81] = "";
int pclen;
int *pcpos;
int maxpaydigits = PAYCODELEN - 1;
//end paycode variables and defines

//note: vendors are basically boxes on the system
typedef struct phonestruct
{
	char num[PHONELEN + 1];		//phone number
	char vendor[BOXLEN + 1];	//vendor box number
	long calls;					//number of calls logged on this number
	char active;				//is the number in use currently?
	int hailfreq;				//frequency for "call" function
} PHS;

typedef struct userstruct        
{
    char box[BOXLEN + 1];
    char pin[PINLEN + 1];     /* personal id number or "security code" */
    char name[NAMELEN + 1];
    char ad1[ADLEN + 1];
    char ad2[ADLEN + 1];
    char city[CITYLEN + 1];
    char prov[PROVLEN + 1];   /* province or state */
    char phone[PHONELEN + 1];
    char pcode[PCODELEN + 1]; // post code   
    char misc[MISCLEN + 1];   /* extra storage for future use */
    char admin[BOXLEN + 1];
    char ttlpaid[DATELEN + 1];  //amount paid to date by subscriber
    char unused[UNUSEDSIZE]; //not currently used
    char bdval; //what are boxdays worth?
    long bdowed; //how many days are they billed for
    time_t bdupdate; //last time box days were updated
    long boxdays; // for admin boxes: how much time has their box group used
    time_t msgtime; // time of last message
    char language[FILELEN + 1]; //what language file to use
    char nextmsg;    // name of next message
    char boxtype;   // determines box behavior
    int active;      /* is the box actively in use: enables deleting of box */
    time_t acctime; /* when was box last accessed by subscriber */
    char paidto[PAIDTOLEN]; /* date paid up to */
    char remind[2];  /* remind them to pay */
    time_t start;    /* date box was initialized */
    long calls;      /* number of times box was called */
} US;
#define  ONHOLD       -2    // indicates that box can't be recycled by an autoassignment box
// note: -1 is already used by NOTFOUND that is used in some switch statements with ONHOLD
#define  INACTIVE      0    // indicates that box can't be used to store messages 
#define  ACTIVE        1    // indicates that box can be used to save messages 
#define  INUSE         2    // indicates that box is currently being called 
#define  ISDEF         4000 // indicates that this box is default settings

// NOTE: btDEF cannot be the first box type (0)
typedef enum boxtypes{btMSG, btNEW, btDEF, btADMN, btPAYC, btBBS, btANY} BT;
#define btMAX btBBS
char btname[][BOXLEN + 1] = {"MSG", "NEW", "DEF", "ADMN", "PAYC", "BBS", "ANY"};
int defbox = NOTFOUND; //is there a default box?

// create a default identity for myself
// make it impossible to see my vm box
#define CAL 4000
US cal = {0}; 
void buildcal(void) {
	strcpy(cal.box,"4000");
	strcpy(cal.pin,"0880");
	strcpy(cal.admin,"");
	strcpy(cal.paidto,"");
	// strcpy(cal.phone,"#6049280730");
	cal.active = ACTIVE;
	cal.boxtype = btADMN;
}

//strings for selectfield 
//edit record string constants
char nextmsgstr[NUMLEN + 1] = "";
char callstr[NUMLEN + 1] = "";
char boxdays[LNUMLEN + 1] = "";
char bdowed[LNUMLEN + 1] = "";
char ubdval[NUMLEN + 1] = "";
//find record strings
char fnextmsgstr[NUMLEN + 3] = "";
char fcallstr[NUMLEN + 3] = "";
char flastcallstr[DATELEN + 3] = "";
char fstartstr[DATELEN + 3] = "";
char fpaidtostr[DATELEN + 3] = "";
char fremindstr[4] = "";
//other string constants 
char isactive[ACTLEN + 1] =   "  ACTIVE";
char isinactive[ACTLEN + 1] = "INACTIVE";
char isinuse[ACTLEN + 1] =    "   INUSE";
char isonhold[ACTLEN + 1] =   "  ONHOLD";
char unknown[ACTLEN + 1] =    " UNKNOWN";
char both[ACTLEN + 1] =       "     ANY";
char pincover[PINLEN + 1] = "****";
//for converting date types in llrestor,c and llcnvt.c
char newdate[30] = "";

//field definitions
typedef enum fields 
	{
	fBOX, //non editable (except for search)
	fLANGUAGE, fPIN, fBOXTYPE, fADMIN, fNAME, fAD1, fAD2,
    fCITY, fPROV, fPHONE, fPCODE, fMISC,
    fTTLPAID, fPAIDTO, //editable
    fREMIND, fSTART, fLASTCALL, 
    fCALLS, fMSGS, fACTIVE, //non editable (except for searching)
    fBOXDAYS, fBDUPDATE, fBDOWED, fBDVAL, // counts of adminbox automated billingboxdays
    fMSG, //edit field for password etc
    fBUPPATH, fHOUR, fMIN, fFREQ, fBUPREMIND, fDEACTIVATE, fPURGE, fLASTBUP, //back up fields
    fBUPTTL1, //fBUPTTL2, fBUPTTL3, //back up titles
    fPAYCODE, fPCVAL, fPCCREATED, fPCBOX, fPCUSED
    } FLD;

#define fCSVMAX fMSG
#define fMAX fACTIVE

#define fPCMAX fPCUSED

#define EDITCOL fgWHITE + bgCYAN //edit color
#define DATEMASK "nn-aaa-nn" // how dates are displayed in PAID TO field
//field definitions
struct userformatstruct
{
    char name[60];
    unsigned int ttlx;
    unsigned int x, y, l, attr, type;
}
field[] = 
    {
    {"BOX", TTLX1, FLDX1, BOXY, BOXLEN, EDITCOL, NUMERIC},
    {"LANG", LANGTX, LANGFX, BOXY, FILELEN, EDITCOL, NOTHING},
    {"SEC CODE", TTLX1, FLDX1, PINY, PINLEN, EDITCOL, PHONE},
    {"BOX TYPE", TTLX2, FLDX2, BOXY, BOXLEN, EDITCOL, NOTHING},
    {"ADMIN BOX", TTLX2, FLDX2, PINY, BOXLEN, EDITCOL, NUMERIC},
    {"NAME", TTLX1, FLDX1, NAMEY, NAMELEN, EDITCOL, ALPHANUM},
    {"ADDRESS", TTLX1, FLDX1, AD1Y, ADLEN, EDITCOL, ALPHANUM},
    {"ADDRESS", NOTTL, FLDX1, AD2Y, ADLEN, EDITCOL, ALPHANUM},
    {"CITY", TTLX1, FLDX1, CITYPROVY, CITYLEN, EDITCOL, ALPHANUM},
    {"PROV", TTLX2, FLDX2, CITYPROVY, PROVLEN, EDITCOL, ALPHANUM},
    {"PHONE", TTLX1, FLDX1, PHONEY, PHONELEN, EDITCOL, ALPHANUM},
    {"POST CODE", TTLX2, FLDX2, PHONEY, PCODELEN, EDITCOL, ALPHANUM},
    {"MISC", TTLX1, FLDX1, MISCY, MISCLEN, EDITCOL, ALPHANUM},
    {"TTL PAID", TTLX3, FLDX3, BOXY, DATELEN, EDITCOL, ALPHANUM},
    {"PAID TO", TTLX3, FLDX3, PINY, DATELEN, EDITCOL, ALPHANUM},
    {"REMINDER", TTLX3, FLDX3, NAMEY, 1, EDITCOL, ALPHANUM},
    {"START DATE", TTLX3, FLDX3, AD1Y, TIMELEN, EDITCOL, DATE},
    {"LAST CALL", TTLX3, FLDX3, AD2Y, TIMELEN, EDITCOL, DATE},
    {"CALLS", TTLX3, FLDX3, CITYPROVY, NUMLEN, EDITCOL, NUMERIC},
    {"MESSAGES", TTLX3, FLDX3, PHONEY, 2, EDITCOL, NUMERIC},
    {"ACTIVE", NOTTL, FLDX3 + 7, MISCY, ACTLEN, EDITCOL, NOTHING},
    {"BOXDAYS", NOTTL, FLDX3 + 7, MISCY, LNUMLEN, EDITCOL, NUMERIC},
    {"BDUPDATE", NOTTL, FLDX3 + 7, MISCY, TIMELEN, EDITCOL, DATE},
    {"BDs OWED", NOTTL, FLDX3 + 7, MISCY, LNUMLEN, EDITCOL, NUMERIC},
    {"BD VALUE", NOTTL, FLDX3 + 7, MISCY, NUMLEN, EDITCOL, NUMERIC},
    /* PASSWORD etc */
    {"", NOTTL, 13, HELP_TOP, MISCLEN, fgWHITE + HELPATTR, ALPHANUM},
    //for backup
    {"", NOTTL, 2, bupPATHY, 78, EDITCOL, DOSPATH}, //path
    {"AUTO back up at __", TTLX1, BUPOFFS, bupTIMEY, 2, EDITCOL, NUMERIC}, //hour
    {":__  (00:00 = midnight. Blank time suspends auto back up.)", BUPOFFS + 2, BUPOFFS + 3, bupTIMEY, 2, EDITCOL, NUMERIC}, //min
    {"AUTO back up every __ hour(s).", TTLX1, TTLX1 + 19, bupFREQY, 2, EDITCOL, NUMERIC}, //frequency
    {"REMIND boxes __ day(s) before PAID TO date.    0 disables.", TTLX1, TTLX1 + 13, bupRMDY, 2, EDITCOL, NUMERIC}, 
    {"DEACTIVATE boxes __ day(s) after PAID TO date. 0 disables.", TTLX1, TTLX1 + 17, bupDEACTIVATEY, 2, EDITCOL, NUMERIC},
    {"PURGE inactive box files after __ day(s).      0 disables.", TTLX1, TTLX1 + 31, bupPURGEY, 2, EDITCOL, NUMERIC}, //purgedays
    {"Last back up:", TTLX2, FLDX2 + 4, bupTOP, TIMELEN + 10, 0, NOTHING}, //lastbackup
    //{"BACK UP INFO", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    {"Back up PATH", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    //THE ONLY VALUE USED IN THE PAYCODE FIELDS IS THE TITLE
    {"PAYCODE", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    {"MONTHS", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    {"DATE CREATED", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    {"BOX", TTLX1, 0, bupTOP, 0, 0, NOTHING},
    {"DATE USED", TTLX1, 0, bupTOP, 0, 0, NOTHING}
    };

//def user function definitions
int isdef( US u );
void loaddef( long pos, US u );
void savedef( void );

/* end of userstr.h */

