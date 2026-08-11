/*------------------------------------------------------------------------
#   Module  : main routine of sub daemon
#   File    : pz_daemon_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "sub_daemon.h"
#include    <sys/wait.h>
#include    <sys/param.h>

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int             Sig_No;                 /* signal number                */
int             Tmp_SS;                 /* current time (sec)           */
int             Kill_Flag;              /* pz_fepp_mp USR2(1), sub daemon(0) */
int             StopProc;
char            Bumun[2];               /* sub system name (lowercase)  */
struct pollfd   Poll;                   /* poll file descriptor         */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Mem_SHM(int, int);
void    Sise_SHM(void);
void    Daemon_Config_Read(int);
void    Sub_SHM(void);

void    Main_Process_Daemon(void);
int     Daemon_SHM_Process(void);
int     File_Check_Process(void);
void    File_Compact_Process(void);
void    SHM_Load_Process(void);
void    SHM_Backup_Process(void);
void    User_Signal(void);
void    Start_Process(void);
void    Stop_Process(void);
void    Sig_Handler(int);
void    User_Sleep(int);
void    Sys_Get_Time(void);

/*************************************************************************
    Function        : . main routine
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Main_Process_Daemon(void)
/*----------------------------------------------------------------------*/
{
    int     rt, poll_flag, poll_errno;
    char    tmp[10];

    memset(Bumun, 0, sizeof (Bumun));

    signal(SIGUSR1, Sig_Handler);
    signal(SIGUSR2, Sig_Handler);

    poll_flag = SYS_NO = 0;
    StopProc = 0;

    UtoL(_SubSystem_Name, 2);
    Bumun[0] = _SubSystem_Name[1];                          /* a ~ w    */

    signal(SIGCHLD, SIG_DFL);

    if (INFO(D_K).system_status == 0) {
        /* execute memory_mp    */
        while (1) {
            rt = Daemon_SHM_Process();
            if (rt == FAIL) {
                Log(SAM_FATAL, "ini files have problem !!! _proc main line[%d]", __LINE__);
                User_Sleep(30);
                continue;
            }
            else {
                User_Sleep(1);
                Log(USR_OK, "%c%c SHM created and loaded 01",
                        _System_Name[0], D_K + 'A');
                break;
            }
        }
    }

    if (INFO(D_K).process_status == 0)
        Exit_Process();

    /* attach sub SHM (Shm_Mem) */
    Mem_SHM(1, D_K);

    /* attach sise data SHM */
    /* 2025 sisetr_count���
        if (DAEMON(D_K).sisetr_count > 0) {
            Sise_SHM ();
            Log(USR_OK, "attach Shm_Lg[0]");
        }
    */
    Sise_SHM();
    Log(USR_OK, "attach Shm_Lg[0]");

    /* get the process ID and set pid of sub daemon SHM */
    DAEMON(D_K).process_no = getpid();
    Log(USR_OK, "DAEMON [%d,%d,%d,%s]",
            DAEMON(D_K).p_count, DAEMON(D_K).process_no,
            DAEMON(D_K).date_flag, DAEMON(D_K).date);

    Sys_Get_Time();

    Log(USR_OK, "start:%s(%d) end:%s(%d)",
            DAEMON(D_K).start_time, Start_SS, DAEMON(D_K).end_time, End_SS);

    /* execute filechk_mp   */
    while (1) {
        rt = File_Check_Process();
        if (rt == FAIL) {
            Log(SAM_FATAL, "data files have problem !!!");
            User_Sleep(30);
            continue;
        }
        else {
            User_Sleep(1);
            Log(USR_OK, "integrity of data files checked");
            break;
        }
    }

    signal(SIGCHLD, SIG_IGN);

    /* execute compact_mp   */
    File_Compact_Process();
    User_Sleep(1);
    Log(USR_OK, "data, log and FIFO unlinked");

    /* execute pz_shmload_mp    */
    /* park
        SHM_Load_Process ();
        User_Sleep (1);
        Log (USR_OK, "OK: load sise shared memory data");
    */

    while (1) {
        Sys_Get_Time();

        /*----------------------------------------------------------*/
        /* date_flag (1:D~D 2:D~D+1 3:D+1~D+1 4:D-1~D 5:D-1~D-1     */
        /*  9:run continuously)                                     */
        /*----------------------------------------------------------*/
        if (DAEMON(D_K).date_flag == 9)         /*  run continuously    */ {
            Log(USR_OK, "Start 001");  /* KSW */
            Start_Process();
            break;
        }
        else if (DAEMON(D_K).date_flag == 2 ||
                DAEMON(D_K).date_flag == 4)             /* run in two days  */ {
            /* in business hours    */
            if ((Tmp_SS >= Start_SS && Tmp_SS < 24 * 60 * 60) ||
                    (Tmp_SS >= 0 && Tmp_SS < End_SS)) {
                Log(USR_OK, "Start 002");  /* KSW */
                Start_Process();
                break;
            }
        }
        else                /* run in a day (date_flag is 1, 3 or 5)    */ {
            if (Start_SS < Tmp_SS && Tmp_SS < End_SS) {
                Log(USR_OK, "Start 003");  /* KSW */
                Start_Process();
                break;
            }
        }

        if (DAEMON(D_K).date_flag != 9 && End_SS < Tmp_SS) {
            Log(PRO_WARN, "end time[%d sec] <= current time[%d sec]",
                    End_SS, Tmp_SS);
            User_Sleep(5);

            INFO(D_K).process_status = 2;                       /* end  */
            DAEMON(D_K).process_status = 2;
            Stop_Process();
            User_Sleep(5);

            INFO(D_K).system_status = 0;
            DAEMON(D_K).system_status = 0;

            /* read daemon.ini and set the daemon SHM (INFO)    */
            Daemon_Config_Read(3);
            INFO(D_K).process_status = 2;
            DAEMON(D_K).process_status = 2;
            User_Sleep(1);
            Exit_Process();
        }

        Log(USR_OK, "sleeping... sleep time[%d sec]", Start_SS - Tmp_SS + 10);
        User_Sleep(Start_SS - Tmp_SS + 10);
    }

    while (1) {
        Sys_Get_Time();

        Poll.fd = DTART_FD;
        Poll.events = POLLIN;

        rt = poll(&Poll, 1, 3 * 1000);
        poll_errno = errno;

        User_Signal();

        if (rt == 0) {
            poll_flag ++;
            if (poll_flag < 10)
                continue;

            poll_flag = 0;
        }
        else if (rt < 0) {
            if (poll_errno == EINTR) {
                Log(SYS_OK, "poll:signal was delivered {%d:%s}",
                        poll_errno, strerror(poll_errno));
            }
            else {
                Log(SYS_ERROR, "poll failed {%d:%s}",
                        poll_errno, strerror(poll_errno));
                User_Sleep(3);
            }
            continue;
        }

        if (Poll.revents & POLLHUP) {
            Log(SYS_ERROR, "poll hangup occurred");
            User_Sleep(3);
            continue;
        }

        if (Poll.revents & POLLIN)
            Poll.revents = 0;

        while (1) {
            rt = (int)read(DTART_FD, tmp, sizeof(tmp));
#if defined __linux
            if (rt == 0 || errno == EAGAIN)
#else
                if (rt == 0)
#endif
                break;
        }

        usleep(200000);
        Log(USR_OK, "Start 004");  /* KSW */
        Start_Process();

        sleep(1);
        /* park
                SHM_Backup_Process ();
        */
    }

    return;
}   /* End of Main_Process_Daemon () */

