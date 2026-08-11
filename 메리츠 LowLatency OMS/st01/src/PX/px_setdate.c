#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change work date of a class
#	File	: px_setdate.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, pk;
	char	sub[4], work_date[12], gubun[2], buf[128];
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change work date of processes in a class]\n");
		printf ("(PROC.date will be changed)\n\n");
		printf ("Usage: %s <sub name> <work date>\n\n", argv[0]);
		printf ("  e.g. %s JC 20050404\n", argv[0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (sub, 0, sizeof (sub));
	memset (work_date, 0, sizeof (work_date));

	if (argc == 3)
	{
		memcpy (sub, argv[1], strlen (argv[1]));
		memcpy (work_date, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1msub name (e.g. JA) ?\033[0m ");
		fflush (stdout);
        fgets (sub, sizeof(sub), stdin);
		if (sub[0] == '\0')
			exit (FAIL);

		printf ("\033[1mwork date (YYYYMMDD) ?\033[0m ");
		fflush (stdout);
        fgets (work_date, sizeof(work_date), stdin);
		if (work_date[0] == '\0')
			exit (FAIL);

	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	LtoU (sub, 2);
	dk = sub[1] - 'A';

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s daemon not registered !!!\n", sub);
		exit (FAIL);
	}

	printf ("\n  [%s (%s)]\n\n", sub, DAEMON(dk).process_info);

    for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
	{
		printf ("  [%-30.30s, %s] %s",
			PROC(dk,pk).process_info, PROC(dk,pk).process_id, PROC(dk,pk).date);
		memset (PROC(dk,pk).date, 0, sizeof (PROC(dk,pk).date));
		memcpy (PROC(dk,pk).date, work_date, 8);
		printf (" -> %s\n", PROC(dk,pk).date);
	}

#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
	fgets (buf, sizeof (buf), fp);
	pclose (fp);
	buf[strlen(buf)-1] = '\0';

	Log (USR_OK, "[%s: %s, %s]", buf, sub, work_date);

 	exit (OK);
} 	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setdate.c)
*************************************************************************/
