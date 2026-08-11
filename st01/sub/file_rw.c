/*------------------------------------------------------------------------
#   Module  : read or write to data file
#   File    : file_rw.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
short   Fk;
int     Dk;
u_char  Fifo_Cnt;
char    Ck, Ofn[20];

/*************************************************************************
    Function        : . read data file
    Parameters IN   : . p_type  : process type
                      . p_cnt   : maxmum read count
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int
                        >= 0    : success (read count)
                        -1      : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_R(int p_type, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size, cnt, fd, fd_cached;
    long            offset;
    char            f_name[100], sub[4];
    struct flock    lock;

    if (p_buf == NULL) {
        Log(SAM_ERROR, "F_R:read buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 45 < p_cnt) {
        Log(SAM_ERROR, "F_R:invalid maximum read count[%d]", p_cnt);
        return (NOTOK);
    }

    sprintf(sub, "%s", _SubSystem_Name);
    LtoU(sub, 2);

    Fk = p_type / PS_R_1;
    Ck = p_type % PS_R_1;

    if (Fk < 1 || Fk > 3 || Ck < 0 || Ck > 8) {
        Log(SAM_ERROR, "F_R:invalid process type[%d]", p_type);
        return (NOTOK);
    }

    if (Fk == 1)
        Fk --;
    else
        Fk -= 2;

    sprintf(f_name, "%s/%s/00000000/%s", _FEP_DAT, sub, IFN(D_K,P_K,Fk));
    rec_size = sizeof (FILE_RW_HEAD) + IFS(D_K,P_K,Fk) + 1;

    /* F4: fd 캐시 - 실패 시 기존 open/close 방식으로 폴백 */
    fd = Fd_Cache_Get(f_name, O_RDWR|O_CREAT|O_LARGEFILE, 0664);
    fd_cached = (fd != -1);

    if (!fd_cached)
        fd = open(f_name, O_RDWR|O_CREAT|O_LARGEFILE, 0664);

    if (fd == -1) {
        Log(SAM_FATAL, "F_R:open fail[%s] {%d:%s}", f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    offset = IFR(D_K,P_K,Fk,Ck) * rec_size;

    do {
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = (long)(rec_size * p_cnt);

        rt = fcntl(fd, F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            if (!fd_cached)
                close(fd);
            Log(SAM_FATAL, "F_R:cannot lock(fcntl)[%d,%s] {%d:%s}",
                    p_type, f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = lseek64(fd, offset, SEEK_SET);

    if (rt == -1) {
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        if (!fd_cached)
            close(fd);
        Log(SAM_FATAL, "F_R:lseek64 fail[%ld] {%d:%s}", offset, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    rt = F_R_Proc(fd, p_buf, rec_size, p_cnt);

    if (rt > 0) {
        if (p_type / PS_R_1 >= 2) {
            IFR(D_K,P_K,Fk,Ck) += rt;
            Seq_Save(IFN(D_K,P_K,Fk), D_K, PROC(D_K,P_K).in_f[Fk] - 1);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    if (!fd_cached)
        close(fd);

    return (rt);
}   /* End of F_R ()    */

/*----------------------------------------------------------------------*/
int     F_R_Proc(int p_fd, char *p_buf, int p_size, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int             rt, cnt, i, len, pt, tmp_off;
    char            buf[FILE_BUF_LEN], tmp[FILE_BUF_LEN];
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    len = p_size * p_cnt;
    pt = 0;

    while (1) {
        rt = read(p_fd, buf+pt, len);

        if (rt < 0)
            return (NOTOK);
        else if (rt == 0) {
            if (len + pt == p_size * p_cnt) {
                rt = pt;

                if (rt)
                    break;
            }

            return (OK);
        }

        if (rt == len) {
            rt += pt;
            break;
        }

        len -= rt;
        pt += rt;
    }

    cnt = rt / p_size;

    if (rt != p_size * cnt)
        return (NOTOK);

    memset(tmp, 0, sizeof (tmp));
    tmp_off = 0;

    for (i = 0; i < rt; i += p_size) {
        memcpy(file_rw.Seq, buf+i, sizeof (FILE_RW_HEAD));
        memcpy(buff_rw.Seq, file_rw.Seq+1, sizeof (buff_rw.Seq));
        memcpy(buff_rw.If_Seq, file_rw.If_Seq+1, sizeof (buff_rw.If_Seq));
        memcpy(buff_rw.ApType, file_rw.ApType+1, sizeof (buff_rw.ApType));
        memcpy(buff_rw.ResponseCode, file_rw.ResponseCode+1,
                sizeof (buff_rw.ResponseCode));
        memcpy(buff_rw.RecvTime1, file_rw.RecvTime1+1,
                sizeof (buff_rw.RecvTime1));
        memcpy(buff_rw.RecvTime2, file_rw.RecvTime2+1,
                sizeof (buff_rw.RecvTime2));
        memcpy(buff_rw.DataHeader, file_rw.DataHeader+1,
                sizeof (buff_rw.DataHeader));
        memcpy(&tmp[tmp_off], buff_rw.Seq, sizeof (BUFF_RW_HEAD));
        tmp_off += sizeof (BUFF_RW_HEAD);
        memcpy(&tmp[tmp_off], buf+i+sizeof(FILE_RW_HEAD),
                p_size - sizeof (FILE_RW_HEAD));
        tmp_off += p_size - sizeof (FILE_RW_HEAD);
    }

    memcpy(p_buf, tmp, tmp_off);

    return (cnt);
}   /* End of F_R_Proc ()   */

/*************************************************************************
    Function        : . write to data file
    Parameters IN   : . p_out   : output data SHM number
                      . p_buf   : journal data
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int
                        > 0 : success (write count)
                        -1  : failure
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_W(int p_out, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size, fp_cached;
    char            f_name[100], sub[4];
    FILE            *fp;
    struct flock    lock;

    if (p_buf == NULL) {
        Log(SAM_ERROR, "F_W:write buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 30 < p_cnt) {
        Log(SAM_ERROR, "F_W:invalid write count[%d]", p_cnt);
        return (NOTOK);
    }

    sprintf(sub, "%s", _SubSystem_Name);
    LtoU(sub, 2);

    p_out = p_out / 10;

    if (p_out < 1 || p_out > 99) {
        Log(USR_ERROR, "F_W:invalid output file number[%d:1-99]", p_out);
        return (NOTOK);
    }

    Fk = p_out - 1;

    sprintf(Ofn, "%s", OFN(D_K,P_K,Fk));
    sprintf(f_name, "%s/%s/00000000/%s", _FEP_DAT, sub, Ofn);
    Fifo_Cnt = OFC(D_K,P_K,Fk);
    rec_size = sizeof (BUFF_RW_HEAD) + OFS(D_K,P_K,Fk) + 1;

    /* F4: FILE* 캐시 - 실패 시 기존 fopen/fclose 방식으로 폴백 */
    fp = Fp_Cache_Get(f_name, "a+w");
    fp_cached = (fp != NULL);

    if (!fp_cached)
        fp = fopen64(f_name, "a+w");

    if (fp == NULL) {
        Log(SAM_FATAL, "F_W:fopen64 fail[%s] {%d:%s}",
                f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_END;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            if (!fp_cached)
                fclose(fp);
            Log(SAM_FATAL, "F_W:cannot lock(fcntl)[OF%d,%s] {%d:%s}",
                    p_out, f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = F_W_Proc(fileno(fp), p_buf, rec_size, p_cnt);

    fflush(fp);

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0L;
    lock.l_len = 0L;
    fcntl(fileno(fp), F_SETLK, &lock);
    if (!fp_cached)
        fclose(fp);

    return (rt);
}   /* End of F_W ()    */

/*----------------------------------------------------------------------*/
int     F_W_Proc(int p_fd, char *p_buf, int p_size, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int             rt, cnt, i, j, k, FIFO_fd, len, pt, f_size, null_cnt;
    int             fifo_cached;
    char            buf[FILE_BUF_LEN], sub[4], fifo_name[256], null_flag;
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    memset(buf, 0, sizeof (buf));

    sprintf(sub, "%s", _SubSystem_Name);
    LtoU(sub, 2);
    cnt = OFW(D_K,P_K,Fk,0) + 1;
    f_size = p_size - sizeof (BUFF_RW_HEAD) + sizeof (FILE_RW_HEAD);

    /* 2025EDIT */
    char    Tmp[128];
    memset(Tmp, 0, sizeof(Tmp));
    /* 2025EDIT */

    for (i = 0, k = 0; i < p_size * p_cnt; i += p_size, k += f_size, cnt ++) {
        memcpy(buff_rw.Seq, p_buf+i, sizeof (BUFF_RW_HEAD));
        /* 2025EDIT */
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
        /* 2025EDIT */

        memcpy(&buf[k], file_rw.Seq, sizeof (FILE_RW_HEAD));
        memcpy(&buf[k+sizeof(FILE_RW_HEAD)], &p_buf[i+sizeof(BUFF_RW_HEAD)],
                p_size - sizeof (BUFF_RW_HEAD));

        null_flag = 0;
        null_cnt = 0;

        for (j = 0; j < f_size; j ++) {
            if (buf[k+j] == '\0') {
                buf[k+j] = ' ';
                null_flag = 1;
                null_cnt ++;
            }
        }

        if (null_flag == 1)
            Log(SAM_WARN, "%s:%d null(s) replaced with space:wcnt[%d]",
                    Ofn, null_cnt, cnt);
    }

    len = strlen(buf);
    pt = 0;

    while (1) {
        rt = write(p_fd, buf+pt, len);

        if (rt == -1) {
            Log(SAM_FATAL, "F_W_Proc:cannot write[%d] {%d:%s}",
                    p_fd, SYS_NO, SYS_STR);
            return (NOTOK);
        }
        else if (rt == len)
            break;

        len -= rt;
        pt += rt;
    }

    OFW(D_K,P_K,Fk,0) += p_cnt;

    for (i = 1; i <= Fifo_Cnt; i ++) {
        sprintf(fifo_name, "%s/%s/%s%d", _FEP_FIFO, sub, Ofn, i);

        /* F4: FIFO fd 캐시 - 실패 시 기존 open/close 방식으로 폴백 */
        FIFO_fd = Fd_Cache_Get(fifo_name, O_RDWR|O_NDELAY, 0);
        fifo_cached = (FIFO_fd >= 0);

        if (!fifo_cached)
            FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);

        if (FIFO_fd < 0)
            Log(FIF_FATAL, "F_W_Proc:cannot open FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        rt = write(FIFO_fd, "1", 1);

#if defined __linux
        if (rt < 0 && errno != EAGAIN)
#else
            if (rt < 0)
#endif
            Log(FIF_FATAL, "F_W_Proc:cannot write FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        if (!fifo_cached)
            close(FIFO_fd);
    }

    return (p_cnt);
}   /* End of F_W_Proc ()   */

/*************************************************************************
    Function        : . Add_Count
    Parameters IN   : . p_type  : process type
                      . p_cnt   : read count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . add read count
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Add_Count(int p_type, int p_cnt)
/*----------------------------------------------------------------------*/
{
    if (p_cnt < 0) {
        Log(USR_FATAL, "Add_Count:invalid p_cnt[%d]", p_cnt);
        Exit_Process();
    }

    Fk = p_type / PS_R_1;
    Ck = p_type % PS_R_1;

    if (Fk != 1 || Ck < 0 || Ck > 8) {
        Log(USR_FATAL, "Add_Count:invalid process type[%d]", p_type);
        Exit_Process();
    }

    Fk --;
    IFR(D_K,P_K,Fk,Ck) += p_cnt;
    Seq_Save(IFN(D_K,P_K,Fk), D_K, PROC(D_K,P_K).in_f[Fk] - 1);

    return;
}   /* End of Add_Count ()  */

/*************************************************************************
    Function        : . read data file
    Parameters IN   : . r_seq   : read sequence
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int (>= 0:success (read count), -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_R2(char *p_buf, int r_seq)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size, fd;
    long            offset;
    char            f_name[100], sub[4];
    struct flock    lock;

    if (p_buf == NULL) {
        Log(SAM_ERROR, "F_R2:read buffer is NULL");
        return (NOTOK);
    }

    sprintf(sub, "%s", _SubSystem_Name);
    LtoU(sub, 2);

    sprintf(f_name, "%s/%s/00000000/%s", _FEP_DAT, sub, IFN(D_K,P_K,0));
    rec_size = sizeof (FILE_RW_HEAD) + IFS(D_K,P_K,0) + 1;

    fd = open(f_name, O_RDWR|O_CREAT, 0664);

    if (fd == -1) {
        Log(SAM_FATAL, "F_R2:open fail[%s] {%d:%s}", f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    offset = r_seq * rec_size;

    do {
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = (long)rec_size;

        rt = fcntl(fd, F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            close(fd);
            Log(SAM_FATAL, "F_R2:cannot lock(fcntl)[%s] {%d:%s}",
                    f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = lseek64(fd, offset, SEEK_SET);
    if (rt == -1) {
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        Log(SAM_FATAL, "F_R2:seek failure[%ld] {%d:%s}",
                offset, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    rt = F_R_Proc(fd, p_buf, rec_size, 1);

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return (rt);
}   /* End of F_R2 ()   */

/*************************************************************************
    Function        : . write to data file
    Parameters IN   : . f_name  : file name
                      . p_buf   : journal data
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    F_W2(char *f_name, char *p_buf)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size, i, f_k;
    char            file_name[100], sub[4];
    FILE            *fp;
    struct flock    lock;

    if (f_name == NULL && strlen(f_name) == 0) {
        Log(SAM_ERROR, "F_W2:file name is NULL");
        Exit_Process();
    }

    if (p_buf == NULL && strlen(p_buf) == 0) {
        Log(SAM_ERROR, "F_W2:write buffer is NULL");
        Exit_Process();
    }

    Dk = f_name[1] - 'a';

    for (f_k = 0; f_k < DAEMON(Dk).f_count; f_k ++) {
        if (memcmp(FILEM(Dk,f_k).file_name, f_name, 10) == 0)
            break;

        if (f_k == DAEMON(Dk).f_count - 1) {
            Log(USR_FATAL, "F_W2:unregistered file name[%s]", f_name);
            Exit_Process();
        }
    }

    sprintf(sub, "%2.2s", f_name);
    LtoU(sub, 2);

    sprintf(file_name, "%s/%s/00000000/%s",
            _FEP_DAT, sub, FILEM(Dk,f_k).file_name);
    rec_size = FILEM(Dk,f_k).record_size;
    rec_size += (sizeof (BUFF_RW_HEAD) + 1);

    fp = fopen64(file_name, "a+w");

    if (fp == NULL) {
        Log(SAM_FATAL, "F_W2:fopen64 fail[%s] {%d:%s}",
                file_name, SYS_NO, SYS_STR);
        Exit_Process();
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_END;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            fclose(fp);
            Log(SAM_FATAL, "F_W2:cannot lock(fcntl)[%s] {%d:%s}",
                    file_name, SYS_NO, SYS_STR);
            Exit_Process();
        }
    } while (rt == -1);

    rt = F_W2_Proc(fileno(fp), p_buf, rec_size, f_k);

    fflush(fp);

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0L;
    lock.l_len = 0L;
    fcntl(fileno(fp), F_SETLK, &lock);
    fclose(fp);

    if (rt < 0) {
        Log(SAM_ERROR, "F_W2[%s][%d,%s]", file_name, SYS_NO, SYS_STR);
        Exit_Process();
    }

    return;
}   /* End of F_W2 ()   */

/*----------------------------------------------------------------------*/
int     F_W2_Proc(int p_fd, char *p_buf, int p_size, int fk)
/*----------------------------------------------------------------------*/
{
    int             rt, cnt, i, FIFO_fd, len, pt, f_size, null_cnt, buf_off;
    int             fifo_cached;
    char            buf[FILE_BUF_LEN], sub[4], fifo_name[256], null_flag;
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    memset(buf, 0, sizeof (buf));

    sprintf(sub, "%2.2s", FILEM(Dk,fk).file_name);
    LtoU(sub, 2);
    cnt = FILEM(Dk,fk).w_cnt[0] + 1;
    f_size = p_size - sizeof (BUFF_RW_HEAD) + sizeof (FILE_RW_HEAD);
    Fifo_Cnt = FILEM(Dk,fk).fifo_count;

    /* 2025EDIT */
    char    Tmp[128];
    memset(Tmp, 0, sizeof(Tmp));

    memcpy(buff_rw.Seq, p_buf, sizeof (BUFF_RW_HEAD));
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
    /* 2025EDIT */

    buf_off = strlen(buf);
    memcpy(&buf[buf_off], file_rw.Seq, sizeof (FILE_RW_HEAD));
    buf_off += sizeof (FILE_RW_HEAD);
    memcpy(&buf[buf_off], p_buf+sizeof(BUFF_RW_HEAD),
            p_size - sizeof (BUFF_RW_HEAD));

    null_flag = 0;
    null_cnt = 0;

    for (i = 0; i < f_size; i ++) {
        if (buf[i] == '\0') {
            buf[i] = ' ';
            null_flag = 1;
            null_cnt ++;
        }
    }

    if (null_flag == 1)
        Log(SAM_WARN, "%s:%d null(s) replaced with space",
                FILEM(Dk,fk).file_name, null_cnt);

    len = strlen(buf);
    pt = 0;

    while (1) {
        rt = write(p_fd, buf+pt, len);
        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;
            Log(SAM_FATAL, "F_W2_Proc:cannot write[%d] {%d:%s}",
                    p_fd, SYS_NO, SYS_STR);
            return (NOTOK);
        }
        else if (rt == len)
            break;

        len -= rt;
        pt += rt;
    }

    FILEM(Dk,fk).w_cnt[0] ++;

    for (i = 1; i <= Fifo_Cnt; i ++) {
        sprintf(fifo_name, "%s/%s/%s%d",
                _FEP_FIFO, sub, FILEM(Dk,fk).file_name, i);

        /* F4: FIFO fd 캐시 - 실패 시 기존 open/close 방식으로 폴백 */
        FIFO_fd = Fd_Cache_Get(fifo_name, O_RDWR|O_NDELAY, 0);
        fifo_cached = (FIFO_fd >= 0);

        if (!fifo_cached)
            FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);

        if (FIFO_fd < 0)
            Log(FIF_FATAL, "F_W2_Proc:cannot open FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        rt = write(FIFO_fd, "1", 1);

        if (rt < 0)
            Log(FIF_FATAL, "F_W2_Proc:cannot write FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        if (!fifo_cached)
            close(FIFO_fd);
    }

    return (1);
}   /* End of F_W2_Proc ()  */

/*************************************************************************
    Function        : . read data file
    Parameters IN   : . dk      : damon key
                      . fk      : file key
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int (>= 0:success (read count), -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_R3(int dk, int fk, char *p_buf)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size, fd;
    long            offset;
    char            f_name[100], sub[4];
    struct flock    lock;

    if (p_buf == NULL) {
        Log(SAM_ERROR, "F_R3:read buffer is NULL");
        return (NOTOK);
    }

    sprintf(sub, "%c%c", _System_Name[0], dk + 'A');
    LtoU(sub, 2);

    sprintf(f_name, "%s/%s/00000000/%s",
            _FEP_DAT, sub, FILEM(dk,fk).file_name);
    rec_size = sizeof (FILE_RW_HEAD) + FILEM(dk,fk).record_size + 1;

    fd = open(f_name, O_RDWR|O_CREAT, 0664);

    if (fd == -1) {
        Log(SAM_FATAL, "F_R3:open fail[%s] {%d:%s}", f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    offset = FILEM(dk,fk).r_cnt[0] * rec_size;

    do {
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = (long)rec_size;

        rt = fcntl(fd, F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            close(fd);
            Log(SAM_FATAL, "F_R3:cannot lock(fcntl)[%s] {%d:%s}",
                    f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = lseek64(fd, offset, SEEK_SET);

    if (rt == -1) {
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        Log(SAM_FATAL, "F_R3:seek failure[%ld] {%d:%s}",
                offset, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    rt = F_R_Proc(fd, p_buf, rec_size, 1);

    if (rt > 0) {
        FILEM(dk,fk).r_cnt[0] += rt;
        Seq_Save(FILEM(dk,fk).file_name, dk, fk);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);

    return (rt);
}   /* End of F_R3 ()   */

/*************************************************************************
    Function        : . write to data file
    Parameters IN   : . dk      : daemon key
                      . fk      : file key
                      . p_buf   : journal data
    Parameters OUT  : .
    Return Code     : . int (>= 0:success (write count), -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_W3(int dk, int fk, char *p_buf)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size;
    char            file_name[100], sub[4];
    FILE            *fp;
    struct flock    lock;

    if (p_buf == NULL && strlen(p_buf) == 0) {
        Log(SAM_ERROR, "F_W3:write buffer is NULL");
        return (NOTOK);
    }

    Dk = dk;
    sprintf(sub, "%c%c", _System_Name[0], dk + 'A');
    LtoU(sub, 2);

    sprintf(file_name, "%s/%s/00000000/%s",
            _FEP_DAT, sub, FILEM(dk,fk).file_name);
    rec_size = FILEM(dk,fk).record_size;
    rec_size += (sizeof (BUFF_RW_HEAD) + 1);

    fp = fopen64(file_name, "a+w");

    if (fp == NULL) {
        Log(SAM_FATAL, "F_W3:fopen64 fail[%s] {%d:%s}",
                file_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_END;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            fclose(fp);
            Log(SAM_FATAL, "F_W3:cannot lock(fcntl)[%s] {%d:%s}",
                    file_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = F_W2_Proc(fileno(fp), p_buf, rec_size, fk);

    fflush(fp);

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0L;
    lock.l_len = 0L;
    fcntl(fileno(fp), F_SETLK, &lock);
    fclose(fp);

    return (rt);
}   /* End of F_W3 ()   */

/*************************************************************************
    Function        : . write to data file
    Parameters IN   : . dk      : daemon key
                      . fk      : file key
                      . p_buf   : journal data
                      . p_cnt   : data count
    Parameters OUT  : .
    Return Code     : . int (>= 0:success (write count), -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     F_WB(int dk, int fk, char *p_buf, int p_cnt)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size;
    int             cnt, i, j, k, FIFO_fd, len, pt, f_size, null_cnt;
    int             fifo_cached;
    char            f_name[100], sub[4];
    char            buf[FILE_BUF_LEN], fifo_name[256], null_flag;
    FILE            *fp;
    struct flock    lock;
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    if (p_buf == NULL && strlen(p_buf) == 0) {
        Log(SAM_ERROR, "F_WB:write buffer is NULL");
        return (NOTOK);
    }

    if (p_cnt < 1 || 30 < p_cnt) {
        Log(SAM_ERROR, "F_WB:invalid write count[%d]", p_cnt);
        return (NOTOK);
    }

    sprintf(sub, "%c%c", _System_Name[0], dk + 'A');
    LtoU(sub, 2);

    sprintf(f_name, "%s/%s/00000000/%s",
            _FEP_DAT, sub, FILEM(dk,fk).file_name);
    rec_size = sizeof (BUFF_RW_HEAD) + FILEM(dk,fk).record_size + 1;

    fp = fopen64(f_name, "a+w");

    if (fp == NULL) {
        Log(SAM_FATAL, "F_WB:fopen64 fail[%s] {%d:%s}",
                f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_END;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            fclose(fp);
            Log(SAM_FATAL, "F_WB:cannot lock(fcntl)[%s] {%d:%s}",
                    f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    memset(buf, 0, sizeof (buf));

    cnt = FILEM(dk,fk).w_cnt[0] + 1;
    f_size = rec_size - sizeof (BUFF_RW_HEAD) + sizeof (FILE_RW_HEAD);
    Fifo_Cnt = FILEM(dk,fk).fifo_count;

    /* 2025EDIT */
    char    Tmp[128];
    memset(Tmp, 0, sizeof(Tmp));
    /* 2025EDIT */

    for (i = 0, k = 0; i < rec_size * p_cnt; i += rec_size, k += f_size, cnt ++) {
        memcpy(buff_rw.Seq, p_buf+i, sizeof (BUFF_RW_HEAD));
        /* 2025EDIT */
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
        /* 2025EDIT */

        memcpy(&buf[k], file_rw.Seq, sizeof (FILE_RW_HEAD));
        memcpy(&buf[k+sizeof(FILE_RW_HEAD)], &p_buf[i+sizeof(BUFF_RW_HEAD)],
                rec_size - sizeof (BUFF_RW_HEAD));

        null_flag = 0;
        null_cnt = 0;

        for (j = 0; j < f_size; j ++) {
            if (buf[k+j] == '\0') {
                buf[k+j] = ' ';
                null_flag = 1;
                null_cnt ++;
            }
        }

        if (null_flag == 1)
            Log(SAM_WARN, "%s:%d null(s) replaced with space:wcnt[%d]",
                    FILEM(dk,fk).file_name, null_cnt, cnt);
    }

    len = strlen(buf);
    pt = 0;

    while (1) {
        rt = write(fileno(fp), buf+pt, len);

        if (rt == -1) {
            Log(SAM_FATAL, "F_WB:cannot write[%d] {%d:%s}",
                    fileno(fp), SYS_NO, SYS_STR);
            return (NOTOK);
        }
        else if (rt == len)
            break;

        len -= rt;
        pt += rt;
    }

    FILEM(dk,fk).w_cnt[0] += p_cnt;

    for (i = 1; i <= Fifo_Cnt; i ++) {
        sprintf(fifo_name, "%s/%s/%s%d",
                _FEP_FIFO, sub, FILEM(dk,fk).file_name, i);

        /* F4: FIFO fd 캐시 - 실패 시 기존 open/close 방식으로 폴백 */
        FIFO_fd = Fd_Cache_Get(fifo_name, O_RDWR|O_NDELAY, 0);
        fifo_cached = (FIFO_fd >= 0);

        if (!fifo_cached)
            FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);

        if (FIFO_fd < 0)
            Log(FIF_FATAL, "F_WB:cannot open FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        rt = write(FIFO_fd, "1", 1);

        if (rt < 0)
            Log(FIF_FATAL, "F_WB:cannot write FIFO[%d] {%d:%s}",
                    FIFO_fd, SYS_NO, SYS_STR);

        if (!fifo_cached)
            close(FIFO_fd);
    }

    fflush(fp);

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0L;
    lock.l_len = 0L;
    fcntl(fileno(fp), F_SETLK, &lock);
    fclose(fp);

    return (p_cnt);
}   /* End of F_WB ()   */

/*************************************************************************
    Function        : . Write SISE
    Parameters IN   : . p_type  : process type
                      . p_buf   : sise data
    Parameters OUT  : .
    Return Code     : . int (>= 0:success (write count), -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     SF_W(int p_type, char *p_buf)
/*----------------------------------------------------------------------*/
{
    int             rt, rec_size;
    char            f_name[100], bumun[4];
    FILE            *fp;
    struct flock    lock;

    if (p_buf == NULL) {
        Log(SAM_ERROR, "SF_W:write buffer is NULL");
        return (NOTOK);
    }

    sprintf(bumun, "%s", _SubSystem_Name);
    LtoU(bumun, 2);

    p_type = p_type / 10;

    if (p_type < 1 || p_type > 99) {
        Log(USR_ERROR, "SF_W:invalid output file number[%d]", p_type);
        return (NOTOK);
    }

    Fk = p_type - 1;

    sprintf(f_name, "%s/%s/00000000/%s", _FEP_DAT, bumun, OFN(D_K,P_K,Fk));
    Fifo_Cnt = OFC(D_K,P_K,Fk);
    rec_size = OFS(D_K,P_K,Fk) + sizeof (SISE_RW_HEAD) + 1;

    fp = fopen64(f_name, "a+w");

    if (fp == NULL) {
        Log(SAM_FATAL, "SF_W:fopen64 fail[%s] {%d:%s}",
                f_name, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    do {
        lock.l_type = F_WRLCK;
        lock.l_whence = 0;
        lock.l_start = 0L;
        lock.l_len = 0L;

        rt = fcntl(fileno(fp), F_SETLKW, &lock);

        if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;

            fclose(fp);
            Log(SAM_FATAL, "SF_W:cannot lock(fcntl)[OF%d,%s] {%d:%s}",
                    p_type, f_name, SYS_NO, SYS_STR);
            return (NOTOK);
        }
    } while (rt == -1);

    rt = SF_W_Proc(fileno(fp), p_buf, rec_size);

    fflush(fp);

    lock.l_type = F_UNLCK;
    fcntl(fileno(fp), F_SETLK, &lock);
    fclose(fp);

    return (rt);
}   /* End of SF_W ()   */

/*----------------------------------------------------------------------*/
int     SF_W_Proc(int p_fd, char *p_buf, int p_size)
/*----------------------------------------------------------------------*/
{
    int     rt, len, pt;

    len = p_size;
    pt = 0;

    ItoAf(OFW(D_K,P_K,Fk,0) + 1, p_buf, 8);

    if (T_K >= 0 && _Exe_Name[9] == 'r')
        SISETR(D_K,T_K).count ++;

    while (1) {
        rt = write(p_fd, p_buf+pt, len);

        if (rt == -1) {
            Log(SAM_FATAL, "SF_W_Proc:write fail: fd[%d] {%d:%s}",
                    p_fd, SYS_NO, SYS_STR);
            return (NOTOK);
        }

        if (rt == len)
            break;

        len -= rt;
        pt += rt;
    }

    OFW(D_K,P_K,Fk,0) ++;

    return (1);
}   /* End of SF_W_Proc ()  */

/*************************************************************************
    End of Program (file_rw.c)
*************************************************************************/
