#define     _GLOBAL
/*------------------------------------------------------------------------
 *  Module  : 자동매매 전략 — 선물 업틱 Call매수 / 다운틱 Put매수
 *  File    : pa_5020_mp.c
 *
 *  [개요]
 *  자동매매 전략 프로세스로, 시세 변동에 따라 자동으로 주문을 발행한다.
 *  - 채권 시세의 체결(A3/G7)에 반응
 *  - 체결수량 50계약 이상 + 체결가 상승(업틱) → 첫번째 종목 매수
 *  - 체결수량 50계약 이상 + 체결가 하락(다운틱) → 두번째 종목 매수
 *  - 주문은 1회 처리 후 종료
 *
 *  [동작 흐름]
 *  1) 클라이언트(화면)에서 전략 파라미터(종목, 수량, 조건 등) 수신 (500200)
 *  2) 파라미터 설정 완료 후 시세 FIFO를 감시
 *  3) 시세 신호 발생 → Auto_Logic()으로 조건 판단
 *  4) 조건 충족시 Od_Write_Data()로 KRX 주문 포맷 생성 → 송신 프로세스로 전달
 *
 *  [클라이언트 ↔ 전략 프로토콜]
 *  - 500200: 전략 기동 Setting 요청
 *  - 500210: 정상 기동 응답
 *  - 500220: 기동 오류 응답
 *  - 500300: 종료 요청
 *  - 500310: 정상 종료 응답
 *  - 500410: 강제 종료 응답
 *  - 100120: 주문 응답 수신
 *  - 100140: 체결 확인/거부 수신
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE   500
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     FIFO_EVENT  0           /* poll 이벤트: FIFO (데몬 신호) */
#define     FILE_EVENT  1           /* poll 이벤트: 파일 (응답/체결 수신) */

#define     TCP_TIME_OUT 30         /* 기본 타임아웃 (초) */
#define     DATA_TIME   60 * 1000   /* 데이터 수신 대기 (60초) */
#define     READ_MAX    1           /* DSHM 읽기 최대 건수 */

/* FIFO 카운터 매크로 — SAM_USE 여부에 따라 분기 */
#ifdef  SAM_USE
#define  WR_CNT   W_CNT(0,0)
#define  RD_CNT   R_CNT(0,0)
#else
#define  WR_CNT   IDW_CNT(0,0)
#define  RD_CNT   IDR_CNT(0,0)
#endif

/*------------------------------------------------------------------------
 *  전역 변수 (Global Variables)
 *------------------------------------------------------------------------*/
/* 채권일반주문 기본 포맷 (261바이트 KRX_JUMUN_DATA 샘플) — 초기값으로 사용 */
char    Order_Dat[261] = {"00000000001TCHODR10001G10001002999                    KR70059300031100000000182400000001230000007050020000000000000    0011030     00            000041010000011720172121276805CAE61CDC202110261317010280101                                                        0"};

int                 Pk_mp[2];           /* 주문송신 프로세스의 proc.ini 인덱스 [0]=채권, [1]=파생 */
char                ApType[10];         /* 프로세스 유형 코드 (예: "PA50201") */
char                Cli_handler[5];     /* 클라이언트 핸들러 식별자 */
char                W_DFmt[1024];       /* 클라이언트 응답 전송용 버퍼 */
char                Order_St[1024];     /* KRX 주문 조립용 버퍼 */
FILE_BUFF_FORMAT    W_Fmt, R_Fmt[READ_MAX]; /* FIFO 쓰기/읽기 버퍼 */
struct pollfd       Poll[10];           /* poll 배열 (FIFO + 파일 + 시세 FIFO들) */
int     InPollCnt, PollCnt;             /* InPollCnt=시세FIFO수, PollCnt=총poll수 */

int     OD_SEQ;                         /* 전략 프로세스의 SHM 인덱스 (Shm_Strrg 배열) */
int     Set_Flag;                       /* 전략 설정 완료 플래그 (0=미설정, 1=설정완료) */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_5020_MP(void);               /* 메인 루프 */
int     Init_Parameters(void);          /* 초기화 */
void    Fifo_Event_Rtn(void);           /* 데몬 FIFO 이벤트 처리 */
void    Check_Proc_Status(int);         /* 송신 프로세스 상태 확인 */
int     Od_Write_Data(int, int, int, int, int); /* KRX 주문 생성 및 전달 */

int     Auto_Logic(int);                /* 자동매매 조건 판단 */
void    File_Event_Rtn(void);          /* 응답/체결 파일 이벤트 처리 */
void    Set_In_Param(void);             /* 500200: 전략 기동 Setting */
void    Set_In_Stop(int, int);          /* 500300: 종료요청 / 500410: 강제종료 */
void    Set_Response(void);             /* 100120: 응답확인/거부, 100140: 체결/확인/거부 */

