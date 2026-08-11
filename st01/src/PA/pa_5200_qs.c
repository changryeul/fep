#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : Q주문응답 송신 (Message Queue → 외부)
#   File    : pa_5200_qs.c
#
#   설명  : FIFO에서 주문응답/체결 데이터를 읽어
#             System V Message Queue로 전달하는 프로세스.
#             A5201=채권 응답, A5211=파생 응답,
#             A5401=채권 체결, A5411=파생 체결 빌드 구분.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 *   A5201/A5401: 채권 응답/체결 → 400바이트
 *   A5211/A5411: 파생 응답/체결 → 200바이트
 */
#if defined(A5201) || defined(A5401)
#define     DATA_SIZE       400
#elif defined(A5211) || defined(A5411)
#define     DATA_SIZE       200
#endif
#include    "buf_struct.h"

/* 이벤트 타입 상수 */
#define     FIFO_EVENT      0                   /* FIFO 이벤트 */
#define     SOCKET_EVENT    1                   /* 소켓 이벤트 */
#define     FILE_EVENT      2                   /* 파일 이벤트 */

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     READ_MAX        1                   /* FIFO에서 한 번에 읽을 최대 건수 */

#define     READ_BUF_SIZE   1024                /* 읽기 버퍼 크기 */
#define     QUEUE_MAX_BYTES 10485760            /* 큐 최대 바이트 (10MB) */

/* Message Queue 관련 상수 */
#define     MAXSIZE         4096                /* 메시지 최대 크기 */
#define     PERM            0x1B6               /* 큐 권한 (0666: rw-rw-rw-) */
#define     QUEUE_READY         -1              /* 큐 준비 완료 */
#define     QUEUE_WAIT          -2              /* 큐 대기 중 */
#define     QUEUE_FULL          -3              /* 큐 가득 참 */
#define     QUEUE_NOT_EXIST     -4              /* 큐 존재하지 않음 */
#define     QUEUE_TIME_OUT      -5              /* 큐 타임아웃 */
#define     QUEUE_SEND_ERROR    -6              /* 큐 전송 오류 */
#define     TRUE            1
#define     FALSE           0

/* 매크로 단축명 */
#define     WR_CNT          W_CNT(0,1)          /* 쓰기 카운터 */
#define     RD_CNT          R_CNT(0,1)          /* 읽기 카운터 */
#define     IN_NAME         IFN(D_K,P_K,0)      /* 입력 파일명 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         FIFO_fd, FIFO_fd1;          /* FIFO 파일 디스크립터 */
char            TrCode[8], ApType[10];      /* TR코드, 어플리케이션 타입 */
struct          ip_mreq     mreq;           /* 멀티캐스트 요청 구조체 */
struct sockaddr_in  SvrAddr, ClntAddr;          /* 서버/클라이언트 주소 */

FILE_BUFF_FORMAT    R_Fmt[READ_MAX];            /* FIFO 읽기용 버퍼 (1건) */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_5200_QS(void);
int ReceiveQueue();
int QueueClear(int);
int GetQid(size_t);
int CheckQueue(int);
int MakeQueue(key_t);
int ReceiveQueue(int, long, char *);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 큐 송신 루프 실행.       */
/*----------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                 /* 프로세스 초기화 */
    PA_5200_QS();                      /* 메인 처리 루프 */
    Exit_Process();                    /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_5200_QS: 메인 처리 루프                    */
/*    1) Message Queue 생성 (빌드 옵션별 키 사용)         */
/*    2) FIFO에서 주문응답/체결 데이터 읽기              */
/*    3) Message Queue로 데이터 전달              */
/*----------------------------------------------------------------------*/
void    PA_5200_QS(void) {
    int i, rt;
    int msqid, R_Cnt;                   /* 메시지큐 ID, 읽기 건수 */
    char    fifo_name[100];
    char    m_time[24], key[12];
    size_t  buf_length;                 /* 메시지 버퍼 길이 */
    key_t   new_key, base_key;              /* 메시지큐 키 */

    Msgbuf  sbuf;                       /* 메시지큐 송신 버퍼 */

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s",
            _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /*
     * 빌드 옵션별 Message Queue 키 설정
     * TEST 환경이면 0x01000000을 더해 REAL과 키 충돌 방지
     */
#if defined A5201                               /* 채권 응답송신 */
    new_key = 0x22000002;
#elif defined A5211                             /* 파생 응답송신 */
    new_key = 0x22000012;
#elif defined A5401                             /* 채권 체결송신 */
    new_key = 0x22000004;
#elif defined A5411                             /* 파생 체결송신 */
    new_key = 0x22000014;
#endif
    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        new_key += 0x01000000L;                 /* TEST 환경 키 오프셋 */

    /* Message Queue 생성 */
    msqid = MakeQueue(new_key);
    if (msqid < 0) {
        Log(USR_ERROR, "msgget msqid[%d]\n", msqid);
        Exit_Process();                        /* 큐 생성 실패 → 종료 */
    }
    else
        Log(USR_OK, "msgget: succeeded: msqid = %d new_key [%#x]\n", msqid, new_key);

    sbuf.mtype = 1;                             /* 메시지 타입: 1 (기본) */

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);
            R_Cnt = F_R(PS_R_3, (void *)R_Fmt, READ_MAX);
            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;                          /* 읽을 데이터 없음 → Poll 대기 */

            Log(USR_OK, "RD [%s:%d][%d]", IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,2,0));

            /*
             * 주문응답 전문 구조:
             *   주문응답: 헤더(11+4) + 주문전문(오류) 또는 회원처리호가(확인/거부/자동취소)
             *   체결    : 체결전문만
             *   접수거부(Client): 15(REJC) + 주문전문
             */

            /* Message Queue에 데이터 전송 */
            memset(sbuf.mtext, 0, sizeof (sbuf.mtext));
            memcpy(sbuf.mtext, R_Fmt[0].Data, strlen(R_Fmt[0].Data));

            buf_length = strlen(sbuf.mtext) + 1;
            rt = msgsnd(msqid, &sbuf, buf_length, 0);
            if (rt < 0) {
                /* 큐 전송 실패: 오류 로그 후 종료 */
                Log(USR_ERROR, "ERROR %d, %d, %s, %d", msqid, sbuf.mtype, sbuf.mtext, strlen(sbuf.mtext));
                Exit_Process();
            }
            Log(USR_OK, "Q rt[%d] SD[%s][%d]", rt, R_Fmt[0].Data, strlen(R_Fmt[0].Data));
            Add_Count(PS_R_3, 1);              /* 처리 건수 카운터 증가 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1) {
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        }
        else if (rt == -1)
            continue;                           /* 오류 → 다음 루프 */
    }

    return;
}   /* End of PA_5200_QS () */

/*************************************************************************
    End of program (pa_5200_qs.c)
*************************************************************************/
