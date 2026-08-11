#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : Q 수신 (주문등)
#   File    : pa_5000_qr.c
#
#   설명  : System V Message Queue에서 주문 데이터를 수신하여
#             FIFO 또는 DSHM에 기록하는 프로세스.
#             송신 프로세스(pa_1101_ts 등)가 미기동이면 "REJC"
#             거부 처리하여 Client에 전달한다.
#             A5001=채권주문수신, A5011=금융파생주문수신(IMECO).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 *   A5001: 채권주문 → 400바이트
 *   기타 : 파생(IMECO) → 200바이트
 */
#if defined A5001
#define     DATA_SIZE       400
#else
#define     DATA_SIZE       200
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */

#define     READ_BUF_SIZE   1024                /* 읽기 버퍼 크기 */
#define     QUEUE_MAX_BYTES 10485760            /* 큐 최대 바이트 (10MB) */

/*
 * CHK_PROC: 빌드 옵션별 송신 프로세스명 (기동 여부 확인용)
 *   A5001: 채권 주문수신 → pa_1101_ts 기동 확인
 *   A5011: 파생 주문수신 → pa_2101_ts 기동 확인
 */
#if defined A5001
#define     CHK_PROC        "pa_1101_ts"
#elif defined A5011
#define     CHK_PROC        "pa_2101_ts"
#endif

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int                 FIFO_fd, FIFO_fd1, Pk;      /* FIFO FD, 송신 프로세스 키 */
char                TrCode[8], ApType[10];      /* TR코드, 어플리케이션 타입 */
struct              ip_mreq     mreq;           /* 멀티캐스트 요청 구조체 */
struct sockaddr_in  SvrAddr, ClntAddr;          /* 서버/클라이언트 주소 */

FILE_BUFF_FORMAT    W_Fmt[1];                   /* FIFO/DSHM 쓰기용 버퍼 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_5000_QR(void);
void    Init_Parameters(void);
int     QueueClear(int);
int     GetQid(size_t);
int     CheckQueue(int);
int     MakeQueue(key_t);
int     ReceiveQueue(int, long, char *);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 큐 수신 루프 실행.                   */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */
    PA_5000_QR();                              /* 메인 처리 루프 */
    Exit_Process();                            /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_5000_QR: 메인 처리 루프                                            */