/*************************************************************************
    Function        : . execute memory_mp
                        - create and load shared memory
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0(OK):normal, 1(FAIL):file error)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Daemon_SHM_Process(void)
/*----------------------------------------------------------------------*/
{
    int     process_no, rt, status, fd;
    char    path[256], process_id[20], full_path[300];

    memset(path,       0, sizeof(path));
    memset(full_path,  0, sizeof(full_path));

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);

    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory[%s] {%d:%s}",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    process_no = fork();

    switch (process_no) {
        case    NOTOK:
            Log(SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
            exit(FAIL);
        case    OK:
            memset(process_id, 0, sizeof(process_id));
            sprintf(process_id, "%cz_memory_mp", _Exe_Name[0]);
            snprintf(full_path, sizeof(full_path), "%s/%s", path, process_id);
            strncpy(path, full_path, sizeof(path));
            path[sizeof(path)-1] = '\0';

            for (fd = 0; fd < NOFILE; fd ++)
                close(fd);

            execl(path, process_id, Bumun, (char *)NULL);
            Log(SYS_ERROR, "execl(%s,%s,%s) failure[%s] {%d:%s} line[%d]",
                    path, process_id, Bumun, "create and load shared memory",
                    SYS_NO, SYS_STR, __LINE__);
            exit(FAIL);
        default:
            rt = wait((int *)&status);
    }

    return (WEXITSTATUS(status));
}   /* End of Daemon_SHM_Process () */

