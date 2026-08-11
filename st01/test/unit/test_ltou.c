/*------------------------------------------------------------------------
#   Unit Test : LtoU (lowercase to uppercase conversion)
#   File      : test_ltou.c
#   SUT       : sub/ltou.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* SUT prototype */
extern char *LtoU(char *, int);

void setUp(void) {}
void tearDown(void) {}

/*-- Basic conversion --*/
void test_simple(void)
{
    char buf[] = "hello";
    LtoU(buf, 5);
    TEST_ASSERT_EQUAL_STRING("HELLO", buf);
}

void test_already_upper(void)
{
    char buf[] = "HELLO";
    LtoU(buf, 5);
    TEST_ASSERT_EQUAL_STRING("HELLO", buf);
}

void test_mixed_case(void)
{
    char buf[] = "HeLLo WoRLd";
    LtoU(buf, 11);
    TEST_ASSERT_EQUAL_STRING("HELLO WORLD", buf);
}

/*-- Numbers remain unchanged --*/
void test_numbers_unchanged(void)
{
    char buf[] = "abc123def";
    LtoU(buf, 9);
    TEST_ASSERT_EQUAL_STRING("ABC123DEF", buf);
}

/*-- Special characters unchanged --*/
void test_special_chars(void)
{
    char buf[] = "a-b.c_d";
    LtoU(buf, 7);
    TEST_ASSERT_EQUAL_STRING("A-B.C_D", buf);
}

/*-- Partial length --*/
void test_partial_length(void)
{
    char buf[] = "abcdef";
    LtoU(buf, 3);
    /* Only first 3 converted */
    TEST_ASSERT_EQUAL_MEMORY("ABCdef", buf, 6);
}

/*-- Zero length (no-op) --*/
void test_zero_length(void)
{
    char buf[] = "abc";
    LtoU(buf, 0);
    TEST_ASSERT_EQUAL_STRING("abc", buf);
}

/*-- Returns pointer to same buffer --*/
void test_return_value(void)
{
    char buf[] = "test";
    char *ret = LtoU(buf, 4);
    TEST_ASSERT_EQUAL_PTR(buf, ret);
}

/*-- Empty content (spaces) --*/
void test_spaces(void)
{
    char buf[] = "   ";
    LtoU(buf, 3);
    TEST_ASSERT_EQUAL_STRING("   ", buf);
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simple);
    RUN_TEST(test_already_upper);
    RUN_TEST(test_mixed_case);
    RUN_TEST(test_numbers_unchanged);
    RUN_TEST(test_special_chars);
    RUN_TEST(test_partial_length);
    RUN_TEST(test_zero_length);
    RUN_TEST(test_return_value);
    RUN_TEST(test_spaces);
    return UNITY_END();
}
