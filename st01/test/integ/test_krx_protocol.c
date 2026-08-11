/*------------------------------------------------------------------------
#   Module  : Phase 1 — KRX Protocol Builder/Parser Tests
#   File    : test_krx_protocol.c
#   Purpose : Unity tests for krx_protocol.c message building and parsing.
------------------------------------------------------------------------*/
#include "unity.h"
#include "lib/krx_protocol.h"

#include <string.h>
#include <stdio.h>

void setUp(void) {}
void tearDown(void) {}

/*------------------------------------------------------------------------
    Helper: compare fixed-width field (not null-terminated in raw buffer)
------------------------------------------------------------------------*/
static int field_eq(const char *buf, int offset, int len, const char *expected)
{
    return (memcmp(buf + offset, expected, len) == 0);
}

/*========================================================================
    Header field layout tests
========================================================================*/

void test_header_size_is_82_bytes(void)
{
    /* Verify our constants match the real struct layout */
    TEST_ASSERT_EQUAL_INT(82, KRX_HDR_LEN);
    TEST_ASSERT_EQUAL_INT(24, KRX_BODY_COMMON_LEN);
    TEST_ASSERT_EQUAL_INT(106, KRX_MSG_COMMON_LEN);
}

void test_header_field_offsets(void)
{
    /* Verify field offsets add up correctly:
       8+6+11+11+5+10+10+17+3+1 = 82 */
    TEST_ASSERT_EQUAL_INT(0,  KRX_OFF_BEGIN);
    TEST_ASSERT_EQUAL_INT(8,  KRX_OFF_BODYLEN);
    TEST_ASSERT_EQUAL_INT(14, KRX_OFF_MSGTYPE);
    TEST_ASSERT_EQUAL_INT(25, KRX_OFF_MSGSEQNUM);
    TEST_ASSERT_EQUAL_INT(36, KRX_OFF_SENDER);
    TEST_ASSERT_EQUAL_INT(41, KRX_OFF_DELIVER);
    TEST_ASSERT_EQUAL_INT(51, KRX_OFF_ONBEHALF);
    TEST_ASSERT_EQUAL_INT(61, KRX_OFF_SENDTIME);
    TEST_ASSERT_EQUAL_INT(78, KRX_OFF_DATACNT);
    TEST_ASSERT_EQUAL_INT(81, KRX_OFF_ENCRYPT);
}

/*========================================================================
    Build header tests
========================================================================*/

void test_build_header_basic(void)
{
    char buf[256];
    int  rt;

    rt = krx_build_header(buf, sizeof(buf), "SCHLIQ00000", 41, "00012", 1);
    TEST_ASSERT_EQUAL_INT(82, rt);

    /* BeginString */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_BEGIN, 8, "FEPKRX01"));
    /* BodyLength */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_BODYLEN, 6, "000041"));
    /* MsgType */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, "SCHLIQ00000"));
    /* MsgSeqNum */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGSEQNUM, 11, "00000000001"));
    /* SenderCompID */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_SENDER, 5, "00012"));
    /* Encrypt */
    TEST_ASSERT_EQUAL_CHAR('N', buf[KRX_OFF_ENCRYPT]);
}

void test_build_header_null_buffer(void)
{
    TEST_ASSERT_EQUAL_INT(-1, krx_build_header(NULL, 256, "SCHLIQ00000", 0, "00012", 1));
}

void test_build_header_small_buffer(void)
{
    char buf[10];
    TEST_ASSERT_EQUAL_INT(-1, krx_build_header(buf, 10, "SCHLIQ00000", 0, "00012", 1));
}

/*========================================================================
    Logon message tests
========================================================================*/

void test_build_logon_req(void)
{
    char buf[256];
    int  len;

    len = krx_build_logon_req(buf, sizeof(buf), "00012", "PROC_INFO1");
    TEST_ASSERT_EQUAL_INT(82 + 41, len);    /* 123 bytes total */

    /* Header: MsgType = SCHLIQ00000, BodyLength = 41 */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_LOGON_REQ));
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_BODYLEN, 6, "000041"));

    /* Session data: proc_info at offset 82, 10 chars */
    TEST_ASSERT_TRUE(field_eq(buf, 82, 10, "PROC_INFO1"));

    /* Encrypt flag at offset 82+40 */
    TEST_ASSERT_EQUAL_CHAR('N', buf[82 + 40]);
}

void test_build_logon_resp_success(void)
{
    char buf[256];
    int  len;

    len = krx_build_logon_resp(buf, sizeof(buf), "00012", 1);
    TEST_ASSERT_EQUAL_INT(123, len);

    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_LOGON_RESP));
    /* Success: proc_info = "0000000000" */
    TEST_ASSERT_TRUE(field_eq(buf, 82, 10, "0000000000"));
}

void test_build_logon_resp_failure(void)
{
    char buf[256];
    int  len;

    len = krx_build_logon_resp(buf, sizeof(buf), "00012", 0);
    TEST_ASSERT_EQUAL_INT(123, len);

    /* Failure: proc_info = "9999999999" */
    TEST_ASSERT_TRUE(field_eq(buf, 82, 10, "9999999999"));
}

/*========================================================================
    Link / Poll / Logoff message tests
========================================================================*/

void test_build_link_req(void)
{
    char buf[256];
    int  len;

    len = krx_build_link_req(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, len);
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_LINK_REQ));
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_BODYLEN, 6, "000000"));
}

void test_build_link_resp(void)
{
    char buf[256];
    int  len;

    len = krx_build_link_resp(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, len);
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_LINK_RESP));
}

