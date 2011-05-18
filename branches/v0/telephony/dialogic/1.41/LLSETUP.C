
// llsetup.c - creates batch file to run lifeline program with given parameters

//microsoft include files
#include <dos.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>

// my include files
#include "screen.c"
#include "ansrdefs.h"

// defines
#define VOID -1L
#define TTLX 2
#define PWLEN 11 // maximum password length
#define FLDX (80 - PWLEN)
#define CMDY 3
#define FLDY 8
#define INFOY 25
#define TTLY 1

#define TITLE "LIFELINE CONFIGURATION UTILITY VER " VER "                           Alt-X: EXIT."
#define INFO  "Home/End/Up/Down: Select.  Space: Yes/No.  Ctrl-D: Default Val.  Esc: Old Val."

#define MAXSIZE 32767 //maximum size bat file can be for setup to work
#define BATNAME "LL.BAT"
#define BAKNAME "LL.BAK"
#define PROGNAME "LIFELINE"
#define PRGNMLEN 8

#define CMDCOL fgWHITE + bgBLACK	
#define EDITCOL fgWHITE + bgCYAN
#define INFOCOL fgWHITE + bgBROWN	
#define BACKCOL fgWHITE + bgBLUE	
#define EDITGREY 7 + bgBLUE
#define BACKGREY 7 + bgBLUE

