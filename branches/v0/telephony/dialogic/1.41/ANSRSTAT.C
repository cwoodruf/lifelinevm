/* 
	ansrstat.c : common routines for lifeline stats 
    used by lifeline and llcnvt
*/

#ifdef LLCNVT

/* these are already defined in ansrvar.h for lifeline */

int cperh[24] = {0}; /* keep track of calls for the last 24 hours */
int mperh[24] = {0}; //messages left per hour
int pgcalls[24] = {0}; //cumulative pages
int chanwait[24] = {0}; //maximum ammount of time to wait for free channel
int chansfull[24] = {0}; //seconds all channels have been in use per hour
int chansffreq[24] = {0};
int maxcallshr = 0;
int maxhr = 0;
char s[400]; //general string for messages
int numusers;
int hour, ttlcalls, ttlmsgs;

#endif

int sumhrarray(int a[])
{
	register i;
	int sum = 0;
	
	for (i = 0; i < MAXHR; i++)
		sum += a[i];
	return sum;
}

int maxhrarray(int a[])
{
	register i;
	int max = 0;
	
	for (i = 0; i < MAXHR; i++)
		if (a[i] > max) max = a[i];
	return max;
}

int minhrarray(int a[])
{
	register i;
	int min = 0xFFFF;
	
	for (i = 0; i < MAXHR; i++)
		if (a[i] < min) min = a[i];
	return min;
}

int loadhrarray(int h, int cperh[])
{
	register i;
	int calls;
	
	i = 0;
	while (!eof(h) && i < MAXHR)
	{
		_read(h, &calls, sizeof(int));
		cperh[i] = calls;
		if (cperh[i] > maxcallshr) 
		{
			maxcallshr = cperh[i];
			maxhr = i;
		}
		i++;
	}

	if (i >= MAXHR) return TRUE;
	return FALSE;
}

void loadcperh( void )
{
	int h;

	if ((h = _open(CPERH, _O_BINARY|_O_RDWR)) < 0) 
		return;

    if (loadhrarray(h, cperh)) 
    {
    	if (loadhrarray(h, mperh))
    	{
    		if(loadhrarray(h, chanwait))
    		{
    			if (loadhrarray(h, chansfull))
    			{
    				if (loadhrarray(h, chansffreq))
    					loadhrarray(h, pgcalls);
    			}
    		}
    	}
    }
	
	_close(h);
}

void savehrarray(int h, int cperh[])
{
	register i;
	int calls;
	
	for (i = 0; i < MAXHR; i++)
	{
		calls = cperh[i];
		_write(h, &calls, sizeof(int));
		_commit(h);
	}
}

void savecperh( void )
{
	int h;

	if ((h = _open(CPERH, _O_BINARY|_O_RDWR)) < 0) 
	{
		if ((h = _open(CPERH, _O_RDWR|_O_BINARY|_O_CREAT, _S_IWRITE|_S_IREAD)) < 0) 
			return;
	}

	cperh[hour] = ttlcalls;
	mperh[hour] = ttlmsgs;

    savehrarray(h, cperh); 
    savehrarray(h, mperh);
	savehrarray(h, chanwait);
    savehrarray(h, chansfull);
    savehrarray(h, chansffreq);
    savehrarray(h, pgcalls);
	
	_close(h);
}
