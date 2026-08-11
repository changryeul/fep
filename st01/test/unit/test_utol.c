/*------------------------------------------------------------------------
#   Unit Test : UtoL (uppercase to lowercase conversion)
#   File      : test_utol.c
#   SUT       : sub/utol.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* SUT prototype */
extern char *UtoL(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic conversion --*/
void test_simple(void)
{
    char buf[] = "HELLO";
    UtoL(buf, 5);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_already_lower(void)
{
    char buf[] = "hello";
    UtoL(buf, 5);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_mixed_case(void)
{
    char buf[] = "HeLLo WoRLd";
    UtoL(buf, 11);
    TEST_ASSERT_EQUAL_STRING("hello world", buf);
}

/*-- Numbers unchanged --*/
void test_numbers_unchanged(void)
{
    char buf[] = "ABC123DEF";
    UtoL(buf, 9);
    TEST_ASSERT_EQUAL_STRING("abc123def", buf);
}

/*-- Special characters unchanged --*/
void test_special_chars(void)
{
    char buf[] = "A-B.C_D";
    UtoL(buf, 7);
    TEST_ASSERT_EQUAL_STRING("a-b.c_d", buf);
}

/*-- Partial length --*/
void test_partial_length(void)
{
    char buf[] = "ABCDEF";
    UtoL(buf, 3);
    TEST_ASSERT_EQUAL_MEMORY("abcDEF", buf, 6);
}

/*-- Zero length (no-op) --*/
void test_zero_length(void)
{
    char buf[] = "ABC";
    UtoL(buf, 0);
    TEST_ASSERT_EQUAL_STRING("ABC", buf);
}

/*-- Returns pointer to same buffer --*/
void test_return_value(void)
{
    char buf[] = "TEST";
    char *ret = UtoL(buf, 4);
    TEST_ASSERT_EQUAL_PTR(buf, ret);
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_already_lower);
    RUN_TEST(test_mixed_case);
    RUN_TEST(test_numbers_unchanged);
    RUN_TEST(test_special_chars);
    RUN_TEST(test_partial_length);
    RUN_TEST(test_zero_length);
    RUN_TEST(test_return_value);
    return UNITY_END();
}
