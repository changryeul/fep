#define     _GLOBAL
#if !defined(_LARGE_FILES)
#define     _LARGE_FILES
#endif
/*------------------------------------------------------------------------
#   Module  : check file size
#   File    : pz_filechk.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

void        Sub_SHM(void);
void        Mem_SHM(int, int);
/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Dk;
char    RecoveryFlag, Sub[4];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Check_Argument(int argc, char *argv[]);

/*************************************************************************
    Function        : . super daemon main
    Parameters IN   : . argc    : number of arguments (2 or 3)
                      . argv[0] : execution name
                      . argv[1] : sub system name (a ~ w)
                      . argv[2] : mode (rec:recovery)
    Parameters OUT  : .
    Return Code     : . int (0(OK):normal, 1(FAIL):file error)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int         rt, fk;
    long long   file_size1, file_size2;
    char        error_flag;
    char        dpath[252], spath[256], old_dpath[256], old_spath[260];
    FILE        *dat_fp, *seq_fp;
    struct stat64   f_info;

    RecoveryFlag = error_flag = OFF;

    Check_Argument(argc, argv);

    sprintf(_Exe_Name, "%s", argv[0]);
    sprintf(_Process_Name, "%s", argv[0]);
    sprintf(_SubSystem_Name, "%-2.2s", argv[0]);
    D_K = P_K = -1;

    Check_Environment();
    Setsigfatal();
    Sub_SHM();
    Mem_SHM(1, Dk);

    if (INFO(Dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", Sub);
        exit(FAIL);
    }

    Log(USR_OK, "%s start ... [%s]", _Exe_Name, Sub);

    /* online SAM files */
    for (fk = 0; fk < DAEMON(Dk).f_count; fk ++) {
        sprintf(dpath, "%s/%-2.2s/00000000/%s",
                _FEP_DAT, Sub, FILEM(Dk,fk).file_name);

        f_info.st_size = 0;

        rt = stat64(dpath, &f_info);

        if (rt != 0) {
            Log(SAM_FATAL, "stat64 fail[%s] {%d:%s}", dpath, SYS_NO, SYS_STR);
            exit(FAIL);
        }

        /*
                file_size1 = FILEM(Dk,fk).w_cnt[0] *
                    (sizeof (FILE_RW_HEAD) + FILEM(Dk,fk).record_size + 1);
                file_size2 = FILEM(Dk,fk).w_cnt[0] *
                    (sizeof (SISE_RW_HEAD) + FILEM(Dk,fk).record_size + 1);
        */

        file_size1 = (sizeof (FILE_RW_HEAD) + FILEM(Dk,fk).record_size + 1);
        file_size1 = file_size1 * FILEM(Dk,fk).w_cnt[0];
        file_size2 = (sizeof (SISE_RW_HEAD) + FILEM(Dk,fk).record_size + 1);
        file_size2 = file_size2 * FILEM(Dk,fk).w_cnt[0];

        if (f_info.st_size != file_size1 && f_info.st_size != file_size2) {
            Log(SAM_FATAL, "abnormal data(f_count):[%s][%lld:%lld:%lld][%d]",
                    FILEM(Dk,fk).file_name, f_info.st_size, file_size1, file_size2,
                    FILEM(Dk,fk).w_cnt[0]);
            error_flag = ON;

            if (RecoveryFlag == ON) {
                snprintf(old_dpath, sizeof(old_dpath), "%s.old", dpath);

                rt = link(dpath, old_dpath);

                if (rt) {
                    Log(SAM_FATAL, "link fail[%s][%s]", dpath, old_dpath);
                    exit(FAIL);
                }

                rt = unlink(dpath);

                if (rt) {
                    Log(SAM_FATAL, "unlink fail[%s]", dpath);
                    exit(FAIL);
                }

                dat_fp = fopen(dpath, "w+");

                if (dat_fp == NULL) {
                    Log(SAM_FATAL, "fopen fail[%s] {%d:%s}",
                            dpath, SYS_NO, SYS_STR);
                    exit(FAIL);
                }

                fclose(dat_fp);

                FILEM(Dk,fk).w_cnt[0] = 0;
                FILEM(Dk,fk).r_cnt[0] = 0;
                FILEM(Dk,fk).r_cnt[1] = 0;

                snprintf(spath, sizeof(spath), "%s_seq", dpath);
                snprintf(old_spath, sizeof(old_spath), "%s.old", spath);

                rt = link(spath, old_spath);

                if (rt) {
                    Log(SAM_FATAL, "link fail[%s][%s]", spath, old_spath);
                    exit(FAIL);
                }

                rt = unlink(spath);

                if (rt) {
                    Log(SAM_FATAL, "unlink fail[%s]", spath);
                    exit(FAIL);
                }

                seq_fp = fopen(spath, "w+");

                if (seq_fp == NULL) {
                    Log(SAM_FATAL, "fopen fail[%s] {%d:%s}",
                            spath, SYS_NO, SYS_STR);
                    exit(FAIL);
                }

                fclose(seq_fp);

                Log(SAM_OK, "data recovered:[%s] W[%d] R1[%d] R2[%d]",
                        FILEM(Dk,fk).file_name, FILEM(Dk,fk).w_cnt[0],
                        FILEM(Dk,fk).r_cnt[0], FILEM(Dk,fk).r_cnt[1]);
            }
        }
    }

    /* data SHM backup files    */
    /* park
        for (fk = 0; fk < DAEMON(Dk).d_count; fk ++) {
            sprintf (dpath, "%s/%-2.2s/00000000/%s",
                _FEP_DAT, Sub, DSHM(Dk,fk).data_name);

            f_info.st_size = 0;

            rt = stat64 (dpath, &f_info);

            if (rt != 0) {
                Log (SAM_FATAL, "stat64 fail[%s] {%d:%s}", dpath, SYS_NO, SYS_STR);
                exit (FAIL);
            }

            file_size1 = DSHM(Dk,fk).w_cnt[0] *
                (sizeof (FILE_RW_HEAD) + DSHM(Dk,fk).data_size + 1);
            file_size2 = DSHM(Dk,fk).w_cnt[0] *
                (sizeof (SISE_RW_HEAD) + DSHM(Dk,fk).data_size + 1);

            if (f_info.st_size != file_size1 && f_info.st_size != file_size2) {
                Log (SAM_FATAL, "abnormal data (d_count):[%s][%d:%d:%d][%d]",
                    DSHM(Dk,fk).data_name, f_info.st_size, file_size1, file_size2,
                    DSHM(Dk,fk).w_cnt[0]);
                error_flag = ON;

                if (RecoveryFlag == ON) {
                    sprintf (old_dpath, "%s.old", dpath);

                    rt = link (dpath, old_dpath);

                    if (rt) {
                        Log (SAM_FATAL, "link fail[%s][%s]", dpath, old_dpath);
                        exit (FAIL);
                    }

                    rt = unlink (dpath);

                    if (rt) {
                        Log (SAM_FATAL, "unlink fail[%s]", dpath);
                        exit (FAIL);
                    }

                    dat_fp = fopen (dpath, "w+");

                    if (dat_fp == NULL) {
                        Log (SAM_FATAL, "fopen fail[%s] {%d:%s}",
                            dpath, SYS_NO, SYS_STR);
                        exit (FAIL);
                    }

                    fclose (dat_fp);

                    DSHM(Dk,fk).w_cnt[0] = 0;
                    DSHM(Dk,fk).r_cnt[0] = 0;
                    DSHM(Dk,fk).r_cnt[1] = 0;
                    DSHM(Dk,fk).sm_r_cnt = 0;

                    sprintf (spath, "%s_dseq", dpath);
                    sprintf (old_spath, "%s.old", spath);

                    rt = link (spath, old_spath);

                    if (rt) {
                        Log (SAM_FATAL, "link fail[%s][%s]", spath, old_spath);
                        exit (FAIL);
                    }

                    rt = unlink (spath);

                    if (rt) {
                        Log (SAM_FATAL, "unlink fail[%s]", spath);
                        exit (FAIL);
                    }

                    seq_fp = fopen (spath, "w+");

                    if (seq_fp == NULL) {
                        Log (SAM_FATAL, "fopen fail[%s] {%d:%s}",
                            spath, SYS_NO, SYS_STR);
                        exit (FAIL);
                    }

                    fclose (seq_fp);

                    Log (SAM_OK, "data recovered:[%s] W1[%d] R1[%d] R2[%d] SM[%d]",
                        DSHM(Dk,fk).data_name, DSHM(Dk,fk).w_cnt[0],
                        DSHM(Dk,fk).r_cnt[0], DSHM(Dk,fk).r_cnt[1],
                        DSHM(Dk,fk).sm_r_cnt);
                }
            }
        }
    */

    Log(USR_OK, "... %s end", _Exe_Name);

    if (error_flag == ON)
        exit(FAIL);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    Function        : . check arguments
    Parameters IN   : . argc    : number of arguments (2 or 3)
                      . argv[0] : execution name
                      . argv[1] : sub system name (a ~ w)
                      . argv[2] : mode (rec:recovery)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check_Argument(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    /* check the number of arguments    */
    if (argc == 1 || argc > 3) {
        printf("ERROR: invalid arguments number [%d]\n", argc);
        puts("==========================================================");
        printf("Usage: %s <sub system name(a ~ w)> <rec:recovery>\n",
                argv[0]);
        printf("  e.g. 1) %s a\n", argv[0]);
        printf("       2) %s a rec\n", argv[0]);
        puts("==========================================================");
        exit(FAIL);
    }

    sprintf(Sub, "%c%c", argv[0][0], argv[1][0]);
    LtoU(Sub, 2);

    if (Sub[1] < 'A' || Sub[1] > 'W') {
        printf("invalid sub name[%c]\n", argv[1][0]);
        exit(FAIL);
    }

    Dk = Sub[1] - 'A';

    if (argc == 3) {
        if (memcmp(argv[2], "rec", 3) == 0)
            RecoveryFlag = ON;
        else {
            printf("invalid argument[%s]\n", argv[2]);
            exit(FAIL);
        }
    }

    return;
}   /* End of Check_Argument () */

/*************************************************************************
    End of Program (pz_filechk.c)
*************************************************************************/
