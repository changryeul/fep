#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change counter sequence of a process
#   File    : px_setcseq.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     dk, pk;
    char    proc_id[12], counter_seq[12], sub[4], buf[128];
    FILE    *fp;

    if (argc != 1 && argc != 3) {
        printf("==========================================================\n");
        printf("[change counter sequence of a process]\n");
        printf("Usage: %s <process ID> <counter sequence>\n\n", argv[0]);
        printf("  e.g. %s %cc_1101_ts 10\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(counter_seq, 0, sizeof (counter_seq));

    if (argc == 3) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(counter_seq, argv[2], strlen(argv[2]));
    }
    else {
        printf("\033[1mprocess name(e.g. jc_1101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mcounter sequence ?\033[0m ");
        fflush(stdout);
        fgets(counter_seq, sizeof(counter_seq), stdin);
        if (counter_seq[0] == '\0')
            exit(FAIL);
    }

    if (strlen(proc_id) != 10) {
        printf("ERROR:process name[%s]\n", proc_id);
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
            printf("counter_seq changed [%s: %d ->",
                    PROC(dk,pk).process_id, PROC(dk,pk).counter_seq);

            PROC(dk,pk).counter_seq = atoi(counter_seq);

            printf(" %d]\n", PROC(dk,pk).counter_seq);
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

    Log(USR_OK, "[%s: %s, %s]", buf, proc_id, counter_seq);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setcseq.c)
*************************************************************************/
