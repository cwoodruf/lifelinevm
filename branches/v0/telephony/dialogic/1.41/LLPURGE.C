/* llpurge.c - remove inactive boxes, message and greeting files */

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

/* my source files */
#include "strings.c"   
#include "userstr.h"

#define OUTFILE "USERS.NEW"
#define DELOUTFILE "USERS.DEL"
#define INFILE  USERFILE

char *progname;
char cutoffdatestr[DOSPATHLEN + 1] = ""; 
time_t cutoffdate;
int infh, outfh, deloutfh, removeall = FALSE, removeblank = FALSE;

void progtitles( void )
{                           
    printf("%s: ", progname);
    printf("Remove inactive users. \n\n");
    printf("NOTES: ");
    printf("This program does not delete files. ");
    printf("Use LIFELINE back up to purge \ninactive message and greeting files. ");
    printf("Please keep a back up copy of your old \nUSERS.DAT file after using this program.\n\n");
    printf("Saved users will be stored in %s.\nDeleted users will be stored in %s.\n\n", OUTFILE, DELOUTFILE);
}

void cont( void )
{
	int d, m, y;

    do
    {
	    printf("Enter cut off date for removing inactive records (DD-MMM-YY/ALL/BLANK):");
	    gets(cutoffdatestr);
	    _strupr(cutoffdatestr);
	    getdate(cutoffdatestr, &d, &m, &y);
	    if (strlen(cutoffdatestr) == 0) 
	    {
	    	printf("\n%s aborted!\n", progname);
	    	exit(0);
	    }
	    if (strcmp(cutoffdatestr, "ALL") == 0) 
	    	removeall = TRUE;
	    else if (strcmp(cutoffdatestr, "BLANK") == 0)
	    	removeblank = TRUE;	
    }
    while ( !(removeblank || removeall || (cutoffdate = getdatesecs(d,m,y)) > 0) );

   	if (removeall) printf("Will remove ALL inactive users.\n");
   	else if (removeblank) printf("Will remove inactive users with no cut off date.\n");
    else printf("Will keep all users paid to %d/%d/%d or later.\n", d,m,y);
}

void closefiles( void )
{
    _close(infh);
    _close(outfh);
    _close(deloutfh);
}
   
void openinout( void )
{                       
    printf("Opening input file %s...\n", INFILE);
    if ((infh = _open(INFILE, _O_BINARY|_O_RDONLY)) < 0)
    {
        printf("\n%s: Unable to open user data file.\n", progname, INFILE);
        _close(infh);
        exit(1);
    }     
    
    printf("Opening output file %s (saved users)...\n", OUTFILE);
    _unlink(OUTFILE);
    if ((outfh = _open(OUTFILE, _O_BINARY|_O_RDWR|_O_CREAT, _S_IWRITE)) < 0)
    {
        printf("\n%s: Unable to open output file %s.\n", progname, OUTFILE);
        closefiles();
        exit(2);
    }

    printf("Opening output file %s (deleted users)...\n", DELOUTFILE);
    _unlink(DELOUTFILE);
    if ((deloutfh = _open(DELOUTFILE, _O_BINARY|_O_RDWR|_O_CREAT, _S_IWRITE)) < 0)
    {
        printf("\n%s: Unable to open output file %s.\n", progname, DELOUTFILE);
        closefiles();
        exit(2);
    }
}

int userzero( US *user )
{
	register i;
	char *u = (char *)user;
	for (i = 0; i < sizeof(US); i++) 
		if (*u != 0) return FALSE;
		else u++;
	return TRUE;
}

int oktodelete(US *u)
{
	int d,m,y;
	//only delete inactive users
printf("box %s is paid to %s\n", u->box, u->paidto);
	if (u->active != INACTIVE) return FALSE; 
	//compare dates 
	if (removeall) return TRUE; 
	if (removeblank)
	{
		if (strcmp(u->paidto, "") == 0) 
		{
			return TRUE;
		}	
		else return FALSE;
	}	
	//check for valid date
	if (!getdate(u->paidto, &d,&m,&y)) return FALSE;
	//check that date in box is less than or equal to cutoff date
	if (getdatesecs(d,m,y) > cutoffdate) return FALSE;
	//if date on user record is earlier than set cut off date 
	return TRUE;
}	

void purge( void )
{
    US user;
    long numrec, i, diff = 0L;
    
    printf("Purging inactive users...");
    
    _lseek(infh, 0L, SEEK_SET);    
    _lseek(outfh, 0L, SEEK_SET);
    
    numrec = _filelength(infh)/sizeof(US);
    
    for (i = 0; i < numrec; i++)
    {
        _read(infh, &user, sizeof(US));
        
        if (oktodelete(&user))
        {
            if (_write(deloutfh, &user, sizeof(US)) < 0)
            {
            	printf("\nDISK ERROR! Could not copy to %s.\n", DELOUTFILE);
            	exit(4);
            }
        }
        else
        {
            if (_write(outfh, &user, sizeof(US)) < 0)
            {
            	printf("\nDISK ERROR! Could not copy %s.\n", OUTFILE);
            	exit(3);
            }
        }
    }
    printf("Finished.\n");
}

void main( int argc, char *argv[] )
{
    char *cp = *argv;                        
    cls();
    /* get program name for error messages */
    for (progname = cp; *cp; ++cp)
        if (*cp == '\\') progname = cp + 1;
    
    progtitles();
    if ( argc > 1 )
    {
    	printf("Syntax: %s [no command line parameters]\n\n", progname);
    	exit(0);
    }
    cont();
    openinout();
    purge();
    
    closefiles();
    /* getch(); */
}   

/* end llpurge.c */