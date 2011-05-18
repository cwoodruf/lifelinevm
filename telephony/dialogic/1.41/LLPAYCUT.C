
//llpaycut.c - program to remove used paycodes

/**
 **   Microsoft header files
 **/
#include <malloc.h>
#include <signal.h>
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>

//lifeline header file
#include "screen.c"
#include "ansrdefs.h"
#include "userstr.h"

#define MAXINT 16384
//global data
int pfh, pch; //file handles etc

void openfiles( void )
{
   	_unlink(PAYCODEOLD);
   	printf("Renaming %s %s.\n",PAYCODEFILE, PAYCODEOLD);
   	if (rename(PAYCODEFILE, PAYCODEOLD))
   	{
   		printf("Unable to create back up %s of %s! Program aborted.\n",PAYCODEFILE, PAYCODEOLD);
   		exit(1);
   	}
   	
    if ((pfh = _open(PAYCODEOLD, _O_RDWR|_O_BINARY)) == -1)
    {
    	if ((pfh = _open(PAYCODEOLD, _O_RDWR|_O_BINARY|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)
    	{
    		printf("Unable to open %s. Program aborted!\n", PAYCODEOLD);
    		exit(1);
    	}
    }
    printf("Getting paycodes from %s.\n", PAYCODEOLD);

	_unlink(PAYCODEFILE);
	if ((pch = _open(PAYCODEFILE, _O_RDWR|_O_BINARY|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)
	{
		printf("Unable to open %s. Program aborted!\n", PAYCODEFILE);
		exit(1);
	}
    printf("Writing unused paycodes to %s.\n", PAYCODEFILE);
}

void loadfiles( void )
{
	PAY pc = {0};
	long maxrec, i;
	
	//go to end of file before starting
	_lseek(pfh, 0L, SEEK_SET);
	_lseek(pch, 0L, SEEK_SET);
	
	maxrec = filelength(pfh)/sizeof(PAY);
	
	printf("Writing paycode records .... \n");
	
	for (i = 0L; i < maxrec; i++)
	{
		if (_read(pfh, &pc, sizeof(PAY)) == -1)
		{
			printf("Error reading from %s. Program aborted!\n", PAYCODEOLD);
			exit(1);
		}
		
		//ignore records with a full "box" field
		if (strlen(pc.box) >= BOXLEN) continue;
		
		if (_write(pch, &pc, sizeof(PAY)) == -1)
		{
			printf("Error writing to %s. Program aborted!\n", PAYCODEFILE);
			exit(1);
		}
	}
	
	_close(pch);
	_close(pfh);
}

int main( void )
{
	openfiles();
	loadfiles();
	return 0;
}

//end llpaycut.c

