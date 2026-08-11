#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : data SHM sync manager
#   File    : pw_1000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/

#include    "fep_fepp.h"
#include    <pthread.h>

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME   (2 * 1000)
#define     THD_MAX     100

typedef struct {
    char        RunFlag;
    int         ThdPos;
    int         DshmPos;
    pthread_t   Id;
}   THD_CTX;

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     R_Cnt, ThdCnt;
THD_CTX ThdCtx[THD_MAX];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PW_1000_MP(void);
void    *Sync_Thread(THD_CTX *);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
    PW_1000_MP();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PW_1000_MP(void)
/*----------------------------------------------------------------------*/
{
    int     i, j, rt, shmid;
    char    key[12];
    key_t   base_key;

    j = 0;
    memset(ThdCtx, 0, sizeof (ThdCtx));

    base_key = BASE_SHM_KEY;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        base_key += 0x01000000L;

    sprintf(key, "0x00%02d0000", D_K + 1);
    errno = 0;
    base_key += strtol(key, NULL, 16);
    sprintf(key, "0x0000%2.2s00", _Exe_Name+5);
    errno = 0;
    base_key += strtol(key, NULL, 16);
    Shmptr = Shm_Attach(base_key, &shmid);
    Log(USR_OK, "data SHM attached [%#x]", base_key);

    for (i = 0; i < DAEMON(D_K).d_count; i ++) {
        if (memcmp(DSHM(D_K,i).key_info, _Exe_Name+5, 2) == 0) {
            ThdCtx[j].ThdPos = j;
            ThdCtx[j].DshmPos = i;
            Log(USR_OK, "i[%d] key_info[%s] _Exe_Name[%s]", i, DSHM(D_K,i).key_info, _Exe_Name);
            j ++;

            if (DSHM(D_K,i).key_info[5] == 'E' ||
                    DSHM(D_K,i).key_info[5] == 'O')
            break;
        }
    }

    ThdCnt = j;

    while (START_S < JOB_END) {
        Stat_Save();

        for (i = 0; i < ThdCnt; i ++) {
            if (ThdCtx[i].RunFlag == OFF) {
                rt = pthread_create(&ThdCtx[i].Id, NULL, (void *)Sync_Thread, &ThdCtx[i]);

                if (rt != 0) {
                    Log(USR_ERROR,
                            "pthread_create: ThdPos[%d] DshmPos[%d]",
                            ThdCtx[i].ThdPos, ThdCtx[i].DshmPos);
                    Exit_Process();
                }

                ThdCtx[i].RunFlag = ON;
                usleep(10000);
            }
        }

        sleep(1);
    }

    for (i = 0; ThdCnt; i ++) {
        if (ThdCtx[i].RunFlag == ON)
            pthread_cancel(ThdCtx[i].Id);
    }

    return;
}   /* End of PW_1000_MP () */

