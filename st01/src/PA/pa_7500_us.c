#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 시세Real & Udp 송신
#             모든 시세처리함 (지수선물/Call옵션/Put옵션)
#   File    : pa_7500_us.c
#
#   설명  : FIFO에서 시세 데이터를 읽어 TR코드별로 분류한 후
#             UDP 소켓으로 외부 서버에 송신하는 프로세스.
#             A7102=채권시세, A7103=채권장운영, A7202=금융파생시세,
#             A7203=금융파생장운영 빌드 구분.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 *   A7103/A7203: M4(장운영정보)  → 100바이트
 *   A7102      : 채권시세        → 700바이트
 *   A7202      : 금융파생시세    → 450바이트
 */
#if defined(A7103) || defined(A7203)
#define     DATA_SIZE       100
#elif defined A7102
#define     DATA_SIZE       700
#elif defined A7202
#define     DATA_SIZE       450
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
/*
 * 빌드 옵션별 UDP 포트 번호 (하드코딩)
 * TODO: 포트정보 재설정 필요 (2025)
 */
#if defined A7102
#define     SVR_PORT_NO     60631               /* 채권시세 */
#elif defined A7103
#define     SVR_PORT_NO     60632               /* 채권장운영 */
#elif defined A7202
#define     SVR_PORT_NO     60633               /* 금융파생시세 */
#elif defined A7203
#define     SVR_PORT_NO     60634               /* 금융파생장운영 */
#endif

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         sleep_speed;                        /* 전송 간격 제어용 (미사용) */
int         Sockfd[4], id = -1, Max_IP_Addr;    /* UDP 소켓 FD, 최대 수신처 수 */
char        Svr_IP[20*4];                       /* 수신측 IP 주소 (최대 4개 × 20바이트) */
struct      sockaddr_in Svr_Addr[4];            /* 수신측 주소 배열 */
struct      sockaddr_in Clnt_Addr[4];           /* 로컬 주소 배열 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_7500_US(int, char **);
void    Set_Curr(char *);
void    Set_Sise(char *);
void    Microsec_Sleep(int);
int     Socket_Connect(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 시세 UDP 송신 루프 실행.          */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */
    PA_7500_US(argc, argv);                    /* 메인 처리 루프 */
    Exit_Process();                            /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7500_US: 메인 처리 루프                                            */
