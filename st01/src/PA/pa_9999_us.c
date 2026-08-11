#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 계좌손익정보송신 (UDP)
#   File    : pa_9999_us.c
#
#   설명  : SHM에 저장된 계좌별 손익/수수료/평가손익/리스크 정보를
#             주기적으로 UDP 패킷으로 외부 클라이언트에 송신하는 프로세스.
#             A9999=일반계좌(ACC_NO_CNT개), A9998=ELW계좌(ELW_ACC_NO_CNT개).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     SVR_PORT_NO     UDP_PORT(D_K,P_K,0) /* 설정에서 읽은 UDP 포트 번호 */

/*-------------------------------------------------------------------------
    Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000               /* 최대 버퍼 크기 */
#define     DATA_TIME       5 * 1000            /* Poll 대기 시간 (5초, 밀리초 단위) */

/*-------------------------------------------------------------------------
    Program Global Variable
-------------------------------------------------------------------------*/
/*
 * 빌드 옵션별 계좌 범위 설정:
 *   A9999: 일반계좌 — 0번부터 ACC_NO_CNT개
 *   A9998: ELW계좌  — ACC_NO_CNT번부터 ELW_ACC_NO_CNT개
 */
#if defined A9999
#define     Acc_Cnt     ACC_NO_CNT
#define     Acc_No      0
#elif defined A9998
#define     Acc_Cnt     ELW_ACC_NO_CNT
#define     Acc_No      ACC_NO_CNT
#endif
int     Sockfd[Acc_Cnt], id = -1;               /* UDP 소켓 FD 배열 */
struct  sockaddr_in Svr_Addr[Acc_Cnt];          /* 수신측 주소 배열 */
struct  sockaddr_in Clnt_Addr[Acc_Cnt];         /* 로컬 주소 배열 */
UDP_OUT_DATA        Wdata[Acc_Cnt];             /* 계좌별 송신 데이터 배열 */
UDP_OUT_HEADER      Wheader;                    /* UDP 패킷 헤더 */

