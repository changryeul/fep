#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 시세 송신 (UDP) - 복합 데이터
#   File    : pa_7030_us.c
#
#   설명  : FIFO에서 읽은 시세 데이터를 TR코드별로 분류하여
#             여러 종류의 UDP 소켓(선물/주식호가/주식체결)으로 송신.
#             시세 수신 시간 간격을 재현하여 전송 속도를 조절한다.
#             수신처당 3개 포트(15572/19621/19616)를 사용.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE       790                 /* 수신 데이터 크기 (790바이트) */
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     ITEM_CODE       INT_SEQ             /* 종목 인덱스 (시퀀스 번호 활용) */

/*-------------------------------------------------------------------------
    Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000               /* 최대 버퍼 크기 */
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */

/*-------------------------------------------------------------------------
    Program Global Variable
-------------------------------------------------------------------------*/
/*
 * 소켓/주소를 2차원 배열로 관리:
 *   1차원[4]: 수신처 (최대 4개 IP)
 *   2차원[4]: 데이터 종류별 포트 (선물, 주식호가, 주식체결, 예비)
 */
int     Sockfd[4][4], id = -1;
struct  sockaddr_in Svr_Addr[4][4], Clnt_Addr[4][4];

/*-------------------------------------------------------------------------
    Function Declarations
-------------------------------------------------------------------------*/
void    PA_7030_US(void);
void    Microsec_Sleep(int);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 복합 시세 UDP 송신 루프 실행.       */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */

    PA_7030_US();                              /* 메인 처리 루프 */

    Exit_Process();                             /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7030_US: 메인 처리 루프                                            */
