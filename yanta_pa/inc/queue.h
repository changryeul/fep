/*
 * File name: queue.h
 */
#ifndef __QUEUE_H__
#define __QUEUE_H__

#define PERM                0x1B6
#define MAXSIZE             4096
#define QUEUE_MAX_BYTES     65535
#define WAIT_TIME           60

#define QUEUE_READY         -1
#define QUEUE_WAIT          -2
#define QUEUE_FULL          -3
#define QUEUE_NOT_EXIST     -4
#define QUEUE_TIME_OUT      -5
#define QUEUE_SEND_ERROR    -6

typedef struct {
   long mtype;
   unsigned char mtext[MAXSIZE];
   } Msgbuf;

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

/*
 * Functions of Host site.
 */
extern int MakeQueue();         /* Server porcess Only */
extern int RemoveQueue();       /* Server Process Only */
extern int GetQid();            /* Client Process Only */
extern int CheckQueue();        /* Server & Client */
extern int SendQueue();         /* buffer size (integer) */
extern int ReceiveQueue();      /* only return value */
extern int QueueClear();

#endif
