#define     _GLOBAL
/*========================================================================
 *  Module  : PB 내부 클라이언트(Noa) 데이터 송신 (TCP 서버)
 *  File    : pb_8100_ts.c
 *  Author  : PSH
 *  -----------------------------------------------------------------
 *  채권 모듈(PB) 전용 클라이언트 데이터 송신 프로세스.
 *  pa_8100_ts.c의 PB 버전으로, 구조는 동일하되 다음이 다름:
 *    - DATA_SIZE가 빌드 옵션별로 다름 (200/400/450/1300/2048)
 *    - 매체구분 필터 없음 (PA는 A/C/T 구분)
 *    - poll 이벤트 처리를 수동 for 루프로 수행 (PA는 detect_poll_event)
 *    - HOLIDAY_APPLY에서 break만 수행 (주말 체크 비활성)
 *
 *  빌드 옵션 및 DATA_SIZE:
 *    B8101 : 채권 1번채널  — DATA_SIZE=2048, PS_R_1 사용
 *    B8105 : 채권 2번채널  — DATA_SIZE=200,  PS_R_2 사용
 *    B8111 : 채권 추가1    — DATA_SIZE=400,  PS_R_1 사용
 *    B8116 : 채권 추가2    — DATA_SIZE=2048, PS_R_1 사용
 *    B8117 : 채권 추가3    — DATA_SIZE=1300, PS_R_2 사용
 *    B8118 : 채권 추가4    — DATA_SIZE=450,  PS_R_2 사용
 *
 *  CLI 프로토콜 (pa_8100_ts.c와 동일):
 *    클라이언트→서버: LINK(로그인), DAOK(데이터확인), POOK(POLL확인)
 *    서버→클라이언트: LIOK(로그인응답), DATA(데이터), POLL(상태확인)
 *========================================================================*/

/*------------------------------------------------------------------------
 *  헤더 파일
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"       /* CLI 프로토콜 상수 및 구조체 */
#include    "lat_trace.h"

#define     MAX_CNT         1       /* FIFO에서 한번에 읽는 최대 건수 */

/*
 * 빌드 옵션별 1건당 데이터 크기
 * B8105=200, B8111=400, B8117=1300, B8118=450, 기타=2048
 */
#if defined B8105
#define     DATA_SIZE       200
#elif defined B8111
#define     DATA_SIZE       400
#elif defined B8116
#define     DATA_SIZE       2048
#elif defined B8117
#define     DATA_SIZE       1300
#elif defined B8118
#define     DATA_SIZE       450
#else
#define     DATA_SIZE       2048
#endif

#include    "buf_struct.h"          /* FILE_BUFF_FORMAT 구조체 (DATA_SIZE 의존) */

/*------------------------------------------------------------------------
 *  상수 정의
 *------------------------------------------------------------------------*/
#define     DEVICE_TIME     3 * 1000            /*  3초: 소켓 I/O 타임아웃 */
#define     DATA_TIME       (30-5) * 1000       /* 25초: 데이터 없을때 POLL 간격 */
#define     FOREVER_TIME    60 * 1000           /* 60초: 접속 대기/로그온 대기 */

#define     SOCKET_EVENT    0       /* poll 배열 인덱스: 소켓(클라이언트) */
#define     DATA_EVENT      1       /* poll 배열 인덱스: FIFO(데이터) */

/*------------------------------------------------------------------------
 *  전역 변수
 *------------------------------------------------------------------------*/
