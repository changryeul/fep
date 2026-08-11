#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : 주문접수처리(미체결내역관리)
#   File    : pa_1290_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "lat_trace.h"
#include    "miche_idx.h"

#if defined A1291
#define     MK_GBN      0   /* 0:채권 */
#define     DATA_SIZE   400
#elif defined A2291
#define     MK_GBN      1   /* 1:파생 */
#define     DATA_SIZE   200
#endif

#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000
#define     READ_MAX    1

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int         R_Cnt;
FILE_BUFF_FORMAT    R_Fmt[READ_MAX];

/* F6 miche-index: 계좌별 주문번호 조회 캐시 (프로세스-로컬, 권위는 SHM 스캔) */
static MICHE_IDX    Midx[ACC_NO_CNT];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_1290_MP(void);
void    Analyze_Data(void);
int     Make_MiChe(char *, int);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc(argc, argv);
#ifdef LAT_TRACE
    LAT_INIT(argv[0]);
#endif

    {
        /* F6 miche-index: 조회 캐시 초기화 */
        int     mi;

        for (mi = 0; mi < ACC_NO_CNT; mi ++)
            Miche_Idx_Reset(&Midx[mi]);
    }

    PA_1290_MP();
    Exit_Process();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_1290_MP(void)
/*----------------------------------------------------------------------*/
{
    int     rt, read_flag;

    while (START_S != JOB_END) {
        Stat_Save();

        while (START_S != JOB_END) {
            Stat_Save();
            memset(R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            R_Cnt = F_R(PS_R_2, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0) {
                Log(SAM_FATAL, "cannot read File[%s,%d:%s]",
                        IFN(D_K,P_K,2), SYS_NO, SYS_STR);
                sleep(1);
                Exit_Process();
            }
            else if (R_Cnt == 0)
                break;

            Log(USR_OK, "RD [%s:%d][%d]",
                    IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,1,0));
            Analyze_Data();
        }

        rt = Poll_File(DATA_TIME);

        if (rt == 1)
            Log(USR_OK, "poll timeout <%d>", OFW(D_K,P_K,0,0));
        else if (rt == -1)
            continue;
    }
}   /* End of PA_1290_MP () */

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . analyze and divide data
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data(void)
/*----------------------------------------------------------------------*/
{
    int     i, rt;

    for (i = 0; i < R_Cnt; i ++) {
        /* SKIP로직 */
        /* 읽는 데이터 포멧 : 55 + 주문전문 */
        /* 20250901
            qr에서 수신받아서 ps_1101_ts의 기동이 안되어 있으면 1201mp에 write한다.
            이때 헤더에는 "REJC"으로 사유코드를 넣는다. 이는 테스트를 위함이다.
            계속사용할수도 있다(의사결정필요).
            pa_1101_ts의 세션으로 처리하면 문제가 된다. 그래서 기동여부로 체크한다.
        */
#if defined A1291
        if (memcmp(&R_Fmt[i].Data, "REJ", 3) == 0)  // CLIENT에게 주문거부 송신(REJC/REJE)
#elif defined A2291
            if (memcmp(&R_Fmt[i].Data[5], "REJ", 3) == 0)  // CLIENT에게 주문거부 송신(REJC/REJE)
#endif
            Log(USR_OK, "SKIP DATA Write OK ResponseCode[%4.4s]", &R_Fmt[i].DataHeader[10]);
        else {
#ifdef LAT_TRACE
            /* 미체결 갱신(선형 스캔 포함) 소요 측정 - 전문별 OrderNo 오프셋이
               달라 단조증가 시퀀스를 구간 키로 사용 (F6 개선 전후 비교용) */
            static long lat_seq = 0;
            char        lat_key[16];

            sprintf(lat_key, "%010ld", ++lat_seq);
            LAT_POINT("IN", lat_key, 10);
            rt = Make_MiChe(R_Fmt[i].Data, i);
            LAT_POINT("OUT", lat_key, 10);
#else
            rt = Make_MiChe(R_Fmt[i].Data, i);
#endif
        }
        /* SKIP로직 */

        Add_Count(PS_R_2, 1);
    }

    return;
}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Make_MiChe
    Parameters IN   : . p_buf   : received data
                      . for_d   : data count
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set Acc data SHM (손익/수수료/평가손익)
*************************************************************************/
/*----------------------------------------------------------------------*/
int   Make_MiChe(char *p_buf, int for_d)
/*----------------------------------------------------------------------*/
{
    int     i, j, rt, order_no, loop, mng_flag = 0;
    int     ord_cnt, real_ord_cnt, err_flag, m_size;
    int     mk_gbn, item_seq, acc_seq, trad_flag, imeco_gbn;
    char    tr_code[12];

    /* 현물만 금액이 long이고 채권과 파생은 소수점 2자리다(double) */
    long    org_cnt, edt_cnt;  // 일반용 (채권,금융파생)
    double  org_gum, edt_gum;  // 일반용 (채권,금융파생)

    long    org_cnt01, org_cnt02, edt_cnt01, edt_cnt02;  // 채권LP용 매도
    double  org_gum01, org_gum02, edt_gum01, edt_gum02;  // 채권LP용 매수

    BUFF_RW_HEAD    f_head;

    err_flag = 0;
    /* 처리할 데이터의 포맷이 다양해서 구분하여 처리한다. */
    /* 응답은 주문거부도 들어가기 때문에 DATA에는 (4+11 + 거래소응답) + 전문 */
    memset(tr_code, 0, sizeof (tr_code));
#if defined A1291
    m_size = 60;
    memcpy(tr_code, &p_buf[4+11 +11], 11);
#elif defined A2291
    m_size = 20;
    memcpy(tr_code, &p_buf[20], 1);  // C:Confirm(확인), R:Reject(거부), A:Expired(자동취소)
#endif

    /* ************************************************************************ */
    /* 미체결내역 처리(미니원장)                                               */
    /* ************************************************************************ */
    /* 내가낸 주문만 처리한다.                                                    */
    /* ************************************************************************ */
    /*   TTRODP11301(정산)만 처리한다.                                         */
    /* ************************************************************************ */
    /* - 한도는 신규주문시점에 잡는다.                                           */
    /*          정정/취소는 회원처리호가 수신시점(여기서) 가감한다.               */
    /* - 미체결관리는 아래 로직으로 처리한다.                                   */
    /*   미체결등록은 회원처리호가 정상접수시에 한다.                           */
    /*   그래야 미접수분에 대한 정정/취소를 못하게 할 수 있다.                    */
    /*   미체결은 Main이 수량이다. 매수수량+매도수량.                            */
    /*            정정은 금액변경이 필수이고, 시스템트레이딩에서는                */
    /*            일부정정 하지않는다.(손으로 입력해야해서)                       */
    /* 20211108 처리로직 변경, 주문송신시 미체결수량 가감 선행한다.               */
    /*          선행을 해야 중복주문이 안나간다.                              */
    /* ************************************************************************ */
    /* 회원처리호가에 따른 미체결 처리방안(미체결은 "주문번호+계좌"=Key)      */
    /* 1(신규) +(추가), 2(정정) 추가없음_증거금만변경, 3(취소) -(삭제및감소)   */
    /* TTRODP11301(확인)시 미체결등록은 동일하고 미체결수량 가감만 변경            */
    /*                  1(신규)/2(정정) : 수량은 가감없음, 3(취소) : -           */
    /*                  정정&매수&정정금액증가 : 처리수량만큼 증거금 증가        */
    /*                  정정&매수&정정금액감소 : 처리수량만큼 증거금 감소        */ // 2025로직추가
    /* TTRODP11321(거부)시 미체결등록은 동일하나 미체결수량 가감만 처리            */
    /*                  1(신규) -,2(정정)/2(취소) 가감없음                        */
    /* TTRODP11303(자동취소)시 (취소의 자동취소는 없다)                            */
    /*                  자동취소 수량만큼 감소 & 증거금 감소                   */
    /* ************************************************************************ */
    /* 회원처리호가 정상접수 처리 */
#if defined A1291
    if (memcmp(tr_code, "TTRODP41301", 11) == 0) {  // TTRODP41301(채권일반확인, 291)
        KRX_NOTE_SETTLE_RESP_DATA   *dat    =
    (KRX_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11];

    imeco_gbn = 0;
#elif defined A2291
    if (memcmp(tr_code, "C", 1) == 0) {  // C:Confirm(확인)
        IMECO_SETTLE_RESP_DATA  *dat    =
    (IMECO_SETTLE_RESP_DATA *)&p_buf[20];

    imeco_gbn = -30;
#endif

    mk_gbn      =   AtoIf(dat->MembershipItem+imeco_gbn+35, 1);  // 시장구분
    item_seq    =   AtoIf(dat->MembershipItem+imeco_gbn+37, 5);  // 종목 Seq
    acc_seq     =   AtoIf(dat->MembershipItem+imeco_gbn+42, 2);  // 계좌 seq

    if (memcmp(dat->TradeFlag, "1", 1) == 0)
        trad_flag   =   -1;
    else
        trad_flag   =    1;
    /* ******************************************************** */
    /* MembershipItem => 30 byte부터 사용가능                   */
    /* 파생(IMECO는 20바이트만 준다.)                            */
    /*     따라서 스프레드는 사용못한다.                     */
    /* ******************************************************** */
    /*  4, 5, 6, 7, 8 : 스프레드 근월물 종목Seq(미사용)         */
    /* 16,17,18,19,20 : 스프레드 원월물 종목Seq(미사용)         */
    /* 30 : A(서버자동주문), C(메리츠Client주문) T(윈웨이매체)  */
    /*    : 매체구분 (A:Auto or All, C:메리츠매체, T:윈웨이매체 */
    /*      A는 자동주문/주문응답,체결등 주문관련된건(TR100XXX) 양쪽매체에 보낸다.*/
    /*      Data Header 50 Byte중 20번째 1자리와 같이 사용한다(조회등) */
    /* 31,32,33,34 : 전략에서 사용                              */
    /* 35, 36 : 시장구분은 35 1자리만으로 체크                  */
    /*  (운용상품분류코드)  (시장index) (운영상품index) (시장구분명)*/
    /*          01                   0           1      채권일반
                02                   0           2      채권LP
                11                   1           1      금융파생_국채선물
                12                   1           2      금융파생_통화선물
                13                   1           3      금융파생_금리선물
    */
    /* 37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)           */
    /* 42, 43 : 계좌번호 seq                                    */
    /* 44     : 미사용                                         */
    /* 45     : 스프레드종목이면 1, 아니면 0                    */
    /* 46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2' */
    /*          나머지시장이면 '0'                              */
    /* 47,48,49,50 : ApType 4자리 (50101중 0101만) Set          */
    /* ******************************************************** */

    /* 내가낸 주문만 처리한다 */
    if ((memcmp(dat->MembershipItem+imeco_gbn+30, "A", 1) != 0)    &&
            (memcmp(dat->MembershipItem+imeco_gbn+30, "C", 1) != 0)    &&
            (memcmp(dat->MembershipItem+imeco_gbn+30, "T", 1) != 0)    ) {
        //          Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
        Log(USR_ERROR, "타매체 수신 [%30.30s] [%.10s]", dat->MembershipItem+imeco_gbn+30, dat->OrderNo);
        return (OK);
    }

    /* ******************************************************************************** */
    /* 강제종료 처리                                                                  */
    // 협의필요
    /* ******************************************************************************** */
    /* 정정/취소 주문이고                                                               */
    /* 원주문번호는 서버주문 && 주문번호는 CLIENT주문시 원주문번호의 자동주문 강제종료  */
    /* ******************************************************************************** */
    /* ************************************************************************ */
    /*              응답(1291/2291)의 미체결내역 관리                         */
    /* ------------------------------------------------------------------------ */
    /* 신규       정상 : 신규추가                                                   */
    /*          거부 : SKIP (미체결SKIP, 한도감소)                               */
    /*          자취 : 감소 (미체결/한도 감소)                                     */
    /* ------------------------------------------------------------------------ */
    /* 정정       정상 : 정정주문 신규추가                                      */
    /*                 원주문번호찾아서 주문잔량 감소(실제처리건수로)            */
    /*                 증거금도 변경(증가시,감소시 고려해서, 한도포함)          */
    /*          거부 : SKIP (미체결SKIP, 한도가감)                               */
    /*                 금액증가시 감소, 금액감소시 증가                           */
    /* 취소       정상 : 원주문번호찾아서 주문잔량 감소(실제처리건수로)          */
    /*                 한도포함                                                 */
    /* ************************************************************************ */
    /* 미체결 처리                                                               */
    /* - 신규/정정   : 등록처리한다. 이후 정정은 원주문번호 찾아서 수량을 조정  */
    /* - 취소        : 원주문번호 찾아서 수량을 조정                           */
    /* ------------------------------------------------------------------------ */
    /* 신규 : 등록                                                              */
    /* 정정 : 등록/조정                                                           */
    /* 취소 :      조정                                                         */
    /* ************************************************************************ */

    if ((memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0)  ||
            (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0)  ) {
        while (1) {     /* miche-deadcode-fix B-2: 만석 재시도 복원 */
        for (i = 0; i < MAX_MICHE; i++) {
            /* 신규로 등록 */
            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0) {
                Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]++;

                /* int1. 주문잔량 */
                if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0)
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf(dat->OrderQuantity, 10);
                else
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf(dat->Real_Modify_Cancel_Cnt,    10);
                /* int2. 시장구분 */
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Mk_gbn   = mk_gbn;
                /* int3. 종목일련번호 */
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq = AtoIf(dat->MembershipItem+imeco_gbn+37, 5);
                /* int4. 계좌일련번호 */
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Acc_Seq  = AtoIf(dat->MembershipItem+imeco_gbn+42, 2);
                /* int5. 전략번호 */

                /* 1. 계좌번호 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo,       dat->AccountNo, 12);
                /* 2. 종목코드 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Cd,         dat->ItemCode,  12);
                /* 3. 주문번호 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,         dat->OrderNo,   10);
                /* 4. 원주문번호 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OriginalOrderNo, dat->OriginalOrderNo,       10);
                /* 5. 정정취소구분 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderFlag,       dat->New_Modify_Cancel_gbn, 1);
                /* 6. 매도매수구분 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag,       dat->TradeFlag, 1);
                /* 7. 주문수량 (&) */
                if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0)
                    memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,   &dat->OrderQuantity[2],             8);
                else
                    memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,   &dat->Real_Modify_Cancel_Cnt[2],    8);
                /* 8. 주문가격 (&), 가격없는 주문유형 */
#if defined A1291
                /* 채권은 2:지정가만 허용 */
                /* 가격은 채권은 소수점 2자리, 파생도 소수점 2자리, 단 채권은 부호없고 파생은 젤앞자리1개가 부호임 */
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv = AtoDf(&dat->Price[2], 9);

                /* 한도체크용 정정주문 금액한도감소용 사전처리 */
                if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {
                    edt_gum = AtoDf(&dat->Price[2], 9);
                    edt_cnt = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8);
                }
