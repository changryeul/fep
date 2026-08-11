#define 	_GLOBAL
/*------------------------------------------------------------------------
#	Module	: super daemon
#	File	: pz_fepp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"daemon.h"
#include	<sys/wait.h>

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int				Sig_No;					/* signal number				*/
char			Bumun[2];				/* sub system name (lowercase)	*/
struct pollfd	Poll[1];				/* poll file descriptor			*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void    Check_Argument (int argc, char *argv[]);
void	Main_Process (void);
void	Check_SHM (void);
void	Creat_SHM_Process (void);
void	Check_Sub_Daemons (void);
int		Daemon_SHM_Process (void);
void	Daemon_Process (void);
void	File_Compact_Process (void);
void    User_Signal (void);
void    User_Sleep (int);
void    Start_Process (void);
void    Stop_Process (void);
void 	Sig_Handler (int);
void	Proc_Check_Process (void);
void	Restart_Process (void);

/*************************************************************************
	Function		: . main
	Parameters IN	: . argc	: number of arguments (1)
					  . argv[0]	: execution name
	Parameters OUT	: .
	Return Code		: . 0 (OK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	_System_Name[0] = argv[0][0];
	LtoU (_System_Name, 1);
	sprintf (_Exe_Name, "%s", argv[0]);
	sprintf (_SubSystem_Name, "%-2.2s", argv[0]);
	sprintf (_Process_Name, "%s", argv[0]);
	D_K = _SubSystem_Name[1] - 'a';

	/* check environment values	*/
	Check_Environment ();

	/* check arguments	*/
	Check_Argument (argc, argv);

	/* check if this module runs or not	*/
	Check_Exist ();

	/* make it a daemon - create a new process and set process group ID	*/
	Make_Daemon ();

	Log (USR_OK, "super daemon started[%d]", getpid ());

	/* set fatal signal handlers	*/
	Setsigfatal ();

	/* main routine	*/
	Main_Process ();

	exit (OK);
} 	/* End of main ()	*/

/*************************************************************************
	Function		: . check arguments
	Parameters IN	: . argc	: number of arguments (1)
					  . argv[0]	: execution name
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_Argument (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	/* check the number of arguments	*/
	if (argc != 1)
	{
		Log (USR_FATAL, "invalid argument number[%d]", argc);
		exit (FAIL);
	}

	/* check sub name	*/
	if (_SubSystem_Name[1] != 'z')
	{
		Log (USR_FATAL, "invalid sub name[%s]", _SubSystem_Name);
		exit (FAIL);
	}

	return;
}	/* End of Check_Argument ()	*/

/*************************************************************************
	Function		: . main routine
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Main_Process (void)
/*----------------------------------------------------------------------*/
{
	int 	rt;
	pid_t	pid;
	char	fifo_name[100], bumun[4], pname[20];

	sigrelse (SIGTERM);
	signal (SIGCHLD, SIG_IGN);

	signal (SIGTERM, Sig_Handler);

	signal (SIGUSR2, Sig_Handler);
	signal (SIGUSR1, Sig_Handler);

	/* execute memory_mp	*/
	while (1)
	{
		rt = Daemon_SHM_Process ();
		if (rt == FAIL)
		{
			Log (SAM_FATAL, "ini files have problem !!! main _mp line[%d]", __LINE__);
			User_Sleep (30);
			continue;
		}
		else
		{
			User_Sleep (1);
			Log (USR_OK, "SHM created and loaded");
			break;
		}
	}

	/* attach daemon SHM (INFO)	*/
	Sub_SHM ();

	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	sprintf (fifo_name,
		"%s/%s/%s", _FEP_FIFO, bumun, INFO(D_K).start_FIFO_name);
	SFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);
	if (SFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "cannot open start FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	sprintf (fifo_name, "%s/%s/%s",
		_FEP_FIFO, bumun, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);
	if (DFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	/* check and creat unused sub SHM	*/
	Check_SHM ();

	/* check running sub daemons and update daemon SHM (INFO)	*/
	Log (USR_OK, "checking sub daemons ...");
	Check_Sub_Daemons ();
	sleep (1);

	/* execute compact_mp	*/
	Log (USR_OK, "unlinking data, log and FIFO ...");
	File_Compact_Process ();
	sleep (1);

	/* execute procchk_mp	*/
	sprintf (pname, "%cz_procchk_mp", _Exe_Name[0]);

	Log (USR_OK, "Main_Prc Check_Proc call");
	pid = Check_Proc (pname);

	// Added 20210113
	if (pid <= 0)
	{
		Log (USR_OK, "executing procchk_mp ...");
		Proc_Check_Process ();
		sleep (1);
	}
	else
	{
		rt = kill (pid, SIGUSR1);
		if (rt == -1)
			Log (SYS_ERROR, "kill (USR1) failure[%d,%cz_procchk_mp] {%d:%s}",
				pid, _Exe_Name[0], SYS_NO, SYS_STR);
		else
			Log (SYS_OK, "kill (USR1) %cz_procchk_mp for INFO SHM re-attach pid[%d]",
				_Exe_Name[0], pid);
	}

	Log (USR_WARN, "start emergency logging");

	/* execute sub daemons	*/
	Log (USR_OK, "executing sub daemons ...");
	Daemon_Process ();

	return;
}	/* End of Main_Process ()	*/

