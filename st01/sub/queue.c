#include        "fep_sub.h"

#define     QUEUE_MAX_BYTES 10485760    /* 10 MB max queue size */

/* For Queue */
#define     MAXSIZE         40960       /* 40 KB max message size */
#define PERM                0x1B6
#define QUEUE_READY         -1
#define QUEUE_WAIT          -2
#define QUEUE_FULL          -3
#define QUEUE_NOT_EXIST     -4
#define QUEUE_TIME_OUT      -5
#define QUEUE_SEND_ERROR    -6
#define TRUE                1
#define FALSE               0

/*************************************************************************
    Function        : . Receive message from queue
    Parameters IN   : . int QID;
                      . long owner;
                      . char *data;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************************/
int ReceiveQueue(int QID, long owner, char *data) {
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
    Function        : . Receive message from queue
    Parameters IN   : . int QID;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************************/
/*
 * Gets current queue status.
 */
int CheckQueue(int QID) {
    struct msqid_ds msqid_ds, *buf;
    int     n;

    buf = &msqid_ds;
    if(msgctl(QID,IPC_STAT,buf) == -1)
        return QUEUE_NOT_EXIST;
    n  = QUEUE_MAX_BYTES - buf->msg_cbytes;
    if(n < MAXSIZE)     return QUEUE_FULL;
    else if(buf->msg_cbytes > 0)    return QUEUE_READY;
    else                            return QUEUE_WAIT;
}

/*
 * Make and get local message queue id.
*/
int MakeQueue(size_t KEY) {
    Msgbuf msgbuf;
    int rtv, QID;

    QID = msgget(KEY,PERM|IPC_CREAT);
    if(QID == -1) return FALSE;

    /* select and insert/remove */

    return QID;
}