/*************************************************************************
    Function        : . execute filechk_mp
                        - check the integrity of data files
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int (0(OK):normal, 1(FAIL):file error)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     File_Check_Process(void)
/*----------------------------------------------------------------------*/
{
    int     process_no, rt, status, fd;
    char    path[256], process_id[20], full_path[300];

    memset(path,       0, sizeof(path));
    memset(full_path,  0, sizeof(full_path));

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);

    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory[%s] {%d:%s}",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    process_no = fork();

    switch (process_no) {
        case    NOTOK:
            Log(SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
            exit(FAIL);
        case    OK:
            sprintf(process_id, "%cz_filechk_mp", _Exe_Name[0]);
            snprintf(full_path, sizeof(full_path), "%s/%s", path, process_id);
            strncpy(path, full_path, sizeof(path));
            path[sizeof(path)-1] = '\0';

            Log(USR_OK, "execute[%s %s]", process_id, Bumun);

            for (fd = 0; fd < NOFILE; fd ++)
                close(fd);

            execl(path, process_id, Bumun, (char *)NULL);
            Log(SYS_ERROR, "execl(%s,%s,%s) failure[%s] {%d:%s} line[%d]",
                    path, process_id, Bumun,
                    "unlink the expired data, log and FIFO", SYS_NO, SYS_STR, __LINE__);
            exit(FAIL);
        default:
            wait(&status);
    }

    return (WEXITSTATUS(status));
}   /* End of File_Check_Process () */

/*************************************************************************
    Function        : . execute pz_compact_mp
                        - unlink the expired data, log and FIFO
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    File_Compact_Process(void)
/*----------------------------------------------------------------------*/
{
    int     process_no, rt, status, fd;
    char    path[256], process_id[20], comp_day[4], full_path[300];

    memset(path,       0, sizeof(path));
    memset(full_path,  0, sizeof(full_path));

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);

    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory [%s] {%d:%s}",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    process_no = fork();

    switch (process_no) {
        case    NOTOK:
            Log(SYS_FATAL, "fork failure {%d:%s}", SYS_NO, SYS_STR);
            exit(FAIL);
        case    OK:
            sprintf(process_id, "%cz_compact_mp", _Exe_Name[0]);
            snprintf(full_path, sizeof(full_path), "%s/%s", path, process_id);
            strncpy(path, full_path, sizeof(path));
            path[sizeof(path)-1] = '\0';

            sprintf(comp_day, "%d", INFO(D_K).compact_days);
            Log(USR_OK, "execute[%s %s %s]", process_id, comp_day, Bumun);

            for (fd = 0; fd < NOFILE; fd ++)
                close(fd);

            execl(path, process_id, comp_day, Bumun, (char *)NULL);
            Log(SYS_ERROR, "execl(%s,%s,%s,%s) failure[%s] {%d:%s} line[%d]",
                    path, process_id, comp_day, Bumun,
                    "unlink the expired data and log files", SYS_NO, SYS_STR,  __LINE__);
            exit(FAIL);
        default:
            wait(&status);
    }

    return;
}   /* End of File_Compact_Process ()   */

