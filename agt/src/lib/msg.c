
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "log.h"


typedef struct {
   long mtype;
   char mtext[2048];
}msg_buf;

int msg_queue_init( int key )
{
    key_t  msg_key;
    int    msg_flags;
    int    msgid = 0;

    msg_key= key;
    msg_flags= IPC_CREAT|0666 ;

    msgid = msgget( msg_key, msg_flags);
    if( msgid == -1)
    {
        perror( "msgget");
        switch(errno)
        {
            case EACCES:
               LOG_ERR("권한오류: 기존큐에 접근할수 없습니다.");
               break;
            case ENOMEM:
               LOG_ERR("메모리부족: 커널큐를 만들수 없습니다.");
               break;
            case ENOSPC:
               LOG_ERR("큐개수한도초과 : msgmni 제한초과");
               break;
            default:
               LOG_ERR("큐에러 !!!");
        }
        return -1;
    }
	LOG_INFO( "msgid :%d\n", msgid);
    return msgid;

}

int msg_queue_send( int msgid,  char *data, int data_len )
{
    int rc    =0;
    int count =0;
    struct msqid_ds info;
	msg_buf  msg;

    
    rc = msgctl (msgid, IPC_STAT,&info);
    if( rc == -1)
    {
         LOG_ERR( "msgctl오류: errno[%d:%s]", errno , strerror(errno));
         return -1;
    }

    LOG_INFO( "현재msg수:%lu / 최대byte:%lu / 현재byte:%lu", info.msg_qnum,info.msg_qbytes,info.msg_cbytes);

#if 0
    msg.mtype = g_cfg.order_msg_mtype;
    memcpy( msg.mtext, data, data_len);
#endif

    msg.mtype = 100;
    memcpy( msg.mtext, data, data_len);
MSG_SND:
    rc = msgsnd( msgid, &msg, data_len , IPC_NOWAIT);
    if( rc == -1)
    {
         if(errno == EAGAIN)
         {
              LOG_WARN("msg 큐가 찼습니다(overflow)-- 재시도");
              sleep(2); // 2초마다 재시도
              count ++;
              if( count < 30)
              {
                 goto MSG_SND;
              }
              else
              {
                 LOG_ERR("msg 큐 overflow error !!! errno[%d:%s]", errno, strerror(errno));
                 return -99;
              }
         }
         else
         {
              LOG_ERR("msg 큐 error !!! errno[%d:%s]", errno, strerror(errno));
         }
    }
    return 0;
}

