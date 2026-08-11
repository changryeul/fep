/*------------------------------------------------------------------------
#   Unit Test : Chk_Korean (multi-byte truncation-safety boundary check)
#   File      : test_chk_kor.c
#   SUT       : sub/chk_kor.c
#
#   실제 계약(구현 + 전 호출자 px_chk*.c 기준): 데이터는 UTF-8이며,
#   Chk_Korean은 idx 위치 1바이트를 상태 없이 분류한다.
#     0  : ASCII (상위비트 0) — 절단 안전
#    -1  : UTF-8 continuation 바이트(10xxxxxx) — 멀티바이트 문자 내부, 절단 위험
#     1  : UTF-8 선두 바이트(11xxxxxx) — 새 문자 시작
#
#   호출자는 오직 -1 만 사용: `while (p>0 && Chk_Korean(info,p)==-1) p--;`
#   (경계에서 잘린 멀티바이트 문자를 공백으로 패딩). 따라서 UTF-8
#   continuation 바이트 판별(-1)이 이 함수의 핵심 계약이다.
#
#   주의(2026-08-06): 기존 테스트는 EUC-KR 2바이트 위치모델(첫바이트=1)을
#   가정해 UTF-8 구현/실사용과 불일치 → 3건 오탐 실패했다. UTF-8로 정정.
------------------------------------------------------------------------*/
#include "unity.h"

/* SUT prototype */
extern int Chk_Korean(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Pure ASCII (상위비트 없음) → 0 --*/
void test_ascii_digit(void)
{
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("0123", 0));
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("0123", 3));
}

void test_ascii_alpha(void)
{
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("abcd", 0));
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("ABCD", 2));
}

void test_ascii_space(void)
{
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("   ", 1));
}

/*-- UTF-8 3바이트 한글 "가" (U+AC00) = 0xEA 0xB0 0x80 --*/
void test_utf8_lead_byte(void)
{
    char kr[] = { (char)0xEA, (char)0xB0, (char)0x80, 0 };
    /* idx=0: 선두 바이트(11101010) → 새 문자 시작 → 1 */
    TEST_ASSERT_EQUAL_INT(1, Chk_Korean(kr, 0));
}

void test_utf8_continuation_bytes(void)
{
    char kr[] = { (char)0xEA, (char)0xB0, (char)0x80, 0 };
    /* idx=1,2: continuation 바이트(10xxxxxx) → 문자 내부 → -1 */
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr, 1));
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr, 2));
}

/*-- ASCII 뒤 UTF-8 한글 --*/
void test_mixed_ascii_before_korean(void)
{
    /* "AB" + "가"(EA B0 80) */
    char mix[] = { 'A', 'B', (char)0xEA, (char)0xB0, (char)0x80, 0 };
    TEST_ASSERT_EQUAL_INT(0,  Chk_Korean(mix, 0));   /* 'A' ASCII        */
    TEST_ASSERT_EQUAL_INT(1,  Chk_Korean(mix, 2));   /* 한글 선두        */
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(mix, 3));   /* continuation     */
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(mix, 4));   /* continuation     */
}

/*-- UTF-8 한글 두 글자 연속: "가나" = EA B0 80 EB 82 98 --*/
void test_two_korean_chars(void)
{
    char kr2[] = { (char)0xEA, (char)0xB0, (char)0x80,
                   (char)0xEB, (char)0x82, (char)0x98, 0 };
    TEST_ASSERT_EQUAL_INT(1,  Chk_Korean(kr2, 0));   /* 1st 선두         */
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr2, 1));
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr2, 2));
    TEST_ASSERT_EQUAL_INT(1,  Chk_Korean(kr2, 3));   /* 2nd 선두         */
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr2, 4));
    TEST_ASSERT_EQUAL_INT(-1, Chk_Korean(kr2, 5));
}

/*-- Negative index → 0 --*/
void test_negative_index(void)
{
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean("abc", -1));
}

/*-- 한글 뒤 ASCII: 경계 바이트가 ASCII면 0 --*/
void test_korean_then_ascii(void)
{
    char mix[] = { (char)0xEA, (char)0xB0, (char)0x80, 'X', 0 };
    TEST_ASSERT_EQUAL_INT(0, Chk_Korean(mix, 3));   /* 'X' ASCII */
}

/*-- 실제 호출자 사용 패턴: 경계에서 continuation이면 문자 시작까지 back-up --*/
void test_truncation_backup_usage(void)
{
    /* "A" + "가"(EA B0 80), 버퍼를 idx=2(한글 중간)에서 자르려는 상황.
       호출자: while (p>0 && Chk_Korean==-1) p--;  → 선두(idx 1)까지 후퇴 */
    char mix[] = { 'A', (char)0xEA, (char)0xB0, (char)0x80, 0 };
    int p = 3;
    while (p > 0 && Chk_Korean(mix, p) == -1) p--;
    /* idx3=-1, idx2=-1, idx1=1(멈춤) → p=1 (한글 선두, 안전 절단 경계) */
    TEST_ASSERT_EQUAL_INT(1, p);
    TEST_ASSERT_EQUAL_INT(1, Chk_Korean(mix, p));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ascii_digit);
    RUN_TEST(test_ascii_alpha);
    RUN_TEST(test_ascii_space);
    RUN_TEST(test_utf8_lead_byte);
    RUN_TEST(test_utf8_continuation_bytes);
    RUN_TEST(test_mixed_ascii_before_korean);
    RUN_TEST(test_two_korean_chars);
    RUN_TEST(test_negative_index);
    RUN_TEST(test_korean_then_ascii);
    RUN_TEST(test_truncation_backup_usage);
    return UNITY_END();
}