#elif defined A2291
                if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) {  // !스프레드
                    /* 스프레드가 아니면 미체결 수량과 금액을 감소시켜줘야한다. 금액감소를 위한 사전작업 */
                    org_gum = edt_gum = edt_cnt = 0;
                    // 주문수량*거래승수*금액
                    if ((memcmp(dat->Order_Type, "T", 1) == 0)    ||  // 시장가(1대신 T)
                            (memcmp(dat->Order_Type, "W", 1) == 0)) {  // 최유리 지정가(X대신 W)
                    /* 매도매수 구분없이 주문금액이 없는 주문유형은 최대로 금액을 잡는다(상한가) 한도 때문에 */
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
                }
                else
                    /* 금융파생은 실제 음수를 사용하지 않는다. 로직에서도 음수는 배제했음 */
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv = AtoDf(&dat->Price[2], 9);

                if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {
                    edt_gum = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
                    edt_cnt = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8);
                }
            }
            else {  // 스프레드, 미체결 수량만처리하고 금액은 처리하지 않는다. 금융파생에 스프레드가 있으면 딜러에게 로직 설명받아야함.
            }
#endif
            /* 9. 호가유형코드 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderType,       dat->Order_Type,1);
            /* 10. 호가조건 (0:일반, 3:IOC, 4:FOK) */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].JumunFlag,       dat->Order_Condition,   1);
            /* 11. 회원사처리항목 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem,  dat->MembershipItem,    m_size);

            Log(USR_OK, "MiChe 등록 Mem [%d][%12.12s] acc_seq[%d] JCNT[%10.10s] JN[%10.10s] OJN[%10.10s] MK[%d]",
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,
                    dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

            /* F6: 신규 등록 슬롯을 조회 캐시에 적재 */
            Miche_Idx_Put(&Midx[acc_seq], dat->OrderNo, i);

            break;
        }   /* End of 신규등록 */
    }   /* End of for MAX_MICHE */

        if (i < MAX_MICHE)
            break;      /* 등록 완료 */

        /* miche-deadcode-fix B-2: 만석 시 10초 대기 후 재검색 (데드코드 복원) */
        Log(USR_ERROR, "미체결공간 10,000모두사요 10초후 재시도 시장[%d]", MK_GBN);
        sleep(10);
        }   /* End of while (만석 재시도) */
}   /* Endof 신규 or 정정 */

