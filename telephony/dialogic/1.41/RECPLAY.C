
/* recplay.c - one line record and play routines */

#ifndef RECPLAY
#include "strings.h"
#include "recplay.h"
#endif

void writeoptions( char *desc )
{
    printf("%s -In filename\n", progname);
    printf("-In       Interrupt used by voice processor. Default: %d.\n", DEFINTR);
    printf("filename  File to %s. Default: %s.\n", desc, filename);
}

int getargs( int argc, char *argv[], char *desc )
{
    int d4xint = DEFINTR;    
    int argno;
        
    /* find text screen */
    scr = (CELL far *) getscrtype();
    /* check each argument */
    for (argno=1; argno < argc; argno++)  
    {
        /* check for dash */
        if (*argv[argno] != '-')  {
	        sscanf(argv[argno], "%s", &filename);
	        printf("%s file: %s.\n", desc, filename);
	        break;
        }
        switch (tolower(*(argv[argno]+1)))  {
        case 'i':
            sscanf(argv[argno]+2,"%d",&d4xint);
            printf("Interrupt = %d.\n", d4xint);
        break;
        default: 
            writeoptions(desc);
            exit(0);
        }
    }
            
    return d4xint;
}

int sysinit(int d4xint)
{
      int rc;  /* return code from startsys() */
      int channel = CHANNEL;
      int channels;
      
      /* check for the D/4x driver */
      if (!getvctr())  {
         printf("DIALOG/4x voice driver not installed\n");
         return FALSE;
      }
      else printf("D4x driver found.\n");
      
      /* make sure system is stopped before starting */
      stopsys(); 
      
      rc = startsys(d4xint,SM_EVENT,0,0,&channels);
      if (rc != E_SUCC) {
         printf("Unable to start voice system, Return code %d\n", rc);
         return FALSE;
      }
      else printf("System started.\n");
      
     /* initialize channel */
     setcst(channel, (C_LC+C_RING+C_OFFH+C_ONH), RINGS);
     printf("CST set.\n");
     set_hkstate(H_OFFH); /* put line off hook */
     clrevent(); //get termination of event
     printf("Line off hook.\n");
     clrdtmf(channel);         
     printf("Clearing keypad buffer.\n");
     return TRUE;
}

int set_hkstate( int state )
{
   int rc=0;
   
   if ( (rc = sethook( 1, (state==H_ONH)?H_ONH:H_OFFH)) != E_SUCC ) {
        printf("Failed to change hook state - %d", rc);
   }
   return rc;
}                                        

#ifndef LLPROMPT

void writestatus(char *s)
{   
    printf("%s\n", s);
}                      

void writestatuskey(char *s)
{                   
    char key;
    writestatus(s);
    while((key = getch()) == 0);
}

#endif

int play(int handle)
{
    RWB d4xrwb;
    /* rewind to top of file */
    vhseek(handle,0L,0);
    /* set up read write block for recording */
    clrdtmf(channel);            /* empty out any saved digits */
    clrrwb(&d4xrwb);             /* clear the D/4x read/write block */
    d4xrwb.filehndl = handle;    /* handle of file to play */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.termdtmf = '@';       // terminate on any digit
    //d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */
    /* play vox file on D/4x channel, normal play back 
       (4 bit ADPCM at 6000 hz sampling rate) */
    return(xplayf(channel,PM_NORM,&d4xrwb));
}

int record(int handle)
{
    int silence = 0;
    RWB d4xrwb;
    /* go back to start of file */
    vhseek(handle, 0L, 0);
    /* set up read/write block for recording */
    clrdtmf(channel);
    clrxrwb(&d4xrwb);
    d4xrwb.filehndl = handle;
    d4xrwb.maxsec   = 100;  /* maximum seconds for the record       */
//    d4xrwb.maxdtmf  = 1;    /* terminate if any dtmf                */
//    d4xrwb.termdtmf = '@';  /* terminate if any dtmf                */
    d4xrwb.maxsil   = 10;   /* terminate on 10 seconds of silence  */
    //d4xrwb.loopsig  = 1;    /* terminate on loop signal             */
    d4xrwb.rwbflags = 0x02; /* enable beep before record            */
    d4xrwb.rwbdata1 = 3;    /* .6 second beep                       */
    /* normal record with 4bit ADPCM at 6000 hz sampling rate */
#ifdef LLRECNUM
	silence = RM_SCOMP;
#else
	silence = 0;
#endif
    return (recfile(channel,&d4xrwb,RM_NORM+silence));
}

int closechan( void )
{
	int rc;
	
	rc = stopch(channel);
	clrevent();
	return rc;
}

int event( void )
{   
    EVTBLK evtblk;

	zero = FALSE;
	
    while (TRUE)
    {
        if (_kbhit()) 
        {
        	while ((key = getch()) == 0)
        		zero = TRUE;
        	return closechan();
        }
        if (gtevtblk(&evtblk) == -1)
            return 0;
    }
}

void clrevent( void )
{   
    EVTBLK evtblk;
    while (1)
    {
        if (gtevtblk(&evtblk) == -1)
            return;
    }
}

void recplayloop(char *filename)
/* from ansr1.c main function */
{
    int rc, handle = 0;
    char s[81] = "";
    
    switch (recplay)
    {
        case rpPLAY: 
            if ((handle = vhopen(filename, READ)) == 0)
            {  
                writestatuskey(statstr[sNOFILEERR]);
                rc = -1;
                break;
            }
            writestatus(statstr[sPLAY]);
            rc = play(handle); 
        break;
        case rpRECORD: 
#ifndef LLRECNUM
            _unlink(fbak(filename));
            rename(filename, fbak(filename));
#else            
            _unlink(filename);
#endif            
            if ((handle = vhopen(filename, CREATE)) == 0)  
            {
                writestatuskey(statstr[sNOFILEERR]);
                rc = -1;
                break;
            }    
            writestatus(statstr[sRECORD]);
            rc = record(handle); 
        break;
    }
	
	// MAIN LOOP: 
	// wait for multi-tasking function to complete 
	if (!rc) rc = event(); //check for events
    
	if (rc > 0)
	{
		sprintf(s, "ERROR: %s. Press a key...", d4xerr(rc));
		writestatus(s);
		recplay = rpERROR;
		clrevent(); //clear event
	}
	
	if (handle > 0) vhclose(handle);
	
}            

/* end recplay.c */