/*    1) Message Queue 생성 (빌드 옵션별 키 사용)                     */
/*    2) 큐에서 주문 데이터 수신 (블로킹 대기)                         */
/*    3) 송신 프로세스 기동 여부 확인                                   */
/*    4) 기동중이면 DSHM에, 미기동이면 REJC 거부로 FIFO에 기록           */
/*----------------------------------------------------------------------*/
void    PA_5000_QR(void) {
    int     Chk_G1;
    int     i, rt, w_flag;
    int     msqid;                              /* 메시지큐 ID */
    char    fifo_name[100];
    char    m_time[24], key[12];
    char    u_time[24], tms_time[6], deci_time[6];
    int     tms, deci, st_time, st_micro_time;
    key_t   new_key, base_key;                  /* 메시지큐 키 */

    Init_Parameters();                         /* 송신 프로세스 키(Pk) 조회 */

    char    r_buf[1024];                        /* 큐 수신 버퍼 */

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s",
            _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /*
     * 빌드 옵션별 Message Queue 키 설정
     * TEST 환경이면 0x01000000을 더해 REAL과 키 충돌 방지
     */
#if defined A5001                               /* 채권 주문수신 */
    new_key = 0x22000001;
#elif defined A5011                             /* 금융파생 주문수신 */
    new_key = 0x22000011;
#endif
    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        new_key += 0x01000000L;                 /* TEST 환경 키 오프셋 */

    Chk_G1 = 1;

    /* Message Queue 생성 */
    msqid = MakeQueue(new_key);
    if (msqid < 0) {
        Log(USR_ERROR, "msgget msqid[%d]", msqid);
        Exit_Process();                        /* 큐 생성 실패 → 종료 */
    }
    else
        Log(USR_OK, "msgget: succeeded: msqid = %d new_key [%#x]", msqid, new_key);

    /* === 메인 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        memset(r_buf, 0, sizeof (r_buf));

        /* Message Queue에서 주문 데이터 수신 (블로킹) */
        rt = ReceiveQueue(msqid, 0L, r_buf);
        if (rt < 0) {
            Log(USR_ERROR, "receive fail {%d:%s} rt[%d]", SYS_NO, SYS_STR, rt);
            Exit_Process();
        }

        /*
         * 수신 데이터 처리:
         *   송신 포맷 = "55바이트 헤더 + 주문전문" (동일 포맷으로 기록)
         */
        if (1) {
            /* 쓰기 버퍼 초기화 및 헤더 설정 */
            memset(m_time, 0, sizeof (m_time));
            Get_MicroTime(m_time);

            memset(W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

            /* 버퍼 헤더에 시퀀스, 타입, 응답코드, 수신시간 기록 */
            ItoAf(INT_SEQ + 1, W_Fmt[0].If_Seq, sizeof (W_Fmt[0].If_Seq));
            memcpy(W_Fmt[0].ApType, ApType, sizeof (W_Fmt[0].ApType));
            memcpy(W_Fmt[0].ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
            memcpy(W_Fmt[0].RecvTime1, m_time, sizeof (W_Fmt[0].RecvTime1));
            memcpy(W_Fmt[0].RecvTime2, &m_time[sizeof(W_Fmt[0].RecvTime1)],
                    sizeof (W_Fmt[0].RecvTime2));

            memset(W_Fmt[0].DataHeader, 0x20, sizeof(W_Fmt[0].Data));

            /*
             * 송신 프로세스 기동 여부 확인
             *   TCP2_PSTAT: 0=OFF, 1=ON, 2=END
             *   기동중(1)이면 정상 처리, 미기동이면 거부(REJC) 처리
             */
            if (TCP2_PSTAT(D_K,Pk,S_K) == 1)
                w_flag = 0;                     /* 정상 처리 */
            else
                w_flag = 1;                     /* 거부(REJC) 처리 */

#if defined A5001                               /* 채권 주문수신 */
            if (w_flag) {
                /*
                 * [거부처리] "REJC" + 주문전문을 Client에 전달
                 *   채권일반(TCHODR4000)=254바이트
                 *   채권LP(TCHMOR4000)=255바이트
                 *   Kill Switch(TCHKOR10001)=147바이트
                 */
                memcpy(W_Fmt[0].Data, "REJC00000000000", 15);

                if (memcmp(&r_buf[11], "TCHODR4000", 10) == 0)
                    memcpy(&W_Fmt[0].Data[15], r_buf, sizeof (KRX_NOTE_JUMUN_DATA));
                else if (memcmp(&r_buf[11], "TCHMOR4000", 10) == 0)
                    memcpy(&W_Fmt[0].Data[15], r_buf, sizeof (KRX_LP_NOTE_JUMUN_DATA));
                else if (memcmp(&r_buf[11], "TCHKOR10001", 11) == 0)
                    memcpy(&W_Fmt[0].Data[15], r_buf, 147);
                else {
                    Log(USR_ERROR, "REJC Other TR Recv [%s] size[%d]", r_buf, strlen(r_buf));
                    break;
                }
            }
            else {
                /*
                 * [정상처리] KRX 헤더 82바이트 = Space + 주문전문
                 *   TR코드로 전문 종류 판별 후 해당 크기만큼 복사
                 */
                if (memcmp(&r_buf[11], "TCHODR4000", 10) == 0)
                    memcpy(W_Fmt[0].Data, r_buf, sizeof (KRX_NOTE_JUMUN_DATA));
                else if (memcmp(&r_buf[11], "TCHMOR4000", 10) == 0)
                    memcpy(W_Fmt[0].Data, r_buf, sizeof (KRX_LP_NOTE_JUMUN_DATA));
                else if (memcmp(&r_buf[11], "TCHKOR10001", 11) == 0)
                    memcpy(W_Fmt[0].Data, r_buf, 147);
                else {
                    Log(USR_ERROR, "Other TR Recv [%s] size[%d]", r_buf, strlen(r_buf));
                    break;
                }
            }
#elif defined A5011                             /* 파생 주문수신 */
            if (w_flag) {
                /* [거부처리] IMECO 헤더(20) + "REJC" + 주문전문 */
                memcpy(W_Fmt[0].Data, "0112DREJC00000000001", 20);
                memcpy(&W_Fmt[0].Data[20], r_buf, sizeof (IMECO_JUMUN_DATA));
            }
            else {
                /* [정상처리] IMECO 주문전문만 복사 */
                memcpy(W_Fmt[0].Data, r_buf, sizeof (IMECO_JUMUN_DATA));
            }
#endif
            W_Fmt[0].Data[ODS(D_K,P_K,0)] = '\n';

            if (w_flag) {
                /* 거부(REJC): Client FIFO에 직접 기록 */
                rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
                if (rt < 0) {
                    Log(USR_OK, "REJC Write Error W_Fmt[%s][%d]", W_Fmt[0].Data, strlen(W_Fmt[0].Data));
                    Exit_Process();
                }
            }
            else {
                /* 정상: DSHM(공유메모리)에 기록 → 송신 프로세스가 처리 */
                rt = DSHM_W(TS_W1_1, (void *)&W_Fmt, 1);
                if (rt < 0) {
                    Log(USR_OK, "SHM Write Error W_Fmt[%s][%d]", W_Fmt[0].Data, strlen(W_Fmt[0].Data));
                    Log(DSH_FATAL, "DSHM write[%s]", ODN(D_K,P_K,1));
                    Exit_Process();
                }
            }

            Log(USR_OK, "Recv [%s] size[%d]", r_buf, strlen(r_buf));

            Set_TR_Time();                     /* TR 시간 갱신 */
            INT_SEQ ++;                         /* 내부 시퀀스 번호 증가 */
        }
        else {
            Log(USR_ERROR, "Recv [%s] size[%d]", r_buf, strlen(r_buf));
            Exit_Process();
        }
    }

    return;
}   /* End of PA_5000_QR () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음, 전역변수 Pk 설정)
    Return Code     : void
    Comment         : 송신 프로세스(CHK_PROC)의 프로세스 키(Pk)를
                      설정에서 검색한다. 미등록 시 종료.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    /* 전체 프로세스 목록에서 CHK_PROC 검색 */
    for (Pk = 0; Pk < DAEMON(D_K).p_count; Pk++) {
        if (memcmp(PROC(D_K,Pk).process_id, CHK_PROC, 10) == 0)
            break;                              /* 찾았음 */

        if (Pk == DAEMON(D_K).p_count - 1) {
            /* 마지막까지 못 찾음 → 미등록 프로세스 */
            Log(USR_FATAL, "unregistered process[%s]", CHK_PROC);
            Exit_Process();
        }
    }
}

/*************************************************************************
    End of program (pa_5000_qr.c)
*************************************************************************/