/* 원주문번호 찾아서 수량 조정해줘야 한다 */
if ((memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0)  ||
        (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0)  ) {
    /* F6: 캐시 선조회(검증 포함) - 히트 시 그 슬롯부터 루프 진입, 미스 시 스캔 폴백+보정 */
    i = Miche_Idx_Get(&Midx[acc_seq], dat->OriginalOrderNo);
    if (i >= 0 && memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
            dat->OriginalOrderNo, 10) != 0)
        i = -1;     /* 낡은 캐시 - 폴백 */

    if (i < 0) {
        for (i = 0; i < MAX_MICHE; i++) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                    dat->OriginalOrderNo, 10) == 0)
                break;
        }
        if (i < MAX_MICHE)
            Miche_Idx_Put(&Midx[acc_seq], dat->OriginalOrderNo, i);
    }

    for ( ; i < MAX_MICHE; i++) {
        /* 원주문번호 찾기 */
        if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OriginalOrderNo, 10) == 0) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, dat->AccountNo, 12) != 0) {
                Log(USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 OJN[%10.10s]",
                        dat->OriginalOrderNo);
                return (NOTOK);
            }

            // 실제처리된 수량
            real_ord_cnt = AtoIf(dat->Real_Modify_Cancel_Cnt, sizeof (dat->Real_Modify_Cancel_Cnt));
            /* 정정,취소된 원주문번호의 원래 주문가격, 수량 보관 */
            org_gum = Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv;
            org_cnt = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt, 8);

            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt < 0) {
                Log(USR_ERROR, "확인 수량 부적합 [%d][%d] 주문번호 [%10.10s]",
                        Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, real_ord_cnt, dat->OriginalOrderNo);
                return (NOTOK);
            }

            /* 원주문 주문잔량 조정 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt;

            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

            Log(USR_OK, "MiChe 조정 Mem [%d][%12.12s] acc_seq[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
                    dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

            break;
        }   /* End of 원주문번호 찾기 */
    }   /* End of for MAX_MICHE */

    /* miche-deadcode-fix B-4/B-5: 못찾음 처리 복원 (루프 내부 데드코드였음) */
    if (i >= MAX_MICHE) {
        Log(USR_ERROR, "원주문번호 못찾았음. MK[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
                MK_GBN, dat->OrderNo, dat->OriginalOrderNo, MK_GBN);
        return (NOTOK);
    }

    /* ************************************************************************ */
    /* 기존에는 현물만 사용                                                      */
    /* 한도처리 로직(공매도는 대상제외, 공매도는 시스템영역아님 )                */
    /* ------------------------------------------------------------------------ */
    /* < 신규확인 > Skip                                                        */
    /* < 정정확인 >                                                             */
    /* 원주가격 < 정정가격이면 정정수량*차액 만큼 당일 매수주문금액에 더함.      */
    /* 원주가격 > 정정가격이면 정정수량*차액 만큼 당일 매수주문금액에 뺌.       */
    /* ------------------------------------------------------------------------ */
    /* 신규거부/취소확인 : 신규거부/취소확인전문 수신시,                     */
    /* 당일차감 매수 금액 증가(매수만)                                           */
    /* ************************************************************************ */
