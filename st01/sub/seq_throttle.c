/*************************************************************************
    Module      : . sequence save throttle (F3 seq-save-batch)
    File        : . seq_throttle.c
    Comment     : . FEP_SEQ_SAVE_INTERVAL(초) opt-in 스로틀 판정
                  . 미설정/0 = 매건 저장 (기존 동작, 기본값)
                  . 키별 last_time 테이블 8슬롯, 초과 키는 항상 저장(안전측)
                  . 순수 로직 - 파일 I/O 없음 (단위 테스트 대상)
*************************************************************************/

#include <stdlib.h>
#include <string.h>

#define SEQ_THR_SLOT_MAX    8
#define SEQ_THR_KEY_LEN     64

typedef struct {
    char    key[SEQ_THR_KEY_LEN];
    long    last;
} SEQ_THR_SLOT;

static SEQ_THR_SLOT    _thr_slot[SEQ_THR_SLOT_MAX];
static int             _thr_slot_cnt = 0;

/*************************************************************************
    Function        : . Seq_Throttle_Interval
    Parameters IN   : .
    Return Code     : . int : 저장 간격(초). 0 = 매건 저장(기본)
    Comment         : . FEP_SEQ_SAVE_INTERVAL 1회 평가 후 캐시
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Seq_Throttle_Interval(void)
/*----------------------------------------------------------------------*/
{
    static int  interval = -1;
    char        *env;

    if (interval == -1) {
        env = getenv("FEP_SEQ_SAVE_INTERVAL");
        if (env == NULL)
            interval = 0;
        else {
            interval = atoi(env);
            if (interval < 0)
                interval = 0;
        }
    }

    return (interval);
}   /* End of Seq_Throttle_Interval ()  */

/*************************************************************************
    Function        : . Seq_Throttle_Check
    Parameters IN   : . p_key : 스로틀 키 ("<file>:<dk>:<fk>")
                      . p_now : 현재 시각(초)
    Return Code     : . int : 1=skip(저장 생략), 0=저장 수행
    Comment         : . 저장(0) 리턴 시 내부 last_time 갱신
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Seq_Throttle_Check(const char *p_key, long p_now)
/*----------------------------------------------------------------------*/
{
    int     i, interval;

    interval = Seq_Throttle_Interval();

    if (interval == 0 || p_key == NULL)
        return (0);

    for (i = 0; i < _thr_slot_cnt; i ++) {
        if (strncmp(_thr_slot[i].key, p_key, SEQ_THR_KEY_LEN) == 0) {
            if (p_now - _thr_slot[i].last < interval)
                return (1);

            _thr_slot[i].last = p_now;
            return (0);
        }
    }

    /* 신규 키 등록 (슬롯 초과 시 항상 저장 - 안전측) */
    if (_thr_slot_cnt < SEQ_THR_SLOT_MAX) {
        strncpy(_thr_slot[_thr_slot_cnt].key, p_key, SEQ_THR_KEY_LEN - 1);
        _thr_slot[_thr_slot_cnt].key[SEQ_THR_KEY_LEN - 1] = '\0';
        _thr_slot[_thr_slot_cnt].last = p_now;
        _thr_slot_cnt ++;
    }

    return (0);
}   /* End of Seq_Throttle_Check () */

/*************************************************************************
    End of Program (seq_throttle.c)
*************************************************************************/
