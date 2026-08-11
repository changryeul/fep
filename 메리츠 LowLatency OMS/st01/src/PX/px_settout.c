#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change timeout of a process
#   File	: px_settout.c
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
	char	proc_id[12], time_out[8], sub[4], buf[128];
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change timeout of a process]\n");
		printf ("(PROC.timeout (TIME_VALUE) will be changed)\n\n");
		printf ("Usage: %s <process ID> <timeout (in sec)>\n\n", argv[0]);
		printf ("  e.g. %s %cc_1201_ts 10\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (proc_id, 0, sizeof (proc_id));
	memset (time_out, 0, sizeof (time_out));

	if (argc == 3)
	{
		memcpy (proc_id, argv[1], strlen (argv[1]));
		memcpy (time_out, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1mprocess name (e.g. %cc_1201_ts) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (proc_id, sizeof(proc_id), stdin);
		if (proc_id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mtimeout (in sec) ?\033[0m ");
		fflush (stdout);
        fgets (time_out, sizeof(time_out), stdin);
		if (time_out[0] == '\0')
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
			printf ("timeout changed [%s: %d ->",
				PROC(dk,pk).process_id, TIME_VALUE(dk,pk));

   			TIME_VALUE(dk,pk) = atoi (time_out);

			printf (" %d]\n", TIME_VALUE(dk,pk));
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

	Log (USR_OK, "[%s: %s, %s]", buf, proc_id, time_out);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_settout.c)
*************************************************************************/
