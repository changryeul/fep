#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 15시 04분에 시세정보를 읽어서
			  행사가1,2/전일합성선물값/
			  전일콜보정값/전일풋보정값/
			  전일콜atm이론값/전일풋atm이론값
			  File에 보관. 실행은 cron으로
#	File	: px_getatm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

char    *Get_DateTime (char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     i, h_basis;
    int     strike_price01, strike_price02, fu_str_p;
    int     futures_synthetic_price;
    int     call_revision, put_revision;
    int     call_atm, put_atm;

    int     call01_seq, call02_seq, put01_seq, put02_seq;
    char    path[100], buf[2048], wbuf[100];
    FILE    *r_fp;

    Sub_SHM ();
    Mem_SHM (1, 0);
    Sise_SHM ();

    memset(path, 0, sizeof(path));
    sprintf (path, "%s%s1/utl/overfile/overatm.dat",
        (char *)getenv("_FEP_HOME"), (char *)getenv("_FEP_SYSTEM"));

    if ((r_fp = fopen (path, "r")) == NULL)
    {
        printf ("\033[5mcannot open:[%s]\033[0m\n", path);
        exit (1);
    }

    memset(buf, 0, sizeof(buf));

    fgets (buf, sizeof (buf), r_fp);

    if (AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5) <= 0)
    {
        printf ("%s", buf);
        fclose (r_fp);
        exit (0);
    }

    sprintf (path, "%s%s1/utl/overfile", (char *)getenv("_FEP_HOME"),
                        (char *)getenv("_FEP_SYSTEM"));

    /* 행사가 1, 2 결정하기 */
    fu_str_p = AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5);
    strike_price01 = fu_str_p / 250 * 250;
    strike_price02 = strike_price01 + 250;

    /* 행사가 1, 2의 Call, put seq알아두기 */
    call01_seq = call02_seq = put01_seq = put02_seq = 999;
    for (i = 0; i < AtoIf(Shm_Options[0].Options_A0.cnt, 5); i++)
    {
        if (memcmp (Shm_Options[i].Options_A0.gubun_code, "1", 1) == 0)
        {
            if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "2", 1) == 0)   &&
                (AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price01))
                call01_seq = i;
            else
            if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "3", 1) == 0)   &&
                (AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price01))
                put01_seq = i;
            else
            if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "2", 1) == 0)   &&
                (AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price02))
                call02_seq = i;
            else
            if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "3", 1) == 0)   &&
                (AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price02))
                put02_seq = i;
        }
    }

    if (call01_seq == 999 || call02_seq == 999 || put01_seq == 999 || put02_seq == 999)
    {
        Log (USR_ERROR, "c1[%d] c2[%d] p1[%d] p2[%d]",
            call01_seq, call02_seq, put01_seq, put02_seq);
        fclose (r_fp);
        exit (OK);
    }

    /* 합성선물 가격 */
    futures_synthetic_price =
    ((strike_price01 + AtoIf(Shm_Options[call01_seq].Options_CURR.crprc, 5) -
      AtoIf(Shm_Options[put01_seq].Options_CURR.crprc, 5))  +
     (strike_price02 + AtoIf(Shm_Options[call02_seq].Options_CURR.crprc, 5) -
      AtoIf(Shm_Options[put02_seq].Options_CURR.crprc, 5))  ) / 2;

    /* 합성 베이시스 */
    h_basis = fu_str_p - futures_synthetic_price;

    /* Call 보정값(*100) */
    call_revision = (AtoIf(Shm_Options[call02_seq].Options_CURR.crprc, 5) -
                     AtoIf(Shm_Options[call01_seq].Options_CURR.crprc, 5)) * 10 / 25 ;

    /* Put 보정값(*100) */
    put_revision = (AtoIf(Shm_Options[put02_seq].Options_CURR.crprc, 5) -
                    AtoIf(Shm_Options[put01_seq].Options_CURR.crprc, 5)) * 10 / 25 ;

    /* Call ATM 이론값 */
    call_atm = AtoIf(Shm_Options[call01_seq].Options_CURR.crprc, 5) +
        (((futures_synthetic_price - strike_price01) * call_revision) / 100);

    /* Put ATM 이론값 */
    put_atm = AtoIf(Shm_Options[put01_seq].Options_CURR.crprc, 5) +
        (((futures_synthetic_price - strike_price01) * put_revision) / 100);

    memset(wbuf,    0,  sizeof(wbuf));
    memset(buf,     0,  sizeof(buf));
    /* 행사가1(10) 행사가2(10) 합성선물(10) 합성베이시스(10) Call보정값(10) Put보정값(10)
		Call이론값(10) Put이론값(10) */

    printf("%010d %010d %010d %010d %010d %010d %010d %010d\n",
        strike_price01, strike_price02, futures_synthetic_price, h_basis,
        call_revision, put_revision, call_atm, put_atm);

/*
    printf ("s_p1[%d] s_p2[%d] c1[%d] c2[%d] p1[%d] p2[%d] f_p[%d]\n",
        strike_price01, strike_price02, call01_seq, call02_seq, put01_seq, put02_seq,
        futures_synthetic_price);

    printf ("c1_p[%d] c2_p[%d] p1_p[%d] p2_p[%d]\n",
        AtoIf(Shm_Options[call01_seq].Options_CURR.crprc, 5),
        AtoIf(Shm_Options[call02_seq].Options_CURR.crprc, 5),
        AtoIf(Shm_Options[put01_seq].Options_CURR.crprc, 5),
        AtoIf(Shm_Options[put02_seq].Options_CURR.crprc, 5));

    printf ("C1[%8.8s] C2[%8.8s] P1[%8.8s] P2[%8.8s]\n",
        Shm_Options[call01_seq].Options_CURR.item_code+3,
        Shm_Options[call02_seq].Options_CURR.item_code+3,
        Shm_Options[put01_seq].Options_CURR.item_code+3,
        Shm_Options[put02_seq].Options_CURR.item_code+3);

    printf ("행1[%d] 행2[%d] 합_선[%d] C_보[%d] P_보[%d] C_Atm[%d] P_Atm[%d]\n",
        strike_price01, strike_price02, futures_synthetic_price,
        call_revision, put_revision, call_atm, put_atm);
*/

    fclose (r_fp);

}   /* End of main ()   */

/*************************************************************************
	End of Program (px_getatm.c)
*************************************************************************/

