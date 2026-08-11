#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : PB 내부 클라이언트 통신 (TCP 수신)
 *  File    : pb_8200_tr.c
 *  Author  : Park SH
 *
 *  내부 시스템(Noa)으로부터 TCP로 데이터를 수신하여
 *  FIFO로 분배하는 프로세스. PA의 pa_8200_tr.c와 유사하지만
 *  채권(PB) 전용으로 분배 규칙이 다르다.
 *
 *  빌드 옵션:
 *    B8211 : DATA_SIZE=600 (특정 프로세스용)
 *    기타  : DATA_SIZE=2048 (기본)
 *
 *  프로토콜 (CLI_FORMAT 기반):
 *    - LINK: 로그인 → LIOK 응답
 *    - DATA: 데이터 → DAOK 응답 → Write_Data()
 *    - POLL: Heartbeat → POOK 응답
 *
 *  데이터 분배 규칙 (Write_Data):
 *    700xxx, 900xxx → F_W(TS_W1_1) : pb_1801_dd
 *    KMAxxx         → F_W(TS_W1_1) : KRX 헤더 제거 후 전달
 *    기타           → 에러 로그
 *
 *  PA 대비 차이점:
 *    - 주문(100xxx) 분배 없음
 *    - 전략(500xxx) 분배 없음
 *    - KMA 전문(KRX 헤더 제거) 처리 추가
 *    - detect_poll_event() 사용 (PA는 수동 for 루프)
 *    - 종료 시 Device_Close() 호출
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"

#define     MAX_CNT     1           /* FIFO Write 1건씩 처리 */

/*------------------------------------------------------------------------
 *  빌드 옵션별 데이터 크기
 *------------------------------------------------------------------------*/
#if defined B8211
#define     DATA_SIZE   600         /* B8211 전용 크기 */
#else
#define     DATA_SIZE   2048        /* 기본 크기 */
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/
#define     DEVICE_TIME     3 * 1000        /* 연결 대기 타임아웃: 3초 */
#define     DATA_TIME       (30+5) * 1000   /* Heartbeat 타임아웃: 35초 */
#define     FOREVER_TIME    60 * 1000       /* 무한대기 타임아웃: 60초 */

#define     SOCKET_EVENT    0               /* Poll 이벤트: 소켓 */

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int     Sockfd, Newfd;                  /* 리슨소켓, 접속된 클라이언트 소켓 */
int     PollCnt, FirstSeq, TimeOut, PortNo;
char    ApType[10];                     /* 프로세스 유형 (예: "PB8200TR") */
char    RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char    DeviceSendFlag, LogOnFlag, OpenFlag;    /* 상태 플래그 */

FILE_BUFF_FORMAT    W_Fmt[MAX_CNT];     /* FIFO Write 버퍼 */

CLI_HEAD    *S_Pkt = (CLI_HEAD *)SendPkt;       /* 응답 데이터 헤더 */
CLI_FORMAT  *R_Pkt = (CLI_FORMAT *)RecvPkt;     /* 수신 데이터 전체 */

struct pollfd   Poll[1];                /* poll 이벤트 배열 */

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
void    PB_8200_TR(void);
void    Init_Parameters(void);
void    Fifo_Event_Rtn(void);
void    Socket_Event_Rtn(void);
void    Device_Write(void);
void    Device_Open(void);
void    Device_Close(void);
int     Device_Read(void);
void    Time_Out_Rtn(void);
int     Make_Send_Msg(int);
void    Write_Data(void);

/*------------------------------------------------------------------------
 *  main: 프로세스 초기화 → 클라이언트 통신 루프 → 종료
 *------------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
    PB_8200_TR();
    Exit_Process();
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PB_8200_TR: 메인 이벤트 루프
 *
 *  처리 흐름:
 *    1) 주말 체크 (HOLIDAY_APPLY)
 *    2) 네트워크 상태 체크 (TCP2_NET_STA)
 *    3) 서버 소켓 오픈 → accept 대기
 *    4) poll()로 이벤트 대기
 *    5) detect_poll_event()로 이벤트 분류
 *    6) SOCKET_EVENT → Socket_Event_Rtn()
 *    7) 루프 종료 시 Device_Close() 호출
 *------------------------------------------------------------------------*/
