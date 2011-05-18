#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#define HANDLES_MAX 200
/* test of file open */
void open_many (void) {
	int c = 0,i;
//	FILE * f;
	int h[HANDLES_MAX];
	struct _find_t item;
	_dos_findfirst("*.*",_A_NORMAL,&item);
	do {
//		if ((f = fopen(item.name,"r")) == NULL) {
		if ((h[c] = _open(item.name,_O_RDONLY)) == -1) {
			printf("failed on %s\n",item.name);
			if (errno == EMFILE) printf("ran out of file handles\n");
			break;
		}
		c++;
	} while (c < HANDLES_MAX && !_dos_findnext(&item));
	printf("opened %d files\n",c);
	getch();
	for (i = 0; i < c; i++) _close(h[i]);
}