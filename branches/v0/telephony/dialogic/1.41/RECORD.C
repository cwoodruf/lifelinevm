
/* record.c - record a named file from the command line */

/**
 **   Microsoft header files
 **/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>

/* my source files */   

/* general functions and defs */
#include <screen.c>        /* basic screen handling functions */

/* ansr3 header file */
#include <ansrdefs.h>
/* single line record and play */
#include "recplay.h"
#include "recplay.c"

void main (int argc, char *argv[])
{
    strcpy(filename, "No default");
    progname = getprogname(argv);
    if (argc == 1) 
    {
        writeoptions("Record");
        exit(1);
    }    
    sysinit(getargs(argc,argv,"Record"));
    recloop(filename);
}

/* end record.c */
