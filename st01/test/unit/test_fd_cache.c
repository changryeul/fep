/*------------------------------------------------------------------------
#   Unit Test : Fd_Cache_Get / Fp_Cache_Get / Fd_Cache_Close_All
#   File      : test_fd_cache.c
#   SUT       : sub/fd_cache.c
------------------------------------------------------------------------*/
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* SUT prototypes */
extern int   Fd_Cache_Get(const char *p_path, int p_flags, int p_mode);
extern FILE *Fp_Cache_Get(const char *p_path, const char *p_mode);
extern void  Fd_Cache_Close_All(void);

#define TEST_DIR    "./fdc_test_tmp"

void setUp(void)
{
    char cmd[300];
    sprintf(cmd, "rm -rf %s && mkdir -p %s", TEST_DIR, TEST_DIR);
    system(cmd);
}

void tearDown(void)
{
    char cmd[300];
    Fd_Cache_Close_All();
    sprintf(cmd, "rm -rf %s", TEST_DIR);
    system(cmd);
}

/*-- 동일 경로 반복 요청 → 동일 fd --*/
void test_same_path_same_fd(void)
{
    int fd1, fd2;

    fd1 = Fd_Cache_Get(TEST_DIR "/a.dat", O_RDWR | O_CREAT, 0644);
    fd2 = Fd_Cache_Get(TEST_DIR "/a.dat", O_RDWR | O_CREAT, 0644);

    TEST_ASSERT_TRUE(fd1 >= 0);
    TEST_ASSERT_EQUAL_INT(fd1, fd2);
}

/*-- 다른 경로 → 다른 fd --*/
void test_different_path_different_fd(void)
{
    int fd1, fd2;

    fd1 = Fd_Cache_Get(TEST_DIR "/a.dat", O_RDWR | O_CREAT, 0644);
    fd2 = Fd_Cache_Get(TEST_DIR "/b.dat", O_RDWR | O_CREAT, 0644);

    TEST_ASSERT_TRUE(fd1 >= 0);
    TEST_ASSERT_TRUE(fd2 >= 0);
    TEST_ASSERT_TRUE(fd1 != fd2);
}

/*-- 캐시된 fd로 실제 write/read 가능 --*/
void test_cached_fd_usable(void)
{
    int  fd;
    char buf[8];

    fd = Fd_Cache_Get(TEST_DIR "/c.dat", O_RDWR | O_CREAT, 0644);
    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_EQUAL_INT(4, (int)write(fd, "DATA", 4));

    lseek(fd, 0, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    TEST_ASSERT_EQUAL_INT(4, (int)read(fd, buf, 4));
    TEST_ASSERT_EQUAL_STRING("DATA", buf);
}

/*-- FILE* 변형: 동일 경로 → 동일 FILE*, append 쓰기 동작 --*/
void test_fp_cache(void)
{
    FILE *fp1, *fp2;

    fp1 = Fp_Cache_Get(TEST_DIR "/d.dat", "a+");
    fp2 = Fp_Cache_Get(TEST_DIR "/d.dat", "a+");

    TEST_ASSERT_NOT_NULL(fp1);
    TEST_ASSERT_TRUE(fp1 == fp2);

    fwrite("X", 1, 1, fp1);
    fflush(fp1);
}

/*-- fd와 FILE*는 독립 캐시 (같은 경로라도 별개) --*/
void test_fd_and_fp_independent(void)
{
    int  fd;
    FILE *fp;

    fd = Fd_Cache_Get(TEST_DIR "/e.dat", O_RDWR | O_CREAT, 0644);
    fp = Fp_Cache_Get(TEST_DIR "/e.dat", "a+");

    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_NOT_NULL(fp);
    TEST_ASSERT_TRUE(fileno(fp) != fd);
}

/*-- 존재하지 않는 경로(O_CREAT 없이) → -1/NULL 폴백 --*/
void test_open_failure_returns_fallback(void)
{
    TEST_ASSERT_EQUAL_INT(-1,
        Fd_Cache_Get(TEST_DIR "/no/such/dir/x.dat", O_RDWR, 0644));
    TEST_ASSERT_NULL(Fp_Cache_Get(TEST_DIR "/no/such/dir/y.dat", "r+"));
}

/*-- 슬롯 초과(32개) 시 -1 폴백 --*/
void test_slot_overflow_fallback(void)
{
    char path[300];
    int  i, fd;

    for (i = 0; i < 32; i++) {
        sprintf(path, "%s/f%02d.dat", TEST_DIR, i);
        fd = Fd_Cache_Get(path, O_RDWR | O_CREAT, 0644);
        TEST_ASSERT_TRUE(fd >= 0);
    }

    sprintf(path, "%s/overflow.dat", TEST_DIR);
    TEST_ASSERT_EQUAL_INT(-1, Fd_Cache_Get(path, O_RDWR | O_CREAT, 0644));
}

/*-- Close_All 후 재획득 가능 --*/
void test_close_all_then_reacquire(void)
{
    int fd1, fd2;

    fd1 = Fd_Cache_Get(TEST_DIR "/g.dat", O_RDWR | O_CREAT, 0644);
    TEST_ASSERT_TRUE(fd1 >= 0);

    Fd_Cache_Close_All();

    fd2 = Fd_Cache_Get(TEST_DIR "/g.dat", O_RDWR | O_CREAT, 0644);
    TEST_ASSERT_TRUE(fd2 >= 0);
    /* 캐시가 비워졌으므로 write가 실제로 동작해야 함 */
    TEST_ASSERT_EQUAL_INT(1, (int)write(fd2, "Z", 1));
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_same_path_same_fd);
    RUN_TEST(test_different_path_different_fd);
    RUN_TEST(test_cached_fd_usable);
    RUN_TEST(test_fp_cache);
    RUN_TEST(test_fd_and_fp_independent);
    RUN_TEST(test_open_failure_returns_fallback);
    RUN_TEST(test_slot_overflow_fallback);
    RUN_TEST(test_close_all_then_reacquire);
    return UNITY_END();
}
