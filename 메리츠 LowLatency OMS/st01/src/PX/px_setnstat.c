#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change network status (TCP1, TCP2)
#	File	: px_setnstat.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	char	proc_id[12], line[4], nstat[4], sub[4], buf[128];
	int		dk, pk, l;
	FILE	*fp;

	if (argc != 1 && argc != 4)
	{
		printf ("==========================================================\n");
		printf ("[change network status (TCP1, TCP2)]\n\n");
		printf ("Usage: %s <process ID> <line (P|B|A)> <network status (0|1)>\n", argv[0]);
		printf ("       (line = P:primary B:backup A:all\n");
		printf ("       network status = 0:off 1:on 2:end)\n\n");
		printf ("  e.g. %s %ca_1111_ts P 1\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (proc_id, 0, sizeof (proc_id));
	memset (line, 0, sizeof (line));
	memset (nstat, 0, sizeof (nstat));

	if (argc == 4)
	{
		memcpy (proc_id, argv[1], strlen (argv[1]));
		memcpy (line, argv[2], strlen (argv[2]));
		memcpy (nstat, argv[3], strlen (argv[3]));
	}
	else
	{
		printf ("\033[1mprocess ID (e.g. %ca_1111_ts) ?\033[0m ", argv[0][0]);
		fflush (stdout);
        fgets (proc_id, sizeof(proc_id), stdin);
		if (proc_id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mline flag (P|B|A) ?\033[0m ");
		fflush (stdout);
        fgets (line, sizeof(line), stdin);
		if (line[0] == '\0')
			exit (FAIL);

		printf ("\033[1mnetwork status (0:off 1:on 2:end) ?\033[0m ");
		fflush (stdout);
        fgets (nstat, sizeof(nstat), stdin);
		if (nstat[0] == '\0')
			exit (FAIL);
	}

	if (strlen (proc_id) != 10)
	{
		printf ("ERROR:process ID[%s]\n", proc_id);
		exit (FAIL);
	}

	LtoU (line, 1);

	if (line[0] != 'P' && line[0] != 'B' && line[0] != 'A')
	{
		printf ("ERROR:line flag[%s]\n", line);
		exit (FAIL);
	}

	if (atoi (nstat) < 0 || atoi (nstat) > 3)
	{
		printf ("ERROR:network status[%s]\n", nstat);
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
			if (PROC(dk,pk).type != TY_TRS1 && PROC(dk,pk).type != TY_TRS2)
			{
				puts ("TCP1 or TCP2 process only !!!");
				exit (FAIL);
			}

			printf ("[%-10.10s:%s:network status of ",
				PROC(dk,pk).process_id, PROC(dk,pk).process_info);

			switch (line[0])
			{
				case	'P':
					if (PROC(dk,pk).type == TY_TRS1)
					{
					    TCP1_NSTAT(dk,pk) = atoi (nstat);
					    printf ("primary = %d]\n", TCP1_NSTAT(dk,pk));
					}
					else
					{
					    TCP2_NSTAT(dk,pk,0) = atoi (nstat);
					    printf ("primary = %d]\n", TCP2_NSTAT(dk,pk,0));
					}

					break;
				case	'B':
					if (PROC(dk,pk).type == TY_TRS2)
					{
					    TCP2_NSTAT(dk,pk,1) = atoi (nstat);
					    printf ("backup = %d]\n", TCP2_NSTAT(dk,pk,1));
					}
					else
					{
						puts ("X.25, TCP2 process only !!!");
						exit (FAIL);
					}

					break;
				case	'A':
					printf ("all lines changed]\n");

					if (PROC(dk,pk).type == TY_TRS1)
					{
						TCP1_NSTAT(dk,pk) = atoi (nstat);
						printf ("network status = %d\n", TCP1_NSTAT(dk,pk));
					}
					else if (PROC(dk,pk).type == TY_TRS2)
					{
						for (l = 0; l < 2; l ++)
						{
							if (PROC(dk,pk).l.t2.l[l])
							{
								TCP2_NSTAT(dk,pk,l) = atoi (nstat);
								if (l == 0)
									printf ("network status of primary = ");
								else
									printf ("network status of backup = ");
								printf ("%d\n", TCP2_NSTAT(dk,pk,l));
							}
						}
					}

					break;
				default:
					break;
			}
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

	Log (USR_OK, "[%s: %s, %s, %s]", buf, proc_id, line, nstat);

	exit (OK);
} 	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setnstat.c)
*************************************************************************/
