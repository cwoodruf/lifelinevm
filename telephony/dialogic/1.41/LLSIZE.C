
/* llsize.c used in conjunction with */
/* ansr3.c telephone answering application */
/* from the dialogic ansr2.c program example */

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

/* ansr3 header files */
#include "ansrdefs.h"

//the following was taken from "userstr.h"
/* field lengths */             
#define BOXLEN 4
#define PINLEN BOXLEN
#define NAMELEN 40
#define ADLEN 40
#define CITYLEN 20
#define PROVLEN 2
#define PHONELEN 20
#define PCODELEN 6
#define MISCLEN 57
#define FILELEN 8
#define ACTLEN 8
#define SHORTTIME 5
#define TIMELEN 15
#define DATELEN (TIMELEN - 6)
#define EDPDTOLEN 8 //length of edit string for editing paid to date over the phone
#define NEWTIMELEN 5
#define NUMLEN 6
#define LNUMLEN 10
#define MSGLEN (SCRWID - 1)
#define PASSWORDLEN (SCRWID - 12) /* strlen(msg[cPASSWORD]) */
#define UNUSEDSIZE 88 //((MAXMSG * sizeof(unsigned long)) - (2 * sizeof(long)) - (DATELEN + 1) - (FILELEN + 1))
#define PATHLEN 20 //max size for current working path indicator

typedef struct userstruct        
{
    char box[BOXLEN + 1];
    char pin[PINLEN + 1];     /* personal id number or "security code" */
    char name[NAMELEN + 1];
    char ad1[ADLEN + 1];
    char ad2[ADLEN + 1];
    char city[CITYLEN + 1];
    char prov[PROVLEN + 1];   /* province or state */
    char phone[PHONELEN + 1];
    char pcode[PCODELEN + 1]; // post code   
    char misc[MISCLEN + 1];   /* extra storage for future use */
    char admin[BOXLEN + 1];
    char ttlpaid[DATELEN + 1];  //amount paid to date by subscriber
    char unused[UNUSEDSIZE]; //not currently used
    char bdval; //what are boxdays worth?
    long bdowed; //how many days are they billed for
    time_t bdupdate; //last time box days were updated
    long boxdays; // for admin boxes: how much time has their box group used
    time_t msgtime; // time of last message
    char language[FILELEN + 1]; //what language file to use
    char nextmsg;    // name of next message
    char boxtype;   // determines box behavior
    int active;      /* enables deleting of box */
    time_t acctime; /* when was box last accessed by subscriber */
    char paidto[TIMELEN - 1]; /* date paid up to */
    char remind[2];  /* remind them to pay */
    time_t start;    /* date box was initialized */
    long calls;      /* number of times box was called */
} US;

void getboxsizes(char * filemask);
void printfilesize(char * filemask);
int getactiveboxes();

char *filemask[] = { 
DEFGRT, 
LOGFILE, 
CPERH, 
USERFILE, 
PGQFILE, 
BACKUPFILE, 
DISKUSED, 
ANNOUNCEMENT, 
ANNOUNCETEMP, 
TELCOFILE, 
PAYCODEFILE, 
PAYCODECSV, 
PAYCODEBAK, 
PAYCODEOLD, 
ULOGFILE, 
CSVFILE, 
STATEMENT,
GMASK, 
MMASK, 
RMASK,
""
};
    
long boxsizes[MAXBOX] = {0};
int active[MAXBOX] = {0};

/****************************************************************
*        NAME : main(argc,argv[])
* DESCRIPTION : entry point to application.
*       INPUT : argc = argument count.
*             : argv = array of pointers to arguments.
*      OUTPUT : none.
*     RETURNS : none.
*    CAUTIONS : none.
****************************************************************/
void main(int argc,char **argv)
{
	int m = 0, deleteall = 0;
	
	if (argc == 2 && strcmp(argv[1],"deleteall") == 0) 
	{ 
		deleteall = 1; 
		printf("@echo off\nrem Delete all inactive boxes (use > to save output to file)\n\n");
	}
	else printf ("filename/box, size (bytes), status\n");
        
	for (m == 0; strcmp(filemask[m],"") != 0; m++)
	{   
		if (strlen(filemask[m]) == 0) continue;
		if (strchr(filemask[m], '*') != NULL)
		{
			getboxsizes(filemask[m]);
		}
		else
		{   
			if (!deleteall) printfilesize(filemask[m]);
		}		
	}           
	
	if (getactiveboxes())
	{
	    if (deleteall)
	    {
			for (m == 0; m < MAXBOX; m++)
			{
				if (boxsizes[m] > 0 && active[m] == 0)
				{
					printf("del %04d*.*\n", m);
				}
			}
		}	
		else
		{
			for (m == 0; m < MAXBOX; m++)
			{
				if (boxsizes[m] > 0)
				{				
					printf("%04d, %ld, %s\n", m, boxsizes[m], active[m]? "ACTIVE": "INACTIVE");
				}	
			}
		}	
	}	
    else
    {
		for (m == 0; m < MAXBOX; m++)
		{
			if (boxsizes[m] > 0)
			{
				printf("%04d, %ld, NA\n", m, boxsizes[m]);
			}	
		}
	}	
}    

void getboxsizes(char * filemask)
{
	struct _find_t f;
	int box;
	// ignore invalid masks
	if (filemask[0] != '*') { return; }
	if (filemask[1] != '.') { return; }
	if (!_dos_findfirst(filemask, _A_NORMAL, &f))
	{
		//parse the filename and get the box size if it is from a box
		sscanf(f.name, "%04d", &box);
		boxsizes[box] += f.size;
		//find the next file
		_dos_findnext(&f);
	}	
}

void printfilesize(char * filemask)
{
	struct _find_t f;
	if (!_dos_findfirst(filemask, _A_NORMAL, &f))
	{
		printf("%s, %ld\n", f.name, f.size);
	}
	else
	{
		printf("%s, 0\n", filemask);
	}		
}		

int getactiveboxes()
{
	int ufh, box;
	struct userstruct urec;
	
	if ((ufh = open(USERFILE, _O_RDONLY)) == -1)
	{
		printf("Could not open %s!\n", USERFILE);
		return 0;
	}
	else
	{
		while (!eof(ufh))
		{
			if (read(ufh,&urec,sizeof(struct userstruct)) == -1)
			{
				printf("Error reading %s!\n", USERFILE);
				close(ufh);
				return 0;
			}
			else 
			{   
				box = atoi(urec.box);
				active[box] = urec.active;
			}	
		}
	}
	close(ufh);
	return 1;			
}
