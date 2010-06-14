
//llpayup.c - companion to llpaycd.c - rebuilds paycode.csv file

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
#include "llcsv.c"

//global data
int pfh, pch; //file handles etc

void openfiles( void )
{
    if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY)) == -1)
    {
    	if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)
    	{
    		fprintf(stderr, "Unable to open %s. Program aborted!\n", PAYCODEFILE);
    		exit(1);
    	}
    }
    fprintf(stderr,"Getting input from %s.\n", PAYCODEFILE);

//	if ((pch = _open(PAYCODECSV, _O_RDWR|_O_TEXT)) != -1)
//	{
//    	fprintf(stderr, "Copying %s to %s.\n",PAYCODECSV, PAYCODEBAK);
//    	_close(pch);
//    	_unlink(PAYCODEBAK);
//    	rename(PAYCODECSV, PAYCODEBAK);
//	}

//	if ((pch = _open(PAYCODECSV, _O_RDWR|_O_TEXT|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)
//	{
//		printf("Unable to open %s. Program aborted!\n", PAYCODECSV);
//		exit(1);
//	}
//	printf("Writing comma delimited output to %s.\n", PAYCODECSV);
}
void loadfiles( void )
{
	char csvstr[sizeof(PAY) + 6 + (2 * DATESORTLEN)];
	PAY pc = {0};
	int len;
	long maxrec, i;
	
	//go to end of file before starting
	_lseek(pfh, 0L, SEEK_SET);
	
	maxrec = filelength(pfh)/sizeof(PAY);
	
	fprintf(stderr,"Writing %ld paycode records ...", maxrec);
	
	for (i = 0L; i <= maxrec; i++)
	{
		len = paycsvstr(csvstr, pc);
		if (len < 0) //damaged record
		{
			sprintf(csvstr, "ERROR: damaged paycode. Fix %s with LLPAYFIX.EXE.\n", PAYCODEFILE);
		}
		// print to stdout instead of directly to file
		printf("%s", csvstr);
// old, do not use		
//		if (_write(pch, csvstr, len) == -1)
//		{
//			printf("Error writing to %s. Program aborted!\n", PAYCODECSV);
//			exit(1);
//		}
		if (_read(pfh, &pc, sizeof(PAY)) == -1)
		{
			fprintf(stderr, "Error reading from %s. Program aborted!\n", PAYCODEFILE);
			exit(1);
		}
	}
	
	_close(pfh);
	fprintf(stderr, "Finished.\n");	
}

int main( void )
{
	openfiles();
	loadfiles();
	return 0;
}

//end of llpayup.c


