
//lldate.h - part of ansr3.c

//starting points of number ranges
#define OH 0 	//the "OH" sound in time (eg "twelve oh five" = 12:05)
#define TENS 20
#define TIMENUM TENS
#define AM 24
#define PM 25
#define MONTH 26
#define DIGITSTH 37
#define DATENUM DIGITSTH
#define TENSTH 57
#define OF 59
#define AT 60
#define MASKEND -1
#define MAXNUM AT + 1

//file handle for number file
int numh;

//masks
#define DATEMASKSIZE 7
int datemask[DATEMASKSIZE] = {DATENUM, OF, MONTH, AT, TIMENUM, TIMENUM, AM};
//for array that holds the file date numbers
enum nmsgdate {nDAY, nMONTH, nHOUR, nMIN};

//function definitions
int playtest(int channel); //play all numbers
int playmsgdate(int channel);
int playnum(int channel, int mask[], int num[], int masksize);
int getnumdata(int channel, int mask[], int num[], int masksize);
int gethour(int num, int *is_am);
void addnum(int channel, MS num, int j);



//end lldate.h