void    PB_8200_TR(void) {
    int     rt, i;
    int     size = 1000000;

    Init_Parameters();

    /*------------------------------------------------------------
     *  HOLIDAY_APPLY: 주말 체크
     *  주의: PA는 토/일 sleep이지만, PB는 break만 수행
     *------------------------------------------------------------*/
#if defined HOLIDAY_APPLY
    char    t_time[12], dt[20];
    time_t  t = time(NULL);
    struct  tm  tm, *tp;

    while (1) {
        Get_Time(t_time);

        memset(dt, 0, sizeof (dt));
        sprintf(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
                DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime(dt, "%Y-%m-%d %H:%M:%S", &tm);
        tp = localtime(&t);

        break;  /* PB: 주말 체크 없이 바로 기동 */
    }
#endif

    /*------------------------------------------------------------
     *  메인 이벤트 루프
     *------------------------------------------------------------*/
    while (START_S != END) {
        Stat_Save();

        /* 네트워크 상태 체크 */
        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP) {
            /* 종료/중지 상태 */
            if (LogOnFlag == ON && TCP2_NET_STA(0) == END) {
                Device_Close();
            }
        }
        else    /* 정상 주문시간 */ {
            if (OpenFlag == OFF) {
                /* 서버 소켓 오픈 → accept 대기 */
                Device_Open();

                TCP2_NET_STA(0) = ON;

                Log(USR_OK, "accepting ...");
                Newfd = accept(Sockfd, (struct sockaddr*)NULL, NULL);
                if (Newfd < 0) {
                    if (SYS_NO == ECONNABORTED) {
                        usleep(100000);
                        continue;
                    }

                    Log(TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log(USR_OK, "accepted");

                Set_Socket_Linger(Sockfd);

                TCP2_LINE_ST = OpenFlag = ON;

                /* 클라이언트 소켓으로 poll 전환 */
                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;
                TimeOut = FOREVER_TIME;

            }
            else {
                /* 이미 연결된 상태 */
                if (LogOnFlag == ON) {
                    PollCnt = 1;
                    TimeOut = DATA_TIME;        /* Heartbeat 타임아웃 */
                }
                else {
                    PollCnt = 1;
                    TimeOut = FOREVER_TIME;     /* 로그인 대기 */
                }
            }
        }

        /* poll 이벤트 대기 */
        rt = poll(Poll, PollCnt, TimeOut);
        if (rt < 0) {
            if (SYS_NO == EINTR)
                Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0) {
            Time_Out_Rtn();
            continue;
        }

        /*------------------------------------------------------------
         *  detect_poll_event: POLLHUP/POLLIN 처리
         *  PA와 달리 공통함수 사용 (fep_common.c)
         *  반환값: -1=에러(종료), -2=재시도, >=0=이벤트 인덱스
         *------------------------------------------------------------*/
        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;
        if (i == -2) continue;

        switch (i) {
            case SOCKET_EVENT:
                Socket_Event_Rtn();
                break;
            default:
                Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process();
                break;
        }
    }

    Device_Close();    /* PB: 루프 종료 시 명시적 close */

    return;
}   /* End of PB_8200_TR () */

/*------------------------------------------------------------------------
 *  Init_Parameters: 파라미터 초기화
 *------------------------------------------------------------------------*/
void    Init_Parameters(void) {
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;

    L_K = 0;

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    return;
}   /* End of Init_Parameters ()    */

/*------------------------------------------------------------------------
 *  Socket_Event_Rtn: TCP 데이터 수신 및 응답 처리
 *  PA 버전과 동일한 프로토콜: LINK/DATA/POLL
 *------------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    int     rval, rt, cnt, i;
    char    m_time[24], Chg_Data[KRX_DATA_BUFF_SIZE];

    rval = Device_Read();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;

    if (LogOnFlag == ON) {
        if (memcmp(R_Pkt->Head.MsgType, "DATA", 4) == 0) {
            /* DATA: 데이터 수신 */
            if (memcmp(R_Pkt->Head.ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "DATA Response is NotOk [%32.32s]", R_Pkt);
                Exit_Process();
            }

            rt = Make_Send_Msg(RP_DATA);
            Device_Write();

            if (!rt) {
                Write_Data();      /* FIFO에 분배 */
                INT_SEQ ++;
            }
            Log(USR_OK, "OK01");
        }
        else
            if (memcmp(R_Pkt->Head.MsgType, "POLL", 4) == 0) {
            /* POLL: Heartbeat */
            if (memcmp(R_Pkt->Head.ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "POLL Response is NotOk [%4.4s]", R_Pkt->Head.ResponsCode);
                Exit_Process();
            }

            rt = Make_Send_Msg(RP_POLL);
            Device_Write();
        }
        else {
            Log(USR_ERROR, "MsgType Not Defined Code [%32.32s]", R_Pkt);
            Exit_Process();
        }
    }
    else    /* 로그인 전: LINK 처리 */ {
        if (memcmp(R_Pkt->Head.MsgType, "LINK", 4) != 0) {
            Log(USR_ERROR, "LOGON(LINK) error, check plese Recv Data[%32.32s]", R_Pkt);
            Exit_Process();
        }

        /* LIOK 응답 전송 */
        Make_Send_Msg(RP_LINK);
        Device_Write();

        LogOnFlag = ON;
    }

    Log(USR_OK, "OK02");
    return;
}   /* End of Socket_Event_Rtn ()   */

