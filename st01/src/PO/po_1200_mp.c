#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 주문응답 분배 (OMS 코어 공유 부문) — PO 모듈
#   File    : po_1200_mp.c
#
#   설명  : KRX에서 수신한 주문응답(확인/거부/회원처리호가)을 FIFO에서 읽어,
#           매체구분(A/C/T) 확인 후 자동매매 전략(DSHM) 및 Client FIFO로 분배.
#
#   시장 재편 A안 — Phase 3(po_ 코어 추출): pa_1200_mp.c(응답분배)를 이관.
#   이 프로세스는 시장 무관 크로스커팅(응답분배)이라 letter=시장이 아닌 po_(OMS코어) 소속.
#
#   [MK_GBN 런타임화 = 빌드모델 개선의 실체]
#   기존 pa_1200_mp는 `#if defined A1201`(채권)/`A2201`(파생 IMECO)로 컴파일 분기해
#   시장별 2개 바이너리를 만들었다. IMECO 폐기 + 전 시장 KRX-direct 통일로:
#     - 전 시장 응답이 동일 포맷 `[4 ErrCode][11 seq][전문]` (전문 TrCode @ offset 26)
#     - IMECO 헤더(20B)·offset 보정(imeco_gbn=-30) 소멸 → imeco_gbn=0 고정
#     - 시장은 TR코드 런타임 검출로 구분 (채권 TTRODP4130.. / 현·파 TTRODP113..)
#   → -DA1201/A2201 폐기, 1 소스 = 1 바이너리가 채권+현·파 응답을 모두 분배.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_ttrodp11301.h"     /* 현/파 회원처리호가 응답(318B) */
#include    "seam_queue.h"          /* 부문간 SEAM 스테이징 큐(pc_/pb_ 수신 → 분배 소비) */

/* 전 시장 KRX form: 채권 회원처리호가 291 / 현·파 318 → 4+11+318=333 < 400 */
#define     DATA_SIZE       400
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME       60 * 1000           /* Poll 대기 시간 (60초) */
#define     SEAM_POLL       200                 /* SEAM 재스캔 주기(ms). 프로덕션 저지연은 doorbell FIFO 후속 */
#define     READ_MAX        1

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         R_Cnt;
char        ApType[10];
FILE_BUFF_FORMAT    R_Fmt[READ_MAX];
char        Seam_Buf[READ_MAX * SEAM_Q_RECSZ];  /* SEAM 슬롯(480B) 수신 버퍼 → R_Fmt로 복사 */

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PO_1200_MP(void);
void    Analyze_Data(void);
int     Check_rtn(char *, int);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[]) {
    Init_Proc(argc, argv);
    SEAM_Init();        /* 부문간 SEAM 스테이징 큐 확보(응답 → 분배) */
    PO_1200_MP();
    Exit_Process();
}   /* End of main () */

/*----------------------------------------------------------------------*/
void    PO_1200_MP(void) {
    int     rt;

    sprintf(ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU(ApType, strlen(ApType));

    while (START_S != JOB_END) {
        Stat_Save();

        while (START_S != JOB_END) {
            int     si;

            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            /* 부문간 전달: 파일큐 대신 전역 SEAM 응답 큐에서 소비(리더=분배).
               응답 큐는 미체결(po_1290, SEAM_R_MICHE)·분배(po_1200, SEAM_R_DIST)가
               각자 독립 커서로 읽는 multi-reader. 슬롯(480B)→R_Fmt 복사 후 파싱 동일. */
            R_Cnt = SEAM_R(SEAM_Q_RESP, SEAM_R_DIST, Seam_Buf, READ_MAX);

            if (R_Cnt < 0) {
                Log(SAM_FATAL, "SEAM_R fail q=%d r=%d", SEAM_Q_RESP, SEAM_R_DIST);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;

            for (si = 0; si < R_Cnt; si ++)
                memcpy(&R_Fmt[si], &Seam_Buf[si * SEAM_Q_RECSZ], sizeof (FILE_BUFF_FORMAT));

            Log(USR_OK, "SEAM RD [q=%d r=%d][%d]", SEAM_Q_RESP, SEAM_R_DIST, R_Cnt);
            Analyze_Data();
        }

        rt = Poll_File(SEAM_POLL);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        else if (rt == -1)
            continue;
    }
}   /* End of PO_1200_MP () */

/*************************************************************************
    Function        : Analyze_Data
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void) {
    int     i;

    for (i = 0; i < R_Cnt; i++) {
        Check_rtn(R_Fmt[i].Data, i);
        /* SEAM 소비는 SEAM_R 시점에 r_seq 전진(파일큐 Add_Count 불필요) */
    }

    return;
}   /* End of Analyze_Data () */

