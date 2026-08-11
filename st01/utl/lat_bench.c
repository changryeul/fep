/*------------------------------------------------------------------------
#   Module  : 핫패스 원시 연산 마이크로벤치 (F1 order-latency-metrics)
#   File    : lat_bench.c
#
#   주문 핫패스를 구성하는 원시 I/O 패턴의 회당 비용(µs)을 실측한다.
#   FEP 인프라(SHM/데몬) 없이 파일/세마포어/FIFO 연산만으로 측정 —
#   아키텍처 분석의 syscall 비용 가설과 F2/F3 개선 효과의 정량 근거.
#
#     log_legacy   : Log() 패턴   = stat+open+write+close (매 라인)
#     log_fdcache  : fd 캐시 append write (매 라인)
#     slog_producer: SLog() 생산자 = semop lock+memcpy+semop unlock+FIFO 1B
#     seq_legacy   : Seq_Save 기존 = fopen+fcntl+fseek+fwrite+fflush+fclose
#     seq_fdcache  : Seq_Save F3   = (캐시fd) fcntl+fseek+fwrite+fflush
#     fw_legacy    : F_W 패턴     = fopen(a)+fcntl+fwrite(400B)+fflush+fclose
#                                   + FIFO open+write 1B+close
#     fr_legacy    : F_R 패턴     = open+fcntl+lseek+read(400B)+fcntl+close
#
#   Usage: lat_bench <iterations> [work_dir]
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

static char g_dir[256];

static long long now_usec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

static void report(const char *name, long long total_us, int n)
{
    printf("%-14s %10.2f us/call  (%d calls, %.1f ms total)\n",
            name, (double)total_us / n, n, total_us / 1000.0);
}

/* Log() 패턴: 매 라인 stat + open(APPEND) + write + close */
static void bench_log_legacy(int n)
{
    char path[300], line[128];
    struct stat st;
    int i, fd;
    long long t0;

    sprintf(path, "%s/bench_log.txt", g_dir);
    memset(line, 'L', sizeof(line));
    line[126] = '\n'; line[127] = '\0';

    t0 = now_usec();
    for (i = 0; i < n; i++) {
        stat(path, &st);
        fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
        write(fd, line, 127);
        close(fd);
    }
    report("log_legacy", now_usec() - t0, n);
}

/* fd 캐시 append write */
static void bench_log_fdcache(int n)
{
    char path[300], line[128];
    int i, fd;
    long long t0;

    sprintf(path, "%s/bench_log2.txt", g_dir);
    memset(line, 'L', sizeof(line));
    line[126] = '\n'; line[127] = '\0';

    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    t0 = now_usec();
    for (i = 0; i < n; i++)
        write(fd, line, 127);
    report("log_fdcache", now_usec() - t0, n);
    close(fd);
}

/* SLog() 생산자 패턴: semop lock + 4KB memcpy + semop unlock + FIFO 1B */
static void bench_slog_producer(int n)
{
    char fifo[300], shmbuf[4096], msg[4096];
    int i, semid, fifofd;
    long long t0;
    struct sembuf op;

    sprintf(fifo, "%s/bench.fifo", g_dir);
    unlink(fifo);
    mkfifo(fifo, 0644);
    fifofd = open(fifo, O_RDWR);    /* 자기 자신이 reader 겸 (블록 방지) */

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    /* 초기값 1 */
    op.sem_num = 0; op.sem_op = 1; op.sem_flg = 0;
    semop(semid, &op, 1);

    memset(msg, 'S', sizeof(msg));

    t0 = now_usec();
    for (i = 0; i < n; i++) {
        op.sem_num = 0; op.sem_op = -1; op.sem_flg = SEM_UNDO;
        semop(semid, &op, 1);
        memcpy(shmbuf, msg, 512);           /* SHM_LOG_SIZE급 복사 */
        op.sem_num = 0; op.sem_op = 1; op.sem_flg = SEM_UNDO;
        semop(semid, &op, 1);
        write(fifofd, "1", 1);
        if ((i & 63) == 63) {               /* FIFO 버퍼 드레인 */
            char drain[64];
            read(fifofd, drain, 64);
        }
    }
    report("slog_producer", now_usec() - t0, n);

    close(fifofd);
    unlink(fifo);
    semctl(semid, 0, IPC_RMID);
}

