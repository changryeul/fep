#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 내부 클라이언트 통신 (TCP 수신)
 *  File    : pa_8200_tr.c
 *  Author  : Park SH
 *
 *  내부 시스템(Noa)으로부터 TCP로 데이터를 수신하여
 *  용도별로 FIFO/DSHM에 분배하는 프로세스.
 *  서버 모드로 동작하며, 클라이언트의 접속을 accept하여 처리한다.
 *
 *  프로토콜 (CLI_FORMAT 기반):
 *    - LINK: 로그인 요청 → LIOK 응답
 *    - DATA: 데이터 전송 → DAOK 응답 → Write_Data()로 분배
 *    - POLL: Heartbeat → POOK 응답
 *
 *  데이터 분배 규칙 (Write_Data):
 *    700xxx, 900xxx → TS_W1_1 (pa_6001_mp 조회처리)
 *    500100         → TS_W2_1 (pa_9001_mp 전략기동)
 *    500xxx(기타)   → DSHM_W  (자동매매 프로세스)
 *    100xxx         → DSHM_W  (주문: 유가증권/코스닥/파생 구분)
 *
 *  CLI 헤더 구조 (CLI_HEAD, 50바이트):
 *    Length(4) + MsgType(4) + ResponsCode(4) + TradeDate(8) + SeqNo(8) + ...
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"
#include    "fep_common.h"

#define     MAX_CNT     1           /* FIFO Write 1건씩 처리 */

#define     DATA_SIZE   2048        /* 수신 데이터 최대 크기 */
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
char    ApType[10];                     /* 프로세스 유형 (예: "PA8200TR") */
char    RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char    DeviceSendFlag, LogOnFlag, OpenFlag;    /* 상태 플래그 */

FILE_BUFF_FORMAT    W_Fmt[MAX_CNT];     /* FIFO Write 버퍼 */

CLI_HEAD    *S_Pkt = (CLI_HEAD *)SendPkt;       /* 응답 데이터 헤더 */
CLI_FORMAT  *R_Pkt = (CLI_FORMAT *)RecvPkt;     /* 수신 데이터 전체 */

struct pollfd   Poll[1];                /* poll 이벤트 배열 (소켓 1개) */

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
void    PA_8200_TR(void);
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
    PA_8200_TR();
    Exit_Process();
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PA_8200_TR: 메인 이벤트 루프
 *
 *  처리 흐름:
 *    1) 주말 체크 (HOLIDAY_APPLY: 토/일이면 sleep)
 *    2) 네트워크 상태 체크 (TCP2_NET_STA)
 *    3) 서버 소켓 오픈 → accept 대기
 *    4) poll()로 이벤트 대기
 *    5) POLLIN → Socket_Event_Rtn() (데이터 수신 처리)
 *    6) 타임아웃 → Time_Out_Rtn()
 *------------------------------------------------------------------------*/
