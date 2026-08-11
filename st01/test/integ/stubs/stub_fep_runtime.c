/*------------------------------------------------------------------------
#   Module  : Stub for FEP runtime globals and Log()
#   File    : stub_fep_runtime.c
#   Purpose : Provides minimal stubs so sub sources can link
#             in the integration test environment.
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/*------------------------------------------------------------------------
    Global variables referenced by sub/ sources
------------------------------------------------------------------------*/
char    _FEP_LOG[256]           = "/tmp";
char    _FEP_DAT[256]           = "/tmp";
char    _Exe_Name[64]           = "test_proc";
char    _SubSystem_Name[8]      = "PB";
char    _Process_Name[64]       = "test_integ";
int     SYS_NO                  = 0;    /* alias: errno is used directly via macro */

int     Mem_Shmid[8]            = {0};
int     D_K                     = 1;    /* PB = 1 */
int     P_K                     = 0;

/*------------------------------------------------------------------------
    Log() — print to stdout for test visibility
------------------------------------------------------------------------*/
void Log(int p_err_no, const char *p_fmt, ...)
{
    va_list ap;
    const char *prefix;

    if      (p_err_no < 100) prefix = "USR";
    else if (p_err_no < 200) prefix = "SYS";
    else if (p_err_no < 300) prefix = "PRO";
    else if (p_err_no < 400) prefix = "SAM";
    else if (p_err_no < 500) prefix = "FIF";
    else if (p_err_no < 600) prefix = "ORA";
    else if (p_err_no < 700) prefix = "TCP";
    else if (p_err_no < 800) prefix = "UDP";
    else                      prefix = "DSH";

    fprintf(stdout, "[%s:%03d] ", prefix, p_err_no);
    va_start(ap, p_fmt);
    vfprintf(stdout, p_fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
}

/*------------------------------------------------------------------------
    SLog() — structured log stub (no-op)
------------------------------------------------------------------------*/
void SLog(int p_err_no, const char *p_fmt, ...)
{
    (void)p_err_no;
    (void)p_fmt;
}

/*------------------------------------------------------------------------
    LtoU / UtoL — case conversion stubs (minimal)
    (Real versions in sub/ltou.c, sub/utol.c)
------------------------------------------------------------------------*/
void LtoU(char *p_str, int p_len)
{
    int i;
    for (i = 0; i < p_len && p_str[i]; i++) {
        if (p_str[i] >= 'a' && p_str[i] <= 'z')
            p_str[i] -= 32;
    }
}

void UtoL(char *p_str, int p_len)
{
    int i;
    for (i = 0; i < p_len && p_str[i]; i++) {
        if (p_str[i] >= 'A' && p_str[i] <= 'Z')
            p_str[i] += 32;
    }
}

/*------------------------------------------------------------------------
    AtoIf — simple ASCII-to-int (referenced by select_recv.c)
    (Real version in sub/atoif.c, but we provide a simple one here
     to avoid pulling in that dependency chain)
------------------------------------------------------------------------*/
int AtoIf(char *p_str, int p_len)
{
    int i, val = 0;
    for (i = 0; i < p_len; i++) {
        if (p_str[i] >= '0' && p_str[i] <= '9')
            val = val * 10 + (p_str[i] - '0');
    }
    return val;
}

/*------------------------------------------------------------------------
    End of stub_fep_runtime.c
------------------------------------------------------------------------*/