/*************************************************************************
    Function        : Check_rtn — 응답 분석 후 전략(DSHM)·Client FIFO 분배
    Comment         : 전 시장 KRX form [4 ErrCode][11 seq][전문]. TR코드(@26)로
                      전문 구조체를 골라 회원사용영역(60B) 추출 → 매체/전략 판별.
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Check_rtn(char *p_buf, int for_d) {
    int     rt;
    char    W_Fmt[8192];
    char    member_area[60];                    /* 회원사용영역(매체구분@30+전략@47) */
    int     w_flag, target;
    int     imeco_gbn = 0;                      /* KRX-direct 통일 → 보정 없음(IMECO 폐기) */

    w_flag = 0;
    target = -1;
    memset(member_area, 0, sizeof(member_area));

    /* REJC(거부) 스킵: pa_5000_qr가 송신 미기동 시 거부처리한 데이터는 재처리 안 함 */
    if (memcmp(&p_buf[0], "REJC", 4) == 0) {
        Log(USR_OK, "Reject SKIP Write OK ResponseCode[%10.10s]", p_buf);
        return (OK);
    }

    /*
     * TR코드(@ 4+11+11 = 26)별 회원사용영역 추출 — 전 시장 런타임 분기.
     * 채권(접미사 4/K): TCHODR4000/TCHMOR4000/TCHKOR1000/TTRODP4130/TTRMOP4130/TTRKOP1130
     * 현·파(접미사 1): TTRODP113(회원처리호가 정상11301/거부11321/자동취소11303, 318B)
     */
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
    else if (memcmp(&p_buf[4+11+11], "TTRODP113", 9) == 0) {     /* 현·파 회원처리호가 */
        TTRODP11301_DATA *dat = (TTRODP11301_DATA *)&p_buf[4+11];
        memcpy(member_area, dat->Member_Use_Area, sizeof(dat->Member_Use_Area));
    }
    else {
        Log(USR_WARN, "Check_rtn: unhandled TR[%10.10s] skip", &p_buf[4+11+11]);
        return (OK);
    }

    /*
     * 매체구분 member_area[30]: A=API C=Client T=자동매매 → 자사 주문만 전달
     * (imeco_gbn=0이므로 채권/현·파 동일 오프셋)
     */
    if ((memcmp(&member_area[30+imeco_gbn], "A", 1) == 0) ||
            (memcmp(&member_area[30+imeco_gbn], "C", 1) == 0) ||
            (memcmp(&member_area[30+imeco_gbn], "T", 1) == 0)) {
        w_flag = 1;
    }

    /* 자동매매 전략번호 member_area[47], 4자리 > "0000" → target = (앞2-1)*10 + 뒤2 */
    if (memcmp(&member_area[47+imeco_gbn], "0000", 4) > 0) {
        target = (AtoIf(&member_area[47+imeco_gbn], 2) - 1) * 10;
        target = AtoIf(&member_area[49+imeco_gbn], 2) + target;
    }

    /* 자사 주문 → DSHM(전략) + Client FIFO 분배 */
    if (w_flag > 0) {
        memset(W_Fmt, 0x20, sizeof (W_Fmt));
        memcpy(&W_Fmt, &R_Fmt[for_d], sizeof (FILE_BUFF_FORMAT)-1);
        W_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

        if (target > 0) {
            rt = DSHM_W(target*10, (void *)&W_Fmt, 1);
            if (rt != 1) {
                Log(SAM_FATAL, "Dshm write fail[%s:%d]", ODN(D_K,P_K,target-1), rt);
                Exit_Process();
            }
            Log(USR_OK, "Write OK [%s][%d] target[%d]", W_Fmt, strlen(W_Fmt), target);
        }

        rt = F_W(TS_W1_1, (void *)&W_Fmt, 1);
        if (rt != 1) {
            Log(SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
            Exit_Process();
        }
    }

    return (OK);
}

/*************************************************************************
    End of Program (po_1200_mp.c)
*************************************************************************/
