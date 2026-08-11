/*------------------------------------------------------------------------
#	Module	: write log
#	File	: log_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"
#include	"fep_interface.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
static short	log_flag_p = 1;
static short	log_flag_g = 1;
static short	log_flag_t = 1;
static short	log_flag_e = 1;
static char		save_date[10];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Log (int p_err_no, const char *p_fmt, ...);
void	Log_Proc (char *, char *);
void	Log_Emergency (char *, char *);
void	Log_Save (char *, int);
void	SLog (int p_err_no, const char *p_fmt, ...);
void	Write_SLog (char *p_msg);

/*************************************************************************
	Function		: . write log
	Parameters IN	: . p_err_no	: error code
					  . p_fmt		: log message
	Parameters OUT	: .
	Return Code		: . void
	Global Data		: . char *_FEP_LOG
					  . char _Exe_Name[]
					  . char _SubSystem_Name[]
					  . char _Process_Name[]
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Log (int p_err_no, const char *p_fmt, ...)
/*----------------------------------------------------------------------*/
{
	va_list			ap;
	int				rt, i, size, cnt, d_k, tmp_ss, start_ss, end_ss;
	char			msg[LOG_SIZE], mbuf[LOG_SIZE], buf[LOG_SIZE], sys_date[12];
	char			bumun[4], log_file_name[100], text[256], error_cd[6];
	char			error_tm[8], err_type[4], err_level[8], err_beep[8];
	struct timeval	tv;
	struct tm		*date, date1;
	struct stat		f_info;

	va_start (ap, p_fmt);
	rt = vsnprintf (mbuf, LOG_SIZE, p_fmt, ap);
	va_end (ap);

	if (rt < 0 || rt >= LOG_SIZE)
		mbuf[LOG_SIZE-1] = '\0';

	d_k = _SubSystem_Name[1] - 'a';

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	sprintf (error_cd, "%04d", p_err_no);

	if (memcmp (error_cd+2, "00", 2) != 0)
	{
		sprintf (error_tm, "%02d%02d%02d",
			date->tm_hour, date->tm_min, date->tm_sec);

		if (Mem_Shmid[d_k] && P_K >= 0)
		{
			memcpy (PROC(D_K,P_K).error_cd, error_cd, 4);
			memcpy (PROC(D_K,P_K).error_tm, error_tm, 6);
		}
	}

	strftime (sys_date, 9, "%Y%m%d", date);
	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	/* system date changed	*/
	if (memcmp (save_date, sys_date, 8) != 0)
	{
		memcpy (save_date, sys_date, 8);
		log_flag_p = 1;
		log_flag_g = 1;
		log_flag_t = 1;
		log_flag_e = 1;
	}

	memset (buf, 0, sizeof (buf));

	if (strlen (mbuf) > LOG_SIZE - LOG_HEAD_SIZE)
		sprintf (buf, "%.*s...", LOG_SIZE - LOG_HEAD_SIZE - 3, mbuf);
	else
		sprintf (buf, "%s", mbuf);

	for (i = 0; buf[i] != '\0'; i++)
	{
		if (buf[i] == '\n')
			buf[i] = '$';		/* replace LF (0x0a) with '$' (0x24)	*/
	}

	memset (err_type, 0, sizeof (err_type));

	switch (p_err_no / 100)
	{
		case	0:
			memcpy (err_type, "USR", 3);
			break;
		case	1:
			memcpy (err_type, "SYS", 3);
			break;
		case	2:
			memcpy (err_type, "PRO", 3);
			break;
		case	3:
			memcpy (err_type, "SAM", 3);
			break;
		case	4:
			memcpy (err_type, "FIF", 3);
			break;
		case	5:
			memcpy (err_type, "ORA", 3);
			break;
		case	6:
			memcpy (err_type, "TCP", 3);
			break;
		case	7:
			memcpy (err_type, "UDP", 3);
			break;
		case	8:
			memcpy (err_type, "DSH", 3);
			break;
		default:
			break;
	}

	memset (err_level, 0, sizeof (err_level));
	memset (err_beep, 0, sizeof (err_beep));

	switch (p_err_no % 100)
	{
		case	0:
			memcpy (err_level, "INFO", 4);
			break;
		case	1:
			memcpy (err_level, "FATL", 4);
			memcpy (err_beep, "\007", 4);
			break;
		case	2:
			memcpy (err_level, "DBUG", 4);
			break;
		case	3:
			memcpy (err_level, "WARN", 4);
			break;
		case	5:
			memcpy (err_level, "EROR", 4);
			memcpy (err_beep, "\007", 4);
			break;
		default:
			break;
	}

	sprintf (msg, "[%-15.15s,%02d:%02d:%02d.%06d,%s,%s]%s\n",
		_Exe_Name, date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec,
		err_type, err_level, buf);

	if (log_flag_p)
	{
		if (bumun[1] == 'W' || bumun[1] == 'X' ||
			bumun[1] == 'Y' || bumun[1] == 'Z')
			sprintf (log_file_name, "%s/%s/%s", _FEP_LOG, bumun, sys_date);
		else
			sprintf (log_file_name, "%s/%s/00000000", _FEP_LOG, bumun);

		rt = stat (log_file_name, &_F_Info);

		if (rt == -1)
		{
			mkdir (log_file_name, 0777);
			chmod (log_file_name, 0777);
		}

		log_flag_p = 0;
	}

	if (bumun[1] == 'W' || bumun[1] == 'X' ||
		bumun[1] == 'Y' || bumun[1] == 'Z')
		sprintf (log_file_name, "%s/%s/%s/%s",
			_FEP_LOG, bumun, sys_date, _Exe_Name);
	else
		sprintf (log_file_name, "%s/%s/00000000/%s",
			_FEP_LOG, bumun, _Exe_Name);

	Log_Save (log_file_name, 60 * SIZE_MB);
	Log_Proc (log_file_name, msg);

	if (error_cd[3] == '0')										/* INFO	*/
		return;

	/* emergency log	*/
	if (log_flag_t)
	{
		sprintf (log_file_name, "%s/PZ/%s", _FEP_LOG, sys_date);

		rt = stat (log_file_name, &_F_Info);

		if (rt == -1)
		{
			mkdir (log_file_name, 0777);
			chmod (log_file_name, 0777);
		}

		log_flag_t = 0;
	}

	sprintf (msg, "%s\033[3%dm[%-15.15s,%02d:%02d:%02d.%06d,%s,%s]%s\033[0m\n",
		err_beep, p_err_no % 100, _Exe_Name, date->tm_hour, date->tm_min,
		date->tm_sec, tv.tv_usec, err_type, err_level, buf);
	sprintf (log_file_name, "%s/PZ/%s/pz_emergency", _FEP_LOG, sys_date);
	Log_Emergency (log_file_name, msg);

	return;
}	/* End of Log ()	*/

