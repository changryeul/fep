#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 시세 송신 (UDP)
#   File    : pa_7010_us.c
#
#   설명  : SHM(공유메모리)에 저장된 파생상품 시세 데이터를
#             UDP 소켓으로 외부 클라이언트에 송신하는 프로세스.
#             A7001=지수선물, A7002/A7003=지수옵션 빌드 구분.
#             FIFO 이벤트(Poll)로 송신 시점을 감지한다.
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
#define     ITEM_CODE       INT_SEQ             /* 종목 인덱스 (시퀀스 번호 활용) */

/*-------------------------------------------------------------------------
    Program Define Constants
-------------------------------------------------------------------------*/
#define     MAX_BUF_SIZE    83000               /* 최대 버퍼 크기 */
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */

/*-------------------------------------------------------------------------
    Program Global Variable
-------------------------------------------------------------------------*/
int     Sockfd[ACC_NO_CNT], id = -1;            /* UDP 소켓 FD 배열, 접속 ID */
struct  sockaddr_in Svr_Addr[ACC_NO_CNT];       /* 서버(수신측) 주소 배열 */
struct  sockaddr_in Clnt_Addr[ACC_NO_CNT];      /* 클라이언트(로컬) 주소 배열 */

/*-------------------------------------------------------------------------
    Function Declarations
-------------------------------------------------------------------------*/
void    PA_7010_US(void);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 UDP 송신 루프 실행.             */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */

    PA_7010_US();                              /* 메인 처리 루프 */

    Exit_Process();                             /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_7010_US: 메인 처리 루프                                            */
