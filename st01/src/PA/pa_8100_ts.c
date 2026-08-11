#define     _GLOBAL
/*========================================================================
 *  Module  : 내부 클라이언트(Noa) 데이터 송신 (TCP 서버)
 *  File    : pa_8100_ts.c
 *  Author  : PSH
 *  -----------------------------------------------------------------
 *  내부 허브(Noa) 클라이언트에게 데이터를 송신하는 TCP 서버 프로세스.
 *  서버 모드로 동작: Socket→Bind→Listen→Accept 후 클라이언트 접속 대기.
 *  FIFO에서 데이터를 읽어 CLI 프로토콜로 클라이언트에게 전송.
 *
 *  빌드 옵션:
 *    A8101 : 채권(1번 채널) 데이터 송신  — PS_R_1 사용, 매체구분 A/C만 허용
 *    A8102 : 파생(2번 채널) 데이터 송신  — PS_R_2 사용, 매체구분 A/T만 허용
 *
 *  CLI 프로토콜:
 *    클라이언트→서버: LINK(로그인), DAOK(데이터확인), POOK(POLL확인)
 *    서버→클라이언트: LIOK(로그인응답), DATA(데이터), POLL(상태확인)
 *
 *  이벤트 처리 (poll 기반):
 *    Poll[0] = 소켓 이벤트 (클라이언트 응답 수신)
 *    Poll[1] = FIFO 이벤트 (송신할 데이터 도착 알림)
 *========================================================================*/

/*------------------------------------------------------------------------
 *  헤더 파일
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "cli_interface.h"       /* CLI 프로토콜 상수 및 구조체 */
#include    "fep_common.h"

#define     MAX_CNT         1       /* FIFO에서 한번에 읽는 최대 건수 */

#define     DATA_SIZE       2048    /* 1건당 데이터 크기 (바이트) */
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
void    PA_8100_TS(void);
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
    PA_8100_TS();
    Exit_Process();
}   /* End of main () */

/*======================================================================
 *  PA_8100_TS: 메인 처리 루틴
 *  ------------------------------------------------------------------
 *  1) 초기화 및 휴일 체크
 *  2) 소켓 오픈 → Accept 대기
 *  3) 이벤트 루프:
 *     - SOCKET_EVENT: 클라이언트 응답 수신 (LINK/DAOK/POOK)
 *     - DATA_EVENT:   FIFO 데이터 읽어서 클라이언트로 전송
 *     - 타임아웃:     POLL 전문 전송
 *======================================================================*/