#if defined A2291
    if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0)  // !스프레드, 정정시 미체결 수량X, 차액만 수정
        //            취소시 미체결 수량 && 금액 수정
    {
#endif
        if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {  // 정정
            if (edt_gum > org_gum) {  // 가격증가
                if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수
                    /* 매수주문금액 증가(차액*수량) */
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum +=
                    (double)((edt_gum-org_gum) * real_ord_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
                }
                else {  // 매도
                    /* 매수주문금액 증가(차액*수량) */
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum +=
                    (double)((edt_gum-org_gum) * real_ord_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
                }
            }
            else
                if (edt_gum < org_gum) {  // 가격감소
                if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수
                    /* 매수주문금액 증가(차액*수량) */
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -=
                    (double)((org_gum-edt_gum) * real_ord_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
                }
                else {  // 매도
                    /* 매수주문금액 증가(차액*수량) */
                    Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -=
                    (double)((org_gum-edt_gum) * real_ord_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
                }
            }
        }
        else if (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0) {
            if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수
                /* 종목별 총 미체결수량 관리, 주문시 증가/회원처리호가시 조정  */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt  -= real_ord_cnt;
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -=
                (double)(real_ord_cnt * org_gum * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
            }
            else {  // 매도
                /* 종목별 총 미체결수량 관리, 주문시 증가/회원처리호가시 조정  */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt  -= real_ord_cnt;
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -=
                (double)(real_ord_cnt * org_gum * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
            }
        }
#if defined A2291
    }
    else {  // 스프레드, 미체결 수량만(근/원월물 각각)
        if (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0) {  // 취소, 일부취소없다(취소된만큼 뺀다)
        if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수(근월물매도,원월물매수)
        // 근월물
    Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+4,  5)][acc_seq].item_do_michecnt -= real_ord_cnt;
    // 원월물
    Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+16, 5)][acc_seq].item_su_michecnt -= real_ord_cnt;
}
else {  // 매도(근월물매수,원월물매도)
    // 근월물
Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+4,  5)][acc_seq].item_su_michecnt -= real_ord_cnt;
// 원월물
Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+16, 5)][acc_seq].item_do_michecnt -= real_ord_cnt;
}
}
}
#endif
}   /* End of 정정 or 취소 */

}   /* End of 접수확인 (TTRODP11301/TTRODP41301) */

