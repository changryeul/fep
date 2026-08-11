#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : set interface sequence of TCP client (TCP2) process with
#               counterpart sequence
#   File    : px_sett2seq.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Dk, Pk;

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    sub[4], Sub[4], buf[128], proc_id[12];
    FILE    *fp;

    if (argc != 2 && argc != 3) {
        printf("==========================================================\n");
        printf("[set interface sequence of TCP client(TCP2) process with counterpart sequence]\n\n");
        printf("Usage: %s <sub name> <process ID>\n\n", argv[0]);
        printf("  e.g. 1) %s %cc\n", argv[0], argv[0][0]);
        printf("       2) %s %cc %cc_1101_ts\n",
                argv[0], argv[0][0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    Init_Mana(argc, argv);

    memset(sub, 0, sizeof (sub));
    memset(Sub, 0, sizeof (Sub));
    memset(proc_id, 0, sizeof (proc_id));

    memcpy(sub, argv[1], strlen(argv[1]));
    memcpy(Sub, argv[1], strlen(argv[1]));

    LtoU(Sub, 2);
    Dk = Sub[1] - 'A';

    if (strlen(sub) != 2 || sub[0] != _Exe_Name[0] ||
            sub[1] < 't' || sub[1] > 'u') {
        printf("invalid sub name [%s]\n", sub);
        exit(FAIL);
    }

    if (INFO(Dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", Sub);
        Log(USR_ERROR, "%s daemon not registered !!!", Sub);
        exit(FAIL);
    }

    if (INFO(Dk).process_status == 0) {
        printf("%s daemon not used !!!\n", Sub);
        Log(USR_ERROR, "%s daemon not used !!!", Sub);
        exit(FAIL);
    }

    if (INFO(Dk).process_no == 0) {
        printf("%s daemon not run [start time=%s, end time=%s]\n",
                Sub, INFO(Dk).start_time, INFO(Dk).end_time);
        Log(USR_ERROR, "%s daemon not run [start time=%s, end time=%s]",
                Sub, INFO(Dk).start_time, INFO(Dk).end_time);
        exit(FAIL);
    }

    if (argc == 3) {
        memcpy(proc_id, argv[2], strlen(argv[2]));

        if (memcmp(proc_id+8, "ts", 2) != 0) {
            printf("invalid process id(ts only) [%s]\n", proc_id);
            exit(FAIL);
        }
    }

    for (Pk = 0; Pk < DAEMON(Dk).p_count; Pk ++) {
        if (memcmp(PROC(Dk,Pk).process_type, "TS2", 3) == 0 &&
                PROC(Dk,Pk).counter_seq != IF_SEQ(Dk,Pk) &&
                IF_SEQ(Dk,Pk) != PROC(Dk,Pk).counter_seq && (argc == 2 ||
                (argc == 3 && memcmp(PROC(Dk,Pk).process_id, proc_id, 10) == 0))) {
            printf("if_seq changed [%s: %d ->",
                    PROC(Dk,Pk).process_id, IF_SEQ(Dk,Pk));

            IF_SEQ(Dk,Pk) = PROC(Dk,Pk).counter_seq;
            PROC(Dk,Pk).counter_seq = 0;

            printf(" %d]\n", IF_SEQ(Dk,Pk));
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

    if (argc == 3)
        Log(USR_OK, "[%s: %s, %s]", buf, sub, proc_id);
    else
        Log(USR_OK, "[%s: %s]", buf, sub);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_sett2seq.c)
*************************************************************************/
