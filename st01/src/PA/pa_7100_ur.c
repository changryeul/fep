#define     _GLOBAL
/*========================================================================
 *  Module  : 시세수신 (UDP 멀티캐스트)
 *  File    : pa_7100_ur.c
 *  -----------------------------------------------------------------
 *  KRX(거래소) UDP 멀티캐스트로 시세 데이터를 수신하여
 *  SHM(공유메모리)에 기록하고, FIFO를 통해 DD/전략 프로세스에 전달.
 *
 *  빌드 옵션 및 DATA_SIZE:
 *    A7102 : 채권KTS 시세 (700B) — A301K/G701K/B601K → Shm_Note
 *    A7103 : 채권KTS 시세 (100B) — 소형 TR용
 *    A7201 : 파생종목정보  (1400B) — A006F → Shm_FinFut
 *    A7291 : 파생종목정보 재처리 (1400B)
 *    A7202 : 파생시세     (450B) — A306F/G706F/B606F → Shm_FinFut
 *    A7203 : 파생시세     (100B) — 소형 TR용
 *
 *  수신 TR 코드 목록:
 *    채권(KTS): A301K(체결223B), G701K(호가643B), B601K(우선호가462B)
 *              A701K(장운영68B), M401K(장운영스케쥴83B)
 *    파생:     A006F(종목정보1318B), A306F(체결173B), G706F(호가431B)
 *              B606F(우선호가324B), M406F(장운영스케쥴)
 *
 *  처리 흐름:
 *    1) UDP 멀티캐스트 조인 (IP_ADD_MEMBERSHIP)
 *    2) recvfrom으로 데이터 수신
 *    3) Set_Sise로 SHM에 시세 기록
 *    4) 체결(Che_Gbn>0)이면 Write_Read_Fifo로 자동전략에 알림
 *    5) F_W로 FIFO에 기록 → DD 프로세스 전달
 *========================================================================*/

/*------------------------------------------------------------------------
 *  헤더 파일
 *------------------------------------------------------------------------*/
#include    "pa_struct.h"
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
 *  빌드별 DATA_SIZE 정의
 *
 *  채권KTS 시세:
 *    A7102: 700바이트 (체결/호가/우선호가 통합)
 *    A7103, A7203: 100바이트 (소형 TR)
 *
 *  파생 시세:
 *    A7201, A7291: 1400바이트 (종목정보 A006F)
 *    A7202: 450바이트 (파생 체결/호가/우선호가)
 *------------------------------------------------------------------------*/
#if defined A7102
#define     DATA_SIZE       700
#elif defined(A7103) || defined(A7203)
#define     DATA_SIZE       100
#elif defined(A7201) || defined(A7291)
#define     DATA_SIZE       1400
#elif defined A7202
#define     DATA_SIZE       450
#endif

#include    "buf_struct.h"      /* FILE_BUFF_FORMAT 구조체 (DATA_SIZE 의존) */

#include    <net/if.h>
#include    <ifaddrs.h>

/*------------------------------------------------------------------------
 *  HA(이중화) 관련 상수
 *------------------------------------------------------------------------*/
#if defined(A7102)
#define     HB_PORT_DEFAULT 50010   /* pa_7102_ur */
#elif defined(A7103)
#define     HB_PORT_DEFAULT 50011   /* pa_7103_ur */
#elif defined(A7201)
#define     HB_PORT_DEFAULT 50012   /* pa_7201_ur */
#elif defined(A7202)
#define     HB_PORT_DEFAULT 50013   /* pa_7202_ur */
#elif defined(A7203)
#define     HB_PORT_DEFAULT 50014   /* pa_7203_ur */
#else
#define     HB_PORT_DEFAULT 50010
#endif

#define     HB_SEND_INTVL   1       /* HB 전송 간격 (초) */
#define     HB_TIMEOUT      5       /* 절체 판단 타임아웃 (초) */
#define     HB_STABLE_COUNT 3       /* Failback 연속 수신 횟수 */
#define     HB_DATA_TIMEOUT 5       /* 시세 끊김 연속 횟수 → 절체 */

typedef struct {
    char    hb_mark[2];     /* "HB" */
    int     recv_cnt;       /* 직전 HB 이후 시세 수신 건수 */
} HB_PACKET;

#define     HB_PKT_LEN     sizeof(HB_PACKET)

#define     HA_PRIMARY      1
#define     HA_SECONDARY    2
#define     HA_STANDALONE   0

/*------------------------------------------------------------------------
 *  상수 정의
 *------------------------------------------------------------------------*/
#define     SVR_PORT_NO     UDP_PORT(D_K,P_K,0) /* UDP 수신 포트 (cfg/udpip.ini) */
#define     READ_BUF_SIZE   2048                    /* UDP 수신 버퍼 크기 */

#define     S_H_SIZE        5       /* TR코드 크기 (5바이트): "A301K" 등 */
#define     M_H_SIZE        27      /* 마스터 헤더: TR(5)+SEQ(8)+JCNT(6)+DATE(8) */