/*    1) UDP 소켓 연결 (수신처당 3개 포트)                             */
/*    2) FIFO에서 시세 데이터 읽기                                       */
/*    3) TR코드별 데이터 종류(Sise_Gbn) 판별                          */
/*    4) 수신 시간 간격에 따라 sleep 후 UDP 송신                        */
/*----------------------------------------------------------------------*/
void    PA_7030_US(void) {
    int     i, rt, add_len, R_Cnt, Sise_Gbn, sd_size, len, sleep_speed;
    int     start_flag, sltime;
    double  recv_sec, befo_sec;
    char    tmp[4], bumun[4], send_data[1000];

    FILE_BUFF_FORMAT    R_Fmt[1];               /* FIFO 읽기용 버퍼 (1건) */

    /* 주소 구조체 초기화 */
    bzero((unsigned char *)Svr_Addr, sizeof (struct sockaddr_in) * 3);
    bzero((unsigned char *)Clnt_Addr, sizeof (struct sockaddr_in) * 3);

    /* 모든 소켓 FD를 -1(미연결)로 초기화 */
    for (i = 0; i < 4; i++) {
        Sockfd[i][0] = -1;
        Sockfd[i][1] = -1;
        Sockfd[i][2] = -1;
        Sockfd[i][3] = -1;
    }

    /* UDP 소켓 생성 */
    rt = Socket_Connect();
    if (rt == NOTOK)
        return;

    start_flag = 0;

    /* 전송 속도 조절 값 (0이면 1로 보정, 나눗셈 0 방지) */
    if (DELAY_TIME == 0)
        sleep_speed = 1;
    else
        sleep_speed = DELAY_TIME;

    /* === 메인 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* FIFO에 쓰기 카운트가 읽기 카운트보다 큰 동안 데이터 읽기 */
        while (WRITE_CNT > READ_CNT) {
            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read file[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;                          /* 읽을 데이터 없음 */

            /*
             * TR코드 앞 5바이트로 데이터 종류(Sise_Gbn) 및 크기(sd_size) 판별
             *   G7014: 선물 현재가+호가 → Sise_Gbn=1
             *   B6014: 선물 호가         → Sise_Gbn=1
             *   A3014: 선물 체결         → Sise_Gbn=1
             *   B7021: 주식 호가         → Sise_Gbn=2
             *   A3021: 주식 체결         → Sise_Gbn=3
             */
            if (memcmp(R_Fmt[0].Data, "G7014", 5) == 0) {
                Sise_Gbn = 1;
                sd_size = sizeof(SIF_G7);
            }
            else if (memcmp(R_Fmt[0].Data, "B6014", 5) == 0) {
                Sise_Gbn = 1;
                sd_size = sizeof(SIF_B6);
            }
            else if (memcmp(R_Fmt[0].Data, "A3014", 5) == 0) {
                Sise_Gbn = 1;
                sd_size = sizeof(SIF_A3);
            }
            else if (memcmp(R_Fmt[0].Data, "B7021", 5) == 0) {
                Sise_Gbn = 2;
                sd_size = sizeof(STOCK_B7);
            }
            else if (memcmp(R_Fmt[0].Data, "A3021", 5) == 0) {
                Sise_Gbn = 3;
                sd_size = sizeof(STOCK_A3);
            }

            /*
             * 수신 시간 기반 전송 속도 조절:
             * RecvTime2에서 시:분:초.마이크로초를 초(double)로 변환하여
             * 이전 데이터와의 시간 차이만큼 대기 후 송신
             */
            recv_sec = AtoIf(R_Fmt[0].RecvTime2,   2) * 60 * 60
            + AtoIf(R_Fmt[0].RecvTime2+2, 2) * 60
            + AtoIf(R_Fmt[0].RecvTime2+4, 2)
            + AtoIf(R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

            if (start_flag == 0) {
                start_flag = 1;
                befo_sec = recv_sec;            /* 첫 데이터: 기준 시간 설정 */
            }

            READ_CNT++;

            /* 이전 데이터보다 늦은 시간이면 차이만큼 대기 */
            if (befo_sec < recv_sec) {
                sltime = (recv_sec - befo_sec) * 1000000.0;
                Log(USR_OK, "sleep [%d] sleep_speed [%d]", sltime, sleep_speed);

                if (1.0 <= (recv_sec - befo_sec)) {
                    /* 1초 이상 차이: sleep() 사용 (초 단위) */
                    sltime = (recv_sec - befo_sec) / 1;
                    if (sleep_speed < 0)
                        sleep(sltime * sleep_speed);
                    else
                        sleep(sltime / sleep_speed);
                }
                else if (10000 <= sltime) {
                    /* 10ms 이상 차이: usleep() 사용 (마이크로초) */
                    if (sleep_speed < 0)
                        usleep((sltime - 100) * sleep_speed);
                    else
                        usleep((sltime - 100) / sleep_speed);
                }
                else if (100 <= sltime) {
                    /* 100us 이상 차이: nanosleep() 사용 (나노초) */
                    if (sleep_speed < 0)
                        Microsec_Sleep((sltime - 100) * sleep_speed);
                    else
                        Microsec_Sleep((sltime - 100) / sleep_speed);
                }
                /* 100us 미만 차이: sleep 없이 즉시 처리 */

                befo_sec = recv_sec;
            }

            /* 모든 수신처(최대 4개)에 UDP 시세 데이터 송신 */
            for (i = 0; i < 4; i++) {
                len = sizeof (Svr_Addr[i][Sise_Gbn - 1]);

                rt = sendto(Sockfd[i][Sise_Gbn - 1], R_Fmt[0].Data, sd_size,
                        0, (struct sockaddr *)&Svr_Addr[i][Sise_Gbn - 1], len);

                if (rt < 0) {
                    Log(UDP_FATAL, "Data Send Error [%d:%s] Sockfd [%d]", SYS_NO, SYS_STR, Sockfd[i][Sise_Gbn - 1]);
                    break;
                }
            }
        }
    }
}

/*----------------------------------------------------------------------*/
/*  Socket_Connect: UDP 소켓 생성                                       */
/*    수신처 4개 × 데이터종류 3개 = 최대 12개 소켓 생성.             */
/*    포트: 15572(선물), 19621(주식호가), 19616(주식체결).              */
/*    Return: 1=성공, 0=실패                                            */
/*----------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     i, j;
    char    Svr_IP[20];                         /* 변환된 IP 주소 문자열 */
    static const int ports[3] = { 15572, 19621, 19616 };

    for (i = 0; i < 4; i++) {
        /* 설정에서 IP 주소를 읽어 변환 */
        format_ip_addr(ACCNO(D_K,i).ip_addr, Svr_IP, sizeof(Svr_IP));

        /* 각 포트별 소켓 생성: 송수신 버퍼 각 64KB */
        for (j = 0; j < 3; j++) {
            Sockfd[i][j] = init_udp_socket(&Svr_Addr[i][j], &Clnt_Addr[i][j],
                    Svr_IP, ports[j],
                    1024*64, 1024*64, 1);
            if (Sockfd[i][j] < 0) return 0; /* 소켓 생성 실패 */

                Log(UDP_OK, "socket %d created [%s]", i, Svr_IP);
        }
    }

    return 1;
}

/*************************************************************************
    Function        : Microsec_Sleep
    Parameters IN   : msec - 대기 시간 (마이크로초 단위)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : nanosleep()을 사용한 마이크로초 단위 대기 함수.
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
    End of program (pa_7030_us.c)
*************************************************************************/
