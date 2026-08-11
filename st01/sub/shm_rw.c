/*------------------------------------------------------------------------
#   Module  : read or write to data SHM
#   File    : shm_rw.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/

/*************************************************************************
    Function        : . read data SHM
    Parameters IN   : . p_type  : process type
                      . p_cnt   : maxmum read count
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int
                        >= 0    : success (read count)
                        -1      : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     DSHM_R(int p_type, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    short           fk, sk;
    int             i, fd, rt, rec_size, r_cnt, data_cnt, tmp_off;
    long            offset;
    char            f_name[100], sub[4], buf[FILE_BUF_LEN], tmp[FILE_BUF_LEN];
    char            ck, *cq;
    FILE_RW_HEAD    frw;
    BUFF_RW_HEAD    brw;

    if (p_buf == NULL) {
        Log(USR_ERROR, "DSHM_R:read buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 45 < p_cnt) {
        Log(USR_ERROR, "DSHM_R:invalid maximum read count[%d]", p_cnt);
        return (NOTOK);
    }

    fk = p_type / PS_R_1;
    ck = p_type % PS_R_1;

    if (fk < 1 || fk > 3 || ck < 0 || ck > 8) {
        Log(USR_ERROR, "DSHM_R:invalid process type[%d]", p_type);
        return (NOTOK);
    }

    if (fk == 1)
        fk --;
    else
        fk -= 2;

    sk = AtoIf(IDK(D_K,P_K,fk), 2) - 1;
    rec_size = sizeof (FILE_RW_HEAD) + IDS(D_K,P_K,fk) + 1;
    r_cnt = IDR(D_K,P_K,fk,ck);
    data_cnt = IDW(D_K,P_K,fk,0) - r_cnt;

    if (p_cnt < data_cnt)
        data_cnt = p_cnt;

    memset(tmp, 0, sizeof (tmp));
    tmp_off = 0;
    cq = DShmPtr[sk] + IDO(D_K,P_K,fk);

    for (i = 0; i < data_cnt; i ++) {
        offset = (r_cnt % IDM(D_K,P_K,fk)) * rec_size;
        memcpy(buf, &cq[offset], rec_size);
        memcpy(&frw, buf, sizeof (FILE_RW_HEAD));

        if (i == 0 &&
                AtoIf(frw.Seq+1, sizeof (frw.Seq) - 2) != r_cnt + 1) {
            sprintf(sub, "%s", _SubSystem_Name);
            LtoU(sub, 2);
            sprintf(f_name, "%s/%s/00000000/%s",
                    _FEP_DAT, sub, IDN(D_K,P_K,fk));

            fd = open(f_name, O_RDWR|O_CREAT, 0664);

            if (fd == -1) {
                Log(SAM_FATAL, "DSHM_R:open fail[%s] {%d:%s}",
                        f_name, SYS_NO, SYS_STR);
                return (NOTOK);
            }

            offset = r_cnt * rec_size;

            rt = lseek(fd, offset, SEEK_SET);

            if (rt == -1) {
                close(fd);
                Log(SAM_FATAL, "DSHM_R:lseek fail[%ld] {%d:%s}",
                        offset, SYS_NO, SYS_STR);
                return (NOTOK);
            }

            rt = F_R_Proc(fd, p_buf, rec_size, data_cnt);

            if (rt > 0) {
                if (p_type / PS_R_1 >= 2) {
                    IDR(D_K,P_K,fk,ck) += rt;
                    Dshm_Seq_Save(IDN(D_K,P_K,fk), D_K,
                            PROC(D_K,P_K).in_d[fk] - 1, ck);
                }
            }

            close(fd);
            return (rt);
        }

        memcpy(brw.Seq, frw.Seq+1, sizeof (brw.Seq));
        memcpy(brw.If_Seq, frw.If_Seq+1, sizeof (brw.If_Seq));
        memcpy(brw.ApType, frw.ApType+1, sizeof (brw.ApType));
        memcpy(brw.ResponseCode, frw.ResponseCode+1,
                sizeof (brw.ResponseCode));
        memcpy(brw.RecvTime1, frw.RecvTime1+1, sizeof (brw.RecvTime1));
        memcpy(brw.RecvTime2, frw.RecvTime2+1, sizeof (brw.RecvTime2));
        memcpy(brw.DataHeader, frw.DataHeader+1, sizeof (brw.DataHeader));

        memcpy(&tmp[tmp_off], &brw, sizeof (BUFF_RW_HEAD));
        tmp_off += sizeof (BUFF_RW_HEAD);
        memcpy(&tmp[tmp_off], buf+sizeof(FILE_RW_HEAD),
                rec_size - sizeof (FILE_RW_HEAD));
        tmp_off += rec_size - sizeof (FILE_RW_HEAD);

        r_cnt ++;
    }

    memcpy(p_buf, tmp, tmp_off);

    if (p_type / PS_R_1 >= 2) {
        IDR(D_K,P_K,fk,ck) += data_cnt;
        Dshm_Seq_Save(IDN(D_K,P_K,fk), D_K, PROC(D_K,P_K).in_d[fk] - 1, ck);
    }

    return (data_cnt);
}   /* End of DSHM_R () */