/*------------------------------------------------------------------------
 *  전역 변수
 *------------------------------------------------------------------------*/
int     Sockfd;                 /* UDP 소켓 파일디스크립터 */
int     FIFO_fd;                /* 자동전략 알림용 FIFO 파일디스크립터 */
int     tot_cq;                 /* 누적 체결 수량 */
int     Idx;                    /* SHM 종목 인덱스 (seq_no에서 추출) */
int     Che_Gbn;                /* 체결구분: 1=체결, 0=미해당, -1=무시 */
struct  ip_mreq     mreq;       /* 멀티캐스트 그룹 조인 구조체 */
char    TrCode[8];              /* 수신 TR 코드 (예: "A301K") */
char    ApType[10];             /* 업무식별코드 */
char    Order_St[512];          /* 주문상태 버퍼 */
struct sockaddr_in  SvrAddr, ClntAddr;  /* UDP 서버/클라이언트 주소 */

/*------------------------------------------------------------------------
 *  HA(이중화) 전역 변수
 *------------------------------------------------------------------------*/
int     ha_role      = HA_STANDALONE;
int     ha_active    = ON;              /* 기본: 단독 운영 = 항상 활성 */
int     hb_port      = HB_PORT_DEFAULT;
int     hb_sockfd    = -1;
time_t  hb_last_recv = 0;
time_t  hb_last_send = 0;
struct sockaddr_in hb_peer_addr;
char    ha_peer_ip[20];
int     hb_stable_cnt   = 0;
int     my_recv_cnt     = 0;
int     pri_recv_cnt    = 0;
int     pri_no_data_cnt = 0;

/*------------------------------------------------------------------------
 *  함수 프로토타입
 *------------------------------------------------------------------------*/
void    PA_7100_UR(void);
int     Init_Parameters(void);
int     Socket_Connect(void);
int     Recv_Data(char *);
int     Add_Count_UR(void);
int     Add_Count_DD(char *);
void    Set_Sise(char *);
void    Write_Read_Fifo(int);

/* HA(이중화) 함수 */
int     HA_Init(void);
void    HA_Heartbeat_Send(void);
void    HA_Check_Failover(void);

/*======================================================================
 *  main: 프로세스 진입점
 *======================================================================*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
    PA_7100_UR();
    Exit_Process();
}   /* End of main () */

/*======================================================================
 *  PA_7100_UR: 메인 처리 루틴
 *  ------------------------------------------------------------------
 *  1) 초기화 및 UDP 멀티캐스트 소켓 접속
 *  2) 수신 루프: recvfrom → Set_Sise(SHM 기록) → F_W(FIFO 기록)
 *  3) 체결 데이터면 자동전략 프로세스에 FIFO 알림
 *======================================================================*/
