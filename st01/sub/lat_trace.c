/*************************************************************************
    Module      : . order latency trace (F1 order-latency-metrics)
    File        : . lat_trace.c
    Comment     : . low-overhead latency trace for measurement builds
                  . NOT the Log() path: dedicated stdio stream with
                    64KB full buffering, flush every LAT_FLUSH_CNT events
                  . output: $FEP_LAT_DIR/<proc>.lat (default dir ".")
                  . line  : <epoch_usec>|<proc>|<point>|<key>
                  . single-threaded event-loop processes only
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define LAT_BUF_SIZE    65536
#define LAT_FLUSH_CNT   128
#define LAT_PROC_LEN    32

static  FILE    *Lat_fp = NULL;
static  char    Lat_proc[LAT_PROC_LEN];
static  int     Lat_cnt = 0;
static  int     Lat_flush_n = LAT_FLUSH_CNT;    /* FEP_LAT_FLUSH로 조절 */

/*************************************************************************
    Function        : . Lat_Init
    Parameters IN   : . p_proc : process name (argv[0] ok, basename used)
    Return Code     : . void
    Comment         : . open trace file; on any failure stay disabled
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Lat_Init(const char *p_proc)
/*----------------------------------------------------------------------*/
{
    const char  *dir;
    const char  *base;
    char        path[512];

    if (p_proc == NULL || p_proc[0] == '\0')
        return;

    /* basename */
    base = strrchr(p_proc, '/');
    if (base != NULL)
        base = base + 1;
    else
        base = p_proc;

    strncpy(Lat_proc, base, LAT_PROC_LEN - 1);
    Lat_proc[LAT_PROC_LEN - 1] = '\0';

    dir = getenv("FEP_LAT_DIR");
    if (dir == NULL || dir[0] == '\0')
        dir = ".";

    /* flush 주기 조절 (측정 하니스에서 프로세스 kill 시 버퍼 유실 방지용).
       미설정이면 기본 128건. 1로 두면 매 이벤트 flush. */
    {
        const char *fn = getenv("FEP_LAT_FLUSH");
        if (fn != NULL && fn[0] != '\0') {
            int v = atoi(fn);
            if (v >= 1)
                Lat_flush_n = v;
        }
    }

    sprintf(path, "%.400s/%s.lat", dir, Lat_proc);

    Lat_fp = fopen(path, "a");
    if (Lat_fp == NULL)
        return;

    setvbuf(Lat_fp, NULL, _IOFBF, LAT_BUF_SIZE);
    Lat_cnt = 0;

    return;
}   /* End of Lat_Init ()   */

/*************************************************************************
    Function        : . Lat_Point
    Parameters IN   : . p_point : point name ("IN"/"OUT")
                      . p_key   : record key (fixed-width, no null needed)
                      . p_klen  : key length
    Return Code     : . void
    Comment         : . no-op when not initialized
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Lat_Point(const char *p_point, const char *p_key, int p_klen)
/*----------------------------------------------------------------------*/
{
    struct timeval  tv;

    if (Lat_fp == NULL)
        return;

    gettimeofday(&tv, NULL);

    fprintf(Lat_fp, "%ld%06ld|%s|%s|%.*s\n",
            (long)tv.tv_sec, (long)tv.tv_usec, Lat_proc, p_point,
            p_klen, p_key);

    Lat_cnt++;
    if (Lat_cnt >= Lat_flush_n) {
        fflush(Lat_fp);
        Lat_cnt = 0;
    }

    return;
}   /* End of Lat_Point ()  */

/*************************************************************************
    Function        : . Lat_Close
    Parameters IN   : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Lat_Close(void)
/*----------------------------------------------------------------------*/
{
    if (Lat_fp == NULL)
        return;

    fflush(Lat_fp);
    fclose(Lat_fp);
    Lat_fp = NULL;
    Lat_cnt = 0;

    return;
}   /* End of Lat_Close ()  */

/*************************************************************************
    End of Program (lat_trace.c)
*************************************************************************/
