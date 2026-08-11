/*------------------------------------------------------------------------
#   Unit Test : Log_Hot — FEP_HOT_LOG=shm 모드 (SLog 라우팅 + 캐싱)
#   File      : test_log_hot_shm.c
#   SUT       : sub/log_hot.c (+ stubs/stub_log.c)
#
#   주의: 모드 캐시는 프로세스당 1회 평가되므로 기본(off) 모드는
#         별도 바이너리 test_log_hot_default에서 검증한다.
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

/*-- shm 모드: SLog로 라우팅 --*/
void test_routes_to_slog(void)
{
    Log_Hot(600, "TCP SD [%s](%d)", "ABC", 42);
    TEST_ASSERT_EQUAL_INT(2, stub_log_called);
}

/*-- err_no 그대로 전달 --*/
void test_errno_forwarded(void)
{
    Log_Hot(600, "hello");
    TEST_ASSERT_EQUAL_INT(600, stub_log_errno);
}

/*-- 포맷 인자 정확히 렌더링 --*/
void test_format_forwarded(void)
{
    Log_Hot(600, "OK22 W[%d] R[%d]", 7, 3);
    TEST_ASSERT_EQUAL_STRING("OK22 W[7] R[3]", stub_log_msg);
}

/*-- 캐싱: env 제거 후에도 shm 모드 유지 --*/
void test_mode_cached(void)
{
    unsetenv("FEP_HOT_LOG");
    Log_Hot(600, "still shm");
    TEST_ASSERT_EQUAL_INT(2, stub_log_called);
    TEST_ASSERT_EQUAL_INT(1, Log_Hot_Mode());
}

/*----------------------------------------------------------------------*/
int main(void)
{
    /* 최초 호출 전에 shm 모드 설정 (캐시 1회 평가) */
    setenv("FEP_HOT_LOG", "shm", 1);

    UNITY_BEGIN();
    RUN_TEST(test_routes_to_slog);
    RUN_TEST(test_errno_forwarded);
    RUN_TEST(test_format_forwarded);
    RUN_TEST(test_mode_cached);
    return UNITY_END();
}
