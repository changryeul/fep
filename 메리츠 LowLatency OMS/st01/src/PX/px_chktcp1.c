#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: check interface lines (TCP1)
#	File	: px_chktcp1.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void    Process_Info (int, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, pk;
	char	sub[4], display_all;

	if (argc != 2 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[check interface lines (TCP1)]\n\n");
		printf ("Usage: %s <sub name|\"all\"> <\"all\">\n\n", argv[0]);
		printf ("  e.g. 1) %s %ca\n", argv[0], argv[0][0]);
		printf ("       2) %s %ca all\n", argv[0], argv[0][0]);
		printf ("       3) %s all\n", argv[0]);
		printf ("       4) %s all all\n", argv[0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	if (memcmp (argv[1], "all", 3) == 0)
		dk = 99;
	else
	{
		memset (sub, 0, sizeof (sub));
		memcpy (sub, argv[1], strlen (argv[1]));

		if (strlen (sub) != 2 || sub[0] != _Exe_Name[0] ||
			sub[1] < 'a' || sub[1] > 'z')
		{
			printf ("invalid sub name [%s]\n", sub);
			exit (FAIL);
		}

		LtoU (sub, 2);
		dk = sub[1] - 'A';

		if (INFO(dk).process_id[0] == 0)
		{
			printf ("%s daemon not registered !!!\n", sub);
			exit (FAIL);
		}
	}

	display_all = OFF;

	if (argc == 3 && memcmp (argv[2], "all", 3) == 0)
		display_all = ON;

	puts ("\033[1m* SR (S:stop R:run), L:line_gubun (1:main 2:backup), P:port_type, N:NSTAT\033[0m");

	if (dk == 99)
	{
		for (dk = 0; dk < SHM_MAX_SUB; dk ++)
		{
			if (INFO(dk).process_id[0] == 0)
				continue;

			memset (sub, 0, sizeof (sub));
			sub[0] = _Exe_Name[0];
			sub[1] = dk + 'a';
			LtoU (sub, 2);

			printf ("\033[4m[%s, %s]\033[0m\n", sub, DAEMON(dk).process_info);
			printf ("\033[7mID         Info                            SR L P  Port N        ClientIP:Port \033[0m\n");

			for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
			{
				if (PROC(dk,pk).type == TY_TRS1)
				{
					if (display_all == OFF && PROC(dk,pk).process_no == 0)
						continue;

					Process_Info (dk, pk);
				}
			}
		}
	}
	else
	{
		printf ("\033[4m[%s, %s]\033[0m\n", sub, DAEMON(dk).process_info);
		printf ("\033[7mID         Info                            SR L P  Port N        ClientIP:Port \033[0m\n");

		for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
		{
			if (PROC(dk,pk).type == TY_TRS1)
			{
				if (display_all == OFF && PROC(dk,pk).process_no == 0)
					continue;

				Process_Info (dk, pk);
			}
		}
	}

	exit (OK);
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	Process_Info (int i, int j)
/*----------------------------------------------------------------------*/
{
	u_short		port;
	int			l;
	char		info[40];

	memset (info, 0, sizeof (info));
	memcpy (info, PROC(i,j).process_info, sizeof (PROC(i,j).process_info));

	if (Chk_Korean (info, 32) == -1)
		info[31] = ' ';

	if (PROC(i,j).process_no == 0)
		printf ("\033[1m%10.10s %-32.32s %c",
			PROC(i,j).process_id, info, 'S');
	else
		printf ("%10.10s %-32.32s %c", PROC(i,j).process_id, info, 'R');

	if (PROC(i,j).l.t1.line_gubun != 0)
	{
		if (PROC(i,j).l.t1.port_type == 0)					/* master	*/
			port = TCP1_PORT(i,j);
		else
		{
			if (PROC(i,j).process_status != 9)				/* service	*/
				port = TCP1_SPORT(i,j,PROC(i,j).l.t1.port_type-1);
			else
				port = TCP1('w'-'a',PROC(i,j).l.t1.line_gubun-1).
					service_port_no[PROC(i,j).l.t1.port_type-1];
		}

		printf (" %d %d %5d %d %15.15s:%d",
			PROC(i,j).l.t1.line_gubun, PROC(i,j).l.t1.port_type,
			port, PROC(i,j).l.t1.network_status, PROC(i,j).ip, PROC(i,j).port);
	}

	if (PROC(i,j).process_no == 0)
		printf ("\033[0m\n");
	else
		printf ("\n");

	return;
}	/* End of Process_Info ()	*/

/*************************************************************************
	End of Program (px_chktcp1.c)
*************************************************************************/
