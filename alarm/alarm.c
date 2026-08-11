
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include "alarm.h"
#define  OMS_MSG_KEY 0xff170990
/*********************** 16 bit CRC TABLE for TOKEN ***************************/
unsigned short m_crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};
#define CRC_M(data, accum) ((accum>>8)^m_crctab[(accum^(data&0x00ff))&0x00ff])


typedef struct {
   char    TrCode [6];              /* TrCode               */
   char    Scr_key[4];              /* 화면키   cid로 사용   */
   char    ErrCode[4];              /* Error Code           */
   char    ApType_Cd[5];            /* Aptype Code          */
                                    /* Process 이름 (50101) */
                                    /* 자동기동/자동종료/강제종료시 */
   char    Media_gbn[1];            /* 매체구분(A/C/T)      */
   char    Cid   [10];              /* cid값                */
   char    Filler[20];              /* 예비                 */
}DATAHEAD;

// oms 발송 메시지  
typedef struct 
{
       char scop[  1]; // 알림대상구분코드 C:고객, E:직원
//       char mtyp[  1]; // 메세지구분 1 팝업,2 쪽지,3 팝업+쪽지,4 SMS,5 팝업+SMS,6 쪽지+SMS,7 팝업+쪽지+SMS
       char mcod[  5]; // 알림메시지코드 TSKEIAM51 메시지설정목록에 저장된 메시지코드
       char scen[ 10]; // 거래ID(화면번호)
       char rvid[ 10]; // 수신고객ID,직원번호
       char sdid[ 10]; // 발송직원번호
       char mesg[400]; //메시지 내에 가변문자열 태그가 있을 경우 대치할 문자열 목록 '|'로 구분
} RCV_MSG; // 수신메시지구조체


typedef struct {
   long mtype;
   char mtext[1024];
}msg_buf;

static int l_ftok(ipc_name, ipc_type)
char    *ipc_name;
int ipc_type;
{
    unsigned char    w_cc;
    int  w_crc,  w_token;
    register int ii;

    switch (ipc_type)
    {
    case 'Q': break;        /* message queue */
    case 'M': break;        /* shared memory */
    case 'S': break;        /* semaphore     */
    default:  errno = EFAULT;   /* bad address   */
          return (-1);
    }

    w_crc = 0;
    for (ii = 0; ii < strlen(ipc_name); ii++)
    {
        w_cc  = ipc_name[ii];
        w_crc = CRC_M(w_cc, w_crc);
    }
    w_token = (ipc_type << 16) | w_crc;
    return (w_token);
}


static int l_msgget(queue_name, size, mode)
char    *queue_name;
int    size, mode;
{
    struct  msqid_ds msqid_ds;
    int qkey, qid;
    int rc;

    if (size >= (1024*64))
        size = (1024*64-1);

    if (mode & IPC_CREAT && size == 0)
    {
        errno = EFAULT;         /* Oh! no queue size    */
        return (-1);
    }

#if 0
    qkey= l_ftok( queue_name,'Q');
    if( qkey == -1)
       return -1;
#endif
    qkey = OMS_MSG_KEY;

    qid = msgget( qkey, mode);
    if( qid == -1)
       return -1;

    if (mode & IPC_CREAT)
    {
        rc = msgctl(qid, IPC_STAT, &msqid_ds);
        if (rc == 0 && msqid_ds.msg_qbytes != size)
        {
            msqid_ds.msg_qbytes = size;
            rc = msgctl(qid, IPC_SET, &msqid_ds);
        }
    }
    return (qid);
}


#define     ECM_SMSSENDQUE          "BW161001"
int alrt_msg( int trcode, char *msg ,int msg_len)
{
      int rc     =0;
      int sndlen =0;
//      RCV_MSG       rcvmsg;
      msg_buf       msgbuf;
      char trbuf[7];
      size_t qkey = OMS_MSG_KEY;

      int qid = 0;

      if (msg_len > 400) msg_len = 400;

#if 0
      if((qid = l_msgget( ECM_SMSSENDQUE, 64*1024, IPC_CREAT | 0666)) < 0)
#endif

      if ((qid = msgget(qkey, IPC_CREAT | 0666)) < 0)
      {        
          return -1;
      } 

      DATAHEAD hd;
      memset(&hd, 0x20, sizeof(DATAHEAD));

      sprintf(trbuf, "%06d", trcode); trbuf[6] = 0x00;
      memcpy(hd.TrCode, trbuf, 6);
/*
      memset(&rcvmsg  , 0x00, sizeof(rcvmsg));
      memset(&msgbuf, 0x00, sizeof(msg_buf));

      memcpy(rcvmsg.scop ,"E"     ,sizeof(rcvmsg.scop));
      memcpy(rcvmsg.mcod ,"E0070" ,sizeof(rcvmsg.mcod));
      memcpy(rcvmsg.scen ,"          "  ,sizeof(rcvmsg.scen));
      memcpy(rcvmsg.rvid ,"          "  ,sizeof(rcvmsg.rvid));
      memcpy(rcvmsg.sdid ,"          "  ,sizeof(rcvmsg.sdid) );
      memcpy(rcvmsg.mesg ,msg  ,msg_len);
      sndlen = strlen(&rcvmsg);
*/

      msgbuf.mtype = 100L;
      memcpy(msgbuf.mtext , &hd, sizeof(DATAHEAD));
      memcpy(msgbuf.mtext+sizeof(DATAHEAD), msg, msg_len);
      sndlen = sizeof(DATAHEAD) + msg_len;

      rc = msgsnd(qid, &msgbuf, sndlen, IPC_NOWAIT);
      if (rc != 0)
      {
          return -1;
      }
      return  0;
}
