/*------------------------------------------------------------------------
#   Module  : save read sequence
#   File    : seq_save.c
#
#   F3 seq-save-batch:
#     - fd 캐시: 매건 fopen/fclose 제거 (경로는 00000000 고정 디렉토리라
#       롤오버 없음, compact.sh 삭제 대상 아님 - 프로세스 수명 동안 유효)
#     - 스로틀: FEP_SEQ_SAVE_INTERVAL(초) opt-in, 기본 0 = 매건 저장
#       (sub/seq_throttle.c 판정. 커서 권위 사본은 SHM이므로 스킵분은
#        다음 저장 시 자연 만회)
#     - Seq_Save_Flush: 종료 시 캐시된 전체 슬롯 강제 저장
#       (Exit_Process에서 호출, 시그널 컨텍스트에서는 호출측에서 스킵)
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

#define     SEQ_FD_MAX      8

typedef struct {
    char    f_name[100];    /* 전체 경로 (캐시 키)              */
    char    file[24];       /* 파일명                           */
    int     dk;             /* daemon key                       */
    int     fk;             /* FILEM position                   */
    int     is_seq;         /* 1: _seq(FILEM), 0: _dseq(DSHM)   */
    FILE    *fp;            /* 캐시된 스트림                    */
} SEQ_FD_SLOT;

static  SEQ_FD_SLOT     _seq_fd[SEQ_FD_MAX];
static  int             _seq_fd_cnt = 0;

/*************************************************************************
    Function        : . seq_fd_get (internal)
    Parameters IN   : . p_name : 전체 파일 경로
    Return Code     : . SEQ_FD_SLOT * (NULL: fopen 실패/슬롯 초과)
    Comment         : . (경로) 기준 fd 캐시 조회, 없으면 fopen 후 등록
*************************************************************************/
/*----------------------------------------------------------------------*/
static SEQ_FD_SLOT *seq_fd_get(char *p_name, char *p_file, int dk, int fk,
        int is_seq)
/*----------------------------------------------------------------------*/
{
    int     i;
    FILE    *fp;

    for (i = 0; i < _seq_fd_cnt; i ++) {
        if (strcmp(_seq_fd[i].f_name, p_name) == 0)
            return (&_seq_fd[i]);
    }

    fp = fopen(p_name, "r+");

    if (fp == NULL)
        return (NULL);

    if (_seq_fd_cnt >= SEQ_FD_MAX) {
        /* 슬롯 초과: 캐시 없이 일회용 슬롯처럼 쓸 수 없으므로
           호출측이 기존 방식(즉시 close)으로 처리하도록 NULL 아닌
           마지막 슬롯을 덮지 않고 그냥 닫고 실패 처리하지 않는다.
           실운영 프로세스의 입력 파일은 2~3개라 도달하지 않음 */
        fclose(fp);
        return (NULL);
    }

    strncpy(_seq_fd[_seq_fd_cnt].f_name, p_name,
            sizeof (_seq_fd[0].f_name) - 1);
    _seq_fd[_seq_fd_cnt].f_name[sizeof (_seq_fd[0].f_name) - 1] = '\0';
    strncpy(_seq_fd[_seq_fd_cnt].file, p_file,
            sizeof (_seq_fd[0].file) - 1);
    _seq_fd[_seq_fd_cnt].file[sizeof (_seq_fd[0].file) - 1] = '\0';
    _seq_fd[_seq_fd_cnt].dk = dk;
    _seq_fd[_seq_fd_cnt].fk = fk;
    _seq_fd[_seq_fd_cnt].is_seq = is_seq;
    _seq_fd[_seq_fd_cnt].fp = fp;
    _seq_fd_cnt ++;

    return (&_seq_fd[_seq_fd_cnt - 1]);
}   /* End of seq_fd_get () */

/*************************************************************************
    Function        : . seq_write_slot (internal)
    Parameters IN   : . s : fd 캐시 슬롯
    Return Code     : . int (0: success, -1: failure)
    Comment         : . 잠금 -> SHM 커서 스냅샷 기록 -> 잠금 해제
                        (기존 Seq_Save 본문과 동일, fclose만 제거)
*************************************************************************/
/*----------------------------------------------------------------------*/
static int seq_write_slot(SEQ_FD_SLOT *s)
/*----------------------------------------------------------------------*/
{
    int             i, rt;
    char            buf[512];
    struct flock    lock;

    memset(buf, 0, sizeof (buf));

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = 0;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(s->fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            Log(SYS_FATAL, "Seq_Save:cannot lock(fcntl)[%s] {%d:%s}",
                    s->f_name, SYS_NO, SYS_STR);
            return (-1);
        }
    } while (rt == -1);

    for (i = 0; i < 9; i ++)
        sprintf(&buf[8*i], "%08d", FILEM(s->dk,s->fk).r_cnt[i]);

    fseek(s->fp, 0L, SEEK_SET);

    rt = fwrite(buf, strlen(buf), 1, s->fp);

    fflush(s->fp);

    lock.l_type = F_UNLCK;
    fcntl(fileno(s->fp), F_SETLK, &lock);

    return (OK);
}   /* End of seq_write_slot () */

