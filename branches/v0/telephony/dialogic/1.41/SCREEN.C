
/* screen.c */
/* screen functions for direct screen writes and editing */

#define SCREEN

#include <bios.h>
#include <time.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>      

#define middle(max,size) (((max) - (size) + 1) / 2)
#define NOTFOUND -1
#define ERROR -1
#define OK 0

#define DOSPATHLEN 256

#define ROWS 50
#define COLUMNS 80

#define COLOR_SCR 0xb8000000L
#define MONO_SCR 0xb0000000L

#define BLOCKCURSOR 0x0007
#define LINECURSOR 0x0607
#define NOCURSOR 0x2000

#define bgBLACK     0
#define bgBLUE      16 
#define bgGREEN     32 
#define bgRED       64 

#define bgWHITE     bgBLUE + bgGREEN + bgRED
#define bgMAGENTA   bgBLUE + bgRED
#define bgBROWN     bgGREEN + bgRED
#define bgCYAN      bgBLUE + bgGREEN

#define fgBLUE      1
#define fgGREEN     2
#define fgRED       4 
#define fgGREY      7
#define fgINTENSITY 8
#define fgWHITE     15
#define BLINK       128

#define TRUE   1
#define FALSE  0
#define BOOLYES "Yes"
#define BOOLNO "No"

#define OVERWRITE 0

/* constants for datatype mask see: checkdatatype */
enum datatypes {
    DOSPATH,
    DOSFILE,
    BOOLEAN,
    NUMERIC,
    NRANGE,
    PHONE,
    ALPHANUM,
    INTEGER,
    REAL,
    DATE,
    NOTHING
};

#define DOSCHARS ":\\.ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_~!#%&-{}()@''`*?"
char doschars[] = DOSCHARS;

/* for extended keys use unsigned char instead of char */
/* zero = true */
#define INSERT 82
#define UP     72
#define DOWN   80
#define LEFT   75
#define RIGHT  77
#define HOME   71
#define END    79
#define BKSPC  8
#define DEL    83
#define PGUP   73
#define PGDN   81
#define INS    82
/* zero = false */
#define TAB    9
#define ESC    27
#define ENTER  13
#define SPACE  32

#define CtrlA 1
#define CtrlB 2
#define CtrlC 3
#define CtrlD 4
#define CtrlE 5
#define CtrlF 6
#define CtrlG 7
#define CtrlH 8
#define CtrlI 9
#define CtrlJ 10
#define CtrlK 11
#define CtrlL 12
#define CtrlM 13
#define CtrlN 14
#define CtrlO 15
#define CtrlP 16
#define CtrlQ 17
#define CtrlR 18
#define CtrlS 19
#define CtrlT 20
#define CtrlU 21
#define CtrlV 22
#define CtrlW 23
#define CtrlX 24
#define CtrlY 25
#define CtrlZ 26

/* zero = true */
#define F1     59
#define F2     60
#define F3     61
#define F4     62
#define F5     63
#define F6     64
#define F7     65
#define F8     66
#define F9     67
#define F10    68

/* zero = true */
#define AltF1  104
#define AltF2  105
#define AltF3  106
#define AltF4  107
#define AltF5  108
#define AltF6  109
#define AltF7  110
#define AltF8  111
#define AltF9  112
#define AltF10 113

/* zero = true */
#define CtrlF1 94
#define CtrlF2 95
#define CtrlF3 96
#define CtrlF4 97
#define CtrlF5 98
#define CtrlF6 99
#define CtrlF7 100
#define CtrlF8 101
#define CtrlF9 102
#define CtrlF10 103

#define AltA   30
#define AltB   48
#define AltC   46
#define AltD   32
#define AltE   18
#define AltF   33
#define AltG   34
#define AltH   35
#define AltI   23
#define AltJ   36
#define AltK   37
#define AltL   38
#define AltM   50
#define AltN   49
#define AltO   24
#define AltP   25
#define AltQ   16
#define AltR   19
#define AltS   31
#define AltT   20
#define AltU   22 
#define AltV   47
#define AltW   17
#define AltX   45
#define AltY   21
#define AltZ   44

