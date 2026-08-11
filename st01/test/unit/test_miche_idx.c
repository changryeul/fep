/*------------------------------------------------------------------------
#   Unit Test : Miche_Idx_Reset / Put / Get (미체결 조회 캐시)
#   File      : test_miche_idx.c
#   SUT       : sub/miche_idx.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdio.h>
#include <string.h>

#include "miche_idx.h"

static MICHE_IDX ix;

void setUp(void)
{
    Miche_Idx_Reset(&ix);
}

void tearDown(void) {}

/*-- 기본 Put/Get --*/
void test_put_get(void)
{
    Miche_Idx_Put(&ix, "0000000001", 42);
    TEST_ASSERT_EQUAL_INT(42, Miche_Idx_Get(&ix, "0000000001"));
}

/*-- 미등록 키 → -1 --*/
void test_get_missing(void)
{
    TEST_ASSERT_EQUAL_INT(-1, Miche_Idx_Get(&ix, "9999999999"));
}

/*-- 동일 키 재Put → 갱신 --*/
void test_put_updates(void)
{
    Miche_Idx_Put(&ix, "0000000007", 10);
    Miche_Idx_Put(&ix, "0000000007", 77);
    TEST_ASSERT_EQUAL_INT(77, Miche_Idx_Get(&ix, "0000000007"));
}

/*-- Reset 후 비워짐 --*/
void test_reset_clears(void)
{
    Miche_Idx_Put(&ix, "0000000001", 1);
    Miche_Idx_Reset(&ix);
    TEST_ASSERT_EQUAL_INT(-1, Miche_Idx_Get(&ix, "0000000001"));
}

/*-- 우측정렬 공백 패딩 키 (KRX 주문번호 형태) --*/
void test_space_padded_keys(void)
{
    Miche_Idx_Put(&ix, "       123", 5);
    Miche_Idx_Put(&ix, "      1234", 6);
    TEST_ASSERT_EQUAL_INT(5, Miche_Idx_Get(&ix, "       123"));
    TEST_ASSERT_EQUAL_INT(6, Miche_Idx_Get(&ix, "      1234"));
}

/*-- 10,000키 적재 후 전수 일치 (실운영 MAX_MICHE 규모) --*/
void test_full_load(void)
{
    char key[16];
    int  i;

    for (i = 0; i < 10000; i++) {
        sprintf(key, "%010d", 200000000 + i);
        Miche_Idx_Put(&ix, key, i);
    }

    for (i = 0; i < 10000; i++) {
        sprintf(key, "%010d", 200000000 + i);
        /* eviction 허용 캐시 - 값이 있으면 반드시 정확해야 함 */
        if (Miche_Idx_Get(&ix, key) != -1)
            TEST_ASSERT_EQUAL_INT(i, Miche_Idx_Get(&ix, key));
    }
}

/*-- 10,000키 적재 시 히트율 95% 이상 (로드팩터 0.61) --*/
void test_full_load_hit_rate(void)
{
    char key[16];
    int  i, hit;

    for (i = 0; i < 10000; i++) {
        sprintf(key, "%010d", 200000000 + i);
        Miche_Idx_Put(&ix, key, i);
    }

    hit = 0;
    for (i = 0; i < 10000; i++) {
        sprintf(key, "%010d", 200000000 + i);
        if (Miche_Idx_Get(&ix, key) == i)
            hit++;
    }

    TEST_ASSERT_TRUE(hit >= 9500);
}

/*-- 키 10바이트 초과분 무시 (null 종료 불필요) --*/
void test_key_exactly_ten_bytes(void)
{
    Miche_Idx_Put(&ix, "0000000001GARBAGE", 3);
    TEST_ASSERT_EQUAL_INT(3, Miche_Idx_Get(&ix, "0000000001DIFFERENT"));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_put_get);
    RUN_TEST(test_get_missing);
    RUN_TEST(test_put_updates);
    RUN_TEST(test_reset_clears);
    RUN_TEST(test_space_padded_keys);
    RUN_TEST(test_full_load);
    RUN_TEST(test_full_load_hit_rate);
    RUN_TEST(test_key_exactly_ten_bytes);
    return UNITY_END();
}
