/*------------------------------------------------------------------------
#   System  : HATS System
#   Module  : report order transaction time (total, krx response, hats)
#   File    : px_chkche.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     p_cnt, checnt;
    char    sub[4], yn[4], buf[5120];
    char    name[20], path[128], day[20], d_time[16];
    FILE    *fp;

    if ((argc != 1 && argc != 2 && argc != 3) ||
            (argc == 2 && argv[1][0] == '?')) {
        printf("==========================================================\n");
        printf("Usage: %s <file> <date> <check from> <check to>\n\n", argv[0]);
        printf("  e.g. 1) %s\n", argv[0]);
        printf("       2) %s 31\n", argv[0]);
        printf("       3) %s 31 20100201\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    p_cnt = checnt = 0;

    memset(sub, 0, sizeof (sub));
    memset(yn, 0, sizeof (yn));
    memset(buf, 0, sizeof (buf));
    memset(name, 0, sizeof (name));
    memset(path, 0, sizeof (path));
    memset(day, 0, sizeof (day));

    if (argc == 2 || argc == 3) {
        memcpy(name, argv[1], strlen(argv[1]));

        if (argc == 3) {
            memcpy(day, argv[2], strlen(argv[2]));
        }
        else
            memcpy(day, "00000000", 8);
    }

    {
        char *env_pdat = getenv("_P_DAT");
        if (env_pdat == NULL) {
            printf("ERROR: getenv(_P_DAT) is NULL\n");
            exit(1);
        }
        sprintf(path, "%s/PA/%s/pa_1401_mp", env_pdat, day);
    }

    if ((fp = fopen(path, "r")) == NULL) {
        printf("\033[5mcannot open:[%s]\033[0m\n", path);
        exit(1);
    }

    Get_DateTime(d_time);
    printf("[%.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n",
            d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);

    printf("================================================================================\n");
    printf("HATS AUTO NEW CHE\n");
    printf(" Che_NO       Order Recv      Che_Price  CheCnt  Order_No  Code\n");
    printf("--------------------------------------------------------------------------------\n");

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        /* �����ڵ��ֹ�+�ű��ֹ�: ȸ���翵��(DOPxxA), Ʈ������ڵ�(TCHODR10001) */
        if ((memcmp(buf+177, "TTRTDP", 6) != 0) ||
                (memcmp(buf+345,"DOP", 3) != 0) ||
                (memcmp(buf+350,"A", 1) != 0) ) {
            memset(buf, 0, sizeof (buf));
            continue;
        }

        /* park */
        if (    (memcmp((buf+50), "0800", 4) <= 0)
                ||  (memcmp((buf+348), name, 2) != 0)  )
        continue;

        p_cnt ++;

        checnt += AtoIf(buf+258, 5);

        printf("%11.11s %.2s:%.2s:%.2s.%.6s  %.11s  %.5s  %.6s   %.8s\n",
                buf+231, buf+49, buf+51, buf+53, buf+55,
                buf+242, buf+258, buf+199, buf+219);
        memset(buf, 0, sizeof (buf));
    }

    printf("================================================================================\n");
    printf("%d packets\n", p_cnt);
    printf("Total Che Cnt [%d]\n\n", checnt);
    printf("\n[%s]\n", path);

    printf("================================================================================\n");
    fclose(fp);

    exit(0);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_chkche.c)
*************************************************************************/
