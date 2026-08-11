#define 	_GLOBAL
/*------------------------------------------------------------------------
#	Module	: check process and disk status	(pz_procchk_mp)
#	File	: pz_procchk.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	<sys/wait.h>
#include	<sys/statvfs.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<sys/timeb.h>
#include 	<math.h>
#include 	<sys/vfs.h>

#if defined __hpux
#include	<sys/syscall.h>
#include	<sys/pstat.h>
#elif defined sun || defined(__linux)
#include	<stdlib.h>
#include	<dirent.h>
#include	<limits.h>
#include	<sys/syscall.h>
#include	<sys/procfs.h>
#include	<linux/fs.h>
#elif defined _AIX
#include	<procinfo.h>
#endif

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TIMEOUT		60

#if defined __hpux
#define		BURST		((size_t)500)
#elif defined _AIX 
#define		BURST		200
#endif

typedef struct _process_list {
	pid_t					pid;
	char					*exec_name;
	struct _process_list	*next;
}	PROCESS_T;

typedef	struct Mem_Read_Data {
	char	daemon_id[20];	/* ºÎ¹® daemonÀÇ process ID					*/
	char	start_time[6];	/* ºÎ¹® ±âµ¿ ½Ã°¢ (hhmm)					*/
	char	end_time[6];	/* ºÎ¹® Á¾·á ½Ã°¢ (hhmm)					*/
	char	pname[1000][20];/* ¾÷¹« process ID							*/
	u_char	pstat[1000];	/* process status (1:run 2:stop 9:not run)	*/
	short	count;			/* ½ÇÇà ´ë»ó process °³¼ö					*/
	char	chk_flag;		/* ºÎ¹® daemon ±âµ¿ ¿©ºÎ (0:¹Ì±âµ¿ 1:±âµ¿)	*/
}	Data_Info;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Sig_No, NR;
int			Attach_Flag[SHM_MAX_SUB], Report_Flag[SHM_MAX_SUB];
float		Used[2], DiskSpace;
char		Dname[2][128];
PROCESS_T	*Head, *Tail;
Data_Info	Data[SHM_MAX_SUB];								/* a ~ z	*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void		Sig_Handler (int);
int			Process_Exist_Chk (void);
void		Shm_Data_Save (void);
void		Report_Time_Check (void);
int			Job_Time_Chk (int);
#if defined __hpux
void		Compose_List_HPUX (void);
#elif defined sun
void		Compose_List_SUN (void);
#elif defined _AIX 
void		Compose_List_AIX (void);
#elif defined __linux
void		Compose_List_LINUX (void);
#endif
void		Initialize_Link (void);
PROCESS_T	*Ordered_Insert (pid_t, char *);
PROCESS_T	*Search_Node (char *);
void		Release_Link (void);
void		Disk_Space_Check (void);
void		Make_Report (int, int);
void		File_Compact_Process (void);
void		Kill_Curses (void);

void	Make_Daemon (void);
void	Sub_SHM (void);
void	Mem_SHM (int, int);
void	Sise_SHM (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int 	cnt;

	NR = 0;

	Init_Mana (argc, argv);
	Log (USR_OK, "procchk_mp start ...");
	Make_Daemon ();
	sleep (120);
	Kill_Curses ();
	Setsigfatal ();

	DiskSpace = 90.0;
	cnt = 5;
	Sig_No = 0;
	memset (Report_Flag, 0, sizeof (Report_Flag));
	memset (Attach_Flag, 0, sizeof (Attach_Flag));

	signal (SIGCHLD, SIG_IGN);
	signal (SIGUSR1, Sig_Handler);
	signal (SIGUSR2, Sig_Handler);

	while (1)
	{	
		if (Sig_No == SIGUSR1)			/* = 16 (user defined signal 1)	*/
		{
			/* re-attach daemon SHM after super daemon re-starts	*/
			SHM_Detach ((char *)SHM_All_Daemon_Info);
			Sub_SHM ();
			Log (USR_WARN, "daemon SHM (INFO) re-attached");

			Kill_Curses ();
			Sig_No = 0;
		}

		Report_Time_Check ();

		if (cnt >= 5)
		{
			if (Dname[0][0] != '\0')
				Log (USR_DEBUG, "used: DAT[%.02f%%] LOG[%.02f%%]",
					Used[0], Used[1]);

			cnt = 0;
		}

		if (Process_Exist_Chk () == OK)
			cnt ++;
		else
			cnt = 0;

		Disk_Space_Check ();						/* check disk space	*/
		sleep (TIMEOUT);
	}
}	/* End of main ()	*/

