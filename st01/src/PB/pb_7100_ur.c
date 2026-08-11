#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 채권 시세수신 (UDP 멀티캐스트)
 *  File    : pb_7100_ur.c
 *
 *  KRX로부터 UDP 멀티캐스트로 채권 시세를 수신하여
 *  TCP로 연결된 서버에 재전송하는 프로세스.
 *
 *  빌드 옵션:
 *    B7102 : 채권시세 (700B) - 체결+호가 데이터
 *    B7103 : 채권시세 (100B) - 장운영 데이터
 *
 *  처리 흐름:
 *    1) UDP 멀티캐스트 소켓 생성 및 Join (Socket_Connect)
 *    2) TCP 서버 연결 (Device_Open)
 *    3) UDP 수신 데이터를 TR코드별 분류
 *    4) A3/G7/B6/M4 → TCP 전달, 기타 → LK 응답 전송
 *
 *  네트워크 구성:
 *    - UDP: 멀티캐스트 그룹 조인 (udpip.ini 설정)
 *    - TCP: 데이터 재전송용 서버 연결 (tcp2.ini 설정)
 *    - NIC: 개발서버=ens7f3, 운영서버=ens6f3
 *
 *  HA(이중화) 구성:
 *    - 2대 서버(REAL1/REAL2)에서 동시에 UDP 수신
 *    - Primary(REAL1)만 TCP 전송, Secondary(REAL2)는 대기
 *    - Primary 장애 시 Secondary가 자동으로 TCP 전송 시작
 *    - Primary 복구 시 Secondary가 자동으로 대기 복귀
 *    - 역할 결정: _FEP_DIV 환경변수 (REAL1=Primary, REAL2=Secondary)
 *    - 절체 판단: UDP heartbeat (Primary→Secondary, 1초 간격)
 *------------------------------------------------------------------------*/

#include    "pa_struct.h"
#include    "fep_fepp.h"

#include    <net/if.h>
#include    <ifaddrs.h>

/* 기본정보채권 KTS 시세 TR코드 목록
 * A701K : 68   장운영 TS
 * M401K : 83   장운영스케줄
 * B601K : 462  일반채권, 국고채권 우선호가
 * A301K : 223  채권체결
 * G701K : 643  일반채권, 국고채권 체결 + 우선호가
 * A601K : 57   채권종목마감
 */

/* 12000: 10 Polling, Log만 File온 처리안함 */

/*------------------------------------------------------------------------
 *  빌드 옵션별 데이터 크기
 *------------------------------------------------------------------------*/
#if defined B7102
#define     DATA_SIZE   700     /* 채권시세(체결+호가) */
#elif defined B7103
#define     DATA_SIZE   100     /* 채권시세(장운영) */
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/
#define     SVR_PORT_NO     UDP_PORT(D_K, P_K, 0)   /* UDP 수신 포트 번호 */
#define     READ_BUF_SIZE   2048                    /* 수신 버퍼 크기 */

/* 시세 Seq 위치 Size */
#define     SH_SIZE     5       /* TR코드(5바이트) */
/* 기본마스터 Seq 위치 Size */
#define     M_H_SIZE    27      /* TR(5) + SEQ(8) + JCNT(6) + DATE(8) */

/*------------------------------------------------------------------------
 *  HA(이중화) 관련 상수
 *------------------------------------------------------------------------
 *  HB = Heartbeat
 *
 *  Primary(REAL1)는 매 루프마다 Secondary에게 heartbeat UDP를 전송한다.
 *  Secondary(REAL2)는 heartbeat 수신 여부로 Primary 생존을 판단한다.
 *
 *  HB_PORT       : heartbeat 송수신에 사용하는 UDP 포트
 *  HB_SEND_INTVL : Primary가 heartbeat를 보내는 간격 (초)
 *  HB_TIMEOUT    : Secondary가 "Primary 장애"로 판단하는 타임아웃 (초)
 *  HB_MSG        : heartbeat 패킷 내용 (고정 문자열)
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 *  HB 포트: 프로세스별 분리 (pb_7102_ur=50000, pb_7103_ur=50001)
 *  같은 서버에서 두 프로세스 동시 실행 시 포트 충돌 방지
 *------------------------------------------------------------------------*/
#if defined B7102
#define     HB_PORT_DEFAULT 50000   /* heartbeat UDP 포트 (pb_7102_ur)   */
#elif defined B7103
#define     HB_PORT_DEFAULT 50001   /* heartbeat UDP 포트 (pb_7103_ur)   */
#else
#define     HB_PORT_DEFAULT 50000   /* heartbeat UDP 포트 기본값         */
#endif
#define     HB_SEND_INTVL   1       /* heartbeat 전송 간격 (초)          */
#define     HB_TIMEOUT      5       /* heartbeat 미수신 → 절체 판단 (초) */
#define     HB_STABLE_COUNT 3       /* Failback 조건: 연속 수신 횟수     */
#define     HB_DATA_TIMEOUT 5       /* Primary 시세 끊김 연속 N회 → 절체 */

/*------------------------------------------------------------------------
 *  HB 패킷 구조
 *  - hb_mark[2]  : "HB" 고정 문자열
 *  - recv_cnt    : Primary가 직전 1초간 수신한 시세 건수
 *                  Secondary는 이 값으로 Primary 시세 수신 상태를 판단
 *------------------------------------------------------------------------*/
typedef struct {
    char    hb_mark[2];     /* "HB"                              */
    int     recv_cnt;       /* 직전 HB 이후 시세 수신 건수       */
} HB_PACKET;

#define     HB_PKT_LEN     sizeof(HB_PACKET)

/*------------------------------------------------------------------------
 *  HA 역할 정의
 *------------------------------------------------------------------------
 *  HA_PRIMARY   : TCP 전송을 담당하는 주 서버 (REAL1/podm11)
 *  HA_SECONDARY : 대기 서버, Primary 장애 시 자동 절체 (REAL2/podm12)
 *  HA_STANDALONE: 이중화 미사용 (TEST 환경 등, 단독 운영)
 *------------------------------------------------------------------------*/
#define     HA_PRIMARY      1       /* 주 서버 (항상 TCP 전송)           */
#define     HA_SECONDARY    2       /* 대기 서버 (장애 시에만 TCP 전송)  */
#define     HA_STANDALONE   0       /* 단독 운영 (이중화 미사용)         */

/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
struct sockaddr_in SvrAddr, ClntAddr;   /* UDP 서버/클라이언트 주소 */

