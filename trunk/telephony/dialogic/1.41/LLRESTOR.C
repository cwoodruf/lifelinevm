
//llrestor.c - will restore USERS.DAT (as USERS.NEW) from a comma delimited file
#define LLRESTOR

#include <direct.h>
#include <dos.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>  
#include <graph.h>

/* my source files */
#include "screen.c"   
#include "userstr.h"

#define INFILE "USERS.CSV"
#define OUTFILE "USERS.NEW"

char *progname;
int infh, outfh, i;
char outfile[20] = OUTFILE;
char infile[20] = INFILE;

char *selectfield(int i, US *user)
{
	return "";
}

#include "llcsv.c"

void progtitles( void )
{                           
    printf("%s: convert ASCII comma delimited file to user data\n", progname);
    printf("Syntax: %s [[infile] outfile]\n",progname);
   	printf("Input file: %s, output file: %s.\n\n", infile, outfile);
}

void closefiles( void )
{
    _close(infh);
    _close(outfh);
}
   
void openinout( void )
{
    printf("Opening input file %s...\n", infile);
    if ((infh = _open(infile, _O_TEXT|_O_RDONLY)) < 0)
    {
        printf("\n%s: Unable to open input file.\n", progname);
        _close(infh);
        exit(1);
    }     
    
    printf("Opening output file %s...\n", outfile);
    _unlink(outfile);
    if ((outfh = _open(outfile, _O_BINARY|_O_WRONLY|_O_CREAT, _S_IWRITE)) < 0)
    {
        printf("\n%s: Unable to open output file %s.\n", progname, outfile);
        closefiles();
        exit(2);
    }
}

void doconversion( void )
{

#define INSTRSIZE (2 * sizeof(US))

    //struct _rccoord coord;
    int progrow = 7, progcol = 14;
    char instring[INSTRSIZE];
    char progresstr[] = "Creating box      ...";
    US blankuser = {0};
    US user;
    
    
    printf("%s", progresstr);
    //coord = _gettextposition();
    _lseek(infh, 0L, SEEK_SET);    
    _lseek(outfh, 0L, SEEK_SET);
	//_write(outfh, &blankuser, sizeof(US));    
	// write records to output file
    while (!(_eof(infh)))
    {
        user = blankuser;
        //read the input string
        for (i = 0; i < INSTRSIZE; i++)
        {
        	_read(infh, instring + i, sizeof(char));
			if (instring[i] == 10) break;
			if (_eof(infh)) break;
        }
        instring[i] = 0;                                     
        // ignore any inputs that don't start with numbers
        if (instring[0] < '0' || instring[0] > '9') continue;
        //parse the input string
        if (!readcsvstr(&user, instring)) continue;
        //check for valid box
        if (strlen(user.box) != field[fBOX].l) continue; 
        //update progress
        _settextposition(progrow, progcol);
        printf("%s", user.box); 
        //write new record out
        if (_write(outfh, &user, sizeof(US)) < 0)
        {
        	printf("\nDISK error.\n");
        	exit(3);
        }

    }
    _settextposition(progrow, strlen(progresstr) + 1);
    printf(" Finished.\n");

#undef INSTRSIZE

}

void getargs(int argc, char *argv[])
{
    char *cp = *argv;                        

    /* get program name for error messages */
    for (progname = cp; *cp; ++cp)
        if (*cp == '\\') progname = cp + 1;
    
    switch (argc)
    {
		case 1: break;
		case 2:
			strcpy(outfile, argv[1]);
		break;
		case 3:
		    strcpy(infile, argv[1]);
		    strcpy(outfile, argv[2]);
		break;
	    default: //illegal value
	    	progtitles();
	    	exit(1);
    }
}

void main(int argc, char *argv[])
{
    getargs(argc, argv);
    cls();
    progtitles();
/*    
    printf("\nContinue? Y/N (Enter = Y)");
    yn = getch(); 
    yn = toupper(yn);
    if (yn != 'Y' || yn != ENTER) exit(0);
    printf("\n");
*/    
    openinout();
    doconversion();
    closefiles();
}   

//end llrestor.c




