#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include "context.h"

#define	SECOND1DAY	(24*60*60)

static	void holiday(MARKET *market)
{
	MDINFO	*info = market->info;
	char	line_b[256];
	char	hPath[128];
	struct	tm tm;
	char	hymd[80];
	int	ymd, year, yy, mm, dd;
	FILE	*hFile;
	time_t	clock;
	int	slen;
	int	ii;

	clock = time(0);
	localtime_r(&clock, &tm);
	year = tm.tm_year + 1900;

	sprintf(hPath, "%s/HOLIDAY", market->dirp);
	if ((hFile = fopen(hPath, "r")) == NULL)
		return;
	while ((fgets(line_b, sizeof(line_b), hFile)) == line_b)
	{
		if (line_b[0] == '#' || line_b[0] == '*' || line_b[0] == '\0')
			continue;
		if ((slen = strlen(line_b)) < 4)
			continue;
		if (line_b[slen-1] == '\n' || line_b[slen-1] == '\r')
			slen--;
		if (line_b[slen-1] == '\n' || line_b[slen-1] == '\r')
			slen--;
		line_b[slen] = '\0';

		hymd[0] = '\0';
		sscanf(line_b, "%s", hymd);
		if (strlen(hymd) != 8)
			continue;

		for (ii = 0; ii < strlen(hymd); ii++)
		{
			if (!(hymd[ii] >= '0' && hymd[ii] <= '9'))
				break;
		}
		if (ii < strlen(hymd))
			continue;
                ymd = atoi(hymd);
                yy = ymd / 10000;
                mm = ((ymd % 10000) / 100) - 1;
                dd = (ymd % 100) - 1;
		if (mm >= 12 || mm < 0 || dd < 0 || dd >= 31)
			continue;
		if (yy == info->year[0])
                	info->hday[0][mm][dd] = HOLIDAY;
		else if (yy == info->year[1])
                	info->hday[1][mm][dd] = HOLIDAY;
		else if (yy == info->year[2])
                	info->hday[2][mm][dd] = HOLIDAY;
	}
	fclose(hFile);
}

//
// mds_nextday()
// return the next date
//
uint32_t mds_nextday(uint32_t ymd)
{
	uint32_t date, jd;

	jd = mds_date2julian(ymd);
	jd++;
	date = mds_julian2date(jd);
	return(date);
}

//
// mds_prevday()
// return the previous date
//
uint32_t mds_prevday(uint32_t ymd)
{
	uint32_t date, jd;

	jd = mds_date2julian(ymd);
	jd--;
	date = mds_julian2date(jd);
	return(date);
}

//
// mds_timezone()
// Initialize time zone
//
void mds_timezone(MARKET *market)
{
	struct	tm tm, tx;
	time_t	clock1, clock2;
	int	isdst;

	// 20170810
	putenv(TZ_KST);			
	tzset();
	///////////
	clock1 = time(0);
	localtime_r(&clock1, &tm);
	putenv(market->TZ);			// couuntry of exchange
	tzset();
	localtime_r(&clock1, &tx);
	isdst = tx.tm_isdst;
	memcpy(&tx, &tm, sizeof(struct tm));
	tx.tm_isdst = isdst;
	market->isdst = isdst;
	clock2 = mktime(&tx);
	market->e2lt = clock2 - clock1;
	
	gmtime_r(&clock1, &tm);
	memcpy(&tx, &tm, sizeof(struct tm));
	tx.tm_isdst = isdst;
	clock2 = mktime(&tx);
	market->g2et = clock2 - clock1;
}


