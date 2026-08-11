#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change start/end time and date flag of a sub daemon
#   File    : px_setdtime.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     dk;
    char    sub[4], stime[8], etime[8], dflag[4], buf[128];
    FILE    *fp;

    if (argc != 1 && argc != 5) {
        printf("==========================================================\n");
        printf("[change start/end time and date flag of a sub daemon]\n");
        printf("(INFO.start_time, end_time, date_flag will be changed)\n\n");
        printf("Usage: %s <sub name> <start time> <end time> <date flag>\n",
                argv[0]);
        printf("  e.g. %s %ca 0530 0300 2\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(sub, 0, sizeof (sub));
    memset(stime, 0, sizeof (stime));
    memset(etime, 0, sizeof (etime));
    memset(dflag, 0, sizeof (dflag));

    if (argc == 5) {
        memcpy(sub, argv[1], strlen(argv[1]));
        memcpy(stime, argv[2], strlen(argv[2]));
        memcpy(etime, argv[3], strlen(argv[3]));
        memcpy(dflag, argv[4], strlen(argv[4]));
    }
    else {
        printf("\033[1msub name(e.g. %ca) ?\033[0m ", argv[0][0]);
        fflush(stdout);
        fgets(sub, sizeof(sub), stdin);
        if (sub[0] == '\0')
            exit(FAIL);

        printf("\033[1mstart time(HHMM, e.g. 0530) ?\033[0m ");
        fflush(stdout);
        fgets(stime, sizeof(stime), stdin);
        if (stime[0] == '\0')
            exit(FAIL);

        printf("\033[1mend time(HHMM, e.g. 0300) ?\033[0m ");
        fflush(stdout);
        fgets(etime, sizeof(etime), stdin);
        if (etime[0] == '\0')
            exit(FAIL);

        printf("\033[1mdate flag(1:D~D 2:D~D+1 3:D+1~D+1 4:D-1~D 5:D-1~D-1) ?\033[0m ");
        fflush(stdout);
        fgets(dflag, sizeof(dflag), stdin);
        if (dflag[0] == '\0')
            exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    printf("DAEMON start_time/end_time/date_flag changed [%s: %s %s %d ->",
            sub, DAEMON(dk).start_time, DAEMON(dk).end_time, DAEMON(dk).date_flag);

    memcpy(DAEMON(dk).start_time, stime, 4);
    memcpy(DAEMON(dk).end_time, etime, 4);
    DAEMON(dk).date_flag = atoi(dflag);

    printf(" %s %s %d]\n",
            DAEMON(dk).start_time, DAEMON(dk).end_time, DAEMON(dk).date_flag);

    printf("INFO start_time/end_time/date_flag changed [%s: %s %s %d ->",
            sub, INFO(dk).start_time, INFO(dk).end_time, INFO(dk).date_flag);

    memcpy(INFO(dk).start_time, stime, 4);
    memcpy(INFO(dk).end_time, etime, 4);
    INFO(dk).date_flag = atoi(dflag);

    printf(" %s %s %d]\n",
            INFO(dk).start_time, INFO(dk).end_time, INFO(dk).date_flag);

#if defined __hpux
    fp = popen("who -mR", "r");
#elif defined(sun) || defined(_AIX) || defined(__linux)
    fp = popen("who -m", "r");
#endif
    fgets(buf, sizeof (buf), fp);
    pclose(fp);
    buf[strlen(buf)-1] = '\0';

    Log(USR_OK, "[%s: %s, %s, %s, %s]", buf, sub, stime, etime, dflag);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setdtime.c)
*************************************************************************/
