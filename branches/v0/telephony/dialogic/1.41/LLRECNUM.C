
//llrecnum.c - program to record the number file

/**
 **   Microsoft header files
 **/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>

/* my source files */   

/* general functions and defs */
#include "screen.c"        /* basic screen handling functions */
#include "strings.c"
#include "disk.c"

/* ansr3 header file */
#include "ansrdefs.h"
#include "idxstrct.h"
/* single line record and play */
#include "recplay.h"

/* DEFINES */

#define LLPROMPT
#define LLRECNUM

#define TTL_ATTR bgWHITE
#define SCRATTR (fgWHITE + bgBLACK)

#define TITLE ": Press ENTER to record word, Alt-P to play word. Alt-X: Exit."

//mask for finding other number text files
#define SCRW_MASK "*.NUM"

#define SCRW_COLS	4
#define SCRW_WID	15
#define SCRW_HATTR	TTL_ATTR
#define SCRW_ATTR	SCRATTR
#define SCRW_MAX	60
#define SCRW_TITLE	"Select File:"

#define TTL_X 1
#define TTL_Y 1
#define STAT_X 1
#define STAT_Y 25

#define TIMY 24 /* line to display prompt length in seconds */
#define MINX 1
#define MINY 2
#define MAXX 79
#define MAXY 23

#define FILEMAX 1000    /* maximum number of files in number script */
#define MINSIZE 300L   // cut off 1/10th of a second from the end of the recording
#define MAXSIZE 65535L /* maximum number script file size */
#define EOL 13 //end of line character

/* struct to hold position of currently played message */
typedef struct msgstruct 
{
    unsigned long offset; /* offset into the file */
    unsigned long length; /* length of the message */
}MS;

/* function defs */
void writestatus(char *s);
void writestatuskey(char *s);
void selectscrfile( void );
void loadscrfile( void );
int splitindex( void );
char *getfname( int filenum );
long length(char* filename);
void writelength( long secs );
void showscript( int filenum );
void titles( void );
void checkevent( void );

int idxchanged = FALSE; //changed to true if record is chosen at any time
char isnew[FILEMAX] = {0}; //keep track of new files (for reducing blip at end of file)
int attr = TTL_ATTR; /* attribute for title and status lines */
char buff[MAXSIZE] = {0}; /* keeps the script file in memory */
char files[FILEMAX][40] = {0}; /* start of number scripts in number script file */
long filesize;
int maxfile;
CELL voidbuff[23 * MAXWIDTH] = {0}; //for clearing center section of screen
char s[81]; //for messages that require printf formatting

/* one line record and play routines */
#include "recplay.c"

void main( int argc, char *argv[] )
{
    
    progname = getprogname(argv);   //load program name from command line
    sysactive = sysinit(getargs(argc, argv, "use as number script")); 
	selectscrfile();			//get number script that you want to open if one is not entered on command line
    loadscrfile();				//open number script file, get number scripts & file names
    splitindex();
    _settextcursor(NOCURSOR);   //get rid of text cursor 
    checkevent();				//record and/or play files
    set_hkstate(H_ONH);			//put phone back on hook
    stopsys();					//stop voice system
    cls();						//clear screen
    _settextcursor(LINECURSOR); //restore text cursor
    //to do: ask to and if yes reassemble vox file and idx file
}

void writestatus(char *s)
{
    writestr(STAT_X, STAT_Y, 0, 80, attr, s);
}   

void writestatuskey(char *s)
{
    char key;
    writestr(STAT_X, STAT_Y, 0, 80, attr, s);
    while((key = getch()) == 0);
}   

void hilitescr(char scripts[][SCRW_WID + 1], int script, int attr, WP scrw)
{
	writestr(((script % SCRW_COLS) * SCRW_WID) + scrw.left, //x position 
			 (script / SCRW_COLS) + scrw.top,  //y position
			 0,
			 SCRW_WID,
			 attr,
			 scripts[script]);
}