int     Sockfd;                 /* 리스닝 소켓 파일디스크립터 */
int     Newfd;                  /* Accept된 클라이언트 소켓 */
int     SendLen;                /* 송신 데이터 길이 */
int     MsgLen;                 /* 메시지 길이 */
int     ReTrCode;               /* 재전송 코드 */
int     PollCnt;                /* poll 감시 대상 수 (1 또는 2) */
int     FirstSeq;               /* 최초 시퀀스 */
int     TimeOut;                /* poll 타임아웃 (밀리초) */
int     Pk;                     /* 패킷 인덱스 */
int     PortNo;                 /* 리스닝 포트 번호 */
int     SendFlag;               /* 데이터 송신 여부 플래그 */
char    ApType[10];             /* 업무식별코드 */
char    RecvPkt[CLI_BUFF_MAX_LEN];  /* TCP 수신 버퍼 */
char    SendPkt[CLI_BUFF_MAX_LEN];  /* TCP 송신 버퍼 */
char    DeviceSendFlag;         /* 데이터 송신 완료 대기 플래그 */
char    LogOnFlag;              /* 로그인 완료 여부 */
char    OpenFlag;               /* 소켓 바인드/리슨 완료 여부 */

FILE_BUFF_FORMAT    R_Fmt[MAX_CNT], W_Fmt[MAX_CNT]; /* FIFO 읽기/쓰기 버퍼 */

CLI_FORMAT          *S_Pkt = (CLI_FORMAT *)SendPkt; /* 송신 패킷 (헤더+데이터) */
CLI_HEAD            *R_Pkt = (CLI_HEAD *)RecvPkt;   /* 수신 패킷 (헤더만) */

struct pollfd       Poll[2];        /* poll 이벤트 배열 [소켓, FIFO] */

/*------------------------------------------------------------------------
 *  함수 프로토타입
 *------------------------------------------------------------------------*/
void    PB_8100_TS(void);
void    Init_Parameters(void);
void    Fifo_Event_Rtn(void);
void    Socket_Event_Rtn(void);
void    Data_Event_Rtn(void);
void    Device_Write(void);
void    Device_Open(void);
void    Device_Close(void);
int     Device_Read(void);
void    Time_Out_Rtn(void);
void    Make_Send_Msg(int);

/*======================================================================
 *  main: 프로세스 진입점
 *======================================================================*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
#ifdef LAT_TRACE
    LAT_INIT(argv[0]);
#endif
    PB_8100_TS();
    Exit_Process();
}   /* End of main () */

/*======================================================================
 *  PB_8100_TS: 메인 처리 루틴
 *  ------------------------------------------------------------------
 *  pa_8100_ts.c의 PA_8100_TS와 동일한 구조이나 다른 점:
 *    1) HOLIDAY_APPLY: break만 수행 (주말 체크 비활성)
 *    2) FIFO 카운트 비교: R_CNT(0,0)만 사용 (PA는 빌드별 분기)
 *    3) poll 이벤트: for 루프로 수동 처리 (PA는 detect_poll_event)
 *======================================================================*/
void    PB_8100_TS(void) {
    int     rt, i;

    Init_Parameters();

#if defined HOLIDAY_APPLY
    /*--------------------------------------------------------------
     *  HOLIDAY_APPLY: PB에서는 즉시 break (주말 체크 비활성)
     *--------------------------------------------------------------*/
    {
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

            break;      /* PB: 즉시 탈출 (주말 체크 없음) */
        }
    }
