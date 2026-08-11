/*------------------------------------------------------------------------
#   Module  : main routine of pz_memory_mp
#   File    : pz_memory_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "daemon.h"
#include    "config_loader.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Main_Process_Memory(void);
void    Check_Work_Dir(void);
int     Chk_Digit(char *, int);
void    Shm_Conf_Process(int);
void    Sub_SHM_Creat(void);
void    Mem_SHM_Creat(void);
void    Sise_SHM_Creat(void);

/*************************************************************************
    Function        : . main routine
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Main_Process_Memory(void)
/*----------------------------------------------------------------------*/
{
    /* executed by super daemon (fepp_mp) or user (manually)    */
    if (D_K + 'A' == 'Z' || D_K == -1) {
        /* initialize daemon SHM (INFO) */
        Sub_SHM_Creat();

        /* read daemon config and set the daemon SHM (INFO)
           DB mode: cfg_db_load_daemon, INI fallback: Daemon_Config_Read */
        Config_Reload_Group("daemon", 0);
    }
    /* executed by sub daemons (daemon_mp)  */
    else {
        /* attach daemon SHM (INFO) */
        Sub_SHM();
    }

    /* executed by user or sub daemons  */
    if (D_K + 'A' != 'Z') {
        Log(USR_OK, "00. pz_memory_proc Mem_SHM_Creat Start!!");
        /* initialize sub SHM and set daemon SHM (INFO, DAEMON) */
        Mem_SHM_Creat();
        Log(USR_OK, "01. pz_memory_proc Mem_SHM_Creat End!!");

        /* initialize sise data SHM */
        if (D_K != -1) {
            Sise_SHM_Creat();
        }

        /* create and backup the work directory */
        Check_Work_Dir();

        /* Defense-in-depth: ensure DAEMON() has all fields from INFO().
           Daemon_Config_Read(1) inside Mem_SHM_Creat() should have already
           set these, but copy again as safety net for any edge cases.
           Includes counts, times, date_flag, and date. */
        {
            int dk;
            if (D_K == -1) {
                for (dk = 0; dk < 26; dk++) {
                    if (INFO(dk).process_id[0] == 0)
                        continue;
                    DAEMON(dk).process_count = INFO(dk).process_count;
                    DAEMON(dk).file_count    = INFO(dk).file_count;
                    DAEMON(dk).dshm_count    = INFO(dk).dshm_count;
                    DAEMON(dk).tcp1_count    = INFO(dk).tcp1_count;
                    DAEMON(dk).tcp2_count    = INFO(dk).tcp2_count;
                    DAEMON(dk).udpip_count   = INFO(dk).udpip_count;
                    DAEMON(dk).data_count    = INFO(dk).data_count;
                    memcpy(DAEMON(dk).start_time, INFO(dk).start_time,
                            sizeof(DAEMON(dk).start_time));
                    memcpy(DAEMON(dk).end_time, INFO(dk).end_time,
                            sizeof(DAEMON(dk).end_time));
                    DAEMON(dk).date_flag = INFO(dk).date_flag;
                    memcpy(DAEMON(dk).date, INFO(dk).date,
                            sizeof(DAEMON(dk).date));
                }
            }
            else {
                /* 방어(crash-hardening): 해당 부문 데몬이 미설정이면 Mem_SHM_Creat의
                   `if (!Shmsize) continue`로 Shm_Mem[D_K].Daemon이 NULL로 남는다.
                   과거엔 아래 DAEMON(D_K) 대입이 NULL 역참조로 SIGSEGV → 원인불명 크래시.
                   D_K==-1 루프(위)의 `INFO().process_id[0]==0 continue` 가드와 동형으로,
                   여기서도 미설정을 잡아 명확한 FATAL로 종료(오진 방지).
                   실제 원인은 대개 config 경로 불일치(_P_CFG가 daemon 설정 없는 트리를 가리킴). */
                if (Shm_Mem[D_K].Daemon == NULL || INFO(D_K).process_id[0] == 0) {
                    Log(USR_FATAL, "daemon[%c](D_K=%d) not configured: "
                            "SHM unmapped(proc_count=%d). config 경로 확인 요 _P_CFG=[%s]/daemon.ini",
                            D_K + 'A', D_K, INFO(D_K).process_count,
                            _FEP_CFG ? _FEP_CFG : "(null)");
                    sleep(3);
                    exit(FAIL);
                }
                DAEMON(D_K).process_count = INFO(D_K).process_count;
                DAEMON(D_K).file_count    = INFO(D_K).file_count;
                DAEMON(D_K).dshm_count    = INFO(D_K).dshm_count;
                DAEMON(D_K).tcp1_count    = INFO(D_K).tcp1_count;
                DAEMON(D_K).tcp2_count    = INFO(D_K).tcp2_count;
                DAEMON(D_K).udpip_count   = INFO(D_K).udpip_count;
                DAEMON(D_K).data_count    = INFO(D_K).data_count;
                memcpy(DAEMON(D_K).start_time, INFO(D_K).start_time,
                        sizeof(DAEMON(D_K).start_time));
                memcpy(DAEMON(D_K).end_time, INFO(D_K).end_time,
                        sizeof(DAEMON(D_K).end_time));
                DAEMON(D_K).date_flag = INFO(D_K).date_flag;
                memcpy(DAEMON(D_K).date, INFO(D_K).date,
                        sizeof(DAEMON(D_K).date));
            }
            Log(USR_OK, "02. INFO->DAEMON copy done D_K=%d pcount=%d fcount=%d",
                    D_K,
                    DAEMON(D_K == -1 ? 0 : D_K).process_count,
                    DAEMON(D_K == -1 ? 0 : D_K).file_count);
        }

        if (D_K == -1)
            Shm_Conf_Process(0);               /* set sub SHM - all    */
        else
            Shm_Conf_Process(1);               /* set sub SHM - one    */
    }

    return;
}   /* End of Main_Process_Memory ()    */

