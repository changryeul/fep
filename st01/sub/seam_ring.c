/*------------------------------------------------------------------------
 *  seam_ring.c — SEAM 스테이징 링버퍼 순수 코어 (SHM/락 무관)
 *
 *  부문 간(pc_/pb_ 수신 → po_ OMS코어) 메시지 전달 링버퍼의 커서 산술.
 *  SHM attach·세마포어는 seam_queue.c가 감싼다. 이 파일은 순수 함수라
 *  단위테스트(test/unit/test_seam_queue.c)로 직접 검증 가능.
 *
 *  단일 writer(락은 호출측) / 리더별 독립 커서(multi-reader) 가정.
 *  w_seq 는 단조증가 발행 커서, r_seq[ridx] 는 리더별 소비 커서.
 *------------------------------------------------------------------------*/
#include    <string.h>
#include    "seam_queue.h"

/*------------------------------------------------------------------------
 *  seam_ring_write — 1건 기록. slot = w_seq % SLOTS.
 *    데이터를 슬롯에 복사(공백 패딩) 후 w_seq 증가(발행).
 *    반환: 성공 1.
 *------------------------------------------------------------------------*/
int     seam_ring_write(SEAM_QUEUE *q, const void *rec, int recsz)
{
    long    slot;

    if (q == NULL || rec == NULL)
        return -1;
    if (recsz < 0)
        recsz = 0;
    if (recsz > SEAM_Q_RECSZ)
        recsz = SEAM_Q_RECSZ;

    slot = q->w_seq % SEAM_Q_SLOTS;

    memset(q->slot[slot], ' ', SEAM_Q_RECSZ);      /* FEP 파일큐 공백패딩 관례 */
    memcpy(q->slot[slot], rec, (size_t)recsz);

    q->w_seq++;                                     /* 데이터 기록 후 발행     */
    return 1;
}

/*------------------------------------------------------------------------
 *  seam_ring_read — 리더 ridx가 미소비분을 최대 maxrec건까지 out에 복사.
 *    오버플로(w_seq - r_seq > SLOTS)면 유실분을 dropped[ridx]에 누계하고
 *    r_seq 를 최신 창(w_seq - SLOTS)으로 점프 후 진행(무음유실 금지).
 *    반환: 읽은 건수(0 이상), 인자 오류 시 -1.
 *------------------------------------------------------------------------*/
int     seam_ring_read(SEAM_QUEUE *q, int ridx, void *out, int maxrec)
{
    long    avail, slot;
    int     n = 0;
    char   *o = (char *)out;

    if (q == NULL || out == NULL)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;
    if (maxrec <= 0)
        return 0;

    /* 리더가 링 용량보다 뒤처졌으면 오버런분 유실 회계 후 최신창 점프 */
    avail = q->w_seq - q->r_seq[ridx];
    if (avail > SEAM_Q_SLOTS) {
        q->dropped[ridx] += (avail - SEAM_Q_SLOTS);
        q->r_seq[ridx]    = q->w_seq - SEAM_Q_SLOTS;
    }

    while (q->r_seq[ridx] < q->w_seq && n < maxrec) {
        slot = q->r_seq[ridx] % SEAM_Q_SLOTS;
        memcpy(o + (long)n * SEAM_Q_RECSZ, q->slot[slot], SEAM_Q_RECSZ);
        q->r_seq[ridx]++;
        n++;
    }
    return n;
}

/*------------------------------------------------------------------------
 *  seam_ring_peek — r_seq 전진 없이 미소비분을 out에 복사(오버플로 점프만 반영).
 *    at-least-once: 처리 완료 후 seam_ring_commit로 별도 전진. commit 전
 *    크래시 시 재기동하면 동일 건이 다시 peek됨(유실 없음, 소비자 멱등 필요).
 *    반환: 조회 건수(0 이상), 인자 오류 시 -1.
 *------------------------------------------------------------------------*/
int     seam_ring_peek(SEAM_QUEUE *q, int ridx, void *out, int maxrec)
{
    long    avail, cur, slot;
    int     n = 0;
    char   *o = (char *)out;

    if (q == NULL || out == NULL)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;
    if (maxrec <= 0)
        return 0;

    /* 링 용량 초과분은 이미 유실 → 즉시 회계+커서 점프(재조회해도 없음) */
    avail = q->w_seq - q->r_seq[ridx];
    if (avail > SEAM_Q_SLOTS) {
        q->dropped[ridx] += (avail - SEAM_Q_SLOTS);
        q->r_seq[ridx]    = q->w_seq - SEAM_Q_SLOTS;
    }

    /* r_seq는 건드리지 않고 로컬 커서로 조회만 */
    cur = q->r_seq[ridx];
    while (cur < q->w_seq && n < maxrec) {
        slot = cur % SEAM_Q_SLOTS;
        memcpy(o + (long)n * SEAM_Q_RECSZ, q->slot[slot], SEAM_Q_RECSZ);
        cur++;
        n++;
    }
    return n;
}

/*------------------------------------------------------------------------
 *  seam_ring_commit — 처리 완료분 n건만큼 r_seq[ridx] 전진.
 *    w_seq 초과하지 않도록 클램프(방어적).
 *------------------------------------------------------------------------*/
void    seam_ring_commit(SEAM_QUEUE *q, int ridx, int n)
{
    if (q == NULL || ridx < 0 || ridx >= SEAM_Q_NREADER || n <= 0)
        return;

    q->r_seq[ridx] += n;
    if (q->r_seq[ridx] > q->w_seq)      /* 방어: 초과 커밋 방지 */
        q->r_seq[ridx] = q->w_seq;
}

/*------------------------------------------------------------------------
 *  Seam_Order_Queue — 시장 letter → 주문큐 인덱스 (§5.3, 순수).
 *    전략이 자기 시장으로 주문큐를 선택하는 라우팅 키. 차익은 leg별 2회 호출.
 *    미지원 letter(현물 미배선/OMS/관리 등) → -1 (잘못된 시장 주문 방지).
 *------------------------------------------------------------------------*/
int     Seam_Order_Queue(char letter)
{
    switch (letter) {
        case 'b': return SEAM_ORDQ_BOND;    /* 채권 → pb_1101_ts            */
        case 'c': return SEAM_ORDQ_DERIV;   /* 파생/통화선물 → pc_1101_ts   */
        case 'f': return SEAM_ORDQ_FX;      /* FX → pf_1101_ts(신규)        */
        default:  return -1;
    }
}

/*************************************************************************
    End of Program (seam_ring.c)
*************************************************************************/
