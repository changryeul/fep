/*------------------------------------------------------------------------
#   Module  : OMS 알림(alarm) 송신 — libfepP 편입판
#   File    : alarm.c
#
#   원본 = alarm/alarm.c (자립 라이브러리 libalarm.a, libc/POSIX only).
#   libfepP 편입(2026-08-10):
#     - 실제 함수 alrt_msg(trcode, msg, msg_len): OMS 알림 msgq(OMS_MSG_KEY)로
#       DATAHEAD(50B) + body 를 msgsnd. wire 계약(키/헤더/mtype) 원본 보존.
#     - "로그체계 합침": 실패 시 조용히 -1 하던 것을 FEP Log() 로 관측 가능화.
#       libfepP 내부 호출이라 외부 의존성은 새로 생기지 않음(독립성 유지).
#     - 원본 l_ftok/l_msgget + 256엔트리 CRC 테이블은 #if 0 죽은 경로(대체
#       키잉)라 제거 — 실 경로는 고정키 OMS_MSG_KEY 직접 사용. C89 정리.
------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <string.h>
#include    <errno.h>
#include    <sys/ipc.h>
#include    <sys/types.h>
#include    <sys/msg.h>
#include    "fep_sub.h"                 /* Log(), USR_OK/USR_ERROR */
#include    "alarm.h"

#define     OMS_MSG_KEY     0xff170990

/* OMS 알림 전문 헤더 (50B, wire 계약 — 원본 DATAHEAD 그대로) */
typedef struct {
    char    TrCode    [6];              /* TrCode                       */
    char    Scr_key   [4];              /* 화면키 (cid 대용)            */
    char    ErrCode   [4];              /* Error Code                   */
    char    ApType_Cd [5];              /* Aptype Code (Process 이름)   */
    char    Media_gbn [1];              /* 매체구분 (A/C/T)             */
    char    Cid       [10];             /* cid                          */
    char    Filler    [20];             /* 여유                         */
}   ALARM_DATAHEAD;

typedef struct {
    long    mtype;
    char    mtext[1024];
}   ALARM_MSG_BUF;

/*----------------------------------------------------------------------*/
int     alrt_msg(int trcode, char *msg, int msg_len)
/*----------------------------------------------------------------------*/
{
    ALARM_MSG_BUF   msgbuf;
    ALARM_DATAHEAD  hd;
    char            trbuf[7];
    key_t           qkey = OMS_MSG_KEY;
    int             qid, rc, sndlen;

    if (msg == NULL) {
        Log(USR_ERROR, "alrt_msg: null msg (trcode=%d)", trcode);
        return (-1);
    }
    if (msg_len > 400) msg_len = 400;
    if (msg_len < 0)   msg_len = 0;

    if ((qid = msgget(qkey, IPC_CREAT | 0666)) < 0) {
        Log(USR_ERROR, "alrt_msg: msgget fail key=0x%x [%d:%s]",
                (unsigned)qkey, errno, strerror(errno));
        return (-1);
    }

    memset(&hd, 0x20, sizeof(hd));
    sprintf(trbuf, "%06d", trcode); trbuf[6] = 0x00;
    memcpy(hd.TrCode, trbuf, 6);

    msgbuf.mtype = 100L;
    memcpy(msgbuf.mtext, &hd, sizeof(hd));
    memcpy(msgbuf.mtext + sizeof(hd), msg, msg_len);
    sndlen = (int)sizeof(hd) + msg_len;

    rc = msgsnd(qid, &msgbuf, sndlen, IPC_NOWAIT);
    if (rc != 0) {
        Log(USR_ERROR, "alrt_msg: msgsnd fail trcode=%d len=%d [%d:%s]",
                trcode, sndlen, errno, strerror(errno));
        return (-1);
    }

    Log(USR_OK, "alrt_msg: sent trcode=%d len=%d", trcode, msg_len);
    return (0);
}

/*************************************************************************
    End of Program (alarm.c)
*************************************************************************/
