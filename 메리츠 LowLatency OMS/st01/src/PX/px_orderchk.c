/*------------------------------------------------------------------------
#	System	: HATS System
#	Module	: report order transaction time (total, krx response, hats)
#	File	: px_orderchk.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		first_flag, chk_tm;
	double	gap_tm, fst_gap_tm, curr_sec, befor_sec;
	char	sub[4], yn[4], chk_from[12], chk_to[12], buf[1024];
	char	name[20], path[128], day[20], d_time[16], bbuf[1024];
	char	start_flag;
	FILE	*fp;

	if ((argc != 2 && argc != 4) ||
		(argc == 2 && argv[1][0] == '?'))
	{
		printf ("==============================================================\n");
		printf ("Usage: %s <file> <date> <check from> <check to>\n\n", argv[0]);
		printf ("  e.g. 1) %s\n", argv[0]);
		printf ("       2) %s pa_1201_mp\n", argv[0]);
		printf ("       3) %s pa_1201_mp 20100203 3000\n", argv[0]);
		printf ("==============================================================\n");
		exit (FAIL);
	}

	first_flag = 0;
	befor_sec = 0;
	curr_sec = 0;
	chk_tm = 0;

	memset (sub, 0, sizeof (sub));
	memset (yn, 0, sizeof (yn));
	memset (chk_from, 0, sizeof (chk_from));
	memset (chk_to, 0, sizeof (chk_to));
	memset (buf, 0, sizeof (buf));
	memset (bbuf, 0, sizeof(bbuf));
	memset (name, 0, sizeof (name));
	memset (path, 0, sizeof (path));
	memset (day, 0, sizeof (day));

	memcpy (name, argv[1], strlen (argv[1]));

	if (argc == 4)
	{
		memcpy (day, argv[2], strlen (argv[2]));
		chk_tm = AtoIf(argv[3], strlen (argv[3]));
	}
	else
	{
		memcpy (day, "00000000", 8);
		chk_tm = 3000;
	}

	memcpy (sub, name, 2);
	LtoU (sub, 2);

	sprintf (path, "%s/%s/%s/%s", (char *)getenv ("_P_DAT"), sub, day, name);

	if ((fp = fopen (path, "r")) == NULL)
	{
		printf ("\033[5mcannot open:[%s]\033[0m\n", path);
		exit (1);
	}

	Get_DateTime (d_time);
	printf ("[%.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n",
		d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);
	printf ("[%s]\n", path);
	printf ("================================================================================\n");
	printf ("--------------------------------------------------------------------------------\n");

	while (fgets (buf, sizeof (buf), fp) != NULL)
	{
/* park */
		if (memcmp ((buf+49), "080", 3) <= 0)
			continue;

		befor_sec = curr_sec;

		curr_sec = AtoIf (buf+49, 2) * 60 * 60 + AtoIf (buf+51, 2) * 60 +
			AtoIf (buf+53, 2) + AtoIf (buf+55, 6) / 1000000.0;

		if (befor_sec == 0)
			continue;

		gap_tm = curr_sec - befor_sec;
		if ( (gap_tm < (chk_tm / 1000000.0)) )
		{
			if (first_flag == 0)
			{
	printf ("--------------------------------------------------------------------------------\n");
				printf ("\n\t\tTIME:[%.12s]\n", 	bbuf+49);
				printf ("F Gbn[%.12s] Gap_Time[%06d] Order_No:[%.10s] OrCnt[%.5s] Code:[%.8s]\n", 
					bbuf+377, first_flag, bbuf+219, bbuf+273, bbuf+246); 
				printf ("\tData:[%89.89s]\n", 	bbuf+201);
				fst_gap_tm = 0;
			}

			printf ("  Gbn[%.12s] Gap_Time[%06.06f] Order_No:[%.10s] OrCnt[%.5s] Code:[%.8s]\n", 
				buf+377, gap_tm + fst_gap_tm, buf+219, buf+273, buf+246); 
			printf ("\tData:[%89.89s]\n", 	buf+201);
			
			if (first_flag == 0)
			{
				fst_gap_tm = gap_tm;
			}

			first_flag = 1;
		}
		else
			first_flag = 0;

		memcpy (bbuf, buf, strlen(buf));
		memset (buf, 0, sizeof (buf));
	}

	printf ("================================================================================\n");
	printf ("\n[%s]\n", path);
	fclose (fp);

	exit (0);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_orderchk.c)
*************************************************************************/
