/* llcnvrt.c - conversion utility to export USERS.DAT to USERS.CSV 
   a comma delimited ascii file */

#define LLCNVT

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
#include "ansrdefs.h"
#include "screen.c"   
#include "userstr.h"
#include "idxstrct.h"
#include "idxplay.h"

//#define OUTFILE "USERS.CSV"
#define INFILE  USERFILE
#define BOTH    -1

char *convertdate(char *s);

char *progname;
int infh, i;
int activeonly = BOTH;
char infile[20] = INFILE;

void progtitles( void )
{                           
    fprintf(stderr,"%s: Convert user data file to an ASCII comma delimited format\n", progname);
    fprintf(stderr,"Syntax: %s [inputfile] [-A[CTIVE] | -I[NACTIVE]]\n",progname);
    if (strcmp(infile, INFILE) == 0)
    	fprintf(stderr,"Default input file: %s.\n\n", infile);
    else fprintf(stderr,"Input file: %s. \n\n", infile);
}

void closefiles( void )
{
    _close(infh);
}
   
void openinout( void )
{
    fprintf(stderr,"Opening input file %s...\n", infile);
    if ((infh = _open(infile, _O_BINARY|_O_RDONLY)) < 0)
    {
        fprintf(stderr,"\n%s: Unable to open input file %s.\n", progname, infile);
        _close(infh);
        exit(1);
    }     
}

#include "ansrstat.c"

#include "llcsv.c"

void doconversion( void )
{
	int len;
    static char outstring[1000] = {0};
    US user;
    
    _lseek(infh, 0L, SEEK_SET);    
    len = makecsvstr(NULL, outstring);
	printf("%s",outstring);

    while (!(_eof(infh)))
    {
        _read(infh, &user, sizeof(US));
        if (user.active > 0 && user.active < 4000) numusers++;
        if ((user.active == INACTIVE && activeonly == FALSE) ||
            (user.active == ACTIVE && activeonly == TRUE) ||
            (activeonly == BOTH && 
             (user.active == ACTIVE || user.active == INACTIVE)
            )
           )
        {
	        makecsvstr(&user, outstring);
	        printf("%s",outstring);
		}
    }
}

void getargs(int argc, char *argv[])
{
    char *cp = *argv;                        
    int i;      
    /* get program name for error messages */
    for (progname = cp; *cp; ++cp)
        if (*cp == '\\') progname = cp + 1;
    
    if (argc > 1)            
    {                    
        for (i = 1; i < argc; i++)
        {                          
            if (argv[i][0] == '-')
            {
                switch (toupper( argv[i][1] )) 
                {
                	case 'I': activeonly = FALSE; break;
                	case 'A': activeonly = TRUE; break;
                	default : progtitles(); exit(0); 
                }
            }
            else strcpy(infile, argv[i]);
        }
    }
}

void main(int argc, char *argv[])
{
    getargs(argc, argv);
    cls();
    progtitles();
	// load call stats if there are any
	loadcperh();
    openinout();
    doconversion();
    makestatsstr(s);
    printf("%s",s);
    closefiles();
}   

/* end of llcnvrt.c */   