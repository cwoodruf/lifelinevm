
//idxplay.h - part of ansr3.c
/*
english.num:

0  OH
1  one
2  two
3  three
4  four
5  five
6  six
7  seven
8  eight
9  nine
10 ten
11 eleven
12 twelve
13 thirteen
14 fourteen
15 fifteen
16 sixteen
17 seventeen
18 eighteen
19 nineteen
20 twenty
21 thirty
22 fourty
23 fifty
24 sixty
25 seventy
26 eighty
27 ninety
28 hundred
29 thousand
30 million
31 billion
32 trillion
33 AM
34 PM
35 January
36 February
37 March
38 April
39 May
40 June
41 July
42 August
43 September
44 October
45 November
46 December
47 first
48 second
49 third
50 fourth
51 fifth
52 sixth
53 seventh
54 eighth
55 nineth
56 tenth
57 eleventh
58 twelfth
59 thirteenth
60 fourteenth
61 fifteenth
62 sixteenth
63 seventeenth
64 eighteenth
65 nineteenth
66 twentieth
67 thirtieth
68 fourtieth
69 fiftieth
70 sixtieth
71 seventieth
72 eightieth
73 ninetieth
74 hundredth
75 thousandth
76 millionth
77 billionth
78 trillionth
79 of
70 at
81 star
82 pound
83 no
84 zero
85 point
86 percent
*/

//starting points of number ranges
#define OH 0 	//the "OH" sound in time (eg "twelve oh five" = 12:05)
#define INT 1
#define TENS 20
#define MINUTE TENS
#define HOUR (MINUTE + 1)
#define YEARNUM (HOUR + 1)
#define HUNDRED 28
#define THOUSAND 29
#define MILLION 30
#define BILLION 31
#define TRILLION 32
#define AM 33
#define PM 34
#define MONTH 35
#define DIGITSTH 46
#define DATENUM DIGITSTH
#define TENSTH 66
#define HUNDREDTH 74
#define THOUSANDTH 75
#define MILLIONTH 76
#define BILLIONTH 77
#define TRILLIONTH 78
#define OF 79
#define AT 80
#define STAR 81
#define POUND 82
#define MINUS 83
#define ZERO 84
#define POINT 85
#define PERCENT 86
#define MASKEND -1
#define MAXNUM (PERCENT + 1)

#include "idxstrct.h"

//data for index play of message dates etc
/* struct to hold position of currently played message */
typedef struct msgstruct 
{
    unsigned long offset; /* offset into the file */
    unsigned long length; /* length of the message */
}MS;

//data array for playing dates and times
MS numdata[MAXCHAN][30]; //15 is an arbitrary number greater than space needed
#define DATA numdata[CHAN]

#ifndef LLCNVT
//structure for holding information on various prompt files
typedef struct prompttype 
{
	char name[FILELEN + 1]; //root language name
	int ph;					//handle to actual prompts
	int pih;				//handle to prompt index
	int nh;					//handle to numeric prompts
	int nih;				//handle to numeric index
	MS pidx[MAXSTATE];		//prompt position and length parameters from index
	MS nidx[MAXNUM];		//numeric prompt position and length parameters from index
}PT;

PT promptf[MAXPROMPTF] = {0}; //holds multiple multi-prompt files
#endif

char maxlanguage = 0; //actual number of multiprompt files available
char deflanguage = 0; //language to use if there is no indication of which
char findlanguage = 0; //language visible from find record

//end section for multiprompt files    

//for array that holds the file date numbers
enum nmsgdate {nDAY, nMONTH, nHOUR, nMIN};

//masks
#define TIMEMASKSIZE 5
#define TNULL (MAXNUM + 1)
int timemask[TIMEMASKSIZE] = {TNULL, TNULL, HOUR, MINUTE, AM};

#define DATEMASKSIZE 7
int datemask[DATEMASKSIZE] = {DATENUM, OF, MONTH, AT, HOUR, MINUTE, AM};

#define PDTOMASKSIZE 4
#define YEARPOS 2
int pdtomask[PDTOMASKSIZE] = {DATENUM, OF, MONTH, YEARNUM};

int boxmask[BOXLEN] = {INT, INT, INT, INT};
int paycodemask[PAYCODELEN] = {INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT, INT};

#ifndef LLCNVT

//function definitions
int playtest(int channel); //play all numbers
int playmsgdate(int channel);
int playnum(int channel, int mask[], long num[], int masksize);
int playint(int channel, int n);
int playlong(int channel, long n);
int getnumdata(int channel, int mask[], void * num, int masksize);
int gethour(int num, int *is_am);
void addnum(int channel, MS num, int j);

#endif

//end idxplay.h






