/*------------------------------------------------------------------------
#   Module  : Phase 2 — FIFO I/O Integration Tests
#   File    : test_fifo_rw.c
#   Purpose : Tests FIFO read/write using FILE_RW_HEAD format.
#             Uses flat files (not mkfifo) for macOS compatibility.
#             Tests the inject → read → verify cycle directly.
------------------------------------------------------------------------*/
#include "unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

/*------------------------------------------------------------------------
    FILE_RW_HEAD constants (matching fep_file.h)
------------------------------------------------------------------------*/
#define FRH_SEQ_LEN         (8+2)
#define FRH_IFSEQ_LEN       (8+2)
#define FRH_APTYPE_LEN      (8+2)
#define FRH_RESPCODE_LEN    (4+2)
#define FRH_RECVTIME1_LEN   (10+2)
#define FRH_RECVTIME2_LEN   (12+2)
#define FRH_DATAHDR_LEN     (20+2)
#define FILE_RW_HEAD_SIZE   84

/* BUFF_RW_HEAD (no delimiters) = 70 bytes */
#define BUFF_RW_HEAD_SIZE   70

#define MAX_BUF_SIZE        4096
#define TEST_DIR            "/tmp/fep_integ_test"

static char g_test_file[256];

void setUp(void)
{
    mkdir(TEST_DIR, 0755);
    sprintf(g_test_file, "%s/fifo_test_%d", TEST_DIR, getpid());
    unlink(g_test_file);
}

void tearDown(void)
{
    unlink(g_test_file);
}

/*------------------------------------------------------------------------
    Helper: fill delimited field (same as fifo_inject)
------------------------------------------------------------------------*/
static void fill_frh_field(char *dst, int total_width, const char *value)
{
    int vlen, data_width;

    data_width = total_width - 2;
    memset(dst, ' ', total_width);

    if (value) {
        vlen = (int)strlen(value);
        if (vlen > data_width) vlen = data_width;
        memcpy(dst, value, vlen);
    }

    dst[total_width - 2] = '|';
    dst[total_width - 1] = ' ';
}

/*------------------------------------------------------------------------
    Helper: build FILE_RW_HEAD
------------------------------------------------------------------------*/
static void build_test_frh(char *head, int seq)
{
    char tmp[32];
    int offset = 0;

    memset(head, ' ', FILE_RW_HEAD_SIZE);

    sprintf(tmp, "%08d", seq);
    fill_frh_field(head + offset, FRH_SEQ_LEN, tmp);       offset += FRH_SEQ_LEN;
    fill_frh_field(head + offset, FRH_IFSEQ_LEN, tmp);     offset += FRH_IFSEQ_LEN;
    fill_frh_field(head + offset, FRH_APTYPE_LEN, "PB_TS");offset += FRH_APTYPE_LEN;
    fill_frh_field(head + offset, FRH_RESPCODE_LEN, "0000");offset += FRH_RESPCODE_LEN;

    sprintf(tmp, "%010ld", (long)time(NULL));
    fill_frh_field(head + offset, FRH_RECVTIME1_LEN, tmp);  offset += FRH_RECVTIME1_LEN;
    fill_frh_field(head + offset, FRH_RECVTIME2_LEN, "00:00:00-000"); offset += FRH_RECVTIME2_LEN;
    fill_frh_field(head + offset, FRH_DATAHDR_LEN, "TEST_DATA_HEADER");
}

/*========================================================================
    Test: FILE_RW_HEAD size verification
========================================================================*/

void test_file_rw_head_size(void)
{
    /* Verify our constants match:
       10+10+10+6+12+14+22 = 84 */
    TEST_ASSERT_EQUAL_INT(84,
        FRH_SEQ_LEN + FRH_IFSEQ_LEN + FRH_APTYPE_LEN +
        FRH_RESPCODE_LEN + FRH_RECVTIME1_LEN +
        FRH_RECVTIME2_LEN + FRH_DATAHDR_LEN);
}

void test_buff_rw_head_size(void)
{
    /* BUFF_RW_HEAD (no delimiters):
       8+8+8+4+10+12+20 = 70 */
    TEST_ASSERT_EQUAL_INT(70, BUFF_RW_HEAD_SIZE);
}

/*========================================================================
    Test: Write and read back FILE_RW_HEAD + payload
========================================================================*/

