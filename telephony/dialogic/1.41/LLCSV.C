
// llcsv.c - converts pay and user info from lifeline to ascii delimited format
// with special date format

#define MIDS ","

char *convertdate(char *date)
{
#define DAY 0
#define MON 1
#define YR  2 
#define DT 3
	
	char s[DT][5] = {0};
	int n[DT] = {0};
	unsigned int i, j = 0, k = 0, l;
	char mask[20] = DATEMASK;
	
	//check length
	if ((l = strlen(mask)) != strlen(date))
	    return date;
	//load strings    
	for (i = 0; i < l; i++)
	{
		switch(mask[i])
		{
			case 'n':
				//make sure the numbers are numbers
				if (date[i] < '0' || date[i] > '9')
					return date;
			case 'a':
				s[j][k++] = date[i];
			break;
			case '-':
				// and the divider is a divider
				if (date[i] != '-') 
					return date;
				s[j][k] = '\0';
				j++;
				k = 0;
			break;
		}
	}
	// load up integer values
	for (i = 0; i < DT; i++)
	{
		switch(i)
		{
			case DAY:
			case YR:
				n[i] = atoi(s[i]);
			break;
			case MON:
				for (j = 0; j < 12; j++) 
				{
					if (strcmp(months[j], s[i]) == 0) 
						break;
				}
				n[i] = j + 1;
			break;
		}
	}
	//create new date string (4 digit year/month/day)
	strcpy(newdate, "");
	for (i = YR; i >= DAY; i--)
	{
		switch(i)
		{
			case YR:
				strcat(newdate, (n[YR] >= 97? "19":"20"));
				strcat(newdate, s[YR]);
			break;
			default:
				if (n[i] < 10) strcat(newdate, "0");
				strcat(newdate, itoa(n[i],s[i],10));

		}
		if (i != DAY) strcat(newdate, "/");
	}
	//return new date string
	return newdate;
#undef DAY 
#undef MON 
#undef YR   
#undef DT
}

#ifndef LIFELINE

char *unconvertdate(char *dstr)
{
	char *p = dstr;
	char ychunk[5];
	char mchunk[5];
	int m;
	if (strlen(p) != strlen("YYYY/MM/DD")) 
	{
		strcpy(newdate,dstr);
		return newdate;
	}  
	strncpy(ychunk, p, 4);
	ychunk[4] = 0;
	p += 5;
	strncpy(mchunk, p, 2);
	m = atoi(mchunk);
	strcpy(mchunk, months[m - 1]);
	p += 3;
	strncpy(newdate, p, 2);
	newdate[2] = 0;
	strcat(newdate, "-");
	strcat(newdate, mchunk);
	strcat(newdate, "-");
	strcat(newdate, ychunk + 2);
	return newdate;
}