char *getscriptname(char scripts[][SCRW_WID + 1], int scrmax, WP scrw)
{
	unsigned char key, zero = FALSE;
	int slen, script = 0;
	
	do
	{
		while ((key = getch()) == 0) zero = TRUE;
		hilitescr(scripts, script, SCRW_ATTR, scrw);
		if (zero)
		switch (key)
		{
			case UP:
				if (script - SCRW_COLS >= 0) script -= SCRW_COLS;
			break;
			case DOWN:
				if (script + SCRW_COLS <= scrmax) script += SCRW_COLS;
			break;
			case ShiftTAB:
			case LEFT:
				if (script > 0) script--;
			break;
			case RIGHT:
				if (script < scrmax) script++;
			break;
			case HOME:
				script = 0;
			break;
			case END:
				script = scrmax;
			break;
		}
		else
		switch (key)
		{
			case TAB:
				if (script < scrmax) script++;
			break;
		}
		hilitescr(scripts, script, SCRW_HATTR, scrw);
	}
	while (key != ENTER && zero);
	
	//remove spaces at end of file name
	slen = strlen(scripts[script]) - 1;
	while (scripts[script][slen] <= SPACE) slen--;
	scripts[script][slen + 1] = 0;
	
	return scripts[script];
} 

int compare( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
   return _stricmp( * ( char** ) arg1, * ( char** ) arg2 );
}

WP showscripts(char scripts[][SCRW_WID + 1], int scrmax)
{
	register i;
	WP scrw;
	int tlen;
	char s[(SCRW_MAX * SCRW_WID) + (SCRW_MAX / SCRW_COLS) + 1] = "";
	
	for (i = 0; i < scrmax; i++)
	{
		if (i > 0 && i % SCRW_COLS == 0)	strcat(s, "|");
		while (strlen(scripts[i]) < SCRW_WID) strcat(scripts[i], " ");
		strcat(s, scripts[i]);
	}
	
	scrw = strwin(SCRW_ATTR, jLEFT, s);	
	tlen = strlen(SCRW_TITLE);
	writestr(middle(80, tlen), scrw.top - 1, 0, tlen, SCRW_ATTR, SCRW_TITLE);
	return scrw;
}

void selectscrfile( void )
{
	char scripts[SCRW_MAX][SCRW_WID + 1];
	int err, i = 0, scrmax = 0;
	struct _find_t f;
	WP scrw;
	
	if (strlen(filename) > 0) return; //filename already entered on command line
	
	err = _dos_findfirst(SCRW_MASK, _A_NORMAL, &f);
	
	while (!err)
	{
		strcpy(scripts[i], f.name);
		i++;
		err = _dos_findnext(&f);
	}
	
	scrmax = i;

	if (!scrmax)
	{
		printf("%s: No number script files (\"%s\") to open!", progname, SCRW_MASK);
		exit(3);
	}
	
    _settextcursor(NOCURSOR);   //get rid of text cursor 
	cls();						//clear screen
	qsort( (void *)scripts, (size_t)scrmax, SCRW_WID + 1, compare );
	scrw = showscripts(scripts, scrmax); //build scripts window
	hilitescr(scripts, 0, SCRW_HATTR, scrw); //hilite first script
	strcpy(filename, getscriptname(scripts, scrmax - 1, scrw)); //get file name
    cls();
    _settextcursor(LINECURSOR); //restore text cursor
}

void loadscrfile( void )
{
    register i;
    int scrfh;
    int fcount = 0;
    char *pos;
    char *bp;
    
    if ((scrfh = _open(filename, _O_RDONLY|_O_BINARY)) == NOTFOUND)
    {
        printf("%s: Unable to open number script file %s!\n", progname, filename);
        exit (1);
    }
    
    filesize = _filelength(scrfh);
    if (filesize > MAXSIZE)
    {
    	printf("Number script file %s too large (%ld bytes, %ld max).", 
    			filename, filesize, MAXSIZE);
    	exit(2);
   	}
   	
    _read(scrfh, buff, (int) filesize);
    
    _close(scrfh);
    
    bp = buff;
    
    for (i = 0; i < FILEMAX; i++)
    {
    	pos = strchr(bp, EOL); //check for carriage return
    	if (pos == NULL) break;
    	//check for length of file name
    	if (pos - bp) 
    	{
    		strncpy(files[i], bp, pos - bp);
    		files[i][pos - bp] = 0;
    	}
    	else i--;
    	bp = pos;
    	bp += 2; //skip carriage return and linefeed characters
    }

	if (pos == NULL) 
	{
		pos = strchr(bp, '\0');
		if (pos - bp)
			strncpy(files[i], bp, pos - bp);
		else i--;
	}

    maxfile = i;
}

