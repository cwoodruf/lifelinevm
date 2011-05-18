
//lldate.c - part of the ansr3.c program

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

void loadmsgtime(int channel, int *is_am, int *i, int num[])
{
	int j = *i;
	
	if (*is_am == NOTFOUND) //hour
	{
		DATA[j] = numbers[gethour(num[nHOUR], is_am)];
	}
	else //minute
	{
		if (num[nMIN] >= 20)
		{
			DATA[j] = numbers[TENS + (num[nMIN] - 20)/10];
			j++;
			DATA[j] = numbers[num[nMIN] % 10];
		}
		else 
		{
			if (num[nMIN] < 10)
			{
				DATA[j] = numbers[OH];
				j++;
			}
			DATA[j] = numbers[num[nMIN]];
		}
	}
	*i = j;
}

int getnumdata(int channel, int mask[], int num[], int masksize)
{
	int i, j, ampm, offset;
	int is_am = NOTFOUND;
	
	for (i = 0, j = 0; i < masksize; i++,j++) 
	{
		switch (mask[i])
		{
			case DATENUM:
				if (num[nDAY] >= 20)
				{
					if (num[nDAY] % 10 == 0) offset = TENSTH;
					else offset = TENS;
					DATA[j] = numbers[offset + (num[nDAY] - 20)/10];
					j++;
					DATA[j] = numbers[DIGITSTH + (num[nDAY] % 10)];
				}
				else DATA[j] = numbers[DIGITSTH + num[nDAY]];
			break;
			case OF:
				DATA[j] = numbers[OF];
			break;
			case MONTH:
				DATA[j] = numbers[MONTH + num[nMONTH] - 1];
			break;
			case AT:
				DATA[j] = numbers[AT];
			break;
			case TIMENUM:
				if (is_am != NOTFOUND && num[nMIN] == 0) 
					continue; //ignore zero minutes
				loadmsgtime(channel, &is_am, &j, num);
			break;
			case AM:
				if (is_am) 
					ampm = AM;
				else ampm = PM;
				DATA[j] = numbers[ampm];
			break;
			default: break;
		}
	}
	DATA[j].offset = DATASTOP;
	return j;
}

int playnum(int channel, int mask[], int num[], int masksize)
{
    int size; 
    size = getnumdata(channel, mask, num, masksize);
    if (size <= 0) return ERR; //no data
        
    clrdtmf(channel);            /* empty out any saved digits */
    clrxrwb(&d4xrwb);            /* clear the D/4x read/write block */
    d4xrwb.filehndl = numh;      /* handle of number file */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */
    d4xrwb.indexseg = d4getseg((char *)DATA);
    d4xrwb.indexoff = d4getoff((char *)DATA); 
        
    return (xplayf(channel, PM_NDX, &d4xrwb));
}

int playmsgdate(int channel)
{
	int msgdate[4]; //day, mon, hour, min
	MI minfo;
	struct _find_t f;
	
	_dos_findfirst(msgname(channel), _A_NORMAL, &f);
	
	fillmsginfo(&minfo, &f);
	
	msgdate[nDAY] = minfo.Day;
	msgdate[nMONTH] = minfo.Month;
	msgdate[nHOUR] = minfo.Hour;
	msgdate[nMIN] = minfo.Minute;

	return playnum(channel, datemask, msgdate, DATEMASKSIZE);
}

int playtest(int channel) //play all numbers
{
	static MS testms[AT + 2] = {0};
	int i = 1;
	
	for (i = 1; i <= AT; i++)
	{
		testms[i - 1] = numbers[i];
	}
	testms[AT].offset = DATASTOP;

    clrdtmf(channel);            /* empty out any saved digits */
    clrxrwb(&d4xrwb);            /* clear the D/4x read/write block */
    d4xrwb.filehndl = numh;    /* handle of file to play */
    d4xrwb.maxdtmf  = 1;         /* cause an event if max digits */
    d4xrwb.loopsig  = 1;         /* terminate on loop signal drop */
    d4xrwb.indexseg = d4getseg((char *)testms);
    d4xrwb.indexoff = d4getoff((char *)testms); 
        
    return (xplayf(channel, PM_NDX, &d4xrwb));
}	
//end lldate.c