/*************************************************************************
	Function		: . set signal handler
	Parameters IN	: . p_signo	: signal number
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Sig_Handler (int p_signo)
/*----------------------------------------------------------------------*/
{
	Sig_No = p_signo;

	switch (p_signo)
	{
		case	SIGUSR1:
			Log (SYS_OK, "signal caught [%d:SIGUSR1], pz_procchk", p_signo);
			signal (SIGUSR1, Sig_Handler);
			break;
		case	SIGUSR2:
			Log (SYS_OK, "signal caught [%d:SIGUSR2], pz_procchk", p_signo);
			Exit_Process ();
			break;
		default:
			Log (SYS_OK, "signal caught [%d:XXXXXXX], pz_procchk", p_signo);
			signal (p_signo, Sig_Handler);
			break;
	}

	return;
}	/* End of Sig_Handler ()	*/

/*----------------------------------------------------------------------*/
int		Process_Exist_Chk (void)
/*----------------------------------------------------------------------*/
{
	int			k, rt, i, not_cnt;
	char		buf[100], path[100];
	FILE    	*fp_b;
	PROCESS_T	*plp;

	rt = not_cnt = 0;

	/* read SHM and make the process list	*/
	Shm_Data_Save ();

	/* register Release_Link () to be called at program termination	*/
	atexit (Release_Link);

	/* initialize linked list	*/
	Initialize_Link ();

	/* compose process list	*/
#if defined __hpux
	Compose_List_HPUX ();
#elif defined sun
	Compose_List_SUN ();
#elif defined _AIX 
	Compose_List_AIX ();
#elif defined __linux
	Compose_List_LINUX ();
#endif

	for (i = 0; i < Process_Count; i ++)
	{
		if (Data[i].chk_flag == 0)
			continue;
		
		if (INFO(i).date_flag != 9)
		{
			rt = Job_Time_Chk (i);
			if (rt != OK)									/* ¾÷¹«Á¾·á	*/
				continue;
		}

		for (k = 0; k < Data[i].count; k ++) 
		{
#if (0)
			Log (USR_DEBUG, "process[%d,%s]", k + 1, Data[i].pname[k]);
#endif

			if ((plp = Search_Node (Data[i].pname[k])) != Tail)
			{
				continue; 
				NR = 0;
			}
			else 
			{
				not_cnt ++;

                if (Data[i].pstat[k] == 2)                      /* stop */
                    Log (PRO_WARN, "process(%d,%s) stopped",
                        not_cnt, Data[i].pname[k]);
                else                                            /* run  */
                {
                    if (NR > 0)
                    {
                        /* add */
                        rt = 0;
                        sprintf (path, "ps -ef|grep %s|egrep -v 'grep|ps|sh|ls|vi|view|tail|ls'|wc -l > /tmp/mrt1", Data[i].pname[k]);
                        system(path);
                        if ((fp_b = fopen ("/tmp/mrt1", "r")) == NULL)
                        {
                            Log (USR_ERROR, "cannot open:[/tmp/mrt1]");
                        }
                        else
                        {
                            if (fgets (buf, sizeof (buf), fp_b) != NULL)
                                rt = atoi (buf);
                        }
                        /* add */

                        if (rt == 0)
                            Log (PRO_ERROR, "process(%d,%s) not run pstat[%d]",
                                not_cnt, Data[i].pname[k], Data[i].pstat[k]);
                        NR = 0;
                    }
                    else
                        NR ++;
                }
			}
		}
	}
	
	Release_Link ();

	return (not_cnt);
}	/* End of Process_Exist_Chk ()	*/