/* 회원처리호가 거부/자동취소 처리 */
else
#if defined A1291
    if ((memcmp(tr_code, "TTRODP41302", 11) == 0)  ||  // TTRODP41301(확인), TTRODP41302(거부), TTRODP41303(자동취소)
            (memcmp(tr_code, "TTRODP41303", 11) == 0)) {
    KRX_NOTE_SETTLE_RESP_DATA   *dat    =
    (KRX_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11];

    imeco_gbn = 0;
#elif defined A2291
    if ((memcmp(tr_code, "R", 1) == 0) ||  // C(확인), R(거부), A(자동취소)
            (memcmp(tr_code, "A", 1) == 0)) {
        IMECO_SETTLE_RESP_DATA  *dat    =
        (IMECO_SETTLE_RESP_DATA *)&p_buf[20];

        imeco_gbn = -30;
#endif

        mk_gbn      =   AtoIf(dat->MembershipItem+imeco_gbn+35, 1);  // 시장구분
        item_seq    =   AtoIf(dat->MembershipItem+imeco_gbn+37, 5);  // 종목 Seq
        acc_seq     =   AtoIf(dat->MembershipItem+imeco_gbn+42, 2);  // 계좌 seq

        /* ******************************************** */
        /* 한도처리 로직                              */
        /* 신규 거부나면 당일차감 매수 금액 증가        */
        /* ******************************************** */

#if defined A1291
        /* 채권은 지정가만 허용 */
        edt_gum = AtoDf(&dat->Price[2], 9);

        if (memcmp(tr_code, "TTRODP41302", 11) == 0)  // TTRODP41302(거부)
            edt_cnt = AtoLf(dat->OrderQuantity, 10);
        else  // TTRODP41303(자동취소)
            edt_cnt = AtoLf(&dat->Real_Modify_Cancel_Cnt[2],   8);
#elif defined A2291
        if (memcmp(dat->MembershipItem+imeco_gbn+45, "1", 1) != 0) {  // !스프레드, 미체결 수량 && 금액 감소
            /* 주문가격 */
            edt_gum = 0;
            if ((memcmp(dat->Order_Type, "T", 1) == 0)    ||  // 시장가(1대신 T)
                    (memcmp(dat->Order_Type, "W", 1) == 0)) {  // 최유리 지정가(X대신 W)
            edt_gum = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
        }
        else
            edt_gum = AtoDf(&dat->Price[2], 9);

        if (memcmp(tr_code, "R", 1) == 0)  // R(거부)
            edt_cnt = AtoLf(dat->OrderQuantity, 10);
        else  // A(자동취소)
            edt_cnt = AtoLf(&dat->Real_Modify_Cancel_Cnt[2],   8);
#endif
        if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {  // 신규
            if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수
                /* 매수주문금액 차감(차액*수량) */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt  -= edt_cnt;
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum -=
                (double)(edt_gum * edt_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
            }
            else {
                /* 매도주문금액 차감(차액*수량) */
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt  -= edt_cnt;
                Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum -=
                (double)(edt_gum * edt_cnt * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);
            }
        }
        /* 정정/취소의 거부는 무시 */
#if defined A2291
    }
    else {  // 스프레드
        if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {  // 신규
            /* 스프레드는 거부된 주문수량만 감소처리 한다. */
            if (memcmp(dat->TradeFlag, "2", 1) == 0) {  // 매수(근월물매도,원월물매수)
                /* 근월물(반대방향) */
                Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+4,  5)][acc_seq].item_do_michecnt -= AtoIf(&dat->OrderQuantity[2], 8);
            /* 원월물(반대방향) */
            Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+16, 5)][acc_seq].item_su_michecnt -= AtoIf(&dat->OrderQuantity[2], 8);
        }
        else {
            /* 근월물(반대방향) */
            Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+4,  5)][acc_seq].item_su_michecnt -= AtoIf(&dat->OrderQuantity[2], 8);
            /* 원월물(반대방향) */
            Shm_Risk[0].ProFit[mk_gbn][AtoIf(dat->MembershipItem+imeco_gbn+16, 5)][acc_seq].item_do_michecnt -= AtoIf(&dat->OrderQuantity[2], 8);
        }
    }
    /* 정정/취소의 거부는 무시 */
}
#endif

