/*------------------------------------------------------------------------
#   Unit Test : Log_Hot — 기본 모드 (FEP_HOT_LOG 미설정 → Log 위임)
#   File      : test_log_hot_default.c
#   SUT       : sub/log_hot.c (+ stubs/stub_log.c)
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdlib.h>
#include <string.h>

/* SUT */
extern int  Log_Hot_Mode(void);
extern void Log_Hot(int p_err_no, const char *p_fmt, ...);

/* stub 캡처 변수 */
extern int  stub_log_called;
extern int  stub_log_errno;
extern char stub_log_msg[];

void setUp(void)
{
    stub_log_called = 0;
    stub_log_errno  = -1;
    stub_log_msg[0] = '\0';
}

void tearDown(void) {}

/*-- 기본: Log로 라우팅 (기존 동작 불변) --*/
void test_routes_to_log(void)
{
    Log_Hot(600, "TCP RD [%s](%d)", "XYZ", 9);
    TEST_ASSERT_EQUAL_INT(1, stub_log_called);
    TEST_ASSERT_EQUAL_INT(0, Log_Hot_Mode());
}

/*-- 포맷/errno 전달 --*/
void test_format_and_errno(void)
{
    Log_Hot(1600, "file write[%s:%d]", "pb_1402_ts", 5);
    TEST_ASSERT_EQUAL_INT(1600, stub_log_errno);
    TEST_ASSERT_EQUAL_STRING("file write[pb_1402_ts:5]", stub_log_msg);
}

/*-- 캐싱: 이후 env를 shm으로 바꿔도 기본 모드 유지 --*/
void test_mode_cached(void)
{
    setenv("FEP_HOT_LOG", "shm", 1);
    Log_Hot(600, "still file");
    TEST_ASSERT_EQUAL_INT(1, stub_log_called);
    TEST_ASSERT_EQUAL_INT(0, Log_Hot_Mode());
}

/*-- "shm" 아닌 값은 기본 모드 --*/
/* (본 바이너리는 main에서 FEP_HOT_LOG=file로 설정하고 시작) */

/*----------------------------------------------------------------------*/
int main(void)
{
    /* "shm"이 아닌 임의 값 → 기본(Log) 모드여야 함 */
    setenv("FEP_HOT_LOG", "file", 1);

    UNITY_BEGIN();
    RUN_TEST(test_routes_to_log);
    RUN_TEST(test_format_and_errno);
    RUN_TEST(test_mode_cached);
    return UNITY_END();
}