void    PA_8200_TR(void) {
    int     rt, i;
    int     size = 1000000;

    Init_Parameters();

    /*------------------------------------------------------------
     *  HOLIDAY_APPLY: 주말(토/일) 체크 → sleep
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

        if (tp->tm_wday == 0 || tp->tm_wday == 6)       /* 일요일(0), 토요일(6) */ {
            Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            PROC(D_K,P_K).start_status = JOB_END;
            PROC(D_K,P_K).process_status = 9;
            sleep(60);
        }
        else {
            break;  /* 평일 → 정상 기동 */
        }
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
                        usleep(100000);    /* 100ms 대기 후 재시도 */
                        continue;
                    }

                    Log(TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log(USR_OK, "accepted");

                Set_Socket_Linger(Sockfd); /* SO_LINGER 설정 */

                TCP2_LINE_ST = OpenFlag = ON;

                /* 클라이언트 소켓으로 poll 전환 */
                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;
                TimeOut = FOREVER_TIME;

            }
            else {
                /* 이미 연결된 상태: 로그인 여부에 따라 타임아웃 설정 */
                if (LogOnFlag == ON) {
                    PollCnt = 1;
                    TimeOut = DATA_TIME;        /* Heartbeat 타임아웃 (35초) */
                }
                else {
                    PollCnt = 1;
                    TimeOut = FOREVER_TIME;     /* 로그인 대기 (60초) */
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

        /* poll 이벤트 단일 순회 — POLLHUP 체크 후 POLLIN 처리 */
        for (i = 0; i < PollCnt; i ++) {
            if (Poll[i].revents & POLLHUP) {
                if (i == SOCKET_EVENT) {
                    Log(TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
                    return;
                }

                Log(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
            }

            if (Poll[i].revents & POLLIN) {
                Poll[i].revents = 0;

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
        }
    }

    return;
}   /* End of PA_8200_TR () */

/*------------------------------------------------------------------------
 *  Init_Parameters: 파라미터 초기화
 *  - 상태 플래그 초기화 (OFF)
 *  - TCP2 포트 번호 설정
 *  - ApType 문자열 생성
 *------------------------------------------------------------------------*/
void    Init_Parameters(void) {
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;

    L_K = 0;

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;      /* 프로세스 상태: 기동 */
    TCP2_LINE_ST = OFF;     /* 회선 상태: 미연결 */

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    return;
}   /* End of Init_Parameters ()    */

/*------------------------------------------------------------------------
 *  Socket_Event_Rtn: TCP 데이터 수신 및 응답 처리
 *
 *  프로토콜 분기:
 *    로그인 전: LINK 수신 → LIOK 응답, LogOnFlag=ON
 *    로그인 후:
 *      DATA 수신 → DAOK 응답 → Write_Data()로 FIFO 분배
 *      POLL 수신 → POOK 응답 (Heartbeat)
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

            /* DAOK 응답 전송 */
            rt = Make_Send_Msg(RP_DATA);
            Device_Write();

            if (!rt) {
                Write_Data();      /* FIFO/DSHM에 분배 */
                INT_SEQ ++;
            }
        }
        else
            if (memcmp(R_Pkt->Head.MsgType, "POLL", 4) == 0) {
            /* POLL: Heartbeat */
            if (memcmp(R_Pkt->Head.ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "POLL Response is NotOk [%4.4s]", R_Pkt->Head.ResponsCode);
                Exit_Process();
            }

            /* POOK 응답 전송 */
            rt = Make_Send_Msg(RP_POLL);
            Device_Write();
        }
        else {
            /* 미정의 MsgType */
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

        LogOnFlag = ON;     /* 로그인 완료 */
    }

    return;
}   /* End of Socket_Event_Rtn ()   */

/*------------------------------------------------------------------------
 *  Device_Open: TCP 서버 소켓 생성 (Socket → Bind → Listen)
 *  - Socket() → setsockopt(SO_REUSEADDR) → Bind() → Listen()
 *  - Poll 배열에 리슨 소켓 등록
 *------------------------------------------------------------------------*/
void    Device_Open(void) {
    int     rt;
    const int on = 1;

    sleep(5);  /* 이전 소켓 정리 대기 */

    /* 소켓 생성 */
    Sockfd = Socket();
    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);

    /* SO_REUSEADDR: 포트 재사용 허용 */
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0) {
        Log(TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "setsockopt[%d]", Sockfd);

    /* Bind */
    rt = Bind(Sockfd, PortNo);
    if (rt < 0) {
        Log(TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PortNo, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "bind[%d]", Sockfd);

    /* Listen */
    Listen(Sockfd);
    Log(USR_OK, "listen[%d]", Sockfd);

    /* Poll 배열에 리슨 소켓 등록 */
    Poll[0].fd = Sockfd;
    Poll[0].events = POLLIN;

    return;
}   /* End of Device_Open ()    */

/*------------------------------------------------------------------------
 *  Device_Close: TCP 연결 종료
 *  - 리슨 소켓 + 클라이언트 소켓 모두 닫기
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
 *  - Select_Receive_Cli()로 CLI 프로토콜 수신
 *  - 수신 실패/크기 이상 시 Device_Close()
 *
 *  반환값: OK(0) 성공, NOTOK(-1) 실패
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
 *  - SendPkt 버퍼의 응답 데이터를 클라이언트에 전송
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
 *  - Heartbeat 타임아웃 시 로그만 기록
 *------------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    Log(USR_OK, "TimeOut");

    return;
}   /* End of Time_Out_Rtn ()   */

/*------------------------------------------------------------------------
 *  Make_Send_Msg: 응답 메시지 생성
 *
 *  CLI 헤더(50바이트)를 구성:
 *    Length: CLI_HEAD_LEN - 4 (자기 자신 크기 제외)
 *    MsgType: DAOK(DATA응답) / LIOK(LINK응답) / POOK(POLL응답)
 *    ResponsCode: "0000" (정상)
 *    TradeDate: 현재 일시
 *    SeqNo: 현재 시퀀스 번호
 *
 *  반환값: 0=정상
 *------------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code) {
    int     rt = 0;
    int     r_cnt;
    int     arry_cnt, arry_len, arry_dat;
    char    d_time[16];

    memset(SendPkt, 0, sizeof (SendPkt));
    memset(SendPkt, 0x20, CLI_HEAD_LEN);   /* 헤더 50바이트 공백 초기화 */

    Get_DateTime(d_time);

    /* 1. Packet Length (50-4=46) */
    ItoAf(CLI_HEAD_LEN - sizeof (S_Pkt->Length),
            S_Pkt->Length, sizeof (S_Pkt->Length));
    /* 3. ResponsCode */
    memcpy(S_Pkt->ResponsCode, "0000", sizeof (S_Pkt->ResponsCode));
    /* 4. TradeDate */
    memcpy(S_Pkt->TradeDate, d_time, sizeof (S_Pkt->TradeDate));

    switch (tr_code) {
        case RP_DATA:
            /* MsgType: DAOK (DATA 정상 응답) */
            memcpy(S_Pkt->MsgType, "DAOK", sizeof (S_Pkt->MsgType));
            /* SeqNo: 다음 시퀀스 */
            ItoAf(INT_SEQ + 1, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        case RP_LINK:
            /* MsgType: LIOK (LINK 정상 응답) */
            memcpy(S_Pkt->MsgType, "LIOK", sizeof (S_Pkt->MsgType));
            /* SeqNo: 현재 시퀀스 */
            ItoAf(INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        case RP_POLL:
            /* MsgType: POOK (POLL 정상 응답) */
            memcpy(S_Pkt->MsgType, "POOK", sizeof (S_Pkt->MsgType));
            /* SeqNo: 현재 시퀀스 */
            ItoAf(INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
            break;

        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg ()  */

/*------------------------------------------------------------------------
 *  Write_Data: 수신 데이터를 용도별로 FIFO/DSHM에 분배
 *
 *  분배 규칙 (Data 앞 3~6자리로 판단):
 *    700xxx, 900xxx → F_W(TS_W1_1) : pa_6001_mp (조회처리)
 *    500100         → F_W(TS_W2_1) : pa_9001_mp (전략 기동요청)
 *    500xxx(기타)   → DSHM_W       : 자동매매 프로세스
 *    100xxx         → DSHM_W       : 주문 (시장구분별)
 *      +35="05"/"07" → TS_W1_1 (유가증권/ELW/ETF/ETN)
 *      +35="05"     → TS_W2_1 (코스닥) [주의: 위와 중복 조건]
 *      기타         → TS_W3_1 (파생)
 *------------------------------------------------------------------------*/
void    Write_Data(void) {
    int     rt, cnt, i, target;
    char    m_time[24];

    memset(W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset(m_time, 0, sizeof (m_time));

    Get_MicroTime(m_time);

    /* FIFO 공통 헤더 설정 */
    /* 1. Seq: F_W 함수에서 자동 설정 */
    /* 2. If_Seq */
    ItoAf(INT_SEQ+1, W_Fmt[0].If_Seq, sizeof (W_Fmt[0].If_Seq));
    /* 3. ApType */
    memcpy(W_Fmt[0].ApType, ApType, sizeof (W_Fmt[0].ApType));
    /* 4. ResponseCode */
    memcpy(W_Fmt[0].ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
    /* 5. RecvTime1 (수신시각) */
    memcpy(W_Fmt[0].RecvTime1, m_time, sizeof (W_Fmt[0].RecvTime1));
    /* 6. RecvTime2 (마이크로초) */
    memcpy(W_Fmt[0].RecvTime2, m_time+10, sizeof (W_Fmt[0].RecvTime2));
    /* 7. DataHeader (데이터 길이) */
    ItoAf(strlen(R_Pkt->Data), W_Fmt[0].DataHeader, 4);

    /*------------------------------------------------------------
     *  데이터 분배: 앞 3자리로 구분
     *------------------------------------------------------------*/
    if ((memcmp(R_Pkt->Data, "700", 3) == 0) ||
            (memcmp(R_Pkt->Data, "900", 3) == 0)) {
        /* 700xxx/900xxx: 조회 처리 → pa_6001_mp */
        memcpy(W_Fmt[0].Data, R_Pkt->Data, strlen(R_Pkt->Data));
        W_Fmt[0].LineFeed[0] = '\n';

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);      /* pa_6001_mp */
    }
    else if (memcmp(R_Pkt->Data, "500", 3) == 0) {
        /* 500xxx: 전략 관련 */
        memcpy(W_Fmt[0].Data, R_Pkt->Data, strlen(R_Pkt->Data));
        W_Fmt[0].LineFeed[0] = '\n';

        if (memcmp(R_Pkt->Data, "500100", 6) == 0) /* 전략 기동요청 */
            rt = F_W(TS_W2_1, (void *)&W_Fmt, 1);      /* pa_9001_mp */
        else {
            /* 자동매매 프로세스로 분배 (DSHM) */
            SEARCH_HEADER   *Header = (SEARCH_HEADER *)&R_Pkt->Data;
            target = (AtoIf(&Header->ApType_Cd[1], 2) - 1) * 10;
            target = AtoIf(&Header->ApType_Cd[3], 2) + 3 + target;
            rt = DSHM_W(target*10, (void *)&W_Fmt, 1); /* 자동Process */
        }
    }
    else if (memcmp(R_Pkt->Data, "100", 3) == 0)       /* 100100 주문 */ {
        /* 주문: SEARCH_HEADER 제외하고 데이터만 복사 */
        memcpy(W_Fmt[0].Data, &R_Pkt->Data[SEARCH_HEADER_LEN], strlen(R_Pkt->Data)-SEARCH_HEADER_LEN);
        W_Fmt[0].LineFeed[0] = '\n';

        /* 시장구분(+35)에 따라 분배 대상 결정 */
        KRX_JUMUN_DATA  *dat = (KRX_JUMUN_DATA *)R_Pkt->Data;
        if ((memcmp(dat->MembershipItem+35, "05", 2) == 0) ||
                (memcmp(dat->MembershipItem+35, "07", 2) == 0) ) {
            rt = DSHM_W(TS_W1_1, (void *)&W_Fmt, 1);   /* 유가증권, ELW/ETF/ETN */
        }
        else
            if (memcmp(dat->MembershipItem+35, "05", 2) == 0) {
            rt = DSHM_W(TS_W2_1, (void *)&W_Fmt, 1);   /* 코스닥 */
        }
        else
            rt = DSHM_W(TS_W3_1, (void *)&W_Fmt, 1);   /* 파생 */
    }
    else {
        Log(SAM_FATAL, "Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
    }

    if (rt != 1) {
        Log(SAM_FATAL, "file write [%3.3s]", R_Pkt->Data);
        Exit_Process();
    }

    Log(USR_OK, "file write[%3.3s] rt[%d]", R_Pkt->Data, rt);
    return;

}   /* End of Write_Data () */

/*************************************************************************
 *  End of Program (pa_8200_tr.c)
 *************************************************************************/