/* Seq_Save 기존 패턴 */
static void bench_seq_legacy(int n)
{
    char path[300], buf[80];
    int i;
    long long t0;
    FILE *fp;
    struct flock lk;

    sprintf(path, "%s/bench_seq.txt", g_dir);
    fp = fopen(path, "w"); fwrite("0", 1, 1, fp); fclose(fp);

    memset(buf, '0', 72); buf[72] = '\0';

    t0 = now_usec();
    for (i = 0; i < n; i++) {
        fp = fopen(path, "r+");
        lk.l_type = F_WRLCK; lk.l_whence = 0; lk.l_start = 0; lk.l_len = 0;
        fcntl(fileno(fp), F_SETLKW, &lk);
        fseek(fp, 0L, SEEK_SET);
        fwrite(buf, 72, 1, fp);
        fflush(fp);
        lk.l_type = F_UNLCK;
        fcntl(fileno(fp), F_SETLK, &lk);
        fclose(fp);
    }
    report("seq_legacy", now_usec() - t0, n);
}

/* Seq_Save F3 패턴 (fd 캐시) */
static void bench_seq_fdcache(int n)
{
    char path[300], buf[80];
    int i;
    long long t0;
    FILE *fp;
    struct flock lk;

    sprintf(path, "%s/bench_seq2.txt", g_dir);
    fp = fopen(path, "w"); fwrite("0", 1, 1, fp); fclose(fp);

    memset(buf, '0', 72); buf[72] = '\0';

    fp = fopen(path, "r+");
    t0 = now_usec();
    for (i = 0; i < n; i++) {
        lk.l_type = F_WRLCK; lk.l_whence = 0; lk.l_start = 0; lk.l_len = 0;
        fcntl(fileno(fp), F_SETLKW, &lk);
        fseek(fp, 0L, SEEK_SET);
        fwrite(buf, 72, 1, fp);
        fflush(fp);
        lk.l_type = F_UNLCK;
        fcntl(fileno(fp), F_SETLK, &lk);
    }
    report("seq_fdcache", now_usec() - t0, n);
    fclose(fp);
}

/* F_W 패턴: 데이터 파일 append + FIFO 통지 (프로세스 간 홉의 쓰기 측) */
static void bench_fw_legacy(int n)
{
    char path[300], fifo[300], rec[400];
    int i, fifofd;
    long long t0;
    FILE *fp;
    struct flock lk;

    sprintf(path, "%s/bench_fw.dat", g_dir);
    sprintf(fifo, "%s/bench_fw.fifo", g_dir);
    unlink(path); unlink(fifo);
    mkfifo(fifo, 0644);

    {   /* FIFO 드레인용 리더 fd */
        int rd = open(fifo, O_RDWR);
        memset(rec, 'W', sizeof(rec));
        rec[399] = '\n';

        t0 = now_usec();
        for (i = 0; i < n; i++) {
            fp = fopen(path, "a+");
            lk.l_type = F_WRLCK; lk.l_whence = SEEK_END;
            lk.l_start = 0; lk.l_len = 400;
            fcntl(fileno(fp), F_SETLKW, &lk);
            fwrite(rec, 400, 1, fp);
            fflush(fp);
            lk.l_type = F_UNLCK; lk.l_whence = 0;
            fcntl(fileno(fp), F_SETLK, &lk);
            fclose(fp);

            fifofd = open(fifo, O_WRONLY);
            write(fifofd, "1", 1);
            close(fifofd);

            if ((i & 63) == 63) {
                char drain[64];
                read(rd, drain, 64);
            }
        }
        report("fw_legacy", now_usec() - t0, n);
        close(rd);
    }
    unlink(fifo);
}

