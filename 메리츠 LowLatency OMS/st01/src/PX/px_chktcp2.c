#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: check interface lines (TCP2)
#	File	: px_chktcp2.c
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
	char	sub[4];

	if (argc != 2)
	{
		printf ("==========================================================\n");
		printf ("[check interface lines (TCP2)]\n\n");
		printf ("Usage: %s <sub name|\"all\">\n\n", argv[0]);
		printf ("  e.g. 1) %s %cc\n", argv[0], argv[0][0]);
		printf ("       2) %s all\n", argv[0]);
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

	puts ("\033[1m* C:connect status (1:connected), SR (S:stop R:run), MB (M:main B:backup),\033[0m");
	puts ("\033[1m  P:process status L:line status N:network status\033[0m");

	if (dk == 99)
	{
		for (dk = 0; dk < SHM_MAX_SUB; dk ++)
		{
			if (INFO(dk).process_id[0] == 0 || INFO(dk).tcp2_count == 0)
				continue;

			memset (sub, 0, sizeof (sub));
			sub[0] = _Exe_Name[0];
			sub[1] = dk + 'a';
			LtoU (sub, 2);

			printf ("\033[4m[%s, %s]\033[0m\n", sub, DAEMON(dk).process_info);
			printf ("\033[7mID         C SR MB  IP              Port  P L N   IP              Port  P L N Info                          \033[0m\n");

			for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
			{
				if (PROC(dk,pk).type == TY_TRS2)
					Process_Info (dk, pk);
			}
		}
	}
	else
	{
		if (INFO(dk).tcp2_count == 0)
		{
			printf ("TCP2 process not registered in %s\n", sub);
			exit (FAIL);
		}

		printf ("\033[4m[%s, %s]\033[0m\n", sub, DAEMON(dk).process_info);
		printf ("\033[7mID         C SR MB  IP              Port  P L N   IP              Port  P L N Info                          \033[0m\n");

		for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
		{
			if (PROC(dk,pk).type == TY_TRS2)
				Process_Info (dk, pk);
		}
	}

	exit (OK);
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	Process_Info (int i, int j)
/*----------------------------------------------------------------------*/
{
	int		l;
	char	ip[16], info[44];

	memset (info, 0, sizeof (info));
	memcpy (info, PROC(i,j).process_info, sizeof (PROC(i,j).process_info));

	if (Chk_Korean (info, 30) == -1)
		info[29] = ' ';

	if (PROC(i,j).process_no == 0)
		printf ("\033[1m%10.10s %d %c\033[0m", PROC(i,j).process_id,
			PROC(i,j).l.t2.connect_status, 'S');
	else
		printf ("%10.10s %d %c", PROC(i,j).process_id,
			PROC(i,j).l.t2.connect_status, 'R');

	if (PROC(i,j).l.t2.line_gubun == 0)
		printf ("  M");
	else
		printf ("  \033[35mB\033[0m");

	for (l = 0; l < 2; l ++)
	{
		if(PROC(i,j).l.t2.l[l])
		{
			sprintf (ip, "%d.%d.%d.%d", TCP2_IP1(i,j,l), TCP2_IP2(i,j,l),
				TCP2_IP3(i,j,l), TCP2_IP4(i,j,l));
			printf ("   %-15.15s %5d %d,",
				ip, TCP2_PORT(i,j,l), TCP2_PSTAT(i,j,l));

			if (TCP2_LSTAT(i,j,l) == OFF)
				printf ("\033[31m%d\033[0m,%d",
					TCP2_LSTAT(i,j,l), TCP2_NSTAT(i,j,l));
			else
				printf ("%d,%d", TCP2_LSTAT(i,j,l), TCP2_NSTAT(i,j,l));
		}
	}

	if (PROC(i,j).process_no == 0)
		printf ("\033[1m %-30.30s\033[0m", info);
	else
		printf (" %-30.30s", info);

	printf ("\n");

	return;
}	/* End of Process_Info ()	*/

/*************************************************************************
	End of Program (px_chktcp2.c)
*************************************************************************/
