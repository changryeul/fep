#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change line status (TCP2)
#   File    : px_setlstat.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    proc_id[12], line[4], lstat[4], sub[4], buf[128];
    int     dk, pk, l;
    FILE    *fp;

    if (argc != 1 && argc != 4) {
        printf("==========================================================\n");
        printf("[change line status(TCP2)]\n\n");
        printf("Usage: %s <process ID> <line(P|B|A)> <line status(0|1)>\n", argv[0]);
        printf("       (line = P:primary B:backup A:all\n");
        printf("        line status = 0:off 1:on)\n\n");
        printf("  e.g. %s %ca_1111_ts P 1\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(line, 0, sizeof (line));
    memset(lstat, 0, sizeof (lstat));

    if (argc == 4) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(line, argv[2], strlen(argv[2]));
        memcpy(lstat, argv[3], strlen(argv[3]));
    }
    else {
        printf("\033[1mprocess ID(e.g. %ca_1111_ts) ?\033[0m ", argv[0][0]);
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mline flag(P|B|A) ?\033[0m ");
        fflush(stdout);
        fgets(line, sizeof(line), stdin);
        if (line[0] == '\0')
            exit(FAIL);

        printf("\033[1mline status(0:off 1:on) ?\033[0m ");
        fflush(stdout);
        fgets(lstat, sizeof(lstat), stdin);
        if (lstat[0] == '\0')
            exit(FAIL);
    }

    if (strlen(proc_id) != 10) {
        printf("ERROR:process ID[%s]\n", proc_id);
        exit(FAIL);
    }

    LtoU(line, 1);

    if (line[0] != 'P' && line[0] != 'B' && line[0] != 'A') {
        printf("ERROR:line flag[%s]\n", line);
        exit(FAIL);
    }

    if (atoi(lstat) != 0 && atoi(lstat) != 1) {
        printf("ERROR:line status[%s]\n", lstat);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    sprintf(sub, "%-2.2s", proc_id);
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (pk = 0; pk < DAEMON(dk).p_count; pk ++) {
        if (memcmp(PROC(dk,pk).process_id, proc_id, 10) == 0) {
            if (PROC(dk,pk).type != TY_TRS2) {
                puts("TCP2 process only !!!");
                exit(FAIL);
            }

            printf("[%-10.10s:%s:line status of ",
                    PROC(dk,pk).process_id, PROC(dk,pk).process_info);

            switch (line[0]) {
                case    'P':
                    TCP2_LSTAT(dk,pk,0) = atoi(lstat);
                    printf("primary = %d]\n", TCP2_LSTAT(dk,pk,0));
                    break;
                case    'B':
                    TCP2_LSTAT(dk,pk,1) = atoi(lstat);
                    printf("backup = %d]\n", TCP2_LSTAT(dk,pk,1));
                    break;
                case    'A':
                    printf("all lines changed]\n");

                    for (l = 0; l < 2; l ++) {
                        if (PROC(dk,pk).l.t2.l[l]) {
                            TCP2_LSTAT(dk,pk,l) = atoi(lstat);
                            if (l == 0)
                                printf("line status of primary = ");
                            else
                                printf("line status of backup = ");
                            printf("%d\n", TCP2_LSTAT(dk,pk,l));
                    }
                }

                break;
                default:
                    break;
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

    Log(USR_OK, "[%s: %s, %s, %s]", buf, proc_id, line, lstat);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setlstat.c)
*************************************************************************/