/* F_W F4 패턴: 캐시 FILE* + 캐시 FIFO fd (open/close 제거) */
static void bench_fw_fdcache(int n)
{
    char path[300], fifo[300], rec[400];
    int i, fifofd, rd;
    long long t0;
    FILE *fp;
    struct flock lk;

    sprintf(path, "%s/bench_fw2.dat", g_dir);
    sprintf(fifo, "%s/bench_fw2.fifo", g_dir);
    unlink(path); unlink(fifo);
    mkfifo(fifo, 0644);

    rd = open(fifo, O_RDWR);            /* 드레인 겸 캐시 리더 */
    fifofd = open(fifo, O_WRONLY);      /* 캐시된 통지 fd */
    fp = fopen(path, "a+");             /* 캐시된 스트림 */
    memset(rec, 'W', sizeof(rec));
    rec[399] = '\n';

    t0 = now_usec();
    for (i = 0; i < n; i++) {
        lk.l_type = F_WRLCK; lk.l_whence = SEEK_END;
        lk.l_start = 0; lk.l_len = 400;
        fcntl(fileno(fp), F_SETLKW, &lk);
        fwrite(rec, 400, 1, fp);
        fflush(fp);
        lk.l_type = F_UNLCK; lk.l_whence = 0;
        fcntl(fileno(fp), F_SETLK, &lk);

        write(fifofd, "1", 1);

        if ((i & 63) == 63) {
            char drain[64];
            read(rd, drain, 64);
        }
    }
    report("fw_fdcache", now_usec() - t0, n);

    fclose(fp);
    close(fifofd);
    close(rd);
    unlink(fifo);
}

/* F_R F4 패턴: 캐시 fd 레코드 읽기 */
static void bench_fr_fdcache(int n)
{
    char path[300], rec[400];
    int i, fd;
    long long t0, off;
    struct flock lk;

    sprintf(path, "%s/bench_fw.dat", g_dir);

    fd = open(path, O_RDWR);            /* 캐시된 fd */
    t0 = now_usec();
    for (i = 0; i < n; i++) {
        off = (long long)(i % 1000) * 400;
        lk.l_type = F_RDLCK; lk.l_whence = 0;
        lk.l_start = off; lk.l_len = 400;
        fcntl(fd, F_SETLKW, &lk);
        lseek(fd, off, SEEK_SET);
        read(fd, rec, 400);
        lk.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lk);
    }
    report("fr_fdcache", now_usec() - t0, n);
    close(fd);
}

/* F_R 패턴: 데이터 파일 레코드 읽기 (프로세스 간 홉의 읽기 측) */
static void bench_fr_legacy(int n)
{
    char path[300], rec[400];
    int i, fd;
    long long t0, off;
    struct flock lk;

    sprintf(path, "%s/bench_fw.dat", g_dir);    /* fw가 만든 파일 재사용 */

    t0 = now_usec();
    for (i = 0; i < n; i++) {
        off = (long long)(i % 1000) * 400;
        fd = open(path, O_RDWR);
        lk.l_type = F_RDLCK; lk.l_whence = 0;
        lk.l_start = off; lk.l_len = 400;
        fcntl(fd, F_SETLKW, &lk);
        lseek(fd, off, SEEK_SET);
        read(fd, rec, 400);
        lk.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lk);
        close(fd);
    }
    report("fr_legacy", now_usec() - t0, n);
}

int main(int argc, char *argv[])
{
    int n;

    if (argc < 2) {
        printf("Usage: lat_bench <iterations> [work_dir]\n");
        return 1;
    }

    n = atoi(argv[1]);
    if (n < 1) n = 1000;

    if (argc > 2)
        snprintf(g_dir, sizeof(g_dir), "%s", argv[2]);
    else
        snprintf(g_dir, sizeof(g_dir), "/tmp/lat_bench_%d", (int)getpid());

    mkdir(g_dir, 0755);

    printf("=== FEP hot-path primitive micro-benchmark (n=%d) ===\n", n);
    printf("work dir: %s\n\n", g_dir);

    bench_log_legacy(n);
    bench_log_fdcache(n);
    bench_slog_producer(n);
    printf("\n");
    bench_seq_legacy(n);
    bench_seq_fdcache(n);
    printf("\n");
    bench_fw_legacy(n);
    bench_fr_legacy(n);
    bench_fw_fdcache(n);
    bench_fr_fdcache(n);

    printf("\nnote: fw+fr = 프로세스 간 홉 1회의 파일 큐 왕복 비용 (legacy vs F4 fdcache)\n");
    return 0;
}

/*************************************************************************
    End of Program (lat_bench.c)
*************************************************************************/
