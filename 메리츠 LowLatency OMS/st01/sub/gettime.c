/*------------------------------------------------------------------------
#	Module	: get date and time
#	File	: gettime.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*************************************************************************
	Function		: . get time (HHMMSSmmm, mmm in millisec)
	Parameters IN	: .
	Parameters OUT	: . p_time	: time string
	Return Code		: . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Get_Time (char *p_time)
/*----------------------------------------------------------------------*/
{
	struct timeval	tv;
	struct tm		*date, date1;

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	/* HHMMSSmmm (9 bytes)	*/
	sprintf (p_time, "%02d%02d%02d%03d", 
		date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec / 1000);

	return (p_time);
}	/* End of Get_Time ()	*/

/*************************************************************************
	Function		: . get date and time (yyyymmddhhmmss)
	Parameters IN	: .
	Parameters OUT	: . p_time	: time string
	Return Code		: . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Get_DateTime (char *p_time)
/*----------------------------------------------------------------------*/
{
    struct timeval	tv;
    struct tm		*date, date1;

    gettimeofday (&tv, NULL);
    date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	/* yyyymmddhhmmss (14 bytes)	*/
	strftime (p_time, 15, "%Y%m%d%H%M%S", date);

	return (p_time);
}	/* End of Get_DateTime ()	*/

/*************************************************************************
	Function        : . get date and millitime (yyyymmddhhmmssmmm)
	Parameters IN   : .
	Parameters OUT  : . p_time  : time string
	Return Code     : . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Get_DateMilliTime (char *p_time)
/*----------------------------------------------------------------------*/
{
	struct timeval  tv;
	struct tm       *date, date1;

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	/* yyyymmddhhmmss (14 bytes)    */
	strftime (p_time, 15, "%Y%m%d%H%M%S", date);
	sprintf (&p_time[14], "%03d", tv.tv_usec / 1000);

	return (p_time);
}	/* End of Get_DateMilliTime () */

/*************************************************************************
	Function		: . get time to the unit of millisec
	Parameters IN	: .
	Parameters OUT	: . p_time	: time string
	Return Code		: . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Get_MilliTime (char *p_time)
/*----------------------------------------------------------------------*/
{
	struct timeval	tv;
	struct tm		*date, date1;

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	/* system time (in sec, 10) + HH:MM:SS-mmm (12) = 22 bytes	*/
	sprintf (p_time, "%010d%02d:%02d:%02d-%03d", tv.tv_sec,
		date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec / 1000);

	return (p_time);
}	/* End of Get_MilliTime ()	*/

/*************************************************************************
	Function		: . get time to the unit of microsec
	Parameters IN	: .
	Parameters OUT	: . p_time  : time string
	Return Code		: . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*Get_MicroTime (char *p_time)
/*----------------------------------------------------------------------*/
{
	struct timeval	tv;
	struct tm		*date, date1;

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	/* system time (in sec, 10) + HHMMSSmmmmmm (12) = 22 bytes	*/
	sprintf (p_time, "%010d%02d%02d%02d%06d", tv.tv_sec,
		date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec);

	return (p_time);
}	/* End of Get_MicroTime ()	*/

/*************************************************************************
	End of Program (gettime.c)
*************************************************************************/
