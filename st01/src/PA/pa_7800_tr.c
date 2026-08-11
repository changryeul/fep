#define     _GLOBAL

/*------------------------------------------------------------------------
#   Module  : RDS 일괄송신 수신 (KRX 직접연결)
#   File    : pa_7800_tr.c
#
#   설명  : KRX에서 RDS(일괄송신) 방식으로 전달하는 종목정보를
#             TCP로 수신하는 프로세스. 야간 배치로 동작하며
#             채권종목정보(RDS01)와 KTS종목정보(RDS02)를
#             수신하여 FIFO로 DD 프로세스에 전달한다.
#
#   KRX RDS TR코드:
#             TRDESP50101 : 채권종목정보 (1,173바이트) → pa_7801_dd
#             TRDESP50102 : KTS종목정보  (350바이트)   → pa_7802_dd
#             그 외 TR코드는 Seq만 처리하고 데이터는 저장하지 않음
#
#   프로토콜 흐름:
#             1) KRX 접속 → 로그인 (SCHLIQ00000)
#             2) 로그인 응답 대기 (SCHLIR00000)
#             3) 데이터 수신 → 응답 전송 (TR별 반복)
#             4) 배치 종료(SEQ=999999) 시 INT_SEQ 리셋
#             5) 회선시험(HeartBeat) 주기적 처리
#
#   공통 라이브러리 사용:
#             fep_common.c의 공유 함수 사용 (Device_Read, Device_Write 등)
#             KR_Fmt(KRX_R_SESSION_FMT)을 FmtPtr에 할당
#             PA 모듈이므로 Handshake()는 스텁(NOTOK 리턴)
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_trcode.h"
#include    "ifaddrs.h"
#include    "fep_common.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DEVICE_TIME 3  * 1000       /* 재접속 대기 시간 (3초) */
#define     MAIN_TIME   35 * 1000       /* 하트비트 간격 (35초) */
#define     FOREVER_TIME    60 * 1000       /* 최대 Poll 대기 (60초) */

#define     FIFO_EVENT  0           /* Poll 이벤트: FIFO (Daemon 신호) */
#define     SOCKET_EVENT    1           /* Poll 이벤트: TCP 소켓 */

#define     MAX_CNT     7           /* RDS 묶음 처리 최대 건수 */

#define     NO_TIME     "0600"          /* RDS는 야간 배치이므로 의미 없는 시간 */

#define     PORT_NO     TCP2_PORT_NO

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int ConnectRetryCnt, D_End;             /* 재접속 횟수, 배치종료 플래그 */
int Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int PollCnt, FirstSeq, TimeOut, Pk;
int back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag;
char    DataBuff[KRX_DATA_BUFF_SIZE], IpAddr[20], ApType[10];
char    NoTime[] = NO_TIME;

KRX_HEADER          Header_Fmt;     /* KRX 헤더 (82바이트) */
KRX_JUMUN_R_FMT         Reply;          /* 주문 응답 포맷 (82 + 4+11) */
FILE_DATA_HEAD          File_Data_Head;     /* FIFO 데이터 헤더 (20바이트) */
KRX_R_SESSION_FMT       KR_Fmt;         /* KRX 세션 포맷 (82 + 116) */
struct pollfd           Poll[2];        /* poll: [0]=FIFO, [1]=소켓 */

/* fep_common.c 공유 함수용 FmtPtr 설정 */
void *FmtPtr = (void *)&KR_Fmt;

/* PA 모듈: 암호화 미사용 (PB만 사용), 스텁 함수 */
int     Handshake(void) { return (NOTOK); }

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_7800_TR(void);
void    Init_Parameters(void);
void    Socket_Event_Rtn(void);
void    Device_Open(int);
void    Time_Out_Rtn(void);
int Analyze_Data(void);
void    Write_Data(int);
int Make_Send_Msg(int);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 RDS 수신 루프 실행.             */
/*----------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    Init_Proc(argc, argv);     /* 프로세스 초기화 */
    PA_7800_TR();          /* 메인 처리 루프 */
    Exit_Process();        /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7800_TR: 메인 처리 루프                                            */
