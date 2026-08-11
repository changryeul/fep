#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change in/out data SHM or fifo name of a process
#   File    : px_setdname.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Dk, Pk;
char    NewDshm[20];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
int     Change_Dshm_Name(void);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     k;
    char    proc_id[12], dshm_flag[4], sub[4], buf[128];
    char    old_dshm[20], old_fifo[20], new_fifo[20];
    FILE    *fp;

    if (argc != 1 && argc != 4 && argc != 5) {
        printf("==========================================================\n");
        printf("[change in/out data SHM or fifo name of a process]\n\n");
        printf("Usage: 1) %s <process ID> <dshm flag(I1 ~ I3)> <data name> <fifo name>\n", argv[0]);
        printf("          e.g. %s jc_1201_dd I1 ja_1201_dd ja_1201_dd1\n",
                argv[0]);
        printf("       2) %s <process ID> <dshm flag(O1 ~ O9)> <data name>\n",
                argv[0]);
        printf("          e.g. %s jc_1201_dd O1 jc_1201_ts\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(dshm_flag, 0, sizeof (dshm_flag));
    memset(NewDshm, 0, sizeof (NewDshm));
    memset(new_fifo, 0, sizeof (new_fifo));

    if (argc == 4 || argc == 5) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(dshm_flag, argv[2], strlen(argv[2]));
        LtoU(dshm_flag, 1);
        memcpy(NewDshm, argv[3], strlen(argv[3]));

        if (argc == 5)
            memcpy(new_fifo, argv[4], strlen(argv[4]));
    }
    else {
        printf("\033[1mprocess ID(e.g. jc_1201_dd) ?\033[0m ");
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mdata SHM flag(I1 ~ I3, O1 ~ O9) ?\033[0m ");
        fflush(stdout);
        fgets(dshm_flag, sizeof(dshm_flag), stdin);
        if (dshm_flag[0] == '\0')
            exit(FAIL);

        printf("\033[1mdata SHM name(e.g. jc_1201_dd) ?\033[0m ");
        fflush(stdout);
        fgets(NewDshm, sizeof(NewDshm), stdin);
        if (NewDshm[0] == '\0')
            exit(FAIL);

        LtoU(dshm_flag, 1);
        if (dshm_flag[0] == 'I') {
            printf("\033[1mfifo name(e.g. jc_1201_dd1) ?\033[0m ");
            fflush(stdout);
            fgets(new_fifo, sizeof(new_fifo), stdin);
            if (new_fifo[0] == '\0')
                exit(FAIL);
        }
    }

    if (strlen(proc_id) != 10) {
        printf("check the process ID\n");
        exit(FAIL);
    }

    if (memcmp(dshm_flag, "I1", 2) != 0 && memcmp(dshm_flag, "I2", 2) != 0 &&
            memcmp(dshm_flag, "I3", 2) != 0 && memcmp(dshm_flag, "O1", 2) != 0 &&
            memcmp(dshm_flag, "O2", 2) != 0 && memcmp(dshm_flag, "O3", 2) != 0 &&
            memcmp(dshm_flag, "O4", 2) != 0 && memcmp(dshm_flag, "O5", 2) != 0 &&
            memcmp(dshm_flag, "O6", 2) != 0 && memcmp(dshm_flag, "O7", 2) != 0 &&
            memcmp(dshm_flag, "O8", 2) != 0 && memcmp(dshm_flag, "O9", 2) != 0) {
        printf("ERROR:data SHM flag[%s]\n", dshm_flag);
        exit(FAIL);
    }

    if (strlen(NewDshm) != 10) {
        printf("ERROR:data SHM name[%s]\n", NewDshm);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    memset(old_dshm, 0, sizeof (old_dshm));
    memset(old_fifo, 0, sizeof (old_fifo));
    sprintf(sub, "%-2.2s", proc_id);
    LtoU(sub, 2);
    Dk = sub[1] - 'A';
    k = AtoIf(&dshm_flag[1], 1) - 1;

    if (INFO(Dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (Pk = 0; Pk < DAEMON(Dk).p_count; Pk ++) {
        if (memcmp(PROC(Dk,Pk).process_id, proc_id, 10) == 0) {
            if (dshm_flag[0] == 'I') {
                strncpy(old_dshm, IDN(Dk,Pk,k), sizeof(old_dshm) - 1);
                old_dshm[sizeof(old_dshm) - 1] = '\0';
                PROC(Dk,Pk).in_d[k] = Change_Dshm_Name();

                strncpy(old_fifo, FFN(Dk,Pk,k), sizeof(old_fifo) - 1);
                old_fifo[sizeof(old_fifo) - 1] = '\0';
                memcpy(FFN(Dk,Pk,k), new_fifo, strlen(new_fifo));
                printf("IDN %c & FFN %c of %s changed [%s -> %s;%s -> %s]\n",
                        dshm_flag[1], dshm_flag[1], PROC(Dk,Pk).process_id,
                        old_dshm, IDN(Dk,Pk,k), old_fifo, FFN(Dk,Pk,k));
            }
            else if (dshm_flag[0] == 'O') {
                strncpy(old_dshm, ODN(Dk,Pk,k), sizeof(old_dshm) - 1);
                old_dshm[sizeof(old_dshm) - 1] = '\0';
                PROC(Dk,Pk).out_d[k] = Change_Dshm_Name();
                printf("ODN %c of %s changed [%s -> %s]\n",
                        dshm_flag[1], PROC(Dk,Pk).process_id, old_dshm,
                        ODN(Dk,Pk,k));
            }
            break;
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

    if (dshm_flag[0] == 'I')
        Log(USR_OK, "[%s: %s, %s, %s, %s]",
                buf, proc_id, dshm_flag, NewDshm, new_fifo);
    else
        Log(USR_OK, "[%s: %s, %s, %s]", buf, proc_id, dshm_flag, NewDshm);

    exit(OK);
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
int     Change_Dshm_Name(void)
/*----------------------------------------------------------------------*/
{
    int     fk;

    for (fk = 0; fk < DAEMON(Dk).d_count; fk ++) {
        if (memcmp(NewDshm, DSHM(Dk,fk).data_name, 10) == 0)
            return (fk + 1);

        if (fk == DAEMON(Dk).d_count - 1) {
            Log(PRO_FATAL, "%s unregistered", NewDshm);
            exit(FAIL);
        }
    }

    Log(PRO_FATAL, "%s unregistered(d_count=0)", NewDshm);
    exit(FAIL);
    return (0);
}   /* End of Change_Dshm_Name ()   */

/*************************************************************************
    End of Program (px_setdname.c)
*************************************************************************/
