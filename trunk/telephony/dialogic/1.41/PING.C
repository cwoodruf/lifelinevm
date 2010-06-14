/* ping.c - call a number (the voicemail system) and check response */
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <signal.h>

/**
 **   Dialogic header files
 **/
#include <vfcns.h>
#include <d40.h>           /* standard DIALOG/4x header  */
#include <d40lib.h>

#define STRSIZE 256
#define TRUE    1
#define FALSE   0
#define MINUTE  60
#define DEFCHAN 1
#define DEFINT  3
#define DEFWAIT 30
#define MAXNOTIFYCALLS 10
#define PORT    port[channel]

#define PINGCONF "pingconf.txt"
#define PINGFILE "ping.txt"
#define NOTIFYFILE "notify.txt"
#define PINGLOG "pinglog.txt"
#define NOTIFYPROMPT "notify.vox"

FILE *logh;
RWB d4xrwb = {0};             /* read/write block for the D/4x */
CSB csb = {0};                /* call status block */
EVTBLK event;                 /* event data block               */
CPB cpb; 					  // channel parameter block
char helloedge = TRUE;        // determines when connect is detected
int notifycalls = 0;		  // number of times notification was attempted
char msg[STRSIZE+1] = {0};

int pause(int);		int pause_cmplt(int);
int offhk(int); 	int offhk_cmplt(int);
int ping(int); 		int ping_cmplt(int);
int notify(int); 	int notify_cmplt(int);
int nprompt(int);	int nprompt_cmplt(int);
int onhk(int); 		int onhk_cmplt(int);

int getevent(EVTBLK*);
int set_hkstate(int,int);
char *timestr(void);
void open_log (void);
void load_conf(int*,int*,int*);
int sysinit(int,int);
int call_response (int,int);
int play(int,int);
void logmsg(char *);
char *getphone(char *);
void ctrlc(int);

enum stateids {
	sWAIT,
	sOFFHK,
	sPING,
	sNOTIFY,
	sNPROMPT,
	sONHK
};
struct a_state {
	enum stateids state;
	char desc[STRSIZE+1];
	int (*init)();
	int (*next)();
	int h;
	char file[STRSIZE+1];
} states[] = {
	{sWAIT,"waiting to make a call\n",pause,pause_cmplt,0,""},
	{sOFFHK,"off hook\n",offhk,offhk_cmplt,0,""},
	{sPING,"calling ping number: ",ping,ping_cmplt,0,""},
	{sNOTIFY,"calling notify number: ",notify,notify_cmplt,0,""},
	{sNPROMPT,"notify prompt\n",nprompt,nprompt_cmplt,0,NOTIFYPROMPT},
	{sONHK,"hanging up\n",onhk,onhk_cmplt,0,""}
};