/*----------------------------------------------------------------------*/
void    *Sync_Thread(THD_CTX *ctx)
/*----------------------------------------------------------------------*/
{
    int             fd, fifo_fd;
    int             i, rec_size, rt, sk, tk, status, len, pt, cnt;
    int             rcnt, wcnt, data_cnt, max_cnt, poll_cnt;
    long            offset;
    char            r_buf[KRX_DATA_BUFF_SIZE], w_buf[KRX_DATA_BUFF_SIZE], sub[4], poll_flag;
    char            fifo_name[128], f_name[128];
    char            tmp[128];
    struct pollfd   Poll;
    char            *cq;

    if (ctx->Id < 0) {
        Log(FIF_FATAL, "Id Error [%d]", ctx->Id);
        ThdCtx[tk].RunFlag = OFF;
        pthread_exit((void *)&status);
    }

    sk = ctx->DshmPos;
    tk = ctx->ThdPos;
    poll_cnt = 0;
    poll_flag = OFF;

    rec_size = sizeof (FILE_RW_HEAD) + DSHM(D_K,sk).data_size + 1;
    max_cnt = KRX_DATA_BUFF_SIZE / rec_size;

    sprintf(sub, "%s", _SubSystem_Name);
    LtoU(sub, 2);
    sprintf(fifo_name, "%s/%-2.2s/%s0",
            _FEP_FIFO, sub, DSHM(D_K,sk).data_name);
    Log(USR_OK,
            "[%s] Id[%d] DshmPos[%d] ThsPos[%d] rec_size[%d] max_cnt[%d] fifo[%s]",
            DSHM(D_K,sk).data_name, ctx->Id, ctx->DshmPos, ctx->ThdPos, rec_size,
            max_cnt, fifo_name);

    fifo_fd = open(fifo_name, O_RDWR|O_NDELAY);
    if (fifo_fd < 0) {
        Log(FIF_FATAL, "cannot open FIFO [%s] {%d:%s} fifo_fd[%d]",
                fifo_name, SYS_NO, SYS_STR, fifo_fd);
        ThdCtx[tk].RunFlag = OFF;
        pthread_exit((void *)&status);
    }

    Poll.fd = fifo_fd;
    Poll.events = POLLIN;

    /* output file  */
    sprintf(f_name, "%s/%-2.2s/00000000/%s",
            _FEP_DAT, sub, DSHM(D_K,sk).data_name);

    fd = open(f_name, O_WRONLY|O_APPEND|O_CREAT|O_LARGEFILE, 0664);

    if (fd < 0) {
        close(fifo_fd);
        Log(SAM_FATAL, "open fail[%s] {%d:%s} fd[%d]", f_name, SYS_NO, SYS_STR, fd);
        ThdCtx[tk].RunFlag = OFF;
        pthread_exit((void *)&status);
    }

    cq = Shmptr + DSHM(D_K,sk).offset;

    while (START_S != JOB_END) {
        rcnt = DSHM(D_K,sk).sm_r_cnt;
        wcnt = DSHM(D_K,sk).w_cnt[0];
        data_cnt = wcnt - rcnt;

        if (poll_flag == ON && data_cnt == 0) {
            while (1) {
                rt = read(Poll.fd, tmp, sizeof (tmp));
                if (rt >= 0) {
                    poll_flag = OFF;
                    break;
                }
                else if (rt < 0) {
                    if (SYS_NO == EINTR)
                        continue;

                    Log(FIF_ERROR, "[%s] cannot read FIFO {%d:%s} fifo_name[%s] rt[%d]",
                            DSHM(D_K,sk).data_name, SYS_NO, SYS_STR, fifo_name, rt);
                    break;
                }
            }
        }

        if (data_cnt > 0) {
            if (data_cnt > max_cnt)
                cnt = max_cnt;
            else
                cnt = data_cnt;

            memset(r_buf, 0, sizeof (r_buf));
            memset(w_buf, 0, sizeof (w_buf));

            for (i = 0; i < cnt; i ++) {
                offset = ((DSHM(D_K,sk).sm_r_cnt + i) % DSHM(D_K,sk).max_rec) *
                rec_size;
                memcpy(&w_buf[rec_size*i], &cq[offset], rec_size);

                if (w_buf[rec_size*i] == '\n')
                    break;
            }

            if (i < cnt) {
                if (i == 0) {
                    DSHM(D_K,sk).sm_r_cnt ++;
                    Dshm_Seq_Save(DSHM(D_K,sk).data_name, D_K, sk, 9);
                    usleep(10000);
                }

                cnt = i;
            }

            if (cnt > 0) {
                len = rec_size * cnt;

                rt = lseek(fd, len, SEEK_SET);

                if (rt == -1) {
                    close(fifo_fd);
                    close(fd);
                    Log(SAM_FATAL, "seek fail[%ld] {%d:%s}",
                            len, SYS_NO, SYS_STR);
                    ThdCtx[tk].RunFlag = OFF;
                    pthread_exit((void *)&status);
                }

                pt = 0;

                while (1) {
                    rt = write(fd, w_buf+pt, len);

                    if (rt == -1) {
                        Log(SAM_FATAL, "cannot write[%d] {%d:%s}",
                                fd, SYS_NO, SYS_STR);
                        close(fifo_fd);
                        close(fd);
                        ThdCtx[tk].RunFlag = OFF;
                        pthread_exit((void *)&status);
                    }
                    else if (rt == len)
                        break;

                    len -= rt;
                    pt += rt;
                }

                DSHM(D_K,sk).sm_r_cnt += cnt;
                Dshm_Seq_Save(DSHM(D_K,sk).data_name, D_K, sk, 9);
                poll_cnt = 0;
            }
        }
        else {
            poll_cnt ++;

            rt = poll(&Poll, 1, DATA_TIME);

            if (rt > 0) {
                if (Poll.revents & POLLIN) {
                    Poll.revents = 0;
                    poll_flag = ON;
                    continue;
                }

                if (Poll.revents & POLLHUP) {
                    Log(SYS_FATAL, "[%s] poll hangup", DSHM(D_K,sk).data_name);
                    continue;
                }
            }
            else if (rt == 0) {
                if (poll_cnt == 30) {
                    Log(USR_OK, "[%s] poll timeout <%d>",
                            DSHM(D_K,sk).data_name, DSHM(D_K,sk).sm_r_cnt);
                    poll_cnt = 0;
                }

                continue;
            }
            else {
                if (SYS_NO == EINTR)
                    continue;

                Log(SYS_ERROR, "[%s] poll fail {%d:%s}",
                        DSHM(D_K,sk).data_name, SYS_NO, SYS_STR);
                continue;
            }
        }
    }

    close(fifo_fd);
    close(fd);

}   /* End of Sync_Thread ()    */

/*************************************************************************
    End of Program (pw_1000_mp.c)
*************************************************************************/