/*------------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*------------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
    PA_5020_MP();
    Exit_Process();
}   /* End of main */

/*------------------------------------------------------------------------
 *  PA_5020_MP — 메인 이벤트 루프
 *
 *  [처리 흐름]
 *  1) 5초 대기 (클라이언트에서 파라미터 수신 대기)
 *  2) Init_Parameters()로 초기화
 *  3) 내부 while: 미처리 응답/체결이 있으면 먼저 처리
 *  4) poll 대기 → FIFO/파일/시세FIFO 이벤트 분기
 *  5) 시세 이벤트 → Auto_Logic()으로 조건 판단 → Od_Write_Data()로 주문
 *------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
void        PA_5020_MP(void)
/*------------------------------------------------------------------------*/
{
    int     i, j, rt, bi_y, rc, ju_edt;
    int     as_p, o_price, o_ticks01, o_ticks03;
    int     call_jan, call_ticks, put_jan, put_ticks, fu_jan, jm_su;
    int     call_price, put_price, fu_price, for_cnt, op_gbn;
    int     f_rp_c, f_rp_p, ju_gbn, o_rp;
    char    t_time[12];

    Set_Flag = 0;
    sleep(5);           /* Client로부터 Parameter를 받을 시간을 준다 */
    rt = Init_Parameters();

    if (rt != OK) {
        Exit_Process();
        sleep(1);
    }

    while (START_S < JOB_END) {
        Stat_Save();

        /* ============================================================ */
        /* 미처리 응답/체결이 있으면 poll 전에 먼저 소진                  */
        /* ============================================================ */
        while (START_S < JOB_END) {
            /* **************************************************************************************************** */
            /* 자동로직 시작 2021.11.02                                                                                   */
            /* **************************************************************************************************** */
            if (WR_CNT > RD_CNT) {
                File_Event_Rtn();       /* 응답/체결 처리 */
                if (Set_Flag > 0)
                    PollCnt = InPollCnt + 2;    /* FIFO + 파일 + 시세 FIFO들 */
                else
                    PollCnt = 2;                /* FIFO + 파일만 */
                continue;
            }
            else
                break;
        }

        /* ============================================================ */
        /* poll 대기                                                     */
        /* ============================================================ */
        rt = poll(Poll, PollCnt, TIME_OUT * 1000);
        if (rt > 0) {
            /* 이벤트 발생한 fd 찾기 */
            for (i = 0; i < PollCnt; i++) {
                if (Poll[i].revents & POLLIN) {
                    Poll[i].revents = 0;
                    break;
                }

                if (Poll[i].revents & POLLHUP) {
                    SLog(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                    continue;
                }

            }
        }
        else {
            if (rt == 0) {
                SLog(TCP_ERROR, "poll timeout(no response)");
                continue;
            }
            else {
                if (SYS_NO == EINTR)
                    SLog(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
                else
                    SLog(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

                continue;
            }
        }

        /* ============================================================ */
        /* 이벤트 종류별 분기                                            */
        /* ============================================================ */
        switch (i) {
            case FIFO_EVENT:                /* 데몬 FIFO 신호 */
                Fifo_Event_Rtn();
                break;
            case FILE_EVENT:                /* 응답/체결 파일 이벤트 */
                File_Event_Rtn();
                if (Set_Flag > 0)
                    PollCnt = InPollCnt + 2;
                else
                    PollCnt = 2;

                /* 미처리 데이터가 남아있으면 계속 처리 */
                while(START_S < JOB_END) {
                    if (WR_CNT > RD_CNT) {
                        File_Event_Rtn();
                        if (Set_Flag > 0)
                            PollCnt = InPollCnt + 2;
                        else
                            PollCnt = 2;
                        continue;
                }
                else
                    break;
            }
            break;

            case 2:         /* 첫번째 시장 시세 신호 (예: 채권) */
            case 3:         /* 두번째 시장 시세 신호 (예: 금융파생) */
                rt = Auto_Logic(i-2);       /* 자동매매 조건 판단 */
                if (rt) {
                    /* Od_Write_Data 파라미터 설명:
                     * mk_gbn  : 시장구분 (1~10)
                     * nmc_gbn : 1:신규, 2:정정, 3:취소
                     * p_flag  : write flag (1:채권, 2:파생시장주문)
                     * mm_gbn  : 1:매도, 2:매수
                     * arry    : 주문 대상의 배열 인덱스
                     */
                    rt = Od_Write_Data(1, 1, 1, 2, 1);
            }
            else
                return;

            break;

            default:
                SLog(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                return;
                break;
        }
    }

    /* 종료 시 주문 프로세스 상태 확인 */
    Check_Proc_Status(0);
    Check_Proc_Status(1);

}   /* End of PA_5020_MP () */

/*************************************************************************
 *  File_Event_Rtn — 응답/체결 파일 이벤트 처리
 *
 *  DSHM(데이터 공유메모리)에서 1건 읽어 TR코드별 분기:
 *  - 500200 : Set_In_Param()  (전략 기동 파라미터 설정)
 *  - 500300 : Set_In_Stop()   (종료 요청)
 *  - 100120 : Set_Response()  (주문 응답 확인/거부)
 *  - 100140 : Set_Response()  (체결 확인/거부)
 *************************************************************************/
/*------------------------------------------------------------------------*/
void    File_Event_Rtn(void)
/*------------------------------------------------------------------------*/
{
    int rt, R_Cnt;
    char tmp[128];

    /* DSHM에서 1건 읽기 */
    R_Cnt = DSHM_R(TS_W1_1, (void *)R_Fmt, 1);
    if (R_Cnt < 0) {
        Log(SAM_FATAL, "cannot read file[%s,%d:%s] W[%d]R[%d]",
                IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
        sleep(1);
        Exit_Process();
    }
    else if (R_Cnt == 0)
        return;

    /* TR코드(6바이트)별 분기 */
    if (memcmp(R_Fmt[0].Data, "500200", 6) == 0)            /* 전략 기동 Setting */
        Set_In_Param();
    else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)       /* 종료 요청 */ {
        if (Set_Flag == 0)
            Set_In_Stop(0,0);       /* 감소없이 종료 */
        else
            Set_In_Stop(0,1);       /* auto_use 감소 후 종료 */
    }
    else if ((memcmp(R_Fmt[0].Data, "100120", 6) == 0) ||       /* 응답(100120) */
            (memcmp(R_Fmt[0].Data, "100140", 6) == 0))     /* 체결(100140) */
    Set_Response();
    else {
        Log(USR_ERROR, "Other Tr Code Check Plz [%20.20s]", R_Fmt[0].Data);
    }

    /* DSHM 읽기 카운터 증가 */
    Dshm_Add_Count(TS_W1_1, 1);

    /* FIFO 잔여 데이터 소진 */
    while(1) {
        rt = read(INPUT_FD, tmp, sizeof(tmp));
#if defined __linux
        if (rt == 0 || errno == EAGAIN)
#else
            if (rt == 0)
#endif
            break;
    }

    return;
}   /* File_Event_Rtn() */

/*************************************************************************
 *  Auto_Logic — 자동매매 조건 판단
 *
 *  [전략 로직]
 *  - 채권시세의 체결(A3/G7)에만 반응
 *  - 체결수량 50계약 이상 AND 체결틱 상승 → rt=1 (첫번째 종목 매수)
 *  - 체결수량 50계약 이상 AND 체결틱 하락 → rt=2 (두번째 종목 매수)
 *  - 전략 설정시 3종목 수신:
 *    첫번째 = 조건 종목, 두번째/세번째 = 주문 대상 종목
 *
 *  [참고]
 *  fd_gbn: Poll 배열에서 시세 FIFO 인덱스 (0=첫번째시장, 1=두번째시장)
 *************************************************************************/
/*------------------------------------------------------------------------*/
int Auto_Logic(int fd_gbn)
/*------------------------------------------------------------------------*/
{
    int     rt = 0;
    int     curr_key, be_curr_key;
    /* **************************************************************************** */
    /* 채권시장                                                                     */
    /* 1. 조건은 체결(A3/G7)에만 반응한다.                                         */
    /* 2. 샘플전략은 채권의 체결수량이 50계약 이상 && 체결틱이 상승 => 매수          */
    /*                 체결수량이 50계약 이상 && 체결틱이 하락 => 매도               */
    /* 3. 주문은 1회만 처리하고 종료한다.                                            */
    /* 4. 전략설정시 3개의 종목을 받는데 첫번째는 조건이 되는 종목이고                */
    /*                       두번째는 주문대상이 되는 종목이다                     */
    /* 5. 또한 화면(전략기동)에서도 3종목은 순서대로 받기로 하였다.                     */
    /* **************************************************************************** */

    /* 장 개시 전에는 주문 처리 안함 — 누적거래량이 0이면 리턴 */
    if (AtoIf(&Shm_Note[0].G7.bond_accum_exec_vol[7], 8) <= 0)
        return (0);

    /* 조건 종목의 최종호가구분이 0(A3 또는 G7)인 경우만 판단 */
    if (Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].HogaLastGbn == 0) {
        /* 현재 체결가 배열 키 */
        curr_key    = Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry_Key;
        /* 직전 체결가 배열 키 */
        be_curr_key = Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].Befor_CURR_Arry_Key;

        /* 체결수량 50계약 이상 체크 */
        if (memcmp(&Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].volume[2], "00000050", 8) > 0) {
            /* 현재가 > 직전가 → 업틱 (상승) */
            if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
                    Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) > 0) {
                rt = 1;         /* 첫번째 종목 매수 */
            }
            else
                /* 현재가 < 직전가 → 다운틱 (하락) */
                if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
                        Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) < 0) {
                rt = 2;         /* 두번째 종목 매수 */
            }
        }
    }

    return (rt);

}   /* End of Auto_Logic() */

