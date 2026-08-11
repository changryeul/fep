#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change the start status of a process
#	File	: px_setsstat.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	char	proc_id[12], sstat[4], sub[4], buf[128];
	int		dk, pk;
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change the start status of a process]\n");
		printf ("(PROC.start_status will be changed)\n\n");
		printf ("Usage: %s <process ID> <start status (0:init 1:start 2:end 3:stop)>\n\n", argv[0]);
		printf ("  e.g. 1) %s %cc_1101_tr 3\n", argv[0], argv[0][0]);
		printf ("       2) %s %ccts 1\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (proc_id, 0, sizeof (proc_id));
	memset (sstat, 0, sizeof (sstat));

	if (argc == 3)
	{
		memcpy (proc_id, argv[1], strlen (argv[1]));
		memcpy (sstat, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1mprocess ID (e.g. %ca_1101_tr) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (proc_id, sizeof(proc_id), stdin);
		if (proc_id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mstart status (0:init 1:start 2:end 3:stop) ?\033[0m ");
		fflush (stdout);
        fgets (sstat, sizeof(sstat), stdin);
		if (sstat[0] == '\0')
			exit (FAIL);
	}

	if (strlen (proc_id) != 10 && strlen (proc_id) != 4)
	{
		printf ("ERROR:process ID[%s]\n", proc_id);
		exit (FAIL);
	}

	if (atoi (sstat) < 0 || atoi (sstat) > 3)
	{
		printf ("ERROR:start status[%s]\n", sstat);
		exit (FAIL);
	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
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
		if ((strlen (proc_id) == 10 &&
			memcmp (PROC(dk,pk).process_id, proc_id, 10) == 0) ||
			(strlen (proc_id) == 4 &&
			memcmp (PROC(dk,pk).process_id, proc_id, 2) == 0 &&
			memcmp (PROC(dk,pk).process_id+8, proc_id+2, 2) == 0))
		{
			printf ("start_status changed [%s: %d ->",
				PROC(dk,pk).process_id, PROC(dk,pk).start_status);

			PROC(dk,pk).start_status = atoi (sstat);

			printf (" %d]\n", PROC(dk,pk).start_status);

			if (strlen (proc_id) == 10)
				break;
		}

		if (strlen (proc_id) == 10 && pk == DAEMON(dk).p_count - 1)
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

	Log (USR_OK, "[%s: %s, %s]", buf, proc_id, sstat);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setsstat.c)
*************************************************************************/
