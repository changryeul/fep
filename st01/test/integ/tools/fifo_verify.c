/*------------------------------------------------------------------------
#   Module  : FIFO Verify Tool
#   File    : fifo_verify.c
#   Purpose : Reads from a FIFO/file and verifies content against
#             an expected pattern.
#
#   Usage   : fifo_verify <fifo_path> <expected_pattern>
#             fifo_verify /tmp/test_fifo "TCHODR"
#
#   Output  : PASS if pattern found in data portion, FAIL otherwise.
#   Exit    : 0 for PASS, 1 for FAIL
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_RW_HEAD_SIZE   84
#define MAX_RECORD_SIZE     4096

int main(int argc, char *argv[])
{
    const char *fifo_path;
    const char *pattern;
    char buf[MAX_RECORD_SIZE + 1];
    int fd, nread, pat_len;
    int head_ok, data_found;

    if (argc < 3) {
        fprintf(stderr, "Usage: fifo_verify <fifo_path> <expected_pattern>\n");
        return 1;
    }

    fifo_path = argv[1];
    pattern   = argv[2];
    pat_len   = (int)strlen(pattern);

    /* Open file/FIFO for reading */
    fd = open(fifo_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        fprintf(stderr, "FAIL: Cannot open %s\n", fifo_path);
        return 1;
    }

    /* Read entire record */
    nread = (int)read(fd, buf, MAX_RECORD_SIZE);
    close(fd);

    if (nread <= 0) {
        fprintf(stderr, "FAIL: No data read from %s\n", fifo_path);
        return 1;
    }
    buf[nread] = '\0';

    printf("VERIFY: Read %d bytes from %s\n", nread, fifo_path);

    /* Check minimum size: FILE_RW_HEAD + at least 1 byte data + LF */
    head_ok = (nread > FILE_RW_HEAD_SIZE);
    if (!head_ok) {
        fprintf(stderr, "FAIL: Record too short (%d bytes, need > %d)\n",
                nread, FILE_RW_HEAD_SIZE);
        return 1;
    }

    printf("VERIFY: Header = %d bytes, Data = %d bytes\n",
           FILE_RW_HEAD_SIZE, nread - FILE_RW_HEAD_SIZE);

    /* Search for pattern in the data portion (after header) */
    data_found = 0;
    {
        const char *data_start = buf + FILE_RW_HEAD_SIZE;
        int data_len = nread - FILE_RW_HEAD_SIZE;
        int i;

        for (i = 0; i <= data_len - pat_len; i++) {
            if (memcmp(data_start + i, pattern, pat_len) == 0) {
                data_found = 1;
                break;
            }
        }
    }

    if (data_found) {
        printf("PASS: Pattern \"%s\" found in data\n", pattern);
        return 0;
    }
    else {
        fprintf(stderr, "FAIL: Pattern \"%s\" not found in data\n", pattern);

        /* Dump data portion for debugging */
        {
            int dump_len = nread - FILE_RW_HEAD_SIZE;
            if (dump_len > 200) dump_len = 200;
            printf("VERIFY: Data portion (first %d bytes): [", dump_len);
            fwrite(buf + FILE_RW_HEAD_SIZE, 1, dump_len, stdout);
            printf("]\n");
        }

        return 1;
    }
}
