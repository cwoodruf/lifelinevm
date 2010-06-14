
/* util.h - part of ansr3 program */
//types specific to util.c

#define BUFSIZE 4096

enum kb {kbIDLE, kbBACKUP, kbPARTBUP, kbCSVBUP, kbCSVABUP} kbaction = kbIDLE;
#define DOINGBACKUP (kbaction == kbBACKUP || kbaction == kbPARTBUP || kbaction == kbCSVBUP || kbaction == kbCSVABUP)
#define SKIPBACKUP (kbaction == kbSKIPBACKUP)

typedef enum stages 
	{
	bIDLE,
	bWAITDISK,
	bINITUSER,
	bINIT,
	bPAYCSVOPEN,
	bPAYCSVCOPY,
	bPAYCSVCLOSE,
	bDATINIT,
	bDATDEL,
	bDATINITOPEN,
	bDATOPEN,
	bDATCOPY,
	bDATCLOSE,
	bDATNEXT,
	bWAIT,
	bSTARTUSERCOPY,
	bCOPYUSER,
	bWRITEREC,
	bUPDATECOUNT,
	bCHECKRECTYPE,
	bMAKECSV,
	bWRITECSV,
	bINITCOPY,
	bINITDEL,
	bDEL,
	bINITOPEN,
	bOPEN,
	bCOPY,
	bCLOSE,
	bNEXT,
	bCLOSEUSER, 
	bERR,
	bAUTOABORT
	} BSTG;

BSTG bupstage = bIDLE, bSTART = bPAYCSVOPEN;

int buperr = FALSE;
int keypressed = FALSE;

int p_err;
long p_pos;
long p_size;
struct _find_t p_f;
 
//function declarations
void voidport(int channel);
void recycleport(int channel);
int record(int channel, int fh);
int loaddigits(char *dtmf, int channel, int maxdt, int waitsec);
unsigned char waitevt(EVTBLK *evtp);
int play(int channel, int h);
int indexplay(int channel, int state);
int playstate(int channel, int state);
int playsingletone(int channel, int freq, int msec);
int set_hkstate(int channel, int state );
char *fname(char *name, char *ext);
void _vhclose(int handle);
void closermd(int channel);
int checkgrt(int channel);
int opengrt(int channel);
int checkrmd(int channel, char *admin);
int openrmd(int channel);
void checkmsg(int channel);
void removesilence(long *size, int h);
//int playmsgnum(int channel, int mh, unsigned long offset, unsigned long length);
void gettelcochans( void );
void writechanmsg( int channel, unsigned int evtcode, int error, char *extramsg);
unsigned long getusersize( void );
long numavailboxes( void );
unsigned long wastage( void );
void getsizes( void );
long boxleft(int channel);
void writestats( void );
void dobackup( char firstmask, char * bpath );
void autormd(US *u, time_t now);
int openfile(char *fname);
int bupopen(char *targmask, int targlen, char *name, int *hsrc, int *htarg);
void bupclose(int hsrc, int htarg);
int writecsv(int hscsv, int htcsv, char *s, US *u);
int bupcopy ( int hsource, int htarget );
void updatechanmsgs( void );
void loadcperh( void );
void savecperh( void );
//char *cperhstr( void );
//void showcperh( void );
void simulatecalls( void );
void resetports( void );
void getpagechannel( void );
int portsidle( void );
double getevaldays( void );
void regnow( void );
void saveudisplay(unsigned char far *scrnbuff);
void loadudisplay(unsigned char far *scrnbuff);
void checkeveryboxinuse(void);
long getboxsize(char *box);
int sumhrarray(int a[]);
int maxhrarray(int a[]);
int minhrarray(int a[]);

/* end util.h */