/*----------------------------------------------------------------------*/
void	Shm_Data_Save (void)
/*----------------------------------------------------------------------*/
{
	int				rt, l, k, dk, pk, tmp_ss, start_ss, end_ss;
	struct timeval	tv;
	struct tm		*date, date1;

	k = 0;
	memset (Data, 0, sizeof (Data));

	for (dk = 0; dk < Process_Count; dk ++) 
	{
		if (INFO(dk).process_status == 0 || INFO(dk).process_no == 0)
		{
			if (Attach_Flag[dk] == ON)
			{
				SHM_Detach ((char *)SHM_Mem[dk]);
				Log (USR_OK, "%c%c SHM detached", _System_Name[0], dk + 'A');
#if 0
				// 20210929
				rt = shmdt(Shm_Futures);
				Log (USR_OK, "Shm_Futures detached return:[%d]", rt);
				rt = shmdt(Shm_Sise);
				Log (USR_OK, "Shm_Sise detached return:[%d]", rt);
				rt = shmdt(Shm_Lg);
				Log (USR_OK, "Shm_Lg detached return:[%d]", rt);
				// 20210929
#endif
				Attach_Flag[dk] = OFF;
				Kill_Curses ();
			}
			continue;
		}
		else
		{
			if (INFO(dk).check_status == 1)
			{
				INFO(dk).check_status = 0;
				SHM_Detach ((char *)SHM_Mem[dk]);
				Mem_SHM (1, dk);
				Attach_Flag[dk] = ON;
				Log (USR_OK, "%c%c SHM re-attached", _System_Name[0], dk + 'A');
				Kill_Curses ();
			}
			else if (Attach_Flag[dk] == OFF)
			{
				/* attach sub SHM (Shm_Mem) */
				Mem_SHM (1, dk);
				Log (USR_OK, "%c%c SHM attached", _System_Name[0], dk + 'A');
				/* attach sise data SHM */
				// 20210929
				Sise_SHM (); 
				Log(USR_OK, "attach Sise_SHM"); 
				// 20210929
				Attach_Flag[dk] = ON;
				Report_Flag[dk] = OFF;
				Kill_Curses ();
			}
		}

		Data[dk].chk_flag = 0;
		if (strlen (INFO(dk).process_id) == 0)
			continue;

		Data[dk].chk_flag = 1;
		sprintf (Data[dk].daemon_id, "%s", INFO(dk).process_id);
		sprintf (Data[dk].start_time, "%s", INFO(dk).start_time);
		sprintf (Data[dk].end_time, "%s", INFO(dk).end_time);

		/* Daemon Process ID Save	*/
		k = 0;
		sprintf (Data[dk].pname[k++], "%s", Data[dk].daemon_id);

		for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
		{
			gettimeofday (&tv, NULL);
			date = (struct tm *)localtime_r (&(tv.tv_sec), &date1);

			tmp_ss = (date->tm_hour * 60 * 60) + (date->tm_min * 60);
			start_ss = AtoIf (PROC(dk,pk).start_time, 2) * 60 * 60 +
				AtoIf (PROC(dk,pk).start_time+2, 2) * 60;
			end_ss = AtoIf (PROC(dk,pk).end_time, 2) * 60 * 60 +
				AtoIf (PROC(dk,pk).end_time+2, 2) * 60;

			/* out of business hours	*/
			if ((start_ss > end_ss &&
				(tmp_ss <= start_ss && tmp_ss >= end_ss)) ||
				(start_ss < end_ss &&
				(tmp_ss <= start_ss || tmp_ss >= end_ss)))
				continue;

			if (PROC(dk,pk).start_status == JOB_END)
				continue;

			if (PROC(dk,pk).process_status == 1 ||
				PROC(dk,pk).process_status == 2) 
			{
				Data[dk].pstat[k] = PROC(dk,pk).process_status;
				sprintf (Data[dk].pname[k++], "%s", PROC(dk,pk).process_id);
			}
		}

		Data[dk].count = k;		/* 0ºÎÅÍ k-1±îÁö Proc_Id (pname) Save	*/
	}

	return;
}	/* End of Shm_Data_Save ()	*/

/*----------------------------------------------------------------------*/
void	Report_Time_Check (void)
/*----------------------------------------------------------------------*/
{
	char	d_time[16];

	Get_DateTime (d_time);

	if (AtoIf (d_time+8, 4) == 2350)
	{
		Make_Report (-1, 2);		/* report number of free disk blocks*/
		sleep (1);
		Make_Report (-1, 3);				/* report SHM information	*/

		Log (USR_OK, "make report: df (or bdf), chkshm ...");

		Log (USR_OK, "unlinking data and log ...");
		File_Compact_Process ();				/* execute compact_mp	*/
		sleep (1);
	}

	return;
}	/* End of Report_Time_Check ()	*/

