/*------------------------------------------------------------------------
#   Unit Test : Strategy_Letter (전략번호 → 시장 letter 매핑)
#   File      : test_strategy_letter.c
#   SUT       : sub/strategy_letter.c
#
#   po_9000_mp의 pa_ 하드코딩(걸림돌 ①)을 대체하는 순수 매핑 검증:
#   단일시장(채권 LP=b), 크로스마켓(차익=o), 밴드 흡수(같은 시장 추가 LP),
#   longest-prefix match, 미지정 기본값('o').
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

#include "strategy_letter.h"

void setUp(void) {}
void tearDown(void) {}

/*-- 채권 LP(5050) → 'b' (단일시장=채권) --*/
void test_bond_lp(void)
{
    TEST_ASSERT_EQUAL_INT('b', Strategy_Letter("5050"));
}

/*-- 통화선물×FX 차익(52xx) → 'o' (크로스마켓=OMS 코어) --*/
void test_arb_cross(void)
{
    TEST_ASSERT_EQUAL_INT('o', Strategy_Letter("5201"));
    TEST_ASSERT_EQUAL_INT('o', Strategy_Letter("5240"));
}

/*-- 같은 채권 시장에 다른 LP 추가(50xx 밴드) → 무변경 'b' 흡수 --*/
void test_bond_band_absorbs_new_lp(void)
{
    TEST_ASSERT_EQUAL_INT('b', Strategy_Letter("5051"));  /* 다른 채권 LP */
    TEST_ASSERT_EQUAL_INT('b', Strategy_Letter("5099"));  /* 또다른 채권 전략 */
}

/*-- 미지정 전략번호 → 기본 'o'(OMS 코어) --*/
void test_default_oms(void)
{
    TEST_ASSERT_EQUAL_INT('o', Strategy_Letter("5900"));
    TEST_ASSERT_EQUAL_INT('o', Strategy_Letter("5310"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_bond_lp);
    RUN_TEST(test_arb_cross);
    RUN_TEST(test_bond_band_absorbs_new_lp);
    RUN_TEST(test_default_oms);
    return UNITY_END();
}