void    PA_8100_TS(void) {
    int     rt, i;

    Init_Parameters();

#if defined HOLIDAY_APPLY
    /*--------------------------------------------------------------
     *  휴일 체크: 토(6)/일(0)요일이면 60초 sleep
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

            if (tp->tm_wday == 0 || tp->tm_wday == 6)       /* 일, 토 */ {
                Log(USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
                sleep(60);
            }
            else {
                Log(USR_OK, "it's not weekend. not sleeping...[%d]", tp->tm_wday);
                break;
            }
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
                /*
                 * 소켓 미오픈 상태: Device_Open으로 Bind/Listen 후 Accept 대기
                 */
                Device_Open();
                TCP2_NET_STA(0) = ON;

                Log(USR_OK, "acceptiong ...");
                Newfd = accept(Sockfd, (struct sockaddr *)NULL, NULL);
                if (Newfd < 0) {
                    if (SYS_NO == ECONNABORTED) {
                        usleep(100000);    /* 100ms 후 재시도 */
                        continue;
                    }

                    Log(TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
                    return;
                }
                Log(USR_OK, "accepted");

                Set_Socket_Linger(Newfd);

                TCP2_LINE_ST = OpenFlag = ON;

                /* Accept된 소켓을 poll 감시 대상에 등록 */
                Poll[0].fd = Newfd;
                Poll[0].events = POLLIN;

                PollCnt = 1;            /* 소켓만 감시 (로그인 전) */
                TimeOut = FOREVER_TIME;
            }
            else {
                /*
                 * 소켓 오픈 완료 상태
                 */
                if (LogOnFlag == ON) {
                    if (DeviceSendFlag == ON) {
                        /* 데이터 송신 후 응답 대기 중 → 소켓만 감시 */
                        PollCnt = 1;
                        TimeOut = FOREVER_TIME;
                    }
                    else {
                        /* 데이터 수신 가능 → 소켓+FIFO 감시 */
                        PollCnt = 2;
                        TimeOut = DATA_TIME;

                        /*
                         * FIFO에 미처리 데이터가 있으면 poll 없이 즉시 처리
                         * WRITE_CNT = FIFO 기록 카운트 (타 프로세스가 증가)
                         * R_CNT     = FIFO 읽기 카운트 (이 프로세스가 증가)
                         */
#if defined A8101
                        if (WRITE_CNT > R_CNT(0,0))
#elif defined A8102
                            if (WRITE_CNT > R_CNT(0,1))
#endif
                            {
                            Data_Event_Rtn();
                            continue;
                        }
                    }
                }
                else {
                    /* 로그인 전: 소켓만 감시 (LINK 대기) */
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
            Time_Out_Rtn();    /* 타임아웃 → POLL 전문 전송 */
            continue;
        }

        /*
         * detect_poll_event: POLLHUP/POLLIN 이벤트를 자동 감지
         * 반환값: 이벤트 발생한 poll 인덱스, -1=종료, -2=continue
         */
        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;
        if (i == -2) continue;

        switch (i) {
            case    SOCKET_EVENT:
                Socket_Event_Rtn();    /* 클라이언트 응답 처리 */
                break;
            case    DATA_EVENT:
                Data_Event_Rtn();      /* FIFO 데이터 송신 */
                break;
            default:
                Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process();
                break;
        }
    }

    return;
}   /* End of PA_8100_TS () */

/*======================================================================
 *  Init_Parameters: 전역 변수 초기화
 *  ------------------------------------------------------------------
 *  - 각종 플래그 OFF
 *  - 리스닝 포트: cfg/tcp2.ini에서 읽어옴
 *  - Poll[1]에 FIFO 파일디스크립터 등록
 *======================================================================*/
void    Init_Parameters(void) {
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;

    L_K = 0;

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;      /* 프로세스 상태: 가동중 */
    TCP2_LINE_ST = OFF;     /* 회선 상태: 미연결 */

    /* FIFO 파일디스크립터를 poll 감시 대상에 등록 */
    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    return;
}   /* End of Init_Parameters () */

/*======================================================================
 *  Socket_Event_Rtn: 클라이언트 응답 수신 처리
 *  ------------------------------------------------------------------
 *  로그인 완료 상태:
 *    DAOK(데이터확인) → 응답코드 확인, 읽기 카운트 증가
 *    POOK(POLL확인)   → 응답코드 확인만
 *
 *  로그인 전:
 *    LINK(로그인요청) → 시퀀스 동기화 후 LIOK 응답 송신
 *======================================================================*/
void    Socket_Event_Rtn(void) {
    int     rval, r_seq, rt;
    char    m_time[24], Chg_Data[KRX_DATA_BUFF_SIZE];

    rval = Device_Read();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;

    if (LogOnFlag == ON) {
        /*----------------------------------------------------------
         *  로그인 완료 상태: DAOK 또는 POOK 응답 처리
         *----------------------------------------------------------*/
        if (memcmp(R_Pkt->MsgType, "DAOK", 4) == 0) {
            /* 데이터 확인 응답: 응답코드 ≠ 0000이면 오류 */
            if (memcmp(R_Pkt->ResponsCode, "0000", 4) != 0) {
                Log(USR_ERROR, "DAOK Response is NotOk [%50.50s]", R_Pkt);
                Exit_Process();
            }

            /* FIFO 읽기 카운트 증가 (다음 건 읽기 가능) */
#if defined A8101
            Add_Count(PS_R_1, 1);
#elif defined A8102
            Add_Count(PS_R_2, 1);
#endif
            INT_SEQ ++;
        }
        else
            if (memcmp(R_Pkt->MsgType, "POOK", 4) == 0) {
            /* POLL 확인 응답 */
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
        /*----------------------------------------------------------
         *  로그인 전: LINK(로그인) 요청 처리
         *----------------------------------------------------------*/
        if (memcmp(R_Pkt->MsgType, "LINK", 4) != 0) {
            Log(USR_ERROR, "LOGON(LINK) error, check plese Recv Data[%s]", R_Pkt);
            Exit_Process();
        }

        /* 시퀀스 동기화: 클라이언트가 보낸 SeqNo와 현재 시퀀스 비교 */
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

        /* LIOK(로그인응답) 전송 후 로그인 상태 전환 */
        Make_Send_Msg(RP_LINK);
        Device_Write();
        LogOnFlag = ON;
    }

    return;
}   /* End of Socket_Event_Rtn () */

/*======================================================================
 *  Data_Event_Rtn: FIFO 데이터 읽어서 클라이언트로 전송
 *  ------------------------------------------------------------------
 *  1) Make_Send_Msg(TR_DATA)로 FIFO에서 읽어 송신 패킷 생성
 *  2) 송신할 데이터가 없으면 POLL 패킷 생성
 *  3) Device_Write로 TCP 전송
 *  4) FIFO 잔여 알림 데이터 비우기 (read로 drain)
 *======================================================================*/
void    Data_Event_Rtn(void) {
    int     rt, r_cnt, i, j;
    char    tmp[128];

    SendFlag = 0;
    Make_Send_Msg(TR_DATA);

    /* 데이터가 없으면 POLL 패킷으로 대체 */
    if (SendFlag == 0)
        Make_Send_Msg(TR_POLL);

    /* TCP 전송 */
    Device_Write();
    Set_TR_Time();
    DeviceSendFlag = ON;

    /* FIFO 알림 데이터 drain (비차단 모드로 전부 읽어냄) */
    while (1) {
        rt = read(INPUT_FD, tmp, sizeof(tmp));

        if (rt == 0)
            break;
        else if (rt == -1) {
            if (SYS_NO == EINTR)
                continue;
            if (SYS_NO != 11)       /* EAGAIN(11) 아니면 오류 로그 */
                Log(FIF_ERROR, "Poll:cannot read FIFO {%d:%s}", SYS_NO, SYS_STR);
            break;
        }
    }

    return;
}   /* End of Data_Event_Rtn () */

/*======================================================================
 *  Device_Open: TCP 서버 소켓 생성 및 바인드/리슨
 *  ------------------------------------------------------------------
 *  서버 모드: Socket → SO_REUSEADDR → Bind → Listen
 *  5초 sleep 후 시작 (다른 프로세스 초기화 대기)
 *======================================================================*/
void    Device_Open(void) {
    int         rt;
    const   int on = 1;

    sleep(5);

    /* 1. 소켓 생성 */
    Sockfd = Socket();
    if (Sockfd < 0) {
        Log(TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "socket created:Sockfd[%d]", Sockfd);

    /* 2. SO_REUSEADDR: 포트 재사용 허용 (TIME_WAIT 방지) */
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0) {
        Log(TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
        shutdown(Sockfd, SHUT_RDWR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "setsockopt[%d]", Sockfd);

    /* 3. Bind: 포트 바인드 */
    rt = Bind(Sockfd, PortNo);
    if (rt < 0) {
        Log(TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PortNo, SYS_NO, SYS_STR);
        close(Sockfd);
        TCP2_NET_STA(0) = OFF;
        Exit_Process();
    }
    Log(USR_OK, "bind[%d]", Sockfd);

    /* 4. Listen: 접속 대기 */
    Listen(Sockfd);
    Log(USR_OK, "listen[%d]", Sockfd);

    /* 리스닝 소켓을 poll 감시 대상에 임시 등록 */
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
 *  ------------------------------------------------------------------
 *  Select_Receive_Cli: CLI 프로토콜 전용 수신 (길이 기반)
 *  반환값: OK(0) 또는 NOTOK(-1)
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

    Log(TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen(RecvPkt), INT_SEQ);

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

    Log(TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
}   /* End of Device_Write () */

/*======================================================================
 *  Time_Out_Rtn: 타임아웃 처리
 *  ------------------------------------------------------------------
 *  PollCnt=1 (로그인 대기중): 접속/응답 타임아웃 → 회선 끊김
 *  PollCnt=2 (데이터 대기중): POLL 전문 전송 (하트비트)
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
            /* 데이터 없이 25초 경과 → POLL(하트비트) 전송 */
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
 *  tr_code별 처리:
 *    TR_DATA: FIFO에서 읽은 데이터를 DATA 전문으로 포장
 *    RP_LINK: 로그인 응답 (LIOK) 전문 생성
 *    TR_POLL: 상태확인 (POLL) 전문 생성
 *
 *  CLI_HEAD 구조 (50바이트):
 *    Length(4) + MsgType(4) + ResponsCode(4) + TradeDate(8)
 *    + SeqNo(8) + ... = 50바이트
 *
 *  매체구분 필터 (Data[19]):
 *    A8101: A(전체)/C(1번채널) → 전송, T(2번채널) → 건너뜀
 *    A8102: A(전체)/T(2번채널) → 전송, C(1번채널) → 건너뜀
 *    100으로 시작하면 주문 데이터 → 무조건 전송
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
#if defined A8101
            r_cnt = F_R(PS_R_1, (void *)R_Fmt, MAX_CNT);
#elif defined A8102
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
            /*
             * 매체구분 필터: Data[19] 위치의 1바이트로 판별
             *   A = All (전체) → 항상 전송
             *   C = 1번 채널   → A8101만 전송
             *   T = 2번 채널   → A8102만 전송
             *   "100"으로 시작 = 주문 → 무조건 전송
             */
#if defined A8101
            if (memcmp(&R_Fmt[0].Data[19], "A", 1) != 0    &&
                    memcmp(&R_Fmt[0].Data[19], "C", 1) != 0    &&
                    memcmp(R_Fmt[0].Data, "100", 3) != 0) {
                if (memcmp(&R_Fmt[0].Data[19], "T", 1) != 0)
                    Log(USR_ERROR, "매체구분 오류 [%50.50s][%c]", R_Fmt[0].Data, &R_Fmt[0].Data[19]);
                Add_Count(PS_R_1, 1);
                INT_SEQ ++;
                return;
            }
#elif defined A8102
            if (memcmp(&R_Fmt[0].Data[19], "A", 1) != 0    &&
                    memcmp(&R_Fmt[0].Data[19], "T", 1) != 0    &&
                    memcmp(R_Fmt[0].Data, "100", 3) != 0) {
                if (memcmp(&R_Fmt[0].Data[19], "C", 1) != 0)
                    Log(USR_ERROR, "매체구분 오류 [%50.50s][%c]", R_Fmt[0].Data, &R_Fmt[0].Data[19]);
                Add_Count(PS_R_2, 1);
                INT_SEQ ++;
                return;
            }
#endif
        }
        SendFlag = 1;

        /* 패킷 길이: 헤더(50-4) + 데이터 크기 */
        data_length = DATA_SIZE;
        ItoAf(data_length+46, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));

        /* 데이터 복사 */
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
 *  End of Program (pa_8100_ts.c)
 *========================================================================*/