/*************************************************************************
	Function		: . open log file and write message to it
	Parameters IN	: . p_file_name	: log file name
					  . p_msg		: log message
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Log_Proc (char *p_file_name, char *p_msg)
/*----------------------------------------------------------------------*/
{
	int		fd, rt;

	fd = open (p_file_name, O_RDWR|O_APPEND|O_CREAT, 0666);

	if (fd == -1)
		return;

	if (log_flag_g)
	{
		fchmod (fd, 0666);
		log_flag_g = 0;
	}

	write (fd, p_msg, strlen (p_msg));
	close (fd);

	return;
}	/* End of Log_Proc ()	*/

/*************************************************************************
	Function		: . open emergency log file and write error message to it
	Parameters IN	: . p_file_name	: log file name
					  . p_msg		: log message
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Log_Emergency (char *p_file_name, char *p_msg)
/*----------------------------------------------------------------------*/
{
	int		fd, rt;

	fd = open (p_file_name, O_RDWR|O_APPEND|O_CREAT|O_LARGEFILE, 0666);

	if (fd == -1)
		return;

	if (log_flag_e)
	{
		fchmod (fd, 0666);
		log_flag_e = 0;
	}

	write (fd, p_msg, strlen (p_msg));
	close (fd);

	return;
}	/* End of Log_Emergency ()	*/

/*************************************************************************
	Function		: . when exceed the size limit, backup to old file
	Parameters IN	: . p_file_name	: log file name to backup
					  . p_size		: file size
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Log_Save (char *p_file_name, int p_size)
/*----------------------------------------------------------------------*/
{
	int				rt;
	char			old_file_name[256];
	struct timeval	tv;
	struct tm		*date, date1;
	struct stat		f_info;

	f_info.st_size = 0;

	rt = stat (p_file_name, &f_info);

	if (rt == -1 || f_info.st_size < p_size)
		return;

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	sprintf (old_file_name, "%s.%02d%02d%02d",
		p_file_name, date->tm_hour, date->tm_min, date->tm_sec);

	rt = stat (old_file_name, &f_info);

	if (rt == 0)
		unlink (old_file_name);

	rt = link (p_file_name, old_file_name);

	if (rt == -1)
		return;

	unlink (p_file_name);

	return;
}	/* End of Log_Save ()	*/

