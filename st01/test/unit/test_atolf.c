/*------------------------------------------------------------------------
#   Unit Test : AtoLf (string to long conversion)
#   File      : test_atolf.c
#   SUT       : sub/atolf.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <limits.h>

/* SUT prototype */
extern long AtoLf(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic conversion --*/
void test_simple_number(void)
{
    TEST_ASSERT_EQUAL_INT64(123L, AtoLf("123", 3));
}

void test_zero(void)
{
    TEST_ASSERT_EQUAL_INT64(0L, AtoLf("0", 1));
    TEST_ASSERT_EQUAL_INT64(0L, AtoLf("000", 3));
}

/*-- Leading zeros --*/
void test_leading_zeros(void)
{
    TEST_ASSERT_EQUAL_INT64(42L, AtoLf("0042", 4));
}

/*-- KRX DataSeq field (11 digits) --*/
void test_krx_dataseq(void)
{
#if LONG_MAX > 2147483647L
    TEST_ASSERT_EQUAL_INT64(12345678901L, AtoLf("12345678901", 11));
#else
    /* 32-bit long: skip large value test */
    TEST_IGNORE_MESSAGE("32-bit long: skip 11-digit test");
#endif
}

void test_krx_dataseq_leading_zeros(void)
{
#if LONG_MAX > 2147483647L
    TEST_ASSERT_EQUAL_INT64(1L, AtoLf("00000000001", 11));
#else
    TEST_IGNORE_MESSAGE("32-bit long: skip 11-digit test");
#endif
}

/*-- Space padding --*/
void test_space_padded(void)
{
    TEST_ASSERT_EQUAL_INT64(100L, AtoLf("  100  ", 7));
}

/*-- Negative --*/
void test_negative(void)
{
    TEST_ASSERT_EQUAL_INT64(-999L, AtoLf("-999", 4));
}

void test_negative_long(void)
{
#if LONG_MAX > 2147483647L
    TEST_ASSERT_EQUAL_INT64(-12345678901L, AtoLf("-12345678901", 12));
#else
    TEST_IGNORE_MESSAGE("32-bit long: skip large negative test");
#endif
}

/*-- Partial length --*/
void test_partial_length(void)
{
    TEST_ASSERT_EQUAL_INT64(123L, AtoLf("12345", 3));
}

/*-- Zero length --*/
void test_zero_length(void)
{
    TEST_ASSERT_EQUAL_INT64(0L, AtoLf("999", 0));
}

/*-- 32-bit boundary --*/
void test_max_32bit(void)
{
    TEST_ASSERT_EQUAL_INT64(2147483647L, AtoLf("2147483647", 10));
}

/*-- All spaces --*/
void test_all_spaces(void)
{
    TEST_ASSERT_EQUAL_INT64(0L, AtoLf("     ", 5));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simple_number);
    RUN_TEST(test_zero);
    RUN_TEST(test_leading_zeros);
    RUN_TEST(test_krx_dataseq);
    RUN_TEST(test_krx_dataseq_leading_zeros);
    RUN_TEST(test_space_padded);
    RUN_TEST(test_negative);
    RUN_TEST(test_negative_long);
    RUN_TEST(test_partial_length);
    RUN_TEST(test_zero_length);
    RUN_TEST(test_max_32bit);
    RUN_TEST(test_all_spaces);
    return UNITY_END();
}
