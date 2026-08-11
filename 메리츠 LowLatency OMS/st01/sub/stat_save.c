/*------------------------------------------------------------------------
#	Module	: save process status
#	File	: stat_save.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Stat_Save (void);

/*************************************************************************
	Function		: . save process status
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Global Data		: . char *_FEP_DAT
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Stat_Save (void)
/*----------------------------------------------------------------------*/
{
	int				rt, i, tmp_ss, start_ss, end_ss;
	char			f_name[100], buf[60], tmp[20], bumun[4];
	FILE			*fp;
	struct timeval	tv;
	struct tm		*date, date1;
	struct flock	lock;

	/* check the process operating time	*/
	gettimeofday (&tv, NULL);
	date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);
	tmp_ss = (date->tm_hour * 60 * 60) +
		(date->tm_min * 60) + date->tm_sec;				/* current time	*/

	start_ss = AtoIf (PROC(D_K,P_K).start_time, 2) * 60 * 60 +
		AtoIf (PROC(D_K,P_K).start_time+2, 2) * 60;		/* start time	*/
	end_ss = AtoIf (PROC(D_K,P_K).end_time, 2) * 60 * 60 +
		AtoIf (PROC(D_K,P_K).end_time+2, 2) * 60;			/* end time	*/

	if (start_ss != end_ss)
	{
		/* out of business hours	*/
		if ((start_ss > end_ss &&
			(tmp_ss < start_ss && tmp_ss >= end_ss + 60)) ||
			(start_ss < end_ss &&
			(tmp_ss < start_ss || tmp_ss >= end_ss + 60)))
		{
			Log (PRO_WARN, "out of process execution time[%d,%s,%s,%s,%s]",
				PROC(D_K,P_K).process_no, PROC(D_K,P_K).process_id, 
				PROC(D_K,P_K).start_time, PROC(D_K,P_K).end_time, 
				PROC(D_K,P_K).process_info);

			if (PROC(D_K,P_K).process_status == 1 ||			/* run	*/
				PROC(D_K,P_K).process_status == 2)				/* stop	*/
				PROC(D_K,P_K).process_status = 4;
			else											/* TR1, TS1	*/
				PROC(D_K,P_K).process_status = 8;

			Exit_Process ();
		}
	}
	sprintf (bumun, "%s", _SubSystem_Name);
	sprintf (f_name, "%s/%s/00000000/%s_stat",
		_FEP_DAT, LtoU (bumun, 2), PROC(D_K,P_K).process_id);
	memset (buf, 0, sizeof (buf));

	fp = fopen (f_name, "r+");

	if (fp == NULL)
	{
		Log (SAM_FATAL, "Stat_Save:fopen failure[%s] {%d:%s}",
			f_name, SYS_NO, SYS_STR);
		return;
	}

	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fileno (fp), F_SETLKW, &lock);

		if (rt == -1) 
		{
			if (SYS_NO == EINTR)
				continue;

			fclose (fp);
			Log (SAM_FATAL, "Stat_Save:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return;
		}
	} while (rt == -1);

	if (PROC(D_K,P_K).type == TY_TRS1) 
	{
		sprintf (buf, "%08d%d%d%-18.18s",
			IF_SEQ(D_K,P_K), START_STAT(D_K,P_K), TCP1_NSTAT(D_K,P_K), " ");
	}
	else if (PROC(D_K,P_K).type == TY_TRS2) 
	{
		sprintf (buf, "%08d%d%d",
			IF_SEQ(D_K,P_K), START_STAT(D_K,P_K), TCP2_LINE_GUBUN(D_K,P_K));

		for (i = 0; i < 2; i ++) 
		{
			if (PROC(D_K,P_K).l.t2.l[i]) 
				sprintf (tmp, "%d%d%d", TCP2_LSTAT(D_K,P_K,i),
				TCP2_PSTAT(D_K,P_K,i), TCP2_NSTAT(D_K,P_K,i));
			else 
				sprintf (tmp, "XXX");

			strcat (buf, tmp);
		}

		sprintf (tmp, "%-12.12s", " ");
		strcat (buf, tmp);
	}
	else
		sprintf (buf, "%08d%d%-19.19s",
			IF_SEQ(D_K,P_K), START_STAT(D_K,P_K), " ");

	fseek (fp, 0L, SEEK_SET);

	rt = fwrite (buf, strlen (buf), 1, fp);

	fflush (fp);

	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
	fclose (fp);

	return;
}	/* End of Stat_Save ()	*/

/*************************************************************************
	End of Program (stat_save.c)
*************************************************************************/
