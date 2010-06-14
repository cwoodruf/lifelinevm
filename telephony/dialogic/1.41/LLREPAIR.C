
//llrepair.c - program that will show records and then produce a new users.dat
//file.

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

#define INFILE "USERS.DAT"
#define OUTFILE "USERS.NEW"
#define NONE 0
#define NEXT 2
#define PREV 3
#define ABUFFSIZE (80 * sizeof(US))

char *progname;
int infh, outfh, i;
char outfile[20] = OUTFILE;
char infile[20] = INFILE;
char s[80];
char autorepair = FALSE;

char buff[ABUFFSIZE];

void doautorepair();
size_t studybuff(char *buff, int bsize);

void printinout( void )
{
	sprintf(s,"Input:%s, output:%s.\n", infile, outfile);
	strupr(s);
	printf(s);
}

void progtitles( void )
{                           
    printf("\n%s: VIEW USER DATA AND SAVE RECORDS TO ANOTHER FILE.\n", progname);
    printf("SYNTAX: %s [INFILE [OUTFILE]]\nDEFAULTS: ",progname);
   	printinout();
}

void closefiles( void )
{
    _close(infh);
    _close(outfh);
}
   
void openinout( void )
{
    printf("Opening input file %s...\n", infile);
    if ((infh = _open(infile, _O_BINARY|_O_RDONLY)) < 0)
    {
        printf("\n%s: Unable to open input file.\n", progname);
        progtitles();
        _close(infh);
        exit(1);
    }     
    
    printf("Opening output file %s...\n", outfile);
    _unlink(outfile);
    if ((outfh = _open(outfile, _O_BINARY|_O_WRONLY|_O_CREAT, _S_IWRITE)) < 0)
    {
        printf("\n%s: Unable to open output file %s.\n", progname, outfile);
        progtitles();
        closefiles();
        exit(2);
    }
}

/* return a specific field string checking what userstruct variable it 
   is for */     
char *selectfield(enum fields f, struct userstruct *user)
{      
    time_t t;
    register i;
    
    if (user == NULL) return field[f].name;
    switch (f)
    {
		case fBOX:
			strncpy(s,user->box,65); break;
		case fLANGUAGE:
			strncpy(s,user->language,65); break;
		case fPIN:
			strncpy(s,user->pin,65); break;
		case fADMIN:
			strncpy(s,user->admin,65); break;
		case fNAME:
			strncpy(s,user->name,65); break;
		case fAD1:
			strncpy(s,user->ad1,65); break;
		case fAD2:
			strncpy(s,user->ad2,65); break;
		case fCITY:
			strncpy(s,user->city,65); break;
		case fPROV:
			strncpy(s,user->prov,65); break;
		case fPHONE:
			strncpy(s,user->phone,65); break;
		case fPCODE:
			strncpy(s,user->pcode,65); break;
		case fMISC:
			strncpy(s,user->misc,65); break;
		case fTTLPAID:
			strncpy(s,user->ttlpaid,65); break;
		case fREMIND:
			strncpy(s,user->remind,65); break;
		case fACTIVE:
			switch (user->active)
			{
				case ACTIVE: 
					strncpy(s,isactive,65); break; 
				case INACTIVE: strncpy(s,isinactive,65); break; 
				case NOTFOUND: strncpy(s,"",65); break; 
			}
		break;
		case fBOXTYPE:
			strncpy(s,btname[user->boxtype],65); break;
        case fPAIDTO:
            strncpy(s,user->paidto,65); break;
        case fSTART:
            t = user->start;
            strncpy(s,datesortstr(&t),65); break;
        case fLASTCALL:      
            t = user->acctime;
            strncpy(s,datesortstr(&t),65); break;
        case fCALLS:
            ltoa(user->calls, s, 10); break;
        case fMSGS:
            itoa(user->nextmsg, s, 10); break;
    }
    //remove whitespace
    for (i = 0; (unsigned) i < strlen(s); i++)
    	if (s[i] < SPACE) s[i] = SPACE;
    	
    return s;
}

void showrec(US *u)
{
	register i;
	
	cls();
	
	printinout();
	printf("UP/DOWN/PGUP/PGDN/HOME/END: VIEW.  ");
	printf("\"S\"/\"Z\": SAVE & GO NEXT/PREV.   Alt-X: EXIT.\n");
	
	for (i = fBOX; i <= fMAX; i++)
	{
		printf("%-11s: %s\n", field[i].name, selectfield(i, u));
	}
}

int boxnumeric(char *box) {
	int i;
	if (strlen(box) != BOXLEN) return FALSE;
	for (i=0;i<BOXLEN;i++) {
		if (box[i] < '0' || box[i] > '9') return FALSE;
	}
	return TRUE;
}

