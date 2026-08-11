#define		_GLOBAL
/*------------------------------------------------------------------------
#	File	: px_jisudat_p.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		i, idx, rt, len, R_Cnt, flag, start_time, sleep_time;
	char 	buf[5120];
	char    path[128], day[20], jong[10];
	FILE    *fp;

    if (argc != 3)
	{
		printf ("==========================================================\n");
		printf ("Usage: %s <date>\n\n", argv[0]);
		printf ("  e.g. 1) %s\n", argv[0]);
		printf ("       2) %s 20101212 201F1275\n", argv[0]);
		printf ("==========================================================\n");
		exit (FAIL);
	}

	memset (path, 0, sizeof (path));
	memset (day, 0, sizeof (day));
	memset (jong, 0, sizeof (jong));

	memcpy (day, argv[1], strlen (argv[1]));
	memcpy (jong, argv[2], strlen (argv[2]));

	if (memcmp (jong, "2", 1) == 0)
		sprintf (path, "%s/PA/%s/pa_7202_dd", (char *)getenv ("_P_DAT"), day);
	else if (memcmp (jong, "3", 1) == 0)
		sprintf (path, "%s/PA/%s/pa_7203_dd", (char *)getenv ("_P_DAT"), day);
	else
	{
		printf ("\033[5mcannot open:[%s]\033[0m\n", path);
		exit (1);
	}

	if ((fp = fopen (path, "r")) == NULL)
    {
        printf ("\033[5mcannot open:[%s]\033[0m\n", path);
        exit (1);
    }

    while (fgets (buf, sizeof (buf), fp) != NULL)
    {
        if (memcmp (buf+92, jong, 8) != 0)
        {
            memset (buf, 0, sizeof (buf));
            continue;
        }

        if (memcmp (buf+84, "A3", 2) == 0)
		{
			/* 시간(12) + 가격(5) + 체결수량(7) */
			printf ("A3 %12.12s %5.5s %7.7s\n", buf+49, buf+104, buf+109);
		}
        else if (memcmp (buf+84, "G7", 2) == 0)
		{
/* 시간(12),가격(5),체결수량(7),매수1호가(5),매도1호가(5),매수잔량(7*5),매도잔량(7*5),매수건수(4*5),매도건수(4*5)*/
			printf ("G7 %12.12s %5.5s %7.7s %5.5s %5.5s %7.7s %7.7s %7.7s %7.7s %7.7s %7.7s %7.7s %7.7s %7.7s %7.7s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s\n",
				buf+49,  buf+104, buf+109, buf+174, buf+241,
				buf+179, buf+191, buf+203, buf+215, buf+227,
				buf+246, buf+258, buf+270, buf+282, buf+294,
				buf+306, buf+310, buf+314, buf+318, buf+322,
				buf+331, buf+335, buf+339, buf+343, buf+347);
		}
        else if (memcmp (buf+84, "B6", 2) == 0)
		{
/* 시간(12),가격(5),체결수량(6),매수잔량(7*5),매도잔량(7*5),매수건수(4*5),매도건수(4*5)*/
			printf ("B6 %12.12s XXXXX XXXXXX %7.7s %7.7s %7.7s %7.7s %7.7s  %7.7s %7.7s %7.7s %7.7s %7.7s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s %4.4s\n",
					buf+49,  buf+118, buf+130, buf+142, buf+154, buf+166,
					buf+185, buf+197, buf+209, buf+221, buf+233,
					buf+245, buf+249, buf+253, buf+257, buf+261,
					buf+270, buf+274, buf+278, buf+282, buf+286);
		}
		else
        {
            memset (buf, 0, sizeof (buf));
            continue;
        }

        memset (buf, 0, sizeof (buf));
	}
	
	fclose (fp);

	return;
}	/* End of PX_JISUDAT ()	*/

/*************************************************************************
	End of program (px_jisudat_p.c)
*************************************************************************/ 
