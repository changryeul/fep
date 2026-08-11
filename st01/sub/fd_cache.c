/*************************************************************************
    Module      : . path-keyed fd/FILE* cache (F4 file-rw-fd-pool)
    File        : . fd_cache.c
    Comment     : . 매건 open/close 제거용 범용 캐시
                  . 실패/슬롯 초과 시 -1/NULL 리턴 - 호출측이 기존
                    open/close 방식으로 폴백 (동작 보존, 순수 최적화)
                  . 경로는 00000000 고정 디렉토리 전제 (롤오버 없음)
                  . 단일 스레드 이벤트 루프 전용
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FDC_SLOT_MAX    32
#define FDC_PATH_LEN    128

typedef struct {
    char    path[FDC_PATH_LEN];
    int     fd;             /* raw fd (-1: 미사용)      */
    FILE    *fp;            /* FILE*  (NULL: 미사용)    */
} FDC_SLOT;

static FDC_SLOT     _fdc[FDC_SLOT_MAX];
static int          _fdc_cnt = 0;

/*----------------------------------------------------------------------*/
static FDC_SLOT *fdc_find(const char *p_path, int want_fp)
/*----------------------------------------------------------------------*/
{
    int     i;

    for (i = 0; i < _fdc_cnt; i ++) {
        if (strcmp(_fdc[i].path, p_path) != 0)
            continue;
        if (want_fp && _fdc[i].fp != NULL)
            return (&_fdc[i]);
        if (!want_fp && _fdc[i].fd != -1)
            return (&_fdc[i]);
    }

    return (NULL);
}

/*----------------------------------------------------------------------*/
static FDC_SLOT *fdc_new(const char *p_path)
/*----------------------------------------------------------------------*/
{
    FDC_SLOT    *s;

    if (_fdc_cnt >= FDC_SLOT_MAX)
        return (NULL);

    if (strlen(p_path) >= FDC_PATH_LEN)
        return (NULL);

    s = &_fdc[_fdc_cnt];
    strcpy(s->path, p_path);
    s->fd = -1;
    s->fp = NULL;
    _fdc_cnt ++;

    return (s);
}

/*************************************************************************
    Function        : . Fd_Cache_Get
    Parameters IN   : . p_path  : file path (cache key)
                      . p_flags : open flags
                      . p_mode  : open mode
    Return Code     : . int : cached fd, -1 = 캐시 불가(호출측 폴백)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Fd_Cache_Get(const char *p_path, int p_flags, int p_mode)
/*----------------------------------------------------------------------*/
{
    FDC_SLOT    *s;
    int         fd;

    if (p_path == NULL)
        return (-1);

    s = fdc_find(p_path, 0);
    if (s != NULL)
        return (s->fd);

    fd = open(p_path, p_flags, p_mode);
    if (fd == -1)
        return (-1);

    s = fdc_new(p_path);
    if (s == NULL) {
        close(fd);
        return (-1);
    }

    s->fd = fd;

    return (fd);
}   /* End of Fd_Cache_Get ()   */

/*************************************************************************
    Function        : . Fp_Cache_Get
    Parameters IN   : . p_path : file path (cache key)
                      . p_mode : fopen mode
    Return Code     : . FILE * : cached stream, NULL = 캐시 불가(폴백)
*************************************************************************/
/*----------------------------------------------------------------------*/
FILE    *Fp_Cache_Get(const char *p_path, const char *p_mode)
/*----------------------------------------------------------------------*/
{
    FDC_SLOT    *s;
    FILE        *fp;

    if (p_path == NULL || p_mode == NULL)
        return (NULL);

    s = fdc_find(p_path, 1);
    if (s != NULL)
        return (s->fp);

    /* F_W 계열과 동일하게 Linux에서는 LFS(fopen64) 사용 */
#if defined __linux
    fp = fopen64(p_path, p_mode);
#else
    fp = fopen(p_path, p_mode);
#endif
    if (fp == NULL)
        return (NULL);

    s = fdc_new(p_path);
    if (s == NULL) {
        fclose(fp);
        return (NULL);
    }

    s->fp = fp;

    return (fp);
}   /* End of Fp_Cache_Get ()   */

/*************************************************************************
    Function        : . Fd_Cache_Close_All
    Parameters IN   : .
    Return Code     : . void
    Comment         : . 캐시 전체 close (테스트/종료용)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Fd_Cache_Close_All(void)
/*----------------------------------------------------------------------*/
{
    int     i;

    for (i = 0; i < _fdc_cnt; i ++) {
        if (_fdc[i].fd != -1) {
            close(_fdc[i].fd);
            _fdc[i].fd = -1;
        }
        if (_fdc[i].fp != NULL) {
            fclose(_fdc[i].fp);
            _fdc[i].fp = NULL;
        }
        _fdc[i].path[0] = '\0';
    }

    _fdc_cnt = 0;

    return;
}   /* End of Fd_Cache_Close_All ()    */

/*************************************************************************
    End of Program (fd_cache.c)
*************************************************************************/
