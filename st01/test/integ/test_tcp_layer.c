/*------------------------------------------------------------------------
#   Module  : Phase 1 — TCP Layer Integration Tests
#   File    : test_tcp_layer.c
#   Purpose : Tests real TCP socket communication with Mock KRX server.
#             The mock server must be running before this test starts.
#
#   Usage   : 1) bin/mock_krx_server 19999 &
#             2) sleep 1
#             3) bin/test_tcp_layer
#             4) kill %1
------------------------------------------------------------------------*/
#include "unity.h"
#include "lib/krx_protocol.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Functions from sub/ that we link against */
extern int Socket(void);
extern int Connect(int p_sfd, char *p_ip_addr, int p_port_no);
extern int Sendn(int p_sfd, char *p_buf, int p_len);
extern int Recvn(int p_sfd, char *p_buf, int p_len);

#define TEST_PORT   19999
#define TEST_IP     "127.0.0.1"

static int g_sockfd = -1;

void setUp(void) {}
void tearDown(void) {}

/*------------------------------------------------------------------------
    Helper: connect to mock server
------------------------------------------------------------------------*/
static int connect_to_mock(void)
{
    int sfd;

    sfd = Socket();
    if (sfd < 0) return -1;

    if (Connect(sfd, TEST_IP, TEST_PORT) < 0) {
        close(sfd);
        return -1;
    }

    return sfd;
}

/*------------------------------------------------------------------------
    Helper: receive KRX message (header + body)
------------------------------------------------------------------------*/
static int recv_krx_msg(int fd, char *buf, int bufsize)
{
    int rt, body_len;

    rt = Recvn(fd, buf, KRX_HDR_LEN);
    if (rt <= 0) return rt;

    body_len = krx_get_body_length(buf);
    if (body_len < 0 || body_len > bufsize - KRX_HDR_LEN)
        return -1;

    if (body_len > 0) {
        rt = Recvn(fd, buf + KRX_HDR_LEN, body_len);
        if (rt <= 0) return rt;
    }

    return KRX_HDR_LEN + body_len;
}

/*========================================================================
    Test: Socket creation
========================================================================*/

void test_socket_create(void)
{
    int sfd = Socket();
    TEST_ASSERT_TRUE(sfd >= 0);
    close(sfd);
}

/*========================================================================
    Test: Connect to mock server
========================================================================*/

void test_connect_to_mock_server(void)
{
    g_sockfd = connect_to_mock();
    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0,
        "Failed to connect to mock KRX server on 127.0.0.1:19999. "
        "Is mock_krx_server running?");
}

/*========================================================================
    Test: LOGON handshake
========================================================================*/

void test_logon_handshake(void)
{
    char send_buf[256], recv_buf[256];
    KRX_PARSED_HEADER hdr;
    int slen, rlen;

    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0, "Socket not connected");

    /* Build and send LOGON request */
    slen = krx_build_logon_req(send_buf, sizeof(send_buf), "00012", "INTEG_TEST");
    TEST_ASSERT_EQUAL_INT(123, slen);

    TEST_ASSERT_EQUAL_INT(slen, Sendn(g_sockfd, send_buf, slen));

    /* Receive LOGON response */
    rlen = recv_krx_msg(g_sockfd, recv_buf, sizeof(recv_buf));
    TEST_ASSERT_EQUAL_INT(123, rlen);

    /* Parse and verify */
    TEST_ASSERT_EQUAL_INT(0, krx_parse_header(recv_buf, rlen, &hdr));
    TEST_ASSERT_EQUAL_STRING("SCHLIR00000", hdr.MsgType);
    TEST_ASSERT_EQUAL_STRING("FEPKRX01", hdr.BeginString);
    TEST_ASSERT_EQUAL_INT(41, hdr.body_length);
}

/*========================================================================
    Test: LINK handshake
========================================================================*/