/*************************************************************************
 *  Check_Proc_Status — 주문 송신 프로세스 상태 확인
 *
 *  mk: 0=채권, 1=파생
 *  (현재 미구현 — 향후 확장용)
 *************************************************************************/
/*------------------------------------------------------------------------*/
void    Check_Proc_Status(int mk)
/*------------------------------------------------------------------------*/
{
    int rt;

    return;
}   /* End of Check_Proc_Status() */

/*************************************************************************
 *  Init_Parameters — 초기화
 *
 *  - Poll[0]=데몬FIFO, Poll[1]=응답/체결 파일
 *  - ApType 생성: 실행파일명에서 추출
 *  - OD_SEQ 계산: Shm_Strrg 배열에서의 인덱스
 *  - Pk_mp[0], Pk_mp[1]: 주문 송신 프로세스의 proc.ini 인덱스 탐색
 *************************************************************************/
/*------------------------------------------------------------------------*/
int     Init_Parameters()
/*------------------------------------------------------------------------*/
{
    int     i, rt;

    SEARCH_HEADER       *hd;
    KRX_JUMUN_DATA      *KrxOrder;

    Poll[0].fd = START_FD;      /* 데몬 FIFO */
    Poll[0].events = POLLIN;
    Poll[1].fd = INPUT_FD;      /* 응답/체결 파일 */
    Poll[1].events = POLLIN;

    if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;

    /* ApType 생성: 실행파일명에서 모듈코드 추출 (예: "PA50201") */
    memset(ApType, 0, sizeof(ApType));
    sprintf(ApType, "%-2.2s%-5.5s", _Exe_Name, _Exe_Name+3);
    LtoU(ApType, strlen(ApType));

    /* OD_SEQ 계산: Shm_Strrg 배열의 인덱스
     * ApType 3번째부터 2자리 = 전략그룹, 5번째부터 2자리 = 전략순번
     * 예: PA50201 → (02-1)*10 + 01-1 = 10 */
    OD_SEQ = ((AtoIf(&ApType[3], 2) -1) * 10) + AtoIf(&ApType[5], 2) -1;

    /* 주문 송신 프로세스(pa_1100_ts 등) 탐색 */
    for(i = 0; i < 2; i++) {
        for(Pk_mp[i] = 0; Pk_mp[i] < DAEMON(D_K).p_count; Pk_mp[i] ++) {
            if (memcmp(PROC(D_K,Pk_mp[i]).process_id, ODN(D_K,P_K,i), 10) == 0)
                break;

            if (Pk_mp[i] == DAEMON(D_K).p_count - 1) {
                Log(USR_FATAL, "unregistered process[%s]", ODN(D_K,P_K,i));
                TCP1_NET_STA = OFF;
                Exit_Process();
            }
        }
    }

}   /* End of Init_Parameters() */