/*************************************************************************
    Function        : . create and backup the work directory
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check_Work_Dir(void)
/*----------------------------------------------------------------------*/
{
    int     i, rt;
    char    file_name[128], sub[4], Sub[4], data[10], buf[256], w_date[12];
    char    file_name2[128], file_name3[128], file_name4[128], file_name5[128];
    char    file_name6[128], file_name7[128], file_name8[128], file_name9[128];
    char    file_name10[128];
    FILE    *fp = NULL;

    for (i = 0; i < SHM_MAX_SUB; i++) {
        if (INFO(i).date_flag == 9)
            continue;

        if ((D_K == -1 || D_K == i) && INFO(i).process_count != 0) {
            sprintf(sub, "%c%c", _Exe_Name[0], i + 'a');
            memcpy(Sub, sub, 2);
            LtoU(Sub, 2);
            memset(w_date, 0, sizeof (w_date));

            if (D_K == -1)
                memcpy(w_date, INFO(i).date, 8);
            else {
                if (INFO(i).process_count != 0)
                    memcpy(w_date, DAEMON(i).date, 8);
                else
                    memset(w_date, ' ', 8);
            }

            sprintf(file_name, "%s/%cZ/00000000", _FEP_DAT, _System_Name[0]);

            rt = stat(file_name, &_F_Info);

            if (rt == -1) {
                Create_Dir(file_name);
                Log(SYS_OK, "work date directory(%s) created", file_name);
            }

            sprintf(file_name2, "%s/%.2s/00000000", _FEP_DAT, Sub);

            rt = stat(file_name2, &_F_Info);

            if (rt == -1) {
                Create_Dir(file_name2);
                Log(SYS_OK, "data directory(%s) created", file_name2);
                sprintf(file_name3, "%s/%cZ/00000000/%.2s_date",
                        _FEP_DAT, _System_Name[0], sub);
                memset(buf, 0, sizeof (buf));

                fp = fopen(file_name3, "w");

                if (fp == NULL) {
                    Log(SAM_FATAL, "cannot open file1[%s] {%d:%s}",
                            file_name3, SYS_NO, SYS_STR);
                    exit(FAIL);
                }

                fwrite(w_date, 8, 1, fp);
                fflush(fp);
                fclose(fp); fp = NULL;
            }
            else {
                memset(buf, 0, sizeof (buf));
                sprintf(file_name4, "%s/%cZ/00000000/%.2s_date",
                        _FEP_DAT, _System_Name[0], sub);

                rt = stat(file_name4, &_F_Info);

                if (rt == -1) {
                    fp = fopen(file_name4, "w");

                    if (fp == NULL) {
                        Log(SAM_FATAL, "cannot open file2[%s] {%d:%s}",
                                file_name4, SYS_NO, SYS_STR);
                        exit(FAIL);
                    }

                    fwrite(w_date, 8, 1, fp);
                    fflush(fp);
                    fclose(fp); fp = NULL ;
                }

                sprintf(file_name10, "%s/%cZ/00000000/%.2s_date",
                        _FEP_DAT, _System_Name[0], sub);
                if (fp == NULL) {
                    fp = fopen(file_name10, "r+");

                    if (fp == NULL) {
                        Log(SAM_FATAL, "cannot open file[%s] {%d:%s}",
                                file_name10, SYS_NO, SYS_STR);
                        exit(FAIL);
                    }
                }

                fgets(data, 10, fp);

                if (memcmp(data, w_date, 8) != 0) {
                    if (Chk_Digit(data, 8) == OK) {
                        /* move data files  */
                        sprintf(buf, "mv %s/%.2s/00000000 %s/%.2s/%.8s",
                                _FEP_DAT, Sub, _FEP_DAT, Sub, data);

                        rt = system(buf);
#if defined(__hpux) || defined(sun) || defined(_AIX)
                        if (rt < 0) {
                            Log(SYS_FATAL, "system call failure [%s] 01 rt[%d]", buf, rt);
                            fclose(fp); fp = NULL ;
                            exit(FAIL);
                        }
#endif

                        /* move work date file  */
                        sprintf(file_name5, "%s/%cZ/%.8s",
                                _FEP_DAT, _System_Name[0], data);

                        rt = stat(file_name5, &_F_Info);

                        if (rt == -1) {
                            Create_Dir(file_name5);
                        }

                        sprintf(buf,
                                "mv %s/%cZ/00000000/%.2s_date %s/%cZ/%.8s",
                                _FEP_DAT, _System_Name[0], sub, _FEP_DAT,
                                _System_Name[0], data);

                        rt = system(buf);
#if defined(__hpux) || defined(sun) || defined(_AIX)
                        if (rt < 0) {
                            Log(SYS_FATAL, "system call failure[%s] 02 rt[%d]", buf, rt);
                            fclose(fp); fp = NULL ;
                            exit(FAIL);
                        }
#endif
                    }
                    else {
                        fwrite(w_date, 8, 1, fp);
                        fflush(fp);
                        fclose(fp); fp = NULL ;

                        if (D_K == i)
                            break;
                        else
                            continue;
                    }

                    fclose(fp);
                    sleep(2);
                    sprintf(file_name6, "%s/%.2s/00000000", _FEP_DAT, Sub);
                    Create_Dir(file_name6);
                    sprintf(file_name7, "%s/%cZ/00000000/%.2s_date",
                            _FEP_DAT, _System_Name[0], sub);
                    memset(buf, 0, sizeof (buf));

                    fp = fopen(file_name7, "w");

                    if (fp == NULL) {
                        Log(SAM_FATAL, "cannot open file4 [%s]", file_name7);
                        exit(FAIL);
                    }

                    fwrite(w_date, 8, 1, fp);
                    fflush(fp);
                    fclose(fp); fp = NULL ;

                    /* move log files   */
                    sprintf(file_name8, "%s/%.2s/00000000", _FEP_LOG, Sub);

                    rt = stat(file_name8, &_F_Info);

                    if (rt != -1) {
                        if (Chk_Digit(data, 8) == OK) {
                            sprintf(buf, "mv %s/%.2s/00000000 %s/%.2s/%.8s",
                                    _FEP_LOG, Sub, _FEP_LOG, Sub, data);

                            rt = system(buf);

#if defined(__hpux) || defined(sun) || defined(_AIX)
                            if (rt < 0) {
                                Log(SYS_FATAL, "system call failure[%s] 03 rt[%d]", buf, rt);
                                exit(FAIL);
                            }
                            else
                                Log(USR_OK, "system call OK 03");
#endif
                        }
                        else {
                            Log(USR_FATAL, "%.2s:invalid work date(%.8s)",
                                    Sub, data);
                            exit(FAIL);
                        }

                        sleep(2);
                    }

                    sprintf(file_name9, "%s/%.2s/00000000", _FEP_LOG, Sub);

                    rt = stat(file_name9, &_F_Info);

                    if (rt == -1) {
                        Create_Dir(file_name9);
                        Log(SYS_OK, "log directory(%s) created", file_name9);
                    }
                }
                else {
                    fclose(fp);
                    fp = NULL ;
                }
            }

            if (D_K == i)
                break;
        }
    }

    return;
}   /* End of Check_Work_Dir () */

