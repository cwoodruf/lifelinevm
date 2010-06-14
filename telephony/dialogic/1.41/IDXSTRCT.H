
//idxstrct.h - part of Lifeline group of programs

//Special data structures and variables for multi-prompt files
#define ANYLANG "ANY"
#define DEFLANG	"ENGLISH" //root name of default language multi-prompt files
#define VXP ".VXP" //multi prompt vox file
#define PMASK "*.VXP"
#define IXP ".IXP" //index file for multi prompt file

#define VXN ".VXN" //multi prompt vox file for date and number elements
#define NMASK "*.VXN"
#define IXN ".IXN" //index file for multi prompt number file

#define  MAXPROMPTF   10	   //maximum number of prompt files for multi-language support
#define  PROMPT(x,y)  promptf[x].pidx[states[(y)].fh] //which prompt you are playing from indexed prompt file
#define  NUMBERS promptf[PRT.language].nidx
#define  NUMH	 promptf[PRT.language].nh

#ifndef LLCNVT
//used to store index information for prompts
typedef struct idxstruct
{
	char name[20];
	unsigned long pos;
	unsigned long len;
}IS;
#endif

//end of idxstrct.h


