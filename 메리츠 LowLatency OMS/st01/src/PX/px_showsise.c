#define		_GLOBAL
/*------------------------------------------------------------------------
#	File	: px_showsise.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

char    *Get_DateTime (char *);
KS_EXPCODE	Key;
KS_LONGCODE	LKey;

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     i, j, rt, max;
	int		mk_gbn, acc_seq, curr_cnt, tot_cnt;
	char	d_time[20], wbuf[1024];

	if (argc != 3 && argc != 4 && argc != 5)
	{
		printf ("=============================================\n");
		printf (" Ex) px_showsise_mp 1 A0					  \n");
		printf (" Ex) px_showsise_mp miche 5(mk_gbn) 0(ac_seq)\n");
		printf (" Ex) px_showsise_mp 5 Search KR7005930003    \n");
		printf (" Ex) px_showsise_mp 5 700 100				  \n");
		printf (" Ex) px_showsise_mp 5 900 1				  \n");
		printf ("---------------------------------------------\n");
		printf ("     Market Gbn : 1		/* Futures   	*/\n");
		printf ("				   2		/* Options   	*/\n");
		printf ("				   3		/* S Futures 	*/\n");
		printf ("				   4		/* S Options 	*/\n");
		printf ("				   5		/* Kospi	 	*/\n");
		printf ("				   6		/* Kosdaq	 	*/\n");
		printf ("				   8		/* KRX300	 	*/\n");
		printf ("				   9		/* Kosdaq150 F	*/\n");
		printf ("				  10		/* Kosdaq150 O	*/\n");
		printf ("				  20		/* Jisu			*/\n");
		printf ("				  31		/* CME			*/\n");
		printf ("---------------------------------------------\n");
		printf (" TR Gbn : CURR, A0, A3, G7, B7, M4, V1, S_SISE, KEY, Search... \n");
		printf ("=============================================\n");
		exit (FAIL);
	}

    Sub_SHM ();
    Mem_SHM (1, 0);
    Sise_SHM ();

	if (memcmp (argv[1], "1", 1) == 0)	// Futures
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_Futures[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_JF, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0 rt[%d][%s]\n", rt, Shm_Futures[rt].A0.tr_gbn);
				printf ("Key rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].F_Key[rt].idx, Shm_Item[0].F_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_Futures[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Futures[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Futures[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Futures[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Futures[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Futures[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].F_Key[i].idx, Shm_Item[0].F_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[1][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "2", 1) == 0 && strlen(argv[1]) == 1)	// Options
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_Options[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_JO, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_Options[i].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].O_Key[rt].idx, Shm_Item[0].O_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_Options[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Options[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Options[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Options[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Options[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Options[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].O_Key[i].idx, Shm_Item[0].O_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[2][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "3", 1) == 0 && strlen(argv[1]) == 1)	// SFutures
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_SFutures[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_SF, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_SFutures[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].SF_Key[rt].idx, Shm_Item[0].SF_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_SFutures[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SFutures[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SFutures[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SFutures[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SFutures[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SFutures[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].SF_Key[i].idx, Shm_Item[0].SF_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[3][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "4", 1) == 0)	// SOption
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_SOptions[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_SO, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_SOptions[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].SO_Key[rt].idx, Shm_Item[0].SO_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_SOptions[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SOptions[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SOptions[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SOptions[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SOptions[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SOptions[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].SO_Key[i].idx, Shm_Item[0].SO_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[3][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "5", 1) == 0)		// KOSPI
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n", argv[1], Shm_Stock[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_Stock[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].S_Key[rt].idx, Shm_Item[0].S_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_Stock[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Stock[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Stock[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Stock[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].S_Key[i].idx, Shm_Item[0].S_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[5][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "6", 1) == 0)		// Kosdaq
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n", argv[1], Shm_Kosdaq[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_KOSDAQ, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_Kosdaq[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].K_Key[rt].idx, Shm_Item[0].K_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_Kosdaq[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Kosdaq[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Kosdaq[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_Kosdaq[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].K_Key[i].idx, Shm_Item[0].K_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[6][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "8", 1) == 0)	// KRX300
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_K300[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_K300, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_K300[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].K300_Key[rt].idx, Shm_Item[0].K300_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_K300[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K300[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K300[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K300[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K300[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K300[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].K300_Key[i].idx, Shm_Item[0].K300_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[8][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "9", 1) == 0 && strlen(argv[1]) == 1)	// Kosdaq150 Futures
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_K150F[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_K150F, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_K150F[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].K150F_Key[rt].idx, Shm_Item[0].K150F_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_K150F[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150F[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150F[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150F[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150F[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150F[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].K150F_Key[i].idx, Shm_Item[0].K150F_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[9][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "10", 1) == 0)	// Kosdaq150 Options
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_K150O[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_K150O, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_K150O[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].K150O_Key[rt].idx, Shm_Item[0].K150O_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_K150O[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150O[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150O[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150O[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150O[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_K150O[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].K150O_Key[i].idx, Shm_Item[0].K150O_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[10][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "11", 1) == 0)	// 미니선물
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_MF[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_MF, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_MF[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].MF_Key[rt].idx, Shm_Item[0].MF_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_MF[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MF[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MF[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MF[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MF[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MF[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].MF_Key[i].idx, Shm_Item[0].MF_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[11][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "12", 1) == 0)	// 미니옵션
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_MO[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&Key, 0, sizeof (Key));
			sprintf (Key.expcode, "%-12.12s", argv[3]);
			rt = 0;
			rt = Key_Search (MK_MO, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_MO[rt].A0.tr_gbn);
				printf ("KEY rt[%d] idx[%d] code[%12.12s]\n",
					rt, Shm_Item[0].MO_Key[rt].idx, Shm_Item[0].MO_Key[rt].expcode);
			}
			else
				printf ("Not Found ItemCode[%12.12s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i <= Shm_MO[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MO[i].A0.tr_gbn);
				else
				if (memcmp (argv[2], "A3", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MO[i].A3.tr_gbn);
				else
				if (memcmp (argv[2], "G7", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MO[i].G7.tr_gbn);
				else
				if (memcmp (argv[2], "B6", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MO[i].B6.tr_gbn);
				else
				if (memcmp (argv[2], "V1", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_MO[i].V1.tr_gbn);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%12.12s]\n",
						i, Shm_Item[0].MO_Key[i].idx, Shm_Item[0].MO_Key[i].expcode);
				else
				if (memcmp (argv[2], "S_SISE", 6) == 0)
					printf ("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[12][i].m_item_cd);
			}
		}
	}
	else
	if (memcmp (argv[1], "20", 2) == 0)
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n", argv[1], Shm_Jisu[0].total_item_cnt);

		for (i = 0; i < Shm_Jisu[0].total_item_cnt; i++)
		{
			if (memcmp (argv[2], "KEY", 3) == 0)
				printf ("i[%d] idx[%d] code[%12.12s]\n",
					i, Shm_Item[0].J_Key[i].idx, Shm_Item[0].J_Key[i].expcode);
		}
	}
	else
	if (memcmp (argv[1], "miche", 5) == 0)
	{
		curr_cnt = 0;
		mk_gbn   = AtoIf (argv[2], strlen(argv[2]));
		acc_seq  = AtoIf (argv[3], strlen(argv[3]));
		tot_cnt  = Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq];
		printf ("Show miche Market_Gbn[%d] Acc_Seq[%d] total_miche_cnt[%d]\n", mk_gbn, acc_seq, tot_cnt);

		for (i = 0; i < MAX_MICHE; i++)
		{
			if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt > 0)
			{
				curr_cnt++;
				printf ("i[%d] Jan_Cnt[%d] Mk_gbn[%d] Item_Seq[%d]\n",
					i, Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Mk_gbn,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq);
				printf ("       Item_Cd[%12.12s] OrderNo[%10.10s] OriginalOrderNo[%10.10s] OrderFlag[%1.1s] TradeFlag[%1.1s] Order_Cnt[%8.8s] Order_Price[%9.9s] MembershipItem[%30.30s]\n",
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Cd,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OriginalOrderNo,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderFlag,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,
					Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price,
					&Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem[30]);
			}

			if (curr_cnt >= tot_cnt)
				break;
		}
	}
	else
	if (memcmp (argv[1], "31", 2) == 0)	// CME
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_CME[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&LKey, 0, sizeof (LKey));
			sprintf (LKey.longcode, "%-6.6s              ", argv[3]);
			rt = 0;
			rt = Key_Search (MK_CME, KEY_LONGCODE, (char *)&LKey);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_CME[rt].A0.type);
				printf ("Key rt[%d] idx[%d] code[%20.20s]\n",
					rt, Shm_Item[0].CME_Key[rt].idx, Shm_Item[0].CME_Key[rt].longcode);
			}
			else
				printf ("Not Found ItemCode[%s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i < Shm_CME[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_CME[i].A0.type);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%20.20s]\n",
						i, Shm_Item[0].CME_Key[i].idx, Shm_Item[0].CME_Key[i].longcode);
			}
		}
	}
	else
	if (memcmp (argv[1], "33", 2) == 0)	// SGX
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_SGX[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&LKey, 0, sizeof (LKey));
			sprintf (LKey.longcode, "%-6.6s              ", argv[3]);
			rt = 0;
			rt = Key_Search (MK_SGX, KEY_LONGCODE, (char *)&LKey);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_SGX[rt].A0.type);
				printf ("Key rt[%d] idx[%d] code[%20.20s]\n",
					rt, Shm_Item[0].SGX_Key[rt].idx, Shm_Item[0].SGX_Key[rt].longcode);
			}
			else
				printf ("Not Found ItemCode[%s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i < Shm_SGX[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_SGX[i].A0.type);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%20.20s]\n",
						i, Shm_Item[0].SGX_Key[i].idx, Shm_Item[0].SGX_Key[i].longcode);
			}
		}
	}
	else
	if (memcmp (argv[1], "34", 2) == 0)	// ERX
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_ERX[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&LKey, 0, sizeof (LKey));
			sprintf (LKey.longcode, "%-6.6s              ", argv[3]);
			rt = 0;
			rt = Key_Search (MK_ERX, KEY_LONGCODE, (char *)&LKey);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_ERX[rt].A0.type);
				printf ("Key rt[%d] idx[%d] code[%20.20s]\n",
					rt, Shm_Item[0].ERX_Key[rt].idx, Shm_Item[0].ERX_Key[rt].longcode);
			}
			else
				printf ("Not Found ItemCode[%s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i < Shm_ERX[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_ERX[i].A0.type);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%20.20s]\n",
						i, Shm_Item[0].ERX_Key[i].idx, Shm_Item[0].ERX_Key[i].longcode);
			}
		}
	}
	else
	if (memcmp (argv[1], "35", 2) == 0)	// HKE
	{
		printf ("Show Sise Market_Gbn[%s] total_item_cnt[%d]\n",
			argv[1], Shm_HKE[0].total_item_cnt);

		if (memcmp (argv[2], "Search", 6) == 0)
		{
			memset (&LKey, 0, sizeof (LKey));
			sprintf (LKey.longcode, "%-6.6s              ", argv[3]);
			rt = 0;
			rt = Key_Search (MK_HKE, KEY_LONGCODE, (char *)&LKey);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_HKE[rt].A0.type);
				printf ("Key rt[%d] idx[%d] code[%20.20s]\n",
					rt, Shm_Item[0].HKE_Key[rt].idx, Shm_Item[0].HKE_Key[rt].longcode);
			}
			else
				printf ("Not Found ItemCode[%s]\n", argv[3]);
		}
		else
		{
			for (i = 0; i < Shm_HKE[0].total_item_cnt; i++)
			{
				if (memcmp (argv[2], "A0", 2) == 0)
					printf ("i[%d][%s]\n", i, Shm_HKE[i].A0.type);
				else
				if (memcmp (argv[2], "KEY", 3) == 0)
					printf ("i[%d] idx[%d] code[%20.20s]\n",
						i, Shm_Item[0].HKE_Key[i].idx, Shm_Item[0].HKE_Key[i].longcode);
			}
		}
	}
	else
	if (memcmp (argv[1], "700", 3) == 0)			// 700100 View
	{
		if (memcmp (argv[2], "100", 3) == 0)		// 700100 View
		{
			for (i = 0; i < MAX_AUTO_PROC; i++)
			{
				printf ("i[%02d] run_gbn[%1.1s] ApType[%5.5s] Str_No[%4.4s] Item_Code[%12.12s]\n",
					Shm_Risk[0].Auto_Stat[i].Run_Gbn, Shm_Risk[0].Auto_Stat[i].ApType,
					Shm_Risk[0].Auto_Stat[i].Str_No, Shm_Risk[0].Auto_Stat[i].Item_Code);
			}
		}
	}
	else
	if (memcmp (argv[1], "900", 3) == 0)			// 900 Set
	{
		if (memcmp (argv[2], "1", 1) == 0)			// 900001 Set
		{
			for (i = 0; i < ACC_NO_CNT; i++)
				printf ("i[%02d] acc_no[%12.12s] mm_gbn[%1.1s]",
					Shm_Risk[0].Mst_Acc[i].acc_no, Shm_Risk[0].Mst_Acc[i].mk_gbn);
		}
		else if (memcmp (argv[2], "2", 1) == 0)		// 900002 Set
		{
			for (i = 0; i < ACC_NO_CNT; i++)
			{
				for (j = 0; j < RISK_MK_CNT; j++)
				{
					printf ("acc[%02d] RiskMk[%02d] qty_gbn[%1.1s] qty[%08ld] money_gbn[%1.1s] money[%12.0f] tick_gbn[%1.1s] tick[%08ld]\n",
						i, j, Shm_Risk[0].O_M_Fund[j][i].qty_gbn,
						Shm_Risk[0].O_M_Fund[j][i].qty,
						Shm_Risk[0].O_M_Fund[j][i].money_gbn,
						Shm_Risk[0].O_M_Fund[j][i].money,
						Shm_Risk[0].O_M_Fund[j][i].tick_gbn,
						Shm_Risk[0].O_M_Fund[j][i].tick);
					printf ("          do_cnt_gbn[%1.1s] do_cnt[%08d] do_money_gbn[%1.1s] do_money[%12.0f] su_cnt_gbn[%1.1s] su_cnt[%08d] su_money_gbn[%1.1s] su_money[%12.0f]\n",
						Shm_Risk[0].T_M_Fund[j][i].do_cnt_gbn,
						Shm_Risk[0].T_M_Fund[j][i].do_cnt,
						Shm_Risk[0].T_M_Fund[j][i].do_money_gbn,
						Shm_Risk[0].T_M_Fund[j][i].do_money,
						Shm_Risk[0].T_M_Fund[j][i].su_cnt_gbn,
						Shm_Risk[0].T_M_Fund[j][i].su_cnt,
						Shm_Risk[0].T_M_Fund[j][i].su_money_gbn,
						Shm_Risk[0].T_M_Fund[j][i].su_money);
				}
			}
		}
		else if (memcmp (argv[2], "3", 1) == 0)		// 900003 Set
		{
			if (argc != 5)
			{
				printf (" Ex) px_showsise_mp 900 3 5(mk_gbn) 1(acc_seq)\n");
				exit(0);
			}

			mk_gbn   = AtoIf (argv[3], strlen(argv[3]));
			acc_seq  = AtoIf (argv[4], strlen(argv[4]));

#if 0
			if (mk_gbn == 1)	max = SHM_MAX_FUTURES;
			if (mk_gbn == 2)	max = SHM_MAX_OPTIONS;
			if (mk_gbn == 3)	max = SHM_MAX_S_FUTURES;
			if (mk_gbn == 4)	max = SHM_MAX_S_OPTIONS;
			if (mk_gbn == 5)	max = SHM_MAX_STOCK;
			if (mk_gbn == 6)	max = SHM_MAX_KOSDAQ;
			if (mk_gbn == 8)	max = SHM_MAX_K300;
			if (mk_gbn == 9)	max = SHM_MAX_K150F;
			if (mk_gbn == 10)	max = SHM_MAX_K150O;
			if (mk_gbn == 11)	max = SHM_MAX_MF;
			if (mk_gbn == 12)	max = SHM_MAX_MO;
				
			for (i = 0; i <= max; i++)
#endif
			printf ("mk_gbn[%d] acc_seq[%d]\n", mk_gbn, acc_seq);
			for (i = 0; i <= Shm_Risk[0].Max_Seq[mk_gbn]; i++)
			{
				if (Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_getcnt != 0)
					printf ("종목코드[%12.12s] 잔고[%ld] 금액[%ld] 차입[%ld] 매수체누[%ld] 매도체누[%ld]\n",
						Shm_Risk[0].S_Sise[mk_gbn][i].m_item_cd,
						Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_getcnt,
						Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_getmoney,
						Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_borrow_cnt,
						Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_totsu_che,
						Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_totdo_che);
			}
		}
		else if (memcmp (argv[2], "4", 1) == 0)		// 900004 Set
		{
			if (argc != 4)
			{
				printf (" Ex) px_showsise_mp 900 4 5(mk_gbn)\n");
				exit(0);
			}

			mk_gbn   = AtoIf (argv[3], strlen(argv[3]));

			printf ("mk_gbn[%d]\n", mk_gbn);

#if 0
			if (mk_gbn == 1)	max = SHM_MAX_FUTURES;
			if (mk_gbn == 2)	max = SHM_MAX_OPTIONS;
			if (mk_gbn == 3)	max = SHM_MAX_S_FUTURES;
			if (mk_gbn == 4)	max = SHM_MAX_S_OPTIONS;
			if (mk_gbn == 5)	max = SHM_MAX_STOCK;
			if (mk_gbn == 6)	max = SHM_MAX_KOSDAQ;
			if (mk_gbn == 8)	max = SHM_MAX_K300;
			if (mk_gbn == 9)	max = SHM_MAX_K150F;
			if (mk_gbn == 10)	max = SHM_MAX_K150O;
			if (mk_gbn == 11)	max = SHM_MAX_MF;
			if (mk_gbn == 12)	max = SHM_MAX_MO;
				
			for (i = 0; i <= max; i++)
#endif
			for (i = 0; i <= Shm_Risk[0].Max_Seq[mk_gbn]; i++)
			{
				if (Shm_Risk[0].S_Sise[mk_gbn][i].dont_trade > 0)
					printf ("내부매매가능여부[%d]\n", Shm_Risk[0].S_Sise[mk_gbn][i].dont_trade);
			}
		}
		else if (memcmp (argv[2], "5", 1) == 0)		// 900005 Set
		{
			printf ("기초자산코드 [%f]\n", Shm_Risk[0].indv_rate[8]);
		}
		else if (memcmp (argv[2], "6", 1) == 0)		// 900006 Set
		{
			printf ("비지니스Day[%8.8s]\n", Shm_Risk[0].business_day);
		}
		else if (memcmp (argv[2], "7", 1) == 0)		// 900007 Set
		{
			printf ("CD금리[%f]\n", Shm_Risk[0].cd_rate);
		}
	}
	else
	if (memcmp (argv[1], "Batch", 5) == 0)		// 900 Set
	{
		for (i = 0; i < 7; i++)
			printf ("i[%02d] Wcnt[%d] Rcnt[%d]\n", i, Shm_Risk[0].Batch_Cnt[i].W_cnt, Shm_Risk[0].Batch_Cnt[i].R_cnt);
	}

}   /* End of main ()   */

/*************************************************************************
	End of Program (px_getatm.c)
*************************************************************************/