#define ShiftTAB 15 
#define CtrlPGUP 132
#define CtrlPGDN 118
#define CtrlEND  117
#define CtrlHOME 119
#define CtrlLEFT  115
#define CtrlRIGHT 116

/* end of key defs */

/* box styles for create_win */
#define bsNOBORDER -1
#define bsSINGLE 0
#define bsDOUBLE 1
#define bsMAXSTYLE 2

#define NEWLINE '|' /* for writing a mask array to a window see: create_win */
unsigned char fillchar = SPACE;

/* screen height and width variables */
#define MAXHEIGHT 25
#define MAXWIDTH 80

typedef struct cell
{
    char c;
    char attr;
} CELL;

CELL far *scr; /* pointer to screen using cell struct */

int maxheight = MAXHEIGHT;
int maxwidth = MAXWIDTH;

unsigned char scrnbuff[2 * MAXHEIGHT * MAXWIDTH];

unsigned char boolkey = SPACE; // assignable key for boolean data type
unsigned char key; /* key keeps the key value; */
char zero; /* zero tracks whether there is an initial zero */

int shadow = TRUE; /* make a shadow around box */
char typemode = INSERT; /* keeps track of how text is typed in
                           INSERT and OVERWRITE are the two values */
int cursortype = LINECURSOR; /* appearance of cursor (relates to typemode) */

/* text justify in window */
enum justify {jLEFT, jRIGHT, jCENTER};
int justifytext = jLEFT; 

long scrtype = COLOR_SCR;    /* start address of text screen */