/*************************************************************************
 *  Set_In_Param — 전략 기동 파라미터 설정 (TR: 500200)
 *
 *  클라이언트에서 전략 파라미터를 수신하여 SHM에 설정:
 *  - next_flag: O=1건, S=시작, N=계속, E=종료 (다건 전송 지원)
 *  - 공통 파라미터: 종목수, 시장수, 주문수량, 계좌번호
 *  - 종목별 파라미터: 시장구분, 종목코드, 타켓/참조, 주문수량, 주문타입 등
 *
 *  설정 완료 후:
 *  - 정상: 500210 응답 + auto_use 증가
 *  - 오류: 500220 응답 + Set_In_Stop()로 종료
 *************************************************************************/
/*------------------------------------------------------------------------*/
void    Set_In_Param(void)
/*------------------------------------------------------------------------*/
{
    int     i, rt, arry, mk_gbn, item_seq;
    int     return_flag = 0;
    char    fifo_name[256];
    char    err_no[10], head_size[10];

    IN_SAMPLE01         *dat;

    dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];

    /* ============================================================ */
    /* 공통 파라미터 설정 (next_flag = "O" 또는 "S")                 */
    /* ============================================================ */
    if ((memcmp(dat->next_flag, "O", 1) == 0) ||        /* O = 1건으로 완결 */
            (memcmp(dat->next_flag, "S", 1) == 0))          /* S = 다건 전송 시작 */ {
        Set_Flag = 0;
        /* 사용될 총 종목 수량 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt   = AtoIf(dat->tot_item_cnt, 2);
        /* 사용될 총 시장 수 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].use_market_cnt = AtoIf(dat->use_market_cnt, 2);
        /* 1회 주문시 주문 수량 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_order_cnt  = AtoIf(dat->tot_order_cnt, 2);
        /* 주문낼 현물 계좌번호 */
        memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, dat->st_accno, sizeof(dat->st_accno));
        /* 주문낼 파생 계좌번호 */
        memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].dv_accno, dat->dv_accno, sizeof(dat->dv_accno));

        /* 시장별 시세 FIFO의 fd값 설정 */
        InPollCnt = AtoIf(dat->use_market_cnt, 2);

        /* 시장별 시세 FIFO 열기 */
        for(i = 0; i < InPollCnt; i++) {
            memset(fifo_name, 0, sizeof(fifo_name));
            if (memcmp(dat->market_gbn[i], "10", 2) == 0)
                sprintf(fifo_name, "%s/PA/pa_7912_ur1", _FEP_FIFO);
            else
                sprintf(fifo_name, "%s/PA/pa_7%1.1s11_ur1", _FEP_FIFO, dat->market_gbn[i]+1);

            Poll[i+2].fd = open(fifo_name, O_RDWR|O_NDELAY);
            Poll[i+2].events = POLLIN;
            if (Poll[i+2].fd < 0) {
                Log(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
                        i, fifo_name, Poll[i+2].fd, SYS_NO, SYS_STR);
                return_flag = -1;
            }
        }
        Log(USR_OK, "Poll Cnt = [%d]", InPollCnt);

        if (memcmp(dat->next_flag, "O", 1) == 0)
            Set_Flag = 1;       /* 1건 완결 → 설정 완료 */
        else {
            if (return_flag == 0)
                return;         /* S(시작) → 다음 전송 대기 */
        }
    }
    else
        /* ============================================================ */
        /* 종목별 파라미터 설정 (next_flag = "N" 또는 "E")               */
        /* ============================================================ */
        if ((memcmp(dat->next_flag, "N", 1) == 0) ||        /* N = 계속 */
                (memcmp(dat->next_flag, "E", 1) == 0))          /* E = 마지막 종목 */ {
        /* 배열 번호 (순번) */
        arry = AtoIf(dat->Condition.arry_no, 3);
        /* 시장구분 */
        mk_gbn = AtoIf(dat->Condition.market_gbn, 2);
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn = mk_gbn;
        /* 종목 SHM 시퀀스 */
        item_seq = AtoIf(dat->Condition.item_seq, 5);
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq = item_seq;
        /* 종목코드 정합성 체크 — SHM의 종목코드와 클라이언트 종목코드 비교 */
        if (memcmp(Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_cd,
                dat->Condition.item_code, 12) != 0) {
            return_flag = -2;
        }
        memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_code,
                dat->Condition.item_code, 12);
        /* 타겟인지 참조인지 여부 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].target_gbn =
        AtoIf(dat->Condition.target_gbn, 1);
        /* 1회 주문수량 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt =
        AtoIf(dat->Condition.order_cnt, 8);
        /* 주문타입 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type =
        AtoIf(dat->Condition.order_type, 1);
        /* 주문가격구분 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn =
        AtoIf(dat->Condition.order_price_gbn, 1);
        /* 주문가격조건 */
        Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition =
        AtoIf(dat->Condition.order_price_condition, 1);

        if (memcmp(dat->next_flag, "E", 1) == 0)
            Set_Flag = 1;       /* E(마지막) → 설정 완료 */
        else {
            if (return_flag == 0)
                return;         /* N(계속) → 다음 전송 대기 */
        }
    }

    /* ********************************************************* */
    /* 클라이언트에 설정 결과 응답 전송                            */
    /* ********************************************************* */
    memset(W_DFmt, 0x20, sizeof(W_DFmt));
    memcpy(&W_DFmt, &R_Fmt, sizeof(BUFF_RW_HEAD) + 20);
    /* 기동 Set 오류 */
    if (return_flag < 0) {
        /* 오류 → Client에 500220 (에러) 응답 */
        memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500220", 6);     /* SEARCH_HEADER(20), Tr Code */
    }
    else {
        /* 정상 → Client에 500210 (기동완료) 응답 */
        memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500210", 6);     /* SEARCH_HEADER(20), Tr Code */
    }
    memset(err_no, 0, sizeof(err_no));
    sprintf(err_no, "%04d", -1*return_flag);
    memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)+10], err_no, 4);    /* SEARCH_HEADER(20), ErrCode(0000:Normal) */
    W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';

    rt = F_W(TS_W1_1, (void *)&W_DFmt, 1);
    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
        Exit_Process();
    }

    Log(USR_OK, "file write[%s:%d:%d]",
            OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
    /* ********************************************************** */

    /* 시장별 시세의 auto_use를 증가 — 시세 프로세스에서 신호를 보내기 위해 */
    if (Set_Flag == 1 && return_flag == 0)      /* 정상 설정 완료 */ {
        for(i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++) {
            mk_gbn   = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn;
            item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq;
            Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use++;
        }

        /* 9001이 Set 한다.
        Shm_Strrg[0].auto_run_flag[OD_SEQ] = 1;
        */
    }
    else if (return_flag < 0) {
        Set_In_Stop(1, 0);      /* 감소없이 강제 종료 */
    }

    return;
}   /* End of Set_In_Param */

