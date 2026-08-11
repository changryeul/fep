#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change in/out file or fifo name of a process
#   File    : px_setfname.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Dk, Pk;
char    NewFile[20];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
int     Change_File_Name(void);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     k;
    char    proc_id[12], file_flag[4], sub[4], buf[128];
    char    old_file[20], old_fifo[20], new_fifo[20];
    FILE    *fp;

    if (argc != 1 && argc != 4 && argc != 5) {
        printf("==========================================================\n");
        printf("[change in/out file or fifo name of a process]\n\n");
        printf("Usage: 1) %s <process ID> <file flag(I1 ~ I3)> <file name> <fifo name>\n", argv[0]);
        printf("          e.g. %s jc_4101_ts I1 jc_4101_ts jc_4101_ts1\n",
                argv[0]);
        printf("       2) %s <process ID> <file flag(O1 ~ O9)> <file name>\n",
                argv[0]);
        printf("          e.g. %s jc_4101_tr O1 jc_4101_ts\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(file_flag, 0, sizeof (file_flag));
    memset(NewFile, 0, sizeof (NewFile));
    memset(new_fifo, 0, sizeof (new_fifo));

    if (argc == 4 || argc == 5) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(file_flag, argv[2], strlen(argv[2]));
        LtoU(file_flag, 1);
        memcpy(NewFile, argv[3], strlen(argv[3]));

        if (argc == 5)
            memcpy(new_fifo, argv[4], strlen(argv[4]));
    }
    else {
        printf("\033[1mprocess ID(e.g. jc_4101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mfile flag(I1 ~ I3, O1 ~ O9) ?\033[0m ");
        fflush(stdout);
        fgets(file_flag, sizeof(file_flag), stdin);
        if (file_flag[0] == '\0')
            exit(FAIL);

        printf("\033[1mfile name(e.g. jc_4101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(NewFile, sizeof(NewFile), stdin);
        if (NewFile[0] == '\0')
            exit(FAIL);

        LtoU(file_flag, 1);
        if (file_flag[0] == 'I') {
            printf("\033[1mfifo name(e.g. jc_4101_ts1) ?\033[0m ");
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

    if (memcmp(file_flag, "I1", 2) != 0 && memcmp(file_flag, "I2", 2) != 0 &&
            memcmp(file_flag, "I3", 2) != 0 && memcmp(file_flag, "O1", 2) != 0 &&
            memcmp(file_flag, "O2", 2) != 0 && memcmp(file_flag, "O3", 2) != 0 &&
            memcmp(file_flag, "O4", 2) != 0 && memcmp(file_flag, "O5", 2) != 0 &&
            memcmp(file_flag, "O6", 2) != 0 && memcmp(file_flag, "O7", 2) != 0 &&
            memcmp(file_flag, "O8", 2) != 0 && memcmp(file_flag, "O9", 2) != 0) {
        printf("ERROR:file flag[%s]\n", file_flag);
        exit(FAIL);
    }

    if (strlen(NewFile) != 10) {
        printf("ERROR:file name[%s]\n", NewFile);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    memset(old_file, 0, sizeof (old_file));
    memset(old_fifo, 0, sizeof (old_fifo));
    sprintf(sub, "%-2.2s", proc_id);
    LtoU(sub, 2);
    Dk = sub[1] - 'A';
    k = AtoIf(&file_flag[1], 1) - 1;

    if (INFO(Dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (Pk = 0; Pk < DAEMON(Dk).p_count; Pk ++) {
        if (memcmp(PROC(Dk,Pk).process_id, proc_id, 10) == 0) {
            if (file_flag[0] == 'I') {
                strncpy(old_file, IFN(Dk,Pk,k), sizeof(old_file) - 1);
                old_file[sizeof(old_file) - 1] = '\0';
                PROC(Dk,Pk).in_f[k] = Change_File_Name();

                strncpy(old_fifo, FFN(Dk,Pk,k), sizeof(old_fifo) - 1);
                old_fifo[sizeof(old_fifo) - 1] = '\0';
                memcpy(FFN(Dk,Pk,k), new_fifo, strlen(new_fifo));
                printf("IFN %c & FFN %c of %s changed [%s -> %s;%s -> %s]\n",
                        file_flag[1], file_flag[1], PROC(Dk,Pk).process_id,
                        old_file, IFN(Dk,Pk,k), old_fifo, FFN(Dk,Pk,k));
            }
            else if (file_flag[0] == 'O') {
                strncpy(old_file, OFN(Dk,Pk,k), sizeof(old_file) - 1);
                old_file[sizeof(old_file) - 1] = '\0';
                PROC(Dk,Pk).out_f[k] = Change_File_Name();
                printf("OFN %c of %s changed [%s -> %s]\n",
                        file_flag[1], PROC(Dk,Pk).process_id, old_file,
                        OFN(Dk,Pk,k));
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

    if (file_flag[0] == 'I')
        Log(USR_OK, "[%s: %s, %s, %s, %s]",
                buf, proc_id, file_flag, NewFile, new_fifo);
    else
        Log(USR_OK, "[%s: %s, %s, %s]", buf, proc_id, file_flag, NewFile);

    exit(OK);
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
int     Change_File_Name(void)
/*----------------------------------------------------------------------*/
{
    int     fk;

    for (fk = 0; fk < DAEMON(Dk).f_count; fk ++) {
        if (memcmp(NewFile, FILEM(Dk,fk).file_name, 10) == 0)
            return (fk + 1);

        if (fk == DAEMON(Dk).f_count - 1) {
            Log(PRO_FATAL, "%s unregistered", NewFile);
            exit(FAIL);
        }
    }

    Log(PRO_FATAL, "%s unregistered(f_count=0)", NewFile);
    exit(FAIL);
    return (0);
}   /* End of Change_File_Name ()   */

/*************************************************************************
    End of Program (px_setfname.c)
*************************************************************************/
