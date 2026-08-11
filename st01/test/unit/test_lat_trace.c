/*------------------------------------------------------------------------
#   Unit Test : Lat_Init / Lat_Point / Lat_Close (order latency trace)
#   File      : test_lat_trace.c
#   SUT       : sub/lat_trace.c
#
#   FEP_LAT_DIR 환경변수 밑에 <proc>.lat 파일을 만들고
#   "<epoch_usec>|<proc>|<point>|<key>" 라인을 기록하는지 검증.
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SUT prototypes (inc/lat_trace.h) */
extern void Lat_Init(const char *p_proc);
extern void Lat_Point(const char *p_point, const char *p_key, int p_klen);
extern void Lat_Close(void);

#define TEST_DIR    "./lat_test_tmp"

static char g_path[256];

void setUp(void)
{
    char cmd[300];
    sprintf(cmd, "rm -rf %s && mkdir -p %s", TEST_DIR, TEST_DIR);
    system(cmd);
    setenv("FEP_LAT_DIR", TEST_DIR, 1);
}

void tearDown(void)
{
    char cmd[300];
    Lat_Close();
    sprintf(cmd, "rm -rf %s", TEST_DIR);
    system(cmd);
}

/* 파일 전체를 읽어 라인 수를 세고 첫 라인을 buf에 복사 */
static int read_lines(const char *path, char *first, int flen)
{
    FILE *fp;
    char line[512];
    int  cnt = 0;

    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (cnt == 0 && first != NULL) {
            strncpy(first, line, flen - 1);
            first[flen - 1] = '\0';
        }
        cnt++;
    }
    fclose(fp);
    return cnt;
}

/*-- 기본: init -> point -> close 후 파일에 1라인 --*/
void test_writes_one_line(void)
{
    char first[512];
    int  cnt;

    Lat_Init("pb_1101_ts");
    Lat_Point("IN", "0000000001", 10);
    Lat_Close();

    sprintf(g_path, "%s/pb_1101_ts.lat", TEST_DIR);
    cnt = read_lines(g_path, first, sizeof(first));
    TEST_ASSERT_EQUAL_INT(1, cnt);
}

/*-- 라인 포맷: usec|proc|point|key --*/
void test_line_format(void)
{
    char first[512];
    char *p1, *p2, *p3;

    Lat_Init("pb_1101_ts");
    Lat_Point("IN", "0000000001", 10);
    Lat_Close();

    sprintf(g_path, "%s/pb_1101_ts.lat", TEST_DIR);
    read_lines(g_path, first, sizeof(first));

    /* 필드 구분자 3개 */
    p1 = strchr(first, '|');
    TEST_ASSERT_NOT_NULL(p1);
    p2 = strchr(p1 + 1, '|');
    TEST_ASSERT_NOT_NULL(p2);
    p3 = strchr(p2 + 1, '|');
    TEST_ASSERT_NOT_NULL(p3);

    /* 1필드: epoch usec 숫자 (16자리 내외, 최소 15자리) */
    TEST_ASSERT_TRUE(p1 - first >= 15);
    /* 2~4필드 내용 */
    TEST_ASSERT_EQUAL_INT(0, strncmp(p1 + 1, "pb_1101_ts", 10));
    TEST_ASSERT_EQUAL_INT(0, strncmp(p2 + 1, "IN", 2));
    TEST_ASSERT_EQUAL_INT(0, strncmp(p3 + 1, "0000000001", 10));
}

/*-- key는 klen 만큼만 기록 (null 종료 없는 고정폭 필드 대응) --*/
void test_key_length_limited(void)
{
    char first[512];
    char *p3;

    Lat_Init("pb_1101_ts");
    /* 뒤에 쓰레기가 붙은 버퍼에서 앞 4바이트만 */
    Lat_Point("OUT", "1234GARBAGE", 4);
    Lat_Close();

    sprintf(g_path, "%s/pb_1101_ts.lat", TEST_DIR);
    read_lines(g_path, first, sizeof(first));

    p3 = strrchr(first, '|');
    TEST_ASSERT_NOT_NULL(p3);
    TEST_ASSERT_EQUAL_INT(0, strncmp(p3 + 1, "1234\n", 5));
}

/*-- proc 인자에 경로가 와도 basename만 사용 (argv[0] 대응) --*/
void test_proc_basename(void)
{
    int cnt;

    Lat_Init("/home/fepp/fep/st01/bin/pb_1101_ts");
    Lat_Point("IN", "0000000001", 10);
    Lat_Close();

    sprintf(g_path, "%s/pb_1101_ts.lat", TEST_DIR);
    cnt = read_lines(g_path, NULL, 0);
    TEST_ASSERT_EQUAL_INT(1, cnt);
}

/*-- Init 없이 Point 호출 시 no-op (crash 없음) --*/
void test_noop_without_init(void)
{
    Lat_Point("IN", "0000000001", 10);
    Lat_Close();
    TEST_PASS();
}

/*-- 여러 이벤트 누적 --*/
void test_multiple_events(void)
{
    int i, cnt;

    Lat_Init("pa_1291_mp");
    for (i = 0; i < 300; i++) {
        Lat_Point("IN",  "0000000001", 10);
        Lat_Point("OUT", "0000000001", 10);
    }
    Lat_Close();

    sprintf(g_path, "%s/pa_1291_mp.lat", TEST_DIR);
    cnt = read_lines(g_path, NULL, 0);
    TEST_ASSERT_EQUAL_INT(600, cnt);
}

/*-- 시각이 단조 비감소 --*/
void test_timestamp_monotonic(void)
{
    FILE *fp;
    char line[512];
    double prev, cur;
    int  first_line;

    Lat_Init("pb_1201_tr");
    Lat_Point("IN",  "00000000001", 11);
    Lat_Point("OUT", "00000000001", 11);
    Lat_Close();

    sprintf(g_path, "%s/pb_1201_tr.lat", TEST_DIR);
    fp = fopen(g_path, "r");
    TEST_ASSERT_NOT_NULL(fp);

    prev = 0;
    first_line = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        cur = atof(line);   /* usec 앞부분만 숫자로 */
        if (!first_line)
            TEST_ASSERT_TRUE(cur >= prev);
        prev = cur;
        first_line = 0;
    }
    fclose(fp);
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_writes_one_line);
    RUN_TEST(test_line_format);
    RUN_TEST(test_key_length_limited);
    RUN_TEST(test_proc_basename);
    RUN_TEST(test_noop_without_init);
    RUN_TEST(test_multiple_events);
    RUN_TEST(test_timestamp_monotonic);
    return UNITY_END();
}