/*************************************************************************
    Function        : . Dshm_Add_Count
    Parameters IN   : . p_type  : process type
                      . p_cnt   : read count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . add read count
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Dshm_Add_Count(int p_type, int p_cnt)
/*----------------------------------------------------------------------*/
{
    short   fk;
    char    ck;

    if (p_cnt < 0) {
        Log(USR_FATAL, "Dshm_Add_Count:invalid p_cnt[%d]", p_cnt);
        Exit_Process();
    }

    fk = p_type / PS_R_1;
    ck = p_type % PS_R_1;

    if (fk != 1 || ck < 0 || ck > 8) {
        Log(USR_FATAL, "Dshm_Add_Count:invalid process type[%d]", p_type);
        Exit_Process();
    }

    fk --;
    IDR(D_K,P_K,fk,ck) += p_cnt;
    Dshm_Seq_Save(IDN(D_K,P_K,fk), D_K, PROC(D_K,P_K).in_d[fk] - 1, ck);

    return;
}   /* End of Dshm_Add_Count () */

/*************************************************************************
    Function        : . core write logic for data SHM (shared by W/W2/WT)
    Parameters IN   : . dk      : daemon key
                      . pk      : process key
                      . fk      : file key (0-based)
                      . p_buf   : write data buffer
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int
                        > 0 : success (write count)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
static int  DSHM_W_Core(int dk, int pk, int fk,
        char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    short           sk;
    int             rt, rec_size, cnt, i, f_size;
    long            offset;
    u_char          fifo_cnt;
    char            Tmp[128];
    char            *cq;
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    sk = AtoIf(ODK(dk,pk,fk), 2) - 1;
    fifo_cnt = ODC(dk,pk,fk);
    rec_size = sizeof (BUFF_RW_HEAD) + ODS(dk,pk,fk) + 1;
    f_size = rec_size - sizeof (BUFF_RW_HEAD) + sizeof (FILE_RW_HEAD);

    if (SemId[fk] != -1)
        SEM_Lock(SemId[fk]);

    cnt = ODW(dk,pk,fk,0) + 1;
    memset(Tmp, 0, sizeof (Tmp));

    for (i = 0; i < rec_size * p_cnt; i += rec_size, cnt ++) {
        offset = (ODW(dk,pk,fk,0) % ODM(dk,pk,fk)) * f_size;

        memcpy(buff_rw.Seq, p_buf + i, sizeof (BUFF_RW_HEAD));

        /* Seq (10) */
        sprintf(Tmp, "[%08d]", cnt);
        memcpy(file_rw.Seq, Tmp, 10);
        /* If_Seq (10) */
        sprintf(Tmp, "[%-8.8s]", buff_rw.If_Seq);
        memcpy(file_rw.If_Seq, Tmp, 10);
        /* ApType (10) */
        sprintf(Tmp, "[%-8.8s]", buff_rw.ApType);
        memcpy(file_rw.ApType, Tmp, 10);
        /* ResponseCode (6) */
        sprintf(Tmp, "[%-4.4s]", buff_rw.ResponseCode);
        memcpy(file_rw.ResponseCode, Tmp, 6);
        /* RecvTime1 (12) */
        sprintf(Tmp, "[%-10.10s]", buff_rw.RecvTime1);
        memcpy(file_rw.RecvTime1, Tmp, 12);
        /* RecvTime2 (14) */
        sprintf(Tmp, "[%-12.12s]", buff_rw.RecvTime2);
        memcpy(file_rw.RecvTime2, Tmp, 14);
        /* DataHeader (22) */
        sprintf(Tmp, "[%-20.20s]", buff_rw.DataHeader);
        memcpy(file_rw.DataHeader, Tmp, 22);

        cq = DShmPtr[sk] + ODO(dk,pk,fk);
        memcpy(&cq[offset], file_rw.Seq, sizeof (FILE_RW_HEAD));
        memcpy(&cq[offset + sizeof(FILE_RW_HEAD)],
                &p_buf[i + sizeof(BUFF_RW_HEAD)],
                rec_size - sizeof (BUFF_RW_HEAD));

        ODW(dk,pk,fk,0) ++;
    }

    if (SemId[fk] != -1)
        SEM_UnLock(SemId[fk]);

    /* notify process FIFOs */
    for (i = 1; i <= fifo_cnt; i ++) {
        if (OD_FIFO_fd[fk][i] <= 0) {
            Log(FIF_FATAL, "DSHM_W:cannot open FIFO[%d][%d:%s]",
                    OD_FIFO_fd[fk][i], SYS_NO, SYS_STR);
        }

        rt = write(OD_FIFO_fd[fk][i], "1", 1);

        if (rt < 0)
            Log(FIF_FATAL, "DSHM_W:cannot write FIFO[%d][%d:%s]",
                    OD_FIFO_fd[fk][i], SYS_NO, SYS_STR);
    }

    /* FIFO for DSHM sync manager */
    if (OD_FIFO_fd[fk][0] <= 0) {
        Log(FIF_FATAL, "DSHM_W:cannot open FIFO[%d][%d:%s]",
                OD_FIFO_fd[fk][0], SYS_NO, SYS_STR);
    }

    rt = write(OD_FIFO_fd[fk][0], "1", 1);

    if (rt < 0)
        Log(FIF_FATAL, "DSHM_W:cannot write FIFO[%d][%d:%s]",
                OD_FIFO_fd[fk][0], SYS_NO, SYS_STR);

    return (p_cnt);
}   /* End of DSHM_W_Core ()    */

