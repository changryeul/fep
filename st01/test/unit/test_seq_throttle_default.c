/*------------------------------------------------------------------------
#   Unit Test : Seq_Throttle — 기본 모드 (FEP_SEQ_SAVE_INTERVAL 미설정)
#   File      : test_seq_throttle_default.c
#   SUT       : sub/seq_throttle.c
#
#   기본(미설정) = interval 0 = 매건 저장 (기존 동작 불변)
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdlib.h>

extern int Seq_Throttle_Interval(void);
extern int Seq_Throttle_Check(const char *p_key, long p_now);

void setUp(void) {}
void tearDown(void) {}

/*-- 미설정: interval 0 --*/
void test_interval_zero_by_default(void)
{
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Interval());
}

/*-- interval 0이면 연속 호출도 전부 저장(skip 없음) --*/
void test_never_skips(void)
{
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("f1:0:0", 1000));
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("f1:0:0", 1000));
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("f1:0:0", 1001));
}

/*-- env 캐시: 이후 설정해도 기본 모드 유지 --*/
void test_interval_cached(void)
{
    setenv("FEP_SEQ_SAVE_INTERVAL", "5", 1);
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Interval());
    TEST_ASSERT_EQUAL_INT(0, Seq_Throttle_Check("f1:0:0", 2000));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    unsetenv("FEP_SEQ_SAVE_INTERVAL");

    UNITY_BEGIN();
    RUN_TEST(test_interval_zero_by_default);
    RUN_TEST(test_never_skips);
    RUN_TEST(test_interval_cached);
    return UNITY_END();
}