/*------------------------------------------------------------------------
 *  Device_Open: TCP 서버 소켓 생성
 *  - Socket → setsockopt(SO_REUSEADDR) → Bind → Listen
 *------------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt;
    const int on = 1;

    sleep(5);

    Sockfd = Socket();
    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);

    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0) {
        Log(TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "setsockopt[%d]", Sockfd);

    rt = Bind(Sockfd, PortNo);
    if (rt < 0) {
        Log(TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PortNo, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "bind[%d]", Sockfd);

    Listen(Sockfd);
    Log(USR_OK, "listen[%d]", Sockfd);

    Poll[0].fd = Sockfd;
    Poll[0].events = POLLIN;

    /*  TCP2_LINE_ST = OpenFlag = ON;   */  /* 미사용: accept 후 설정 */

    return;
}   /* End of Device_Open ()    */

/*------------------------------------------------------------------------
 *  Device_Close: TCP 연결 종료
 *------------------------------------------------------------------------*/
void    Device_Close(void) {
    close(Sockfd);
    close(Newfd);
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
}   /* End of Device_Close ()   */

/*------------------------------------------------------------------------
 *  Device_Read: TCP 데이터 수신
 *------------------------------------------------------------------------*/
int     Device_Read(void) {
    int     rt;
    char    m_time[24];

    memset(RecvPkt, 0, sizeof (RecvPkt));

    rt = Select_Receive_Cli(Newfd, RecvPkt);

    if (rt <= 0 || rt > KRX_DATA_BUFF_SIZE) {
        Log(TCP_ERROR, "TCP RD Worng Length[%s] (%d)<%d>", RecvPkt, rt, INT_SEQ);
        Device_Close();
        return (NOTOK);
    }

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    return (OK);
}   /* End of Device_Read ()    */

/*------------------------------------------------------------------------
 *  Device_Write: TCP 데이터 전송
 *------------------------------------------------------------------------*/
