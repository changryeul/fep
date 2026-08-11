/*------------------------------------------------------------------------
#   Stub      : Log / SLog 캡처 스텁 (test_log_hot_* 용)
#   File      : stubs/stub_log.c
#
#   Log_Hot 라우팅 검증: 마지막 호출이 Log(1)인지 SLog(2)인지,
#   err_no와 최종 포맷된 메시지를 기록한다.
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>

int     stub_log_called = 0;        /* 0=none, 1=Log, 2=SLog */
int     stub_log_errno  = -1;
char    stub_log_msg[1024];

void    Log(int p_err_no, const char *p_fmt, ...)
{
    va_list ap;

    stub_log_called = 1;
    stub_log_errno  = p_err_no;
    va_start(ap, p_fmt);
    vsnprintf(stub_log_msg, sizeof(stub_log_msg), p_fmt, ap);
    va_end(ap);
}

void    SLog(int p_err_no, const char *p_fmt, ...)
{
    va_list ap;

    stub_log_called = 2;
    stub_log_errno  = p_err_no;
    va_start(ap, p_fmt);
    vsnprintf(stub_log_msg, sizeof(stub_log_msg), p_fmt, ap);
    va_end(ap);
}