//
// mds_calendar()
// Initialize time 
//
void mds_calendar(MARKET *market)
{
	MDINFO	*info = market->info;
	struct	tm tm;
	time_t	clock;
	int	year, yy, mm, dd;
	int	ok = 0;

	if (market->whoami != I_AM_COOKER)
		return;

	clock = time(0);
	localtime_r(&clock, &tm);
	info->year[0] = tm.tm_year + 1900 - 1;	// previous year
	info->year[1] = tm.tm_year + 1900;	// current year
	info->year[2] = info->year[1] + 1;	// next year

	year = tm.tm_year - 1;			// previous year
	tm.tm_year = year;
	tm.tm_mon = 0;				// start of current year
	tm.tm_mday = 1;
	tm.tm_hour = 12;
	clock = mktime(&tm);

	for (yy= 0; yy < 3; yy++)
		for (mm = 0; mm < 12; mm++)
			for (dd = 0; dd < 31; dd++)
				info->hday[yy][mm][dd] = NODATE;
	for (;;)
	{
		localtime_r(&clock, &tm);
		yy = tm.tm_year - year;
		if (yy >= 3)
			break;
		mm = tm.tm_mon;
		dd = tm.tm_mday - 1;
		switch (tm.tm_wday)
		{
		case 0:  info->hday[yy][mm][dd] = SUNDAY;   break;
		case 6:  info->hday[yy][mm][dd] = SATURDAY; break;
		default: info->hday[yy][mm][dd] = TRADDAY;  break;
		}
		clock += SECOND1DAY;
	}
	holiday(market);		// initialize holiday

	//
	// For FIX trading day on weekend and CME HUB BUG-FIXING
	// 
	localtime_r(&clock, &tm);
	yy = 1;
	mm = tm.tm_mon;
	dd = tm.tm_mday - 1;
	for (; ok < 2;)
	{
		switch (info->hday[yy][mm][dd])
		{
		case TRADDAY:
			switch (ok)
			{
			case 0:	market->tymd = YMD(info->year[yy], (mm+1), (dd+1)); break; // current trading day
			case 1: market->nymd = YMD(info->year[yy], (mm+1), (dd+1)); break; // next trading day
			}
			ok++;
			break;
		default:
			dd++;
			if (dd >= 31)
			{
				dd = 0;
				mm++;
				if (mm >= 12)
				{
					mm = 0;
					yy++;
				}
			}
			break;
		}
	}
}

//
// mds_holiday()
// Check the specified date is holiday ?
//
int mds_holiday(MARKET *market, uint32_t locdate, int caller)
{
	MDINFO	*info = market->info;
	int	y = YEAR(locdate);
	int	m = MONTH(locdate);
	int	d = MDAY(locdate);
	uint32_t ymd;
	struct	tm tm;
	time_t	clock;
	int	ix;

	if (locdate == 0)
	{
		clock = time(0);
		localtime_r(&clock, &tm);
		y = tm.tm_year + 1900;
		m = tm.tm_mon + 1;
		d = tm.tm_mday;
	}
	
	if (y == info->year[0])
		ix = 0;
	else if (y == info->year[1])
		ix = 1;
	else if (y == info->year[2])
		ix = 2;
	else
		return(1);

	switch (info->hday[ix][m-1][d-1])
	{
	case TRADDAY:
		return(0);
	case SATURDAY:
		if (!(market->xchg->trading.wday & 0x40) || !caller)// Saturday = No trading day
			return(1);
		break;
	case SUNDAY:
		if (!(market->xchg->trading.wday & 0x01) || !caller)// Sunday = No trading day
			return(1);
		break;
	case HOLIDAY:
		if (!caller)					// no cooker ?
			return(1);
		if (market->xchg->trading.h24t)
			return(0);
		return(1);
	default:
		return(1);
	}
	// saturday, sunday, holiday
	if (!market->xchg->trading.h24t)		// no 24 hour trading
		return(1);
	// check next day
	ymd = YMD(y, m, d);
	ymd = mds_nextday(ymd);
	y = YEAR(locdate);
	m = MONTH(locdate);
	d = MDAY(locdate);
	if (info->year[ix] != y)
		return(1);

#if 0
	// saturday, sunday, holiday
	switch (info->hday[ix][m-1][d-1])
	{
	case TRADDAY:
		return(0);
	case SATURDAY:
	case SUNDAY:
	case HOLIDAY:
	default:
		return(1);
	}
#endif
	return(0);
}

//
// mds_chckday()
// Check a the specified date is trading day ?
//
int mds_chckday(MARKET *market, uint32_t xymd)
{
	MDINFO	*info = market->info;
	int	year = YEAR(xymd);
	int	mm = MONTH(xymd);
	int	dd = MDAY(xymd);
	int	yy;

	if (mm <= 0 || mm > 12 || dd <= 0 || dd > 31)
		return(NODATE);
		
	for (yy = 0; yy < 3; yy++)
	{
		if (info->year[yy] == year)
			break;
	}
	if (yy >= 3)
		return(NODATE);
	mm--;
	dd--;
	return(info->hday[yy][mm][dd]);
}