/*************************************************************************
    Function        : . write to data SHM (multi-record, p_out/10)
    Parameters IN   : . p_out   : output data SHM number (x10)
                      . p_buf   : write data buffer
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int
                        > 0 : success (write count)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     DSHM_W(int p_out, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int     fk;

    if (p_buf == NULL) {
        Log(USR_ERROR, "DSHM_W:write buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 30 < p_cnt) {
        Log(USR_ERROR, "DSHM_W:invalid write count[%d]", p_cnt);
        return (NOTOK);
    }

    p_out = p_out / 10;

    if (p_out < 1 || p_out > 99) {
        Log(USR_ERROR, "DSHM_W:invalid output data SHM number[%d:1-99]",
                p_out);
        return (NOTOK);
    }

    fk = p_out - 1;

    return DSHM_W_Core(D_K, P_K, fk, p_buf, p_cnt);
}   /* End of DSHM_W () */

/*************************************************************************
    Function        : . write to data SHM (single-record, p_out direct)
    Parameters IN   : . p_out   : output data SHM number (direct)
                      . p_buf   : write data buffer
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int
                        > 0 : success (write count)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     DSHM_W2(int p_out, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int     fk;

    if (p_buf == NULL) {
        Log(USR_ERROR, "DSHM_W:write buffer is NULL");
        return (NOTOK);
    }

    fk = p_out - 1;

    return DSHM_W_Core(D_K, P_K, fk, p_buf, p_cnt);
}   /* End of DSHM_W2 ()    */