/*************************************************************************
 *  Set_In_Stop — 전략 종료 처리
 *
 *  tr_gbn   : 0=정상종료(500310), 1=강제종료(500410)
 *  min_flag : 0=auto_use 감소없이, 1=auto_use 감소시키고 종료
 *
 *  종료 절차:
 *  1) 클라이언트에 종료 응답 전송
 *  2) min_flag=1이면 시세 auto_use 감소
 *  3) SHM 전략 상태 초기화
 *  4) 프로세스 상태를 JOB_END로 변경
 *  5) 데몬에게 FIFO 신호 전송 (즉시 종료 요청)
 *************************************************************************/
/*------------------------------------------------------------------------*/
void    Set_In_Stop(int tr_gbn, int min_flag)
/*------------------------------------------------------------------------*/
{
    int     i, rt, mk_gbn, item_seq;
    char    err_no[10], fifo_name[128];

    /* ************************************************** */
    /* 클라이언트에 종료 응답 전송                          */
    /* ************************************************** */
    memset(W_DFmt, 0x20, sizeof (W_DFmt));
    memcpy(&W_DFmt, &R_Fmt, sizeof (BUFF_RW_HEAD)+20);
    if (tr_gbn == 0)
        memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)], "500310", 6);       /* 정상종료 응답 */
    else
        memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)], "500410", 6);       /* 강제종료 응답 */
    memset(err_no, 0, sizeof (err_no));
    sprintf(err_no, "%04d", 0);
    memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)+10], err_no, 4);          /* ErrCode(0000:Normal) */
    W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';

    rt = F_W(TS_W1_1, (void *)&W_DFmt, 1);
    if (rt != 1) {
        Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
        Exit_Process();
    }

    Log(USR_OK, "file write[%s:%d:%d]",
            OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
    /* ************************************************** */

    /* 시장별 시세의 auto_use 감소 — 시세 신호 중지 */
    if (min_flag == 1) {
        for (i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++) {
            mk_gbn   = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].market_gbn;
            item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].item_seq;

            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use < 0)
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use = 0;
            else
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use--;
        }
    }

    /* 전략 종료 상태 설정 */
    Shm_Strrg[0].auto_run_flag[OD_SEQ] = 0;
    PROC(D_K,P_K).start_status = JOB_END;
    PROC(D_K,P_K).process_status = 9;

    /* 데몬에게 즉각적인 종료 요청을 위해 FIFO 신호 전송 */
    sprintf(fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);
    DFIFD(D_K) = open(fifo_name, O_RDWR | O_NDELAY);

    if (DFIFD(D_K) == -1) {
        Log(FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
                fifo_name, SYS_NO, SYS_STR);
        exit(FAIL);
    }

    write(DFIFD(D_K), "1", 1);
}   /* End of Set_In_Stop () */