/**************************************************************************
   Functions      : . SHM Load Process �⵿
   Parameters IN  : .
   Parameters OUT : .
   Return Code    : . void
**************************************************************************/
/*-----------------------------------------------------------------------*/
void    SHM_Load_Process(void)
/*-----------------------------------------------------------------------*/
{
    int     rt, status, process_no, fd;
    char    path[256], process_id[20], full_path[300];

    memset(path,       0, sizeof(path));
    memset(full_path,  0, sizeof(full_path));

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);
    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory [%s] [%d:%s]",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    /* SHM Load Process �⵿ */
    process_no = fork();

    switch (process_no) {
        case  NOTOK :      /* fork fail */
            Log(SYS_FATAL, "fork failure[%d:%s]", SYS_NO, SYS_STR);
            exit(FAIL);

        case   OK :
            sprintf(process_id, "%cz_shmload_mp", _Exe_Name[0]);
            snprintf(full_path, sizeof(full_path), "%s/%s", path, process_id);
            strncpy(path, full_path, sizeof(path));
            path[sizeof(path)-1] = '\0';

            Log(USR_OK, "execute[%s %s]", process_id, Bumun);

            for (fd = 0; fd < NOFILE; fd ++)
                close(fd);

            execl(path, process_id, Bumun, (char *)NULL);
            Log(SYS_ERROR, "execl(%s,%s,%s) failure[%s][%d:%s] line[%d]",
                    path, process_id, Bumun,
                    "unlink the expired data, log and FIFO", SYS_NO, SYS_STR, __LINE__);
            exit(FAIL);

        default:
            wait(&status);
    }

    return;
}/* end of SHM_Load_Process () */

/**************************************************************************
   Functions      : . SHM Backup Process �⵿
   Parameters IN  : .
   Parameters OUT : .
   Return Code    : . void
**************************************************************************/
/*-----------------------------------------------------------------------*/
void    SHM_Backup_Process(void)
/*-----------------------------------------------------------------------*/
{
    int     process_no, rt, status, fd;
    char    path[256], process_id[20], full_path[300];

    memset(path,       0, sizeof(path));
    memset(full_path,  0, sizeof(full_path));

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);
    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory [%s] [%d:%s]",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    /* SHM Backup Process �⵿ */
    process_no = fork();

    switch (process_no) {
        case  NOTOK :      /* fork fail */
            Log(SYS_FATAL, "fork failure[%d:%s]", SYS_NO, SYS_STR);
            exit(FAIL);

        case   OK :
            sprintf(process_id, "%cz_shmback_mp", _Exe_Name[0]);
            snprintf(full_path, sizeof(full_path), "%s/%s", path, process_id);
            strncpy(path, full_path, sizeof(path));
            path[sizeof(path)-1] = '\0';

            Log(USR_OK, "execute[%s %s]", process_id, Bumun);
            for (fd = 0; fd < NOFILE; fd ++)
                close(fd);

            execl(path, process_id, Bumun, (char *)NULL);
            Log(SYS_ERROR, "execl(%s,%s,%s) failure[%s][%d:%s] line[%d]",
                    path, process_id, Bumun,
                    "unlink the expired data, log and FIFO", SYS_NO, SYS_STR,  __LINE__);
            exit(FAIL);

        default:
            wait(&status);
    }

    return;
}   /* end of SHM_Backup_Process () */

