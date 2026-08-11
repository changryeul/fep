/*------------------------------------------------------------------------
#   Unit Test : lat_report core (parse / pair / percentile)
#   File      : test_lat_report.c
#   SUT       : utl/lat_report.c (-DLAT_REPORT_NO_MAIN 으로 main 제외)
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* SUT (utl/lat_report.c) */
#include "lat_report.c"

void setUp(void) {}
void tearDown(void) {}

/*======================================================================
    lat_parse_line
======================================================================*/
void test_parse_valid_line(void)
{
    LAT_EV ev;
    int rt;

    rt = lat_parse_line("1754300000123456|pb_1101_ts|IN|0000000001\n", &ev);
    TEST_ASSERT_EQUAL_INT(0, rt);
    TEST_ASSERT_TRUE(ev.usec == 1754300000123456LL);
    TEST_ASSERT_EQUAL_STRING("pb_1101_ts", ev.proc);
    TEST_ASSERT_EQUAL_STRING("IN", ev.point);
    TEST_ASSERT_EQUAL_STRING("0000000001", ev.key);
}

void test_parse_line_without_newline(void)
{
    LAT_EV ev;
    TEST_ASSERT_EQUAL_INT(0,
        lat_parse_line("1754300000123456|order_inject|OUT|0000000042", &ev));
    TEST_ASSERT_EQUAL_STRING("0000000042", ev.key);
}

void test_parse_missing_field(void)
{
    LAT_EV ev;
    TEST_ASSERT_EQUAL_INT(-1, lat_parse_line("1754300000123456|proc|IN\n", &ev));
    TEST_ASSERT_EQUAL_INT(-1, lat_parse_line("\n", &ev));
    TEST_ASSERT_EQUAL_INT(-1, lat_parse_line("", &ev));
}

void test_parse_non_numeric_timestamp(void)
{
    LAT_EV ev;
    TEST_ASSERT_EQUAL_INT(-1, lat_parse_line("abc|proc|IN|key\n", &ev));
}

/*======================================================================
    lat_pair
======================================================================*/
void test_pair_single_interval(void)
{
    LAT_EV evs[2];
    long   out[8];
    int    n;

    lat_parse_line("1000000000100000|p1|IN|K1", &evs[0]);
    lat_parse_line("1000000000100250|p1|OUT|K1", &evs[1]);

    n = lat_pair(evs, 2, out);
    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_EQUAL_INT(250, (int)out[0]);
}

void test_pair_unmatched_events(void)
{
    LAT_EV evs[2];
    long   out[8];

    /* OUT만 / IN만 - 짝 없음 */
    lat_parse_line("1000000000100000|p1|OUT|K1", &evs[0]);
    lat_parse_line("1000000000200000|p1|IN|K2", &evs[1]);

    TEST_ASSERT_EQUAL_INT(0, lat_pair(evs, 2, out));
}

void test_pair_key_isolation(void)
{
    LAT_EV evs[2];
    long   out[8];

    /* 다른 key끼리 짝지어지면 안 됨 */
    lat_parse_line("1000000000100000|p1|IN|K1", &evs[0]);
    lat_parse_line("1000000000100300|p1|OUT|K2", &evs[1]);

    TEST_ASSERT_EQUAL_INT(0, lat_pair(evs, 2, out));
}

void test_pair_proc_isolation(void)
{
    LAT_EV evs[2];
    long   out[8];

    /* 다른 proc끼리 짝지어지면 안 됨 */
    lat_parse_line("1000000000100000|p1|IN|K1", &evs[0]);
    lat_parse_line("1000000000100300|p2|OUT|K1", &evs[1]);

    TEST_ASSERT_EQUAL_INT(0, lat_pair(evs, 2, out));
}

void test_pair_multiple_and_unsorted_input(void)
{
    LAT_EV evs[6];
    long   out[8];
    int    n;

    /* 입력 순서가 뒤섞여도 (proc,key,시각) 정렬 후 짝지어야 함 */
    lat_parse_line("1000000000300000|p1|IN|K2",  &evs[0]);
    lat_parse_line("1000000000100100|p1|OUT|K1", &evs[1]);
    lat_parse_line("1000000000100000|p1|IN|K1",  &evs[2]);
    lat_parse_line("1000000000300500|p1|OUT|K2", &evs[3]);
    lat_parse_line("1000000000400000|p2|IN|K1",  &evs[4]);
    lat_parse_line("1000000000400070|p2|OUT|K1", &evs[5]);

    n = lat_pair(evs, 6, out);
    TEST_ASSERT_EQUAL_INT(3, n);
    /* 정렬 후 순서: p1/K1=100, p1/K2=500, p2/K1=70 */
    TEST_ASSERT_EQUAL_INT(100, (int)out[0]);
    TEST_ASSERT_EQUAL_INT(500, (int)out[1]);
    TEST_ASSERT_EQUAL_INT(70,  (int)out[2]);
}

void test_pair_same_key_reused(void)
{
    LAT_EV evs[4];
    long   out[8];
    int    n;

    /* 같은 key로 IN/OUT 두 사이클 (시각순 순차 짝) */
    lat_parse_line("1000000000100000|p1|IN|K1",  &evs[0]);
    lat_parse_line("1000000000100010|p1|OUT|K1", &evs[1]);
    lat_parse_line("1000000000200000|p1|IN|K1",  &evs[2]);
    lat_parse_line("1000000000200020|p1|OUT|K1", &evs[3]);

    n = lat_pair(evs, 4, out);
    TEST_ASSERT_EQUAL_INT(2, n);
    TEST_ASSERT_EQUAL_INT(10, (int)out[0]);
    TEST_ASSERT_EQUAL_INT(20, (int)out[1]);
}

/*======================================================================
    lat_pctl (nearest-rank, 입력은 오름차순 정렬 전제)
======================================================================*/
void test_pctl_single(void)
{
    long v[1];
    v[0] = 42;
    TEST_ASSERT_EQUAL_INT(42, (int)lat_pctl(v, 1, 0.50));
    TEST_ASSERT_EQUAL_INT(42, (int)lat_pctl(v, 1, 0.99));
}

void test_pctl_hundred(void)
{
    long v[100];
    int  i;

    for (i = 0; i < 100; i++)
        v[i] = (i + 1) * 10;    /* 10..1000 */

    TEST_ASSERT_EQUAL_INT(500,  (int)lat_pctl(v, 100, 0.50));  /* rank 50  */
    TEST_ASSERT_EQUAL_INT(900,  (int)lat_pctl(v, 100, 0.90));  /* rank 90  */
    TEST_ASSERT_EQUAL_INT(990,  (int)lat_pctl(v, 100, 0.99));  /* rank 99  */
    TEST_ASSERT_EQUAL_INT(10,   (int)lat_pctl(v, 100, 0.0));   /* min      */
    TEST_ASSERT_EQUAL_INT(1000, (int)lat_pctl(v, 100, 1.0));   /* max      */
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_valid_line);
    RUN_TEST(test_parse_line_without_newline);
    RUN_TEST(test_parse_missing_field);
    RUN_TEST(test_parse_non_numeric_timestamp);
    RUN_TEST(test_pair_single_interval);
    RUN_TEST(test_pair_unmatched_events);
    RUN_TEST(test_pair_key_isolation);
    RUN_TEST(test_pair_proc_isolation);
    RUN_TEST(test_pair_multiple_and_unsorted_input);
    RUN_TEST(test_pair_same_key_reused);
    RUN_TEST(test_pctl_single);
    RUN_TEST(test_pctl_hundred);
    return UNITY_END();
}
