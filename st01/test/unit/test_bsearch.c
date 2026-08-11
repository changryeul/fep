/*------------------------------------------------------------------------
#   Unit Test : BsearchMode, CmpExpcode, CmpLongcode
#   File      : test_bsearch.c
#   SUT       : sub/key_search.c
#
#   Tests BsearchMode with 5 modes: LT(1), LE(2), EQ(3), GE(4), GT(5)
#   Key_Search is NOT tested here (requires SHM).
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* Mode constants from key_code.h */
#define LT   1
#define LE   2
#define EQ   3
#define GE   4
#define GT   5

/* Structs from key_code.h */
typedef struct { int idx; char expcode[12]; } KS_EXPCODE;
typedef struct { int idx; char longcode[20]; } KS_LONGCODE;

/* SUT prototypes */
extern int BsearchMode(char *, char *, int, int, int (*)(), int);
extern int CmpExpcode(const void *, const void *);
extern int CmpLongcode(const void *, const void *);

void setUp(void) {}
void tearDown(void) {}

/*========================================================================
 * Helper: build a sorted KS_EXPCODE array
 *========================================================================*/
static int test_int_cmp(const void *a, const void *b)
{
    return (*(const int *)a - *(const int *)b);
}

/*========================================================================
 * BsearchMode with simple int comparator
 *========================================================================*/

/* Sorted array: {10, 20, 30, 40, 50} */
static int sorted5[] = { 10, 20, 30, 40, 50 };
#define N5 5

/*-- EQ mode --*/
void test_eq_found(void)
{
    int key = 30;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, EQ);
    TEST_ASSERT_EQUAL_INT(2, pos);
}

void test_eq_first(void)
{
    int key = 10;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, EQ);
    TEST_ASSERT_EQUAL_INT(0, pos);
}

void test_eq_last(void)
{
    int key = 50;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, EQ);
    TEST_ASSERT_EQUAL_INT(4, pos);
}

void test_eq_not_found(void)
{
    int key = 25;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, EQ);
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

/*-- LE mode (less than or equal) --*/
void test_le_exact(void)
{
    int key = 30;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, LE);
    TEST_ASSERT_EQUAL_INT(2, pos);
}

void test_le_between(void)
{
    int key = 25;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, LE);
    /* 20 <= 25, so return index of 20 */
    TEST_ASSERT_EQUAL_INT(1, pos);
}

void test_le_below_min(void)
{
    int key = 5;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, LE);
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

/*-- LT mode (strictly less than) --*/
void test_lt_exact(void)
{
    int key = 30;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, LT);
    /* strictly less: return index of 20 */
    TEST_ASSERT_EQUAL_INT(1, pos);
}

void test_lt_first_element(void)
{
    int key = 10;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, LT);
    /* nothing < 10 in array */
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

/*-- GE mode (greater than or equal) --*/
void test_ge_exact(void)
{
    int key = 30;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, GE);
    TEST_ASSERT_EQUAL_INT(2, pos);
}

void test_ge_between(void)
{
    int key = 25;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, GE);
    /* 30 >= 25, so return index of 30 */
    TEST_ASSERT_EQUAL_INT(2, pos);
}

void test_ge_above_max(void)
{
    int key = 55;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, GE);
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

/*-- GT mode (strictly greater than) --*/
void test_gt_exact(void)
{
    int key = 30;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, GT);
    /* strictly greater: return index of 40 */
    TEST_ASSERT_EQUAL_INT(3, pos);
}

void test_gt_last_element(void)
{
    int key = 50;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           N5, sizeof(int), test_int_cmp, GT);
    /* nothing > 50 */
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

/*-- Edge cases --*/
void test_empty_array(void)
{
    int key = 10;
    int pos = BsearchMode((char *)sorted5, (char *)&key,
                           0, sizeof(int), test_int_cmp, EQ);
    TEST_ASSERT_EQUAL_INT(-1, pos);
}

