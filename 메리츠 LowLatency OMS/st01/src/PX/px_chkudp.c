#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: check interface lines (UDP)
#	File	: px_chkudp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
char	Type[4];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void    Process_Info (int, int, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	char	sub[4];
	int		dk, pk;

	if (argc != 2 && argc != 3)
	{
		printf ("==========================================================\n");
		printf ("[check interface lines (UDP)]\n\n");
		printf ("Usage: %s <sub name> <type>\n", argv[0]);
		printf ("  e.g. 1) %s %ca\n", argv[0], argv[0][0]);
		printf ("       2) %s %ca ur\n", argv[0], argv[0][0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	/* initialize global variables and attach to daemon SHM (INFO)	*/
	Init_Mana (argc, argv);

	memset (sub, 0, sizeof (sub));
	memcpy (sub, argv[1], strlen (argv[1]));
	LtoU (sub, 2);
	dk = sub[1] - 'A';

	if (argc == 3)
	{
		memset (Type, 0, sizeof (Type));
		memcpy (Type, argv[2], strlen (argv[2]));
	}

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s daemon not registered !!!\n", sub);
		exit (FAIL);
	}

	printf ("\033[4m[%s, %s]\033[0m\n", sub, DAEMON(dk).process_info);
	puts ("\033[1m* SR (S:stop R:run)\033[0m");
	printf ("\033[7mID         SR IP             :Port   Info                   \033[0m\n");

	for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
	{
		if (PROC(dk,pk).type == TY_URS)
			Process_Info (dk, pk, argc);
	}

	exit (OK);
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	Process_Info (int dk, int pk, int argnum)
/*----------------------------------------------------------------------*/
{
	int		i;
	char	info[44], ip[16];

	if (argnum == 3 &&
		memcmp (PROC(dk,pk).process_id+8, Type, strlen (Type)) != 0)
		return;

	memset (info, 0, sizeof (info));
	memcpy (info, PROC(dk,pk).process_info, sizeof (PROC(dk,pk).process_info));

	if (Chk_Korean (info, 23) == -1)
		info[22] = ' ';

	if (PROC(dk,pk).process_no == 0)
		printf ("\033[1m%10.10s %c \033[0m",
			PROC(dk,pk).process_id, 'S');
	else
		printf ("%10.10s %c ", PROC(dk,pk).process_id, 'R');

	for (i = 0; i < 20; i ++)
	{
		if (UDP_PORT(dk,pk,i) != 0)
		{
			if (i % 3 == 2)
				printf ("\n              ");

			sprintf (ip, "%d.%d.%d.%d", UDP_IP1(dk,pk,i), UDP_IP2(dk,pk,i),
				UDP_IP3(dk,pk,i), UDP_IP4(dk,pk,i));
			printf (" %-15.15s:%d", ip, UDP_PORT(dk,pk,i));
		}
	}

	if (PROC(dk,pk).process_no == 0)
		printf ("\033[1m  %-23.23s\033[0m", info);
	else
		printf ("  %-23.23s", info);

	printf ("\n");

	return;
}	/* End of Process_Info ()	*/

/*************************************************************************
	End of Program (px_chkudp.c)
*************************************************************************/
