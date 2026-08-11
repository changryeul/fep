#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change delay check time of a process
#   File	: px_setdelay.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, pk;
	char	proc_id[12], delay_time[8], sub[4], buf[128];
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change delay check time of a process]\n");
		printf ("(PROC.delay (DELAY_TIME) will be changed)\n\n");
		printf ("Usage: %s <process ID> <delay check time (in millisec)>\n\n",
			argv[0]);
		printf ("  e.g. %s %cc_1101_ts 1000\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (proc_id, 0, sizeof (proc_id));
	memset (delay_time, 0, sizeof (delay_time));

	if (argc == 3)
	{
		memcpy (proc_id, argv[1], strlen (argv[1]));
		memcpy (delay_time, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1mprocess name (e.g. %cc_1101_ts) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (proc_id, sizeof(proc_id), stdin);
		if (proc_id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mdelay check time (in millisec) ?\033[0m ");
		fflush (stdout);
        fgets (delay_time, sizeof(delay_time), stdin);
		if (delay_time[0] == '\0')
			exit (FAIL);
	}

	if (strlen (proc_id) != 10)
	{
		printf ("ERROR:process name[%s]\n", proc_id);
		exit (FAIL);
	}

	Init_Mana (argc, argv);

	sprintf (sub, "%-2.2s", proc_id);
	LtoU (sub, 2);
	dk = sub[1] - 'A';

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s daemon not registered !!!\n", sub);
		exit (FAIL);
	}

	for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
	{
		if (memcmp (PROC(dk,pk).process_id, proc_id, 10) == 0)
		{
			printf ("delay check time changed [%s: %d ->",
				PROC(dk,pk).process_id, PROC(dk,pk).delay);

   			PROC(dk,pk).delay = atoi (delay_time);

			printf (" %d]\n", PROC(dk,pk).delay);
			break;
		}

		if (pk == DAEMON(dk).p_count - 1)
		{
			printf ("ERROR:process not registered[%s]\n", proc_id);
			exit (FAIL);
		}
	}

#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
	fgets (buf, sizeof (buf), fp);
	pclose (fp);
	buf[strlen(buf)-1] = '\0';

	Log (USR_OK, "[%s: %s, %s]", buf, proc_id, delay_time);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setdelay.c)
*************************************************************************/
