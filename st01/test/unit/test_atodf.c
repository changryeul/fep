/*------------------------------------------------------------------------
#   Unit Test : AtoDf (string to double conversion)
#   File      : test_atodf.c
#   SUT       : sub/atodf.c
------------------------------------------------------------------------*/
#include "unity.h"

/* SUT prototype */
extern double AtoDf(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic integer --*/
void test_integer(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(123.0, AtoDf("123", 3));
}

void test_zero(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(0.0, AtoDf("0", 1));
}

/*-- Decimal --*/
void test_simple_decimal(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 12.34, AtoDf("12.34", 5));
}

void test_decimal_leading_zero(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.5, AtoDf("0.5", 3));
}

void test_decimal_trailing_zeros(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 1.50, AtoDf("1.50", 4));
}

/*-- Bond price (KRX: 9999.50 format) --*/
void test_bond_price(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 9999.50, AtoDf("09999.50", 8));
}

void test_bond_price_high_precision(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 100.125, AtoDf("0100.125", 8));
}

/*-- Negative --*/
void test_negative_integer(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(-42.0, AtoDf("-42", 3));
}

void test_negative_decimal(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.001, -3.14, AtoDf("-3.14", 5));
}

/*-- Space padding --*/
void test_space_padded(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 12.5, AtoDf("  12.5 ", 7));
}

/*-- Leading zeros --*/
void test_leading_zeros(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 5.25, AtoDf("005.25", 6));
}

/*-- Integer only (no dot) --*/
void test_large_integer(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(999999.0, AtoDf("999999", 6));
}

/*-- Partial length --*/
void test_partial_length(void)
{
    /* Read only "12.3" from "12.34" */
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 12.3, AtoDf("12.34", 4));
}

/*-- Zero length --*/
void test_zero_length(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(0.0, AtoDf("99.99", 0));
}

/*-- All zeros --*/
void test_all_zeros(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(0.0, AtoDf("000.00", 6));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_integer);
    RUN_TEST(test_zero);
    RUN_TEST(test_simple_decimal);
    RUN_TEST(test_decimal_leading_zero);
    RUN_TEST(test_decimal_trailing_zeros);
    RUN_TEST(test_bond_price);
    RUN_TEST(test_bond_price_high_precision);
    RUN_TEST(test_negative_integer);
    RUN_TEST(test_negative_decimal);
    RUN_TEST(test_space_padded);
    RUN_TEST(test_leading_zeros);
    RUN_TEST(test_large_integer);
    RUN_TEST(test_partial_length);
    RUN_TEST(test_zero_length);
    RUN_TEST(test_all_zeros);
    return UNITY_END();
}
