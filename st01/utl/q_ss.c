#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

/* For Queue */
#define READ_BUF_SIZE       1024
#define QUEUE_MAX_BYTES     65535
#define MAXSIZE             4096
#define PERM                0x1B6
#define WAIT_TIME           60
#define QUEUE_READY         -1
#define QUEUE_WAIT          -2
#define QUEUE_FULL          -3
#define QUEUE_NOT_EXIST     -4
#define QUEUE_TIME_OUT      -5
#define QUEUE_SEND_ERROR    -6
#define TRUE                1
#define FALSE               0

typedef struct {
    long mtype;
    unsigned char mtext[MAXSIZE];
} Msgbuf;

char    *Get_MicroTime(char *p_time);
char    u_time[20];

int     main() {
    int i, rt, msqid, buf_length;

    memset(u_time, 0, sizeof (u_time));

    key_t   new_key;
    new_key = 0x33000003;
    Msgbuf  sbuf;

    msqid = MakeQueue(new_key);
    if (msqid < 0)
        printf("msgget fail msqid[%d]\n", msqid);
    else
        printf("msgget OK msqid[%d]\n", msqid);

    sbuf.mtype = 1;

    for (i = 0; i < 1; i++) {
        memset(sbuf.mtext, 0,      sizeof(sbuf.mtext));
        memset(sbuf.mtext, 0x20,   500);
        memcpy(sbuf.mtext, "500100        0301", 18);

        buf_length = strlen(sbuf.mtext)+1;
        Get_MicroTime(u_time);
        printf("Send i[%d] tm[%2.2s:%2.2s:%2.2s.%6.6s]\n", i, u_time, u_time+2, u_time+4, u_time+6);
        rt = msgsnd(msqid, &sbuf, buf_length, 0);
        if (rt < 0) {
            printf("ERROR [%d] [%d] [%s] [%d]\n", msqid, sbuf.mtype, sbuf.mtext, strlen(sbuf.mtext));
            exit(1);
        }
        sleep(1);
    }
}

/*************************************************************************
    Function        : . Receive message from queue
    Parameters IN   : . int QID;
                      . long owner;
                      . char *data;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************************/
int ReceiveQueue(QID, owner, data)
int QID;
long owner;
char *data;
{
    int rtrn;
    Msgbuf msgbuf;

    msgbuf.mtype = (size_t) owner;
    rtrn = msgrcv(QID,&msgbuf,MAXSIZE,msgbuf.mtype,0);
    if(rtrn > 0) {
        memcpy(data,msgbuf.mtext,rtrn);
        return rtrn;
    }
    else {
        return QUEUE_NOT_EXIST;
    }
}

/*
 * Make and get local message queue id.
*/
int MakeQueue(KEY)
size_t KEY;
{
    char buff[MAXSIZE];
    Msgbuf msgbuf;
    int rtv, QID;

    QID = msgget(KEY,PERM|IPC_CREAT);
    if(QID == -1) return FALSE;

    return QID;
}

/*************************************************************************
    Function        : . Receive message from queue
    Parameters IN   : . int QID;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************************/
int QueueClear(int QID) {
    int status;
    Msgbuf msgbuf;

    msgbuf.mtype = (size_t)0;
    do{
        status=msgrcv(QID,&msgbuf,MAXSIZE,msgbuf.mtype,
                IPC_NOWAIT);
        if(status<0) status=0;
    }while(status);
    return TRUE ;
}

/*************************************************************************
    Function        : . Receive message from queue
    Parameters IN   : . size_t KEY;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************************/
/*
 * Make queue and get message queue id.
 */
int GetQid(size_t KEY) {
    int QID;

    QID = msgget(KEY,PERM);
    if(QID==-1) return -1;
    return QID;
}

/*************************************************************************
    Function        : . get time to the unit of microsec
    Parameters IN   : .
    Parameters OUT  : . p_time  : time string
    Return Code     : . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *Get_MicroTime(char *p_time)
/*----------------------------------------------------------------------*/
{
    struct timeval  tv;
    struct tm       *date, date1;

    gettimeofday(&tv, NULL);
    date = (struct tm *)localtime_r(&(tv.tv_sec), &date1);

    /* HHMMSSmmmmmm (12) = 22 bytes  */
    sprintf(p_time, "%02d%02d%02d%06ld",
            date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec);

    return (p_time);
}   /* End of Get_MicroTime ()  */
