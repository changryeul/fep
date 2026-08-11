#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change the read/write count of a file or data SHM
#   File    : px_setfcnt.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    file_name[12], rw_flag[4], cnt[12], sub[4], buf[128];
    int     dk, fk;
    FILE    *fp;

    if (argc != 1 && argc != 4) {
        printf("==========================================================\n");
        printf("[change the read/write count of a file or data SHM]\n\n");
        printf("Usage: %s <data name> <R/W flag(W1, SM, R1 ~ R9)> <count>\n\n", argv[0]);
        printf("  e.g. %s %cc_1101_ts R1 0\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(file_name, 0, sizeof (file_name));
    memset(rw_flag, 0, sizeof (rw_flag));
    memset(cnt, 0, sizeof (cnt));

    if (argc == 4) {
        memcpy(file_name, argv[1], strlen(argv[1]));
        memcpy(rw_flag, argv[2], strlen(argv[2]));
        memcpy(cnt, argv[3], strlen(argv[3]));
    }
    else {
        printf("\033[1mdata name(e.g. jc_1101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(file_name, sizeof(file_name), stdin);
        if (file_name[0] == '\0')
            exit(FAIL);

        printf("\033[1mread/write flag(W1, SM, R1 ~ R9) ?\033[0m ");
        fflush(stdout);
        fgets(rw_flag, sizeof(rw_flag), stdin);
        if (rw_flag[0] == '\0')
            exit(FAIL);

        printf("\033[1mcount ?\033[0m ");
        fflush(stdout);
        fgets(cnt, sizeof(cnt), stdin);
        if (cnt[0] == '\0')
            exit(FAIL);
    }

    if (strlen(file_name) != 10) {
        printf("ERROR:file name[%s]\n", file_name);
        exit(FAIL);
    }

    LtoU(rw_flag, 2);
    if (memcmp(rw_flag, "R1", 2) != 0 && memcmp(rw_flag, "R2", 2) != 0 &&
            memcmp(rw_flag, "R3", 2) != 0 && memcmp(rw_flag, "R4", 2) != 0 &&
            memcmp(rw_flag, "R5", 2) != 0 && memcmp(rw_flag, "R6", 2) != 0 &&
            memcmp(rw_flag, "R7", 2) != 0 && memcmp(rw_flag, "R8", 2) != 0 &&
            memcmp(rw_flag, "R9", 2) != 0 && memcmp(rw_flag, "SM", 2) != 0 &&
            memcmp(rw_flag, "W1", 2) != 0) {
        printf("ERROR:read/write flag[%s]\n", rw_flag);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    sprintf(sub, "%-2.2s", file_name);
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (fk = 0; fk < DAEMON(dk).d_count; fk ++) {
        if (memcmp(DSHM(dk,fk).data_name, file_name, 10) == 0) {
            if (rw_flag[0] == 'R') {
                if (rw_flag[1] == '1')
                    DSHM(dk,fk).r_cnt[0] = atoi(cnt);
                else if (rw_flag[1] == '2')
                    DSHM(dk,fk).r_cnt[1] = atoi(cnt);
                else if (rw_flag[1] == '3')
                    DSHM(dk,fk).r_cnt[2] = atoi(cnt);
                else if (rw_flag[1] == '4')
                    DSHM(dk,fk).r_cnt[3] = atoi(cnt);
                else if (rw_flag[1] == '5')
                    DSHM(dk,fk).r_cnt[4] = atoi(cnt);
                else if (rw_flag[1] == '6')
                    DSHM(dk,fk).r_cnt[5] = atoi(cnt);
                else if (rw_flag[1] == '7')
                    DSHM(dk,fk).r_cnt[6] = atoi(cnt);
                else if (rw_flag[1] == '8')
                    DSHM(dk,fk).r_cnt[7] = atoi(cnt);
                else if (rw_flag[1] == '9')
                    DSHM(dk,fk).r_cnt[8] = atoi(cnt);

                break;
            }
            else if (rw_flag[0] == 'S') {
                DSHM(dk,fk).sm_r_cnt = atoi(cnt);
                break;
            }
            else if (rw_flag[0] == 'W') {
                if (rw_flag[1] == '1')
                    DSHM(dk,fk).w_cnt[0] = atoi(cnt);
                else if (rw_flag[1] == '2')
                    DSHM(dk,fk).w_cnt[1] = atoi(cnt);
            }
        }
    }

    if (fk == DAEMON(dk).d_count) {
        for (fk = 0; fk < DAEMON(dk).f_count; fk ++) {
            if (memcmp(FILEM(dk,fk).file_name, file_name, 10) == 0) {
                if (rw_flag[0] == 'R') {
                    if (rw_flag[1] == '1')
                        FILEM(dk,fk).r_cnt[0] = atoi(cnt);
                    else if (rw_flag[1] == '2')
                        FILEM(dk,fk).r_cnt[1] = atoi(cnt);
                    else if (rw_flag[1] == '3')
                        FILEM(dk,fk).r_cnt[2] = atoi(cnt);
                    else if (rw_flag[1] == '4')
                        FILEM(dk,fk).r_cnt[3] = atoi(cnt);
                    else if (rw_flag[1] == '5')
                        FILEM(dk,fk).r_cnt[4] = atoi(cnt);
                    else if (rw_flag[1] == '6')
                        FILEM(dk,fk).r_cnt[5] = atoi(cnt);
                    else if (rw_flag[1] == '7')
                        FILEM(dk,fk).r_cnt[6] = atoi(cnt);
                    else if (rw_flag[1] == '8')
                        FILEM(dk,fk).r_cnt[7] = atoi(cnt);
                    else if (rw_flag[1] == '9')
                        FILEM(dk,fk).r_cnt[8] = atoi(cnt);

                    break;
                }
                else if (rw_flag[0] == 'W') {
                    if (rw_flag[1] == '1')
                        FILEM(dk,fk).w_cnt[0] = atoi(cnt);
                    else if (rw_flag[1] == '2')
                        FILEM(dk,fk).w_cnt[1] = atoi(cnt);

                    break;
                }
            }

            if (fk == DAEMON(dk).f_count - 1) {
                printf("ERROR:data not registered[%s]\n", file_name);
                exit(FAIL);
            }
        }
    }

#if defined __hpux
    fp = popen("who -mR", "r");
#elif defined(sun) || defined(_AIX) || defined(__linux)
    fp = popen("who -m", "r");
#endif
    fgets(buf, sizeof (buf), fp);
    pclose(fp);
    buf[strlen(buf)-1] = '\0';

    Log(USR_OK, "[%s: %s, %s, %s]", buf, file_name, rw_flag, cnt);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setfcnt.c)
*************************************************************************/