/*    1) 초기 상태: 로그인 미완료 → 3초 간격 재시도                 */
/*    2) 로그인 완료 후: 35초 하트비트 간격으로 poll 대기                */
/*    3) 3회 접속 실패 시 Line_Change()로 백업 회선 전환             */
/*    4) FIFO_EVENT → Daemon 신호 처리                                  */
/*    5) SOCKET_EVENT → KRX 데이터 수신 처리                           */
/*----------------------------------------------------------------------*/
void    PA_7800_TR(void) {
    int     rt, i;

    Init_Parameters();

    while (START_S != END) {
        Stat_Save();

        if (OpenFlag == OFF) {
            if (LogOnFlag == OFF)           /* 로그온 미완료 → 접속 시도 */
                Device_Open(TR_LOON);

            if (LogOnFlag == OFF)           /* 아직 로그온 미완료 */ {
                ConnectRetryCnt ++;
                if (ConnectRetryCnt >= 3)   /* 3회 실패 → 백업 회선 전환 */ {
                    Line_Change();
                    ConnectRetryCnt = 0;
                }

                PollCnt = 1;                /* FIFO만 감시 */
                TimeOut = DEVICE_TIME;      /* 3초 타임아웃 */
            }
            else                    /* 로그온 완료 → 데이터 대기 */ {
                PollCnt = 2;            /* FIFO + 소켓 감시 */
                TimeOut = MAIN_TIME;        /* 35초 타임아웃 */
            }
        }
        else                    /* 개시 완료 → 데이터 대기 */ {
            PollCnt = 2;
            TimeOut = MAIN_TIME;
        }

        /* poll() 이벤트 대기 */
        rt = poll(Poll, PollCnt, TimeOut);
        if (rt < 0) {
            if (SYS_NO == EINTR)
                Log(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0) {
            Time_Out_Rtn();                /* 타임아웃 → 하트비트 또는 로그 */
            continue;
        }

        /* 이벤트 감지 */
        i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);
        if (i == -1) return;                /* 심각한 오류 → 종료 */
            if (i == -2) continue;              /* 무시할 이벤트 → 다음 루프 */

            switch (i) {
            case FIFO_EVENT:                /* Daemon 신호 (시작/종료) */
                Fifo_Event_Rtn();
                break;
            case SOCKET_EVENT:              /* KRX 데이터 수신 */
                Socket_Event_Rtn();
                break;
            default:
                Log(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process();
                break;
        }
    }

    Device_Close();

    return;
}   /* End of PA_7800_TR () */

/*************************************************************************
    Function        : Init_Parameters
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 전역 변수 초기화.
                      TCP 접속 IP/Port 설정, 상태 플래그 OFF,
                      FIFO 이벤트용 poll FD 등록.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters(void) {
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;
    D_End = ErrCd = 0;
    ConnectRetryCnt = 0;

    S_K = TCP2_LINE_GU;                     /* 회선 구분 (MAIN/BACKUP) */

    /* TCP 접속 대상 IP/Port 설정 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
            TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log(TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    /* MAIN/BACKUP 프로세스 상태 ON, 라인 상태 OFF */
    TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

    /* ApType 생성 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /* Daemon 신호용 FIFO poll FD 등록 */
    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;

    return;
}   /* End of Init_Parameters () */