/* for mktimestr, timestr funcs */
#define DATESORTLEN 19
char *months[] = 
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
// for getsecs func
int lmonth[] = 
{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int llymonth[] =
{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define MINYEAR		1970L
#define MINSECS		60L
#define HOURSECS	3600L
#define DAYSECS		(24L * HOURSECS)

time_t lmonsecs[] =
	{31 * DAYSECS, 
	 59 * DAYSECS, 
	 90 * DAYSECS, 
	 120 * DAYSECS, 
	 151 * DAYSECS, 
	 181 * DAYSECS, 
	 212 * DAYSECS, 
	 243 * DAYSECS, 
	 273 * DAYSECS, 
	 304 * DAYSECS, 
	 334 * DAYSECS, 
	 365 * DAYSECS};

time_t llymonsecs[] =
	{31 * DAYSECS, 
	 60 * DAYSECS, 
	 91 * DAYSECS, 
	 121 * DAYSECS, 
	 152 * DAYSECS, 
	 182 * DAYSECS, 
	 213 * DAYSECS, 
	 244 * DAYSECS, 
	 274 * DAYSECS, 
	 305 * DAYSECS, 
	 335 * DAYSECS, 
	 366 * DAYSECS};

#define COVERCHAR '*'
char coverchar = 0; /* cover character for passwords etc */

#define EDHELPLEN 1000                                         
char *edithelpptr = NULL; /* points to current edit help string */
char edithelpstr[EDHELPLEN] = /* default edit help routine string */
" EDIT HELP:| Left and right arrows/home/end to move cursor in string.| Backspace/Del to delete. Ctrl-Y to clear string.| Enter/Esc to exit.";
int edithelptop = 0; /* top line of edit help window */
int helpavail = TRUE; /* determine if help screen can be shown */
int helpon = FALSE; /* indicates if help screen is showing */
int edithelpattr = 0; //help color: 0 means use default color

//search criteria
#define WILDCARD '?' /* any nonzero field */
#define EMPTY '~' /* any zero field */
#define GREATER '>' //compare field with following string
#define LESSER '<' //compare field with following string
#define TAG '@' //indicates that field is using field identifier (tag) as following string

int usewildcard = FALSE; /* use wild card char or negation char? */

typedef struct window_properties
	{
		int top;
		int left;
		int height;
		int width;
	}WP;
	
// some function definitions
long getscrtype( void );
void getlinestats(char *mask, 
                  int *newlines, 
                  int *maxlength);
unsigned char far *scrnaddress(int x, int y);
void showmask(int c, int r, int color, char m, int *j);
void shade(int x, int y);
void makeborder(unsigned int x, 
                unsigned int y, 
                unsigned int width, 
                unsigned int height, 
                int color, 
                int style);
void createshadows(int x, int y, int width, int height);
void savescrn(unsigned char far *scrnbuff);
void loadscrn(unsigned char far *scrnbuff);
void cls( void );
void create_win(int x,
                int y,
                int width,
                int height,
                int color,
                int style,
                char *mask);
void writestr(int x, int y, int p, int l, int attrib, char *s);
char *mktimestr(time_t *datetime, char *datedivider, char *timedivider, int style);
char *datestr(time_t *datetime);
char *timestr(time_t *datetime);
char *datesortstr(time_t *datetime);
long getdatesecs(int d, int m, int y);
long gettimesecs(int s, int m, int h, int d, int mo, int y);
int getdate(char *str, int *day, int *mon, int *yr);
WP strwin(int attrib, int justify, char *str);
void showedithelp(int attrib);
int checkdatatype(unsigned char *key, int datatype);
void getkey(unsigned char *key, char *zero);
int yes( void );
int edit(int x, 
         int y, 
         int l, 
         int *startpos,
         int *pos, 
         int lenofstr,
         int attrib, 
         char key, 
         char zero,  
         int datatype,
         char *s);

/**************************************************************************/
//for removing lifeline program errors (ansr3.c etc) only
void awritemsg(char * s) //write user help message
{
	//write the message
	writestr(2, /* x pos */
			 24,  /* y pos */
			 0,   /* start in string at */
			 80, /* size of window */
			 bgRED + fgWHITE, /* color */
			 ((s[0] <= 32 && s[0] > 0) ? s + 1: s)); 
			 /* string: increment if first char is whitespace */
}

//use below for error check purposes only!!!!
void awritech(char * s)
{
	awritemsg(s);
	getch();
}
//end error check funcs

/* get correct text address relative to video mode */
long getscrtype( void )
{
    unsigned list;
    list = _bios_equiplist();
    
    /* check for bit 5 = black and white */
    if (list & 16) scrtype = MONO_SCR;
    else scrtype = COLOR_SCR;
    
    return scrtype;
}

/* get parameters for sizing a window from one of my formatted strings */
void getlinestats(char *mask, 
                  int *newlines, 
                  int *maxlength)
{
    int nl = 1;
    int ml = 0;
    int sl;
    register int i;
    *newlines = 0;
    *maxlength = 0;
    sl = strlen(mask) - 1;
    for(i = 0; i <= sl; i++) {
        ml++;
        if (mask[i] == NEWLINE || i == sl) {
            if (i != sl) nl++;
            if (ml > *maxlength) *maxlength = ml;
            ml = 0;
        }
    }

    *newlines = nl;
}             

/* direct screen write window */
unsigned char far *scrnaddress(int x, int y)
{
    if (y < 1) y = 1;
    if (y > maxheight) y = maxheight;
    if (x < 1) x = 1;
    if (x > maxwidth) x = maxwidth;
    return (char far *) (getscrtype() + 160 * (y - 1) + 2 * x - 2);  
}

/* actually write the window mask to the window */
void showmask(int c, int r, int color, char m, int *j)
{
    switch (m) {
         case NEWLINE:
         case '\0': 
            *(scrnaddress(c, r)) = fillchar;
         break;
         default:
            *(scrnaddress(c, r))= m;
            (*j)++; 
    }
    *(scrnaddress(c, r) + 1) = color;
}

/* make shading for window */
void shade(int x, int y)
{                                              
    int shadow = 7;
    int blink = 128;

    if (*(scrnaddress(x, y) + 1) >= blink)
        *(scrnaddress(x, y) + 1) = shadow + blink;
    else *(scrnaddress(x, y) + 1) = shadow;
}

/* draw a line around window if needed */
void makeborder(unsigned int x, 
                unsigned int y, 
                unsigned int width, 
                unsigned int height, 
                int color, 
                int style)
{
    register unsigned int c, r;
    enum stylepositions
    {
        top_left,
        top_right,
        bottom_left,
        bottom_right,
        top_bottom,
        sides
    };
    
    unsigned char stylelist[bsMAXSTYLE][6] =
                  {{218,191,192,217,196,179}, /* single line */
                   {201,187,200,188,205,186}}; /* double line */
    
    unsigned int r1 = y - 1;
    unsigned int r2 = y + height; 
    
    for (c = x - 1; c <= x + width; c++)            
    {
        *(scrnaddress(c, r1) + 1) = color;
        if (c == x - 1) 
            *(scrnaddress(c, r1)) = stylelist[style][top_left];
        else if (c == x + width) 
                *(scrnaddress(c, r1)) = stylelist[style][top_right];
        else *(scrnaddress(c, r1)) = stylelist[style][top_bottom];        
        
        *(scrnaddress(c, r2) + 1) = color;
        if (c == x - 1) 
            *(scrnaddress(c, r2)) = stylelist[style][bottom_left];
        else if (c == x + width) 
                *(scrnaddress(c, r2)) = stylelist[style][bottom_right];
        else *(scrnaddress(c, r2)) = stylelist[style][top_bottom];        
    }
        
    for (r = y; r <= y + height - 1; r++) 
    {
        *(scrnaddress(x - 1, r) + 1) = color;
        *(scrnaddress(x + width, r) + 1) = color;
        
        *(scrnaddress(x - 1, r)) = stylelist[style][sides];
        *(scrnaddress(x + width, r)) = stylelist[style][sides];
    }        

    if(shadow)
    {
	    for (c = x - 1; c <= x + width; c++)            
	        shade(c + 2, r2 + 1); /* create bottom shadow */
	    for (r = y; r <= y + height; r++) 
	    {
	        shade(x + width + 1, r); /* create side shadow */
	        shade(x + width + 2, r);
	    }
	}
}

void createshadows(int x, int y, int width, int height)
{
    register int i;                              
    int shadow = 7;
    
    if (y + height <= maxheight) 
        for (i = x + 2; i <= x + width - 1; i++) shade(i, y + height); 
    
    if (x + width <= maxwidth)     
        for (i = y + 1; i <= y + height; i++)
            if (i <= maxheight) shade(x + width, i);
    
    if (x + width + 1 <= maxwidth)     
        for (i = y + 1; i <= y + height; i++)
            if (i <= maxheight) shade(x + width + 1, i);
}

/* put current screen contents into buffer */
void savescrn(unsigned char far *scrnbuff)
{
    unsigned char far *scrn;
    scrn = scrnaddress(1,1);
    
    _fmemmove(scrnbuff, scrn, (sizeof(CELL) * MAXHEIGHT * MAXWIDTH) - 1);
}

/* write to screen from buffer */
void loadscrn(unsigned char far *scrnbuff)
{
    unsigned char far *scrn;
    scrn = scrnaddress(1,1);
    
    _fmemmove(scrn, scrnbuff, (sizeof(CELL) * MAXHEIGHT * MAXWIDTH) - 1);
}

void cls( void )
{
    /* clear text screen */
    _clearscreen(_GCLEARSCREEN);
}

void create_win(int x,
                int y,
                int width,
                int height,
                int color,
                int style,
                char *mask)
{
    register int c, r; /* column and row variables */
    int j = 0; /* place in mask string */
    int len, startpos;
    
    /* make borders if needed */
    if (style != bsNOBORDER) 
        makeborder(x, y, width, height, color, style);
    else createshadows(x,y,width,height);
    
    if (mask == NULL)
    {
    	for (c = x; c < x + width; c++)
		    for (r = y; r < y + height; r++)
		    {
			    *(scrnaddress(c, r) + 1) = color;
				*(scrnaddress(c, r)) = fillchar;
			}
	}
    else
    for (r = y; r <= y + height - 1; r++) {
    
        /* find length of next fragment for justification */
        for (c = j,len = 0; 
             c < j + width && mask[c] != NEWLINE && mask[c] != '\0'; 
             c++,len++);
             
        switch (justifytext)
        {
            case jRIGHT:  startpos = x + width - len;
            break;
            case jCENTER: startpos = x + ((width - len) / 2);
            break;
            case jLEFT: startpos = x;
            break;
        }
        
        for(c = startpos; c < x + width; c++)
            showmask(c, r, color, mask[j], &j);
            
        if (mask[j] == NEWLINE) j++;
        
    }
}

/****************************************************************************/
/* function for direct screen write of strings */
void writestr(int x, int y, int p, int l, int attrib, char *s)
/*
    x = current column
    y = current row
    p = starting point in string
    l = length of window
    attrib = color
    *s = points to string in memory
*/
{
    register i, j;              
    char fill = FALSE;
    
    char far *scrn = (char far *) (getscrtype() + 160 * (y - 1) + 2 * (x - 1)); 
    l--; /* make sure length does not exceed end of string */ 
    
    if (y > ROWS) y = ROWS;
    
    if (x + l > COLUMNS) l = COLUMNS - x;
    
    i = 0;
    j = 0; 
    while (j <= l)
    {               
        if (s[j + p] == '\0') break;
        else if (coverchar == 0)
            *(scrn + i) = s[j + p];   
        /* if you don't want to show what you are typing */    
        else *(scrn + i) = coverchar;     
        if (attrib > 0) /* leave color as is if attrib is zero */
            *(scrn + i + 1) = attrib;
        i += 2;
        j++;
    }       
                
    while (j <= l)
    {
        *(scrn + i) = fillchar;
        *(scrn + i + 1) = attrib;
        i += 2;
        j++;
    }
}
/****************************************************************************/
void wait(int seconds) {
	time_t now, stop;
	time(&stop);
	stop += seconds;
	do time(&now);
	while (now <= stop);
}
/****************************************************************************/
char *mktimestr(time_t *datetime, char *datedivider, char *timedivider, int style)
{
    static char ts[40];
    struct tm *t;
    char dateonly = FALSE;
    
    strcpy(ts, "");
    if (*datetime == 0L)
    {
        return ts;
    }
    t = localtime(datetime);
    
	if (!strlen(timedivider)) 
		dateonly = TRUE;
	
	switch (style)
	{
		case ALPHANUM:
			sprintf(ts, "%02d%s%s%s%02d%c%02d%s%02d%s%02d", 
						t->tm_mday, 
						datedivider,
						months[t->tm_mon], 
						datedivider,
						t->tm_year % 100,
						(dateonly ? 0 : SPACE),
						t->tm_hour, 
						timedivider,
						t->tm_min, 
						timedivider,
						t->tm_sec);
		break;
		case NUMERIC:
			sprintf(ts, "%04d%s%02d%s%02d%c%02d%s%02d%s%02d", 
						t->tm_year + 1900, 
						datedivider,
						t->tm_mon + 1, 
						datedivider,
						t->tm_mday, 
						(dateonly ? 0 : SPACE),
						t->tm_hour, 
						timedivider,
						t->tm_min, 
						timedivider,
						t->tm_sec);
		break;
	}	    
    return ts;
}

char *datestr(time_t *datetime)
{
    // no time divider tells mktimestr that you don't want time
    return mktimestr(datetime, "-", "", ALPHANUM);
}

char *timestr(time_t *datetime)
{
    return mktimestr(datetime, "-", ":", ALPHANUM);
}

char *datesortstr(time_t *datetime)
{
	return mktimestr(datetime, "/", ":", NUMERIC);
}

char *shortdatesortstr(time_t *date)
{
	return mktimestr(date,"/","",NUMERIC);
}

int monlength(int m, int y)
{
    m--;
    
    if (y % 4 == 0) 
		return llymonth[m];	
    else return lmonth[m];
}

time_t monsecs(int m, int y)
{
	if (m <= 1) return 0L;
	
	m -= 2;
	
	if (y % 4 == 0) 
		return llymonsecs[m];
	else return lmonsecs[m];
}

time_t yrsecs(int y)
{
	int ydiff;
	time_t ys;
	
	//note that time_t type is number of seconds from 1970
	//previous leap year to 1970 is 1968 the next is 1972
	
	if (y <= 1970) return 0L;
	
	ydiff = y - 1970;
	
	ys = (time_t)ydiff * 365 * DAYSECS;
	
	if (y >= 1972)
		ys += (time_t)(((y - 1972) / 4) + 1) * DAYSECS;
		
	return ys;
}

long getdatesecs(int d, int m, int y)
{
	time_t msecs, dsecs, ysecs;
	
	if (y < 1970) return NOTFOUND;
	if (m > 12 || m < 1) return NOTFOUND;
	if (d <= 0) return NOTFOUND;
	if (d > monlength(m, y)) d = monlength(m, y);
	
	dsecs = (time_t)d * DAYSECS;
	msecs = monsecs(m, y);
	ysecs = yrsecs(y);
	
	return dsecs + msecs + ysecs;
}

long gettimesecs(int s, int m, int h, int d, int mo, int y)
{
 	long datesecs;
 	
 	if ((datesecs = getdatesecs(d, mo, y)) == NOTFOUND) return NOTFOUND;
 	if (h > 23 || h < 0) return NOTFOUND;
 	if (m > 59 || m < 0) return NOTFOUND;
 	if (s > 59 || s < 0) return NOTFOUND;
 	
 	return datesecs + (3600 * h) + (60 * m) + s;
}

int getdate(char *str, int *day, int *mon, int *yr)
{
	char *p;
	//chop date up
	//get day
	if ((p = strchr(str, '-')) == NULL) return FALSE;
	*day = atoi(str); //it will stop automatically at the - sign
	p++;
	//get month using last letter of 3 letter string
	switch (tolower(p[2]))
	{
		case 'n': 					//jan or jun
			if (tolower(p[1]) == 'a') *mon = 1; 
			else *mon = 6;
		break; 
		case 'b': *mon = 2; break;  //feb
		case 'r': 					//mar or apr
			if (tolower(p[1]) == 'a') *mon = 3; 
			else *mon = 4;
		break;
		case 'y': *mon = 5; break;  //may
		case 'l': *mon = 7; break;  //jul
		case 'g': *mon = 8; break;  //aug
		case 'p': *mon = 9; break;  //sep
		case 't': *mon = 10; break; //oct
		case 'v': *mon = 11; break; //nov
		case 'c': *mon = 12; break; //dec

		default: return FALSE;
	}
    // make sure the month string is correct
    if (strnicmp(p, months[*mon - 1], 3) != 0) return FALSE;
	
	//get year
	if ((p = strchr(p, '-')) == NULL) return FALSE;
	p++;
	*yr = atoi(p);
	if (*yr < 97) *yr += 2000;
	else *yr += 1900;
	//check for valid year
    if (*yr < 1970) return FALSE;
    //check for valid day for that year
    if (*day > monlength(*mon, *yr) || *day < 1) return FALSE;

	return TRUE;
}

int ismonth(int m, char *s)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (toupper(months[m][i]) != toupper(s[i])) return FALSE;
	}
	return TRUE;
}