/*************************************************************************
    Function        : . start or stop the processes when a signal was caught
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    User_Signal(void)
/*----------------------------------------------------------------------*/
{
    int     rt, pk;
    char    tmp[10];

    if (Sig_No == SIGUSR1)              /* = 16 (user defined signal 1) */ {
        rt = (int)read(EXIT_FD, tmp, sizeof(tmp));
        if (rt > 0)                     /* killed by super daemon   */ {
            /* re-attach daemon SHM after super daemon re-starts    */
            SHM_Detach((char *)SHM_All_Daemon_Info);
            Sub_SHM();
            Log(USR_OK, "daemon SHM(INFO) re-attached rt[%d]", rt);
        }
        else                                        /* killed by user   */ {
            INFO(D_K).process_status = 3;                       /* stop */
            DAEMON(D_K).process_status = 3;

            Stop_Process();
            Log(USR_OK, "%c%c processes stopped", _System_Name[0], D_K + 'A');

            SHM_Detach((char *)SHM_Mem[D_K]);
            Log(USR_OK, "%c%c SHM detached", _System_Name[0], D_K + 'A');

            /* execute memory_mp    */
            /* 2025EDIT Update */
            {
                sigset_t set;
                sigemptyset(&set);
                sigaddset(&set, SIGCHLD);
                sigprocmask(SIG_UNBLOCK, &set, NULL);
            }
            /* 2025EDIT Update */

            Log(USR_OK, "execute memory_mp");

            while (1) {
                rt = Daemon_SHM_Process();
                if (rt == FAIL) {
                    Log(SAM_FATAL, "ini files have problem !!! _proc user line[%d]", __LINE__);
                    User_Sleep(30);
                    continue;
                }
                else {
                    User_Sleep(1);
                    Log(USR_OK, "%c%c SHM created and loaded 02",
                            _System_Name[0], D_K + 'A');
                    break;
                }
            }

            signal(SIGCHLD, SIG_IGN);

            Mem_SHM(1, D_K);
            Log(USR_OK, "%c%c SHM attached", _System_Name[0], D_K + 'A');

            /* get the process ID and set pid of sub daemon SHM */
            DAEMON(D_K).process_no = getpid();
            INFO(D_K).process_no = getpid();
            Log(USR_OK, "process_no reset[%d]", DAEMON(D_K).process_no);

            /* make pz_procchk_mp re-attach sub SHM */
            INFO(D_K).check_status = 1;

            Start_Process();
        }

        User_Sleep(5);
        Sig_No = 0;
        return;
    }
    /* killed by user or super daemon   */
    else if (Sig_No == SIGUSR2)         /* = 17 (user defined signal 2) */ {
        INFO(D_K).process_status = 3;                           /* stop */
        DAEMON(D_K).process_status = 3;

        Stop_Process();
        Log(USR_OK, "%c%c processes stopped when killed",
                _System_Name[0], D_K + 'A');

        User_Sleep(5);
        Sig_No = 0;

        rt = read(EXIT_FD, tmp, sizeof(tmp));
        if (rt > 0)                     /* killed by super daemon   */ {
            Log(USR_OK, "killed by super daemon");

            INFO(D_K).system_status = 0;
            DAEMON(D_K).system_status = 0;
            Exit_Process();
        }
    }

    /* run in two days  */
    if (DAEMON(D_K).date_flag == 2 || DAEMON(D_K).date_flag == 4) {
        /* out of business hours    */
        if (Tmp_SS > End_SS && Tmp_SS < Start_SS) {
            INFO(D_K).process_status = 2;
            DAEMON(D_K).process_status = 2;
            Stop_Process();
            User_Sleep(5);

            INFO(D_K).system_status = 0;
            DAEMON(D_K).system_status = 0;

            /* read daemon.ini and set the daemon SHM (INFO)    */
            Daemon_Config_Read(3);
            INFO(D_K).process_status = 2;
            DAEMON(D_K).process_status = 2;
            User_Sleep(1);
            Exit_Process();
        }
    }
    /* run in a day */
    else if (DAEMON(D_K).date_flag == 1 || DAEMON(D_K).date_flag == 3 ||
            DAEMON(D_K).date_flag == 5) {
        if (End_SS < Tmp_SS) {
            INFO(D_K).process_status = 2;
            DAEMON(D_K).process_status = 2;
            Stop_Process();
            User_Sleep(5);

            INFO(D_K).system_status = 0;
            DAEMON(D_K).system_status = 0;

            /* read daemon.ini and set the daemon SHM (INFO)    */
            Daemon_Config_Read(3);
            INFO(D_K).process_status = 2;
            DAEMON(D_K).process_status = 2;
            User_Sleep(1);
            Exit_Process();
        }
    }

    return;
}   /* End of User_Signal ()    */