//
// mds_time()
// Get current time
//
void mds_time(MARKET *market, time_t clock, uint32_t *xymd, uint32_t *xhms, uint32_t *kymd, uint32_t *khms)
{
	time_t	tclock;
	struct	tm tm;

	if (clock == 0)
		tclock = time(0);
	else
		tclock = clock;
	if (xymd != NULL || xhms != NULL)
	{
		localtime_r(&tclock, &tm);
		if (xymd != NULL)
			*xymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (xhms != NULL)
			*xhms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
//	printf("tclock:%ld \n", tclock);
	localtime_r(&tclock, &tm);
	//printf("[%4d] %04d.%02d.%02d %02d:%02d:%02d \n", __LINE__, tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (kymd != NULL || khms != NULL)
	{
		tclock += market->e2lt;
		//printf("[%4d] %04d.%02d.%02d %02d:%02d:%02d \n", __LINE__, tm.tm_year + 1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		localtime_r(&tclock, &tm);
		if (kymd != NULL)
			*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (khms != NULL)
			*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
}

void mds_ktime(MARKET *market, time_t clock, uint32_t *kymd, uint32_t *khms)
{
	time_t	tclock;
	struct	tm tm;

	if (clock == 0)
		tclock = time(0);
	else
		tclock = clock;

	if (kymd != NULL || khms != NULL)
	{
		localtime_r(&tclock, &tm);
		if (kymd != NULL)
			*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (khms != NULL)
			*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	/*
	localtime_r(&tclock, &tm);
	if (kymd != NULL || khms != NULL)
	{
		tclock += market->e2lt;
		localtime_r(&tclock, &tm);
		if (kymd != NULL)
			*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
		if (khms != NULL)
			*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	*/
}

//
// mds_kortime()
// Get KOR date & time from local date & tgim
//
void mds_kortime(MARKET *market, uint32_t xymd, uint32_t xhms, uint32_t *kymd, uint32_t *khms)
{
	struct	tm  tm, tx;
	time_t	clock;
	char	isdst;


	putenv(TZ_KST);
	tzset();

	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(xymd) - 1900;
	tx.tm_mon  = MONTH(xymd) - 1;
	tx.tm_mday = MDAY(xymd);
	clock = mktime(&tx);

	putenv(market->TZ);
	tzset();
	localtime_r(&clock, &tm);
	isdst = tm.tm_isdst;

	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(xymd) - 1900;
	tx.tm_mon  = MONTH(xymd) - 1;
	tx.tm_mday = MDAY(xymd);
	tx.tm_hour = HOUR(xhms);
	tx.tm_min  = MINUTE(xhms);
	tx.tm_sec  = SECOND(xhms);
	tx.tm_isdst = isdst;
	clock = mktime(&tx);

	putenv(TZ_KST);
	tzset();
	localtime_r(&clock, &tm);
	*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
	*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);

	putenv(market->TZ);
	tzset();

}

void mds_kortm(MARKET *market, struct tm *xtm, struct tm *ktm)
{
	struct	tm  tm, tx;
	time_t	clock;

	memset(&tx, 0, sizeof(struct tm));
	memcpy(&tx, xtm, sizeof(struct tm));
	clock = mktime(&tx);
	clock += market->e2lt;
	localtime_r(&clock, &tm);
	memcpy(ktm, &tm, sizeof(struct tm));
	//*kymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
	//*khms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
}

//
// mds_loctime()
// Get Local date & time from korean date & time
//
void mds_loctime(MARKET *market, uint32_t kymd, uint32_t khms, uint32_t *xymd, uint32_t *xhms)
{
	struct	tm  tm, tx;
	time_t	clock;

	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(kymd) - 1900;
	tx.tm_mon  = MONTH(kymd) - 1;
	tx.tm_mday = MDAY(kymd);
	tx.tm_hour = HOUR(khms);
	tx.tm_min  = MINUTE(khms);
	tx.tm_sec  = SECOND(khms);
	tx.tm_isdst = market->isdst;
	clock = mktime(&tx);
	clock -= market->e2lt;
	localtime_r(&clock, &tm);
	*xymd = YMD((tm.tm_year + 1900), (tm.tm_mon+1), tm.tm_mday);
	*xhms = HMS(tm.tm_hour, tm.tm_min, tm.tm_sec);
}

//
// mds_pastday()
// Get past date or future date
//
uint32_t mds_pastday(MARKET *market, uint32_t base, int doff)
{
	time_t	clock;
	struct	tm tm, tx;
	int	ymd;

	if (base == 0)
		mds_time(market, 0, &base, NULL, NULL, NULL);

	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(base) - 1900;
	tx.tm_mon  = MONTH(base) - 1;
	tx.tm_mday = MDAY(base);
	tx.tm_hour = 12;

	clock = mktime(&tx);
	clock -= (doff * (24*60*60));
	localtime_r(&clock, &tm);
	ymd = YMD((tm.tm_year+1900), (tm.tm_mon+1), tm.tm_mday);
	return(ymd);
}

//
// mds_korhour()
// Get KST hour
//
int mds_korhour(MARKET *market, time_t clock)
{
	uint32_t khms;
	int	 hour;

	mds_time(market, clock, NULL, NULL, NULL, &khms);
	hour = khms / 10000;
	return(hour);
}

//
// mds_fixtime()
// Converti FIX's SendingTime to YMD, HMS
//
// Input String :  20131220-08:47:24.419 or
// 		   20131220084714419
//
int mds_fixtime(MARKET *market, const char *string, uint32_t *xymd, uint32_t *xhms, uint32_t *kymd, uint32_t *khms)
{
	int	ymd, year, month, mday, hour, min, sec, msec;
	const   char *lptr;
	char	tmpb[16];
	struct	tm tm;
	time_t	clock;

	if ((lptr = strchr(string, ':')) == NULL && strlen(string) == 17)
	{	
		// YYYYMMDDHHMMSSmmm
		memcpy(tmpb, &string[0], 4); tmpb[4] = '\0';
		year = atoi(tmpb);
		memcpy(tmpb, &string[4], 2); tmpb[2] = '\0';
		month = atoi(tmpb);
		memcpy(tmpb, &string[6], 2); tmpb[2] = '\0';
		mday = atoi(tmpb);
		memcpy(tmpb, &string[8], 2); tmpb[2] = '\0';
		hour = atoi(tmpb);
		memcpy(tmpb, &string[10], 2); tmpb[2] = '\0';
		min = atoi(tmpb);
		memcpy(tmpb, &string[12], 2); tmpb[2] = '\0';
		sec = atoi(tmpb);
		memcpy(tmpb, &string[14], 3); tmpb[3] = '\0';
		msec = atoi(tmpb);
	}
	else
	{
		// YYYYMMDD?hh:mm:ss.mmm
		ymd   = atoi(string);
		year  = YEAR(ymd);
		month = MONTH(ymd);
		mday  = MDAY(ymd);

		lptr = &string[9];
		hour = atoi(lptr); lptr += 3;
		min  = atoi(lptr); lptr += 3;
		sec  = atoi(lptr); lptr += 3;
		msec = atoi(lptr);
	}

	memset(&tm, 0, sizeof(struct tm));
	tm.tm_year = year - 1900;
	tm.tm_mon  = month - 1;
	tm.tm_mday = mday;
	tm.tm_hour = hour;
	tm.tm_min  = min;
	tm.tm_sec  = sec;
	tm.tm_isdst = market->isdst;
	clock = mktime(&tm);
	clock -= market->g2et;
	mds_time(market, clock, xymd, xhms, kymd, khms);
	return(msec);
}

//
// mds_utc2loc()
// UTC time to local time
//
uint32_t mds_utc2loc(MARKET *market, const char *tstring)
{
	int	hh, mm, ss;
	int 	hms;

	hms = atoi(tstring);
	hms /= 1000;		// truncate milli-seconds
	hh = HOUR(hms);
	mm = MINUTE(hms);
	ss = SECOND(hms);
	hms  = (hh * 60 * 60) + (mm * 60) + ss;
	hms -= market->g2et;
	if (hms < 0)
		hms += SECOND1DAY;
	hms %= SECOND1DAY;
	hh   = hms / 3600;
	mm   = (hms % 3600)/60;
	ss   = hms % 60;
	hms = HMS(hh,mm,ss);
	return((uint32_t)hms);
}

// date & time functions
#define CENTURY(d)      (int) ( (d) / 1000000L)
#define YY(d)         	(int) (((d) % 1000000L) / 10000L)
#define MM(d)       	(int) (((d) % 10000L) / 100)
#define DD(d)          	(int) ( (d) % 100)

//
// NAME	: julian4date
// DESC : Synopsis: Converts the date into a number of days since a distant but
// 	  unspecified epoch.  You can use this function to calculate differences
// 	  between dates, and forward dates.  Use days_to_date() to calculate the
// 	  reverse function.  Author: Robert G. Tantzen, translated from the Algol
// 	  original in Collected Algorithms of the CACM (algorithm 199).  Original
// 	  translation into C by Nat Howard, posted to Usenet 5 Jul 1985.
//
uint32_t mds_date2julian(uint32_t date)
{
    int year    = YY(date);
    int century = CENTURY(date);
    int month   = MM(date);
    int day     = DD(date);

    if (month > 2)
        month -= 3;
    else
    {
        month += 9;
        if (year)
            year--;
        else
        {
            year = 99;
            century--;
        }
    }
    return ((146097 * century)   / 4 +
            (1461   * year)      / 4 +
            (153    * month + 2) / 5 +
                      day   + 1721119);
}

//
// NAME : mds_julian2date()
// DESC : Synopsis: Converts a number of days since some distant but unspecified
// 	  epoch into a date.  You can use this function to calculate differences
// 	  between dates, and forward dates.  Use date_to_days() to calculate the
// 	  reverse function.  Author: Robert G. Tantzen, translated from the Algol
// 	  original in Collected Algorithms of the CACM (algorithm 199).  Original
// 	  translation into C by Nat Howard, posted to Usenet 5 Jul 1985.
//
#define MAKE_DATE(c,y,m,d)  (int) (c) * 1000000L +  \
                            (int) (y) * 10000L +    \
                            (int) (m) * 100 + (d)

uint32_t mds_julian2date(uint32_t days)
{
    	int century;
    	int year;
    	int month;
    	int day;

    	days   -= 1721119;
    	century = (4 * days - 1) / 146097;
    	days    =  4 * days - 1  - 146097 * century;
    	day     =  days / 4;

    	year    = (4 * day + 3) / 1461;
    	day     =  4 * day + 3  - 1461 * year;
    	day     = (day + 4) / 4;

    	month   = (5 * day - 3) / 153;
    	day     =  5 * day - 3   - 153 * month;
    	day     = (day + 5) / 5;

    	if (month < 10)
        	month += 3;
    	else
    	{
        	month -= 9;
        	if (year++ == 99)
        	{
            		year = 0;
            		century++;
        	}
    	}
    	return (MAKE_DATE (century, year, month, day));
}

//
// mds_day4week()
// Returns the day of the week where 0 is Sunday, 1 is Monday,
// ... 6 is Saturday.  Uses Zeller's Congurence algorithm.
//
int mds_day4week (uint32_t date)
{
    int yy = YEAR(date);
    int mm = MONTH(date);
    int dd = MDAY(date);

    if (mm > 2)
        mm -= 2;
    else
    {
        mm += 10;
        yy--;
    }
    dd = ((13 * mm - 1) / 5) + dd + (yy % 100) + ((yy % 100) / 4) + ((yy / 100) / 4) - 2 * (yy / 100) + 77;
    return (dd - 7 * (dd / 7));
}

//
// leap_year()
// Returns TRUE if the year is a leap year.  You must supply a
// 4-digit value for the year: 90 is taken to mean 90 ad.  Handles leap
// centuries correctly.
//
static unsigned int leap_year(int year)
{
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

//
// mds_day4year() 
// Returns the number of days since 31 December last year. 
// The Julian date(day of year) of 1 January is 1.
//
int mds_day4year(uint32_t date)
{
    static int days [12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int julian;

    julian = days[MONTH(date) - 1] + MDAY(date);
    if (MONTH(date) > 2 && leap_year (YEAR(date)))
        julian++;
    return (julian);
}


time_t mds_mktime(MARKET *market, uint32_t xymd, uint32_t xhms)
{
	struct tm tm;
	memset(&tm, 0x00, sizeof(tm));
	tm.tm_year = YEAR(xymd) - 1900;
	tm.tm_mon  = MONTH(xymd) - 1;
	tm.tm_mday = MDAY(xymd);
	tm.tm_hour = HOUR(xhms);
	tm.tm_min = MINUTE(xhms);
	tm.tm_sec = SECOND(xhms);
	tm.tm_isdst = market->isdst;
	return mktime(&tm);
}
 

void mds_plussecound(int nPlus, uint32_t xymd, uint32_t xhms, uint32_t *kymd, uint32_t *khms)
{
	struct	tm  tm, tx;
	time_t	clock;
	char	isdst;
	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(xymd) - 1900;
	tx.tm_mon  = MONTH(xymd) - 1;
	tx.tm_mday = MDAY(xymd);
	clock = mktime(&tx);



	localtime_r(&clock, &tm);


	memset(&tx, 0, sizeof(struct tm));
	tx.tm_year = YEAR(xymd) - 1900;
	tx.tm_mon  = MONTH(xymd) - 1;
	tx.tm_mday = MDAY(xymd);
	tx.tm_hour = HOUR(xhms);
	tx.tm_min  = MINUTE(xhms);
	tx.tm_sec  = SECOND(xhms)+ nPlus;
	clock = mktime(&tx);

	localtime_r(&clock, &tx);
	*kymd = YMD((tx.tm_year + 1900), (tx.tm_mon+1), tx.tm_mday);
	*khms = HMS(tx.tm_hour, tx.tm_min, tx.tm_sec);



}