/****************************************************************************/
WP strwin(int attrib, int justify, char *str)
{
    WP props;
    
    int nl, ml, oldj;
    
    oldj = justifytext;
    justifytext = justify;
    
    getlinestats(str, &nl, &ml);
    
    props.left = middle(80,ml);     /* x position */
	props.top = middle(25,nl);     /* y position */
	props.width = ml;                /* width */
    props.height = nl;                /* height */
    
    create_win( props.left,
                props.top,
                props.width,
                props.height,
                attrib,            /* color */
                bsSINGLE,            /* style */
                str);              /* what to put there */
    
    justifytext = oldj;
    
    return props;
}

void showedithelp(int attrib)
{                 
    char pressany[] = "| Press a key to close window...";
    int nl, ml; /* number of lines, maximum length */
    int top;
    
    /* edit help routine display */
    if (edithelpptr == NULL)
        edithelpptr = edithelpstr;
    /* strcat(edithelpptr, pressany); */
    _settextcursor(NOCURSOR);
    getlinestats(edithelpptr, &nl, &ml);
    savescrn(scrnbuff);
    if (edithelptop == 0)
        top = middle(25,nl);
    else top = edithelptop;    
    if (edithelpattr > 0)    
        attrib = edithelpattr;
    create_win( middle(80,ml),     /* x position */
                top,               /* y position */
                ml,                /* width */
                nl,                /* height */
                edithelpattr,      /* color */
                bsSINGLE,            /* style */
                edithelpptr);      /* what to put there */
}