void    PA_7100_UR(void) {
    char    m_time[24];
    char    Wdt[30], W2_Fmt[512];
    char    r_buf[READ_BUF_SIZE];
    int     tr_gbn, rt, len, skip_rt;

    BUFF_RW_HEAD        f_head;
    FILE_BUFF_FORMAT    W_Fmt;

    rt = Init_Parameters();
    if (rt == NOTOK)
        return;

    rt = Socket_Connect();
    if (rt == NOTOK)
        return;

    SLog(USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

    /* HA 초기화 */
    rt = HA_Init();
    if (rt == NOTOK)
        SLog(USR_OK, "HA init failed, running as STANDALONE");

    /*--------------------------------------------------------------
     *  수신 루프: JOB_END까지 UDP 데이터 수신
     *--------------------------------------------------------------*/
    while (START_S != JOB_END) {

        /* HA heartbeat 처리 */
        if (ha_role == HA_PRIMARY)
            HA_Heartbeat_Send();
        else if (ha_role == HA_SECONDARY)
            HA_Check_Failover();

        memset(r_buf, 0, sizeof (r_buf));

        /* UDP 수신 (select 기반 타임아웃 1초) */
        rt = Recv_Data(r_buf);

        if (rt == 0) {
            continue;           /* 1초 타임아웃 → 루프 재진입 (HB 처리) */
        }
        else if (rt < 0) {
            SLog(UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);

            /* 재접속 시도 */
            rt = Socket_Connect();

            if (rt == NOTOK)
                return;

            continue;
        }

        my_recv_cnt++;  /* 시세 수신 건수 (HB 전송 시 리셋) */

        /* Standby → SHM 기록 + FIFO 전달 skip */
        if (ha_active == OFF) {
            continue;
        }

        /* TR코드 추출 (첫 5바이트) */
        memset(TrCode, 0, sizeof (TrCode));
        sprintf(TrCode, "%-5.5s", r_buf);

        /*
         * Set_Sise: TR코드에 따라 SHM에 시세 기록
         *   Che_Gbn: 1=체결관련, 0=미해당, -1=무시할 TR
         *   Idx:     SHM 종목 인덱스
         */
        Che_Gbn = Idx = 0;
        Set_Sise(r_buf);

        /*
         * 체결 데이터 수신시: 자동전략 프로세스에 FIFO 알림
         * S_Sise[x][Idx].auto_use > 0 = 해당 종목에 자동전략이 기동중
         */
        if (Che_Gbn > 0) {
#if defined A7102
            if (Shm_Risk[0].S_Sise[0][Idx].auto_use > 0)
                Write_Read_Fifo(1);
#elif defined A7202
            if (Shm_Risk[0].S_Sise[1][Idx].auto_use > 0)
                Write_Read_Fifo(1);
#endif
        }
        else if (Che_Gbn < 0) {
            continue;       /* 무시할 TR → 다음 수신 */
        }

        /*
         * FIFO(파일) 기록: DD 프로세스에 전달
         * FILE_BUFF_FORMAT: If_Seq + ApType + ResponseCode + RecvTime + Data
         */
        memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

        ItoAf(O_W_CNT1 + 1, W_Fmt.If_Seq,  sizeof (W_Fmt.If_Seq));
        memcpy(W_Fmt.ApType, ApType,           sizeof (W_Fmt.ApType));
        memcpy(W_Fmt.ResponseCode, RES_NORMAL, strlen(RES_NORMAL));
        memcpy(W_Fmt.RecvTime1, m_time,        sizeof (W_Fmt.RecvTime1));
        memcpy(W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
                sizeof (W_Fmt.RecvTime2));

        memset(W_Fmt.DataHeader, 0x20, 20+DATA_SIZE);
        memcpy(W_Fmt.Data, r_buf, strlen(r_buf));
        W_Fmt.LineFeed[0] = '\n';

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            SLog(SAM_FATAL, "file write fail[%s] rt[%d]", OFN(D_K,P_K,0), rt);
            close(Sockfd);
            return;
        }

        /* DD 카운트 증가 (sisetr.ini의 dd_count) */
        Add_Count_DD(TrCode);
    }

    /* 종료 시 HB 소켓 닫기 */
    if (hb_sockfd >= 0) {
        close(hb_sockfd);
        hb_sockfd = -1;
    }

    return;
}   /* End of PA_7100_UR () */

/*======================================================================
 *  Init_Parameters: 전역 변수 초기화
 *  ------------------------------------------------------------------
 *  - 업무식별코드(ApType) 생성
 *  - A7102/A7202: 자동전략 전달용 FIFO 오픈
 *======================================================================*/
int     Init_Parameters() {
    int     i, j, rt, flag;
    char    item_code[20], memberitem[10], tmp[128];
    char    d_time[16], head_size[10], cli_orsnd[21];
    char    fifo_name[100], bumun[4], m_time[24];

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /*--------------------------------------------------------------
     *  A7102/A7202: 전략 프로세스에 시세 알림용 FIFO 오픈
     *  시세 수신 → "1" write → 전략 프로세스가 SHM에서 최신 시세 참조
     *--------------------------------------------------------------*/
#if defined A7102
    sprintf(fifo_name, "%s/PA/pa_7102_ur1", _FEP_FIFO);
#elif defined A7202
    sprintf(fifo_name, "%s/PA/pa_7202_ur1", _FEP_FIFO);
#endif

#if defined(A7102) || defined(A7202)
    FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);
    if (FIFO_fd < 0)
        SLog(FIF_FATAL, "cannot open FIFO[%s][%d][%d:%s]",
                fifo_name, FIFO_fd, SYS_NO, SYS_STR);
#endif

    tot_cq = 0;     /* 누적 체결 수량 초기화 */

    return (OK);
}   /* End of Init_Parameters () */

/*======================================================================
 *  Socket_Connect: UDP 멀티캐스트 소켓 생성 및 그룹 조인
 *  ------------------------------------------------------------------
 *  1) SOCK_DGRAM 소켓 생성
 *  2) SO_REUSEADDR 설정 + 수신 버퍼 확대 (UDP_SOCK_RCVBUF_SIZE)
 *  3) bind로 멀티캐스트 주소에 바인드
 *  4) IP_ADD_MEMBERSHIP으로 멀티캐스트 그룹 조인
 *======================================================================*/