/*    1) FIFO 열고 UDP 소켓 연결                                      */
/*    2) FIFO에서 시세 데이터 읽기                                       */
/*    3) TR코드별 분류 후 UDP로 송신                                 */
/*----------------------------------------------------------------------*/
void    PA_7500_US(int argc, char **argv) {
    int     i, idx, rt, len, R_Cnt, flag, start_flag, sltime, sd_size;
    int     FIFO_fd, FIFO_fd1;
    char    fifo_name[100], fifo_name1[100], tmp[128], bumun[4], send_data[1000];
    char    r_buf[1024];
    double  recv_sec, befo_sec;
    FILE_BUFF_FORMAT    R_Fmt[1];               /* FIFO 읽기용 버퍼 (1건) */

    /* FIFO 파일 경로 생성 */
    sprintf(bumun, "%s", _SubSystem_Name);
    LtoU(bumun, 2);                            /* 소문자 → 대문자 변환 */

    /* 수신 서버 IP 설정 (하드코딩) */
    Max_IP_Addr = 1;
    sprintf(Svr_IP, "%d.%d.%d.%d",
            TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0),
            TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));

    /* UDP 소켓 생성 */
    rt = Socket_Connect();

    start_flag = 0;

    /* === 메인 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* FIFO에 쓰기 카운트가 읽기 카운트보다 큰 동안 데이터 읽기 */
        while (WRITE_CNT > READ_CNT2) {
            R_Cnt = F_R(PS_R_2, (void *)R_Fmt, 1);
            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read file[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0) {
                break;                          /* 읽을 데이터 없음 */
            }

            memset(send_data, 0x00, sizeof(send_data));

            /*
             * TR코드 앞 5바이트로 시세 유형 판별 후 해당 구조체 크기만큼 복사
             * 각 빌드 옵션별로 처리하는 TR코드가 다름
             */
#if defined A7102
            /* [채권시세] B6=호가, A3=체결, G7=체결+호가, A6=종목마감 */
            if (memcmp(R_Fmt[0].Data, "B601K", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_B601K));        /* 우선호가(채권) */
            else if (memcmp(R_Fmt[0].Data, "A301K", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_A301K));        /* 체결(채권) */
            else if (memcmp(R_Fmt[0].Data, "G701K", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_G701K));        /* 체결+호가(채권) */
            else if (memcmp(R_Fmt[0].Data, "A601K", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_A601K));        /* 종목마감(채권) */
#elif defined A7103
            /* [채권장운영] M4=전체장운영정보 */
            if (memcmp(R_Fmt[0].Data, "M401K", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_M401K));        /* 장운영정보(채권) */
#elif defined A7202
            /* [금융파생시세] B6=호가, A3=체결, G7=체결+호가, A6=종목마감 */
            if (memcmp(R_Fmt[0].Data, "B606F", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_B601F));        /* 우선호가(파생) */
            else if (memcmp(R_Fmt[0].Data, "A306F", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_A301F));        /* 체결(파생) */
            else if (memcmp(R_Fmt[0].Data, "G706F", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_G701F));        /* 체결+호가(파생) */
            else if (memcmp(R_Fmt[0].Data, "A606F", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_A601F));        /* 종목마감(파생) */
#elif defined A7203
            /* [금융파생장운영] M4=전체장운영정보 */
            if (memcmp(R_Fmt[0].Data, "M406F", 5) == 0)
                memcpy(send_data, R_Fmt[0].Data, sizeof (CO_M401F));        /* 장운영정보(파생) */
#endif
            /* 유효한 데이터가 있으면 (10바이트 이상) UDP 송신 */
            if (strlen(send_data) > 10) {
                len = sizeof (Svr_Addr[0]);

                for (i = 0; i < Max_IP_Addr; i++) {
                    rt = sendto(Sockfd[i], send_data, strlen(send_data),
                            0, (struct sockaddr *)&Svr_Addr[i], len);
                    if (rt < 0) {
                        Log(UDP_FATAL, "Data Send Error [%d:%s] Sockfd [%d]", SYS_NO, SYS_STR, Sockfd[i]);
                        break;
                    }

                    Log(USR_OK, "UDP S [%.100s](%d)", send_data, strlen(send_data));
                }
            }
        }

        sleep(10);                              /* 10초 대기 후 다음 루프 */
    }

    return;
}   /* End of PA_7500_US () */

/*----------------------------------------------------------------------*/
/*  Socket_Connect: UDP 소켓 생성 및 수신측 주소 설정                   */
/*    Svr_IP 배열에 저장된 IP로 Max_IP_Addr개 소켓 생성.                */
/*    Return: 1=성공, 0=실패                                            */
/*----------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     i;

    for (i = 0; i < Max_IP_Addr; i++) {
        /* UDP 소켓 생성: 송수신 버퍼 각 64KB */
        Sockfd[i] = init_udp_socket(&Svr_Addr[i], &Clnt_Addr[i],
                &Svr_IP[20*i], SVR_PORT_NO,
                1024*64, 1024*64, 0);
        if (Sockfd[i] < 0) return 0;            /* 소켓 생성 실패 */

            Log(UDP_OK, "socket %d created [%s:%d]", i, &Svr_IP[20*i], SVR_PORT_NO);
    }

    return 1;
}

/*************************************************************************
    Function        : Microsec_Sleep
    Parameters IN   : msec - 대기 시간 (마이크로초 단위)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : nanosleep()을 사용한 마이크로초 단위 대기 함수.
                      msec를 나노초로 변환하여 정밀 대기한다.
*************************************************************************/
void    Microsec_Sleep(int msec) {
    int             rt;
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = msec * 1000;                   /* 마이크로초 → 나노초 변환 */

    rt = nanosleep(&ts, NULL);

    if (rt == -1)
        Log(SYS_ERROR, "nanosleep fail {%d:%s}", SYS_NO, SYS_STR);

    return;
}   /* End of Microsec_Sleep () */

/*************************************************************************
    End of program (pa_7500_us.c)
*************************************************************************/