/*----------------------------------------------------------------------*/
int		Job_Time_Chk (int dk)
/*----------------------------------------------------------------------*/
{
	int		c_time, s_time, e_time;
	u_char	c_hour, c_min, e_hour, e_min;
	char	d_time[16];
	
	memset (d_time, 0, sizeof (d_time));
	Get_DateTime (d_time);

	c_time = AtoIf (d_time+8, 4);
	s_time = AtoIf (Data[dk].start_time, 4);
	e_time = AtoIf (Data[dk].end_time, 4);

	c_hour = AtoIf (d_time+8, 2);
	c_min = AtoIf (d_time+10, 1);
	e_hour = AtoIf (Data[dk].end_time, 2);
	e_min = AtoIf (Data[dk].end_time+2, 1);

	if (Report_Flag[dk] == OFF &&
		c_min == (e_min == 0 ? 6 : e_min) - 1 &&
		c_hour == (e_min == 0 ? ((e_hour == 0 ? 24 : e_hour) - 1) : e_hour))
	{
		Make_Report (dk, 1);					/* report file sequence	*/
		Report_Flag[dk] = ON;
	}

	/* run in two days (through midnight)	*/
	if (INFO(dk).date_flag == 2 || INFO(dk).date_flag == 4)
	{
		/* in business hours	*/
		if ((c_time > s_time && c_time < 2400) ||
			(c_time >= 0 && c_time < e_time))
			return (OK);
	}
	/* run in a day (date_flag is 1, 3 or 5)	*/
	else
	{
		if (c_time > s_time && c_time < e_time)
			return (OK);
	}

	return (NOTOK);
}	/* End of Job_Time_Chk ()	*/

#if defined __hpux
/*----------------------------------------------------------------------*/
void	Compose_List_HPUX (void)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	struct pst_status	pst[BURST];

	/* index within the context	*/
	idx = 0;
	uid = getuid ();

	/* loop until cnt == 0, will occur all have been returned */
	while ((cnt = pstat_getproc (pst, sizeof (pst[0]), BURST, idx)) > 0) 
	{
		/* got cnt (max of BURST) this time. process them	*/
		for (i = 0; i < cnt; i ++)
		{
			if (uid != pst[i].pst_uid ||
				pst[i].pst_cmd[0] != _SubSystem_Name[0] ||
				pst[i].pst_cmd[2] != '_' ||
				memcmp (pst[i].pst_cmd, "telnet", 6) == 0 ||
				memcmp (pst[i].pst_cmd, "fgrep", 5) == 0 ||
				memcmp (pst[i].pst_cmd, "egrep", 5) == 0 ||
				memcmp (pst[i].pst_cmd, "tail", 4) == 0 ||
				memcmp (pst[i].pst_cmd, "grep", 4) == 0 ||
				memcmp (pst[i].pst_cmd, "view", 4) == 0 ||
				memcmp (pst[i].pst_cmd, "ftp", 3) == 0 ||
				memcmp (pst[i].pst_cmd, "ksh", 3) == 0 ||
				memcmp (pst[i].pst_cmd, "vi", 2) == 0 ||
				memcmp (pst[i].pst_cmd, "sh", 2) == 0 ||
				memcmp (pst[i].pst_cmd, "ps", 2) == 0 ||
				memcmp (pst[i].pst_cmd, "ls", 2) == 0)
				continue;

			Ordered_Insert (pst[i].pst_pid, pst[i].pst_cmd);
		}
	
		/*
		go back and do it again, using the next index after the current 'burst'
		*/
		idx = pst[cnt-1].pst_idx + 1;
	}
	
	if (cnt == -1) 
		Log (SYS_ERROR, "pstat_getproc {%d:%s}", SYS_NO, SYS_STR);

	return;
}	/* End of Compose_List_HPUX ()	*/
#endif