/*************************************************************************
	Function		: . check and creat unused sub SHM
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_SHM (void)
/*----------------------------------------------------------------------*/
{
	int		i, p_id;
	char	key[12];
	key_t	base_key, shm_key;

	base_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		base_key += 0x01000000L;

	/* Process_Count = SHM_MAX_SUB, (maximum No of sub daemon) */
	for (i = 0; i < Process_Count; i++)
	{
		p_id = -1;
		sprintf (key, "0x00%02d0000", i + 1);
		errno = 0;
		shm_key = base_key + strtol (key, NULL, 16);

		if (INFO(i).process_id[0] != 0)
		{
			p_id = SHM_Creat (shm_key, 0);
			if (p_id == -1)
			{
				memset (Bumun, 0, sizeof (Bumun));
				Bumun[0] = i + 'a';
				Creat_SHM_Process ();
				INFO(i).system_status = 0;
				sleep (1);
				Log (USR_OK, "Check_SHM:%c%c SHM created and loaded",
					_System_Name[0], i + 'A');
			}
		}
	}

	return;
}	/* End of Check_SHM ()	*/

/*************************************************************************
	Function		: . execute memory_mp
						- create and load shared memory
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Creat_SHM_Process (void)
/*----------------------------------------------------------------------*/
{
	int		process_no, rt, status, fd;
	char	path[256], process_id[20];

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
			exit (FAIL);
		case	OK:
			sprintf (process_id, "%cz_memory_mp", _Exe_Name[0]);
			sprintf (path, "%s/%s", path, process_id);

			for (fd = 0; fd < NOFILE; fd ++)
				close (fd);

			execl (path, process_id, Bumun, (char *)NULL);
			Log (SYS_ERROR, "execl(%s,%s,%s) failure[%s] {%d:%s}",
				path, process_id, Bumun, "create and load shared memory",
				SYS_NO, SYS_STR);
			exit (FAIL);
		default:
			wait (&status);
	}

	return;
}	/* End of Creat_SHM_Process ()	*/