#endif

    /*--------------------------------------------------------------
     *  메인 이벤트 루프
     *--------------------------------------------------------------*/
    while (START_S != END) {
        Stat_Save();

        /*----------------------------------------------------------
         *  종료/중지 상태 체크
         *----------------------------------------------------------*/
        if (TCP2_NET_STA(0) == END || TCP2_NET_STA(0) == JOB_STOP) {
            if (LogOnFlag == ON && TCP2_NET_STA(0) == END) {
                Device_Close();
            }
        }
        /*----------------------------------------------------------
         *  정상 운영 시간
         *----------------------------------------------------------*/
        else {
            if (OpenFlag == OFF) {
                /* 소켓 미오픈: Bind/Listen 후 Accept 대기 */
                Device_Open();
                TCP2_NET_STA(0) = ON;

                Log(USR_OK, "acceptiong ...");
                Newfd = accept(Sockfd, (struct sockaddr *)NULL, NULL);
                if (Newfd < 0) {
                    if (SYS_NO == ECONNABORTED) {
                        usleep(100000);
                        continue;
                    }

                    Log(TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log(USR_OK, "accepted");

                Set_Socket_Linger(Newfd);

                TCP2_LINE_ST = OpenFlag = ON;

                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;
                TimeOut = FOREVER_TIME;
            }
            else {
                if (LogOnFlag == ON) {
                    if (DeviceSendFlag == ON) {
                        PollCnt = 1;
                        TimeOut = FOREVER_TIME;
                    }
                    else {
                        PollCnt = 2;
                        TimeOut = DATA_TIME;

                        /* PB: R_CNT(0,0)만 사용 (PA는 빌드별 분기) */
                        if (WRITE_CNT > R_CNT(0,0)) {
                            Data_Event_Rtn();
                            continue;
                        }
                    }
                }
                else {
                    PollCnt = 1;
                    TimeOut = FOREVER_TIME;
                }
            }
        }

        /*----------------------------------------------------------
         *  poll 이벤트 대기
         *----------------------------------------------------------*/
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
                    case    SOCKET_EVENT:
                        Socket_Event_Rtn();
                        break;
                    case    DATA_EVENT:
                        Data_Event_Rtn();
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
}   /* End of PB_8100_TS () */

/*======================================================================
 *  Init_Parameters: 전역 변수 초기화
 *======================================================================*/
void    Init_Parameters(void) {
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;

    L_K = 0;

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    return;
}   /* End of Init_Parameters () */

/*======================================================================
 *  Socket_Event_Rtn: 클라이언트 응답 수신 처리
 *  ------------------------------------------------------------------
 *  로그인 완료 상태:
 *    DAOK → Add_Count 호출 (빌드별로 PS_R_1 또는 PS_R_2)
 *    POOK → 응답코드 확인만
 *
 *  로그인 전:
 *    LINK → 시퀀스 동기화 후 LIOK 응답 송신
 *======================================================================*/
void    Socket_Event_Rtn(void) {
    int     rval, r_seq, rt;
    char    m_time[24], Chg_Data[KRX_DATA_BUFF_SIZE];

    rval = Device_Read();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;

    if (LogOnFlag == ON) {
        if (memcmp(R_Pkt->MsgType, "DAOK", 4) == 0) {
            if (memcmp(R_Pkt->ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "DAOK Response is NotOk [%50.50s]", R_Pkt);
                Exit_Process();
            }

            /* FIFO 읽기 카운트 증가: 빌드별로 다른 FIFO */
#if defined(B8101) || defined(B8111) || defined(B8116)
            Add_Count(PS_R_1, 1);
#elif defined(B8105) || defined(B8117) || defined(B8118)
            Add_Count(PS_R_2, 1);
#endif
            INT_SEQ ++;
        }
        else
            if (memcmp(R_Pkt->MsgType, "POOK", 4) == 0) {
            if (memcmp(R_Pkt->ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "POOK Response is NotOk [%4.4s]", R_Pkt->ResponsCode);
                Exit_Process();
            }
        }
        else {
            Log(USR_ERROR, "MsgType Not Defined Code [%s]", R_Pkt);
            Exit_Process();
        }
    }
    else {
        /* 로그인 전: LINK 요청 처리 */
        if (memcmp(R_Pkt->MsgType, "LINK", 4) != 0) {
            Log(USR_ERROR, "LOGON(LINK) error, check plese Recv Data[%s]", R_Pkt);
            Exit_Process();
        }

        /* 시퀀스 동기화 */
        r_seq = AtoIf(R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));
        if (r_seq > WRITE_CNT || r_seq < 0 || r_seq > INT_SEQ+2) {
            TCP2_LINE_ST = OpenFlag = OFF;
            Log(USR_ERROR, "TR_LIOK recv:invalid HUB SEQ <%d> > INT_SEQ <%d> WRITE_CNT<%d>",
                    r_seq, INT_SEQ, WRITE_CNT);
            Exit_Process();
        }
        else
            if (r_seq != INT_SEQ && r_seq <= WRITE_CNT) {
            Log(USR_WARN, "TR_LIOK recv:check HUB SEQ <%d> < <%d>", r_seq, INT_SEQ);
        }

        /* LIOK 응답 전송 후 로그인 상태 전환 */
        Make_Send_Msg(RP_LINK);
        Device_Write();
        LogOnFlag = ON;
    }

    return;
}   /* End of Socket_Event_Rtn () */

/*======================================================================
 *  Data_Event_Rtn: FIFO 데이터 읽어서 클라이언트로 전송
 *======================================================================*/
void    Data_Event_Rtn(void) {
    int     rt, r_cnt, i, j;
    char    tmp[128];

    SendFlag = 0;
    Make_Send_Msg(TR_DATA);

    if (SendFlag == 0)
        Make_Send_Msg(TR_POLL);

    Device_Write();
#ifdef LAT_TRACE
    if (SendFlag == 1)
        LAT_POINT("OUT", R_Fmt[0].Data, 11);
#endif
    Set_TR_Time();
    DeviceSendFlag = ON;

    /* FIFO 알림 데이터 drain */
    while (1) {
        rt = read(INPUT_FD, tmp, sizeof(tmp));

        if (rt == 0)
            break;
        else if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;
            if (SYS_NO != 11)
                Log(FIF_ERROR, "Poll:cannot read FIFO {%d:%s}", SYS_NO, SYS_STR);
            break;
        }
    }

    return;
}   /* End of Data_Event_Rtn () */

