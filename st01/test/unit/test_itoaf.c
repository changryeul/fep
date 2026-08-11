/*------------------------------------------------------------------------
#   Unit Test : ItoAf (integer to string conversion)
#   File      : test_itoaf.c
#   SUT       : sub/itoaf.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* SUT prototype */
extern char *ItoAf(int, char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic conversion (right-aligned, zero-padded) --*/
void test_simple(void)
{
    char buf[8];
    ItoAf(42, buf, 4);
    TEST_ASSERT_EQUAL_MEMORY("0042", buf, 4);
}

void test_zero(void)
{
    char buf[8];
    ItoAf(0, buf, 4);
    TEST_ASSERT_EQUAL_MEMORY("0000", buf, 4);
}

void test_exact_fit(void)
{
    char buf[8];
    ItoAf(1234, buf, 4);
    TEST_ASSERT_EQUAL_MEMORY("1234", buf, 4);
}

/*-- KRX process ID format (4 digits) --*/
void test_krx_process_id_1101(void)
{
    char buf[8];
    ItoAf(1101, buf, 4);
    TEST_ASSERT_EQUAL_MEMORY("1101", buf, 4);
}

void test_krx_process_id_2201(void)
{
    char buf[8];
    ItoAf(2201, buf, 4);
    TEST_ASSERT_EQUAL_MEMORY("2201", buf, 4);
}

/*-- Single digit --*/
void test_single_digit(void)
{
    char buf[8];
    ItoAf(7, buf, 1);
    TEST_ASSERT_EQUAL_MEMORY("7", buf, 1);
}

/*-- Larger field width --*/
void test_wide_field(void)
{
    char buf[16];
    ItoAf(123, buf, 8);
    TEST_ASSERT_EQUAL_MEMORY("00000123", buf, 8);
}

/*-- Returns pointer to same buffer --*/
void test_return_value(void)
{
    char buf[8];
    char *ret = ItoAf(99, buf, 4);
    TEST_ASSERT_EQUAL_PTR(buf, ret);
}

/*-- KRX port number (5 digits) --*/
void test_krx_port(void)
{
    char buf[8];
    ItoAf(21101, buf, 5);
    TEST_ASSERT_EQUAL_MEMORY("21101", buf, 5);
}

/*-- Overflow: value wider than field (truncation from left) --*/
void test_overflow(void)
{
    char buf[8];
    /* 12345 in 3 chars: takes lowest 3 digits via modulo */
    ItoAf(12345, buf, 3);
    TEST_ASSERT_EQUAL_MEMORY("345", buf, 3);
}

/*
 * NOTE: ItoAf does NOT support negative numbers.
 * KRX fixed-width fields are always non-negative.
 * Negative input produces undefined output (modulo behavior).
 */

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_zero);
    RUN_TEST(test_exact_fit);
    RUN_TEST(test_krx_process_id_1101);
    RUN_TEST(test_krx_process_id_2201);
    RUN_TEST(test_single_digit);
    RUN_TEST(test_wide_field);
    RUN_TEST(test_return_value);
    RUN_TEST(test_krx_port);
    RUN_TEST(test_overflow);
    return UNITY_END();
}