/*************************************************************************
    Function        : Socket_Event_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : KRX 데이터 수신 이벤트 처리.
                      1) Device_Read()로 KRX 전문 수신
                      2) KRX 헤더(82바이트) 파싱
                      3) Analyze_Data()로 TR 유형 판별
                      4) TR 유형별 처리:
                         RP_DATA  → Skip 데이터 (Seq만 처리 + 응답)
                         TR_DATA  → 종목정보 FIFO 기록 + 응답
                         RP_POLL  → 하트비트 응답 (로그만)
                         RP_LOON  → 로그인 응답 확인
                         RP_STOP  → 로그아웃 → 프로세스 종료
                      5) D_End=1이면 INT_SEQ 리셋 (배치 종료)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Socket_Event_Rtn(void) {
    int rval, rt, cnt, r_meg_no, r_meg_seq;
    char    t_time[12];

    rval = Device_Read();

    if (rval < 0)
        return;

    DeviceSendFlag = OFF;
    ErrCd = 0;

    /* KRX 헤더(82바이트) 파싱 */
    memset(&Header_Fmt, 0, KRX_HEAD_LEN);
    memcpy(&Header_Fmt, DataBuff, KRX_HEAD_LEN);

    /* TR 유형 판별 */
    ReTrCode = Analyze_Data();

    rt = 0;

    /*
     * TR 유형별 처리:
     *   RP_DATA: 처리 대상이 아닌 TR (Seq만 관리, 응답 전송)
     *   TR_DATA: 종목정보 (RDS01/RDS02) → FIFO 기록 + 응답
     *   RP_POLL: 하트비트 응답 수신 (로그만)
     *   RP_LOON: 로그인 응답 (오류 시 종료)
     *   RP_STOP: 로그아웃 완료 → 종료
     */
    switch (ReTrCode) {
        case RP_DATA:
            /* Skip 데이터: Seq만 처리하고 응답 전송 */
            Log(USR_OK, "Else Case Recv:Data[%82.82s]", DataBuff);

            Write_Data(0);                     /* Seq만 처리 (데이터 저장 안 함) */
            rt = Make_Send_Msg(TR_DATA);
            Device_Write();
            Log(USR_OK, "Reply OK!!");

            break;

        case TR_DATA:
            /*
             * Seq 연속성 체크:
             * KRX 데이터는 INT_SEQ + 1로 수신되어야 함
             * 불일치 시 회선 오류 → 3초 후 종료
             */
            if (INT_SEQ + 1 != FirstSeq) {
                TCP2_LINE_ST = OpenFlag = OFF;

                Log(USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
                        FirstSeq, INT_SEQ);

                sleep(3);
                Device_Close();
                Exit_Process();
        }

        /*
         * TR 구분: KRX_MSG_COMMON 매크로로 판별
         *   TR_RDS01_BOND_ITEM → 채권종목정보 (1173바이트) → Write_Data(1)
         *   TR_RDS02_KTS_ITEM  → KTS종목정보 (350바이트)  → Write_Data(2)
         */
        {
            KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
            if (IS_TR(msg, TR_RDS01_BOND_ITEM)) {
                Write_Data(1);             /* → pa_7801_dd (1400바이트 버퍼) */
            }
            else
                if (IS_TR(msg, TR_RDS02_KTS_ITEM)) {
                Write_Data(2);             /* → pa_7802_dd (350바이트 버퍼) */
            }
        }   /* end of KRX_MSG_COMMON *msg scope */

        /* 데이터 수신 응답 전송 */
        rt = Make_Send_Msg(RP_DATA);
        Device_Write();
        Log(USR_OK, "Reply OK!!");

        break;

        case RP_POLL:
            /* 하트비트 응답 수신 */
            Log(USR_OK, "POLL OK Recv");
            break;

        case RP_LOON:
            /* 로그인 응답: 오류이면 종료 */
            if (!IS_RESP_OK(DataBuff)) {
                Log(USR_ERROR, "LOGON Error [%4.4s]", &DataBuff[KRX_HEAD_LEN]);
                sleep(3);
                Device_Close();
                Exit_Process();
        }

        break;

        case RP_STOP:
            /* 로그아웃 완료 → 3초 후 종료 */
            Log(USR_OK, "LOGOUT OK Done");

            sleep(3);
            Device_Close();

            PROC(D_K,P_K).start_status = JOB_END;
            Exit_Process();
            break;

        default:
            break;
    }

    /*
     * 배치 종료 처리:
     * D_End=1이면 해당 TR의 마지막 데이터(SEQ=999999)를 받았으므로
     * INT_SEQ를 0으로 리셋하여 다음 TR을 위한 준비
     */
    if (D_End == 1) {
        INT_SEQ = 0;
        D_End = 0;
    }

    return;
}   /* End of Socket_Event_Rtn () */