/*************************************************************************
    Function        : . save file read sequence
    Parameters IN   : . p_file  : file name
                      . dk      : daemon key
                      . fk      : FILEM position
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
    Global Data     : . char *_FEP_DAT
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Seq_Save(char *p_file, int dk, int fk)
/*----------------------------------------------------------------------*/
{
    char            f_name[100], bumun[4], thr_key[80];
    SEQ_FD_SLOT     *slot;

    sprintf(bumun, "%-2.2s", p_file);
    sprintf(f_name, "%s/%s/00000000/%s_seq",
            _FEP_DAT, LtoU(bumun, 2), p_file);

    slot = seq_fd_get(f_name, p_file, dk, fk, 1);

    if (slot == NULL) {
        Log(SAM_FATAL, "Seq_Save:fopen failure[%s] {%d:%s}",
                f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* opt-in 스로틀: FEP_SEQ_SAVE_INTERVAL 미설정(기본)이면 매건 저장 */
    sprintf(thr_key, "%.64s:%d:%d", p_file, dk, fk);

    if (Seq_Throttle_Check(thr_key, (long)time(NULL)))
        return (OK);

    return (seq_write_slot(slot));
}   /* End of Seq_Save ()   */

/*************************************************************************
    Function        : . Seq_Save_Flush
    Parameters IN   : .
    Return Code     : . void
    Comment         : . 캐시된 전체 슬롯 강제 저장 (스로틀 무시)
                      . 프로세스 정상 종료 경로에서 호출
                        (시그널 컨텍스트 스킵은 호출측 책임)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Seq_Save_Flush(void)
/*----------------------------------------------------------------------*/
{
    int     i;

    for (i = 0; i < _seq_fd_cnt; i ++) {
        if (_seq_fd[i].fp != NULL && _seq_fd[i].is_seq == 1)
            seq_write_slot(&_seq_fd[i]);
    }

    return;
}   /* End of Seq_Save_Flush () */

/*************************************************************************
    Function        : . save data SHM read sequence
    Parameters IN   : . p_file  : data SHM base name
                      . dk      : daemon key
                      . fk      : DSHM position
                      . ck      : read count key (0 ~ 8)
    Parameters OUT  : .
    Return Code     : . int (0: success, -1: failure)
    Global Data     : . char *_FEP_DAT
    Comment         : . F3: fd 캐시만 적용 (ck별 부분 잠금 구조라
                        스로틀은 1차 범위 제외)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Dshm_Seq_Save(char *p_file, int dk, int fk, int ck)
/*----------------------------------------------------------------------*/
{
    int             rt;
    char            f_name[100], buf[512], bumun[4];
    SEQ_FD_SLOT     *slot;
    struct flock    lock;

    sprintf(bumun, "%-2.2s", p_file);
    sprintf(f_name, "%s/%s/00000000/%s_dseq",
            _FEP_DAT, LtoU(bumun, 2), p_file);
    memset(buf, 0, sizeof (buf));

    slot = seq_fd_get(f_name, p_file, dk, fk, 0);

    if (slot == NULL) {
        Log(SAM_FATAL, "Dshm_Seq_Save:fopen fail[%s] {%d:%s}",
                f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 8L * ck;
        lock.l_len = 8L;

        rt = fcntl(fileno(slot->fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            Log(SYS_FATAL, "Dshm_Seq_Save:cannot lock(fcntl)[%s] {%d:%s}",
                    f_name, SYS_NO, SYS_STR);
            return (-1);
        }
    } while (rt == -1);

    fseek(slot->fp, 8L * ck, SEEK_SET);

    if (ck == 9)
        sprintf(buf, "%08d", DSHM(dk,fk).sm_r_cnt);
    else
        sprintf(buf, "%08d", DSHM(dk,fk).r_cnt[ck]);

    rt = fwrite(buf, strlen(buf), 1, slot->fp);

    fflush(slot->fp);

    lock.l_type = F_UNLCK;
    fcntl(fileno(slot->fp), F_SETLK, &lock);

    return (OK);
}   /* End of Dshm_Seq_Save ()  */

/*************************************************************************
    End of Program (seq_save.c)
*************************************************************************/
