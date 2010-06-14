
/* LLPROMPT.C - view prompt scripts and record prompt files */

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

#define TTL_ATTR bgWHITE
#define SCRATTR (fgWHITE + bgBLACK)

#define TITLE ": Alt-R: Record file. Alt-P: Play file. Alt-X: Exit."
#define LIFELINE_SCR "LIFELINE.SCR"

#define SCRW_MASK "*.SCR"
#define SCRW_COLS	4
#define SCRW_WID	15
#define SCRW_HATTR	TTL_ATTR
#define SCRW_ATTR	SCRATTR
#define SCRW_MAX	60
#define SCRW_TITLE	"Select Script: "

#define TTL_X 1
#define TTL_Y 1
#define STAT_X 1
#define STAT_Y 25

#define TIMY 24 /* line to display prompt length in seconds */
#define MINX 1
#define MINY 2
#define MAXX 79
#define MAXY 23

#define FILEIND "FILE:" /* what indicates the file name in the script file */
#define FILEMAX 1000    /* maximum number of files in script */
#define MAXSIZE 65535L /* maximum script file size */

/* function defs */
void writestatus(char *s);
void writestatuskey(char *s);
void selectscrfile( void );
void loadscrfile( void );
void savescrfile( void );
char *getfname( int filenum );
long length(char* filename);
void writelength( long secs );
void showscript( int filenum );
void titles( void );
int splitindex( void );
void checkevent( void );

int attr = TTL_ATTR; /* attribute for title and status lines */
char buff[MAXSIZE] = {0}; /* keeps the script file in memory */
long files[FILEMAX + 1]; /* start of scripts in script file */
char fnames[FILEMAX + 1]; //list of filenames
int indlen; /* indicator length */
long filesize;
int maxfile;
char s[81]; //for messages that require printf formatting
char changed = FALSE; //has the buffer changed ?

/* one line record and play routines */
#include "recplay.c"

void main( int argc, char *argv[] )
{
    //strcpy(filename, LIFELINE_SCR); //get script file name
    progname = getprogname(argv);   //load program name from command line
    sysactive = sysinit(getargs(argc, argv, "use as script")); 
	selectscrfile();			//get script that you want to open if one is not entered on command line
    loadscrfile();				//open script file, get scripts & file names
	if (!splitindex())			//create individual prompt files
		printf("Error creating individual prompts from indexed file!\n");
    _settextcursor(NOCURSOR);   //get rid of text cursor 
    checkevent();				//record and/or play files
    set_hkstate(H_ONH);			//put phone back on hook
    stopsys();					//stop voice system
    cls();						//clear screen
    _settextcursor(LINECURSOR); //restore text cursor
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
		printf("%s: No script files (\"%s\") to open!", progname, SCRW_MASK);
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
    int scrfh;
    long fptr = 0;
    int fcount = 0;

    if ((scrfh = _open(filename, _O_RDONLY|_O_BINARY)) == NOTFOUND)
    {
        printf("%s: Unable to open script file %s!\n", progname, filename);
        exit (1);
    }
    
    filesize = _filelength(scrfh);
    if (filesize > MAXSIZE)
    {
    	printf("Script file %s too large (%ld bytes, %ld max).", 
    			filename, filesize, MAXSIZE);
    	exit(2);
   	}
   	
    _read(scrfh, buff, (int) filesize);
    
    _close(scrfh);
    
    indlen = strlen(FILEIND);
    while ((fptr = pos(FILEIND, buff + fptr) + 1) != 0 && fcount < FILEMAX)
    {
        if (fcount > 0) 
            fptr += (files[fcount - 1] + indlen);
        files[fcount] = fptr;
        fptr += indlen;
        fcount++;
    }

    files[fcount] = filesize;
    maxfile = fcount - 1;
}

void savescrfile( void )
{
    int scrfh;
    long fptr = 0;
    int fcount = 0;

    if ((scrfh = _open(filename, _O_RDWR|_O_BINARY)) == NOTFOUND)
    {
		if ((scrfh = _open(filename, _O_CREAT|_O_RDWR|_O_BINARY, _S_IREAD|_S_IWRITE)) == NOTFOUND)
		{
		    printf("%s: Unable to open or create script file %s!\n", progname, filename);
		}
    }
    
    _write(scrfh, buff, sizeof(buff));
    
    _close(scrfh);
}

char *getfname(int filenum)
{
    static char filename[DOSPATHLEN] = {0};
    long i, j, p;

    p = files[filenum] + pos(FILEIND, files[filenum]) + indlen;

    for (i = p, j = 0; buff[i] > SPACE; i++, j++)
        filename[j] = buff[i];

    filename[j] = '\0';
    return filename;
}

void markfname(int filenum)
{
    long i, p;

    changed = TRUE;
    p = files[filenum] + pos(FILEIND, files[filenum]) + indlen;

    for (i = p; buff[i] > SPACE; i++)
    {
        if (buff[i] == toupper(buff[i]))
        	buff[i] = tolower(buff[i]);
        else buff[i] = toupper(buff[i]);
    }
}

long length(char* filename)
{
    struct _find_t f;
    if (_dos_findfirst(filename, _A_NORMAL, &f) == 0) return f.size;
    return 0L;
}

