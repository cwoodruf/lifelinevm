
/* recplay.h - header for recplay.c single line record and playback functions */
/**
 **   Microsoft header files
 **/
 
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

/**
 **   Dialogic header files
 **/
#include "d40.h"           /* standard DIALOG/4x header  */
#include "d40lib.h"
#include <vfcns.h>

/* defines */

#define RECPLAY

#define DEFINTR 5 /* default d4x interrupt */
#define TRUE 1
#define FALSE 0

#define CHANNEL 1 /* default channel */
#define RINGS 1 /* respond on one ring */

#define BADCODE -1 /* for loop function */

/* some standard strings to use in programs */
#define SYSINACTIVE "Voice system is not available!"
#define RECPROMPT "Press ENTER to start recording (ESC to exit)..."
#define RECSTR "Recording (press a key to end record)..."
#define RECERRSTR "Error recording file. Press a key..."
#define PLAYPROMPT "Press ENTER to start playback (ESC to exit)..."
#define PLAYSTR "Playing file (press a key to end playback)..."
#define PLAYERRSTR "Error playing file. Press a key..."
#define INDEXPROMPT "Press ENTER to start index record (ESC to exit)..."
#define INDEXSTR "Record file. ENTER marks sections. ESC to exit)..."
#define INDEXERRSTR "Error indexing file. Press a key..."
#define NOFILEERR "Unable to open file! Press a key..."
#define NOIDXFILEERR "Unable to open index file! Press a key..."

void writeoptions( char *desc );
int getargs( int argc, char *argv[], char *desc );
int sysinit(int d4xint);
int set_hkstate( int state );
void recplayloop( char *fname );
int event( void );
void clrevent( void );
int play(int handle);
int record(int handle);
int loop(char *fname);

/* global variables */
enum status {sREADY, 
			 sWAITREC, sRECORD, 
			 sWAITPLAY, sPLAY, 
			 sWAITINDEX, sINDEX, 
			 sRECERR, sPLAYERR, 
			 sINDEXERR,
			 sNOFILEERR,
			 sNOIDXFILEERR};

char statstr[][81] = 
    {
#ifdef LLRECNUM
    "HOME/END/PGUP/PGDN to select word to record.",
#else
    "HOME/END/PGUP/PGDN to select script to record.",
#endif
    RECPROMPT,
    RECSTR,
    PLAYPROMPT,
    PLAYSTR,
    INDEXPROMPT,
    INDEXSTR,
    RECERRSTR,
    PLAYERRSTR,
    INDEXERRSTR,
    NOFILEERR,
    NOIDXFILEERR
    };

enum rp {rpIDLE, rpRECORD, rpPLAY, rpINDEX, rpERROR};
enum rp recplay = FALSE;

int channel = CHANNEL;
char filename[DOSPATHLEN] = ""; 
int d4xint; /* interrupt */
char *progname;
int sysactive; //variable that lets program know if voice system is on

/* end of recplay.h */