/*************************************************************************
	Function		: . write log to SHM
	Parameters IN	: . p_err_no	: error code
					  . p_fmt		: log message
	Parameters OUT	: .
	Return Code		: . void
	Global Data		: . char *_FEP_LOG
					  . char _Exe_Name[]
					  . char _SubSystem_Name[]
					  . char _Process_Name[]
*************************************************************************/
/*----------------------------------------------------------------------*/
void	SLog (int p_err_no, const char *p_fmt, ...)
/*----------------------------------------------------------------------*/
{
	va_list			ap;
	int				rt, i, d_k;
	long			offset;
	char			msg[SHM_LOG_SIZE], mbuf[SHM_LOG_SIZE], buf[SHM_LOG_SIZE];
	char			error_cd[6], error_tm[8], sys_date[12];
	struct timeval	tv;
	struct tm		*date, date1;
	struct stat		f_info;

	d_k = _SubSystem_Name[1] - 'a';

	va_start (ap, p_fmt);
	rt = vsnprintf (mbuf, SHM_LOG_SIZE, p_fmt, ap);
	va_end (ap);

	if (rt < 0 || rt >= SHM_LOG_SIZE)
		mbuf[SHM_LOG_SIZE-1] = '\0';

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

	if (p_err_no % 10 != 0)
	{
		if (Mem_Shmid[d_k] && P_K >= 0)
		{
			sprintf (error_cd, "%04d", p_err_no);
			sprintf (PROC(D_K,P_K).error_cd, "%04d", p_err_no);
			sprintf (error_tm, "%02d%02d%02d",
				date->tm_hour, date->tm_min, date->tm_sec);
			memcpy (PROC(D_K,P_K).error_tm, error_tm, 6);
		}
	}

	strftime (sys_date, 9, "%Y%m%d", date);

	/* system date changed	*/
	if (memcmp (save_date, sys_date, 8) != 0)
	{
		memcpy (save_date, sys_date, 8);
		log_flag_p = 1;
		log_flag_g = 1;
		log_flag_t = 1;
		log_flag_e = 1;
	}

	memset (buf, 0, sizeof (buf));

	if (strlen (mbuf) > SHM_LOG_SIZE - SHM_LOG_HEAD_SIZE)
		sprintf (buf, "%.*s...", SHM_LOG_SIZE - SHM_LOG_HEAD_SIZE - 3, mbuf);
	else
		sprintf (buf, "%s", mbuf);

	for (i = 0; buf[i] != '\0'; i++)
	{
		if (buf[i] == '\n')
			buf[i] = '$';		/* replace LF (0x0a) with '$' (0x24)	*/
	}

	sprintf (msg, "%04d%04d%-10.10s%02d%02d%02d%06d%s",
		SHM_LOG_HEAD_SIZE + strlen (buf), p_err_no, _Exe_Name,
		date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec, buf);

	if (ShmLogSemId != -1)
		SEM_Lock (ShmLogSemId);

	offset = (SLOGW(D_K) % SHM_LOG_MAX) * SHM_LOG_SIZE;
	memcpy (&ShmLogPtr[offset], msg, strlen (msg));
	SLOGW(D_K) ++;

	if (ShmLogSemId != -1)
		SEM_UnLock (ShmLogSemId);

	if (ShmLogFifoFd <= 0)
	{
		Log (FIF_FATAL, "SLog:cannot open SHM log FIFO[%d][%d:%s]",
			ShmLogFifoFd, SYS_NO, SYS_STR);
	}

	rt = write (ShmLogFifoFd, "1", 1);

	if (rt < 0) 
		Log (FIF_FATAL, "SLog:cannot write SHM log FIFO[%d][%d:%s]",
			ShmLogFifoFd, SYS_NO, SYS_STR);

	return;
}	/* End of SLog ()	*/

