/*************************************************************************
    Module      : . hot-path log router (F2 hotpath-log-slog)
    File        : . log_hot.c
    Comment     : . route per-message INFO logs to SLog (SHM ring) when
                    FEP_HOT_LOG=shm, otherwise to legacy Log (default)
                  . C89: no variadic macro - pre-format then delegate
                  . error/FATAL levels must keep calling Log directly
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define LOG_HOT_BUF     4096

extern  void    Log(int, const char *, ...);
extern  void    SLog(int, const char *, ...);

/*************************************************************************
    Function        : . Log_Hot_Mode
    Parameters IN   : .
    Return Code     : . int : 0=file(Log, 기본), 1=shm(SLog)
    Comment         : . FEP_HOT_LOG 환경변수 1회 평가 후 캐시
                        (is_structured_log 패턴)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Log_Hot_Mode(void)
/*----------------------------------------------------------------------*/
{
    static int  mode = -1;
    char        *env;

    if (mode == -1) {
        env = getenv("FEP_HOT_LOG");
        if (env != NULL && strcmp(env, "shm") == 0)
            mode = 1;
        else
            mode = 0;
    }

    return (mode);
}   /* End of Log_Hot_Mode ()   */

/*************************************************************************
    Function        : . Log_Hot
    Parameters IN   : . p_err_no : error/status code (Log과 동일)
                      . p_fmt    : printf format + 가변인자
    Return Code     : . void
    Comment         : . 선포맷 후 모드에 따라 Log/SLog에 "%s"로 위임
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Hot(int p_err_no, const char *p_fmt, ...)
/*----------------------------------------------------------------------*/
{
    va_list ap;
    int     rt;
    char    buf[LOG_HOT_BUF];

    va_start(ap, p_fmt);
    rt = vsnprintf(buf, sizeof (buf), p_fmt, ap);
    va_end(ap);

    if (rt < 0 || rt >= LOG_HOT_BUF)
        buf[LOG_HOT_BUF - 1] = '\0';

    if (Log_Hot_Mode() == 1)
        SLog(p_err_no, "%s", buf);
    else
        Log(p_err_no, "%s", buf);

    return;
}   /* End of Log_Hot ()    */

/*************************************************************************
    End of Program (log_hot.c)
*************************************************************************/