/****************************************************************************/
int checknum(unsigned char *key)
{
    if (*key >= '0' && *key <= '9') return TRUE;
    else return FALSE;
}

int checkdos(unsigned char *key)
{
	register i;   
	*key = toupper(*key);
	for (i = 0; i < 54; i++)
		if (doschars[i] == *key) return TRUE;
	return FALSE;
}	

/****************************************************************************/
int checkfname(char *fname)
{
	register i;
	int len;
	len = strlen(fname);
	if (len == 0 || len > 13) return FALSE;
	for (i = 0; i < len; i++)
	{
		if (fname[i] == ':' || fname[i] == '\\') return FALSE;
		if (!checkdos(fname + i)) return FALSE;
		if (fname[i] == '.') 
		{
			if (i > 9) return FALSE;
			if (len - i > 4) return FALSE;
		}
	}
	return TRUE;
		
}

/****************************************************************************/

int checkdatatype(unsigned char *key, int datatype)
{            
    if (usewildcard) return TRUE; //for field comparisons mainly???
    
    switch (datatype) {
        case BOOLEAN: 
        	if (*key == boolkey || 
        	   (*key == SPACE && zero == FALSE)) return TRUE;
        case NUMERIC: 
             return checknum(key);
        case NRANGE: 
             if (*key == SPACE && zero == FALSE) return TRUE;
             return checknum(key);
        case PHONE: //check for the keys on the phone keypad
        		if (!checknum(key) && !(*key == '*' || *key == '#')) 
        			return FALSE;
        		return TRUE;
        case DOSPATH:        
            return checkdos(key);
        case DOSFILE:
        	if (*key != ':' && *key != '\\') return checkdos(key);
        	else return FALSE;
        case DATE:
        case ALPHANUM: 
            return TRUE;
        case INTEGER:
            if ((checknum(key)) || *key == '-') return TRUE;
        case REAL:            
            if ((checknum(key)) || *key == '-' || *key == '.') return TRUE;
        case NOTHING: return FALSE;
    }
    return FALSE;
}