/* 20220323 자동취소시 미체결정리 추가 */
#if defined A1291
if (memcmp(tr_code, "TTRODP11303", 11) == 0)  // TTRODP11303:자동취소
#elif defined A2291
    if (memcmp(tr_code, "A", 1) == 0)  // A:자동취소
#endif
    {
    /* 주문번호 찾기 */
    for (i = 0; i < MAX_MICHE; i++) {
        if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OrderNo, 10) == 0) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, dat->AccountNo, 12) != 0) {
                Log(USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 JN[%10.10s]", dat->OrderNo);
                return (NOTOK);
            }

            /* 실제처리된 수량 처리 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt -=
            AtoIf(dat->Real_Modify_Cancel_Cnt, sizeof (dat->Real_Modify_Cancel_Cnt));

            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

            break;
        }   /* End of 주문번호 찾기 */
    }   /* End of For */

    /* miche-deadcode-fix B-6: 못찾음 처리 복원 (루프 내부 데드코드였음) */
    if (i >= MAX_MICHE) {
        Log(USR_ERROR, "주문번호 못찾았음. MK[%d] JN[%10.10s] acc_seq[%d]",
                MK_GBN, dat->OrderNo, acc_seq);
        return (NOTOK);
    }
}
/* 20220323 자동취소시 미체결정리 추가 */
}   /* End of TTRODP41302, TTRODP41303 */

#if defined A1291
/* ****************************************************************************** */
/* 채권LP만 처리, LP미사용시 이하 필요없는 소스                                    */
/* ****************************************************************************** */
if (memcmp(tr_code, "TTRMOP41301", 11) == 0) {  // TTRMOP41301(채권LP일반확인, 295)
    KRX_LP_NOTE_SETTLE_RESP_DATA    *dat    =
(KRX_LP_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11];

imeco_gbn = 0;

mk_gbn      =   AtoIf(dat->MembershipItem+imeco_gbn+35, 1);  // 시장구분
item_seq    =   AtoIf(dat->MembershipItem+imeco_gbn+37, 5);  // 종목 Seq
acc_seq     =   AtoIf(dat->MembershipItem+imeco_gbn+42, 2);  // 계좌 seq

if (memcmp(dat->TradeFlag, "1", 1) == 0)
    trad_flag   =   -1;
else
    trad_flag   =    1;

/* 내가낸 주문만 처리한다 */
if ((memcmp(dat->MembershipItem+imeco_gbn+30, "A", 1) != 0)    &&
        (memcmp(dat->MembershipItem+imeco_gbn+30, "C", 1) != 0)    &&
        (memcmp(dat->MembershipItem+imeco_gbn+30, "T", 1) != 0)    ) {
    //          Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
    Log(USR_ERROR, "타매체 수신 [%30.30s] [%.10s]", dat->MembershipItem+imeco_gbn+30, dat->OrderNo);
    return (OK);
}

if ((memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0)  ||
        (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0)  ) {
    while (1) {     /* miche-deadcode-fix B-3: 만석 재시도 복원 (LP) */
    for (i = 0; i < MAX_MICHE; i++) {
        /* 신규로 등록 */
        if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0) {
            Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]++;

            /* LP는 매도매수 동시에 낼 수 도 있음 */
            /* int1. 주문잔량 */
            if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {  // 신규
                Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
                AtoIf(dat->Ask_Offer_Qty, 10) + AtoIf(dat->Bid_Offer_Qty, 10);
            }
            else {  // 정정,취소
                /* 2025 채권LP 회원처리호가는 어떻게 처리해야되? 실정정취소수량 항목이 없어, 어떻게 정정과 취소를 하는지 확인필요
                                        Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt = AtoIf (dat->Real_Modify_Cancel_Cnt,    10);
                */
            }
            /* int2. 시장구분 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Mk_gbn   = 0;
            /* int3. 종목일련번호 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq = AtoIf(dat->MembershipItem+imeco_gbn+37, 5);
            /* int4. 계좌일련번호 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Acc_Seq  = AtoIf(dat->MembershipItem+imeco_gbn+42, 2);
            /* int5. 전략번호 */

            /* 1. 계좌번호 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo,       dat->AccountNo, 12);
            /* 2. 종목코드 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Cd,         dat->ItemCode,  12);
            /* 3. 주문번호 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,         dat->OrderNo,   10);
            /* 4. 원주문번호 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OriginalOrderNo, dat->OriginalOrderNo,       10);
            /* 5. 정정취소구분 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderFlag,       dat->New_Modify_Cancel_gbn, 1);
            /* 6. 매도매수구분 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag,       dat->TradeFlag, 1);  // LP는 매도매수(3)도 가능
            /* 7. 주문수량 (&) */
            if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {
                /* 매도,매수,매도매수 구분없이 처리한다 */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp01,  &dat->Ask_Offer_Qty[2], 8);
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,  &dat->Bid_Offer_Qty[2], 8);
            }
            else
                /* 2025 채권LP 회원처리호가는 어떻게 처리해야되? 실정정취소수량 항목이 없어, 어떻게 정정과 취소를 하지
                                        memcpy (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,  &dat->Real_Modify_Cancel_Cnt[2],    8);
                */
                /* 8. 주문가격 (&), 채권은 지정가만 허용: */
                memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01,    &dat->Ask_Offer_Prc[2], 9);
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02,    &dat->Bid_Offer_Prc[2], 9);

            /* 한도체크용 정정주문 금액한도감소용 사전처리 */
            if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {
                edt_gum01 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01, 9);  // 매도금액
                edt_cnt01 = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp01,   8);  // 매도수량
                edt_gum02 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02, 9);  // 매수금액
                edt_cnt02 = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,   8);  // 매수수량
            }
            /* 9. 호가유형코드 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderType,       dat->Order_Type,1);
            /* 10. 호가조건 (0:일반, 3:IOC, 4:FOK) */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].JumunFlag,       dat->Order_Condition,   1);
            /* 11. 회원사처리항목 */
            memcpy(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem,  dat->MembershipItem,    m_size);

            Log(USR_OK, "MiChe 등록 Mem [%d][%12.12s] acc_seq[%d] JCNT_1[%10.10s] JCNT_2[%10.10s] JN[%10.10s] OJN[%10.10s] MK[%d]",
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp01,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,
                    dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

            /* F6: 신규 등록 슬롯을 조회 캐시에 적재 (채권LP) */
            Miche_Idx_Put(&Midx[acc_seq], dat->OrderNo, i);

            break;
        }   /* End of 신규등록 */
    }   /* End of for MAX_MICHE */

    if (i < MAX_MICHE)
        break;      /* 등록 완료 */

    /* miche-deadcode-fix B-3: 만석 시 10초 대기 후 재검색 (데드코드 복원) */
    Log(USR_ERROR, "미체결공간 10,000모두사요 10초후 재시도 시장[%d]", MK_GBN);
    sleep(10);
    }   /* End of while (만석 재시도) */
}   /* Endof 신규 or 정정 */

