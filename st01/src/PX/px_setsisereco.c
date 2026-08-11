#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change interface sequence of a process
#   File    : px_setsisereco.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     dk, pk, tr_seq;
    char    proc_id[12], sub[4], buf[128];
    char    tr_code[11], arg3[11], arg4[11];
    FILE    *fp;

    if (argc <= 2) {
        printf("==========================================================\n");
        printf("[change interface sequence of a process]\n");
        printf("Usage: %s <process ID> <recovery TR(REQJM/REQIF)>\n\n", argv[0]);
        printf("  e.g. %s %ca_7702_tr REQJM\n", argv[0], argv[0][0]);
        printf("  e.g. %s %ca_7801_tr tr  101\n", argv[0], argv[0][0]);
        printf("  e.g. %s %ca_7801_tr seq ABC\n", argv[0], argv[0][0]);
        printf("  e.g. %s %ca_1801_ts tr  101\n", argv[0], argv[0][0]);
        printf("  e.g. %s %ca_1801_ts seq ABC\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    if ((memcmp(argv[1], "pa_7702_tr", 10) != 0)   &&
            (memcmp(argv[1], "pa_7801_tr", 10) != 0)   &&
            (memcmp(argv[1], "pa_1801_ts", 10) != 0)   ) {
        printf("\033[1mprocess���� �ٸ��ϴ�. pa_7702_tr or pa_7801_tr or pa_1801_ts �������մϴ�.\n\033[0m ");
        exit(FAIL);
    }

    memset(proc_id,    0,  sizeof (proc_id));
    memset(tr_code,    0,  sizeof (tr_code));
    memset(arg3,       0,  sizeof (arg3));
    memset(arg4,       0,  sizeof (arg4));

    memcpy(proc_id, argv[1], strlen(argv[1]));
    if (argc == 3) {
        memcpy(tr_code, argv[2], strlen(argv[2]));
    }
    else if (argc == 4) {
        memcpy(arg3, argv[2], strlen(argv[2]));
        memcpy(arg4, argv[3], strlen(argv[3]));
    }
    else {
        printf("INPUT ERROR\n");
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
            if (argc == 3) {
                printf("curr_tr changed [%s: %s ->",
                        PROC(dk,pk).process_id, PROC(dk,pk).curr_tr);
                memcpy(PROC(dk,pk).curr_tr, tr_code, strlen(tr_code));
                printf(" %s]\n", PROC(dk,pk).curr_tr);
            }

            if (argc == 4) {
                if ((memcmp(arg3, "tr", 2) == 0)   && strlen(arg3) == 2) {
                    printf("curr_tr changed [%s: %s ->",
                            PROC(dk,pk).process_id, PROC(dk,pk).curr_tr);
                    memcpy(PROC(dk,pk).curr_tr, arg4, strlen(arg4));
                    printf(" %s]\n", PROC(dk,pk).curr_tr);
                }
                else if ((memcmp(arg3, "seq", 3) == 0)  && strlen(arg3) == 3) {
                    printf("tr_seq changed [%s: %d ->",
                            PROC(dk,pk).process_id, PROC(dk,pk).tr_seq);
                    PROC(dk,pk).tr_seq = AtoIf(arg4, strlen(arg4));
                    printf(" %d]\n", PROC(dk,pk).tr_seq);
                }
                else {
                    printf("INPUT ERROR\n");
                    exit(FAIL);
                }

            }

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

    if (argc == 3)
        Log(USR_OK, "[%s: %s, %s]", buf, proc_id, tr_code);
    else
        Log(USR_OK, "[%s: %s, %s  %s]", buf, proc_id, arg3, arg4);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setsisereco.c)
*************************************************************************/