int     Sockfd, FIFO_fd, tot_cq;        /* UDP 소켓 fd, FIFO fd, 누적체결수량 */
int     SockTcpfd = -1, PortNo;         /* TCP 소켓 fd, TCP 포트 번호 */
int     Idx, Che_Gbn;                   /* SHM 인덱스, 체결구분 */
struct  ip_mreq mreq;                   /* 멀티캐스트 요청 구조체 */
char    TrCode[8], ApType[10], Order_St[512], IpAddr[20];
char    LogOnFlag, OpenFlag;            /* 로그온/오픈 상태 플래그 */
char    RecvPkt[CLI_BUFF_MAX_LEN], SendPkt[CLI_BUFF_MAX_LEN];

/*------------------------------------------------------------------------
 *  HA(이중화) 전역 변수
 *------------------------------------------------------------------------
 *  ha_role       : 현재 서버의 HA 역할 (HA_PRIMARY / HA_SECONDARY / HA_STANDALONE)
 *  ha_active     : TCP 전송 활성화 여부 (ON=전송, OFF=대기)
 *  hb_sockfd     : heartbeat 송수신용 UDP 소켓 fd
 *  hb_last_recv  : Secondary가 마지막으로 heartbeat를 수신한 시각 (time_t)
 *  hb_last_send  : Primary가 마지막으로 heartbeat를 전송한 시각 (time_t)
 *  hb_peer_addr  : heartbeat 상대 서버 주소 (Primary→Secondary 전송용)
 *  ha_peer_ip    : 상대 서버 IP (cfg의 Tcp2 설정에서 가져옴)
 *------------------------------------------------------------------------*/
int     ha_role      = HA_STANDALONE;   /* 기본값: 단독 운영                 */
int     ha_active    = ON;              /* 기본값: TCP 전송 활성 (단독 운영)  */
int     hb_port      = HB_PORT_DEFAULT; /* heartbeat 포트 (환경변수로 변경 가능) */
int     hb_sockfd    = -1;              /* heartbeat 소켓 fd                 */
time_t  hb_last_recv = 0;              /* 마지막 heartbeat 수신 시각         */
time_t  hb_last_send = 0;              /* 마지막 heartbeat 전송 시각         */
struct sockaddr_in hb_peer_addr;       /* heartbeat 상대 서버 주소           */
char    ha_peer_ip[20];                /* 상대 서버 IP 문자열                */
int     hb_stable_cnt = 0;            /* heartbeat 연속 수신 카운터         */
int     my_recv_cnt   = 0;            /* 자기 시세 수신 건수 (매 HB 리셋)  */
int     pri_recv_cnt  = 0;            /* Primary의 시세 수신 건수 (HB수신) */
int     pri_no_data_cnt = 0;          /* Primary 시세 없음 연속 횟수       */
time_t  tcp_last_try    = 0;          /* TCP 재연결 마지막 시도 시각        */
#define TCP_RETRY_INTVL 5             /* TCP 재연결 시도 간격 (초)          */

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
void    PB_7100_UR(void);
int     Init_Parameters(void);
int     Socket_Connect(void);
int     Recv_Data(char *);
int     Add_Count_UR(void);
int     Add_Count_DD(char *);
int     find_ifname_by_192(char *ifnm);

/* HA(이중화) 함수 */
int     HA_Init(void);
void    HA_Heartbeat_Send(void);
void    HA_Check_Failover(void);

/*------------------------------------------------------------------------
 *  Device_Open: TCP 서버 연결
 *  - Socket() → Connect()로 TCP 서버에 접속
 *  - TEST/REAL 환경별로 접속 IP/포트가 다름
 *------------------------------------------------------------------------*/
