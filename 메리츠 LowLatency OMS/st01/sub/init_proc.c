/*------------------------------------------------------------------------
#	Module	: initialize global variables and attach to daemon SHM
#				- used by job processes
#	File	: init_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*----------------------------------------------------------------------*/
void	Init_Proc (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		i, j, pk, shmid, sk;
	char	fifo_name[256], bumun[4], key[12], attach_flag[MAX_DSHM_SEG];
	key_t	base_key, new_key;

	if (argv[0][1] < 'a' || argv[0][1] > 'z')
	{
		printf ("ERROR:invalid sub system name[%c]\n", argv[0][1]);
		exit (FAIL);
	}

	_System_Name[0] = argv[0][0];						/* system name	*/
	LtoU (_System_Name, 1);

	sprintf (_Exe_Name, "%s", argv[0]);				/* execution name	*/
	sprintf (_Process_Name, "%-10.10s", argv[0]);
	sprintf (_SubSystem_Name, "%-2.2s", argv[0]);			/* sub name	*/
	
	/* check environment values	*/
	Check_Environment ();

	D_K = _SubSystem_Name[1] - 'a';						/* daemon key	*/
	P_K = -1;											/* process key	*/
	if (D_K < 0 || D_K > SHM_MAX_SUB)
	{
		Log (PRO_FATAL, "Init_Proc:invalid daemon key[%d]", D_K);
		Exit_Process ();
	}

	Log (PRO_OK, "process START");

	/* check if this module run or not	*/
	Check_Exist ();

	/* make it a daemon	*/
	Make_Daemon ();

	/* set fatal signal handlers	*/
	Setsigfatal ();

	/* attach daemon SHM (INFO)	*/
	Sub_SHM ();

	/* attach sub SHM	*/
	Mem_SHM (1, D_K);

	/* attach sise data SHM	*/
	if (DAEMON(D_K).sisetr_count > 0)
		Sise_SHM ();

	/* detach sub SHM	*/
	SHM_Detach ((char *)SHM_All_Daemon_Info);

	for (pk = 0; pk < DAEMON(D_K).p_count; pk ++)
	{
		if (memcmp (PROC(D_K,pk).process_id,
			_Process_Name, strlen (_Process_Name)) == 0)
			break;
	}
	
	if (pk >= DAEMON(D_K).p_count)
	{
		Log (PRO_FATAL, "Init_Proc:unregistered process[%s]", _Process_Name);
		Exit_Process ();
	}
	
	P_K = pk;

	sprintf (bumun, "%s", _SubSystem_Name);
	LtoU (bumun, 2);

	sprintf (fifo_name,
		"%s/%s/%s", _FEP_FIFO, bumun, DAEMON(D_K).start_FIFO_name);

	SFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);

	if (SFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "Init_Proc:cannot open start FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	sprintf (fifo_name,
		"%s/%s/%s", _FEP_FIFO, bumun, DAEMON(D_K).daemon_FIFO_name);

	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);

	if (DFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "Init_Proc:cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	base_key = BASE_SHM_KEY;
	memset (attach_flag, 0, sizeof (attach_flag));

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		base_key += 0x01000000L;

	sprintf (key, "0x00%02d0000", D_K + 1);
	base_key += strtol (key, NULL, 16);

	if (DAEMON(D_K).shm_log == 1)			/* use SHM (delayed) log	*/
	{
		new_key = base_key + 0x00000001L;

		ShmLogSemId = SEM_Creat_Excl (new_key);

		if (ShmLogSemId == -1)
		{
			ShmLogSemId = SEM_Creat (new_key);

			if ( ShmLogSemId == -1)
			{
				Log (SYS_FATAL, "SEM_Creat (%#x) fail {%d:%s}",
					new_key, SYS_NO, SYS_STR);
				Exit_Process ();
			}
		}

		sprintf (fifo_name,
			"%s/%s/%s_slog", _FEP_FIFO, bumun, DAEMON(D_K).start_FIFO_name);

		ShmLogFifoFd = open (fifo_name, O_RDWR|O_NDELAY);

		if (ShmLogFifoFd == -1)
		{
			Log (FIF_FATAL, "Init_Proc:cannot open SHM log FIFO[%s] {%d:%s}",
				fifo_name, SYS_NO, SYS_STR);
			Exit_Process ();
		}
	}

	for (i = 0; i < 3; i ++)
	{
		if (PROCI.fifo_f[i][0] != 0)
		{
			sprintf (fifo_name, "%s/%s/%s", _FEP_FIFO, bumun, PROCI.fifo_f[i]);

			PROCI.in_FIFO_fd[i] = open (fifo_name, O_RDWR|O_NDELAY);

			if (PROCI.in_FIFO_fd[i] == -1)
			{
				Log (FIF_FATAL, "Init_Proc:cannot open IFIFD %d[%s] {%d:%s}",
					i + 1, fifo_name, SYS_NO, SYS_STR);
				Exit_Process ();
			}
		}

		if (PROCI.in_d[i] != 0)
		{
			sk = AtoIf (IDK(D_K,P_K,i), 2) - 1;

			if (sk < 0)
			{
				Log (USR_FATAL, "Init_Proc:invalid DSHM key[%s]",
					IDK(D_K,P_K,i));
				Exit_Process ();
			}

			if (attach_flag[sk] == OFF)
			{
				sprintf (key, "0x0000%02d00", sk + 1);
				new_key = base_key + strtol (key, NULL, 16);
				DShmPtr[sk] = Shm_Attach (new_key, &shmid);
				attach_flag[sk] = ON;
			}
		}
	}

	for (i = 0; i < 99; i ++)
	{
		if (PROCI.out_d[i] == 0)
			break;
		else
		{
			/* open output data FIFOs	*/
			for (j = 0; j <= ODC(D_K,P_K,i); j ++)
			{
				sprintf (fifo_name, "%s/%s/%s%d",
					_FEP_FIFO, bumun, ODN(D_K,P_K,i), j);

				OD_FIFO_fd[i][j] = open (fifo_name, O_RDWR|O_NDELAY);

				if (OD_FIFO_fd[i][j] == -1)
				{
					Log (FIF_FATAL,
						"Init_Proc:cannot open out data FIFO[%s] {%d:%s}",
						fifo_name, SYS_NO, SYS_STR);
					Exit_Process ();
				}
			}

			/* attach data SHM	*/
			sk = AtoIf (ODK(D_K,P_K,i), 2) - 1;

			if (sk < 0)
			{
				Log (USR_FATAL, "Init_Proc:invalid DSHM key[%s]",
					ODK(D_K,P_K,i));
				Exit_Process ();
			}

			if (attach_flag[sk] == OFF)
			{
				sprintf (key, "0x0000%02d00", sk + 1);
				new_key = base_key + strtol (key, NULL, 16);
				DShmPtr[sk] = Shm_Attach (new_key, &shmid);
				attach_flag[sk] = ON;
			}

			/* semaphore	*/
			sprintf (key, "0x0000%4.4s", ODK(D_K,P_K,i));
			new_key = base_key + strtol (key, NULL, 16);

			SemId[i] = SEM_Creat_Excl (new_key);

			if (SemId[i] == -1)
			{
				SemId[i] = SEM_Creat (new_key);

				if (SemId[i] == -1)
				{
					Log (SYS_FATAL, "SEM_Creat fail {%d:%s}",
						SYS_NO, SYS_STR);
					Exit_Process ();
				}
			}
		}
	}

	Log (USR_OK,
	"process initialized [%s,%d,%d:%d,%d,%s,%s,%d,%s,%d,%d:%d,%d:%s]",
		PROCI.process_id, PROCI.process_status, PROCI.in_FIFO_fd[0],
		PROCI.start_status, PROCI.timeout, PROCI.start_time, PROCI.end_time,
		PROCI.backup, PROCI.process_type, PROCI.if_seq, PROCI.type,
		SFIFD(D_K), DFIFD(D_K), PROCI.process_info);

	if (PROCI.type == TY_MP || PROCI.type == TY_DD ||
		PROCI.type == TY_TRS1 || PROCI.type == TY_TRS2 ||
		PROCI.type == TY_BRS || PROCI.type == TY_URS)
		PROCI.process_no = getpid ();
	else
	{
		Log (PRO_FATAL, "Init_Proc:invalid process type[%d]",
			PROC(D_K,P_K).type);
		Exit_Process ();
	}

	return;
}	/* End of Init_Proc ()	*/

/*************************************************************************
	End of Program (init_proc.c)
*************************************************************************/