void test_link_handshake(void)
{
    char send_buf[256], recv_buf[256];
    KRX_PARSED_HEADER hdr;
    int slen, rlen;

    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0, "Socket not connected");

    /* Build and send LINK request */
    slen = krx_build_link_req(send_buf, sizeof(send_buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, slen);

    TEST_ASSERT_EQUAL_INT(slen, Sendn(g_sockfd, send_buf, slen));

    /* Receive LINK response */
    rlen = recv_krx_msg(g_sockfd, recv_buf, sizeof(recv_buf));
    TEST_ASSERT_EQUAL_INT(82, rlen);

    /* Parse and verify */
    TEST_ASSERT_EQUAL_INT(0, krx_parse_header(recv_buf, rlen, &hdr));
    TEST_ASSERT_EQUAL_STRING("SCHOPR00000", hdr.MsgType);
    TEST_ASSERT_EQUAL_INT(0, hdr.body_length);
}

/*========================================================================
    Test: Order data roundtrip
========================================================================*/

void test_order_data_roundtrip(void)
{
    char send_buf[512], recv_buf[512];
    KRX_PARSED_HEADER hdr;
    int slen, rlen;
    char payload[] = "TEST_ORDER_DATA_ABCDEFGH";
    int  pay_len = (int)strlen(payload);

    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0, "Socket not connected");

    /* Build and send ORDER message */
    slen = krx_build_order_msg(send_buf, sizeof(send_buf), "00012",
                               "TTRTDP42301", payload, pay_len);
    TEST_ASSERT_TRUE(slen > 0);

    TEST_ASSERT_EQUAL_INT(slen, Sendn(g_sockfd, send_buf, slen));

    /* Receive echoed response */
    rlen = recv_krx_msg(g_sockfd, recv_buf, sizeof(recv_buf));
    TEST_ASSERT_EQUAL_INT(slen, rlen);

    /* Parse header */
    TEST_ASSERT_EQUAL_INT(0, krx_parse_header(recv_buf, rlen, &hdr));

    /* Verify payload echoed back */
    TEST_ASSERT_EQUAL_MEMORY(payload, recv_buf + KRX_MSG_COMMON_LEN, pay_len);

    /* Verify TR code in body_common */
    TEST_ASSERT_EQUAL_MEMORY("TTRTDP42301", recv_buf + KRX_OFF_TRCODE, 11);
}

/*========================================================================
    Test: POLL heartbeat
========================================================================*/

void test_poll_heartbeat(void)
{
    char send_buf[256], recv_buf[256];
    KRX_PARSED_HEADER hdr;
    int slen, rlen;

    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0, "Socket not connected");

    /* Build and send POLL request */
    slen = krx_build_poll_req(send_buf, sizeof(send_buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, slen);

    TEST_ASSERT_EQUAL_INT(slen, Sendn(g_sockfd, send_buf, slen));

    /* Receive POLL response */
    rlen = recv_krx_msg(g_sockfd, recv_buf, sizeof(recv_buf));
    TEST_ASSERT_EQUAL_INT(82, rlen);

    TEST_ASSERT_EQUAL_INT(0, krx_parse_header(recv_buf, rlen, &hdr));
    TEST_ASSERT_EQUAL_STRING("SCHHER00000", hdr.MsgType);
}

/*========================================================================
    Test: LOGOFF and close
========================================================================*/

void test_logoff_close(void)
{
    char send_buf[256];
    int slen;

    TEST_ASSERT_TRUE_MESSAGE(g_sockfd >= 0, "Socket not connected");

    /* Send LOGOFF */
    slen = krx_build_logoff_req(send_buf, sizeof(send_buf), "00012");
    TEST_ASSERT_EQUAL_INT(82, slen);

    TEST_ASSERT_EQUAL_INT(slen, Sendn(g_sockfd, send_buf, slen));

    /* Close socket */
    close(g_sockfd);
    g_sockfd = -1;
}

/*========================================================================
    Main — tests must run in sequence (stateful TCP session)
========================================================================*/

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_socket_create);
    RUN_TEST(test_connect_to_mock_server);
    RUN_TEST(test_logon_handshake);
    RUN_TEST(test_link_handshake);
    RUN_TEST(test_order_data_roundtrip);
    RUN_TEST(test_poll_heartbeat);
    RUN_TEST(test_logoff_close);

    return UNITY_END();
}