int isnumericdate(char *dstr)
{
	register i;
	int len;
	char delimiter = '/';
	
	len = strlen(dstr);
	if (len < 6) return FALSE;
	for (i = 0; i < len; i++)
	{
		switch(dstr[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			case ':':
			case '/':
				if (dstr[i] != delimiter) return FALSE;
				break;
			case ' ':
				delimiter = ':';
				break;
			default: 
				return FALSE;
		}
	}
	return TRUE;
}

int isalphanumericdate(char *dstr)
{
	register i;
	int len;
	char delimiter = '-';
	
	len = strlen(dstr);
	if (len < 6) return FALSE;
	for (i = 0; i < len; i++)
	{
		switch(dstr[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			case 'J': 
				if (strncmp(dstr + i, "Jan", 3) == 0) { i+=3; break; }
				if (strncmp(dstr + i, "Jun", 3) == 0) { i+=3; break; }
				if (strncmp(dstr + i, "Jul", 3) == 0) { i+=3; break; }
			case 'F':
				if (strncmp(dstr + i, "Feb", 3) == 0) { i+=3; break; }
			case 'M':
				if (strncmp(dstr + i, "Mar", 3) == 0) { i+=3; break; }
				if (strncmp(dstr + i, "May", 3) == 0) { i+=3; break; }
			case 'A':
				if (strncmp(dstr + i, "Apr", 3) == 0) { i+=3; break; }
				if (strncmp(dstr + i, "Aug", 3) == 0) { i+=3; break; }
			case 'S':
				if (strncmp(dstr + i, "Sep", 3) == 0) { i+=3; break; }
			case 'O':
				if (strncmp(dstr + i, "Oct", 3) == 0) { i+=3; break; }
			case 'N':
				if (strncmp(dstr + i, "Nov", 3) == 0) { i+=3; break; }
			case 'D':	
				if (strncmp(dstr + i, "Dec", 3) == 0) { i+=3; break; }
			case ':':
			case '-':
				// check for delimeter in wrong place
				if (dstr[i] != delimiter) return FALSE;
				break;
			case ' ':
				delimiter = ':';
				break;
			default: 
				return FALSE;
		}
	}
	return TRUE;
}

#define NUMERIC 2
int isdate(char *dstr)
{
	// check for invalid characters
	if (isnumericdate(dstr)) return NUMERIC;
	if (isalphanumericdate(dstr)) return TRUE;
	return FALSE;
}

int undatesortstr(char *dstr, 
				   time_t *y,
				   time_t *m,
				   time_t *d,
				   time_t *h,
				   time_t *mn,
				   time_t *s)
{
	char *p = dstr;
	char chunk[5] = {0};
	int i, j, len;
	char datetype;
	// for excel date formats (thanks bill)
	enum DATEPARTS {month,day,year,hour,minute} datepart;
	
	if ((datetype = isdate(dstr)) == FALSE) return FALSE;
		    
	*y = *m = *d = *h = *mn = *s = 0L;
	switch (datetype)
	{
		case NUMERIC: //two kinds - the full type and the excel kind
		if (strlen(p) == strlen("YYYY/MM/DD HH:MM:SS")) 
		{
			strncpy(chunk, p, 4);
			chunk[4] = 0;
			*y = atol(chunk);
			p += 5;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*m = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*d = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*h = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*mn = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*s = atol(chunk);
		}
		else 
		{	
			//this is for the [M]M/[D]D/YY [H]H:MM strings from excel
			//thanks bill (@%#$@!!!)
			len = strlen(dstr);
			datepart = month;
			j = 0;
			for (i = 0; i < len; i++)
			{
				switch(dstr[i])
				{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						chunk[j++] = dstr[i];
						break;
					case ' ':
					case ':':
					case '/':
						chunk[j] = 0;
						switch (datepart)
						{
							case month:	*m = atol(chunk);
								break;
							case day: *d = atol(chunk);
								break;
							case year:	*y = atol(chunk);
								if (*y > 97) *y += 1900;
								else *y += 2000;
								break;
							case hour:	*h = atol(chunk);
								break;
						}
						datepart++;
						j = 0;
						strcpy(chunk, "");
						break;
				}
			}
			chunk[j] = 0;
			*mn = atol(chunk);
			j = 0;
			strcpy(chunk, "");
		}
		break;
		default: // assuming an alphanumeric date here
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*d = atol(chunk);
			p += 3;
			strncpy(chunk, p, 3);
			chunk[3] = 0;
			*m = 0L;
			switch (toupper(chunk[0]))
			{
				case 'J': // Jan, Jun, Jul
					switch (tolower(chunk[1]))
					{
						case 'a': *m = 1; break;
						case 'u':
							switch (tolower(chunk[2]))
							{
								case 'l': *m = 7; break;
								case 'n': *m = 6; break;
							}
						break;
					}
				break;
				case 'F': // Feb
					*m = 2; break;
				case 'M': // Mar, May
					switch (tolower(chunk[2]))
					{
						case 'r': *m = 3; break;
						case 'y': *m = 5; break;
					}
				break;
				case 'A': // Apr, Aug
					switch (tolower(chunk[1]))
					{
						case 'p': *m = 4; break;
						case 'u': *m = 8; break;
					}
				break;
				case 'S': // Sep
					*m = 9; break;
				case 'O': // Oct
					*m = 10; break;
				case 'N': // Nov
					*m = 11; break;
				case 'D': // Dec
					*m = 12;
			}
			p += 4;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*y = atol(chunk);
			if (*y > 97) *y += 1900;
			else *y += 2000;
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*h = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*mn = atol(chunk);
			p += 3;
			strncpy(chunk, p, 2);
			chunk[2] = 0;
			*s = atol(chunk);
	}	
	return TRUE;
}				   

time_t datestrtotime(char *dstr)
{
	time_t r,y,m,d,h,mn,s;
	
	if (!undatesortstr(dstr, &y, &m, &d, &h, &mn, &s))
		return 0L;

	r = getdatesecs((int)d,(int)m,(int)y);
	r -= DAYSECS;
	r += (h + 7) * HOURSECS + mn * MINSECS + s; //add 8 to hours for local time correction
	return  r;
}
#undef NUMERIC

/* fill a specific field with a chunk */
void cnvtstr(enum fields i, struct userstruct *user, char *start, char *end)
{      
    size_t len;
    register j;
    char buff[DOSPATHLEN] = "";
    char num[DOSPATHLEN] = "";
    char *p;
    
    if (end == NULL) len = strlen(start);
    else len = (size_t)(end - start);

	if (len > 0L)
	{
		switch (i)
		{
			case fPAIDTO:
			case fLASTCALL:
			case fSTART: break;
			default: len = len < field[i].l? len: field[i].l;
		}
		strncpy(buff, start, len);
		buff[len] = 0;
    }
    
    switch (i)
    {
		case fBOX: //create valid 4 digit box before saving
			if (strcmp(buff,"BOX") == 0)
				sprintf(user->box, "");
			else sprintf(user->box,"%04d",(atoi(buff)));
		break;
		case fLANGUAGE:
			strcpy(user->language, buff);
		break;
		case fPIN:
			sprintf(user->pin,"%04s",buff);
		break;
		case fADMIN: //create valid 4 digit box before saving
			if (strlen(buff) == 0)
				strcpy(user->admin,"");
			else sprintf(user->admin,"%04d",(atoi(buff)));
		break;
		case fNAME:
			strcpy(user->name, buff);
		break;
		case fAD1:
			strcpy(user->ad1, buff);
		break;
		case fAD2:
			strcpy(user->ad2, buff);
		break;
		case fCITY:
			strcpy(user->city, buff);
		break;
		case fPROV:
			strcpy(user->prov, buff);
		break;
		case fPHONE:
			strcpy(user->phone, buff);
		break;
		case fPCODE:
			strcpy(user->pcode, buff);
		break;
		case fMISC:
			strcpy(user->misc, buff);
		break;
		case fTTLPAID:
			strcpy(user->ttlpaid, buff);
		break;
		case fREMIND:
			strcpy(user->remind, buff);
		break;
		case fACTIVE:
			user->active = FALSE;
			if (strcmp(isactive, buff) == 0) user->active = ACTIVE;
			else if (strcmp(isinactive, buff) == 0) user->active = INACTIVE;
			else if (strcmp(isonhold, buff) == 0) user->active = ONHOLD;
		break;	
		case fBOXTYPE:
			for (j = 0; j < btANY; j++)
				if (strcmp(btname[j], buff) == 0) break;
			user->boxtype = j;
		break;
        case fPAIDTO:
            strcpy(user->paidto, unconvertdate(buff));
            if (strlen(user->paidto) <= PAIDTOLEN)
            {
            	p = strchr(user->paidto, '-');
            	if (p - user->paidto == 1)
            	{
            		//try and fix single digit days
            		strcpy(buff, "0");
            		strcat(buff, user->paidto); 
            		strcpy(user->paidto, buff);
            	}	
            }	
            	
		break;
        case fSTART:
			user->start = datestrtotime(buff);
		break;
        case fLASTCALL:      
			user->acctime = datestrtotime(buff);
		break;
        case fCALLS:
			user->calls = atol(buff);
		break;
        case fMSGS:
        	user->nextmsg = atoi(buff);
		break;
    }
}

int readcsvstr(US *user, char *s)
{
    register i;
    char *ends;
    char *is;
    
    is = s;
    ends = strstr(is, MIDS);
    
    for (i = fBOX; ends != NULL, i <= fCSVMAX; i++)
    {
    	cnvtstr(i, user, is, ends);
    	is = ends + strlen(MIDS);
    	ends = strstr(is, MIDS);
    }
    
    if (i <= fCSVMAX) return FALSE; //error condition
    return TRUE;
}

/* fill a specific field with a chunk */
int cnvtpcstr(int i, PAY *pc, char *start, char *end)
{      
    size_t len;
    unsigned char c;
    char s1[BOXLEN + 1];
    char s2[BOXLEN + 1];
    char s3[BOXLEN + 1];
    char s4[BOXLEN + 1];
    char buff[DOSPATHLEN] = "";
    char num[DOSPATHLEN] = "";
    
    if (end == NULL) len = strlen(start);
    else len = (size_t)(end - start);

	if (len > 0L)
	{
		strncpy(buff, start, len);
		buff[len] = 0;
    }
    
    switch (i)
    {
		case fPCBOX: 
			sprintf(pc->box, "%04s", buff);
			// strcpy(pc->box, buff);
		break;
		case fPAYCODE:
        	sscanf(buff, "%4s %4s %4s %4s", s1,s2,s3,s4);
        	c = (unsigned char) s1[0];
        	if (!checknum(&c)) return FALSE;
        	sprintf(buff, "%4s%4s%4s%4s", s1,s2,s3,s4);
        	strcpy(pc->paycode, buff);
        break;
        case fPCCREATED:
			pc->created = datestrtotime(buff);
		break;
        case fPCUSED:      
			pc->used = datestrtotime(buff);
		break;
        case fPCVAL:
        	c = (unsigned char)atoi(buff);
        	pc->value = c;
		break;
    }
    return TRUE;
}

int readpcstr(PAY *pc, char *s)
{
    register i;
    char *ends;
    char *is;
    
    is = s;
    ends = strstr(is, MIDS);
    
    for (i = fPAYCODE; ends != NULL, i <= fPCUSED; i++)
    {
    	if (!cnvtpcstr(i, pc, is, ends)) return FALSE;
    	is = ends + strlen(MIDS);
    	ends = strstr(is, MIDS);
    }
    
    if (i <= fPCUSED) return FALSE; //error condition
    else return TRUE;
}

#endif

int paycsvstr(char *s, PAY pc)
{
	time_t created;
	time_t used;
	int len;

	char area[BOXLEN + 1], vend[BOXLEN + 1], numb[BOXLEN + 1], code[BOXLEN + 1];
	char cstr[DATESORTLEN];
	char ustr[DATESORTLEN];
	
	if ((len = strlen(pc.paycode)) == 0)
	{
		strcpy(s, "PAYCODE,MONTHS,CREATED,BOX,USED\n");
		return strlen(s);
	}
	else if (len != PAYCODELEN)
	{
		return -2;
	}
	if ((len = strlen(pc.box)) != BOXLEN && len != 0)
	{
		return -1;
	}		
		
	created = pc.created;
	used = pc.used;
	strcpy(cstr, datesortstr(&created));
	strcpy(ustr, (pc.used?datesortstr(&used):"UNUSED"));
	// sometimes boxes don't get the line termination character when written to the paycode file
	
	sscanf(pc.paycode, "%4s%4s%4s%4s", area, vend, numb, code);
	
	sprintf(s, "%s %s %s %s,%d,%s,%s,%s\n",
				area, vend, numb, code,
				pc.value,
				cstr,
				pc.box,
				ustr);
	return strlen(s);
}

char *removecommas(char *s)
{
	register i;
	int l = strlen(s);
	
	for (i = 0; i < l; i++)
		if (s[i] == ',') 
		{
			s[i] = SPACE;
		}
	return s;
}

char *tstr(time_t *t)
{
#ifdef LLCNVT
			return datesortstr(t);
#endif
			
#ifdef LLRESTOR
			return datesortstr(t);
#endif

#ifndef LLCNVT
#ifndef LLRESTOR
	//determine if we are doing a backup
	//if not use DD-MMM-YY format
	//otherwise use YYYY/MM/DD format
			if (kbaction == kbIDLE)
				return timestr(t);
			return datesortstr(t);
#endif			
#endif			
}

/* return a specific field string checking what userstruct variable it 
   is for */     

#ifdef LLCNVT
#ifndef LIFELINE
#define LIFELINE
#endif
#endif

#ifdef LIFELINE
char *selectfield(FLD fld, struct userstruct *u)
{      
#define FINDUSER (u->acctime == NOTFOUND)

	time_t t;
#ifndef LLCNVT	
	char availstate;
#endif

    if (u == NULL) return field[fld].name;
    
	switch (fld)
	{
		case fBOX:
			return u->box;
		case fLANGUAGE:
			if (FINDUSER) 
			{
				if (findlanguage == maxlanguage)
					return ANYLANG;
			}
			if (strlen(u->language) == 0) 
				return DEFLANG;
			return u->language;
		case fPIN:
			if (FINDUSER) return u->pin;
#ifndef LLCNVT
			if (keystate == ksVIEW && kbaction == kbIDLE) return pincover;
#endif			
			return u->pin;
		case fADMIN:
			return u->admin;
		case fNAME:
			return u->name;
		case fAD1:
			return u->ad1;
		case fAD2:
			return u->ad2;
		case fCITY:
			return u->city;
		case fPROV:
			return u->prov;
		case fPHONE:
			return u->phone;
		case fPCODE:
			return u->pcode;
		case fMISC:
			return u->misc;
		case fTTLPAID:
			return u->ttlpaid;
		case fPAIDTO:
			return u->paidto;
		case fREMIND:
			return u->remind;
		case fSTART:
			if FINDUSER return fstartstr;
			t = u->start;
			return tstr(&t);
		case fLASTCALL:      
			if FINDUSER
				return flastcallstr;
			if(u->acctime >= u->msgtime) t = u->acctime ;
			else t = u->msgtime;
			return tstr(&t);
		case fCALLS:
			if FINDUSER
				return fcallstr;
			ltoa(u->calls, callstr, 10);
			return callstr;
		case fMSGS:
			if FINDUSER
				return fnextmsgstr;
			itoa(u->nextmsg, nextmsgstr, 10);
			return nextmsgstr;
		case fACTIVE:
#ifdef LLCNVT
			switch (u->active)
			{
				case ACTIVE:   return isactive; 
				case INACTIVE: return isinactive; 
				case ONHOLD:   return isonhold;
				default: return ""; 
			}
#else
			if (DOINGBACKUP)
			{
				switch (u->active)
				{
					case ACTIVE:   return isactive; 
					case INACTIVE: return isinactive; 
					case ONHOLD:   return isonhold;
					default: return ""; 
				}
				break;
			}            
			
			if FINDUSER availstate = u->active;
			else availstate = useridx[atoi(u->box)].avail;
			
			switch(availstate)
			{
				case ACTIVE:   return isactive; 
				case INACTIVE: return isinactive; 
				case ONHOLD:   return isonhold;
				default: 
					if FINDUSER return both;
					if (availstate >= INUSE)
						return isinuse;
					return unknown;
			}
#endif								
		case fBOXTYPE:
			return btname[u->boxtype];
		case fBOXDAYS:
			return ltoa(u->boxdays, boxdays, 10);
		case fBDUPDATE:
			t = u->bdupdate;
			return tstr(&t);
		case fBDOWED:
			return ltoa(u->bdowed, bdowed, 10);
		case fBDVAL:
			return itoa(u->bdval, ubdval, 10);
#ifndef LLCNVT
	    //back up record
		case fHOUR: return backup.hour;
		case fMIN: return backup.min;
		case fLASTBUP: 
		{
			t = backup.lastbackup;
			strcpy(s, tstr(&t));
			strcat(s, (backup.partbackup? " (PART)": " (FULL)"));
			return s;
		}
		case fFREQ: return backup.frequency;
		case fBUPREMIND: return backup.remind;
		case fDEACTIVATE: return backup.deactivate;
		case fPURGE: return backup.purgedays;
		case fBUPPATH: return backup.path;
#endif		
	}
#undef FINDUSER
}

int makecsvstr(US *user, char *os)
{
    int i = fBOX;     
    
    strcpy(os, selectfield(fBOX, user));
    i = (fBOX + 1);
    for (; i < fCSVMAX; i++)
    {
        strcat(os, MIDS);
        strcat(os,removecommas(selectfield(i, user)));
    }
    strcat(os, "\n");
    return strlen(os);
}

int writecsv(int src, int targ, char *s, US *u)
{
	makecsvstr(u, s);
	_write(src, s, strlen(s));
	return _write(targ, s, strlen(s));
}                                           
// build a string of system stats the same as that of the 
// sNUM stats reporting sequence in ansr3.c
int makestatsstr(char *os) 
{
	int paycodes = 0;
	time_t now;
	char *fields = "SFIELDS,DATE,USERS,CALLS,MESSAGES,PAGES,WAIT,CHANS FULL\n";
	
	time(&now);
	sprintf(os,"%sSTATS,%s,%d,%d,%d,%d,%d,%d\n",
		fields,
		shortdatesortstr(&now),			// date stats were made
		numusers,                       // current number of users
		sumhrarray(cperh),              // calls in last 24 hours
		sumhrarray(mperh),              // messages in last 24 hours
		sumhrarray(pgcalls),             // pagecalls
		maxhrarray(chanwait),			// maximum wait time in seconds
		sumhrarray(chansfull)           // seconds channels are full
	); 
	
	return strlen(os);
}
// build a stats string and write it to two file handles
int writestatsstr(int src, int targ, char *os) {
	makestatsstr(s);
	_write(src, s, strlen(s));
	return _write(targ, s, strlen(s));
}
#undef MIDS
#endif
//end of llcsv.c