/*************************************************************************
    Function        : Device_Open
    Parameters IN   : tr_code - 접속 유형 (TR_LOON=로그인)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : KRX 서버에 TCP 접속 및 로그인.
                      fep_common.c의 Device_Open_Logon() 사용.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Device_Open(int tr_code) {
    int     rt, rval;

    if (tr_code == TR_LOON) {
        Device_Open_Logon(0, 1);           /* 공통 로그인 함수 호출 */
    }

    return;
}   /* End of Device_Open () */

/*----------------------------------------------------------------------*/
void    Device_Close(void) {
    Device_Close_Base();                   /* 공통 접속 해제 함수 호출 */
}   /* End of Device_Close () */

/*************************************************************************
    Function        : Time_Out_Rtn
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 타임아웃 처리.
                      PollCnt=1(미접속): 대기 로그만 출력
                      PollCnt=2(접속중): 하트비트(TR_POLL) 전송
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Time_Out_Rtn(void) {
    memset(DataBuff, 0, sizeof (DataBuff));

    switch (PollCnt) {
        case 1:                             /* 미접속 상태: 로그만 */
            if (TimeOut == FOREVER_TIME)
                Log(USR_OK, "poll timeout <%d>:NSTAT[%d]",
                        INT_SEQ, TCP2_NET_STA(S_K));
            break;
        case 2:                             /* 접속 상태: 하트비트 전송 */
            Make_Send_Msg(TR_POLL);
            memcpy(DataBuff, &Reply, sizeof (KRX_JUMUN_R_FMT));
            Device_Write();
            break;
        default:
            break;
    }
}   /* End of Time_Out_Rtn () */

/*************************************************************************
    Function        : Analyze_Data
    Parameters IN   : (없음, 전역변수 Header_Fmt/DataBuff 사용)
    Parameters OUT  : (없음)
    Return Code     : int (TR 유형 코드)
    Comment         : KRX 헤더의 MsgType으로 TR 유형을 판별한다.
                      SCHLIR00000 → RP_LOON (로그인 응답)
                      SCHLOR00000 → RP_STOP (로그아웃 응답)
                      SCHHER00000 → RP_POLL (하트비트 응답)
                      TRDESP50101/50102 → TR_DATA (종목정보 데이터)
                      그 외 → RP_DATA (Skip 데이터)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Analyze_Data(void) {
    FirstSeq = AtoIf(Header_Fmt.MsgSeqNum, sizeof (Header_Fmt.MsgSeqNum));

    if (memcmp(Header_Fmt.MsgType, "SCHLIR00000", 11) == 0) {
        Log(USR_OK, "LogOn응답(SCHLIR00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_LOON);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHLOR00000", 11) == 0) {
        Log(USR_OK, "LogOut응답(SCHLOR00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_STOP);
    }
    else if (memcmp(Header_Fmt.MsgType, "SCHHER00000", 11) == 0) {
        Log(USR_OK, "회선시험응답(SCHHER00000): [%4.4s] <%d:%d:%d>",
                &DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
        return (RP_POLL);
    }
    else if ((memcmp(Header_Fmt.MsgType, "TRDESP50101", 11) == 0) ||
            (memcmp(Header_Fmt.MsgType, "TRDESP50102", 11) == 0)) {
        Log(USR_OK, "DATA수신(TRDESP50101/2): <%d:%d>", FirstSeq, INT_SEQ);
        return (TR_DATA);
    }
    else {
        Log(USR_OK, "Else Case 수신(Skip Data): <%d:%d> Tr[%11.11s]",
                FirstSeq, INT_SEQ, Header_Fmt.MsgType);
        return (RP_DATA);
    }

}   /* End of Analyze_Data () */

