/*------------------------------------------------------------------------
#   System  : HATS System
#   Module  : report order transaction time (total, krx response, hats)
#   File    : px_autoju_chk.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     p_cnt, orcnt;
    double  min_tot, min_krx, min_fep, max_tot, max_krx, max_fep;
    double  avg_tot, avg_krx, avg_fep, recv_sec, send_sec;
    double  tot_tm, krx_tm, fep_tm, start_tm, end_tm;
    char    sub[4], yn[4], chk_from[12], chk_to[12], buf[5120];
    char    name[20], path[128], day[20], d_time[16];
    char    start_flag;
    FILE    *fp;

    if ((argc != 1 && argc != 2 && argc != 5) ||
            (argc == 2 && argv[1][0] == '?')) {
        printf("==========================================================\n");
        printf("Usage: %s <file> <date> <check from> <check to>\n\n", argv[0]);
        printf("  e.g. 1) %s\n", argv[0]);
        printf("       2) %s 31\n", argv[0]);
        printf("       3) %s 31 20100201 092030 133000\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    p_cnt = 0;
    min_tot = min_krx = min_fep = max_tot = max_krx = max_fep = 0;
    avg_tot = avg_krx = avg_fep = orcnt = 0;
    start_flag = 0;

    memset(sub, 0, sizeof (sub));
    memset(yn, 0, sizeof (yn));
    memset(chk_from, 0, sizeof (chk_from));
    memset(chk_to, 0, sizeof (chk_to));
    memset(buf, 0, sizeof (buf));
    memset(name, 0, sizeof (name));
    memset(path, 0, sizeof (path));
    memset(day, 0, sizeof (day));

    if (argc == 2 || argc == 5) {
        memcpy(name, argv[1], strlen(argv[1]));

        if (argc == 5) {
            memcpy(day, argv[2], strlen(argv[2]));
            memcpy(chk_from, argv[3], strlen(argv[3]));
            memcpy(chk_to, argv[4], strlen(argv[4]));
        }
        else
            memcpy(day, "00000000", 8);
    }
    else {
        printf("\033[1mlog file name(e.g. 31) ?\033[0m ");
        fflush(stdout);
        fgets(name, sizeof(name), stdin);
        if (name[0] == '\0')
            exit(1);
        name[strlen(name)-1] = '\0';

        printf("\033[1mdate(e.g. 00000000, 31, ...) ?\033[0m ");
        fflush(stdout);
        fgets(day, sizeof(day), stdin);
        if (day[0] == '\0')
            exit(1);
        day[strlen(day)-1] = '\0';

        printf("\033[1mset check range(from ~ to) (y/n) ?\033[0m ");
        fflush(stdout);
        fgets(yn, sizeof(yn), stdin);
        if (yn[0] == '\0')
            exit(1);
        yn[strlen(yn)-1] = '\0';

        if (yn[0] == 'y') {
            printf("\033[1mcheck from(hhmmss, e.g. 090530) ?\033[0m ");
            fflush(stdout);
            fgets(chk_from, sizeof(chk_from), stdin);
            if (chk_from[0] == '\0')
                exit(1);
            chk_from[strlen(chk_from)-1] = '\0';

            printf("\033[1mcheck to(hhmmss, e.g. 142050) ?\033[0m ");
            fflush(stdout);
            fgets(chk_to, sizeof(chk_to), stdin);
            if (chk_to[0] == '\0')
                exit(1);
            chk_to[strlen(chk_to)-1] = '\0';
        }
    }

    {
        char *env_jlog = getenv("_J_LOG");
        char *env_plog = getenv("_P_LOG");
        if (env_jlog == NULL || env_plog == NULL) {
            printf("ERROR: getenv(_J_LOG or _P_LOG) is NULL\n");
            exit(1);
        }
        if (memcmp(argv[0], "jx", 2) == 0)
            sprintf(path, "%s/JC/%s/jc_1251_ts", env_jlog, day);
        else
            sprintf(path, "%s/PB/%s/pb_1252_ts", env_plog, day);
    }

    if ((fp = fopen(path, "r")) == NULL) {
        printf("\033[5mcannot open:[%s]\033[0m\n", path);
        exit(1);
    }

    Get_DateTime(d_time);
    printf("[%.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n",
            d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);

    printf("================================================================================\n");
    printf("HATS AUTO NEW ORDER\n");
    printf("Line   Seq Cnt      Order Recv         KRX        FEP   Order No OrCnt     Code\n");
    printf("--------------------------------------------------------------------------------\n");

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        /* �����ڵ��ֹ�+�ű��ֹ�: ȸ���翵��(DOPxxA), Ʈ������ڵ�(TCHODR10001) */
        if ((memcmp(buf+62, "][", 2) != 0) ||
                (memcmp(buf+459,"DOP", 3) != 0) ||
                (memcmp(buf+464,"A", 1) != 0) ||
                (memcmp(buf+283,"TCHODR10001", 11) != 0)) {
            memset(buf, 0, sizeof (buf));
            continue;
        }

        /* park */
        if (    (memcmp((buf+50), "0800", 4) <= 0)
                ||  (memcmp((buf+462), name, 2) != 0)  )
        continue;

        recv_sec = AtoIf(buf+50, 2) * 60 * 60 + AtoIf(buf+52, 2) * 60 +
        AtoIf(buf+54, 2) + AtoIf(buf+56, 6) / 1000000.0;
        send_sec = AtoIf(buf+17, 2) * 60 * 60 + AtoIf(buf+20, 2) * 60 +
        AtoIf(buf+23, 2) + AtoIf(buf+26, 6) / 1000000.0;

        if (start_flag == 0) {
            start_tm = recv_sec;
            start_flag = 1;
        }

        if (chk_from[0] != '\0') {
            if (send_sec < AtoIf(chk_from, 2) * 60 * 60 +
                    AtoIf(chk_from+2, 2) * 60 + AtoIf(chk_from+4, 2))
            continue;
            else if (send_sec >= AtoIf(chk_to, 2) * 60 * 60 +
                    AtoIf(chk_to+2, 2) * 60 + AtoIf(chk_to+4, 2) + 1)
            break;
        }

        tot_tm = send_sec - recv_sec;
        krx_tm = AtoIf(buf+64, 3) + AtoIf(buf+68, 6) / 1000000.0;
        fep_tm = tot_tm - krx_tm;

        /* total    */
        if (min_tot == 0 || tot_tm < min_tot)
            min_tot = tot_tm;

        if (tot_tm > max_tot)
            max_tot = tot_tm;

        avg_tot += tot_tm;

        /* krx  */
        if (min_krx == 0 || krx_tm < min_krx)
            min_krx = krx_tm;

        if (krx_tm > max_krx)
            max_krx = krx_tm;

        avg_krx += krx_tm;

        /* fep  */
        if (min_fep == 0 || fep_tm < min_fep)
            min_fep = fep_tm;

        if (fep_tm > max_fep)
            max_fep = fep_tm;

        avg_fep += fep_tm;

        p_cnt ++;

        orcnt += AtoIf(buf+356, 5);

        printf("%2.2s %8.8s %2.2s  %.2s:%.2s:%.2s.%.6s  %010.06f %010.06f %.10s %.5s %.8s\n",
                buf+163, buf+114, buf+122, buf+50, buf+52, buf+54, buf+56,
                krx_tm, fep_tm, buf+302, buf+356, buf+328);
        memset(buf, 0, sizeof (buf));
    }

    end_tm = send_sec;

    if (p_cnt != 0) {
        avg_tot /= p_cnt;
        avg_krx /= p_cnt;
        avg_fep /= p_cnt;
    }

    printf("================================================================================\n");
    printf("%d packets, %06f sec elapsed\n", p_cnt, end_tm - start_tm);
    printf("Total Order Cnt [%d]\n\n", orcnt);
    printf("min: KRX = %010.06f\n", min_krx);
    printf("max: KRX = %010.06f\n", max_krx);
    printf("avg: KRX = %010.06f\n", avg_krx);
    printf("\n[%s]\n", path);

    if (chk_from[0] == '\0')
        printf("range: all\n");
    else
        printf("range: %s ~ %s\n", chk_from, chk_to);

    printf("================================================================================\n");
    fclose(fp);

    exit(0);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_autoju_chk.c)
*************************************************************************/