/*************************************************************************
	Function		: . write SHM log to file
	Parameters IN	: .  p_msg	: log message
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Write_SLog (char *p_msg)
/*----------------------------------------------------------------------*/
{
	int				rt, err_no, len;
	char			msg[2048], sys_date[12];
	char			bumun[4], log_file_name[100];
	char			err_type[4], err_level[8], err_beep[8];
	struct timeval	tv;
	struct tm		*date, date1;
	SHM_LOG_HEAD	*hd = (SHM_LOG_HEAD *)p_msg;

	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	len = AtoIf (hd->Length, sizeof (hd->Length));
	err_no = AtoIf (hd->ErrCd, sizeof (hd->ErrCd));
	memset (err_type, 0, sizeof (err_type));

	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);
	strftime (sys_date, 9, "%Y%m%d", date);

	switch (err_no / 100)
	{
		case	0:
			memcpy (err_type, "USR", 3);
			break;
		case	1:
			memcpy (err_type, "SYS", 3);
			break;
		case	2:
			memcpy (err_type, "PRO", 3);
			break;
		case	3:
			memcpy (err_type, "SAM", 3);
			break;
		case	4:
			memcpy (err_type, "FIF", 3);
			break;
		case    5:
			memcpy (err_type, "ORA", 3);
			break;
		case    6:
			memcpy (err_type, "TCP", 3);
			break;
		case    7:
			memcpy (err_type, "UDP", 3);
			break;
		case    8:
			memcpy (err_type, "DSH", 3);
			break;
		default:
			break;
	}

	memset (err_level, 0, sizeof (err_level));
	memset (err_beep, 0, sizeof (err_beep));

	switch (err_no % 100)
	{
		case	0:
			memcpy (err_level, "INFO", 4);
			break;
		case	1:
			memcpy (err_level, "FATL", 4);
			memcpy (err_beep, "\007", 4);
			break;
		case	2:
			memcpy (err_level, "DBUG", 4);
			break;
		case	3:
			memcpy (err_level, "WARN", 4);
			break;
		case	5:
			memcpy (err_level, "EROR", 4);
			memcpy (err_beep, "\007", 4);
			break;
		default:
			break;
	}

	sprintf (msg, "[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\n",
		hd->LogName, hd->Time, hd->Time+2, hd->Time+4, hd->Time+6, err_type,
		err_level, len - SHM_LOG_HEAD_SIZE, &p_msg[SHM_LOG_HEAD_SIZE]);

	if (log_flag_p)
	{
		if (bumun[1] == 'W' || bumun[1] == 'X' ||
			bumun[1] == 'Y' || bumun[1] == 'Z')
			sprintf (log_file_name, "%s/%s/%s", _FEP_LOG, bumun, sys_date);
		else
			sprintf (log_file_name, "%s/%s/00000000", _FEP_LOG, bumun);

		rt = stat (log_file_name, &_F_Info);

		if (rt == -1)
		{
			mkdir (log_file_name, 0777);
			chmod (log_file_name, 0777);
		}

		log_flag_p = 0;
	}

	if (bumun[1] == 'W' || bumun[1] == 'X' ||
		bumun[1] == 'Y' || bumun[1] == 'Z')
		sprintf (log_file_name, "%s/%s/%s/%.10s",
			_FEP_LOG, bumun, sys_date, hd->LogName);
	else
		sprintf (log_file_name, "%s/%s/00000000/%.10s",
			_FEP_LOG, bumun, hd->LogName);

	Log_Save (log_file_name, 60 * SIZE_MB);
	Log_Proc (log_file_name, msg);

	if (hd->ErrCd[3] == '0')									/* INFO	*/
		return;

	/* emergency log	*/
	if (log_flag_t)
	{
		sprintf (log_file_name, "%s/PZ/%s", _FEP_LOG, sys_date);

		rt = stat (log_file_name, &_F_Info);

		if (rt == -1)
		{
			mkdir (log_file_name, 0777);
			chmod (log_file_name, 0777);
		}

		log_flag_t = 0;
	}

	sprintf (msg,
		"%s\033[3%dm[%-10.10s     ,%.2s:%.2s:%.2s.%.6s,%s,%s]%.*s\033[0m\n",
		err_beep, err_no % 100, hd->LogName, hd->Time, hd->Time+2, hd->Time+4,
		hd->Time+6, err_type, err_level, len - SHM_LOG_HEAD_SIZE,
		&p_msg[SHM_LOG_HEAD_SIZE]);
	sprintf (log_file_name, "%s/PZ/%s/pz_emergency", _FEP_LOG, sys_date);
	Log_Emergency (log_file_name, msg);

	return;
}	/* End of Write_SLog ()	*/

/*************************************************************************
	End of Program (log_proc.c)
*************************************************************************/
