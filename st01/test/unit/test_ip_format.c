/*------------------------------------------------------------------------
#   Unit Test : format_ip_addr (12-byte packed IP -> dotted notation)
#   File      : test_ip_format.c
#   SUT       : sub/ip_format.c (+ sub/atoif.c dependency)
------------------------------------------------------------------------*/
#include "unity.h"
#include <string.h>

/* SUT prototype */
extern void format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size);

void setUp(void) {}
void tearDown(void) {}

/*-- Standard IP --*/
void test_localhost(void)
{
    char result[16];
    format_ip_addr("127000000001", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("127.0.0.1", result);
}

void test_standard_ip(void)
{
    char result[16];
    format_ip_addr("010001002003", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("10.1.2.3", result);
}

void test_class_c(void)
{
    char result[16];
    format_ip_addr("192168001100", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", result);
}

/*-- All zeros --*/
void test_all_zeros(void)
{
    char result[16];
    format_ip_addr("000000000000", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("0.0.0.0", result);
}

/*-- Max values (255 per octet) --*/
void test_broadcast(void)
{
    char result[16];
    format_ip_addr("255255255255", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("255.255.255.255", result);
}

/*-- KRX server IPs (typical 10.x.x.x range) --*/
void test_krx_server(void)
{
    char result[16];
    format_ip_addr("010100200030", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("10.100.200.30", result);
}

/*-- Leading zeros in octets --*/
void test_leading_zeros(void)
{
    char result[16];
    format_ip_addr("001002003004", result, sizeof(result));
    TEST_ASSERT_EQUAL_STRING("1.2.3.4", result);
}

/*-- Output buffer is cleared --*/
void test_buffer_cleared(void)
{
    char result[16];
    memset(result, 'X', sizeof(result));
    format_ip_addr("010000000001", result, sizeof(result));
    /* After the string, buffer should be zero-terminated */
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", result);
}

/*----------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_localhost);
    RUN_TEST(test_standard_ip);
    RUN_TEST(test_class_c);
    RUN_TEST(test_all_zeros);
    RUN_TEST(test_broadcast);
    RUN_TEST(test_krx_server);
    RUN_TEST(test_leading_zeros);
    RUN_TEST(test_buffer_cleared);
    return UNITY_END();
}