void    Device_Write(void) {
    int     rt;

    rt = Select_Send(Newfd, SendPkt, strlen(SendPkt));

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* End of Device_Write ()   */

/*------------------------------------------------------------------------
 *  Time_Out_Rtn: 타임아웃 처리
 *------------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    Log(USR_OK, "TimeOut");

    return;
}   /* End of Time_Out_Rtn ()   */

/*------------------------------------------------------------------------
 *  Make_Send_Msg: 응답 메시지 생성
 *  PA 버전과 동일: DAOK / LIOK / POOK
 *------------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code) {
    int     rt = 0;
    int     r_cnt;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset(SendPkt, 0, sizeof (SendPkt));
    memset(SendPkt, 0x20, CLI_HEAD_LEN);   /* 헤더 50바이트 공백 초기화 */

    Get_DateTime(d_time);

    /* 1. Packet Length */
    ItoAf(CLI_HEAD_LEN - sizeof (S_Pkt->Length),
            S_Pkt->Length, sizeof (S_Pkt->Length));
    /* 3. ResponsCode */
    memcpy(S_Pkt->ResponsCode, "0000", sizeof (S_Pkt->ResponsCode));
    /* 4. TradeDate */
    memcpy(S_Pkt->TradeDate, d_time, sizeof (S_Pkt->TradeDate));

    switch (tr_code) {
        case RP_DATA:
            memcpy(S_Pkt->MsgType, "DAOK", sizeof (S_Pkt->MsgType));
            ItoAf(INT_SEQ + 1, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        case RP_LINK:
            memcpy(S_Pkt->MsgType, "LIOK", sizeof (S_Pkt->MsgType));
            ItoAf(INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        case RP_POLL:
            memcpy(S_Pkt->MsgType, "POOK", sizeof (S_Pkt->MsgType));
            ItoAf(INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*------------------------------------------------------------------------
 *  Write_Data: 수신 데이터를 FIFO로 분배
 *
 *  PA 대비 차이점:
 *    - 주문(100xxx)/전략(500xxx) 분배 없음
 *    - KMA 전문: KRX 헤더(82바이트) 제거 후 전달
 *    - 700/900/KMA → F_W(TS_W1_1) : pb_1801_dd
 *------------------------------------------------------------------------*/
void    Write_Data(void) {
    int     rt = 0, cnt, i, target;
    char    m_time[24];

    memset(W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset(m_time, 0, sizeof (m_time));

    Get_MicroTime(m_time);

    /* FIFO 공통 헤더 설정 */
    ItoAf(INT_SEQ+1, W_Fmt[0].If_Seq, sizeof (W_Fmt[0].If_Seq));
    memcpy(W_Fmt[0].ApType, ApType, sizeof (W_Fmt[0].ApType));
    memcpy(W_Fmt[0].ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    memcpy(W_Fmt[0].RecvTime1, m_time, sizeof (W_Fmt[0].RecvTime1));
    memcpy(W_Fmt[0].RecvTime2, m_time+10, sizeof (W_Fmt[0].RecvTime2));
    ItoAf(strlen(R_Pkt->Data), W_Fmt[0].DataHeader, 4);

    /*------------------------------------------------------------
     *  데이터 분배: 700/900/KMA → pb_1801_dd
     *------------------------------------------------------------*/
    if ((memcmp(R_Pkt->Data, "700", 3) == 0) ||
            (memcmp(R_Pkt->Data, "900", 3) == 0) ||
            (memcmp(R_Pkt->Data, "KMA", 3) == 0) ) {
        if (memcmp(R_Pkt->Data, "KMA", 3) == 0) {
            /* KMA 전문: KRX 헤더(82바이트) 제거 후 데이터만 복사 */
            memcpy(W_Fmt[0].Data, R_Pkt->Data+KRX_HEAD_LEN, strlen(R_Pkt->Data) - KRX_HEAD_LEN);
            W_Fmt[0].LineFeed[0] = '\n';
        }
        else {
            /* 700/900: 데이터 전체 복사 */
            memcpy(W_Fmt[0].Data, R_Pkt->Data, strlen(R_Pkt->Data));
            W_Fmt[0].LineFeed[0] = '\n';
        }

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);      /* pb_1801_dd */
    }
    else {
        Log(SAM_FATAL, "Else Case Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
        return;
    }

    if (rt != 1) {
        Log(SAM_FATAL, "file write [%3.3s]", R_Pkt->Data);
        Exit_Process();
    }

    Log(USR_OK, "file write[%3.3s] rt[%d]", R_Pkt->Data, rt);
    return;

}   /* End of Write_Data () */

/*************************************************************************
 *  End of Program (pb_8200_tr.c)
 *************************************************************************/