/*======================================================================
 *  Device_Open: TCP 서버 소켓 생성 및 바인드/리슨
 *======================================================================*/
void    Device_Open(void) {
    int         rt;
    const   int on = 1;

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
        shutdown(Sockfd, SHUT_RDWR);
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

    return;
}   /* End of Device_Open () */

/*======================================================================
 *  Device_Close: TCP 접속 해제
 *======================================================================*/
void    Device_Close(void) {
    close(Sockfd);
    close(Newfd);
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
}   /* End of Device_Close () */

/*======================================================================
 *  Device_Read: 클라이언트로부터 TCP 패킷 수신
 *======================================================================*/
int     Device_Read(void) {
    int     rt;
    char    m_time[24];

    memset(RecvPkt, 0, sizeof (RecvPkt));

    rt = Select_Receive_Cli(Newfd, RecvPkt);

    if (rt <= 0 || rt > KRX_DATA_BUFF_SIZE) {
        Log(TCP_ERROR, "TCP RD Worng Lenth[%s] (%d)<%d>", RecvPkt, rt, INT_SEQ);
        Device_Close();
        return (NOTOK);
    }

    Log_Hot(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

    return (OK);
}   /* End of Device_Read () */

/*======================================================================
 *  Device_Write: 클라이언트에게 TCP 패킷 송신
 *======================================================================*/
void    Device_Write(void) {
    int     rt;

    rt = Select_Send(Newfd, SendPkt, strlen(SendPkt));

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    Log_Hot(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* End of Device_Write () */

/*======================================================================
 *  Time_Out_Rtn: 타임아웃 처리
 *======================================================================*/
void    Time_Out_Rtn(void) {
    int     rt;
    char    in_num[10];

    switch (PollCnt) {
        case    1:
            if (OpenFlag == ON) {
                Log(TCP_ERROR, "no Response from Client. check status <%d>", INT_SEQ);
                Device_Close();
        }
        else {
            Log(USR_OK, "connect timeout <%d>", INT_SEQ);
        }
        break;
        case    2:
            Make_Send_Msg(TR_POLL);
            Device_Write();
            DeviceSendFlag = ON;
            break;
        default:
            break;
    }

    return;
}   /* End of Time_Out_Rtn () */

/*======================================================================
 *  Make_Send_Msg: 송신 패킷 생성
 *  ------------------------------------------------------------------
 *  pa_8100_ts.c와 동일한 구조이나 다음이 다름:
 *    - 매체구분 필터 없음 (B8101/B8105 등은 필터 없이 모두 전송)
 *    - #if 블록이 빈 상태 (향후 확장 예비)
 *    - FIFO 선택: B8101/B8111/B8116 → PS_R_1, B8105/B8117/B8118 → PS_R_2
 *======================================================================*/
void    Make_Send_Msg(int tr_code) {
    int     r_cnt, data_length;
    char    d_time[16];

    memset(SendPkt,       0,   sizeof (SendPkt));
    memset(SendPkt,    0x20,       CLI_HEAD_LEN);  /* 50바이트 공백 초기화 */

    Get_DateTime(d_time);

    /* 공통 헤더 설정 */
    memcpy(S_Pkt->Head.ResponsCode,"0000", sizeof(S_Pkt->Head.ResponsCode));
    memcpy(S_Pkt->Head.TradeDate,  d_time, sizeof(S_Pkt->Head.TradeDate));

    switch (tr_code) {
        case    TR_DATA:
            /*--------------------------------------------------
             *  DATA 전문: FIFO에서 읽은 데이터 포장
             *--------------------------------------------------*/
            memcpy(S_Pkt->Head.MsgType, "DATA", sizeof(S_Pkt->Head.MsgType));
            ItoAf(INT_SEQ+1, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

            /* FIFO 읽기: 빌드별로 다른 FIFO 사용 */
#if defined(B8101) || defined(B8111) || defined(B8116)
            r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
#elif defined(B8105) || defined(B8117) || defined(B8118)
            r_cnt = F_R(PS_R_2, (void *)R_Fmt, MAX_CNT);
#endif
            if (r_cnt < 0 || r_cnt > MAX_CNT) {
                Log(SAM_FATAL, "F_R(PS_R_1) R cnt Err [%s] r_cnt[%d]", IDN(D_K,P_K,0), r_cnt);
                Exit_Process();
        }
        else if (r_cnt == 0) {
            return;     /* 읽을 데이터 없음 */
        }
        else {
            /* PB: 매체구분 필터 없음 (빈 블록, 향후 확장 예비) */
#if defined(B8101) || defined(B8111) || defined(B8116)
#elif defined(B8105) || defined(B8117) || defined(B8118)
#endif
        }
        SendFlag = 1;
#ifdef LAT_TRACE
        /* 전달 데이터 선두 DataSeq(11)를 구간 키로 사용 */
        LAT_POINT("IN", R_Fmt[0].Data, 11);
#endif

        data_length = DATA_SIZE;
        ItoAf(data_length+46, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));

        memcpy(S_Pkt->Data, R_Fmt[0].Data, data_length);

        break;

        case    RP_LINK:
            /*--------------------------------------------------
             *  LIOK 전문: 로그인 응답
             *--------------------------------------------------*/
            ItoAf(CLI_HEAD_LEN - sizeof(S_Pkt->Head.Length),
                    S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
            memcpy(S_Pkt->Head.MsgType, "LIOK", sizeof(S_Pkt->Head.MsgType));
            ItoAf(INT_SEQ, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            break;

        case    TR_POLL:
            /*--------------------------------------------------
             *  POLL 전문: 상태확인 (하트비트)
             *--------------------------------------------------*/
            ItoAf(CLI_HEAD_LEN - sizeof(S_Pkt->Head.Length),
                    S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
            memcpy(S_Pkt->Head.MsgType, "POLL", sizeof(S_Pkt->Head.MsgType));
            ItoAf(INT_SEQ, S_Pkt->Head.SeqNo, sizeof(S_Pkt->Head.SeqNo));

            break;

        default:
            break;
    }

    return;
}   /* End of Make_Send_Msg () */

/*========================================================================
 *  End of Program (pb_8100_ts.c)
 *========================================================================*/
