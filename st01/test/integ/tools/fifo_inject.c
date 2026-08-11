/*------------------------------------------------------------------------
#   Module  : FIFO Inject Tool
#   File    : fifo_inject.c
#   Purpose : Writes a test message to a FIFO/file in FILE_RW_HEAD format.
#             FILE_RW_HEAD is 84 bytes (62 data + 22 delimiters).
#
#   Usage   : fifo_inject <fifo_path> <message_data>
#             fifo_inject /tmp/test_fifo "TCHODR01234..."
#
#   Format  : FILE_RW_HEAD(84B) + data + LF
#             Each FILE_RW_HEAD field is followed by a 2-byte delimiter
#             (field separator), except the last field.
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

/*------------------------------------------------------------------------
    FILE_RW_HEAD layout (matching fep_file.h):
      Seq[10]  If_Seq[10]  ApType[10]  ResponseCode[6]
      RecvTime1[12]  RecvTime2[14]  DataHeader[22]
    Total: 10+10+10+6+12+14+22 = 84 bytes
    (Each field is base_size + 2 delimiter bytes)
------------------------------------------------------------------------*/
#define FRH_SEQ_LEN         (8+2)       /* 10 */
#define FRH_IFSEQ_LEN       (8+2)       /* 10 */
#define FRH_APTYPE_LEN      (8+2)       /* 10 */
#define FRH_RESPCODE_LEN    (4+2)       /* 6  */
#define FRH_RECVTIME1_LEN   (10+2)      /* 12 */
#define FRH_RECVTIME2_LEN   (12+2)      /* 14 */
#define FRH_DATAHDR_LEN     (20+2)      /* 22 */
#define FILE_RW_HEAD_SIZE   84

/*------------------------------------------------------------------------
    Fill field: space-pad + append delimiter (pipe + space)
------------------------------------------------------------------------*/
static void fill_frh_field(char *dst, int total_width, const char *value)
{
    int vlen, data_width;

    data_width = total_width - 2;  /* last 2 bytes are delimiter */
    memset(dst, ' ', total_width);

    if (value) {
        vlen = (int)strlen(value);
        if (vlen > data_width) vlen = data_width;
        memcpy(dst, value, vlen);
    }

    /* Delimiter: pipe + space */
    dst[total_width - 2] = '|';
    dst[total_width - 1] = ' ';
}

/*------------------------------------------------------------------------
    Build FILE_RW_HEAD (84 bytes)
------------------------------------------------------------------------*/
static void build_file_rw_head(char *head, int seq)
{
    char seq_str[12], time_str[16];
    time_t now;
    struct tm *tm;
    int offset = 0;

    memset(head, ' ', FILE_RW_HEAD_SIZE);

    /* Seq */
    sprintf(seq_str, "%08d", seq);
    fill_frh_field(head + offset, FRH_SEQ_LEN, seq_str);
    offset += FRH_SEQ_LEN;

    /* If_Seq */
    fill_frh_field(head + offset, FRH_IFSEQ_LEN, seq_str);
    offset += FRH_IFSEQ_LEN;

    /* ApType */
    fill_frh_field(head + offset, FRH_APTYPE_LEN, "PB_TS   ");
    offset += FRH_APTYPE_LEN;

    /* ResponseCode */
    fill_frh_field(head + offset, FRH_RESPCODE_LEN, "0000");
    offset += FRH_RESPCODE_LEN;

    /* RecvTime1 (epoch seconds) */
    now = time(NULL);
    sprintf(time_str, "%010ld", (long)now);
    fill_frh_field(head + offset, FRH_RECVTIME1_LEN, time_str);
    offset += FRH_RECVTIME1_LEN;

    /* RecvTime2 (HH:MM:SS-mmm) */
    tm = localtime(&now);
    sprintf(time_str, "%02d:%02d:%02d-000", tm->tm_hour, tm->tm_min, tm->tm_sec);
    fill_frh_field(head + offset, FRH_RECVTIME2_LEN, time_str);
    offset += FRH_RECVTIME2_LEN;

    /* DataHeader */
    fill_frh_field(head + offset, FRH_DATAHDR_LEN, "INTEG_TEST_HDR      ");
    /* offset += FRH_DATAHDR_LEN; */
}

/*------------------------------------------------------------------------
    Main
------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    const char *fifo_path;
    const char *message;
    char head[FILE_RW_HEAD_SIZE];
    int fd, data_len;
    static int seq = 1;

    if (argc < 3) {
        fprintf(stderr, "Usage: fifo_inject <fifo_path> <message_data>\n");
        return 1;
    }

    fifo_path = argv[1];
    message   = argv[2];
    data_len  = (int)strlen(message);

    /* Build FILE_RW_HEAD */
    build_file_rw_head(head, seq++);

    /* Open file/FIFO for writing */
    fd = open(fifo_path, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* Write: FILE_RW_HEAD + data + LF */
    if (write(fd, head, FILE_RW_HEAD_SIZE) != FILE_RW_HEAD_SIZE) {
        perror("write header");
        close(fd);
        return 1;
    }
    if (data_len > 0 && write(fd, message, data_len) != data_len) {
        perror("write data");
        close(fd);
        return 1;
    }
    {
        char lf = '\n';
        write(fd, &lf, 1);
    }

    close(fd);

    printf("INJECT: Wrote %d bytes to %s (head=%d + data=%d + LF=1)\n",
           FILE_RW_HEAD_SIZE + data_len + 1, fifo_path,
           FILE_RW_HEAD_SIZE, data_len);

    return 0;
}