/* 원주문번호 찾아서 수량 조정해줘야 한다 */
if ((memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0)  ||
        (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0)  ) {
    /* F6: 캐시 선조회(검증 포함) - 히트 시 그 슬롯부터 루프 진입, 미스 시 스캔 폴백+보정 */
    i = Miche_Idx_Get(&Midx[acc_seq], dat->OriginalOrderNo);
    if (i >= 0 && memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
            dat->OriginalOrderNo, 10) != 0)
        i = -1;     /* 낡은 캐시 - 폴백 */

    if (i < 0) {
        for (i = 0; i < MAX_MICHE; i++) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                    dat->OriginalOrderNo, 10) == 0)
                break;
        }
        if (i < MAX_MICHE)
            Miche_Idx_Put(&Midx[acc_seq], dat->OriginalOrderNo, i);
    }

    for ( ; i < MAX_MICHE; i++) {
        /* 원주문번호 찾기 */
        if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OriginalOrderNo, 10) == 0) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, dat->AccountNo, 12) != 0) {
                Log(USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 OJN[%10.10s]",
                        dat->OriginalOrderNo);
                return (NOTOK);
            }

            /* 한도체크용 정정주문한도 */
            if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {
                org_gum01 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01, 9);
                org_gum02 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02, 9);
            }
            else if (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0) {
                /* 취소시 원주문금액으로 계산 */
                org_gum01 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01, 9);
                org_cnt01 = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp01,   9);
                org_gum02 = AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02, 9);
                org_cnt02 = AtoLf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,   9);
            }

            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt < 0) {
                Log(USR_ERROR, "확인 수량 부적합 [%d][%d] 주문번호 [%10.10s]",
                        Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt, real_ord_cnt, dat->OriginalOrderNo);
                return (NOTOK);
            }

            /* 주문잔량 조정 */
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt =
            Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt - real_ord_cnt;

            if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt <= 0)
                Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq]--;

            Log(USR_OK, "MiChe 조정 Mem [%d][%12.12s] acc_seq[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, acc_seq,
                    dat->OrderNo, dat->OriginalOrderNo, MK_GBN);

            break;
        }   /* End of 원주문번호 찾기 */
    }   /* End of for MAX_MICHE */

    /* miche-deadcode-fix B-4/B-5: 못찾음 처리 복원 (루프 내부 데드코드였음) */
    if (i >= MAX_MICHE) {
        Log(USR_ERROR, "원주문번호 못찾았음. MK[%d] JN[%10.10s] O_JN[%10.10s] MK[%d]",
                MK_GBN, dat->OrderNo, dat->OriginalOrderNo, MK_GBN);
        return (NOTOK);
    }

    /* **************************************************** */
    /* 한도처리 로직(매수만)                             */
    /* ---------------------------------------------------- */
    /* 정정확인 : 정정확인 전문 수신시 ,                     */
    /* 원주문증거금 < 정정증거금 이면 증가 정정시에만(org)  */
    /* 원주가격 < 정정가격이면 정정수량*차액 만큼(수정본)    */
    /* 당일 매수주문금액에 더함.(감소정정시에는 반영없음) */
    /* ---------------------------------------------------- */
    /* 신규거부/취소확인 : 신규거부/취소확인전문 수신시, */
    /* 당일차감 매수 금액 증가(매수만)                       */
    /* **************************************************** */
    if ((memcmp(dat->TradeFlag, "2", 1) == 0)  ||  // 2:매수
            (memcmp(dat->TradeFlag, "3", 1) == 0)  ) {  // 3:매도수
        if (memcmp(dat->New_Modify_Cancel_gbn, "2", 1) == 0) {
            if (edt_gum02 > org_gum02) {
                /* 매수주문금액 누적(차액*수량) */
                //                      Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_tot_sugum += ((edt_gum02-org_gum02) * real_ord_cnt);
            }
        }
        else if (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0) {
            /* 매수당일차감금액 누적 */
            //                  Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum += (org_gum02 * real_ord_cnt);
        }
    }

    if ((memcmp(dat->TradeFlag, "2", 1) == 0)  ||  // 1:매도
            (memcmp(dat->TradeFlag, "3", 1) == 0)  ) {  // 3:매도수
        if (memcmp(dat->New_Modify_Cancel_gbn, "3", 1) == 0) {
            /* 매도시 취소의 수량만큼 감소처리 한다. */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= real_ord_cnt;
        }
    }
}   /* End of 정정 or 취소 */

}   /* End of TTRMOP41301 */
/* 채권LP */
else
    if ((memcmp(tr_code, "TTRMOP41302", 11) == 0)  ||  // LP, TTRMOP41301(확인), TTRMOP41302(거부), TTRMOP41303(자동취소)
            (memcmp(tr_code, "TTRMOP41303", 11) == 0)) {
    KRX_LP_NOTE_SETTLE_RESP_DATA    *dat    =
    (KRX_LP_NOTE_SETTLE_RESP_DATA *)&p_buf[4+11];

    imeco_gbn = 0;

    mk_gbn      =   AtoIf(dat->MembershipItem+imeco_gbn+35, 1);  // 시장구분
    item_seq    =   AtoIf(dat->MembershipItem+imeco_gbn+37, 5);  // 종목 Seq
    acc_seq     =   AtoIf(dat->MembershipItem+imeco_gbn+42, 2);  // 계좌 seq

    /* ******************************************** */
    /* 한도처리 로직                              */
    /* 신규 거부나면 당일차감 매수 금액 증가        */
    /* ******************************************** */
    if ((memcmp(dat->TradeFlag, "2", 1) == 0)  ||  // 1:매수
            (memcmp(dat->TradeFlag, "3", 1) == 0)  ) {  // 3:매도수
        if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {  // 신규
            /* 채권은 2:지정가만 허용 */
            edt_gum02 = AtoLf(dat->Bid_Offer_Prc, 11);
            edt_cnt02 = AtoLf(dat->Bid_Offer_Qty, 10);

            /* 매수당일차감금액 누적 */
            //              Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum += (edt_gum02 * edt_cnt02);
        }
    }

    if ((memcmp(dat->TradeFlag, "1", 1) == 0)  ||  // 1:매도
            (memcmp(dat->TradeFlag, "3", 1) == 0)  ) {  // 3:매도수
        if (memcmp(dat->New_Modify_Cancel_gbn, "1", 1) == 0) {  // 신규
            /* 매도시 거부된 주문수량만 감소처리 한다. */
            Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt -= AtoIf(&dat->Ask_Offer_Qty[2], 8);
        }
    }

    /* 20220323 자동취소시 미체결정리 추가 */
    if (memcmp(tr_code, "TTRMOP41303", 11) == 0) {  // LP자동취소
        /* 주문번호 찾기 */
        for (i = 0; i < MAX_MICHE; i++) {
            if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo, dat->OrderNo, 10) == 0) {
                if (memcmp(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].AccountNo, dat->AccountNo, 12) != 0) {
                    Log(USR_ERROR, "주문주체와 원주문주체의 계좌가 다르다. 확인요망 JN[%10.10s]", dat->OrderNo);
                    return (NOTOK);
                }

                break;
            }   /* End of 주문번호 찾기 */
        }   /* End of For */

        /* miche-deadcode-fix B-7: 못찾음 처리 복원 (루프 내부 데드코드였음) */
        if (i >= MAX_MICHE) {
            Log(USR_ERROR, "주문번호 못찾았음. MK[%d] JN[%10.10s] acc_seq[%d]",
                    MK_GBN, dat->OrderNo, acc_seq);
            return (NOTOK);
        }
    }
    /* 20220323 자동취소시 미체결정리 추가 */
}   /* End of TTRMOP41302, TTRMOP41303 */
#endif

return (OK);
}   /* End of MakeMiChe */

/*************************************************************************
    End of Program (pa_1290_mp.c)
*************************************************************************/