/*************************************************************************
 *  Set_Response — 주문 응답/체결 처리
 *
 *  - 100120: 주문 응답 (확인/거부)
 *  - 100140: 체결 (확인/거부)
 *
 *  현재 미구현 — 전략에 맞게 확장 필요
 *  (응답/체결 조건에 맞게 추가 주문, 미체결 관리 등)
 *************************************************************************/
/*------------------------------------------------------------------------*/
void    Set_Response(void)
/*------------------------------------------------------------------------*/
{
    /* 여기에서 응답/체결을 받고 조건에 맞게 하시면 됩니다. */

    return;
}   /* End of Set_Response () */

/*************************************************************************
 *  Od_Write_Data — KRX 주문 전문 생성 및 송신 프로세스로 전달
 *
 *  [파라미터]
 *  mk_gbn  : 시장구분 (1=채권, 2=금융파생, 3~10=기타)
 *  nmc_gbn : 1=신규, 2=정정, 3=취소
 *  p_flag  : 주문유형 (1=일반, 2=LP)
 *  mm_gbn  : 1=매도, 2=매수
 *  arry    : 주문 대상의 배열 인덱스 (전략 설정시 수신한 종목 순번)
 *
 *  [KRX 채권일반주문 전문 구성] (261바이트)
 *  - 주문번호 채번: Shm_Risk.Order_No에서 순번 발급
 *  - 종목코드, 매도매수구분, 정정취소구분, 계좌번호, 수량, 가격 설정
 *  - 호가가격: 지정가=매도1호가/매수1호가, 시장가=상/하한가
 *
 *  [회원사처리항목 60바이트 규약]
 *  앞 30바이트: 원장 요청값
 *  뒤 30바이트:
 *    [30] A(서버자동), C(Client)
 *    [31-32] ST(Strategy)
 *    [33-34] 전략번호 01~99
 *    [35-36] 시장구분 (1=지수선물 ... 10=Kosdaq150옵션)
 *    [37-41] A0 Seq번호
 *    [42-43] 계좌번호 Seq
 *    [44] 유가증권/주식선물 서브구분
 *    [45] 스프레드여부
 *    [46-49] Process Nick Name
 *************************************************************************/