int     Socket_Connect(void) {
    int     rt, val, len;
    char    ip_addr[16];

    /* UDP 멀티캐스트 IP 주소: cfg/udpip.ini에서 읽어옴 */
    sprintf(ip_addr, "%d.%d.%d.%d", UDP_IP1(D_K,P_K,0), UDP_IP2(D_K,P_K,0),
            UDP_IP3(D_K,P_K,0), UDP_IP4(D_K,P_K,0));

    SLog(USR_OK, "Ip_Addr -> [%s]", ip_addr);

    /* 서버 주소 설정 */
    bzero((unsigned char *)&SvrAddr, sizeof (SvrAddr));
    SvrAddr.sin_family         = AF_INET;
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);
    SvrAddr.sin_port           = htons(SVR_PORT_NO);

    /* 멀티캐스트 그룹 설정 */
    bzero((unsigned char *)&mreq, sizeof (mreq));
    mreq.imr_multiaddr          = SvrAddr.sin_addr;
    mreq.imr_interface.s_addr   = htonl(INADDR_ANY);

    /* 1. UDP 소켓 생성 */
    Sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (Sockfd < 0) {
        SLog(UDP_FATAL, "socket open fail {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* 2-1. SO_REUSEADDR: 동일 포트 재사용 허용 */
    val = 1;
    len = sizeof (val);
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, len);
    if (rt == -1) {
        SLog(UDP_WARN, "setsockopt(SO_REUSEADDR) fail {%d:%s}", SYS_NO, SYS_STR);
    }

    /* 2-2. SO_RCVBUF: 수신 버퍼 크기 설정 (UDP_SOCK_RCVBUF_SIZE = 약 1200KB) */
    val = UDP_SOCK_RCVBUF_SIZE;
    len = sizeof (val);
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&val, len);
    if (rt == -1) {
        SLog(UDP_WARN, "setsockopt(SO_RCVBUF) fail {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* 3. 멀티캐스트 주소에 바인드 */
    if (bind(Sockfd, (struct sockaddr *)&SvrAddr, sizeof (SvrAddr)) < 0) {
        SLog(UDP_FATAL, "bind fail {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* 4. 멀티캐스트 그룹 조인 */
    rt = setsockopt(Sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (rt == -1) {
        SLog(UDP_WARN, "setsockopt(MULTICAST[%d]) fail {%d:%s}", mreq, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    return (OK);
}   /* End of Socket_Connect () */

/*======================================================================
 *  Recv_Data: UDP 데이터 수신 + HB 수신
 *  ------------------------------------------------------------------
 *  select로 1초 타임아웃 설정 후 recvfrom으로 수신.
 *  Secondary일 경우 hb_sockfd도 감시하여 HB 패킷 처리.
 *  반환값:
 *    > 0  = 시세 수신 바이트 수
 *    == 0 = 타임아웃 또는 HB만 도착 (시세 없음)
 *    < 0  = 오류
 *======================================================================*/
int     Recv_Data(char *p_str) {
    int             rt, len, maxfd;
#if defined __linux
    fd_set  read_set;
#else
    struct fd_set   read_set;
#endif
    struct timeval  timeout;

    len = sizeof (ClntAddr);

    FD_ZERO(&read_set);
    FD_SET(Sockfd, &read_set);
    maxfd = Sockfd;

    /* HB 소켓 추가 감시 (Secondary만) */
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY) {
        FD_SET(hb_sockfd, &read_set);
        if (hb_sockfd > maxfd)
            maxfd = hb_sockfd;
    }

    /* 타임아웃 1초 (기존 70초에서 변경) */
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    rt = select(maxfd + 1, &read_set, NULL, NULL, &timeout);

    if (rt < 0) {
        SLog(SYS_FATAL, "select fail {%d:%s}", SYS_NO, SYS_STR);
        return (NOTOK);
    }
    if (rt == 0) return (0);   /* 1초 타임아웃 */

    /* HB 수신 처리 (Secondary) */
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY &&
        FD_ISSET(hb_sockfd, &read_set)) {
        char hb_buf[32];
        struct sockaddr_in from_addr;
        socklen_t from_len;

        while (1) {
            fd_set hb_set;
            struct timeval tv_zero;

            from_len = sizeof(from_addr);
            rt = recvfrom(hb_sockfd, hb_buf, sizeof(hb_buf), 0,
                          (struct sockaddr *)&from_addr, &from_len);
            if (rt < 0) {
                SLog(UDP_WARN, "HA HB recvfrom fail(%d:%s)", SYS_NO, SYS_STR);
                break;
            }
            if (rt >= (int)HB_PKT_LEN && memcmp(hb_buf, "HB", 2) == 0) {
                HB_PACKET *p = (HB_PACKET *)hb_buf;
                pri_recv_cnt = p->recv_cnt;
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
                SLog(USR_OK, "HA HB recv ok (pri_cnt=%d, stable=%d)",
                    pri_recv_cnt, hb_stable_cnt);
            }
            else if (rt >= 2 && memcmp(hb_buf, "HB", 2) == 0) {
                pri_recv_cnt = -1;
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
            }

            /* 추가 패킷 확인 */
            FD_ZERO(&hb_set);
            FD_SET(hb_sockfd, &hb_set);
            tv_zero.tv_sec = 0;
            tv_zero.tv_usec = 0;
            if (select(hb_sockfd + 1, &hb_set, NULL, NULL, &tv_zero) <= 0)
                break;
        }
    }

    /* 시세 데이터 수신 */
    if (FD_ISSET(Sockfd, &read_set)) {
#if defined __linux
        rt = recvfrom(Sockfd, p_str, READ_BUF_SIZE, 0,
                (struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#else
        rt = recvfrom(Sockfd, p_str, READ_BUF_SIZE, NULL,
                (struct sockaddr *)&ClntAddr, (socklen_t *)&len);
#endif
        return (rt);
    }

    return (0);     /* HB만 도착, 시세 없음 */
}   /* End of Recv_Data () */

/*======================================================================
 *  Add_Count_UR: 시세 TR별 수신 카운트 증가
 *  ------------------------------------------------------------------
 *  sisetr.ini의 TR 목록에서 TrCode를 찾아 ur_count++
 *  반환값: 데이터 길이 또는 NOTOK(-1)/0(미사용)
 *======================================================================*/
int     Add_Count_UR(void) {
    int     rt;
    int     i;

    rt = NOTOK;
    T_K = NOTOK;

    for (i = 0; i < DAEMON(D_K).sisetr_count; i ++) {
        if (SISETR(D_K,i).tr[0] == '\0')
            break;

        if (memcmp(SISETR(D_K,i).tr, TrCode, 5) == 0) {
            if (SISETR(D_K,i).queue == 0)       /* 미사용 TR */ {
                rt = 0;
                break;
            }
            else {
                SISETR(D_K,i).ur_count ++;

                T_K = i;
                rt = SISETR(D_K,T_K).length;
                break;
            }
        }
    }

    return (rt);
}   /* End of Add_Count_UR () */

/*======================================================================
 *  Add_Count_DD: 시세 TR별 분배(DD) 카운트 증가
 *======================================================================*/
int     Add_Count_DD(char *p_tr) {
    int         rt = NOTOK;
    register int    i;

    for (i = 0; i < DAEMON(D_K).sisetr_count; i ++) {
        if (SISETR(D_K,i).tr[0] == '\0')
            break;

        if (memcmp(SISETR(D_K,i).tr, p_tr, strlen(p_tr)) == 0) {
            SISETR(D_K,i).dd_count ++;

            rt = SISETR(D_K,i).length;
            break;
        }
    }

    return (rt);
}   /* End of Add_Count_DD () */

/*======================================================================
 *  Set_Sise: TR코드별로 SHM에 시세 데이터 기록
 *  ------------------------------------------------------------------
 *  빌드별 처리:
 *
 *  [A7201] 파생 종목정보:
 *    A006F만 처리 → 기타 무시 (Che_Gbn=-1)
 *
 *  [A7102/A7103] 채권KTS 시세:
 *    A301K(체결)   → Shm_Note[Idx].A3, S_Sise[0][Idx].crprc
 *    G701K(호가)   → Shm_Note[Idx].G7, crprc/sell_1/buy_1
 *    B601K(우선호가)→ Shm_Note[Idx].B6, sell_1/buy_1
 *    M401K(장운영)  → Shm_Note[0].M4
 *    A701K(장운영)  → Shm_Note[0].A7
 *
 *  [A7202/A7203] 파생 시세:
 *    A306F(체결)   → Shm_FinFut[Idx].A3, crprc/상하한가
 *    G706F(호가)   → Shm_FinFut[Idx].G7, crprc/상하한가/매도1/매수1
 *    B606F(우선호가)→ Shm_FinFut[Idx].B6, sell_1/buy_1
 *    M406F(장운영)  → Shm_FinFut[0].M4
 *
 *  파생 가격: 첫 바이트가 '-'이면 음수, sign 변수로 부호 처리
 *======================================================================*/
void    Set_Sise(char *p_buf) {
    int         s_k, sign;

    /*--------------------------------------------------------------
     *  [A7201] 파생 종목정보: A006F만 처리
     *--------------------------------------------------------------*/
#if defined A7201
    if (memcmp(p_buf, "A006F", 5) != 0) {
        Che_Gbn = -1;
        return;
    }

    /*--------------------------------------------------------------
     *  [A7102/A7103] 채권KTS 시세
     *--------------------------------------------------------------*/
#elif defined(A7102) || defined(A7103)
    if (memcmp(p_buf, "A301K", 5) == 0) {
        /* A301K: 채권 체결 (223바이트) */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].A3.seq_no));
        memcpy(Shm_Note[Idx].A3.tr_gbn, p_buf,     sizeof (CO_A301K));

        /* 공용 현재가(체결가) */
        Shm_Risk[0].S_Sise[0][Idx].crprc =
        (double)AtoDf(Shm_Note[Idx].A3.crprc,      sizeof (Shm_Note[0].A3.crprc));
    }
    else if (memcmp(p_buf, "G701K", 5) == 0) {
        /* G701K: 채권 호가 (643바이트) — 체결가+매도1호가+매수1호가 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].G7.seq_no));
        Shm_Note[Idx].HogaLastGbn = 0;      /* 0=G7(호가) 최신 */
        memcpy(Shm_Note[Idx].G7.tr_gbn, p_buf,     sizeof (CO_G701K));

        Shm_Risk[0].S_Sise[0][Idx].crprc =
        (double)AtoDf(Shm_Note[Idx].G7.crprc,      sizeof (Shm_Note[0].G7.crprc));
        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
        (double)AtoDf(Shm_Note[Idx].G7.ask1_price,sizeof (Shm_Note[0].G7.ask1_price));
        Shm_Risk[0].S_Sise[0][Idx].buy_1_price =
        (double)AtoDf(Shm_Note[Idx].G7.bid1_price,sizeof (Shm_Note[0].G7.bid1_price));
    }
    else if (memcmp(p_buf, "B601K", 5) == 0) {
        /* B601K: 채권 우선호가 (462바이트) — 매도1호가+매수1호가 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].B6.seq_no));
        Shm_Note[Idx].HogaLastGbn = 1;      /* 1=B6(우선호가) 최신 */
        memcpy(Shm_Note[Idx].B6.tr_gbn, p_buf,     sizeof (CO_B601K));

        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
        (double)AtoDf(Shm_Note[Idx].B6.ask1_price,sizeof (Shm_Note[0].B6.ask1_price));
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(Shm_Note[Idx].B6.bid1_price,sizeof (Shm_Note[0].B6.bid1_price));
    }
    else if (memcmp(p_buf, "M401K", 5) == 0) {
        /* M401K: 장운영 스케쥴 (83바이트) */
        memcpy(Shm_Note[0].M4.tr_gbn, p_buf,           sizeof (CO_M401K));
    }
    else if (memcmp(p_buf, "A701K", 5) == 0) {
        /* A701K: 장운영 정보 (68바이트) */
        memcpy(Shm_Note[0].A7.tr_gbn, p_buf,           sizeof (CO_A701A));
    }
    else {
        Log(USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }

    /*--------------------------------------------------------------
     *  [A7202/A7203] 파생 시세
     *  주의: 파생 가격은 첫 바이트가 부호('-' 또는 space)
     *        → 부호 제거 후 AtoDf 변환, sign으로 부호 복원
     *--------------------------------------------------------------*/
#elif defined(A7202) || defined(A7203)
    if (memcmp(p_buf, "A306F", 5) == 0) {
        /* A306F: 파생 체결 (173바이트) — 현재가+실시간상하한가 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
        memcpy(Shm_FinFut[Idx].A3.tr_gbn, p_buf,       sizeof (CO_A301F));

        /* 현재가: 부호 처리 */
        if (memcmp(Shm_FinFut[Idx].A3.crprc, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].crprc =
        sign *
        (double)AtoDf(&Shm_FinFut[Idx].A3.crprc[1],sizeof (Shm_FinFut[0].A3.crprc) - 1);

        /* 실시간 상한가 */
        if (memcmp(Shm_FinFut[Idx].A3.dyn_upper_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
        (double)AtoDf(&Shm_FinFut[Idx].A3.dyn_upper_limit[1],
                sizeof (Shm_FinFut[0].A3.dyn_upper_limit) - 1);

        /* 실시간 하한가 */
        if (memcmp(Shm_FinFut[Idx].A3.dyn_lower_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
        (double)AtoDf(&Shm_FinFut[Idx].A3.dyn_lower_limit[1],
                sizeof (Shm_FinFut[0].A3.dyn_lower_limit) - 1);
    }
    else if (memcmp(p_buf, "G706F", 5) == 0) {
        /* G706F: 파생 호가 (431바이트) — 현재가+상하한가+매도1/매수1 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
        Shm_FinFut[Idx].HogaLastGbn = 0;
        memcpy(Shm_FinFut[Idx].G7.tr_gbn, p_buf,       sizeof (CO_G701F));

        /* 현재가 */
        if (memcmp(Shm_FinFut[Idx].G7.crprc, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].crprc =
        sign *
        (double)AtoDf(&Shm_FinFut[Idx].G7.crprc[1],sizeof (Shm_FinFut[0].G7.crprc) - 1);

        /* 실시간 상한가 */
        if (memcmp(Shm_FinFut[Idx].G7.dyn_upper_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_hprc =
        (double)AtoDf(&Shm_FinFut[Idx].G7.dyn_upper_limit[1],
                sizeof(Shm_FinFut[0].G7.dyn_upper_limit) - 1);

        /* 실시간 하한가 */
        if (memcmp(Shm_FinFut[Idx].G7.dyn_lower_limit, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].realtime_lprc =
        (double)AtoDf(&Shm_FinFut[Idx].G7.dyn_lower_limit[1],
                sizeof(Shm_FinFut[0].G7.dyn_lower_limit) - 1);

        /* 매도1호가 */
        if (memcmp(Shm_FinFut[Idx].G7.ask1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].G7.ask1_price[1],
                sizeof(Shm_FinFut[0].G7.ask1_price) - 1);

        /* 매수1호가 */
        if (memcmp(Shm_FinFut[Idx].G7.bid1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].G7.bid1_price[1],
                sizeof(Shm_FinFut[0].G7.bid1_price) - 1);
    }
    else if (memcmp(p_buf, "B606F", 5) == 0) {
        /* B606F: 파생 우선호가 (324바이트) — 매도1/매수1 */
        Che_Gbn = 1;
        Idx = AtoIf(&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
        Shm_FinFut[Idx].HogaLastGbn = 1;
        memcpy(Shm_FinFut[Idx].B6.tr_gbn, p_buf, sizeof (CO_B601F));

        /* 매도1호가 */
        if (memcmp(Shm_FinFut[Idx].B6.ask1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].sell_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].B6.ask1_price[1],
                sizeof(Shm_FinFut[0].B6.ask1_price) - 1);

        /* 매수1호가 */
        if (memcmp(Shm_FinFut[Idx].B6.bid1_price, "-", 1) == 0)
            sign = -1;
        else
            sign = 1;
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
        (double)AtoDf(&Shm_FinFut[Idx].B6.bid1_price[1],
                sizeof(Shm_FinFut[0].B6.bid1_price) - 1);
    }
    else if (memcmp(p_buf, "M406F", 5) == 0) {
        /* M406F: 파생 장운영 스케쥴 */
        memcpy(Shm_FinFut[0].M4.tr_gbn, p_buf, sizeof (CO_A701A));
    }
    else {
        Log(USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }
#endif

    return;
}   /* End of Set_Sise () */

/*======================================================================
 *  Write_Read_Fifo: 자동전략 프로세스에 시세 알림
 *  ------------------------------------------------------------------
 *  FIFO에 "1" 기록 → 전략 프로세스가 깨어남 → SHM에서 최신 시세 참조
 *  기록 후 FIFO 잔여 데이터 drain (자기가 쓴 것 읽어서 비움)
 *======================================================================*/
void    Write_Read_Fifo(int c0h0) {
    int     rt;
    char    tmp[128];

    rt = write(FIFO_fd, "1", 1);

    if (rt < 0)
        SLog(FIF_FATAL, "cannot write FIFO[%d][%d:%s]",
                FIFO_fd, SYS_NO, SYS_STR);

    /* FIFO drain: 자기가 쓴 데이터 읽어서 비움 */
    while (1) {
        rt = read(FIFO_fd, tmp, sizeof(tmp));
#if defined __linux
        if (rt == 0 || errno == EAGAIN)
#else
            if (rt == 0)
#endif
            break;
    }

    return;
}   /* End of Write_Read_Fifo () */

/*======================================================================
 *  HA_Init: HA 이중화 초기화
 *  ------------------------------------------------------------------
 *  _FEP_DIV로 역할 결정 (REAL1=Primary, REAL2=Secondary)
 *  HB용 UDP 소켓 생성 및 설정
 *======================================================================*/
int     HA_Init(void) {
    int rt, val;
    char *peer_ip;

    /* 1. _FEP_DIV로 역할 결정 */
    if (memcmp(_FEP_DIV, "REAL1", 5) == 0) {
        ha_role   = HA_PRIMARY;
        ha_active = ON;
        SLog(USR_OK, "HA role: PRIMARY (always active)");
    }
    else if (memcmp(_FEP_DIV, "REAL2", 5) == 0) {
        ha_role   = HA_SECONDARY;
        ha_active = OFF;
        SLog(USR_OK, "HA role: SECONDARY (standby)");
    }
    else {
        ha_role   = HA_STANDALONE;
        ha_active = ON;
        SLog(USR_OK, "HA role: STANDALONE (no HA)");
        return (OK);
    }

    /* 2. peer IP */
    peer_ip = (char *)getenv("_HA_PEER_IP");
    if (peer_ip == NULL || peer_ip[0] == '\0') {
        SLog(UDP_ERROR, "HA _HA_PEER_IP not set");
        return (NOTOK);
    }
    strncpy(ha_peer_ip, peer_ip, sizeof(ha_peer_ip) - 1);
    ha_peer_ip[sizeof(ha_peer_ip) - 1] = '\0';

    /* 3. HB 포트 (PA 전용 환경변수) */
    {
        char *port_str = (char *)getenv("_PA_HA_HB_PORT");
        if (port_str != NULL && port_str[0] != '\0') {
            int base_port = atoi(port_str);
            if (base_port > 0 && base_port <= 65535) {
#if defined(A7102)
                hb_port = base_port;        /* +0 */
#elif defined(A7103)
                hb_port = base_port + 1;    /* +1 */
#elif defined(A7201)
                hb_port = base_port + 2;    /* +2 */
#elif defined(A7202)
                hb_port = base_port + 3;    /* +3 */
#elif defined(A7203)
                hb_port = base_port + 4;    /* +4 */
#else
                hb_port = base_port;
#endif
            }
        }
    }

    /* 4. UDP 소켓 생성 */
    hb_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hb_sockfd < 0) {
        SLog(UDP_ERROR, "HA HB socket fail (%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    val = 1;
    setsockopt(hb_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));

    /* 5. peer 주소 설정 */
    memset(&hb_peer_addr, 0, sizeof(hb_peer_addr));
    hb_peer_addr.sin_family = AF_INET;
    hb_peer_addr.sin_port   = htons(hb_port);

    if (ha_role == HA_PRIMARY) {
        inet_pton(AF_INET, ha_peer_ip, &hb_peer_addr.sin_addr);
    }
    else {
        /* Secondary: bind for receiving */
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family      = AF_INET;
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind_addr.sin_port        = htons(hb_port);

        if (bind(hb_sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            SLog(UDP_ERROR, "HA HB bind fail (%d:%s)", SYS_NO, SYS_STR);
            close(hb_sockfd);
            hb_sockfd = -1;
            return (NOTOK);
        }
        hb_last_recv = time(NULL);
    }

    SLog(USR_OK, "HA init OK: role=%d active=%d hb_fd=%d hb_port=%d peer=%s",
        ha_role, ha_active, hb_sockfd, hb_port, ha_peer_ip);

    return (OK);
}   /* End of HA_Init () */

/*======================================================================
 *  HA_Heartbeat_Send: Primary → Secondary HB 전송 (1초 간격)
 *======================================================================*/
void    HA_Heartbeat_Send(void) {
    time_t now;
    int    rt;
    HB_PACKET pkt;

    now = time(NULL);
    if ((now - hb_last_send) < HB_SEND_INTVL)
        return;

    memcpy(pkt.hb_mark, "HB", 2);
    pkt.recv_cnt = my_recv_cnt;
    my_recv_cnt = 0;

    rt = sendto(hb_sockfd, (char *)&pkt, HB_PKT_LEN, 0,
                (struct sockaddr *)&hb_peer_addr, sizeof(hb_peer_addr));

    if (rt < 0)
        SLog(UDP_WARN, "HA HB send fail (%d:%s)", SYS_NO, SYS_STR);
    else
        SLog(USR_OK, "HA HB sent (rt=%d, cnt=%d, to=%s:%d)",
            rt, pkt.recv_cnt, ha_peer_ip, hb_port);

    hb_last_send = now;
}   /* End of HA_Heartbeat_Send () */

/*======================================================================
 *  HA_Check_Failover: Secondary 절체/복귀 판단
 *  ------------------------------------------------------------------
 *  Standby 상태에서:
 *    FAILOVER 1: HB 타임아웃 → Active 전환
 *    FAILOVER 2: Primary 시세 끊김 (5연속) → Active 전환
 *  Active 상태에서 (절체 후):
 *    FAILBACK: HB 3회 연속 수신 + Primary 시세 정상 → Standby 복귀
 *======================================================================*/
void    HA_Check_Failover(void) {
    time_t now;
    int    elapsed, my_data;

    now     = time(NULL);
    elapsed = (int)(now - hb_last_recv);
    my_data = my_recv_cnt;
    my_recv_cnt = 0;

    if (ha_active == OFF) {
        /* FAILOVER 1: HB timeout */
        if (elapsed >= HB_TIMEOUT) {
            ha_active = ON;
            hb_stable_cnt = 0;
            pri_no_data_cnt = 0;
            SLog(USR_OK, "HA FAILOVER: HB timeout (%dsec)", elapsed);
            return;
        }
        /* FAILOVER 2: Primary 시세 끊김 */
        if (pri_recv_cnt == 0 && my_data > 0) {
            pri_no_data_cnt++;
            if (pri_no_data_cnt >= HB_DATA_TIMEOUT) {
                ha_active = ON;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                SLog(USR_OK, "HA FAILOVER: Primary no data (%d consecutive)",
                    HB_DATA_TIMEOUT);
            }
        } else {
            pri_no_data_cnt = 0;
        }
    }
    else {
        /* FAILBACK */
        if (elapsed >= HB_TIMEOUT) {
            hb_stable_cnt = 0;
        }
        else if (hb_stable_cnt >= HB_STABLE_COUNT) {
            if (pri_recv_cnt > 0 || my_data == 0) {
                ha_active = OFF;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                SLog(USR_OK, "HA FAILBACK: Primary stable (%d HB, pri=%d)",
                    HB_STABLE_COUNT, pri_recv_cnt);
            } else {
                hb_stable_cnt = 0;
                SLog(USR_OK, "HA FAILBACK blocked: Primary no data (pri=%d, my=%d)",
                    pri_recv_cnt, my_data);
            }
        }
    }
}   /* End of HA_Check_Failover () */

/*========================================================================
 *  End of program (pa_7100_ur.c)
 *========================================================================*/