/*-------------------------------------------------------------------------
    Function Declarations
-------------------------------------------------------------------------*/
void    PA_9999_US(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 UDP 송신 루프 실행.             */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */

    PA_9999_US();                              /* 메인 처리 루프 */

    Exit_Process();                             /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_9999_US: 메인 처리 루프                                            */
/*    1) Poll로 5초 주기 대기                                         */
/*    2) SHM에서 계좌별 손익/수수료/리스크 정보 수집                 */
/*    3) UDP 패킷 조립 후 각 클라이언트에 송신                            */
/*----------------------------------------------------------------------*/
void    PA_9999_US(void) {
    int     i, rt, add_len;
    char    send_data[1000];                    /* UDP 송신 버퍼 */
    char    packet_size[5];                     /* 전체 패킷 크기 문자열 */
    char    data_size[4];                       /* 개별 데이터 크기 문자열 */
    char    data_cnt[3];                        /* 데이터 건수 문자열 */

    /* 주소 구조체 초기화 */
    bzero((unsigned char *)Svr_Addr,
            sizeof (struct sockaddr_in) * Acc_Cnt);
    bzero((unsigned char *)Clnt_Addr,
            sizeof (struct sockaddr_in) * Acc_Cnt);

    Sockfd[0] = -1;

    /* UDP 소켓 생성 및 연결 */
    rt = Socket_Connect();
    if (rt == NOTOK)
        return;

    Log(USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

    /* === 메인 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        rt = Poll_File(DATA_TIME);             /* 5초 주기 대기 */

        /* 송신 버퍼 초기화 */
        memset(&Wdata, 0x00, sizeof(UDP_OUT_DATA) * Acc_Cnt);
        memset(&Wheader, 0x00, sizeof(Wheader));
        memset(packet_size, 0x00, sizeof(packet_size));
        memset(data_size, 0x00, sizeof(data_size));
        memset(send_data, 0x00, sizeof(send_data));
        memset(data_cnt, 0x00, sizeof(data_cnt));

        if (rt == 1) {
            /* 타임아웃: 하트비트 패킷 송신 (빈 데이터) */
            memcpy(send_data, "0010P00000", sizeof(UDP_OUT_HEADER));
            Log(USR_OK, "poll timeout <%d>", INT_SEQ);
        }
        else if (rt == -1)
            continue;                           /* 오류 → 다음 루프 */
        else {
            /*
             * UDP 패킷 조립:
             *   헤더: Length(4) + Dgbn(1,'D') + Dlength(3) + Data_Cnt(2) = 10바이트
             *   바디: Acc_Cnt개 × UDP_OUT_DATA (계좌별 손익정보)
             */
            sprintf(packet_size, "%04d",
                    sizeof(UDP_OUT_HEADER) + Acc_Cnt * sizeof(UDP_OUT_DATA));
            sprintf(data_size, "%03d", sizeof(UDP_OUT_DATA));
            sprintf(data_cnt, "%02d", Acc_Cnt);

            memcpy(Wheader.Length, packet_size, sizeof(Wheader.Length));
            memcpy(Wheader.Dgbn, "D", sizeof(Wheader.Dgbn));
            memcpy(Wheader.Dlength, data_size, sizeof(Wheader.Dlength));
            memcpy(Wheader.Data_Cnt, data_cnt, sizeof(Wheader.Data_Cnt));

            /* 각 계좌의 손익 데이터를 SHM에서 수집 */
            for (i = 0; i < Acc_Cnt; i++) {
                /* 계좌 구분 코드 */
                memcpy(Wdata[i].Ap_Gbn, ACCNO(D_K,i+Acc_No).aptype_code,
                        sizeof(Wdata[i].Ap_Gbn));

                /* 매매손익 (실현손익) */
                sprintf(Wdata[i].Acc_real_prft, "%10d",
                        ACCNO(D_K,i+Acc_No).acc_real_prft);

                /* 수수료 */
                sprintf(Wdata[i].Acc_fee, "%10d", ACCNO(D_K,i+Acc_No).acc_fee);

                /* 평가손익 (전체 평가손익 - 이월 평가손익) */
                sprintf(Wdata[i].Acc_ver_prft, "%10d",
                        ACCNO(D_K,i+Acc_No).acc_ver_prft
                        - ACCNO(D_K,i+Acc_No).over_acc_ver_prft);

                /* 리스크 한도 구분 플래그 */
                sprintf(Wdata[i].Risk_Gbn, "%1d", ACCNO(D_K,i+Acc_No).risk_flag);

                /* 기타 리스크 구분 플래그 */
                sprintf(Wdata[i].Etc_Risk_Gbn, "%1d", ACCNO(D_K,i+Acc_No).etc_risk_flag);
            }

            /* 헤더 + 데이터를 하나의 패킷으로 조립 */
            memcpy(send_data, &Wheader, sizeof(UDP_OUT_HEADER));
            memcpy(&send_data[sizeof(UDP_OUT_HEADER)],
                    &Wdata, sizeof(UDP_OUT_DATA) * Acc_Cnt);
        }

        /* 설정된 대기 시간만큼 지연 (timeout × 10ms) */
        usleep(PROC(D_K,P_K).timeout * 10000);

        /* 모든 접속처에 UDP 패킷 송신 */
        add_len = sizeof (Svr_Addr[0]);

        for (i = 0; i < Acc_Cnt; i++) {
            /* IP가 "000"으로 시작하면 미설정 → 건너뜀 */
            if (memcmp(ACCNO(D_K,i+Acc_No).ip_addr, "000", 3) == 0)
                continue;

            rt = sendto(Sockfd[i], send_data, strlen(send_data),
                    0, (struct sockaddr *)&Svr_Addr[i], add_len);
            if (rt < 0) {
                Log(UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
                break;
            }
        }

        /* TEST 환경에서만 송신 로그 출력 */
        if (memcmp(_FEP_DIV, "TEST", 4) == 0)
            Log(USR_OK, "UDP S [%.10s](%d)", send_data, strlen(send_data));
    }
}

/*----------------------------------------------------------------------*/
/*  Socket_Connect: UDP 소켓 생성 및 수신측 주소 설정                   */
/*    설정(ACCNO)에서 IP를 읽어 Acc_Cnt개 UDP 소켓을 생성한다.         */
/*    IP가 "000"으로 시작하는 미설정 계좌는 건너뜀.                     */
/*    Return: 1=성공, 0=실패                                            */
/*----------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     i;
    char    Svr_IP[20];                         /* 변환된 IP 주소 문자열 */

    for (i = 0; i < Acc_Cnt; i++) {
        /* IP가 "000"으로 시작하면 미설정 → 건너뜀 */
        if (memcmp(ACCNO(D_K,i+Acc_No).ip_addr, "000", 3) == 0)
            continue;

        /* 설정에서 IP 주소를 읽어 "x.x.x.x" 형식으로 변환 */
        format_ip_addr(ACCNO(D_K,i+Acc_No).ip_addr, Svr_IP, sizeof(Svr_IP));

        /* UDP 소켓 생성: 수신버퍼 64KB, 송신전용 */
        Sockfd[i] = init_udp_socket(&Svr_Addr[i], &Clnt_Addr[i],
                Svr_IP, SVR_PORT_NO,
                1024*64, 0, 1);
        if (Sockfd[i] < 0) return 0;            /* 소켓 생성 실패 */

            Log(UDP_OK, "socket %d created [%s:%s:%d]", i, ACCNO(D_K,i+Acc_No).ip_addr,
                    Svr_IP, SVR_PORT_NO);
    }

    return 1;
}

/*************************************************************************
    End of program (pa_9999_us.c)
*************************************************************************/