/*    1) FIFO 열고 UDP 소켓 연결                                      */
/*    2) Poll로 FIFO 이벤트 대기                                      */
/*    3) SHM에서 시세 데이터 읽어 UDP로 송신                            */
/*----------------------------------------------------------------------*/
void    PA_7010_US(void) {
    int     i, rt, add_len, FIFO_fd;
    char    sub[4], tmp[4], bumun[4], fifo_name[256], send_data[1000];

    /* 주소 구조체 초기화 */
    bzero((unsigned char *)Svr_Addr,
            sizeof (struct sockaddr_in) * ACC_NO_CNT);
    bzero((unsigned char *)Clnt_Addr,
            sizeof (struct sockaddr_in) * ACC_NO_CNT);

    Sockfd[0] = -1;

    /*
     * FIFO 파일 경로 생성 및 열기
     * 경로: {FIFO디렉토리}/{모듈명(PA)}/{설정파일명}
     */
    sprintf(sub, "%2.2s", _Exe_Name);
    LtoU(sub, 2);                              /* 소문자 → 대문자 변환 */
    sprintf(fifo_name, "%s/%s/%s", _FEP_FIFO, sub, FFN(D_K,P_K,0));

    FIFO_fd = open(fifo_name, O_RDWR|O_NDELAY);

    if (FIFO_fd < 0)
        Log(FIF_FATAL, "cannot open FIFO[%s] FIFO_fd[%d] {%d:%s}",
                fifo_name, FIFO_fd, SYS_NO, SYS_STR);

    /* UDP 소켓 생성 및 연결 */
    rt = Socket_Connect();
    if (rt == NOTOK)
        return;

    Log(USR_OK, "socket connected:port[%d]", SVR_PORT_NO);

    /* === 메인 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        rt = Poll_File(DATA_TIME);             /* FIFO 이벤트 대기 (최대 60초) */

        memset(send_data, 0x00, sizeof(send_data));

        if (rt == 1) {
            Log(USR_OK, "poll timeout <%d>", INT_SEQ);
            continue;                           /* 타임아웃 → 다음 루프 */
        }
        else if (rt == -1)
            continue;                           /* 오류 → 다음 루프 */
        else {
            /* FIFO 이벤트 발생: FIFO 내 알림 데이터 소진 */
            while (1) {
                rt = read(FIFO_fd, tmp, 1);
                if (rt == 0)
                    break;                      /* FIFO 비어있음 → 읽기 완료 */
            }

            /*
             * SHM에서 시세 데이터를 send_data에 복사
             * 빌드 옵션에 따라 선물/옵션 데이터 선택
             */
#if defined A7001                                   /* 지수선물시세 */
            /*
             * 현재 체결수량과 이전 체결수량 비교:
             *   동일하면 → 호가(B6) 데이터 송신
             *   다르면   → 현재가(G7) 데이터 송신
             */
            if (memcmp(Shm_Futures[0].Futures_CURR_Arry[Shm_Futures[0].CURR_Arry_Key].tot_con_qty,
                    Shm_Futures[0].Futures_CURR_Arry[Shm_Futures[0].Befor_CURR_Arry_Key].tot_con_qty,
                    sizeof(Shm_Futures[0].Futures_CURR.tot_con_qty)) == 0) {
                memcpy(send_data, Shm_Futures[0].Futures_B6.tr_gbn, sizeof (SIF_B6));
            }
            else {
                memcpy(send_data, Shm_Futures[0].Futures_CURR.tr_gbn, sizeof (SIF_G7));
            }

#elif defined(A7002) || defined(A7003)                      /* 지수옵션시세 */
            /*
             * 옵션 종목의 현재 vs 이전 체결수량 비교:
             *   동일하면 → 호가(B6) 데이터 송신
             *   다르면   → 현재가(G7) 데이터 송신
             */
            if (memcmp(Shm_Options[ITEM_CODE].Options_CURR_Arry[Shm_Options[ITEM_CODE].CURR_Arry_Key].tot_con_qty,
                    Shm_Options[ITEM_CODE].Options_CURR_Arry[Shm_Options[ITEM_CODE].Befor_CURR_Arry_Key].tot_con_qty,
                    sizeof(Shm_Options[ITEM_CODE].Options_CURR.tot_con_qty)) == 0) {
                memcpy(send_data, Shm_Options[0].Options_B6.tr_gbn, sizeof (SIO_B6));
            }
            else {
                memcpy(send_data, Shm_Options[0].Options_CURR.tr_gbn, sizeof (SIO_G7));
            }
#endif
        }

        /* 모든 접속처(최대 ACC_NO_CNT개)에 UDP 시세 데이터 송신 */
        add_len = sizeof (Svr_Addr[0]);

        for (i = 0; i < ACC_NO_CNT; i++) {
            rt = sendto(Sockfd[i], send_data, strlen(send_data),
                    0, (struct sockaddr *)&Svr_Addr[i], add_len);
            if (rt < 0) {
                Log(UDP_FATAL, "Data Send Error [%d:%s]", SYS_NO, SYS_STR);
                break;
            }
        }

        Log(USR_OK, "UDP S [%.10s](%d)", send_data, strlen(send_data));
    }
}

/*----------------------------------------------------------------------*/
/*  Socket_Connect: UDP 소켓 생성 및 수신측 주소 설정                   */
/*    설정(ACCNO)에서 IP를 읽어 최대 4개 UDP 소켓을 생성한다.            */
/*    Return: 1=성공, 0=실패                                            */
/*----------------------------------------------------------------------*/
int     Socket_Connect(void) {
    int     i;
    char    Svr_IP[20];                         /* 변환된 IP 주소 문자열 */

    for (i = 0; i < 4; i++) {
        /* 설정에서 IP 주소를 읽어 "x.x.x.x" 형식으로 변환 */
        format_ip_addr(ACCNO(D_K,i).ip_addr, Svr_IP, sizeof(Svr_IP));

        /* UDP 소켓 생성: 수신버퍼 64KB, 송신전용 */
        Sockfd[i] = init_udp_socket(&Svr_Addr[i], &Clnt_Addr[i],
                Svr_IP, SVR_PORT_NO,
                1024*64, 0, 1);
        if (Sockfd[i] < 0) return 0;            /* 소켓 생성 실패 */

            Log(UDP_OK, "socket %d created [%s:%s:%d]", i, ACCNO(D_K,i).ip_addr,
                    Svr_IP, SVR_PORT_NO);
    }

    return 1;
}

/*************************************************************************
    End of program (pa_7010_us.c)
*************************************************************************/