char *getfname(int filenum)
{
    static char fn[40];
    int len;
    
    if ((len = strlen(files[filenum])) <= 8)
    	return files[filenum];
    //if file is too long then 
    //make a name out of the first 8 letters
    //use any extra letters as the extension
    strcpy(fn, files[filenum]);
    fn[8] = '.'; //delimit name
    fn[13] = '\0'; //truncate
    
    return fn;
}

long length(char* filename)
{
    struct _find_t f;
    if (_dos_findfirst(filename, _A_NORMAL, &f) == 0) return f.size;
    return 0L;
}

void writelength(long bytes)
{
    char msecs[80], timestr[80] = "LENGTH:";
    ltoa(bytes/3, msecs, 10);
    strcat(timestr, msecs);
    strcat(timestr, " milliseconds");
    writestr(MINX, TIMY, 0, strlen(timestr), SCRATTR, timestr);
}

void voidscrn( void )
{
    unsigned char far *scrn;
    scrn = scrnaddress(1,2);
    
    _fmemmove(scrn, voidbuff, (sizeof(CELL) * 23 * MAXWIDTH) - 1);
}

void showscript( int filenum )
{                
    /* clear screen */
	voidscrn();
	//put up default title
    titles();
    //show size of file
    writelength(length(getfname(filenum)));
    //show number to be recorded
    strwin(SCRATTR, jCENTER, files[filenum]);

}
 
void titles( void )
{
    char title[81] = "";
    
    strcat(title, filename);
    strcat(title, TITLE);
       
    writestr(TTL_X, TTL_Y, 0, 80, attr, title);
    writestatus(statstr[sREADY]);
}

int checkfiles( void )
{
	register i;
	int h;
	
	writestatus("Checking files.");
	for (i = 0; i < maxfile; i++)
	{
		if ((h = _open(getfname(i), _O_RDWR|_O_BINARY)) == NOTFOUND) 
		{
			sprintf(s, "ERROR: unable to open %s.", getfname(i));
			writestatus(s);
			return FALSE;
		}
		if (_filelength(h) == 0L)
		{
			sprintf(s, "ERROR: %s is empty.", getfname(i));
			writestatus(s);
			return FALSE;
		}
		_close(h);
	}
	return TRUE;
}

int makeidxfile( char *nfile )
{
	_unlink(nfile);
	return _open(nfile, _O_CREAT|_O_RDWR|_O_BINARY, _S_IWRITE);
}

int openidxfile( char *nfile )
{
	return _open(nfile, _O_RDWR|_O_BINARY);
}

int splitindex( void )
{
	int hidx, hword, hlib;
	register i;
	MS idx;
	char idxname[40];
	char libname[40];
	char fname[40];
	
	printf("Creating single files from indexed file...");
	strcpy(idxname, newext(filename, IXN));
	strcpy(libname, newext(filename, VXN));
	
	if ((hidx = openidxfile(idxname)) == NOTFOUND) 
		return FALSE;
	if ((hlib = openidxfile(libname)) == NOTFOUND) 
	{
		_close(hidx);
		return FALSE;
	}

	for (i = 0; i <= maxfile; i++)
	{
		//get index information
		if (_read(hidx, &idx, sizeof(MS)) == NOTFOUND)
			break;
		strcpy(fname, getfname(i));
		//open prompt file (keep older version of file)
		if ((hword = _open(fname, _O_RDWR|_O_BINARY)) == NOTFOUND)
		{
		    if ((hword = _open(fname, _O_CREAT|_O_RDWR|_O_BINARY, _S_IWRITE)) == NOTFOUND)
			break;
			//do the actual copy
			if (copyfilesection(hlib, hword, idx.offset, idx.length)) break;
		}
		//close file
		_close(hword);
	}
	_close(hlib);
	_close(hidx);
	
	if (i <= maxfile)
	{
		printf("\nError splitting file!\n");
		_close(hword);
		return FALSE;
	}
	printf("finished.\n");
	return TRUE;
}