void test_write_read_roundtrip(void)
{
    char head[FILE_RW_HEAD_SIZE];
    char payload[] = "TEST_PAYLOAD_ROUNDTRIP_DATA_1234567890";
    int  pay_len = (int)strlen(payload);
    int  fd, nread;
    char read_buf[MAX_BUF_SIZE];
    char lf = '\n';

    /* Write */
    build_test_frh(head, 1);

    fd = open(g_test_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    TEST_ASSERT_TRUE(fd >= 0);

    TEST_ASSERT_EQUAL_INT(FILE_RW_HEAD_SIZE,
                          (int)write(fd, head, FILE_RW_HEAD_SIZE));
    TEST_ASSERT_EQUAL_INT(pay_len, (int)write(fd, payload, pay_len));
    TEST_ASSERT_EQUAL_INT(1, (int)write(fd, &lf, 1));
    close(fd);

    /* Read back */
    fd = open(g_test_file, O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);

    nread = (int)read(fd, read_buf, sizeof(read_buf));
    close(fd);

    TEST_ASSERT_EQUAL_INT(FILE_RW_HEAD_SIZE + pay_len + 1, nread);

    /* Verify header: Seq field starts at offset 0, length 8 */
    TEST_ASSERT_EQUAL_MEMORY("00000001", read_buf, 8);

    /* Verify delimiter after Seq: pipe + space */
    TEST_ASSERT_EQUAL_CHAR('|', read_buf[8]);
    TEST_ASSERT_EQUAL_CHAR(' ', read_buf[9]);

    /* Verify payload */
    TEST_ASSERT_EQUAL_MEMORY(payload, read_buf + FILE_RW_HEAD_SIZE, pay_len);

    /* Verify trailing LF */
    TEST_ASSERT_EQUAL_CHAR('\n', read_buf[FILE_RW_HEAD_SIZE + pay_len]);
}

/*========================================================================
    Test: Multiple records in one file
========================================================================*/

void test_multiple_records(void)
{
    char head[FILE_RW_HEAD_SIZE];
    char payload1[] = "RECORD_ONE_DATA";
    char payload2[] = "RECORD_TWO_DATA_LONGER";
    int  fd, nread;
    char read_buf[MAX_BUF_SIZE];
    char lf = '\n';
    int  rec1_len, rec2_len, total;

    rec1_len = FILE_RW_HEAD_SIZE + (int)strlen(payload1) + 1;
    rec2_len = FILE_RW_HEAD_SIZE + (int)strlen(payload2) + 1;
    total = rec1_len + rec2_len;

    /* Write two records */
    fd = open(g_test_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    TEST_ASSERT_TRUE(fd >= 0);

    build_test_frh(head, 1);
    write(fd, head, FILE_RW_HEAD_SIZE);
    write(fd, payload1, strlen(payload1));
    write(fd, &lf, 1);

    build_test_frh(head, 2);
    write(fd, head, FILE_RW_HEAD_SIZE);
    write(fd, payload2, strlen(payload2));
    write(fd, &lf, 1);

    close(fd);

    /* Read all */
    fd = open(g_test_file, O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    nread = (int)read(fd, read_buf, sizeof(read_buf));
    close(fd);

    TEST_ASSERT_EQUAL_INT(total, nread);

    /* Verify record 1 seq */
    TEST_ASSERT_EQUAL_MEMORY("00000001", read_buf, 8);

    /* Verify record 2 seq */
    TEST_ASSERT_EQUAL_MEMORY("00000002", read_buf + rec1_len, 8);

    /* Verify record 2 payload */
    TEST_ASSERT_EQUAL_MEMORY(payload2,
                             read_buf + rec1_len + FILE_RW_HEAD_SIZE,
                             (int)strlen(payload2));
}

/*========================================================================
    Test: Header field boundaries
========================================================================*/

void test_header_field_boundaries(void)
{
    char head[FILE_RW_HEAD_SIZE];
    int  offset;

    build_test_frh(head, 42);

    /* Check each field delimiter position */

    /* Seq delimiter at offset 8,9 */
    offset = FRH_SEQ_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* If_Seq delimiter */
    offset = FRH_SEQ_LEN + FRH_IFSEQ_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* ApType delimiter */
    offset = FRH_SEQ_LEN + FRH_IFSEQ_LEN + FRH_APTYPE_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* ResponseCode delimiter */
    offset = FRH_SEQ_LEN + FRH_IFSEQ_LEN + FRH_APTYPE_LEN + FRH_RESPCODE_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* RecvTime1 delimiter */
    offset = FRH_SEQ_LEN + FRH_IFSEQ_LEN + FRH_APTYPE_LEN +
             FRH_RESPCODE_LEN + FRH_RECVTIME1_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* RecvTime2 delimiter */
    offset = FRH_SEQ_LEN + FRH_IFSEQ_LEN + FRH_APTYPE_LEN +
             FRH_RESPCODE_LEN + FRH_RECVTIME1_LEN + FRH_RECVTIME2_LEN - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);

    /* DataHeader delimiter (last field) */
    offset = FILE_RW_HEAD_SIZE - 2;
    TEST_ASSERT_EQUAL_CHAR('|', head[offset]);
}

/*========================================================================
    Test: KRX message payload in FIFO format
========================================================================*/

void test_krx_payload_in_fifo(void)
{
    char head[FILE_RW_HEAD_SIZE];
    char krx_msg[256];
    int  fd, nread;
    char read_buf[MAX_BUF_SIZE];
    char lf = '\n';
    int  krx_len;

    /* Build a KRX-like payload (not using krx_protocol.c to avoid dependency) */
    memset(krx_msg, ' ', 106);
    memcpy(krx_msg + 0, "FEPKRX01", 8);        /* BeginString */
    memcpy(krx_msg + 8, "000024", 6);           /* BodyLength */
    memcpy(krx_msg + 14, "TCHODR00000", 11);    /* MsgType */
    memcpy(krx_msg + 93, "TTRTDP42301", 11);    /* Transaction_Code */
    krx_len = 106;

    /* Write FIFO record */
    build_test_frh(head, 100);

    fd = open(g_test_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    TEST_ASSERT_TRUE(fd >= 0);
    write(fd, head, FILE_RW_HEAD_SIZE);
    write(fd, krx_msg, krx_len);
    write(fd, &lf, 1);
    close(fd);

    /* Read and verify KRX content */
    fd = open(g_test_file, O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    nread = (int)read(fd, read_buf, sizeof(read_buf));
    close(fd);

    TEST_ASSERT_EQUAL_INT(FILE_RW_HEAD_SIZE + krx_len + 1, nread);

    /* Verify BeginString in data portion */
    TEST_ASSERT_EQUAL_MEMORY("FEPKRX01",
                             read_buf + FILE_RW_HEAD_SIZE, 8);

    /* Verify MsgType */
    TEST_ASSERT_EQUAL_MEMORY("TCHODR00000",
                             read_buf + FILE_RW_HEAD_SIZE + 14, 11);

    /* Verify Transaction_Code */
    TEST_ASSERT_EQUAL_MEMORY("TTRTDP42301",
                             read_buf + FILE_RW_HEAD_SIZE + 93, 11);
}

/*========================================================================
    Test: Empty payload
========================================================================*/

void test_empty_payload(void)
{
    char head[FILE_RW_HEAD_SIZE];
    int  fd, nread;
    char read_buf[MAX_BUF_SIZE];
    char lf = '\n';

    build_test_frh(head, 0);

    fd = open(g_test_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    TEST_ASSERT_TRUE(fd >= 0);
    write(fd, head, FILE_RW_HEAD_SIZE);
    write(fd, &lf, 1);
    close(fd);

    fd = open(g_test_file, O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    nread = (int)read(fd, read_buf, sizeof(read_buf));
    close(fd);

    /* header + LF only */
    TEST_ASSERT_EQUAL_INT(FILE_RW_HEAD_SIZE + 1, nread);
    TEST_ASSERT_EQUAL_CHAR('\n', read_buf[FILE_RW_HEAD_SIZE]);
}

/*========================================================================
    Main
========================================================================*/

int main(void)
{
    UNITY_BEGIN();

    /* Structure verification */
    RUN_TEST(test_file_rw_head_size);
    RUN_TEST(test_buff_rw_head_size);

    /* I/O roundtrip */
    RUN_TEST(test_write_read_roundtrip);
    RUN_TEST(test_multiple_records);

    /* Field boundaries */
    RUN_TEST(test_header_field_boundaries);

    /* KRX payload format */
    RUN_TEST(test_krx_payload_in_fifo);

    /* Edge cases */
    RUN_TEST(test_empty_payload);

    return UNITY_END();
}