/*************************************************************************
    Function        : . execute processes
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Start_Process(void)
/*----------------------------------------------------------------------*/
{
    int         pk, k, rt, flag_p, flag_b, flag_e, flag_c, flag_ap, flag_ab;
    int         tmp_ss, start_ss, end_ss, fd;
    long        proc_no, proc_x_no;
    char        path[256], proc[40], info[50], cmd[128];
    time_t      sys_time;
    struct tm   *date;

    flag_p = flag_b = flag_e = flag_c = flag_ap = flag_ab = 1;
    UtoL(_SubSystem_Name, 2);

    sprintf(path, "%s", _FEP_BIN);

    rt = chdir(path);
    if (rt != 0) {
        Log(SYS_FATAL, "cannot change directory[%s] {%d:%s}",
                path, SYS_NO, SYS_STR);
        Exit_Process();
    }

    for (pk = 0; pk < DAEMON(D_K).p_count; pk ++) {
        if (PROC(D_K,pk).start_status == JOB_END)
            continue;

        flag_p = 1;
        flag_b = 1;
        flag_e = 1;
        flag_c = 1;
        flag_ap = 1;
        flag_ab = 1;

        if (strlen(PROC(D_K,pk).process_id) == 0)
            continue;

        /* check the process operating time */
        time(&sys_time);
        date = localtime(&sys_time);
        tmp_ss = (date->tm_hour * 60 * 60) +
        (date->tm_min * 60) + date->tm_sec;         /* current time */

        start_ss = AtoIf(PROC(D_K,pk).start_time, 2) * 60 * 60 +
        AtoIf(PROC(D_K,pk).start_time+2, 2) * 60;  /* start time   */
        end_ss = AtoIf(PROC(D_K,pk).end_time, 2) * 60 * 60 +
        AtoIf(PROC(D_K,pk).end_time+2, 2) * 60;        /* end time */

        /* out of business hours    */
        if ((start_ss > end_ss && (tmp_ss < start_ss && tmp_ss > end_ss)) ||
                (start_ss < end_ss && (tmp_ss < start_ss || tmp_ss > end_ss))) {
            proc_no = PROC(D_K,pk).process_no;

            if (proc_no != 0) {
                Log(SYS_OK, "STOP [%d,%d,%s:%s,%s,%s]",
                        proc_no, PROC(D_K,pk).process_no, PROC(D_K,pk).process_id,
                        PROC(D_K,pk).start_time, PROC(D_K,pk).end_time,
                        PROC(D_K,pk).process_info);

                rt = kill(proc_no, SIGTERM);

                if (rt == -1)
                    Log(SYS_OK, "kill failure(%d,%s) {%d:%s}",
                            proc_no, PROC(D_K,pk).process_id,
                            SYS_NO, SYS_STR);

                PROC(D_K,pk).process_no = 0;
            }

            continue;
        }

        if (PROC(D_K,pk).process_status != 1) {
            if ( (Sig_No == SIGUSR1 || StopProc == 2) && PROC(D_K,pk).process_status == 2) /* KSW */ {
                PROC(D_K,pk).process_status = 1;
                Log(USR_OK, "[Start_Process] process_status: 2->1");  /* KSW */
            }
            else
                continue;
        }
        else if (PROC(D_K,pk).process_no != 0)
            continue;

        if (PROC(D_K,pk).type == TY_TRS1 ||                 /* TCP/IP 1 */
                PROC(D_K,pk).type == TY_TRS2 ||                 /* TCP/IP 2 */
                PROC(D_K,pk).type == TY_BRS ||                      /* DB   */
                PROC(D_K,pk).type == TY_URS ||                      /* UDP  */
                PROC(D_K,pk).type == TY_MP ||                       /* MP   */
                PROC(D_K,pk).type == TY_DD)                         /* DD   */ {
            sprintf(path, "%s", PROC(D_K,pk).process_path);
            sprintf(proc, "%s", PROC(D_K,pk).process_id);
            sprintf(info, "%s", PROC(D_K,pk).process_info);
        }
        else
            continue;

        while (1) {
            rt = fork();

            switch (rt) {
                case    NOTOK:
                    Log(SYS_ERROR, "fork failure {%d:%s}",
                            SYS_NO, SYS_STR);
                    User_Sleep(5);
                    continue;
                    break;
                case    OK:
                    Log(SYS_OK, "START [%s,%s] [%.4s]", proc, info, PROC(D_K,pk).start_time);

                    for (fd = 0; fd < NOFILE; fd ++)
                        close(fd);

                    execl(path, proc, (char *)NULL);
                    Log(SYS_ERROR, "execl(%s,%s) failure {%d:%s} line[%d]",
                            path, proc, SYS_NO, SYS_STR,  __LINE__);
                    exit(FAIL);
                    break;
                default:
                    usleep(100000);
                    break;
            }

            break;
        }
    }

    return;
}   /* End of Start_Process ()  */