void viewdata( void )
{
    //struct _rccoord coord;
    struct userstruct user;
    long inpos, outpos, inlen;
    int saverec = FALSE;
    
    zero = FALSE; key = 0;
    inpos = outpos = 0L;
    inlen = _filelength(infh) - sizeof(US);
    
	// write records to output file
    do
    {
	    _lseek(infh, inpos, SEEK_SET);    
	    _read(infh, &user, sizeof(US));
	    showrec(&user);
        
    	if (inpos == 0L || inpos >= inlen)
    	{
    		if (inpos == 0L) 
    			printf("First record. ");
    		else
	    		printf("Last record. ");
	    	if (saverec) 
	    		printf("Record saved...");
	    }
        else
        switch (saverec) 
        {
        	case NONE:
        		printf("Record NOT saved (bad box number)");
        	break;
        	case NEXT:
				printf("Previous record saved...");
	        break;
	        case PREV:
				printf("Following record saved...");
	        break;
	    }
        
        if (saverec) saverec = FALSE;
        
        zero = FALSE;
        while ((key = getch()) == 0) 
        	zero = TRUE;
        
        if (!saverec && zero)
        switch(key)
        {   
        
        	case UP:
        		if (inpos > 0L) inpos--;
        	break;
        	case DOWN:
        		if (inpos < inlen) inpos++;
        	break;
        	case PGUP:
        		if (inpos >= sizeof(US)) inpos -= sizeof(US);
        	break;
        	case PGDN:
        		if (inpos <= inlen - sizeof(US)) inpos += sizeof(US);
        	break;
        	case HOME:
        		inpos = 0L;
        	break;
        	case END:
        		inpos = inlen;
        	break;
        	case AltX: break;
        }
        else
        switch(toupper(key))
        {
        	case 'S':
        		if (saverec) break;
    			//write to outfile
    			if (boxnumeric(user.box)) {
    				_write(outfh, &user, sizeof(US));
    				outpos = _tell(outfh);
	        		saverec = NEXT;
    			}
    			//go to next record
    			if (inpos <= inlen - sizeof(US)) 
    				inpos += sizeof(US);
    		break;
        	case 'Z':
        		if (saverec) break;
    			//write to outfile
    			if (boxnumeric(user.box)) {
    				_write(outfh, &user, sizeof(US));
					outpos = _tell(outfh);
        			saverec = PREV;
				} else saverec = NONE;
    			//go to next record
    			if (inpos >= sizeof(US)) 
    				inpos -= sizeof(US);
    		break;
        }
    }
    while (key != AltX || zero != TRUE);
}

void getargs(int argc, char *argv[])
{
    char *cp = *argv;                        

    /* get program name for error messages */
    for (progname = cp; *cp; ++cp)
        if (*cp == '\\') progname = cp + 1;
    
    if (strchr(argv[1],'?') != NULL)
    {
    	progtitles();
    	exit(0);
    }
    if (!stricmp(argv[1],"-auto"))
    	autorepair = TRUE;
    else {
    	switch (argc)
    	{
			case 1: break;
			case 2:
				strcpy(infile, argv[1]);
			break;
			case 3:
			    strcpy(infile, argv[1]);
			    strcpy(outfile, argv[2]);
			break;
		    default: //illegal value
		    	progtitles();
		    	exit(0);
    	}
    }
}
// the idea of autorepair to read through the file sequentially 
// and look for valid records instead of using _seek
void doautorepair (void) {
	int leftover,bsize;
	leftover = 0L;
	while (!_eof(infh)) {
		if ((bsize = _read(infh,buff + leftover,(int)(ABUFFSIZE-leftover))) == -1) break;
		// now look for records
		leftover = studybuff(buff,bsize);
	}
}

size_t studybuff(char *buff, int bsize) {
	int i, istart, j, k;
	US u;
	union studyframe {
		US u;
		char buff[sizeof(US)];
	} s;
	
	j = k = 0;
	for (i=0;i<bsize;i++) {
	    if (j<sizeof(US)) s.buff[j++] = buff[i];
	    else {
    		j = 0;
    		// making the assumption that no bad records will 
    		// have both a numeric box and a numeric pin
    		printf("found box %s, pin %s\n",s.u.box,s.u.pin);
	    	if (boxnumeric(s.u.box) && boxnumeric(s.u.pin)) {
	    		u = s.u;
	    		i--;
	    		printf("Writing record: %s\n",u.box);
	    		if (_write(outfh,&u,sizeof(US)) == -1) {
	    			printf("Error writing to output file!\n");
	    			exit(1);
	    		}
	    	} else {
	    		// try and look ahead here and find a box number
	    		i -= sizeof(US) - 1;
	    		while (i<bsize) {
	    			istart = i;
	    			for (k=0;k<=BOXLEN && i<bsize;k++,i++) {
	    				s.buff[k] = buff[i];
	    			}
	    			s.buff[k] = '\0';
//					printf("box:\"%s\", i %d, k %d, bsize %d...\n",s.u.box,i,k,bsize);
	    			if (boxnumeric(s.u.box)) {
	    				printf("Found box number: %s.\n",s.u.box);
	    				j = k;
	    				i--;
	    				break;
	    			} else i = istart + 1;
	    		}
	    	}
	    }
	}
	// put any left over stuff from end of buffer at start of buffer
	j = sizeof(US) - j; // this is the leftover number of bytes that were not incorporated into a test record
	for (i=0;i<j;i++) {
		buff[i] = buff[bsize-j+i];
	}
	return j;
}

void main(int argc, char *argv[])
{
    getargs(argc, argv);
    openinout();        
    if (autorepair) doautorepair();
    else viewdata();
    closefiles();
}   

//end llrepair.c