void test_single_element_found(void)
{
    int arr[] = { 42 };
    int key = 42;
    TEST_ASSERT_EQUAL_INT(0, BsearchMode((char *)arr, (char *)&key,
                           1, sizeof(int), test_int_cmp, EQ));
    TEST_ASSERT_EQUAL_INT(0, BsearchMode((char *)arr, (char *)&key,
                           1, sizeof(int), test_int_cmp, LE));
    TEST_ASSERT_EQUAL_INT(0, BsearchMode((char *)arr, (char *)&key,
                           1, sizeof(int), test_int_cmp, GE));
}

void test_single_element_not_found(void)
{
    int arr[] = { 42 };
    int key = 99;
    TEST_ASSERT_EQUAL_INT(-1, BsearchMode((char *)arr, (char *)&key,
                            1, sizeof(int), test_int_cmp, EQ));
}

void test_two_elements(void)
{
    int arr[] = { 10, 20 };
    int key;

    key = 10;
    TEST_ASSERT_EQUAL_INT(0, BsearchMode((char *)arr, (char *)&key,
                           2, sizeof(int), test_int_cmp, EQ));

    key = 20;
    TEST_ASSERT_EQUAL_INT(1, BsearchMode((char *)arr, (char *)&key,
                           2, sizeof(int), test_int_cmp, EQ));

    key = 15;
    TEST_ASSERT_EQUAL_INT(-1, BsearchMode((char *)arr, (char *)&key,
                            2, sizeof(int), test_int_cmp, EQ));
}

/*========================================================================
 * CmpExpcode
 *========================================================================*/
void test_cmpexpcode_equal(void)
{
    KS_EXPCODE a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memcpy(a.expcode, "KR1234567890", 12);
    memcpy(b.expcode, "KR1234567890", 12);
    TEST_ASSERT_EQUAL_INT(0, CmpExpcode(&a, &b));
}

void test_cmpexpcode_less(void)
{
    KS_EXPCODE a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memcpy(a.expcode, "KR0000000001", 12);
    memcpy(b.expcode, "KR9999999999", 12);
    TEST_ASSERT_TRUE(CmpExpcode(&a, &b) < 0);
}

void test_cmpexpcode_greater(void)
{
    KS_EXPCODE a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memcpy(a.expcode, "KR9999999999", 12);
    memcpy(b.expcode, "KR0000000001", 12);
    TEST_ASSERT_TRUE(CmpExpcode(&a, &b) > 0);
}

/*========================================================================
 * CmpLongcode
 *========================================================================*/
void test_cmplongcode_equal(void)
{
    KS_LONGCODE a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memcpy(a.longcode, "US0378331005        ", 20);
    memcpy(b.longcode, "US0378331005        ", 20);
    TEST_ASSERT_EQUAL_INT(0, CmpLongcode(&a, &b));
}

void test_cmplongcode_less(void)
{
    KS_LONGCODE a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memcpy(a.longcode, "AAAA                ", 20);
    memcpy(b.longcode, "ZZZZ                ", 20);
    TEST_ASSERT_TRUE(CmpLongcode(&a, &b) < 0);
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();

    /* BsearchMode - EQ */
    RUN_TEST(test_eq_found);
    RUN_TEST(test_eq_first);
    RUN_TEST(test_eq_last);
    RUN_TEST(test_eq_not_found);

    /* BsearchMode - LE */
    RUN_TEST(test_le_exact);
    RUN_TEST(test_le_between);
    RUN_TEST(test_le_below_min);

    /* BsearchMode - LT */
    RUN_TEST(test_lt_exact);
    RUN_TEST(test_lt_first_element);

    /* BsearchMode - GE */
    RUN_TEST(test_ge_exact);
    RUN_TEST(test_ge_between);
    RUN_TEST(test_ge_above_max);

    /* BsearchMode - GT */
    RUN_TEST(test_gt_exact);
    RUN_TEST(test_gt_last_element);

    /* Edge cases */
    RUN_TEST(test_empty_array);
    RUN_TEST(test_single_element_found);
    RUN_TEST(test_single_element_not_found);
    RUN_TEST(test_two_elements);

    /* CmpExpcode */
    RUN_TEST(test_cmpexpcode_equal);
    RUN_TEST(test_cmpexpcode_less);
    RUN_TEST(test_cmpexpcode_greater);

    /* CmpLongcode */
    RUN_TEST(test_cmplongcode_equal);
    RUN_TEST(test_cmplongcode_less);

    return UNITY_END();
}