void writelength(long secs)
{
    char timestr[81];

    secs /= (SMPLRATE/10);
    sprintf(timestr, "LENGTH: %ld.%ld seconds", secs / 10, secs % 10);
    writestr(MINX, TIMY, 0, strlen(timestr), SCRATTR, timestr);
}

void scrchar(int x, int y, unsigned char c)
{
    CELL far *pos;          
    y--;
    x--;
    pos = scr + (y * 80) + x;
    pos->c = c;
    pos->attr = SCRATTR;
}

int getmaxx(long i)
{                    
    int maxx;
    maxx = MAXX;
    while ( buff[i + maxx - MINX] > SPACE )
            maxx--;
    if (maxx == 0) maxx = MAXX;
    return maxx;
}

void showscript( int filenum )
{                
    long i;
    int x = MINX, y = MINY, maxx;

    /* clear screen */
    for (x = 1; x <= 80; x++)
        for (y = 2; y <= 24; y++)
            scrchar(x,y,SPACE);
            
    titles();
    
    writelength(length(getfname(filenum)));
    x = MINX;
    y = MINY;
    i = files[filenum] - 1;
    maxx = getmaxx(i);

    for (i = files[filenum] - 1; i < files[filenum + 1] - 1; i++)
    {
        if (x == maxx || 
            buff[i] < SPACE)
        {
            x = MINX;
            maxx = getmaxx(i);
            y++;
            if (y > MAXY) break; 
            while (buff[i] <= SPACE && i < files[filenum + 1] - 1) i++;
            if (i < files[filenum + 1] - 1)
                scrchar(x++,y,buff[i]);
        }
        else 
            scrchar(x++,y,buff[i]);
    }
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
			sprintf(s, "ERROR: %s has nothing in it!", getfname(i));
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
	struct idxstruct idx;
	char idxname[40];
	char libname[40];
	
	printf("Creating individual prompt files from indexed file...");
	strcpy(idxname, newext(filename, IXP));
	strcpy(libname, newext(filename, VXP));
	
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
		if (_read(hidx, &idx, sizeof(struct idxstruct)) == NOTFOUND)
			break;
		//open prompt file
		if ((hword = _open(idx.name, _O_RDWR|_O_BINARY)) == NOTFOUND)
		{
			if ((hword = _open(idx.name, _O_CREAT|_O_RDWR|_O_BINARY, _S_IWRITE)) == NOTFOUND)
				break;
			//do the actual copy
			if (copyfilesection(hlib, hword, idx.pos, idx.len)) 
				printf("Was unable to create %s from %s.\n", idx.name, libname);
		}
		//close file
		_close(hword);
	}
	_close(hlib);
	_close(hidx);
	
	if (i <= maxfile)
	{
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
	struct idxstruct idx;
	char idxname[40];
	char libname[40];
	
	writestatus("Creating index...");

	strcpy(idxname, newext(filename, IXP));
	strcpy(libname, newext(filename, VXP));
	
	if ((hidx = makeidxfile(idxname)) == NOTFOUND) 
		return FALSE;
	if ((hlib = makeidxfile(libname)) == NOTFOUND) 
	{
		_close(hidx);
		return FALSE;
	}
	
	for (i = 0; i <= maxfile; i++)
	{
		if ((hword = _open(getfname(i), _O_RDWR|_O_BINARY)) == NOTFOUND)
			break;
		idx.pos = (unsigned long) (_tell(hlib));
		idx.len = (unsigned long) (_filelength(hword));
		strcpy(idx.name, getfname(i));
		
		if (copyfile(hword, hlib)) break;
		if (_write(hidx, &idx, sizeof(struct idxstruct)) == NOTFOUND)
			break;
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
	if (checkfiles()) 
	{
		writestatus("Create indexed prompt file now? Y/N (Enter = Y)");
		if (yes()) 
		{
			if (!buildindex()) 
			{
				writestatus("Error creating indexed file!");
				getch();
				return FALSE;
			}
		}
	}
	writestatus("Delete individual prompt files? Y/N (Enter = Y)");
	if (yes()) delfiles();
	//check to see if buffer has been changed and save it if it has
	if (changed) savescrfile();
	return TRUE;
}

void checkevent( void )
{
    int j, i = 0;
    char fletter;
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
            	writestatus(statstr[sWAITPLAY]);
            break;
            case AltR:             
            	if (!sysactive) 
            	{
            		writestatus(SYSINACTIVE);
            		break;
            	}
                recplay = rpRECORD;
            	writestatus(statstr[sWAITREC]);
            break;
            case AltX: if (didindex()) return;
        }
        else
    	switch (key)
    	{
    		case ENTER: 
    			if (recplay) 
    			{
    				recplayloop(getfname(i));
    				if (recplay != rpERROR) recplay = FALSE;
    			}
    		break;
    		case ESC:
    			if (recplay) recplay = FALSE;
    		break;
    		case SPACE:
    			markfname(i);
    		break;
    		default:
    			j = i;
    			key = toupper(key);
    			do
    			{
    				if (i < maxfile) i++;
    				else i = 0;
    				fletter = toupper(getfname(i)[0]);
    			}
    			while (j != i && fletter != key);
    	}
    } 
    while (TRUE);        
}


/* end llprompt.c */







