
// llpayfix.c - program to repair paycode files

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

#define INFILE PAYCODEFILE
#define OUTFILE "PAYCODE.NEW"
#undef NEXT
#undef PREV
#define NEXT 2
#define PREV 3
#define CURR 4

char *progname;
int infh, outfh, i;
char outfile[20] = OUTFILE;
char infile[20] = INFILE;
char s[80];

void printinout( void )
{
	sprintf(s,"Input:%s, output:%s.\n", infile, outfile);
	strupr(s);
	printf(s);
}

void progtitles( void )
{                           
    printf("\n%s: VIEW PAYCODE DATA AND SAVE RECORDS TO ANOTHER FILE.\n", progname);
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
char *selectfield(enum fields f, PAY *pc)
{      
    time_t t;
    register i;
    
    if (pc == NULL) return field[f].name;
    switch (f)
    {
		case fPAYCODE:
			strncpy(s,pc->paycode,65); break;
        case fPCVAL:
            itoa(pc->value, s, 10); break;
        case fPCCREATED:
            t = pc->created;
            strncpy(s,datesortstr(&t),65); break;
        case fPCUSED:      
            t = pc->used;
            strncpy(s,datesortstr(&t),65); break;
		case fPCBOX:
			strncpy(s,pc->box,65); break;
    }
    //remove whitespace
    for (i = 0; (unsigned) i < strlen(s); i++)
    	if (s[i] < SPACE) s[i] = SPACE;
    	
    return s;
}

void showrec(PAY *pc)
{
	register i;
	
	cls();
	
	printinout();
	printf("UP/DOWN/PGUP/PGDN/HOME/END: VIEW.  ");
	printf("\"S\"/\"Z\": SAVE & GO NEXT/PREV.   Alt-X: EXIT.\n");
	
	for (i = fPAYCODE; i <= fPCUSED; i++)
	{
		printf("%-12s: %s\n", field[i].name, selectfield(i, pc));
	}
}

void viewdata( void )
{
    //struct _rccoord coord;
    PAY pc;
    long inpos, outpos, inlen;
    int saverec = FALSE;
    
    zero = FALSE; key = 0;
    inpos = outpos = 0L;
    inlen = _filelength(infh) - sizeof(PAY);
    
	// write records to output file
    do
    {
	    _lseek(infh, inpos, SEEK_SET);    
	    _read(infh, &pc, sizeof(PAY));
	    //this is a very common error caused by some sloppy coding
	    pc.box[BOXLEN] = '\0';
	    pc.paycode[PAYCODELEN] = '\0';
	    
	    showrec(&pc);
        
    	if (inpos == 0L || inpos >= inlen)
    	{
    		if (inpos == 0L) 
    			printf("First record. ");
    		else
	    		printf("Last record. ");
	    }

        switch (saverec) 
        {
        	case NEXT:
				printf("Previous record saved...");
	        break;
	        case PREV:
				printf("Following record saved...");
	        break;
	        case CURR:
	        	printf("Current record saved...");
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
        		if (inpos >= sizeof(PAY)) inpos -= sizeof(PAY);
        	break;
        	case PGDN:
        		if (inpos <= inlen - sizeof(PAY)) inpos += sizeof(PAY);
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
        		saverec = NEXT;
    			//write to outfile
    			_write(outfh, &pc, sizeof(PAY));
    			outpos = _tell(outfh);
    			//go to next record
    			if (inpos <= inlen - sizeof(PAY)) 
    				inpos += sizeof(PAY);
    			else saverec = CURR;	
    		break;
        	case 'Z':
        		if (saverec) break;
        		saverec = PREV;
    			//write to outfile
    			_write(outfh, &pc, sizeof(PAY));
    			outpos = _tell(outfh);
    			//go to next record
    			if (inpos >= sizeof(PAY)) 
    				inpos -= sizeof(PAY);
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

void main(int argc, char *argv[])
{
    getargs(argc, argv);
    openinout();
    viewdata();
    closefiles();
}   

// end llpayfix.c

