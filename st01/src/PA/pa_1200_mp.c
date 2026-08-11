#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 주문접수처리
#   File    : pa_1200_mp.c
#
#   설명  : KRX/IMECO에서 수신한 주문응답(확인/거부/회원처리호가)
#             데이터를 FIFO에서 읽어, 매체구분(A/C/T) 확인 후
#             자동매매 전략(DSHM) 및 Client FIFO로 전달하는 프로세스.
#             A1201=채권주문접수, A2201=파생주문접수(IMECO).
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

/*
 * DATA_SIZE: 빌드 옵션에 따라 수신 데이터 크기 결정
 *   A1201: 채권주문접수 → 400바이트
 *   A2201: 파생주문접수 → 200바이트
 */
#if defined A1201
#define     DATA_SIZE       400
#elif defined A2201
#define     DATA_SIZE       200
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초, 밀리초 단위) */
#define     READ_MAX        1                   /* FIFO에서 한 번에 읽을 최대 건수 */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         R_Cnt;                              /* FIFO 읽기 건수 */
char        ApType[10];                         /* 어플리케이션 타입 */
FILE_BUFF_FORMAT    R_Fmt[READ_MAX];            /* FIFO 읽기용 버퍼 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1200_MP(void);
void    Analyze_Data(void);
int     Check_rtn(char *, int);

/*----------------------------------------------------------------------*/
/*  main: 프로세스 시작점. 초기화 후 주문접수 처리 루프 실행.            */
/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);                     /* 프로세스 초기화 */
    PA_1200_MP();                              /* 메인 처리 루프 */
    Exit_Process();                            /* 프로세스 종료 */
}   /* End of main () */

/*----------------------------------------------------------------------*/
/*  PA_1200_MP: 메인 처리 루프                                            */
/*    1) FIFO에서 주문응답 데이터 읽기                                 */
/*    2) Analyze_Data() → Check_rtn()으로 분석 및 전달                 */
/*    3) 데이터가 없으면 Poll_File()로 60초 대기                       */
/*----------------------------------------------------------------------*/
void    PA_1200_MP(void) {
    int     rt, read_flag;

    /* ApType 생성: 실행파일명에서 모듈+기능번호+타입 추출 */
    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    /* === 외부 루프: 프로세스 종료 신호(JOB_END)까지 반복 === */
    while (START_S != JOB_END) {
        Stat_Save();                           /* 프로세스 상태 저장 */

        /* === 내부 루프: FIFO에 데이터가 있는 동안 계속 읽음 === */
        while (START_S != JOB_END) {
            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            R_Cnt = F_R(PS_R_1, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0) {
                /* 읽기 실패: 오류 로그 후 프로세스 종료 */
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;                          /* 읽을 데이터 없음 → Poll 대기 */

            Log(USR_OK, "RD [%s:%d][%d]",
                    IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
            Analyze_Data();                    /* 주문응답 데이터 분석 및 전달 */
        }

        /* FIFO에 데이터가 없으면 최대 60초간 대기 */
        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        else if (rt == -1)
            continue;                           /* 오류 → 다음 루프 */
    }
}   /* End of PA_1200_MP () */

/*************************************************************************
    Function        : Analyze_Data
    Parameters IN   : (없음, 전역변수 R_Fmt/R_Cnt 사용)
    Parameters OUT  : (없음)
    Return Code     : void
    Comment         : 읽은 데이터를 Check_rtn()으로 전달하여 분석.
                      건별로 처리 카운터를 증가시킨다.
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void) {
    int     i, rt;

    for (i = 0; i < R_Cnt; i++) {
        rt = Check_rtn(R_Fmt[i].Data, i);
        Add_Count(PS_R_1, 1);                  /* 처리 건수 카운터 증가 */
    }

    return;
}   /* End of Analyze_Data () */

/*************************************************************************
    Function        : Check_rtn
    Parameters IN   : p_buf  - 수신된 주문응답 데이터 포인터
                      for_d  - 데이터 인덱스 (R_Fmt 배열)
    Parameters OUT  : (없음)
    Return Code     : OK(성공)
    Comment         : 주문응답 데이터를 분석하여:
                      1) REJC(거부) 데이터는 스킵 (이미 Client에 전달됨)
                      2) 매체구분(A/C/T) 확인 — 자사 주문만 처리
                      3) 자동매매 전략번호가 있으면 DSHM에 전달
                      4) Client FIFO(pa_8101_ts)에 주문응답 기록

    데이터 포맷:
      A1201(채권): 4+11 + 0 + 주문/회원처리호가 전문
      A2201(파생): 20바이트 IMECO헤더 + 주문/회원처리호가 전문
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_rtn(char *p_buf, int for_d) {
    int     i, j, s_k, rt, order_no, loop, imeco_gbn;
    double  STime, RTime;
    char    turn_time[20], m_time[24], t_time[12];
    char    W_Fmt[8192];                        /* FIFO/DSHM 쓰기용 버퍼 */
    char    tr_code[12];                        /* TR코드 (전문 유형 식별) */
    char    member_area[60];                    /* 회원사용영역 (매체구분+전략번호 포함) */
    int     w_flag, target;                     /* 처리 플래그, 전략 타겟 번호 */
    BUFF_RW_HEAD    f_head;

    w_flag = 0;
    target = -1;

    memset(member_area, 0, sizeof(member_area));

    /*
     * REJC 거부 데이터 스킵 로직:
     *   pa_5000_qr에서 송신 프로세스 미기동 시 "REJC"으로 거부 처리한
     *   데이터는 이미 Client에 전달되었으므로 여기서 다시 처리하지 않음
     */