/*------------------------------------------------------------------------*/
int     Od_Write_Data(int mk_gbn, int nmc_gbn, int p_flag, int mm_gbn, int arry)
/*------------------------------------------------------------------------*/
{
    int     rt, o_gbn = 0;
    char    m_time[24], str[10];
    char    cdata[30];
    char    tmp[128];

    double  cal_price;

    memset(m_time, 0, sizeof (m_time));
    memset(&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

    /* KRX 주문포맷 초기값(Order_Dat)을 복사 */
    memcpy(&Order_St[sizeof (BUFF_RW_HEAD)], Order_Dat, sizeof(KRX_JUMUN_DATA));

    /* ============================================================ */
    /* 채권일반주문 — KRX전문 항목별 설정                             */
    /* ============================================================ */
    if (mk_gbn == 1) {
        /* ************************************************************************ */
        /* 주문번호 채번 — SHM에서 순번 발급 */
        Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO++;        /* 파생은 Order_No[1] 사용 */
        if (Shm_Risk[0].Order_No[0][OD_SEQ].START_JMNO > Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO  ||
                Shm_Risk[0].Order_No[0][OD_SEQ].END_JMNO  <= Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO) {
            Log(USR_ERROR, "주문번호 사용 영역 FULL!!! jm_no[%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
            return (-1);
        }

        /* TR Code (11바이트) — 현재 전략상 신규주문만이므로 수정 불필요
        if (nmc_gbn == 1)
            memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10001", 11);
        else if (nmc_gbn == 2)
            memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10002", 11);
        else if (nmc_gbn == 3)
            memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10003", 11);    */

        /* 보드ID — G1(기본) 사용 (G1 이외 거래시 변경 필요) */

        /* 주문ID (오프셋 34, 10바이트) */
        memset(tmp, 0, sizeof (tmp));
        sprintf(tmp, "%010ld", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
        memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+34], tmp, 10);

        /* 원주문번호 (오프셋 44) — 신규이므로 불필요
           정정/취소시에는 자기 주문번호를 보관했다가 여기에 넣어야 함
        if (nmc_gbn == 2 || nmc_gbn == 3)
            memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+44], xxxxxxxxxx, 10);   */

        /* 종목코드 (오프셋 54, 12바이트) */
        memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+54],
                Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_item_cd, 12);

        /* 매도매수 구분 (오프셋 66, 1바이트) */
        if (mk_gbn == 1)
            memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+66], "1", 1);
        else if (mk_gbn == 2)
            memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+66], "2", 2);
        else {
            Log(USR_ERROR, "매도매수구분 Error!!! mm_gbn[%d]", mm_gbn);
            return (-2);
        }

        /* 정정/취소구분 (오프셋 67, 1바이트) */
        if (nmc_gbn == 1)
            memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+67], "1", 1);
        else if (nmc_gbn == 2)
            memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+67], "2", 1);
        else if (nmc_gbn == 3)
            memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+67], "3", 1);

        /* 계좌번호 — 현물 (오프셋 68, 12바이트) */
        memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+68], Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, 12);

        /* 호가수량 (오프셋 80, 10바이트) */
        memset(tmp, 0, sizeof (tmp));
        sprintf(tmp, "%010d", Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt);
        memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+80], tmp, 10);

        /* ============================================================ */
        /* 호가가격 산출 (오프셋 90, 11바이트)                           */
        /* ============================================================ */
        memset(tmp, 0, sizeof (tmp));
        if (mm_gbn == 1)        /* 매도 */ {
            if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)   /* 1:지정가 */ {
                if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)      /* 0:자기호가(매도1호가) */ {
                    cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
                }
                else    /* 1:상대호가(매수1호가) */ {
                    cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
                }

                /*
                                cal_price = Hoga_Change (...);  // 호가변환 함수 (향후 확장)
                */
                cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
            }
            else    /* 2:시장가 → A0의 하한가격으로 주문 */ {
                cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
            }
        }
        else        /* 매수 */ {
            if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)       /* 1:지정가 */ {
                if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)  /* 0:자기호가(매수1호가) */ {
                    cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
                }
                else    /* 1:상대호가(매도1호가) */ {
                    cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
                }

                /*
                                cal_price = Hoga_Change (...);  // 호가변환 함수 (향후 확장)
                */
                cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
            }
            else    /* 2:시장가 → A0의 상한가격으로 주문 */ {
                cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
            }
        }

        sprintf(tmp, "%011ld", (long)cal_price);
        memcpy(&Order_St[sizeof (BUFF_RW_HEAD)+90], tmp, 11);

        /* 호가유형코드 — 지정가로 설정 (상/하한가로 주문시에도 지정가+상하한가격 사용) */
        /* 호가조건코드 — Default(0) 사용 */
        /* 시장조성자호가구분번호 — Default(0), LP는 1 */
        /* 기타 전략/계좌 조건에 따라 나머지 값도 설정 필요 */

        /* ******************************************************************** */
        /* 회원사처리항목 (60바이트) 규약                                        */
        /* 앞 30바이트: 원장 요청값                                             */
        /* 뒤 30바이트:                                                        */
        /*  [30]       A(서버자동), C(Client)                                   */
        /*  [31-32]    ST(Strategy)                                             */
        /*  [33-34]    전략번호 01~99                                           */
        /*  [35-36]    시장구분 (1~10)                                          */
        /*  [37-41]    A0 Seq번호 (5자리)                                       */
        /*  [42-43]    계좌번호 Seq                                             */
        /*  [44]       유가증권/주식선물 서브구분                                */
        /*  [45]       스프레드여부 (0:normal, 1:스프레드)                       */
        /*  [46-49]    Process Nick Name                                        */
        /* ******************************************************************** */

    }
    else        /* 파생시장 */ {
        /* 파생시장에 맞게 KRX전문 채우기 (위 현물 참조) */
    }

    /* *************************************************************** */
    /* DSHM을 통해 주문 송신 프로세스로 전달                             */
    /* *************************************************************** */
    memcpy(&W_Fmt, Order_St, sizeof (BUFF_RW_HEAD) + ODS(D_K,P_K,0));
    W_Fmt.LineFeed[0] = '\n';

    if (mk_gbn == 1 && p_flag == 1)                 /* 채권일반 */ {
        o_gbn = 0;
        rt = DSHM_W(TS_W1_1, (void *)&W_Fmt, 1);
    }
    else
        if (mk_gbn == 1 && p_flag == 2)                 /* 채권LP */ {
        o_gbn = 1;
        rt = DSHM_W(TS_W2_1, (void *)&W_Fmt, 1);
    }
    else                                            /* 금융파생 */ {
        o_gbn = 2;
        rt = DSHM_W(TS_W3_1, (void *)&W_Fmt, 1);
    }

    if (rt != 1) {
        Log(SAM_FATAL, "shm write fail [%s]", ODN(D_K,P_K,o_gbn));
        return (NOTOK);
    }

    Log(SAM_OK, "file write [%s:%d:%d]", ODN(D_K,P_K,o_gbn), ODW(D_K,P_K,0,o_gbn), rt);

    return (OK);
}   /* End of Write_Data() */

/*************************************************************************
    End of Program (pa_5020_mp.c)
*************************************************************************/