/*************************************************************************
    Function        : . test for any decimal-digit character
    Parameters IN   : . str : string
                      . len : length of the string
    Parameters OUT  : .
    Return Code     : . 0 (OK: digit only), -1 (NOTOK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Chk_Digit(char *str, int len)
/*----------------------------------------------------------------------*/
{
    int     i;

    for (i = 0; i < len; i++) {
        if (!isdigit(str[i]))
            return (NOTOK);
    }

    return (OK);
}   /* End of Chk_Digit ()  */

/*************************************************************************
    Function        : . read configuration files and load SHM
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Shm_Conf_Process(int flag)
/*----------------------------------------------------------------------*/
{
    CFG_LOAD_RESULT     cfg_result;

    /* Config_Load_All: DB mode or INI fallback
       Loads file/dshm/tcp1/tcp2/udpip/proc into SHM.
       Daemon config is loaded separately in Main_Process_Memory. */
    cfg_result = Config_Load_All(flag);

    Log(USR_OK, "Shm_Conf_Process: source=%s keys=%d(flag=%d)",
            cfg_result.source == CFG_SRC_DB ? "DB" : "INI",
            cfg_result.total_keys, flag);

    return;
}   /* End of Shm_Conf_Process ()   */

/*************************************************************************
    End of Program (pz_memory_proc.c)
*************************************************************************/
