/*------------------------------------------------------------------------
#   Unit Test : AtoIf (string to integer conversion)
#   File      : test_atoif.c
#   SUT       : sub/atoif.c
------------------------------------------------------------------------*/
#include "unity.h"

/* SUT prototype */
extern int AtoIf(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic conversion --*/
void test_simple_number(void)
{
    TEST_ASSERT_EQUAL_INT(123, AtoIf("123", 3));
}

void test_single_digit(void)
{
    TEST_ASSERT_EQUAL_INT(0, AtoIf("0", 1));
    TEST_ASSERT_EQUAL_INT(9, AtoIf("9", 1));
}

void test_zero_string(void)
{
    TEST_ASSERT_EQUAL_INT(0, AtoIf("000", 3));
}

/*-- Leading zeros (KRX fixed-width fields) --*/
void test_leading_zeros(void)
{
    TEST_ASSERT_EQUAL_INT(42, AtoIf("0042", 4));
}

void test_leading_zeros_long(void)
{
    TEST_ASSERT_EQUAL_INT(1101, AtoIf("00001101", 8));
}

/*-- Space padding (KRX fields often space-padded) --*/
void test_space_padded_right(void)
{
    TEST_ASSERT_EQUAL_INT(123, AtoIf("123   ", 6));
}

void test_space_padded_left(void)
{
    TEST_ASSERT_EQUAL_INT(42, AtoIf("   42", 5));
}

void test_space_padded_both(void)
{
    TEST_ASSERT_EQUAL_INT(7, AtoIf("  7  ", 5));
}

/*-- Negative numbers --*/
void test_negative(void)
{
    TEST_ASSERT_EQUAL_INT(-123, AtoIf("-123", 4));
}

void test_negative_with_spaces(void)
{
    TEST_ASSERT_EQUAL_INT(-5, AtoIf(" -5 ", 4));
}

/*-- Partial length --*/
void test_partial_length(void)
{
    /* Only read first 3 chars of "12345" */
    TEST_ASSERT_EQUAL_INT(123, AtoIf("12345", 3));
}

/*-- Zero length --*/
void test_zero_length(void)
{
    TEST_ASSERT_EQUAL_INT(0, AtoIf("999", 0));
}

/*-- KRX process ID format --*/
void test_krx_process_id(void)
{
    TEST_ASSERT_EQUAL_INT(1101, AtoIf("1101", 4));
    TEST_ASSERT_EQUAL_INT(2201, AtoIf("2201", 4));
    TEST_ASSERT_EQUAL_INT(7101, AtoIf("7101", 4));
}

/*-- KRX port number --*/
void test_krx_port(void)
{
    TEST_ASSERT_EQUAL_INT(21101, AtoIf("21101", 5));
}

/*-- All spaces --*/
void test_all_spaces(void)
{
    TEST_ASSERT_EQUAL_INT(0, AtoIf("     ", 5));
}

/*-- Large value --*/
void test_large_value(void)
{
    TEST_ASSERT_EQUAL_INT(999999, AtoIf("999999", 6));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simple_number);
    RUN_TEST(test_single_digit);
    RUN_TEST(test_zero_string);
    RUN_TEST(test_leading_zeros);
    RUN_TEST(test_leading_zeros_long);
    RUN_TEST(test_space_padded_right);
    RUN_TEST(test_space_padded_left);
    RUN_TEST(test_space_padded_both);
    RUN_TEST(test_negative);
    RUN_TEST(test_negative_with_spaces);
    RUN_TEST(test_partial_length);
    RUN_TEST(test_zero_length);
    RUN_TEST(test_krx_process_id);
    RUN_TEST(test_krx_port);
    RUN_TEST(test_all_spaces);
    RUN_TEST(test_large_value);
    return UNITY_END();
}
