/*------------------------------------------------------------------------
#   Unit Test : KRX 전문 struct ↔ EXTURE 3.0 v3.24 스펙 길이 일치 가드
#   File      : test_krx_struct.c
#   SUT       : inc/pa_struct.h (KRX_HEADER, KRX_JUMUN_DATA, KRX_SETTLE_DATA ...)
#
#   전문을 struct로 관리하면 바이트 오프셋 수기계산이 필요 없다. 단,
#   손수작성 struct가 스펙과 어긋나면(과거 S_Fmt 오버플로 사례) 위험하므로,
#   이 테스트가 sizeof를 스펙 길이(interface-list.csv '길이(헤더제외)')에
#   못박아 회귀를 컴파일/런타임에서 잡는다.
#
#   스펙 근거: docs/reference/krx-exture3/ (v3.24, 2026-07-30)
#     KRX_HEADER      = 82  (공통 헤더)
#     KRX_MSG_COMMON  = 106 (헤더82 + 공통바디24)
#     KRX_JUMUN_DATA   = 294 (TCHODR10001 호가입력, 현/파)
#     KRX_SETTLE_DATA  = 233 (TTRTDP21301 회원체결결과, 현/파)
#     TTRODP11301_DATA = 318 (TTRODP11301 회원처리호가 응답, 현/파)
------------------------------------------------------------------------*/
#include "unity.h"
#include <stddef.h>
#include "pa_struct.h"
#include "krx_ttrodp11301.h"
#include "krx_ttrtdp21301.h"

void setUp(void) {}
void tearDown(void) {}

/*-- 공통 헤더 82B --*/
void test_krx_header_82(void)
{
    TEST_ASSERT_EQUAL_INT(82, (int)sizeof(KRX_HEADER));
}

/*-- 공통 바디부(24) + 통합(106) --*/
void test_krx_msg_common(void)
{
    TEST_ASSERT_EQUAL_INT(24,  (int)sizeof(KRX_BODY_COMMON));
    TEST_ASSERT_EQUAL_INT(106, (int)sizeof(KRX_MSG_COMMON));
}

/*-- 현/파 주문(호가입력) TCHODR10001 = 294B --*/
void test_jumun_data_294(void)
{
    TEST_ASSERT_EQUAL_INT(294, (int)sizeof(KRX_JUMUN_DATA));
}

/*-- 현/파 체결결과 TTRTDP21301 = 233B --*/
void test_settle_data_233(void)
{
    TEST_ASSERT_EQUAL_INT(233, (int)sizeof(KRX_SETTLE_DATA));
}

/*-- 현/파 회원처리호가(응답, 정상) TTRODP11301 = 318B --*/
void test_ttrodp11301_data_318(void)
{
    TEST_ASSERT_EQUAL_INT(318, (int)sizeof(TTRODP11301_DATA));
}

/*-- 응답 내부 레이아웃 가드: 앞 2필드(11+11) 뒤 Me_Grp_No 오프셋 = 22 --*/
void test_ttrodp11301_internal_offset(void)
{
    TEST_ASSERT_EQUAL_INT(22, (int)offsetof(TTRODP11301_DATA, Me_Grp_No));
}

/*-- 내부 레이아웃 가드: 앞 3필드(11+11+2) 뒤 Board_Id 오프셋 = 24 --*/
void test_jumun_internal_offset(void)
{
    TEST_ASSERT_EQUAL_INT(24, (int)offsetof(KRX_JUMUN_DATA, Board_Id));
}

/*-- 현/파 회원체결결과 TTRTDP21301 = 233B --*/
void test_ttrtdp21301_data_233(void)
{
    TEST_ASSERT_EQUAL_INT(233, (int)sizeof(TTRTDP21301_DATA));
}

/*-- 체결 내부 레이아웃 가드: Member_Use_Area 오프셋 = 172(회원처리호가감소 파싱 기준) --*/
void test_ttrtdp21301_internal_offset(void)
{
    TEST_ASSERT_EQUAL_INT(172, (int)offsetof(TTRTDP21301_DATA, Member_Use_Area));
    TEST_ASSERT_EQUAL_INT(90,  (int)offsetof(TTRTDP21301_DATA, Trading_Volumn));
    TEST_ASSERT_EQUAL_INT(36,  (int)offsetof(TTRTDP21301_DATA, Order_Identification));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_krx_header_82);
    RUN_TEST(test_krx_msg_common);
    RUN_TEST(test_jumun_data_294);
    RUN_TEST(test_settle_data_233);
    RUN_TEST(test_jumun_internal_offset);
    RUN_TEST(test_ttrodp11301_data_318);
    RUN_TEST(test_ttrodp11301_internal_offset);
    RUN_TEST(test_ttrtdp21301_data_233);
    RUN_TEST(test_ttrtdp21301_internal_offset);
    return UNITY_END();
}