void getkey(unsigned char *key, char *zero)
{
	*key = 0;
	*zero = FALSE;
	
	do
	{
		if((*key = getch()) == 0) 
			*zero = TRUE;
	} 
	while (*key == 0);
}		

int yes( void )
{
	char key, zero;
	
	zero = FALSE;
	while ((key = getch()) == 0) zero = TRUE;
	if (zero || !(toupper(key) == 'Y' || key == ENTER)) 
		return FALSE;
	return TRUE;
}

/****************************************************************************/                   
int edit(int x, 
         int y, 
         int l, 
         int *startpos,
         int *pos, 
         int lenofstr,
         int attrib, 
         char key, 
         char zero,  
         int datatype,
         char *s)
{
    register j;
    int i = *pos;
    int st = *startpos;                                              
    
    /* make sure that length does not exceed the end of the string */    
    lenofstr--; 
    
    /* check for ridiculous position values */
    if (i < 0 || i > lenofstr || st < 0 || st > lenofstr) return ERROR;
    
    _settextcursor(cursortype);
    
    if (helpon) { /* change screen back if edit help is on */
        loadscrn(scrnbuff);            
        _settextcursor(cursortype);
        helpon = FALSE;
    }

    if (zero == FALSE) 
        switch (key) 
        {
            case BKSPC:
                if (strlen(s) == 0) return OK;
                if (i > 0) i--;
                for (j = i; j < lenofstr; j++)
                    s[j] = s[j + 1];
                s[j] = '\0';    
            break;
            
            case TAB:
            case ESC:
            case ENTER: 
            break;
            
            case CtrlY: /* clear string */
                s[0] = '\0';
                i = 0;
            break;
            
            default:           
            if (checkdatatype(&key, datatype))
            {
                if (datatype == BOOLEAN)
                {
                	if (strcmp(s, BOOLYES) == 0) strcpy(s, BOOLNO); 
                	else strcpy(s, BOOLYES);
                	return OK;
                }
                
                if (typemode == INSERT)
                {
                    if (strlen(s) <= (unsigned int)lenofstr) 
                    {
                        if ((unsigned int) i >= strlen(s)) {
                            for (j = strlen(s); j < i; j++)
                                s[j] = SPACE;
                            s[i + 1] = '\0';
                        }        
                        else for (j = lenofstr - 1; j >= i; j--)
                                    s[j + 1] = s[j];
                            
                        s[i] = key;       
                        if (i < lenofstr) i++;
                    }    
                }
                else
                {
                    if ((unsigned int) i >= strlen(s)) {
                        for (j = strlen(s); j < i; j++)
                            s[j] = SPACE;
                        s[i + 1] = '\0';
                    }     
                    s[i] = key;
                    
                    if (i < lenofstr) i++;
                }
            }    
        }
        
    else switch (key)
        {
            case F1:
            case AltH:
                if (helpavail) {
                    helpon = TRUE;
                    showedithelp(attrib);
                    *startpos = st;
                    *pos = i;
                    return OK;
                }
            break;
            
            case LEFT:
                if (i > 0) i--;
            break;
            
            case RIGHT:        
                if (i < lenofstr) i++;
            break;                     
            
            case DEL:               
                if (strlen(s) == 0) return OK;
                for (j = i; j < lenofstr; j++)
                    s[j] = s[j + 1];
                s[j] = '\0';    
            break;     
            
            case HOME:
                i = 0;
            break;            
            
            case END:
                if ((i = strlen(s)) > lenofstr)
                {
	                i = lenofstr;
	                while (s[i] <= SPACE && i >= 0) i--;
	                if (i < lenofstr) i++;
               	}
            break;
            
            case CtrlRIGHT:
            	do 
            		if (i < lenofstr) i++;
            	while(i < lenofstr && s[i] > SPACE);
            break;
            case CtrlLEFT:
            	do 
            		if (i > 0) i--;
            	while (i > 0 && s[i] > SPACE);
            break;
            
            case INSERT:
                if (typemode == INSERT) 
                { 
                    typemode = OVERWRITE;
                    cursortype = BLOCKCURSOR;
                }
                else 
                {
                    typemode = INSERT;      
                    cursortype = LINECURSOR;
                }
                _settextcursor(cursortype);
            break;    
        }                    
    /* decide if startpos needs to be changed */
    if (i < st) st = i; /* if position undershoots start */
    else if (i > st + l - 1) st = i - l + 1; /* if position overshoots end */
    
    _settextposition(y, x + (i - st));    
    writestr(x, y, st, l, attrib, s); 
    
    *startpos = st;
    *pos = i;
    return OK;
}

/* end of screen.c */