struct a_port {
	int channel;
	int wait;
	int rc;
	int ecode;
	enum stateids curr;
	enum stateids next;
	char phone[STRSIZE+1];
} port[] = {
	{0,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""}, // not a real port
	{1,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{2,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{3,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{4,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{5,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{6,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{7,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{8,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{9,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{10,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{11,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""},
	{12,DEFWAIT,E_SUCC,0,sONHK,sWAIT,""}
};

int main (void) {
	int channel = DEFCHAN;
	int d4xint = DEFINT;
	int wait = DEFWAIT; // min between calls
	open_log();
	load_conf(&channel,&d4xint,&wait);
	sprintf(msg,"channel %d, d4xint %d, wait %d minutes\n",channel,d4xint,wait);
	logmsg(msg);
	sysinit(d4xint,channel);
	PORT.wait = wait;
	// sWAIT doesn't really do anything so it needs to be handled differently
	while (PORT.curr == sWAIT || getevent(&event)) {
		channel = event.devchan;
		PORT.ecode = event.evtcode;
		PORT.curr = (*states[PORT.curr].next)(channel);
		logmsg(states[PORT.curr].desc);
		PORT.rc = (*states[PORT.curr].init)(channel);
		if (PORT.rc) {
			sprintf(msg,"FAIL code %d\n",PORT.rc);
			logmsg(msg);
		}
	}
	stopsys();
	return 0;
}

int pause(int channel) {
	time_t t;
	time_t nextcheck;
	nextcheck = time(&t) + (PORT.wait * MINUTE);
	PORT.next = sWAIT; // for the sONHK state
	printf("nextcheck %ld, now %ld\n",nextcheck,t);
	// while (time(&t) < nextcheck) { if (kbhit()) break; }
	while (time(&t) < nextcheck) { }
	return 0;
}

int pause_cmplt(int channel) {
	return sOFFHK;
}

int offhk(int channel) {
	return set_hkstate(channel,H_OFFH);
}

int offhk_cmplt(int channel) {
	if (PORT.ecode == T_OFFH) {
		return sPING;
	} 
	return sONHK;
}

int ping(int channel) {
	clrdtmf(PORT.channel);
	strcpy(PORT.phone,getphone(PINGFILE));
	printf("%s\n",PORT.phone);
	fprintf(logh,"%s\n",PORT.phone); fflush(logh);
	return callp(channel, PORT.phone);
}

int ping_cmplt(int channel) {
	PORT.next = sWAIT;
	return call_response(channel,sONHK);
}

int notify(int channel) {
	notifycalls++;
	clrdtmf(channel);
	strcpy(PORT.phone,getphone(NOTIFYFILE));
	printf("%s\n",PORT.phone);
	fprintf(logh,"%s\n",PORT.phone); fflush(logh);
	return callp(channel, PORT.phone);
}

int notify_cmplt(int channel) {
	if (notifycalls >= MAXNOTIFYCALLS) {
		call_response(channel,sONHK);
		notifycalls = 0;
		PORT.next = sWAIT;
		return sONHK;                            
	}	
	PORT.next = sWAIT;
	return call_response(channel,sNPROMPT);
}

int call_response (int channel, int state) {
	int callok = 0;
	logmsg("call response: ");
	switch (event.evtdata) {
		case CA_CONN: 
			strcpy(msg,"OK connect\n");
			notifycalls = 0;
			callok = 1;
			break;
		case CA_BUSY: 
			strcpy(msg,"WARNING busy\n"); break;
		case CA_NOAN:  
			strcpy(msg,"ERROR no answer\n"); break;
		case CA_NORNG: 
			strcpy(msg,"ERROR no ring\n"); break;
		case CA_OPINT: 
			strcpy(msg,"ERROR operator interrupt\n"); break;
		default: 
			sprintf(msg,"ERROR unknown response %d\n",event.evtdata); break;
	}
	printf(msg);
	fprintf(logh,msg);
	fflush(logh);
	if (!callok) PORT.next = sNOTIFY;
	return state;
}

int nprompt(int channel) {
	if ((states[PORT.curr].h=vhopen(states[PORT.curr].file,READ)) == 0)	return -1;
	return play(channel,states[PORT.curr].h);
}

int nprompt_cmplt(int channel) {
	vhclose(states[PORT.curr].h);
	return sONHK;
}

int onhk(int channel) {
	return set_hkstate(channel,H_ONH);
}

int onhk_cmplt(int channel) {
	time_t sleep;
	time_t t;
	sleep = time(&t) + 10;
	logmsg("sleeping for 10 seconds\n");
	while (time(&t) < sleep) { if (kbhit()) break; }
	return PORT.next; 
}

int set_hkstate(int channel,int state) {
	return sethook(channel, (state==H_ONH)?H_ONH:H_OFFH);
}

/*======================= utility funcs =============================*/
int play(int channel, int h) {
    /* rewind to top of file */
    vhseek(h,0L,0);

    clrdtmf(channel);            /* empty out any saved digits */
    clrxrwb(&d4xrwb);             /* clear the D/4x read/write block */
    d4xrwb.filehndl = h;         /* handle of file to play */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */

    /* play vox file on D/4x channel, normal play back */
    return(xplayf(channel,PM_NORM,&d4xrwb));        
}

void logmsg(char *msg) {
	printf("%s: %s",timestr(),msg);
	fprintf(logh,"%s: %s",timestr(),msg);
	fflush(logh);
}

int getevent(EVTBLK *event) {
	while(1) {
		if (gtevtblk(event) == -1) return TRUE;
    }
    return FALSE;
}

char *timestr(void) {
	static char tstr[STRSIZE+1] = {0};	
	time_t s;
	struct tm *t;
	time(&s);
	t = localtime(&s);
	sprintf(tstr,"%04d/%02d/%02d %02d:%02d:%02d",
				t->tm_year+1900,
				t->tm_mon+1,
				t->tm_mday,
				t->tm_hour,
				t->tm_min,
				t->tm_sec
	);
	return tstr;
}

void open_log(void) {
	if ((logh = fopen(PINGLOG,"a")) != NULL) {
		printf("%s: logging to %s\n",timestr(),PINGLOG);
		if( setvbuf( logh, NULL, _IONBF, 0 ) == 0 ) {
			logmsg("Logging started\n");
		} else {
			printf("%s: error eliminating buffer for log\n");
			exit(2);
		}
	} else {
		printf("%s: can't open log file!\n",timestr());
		exit(1);
	}
}

void load_conf(int *channel, int *d4xint, int *wait) {
	FILE *f;
	char *t;
	char name[STRSIZE+1] = {0};
	char value[STRSIZE+1] = {0};
	char buff[STRSIZE+1] = {0};
	
	if ((f=fopen(PINGCONF,"r")) != NULL) {
		while (!feof(f) && !ferror(f)) {
			if (fgets(buff,STRSIZE,f) == NULL) continue;
			if (strlen(buff) == 0) continue;
			t = strtok(buff,"\n=");
			strcpy(name,t);
			t = strtok(NULL,"\n=");
			strcpy(value,t);
			if (!strcmp(name,"channel")) *channel = atoi(value);
			else if (!strcmp(name, "interrupt")) *d4xint = atoi(value);
			else if (!strcmp(name, "wait")) *wait = atoi(value);
			sprintf(msg,"param: name %s, value %s\n",name,value);
			logmsg(msg);
		}
		fclose(f);
	}
}

void ctrlc(int sig) {
	stopsys();
	exit(3);
}

int sysinit(int d4xint,int channel) {
	int rc;  /* return code from startsys() */
	int numchans;
	if (signal(SIGINT, ctrlc) == SIG_ERR) {
		logmsg("Unable to redirect ctrl-c.\n");
		abort();
	}
	logmsg("Ctrl-C handler installed.\n");
	
	
	/* check for the D/4x driver */
	if (!getvctr())  {
		logmsg("DIALOG/4x voice driver not installed.\n");
		return FALSE;
	}
	logmsg("Found Dialog/4x voice driver.\n");
	//make sure system is shut down before initializing
	stopsys();
	//initialize system (all following commands)
	rc = startsys(d4xint,SM_EVENT,0,0,&numchans);
	if (rc != E_SUCC) {
		sprintf(msg,"Unable to start voice system, Return code %d.\n",rc);
		logmsg(msg);
		return FALSE;
	}
	logmsg("Voice system started.\n");
	
	/* chan can't be greater than maxchan */
	if (channel > numchans) {
		logmsg("invalid channel selected!\n");
		return FALSE;
	}
	      
	/* set channel to make call */
	if (helloedge) {
		clrcpb(&cpb);
		cpb.hedge = 1; //detect connect immediately, disable ans mach detect
	}
	setcst(channel, (C_RING+C_OFFH+C_ONH+C_LC),10); // set call status block so it does not respond to rings
	if (helloedge) setcparm(channel, &cpb);
	set_hkstate(channel,H_ONH); /* put line on hook */ 	
    logmsg("channel set\n");
    
	return TRUE;
}

char *getphone(char *file) {
	int i,len,j;
	char buff[STRSIZE+1] = {0};
	static char num[STRSIZE+1] = {0};
	FILE *f;
	if ((f=fopen(file,"r")) != NULL) {
		fgets(buff,STRSIZE,f);
		fclose(f);
		len = strlen(buff);
		for (i=0,j=0;i<len;i++) {
			if (buff[i] >= '0' && buff[i] <= '9') {
				num[j++] = buff[i];
			}
		}
		num[j] = 0;
		return num;
	} 
	return NULL;
}

/* end of ping.c */
