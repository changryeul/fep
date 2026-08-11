/*------------------------------------------------------------------------
#   Unit Test : seam_ring_write / seam_ring_read (SEAM 스테이징 링버퍼 코어)
#   File      : test_seam_queue.c
#   SUT       : sub/seam_ring.c  (순수 링 코어 — SHM/락 무관)
#
#   부문 간(pc_/pb_ → po_) 메시지 전달 링버퍼의 커서 산술을 검증한다:
#   write/read FIFO 순서, replay(재소비 없음), multi-reader 독립,
#   오버플로 유실 회계(dropped, 무음유실 금지), maxrec 배치 상한.
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "seam_queue.h"

static SEAM_QUEUE *q;
static char        out[8 * SEAM_Q_RECSZ];

void setUp(void)
{
    q = (SEAM_QUEUE *)calloc(1, sizeof(SEAM_QUEUE));   /* w_seq/r_seq/dropped = 0 */
    TEST_ASSERT_NOT_NULL(q);
}

void tearDown(void)
{
    free(q);
    q = NULL;
}

/* recsz=len 인 페이로드 기록(선두에 식별 문자열) */
static void wr(const char *tag)
{
    char rec[SEAM_Q_RECSZ];
    memset(rec, ' ', sizeof(rec));
    memcpy(rec, tag, strlen(tag));
    TEST_ASSERT_EQUAL_INT(1, seam_ring_write(q, rec, (int)strlen(tag)));
}

/*-- 1건 write → 리더가 1건 read, 내용 일치 --*/
void test_write_read_one(void)
{
    int n;
    wr("MSG001");
    n = seam_ring_read(q, SEAM_R_MICHE, out, 8);
    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_EQUAL_MEMORY("MSG001", out, 6);
}

/*-- 3건 write → FIFO 순서로 read --*/
void test_fifo_order(void)
{
    int n;
    wr("AAA"); wr("BBB"); wr("CCC");
    n = seam_ring_read(q, SEAM_R_MICHE, out, 8);
    TEST_ASSERT_EQUAL_INT(3, n);
    TEST_ASSERT_EQUAL_MEMORY("AAA", out + 0 * SEAM_Q_RECSZ, 3);
    TEST_ASSERT_EQUAL_MEMORY("BBB", out + 1 * SEAM_Q_RECSZ, 3);
    TEST_ASSERT_EQUAL_MEMORY("CCC", out + 2 * SEAM_Q_RECSZ, 3);
}

