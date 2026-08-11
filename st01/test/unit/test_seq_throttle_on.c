/*------------------------------------------------------------------------
#   Unit Test : Seq_Throttle — FEP_SEQ_SAVE_INTERVAL=3 모드
#   File      : test_seq_throttle_on.c
#   SUT       : sub/seq_throttle.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>

extern int Seq_Throttle_Interval(void);
extern int Seq_Throttle_Check(const char *p_key, long p_now);

void setUp(void) {}
void tearDown(void) {}

/*-- interval 파싱 --*/
void test_interval_parsed(void)
{
    TEST_ASSERT_EQUAL_INT(3, Seq_Throttle_Interval());
}

/*-- 첫 호출 저장, interval 내 skip, 경과 후 저장 --*/
void test_skip_within_interval(void)
{
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("kA:0:1", 1000)); /* 저장 */
    TEST_ASSERT_EQUAL_INT(1, Seq_Throttle_Check("kA:0:1", 1001)); /* skip */
    TEST_ASSERT_EQUAL_INT(1, Seq_Throttle_Check("kA:0:1", 1002)); /* skip */
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("kA:0:1", 1003)); /* 저장 */
    TEST_ASSERT_EQUAL_INT(1, Seq_Throttle_Check("kA:0:1", 1004)); /* skip */
}

/*-- 키 격리: 다른 키는 서로 영향 없음 --*/
void test_key_isolation(void)
{
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("kB:0:2", 2000));
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("kC:0:3", 2000));
    TEST_ASSERT_EQUAL_INT(1, Seq_Throttle_Check("kB:0:2", 2001));
    TEST_ASSERT_EQUAL_INT(1, Seq_Throttle_Check("kC:0:3", 2002));
}

/*-- 슬롯 초과(8개 초과 키): 항상 저장(안전측) --*/
void test_slot_overflow_always_saves(void)
{
    char key[32];
    int  i;

    /* 8슬롯 채우기 (kA,kB,kC + 신규 5개 = 8) */
    for (i = 0; i < 5; i++) {
        sprintf(key, "fill%d:0:0", i);
        TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check(key, 3000));
    }
    /* 9번째 키: 슬롯 없음 → 항상 저장 */
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("overflow:0:0", 3000));
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("overflow:0:0", 3001));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    setenv("FEP_SEQ_SAVE_INTERVAL", "3", 1);

    UNITY_BEGIN();
    RUN_TEST(test_interval_parsed);
    RUN_TEST(test_skip_within_interval);
    RUN_TEST(test_key_isolation);
    RUN_TEST(test_slot_overflow_always_saves);
    return UNITY_END();
}