struct fieldstruct
{
    char name[FLDX + 1];
    unsigned int ttlattr, x, y, l, size, attr, type;
    char defstr[PWLEN + 1];
    char oldstr[PWLEN + 1];
    char cmd[4];
    char str[DOSPATHLEN];
    long min, max;
    char found;
}
field[] = 
{
	{"LIFELINE.EXE", CMDCOL, TTLX+12, CMDY, 66, 128, CMDCOL, ALPHANUM, "", "", "", "", VOID, VOID, FALSE},
	{"Strict use of admin boxes.", BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -A", "", VOID, VOID, FALSE},
	{"Permit browse feature." ,BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -B", "", VOID, VOID, FALSE},
	{"Number of channels to use", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, MAXCHANSTR, "", " -C", "", MINCHAN, MAXCHAN, FALSE},
	{"Default security code for new boxes", BACKCOL, FLDX, 0, 4, 0, EDITCOL, NUMERIC, DEFPIN, "", " -D", "", VOID, VOID, FALSE},
	{"Save debug info to log file?", BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -E", "", VOID, VOID, FALSE},
	{"Log new messages?",            BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -F", "", VOID, VOID, FALSE},
	{"Number of times user can return to start (0 = no limit)?", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, "0",    "", " -G", "", VOID, VOID, FALSE},
	{"Interrupt level for voice board", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, DEFINTRSTR, "", " -I", "", 2, 15, FALSE},
	{"Size of hard drive in MB if HD size exceeds 2.2GB.", BACKCOL, FLDX, 0, 5, 0, EDITCOL, NUMERIC, "", "", " -M", "", VOID, VOID, FALSE},
	{"Number of messages per box (max = 99)", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, DEFMSGSTR, "", " -N", "", MINMSG, MAXMSG, FALSE},
	{"Remove silence when recording?", BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -Q", "", VOID, VOID, FALSE},
	{"Number of rings to pick up phone (max = 10)", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, DEFRINGSTR, "", " -R", "", 1, MAXRING, FALSE},
//	{"Skeleton security code", BACKCOL, FLDX, 0, 4, 0, EDITCOL, NUMERIC, SKELETONPIN, "", " -S", "", VOID, VOID, FALSE},
	{"System phone number (for numeric paging)", BACKCOL, FLDX, 0, 10, 0, EDITCOL, NUMERIC, SYSPHONE, "", " -P", "", VOID, VOID, FALSE},
	{"Message length in seconds (max = 90)", BACKCOL, FLDX, 0, 2, 0, EDITCOL, NUMERIC, DEFTIMESTR, "", " -T", "", MINTIME, MAXTIME, FALSE},
	{"Disable paging feature?", BACKCOL, FLDX, 0, 3, 0, EDITCOL, BOOLEAN, BOOLNO, "", " -X", "", VOID, VOID, FALSE}
};

enum fields 
{
	fSTART = 1,
	fCMD = 0, 
	fSTRICTADMIN, 
	fBROWSE,
	fCHAN, 
	fDEFCODE, 
	fEVT, 
	fNEWFILE,
	fITERATE, 
	fINTR, 
	fHDMB, 
	fNUMMSG, 
	fSIL, 
	fRING, 
//	fSKELETON, 
	fSYSPHONE,
	fMSGLEN,
	fPGDISABLE,
	fLAST};
#define fFIRST fCMD
enum fields lastfld = fLAST - 1;

char bat[MAXSIZE] = {0};
char cmd[DOSPATHLEN * 2];
char path[MAXSIZE] = {0};
char buff[MAXSIZE] = {0};
char s[DOSPATHLEN * 2] = "";

// function definitions
// parse functions
void batfound( void );
void parsebatfile( void );
int findfield( char *bp );
// initialization functions
void initfields( void );
void writettl(int i);
void writefield(int i);
void writeinfo(int y, char *s);
// editing functions
int editfields( void );
void writedefaults( void );
void createcmdline( void );
void editfield(int i, int *startpos, int *pos);
int checkbounds(int i);
int inbounds(int i);
void checkdid(int n);
// batch functions
void createbat( void );
// end function definitions

// start of program
void main ( int argc, char *argv[] )
{
/*	if (argc > 0)
	{
		printf("%s: sets up command line for the Lifeline voice mail program.\n", getprogname(argv));
		printf("Syntax: %s [no command line parameters]", getprogname(argv));
		exit(0);
	}
*/	
	typemode = OVERWRITE;
	cursortype = BLOCKCURSOR;
	_settextcursor(cursortype);
	
	batfound();
	parsebatfile();
		
	_setbkcolor(fgBLUE);
	cls();
		
	writeinfo(TTLY, TITLE);
	writeinfo(INFOY, INFO);
		
	initfields();
	if (editfields()) createbat();
	else
	{
		_setbkcolor(0);
		cls();
	}
	_settextcursor(LINECURSOR);
}

// start parse functions
void batfound( void )
{
	int h;
	long size = MAXSIZE;
	long i = 0;
	char *start;
	char *pos;
	
	printf("Seeking %s.\n", BATNAME);	
	
	if ((h = _open(BATNAME, _O_RDWR|_O_TEXT)) != NOTFOUND)
	{
		printf("Loading contents.\n");
		size = _read(h, buff, (size_t) size);
		size = _filelength(h);
		buff[size] = '\0';
		_close(h);
	}
	
	if (h < 0 || size < 0)
	{
		strcpy(buff, PROGNAME);
		size = strlen(buff);
	}
	 	
	_strupr(buff);
	start = NULL;
	pos = strstr(buff, PROGNAME);
	
	do 
		start = pos;
	while ((pos = strstr(pos + PRGNMLEN, PROGNAME)) != NULL);
	
	if (start == NULL)
		strcpy(buff, PROGNAME);

    printf("Getting command line.\n");
	if (strlen(buff) > 0 && start - buff > 0)
	{
		printf("Copying path.\n");
		strncpy(path, buff, start - buff);
	}

	strcpy(bat, start);

	//remove following spaces
	printf("Removing following spaces.\n");
	while (bat[(i = strlen(bat) - 1)] <= SPACE) bat[i] = '\0';
}

void parsebatfile( void )
{
	int fld, len;
	char *next;
	char *bp;
	
	printf("Parsing command line.\n");
	//find first argument
	bp = strchr(bat, '-');
	
   	while (bp != NULL) 
	{
		fld = findfield(bp);
		bp += 2;
		next = strstr(bp, " -");
		
		switch (fld)
		{
			case fSTRICTADMIN:
			case fBROWSE:
			case fEVT:
			case fNEWFILE:
			case fSIL:
			case fPGDISABLE:
				strcpy(field[fld].str, BOOLYES);
			break;

			//case fAUTO:
			case fCHAN:
			case fDEFCODE:
			case fITERATE:
			case fINTR:
			case fHDMB:
			case fNUMMSG:
			case fRING:
			//case fSKELETON:
			case fSYSPHONE:
			case fMSGLEN:
            	if (next == NULL)
            	{
            		len = min(strlen(bp), field[fld].l);
					strncpy(field[fld].str, bp, len);
				}
				else 
				{
					len = min(field[fld].l, (unsigned)(next - bp));
					strncpy(field[fld].str, bp, len);
				}
		}
		strcpy(field[fld].oldstr, field[fld].str);

//printf("%s = %s (len = %d)\n", field[fld].name, field[fld].str, len);
		bp = strchr(bp, '-');
    }
   	
//getch();

}

int findfield( char *bp )
{
	int i;
	
	for (i = fSTART; i < fLAST; i++)
	{
		if (toupper(field[i].cmd[2]) == toupper(bp[1])) 
		{
			field[i].found = TRUE;
			return i;
		}
	}
	
	return 0;
}
// end parse functions

// start init functions		
void initfields( void )
{
	int i, j;
	
	for (i = fFIRST, j = 0; i < fLAST; i++, j++)
	{
		//calculate row
		if (field[i].y == 0) field[i].y = FLDY + j - 1;
		//calculate width of field
		if (field[i].size == 0) field[i].size = field[i].l;
		//write field description
		writettl(i);
		//put default values in empty fields
		if (!field[i].found)
		{
			strcpy(field[i].str, field[i].defstr); 
			strcpy(field[i].oldstr, field[i].defstr);
		}
		if (checkbounds(i)) //check field for invalid values
			writefield(i); //if ok write field
	}
}

void writettl(int i)
{                      
	if (i < fFIRST || i >= fLAST) return;
	fillchar = '.';
	writestr(TTLX, /* x pos */
			 field[i].y,  /* y pos */
			 0,   /* start in string at */
			 FLDX - TTLX, /* size of window */
			 field[i].ttlattr, /* color */
			 field[i].name);  //string
	fillchar = ' ';
}

void writefield(int i)
{
	if (i < fFIRST || i >= fLAST) return;
	
	writestr(field[i].x, /* x pos */
			 field[i].y,  /* y pos */
			 0,   /* start in string at */
			 field[i].l, /* size of window */
			 field[i].attr, /* color */
			 field[i].str);  //string
}

void writeinfo(int y, char *s)
{
	if (y < 1 || y > 25 || s == NULL) return;
	// fill in gaps
	writestr(1,y,0,TTLX - 1,INFOCOL, "");
	writestr(79,y,0,80,INFOCOL, "");
	// write title
	writestr(TTLX,
			 y,
			 0,
			 78,
			 INFOCOL,
			 s);
}
// end init functions

// editing functions
int editfields( void )
{
	int fld = fSTART, 
		oldfld = fSTART, 
		startpos = 0, 
		pos = 0;
	
	_settextposition(field[fld].y, field[fld].x);
	
	do
	{
		createcmdline();
		getkey(&key, &zero);
		if (zero)
		{
			writeinfo(INFOY, INFO);
			switch(key)
			{
				case UP: 
					if (fld > fSTART) fld--;
					else fld = lastfld;
				break;
				case DOWN:
					if (fld < lastfld) fld++;
					else fld = fSTART;
				break;
				case HOME: fld = fSTART;
				break;
				case END: fld = lastfld;
				break;
				case AltX: 
					if (zero == FALSE) break;
					if (!checkbounds(fld)) break;
					writeinfo(INFOY, "Save command line in batch file? Y/N (Enter = Y)");
					key = getch();
					if (toupper(key) == 'Y'|| key == ENTER)
						return TRUE;
					else if (toupper(key) == 'N') return FALSE;
					writeinfo(INFOY, INFO);
				break;
				default:
					editfield(fld, &startpos, &pos);
			}    
		}
		else
		{
			switch(key)
			{
				case CtrlD: 
					strcpy(field[fld].str, field[fld].defstr);
					writefield(fld);
//					writedefaults(); //old AltD action
				break;
				case 'Y':
				case 'y':
				if (field[fld].type == BOOLEAN)
				{ 
					strcpy(field[fld].str, BOOLYES);
					writefield(fld);
				}
				else editfield(fld, &startpos, &pos);
				break;
				
				case 'N':
				case 'n':
				if (field[fld].type == BOOLEAN) 
				{ 
					strcpy(field[fld].str, BOOLNO);
					writefield(fld);
				}
				else editfield(fld, &startpos, &pos);
				break;
				
				case SPACE:
				if (field[fld].type == BOOLEAN)
					{
						if (strcmp(field[fld].str, BOOLYES) == 0)
							strcpy(field[fld].str, BOOLNO);
						else strcpy(field[fld].str, BOOLYES);
						writefield(fld);
					}
					else if (field[fld].type == NRANGE)
							editfield(fld, &startpos, &pos);
				break;
				case ENTER:
					if (fld < lastfld) fld++;
					else fld = fSTART;
				break;
				case ESC:
					strcpy(field[fld].str, field[fld].oldstr);
					writefield(fld);
				break;
				default:
					editfield(fld, &startpos, &pos);
			}
			
			if (!inbounds(fld))
			{
				if (key != ENTER) 
				{
					fld = oldfld;
					sprintf(s, "INVALID VALUE! MIN = %li, MAX = %li.", 
								field[fld].min, field[fld].max);
					writeinfo(INFOY, s);
				}
				else writeinfo(INFOY, INFO);
			}
			else writeinfo(INFOY, INFO);
		}
		
		if (fld != oldfld)
		{
		/*	checkbounds(oldfld); */
			oldfld = fld;
			startpos = 0;
			pos = 0; 
			_settextposition(field[fld].y, field[fld].x);
		} 
		
	}
	while (TRUE);
}

int checkbounds(int i)
{	
/*	int rc;
	if (!(rc = inbounds(i)))
	{
		strcpy(field[i].str, ""); //field[i].oldstr);
		inbounds(i);
		writefield(i);
	}
	return rc; */
	return inbounds(i);
}

void writedefaults( void )
{
	int i;
	
	for (i = fSTART; i < fLAST; i++)
	{
		strcpy(field[i].str, field[i].defstr);
		writefield(i);
	}
	
	createcmdline();
}

void createcmdline( void )
{
	int i;
	
	// clear parameter string
	strcpy(field[fCMD].str,"");
	
	for (i = fSTART; i < fLAST; i++)
	{
		switch (i)
		{
			case fSTRICTADMIN:
			case fBROWSE:
			case fEVT:
			case fNEWFILE:			
			case fSIL:
			case fPGDISABLE:
				if (strcmp(field[i].str, BOOLYES) == 0)
					strcat(field[fCMD].str, field[i].cmd);
			break;
			default:
				// add to command line if not default value
				if (strcmp(field[i].str, field[i].defstr) != 0 && strlen(field[i].str) > 0)
				{
					strcat(field[fCMD].str, field[i].cmd);
					strcat(field[fCMD].str, field[i].str);
				}
		}
	}
	writefield(fCMD);
}

void editfield(int i, int *startpos, int *pos)
{
	edit(field[i].x, 
		 field[i].y, 
		 field[i].l,
		 startpos,
		 pos,
		 field[i].size,
		 field[i].attr,
		 key,
		 zero,                          
		 field[i].type,
		 field[i].str);
}

int inbounds(int i)
{
	int n;
	if (field[i].type != NUMERIC) return TRUE;
	if (field[i].min == VOID && field[i].max == VOID) return TRUE;

	n = atoi(field[i].str); // get value
	itoa(n, field[i].str, 10); // eliminate any illegal characters
	if (n < field[i].min || n > field[i].max) // check value
		return FALSE;
	//check did channels	
	strcpy(field[i].oldstr, field[i].str);
	writefield(i);

	return TRUE;
}
/*
void checkdid(int n)
{
	int j;
	if (n == 0) 
	{
		lastfld = fDID;
		for (j = fWINK; j < fLAST; j++)
		{
			strcpy(field[j].str, BOOLNO);
			field[j].ttlattr = BACKGREY;
			field[j].attr = EDITGREY;
			writettl(j);
			writefield(j);
		}
	}
	else 
	{
		lastfld = fTOLL;
		for (j = fWINK; j < fLAST; j++)
		{
			field[j].ttlattr = BACKCOL;
			field[j].attr = EDITCOL;
			writettl(j);
			writefield(j);
		}
	}
}
*/
// end editing functions

// batch functions
void createbat( void )
{
	int f;
	char outstring[DOSPATHLEN * 2];
	int plen;
	
	_setbkcolor(0);
	_settextcursor(LINECURSOR);
	cls();
	// create backup
//printf("erase\n");
	_unlink(BAKNAME);
//printf("Back up bat file.\n");
	rename(BATNAME, BAKNAME);  
	// create bat file
//printf("open\n");
	if ((f = _open(BATNAME, _O_RDWR|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)  
	{
		printf("ERROR: could not create %s!\n", BATNAME);
		return;
	}
//printf("write\n");
	if (path != NULL) plen = strlen(path);
	else plen = 0;
	//copy path, program name and command line parameters to output string
	if (path != NULL) strcpy(outstring, path);
	else strcpy(outstring, "");
	strcat(outstring, field[fCMD].name);
	strcat(outstring, field[fCMD].str);
	//printf("%s\n", outstring);
	//write string to file
		
	if (_write(f, outstring, strlen(outstring)))
	{
		printf("Use \"%s\" to run the Lifeline program. \n", BATNAME);
	}
	else printf("ERROR: unable to write output string to file!");
//printf("close\n");
	_close(f);
//getch();
}
// end batch functions

// end llsetup.c