void    Device_Open(void) {
    int rt, rval;

    SockTcpfd = Socket();
    if (SockTcpfd < 0) {
        Log(TCP_ERROR, "socket fail:Sockfd [%d] (%d:%s)", SockTcpfd, SYS_NO, SYS_STR);
        return;
    }

    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0), TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));

    Log(USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    /* Connect2: alarm 기반 2초 타임아웃으로 연결 시도.
     * Connect()(타임아웃 없음) 사용 시, TCP 서버 미응답이면
     * connect() 시스템콜이 ~75초 블로킹 → 메인루프 정지 →
     * heartbeat 전송 불가 → Secondary 오판 절체 발생.
     * 2초면 HB 1~2회 누락으로 절체 유발하지 않음 (HB_TIMEOUT=5초). */
    rt = Connect2(SockTcpfd, IpAddr, TCP2_PORT_NO, 2);
    if (rt < 0) {
        Log(TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close(SockTcpfd);
        SockTcpfd = -1;
        return;
    }

    OpenFlag = LogOnFlag = ON;

    return;
} /* End of Device_Open() */

/*------------------------------------------------------------------------
 *  Device_Close: TCP 연결 종료 (에러/정상 종료용)
 *  - 소켓 닫기 → sleep(3) 재연결 대기 → 상태 플래그 초기화
 *------------------------------------------------------------------------*/
void Device_Close(void) {
    close(SockTcpfd);
    SockTcpfd = -1;
    /* sleep(3) 제거: HA 환경에서 메인루프 3초 블록은
     * heartbeat 전송/수신을 차단하여 오판 절체 유발.
     * select 1초 타임아웃이 재연결 간격 역할을 함. */
    Log(TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
} /* End of Device_Close() */

/*------------------------------------------------------------------------
 *  Device_Close_HA: TCP 연결 종료 (HA 절체용, sleep 없음)
 *  - HA failback 시 호출: sleep(3) 없이 즉시 TCP만 닫는다.
 *  - sleep(3) 포함 시 heartbeat 수신 차단 → 재 failover 진동 발생 (FR-08)
 *------------------------------------------------------------------------*/
void Device_Close_HA(void) {
    close(SockTcpfd);
    SockTcpfd = -1;
    Log(TCP_OK, "TCP device close (HA failback, no sleep)");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

    return;
} /* End of Device_Close_HA() */

/*------------------------------------------------------------------------
 *  Device_Write: TCP 데이터 전송
 *  - SendPkt 버퍼의 내용을 TCP로 전송
 *  - 전송 실패 시 Device_Close()
 *------------------------------------------------------------------------*/
void Device_Write(void) {
    int rt;

    rt = Select_Send(SockTcpfd, SendPkt, strlen(SendPkt));

    if (rt != OK) {
        Log(TCP_ERROR, "TCP data send fail");
        Device_Close();
    }

    INT_SEQ++;
    Log(TCP_OK, "TCP SD[%s] (%d)<%d>", SendPkt, strlen(SendPkt), INT_SEQ);

    return;
} /* End of Device_Write() */

/*------------------------------------------------------------------------
 *  main: 프로세스 초기화 → 시세수신 루프 → 종료
 *------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
    PB_7100_UR();
    Exit_Process();
} /* End of main () */

/*------------------------------------------------------------------------
 *  PB_7100_UR: 메인 이벤트 루프
 *    1) 초기화 (Init_Parameters, Socket_Connect, Device_Open)
 *    2) UDP 데이터 수신 (Recv_Data)
 *    3) HA 역할에 따라 TCP 전송 여부 결정
 *    4) TR코드 분류:
 *       - A3/G7/B6/M4 → TCP 서버로 재전송 (시세 데이터)
 *       - 기타 → "0011LK000000000" 응답 전송 (heartbeat 등)
 *------------------------------------------------------------------------*/
void PB_7100_UR(void) {
    char    m_time[24];
    char    Wdt[30], W2_Fmt[512];
    char    rbuf[READ_BUF_SIZE];
    int     tr_gbn, rt, len, skip_rt, rbuf_len;

    BUFF_RW_HEAD        f_head;
    FILE_BUFF_FORMAT    W_Fmt;

    rt = Init_Parameters();
    if (rt == NOTOK)
        return;

    rt = Socket_Connect();
    if (rt == NOTOK)
        return;

    /*----------------------------------------------------------------
     *  HA 초기화
     *  - _FEP_DIV 환경변수로 역할 결정 (REAL1=Primary, REAL2=Secondary)
     *  - heartbeat 소켓 생성 및 상대서버 주소 설정
     *  - 실패 시 단독 운영 모드로 동작 (TCP 전송 항상 활성)
     *----------------------------------------------------------------*/
    rt = HA_Init();
    if (rt == NOTOK) {
        Log(USR_OK, "HA init failed, running as STANDALONE");
    }

    /*----------------------------------------------------------------
     *  TCP 서버 초기 연결
     *  - Primary 또는 STANDALONE: 즉시 연결
     *  - Secondary: 대기 상태이므로 연결하지 않음
     *    (절체 발생 시 ha_active가 ON으로 바뀌면 그때 연결)
     *----------------------------------------------------------------*/
    if (ha_active == ON && SockTcpfd < 0)
        Device_Open();

    while (START_S != JOB_END) {

        /*------------------------------------------------------------
         *  HA heartbeat 처리 (매 루프마다 실행)
         *
         *  Primary  : 1초 간격으로 Secondary에 heartbeat 전송
         *  Secondary: 절체/복귀 판단 (heartbeat 수신은 Recv_Data 내부에서 처리)
         *
         *  heartbeat 수신은 Recv_Data()의 select()에 통합되어 있어
         *  시세 대기 중에도 heartbeat를 즉시 처리할 수 있다.
         *------------------------------------------------------------*/
        if (ha_role == HA_PRIMARY) {
            HA_Heartbeat_Send();
        }
        else if (ha_role == HA_SECONDARY) {
            HA_Check_Failover();
        }

        /*------------------------------------------------------------
         *  TCP 연결 관리
         *  - ha_active == ON 인 경우에만 TCP 연결 유지
         *  - ha_active == OFF 이면 TCP 연결 해제 (대기 상태)
         *------------------------------------------------------------*/
        if (ha_active == ON) {
            if (SockTcpfd < 0) {
                /*----------------------------------------------------
                 *  TCP 재연결 시도 간격 제한 (TCP_RETRY_INTVL=5초)
                 *
                 *  야간 등 TCP 서버 미가동 시, 매 루프마다
                 *  Connect2(2초)를 호출하면 메인루프가 3초/회로 느려져
                 *  HB 전송 간격이 HB_TIMEOUT(5초)과 동일해짐.
                 *  → 5초 간격으로만 재시도하여 나머지 시간은
                 *    메인루프 ~1초(select) 유지 → HB 정상 전송.
                 *----------------------------------------------------*/
                time_t now = time(NULL);
                if (now - tcp_last_try >= TCP_RETRY_INTVL) {
                    Device_Open();
                    tcp_last_try = now;
                }
            }
            /* TCP 연결 실패해도 continue 하지 않음!
             * continue 시 Recv_Data()가 스킵되어 heartbeat 수신이
             * 완전히 차단됨 → Failback 불가 → 양쪽 ACTIVE 고착.
             * TCP 미연결 상태에서도 Recv_Data는 반드시 호출하여
             * heartbeat 처리 + 시세 수신 카운트를 유지한다. */
        }
        else {
            /* 대기 상태: TCP 연결이 남아있으면 정리
             * Device_Close_HA() 사용: sleep(3) 없이 즉시 닫는다.
             * sleep(3) 사용 시 heartbeat 수신 차단 → 재 failover 진동 발생 (FR-08) */
            if (SockTcpfd >= 0) {
                Log(TCP_OK, "HA standby: closing TCP connection");
                Device_Close_HA();
            }
        }

        /*------------------------------------------------------------
         *  UDP 데이터 수신 (역할과 무관하게 항상 수신)
         *
         *  Recv_Data 내부에서 시세 소켓(Sockfd)과 heartbeat 소켓
         *  (hb_sockfd)을 동시에 select로 감시한다.
         *  - 시세 도착 → 수신 바이트 수 리턴
         *  - heartbeat 도착 → hb_last_recv 갱신 후 계속 대기
         *  - 1초 타임아웃 → 0 리턴 (heartbeat 전송/절체 판단 기회)
         *------------------------------------------------------------*/
        rt = Recv_Data(rbuf);
        if (rt == 0) {
            /* 1초 타임아웃: 시세 없음, heartbeat 처리 위해 루프 재진입 */
            continue;
        }
        else if (rt < 0) {
            Log(UDP_ERROR, "receive fail {%d:%s}", SYS_NO, SYS_STR);
            close(Sockfd);

            /* UDP 소켓 재생성 */
            rt = Socket_Connect();
            if (rt == NOTOK)
                return;

            continue;
        }

        rbuf[rt] = '\0';
        rbuf_len = rt;
        my_recv_cnt++;  /* 시세 수신 건수 증가 (HB 전송 시 리셋) */

        /* TR코드 추출 (앞 2바이트) */
        memcpy(TrCode, rbuf, 2);
        TrCode[2] = '\0';

        /*------------------------------------------------------------
         *  대기 상태(ha_active == OFF)면 UDP 수신만 하고 TCP 전송 안 함
         *  - UDP 수신은 계속하여 절체 시 즉시 전송 가능하도록 준비
         *  - 수신 건수 카운트도 계속 유지
         *------------------------------------------------------------*/
        if (ha_active == OFF || SockTcpfd < 0) {
            continue;
        }

        /*------------------------------------------------------------
         *  TR코드별 처리 분기 (ha_active == ON 일 때만 도달)
         *  - A3(체결)/G7(호가)/B6(우선호가)/M4(장운영) → TCP 전달
         *  - 기타(heartbeat 등) → "0011LK000000000" 응답
         *------------------------------------------------------------*/
        if ((memcmp(TrCode, "A3", 2) == 0) ||
                (memcmp(TrCode, "G7", 2) == 0) ||
                (memcmp(TrCode, "B6", 2) == 0) ||
                (memcmp(TrCode, "M4", 2) == 0) ) {
            /* 시세 데이터 → TCP 서버로 전송 */
            rt = Select_Send(SockTcpfd, rbuf, rbuf_len);
            if (rt != OK) {
                Log(TCP_ERROR, "TCP data send fail");
                Device_Close();
            }

            INT_SEQ++;
            Log(TCP_OK, "TCP SD[%s] (%d)<%d>", rbuf, rbuf_len, INT_SEQ);
        }
        else {
            /* 기타 데이터 → LK 응답 (heartbeat 역할) */
            memcpy(rbuf, "0011", 4);            /* 길이 헤더 (4바이트) */
            memcpy(&rbuf[4], "LK000000000", 11);    /* LK 응답 코드 */
            rbuf[15] = '\0';

            rbuf_len = 15;
            rt = Select_Send(SockTcpfd, rbuf, rbuf_len);
            if (rt != OK) {
                Log(TCP_ERROR, "TCP data send fail");
                Device_Close();
            }

            INT_SEQ++;
            Log(TCP_OK, "TCP SD[%s] (%d)<%d>", rbuf, rbuf_len, INT_SEQ);
        }
    }

    /*----------------------------------------------------------------
     *  종료 처리: heartbeat 소켓 닫기
     *----------------------------------------------------------------*/
    if (hb_sockfd >= 0) {
        close(hb_sockfd);
        hb_sockfd = -1;
    }

    return;
} /* End of PB_7100_UR () */

/*------------------------------------------------------------------------
 *  Init_Parameters: 프로세스 파라미터 초기화
 *  - ApType 문자열 생성
 *  - TCP 연결 정보 설정 (IP, 포트)
 *  - 상태 플래그 초기화
 *------------------------------------------------------------------------*/
int     Init_Parameters() {
    int     i, j, rt, flag;
    char    item_code[20], memberitem[10], tmp[128];
    char    d_time[16], head_size[10], cli_orsnd[21];
    char    fifo_name[100], bumun[4], m_time[24];

    /* ApType: 실행파일명에서 모듈+번호+타입 추출 (예: "PB7100UR") */
    sprintf(ApType, "%.2s%.4s%.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /* 누적체결수량 초기화 */
    tot_cq = 0;
    LogOnFlag = OFF;
    OpenFlag = OFF;

    /* TCP2 설정에서 IP 주소 가져오기 */
    sprintf(IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0), TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));

    PortNo = TCP2_PORT_NO;
    Log(TCP_OK, "Server Side port[%d]", PortNo);

    TCP2_PROC_ST = ON;      /* 프로세스 상태: 기동 */
    TCP2_LINE_ST = OFF;     /* 회선 상태: 미연결 */

    return (OK);
} /* End of Init_Parameters () */

/*------------------------------------------------------------------------
 *  Socket_Connect: UDP 멀티캐스트 소켓 생성 및 그룹 Join
 *
 *  처리 순서:
 *    1) UDP 소켓 생성 (SOCK_DGRAM)
 *    2) NIC 인터페이스 결정 (개발: ens7f3, 운영: ens6f3)
 *    3) SO_REUSEADDR 설정
 *    4) bind() - 멀티캐스트 주소에 바인드
 *    5) ioctl로 NIC의 IP 주소 가져오기
 *    6) IP_ADD_MEMBERSHIP으로 멀티캐스트 그룹 Join
 *------------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     rt, val, len;
    char    ip_addr [16];
    char    ifnm [IFNAMSIZ] = {0};  /* 네트워크 인터페이스 이름 */

    /* UDP 설정에서 멀티캐스트 IP 주소 가져오기 */
    sprintf(ip_addr, "%d.%d.%d.%d", UDP_IP1(D_K,P_K,0), UDP_IP2(D_K,P_K,0),
            UDP_IP3(D_K,P_K,0), UDP_IP4(D_K,P_K,0));

    /* 서버 주소 설정 */
    bzero((unsigned char *)&SvrAddr, sizeof (SvrAddr));
    SvrAddr.sin_family          = AF_INET;
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);
    SvrAddr.sin_port            = htons(SVR_PORT_NO);

    /* 멀티캐스트 요청 구조체 설정 */
    bzero((unsigned char *)&mreq, sizeof (mreq));
    mreq.imr_multiaddr          = SvrAddr.sin_addr;
    mreq.imr_interface.s_addr   = htonl(INADDR_ANY);

    /* UDP 소켓 생성 */
    Sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Sockfd < 0) {
        Log(UDP_FATAL, "socket open fail(%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* NIC 인터페이스 결정: 192/10 대역 NIC 자동 탐색 */
    {
        if (find_ifname_by_192(ifnm) != 0) {
            Log(UDP_FATAL, "cannot find network interface for multicast");
            close(Sockfd);
            return (NOTOK);
        }

        Log(USR_OK, "interface name > [%s]", ifnm);
    }

    /* SO_REUSEADDR: 동일 포트 재사용 허용 */
    val = 1;
    len = sizeof(val);
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, len);
    if (rt == -1) {
        Log(UDP_WARN, "setsockopt(SO_REUSEADDR) fail(%d:%s)", SYS_NO, SYS_STR);
    }

    /* 2011.03 시세수신 증속에 따른 수신 버퍼 확대 */
    val = UDP_SOCK_RCVBUF_SIZE;
    len = sizeof(val);
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&val, len);
    if (rt == -1) {
        Log(UDP_WARN, "setsockopt(SO_RCVBUF) fail(%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* 멀티캐스트 주소에 바인드 */
    if (bind(Sockfd, (struct sockaddr *) &SvrAddr, sizeof (SvrAddr)) < 0) {
        Log(UDP_FATAL, "bind fail(%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* NIC의 실제 IP 주소를 가져와서 멀티캐스트 인터페이스로 설정 */
    {

        int     ii;
        struct ifreq ifreq;

        memset(&ifreq, 0, sizeof(ifreq));
        strncpy(ifreq.ifr_name, ifnm, IFNAMSIZ - 1);

        /* ioctl로 NIC에 할당된 IP 주소 가져오기 */
        if (ioctl(Sockfd, SIOCGIFADDR, &ifreq) < 0) {
            close(Sockfd);
            Log(UDP_FATAL, "ioctl(SIOCGIFADDR) fail");
            return (NOTOK);
        }

        /* 멀티캐스트 인터페이스를 NIC의 IP로 설정 */
        memcpy(&mreq.imr_interface, &((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));

        /* 멀티캐스트 그룹 Join */
        rt = setsockopt(Sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq));
        if (rt == -1) {
            Log(UDP_WARN, "setsockopt MULTICAST fail(%d:%s)", SYS_NO, SYS_STR);
            return (NOTOK);
        }

    }

    Log(USR_OK, "socket connected:port[%s] [%d]", ip_addr, SVR_PORT_NO);
    return (OK);
} /* End of Socket_Connect () */

/*------------------------------------------------------------------------
 *  Recv_Data: UDP 데이터 수신 (시세 + heartbeat 동시 감시)
 *
 *  select()로 두 소켓을 동시에 감시한다:
 *    1) Sockfd    : KRX 시세 멀티캐스트 UDP
 *    2) hb_sockfd : HA heartbeat UDP (Secondary만 해당)
 *
 *  타임아웃: 1초
 *    - 기존 70초에서 1초로 변경
 *    - 이유: heartbeat 전송(Primary) 및 절체 판단(Secondary)을
 *      1초 간격으로 수행해야 하므로, select가 최대 1초마다 리턴해야 함
 *    - 시세가 없는 시간에도 heartbeat 처리가 정상 동작
 *
 *  동작:
 *    - heartbeat 도착 → hb_last_recv 갱신 후 계속 대기 (리턴하지 않음)
 *    - 시세 도착 → 수신 데이터를 p_str에 담아 리턴
 *    - 동시 도착 → heartbeat 먼저 처리 후 시세 리턴
 *    - 1초 타임아웃 → 0 리턴
 *
 *  반환값:
 *    >0 : 시세 수신 바이트 수
 *     0 : 타임아웃 (시세 없음)
 *    -1 : 에러
 *------------------------------------------------------------------------*/
int Recv_Data(char *p_str) {
    int rt, len, maxfd;
    fd_set read_set;
    struct timeval timeout;

    len = sizeof (ClntAddr);

    FD_ZERO(&read_set);
    FD_SET(Sockfd, &read_set);
    maxfd = Sockfd;

    /*----------------------------------------------------------------
     *  heartbeat 소켓이 유효하면 select 감시 대상에 추가
     *  - Secondary: hb_sockfd에 bind되어 있어 heartbeat 수신 가능
     *  - Primary/Standalone: hb_sockfd가 -1이거나 bind 안 되어 있으면
     *    Sockfd만 감시 (기존 동작과 동일)
     *----------------------------------------------------------------*/
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY) {
        FD_SET(hb_sockfd, &read_set);
        if (hb_sockfd > maxfd)
            maxfd = hb_sockfd;
    }

    /*----------------------------------------------------------------
     *  타임아웃 1초:
     *  - Primary가 heartbeat를 1초 간격으로 보내야 하므로
     *  - Secondary가 절체 판단을 1초 간격으로 해야 하므로
     *  - 시세가 안 들어와도 1초마다 루프 재진입 보장
     *----------------------------------------------------------------*/
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    rt = select(maxfd + 1, &read_set, NULL, NULL, &timeout);
    if (rt < 0) {
        Log(SYS_FATAL, "select fail(%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }
    if (rt == 0) return 0;  /* 1초 타임아웃: 시세 없음 */

    /*----------------------------------------------------------------
     *  heartbeat 도착 처리 (Secondary만)
     *  - heartbeat 패킷을 읽어서 버림 (내용은 중요하지 않음)
     *  - hb_last_recv를 현재 시각으로 갱신
     *  - 여러 개 쌓여있을 수 있으므로 non-blocking 루프로 모두 소진
     *  - heartbeat 처리 후에도 시세 데이터 확인을 계속 진행
     *----------------------------------------------------------------*/
    if (hb_sockfd >= 0 && ha_role == HA_SECONDARY &&
        FD_ISSET(hb_sockfd, &read_set)) {
        char   hb_buf[32];
        struct sockaddr_in from_addr;
        socklen_t from_len;

        /* select에서 감지된 첫 패킷 + 뒤에 쌓인 패킷 모두 읽기 */
        while (1) {
            fd_set hb_set;
            struct timeval tv_zero;

            from_len = sizeof(from_addr);
            rt = recvfrom(hb_sockfd, hb_buf, sizeof(hb_buf), 0,
                          (struct sockaddr *)&from_addr, &from_len);
            if (rt < 0) {
                /* recvfrom 에러: EINTR 등 — 로그 남기고 루프 탈출 */
                Log(UDP_WARN, "HA HB recvfrom fail(%d:%s)", SYS_NO, SYS_STR);
                break;
            }
            if (rt >= (int)HB_PKT_LEN &&
                    memcmp(hb_buf, "HB", 2) == 0) {
                /* 확장 HB 패킷: 시세 수신 건수 추출 */
                HB_PACKET *p = (HB_PACKET *)hb_buf;
                pri_recv_cnt = p->recv_cnt;
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
                Log(USR_OK, "HA HB recv ok (pri_cnt=%d, stable=%d)",
                    pri_recv_cnt, hb_stable_cnt);
            }
            else if (rt >= 2 && memcmp(hb_buf, "HB", 2) == 0) {
                /* 구 버전 또는 크기 불일치 HB 패킷 (호환) */
                pri_recv_cnt = -1;  /* 판단 불가 */
                hb_last_recv = time(NULL);
                hb_stable_cnt++;
                Log(USR_OK, "HA HB recv ok (legacy, rt=%d, stable=%d)",
                    rt, hb_stable_cnt);
            }
            else {
                /* HB가 아닌 패킷 수신 — 진단용 로그 */
                Log(UDP_WARN, "HA HB unknown pkt (rt=%d, hdr=0x%02X%02X)",
                    rt, (unsigned char)hb_buf[0], (unsigned char)hb_buf[1]);
            }

            /* 추가 패킷 확인 (non-blocking) */
            FD_ZERO(&hb_set);
            FD_SET(hb_sockfd, &hb_set);
            tv_zero.tv_sec  = 0;
            tv_zero.tv_usec = 0;
            if (select(hb_sockfd + 1, &hb_set, NULL, NULL, &tv_zero) <= 0)
                break;  /* 더 이상 패킷 없음 */
        }
    }

    /*----------------------------------------------------------------
     *  시세 데이터 도착 처리
     *  - 시세 소켓에 데이터가 있으면 읽어서 호출자에게 리턴
     *  - heartbeat만 도착하고 시세는 없는 경우 → 0 리턴 (타임아웃과 동일)
     *----------------------------------------------------------------*/
    if (FD_ISSET(Sockfd, &read_set)) {
        rt = recvfrom(Sockfd, p_str, READ_BUF_SIZE, 0,
                      (struct sockaddr *) &ClntAddr, (socklen_t *)&len);
        return(rt);
    }

    return (0);  /* heartbeat만 도착, 시세는 없음 */
} /* End of Recv_Data () */

/*------------------------------------------------------------------------
 *  Add_Count_UR: TR코드별 UDP 수신 건수 카운트
 *  - sisetr.ini에 정의된 TR코드와 비교
 *  - 매칭되면 ur_count 증가
 *
 *  반환값:
 *    >0 : 데이터 길이 (해당 TR의 length)
 *     0 : queue가 0인 TR (수신만 카운트, 분배 안 함)
 *    -1 : 매칭 TR 없음
 *------------------------------------------------------------------------*/
int Add_Count_UR(void) {
    int rt = NOTOK;
    int T_K = NOTOK;
    int i;

    for (i = 0; i < DAEMON(D_K).sisetr_count; i++) {
        if (SISETR(D_K, i).tr[0] == '\0')
            break;  /* TR 목록 끝 */

        if (memcmp(SISETR(D_K, i).tr, TrCode, 5) == 0) {
            if (SISETR(D_K, i).queue == 0) {
                rt = 0;     /* 분배 안 하는 TR */
                break;
            }
            else {
                SISETR(D_K, i).ur_count++; /* 수신 건수 증가 */

                T_K = i;
                rt = SISETR(D_K, T_K).length;
                break;
            }
        }
    }
    return (rt);
} /* End of Add_Count_UR () */

/*------------------------------------------------------------------------
 *  Add_Count_DD: TR코드별 분배(DD) 건수 카운트
 *  - sisetr.ini에 정의된 TR코드와 비교
 *  - 매칭되면 dd_count 증가
 *
 *  반환값:
 *    >0 : 데이터 길이
 *    -1 : 매칭 TR 없음
 *------------------------------------------------------------------------*/
int Add_Count_DD(char *ptr) {
    int rt = NOTOK;
    int i;

    for (i = 0; i < DAEMON(D_K).sisetr_count; i++) {
        if (SISETR(D_K, i).tr[0] == '\0')
            break;

        if (memcmp(SISETR(D_K, i).tr, ptr, strlen(ptr)) == 0) {
            SISETR(D_K, i).dd_count++; /* 분배 건수 증가 */

            rt = SISETR(D_K, i).length;
            break;
        }
    }
    return (rt);
} /* End of Add_Count_DD () */

/*------------------------------------------------------------------------
 *  find_ifname_by_192: 192.x.x.x 또는 10.x.x.x 대역 NIC 찾기
 *  - 우선순위: 192.x.x.x > 10.x.x.x
 *  - getifaddrs()로 전체 인터페이스 탐색
 *
 *  반환값:
 *     0 : 성공 (ifnm에 인터페이스명 저장)
 *    -1 : 실패
 *------------------------------------------------------------------------*/
int find_ifname_by_192(char *ifnm) {
    struct ifaddrs *ifaddr, *ifa;
    char candidate10 [IFNAMSIZ] = {0};  /* 10.x.x.x 후보 */

    if (getifaddrs(&ifaddr) == -1)
        return -1;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
        unsigned int ip = ntohl(sa->sin_addr.s_addr);

        /* 192.x.x.x (0xC0000000) → 최우선 */
        if ((ip & 0xFF000000) == 0xC0000000) {
            strncpy(ifnm, ifa->ifa_name, IFNAMSIZ - 1);
            ifnm[IFNAMSIZ - 1] = '\0';
            freeifaddrs(ifaddr);
            return 0;
        }
        /* 10.x.x.x (0x0A000000) → 후보 저장 */
        if ((ip & 0xFF000000) == 0x0A000000) {
            strncpy(candidate10, ifa->ifa_name, sizeof(candidate10) - 1);
            candidate10[sizeof(candidate10) - 1] = '\0';
        }
    }

    /* 192 대역 없으면 10 대역 사용 */
    if (candidate10[0] != '\0') {
        strncpy(ifnm, candidate10, IFNAMSIZ - 1);
        ifnm[IFNAMSIZ - 1] = '\0';
        freeifaddrs(ifaddr);
        return 0;
    }

    freeifaddrs(ifaddr);
    return -1;
}

/*========================================================================
 *
 *  HA(이중화) 함수들
 *
 *  아래 함수들은 2대 서버 간 Active/Standby 자동 절체를 구현한다.
 *
 *  구조:
 *    Primary(REAL1)  ── HB_PACKET(1초) ──→  Secondary(REAL2)
 *    HB_PACKET = "HB" + recv_cnt (Primary의 시세 수신 건수)
 *
 *  절체 조건 (FAILOVER):
 *    1) HB 미수신 HB_TIMEOUT(5초) → Primary 프로세스 장애
 *    2) HB 정상 + Primary 시세 0건 + Secondary 시세 수신 중
 *       → HB_DATA_TIMEOUT(5회) 연속 시 Primary 시세 끊김 판단
 *
 *  복귀 조건 (FAILBACK):
 *    1) HB 연속 HB_STABLE_COUNT(3회) 수신
 *    2) Primary도 시세 수신 중 (recv_cnt > 0) 또는 양쪽 다 없음
 *
 *========================================================================*/

/*------------------------------------------------------------------------
 *  HA_Init: HA(이중화) 초기화
 *
 *  처리 순서:
 *    1) _FEP_DIV 환경변수로 역할 결정
 *       - "REAL1" → Primary  (항상 TCP 전송)
 *       - "REAL2" → Secondary (대기, 장애 시 절체)
 *       - 기타    → Standalone (단독 운영, 이중화 미사용)
 *    2) 상대 서버(peer) IP 설정
 *       - Primary의 peer = REAL2 서버 IP
 *       - Secondary의 peer = REAL1 서버 IP
 *    3) heartbeat용 UDP 소켓 생성
 *       - Primary: 송신용 (Secondary에게 전송)
 *       - Secondary: 수신용 (bind하여 대기)
 *
 *  반환값:
 *     0 (OK)    : 초기화 성공
 *    -1 (NOTOK) : 초기화 실패 (단독 운영으로 fallback)
 *------------------------------------------------------------------------*/
int HA_Init(void) {
    int rt, val;
    char *peer_ip;

    /*----------------------------------------------------------------
     *  1단계: _FEP_DIV로 역할 결정
     *
     *  _FEP_DIV는 pkg_env.sh에서 hostname 기반으로 설정됨:
     *    podm11 → "REAL1" (Primary)
     *    podm12 → "REAL2" (Secondary)
     *    기타   → "TEST"  (Standalone)
     *----------------------------------------------------------------*/
    if (memcmp(_FEP_DIV, "REAL1", 5) == 0) {
        ha_role   = HA_PRIMARY;
        ha_active = ON;     /* Primary는 항상 TCP 전송 */
        Log(USR_OK, "HA role: PRIMARY (always active)");
    }
    else if (memcmp(_FEP_DIV, "REAL2", 5) == 0) {
        ha_role   = HA_SECONDARY;
        ha_active = OFF;    /* Secondary는 기본 대기 */
        Log(USR_OK, "HA role: SECONDARY (standby)");
    }
    else {
        /*------------------------------------------------------------
         *  TEST 환경 등: 이중화 미사용, 단독 운영
         *  heartbeat 없이 항상 TCP 전송
         *------------------------------------------------------------*/
        ha_role   = HA_STANDALONE;
        ha_active = ON;
        Log(USR_OK, "HA role: STANDALONE (no HA)");
        return (OK);
    }

    /*----------------------------------------------------------------
     *  peer IP를 환경변수에서 가져오기
     *
     *  _HA_PEER_IP는 pkg_env.sh에서 hostname 기반으로 설정됨:
     *    podm11(REAL1) → "192.168.151.12" (podm12의 IP)
     *    podm12(REAL2) → "192.168.151.11" (podm11의 IP)
     *
     *  환경변수 미설정 시 HA 초기화 실패 → STANDALONE fallback
     *----------------------------------------------------------------*/
    peer_ip = (char *)getenv("_HA_PEER_IP");
    if (peer_ip == NULL || peer_ip[0] == '\0') {
        Log(UDP_ERROR, "HA _HA_PEER_IP not set, cannot init HA");
        return (NOTOK);
    }
    strncpy(ha_peer_ip, peer_ip, sizeof(ha_peer_ip) - 1);
    ha_peer_ip[sizeof(ha_peer_ip) - 1] = '\0';
    Log(USR_OK, "HA peer IP: %s (from _HA_PEER_IP)", ha_peer_ip);

    /*----------------------------------------------------------------
     *  heartbeat 포트를 환경변수에서 가져오기
     *
     *  _HA_HB_PORT는 pkg_env.sh에서 설정됨 (양쪽 서버 동일 값)
     *  미설정 시 기본값 HB_PORT_DEFAULT(50000) 사용
     *----------------------------------------------------------------*/
    /*----------------------------------------------------------------
     *  heartbeat 포트 결정:
     *  _HA_HB_PORT 환경변수를 base port로 사용하고,
     *  프로세스별 offset을 자동 적용하여 포트 충돌을 방지한다.
     *
     *  pb_7102_ur: base + 0 (예: 50000)
     *  pb_7103_ur: base + 1 (예: 50001)
     *
     *  환경변수 미설정 시 컴파일 타임 기본값 사용 (HB_PORT_DEFAULT)
     *----------------------------------------------------------------*/
    {
        char *port_str = (char *)getenv("_HA_HB_PORT");
        if (port_str != NULL && port_str[0] != '\0') {
            int base_port = atoi(port_str);
            if (base_port <= 0 || base_port > 65535) {
                Log(UDP_WARN, "HA invalid _HA_HB_PORT [%s], using default %d",
                    port_str, HB_PORT_DEFAULT);
                hb_port = HB_PORT_DEFAULT;
            }
            else {
#if defined B7102
                hb_port = base_port;        /* pb_7102_ur: base + 0 */
#elif defined B7103
                hb_port = base_port + 1;    /* pb_7103_ur: base + 1 */
#else
                hb_port = base_port;
#endif
            }
        }
        Log(USR_OK, "HA heartbeat port: %d (base_env=%s, process=%s)",
            hb_port,
            (port_str != NULL) ? port_str : "unset",
#if defined B7102
            "pb_7102_ur"
#elif defined B7103
            "pb_7103_ur"
#else
            "unknown"
#endif
        );
    }

    /*----------------------------------------------------------------
     *  2단계: heartbeat용 UDP 소켓 생성
     *----------------------------------------------------------------*/
    hb_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hb_sockfd < 0) {
        Log(UDP_ERROR, "HA heartbeat socket fail (%d:%s)", SYS_NO, SYS_STR);
        return (NOTOK);
    }

    /* SO_REUSEADDR 설정: 프로세스 재시작 시 bind 충돌 방지 */
    val = 1;
    setsockopt(hb_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));

    /*----------------------------------------------------------------
     *  3단계: 상대 서버 주소 구조체 설정
     *  - Primary  → peer(Secondary)에게 heartbeat 전송할 주소
     *  - Secondary → peer(Primary)로부터 heartbeat 수신할 주소
     *----------------------------------------------------------------*/
    memset(&hb_peer_addr, 0, sizeof(hb_peer_addr));
    hb_peer_addr.sin_family = AF_INET;
    hb_peer_addr.sin_port   = htons(hb_port);

    if (ha_role == HA_PRIMARY) {
        /*------------------------------------------------------------
         *  Primary: peer(Secondary) IP를 전송 대상으로 설정
         *  소켓을 bind할 필요 없음 (송신 전용)
         *------------------------------------------------------------*/
        inet_pton(AF_INET, ha_peer_ip, &hb_peer_addr.sin_addr);
        Log(USR_OK, "HA heartbeat target: %s:%d", ha_peer_ip, hb_port);
    }
    else {
        /*------------------------------------------------------------
         *  Secondary: 자기 자신의 hb_port에 bind하여 수신 대기
         *  INADDR_ANY로 bind하여 어떤 NIC로 들어와도 수신
         *------------------------------------------------------------*/
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family      = AF_INET;
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind_addr.sin_port        = htons(hb_port);

        if (bind(hb_sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
            Log(UDP_ERROR, "HA heartbeat bind fail (%d:%s)", SYS_NO, SYS_STR);
            close(hb_sockfd);
            hb_sockfd = -1;
            return (NOTOK);
        }

        /* 최초 수신 시각을 현재로 설정 (시작 직후 오탐 절체 방지) */
        hb_last_recv = time(NULL);

        Log(USR_OK, "HA heartbeat listening on port %d (fd=%d, Sockfd=%d)",
            hb_port, hb_sockfd, Sockfd);
    }

    Log(USR_OK, "HA init OK: role=%d active=%d hb_fd=%d hb_port=%d "
        "peer=%s HB_PKT_LEN=%d",
        ha_role, ha_active, hb_sockfd, hb_port,
        ha_peer_ip, (int)HB_PKT_LEN);

    return (OK);
} /* End of HA_Init () */

/*------------------------------------------------------------------------
 *  HA_Heartbeat_Send: Primary → Secondary heartbeat 전송
 *
 *  Primary 전용 함수.
 *  HB_SEND_INTVL(1초) 간격으로 "HB" 패킷을 Secondary에 UDP 전송.
 *  메인 루프에서 매번 호출되지만, 간격 미달 시 즉시 리턴.
 *
 *  전송 실패 시 로그만 남기고 계속 진행 (Primary의 TCP 전송은 유지).
 *------------------------------------------------------------------------*/
void HA_Heartbeat_Send(void) {
    time_t now;
    int    rt;
    HB_PACKET pkt;

    now = time(NULL);

    /* 전송 간격 체크: HB_SEND_INTVL(1초) 미만이면 스킵 */
    if ((now - hb_last_send) < HB_SEND_INTVL)
        return;

    /* HB 패킷 구성: "HB" + 직전 1초간 시세 수신 건수 */
    memcpy(pkt.hb_mark, "HB", 2);
    pkt.recv_cnt = my_recv_cnt;
    my_recv_cnt = 0;    /* 리셋: 다음 HB까지 새로 카운트 */

    /* heartbeat 패킷 전송 */
    rt = sendto(hb_sockfd, (char *)&pkt, HB_PKT_LEN, 0,
                (struct sockaddr *)&hb_peer_addr, sizeof(hb_peer_addr));

    if (rt < 0) {
        Log(UDP_WARN, "HA HB send fail (fd=%d, port=%d, peer=%s, %d:%s)",
            hb_sockfd, hb_port, ha_peer_ip, SYS_NO, SYS_STR);
    }
    else {
        Log(USR_OK, "HA HB sent (rt=%d, cnt=%d, to=%s:%d)",
            rt, pkt.recv_cnt, ha_peer_ip, hb_port);
    }

    hb_last_send = now;
} /* End of HA_Heartbeat_Send () */

/*------------------------------------------------------------------------
 *  HA_Check_Failover: Secondary의 절체/복귀 판단
 *
 *  Secondary 전용 함수.
 *
 *  FAILOVER (Standby → Active):
 *    1) HB 타임아웃 (Primary 프로세스 장애)
 *    2) HB 정상 + Primary 시세 0건 + 나는 시세 수신 중
 *       → HB_DATA_TIMEOUT(5)회 연속 (Primary 시세 장애)
 *
 *  FAILBACK (Active → Standby):
 *    1) HB 연속 HB_STABLE_COUNT(3)회 수신
 *    2) Primary도 시세 수신 중 (또는 양쪽 다 없음 = 장 마감)
 *    ※ Primary가 살아있지만 시세 못 받으면 Failback 차단
 *------------------------------------------------------------------------*/
void HA_Check_Failover(void) {
    time_t now;
    int    elapsed;
    int    my_data;     /* 직전 주기 시세 수신 건수 (스냅샷) */

    now     = time(NULL);
    elapsed = (int)(now - hb_last_recv);

    /* my_recv_cnt 스냅샷 후 리셋 — "직전 1초간 시세 수신 건수"
     * Primary는 HA_Heartbeat_Send에서 리셋하지만
     * Secondary는 여기서 리셋해야 누적값 오판을 방지 */
    my_data = my_recv_cnt;
    my_recv_cnt = 0;

    if (ha_active == OFF) {
        /*------------------------------------------------------------
         *  Failover 판단 1: 대기 중 + heartbeat 타임아웃 초과
         *  → Primary 프로세스 장애로 판단
         *------------------------------------------------------------*/
        if (elapsed >= HB_TIMEOUT) {
            ha_active = ON;
            hb_stable_cnt = 0;
            pri_no_data_cnt = 0;
            Log(USR_OK, "HA FAILOVER: Primary heartbeat timeout (%dsec), "
                        "Secondary now ACTIVE", elapsed);
            return;
        }

        /*------------------------------------------------------------
         *  Failover 판단 2: 대기 중 + Primary 시세 끊김
         *  조건:
         *    1) HB 정상 수신 (Primary 프로세스는 살아있음)
         *    2) Primary의 recv_cnt == 0 (Primary가 시세를 못 받고 있음)
         *    3) 나(Secondary)는 시세를 받고 있음 (my_data > 0)
         *    4) 위 상태가 HB_DATA_TIMEOUT(5)회 연속
         *
         *  양쪽 다 시세 없음(장 마감 등)이면 pri_no_data_cnt 리셋
         *------------------------------------------------------------*/
        if (pri_recv_cnt == 0 && my_data > 0) {
            pri_no_data_cnt++;
            if (pri_no_data_cnt >= HB_DATA_TIMEOUT) {
                ha_active = ON;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                Log(USR_OK, "HA FAILOVER: Primary no market data "
                            "(%d consecutive, my_recv=%d), "
                            "Secondary now ACTIVE",
                            HB_DATA_TIMEOUT, my_data);
            }
        }
        else {
            pri_no_data_cnt = 0;
        }
    }
    else {
        /*------------------------------------------------------------
         *  Failback 판단: Active 중 + heartbeat 연속 N회 수신
         *  → Primary가 안정적으로 복구된 것을 확인 후 Standby 복귀
         *
         *  조건:
         *    1) 최근 heartbeat 수신 (elapsed < HB_TIMEOUT)
         *    2) 연속 HB_STABLE_COUNT(3)회 이상 수신
         *    3) Primary가 시세도 수신 중 (pri_recv_cnt > 0)
         *       또는 양쪽 다 시세 없음 (장 마감 등)
         *
         *  Primary가 살아있지만 시세를 못 받는 경우 Failback 하지 않음
         *------------------------------------------------------------*/
        if (elapsed >= HB_TIMEOUT) {
            /* heartbeat 다시 끊김 → 카운터 리셋, Active 유지 */
            hb_stable_cnt = 0;
        }
        else if (hb_stable_cnt >= HB_STABLE_COUNT) {
            /* Failback 가능 조건: Primary도 시세 수신 중이거나
             * 양쪽 다 시세 없음 (장 마감) */
            if (pri_recv_cnt > 0 || my_data == 0) {
                ha_active = OFF;
                hb_stable_cnt = 0;
                pri_no_data_cnt = 0;
                Log(USR_OK, "HA FAILBACK: Primary stable "
                            "(%d consecutive HB, pri_data=%d), "
                            "Secondary now STANDBY",
                            HB_STABLE_COUNT, pri_recv_cnt);
            }
            else {
                /* Primary HB는 오지만 시세 미수신, Active 유지
                 * stable_cnt 리셋하여 다시 3회 대기 (로그 반복 방지) */
                hb_stable_cnt = 0;
                Log(USR_OK, "HA FAILBACK blocked: Primary alive but "
                            "no market data (pri=%d, my=%d)",
                            pri_recv_cnt, my_data);
            }
        }
    }
} /* End of HA_Check_Failover () */

/*------------------------------------------------------------------------
 *  End of program (pb_7100_ur.c)
 *------------------------------------------------------------------------*/