#if defined A1201                               /* 채권 */
    if (memcmp(R_Fmt[for_d].Data, "REJC", 4) == 0)
#elif defined A2201                             /* 파생 */
        if (memcmp(&R_Fmt[for_d].Data[5], "REJC", 4) == 0)
#endif
        {
        Log(USR_OK, "Reject SKIP Write OK ResponseCode[%10.10s]", R_Fmt[for_d].Data);
        return (OK);
    }

    /*
     * 빌드 옵션별 오프셋 보정값 설정:
     *   A1201(채권): imeco_gbn=0 (KRX 직접연결, 보정 없음)
     *   A2201(파생): imeco_gbn=-30 (IMECO 전문은 30바이트 짧음)
     */
#if defined A1201
    imeco_gbn = 0;
#elif defined A2201
    imeco_gbn = -30;
#endif

    /*
     * TR코드별 회원사용영역(MembershipItem) 추출
     *
     * [A1201 채권] 데이터 구조: 4+11 + 전문
     *   전문 종류를 TR코드(p_buf[4+11+11] 위치)로 판별:
     *   - TCHODR4000 : 채권일반호가(254바이트)
     *   - TCHMOR4000 : 채권LP일반호가(255바이트)
     *   - TCHKOR1000x: Kill Switch(KTS전용)
     *   - TTRODP4130 : 채권일반 회원처리호가(291바이트)
     *   - TTRMOP4130 : 채권LP 회원처리호가(295바이트)
     *   - TTRKOP1130 : Kill Switch 응답
     *
     * [A2201 파생] 데이터 구조: 20바이트 IMECO헤더 + 전문
     *   사유코드(p_buf[5], 4자리)로 판별:
     *   - "0000" 초과: 주문거부 → 주문전문에서 추출
     *   - "0000" 이하: 회원처리호가 → 회원처리호가에서 추출
     */
#if defined A1201
    if (memcmp(&p_buf[4+11+11], "TCHODR4000", 10) == 0) {
        KRX_NOTE_JUMUN_DATA *dat = (KRX_NOTE_JUMUN_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else if (memcmp(&p_buf[4+11+11], "TCHMOR4000", 10) == 0) {
        KRX_LP_NOTE_JUMUN_DATA *dat = (KRX_LP_NOTE_JUMUN_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else if (memcmp(&p_buf[4+11+11], "TCHKOR1000", 10) == 0) {
        KRX_NOTE_KILLSWITCH_DATA *dat = (KRX_NOTE_KILLSWITCH_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else if (memcmp(&p_buf[4+11+11], "TTRODP4130", 10) == 0) {
        KRX_NOTE_SETTLE_RESP_DATA *dat = (KRX_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else if (memcmp(&p_buf[4+11+11], "TTRMOP4130", 10) == 0) {
        KRX_LP_NOTE_JUMUN_DATA *dat = (KRX_LP_NOTE_JUMUN_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else if (memcmp(&p_buf[4+11+11], "TTRKOP1130", 10) == 0) {
        KRX_NOTE_KILLSWITCH_DATA *dat = (KRX_NOTE_KILLSWITCH_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
#elif defined A2201
    if (memcmp(&p_buf[5], "0000", 4) > 0) {
        /* 주문거부: IMECO 헤더(20) 이후 주문전문에서 회원영역 추출 */
        IMECO_JUMUN_DATA *dat = (IMECO_JUMUN_DATA *)&p_buf[20];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
    else {
        /* 회원처리호가(확인/거부/자동취소): 회원처리호가에서 추출 */
        IMECO_SETTLE_DATA *dat = (IMECO_SETTLE_DATA *)&p_buf[20];
        memcpy(member_area, dat->MembershipItem, sizeof(dat->MembershipItem));
    }
#endif

    /*
     * 매체구분 확인: member_area[30+imeco_gbn] 위치
     *   A = API 주문, C = Client 주문, T = 자동매매 주문
     *   이 세 가지 중 하나면 자사 주문 → Client에 전달
     */
    if ((memcmp(&member_area[30+imeco_gbn], "A", 1) == 0) ||
            (memcmp(&member_area[30+imeco_gbn], "C", 1) == 0) ||
            (memcmp(&member_area[30+imeco_gbn], "T", 1) == 0)) {
        w_flag = 1;                             /* 자사 주문 → 전달 대상 */
    }

    /*
     * 자동매매 전략번호 확인: member_area[47+imeco_gbn], 4자리
     * "0000"보다 크면 전략 번호가 있음
     * target = (앞2자리 - 1) × 10 + 뒤2자리
     */
    if (memcmp(&member_area[47+imeco_gbn], "0000", 4) > 0) {
        target = (AtoIf(&member_area[47+imeco_gbn], 2) - 1) * 10;
        target = AtoIf(&member_area[49+imeco_gbn], 2) + target;
    }

    /* 자사 주문이면 DSHM 및 Client FIFO에 전달 */
    if (w_flag > 0) {
        memset(W_Fmt, 0x20, sizeof (W_Fmt));
        memcpy(&W_Fmt, &R_Fmt[for_d], sizeof (FILE_BUFF_FORMAT)-1);
        W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

        /* 자동매매 전략에 데이터 전달 */
        if (target > 0) {
            rt = DSHM_W(target*10, (void *)&W_Fmt, 1);
            if (rt != 1) {
                Log(SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
                Exit_Process();
            }
            Log(USR_OK, "Write OK [%s][%d] target[%d]", W_Fmt, strlen(W_Fmt), target);
        }

        /* Client FIFO(pa_8101_ts)에 주문응답 전달 */
        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
            Exit_Process();
        }
    }

    return (OK);
}

/*************************************************************************
    End of Program (pa_1200_mp.c)
*************************************************************************/