void test_build_poll_req(void)
{
    char buf[256];
    int  len;

    len = krx_build_poll_req(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, len);
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_POLL_REQ));
}

void test_build_poll_resp(void)
{
    char buf[256];
    int  len;

    len = krx_build_poll_resp(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, len);
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_POLL_RESP));
}

void test_build_logoff_req(void)
{
    char buf[256];
    int  len;

    len = krx_build_logoff_req(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, len);
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, KRX_LOGOFF_REQ));
}

/*========================================================================
    Order message tests
========================================================================*/

void test_build_order_msg(void)
{
    char buf[512];
    int  len;
    char payload[] = "ORDER_DATA_12345";
    int  pay_len = (int)strlen(payload);

    len = krx_build_order_msg(buf, sizeof(buf), "00012",
                              "TTRTDP42301", payload, pay_len);

    /* total = 82 + 24 + pay_len */
    TEST_ASSERT_EQUAL_INT(82 + 24 + pay_len, len);

    /* Header MsgType = TCHODR00000 */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MSGTYPE, 11, "TCHODR00000"));

    /* BodyLength = 24 + pay_len */
    {
        char expected_bl[7];
        sprintf(expected_bl, "%06d", 24 + pay_len);
        TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_BODYLEN, 6, expected_bl));
    }

    /* Body common: Transaction_Code at offset 93 */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_TRCODE, 11, "TTRTDP42301"));

    /* Megrp_no at offset 104 */
    TEST_ASSERT_TRUE(field_eq(buf, KRX_OFF_MEGRP, 2, "01"));

    /* Payload at offset 106 */
    TEST_ASSERT_EQUAL_MEMORY(payload, buf + KRX_MSG_COMMON_LEN, pay_len);
}

void test_build_order_msg_null_data(void)
{
    char buf[256];
    int  len;

    len = krx_build_order_msg(buf, sizeof(buf), "00012", "TTRTDP42301", NULL, 0);
    TEST_ASSERT_EQUAL_INT(106, len);    /* header + body_common, no payload */
}

/*========================================================================
    Parser tests
========================================================================*/

void test_parse_header_roundtrip(void)
{
    char buf[256];
    KRX_PARSED_HEADER hdr;
    int rt;

    krx_build_logon_req(buf, sizeof(buf), "00012", "TESTPROC01");
    rt = krx_parse_header(buf, 123, &hdr);

    TEST_ASSERT_EQUAL_INT(0, rt);
    TEST_ASSERT_EQUAL_STRING("FEPKRX01", hdr.BeginString);
    TEST_ASSERT_EQUAL_STRING("SCHLIQ00000", hdr.MsgType);
    TEST_ASSERT_EQUAL_STRING("00012", hdr.SenderCompID);
    TEST_ASSERT_EQUAL_INT(41, hdr.body_length);
    TEST_ASSERT_EQUAL_STRING("N", hdr.Encrypt);
}

void test_parse_header_link_resp(void)
{
    char buf[256];
    KRX_PARSED_HEADER hdr;

    krx_build_link_resp(buf, sizeof(buf), "00012");
    krx_parse_header(buf, 82, &hdr);

    TEST_ASSERT_EQUAL_STRING("SCHOPR00000", hdr.MsgType);
    TEST_ASSERT_EQUAL_INT(0, hdr.body_length);
}

void test_parse_header_null(void)
{
    KRX_PARSED_HEADER hdr;
    TEST_ASSERT_EQUAL_INT(-1, krx_parse_header(NULL, 82, &hdr));
    TEST_ASSERT_EQUAL_INT(-1, krx_parse_header("abc", 82, NULL));
}

void test_parse_header_short_buffer(void)
{
    char buf[40];
    KRX_PARSED_HEADER hdr;
    TEST_ASSERT_EQUAL_INT(-1, krx_parse_header(buf, 40, &hdr));
}

void test_get_body_length(void)
{
    char buf[256];

    krx_build_logon_req(buf, sizeof(buf), "00012", NULL);
    TEST_ASSERT_EQUAL_INT(41, krx_get_body_length(buf));

    krx_build_link_req(buf, sizeof(buf), "00012");
    TEST_ASSERT_EQUAL_INT(0, krx_get_body_length(buf));
}

void test_get_body_length_null(void)
{
    TEST_ASSERT_EQUAL_INT(-1, krx_get_body_length(NULL));
}

/*========================================================================
    Main
========================================================================*/

int main(void)
{
    UNITY_BEGIN();

    /* Layout */
    RUN_TEST(test_header_size_is_82_bytes);
    RUN_TEST(test_header_field_offsets);

    /* Header builder */
    RUN_TEST(test_build_header_basic);
    RUN_TEST(test_build_header_null_buffer);
    RUN_TEST(test_build_header_small_buffer);

    /* Session messages */
    RUN_TEST(test_build_logon_req);
    RUN_TEST(test_build_logon_resp_success);
    RUN_TEST(test_build_logon_resp_failure);
    RUN_TEST(test_build_link_req);
    RUN_TEST(test_build_link_resp);
    RUN_TEST(test_build_poll_req);
    RUN_TEST(test_build_poll_resp);
    RUN_TEST(test_build_logoff_req);

    /* Order message */
    RUN_TEST(test_build_order_msg);
    RUN_TEST(test_build_order_msg_null_data);

    /* Parser */
    RUN_TEST(test_parse_header_roundtrip);
    RUN_TEST(test_parse_header_link_resp);
    RUN_TEST(test_parse_header_null);
    RUN_TEST(test_parse_header_short_buffer);
    RUN_TEST(test_get_body_length);
    RUN_TEST(test_get_body_length_null);

    return UNITY_END();
}
