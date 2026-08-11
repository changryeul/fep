#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change process status of a sub daemon
#	File	: px_setinfostat.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	char	sub[4], pstat[4], buf[128];
	int		dk;
	FILE	*fp;

	if (argc != 1 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[change process status of a sub daemon]\n");
		printf ("(INFO.process_status will be changed)\n\n");
		printf ("Usage: %s <sub name> <process status>\n", argv[0]);
		printf ("       (process status = 0:stop 1:run)\n\n", argv[0]);
		printf ("  e.g. %s %ca 1\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (sub, 0, sizeof (sub));
	memset (pstat, 0, sizeof (pstat));

	if (argc == 3)
	{
		memcpy (sub, argv[1], strlen (argv[1]));
		memcpy (pstat, argv[2], strlen (argv[2]));
	}
	else
	{
		printf ("\033[1msub name (e.g. %ca) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (sub, sizeof(sub), stdin);
		if (sub[0] == '\0')
			exit (FAIL);

		printf ("\033[1mprocess status (0:stop 1:run) ?\033[0m ");
		fflush (stdout);
        fgets (pstat, sizeof(pstat), stdin);
		if (pstat[0] == '\0')
			exit (FAIL);
	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	LtoU (sub, 2);
	dk = sub[1] - 'A';

	if (atoi (pstat) != 0 && atoi (pstat) != 1)
	{
        printf ("ERROR:process status[%s]\n", pstat);
        exit (FAIL);
	}

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s daemon not registered !!!\n", sub);
		exit (FAIL);
	}

	printf ("INFO process_status changed [%s: %d ->",
		INFO(dk).process_id, INFO(dk).process_status);

	INFO(dk).process_status = atoi (pstat);

	printf (" %d]\n", INFO(dk).process_status);

#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
	fgets (buf, sizeof (buf), fp);
	pclose (fp);
	buf[strlen(buf)-1] = '\0';

	Log (USR_OK, "[%s: %s, %s]", buf, sub, pstat);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setinfostat.c)
*************************************************************************/