/*************************************************************************
    Function        : Write_Data
    Parameters IN   : p_flag - 쓰기 대상 FIFO (0=저장안함, 1=RDS01, 2=RDS02)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 수신된 RDS 데이터를 FIFO에 기록한다.
                      KRX 묶음 전문은 한 패킷에 여러 건의 데이터가 포함됨.
                      Header_Fmt.DataCnt로 건수, BodyLength/건수로 건별 크기 계산.

                      SEQ 처리:
                        각 건의 SEQ(6자리)를 확인하여 연속성 체크
                        마지막 건의 SEQ가 999999이면 배치 종료(D_End=1)

                      p_flag=0이면 Seq만 처리하고 FIFO 기록은 하지 않음
                      p_flag=1이면 RDS01용 FIFO(10), p_flag=2이면 RDS02용 FIFO(20)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Write_Data(int p_flag) {
    int     rt, seq, i, for_i, d_size, d_seq;
    char    m_time[24];
    char    w_data[2048];
    BUFF_RW_HEAD    f_head;

    memset(m_time, 0, sizeof (m_time));
    Get_MicroTime(m_time);

    /* 묶음 건수 및 건별 크기 계산 */
    for_i  = AtoIf(Header_Fmt.DataCnt,    sizeof (Header_Fmt.DataCnt));
    if (for_i <= 0) {
        Log(USR_ERROR, "Write_Data: invalid DataCnt[%d]", for_i);
        return;
    }
    d_size = AtoIf(Header_Fmt.BodyLength, sizeof (Header_Fmt.BodyLength)) / for_i;

    for (i = 0; i < for_i; i++) {
        /*
         * SEQ 연속성 체크:
         * 각 건의 데이터는 KRX 헤더 이후 d_size × i 위치에 있으며
         * 오프셋 5에서 6자리가 SEQ 번호
         * 마지막 건의 SEQ가 999999이면 D_End=1 (배치 종료)
         */
        d_seq = AtoIf(&DataBuff[KRX_HEAD_LEN + (d_size * i) + 5], 6);
        if ((i == for_i - 1) && (d_seq == 999999))
            D_End = 1;                      /* 배치 종료 표시 */

        if (INT_SEQ + 1 != d_seq && D_End == 0) {
            Log(USR_ERROR, "PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]",
                    d_seq, INT_SEQ);
            return;
        }

        if (D_End == 0)
            INT_SEQ += 1;                   /* SEQ 999999는 증가시키지 않음 */

        /* p_flag > 0이면 FIFO에 데이터 기록 */
        if (p_flag) {
            memset(w_data, 0x20, sizeof (w_data));
            memset(&File_Data_Head, ' ', HEAD_SIZE);
            memset(&f_head, 0x20, sizeof(f_head));

            /* BUFF_RW_HEAD (50바이트) 조립 */
            ItoAf(d_seq,          f_head.Seq,           sizeof(f_head.Seq));
            ItoAf(d_seq,          f_head.If_Seq,        sizeof(f_head.If_Seq));
            memcpy(f_head.ApType,         ApType,       sizeof(f_head.ApType));
            memcpy(f_head.ResponseCode,   RES_NORMAL,   strlen(RES_NORMAL));
            memcpy(f_head.RecvTime1,      m_time,       sizeof(f_head.RecvTime1));
            memcpy(f_head.RecvTime2,      &m_time[10],  sizeof(f_head.RecvTime2));

            /* FILE_DATA_HEAD (20바이트) 조립 */
            ItoAf(d_size, File_Data_Head.Length,  sizeof (File_Data_Head.Length));
            ItoAf(d_seq,  File_Data_Head.DataSeq, sizeof (File_Data_Head.DataSeq));
            memcpy(File_Data_Head.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
            memcpy(File_Data_Head.LineFlag,     _Exe_Name+4, 3);

            /* 헤더 + 데이터 조립 */
            memcpy(f_head.DataHeader, &File_Data_Head, sizeof(FILE_DATA_HEAD));
            memcpy(w_data,            &f_head,         sizeof(BUFF_RW_HEAD));

            /* 데이터: KRX 헤더 이후 d_size × i 위치부터 d_size 바이트 */
            memcpy(&w_data[sizeof(BUFF_RW_HEAD)],
                    &DataBuff[KRX_HEAD_LEN + (d_size * i)], d_size);
            w_data[sizeof(BUFF_RW_HEAD) + OFS(D_K,P_K,p_flag-1)] = '\n';

            /* FIFO 기록: p_flag × 10이 FIFO 인덱스 (10=RDS01, 20=RDS02) */
            rt = F_W(p_flag * 10, (void *)w_data, 1);
            if (rt != 1) {
                Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
                return;
            }

            Log(USR_OK, "file write[%s:%d:%d]",
                    OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);
        }

        Set_TR_Time();
    }

    return;
}   /* End of Write_Data () */

