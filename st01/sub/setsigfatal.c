/*------------------------------------------------------------------------
#   Module  : set fatal signal handlers
#   File    : setsigfatal.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"
#include    "fep_interface.h"

volatile sig_atomic_t _in_signal_handler = 0;

/*************************************************************************
    Function        : . set fatal signal handlers
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Setsigfatal(void)
/*----------------------------------------------------------------------*/
{
    signal(SIGINT, End_Routine);
    signal(SIGQUIT, End_Routine);
    signal(SIGILL, End_Routine);
    signal(SIGTERM, End_Routine);
    signal(SIGBUS, End_Routine);
    signal(SIGSEGV, End_Routine);
    signal(SIGHUP, End_Routine);

    return;
}   /* End of Setsigfatal ()    */

/*************************************************************************
    Function        : . process end routine
    Parameters IN   : . p_signo : signal number
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    End_Routine(int p_signo)
/*----------------------------------------------------------------------*/
{
    static const char msg_prefix[] = "[SIGNAL] caught signal: ";
    static const char msg_nl[] = "\n";
    static const char *sig_names[] = {
        [SIGINT]  = "SIGINT",
        [SIGQUIT] = "SIGQUIT",
        [SIGILL]  = "SIGILL",
        [SIGTERM] = "SIGTERM",
        [SIGBUS]  = "SIGBUS",
        [SIGSEGV] = "SIGSEGV",
        [SIGHUP]  = "SIGHUP"
    };
    const char *name;

    _in_signal_handler = 1;

    SIG_WRITE_MSG(msg_prefix);
    if (p_signo > 0 && p_signo < (int)(sizeof(sig_names)/sizeof(sig_names[0]))
            && sig_names[p_signo] != NULL) {
        name = sig_names[p_signo];
        {
            const char *p = name;
            size_t nlen = 0;
            while (p[nlen] != '\0') nlen++;
            write(STDERR_FILENO, name, nlen);
        }
    }
    SIG_WRITE_MSG(msg_nl);

    Exit_Process();
    _exit(FAIL);
}   /* End of End_Routine ()    */

/*************************************************************************
    Function        : . initialize process status and exit process
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Exit_Process(void)
/*----------------------------------------------------------------------*/
{
    char    bumun[4];

    if (D_K != -1) {
        if (DD_K != -1)             /* sub daemons (e.g. pa_daemon_mp)  */
            INFO(DD_K).process_no = 0;

        if (P_K != -1) {
            if (PROC(D_K,P_K).type == TY_TRS2)                  /* TCP2 */ {
                if (PROC(D_K,P_K).l.t2.l[1] != 0 && PROC(D_K,P_K).backup == 1) {                                   /* primary + backup */
                        TCP2_LSTAT(D_K,P_K,0) = 0;
                    TCP2_PSTAT(D_K,P_K,0) = 0;

                    TCP2_LSTAT(D_K,P_K,1) = 0;
                    TCP2_PSTAT(D_K,P_K,1) = 0;
                    PROC(D_K,P_K).process_no = 0;
                }
                else {                                       /* primary only */
                    TCP2_LSTAT(D_K,P_K,0) = 0;
                    TCP2_PSTAT(D_K,P_K,0) = 0;
                    PROC(D_K,P_K).process_no = 0;
                }
            }
            else
                PROC(D_K,P_K).process_no = 0;

            if (PROC(D_K,P_K).process_status == 1 &&        /* 1: run   */
                    PROC(D_K,P_K).start_status != 2)            /* 2: end   */ {
                if (write(DTART_FD, "1", 1) == -1) {
                    if (!_in_signal_handler)
                        Log(SYS_WARN, "daemon FIFO write failed!!");
                }
            }
        }
    }

    if (!_in_signal_handler) {
        sprintf(bumun, "%s", _SubSystem_Name);
        LtoU(bumun, 2);

        if (DD_K == -1 && bumun[1] != 'W' && bumun[1] != 'X' &&
                bumun[1] != 'Y' && bumun[1] != 'Z') {
            if (PROC(D_K,P_K).process_status == 4)
                PROC(D_K,P_K).process_status = 2;
            else if (PROC(D_K,P_K).process_status == 8)     /* TR1, TS1 */
                PROC(D_K,P_K).process_status = 9;
            else
                Stat_Save_Force();
        }

        /* F3 seq-save-batch: 스로틀로 지연된 커서를 종료 전 최종 저장
           (시그널 컨텍스트가 아니므로 stdio 사용 안전) */
        Seq_Save_Flush();

        Log(PRO_OK, "process STOP");
        exit(FAIL);
    }

    _exit(FAIL);
}   /* End of Exit_Process ()   */

/*************************************************************************
    End of Program (setsigfatal.c)
*************************************************************************/
