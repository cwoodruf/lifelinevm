/* disk.c - some disk utilities */

#include <dos.h>
#include <errno.h>
#define FCB_SIZE 128 /* file control box size */

int fexist(char *fname)
{
	struct _find_t f;
	
	return !_dos_findfirst(fname, _A_NORMAL, &f);
}	

unsigned getdisk(char *path)
{
	char fpath[DOSPATHLEN + 1];
	
	if (_fullpath(fpath, path, DOSPATHLEN) == NULL) //error
		return (unsigned) -1; //won't return -1 but it will be bigger value than any drive
	return (unsigned)(toupper(fpath[0]) - 'A');
}

unsigned long disksize(unsigned disk)
{
    struct diskfree_t d;
    unsigned long size;
    _dos_getdiskfree(disk, &d);
    size = (unsigned long)d.total_clusters * 
           (unsigned long)d.sectors_per_cluster * 
           (unsigned long)d.bytes_per_sector;
    return (size);
}

unsigned long diskavail(unsigned disk)
{
    struct diskfree_t d;
    unsigned long size;
    _dos_getdiskfree(disk, &d);
    size = (unsigned long)d.avail_clusters * 
           (unsigned long)d.sectors_per_cluster * 
           (unsigned long)d.bytes_per_sector;
    return (size);
}

int diskfull( unsigned disk,unsigned long size )
{
    if (size > diskavail(disk))
        return TRUE;
    return FALSE;    
}

unsigned long diskpercentfree( unsigned disk )
{
   unsigned long dp;
   dp = (diskavail(disk) * 100L) / disksize(disk);
   return (dp); 
}

unsigned long diskpercentfull( unsigned disk )
{
    return 100L - diskpercentfree( disk );
}

unsigned long getsize(char *mask)
{                                       
    struct _find_t f;
    unsigned long size = 0;
    int i;
    
    if (!_dos_findfirst(mask, _A_NORMAL, &f))
    {                        
        i = 0;
        do
        {
        #ifdef DIALOGIC         
        /* do some voice processing every 100 files */
            if ((++i % 100) == 0)
            {
                sched();
                i = 0;
            }    
        #endif
            size += f.size;
        }
        while (!_dos_findnext(&f));
    }                      
    return size;
}

#include <dos.h>
#include <fcntl.h>
#include <conio.h>
#include <stdio.h>

/* Copies one file to another (both specified by path). Dynamically
 * allocates memory for the file buffer. Prompts before overwriting
 * existing file. Returns 0 if successful, or an error number if
 * unsuccessful. This function uses _dos_ functions only; standard
 * C functions are not used.
 */

int copyfile( int hsource, int htarget )
{
    char __far *buf = NULL;
    unsigned ret, segbuf, count;

    /* Attempt to dynamically allocate all of memory (0xffff paragraphs).
     * This will fail, but will return the amount actually available
     * in segbuf. Then allocate this amount.
     */
    _dos_allocmem( 0xffff, &segbuf );
    count = segbuf;
    if( ret = _dos_allocmem( count, &segbuf ) )
        return ret;
    _FP_SEG( buf ) = segbuf;
    /* Read and write until there is nothing left. */
    while( count > 0 )
    {
        /* Read and write input. */
        if( (count = _read( hsource, buf, count)) == -1)
            return errno;
        if( (count = _write( htarget, buf, count)) == -1)
            return errno;
    }
    /* free memory. */
    _dos_freemem( segbuf );
    return 0;
}

//copy only part of the source file
int copyfilesection( int hsource, int htarget, unsigned long start, unsigned long size )
{
    char __far *buf = NULL;
    unsigned ret, segbuf, count;

	//check that file section to be copied does not exceed size of file
	if ((unsigned long) _filelength(hsource) < start + size) return TRUE;
	
    /* Attempt to dynamically allocate all of memory (0xffff paragraphs).
     * This will fail, but will return the amount actually available
     * in segbuf. Then allocate this amount.
     */
    _dos_allocmem( 0xffff, &segbuf );
    count = segbuf;
    if( ret = _dos_allocmem( count, &segbuf ) )
        return ret;
    _FP_SEG( buf ) = segbuf;
    // go to start of section
    _lseek(hsource, start, SEEK_SET);
    /* Read and write until there is nothing left. */
    while( size > 0L && count > 0 )
    {
        //check what you have left to copy
        if ((unsigned long) count >= size)
        {
       		count = (int) size;
       		size = 0L;
       	}
        else size -= (unsigned long) count;
        
        /* Read and write input. */
        if( (count = _read( hsource, buf, count)) == -1)
            return errno;
        if( (count = _write( htarget, buf, count)) == -1)
            return errno;
    }
    /* free memory. */
    _dos_freemem( segbuf );
    return 0;
}
/* end disk.c */