/*-- replay: 다 읽은 뒤 재read → 0건(커서 보존, 재소비 없음) --*/
void test_replay_no_reconsume(void)
{
    wr("X"); wr("Y");
    TEST_ASSERT_EQUAL_INT(2, seam_ring_read(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_INT(0, seam_ring_read(q, SEAM_R_MICHE, out, 8));
    /* 새 write 후에는 그 1건만 */
    wr("Z");
    TEST_ASSERT_EQUAL_INT(1, seam_ring_read(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_MEMORY("Z", out, 1);
}

/*-- multi-reader 독립: 리더0가 다 읽어도 리더1은 전량 read --*/
void test_multi_reader_independent(void)
{
    wr("P"); wr("Q");
    TEST_ASSERT_EQUAL_INT(2, seam_ring_read(q, SEAM_R_MICHE, out, 8));  /* 미체결 리더 */
    TEST_ASSERT_EQUAL_INT(2, seam_ring_read(q, SEAM_R_DIST,  out, 8));  /* 분배 리더  */
    TEST_ASSERT_EQUAL_MEMORY("P", out + 0 * SEAM_Q_RECSZ, 1);
    TEST_ASSERT_EQUAL_MEMORY("Q", out + 1 * SEAM_Q_RECSZ, 1);
    /* 두 리더 모두 소진 상태 */
    TEST_ASSERT_EQUAL_INT(0, seam_ring_read(q, SEAM_R_DIST, out, 8));
}

/*-- maxrec 상한: 10건 중 3건씩 배치로 read --*/
void test_maxrec_batch(void)
{
    int i;
    char tag[8];
    for (i = 0; i < 10; i++) { sprintf(tag, "N%d", i); wr(tag); }

    TEST_ASSERT_EQUAL_INT(3, seam_ring_read(q, SEAM_R_MICHE, out, 3));
    TEST_ASSERT_EQUAL_MEMORY("N0", out + 0 * SEAM_Q_RECSZ, 2);
    TEST_ASSERT_EQUAL_MEMORY("N2", out + 2 * SEAM_Q_RECSZ, 2);

    TEST_ASSERT_EQUAL_INT(3, seam_ring_read(q, SEAM_R_MICHE, out, 3));
    TEST_ASSERT_EQUAL_MEMORY("N3", out + 0 * SEAM_Q_RECSZ, 2);

    TEST_ASSERT_EQUAL_INT(3, seam_ring_read(q, SEAM_R_MICHE, out, 3));
    TEST_ASSERT_EQUAL_INT(1, seam_ring_read(q, SEAM_R_MICHE, out, 3));  /* 나머지 1 */
    TEST_ASSERT_EQUAL_MEMORY("N9", out, 2);
}

/*-- 오버플로: SLOTS 초과 write 후 read → 최신 SLOTS건만, dropped=유실수 --*/
void test_overflow_accounting(void)
{
    int i, got;
    char tag[16];
    char big[64 * SEAM_Q_RECSZ];   /* 배치 read 버퍼 */
    long total = 0;

    for (i = 0; i < SEAM_Q_SLOTS + 2; i++) {   /* 2건 유실 유발 */
        sprintf(tag, "%d", i);
        wr(tag);
    }
    /* 리더가 뒤늦게 소비 → 유실 2건, 잔여 SLOTS건 */
    while ((got = seam_ring_read(q, SEAM_R_MICHE, big, 64)) > 0)
        total += got;

    TEST_ASSERT_EQUAL_INT64(2, q->dropped[SEAM_R_MICHE]);
    TEST_ASSERT_EQUAL_INT64((long)SEAM_Q_SLOTS, total);
}

/*-- peek는 커서를 전진시키지 않음(재peek 시 동일) --*/
void test_peek_no_advance(void)
{
    wr("AAA"); wr("BBB");
    TEST_ASSERT_EQUAL_INT(2, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_MEMORY("AAA", out + 0 * SEAM_Q_RECSZ, 3);
    /* 다시 peek → 여전히 처음부터 2건(커서 불변 = at-least-once 재처리 가능) */
    TEST_ASSERT_EQUAL_INT(2, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_MEMORY("AAA", out + 0 * SEAM_Q_RECSZ, 3);
}

/*-- commit 후 그 건수만큼 전진(다음 peek는 이후분) --*/
void test_commit_advances(void)
{
    wr("P"); wr("Q"); wr("R");
    TEST_ASSERT_EQUAL_INT(3, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    seam_ring_commit(q, SEAM_R_MICHE, 2);       /* 앞 2건만 처리완료 커밋 */
    /* 남은 1건(R)만 peek */
    TEST_ASSERT_EQUAL_INT(1, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_MEMORY("R", out, 1);
    seam_ring_commit(q, SEAM_R_MICHE, 1);
    TEST_ASSERT_EQUAL_INT(0, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
}

/*-- peek 후 commit 안 하면 재처리 가능(at-least-once): 재기동 시뮬 --*/
void test_peek_without_commit_reprocessable(void)
{
    wr("X");
    TEST_ASSERT_EQUAL_INT(1, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    /* commit 없이 "크래시" → 재peek 시 동일 건 재조회(유실 없음) */
    TEST_ASSERT_EQUAL_INT(1, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
    TEST_ASSERT_EQUAL_MEMORY("X", out, 1);
    seam_ring_commit(q, SEAM_R_MICHE, 1);
    TEST_ASSERT_EQUAL_INT(0, seam_ring_peek(q, SEAM_R_MICHE, out, 8));
}

/*-- 주문큐 라우팅: 시장 letter → 주문큐 인덱스 (§5.3) --*/
void test_order_queue_routing(void)
{
    TEST_ASSERT_EQUAL_INT(SEAM_ORDQ_BOND,  Seam_Order_Queue('b'));  /* 채권      */
    TEST_ASSERT_EQUAL_INT(SEAM_ORDQ_DERIV, Seam_Order_Queue('c'));  /* 파생/통화선물 */
    TEST_ASSERT_EQUAL_INT(SEAM_ORDQ_FX,    Seam_Order_Queue('f'));  /* FX        */
}

/*-- 미지원 시장 letter → -1 (전략이 잘못된 시장 주문 방지) --*/
void test_order_queue_unknown(void)
{
    TEST_ASSERT_EQUAL_INT(-1, Seam_Order_Queue('a'));  /* 현물 송신부 미배선 */
    TEST_ASSERT_EQUAL_INT(-1, Seam_Order_Queue('o'));  /* OMS는 시장 아님    */
    TEST_ASSERT_EQUAL_INT(-1, Seam_Order_Queue('z'));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_write_read_one);
    RUN_TEST(test_fifo_order);
    RUN_TEST(test_replay_no_reconsume);
    RUN_TEST(test_multi_reader_independent);
    RUN_TEST(test_maxrec_batch);
    RUN_TEST(test_overflow_accounting);
    RUN_TEST(test_peek_no_advance);
    RUN_TEST(test_commit_advances);
    RUN_TEST(test_peek_without_commit_reprocessable);
    RUN_TEST(test_order_queue_routing);
    RUN_TEST(test_order_queue_unknown);
    return UNITY_END();
}