/*************************************************************************
	Function		: . check running sub daemons and update daemon SHM (INFO)
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_Sub_Daemons (void)
/*----------------------------------------------------------------------*/
{
	int 	dk, rt, FIFO_fd;
	pid_t	pid;
	char	fifo_name[100], proc_id[32];

	for (dk = 0; dk < Process_Count; dk ++) 
	{
		if (dk + 'a' == 'x' || dk + 'a' == 'y' || dk + 'a' == 'z')
			continue;

		sprintf (proc_id, "%c%c_daemon_mp", _Exe_Name[0], dk + 'a');

		Log (USR_OK, "Check_Sub_Daemons Check_Proc call");
		pid = Check_Proc (proc_id);
		if (pid > 0)
		{
			/* attach sub SHM (Shm_Mem)	*/
			Mem_SHM (1, dk);

			INFO(dk).process_no = DAEMON(dk).process_no;
			memcpy (INFO(dk).process_id, DAEMON(dk).process_id,
				sizeof (DAEMON(dk).process_id));
			memcpy (INFO(dk).start_time, DAEMON(dk).start_time,
				sizeof (DAEMON(dk).start_time));
			memcpy (INFO(dk).end_time, DAEMON(dk).end_time,
				sizeof (DAEMON(dk).end_time));
			INFO(dk).system_status = DAEMON(dk).system_status;
			INFO(dk).process_status = DAEMON(dk).process_status;
			memcpy (INFO(dk).start_FIFO_name, DAEMON(dk).start_FIFO_name,
				sizeof (DAEMON(dk).start_FIFO_name));
			memcpy (INFO(dk).exit_FIFO_name, DAEMON(dk).exit_FIFO_name,
				sizeof (DAEMON(dk).exit_FIFO_name));
			memcpy (INFO(dk).daemon_FIFO_name, DAEMON(dk).daemon_FIFO_name,
				sizeof (DAEMON(dk).daemon_FIFO_name));
			INFO(dk).process_count = DAEMON(dk).process_count;
			INFO(dk).file_count = DAEMON(dk).file_count;
			INFO(dk).dshm_count = DAEMON(dk).dshm_count;
#if defined ISAM_INCL
			INFO(dk).cisam_count = DAEMON(dk).cisam_count;
#endif
			INFO(dk).tcp1_count = DAEMON(dk).tcp1_count;
			INFO(dk).tcp2_count = DAEMON(dk).tcp2_count;
			INFO(dk).udpip_count = DAEMON(dk).udpip_count;
/* 202201
			INFO(dk).sisetr_count = DAEMON(dk).sisetr_count;
			INFO(dk).accno_count = DAEMON(dk).accno_count;
*/
			INFO(dk).data_count = DAEMON(dk).data_count;
			memcpy (INFO(dk).date, DAEMON(dk).date, sizeof (DAEMON(dk).date));
			INFO(dk).date_flag = DAEMON(dk).date_flag;
			INFO(dk).compact_days = DAEMON(dk).compact_days;

			SHM_Detach ((char *)SHM_Mem[dk]);
			Log (USR_OK, "update INFO with DAEMON[%c%c]",
				_System_Name[0], dk + 'A');

			/* make sub daemon re-attach daemon SHM (INFO)	*/
			sprintf (fifo_name, "%s/%c%c/%s",
				_FEP_FIFO, _System_Name[0], dk + 'A', INFO(dk).exit_FIFO_name);
			FIFO_fd = open (fifo_name, O_RDWR | O_NDELAY);
			if (FIFO_fd == -1)
			{
				Log (FIF_FATAL, "cannot open daemon exit FIFO[%s] {%d:%s}",
					fifo_name, SYS_NO, SYS_STR);
				Exit_Process ();
			}
			write (FIFO_fd, "1", 1);
			close (FIFO_fd);

			// pid <= 0 Added 20210113
			if (INFO(dk).process_no <= 0)
			{
				Log (SYS_ERROR, "Check_Sub_Daemons pid ERROR {%d}", INFO(dk).process_no);
				Exit_Process ();
			}
			else
			{
				rt = kill (INFO(dk).process_no, SIGUSR1);
				if (rt == -1)
					Log (SYS_ERROR, "kill (USR1) failure[%d,%s] {%d:%s}",
						INFO(dk).process_no, INFO(dk).process_id, SYS_NO, SYS_STR);
				else
					Log (SYS_OK, "kill (USR1) %c%c daemon for INFO SHM re-attach p_no[%d]",
						_System_Name[0], dk + 'A', INFO(dk).process_no);
			}
		}
	}

	return;
}	/* End of Check_Sub_Daemons ()	*/

/*************************************************************************
	Function		: . execute memory_mp
						- create and load shared memory
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0(OK):normal, 1(FAIL):file error)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Daemon_SHM_Process (void)
/*----------------------------------------------------------------------*/
{
	int		process_no, rt, status, fd;
	char	path[256], process_id[20];

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
			exit (FAIL);
		case	OK:
			sprintf (process_id, "%cz_memory_mp", _Exe_Name[0]);
			sprintf (path, "%s/%s", path, process_id);

			for (fd = 0; fd < NOFILE; fd ++)
				close (fd);

			rt = execl (path, process_id, "z", (char *)NULL);
			Log (SYS_ERROR, "execl(%s,%s,z) failure[%s] {%d:%s}",
				path, process_id, "create and load shared memory",
				SYS_NO, SYS_STR);
			exit (FAIL);
		default:
			rt = wait (&status);
	}

	return (WEXITSTATUS (status));
}	/* End of Daemon_SHM_Process ()	*/

