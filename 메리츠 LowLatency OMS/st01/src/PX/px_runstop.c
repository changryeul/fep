#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: start or stop the sub daemon
#	File	: px_runstop.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, rt;
	pid_t	pid;
	u_char	run_flag, reload_flag, query_flag;
	char	daemon_name[16], buf[128], path[256];
	FILE	*fp;

	if (argc != 1)
	{
		printf ("==========================================================\n");
		printf ("[start or stop the sub daemon]\n\n");
		printf ("Usage: %s\n", argv[0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	query_flag = 1;

	while (query_flag)
	{
		printf ("\033[1msub name (e.g. %ca) ? [q:quit]\033[0m ", _Exe_Name[0]);
		fflush (stdout);
		memset (buf, 0, sizeof (buf));
		fflush (stdout);
        fgets (buf, sizeof(buf), stdin);
		UtoL (buf, strlen (buf));

		if (buf[0] == 'q' && strlen (buf) == 1)
			exit (OK);

		if (buf[0] != _Exe_Name[0] || buf[1] < 'a' || buf[1] > 'w' ||
			strlen (buf) != 2 || buf[0] == '\0')
			continue;

		sprintf (daemon_name, "%s_daemon_mp", buf);
		break;
   	}

	dk = daemon_name[1] - 'a';

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s not registered !!!\n", daemon_name);
		exit (FAIL);
	}

	pid = Check_Proc (daemon_name);

	while (query_flag)
	{
		printf ("\033[1mstop or run (s:stop r:run) ? [q:quit]\033[0m ");
		fflush (stdout);
		memset (buf, 0, sizeof (buf));
        fgets (buf, sizeof(buf), stdin);
		UtoL (buf, strlen (buf));
		UtoL (buf, strlen (buf));

		if (buf[0] == 'q' && strlen (buf) == 1)
			exit (OK);

		if ((buf[0] != 's' && buf[0] != 'r') || buf[0] == '\0')
			continue;

		run_flag = buf[0];

		if (run_flag == 's')
		{
			if (pid == 0)
			{
				printf ("ERROR: %s not run !\n", daemon_name);
				exit (FAIL);
			}
			else
			{
				INFO(dk).process_status = 0;
				rt = kill (INFO(dk).process_no, SIGTERM);
				if (rt == -1)
				{
					printf ("ERROR:kill failure [%d] {%d:%s}\n",
						INFO(dk).process_no, SYS_NO, SYS_STR);
					exit (FAIL);
				}
				printf ("\nOK: %s stopped !!!\n", daemon_name);
			}
			query_flag = 0;
		}
		else
		{
			if (pid != 0)
			{
				printf ("ERROR: %s already run !\n", daemon_name);
				exit (FAIL);
			}
		}
		break;
   	}

	while (query_flag)
	{
		printf ("\033[1mreload SHM (y:yes n:no) ? [q:quit]\033[0m ");
		fflush (stdout);
		memset (buf, 0, sizeof (buf));
        fgets (buf, sizeof(buf), stdin);
		UtoL (buf, strlen (buf));
		UtoL (buf, strlen (buf));

		if (buf[0] == 'q' && strlen (buf) == 1)
			exit (OK);

		if ((buf[0] != 'y' && buf[0] != 'n') || buf[0] == '\0')
			continue;

		reload_flag = buf[0];

		if (reload_flag == 'y')
		{
			INFO(dk).system_status = 0;

			sprintf (path, "%s", _FEP_BIN);
			rt = chdir (path);
			if (rt != 0)
			{
				printf ("ERROR:cannot change directory[%s] {%d:%s}\n",
					path, SYS_NO, SYS_STR);
				exit (FAIL);
			}

			rt = system (daemon_name);
			if (rt < 0)
			{
				printf ("ERROR:system call failure[%s] {%d:%s}\n",
					daemon_name, SYS_NO, SYS_STR);
				exit (FAIL);
			}
			printf ("\nOK: %s started (SHM reloaded) !!!\n", daemon_name);
		}
		else
		{
			INFO(dk).system_status = 1;
			INFO(dk).process_status = 1;
			INFO(dk).process_no = 0;
			printf ("\nOK: %s started (current SHM used) !!!\n", daemon_name);
		}
		break;
   	}

	memset (buf, 0, sizeof (buf));
#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
	fgets (buf, sizeof (buf), fp);
	pclose (fp);
	buf[strlen(buf)-1] = '\0';

	Log (USR_OK, "[%s: %s %c %c]", buf, daemon_name, run_flag, reload_flag);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_runstop.c)
*************************************************************************/
