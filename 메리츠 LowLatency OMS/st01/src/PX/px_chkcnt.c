#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: report process interface sequence and data read/write count
#	File	: px_chkcnt.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Dk, ProcFlag, FileFlag;
char	FileName[20];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Display_Count (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk;
	char	d_time[16], hostname[10];

	if (argc < 2 || argc > 3)
	{
		printf ("==========================================================\n");
		printf ("[report process interface sequence and file R/W count]\n\n");
		printf ("Usage: %s <\"all\"|sub name> <file name|\"proc\"|\"file\">\n\n", argv[0]);
		printf ("  e.g. 1) %s all           - 전 부문\n", argv[0]);
		printf ("       2) %s pa            - PA 부문\n", argv[0]);
		printf ("       3) %s pa pa         - PA 부문 process/file\n", argv[0]);
		printf ("       4) %s pa pa_1111_ts - PA 부문 process, 지정 file\n",
			argv[0]);
		printf ("       5) %s pa proc       - PA 부문 process\n", argv[0]);
		printf ("       6) %s pa file       - PA 부문 file\n", argv[0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	ProcFlag = FileFlag = OFF;
	memset (FileName, 0, sizeof (FileName));

	Get_DateTime (d_time);
	gethostname (hostname, sizeof (hostname));
	printf ("[%s: %s, %.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n", argv[0], hostname,
		d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);

	/* initialize global variables and attach to daemon SHM	*/
	Init_Mana (argc, argv);

	Dk = 99;

	UtoL (argv[1], 3);

	if (memcmp (argv[1], "all", 3) != 0)
	{
		Dk = argv[1][1] - 'a';
		if (Dk < 0 || Dk > 25)
			exit (FAIL);
	}

	if (argc == 2) 
		ProcFlag = FileFlag = ON;
	else if (argc == 3)
	{
		if (memcmp (argv[2], "proc", 4) == 0)
			ProcFlag = ON;
		else if (memcmp (argv[2], "file", 4) == 0)
			FileFlag = ON;
		else
		{
			sprintf (FileName, "%s", argv[2]);
			ProcFlag = FileFlag = ON;
		}
	}

	if (Dk == 99)
	{
		for (dk = 0; dk < SHM_MAX_SUB; dk ++)
		{
			if (INFO(dk).process_id[0] == 0)
				continue;

			Display_Count (dk);
		}
	}
	else
	{
		if (INFO(Dk).process_id[0] == 0)
		{
			printf ("%c%c daemon not registered !!!\n",
				_System_Name[0], Dk + 'A');
			exit (FAIL);
		}

		Display_Count (Dk);
	}

	exit (OK);
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	Display_Count (int dk)
/*----------------------------------------------------------------------*/
{
	int		pk, fk;
	char	line[84], aptype[12], info[44];

	if (DAEMON(dk).p_count == 0 && DAEMON(dk).f_count == 0)
		return;

	memset (line, 0, sizeof (line));
	memset (line, '-', 80);

	printf ("%s\n", line);
	printf ("########## %c%c [%s] ##########\n",
		_System_Name[0], dk + 'A', DAEMON(dk).process_info);

	if (ProcFlag == ON)
	{
		printf ("%s\n", line);
		puts ("Process      IfSeq  Process      IfSeq  Process      IfSeq  Process      IfSeq");
		printf ("%s\n", line);

		for (pk = 0; pk < DAEMON(dk).p_count; pk ++) 
		{
			if (pk % 4 == 0)
				printf ("%-10.10s %7d",
					PROC(dk,pk).process_id, PROC(dk,pk).if_seq);
			else if (pk % 4 == 3)
				printf ("  %-10.10s %7d\n",
					PROC(dk,pk).process_id, PROC(dk,pk).if_seq);
			else
				printf ("  %-10.10s %7d",
					PROC(dk,pk).process_id, PROC(dk,pk).if_seq);
		}

		if (pk % 4 != 0)
			printf ("\n");
	}

	if (FileFlag == ON)
	{
		printf ("%s\n", line);
		puts ("Data SHM        W1      SM      R1      R2      R3 Info");
		printf ("%s\n", line);

		for (fk = 0; fk < DAEMON(dk).d_count; fk ++) 
		{
			if (FileName[0] != '\0' && memcmp (DSHM(dk,fk).data_name,
				FileName, strlen (FileName)) != 0)
				continue;

			memset (info, 0, sizeof (info));
			memcpy (info, DSHM(dk,fk).info, sizeof (DSHM(dk,fk).info));

			if (Chk_Korean (info, 28) == -1)
				info[27] = ' ';

			if (dk + 'A' != 'Z')
			{
				printf ("%.10s %7d %7d %7d%c%7d %7d %-28.28s\n",
					DSHM(dk,fk).data_name, DSHM(dk,fk).w_cnt[0],
					DSHM(dk,fk).sm_r_cnt, DSHM(dk,fk).r_cnt[0],
					DSHM(dk,fk).w_cnt[0] == DSHM(dk,fk).r_cnt[0] ? ' ' : '*',
					DSHM(dk,fk).r_cnt[1], DSHM(dk,fk).r_cnt[2], info);
			}
		}

		printf ("%s\n", line);
		puts ("FileName        W1      R1      R2      R3 Info");
		printf ("%s\n", line);

		for (fk = 0; fk < DAEMON(dk).f_count; fk ++) 
		{
			if (FileName[0] != '\0' && memcmp (FILEM(dk,fk).file_name,
				FileName, strlen (FileName)) != 0)
				continue;

			memset (info, 0, sizeof (info));
			memcpy (info, FILEM(dk,fk).file_info,
				sizeof (FILEM(dk,fk).file_info));

			if (Chk_Korean (info, 36) == -1)
				info[35] = ' ';

			if (dk + 'A' != 'Z')
			{
				printf ("%.10s %7d %7d%c%7d %7d %-36.36s\n",
					FILEM(dk,fk).file_name, FILEM(dk,fk).w_cnt[0],
					FILEM(dk,fk).r_cnt[0],
					FILEM(dk,fk).w_cnt[0] == FILEM(dk,fk).r_cnt[0] ? ' ' : '*',
					FILEM(dk,fk).r_cnt[1], FILEM(dk,fk).r_cnt[2], info);
			}
		}
	}

	return;
} 	/* End of Display_Count ()	*/

/*************************************************************************
	End of Program (px_chkcnt.c)
*************************************************************************/
