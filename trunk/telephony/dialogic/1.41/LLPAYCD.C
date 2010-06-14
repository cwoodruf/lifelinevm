
//llpaycd.c - paycode generating program for lifeline 3+

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

#define MAXINT 10000
//global data
int numtoadd, monval, sinit; //program parameters
int pfh; //file handles etc
char tag[(2 * BOXLEN) + 1] = "";

void syntax( void )
{
	printf("\nSyntax for LLPAYCD.EXE:\n\n");
	printf("LLPAYCD -Tnnnnnnnn -Mn -In -Nn\n\n");
	printf("-Tnnnn	Tag (up to 8 digits).\n");
	printf("-Mn 	Number of months (value) of each paycode.\n");
	printf("-In  	What number to initialize series of paycodes at.\n");
	printf("-Nn		How many paycodes to create.\n\n");
	printf("Paycode format tag_____SEQUrand. Where \n");
	printf("tag_____ eight digit tag.\n");
	printf("SEQU 	sequential 4 digit number to distinguish each paycode.\n");
	printf("rand 	random 4 digit number added for security purposes.\n\n");
	exit(1);
}	

void getargs(int argc, char **argv)
{
	time_t now;
	register argno;
	//initialize randomization
	srand((int) time(&now));  
	#undef RAND_MAX
	#define RAND_MAX 9999
	//initialize other variables
	numtoadd = monval = 0;
	sinit = -1;
      /* check each argument */
    for (argno=1; argno < argc; argno++)  
    {
		/* check for dash or slant bar */
		if (*argv[argno] != '-')  {
		    printf("Must begin option with '-'\n");
		    syntax();
		}
		switch (tolower(*(argv[argno]+1)))  
		{
		case 't':
		sscanf(argv[argno]+2,"%s", &tag);
		break;
		case 'm':
		sscanf(argv[argno]+2,"%d",&monval);
		if (monval < 0 || monval > 128)
		{
			printf("Invalid value %d months.\n", monval);
			syntax();
		}
		break;
		case 'n':
		sscanf(argv[argno]+2,"%d",&numtoadd);
		if (numtoadd < 0 || numtoadd > MAXINT)
		{
			printf("Invalid value %d for number of codes to add.\n", numtoadd);
			syntax();
		}
		break;
		case 'i':
		sscanf(argv[argno]+2,"%d",&sinit);
		if (sinit < 0 || sinit > MAXINT)
		{
			printf("Invalid value %d for start value.\n", sinit);
			syntax();
		}
		break;
		default: syntax();
		}
	}
}

void checkparameters( void )
{
	PAY pc;
	char buff[BOXLEN + 1];
	
	if (filelength(pfh) > 0L)
	{
		_lseek(pfh, -1L * (long) sizeof(PAY), SEEK_END);
		_read(pfh, &pc, sizeof(PAY));
		if (strlen(tag) == 0) 
			strncpy(tag, pc.paycode, 2 * BOXLEN);
		if (sinit == -1) 
		{
			strncpy(buff, pc.paycode + 2*BOXLEN, BOXLEN);
			if ((sinit = atoi(buff) + 1) > MAXINT)
			{
				printf("Start passed max start value (%d). Reset to 0.", MAXINT);
				sinit = 0;
			}
		}
		if (monval == 0) monval = pc.value;
	}
	 
	if (sinit == -1) sinit = 0;
	
	printf("Base paycode = %s %04d XXXX\n",
			tag, sinit);

	if (numtoadd == 0 || monval == 0 || strlen(tag) == 0)
	{
		printf("You forgot a parameter!\n");
		syntax();
	}
	
	if (numtoadd + sinit > MAXINT)
	{
		printf("Start value and number to add will result in a value that is out of range!\n");
		syntax();
	}
}

void openfiles( void )
{
    if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY|_O_APPEND)) == -1)
    {
    	if ((pfh = _open(PAYCODEFILE, _O_RDWR|_O_BINARY|_O_CREAT, _S_IREAD|_S_IWRITE)) == -1)
    	{
    		printf("Unable to open %s. Program aborted!\n", PAYCODEFILE);
    		exit(1);
    	}
    }
    printf("Writing output to %s.\n", PAYCODEFILE);
}

void loadfiles( void )
{
	register i;
	PAY pc = {0}, nullpc = {0};
	int rn;
	char k;
	time_t now;
	
	//go to end of file before starting
	_lseek(pfh, 0, SEEK_END);

	printf("Writing %d paycodes at %d months each. Continue? Y/N (Enter = Y)", numtoadd, monval);
	while ((k = getch()) == 0);
	if (tolower(k) != 'y' && k != ENTER) 
	{
		printf("\nProgram aborted!\n");
		exit(1);
	}
	else printf("\nWriting paycodes ...");
	
	for (i = sinit; i < sinit + numtoadd; i++)
	{
		pc = nullpc;
		rn = rand();
		rn %= 10000; //lop of 1st digit of 5 digit number
		sprintf(pc.paycode,"%08s%04d%04d", 
				tag, i, rn);
		pc.paycode[PAYCODELEN] = '\0';
		pc.value = monval;
		pc.created = time(&now);
		
		if (_write(pfh, &pc, sizeof(PAY)) == -1)
		{
			printf("\nError writing to %s. Program aborted!\n", PAYCODEFILE);
			exit(1);
		}
	}
	
	_close(pfh);
	
	printf(" Finished.\n");
}

int main(int argc, char **argv)
{
	openfiles();
	getargs(argc,argv);
	checkparameters();
	loadfiles();
	return 0;
}

//end of llpaycd.c