/*************************************************************************
    Function        : . stop processes
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Stop_Process(void)
/*----------------------------------------------------------------------*/
{
    int     pk, rt;
    long    proc_no;

    /* 2025EDIT */
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGPOLL);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigprocmask(SIG_BLOCK, &set, NULL);
    }
    /* 2025EDIT Update */

    UtoL(_SubSystem_Name, 2);

    for (pk = 0; pk < DAEMON(D_K).p_count; pk ++) {
        if ( (Sig_No == SIGUSR2 || StopProc == 1) && PROC(D_K,pk).process_status == 1) /* KSW */ {
            PROC(D_K,pk).process_status = 2;
            Log(USR_OK, "[Stop_Process] process_status: 1->2");  /* KSW */
        }

        proc_no = PROC(D_K,pk).process_no;

        Log(USR_OK, "[Stop_Process] proc_no:[%d] process_status:[%d]", proc_no, PROC(D_K,pk).process_status );  /* KSW */

        if (proc_no != 0) {
            Log(SYS_OK, "STOP [%d,%d,%s:%s]", proc_no, PROC(D_K,pk).process_no,
                    PROC(D_K,pk).process_id, PROC(D_K,pk).process_info);

            rt = kill(proc_no, SIGTERM);

            if (rt == -1)
                Log(SYS_ERROR, "kill failure[%d,%s] {%d:%s}",
                        proc_no, PROC(D_K,pk).process_id, SYS_NO, SYS_STR);

            PROC(D_K,pk).process_no = 0;
        }

        usleep(10000);
    }

    /* 2025EDIT */
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGPOLL);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
    }
    /* 2025EDIT */

    return;
}   /* End of Stop_Process ()   */

/*************************************************************************
    Function        : . set signal handler
    Parameters IN   : . p_signo : signal number
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Sig_Handler(int p_signo)
/*----------------------------------------------------------------------*/
{
    Sig_No = p_signo;

    switch (p_signo) {
        case    SIGUSR1:
            SIG_WRITE_MSG("[SIGNAL] pa_daemon_proc caught SIGUSR1\n");
            signal(SIGUSR1, Sig_Handler);
            break;
        case    SIGUSR2:
            SIG_WRITE_MSG("[SIGNAL] pa_daemon_proc caught SIGUSR2\n");
            signal(SIGUSR2, Sig_Handler);
            break;
        default:
            SIG_WRITE_MSG("[SIGNAL] pa_daemon_proc caught signal\n");
            signal(p_signo, Sig_Handler);
            break;
    }

    if (p_signo == SIGUSR1) {
        if (write(DTART_FD, "1", 1) != 1)
            Log(FIF_ERROR, "write to DTART_FD failed {%d:%s}", SYS_NO, SYS_STR);
    }

    return;
}   /* End of Sig_Handler ()    */

/*************************************************************************
    Function        : . User_Sleep
    Parameters IN   : . p_sec   : sleep time (sec)
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    User_Sleep(int p_sec)
/*----------------------------------------------------------------------*/
{
    usleep(10000);
    sleep(p_sec);

    return;
}   /* End of User_Sleep () */

/*************************************************************************
    Function        : . get system time
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Sys_Get_Time(void)
/*----------------------------------------------------------------------*/
{
    time_t      sys_time;
    struct tm   *date;

    time(&sys_time);
    date = localtime(&sys_time);

    Tmp_SS = (date->tm_hour * 60 * 60) + (date->tm_min * 60) + date->tm_sec;

    Start_HH = AtoIf(DAEMON(D_K).start_time, 2);
    Start_MM = AtoIf(DAEMON(D_K).start_time+2, 2);
    Start_SS = Start_HH * 60 * 60 + Start_MM * 60;
    End_HH = AtoIf(DAEMON(D_K).end_time, 2);
    End_MM = AtoIf(DAEMON(D_K).end_time+2, 2);
    End_SS = End_HH * 60 * 60 + End_MM * 60;

    return;
}   /* End of Sys_Get_Time ()   */

/************************************************************************
    End of Program (pz_daemon_proc.c)
************************************************************************/
