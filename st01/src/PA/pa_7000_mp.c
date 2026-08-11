#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 시세 Real & UDP 송신
 *  File    : pa_7000_mp.c
 *
 *  FIFO에서 읽은 시세 데이터를 UDP로 외부 시스템에 송신하는 프로세스.
 *  금융파생 시세(선물/콜옵션/풋옵션)를 처리한다.
 *
 *  빌드 옵션:
 *    A7001 : 금융파생 시세 UDP 송신 (자동주문 연동 포함)
 *    A7002 : 금융파생 시세 UDP 송신
 *    A7003 : 금융파생 시세 UDP 송신
 *    A7004 : 금융파생 시세 UDP 송신 (포트번호 다름)
 *
 *  처리 흐름:
 *    1) FIFO 오픈 (호스트명별로 다른 FIFO 경로)
 *    2) UDP 소켓 생성 (시장별 3개: 선물/콜/풋)
 *    3) FIFO에서 시세 읽기 → 시장 구분 → SHM 기록 → UDP 송신
 *    4) 수신 시각 차이만큼 sleep하여 실시간 재생 속도 조절
 *
 *  시장 구분 (Sise_Gbn):
 *    1: 선물 (코드 01), 2: 콜옵션 (코드 03+2), 3: 풋옵션 (코드 03+3)
 *
 *  SHM 구조:
 *    Shm_Futures[i] : 선물 시세 (B6=호가, A3=체결, G7=체결+호가)
 *    Shm_Options[i] : 옵션 시세 (B6=호가, A3=체결, G7=체결+호가)
 *    - CURR_Arry: 최근 30건 현재가 이력 (ring buffer)
 *    - CHE_Arry: 최근 1000건 체결 이력 (ring buffer)
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *  Header Files
 *------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE   300
#include    "buf_struct.h"
/*------------------------------------------------------------------------
 *  Constants and Structures
 *------------------------------------------------------------------------*/

#define     S_H_SIZE    17      /* TR(5) + Code(12) = 17바이트 뒤가 SEQ 시작 */
/*------------------------------------------------------------------------
 *  Global Variables
 *------------------------------------------------------------------------*/
int         Sise_Gbn, sleep_speed;          /* 시장구분(1~3), 재생속도 배수 */
int         Sockfd[4][4], id = -1, Max_IP_Addr; /* UDP 소켓 [IP별][시장별] */
char        Svr_IP[20*4];                   /* 송신 대상 IP 주소 (최대 4개) */
struct      sockaddr_in Svr_Addr[4][4], Clnt_Addr[4][4];    /* 서버/클라이언트 주소 */

/*------------------------------------------------------------------------
 *  Function Prototypes
 *------------------------------------------------------------------------*/
void    PA_7000_MP(int, char **);
void    Set_Curr(char *);
void    Set_Sise(char *);
void    Microsec_Sleep(int);

/*------------------------------------------------------------------------
 *  main: 프로세스 초기화 → 시세송신 루프 → 종료
 *------------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
    PA_7000_MP(argc, argv);
    Exit_Process();
}   /* End of main ()   */

/*------------------------------------------------------------------------
 *  PA_7000_MP: 메인 시세 송신 루프
 *
 *  파라미터 (커맨드라인):
 *    argv[1]: sleep_speed (재생속도 배수, 음수=느리게, 양수=빠르게)
 *    argv[2~4]: 송신 대상 IP 주소 (최대 3개)
 *
 *  처리 흐름:
 *    1) 호스트명별 FIFO 경로 결정 (ap54/ap64/at05/기타)
 *    2) FIFO 오픈: A3/G7용 + B6용 (2개)
 *    3) 커맨드라인에서 IP주소/속도 파싱
 *    4) UDP 소켓 생성 (Socket_Connect)
 *    5) FIFO 읽기 → 시장구분 → Set_Sise(SHM기록) → UDP 송신
 *    6) 수신시각 차이에 따른 sleep (실시간 재생)
 *------------------------------------------------------------------------*/