#if defined sun
/*----------------------------------------------------------------------*/
void	Compose_List_SUN (void)
/*----------------------------------------------------------------------*/
{
	int			uid, fd;
	char			*directory="/proc/";
	char			proc_path[PATH_MAX];
	DIR				*dp;
	prpsinfo_t		pinfo;
	struct dirent	*dirp;
	struct stat		statbuf;

	uid = getuid ();

	if (lstat (directory, &statbuf) < 0)
	{
		Log (SYS_ERROR, "Compose_List_SUN:lstat {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (S_ISDIR (statbuf.st_mode) == 0)
	{
		Log (SYS_ERROR, "Compose_List_SUN:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if ((dp = opendir (directory)) == NULL)
	{
		Log (SYS_ERROR, "Compose_List_SUN:opendir {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	while ((dirp = readdir (dp)) != NULL)
	{
		if (dirp->d_name[0] != '.')
		{
			sprintf (proc_path, "/proc/%s", dirp->d_name);

			if ((fd = open (proc_path, O_RDONLY)) != -1)
			{
				if (ioctl (fd, PIOCPSINFO, &pinfo) != -1)
				{
					if (uid == pinfo.pr_uid &&
						pinfo.pr_psargs[0] == _SubSystem_Name[0] &&
						pinfo.pr_psargs[2] == '_' &&
						memcmp (pinfo.pr_psargs, "telnet", 6) != 0 &&
						memcmp (pinfo.pr_psargs, "fgrep", 5) != 0 &&
						memcmp (pinfo.pr_psargs, "egrep", 5) != 0 &&
						memcmp (pinfo.pr_psargs, "tail", 4) != 0 &&
						memcmp (pinfo.pr_psargs, "grep", 4) != 0 &&
						memcmp (pinfo.pr_psargs, "view", 4) != 0 &&
						memcmp (pinfo.pr_psargs, "ftp", 3) != 0 &&
						memcmp (pinfo.pr_psargs, "ksh", 3) != 0 &&
						memcmp (pinfo.pr_psargs, "vi", 2) != 0 &&
						memcmp (pinfo.pr_psargs, "sh", 2) != 0 &&
						memcmp (pinfo.pr_psargs, "ps", 2) != 0 &&
						memcmp (pinfo.pr_psargs, "ls", 2) != 0)
					{
						Ordered_Insert (pinfo.pr_pid, pinfo.pr_psargs);
					}
				}

				close (fd);
			}
		}
	}

	closedir (dp);

	return;
}	/* End of Compose_List_SUN ()	*/
#endif

#if defined __linux
/*----------------------------------------------------------------------*/
void	Compose_List_LINUX (void)
/*----------------------------------------------------------------------*/
{
	int				c3 = '\t', lenth;
	int				uid, fd, read_cnt;
	pid_t			r_uid;
	char			*directory="/proc/";
	char			proc_path[PATH_MAX];
	char			*sp1, *sp2;
	char			buf[1024], r_pname[100];
	FILE			*fp; 
	DIR				*dp;
	struct dirent	*dirp;
	struct stat		statbuf;

	memset (buf, 0, sizeof(buf));
	memset (r_pname, 0, sizeof(r_pname));
	uid = getuid ();

	if (lstat (directory, &statbuf) < 0)
	{
		Log (SYS_ERROR, "Compose_List_LINUX:lstat {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (S_ISDIR (statbuf.st_mode) == 0)
	{
		Log (SYS_ERROR, "Compose_List_LINUX:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if ((dp = opendir (directory)) == NULL)
	{
		Log (SYS_ERROR, "Compose_List_LINUX:opendir {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	while ((dirp = readdir (dp)) != NULL)
	{
		if (!strcmp (dirp->d_name, ".") || !strcmp (dirp->d_name, "..")
			|| dirp->d_type != 4 || dirp->d_name[0] > 57)
				continue;
		else
		{
			sprintf (proc_path, "/proc/%s/status", dirp->d_name);
			if ((fp = fopen (proc_path, "rt")) == 0)
			{
/*				Log (SYS_WARN, "Compose_List_LINUX:fopen Error [%s] {%d:%s}",
												dirp->d_name, SYS_NO, SYS_STR);
*/
				return;
			}

			read_cnt = 0;
			while ((fgets (buf, sizeof (buf), fp)) != NULL)
			{
				/* ****************** */
				/* Process Name Serch */
				/* ****************** */
				if (strstr (buf, "Name:"))
		        {
		            sp1 = strchr (buf, c3);
		            if (sp1 == NULL)
					{
		                Log (USR_ERROR, "%s Name1 is Null buf[%s]", proc_path, buf);
						fclose (fp);    //SPARROW
						return;
					}

					sprintf (r_pname, "%s", sp1+1);
					read_cnt++;
				}
						
				/* ****************** */
				/* Process Uid  Serch */
				/* ****************** */
				if (strstr (buf, "Uid:"))
				{
					sp1 = strchr (buf, c3);

		            if (sp1 == NULL)
					{
		                Log (USR_ERROR, "%s Uid1 is Null buf[%s]\n", proc_path, buf);
						fclose (fp);    //SPARROW
						return;
					}
		            else
					{
						sp2 = strchr (sp1+1, c3);

						if (sp2 == NULL)
						{
		                	Log (USR_ERROR, "%s Uid2 is Null sp1+1[%s]\n", proc_path, sp1+1);
							fclose (fp);    //SPARROW
							return;
						}

						lenth = sp2 - sp1 - 1;
						r_uid = AtoIf (sp1+1, lenth); 
						read_cnt++;
					}
		        }

				if (read_cnt > 1)
					break;
			}

			if (uid == r_uid &&
				r_pname[0] == _SubSystem_Name[0] &&
				r_pname[2] == '_' &&
				memcmp (r_pname, "telnet", 6) != 0 &&
				memcmp (r_pname, "fgrep", 5) != 0 &&
				memcmp (r_pname, "egrep", 5) != 0 &&
				memcmp (r_pname, "tail", 4) != 0 &&
				memcmp (r_pname, "grep", 4) != 0 &&
				memcmp (r_pname, "view", 4) != 0 &&
				memcmp (r_pname, "ftp", 3) != 0 &&
				memcmp (r_pname, "ksh", 3) != 0 &&
				memcmp (r_pname, "vi", 2) != 0 &&
				memcmp (r_pname, "sh", 2) != 0 &&
				memcmp (r_pname, "ps", 2) != 0 &&
				memcmp (r_pname, "ls", 2) != 0)
			{
				Ordered_Insert (r_uid, r_pname);
			}

			fclose (fp);
		}
	}

	closedir (dp);

	return;

}	/* End of Compose_List_LINUX ()	*/
#endif

#if defined _AIX 
/*----------------------------------------------------------------------*/
void	Compose_List_AIX (void)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	pid_t				pid, next_pid;
	struct procsinfo	pi[BURST];

	/* index within the context	*/
	idx = 0;
	uid = getuid ();

	memset (pi, 0, sizeof (pi));
	next_pid = 0;

	while (1)
	{
		pid = next_pid;

		cnt = getprocs (pi, sizeof (pi[0]), NULL, 0, &pid, BURST);
		if (cnt <= 0)
			break;

		for (i = 0; i < cnt; i ++)
	    {
			if (uid != pi[i].pi_uid ||
				pi[i].pi_comm[0] != _SubSystem_Name[0] ||
				pi[i].pi_comm[2] != '_' ||
				memcmp (pi[i].pi_comm, "telnet", 6) == 0 ||
				memcmp (pi[i].pi_comm, "egrep", 5) == 0 ||
				memcmp (pi[i].pi_comm, "fgrep", 5) == 0 ||
				memcmp (pi[i].pi_comm, "grep", 4) == 0 ||
				memcmp (pi[i].pi_comm, "more", 4) == 0 ||
				memcmp (pi[i].pi_comm, "tail", 4) == 0 ||
				memcmp (pi[i].pi_comm, "view", 4) == 0 ||
				memcmp (pi[i].pi_comm, "ftp", 3) == 0 ||
				memcmp (pi[i].pi_comm, "ksh", 3) == 0 ||
				memcmp (pi[i].pi_comm, "man", 3) == 0 ||
				memcmp (pi[i].pi_comm, "ls", 2) == 0 ||
				memcmp (pi[i].pi_comm, "ps", 2) == 0 ||
				memcmp (pi[i].pi_comm, "sh", 2) == 0 ||
				memcmp (pi[i].pi_comm, "vi", 2) == 0)
				continue;

			Ordered_Insert (pi[i].pi_pid, pi[i].pi_comm);
	    }

		next_pid = pid;
	}

	if (cnt == -1)
		Log (SYS_ERROR, "getprocs {%d:%s}", SYS_NO, SYS_STR);

	return;
}	/* End of Compose_List_AIX ()	*/
#endif

/*----------------------------------------------------------------------*/
void	Initialize_Link (void)
/*----------------------------------------------------------------------*/
{
	Head = (PROCESS_T *)malloc (sizeof (PROCESS_T));
	if (Head == NULL)
	{
		Log (SYS_ERROR, "memory allocation failed (Initialize_Link)");
	}
	else
	{
		Tail = (PROCESS_T *)malloc (sizeof (PROCESS_T));
		Head->next = Tail;

		if (Tail == NULL)
		{
			Log (SYS_ERROR, "memory allocation failed (Initialize_Link)");
			free(Head);  // SPARROW
		}
		else
		{
			Tail->next = Tail;
		}
	}
}

/*----------------------------------------------------------------------*/
PROCESS_T	*Ordered_Insert (pid_t pid, char *exec_name)
/*----------------------------------------------------------------------*/
{
	PROCESS_T	*sptr, *prev_ptr, *aptr;

	prev_ptr = Head;
	sptr = prev_ptr->next;

	while (sptr != Tail && strcmp (sptr->exec_name, exec_name) <= 0)
	{
		prev_ptr = prev_ptr->next;
		sptr = prev_ptr->next;
	}
	
	aptr = (PROCESS_T*)malloc (sizeof (PROCESS_T));
	if (aptr == NULL) 
	{
		Log (SYS_ERROR, "memory allocation failed (PROCESS_T)");
		free(aptr);  // SPARROW
		return (NULL);
	}
	
	aptr->exec_name = (char *)malloc (strlen (exec_name) + 1);
	if (aptr->exec_name == NULL) 
	{
		Log (SYS_ERROR, "memory allocation failed (exec_name)");
		free(aptr);  // SPARROW
		return (NULL);
	}
	strcpy (aptr->exec_name, exec_name);
	aptr->pid = pid;
	
	prev_ptr->next = aptr;
	aptr->next = sptr;

	return (aptr);
}	/* End of Ordered_Insert ()	*/

/*----------------------------------------------------------------------*/
PROCESS_T	*Search_Node (char *exec_name)
/*----------------------------------------------------------------------*/
{
	PROCESS_T	*sptr;
	
	sptr = Head->next;

	while (sptr != Tail) 
	{
		if (strstr (sptr->exec_name, exec_name))
			return (sptr);

		sptr = sptr->next;
	}

	return (sptr);
}	/* End of Search_Node ()	*/

/*----------------------------------------------------------------------*/
void	Release_Link (void)
/*----------------------------------------------------------------------*/
{
	PROCESS_T	*rmp, *seekp;
	
	seekp = Head->next;

	while (seekp != Tail)
	{
		rmp = seekp;
		seekp = seekp->next;
		free (rmp->exec_name);
		free (rmp);
	}
	Head->next = Tail;
}	/* End of Release_Link ()	*/

/*----------------------------------------------------------------------*/
void	Disk_Space_Check (void)
/*----------------------------------------------------------------------*/
{
	int				i, b;
	float			rt;
/*
#if defined(__hpux) || defined(sun)
	struct statvfs	mbuf;
#elif defined(_AIX) || defined(__linux)
	struct statfs	mbuf;
#endif
*/
#if defined(__hpux) || defined(sun)
	struct statvfs mbuf;
#elif defined(_AIX)
	struct statfs mbuf;
#elif defined(__linux)
	struct statvfs mbuf;  // ¿¿¿¿¿ statvfs()¿ ¿¿¿ ¿ ¿¿¿¿¿ ¿
#endif

	memset (Dname, 0, sizeof (Dname));

	strcpy (Dname[0], _FEP_DAT);
	strcpy (Dname[1], _FEP_LOG);

	for (i = 0; i < 2; i ++)
	{
/*
#if defined(__hpux) || defined(sun) || defined(__linux)
		if (statvfs (Dname[i], &mbuf) == 0)
#elif defined _AIX
		if (statfs (Dname[i], &mbuf) == 0)
#endif
*/

#if defined(__hpux) || defined(sun) || defined(__linux)
		if (statvfs(Dname[i], &mbuf) == 0)
#elif defined(_AIX)
		if (statfs(Dname[i], &mbuf) == 0)
#endif

		{
			rt = (float)(mbuf.f_blocks - mbuf.f_bfree);
			//Used[i] = (rt / (float)mbuf.f_blocks) * 100.0;   SPARROW
			Used[i] = (float)((rt / (float)mbuf.f_blocks) * 100.0);

			if (Used[i] > DiskSpace) 
			{
#if defined(__hpux) || defined(sun) || defined(__linux)
				b = (int)(mbuf.f_frsize / 1024);
#elif defined _AIX
				b = (int)(mbuf.f_bsize / 1024);
#endif

				Log (SYS_FATAL,
		"CHECK file system[%s] total[%ld] used[%ld] avail[%ld] %%used[%.02f%%]",
					Dname[i], mbuf.f_blocks * b,
					(mbuf.f_blocks - mbuf.f_bfree) * b,
					mbuf.f_bavail * b, Used[i]);
			}
		}
	}

	return;
}	/* End of Disk_Space_Check ()	*/

/*************************************************************************
	Function		: . execute process and report to file
	Parameters IN	: . dk	: daemon key
					  . flag: 1:chkcnt_mp 2:df[bdf] 3:chkshm_mp
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Make_Report (int dk, int flag)
/*----------------------------------------------------------------------*/
{
	int 	rt;
	char	path[256], cmd[256], d_time[16];

	sprintf (path, "%s", _FEP_BIN);
	rt = chdir (path);
	if (rt != 0)
	{
		Log (SYS_FATAL, "cannot change directory[%s] {%d:%s}",
			path, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	if (flag == 1)
	{
		Log (USR_OK, "chkcnt_mp:report file sequence");
		sprintf (cmd, "%cx_chkcnt_mp %c%c %c%c >> %s/../SEQ/TOTAL/totalseq.%s",
			_Exe_Name[0], _Exe_Name[0], dk + 'a', _Exe_Name[0], dk + 'a',
			_FEP_LOG, INFO(dk).date);
	}
	else if (flag == 2)
	{
		Get_DateTime (d_time);
#if defined __hpux
		Log (USR_OK, "bdf:report number of free disk blocks");
		sprintf (cmd, "bdf > %s/../SEQ/DISC/disc.%.8s", _FEP_LOG, d_time);
#else
		Log (USR_OK, "df -k:report number of free disk blocks");
		sprintf (cmd, "df -k > %s/../SEQ/DISC/disc.%.8s", _FEP_LOG, d_time);
#endif
	}
	else if (flag == 3)
	{
		Log (USR_OK, "chkshm_mp:report the shared memory information");
		Get_DateTime (d_time);
		sprintf (cmd, "%cx_chkshm_mp >> %s/../SEQ/DISC/disc.%.8s",
			_Exe_Name[0], _FEP_LOG, d_time);
	}

	rt = system (cmd);

#if defined __hpux || sun || _AIX
	if (rt < 0)
	{
		Log (SYS_FATAL, "system call failure[%s] {%d:%s} rt[%d]",
			cmd, SYS_NO, SYS_STR, rt);
		Exit_Process ();
	}
#endif

	return;
}	/* End of Make_Report ()	*/

/*************************************************************************
	Function		: . execute compact_mp
						- unlink the expired data, log and FIFO
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	File_Compact_Process (void)
/*----------------------------------------------------------------------*/
{
	int 	process_no, rt, status, fd;
	char	path[256], process_id[20], comp_day[4], full_path[300];

	memset (path,		0, sizeof(path));
	memset (full_path,	0, sizeof(full_path));

	sprintf (path, "%s", _FEP_BIN);

	rt = chdir (path);
	if (rt != 0)
	{
		Log (SYS_FATAL, "cannot change directory[%s] {%d:%s}",
			path, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	process_no = fork ();
	switch (process_no)
	{ 
		case	NOTOK:
			Log (SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
			break;
		case	OK:
			sprintf (process_id, "%.2s_compact_mp", _Exe_Name);
			//2025EDIT
			//sprintf (path, "%s/%s", path, process_id);
			sprintf (full_path, "%s/%s", path, process_id);
            if (strlen(full_path) < sizeof(path))
            {
                memcpy  (path, full_path, strlen(full_path));
                //path[strlen(full_path)] = '\n';
            }
            else
                memcpy  (path, full_path, sizeof(path));
			//2025EDIT

			sprintf (comp_day, "%d", INFO(D_K).compact_days);
			Log (SYS_OK, "execute [%s %s z]", process_id, comp_day);

			for (fd = 0; fd < NOFILE; fd ++)
				close (fd);

			rt = execl (path, process_id, comp_day, "z", (char *)NULL);
			Log (SYS_ERROR, "execl(%s,%s,%s,z) failure[%s] {%d:%s}",
				path, process_id, comp_day,
				"unlink the expired data and log files", SYS_NO, SYS_STR);
			exit (FAIL);
		default:
			rt = wait (&status);
	}

	return;
}	/* End of File_Compact_Process ()	*/

/*************************************************************************
	Function		: . kill (USR1) curses process
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Kill_Curses (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	pid_t	pid;
	char	pname[12];

	while (1)
	{
		sprintf (pname, "%cy_main_cm", _Exe_Name[0]);

		pid = Check_Proc (pname);

		if (pid > 0)
		{
			rt = kill (pid, SIGUSR1);

			if (rt == -1)
				Log (SYS_ERROR, "kill (USR1) failure:%s(%d) {%d:%s}",
					pname, pid, SYS_NO, SYS_STR);
			else
				Log (SYS_OK, "kill (USR1) %s(%d)", pname, pid);

			sleep (1);
		}
		else if (pid < 0)
		{
			Log (SYS_ERROR, "Kill_Curses pid ERROR %s(%d)", pname, pid);
			break;
		}
		else
		{
			Log (SYS_OK, "Kill_Curses Ok Check %s(%d)", pname, pid);
			break;
		}
	}

	return;
}	/* End of Kill_Curses ()	*/

/*************************************************************************
	End of Program (pz_procchk.c)
*************************************************************************/