/*************************************************************************
	Function		: . execute sub daemons
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Daemon_Process (void)
/*----------------------------------------------------------------------*/
{
	int     rt;
	char	tmp[128];

	Start_Process ();

	while (1)
	{
		Poll[0].fd = DTART_FD;
		Poll[0].events = POLLIN;

		rt = poll (Poll, 1, 3 * 1000);

		if (rt < 0)
		{
			if (SYS_NO == EINTR)
				continue;

			Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
			User_Sleep (3);
			continue;
		}

		User_Signal ();

		if (Poll[0].revents & POLLHUP)
		{
			Log (SYS_ERROR, "poll hangup occurred");
			User_Sleep (3);
			continue;
		}

		if (Poll[0].revents & POLLIN)
			Poll[0].revents = 0;

		while (1)
		{
			rt = read (DTART_FD, tmp, sizeof (tmp));
#if defined __linux
			if (rt == 0 || errno == EAGAIN)
#else
			if (rt == 0)
#endif
				break;
		}

		Start_Process ();
	}

	return;
}	/* End of Daemon_Process () */

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
	char	path[256], process_id[20], comp_day[4];

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
			sprintf (process_id, "%cz_compact_mp", _Exe_Name[0]);
			sprintf (path, "%s/%s", path, process_id);
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
	Function		: . start or stop the processes when a signal was caught
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	User_Signal (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	pid_t	pid;
	char	pname[20];

	if (Sig_No == SIGUSR1)
	{
		Start_Process ();
		User_Sleep (5);
		Sig_No = 0;
		return;
	}
	else if (Sig_No == SIGUSR2)
	{
		Stop_Process ();
		User_Sleep (5);
		Sig_No = 0;

		sprintf (pname, "%.2s_procchk_mp", _Exe_Name);
		Log (USR_OK, "User_Signal Check_Proc call");
		pid = Check_Proc (pname);

		rt = kill (pid, SIGTERM);
		if (rt == -1)
			Log (SYS_ERROR, "kill fail (%d,%s) {%d:%s}",
				pid, pname, SYS_NO, SYS_STR);

		Exit_Process ();
	}
	else if (Sig_No == SIGTERM)
		Restart_Process ();

	return;
}	/* End of User_Signal ()	*/

/*************************************************************************
	Function		: . sleep
	Parameters IN	: . p_sec	: sleep time (sec)
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	User_Sleep (int p_sec)
/*----------------------------------------------------------------------*/
{
	usleep (10000);
	sleep (p_sec);

	return;
} 	/* End of User_Sleep ()	*/

