#define		_GLOBAL
/*------------------------------------------------------------------------
#	System	: HATS_H System
#	Module	: change the process status of a process
#	File	: px_sethandsk.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	char	proc_id[12], pstat[4], sub[4], buf[128], fifo_name[128];
    int		dk, pk;
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change handshake status(sesstion_stat) of a process]\n");
		printf ("(PROC.session_stat will be changed)\n\n");
		printf ("Usage: %s <process ID> <status (0:Encrypt 1:NOT Encrypt)>\n\n", argv[0]);
		printf ("  e.g. %s %cc_1101_ts 1\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (proc_id, 0, sizeof (proc_id));
	memset (pstat, 0, sizeof (pstat));

	if (argc == 3)
	{
		memcpy (proc_id, argv[1], strlen (argv[1]));
		memcpy (pstat, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1mprocess ID (e.g. %cc_1101_ts) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (proc_id, sizeof(proc_id), stdin);
		if (proc_id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mprocess handshake status (0:Encrypt 1:NOT Encrypt) ?\033[0m ");
		fflush (stdout);
        fgets (pstat, sizeof(pstat), stdin);
		if (pstat[0] == '\0')
			exit (FAIL);
	}

	if (strlen (proc_id) != 10)
	{
		printf ("ERROR:process ID[%s]\n", proc_id);
		exit (FAIL);
	}

	if (atoi (pstat) != 0 && atoi (pstat) != 1)
	{
		printf ("ERROR:process handshake status[%s]\n", pstat);
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
		if (memcmp (PROC(dk,pk).process_id, proc_id, 10) == 0)
		{
			printf ("process handshake status changed [%s: %d ->",
				PROC(dk,pk).process_id, PROC(dk,pk).session_stat);
			PROC(dk,pk).session_stat = atoi (pstat);
			printf (" %d]\n", PROC(dk,pk).session_stat);
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

	Log (USR_OK, "[%s: %s, %s]", buf, proc_id, pstat);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_sethandsk.c)
*************************************************************************/
