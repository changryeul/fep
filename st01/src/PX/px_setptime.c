#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change start/end time of a process
#   File    : px_setptime.c
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
    int     dk, pk;
    char    proc_id[12], seflag[4], setime[8], sub[4], buf[128];
    FILE    *fp;

    if (argc != 1 && argc != 4) {
        printf("==========================================================\n");
        printf("[change start/end time of a process]\n");
        printf("(PROC.start_time and/or PROC.end_time will be changed)\n\n");
        printf("Usage: %s <process ID> <s/e> <time>\n", argv[0]);
        printf("       (s:start time, e:end time)\n\n");
        printf("  e.g. %s %ca_1101_tr s 0630\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(seflag, 0, sizeof (seflag));
    memset(setime, 0, sizeof (setime));

    if (argc == 4) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(seflag, argv[2], strlen(argv[2]));
        memcpy(setime, argv[3], strlen(argv[3]));
    }
    else {
        printf("\033[1mprocess name(e.g. %ca_1101_tr) ?\033[0m ", argv[0][0]);
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mstart or end(s:start time, e:end time) ?\033[0m ");
        fflush(stdout);
        fgets(seflag, sizeof(seflag), stdin);
        if (seflag[0] == '\0')
            exit(FAIL);

        printf("\033[1mtime(HHMM) ?\033[0m ");
        fflush(stdout);
        fgets(setime, sizeof(setime), stdin);
        if (setime[0] == '\0')
            exit(FAIL);
    }

    if (strlen(proc_id) != 10) {
        printf("ERROR:process name[%s]\n", proc_id);
        exit(FAIL);
    }

    if (seflag[0] != 's' && seflag[0] != 'S' &&
            seflag[0] != 'e' && seflag[0] != 'E') {
        printf("ERROR:start or end[%s]\n", seflag);
        exit(FAIL);
    }

    UtoL(seflag, 1);

    if (strlen(setime) != 4) {
        printf("ERROR:time[%s]\n", setime);
        exit(FAIL);
    }

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
            printf("%s_time changed [%s: %s ->",
                    seflag[0] == 's' ? "start" : "end", PROC(dk,pk).process_id,
                    seflag[0] == 's' ? PROC(dk,pk).start_time :
                    PROC(dk,pk).end_time);

            if (seflag[0] == 's')
                memcpy(PROC(dk,pk).start_time, setime, strlen(setime));
            else
                memcpy(PROC(dk,pk).end_time, setime, strlen(setime));

            printf(" %s]\n", seflag[0] == 's' ?
                    PROC(dk,pk).start_time : PROC(dk,pk).end_time);
            break;
        }

        if (pk == DAEMON(dk).p_count - 1) {
            printf("ERROR:process not registered[%s]\n", proc_id);
            exit(FAIL);
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

    Log(USR_OK, "[%s: %s, %s, %s]", buf, proc_id, seflag, setime);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setptime.c)
*************************************************************************/