void    PA_7000_MP(int argc, char **argv) {
    int     i, idx, rt, len, R_Cnt, flag, start_flag, sltime, sd_size;
    int     FIFO_fd, FIFO_fd1;
    char    fifo_name[100], fifo_name1[100], tmp[128], bumun[4];
    char    r_buf[1024];
    double  recv_sec, befo_sec;
    FILE_BUFF_FORMAT    R_Fmt[1];

    /*------------------------------------------------------------
     *  호스트명별 FIFO 경로 결정
     *  - ap54/ap64: PA/pa_7102_ur1 + PA/pa_7112_ur1
     *  - at05: PA/pa_7102_ur1 + PB/pb_7102_ur1
     *  - 기타(ap67,at07): PB/pb_7102_ur1 + PB/pb_7112_ur1
     *------------------------------------------------------------*/
    sprintf(bumun, "%s", _SubSystem_Name);
    LtoU(bumun, 2);

    {
        char *env_hostname = getenv("host_name");
        if (env_hostname == NULL) env_hostname = "";

        if ( (memcmp(env_hostname, "ap54", 4) == 0) ||
                (memcmp(env_hostname, "ap64", 4) == 0) ) {
            sprintf(fifo_name, "%s/PA/pa_7102_ur1", _FEP_FIFO);
            sprintf(fifo_name1, "%s/PA/pa_7112_ur1", _FEP_FIFO);
        }
        else if (memcmp(env_hostname, "at05", 4) == 0) {
            sprintf(fifo_name, "%s/PA/pa_7102_ur1", _FEP_FIFO);
            sprintf(fifo_name1, "%s/PB/pb_7102_ur1", _FEP_FIFO);
        }
        else                                    /* ap67, at05, at07 */ {
            sprintf(fifo_name, "%s/PB/pb_7102_ur1", _FEP_FIFO);
            sprintf(fifo_name1, "%s/PB/pb_7112_ur1", _FEP_FIFO);
        }
    }

    /* A3,G7(C0) FIFO 오픈 - 체결+호가 시세 통지용 */
    FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);

    if (FIFO_fd < 0)
        Log(FIF_FATAL, "cannot open FIFO[%d][%d:%s][%s]",
                FIFO_fd, SYS_NO, SYS_STR, fifo_name);

    /* B6(H0) FIFO 오픈 - 우선호가 시세 통지용 */
    FIFO_fd1 = open(fifo_name1, O_RDWR|O_NDELAY);

    if (FIFO_fd1 < 0)
        Log(FIF_FATAL, "cannot open FIFO1[%d][%d:%s][%s]",
                FIFO_fd1, SYS_NO, SYS_STR, fifo_name1);

    /*------------------------------------------------------------
     *  커맨드라인 파라미터 파싱
     *  - argv[1]: sleep_speed (재생속도 배수)
     *  - argv[2~4]: 송신 대상 IP (최대 3개, 유니캐스트)
     *------------------------------------------------------------*/
    if (argc > 2)       /* TEST: 속도 + IP주소 */ {
        sleep_speed = AtoIf(argv[1], strlen(argv[1]));
        /* 3명까지는 유니캐스트 처리방식 가능 */
        if (argc > 5)   Max_IP_Addr = 1;
        else            Max_IP_Addr = argc - 2;
    }
    else if (argc == 1)     /* 파라미터 없음: 기본값 */ {
        Max_IP_Addr = 1;
        sleep_speed = 1;
    }
    else if (argc == 2)     /* 속도만 */ {
        Max_IP_Addr = 1;
        sleep_speed = AtoIf(argv[1], strlen(argv[1]));
    }

    if (sleep_speed == 0)
        sleep_speed = 1;

    /* UDP 소켓 초기화 및 IP주소 설정 */
    for (i = 0; i < Max_IP_Addr; i++) {
        Sockfd[i][0] = -1;      /* 선물 */
        Sockfd[i][1] = -1;      /* 콜옵션 */
        Sockfd[i][2] = -1;      /* 풋옵션 */
        /* 최고 3개 IP까지 처리가능 */
        bzero((unsigned char *)Clnt_Addr, sizeof (struct sockaddr_in) * 16);
        bzero((unsigned char *)Svr_Addr, sizeof (struct sockaddr_in) * 16);

        if (argc > 2)
            sprintf(&Svr_IP[20*i], "%s", argv[2+i]);
        else {
            Log(USR_ERROR, "UDP broadcast IP not specified(argc=%d)", argc);
            return;
        }
    }

    /* UDP 소켓 생성 및 바인드 */
    rt = Socket_Connect();

    start_flag = 0;

    /*------------------------------------------------------------
     *  메인 시세 송신 루프
     *  - WRITE_CNT > READ_CNT 동안 FIFO에서 읽기
     *  - 시장구분(선물/콜/풋) 판단 후 UDP 송신
     *  - 수신시각 차이만큼 sleep (실시간 재생)
     *------------------------------------------------------------*/
    while (START_S != JOB_END) {
        Stat_Save();

        while (WRITE_CNT > READ_CNT) {
            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
            if (R_Cnt < 0) {
                Log(SAM_FATAL, "cannot read file[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;

            /*----------------------------------------------------
             *  시장 구분: 데이터 앞부분의 코드로 판단
             *  +2(2): "01" → 선물(1)
             *  +2(2): "03" + +10(1): "2" → 콜옵션(2)
             *  +2(2): "03" + +10(1): "3" → 풋옵션(3)
             *----------------------------------------------------*/
            if (memcmp(&R_Fmt[0].Data[2], "01", 2) == 0) {
                Sise_Gbn = 1;   /* 선물 */
                /* TR코드별 데이터 크기 결정 */
                if (memcmp(R_Fmt[0].Data, "B6", 2) == 0)
                    sd_size = sizeof(SIF_B6);       /* 선물 호가 */
                else if (memcmp(R_Fmt[0].Data, "G7", 2) == 0)
                    sd_size = sizeof(SIF_G7);       /* 선물 체결+호가 */
                else if (memcmp(R_Fmt[0].Data, "A3", 2) == 0)
                    sd_size = sizeof(SIF_A3);       /* 선물 체결 */

                /*----------------------------------------------------
                 *  A7001: 자동주문 연동
                 *  - 선물 체결+호가(G7/A3) 중 종목코드 "01"인 경우
                 *  - SHM에 현재가 기록 후 FIFO에 "1" write (시그널)
                 *----------------------------------------------------*/
#if defined A7001

                if ( (sd_size != sizeof(SIF_B6)) &&
                        (memcmp(&R_Fmt[0].Data[S_H_SIZE], "01", 2) == 0) ) {
                    Set_Curr(R_Fmt[0].Data);

                    /* 자동주문에 시세 통지 (FIFO write "1") */
                    rt = write(FIFO_fd, "1", 1);
                    if (rt < 0)
                        Log(FIF_FATAL, "cannot write FIFO[%d][%d:%s]",
                                FIFO_fd, SYS_NO, SYS_STR);

                    /* FIFO 비우기 (잔여 데이터 제거) */
                    while (1) {
                        rt = read(FIFO_fd, tmp, sizeof(tmp));
#if defined __linux
                        if (rt == 0 || errno == EAGAIN)
#else
                            if (rt == 0)
#endif
                            break;
                    }

                    /* B6(호가) FIFO에도 시세 통지 */
                    rt = write(FIFO_fd1, "1", 1);
                    if (rt < 0)
                        Log(FIF_FATAL, "cannot write FIFO1[%d][%d:%s]",
                                FIFO_fd1, SYS_NO, SYS_STR);

                    while (1) {
                        rt = read(FIFO_fd1, tmp, sizeof(tmp));
#if defined __linux
                        if (rt == 0 || errno == EAGAIN)
#else
                            if (rt == 0)
#endif
                            break;
                    }
                }
                /*
                                else if ( (sd_size == sizeof(SIF_B6)) &&
                                     (memcmp (&R_Fmt[0].Data[S_H_SIZE], "01", 2) == 0) ) {
                                    Set_Curr (R_Fmt[0].Data);
                */

                /* 자동주문에 시세인지 */
                /*
                                    rt = write (FIFO_fd1, "1", 1);
                                    if (rt < 0)
                                    Log (FIF_FATAL, "cannot write FIFO1[%d][%d:%s]",
                                        FIFO_fd1, SYS_NO, SYS_STR);

                                    while (1) {
                #if defined __linux
                                        if (rt == 0 || errno == EAGAIN)
                #else
                                        if (rt == 0)
                #endif
                                        if (rt == 0)
                                            break;
                                    }
                                }
                */
#endif
            }
            else if ( (memcmp(&R_Fmt[0].Data[2], "03", 2) == 0) &&
                    (memcmp(&R_Fmt[0].Data[2+6], "2", 1) == 0) ) {
                Sise_Gbn = 2;   /* 콜옵션 */
                if (memcmp(R_Fmt[0].Data, "B6", 2) == 0)
                    sd_size = sizeof(SIO_B6);       /* 옵션 호가 */
                else if (memcmp(R_Fmt[0].Data, "G7", 2) == 0)
                    sd_size = sizeof(SIO_G7);       /* 옵션 체결+호가 */
                else if (memcmp(R_Fmt[0].Data, "A3", 2) == 0)
                    sd_size = sizeof(SIO_A3);       /* 옵션 체결 */
                /* 자동주문 로직 검증시 필요하나 시세 확인용으로만 개발 */
#if defined A7001
                Set_Curr(R_Fmt[0].Data);
#endif
            }
            else if ( (memcmp(&R_Fmt[0].Data[2], "03", 2) == 0) &&
                    (memcmp(&R_Fmt[0].Data[2+6], "3", 1) == 0) ) {
                Sise_Gbn = 3;   /* 풋옵션 */
                if (memcmp(R_Fmt[0].Data, "B6", 2) == 0)
                    sd_size = sizeof(SIO_B6);
                else if (memcmp(R_Fmt[0].Data, "G7", 2) == 0)
                    sd_size = sizeof(SIO_G7);
                else if (memcmp(R_Fmt[0].Data, "A3", 2) == 0)
                    sd_size = sizeof(SIO_A3);
                /* 자동주문 로직 검증시 필요하나 시세 확인용으로만 개발 */
#if defined A7001
                Set_Curr(R_Fmt[0].Data);
#endif
            }
            else {
                READ_CNT++;
                continue;   /* 미분류 → skip */
            }

            /*----------------------------------------------------
             *  실시간 재생: 수신시각 차이만큼 sleep
             *  - RecvTime2: HHMMSSuuuuuu (시분초+마이크로초)
             *  - 1초 이상 차이: sleep()
             *  - 10ms 이상: usleep()
             *  - 100us 이상: Microsec_Sleep() (nanosleep)
             *  - sleep_speed: 양수=빠르게(나누기), 음수=느리게(곱하기)
             *----------------------------------------------------*/
            if (start_flag == 0) {
                start_flag = 1;
                recv_sec = AtoIf(R_Fmt[0].RecvTime2,   2) * 60 * 60
                + AtoIf(R_Fmt[0].RecvTime2+2, 2) * 60
                + AtoIf(R_Fmt[0].RecvTime2+4, 2)
                + AtoIf(R_Fmt[0].RecvTime2+6, 6) / 1000000.0;
            }
            befo_sec = recv_sec;

            recv_sec = AtoIf(R_Fmt[0].RecvTime2,   2) * 60 * 60
            + AtoIf(R_Fmt[0].RecvTime2+2, 2) * 60
            + AtoIf(R_Fmt[0].RecvTime2+4, 2)
            + AtoIf(R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

            READ_CNT++;

            sltime = (recv_sec - befo_sec) * 1000000.0;

            /* 100 Microsec 미만은 sleep 안하고 처리함 */
            if ( 1.0 <= (recv_sec - befo_sec) ) {
                sltime = (recv_sec - befo_sec) / 1;
                if (sleep_speed < 0)
                    sleep(sltime * sleep_speed);
                else
                    sleep(sltime / sleep_speed);
            }
            else if (10000 <= sltime) {
                if (sleep_speed < 0)
                    usleep((sltime - 100) * sleep_speed);
                else
                    usleep((sltime - 100) / sleep_speed);
            }
            else if (100 <= sltime) {
                if (sleep_speed < 0)
                    Microsec_Sleep((sltime - 100) * sleep_speed);
                else
                    Microsec_Sleep((sltime - 100) / sleep_speed);
            }

            /* SHM에 시세 기록 */
            Set_Sise(R_Fmt[0].Data);

            /* UDP 송신: 각 IP주소의 해당 시장 소켓으로 전송 */
            for (i = 0; i < Max_IP_Addr; i++) {

                len = sizeof (Svr_Addr[i][Sise_Gbn - 1]);
                rt = sendto(Sockfd[i][Sise_Gbn - 1], R_Fmt[0].Data, sd_size,
                        0, (struct sockaddr *)&Svr_Addr[i][Sise_Gbn - 1], len);
                if (rt < 0) {
                    Log(UDP_FATAL, "Data Send Error [%d:%s] Sockfd [%d]", SYS_NO, SYS_STR, Sockfd[i][Sise_Gbn - 1]);
                    break;
                }

            }
        }

        sleep(10);  /* 데이터 없을 때 10초 대기 */
    }

    return;
}   /* End of PA_7000_MP () */

/*------------------------------------------------------------------------
 *  Socket_Connect: UDP 소켓 생성 및 바인드
 *  - IP주소별 × 시장별(선물/콜/풋) 3개 = 최대 12개 소켓
 *  - SO_SNDBUF/SO_RCVBUF/SO_BROADCAST 설정
 *  - A7001~A7003: 포트 60631+j
 *  - A7004: 포트 5572/5515/5516
 *------------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     rt, i = 0, j = 0, bufflen, oplen;

    bufflen = 1024 * 64;        /* 소켓 버퍼: 64KB */
    oplen   = sizeof(bufflen);

    for (i = 0; i < Max_IP_Addr; i++) {
        for (j = 0; j < 3; j++)     /* 0:선물, 1:콜옵션, 2:풋옵션 */ {
            Sockfd[i][j] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (Sockfd[i][j] < 0) {
                Log(UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
                return 0;
            }
            /* 송신 대상 주소 설정 */
            Svr_Addr[i][j].sin_family = AF_INET;
            inet_pton(AF_INET, &Svr_IP[20*i], &Svr_Addr[i][j].sin_addr.s_addr);
            /* 빌드 옵션별 포트 번호 */
#if defined(A7001) || defined(A7002) || defined(A7003)
            Svr_Addr[i][j].sin_port = htons(60631+j);
#endif
#if defined A7004
            if (j == 0)
                Svr_Addr[i][j].sin_port = htons(5572);     /* 선물 */
            else if (j == 1)
                Svr_Addr[i][j].sin_port = htons(5515);     /* 콜옵션 */
            else if (j == 2)
                Svr_Addr[i][j].sin_port = htons(5516);     /* 풋옵션 */
#endif

            Log(UDP_OK, "socket %d created [%s:%d]", i, &Svr_IP[20*i], 60631+j);

            /* 클라이언트 주소 (INADDR_ANY, 임의 포트) */
            Clnt_Addr[i][j].sin_family       = AF_INET;
            Clnt_Addr[i][j].sin_addr.s_addr  = htonl(INADDR_ANY);
            Clnt_Addr[i][j].sin_port         = htons(0);

            /* 소켓 옵션 설정 */
            rt = setsockopt(Sockfd[i][j], SOL_SOCKET, SO_SNDBUF, (void *)&bufflen, oplen);
            if (rt < 0)
                Log(UDP_WARN, "setsockopt SO_SNDBUF fail {%d:%s}", SYS_NO, SYS_STR);
            rt = setsockopt(Sockfd[i][j], SOL_SOCKET, SO_RCVBUF, (void *)&bufflen, oplen);
            if (rt < 0)
                Log(UDP_WARN, "setsockopt SO_RCVBUF fail {%d:%s}", SYS_NO, SYS_STR);

            /* 브로드캐스트 허용 */
            rt = setsockopt(Sockfd[i][j], SOL_SOCKET, SO_BROADCAST, (void *)&bufflen, oplen);
            if (rt < 0)
                Log(UDP_WARN, "setsockopt SO_BROADCAST fail {%d:%s}", SYS_NO, SYS_STR);
            rt = bind(Sockfd[i][j], (struct sockaddr *)&Clnt_Addr[i][j],
                    sizeof (Clnt_Addr[i][j]));
            if (rt < 0) {
                Log(UDP_FATAL, "Call옵션 Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
                return (NOTOK);
            }
        }
    }

    return 1;
}

/*************************************************************************
 *  Set_Curr: 현재가 SHM 기록 (자동주문용)
 *
 *  선물(Sise_Gbn=1):
 *    Shm_Futures[i].Futures_CURR     - 최신 현재가
 *    Shm_Futures[i].Futures_CURR_Arry - 최근 30건 이력 (ring buffer)
 *    Shm_Futures[i].Futures_CHE_Arry  - 체결 이력 (ring buffer, 최대 1000건)
 *
 *  옵션(Sise_Gbn=2,3):
 *    Shm_Options[i].Options_CURR     - 최신 현재가
 *    Shm_Options[i].Options_CURR_Arry - 최근 30건 이력
 *    Shm_Options[i].Options_CHE_Arry  - 체결 이력
 *
 *  TR코드별 처리:
 *    G7(체결+호가): CHE_Arry + CURR 모두 갱신
 *    A3(체결만): 직전 G7으로 초기화 후 A3로 덮어쓰기
 *    B6(호가만): CURR만 갱신 (시장상태 포함)
 *************************************************************************/
void    Set_Curr(char *p_buf) {
    int     i;

    if (Sise_Gbn == 1)      /* 선물 */ {
        /* SEQ 번호 → 배열 인덱스 (1-based → 0-based) */
        i = AtoIf(&p_buf[S_H_SIZE], 2);
        i --;

        /* CURR_Arry Key 갱신 (ring buffer, 0~29) */
        Shm_Futures[i].Befor_CURR_Arry_Key = Shm_Futures[i].CURR_Arry_Key;

        if ( (Shm_Futures[i].CURR_Arry_Key <   0) ||
                (Shm_Futures[i].CURR_Arry_Key >= 29)   )
        Shm_Futures[i].CURR_Arry_Key = 0;
        else
            Shm_Futures[i].CURR_Arry_Key ++;

        if (memcmp(p_buf, "G7014", 5) == 0)            /* 호가 + 체결 */ {
            /* 체결 CHE_Arry Key 갱신 (ring buffer, 0~999) */
            Shm_Futures[i].Befor_CHE_Arry_Key = Shm_Futures[i].CHE_Arry_Key;

            if ( (Shm_Futures[i].CHE_Arry_Key <   0) ||
                    (Shm_Futures[i].CHE_Arry_Key >= 999)   ) {
                Shm_Futures[i].CHE_Cnt = Shm_Futures[i].CHE_Cnt + 1;
                Shm_Futures[i].CHE_Arry_Key = 0;
            }
            else
                Shm_Futures[i].CHE_Arry_Key ++;

            /* CHE_Arry에 기록 (호가접수시 뒷 8자리 제외) */
            memcpy(Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
                    p_buf, sizeof (SIF_G7) - 8);

            /* CURR(최신 현재가)에도 기록 */
            memcpy(Shm_Futures[i].Futures_CURR.tr_gbn, p_buf, sizeof (SIF_G7) - 8);
        }
        else if (memcmp(p_buf, "A3014", 5) == 0)       /* 체결 */ {
            /* 체결 CHE_Arry Key 갱신 */
            Shm_Futures[i].Befor_CHE_Arry_Key = Shm_Futures[i].CHE_Arry_Key;

            if ( (Shm_Futures[i].CHE_Arry_Key <   0) ||
                    (Shm_Futures[i].CHE_Arry_Key >= 999)   ) {
                Shm_Futures[i].CHE_Cnt = Shm_Futures[i].CHE_Cnt + 1;
                Shm_Futures[i].CHE_Arry_Key = 0;
            }
            else
                Shm_Futures[i].CHE_Arry_Key ++;

            /* 직전 G7으로 초기화 후 A3(체결)로 덮어쓰기 */
            memcpy(Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
                    Shm_Futures[i].Futures_G7.tr_gbn, sizeof (SIF_G7) - 8);
            memcpy(Shm_Futures[i].Futures_CHE_Arry[Shm_Futures[i].CHE_Arry_Key].tr_gbn,
                    p_buf, sizeof (SIF_A3));

            memcpy(Shm_Futures[i].Futures_CURR.tr_gbn, p_buf, sizeof (SIF_A3));
        }
        else if (memcmp(p_buf, "B6014", 5) == 0)       /* 호가 */ {
            /* CURR에 호가 데이터 기록 (시장상태 포함) */
            memcpy(Shm_Futures[i].Futures_CURR.tr_gbn,
                    p_buf, sizeof (Shm_Futures[0].Futures_CURR.tr_gbn));
            memcpy(Shm_Futures[i].Futures_CURR.market_state_gubun,
                    &p_buf[S_H_SIZE+2], sizeof (SIF_B6) - S_H_SIZE + 2);
        }
        else {
            SLog(USR_OK, "SISE SKIP [%10.10s]", p_buf);
            return;
        }

        /* CURR → CURR_Arry에 이력 저장 */
        memcpy(Shm_Futures[i].Futures_CURR_Arry[Shm_Futures[i].CURR_Arry_Key].tr_gbn,
                Shm_Futures[i].Futures_CURR.tr_gbn, sizeof (SIF_G7));
    }
    else    /* 옵션 (콜/풋) */ {
        /* SEQ 번호 → 배열 인덱스 (옵션은 3자리) */
        i = AtoIf(&p_buf[S_H_SIZE], 3);
        i --;

        /* CURR_Arry Key 갱신 (ring buffer) */
        Shm_Options[i].Befor_CURR_Arry_Key = Shm_Options[i].CURR_Arry_Key;

        if ( (Shm_Options[i].CURR_Arry_Key <   0) ||
                (Shm_Options[i].CURR_Arry_Key >= 29)   )
        Shm_Options[i].CURR_Arry_Key = 0;
        else
            Shm_Options[i].CURR_Arry_Key ++;

        if (memcmp(p_buf, "G7034", 5) == 0)            /* 호가 + 체결 */ {
            /* 체결 CHE_Arry Key 갱신 */
            Shm_Options[i].Befor_CHE_Arry_Key = Shm_Options[i].CHE_Arry_Key;

            if ( (Shm_Options[i].CHE_Arry_Key <   0) ||
                    (Shm_Options[i].CHE_Arry_Key >= 999)   ) {
                Shm_Options[i].CHE_Cnt = Shm_Options[i].CHE_Cnt + 1;
                Shm_Options[i].CHE_Arry_Key = 0;
            }
            else
                Shm_Options[i].CHE_Arry_Key ++;

            /* CHE_Arry + CURR 기록 */
            memcpy(Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
                    p_buf, sizeof (SIO_G7) - 8);

            memcpy(Shm_Options[i].Options_CURR.tr_gbn, p_buf, sizeof (SIO_G7) - 8);
        }
        else if (memcmp(p_buf, "A3034", 5) == 0)       /* 체결 */ {
            /* 체결 CHE_Arry Key 갱신 */
            Shm_Options[i].Befor_CHE_Arry_Key = Shm_Options[i].CHE_Arry_Key;

            if ( (Shm_Options[i].CHE_Arry_Key <   0) ||
                    (Shm_Options[i].CHE_Arry_Key >= 999)   ) {
                Shm_Options[i].CHE_Cnt = Shm_Options[i].CHE_Cnt + 1;
                Shm_Options[i].CHE_Arry_Key = 0;
            }
            else
                Shm_Options[i].CHE_Arry_Key ++;

            /* 직전 G7으로 초기화 후 A3로 덮어쓰기 */
            memcpy(Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
                    Shm_Options[i].Options_G7.tr_gbn, sizeof (SIO_G7) - 8);
            memcpy(Shm_Options[i].Options_CHE_Arry[Shm_Options[i].CHE_Arry_Key].tr_gbn,
                    p_buf, sizeof (SIO_A3));

            memcpy(Shm_Options[i].Options_CURR.tr_gbn, p_buf, sizeof (SIO_A3));
        }
        else if (memcmp(p_buf, "B6034", 5) == 0)       /* 호가 */ {
            /* CURR에 호가(시장상태) 기록 */
            memcpy(Shm_Options[i].Options_CURR.market_state_gubun,
                    &p_buf[S_H_SIZE+3], sizeof (SIO_B6) - S_H_SIZE + 3);
        }
        else {
            SLog(USR_OK, "SISE SKIP [%10.10s]", p_buf);
            return;
        }

        /* CURR → CURR_Arry에 이력 저장 */
        memcpy(&Shm_Options[i].Options_CURR_Arry[Shm_Options[i].CURR_Arry_Key],
                Shm_Options[i].Options_CURR.tr_gbn, sizeof (SIO_G7));
    }

    return;
}   /* End of Set_Curr ()   */

/*************************************************************************
 *  Set_Sise: 시세 데이터를 SHM에 기록 (기본/호가/체결)
 *
 *  선물(Sise_Gbn=1): Shm_Futures[i].Futures_B6/A3/G7
 *  옵션(Sise_Gbn=2,3): Shm_Options[i].Options_B6/A3/G7
 *************************************************************************/
void    Set_Sise(char *p_buf) {
    int     i;

    if (Sise_Gbn == 1)                      /* 지수선물시세 */ {
        i = AtoIf(&p_buf[S_H_SIZE], 2);
        i --;

        if (memcmp(p_buf, "B6014", 5) == 0)                        /* 호가 */ {
            memcpy(Shm_Futures[i].Futures_B6.tr_gbn, p_buf, sizeof (SIF_B6));
        }
        else if (memcmp(p_buf, "A3014", 5) == 0)                   /* 체결 */ {
            memcpy(Shm_Futures[i].Futures_A3.tr_gbn, p_buf, sizeof (SIF_A3));
        }
        else if (memcmp(p_buf, "G7014", 5) == 0)           /* 체결 + 호가 */ {
            memcpy(Shm_Futures[i].Futures_G7.tr_gbn, p_buf, sizeof (SIF_G7) - 8);
        }

    }
    else                                    /* 지수옵션시세 */ {
        i = AtoIf(&p_buf[S_H_SIZE], 3);
        i --;

        if (memcmp(p_buf, "B6034", 5) == 0)                        /* 호가 */ {
            memcpy(&Shm_Options[i].Options_B6, p_buf, sizeof (SIO_B6));
        }
        else if (memcmp(p_buf, "A3034", 5) == 0)                   /* 체결 */ {
            memcpy(&Shm_Options[i].Options_A3, p_buf, sizeof (SIO_A3));
        }
        else if (memcmp(p_buf, "G7034", 5) == 0)           /* 체결 + 호가 */ {
            memcpy(&Shm_Options[i].Options_G7, p_buf, sizeof (SIO_G7) - 8);
        }

    }

    return;
}   /* End of Set_Sise ()   */

/*************************************************************************
 *  Microsec_Sleep: 마이크로초 단위 sleep
 *  - nanosleep 사용 (usleep보다 정밀)
 *  - msec: 대기할 마이크로초
 *************************************************************************/
void    Microsec_Sleep(int msec) {
    int             rt;
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = msec * 1000;   /* 마이크로초 → 나노초 변환 */

    rt = nanosleep(&ts, NULL);

    if (rt == -1)
        Log(SYS_ERROR, "nanosleep fail {%d:%s}", SYS_NO, SYS_STR);

    return;
}   /* End of Microsec_Sleep () */

/*************************************************************************
 *  End of program (pa_7000_mp.c)
 *************************************************************************/
