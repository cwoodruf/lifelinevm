
//idxplay.c - part of the ansr3.c program - see also play funcs in util.c

int gethour(int num, int *is_am)
{
	if (num == 0)
	{ 
		*is_am = TRUE;
		num = 12;
	}
	else if (num < 12)
		*is_am = TRUE;
	else if (num == 12)
		*is_am = FALSE;
	else
	{
		num -= 12;
		*is_am = FALSE;
	}
	return num;
}

void loadmsgtime(int channel, int *is_am, int *i, int tnum, int type)
{
	int j = *i;
	
	if (type == HOUR) //hour
	{
		DATA[j] = NUMBERS[gethour(tnum, is_am)];
	}
	else //minute or year
	{
		if (tnum >= 20)
		{
			if (tnum > 59) DATA[j] = NUMBERS[(tnum/10)];
			else DATA[j] = NUMBERS[TENS + (tnum - 20)/10];
			if (tnum % 10)
			{
				j++;
				DATA[j] = NUMBERS[tnum % 10];
			}
		}
		else 
		{
			switch (type)
			{
				case YEARNUM:
				case MINUTE:
					if (tnum < 10)
					{
						DATA[j] = NUMBERS[OH];
						j++;
					}
			}
			DATA[j] = NUMBERS[tnum];
		}
	}
	*i = j;
}

void loadlong(int channel, int *p, long num)
{
	int j;
	long l;
	
	j = *p;
	
	if (num >= 1000000000L)
	{
		loadlong(channel, &j, (num / 1000000000L));
		DATA[j++] = NUMBERS[BILLION];
		num %= 1000000000L;
	}
	
	if (num >= 1000000L)
	{
		loadlong(channel, &j, (num / 1000000L));
		DATA[j++] = NUMBERS[MILLION];
		num %= 1000000L;
	}
	
	if (num >= 1000L)
	{
		loadlong(channel, &j, (num / 1000L));
		DATA[j++] = NUMBERS[THOUSAND];
		num %= 1000L;
	}
	
    if (num >= 100L)
	{
		DATA[j++] = NUMBERS[num / 100L];
		DATA[j++] = NUMBERS[HUNDRED];
		num %= 100L;
	}
	
	if (num > 20L)
	{
	    l = (long) TENS + (num / 10L) - 2L;
		DATA[j++] = NUMBERS[l];
		if (num % 10L) DATA[j++] = NUMBERS[num % 10L];
	}
	else DATA[j++] = NUMBERS[num];
	
	*p = j;
}

int getnumdata(int channel, int mask[], long num[], int masksize)
{
	int i, j, n, ampm, century;
	int is_am = NOTFOUND;

	for (i = 0, j = 0, n = 0; i < masksize; i++,j++) 
	{
		switch (mask[i])
		{
			case TNULL: 
				j--; 
			continue;
			case INT:
				if (num[n] < 0) 
				{
					DATA[j++] = NUMBERS[MINUS];
					if (num[n] < 0)
						num[n] = labs(num[n]);					
					else continue;
				}
				loadlong(channel, &j, num[n]);
				j--;
				n++;
			break;
			case DATENUM:
				if (num[n] >= 20)
				{
					if (num[n] % 10 == 0) 
					{
						DATA[j] = NUMBERS[(long) TENSTH + (num[n] - 20L)/10L];
					}
					else 
					{
						DATA[j] = NUMBERS[(long) TENS + (num[n] - 20L)/10L];
						j++;
						DATA[j] = NUMBERS[(long) DIGITSTH + (num[n] % 10L)];
					}
				}
				else DATA[j] = NUMBERS[(long) DIGITSTH + num[n]];
				n++;
			break;
			case OF:
				DATA[j] = NUMBERS[OF];
			break;
			case MONTH:
				DATA[j] = NUMBERS[(long) MONTH + num[n++] - 1L];
			break;
			case AT:
				DATA[j] = NUMBERS[AT];
			break;
			case STAR:
				DATA[j] = NUMBERS[STAR]; n++;
			break;
			case POUND:
				DATA[j] = NUMBERS[POUND]; n++;
			break;
			case MINUTE:
				if(num[nMIN] == 0) 
					continue; //ignore zero minutes
				loadmsgtime(channel, &is_am, &j, (int) num[n++], mask[i]);
			break;
			case HOUR:
				loadmsgtime(channel, &is_am, &j, (int) num[n++], mask[i]);
			break;
			case YEARNUM:
				century = (int) (num[n]/100L);
				num[n] -= century*100L;
				loadmsgtime(channel, &is_am, &j, (int) century, mask[i]);
				j++;
				loadmsgtime(channel, &is_am, &j, (int) num[n], mask[i]);
				n++;
			break;
			case AM:
				if (is_am) 
					ampm = AM;
				else ampm = PM;
				DATA[j] = NUMBERS[ampm];
			break;
			default: break;
		}
		//check for valid values: on error don't play segment
		if (DATA[j].offset + DATA[j].length > 
			(unsigned long) filelength(NUMH))
		{
			DATA[0].offset = DATASTOP;
			return 0;
		}
	}

	DATA[j].offset = DATASTOP;
	return j;
}