/*************************************************************************
    Function        : Make_Send_Msg
    Parameters IN   : tr_code - 전송할 KRX 메시지 유형
                      TR_LOON=로그인, TR_POLL=하트비트,
                      TR_LOOU=로그아웃, TR_DATA/RP_DATA=데이터응답
    Parameters OUT  : (없음, KR_Fmt에 결과 저장)
    Return Code     : int (0=성공, -1=데이터없음)
    Comment         : KRX 프로토콜 포맷으로 송신 메시지를 조립한다.
                      KRX 헤더(82바이트):
                        BeginString: "KMAPv2.0"
                        MsgType: TR별 코드
                        SenderCompID: 회원사 코드
                        SendingTime: 밀리초 시간
                        Encrypt: "N" (PA는 암호화 미사용)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Make_Send_Msg(int tr_code) {
    int     rt;
    char    d_time[18];

    rt = 0;
    memset(&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));

    /* KRX 헤더 공통부 조립 */
    memcpy(KR_Fmt.Header.BeginString, "KMAPv2.0", 8);
    ItoAf(0, KR_Fmt.Header.BodyLength, sizeof (KR_Fmt.Header.BodyLength));
    ItoAf(0, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
    memcpy(KR_Fmt.Header.SenderCompID, TCP_COMPANY, strlen(TCP_COMPANY));
    Get_DateMilliTime(d_time);
    memcpy(KR_Fmt.Header.SendingTime, d_time, sizeof (KR_Fmt.Header.SendingTime));
    ItoAf(0, KR_Fmt.Header.DataCnt, sizeof (KR_Fmt.Header.DataCnt));
    memcpy(KR_Fmt.Header.Encrypt, "N", 1);     /* PA: 암호화 미사용 */
    SendLen = KRX_HEAD_LEN;

    switch (tr_code) {
        case TR_LOON:                       /* 로그인 요청 (SCHLIQ00000) */
            memcpy(KR_Fmt.Header.MsgType, "SCHLIQ00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memset(KR_Fmt.Data, 0x20, 41);
            memcpy(KR_Fmt.Data,       LOGON_ID(D_K,P_K), 10);  /* ID */
            memcpy(&KR_Fmt.Data[10],  LOGON_PW(D_K,P_K), 30);  /* PW */
            memcpy(&KR_Fmt.Data[40],  "N",                 1); /* 암호화 여부 */
            MsgLen = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;

        case TR_POLL:                       /* 회선시험 요청 (SCHHEQ00000) */
            memcpy(KR_Fmt.Header.MsgType, "SCHHEQ00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data, "0000", 4);
            MsgLen = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;

        case TR_LOOU:                       /* 로그아웃 요청 (SCHLOR00000) */
            memcpy(KR_Fmt.Header.MsgType, "SCHLOR00000", 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            MsgLen = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;

        case TR_DATA:                       /* 데이터 수신 응답 */
        case RP_DATA:
            memcpy(KR_Fmt.Header.MsgType, Header_Fmt.MsgType, 11);
            ItoAf(INT_SEQ, KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy(KR_Fmt.Data,    "0000",  4);            /* 응답코드: 정상 */
            ItoAf(INT_SEQ, &KR_Fmt.Data[4], 11);           /* 처리 SEQ */
            MsgLen = strlen(KR_Fmt.Data);
            ItoAf(MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;

        default:
            break;
    }

    return (rt);
}   /* End of Make_Send_Msg () */

/*************************************************************************
    Function        : Log_Out
    Parameters IN   : (없음)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 로그아웃 처리. fep_common.c의 Log_Out_Base() 호출.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Out(void) {
    Log_Out_Base();
}   /* End of Log_Out () */

/*************************************************************************
    End of Program (pa_7800_tr.c)
*************************************************************************/