int buildindex( void )
{
	int hidx, hword, hlib;
	register i;
	MS idx;
	char fname[40];
	char idxname[40];
	char libname[40];
	unsigned long fsize;
	
	writestatus("Creating index...");

	strcpy(idxname, newext(filename, IXN));
	strcpy(libname, newext(filename, VXN));
	
	if ((hidx = makeidxfile(idxname)) == NOTFOUND) 
		return FALSE;
	if ((hlib = makeidxfile(libname)) == NOTFOUND) 
	{
		_close(hidx);
		return FALSE;
	}
	
	for (i = 0; i <= maxfile; i++)
	{
		strcpy(fname, getfname(i));
		if (!strlen(fname)) continue;
		if ((hword = _open(fname, _O_RDWR|_O_BINARY)) == NOTFOUND)
		{
			writestatus("Error opening word file!");
			getch();
			break;
		}
		idx.offset = (unsigned long) (_tell(hlib));
		fsize = (unsigned long) (_filelength(hword));
		//clip "blip" off end of file if it is newly recorded
		if (isnew[i] && fsize > MINSIZE)
			idx.length = fsize - MINSIZE;
		else idx.length = fsize;
		
		if (copyfile(hword, hlib)) 
		{
			writestatus("Error copying file to library!");
			getch();
			break;  
		}
		
		if (_write(hidx, &idx, sizeof(MS)) == NOTFOUND)
		{
			writestatus("Error writing to index!");
			getch();
			break;
		}
		_close(hword);
	}
	_close(hlib);
	_close(hidx);
	
	if (i <= maxfile)
	{
		_close(hword);
		return FALSE;
	}
	writestatus("Finished creating index. Press any key to exit.");
	getch();
	return TRUE;
}

void delfiles( void )
{
	register i;
	
	for (i = 0; i <= maxfile; i++)
		_unlink(getfname(i));
}

int didindex( void )
{
	//if (checkfiles()) {
	writestatus("Create indexed number file now? Y/N (Enter = Y)");
	if (yes()) 
	{
		if (!buildindex()) 
		{
			writestatus("Error creating indexed file!");
			getch();
			return FALSE;
		}
	}
	writestatus("Delete individual number files? Y/N (Enter = Y)");
	if (yes()) delfiles(); //remove individual files
	return TRUE;
}

void checkevent( void )
{
    int i = 0;
    do
    {
        if (!recplay) showscript(i);  
        //note: allow user to view error msg before updating screen
        if (recplay == rpERROR) recplay = FALSE; 
        zero = FALSE;
        while((key = getch()) == 0)
            zero = TRUE;
        if (zero == TRUE)
        switch (key)
        {
            case PGDN: 
                if (i < maxfile) i++;
            break;
            case PGUP:
                if (i > 0) i--;
            break;
            case HOME:   
                i = 0;
            break;    
            case END:
                i = maxfile;
            break;    
            case AltP:
            	if (!sysactive)
            	{
            		writestatus(SYSINACTIVE);
            		break;
            	}
            	recplay = rpPLAY;
				recplayloop(getfname(i));
				if (recplay != rpERROR) recplay = FALSE;
            break;
            case AltX: if (didindex()) return;
        }
        else
    	switch (key)
    	{
    		case ENTER: 
            	if (!sysactive)
            	{
            		writestatus(SYSINACTIVE);
            		break;
            	}
    			idxchanged = TRUE;
    			isnew[i] = TRUE;
                recplay = rpRECORD;
				recplayloop(getfname(i));
				if (recplay != rpERROR) recplay = FALSE;
    		break;
    	}
    } 
    while (TRUE);        
}

//end of llrecnum.c