int playnum(int channel, int mask[], long num[], int masksize)
{
    int size; 
    size = getnumdata(channel, mask, num, masksize);
    //if (size <= 0) return ERR; //no data
        
    clrdtmf(channel);            /* empty out any saved digits */
    clrxrwb(&d4xrwb);            /* clear the D/4x read/write block */
    d4xrwb.filehndl = NUMH;      /* handle of number file */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */
    d4xrwb.indexseg = d4getseg((char *)DATA);
    d4xrwb.indexoff = d4getoff((char *)DATA); 
        
    return (xplayf(channel, PM_NDX, &d4xrwb));
}

int playdate(int channel, int d, int m, int y)
{
	int masksize;
	long date[3] = {0};

	date[0] = (long) d;
	date[1] = (long) m;
	//if the year is this year don't play the year
	if (y > 2000)
	{
		date[2] = (long) y;
		masksize = PDTOMASKSIZE;
	}
	else masksize = PDTOMASKSIZE - 1;

	return playnum(channel, pdtomask, date, masksize);
}

int playtime(int channel, int h, int m)
{
	long time[2] = {0};

	time[0] = (long) h;
	time[1] = (long) m;
//	time[2] = h;
//	time[3] = m;

	return playnum(channel, timemask, time, TIMEMASKSIZE);
}

int playmsgdate(int channel)
{
	long msgdate[4]; //day, mon, hour, min
	MI minfo;
	struct _find_t f;
	
	_dos_findfirst(msgname(channel), _A_NORMAL, &f);
	
	fillmsginfo(&minfo, &f);
	
	msgdate[nDAY] = (long) (minfo.Day);
	msgdate[nMONTH] = (long) (minfo.Month);
	msgdate[nHOUR] = (long) (minfo.Hour);
	msgdate[nMIN] = (long) (minfo.Minute);
	return playnum(channel, datemask, msgdate, DATEMASKSIZE);
}

int playpdtodate(int channel, US u)
{
	int y,m,d;
	char pdtostr[TIMELEN + 1];
	time_t new;
	
	if (u.boxtype == btNEW)
	{
		if (PRT.newpaidto)
		{
			new = PRT.newpaidto;
			strcpy(pdtostr, datestr(&new));
		}
		else strcpy(pdtostr, u.paidto);
	}
	else strcpy(pdtostr, u.paidto);
	
	if (!getdate(pdtostr, &d, &m, &y)) return NOTFOUND;

	return playdate(channel, d, m, y);
}

int playtime_t(int channel, time_t *t)
{
	struct tm *tv;
	
    if (t <= 0L) return NOTFOUND;
    
    tv = localtime( t );      /* Convert to local time. */
	
	return playdate(channel, tv->tm_mday, tv->tm_mon + 1, tv->tm_year + 1900);
}

int play4digits(int channel, char *num)
{
	register i;
	long boxnum[BOXLEN];
	int mask4[BOXLEN];
	
	for (i = 0; i < BOXLEN; i++)
	{
		switch (num[i])
		{
			case '*': 
				mask4[i] = STAR;
				boxnum[i] = (long) NOTFOUND;
			break;
			case '#':
				mask4[i] = POUND;
				boxnum[i] = (long) NOTFOUND;
			break;
			default:
				mask4[i] = INT;
				boxnum[i] = (long) (num[i] - '0');
		}
	}
	
	return playnum(channel, mask4, boxnum, BOXLEN);
}

int playpaycode(int channel, char *num)
{
	register i;
	int len = strlen(num);
	long pc[PAYCODELEN];
	
	for (i = 0; i < len; i++)
	{
		switch (num[i])
		{
			case '*': pc[i] = (long) STAR; break;
			case '#': pc[i] = (long) POUND; break;
			default: pc[i] = (long) (num[i] - '0');
		}
	}
	
	return playnum(channel, paycodemask, pc, len);
}

int playint(int channel, int n)
{
	long ln;
	ln = (long) n;
	return playnum(channel, paycodemask, &ln, 1);
}

int playlong(int channel, long n)
{
	return playnum(channel, paycodemask, &n, 1);
}

int playtest(int channel) //play all numbers
{
	static MS testms[MAXNUM + 2] = {0};
	int i = 1;
	
	for (i = 1; i < MAXNUM; i++)
	{
		testms[i - 1] = NUMBERS[i];
	}
	testms[MAXNUM - 1].offset = DATASTOP;

    clrdtmf(channel);            /* empty out any saved digits */
    clrxrwb(&d4xrwb);            /* clear the D/4x read/write block */
    d4xrwb.filehndl = NUMH;    /* handle of file to play */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */
    d4xrwb.indexseg = d4getseg((char *)testms);
    d4xrwb.indexoff = d4getoff((char *)testms); 
        
    return (xplayf(channel, PM_NDX, &d4xrwb));
}	

//end idxplay.c