/*************************************************************************
	Function		: . execute sub daemons
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Start_Process (void)
/*----------------------------------------------------------------------*/
{
	int 		i, rt, tmp_ss, start_ss, end_ss;
	char		path[256];
	time_t		sys_time;
	struct tm	*date;

	sprintf (path, "%s", _FEP_BIN);

	rt = chdir (path);
	if (rt != 0)
	{
		Log (SYS_FATAL, "cannot change directory[%s] {%d:%s}",
			path, SYS_NO, SYS_STR);
		exit (FAIL);
	}

	/* ??? check ! 2021.1.14 영업일 체크 */ 
	/*
	 *
	 * if (영업일이 아니면) return ;
	 *
	 * */

	for (i = 0; i < SHM_MAX_SUB; i++)
	{
		if (strlen (INFO(i).process_id) == 0)	continue;
		if (INFO(i).process_no != 0)			continue;
		if (INFO(i).process_status == 0)		continue;

		time (&sys_time);
		date = localtime (&sys_time);
		tmp_ss = (date->tm_hour * 60 * 60) + (date->tm_min * 60) + date->tm_sec;

		start_ss = AtoIf (INFO(i).start_time, 2) * 60 * 60 +
			AtoIf (INFO(i).start_time+2, 2) * 60;
		end_ss = AtoIf (INFO(i).end_time, 2) * 60 * 60 +
			AtoIf (INFO(i).end_time+2, 2) * 60;

		/* out of business hours    */
		if ((start_ss > end_ss && (tmp_ss < start_ss && tmp_ss >= end_ss)) ||
			(start_ss < end_ss && (tmp_ss < start_ss || tmp_ss >= end_ss)))
			continue;

		rt = fork ();
		switch (rt)
		{
			case	NOTOK:
				Log (SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
				break;
			case 	OK:
				INFO(i).process_no = getpid ();
				Log (SYS_OK, "START [%d,%d,%s,%s,%s]",
					INFO(i).date_flag, INFO(i).process_no, INFO(i).process_id,
					INFO(i).start_time, INFO(i).end_time);

				sprintf (path, "%s/%s", path, INFO(i).process_id);

				rt = execl (path, INFO(i).process_id, (char *)NULL);
				Log (SYS_ERROR, "execl(%s,%s) failure {%d:%s}",
					path, INFO(i).process_id, SYS_NO, SYS_STR);
				INFO(i).process_no = 0;
				exit (FAIL);
			default:
				usleep (200000);
				break;
		}
	}

	return;
}	/* End of Start_Process ()	*/

/*************************************************************************
	Function		: . stop sub daemons
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Stop_Process (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt, FIFO_fd;
	char	fifo_name[100];

	for (i = 0; i < SHM_MAX_SUB; i++)
	{
		if (INFO(i).process_no != 0)
		{
			if (Sig_No == SIGUSR2)
			{
				sprintf (fifo_name, "%s/%c%c/%s", _FEP_FIFO,
					_System_Name[0], i + 'A', INFO(i).exit_FIFO_name);

				FIFO_fd = open (fifo_name, O_RDWR | O_NDELAY);

				if (FIFO_fd == -1)
				{
					Log (FIF_FATAL, "cannot open daemon exit FIFO[%s] {%d:%s}",
						fifo_name, SYS_NO, SYS_STR);
					Exit_Process ();
				}

				write (FIFO_fd, "1", 1);
				close (FIFO_fd);

				rt = kill (INFO(i).process_no, SIGUSR2);
			}
			else
				rt = kill (INFO(i).process_no, SIGTERM);
			
			if (rt == -1)
			{
				Log (SYS_ERROR, "kill failure[%d] {%d:%s}",
					INFO(i).process_no, SYS_NO, SYS_STR);
				INFO(i).process_no = 0;
			}
			else
			{
				Log (SYS_OK, "STOP [%d,%s,%d,%s,%s,%s]",
					INFO(i).date_flag, INFO(i).date,
					INFO(i).process_no, INFO(i).process_id,
					INFO(i).start_time, INFO(i).end_time);
			}
			INFO(i).process_no = 0;
		}
	}

	return;
}	/* End of Stop_Process ()	*/

/*************************************************************************
	Function		: . set signal handler
	Parameters IN	: . p_signo : signal number
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
		case	SIGTERM:
			Log (SYS_OK, "signal caught [%d:SIGTERM], pz_fepp", p_signo);
			signal (SIGTERM, Sig_Handler);
			break;
		case	SIGUSR1:
			Log (SYS_OK, "signal caught [%d:SIGUSR1], pz_fepp", p_signo);
			signal (SIGUSR1, Sig_Handler);
			break;
		case	SIGUSR2:
			Log (SYS_OK, "signal caught [%d:SIGUSR2], pz_fepp", p_signo);
			signal (SIGUSR2, Sig_Handler);
			break;
		default:
			Log (SYS_OK, "signal caught [%d:XXXXXXX], pz_fepp", p_signo);
			signal (p_signo, Sig_Handler);
			break;
	}


	return;
}	/* End of Sig_Handler ()	*/

/*************************************************************************
	Function		: . execute procchk_mp
						- check process and disk status
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Proc_Check_Process (void)
/*----------------------------------------------------------------------*/
{
	int 	rt, status, process_no, fd;
	char	path[256], process_id[20];

	sprintf (path, "%s", _FEP_BIN);

	rt = chdir (path);
	if (rt != 0)
	{
		Log (SYS_FATAL, "cannot change directory [%s] {%d:%s}",
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
			sprintf (process_id, "%cz_procchk_mp", _Exe_Name[0]);
			sprintf (path, "%s/%s", path, process_id);

			for (fd = 0; fd < NOFILE; fd ++)
				close (fd);

			rt = execl (path, process_id, (char *)NULL);
			Log (SYS_ERROR, "execl(%s,%s) failure[%s] {%d:%s}",
				path, process_id, "check process and disk status",
				SYS_NO, SYS_STR);
			exit (FAIL);
		default:
			break;
	}

	return;
}	/* End of Proc_Check_Process ()	*/

/*************************************************************************
	Function		: . restart super daemon
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Restart_Process (void)
/*----------------------------------------------------------------------*/
{
	int		rt, fd;
	char	pname[100];

	rt = 1;

	/* detach daemon SHM (INFO)	*/
	SHM_Detach ((char *)SHM_All_Daemon_Info);

	while (rt)
	{
		rt = fork ();
		switch (rt)
		{
			case	NOTOK:
				Log (SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
				break;
			case	OK:
				usleep (500000);
				sprintf (pname, "%s/%s", _FEP_BIN, _Exe_Name);

				for (fd = 0; fd < NOFILE; fd ++)
					close (fd);

				rt = execl (pname, _Exe_Name, (char *)NULL);
				Log (SYS_ERROR, "execl(%s,%s) failure[%s] {%d:%s}",
					pname, _Exe_Name, "super daemon", SYS_NO, SYS_STR);
				break;
			default:
				exit (FAIL);
		}
	}

	return;
}	/* End of Restart_Process ()	*/

/*************************************************************************
	End of Program (pz_fepp.c)
*************************************************************************/