/*************************************************************************
    Function        : . write to data SHM
    Parameters IN   : . p_out   : output data SHM number
                      . p_buf   : write data buffer
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int
                        > 0 : success (write count)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     DSHM_WT(int p_out, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    short           fk, sk;
    int             rt, i, j;
    char            fifo_name[256], attach_flag[MAX_DSHM_SEG], key[32];

    /* 20211230 */
    int             pk;
    for (pk = 0; pk < DAEMON(0).p_count; pk ++) {
        if (memcmp(PROC(0,pk).process_id, "pa_8201_tr", 10) == 0)
            break;

        if (pk == DAEMON(0).p_count - 1) {
            printf("Not Search pa_8201_tr process\n");
            return (-1);
        }
    }

    /* get memory info */
    int     shmid;
    key_t   base_key, new_key;

    base_key = BASE_SHM_KEY;
    memset(attach_flag, 0, sizeof (attach_flag));

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        base_key += 0x01000000L;

    memset(key, 0, sizeof(key));

    sprintf(key, "0x00%02d0000", 0 + 1);
    base_key += strtol(key, NULL, 16);

    for (i = 0; i < 3; i ++) {
        if (PROC(0,pk).fifo_f[i][0] != 0) {
            sprintf(fifo_name, "%s/PA/%s", _FEP_FIFO, PROC(0,pk).fifo_f[i]);

            PROC(0,pk).in_FIFO_fd[i] = open(fifo_name, O_RDWR|O_NDELAY);

            if (PROC(0,pk).in_FIFO_fd[i] == -1) {
                Log(FIF_FATAL, "Init_Proc:cannot open IFIFD %d[%s] {%d:%s}",
                        i + 1, fifo_name, SYS_NO, SYS_STR);
                exit(OK);
            }
        }

        if (PROC(0,pk).in_d[i] != 0) {
            sk = AtoIf(IDK(0,pk,i), 2) - 1;

            if (sk < 0) {
                Log(USR_FATAL, "Init_Proc:invalid DSHM key[%s]",
                        IDK(0,pk,i));
                exit(OK);
            }

            if (attach_flag[sk] == OFF) {
                sprintf(key, "0x0000%02d00", sk + 1);
                new_key = base_key + strtol(key, NULL, 16);
                DShmPtr[sk] = Shm_Attach(new_key, &shmid);
                attach_flag[sk] = ON;
            }
        }
    }

    for (i = 0; i < 99; i ++) {
        if (PROC(0,pk).out_d[i] == 0)
            break;
        else {
            /* open output data FIFOs   */
            for (j = 0; j <= ODC(0,pk,i); j ++) {
                sprintf(fifo_name, "%s/PA/%s%d",
                        _FEP_FIFO, ODN(0,pk,i), j);

                OD_FIFO_fd[i][j] = open(fifo_name, O_RDWR|O_NDELAY);

                if (OD_FIFO_fd[i][j] == -1) {
                    Log(FIF_FATAL,
                            "Init_Proc:cannot open out data FIFO[%s] {%d:%s}",
                            fifo_name, SYS_NO, SYS_STR);
                    exit(OK);
                }
            }

            /* attach data SHM  */
            sk = AtoIf(ODK(0,pk,i), 2) - 1;

            if (sk < 0) {
                Log(USR_FATAL, "Init_Proc:invalid DSHM key[%s]",
                        ODK(0,pk,i));
                exit(OK);
            }

            if (attach_flag[sk] == OFF) {
                sprintf(key, "0x0000%02d00", sk + 1);

                new_key = base_key + strtol(key, NULL, 16);
                DShmPtr[sk] = Shm_Attach(new_key, &shmid);
                attach_flag[sk] = ON;
            }

            /* semaphore    */
            sprintf(key, "0x0000%4.4s", ODK(0,pk,i));
            new_key = base_key + strtol(key, NULL, 16);

            SemId[i] = SEM_Creat_Excl(new_key);

            if (SemId[i] == -1) {
                SemId[i] = SEM_Creat(new_key);

                if (SemId[i] == -1) {
                    Log(SYS_FATAL, "SEM_Creat fail {%d:%s}",
                            SYS_NO, SYS_STR);
                    exit(OK);
                }
            }
        }
    }
    /* 20211230 */

    if (p_buf == NULL) {
        printf("DSHM_W:write buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 30 < p_cnt) {
        printf("DSHM_W:invalid write count[%d]", p_cnt);
        return (NOTOK);
    }

    p_out = p_out / 10;

    if (p_out < 1 || p_out > 99) {
        printf("DSHM_W:invalid output data SHM number[%d:1-99]",
                p_out);
        return (NOTOK);
    }

    fk = p_out - 1;

    return DSHM_W_Core(0, pk, fk, p_buf, p_cnt);
}   /* End of DSHM_WT ()    */

/*************************************************************************
    End of Program (shm_rw.c)
*************************************************************************/
