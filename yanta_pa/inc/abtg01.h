#ifndef     __ABTG01_H
#define     __ABTG01_H
/*------------------------------------------------------------------------
#   Module  : common header files
#   File    : abtg01.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include <stdbool.h>
#include  "fep_sub.h"
#include  "fep_interface.h"

#ifndef BASKET_MAX
#define BASKET_MAX 300
#endif

#define ORDR_INFO_SHM_KEY 0x21000015L
int 			ORDR_INFO_Shmid;
extern  int     ORDR_INFO_Shmid;

//------------------------------------------------------------------------------
//  선물후처리내부헤더 (주문내용외 부가정보)
//-------------------------------------------------------------------------------
typedef struct {
    char        body_len                [  3]   ;   // Body Length
    char        channel                 [  1]   ;   // E:원장 G:Algo
    char        product_dstcd           [  1]   ;   // A.통화선물 B.채권선물 C.채권 D.FX
    char        fcm_id                  [ 10]   ;   // 증권/선물 회원사 번호
    char        rulez_no                [ 15]   ;   // Rule 번호 (with 전략번호)
    char        ref_no                  [ 15]   ;   // 참조번호: 자동거래구분코드(1자리) + 실행일자(8자리) + 일련번호(6자리)
    char        strg_grp_no             [ 10]   ;   // 전략그룹번호. OMS에서는 기본 셋팅. 화면에서는 0 셋팅(신규/정정/취소 모두).
    char        bk_no                   [ 11]   ;   // 북번호
} INNERMSG_HDR;

//------------------------------------------------------------------------------
// 선물주문 BODY 
//-------------------------------------------------------------------------------
typedef struct {
    char        msg_cd                  [  1]   ;   // Order Message Type. - 무조건 'S'-Single Order
    char        board_id                [  2]   ;   // KRX Board ID - 무조건 'G1'
    char        ordr_id                 [ 10]   ;   // Order ID - OMS 주문대역 - 6,410,001 ~ 6,450,000
    char        orgl_ordr_id            [ 10]   ;   // Original Order ID. Amend/Cancel KRX Original ID, New Order Space Set
    char        issue_cd                [ 12]   ;   // Issue Code.  KRX ISIN Code
    char        side                    [  1]   ;   // Side.  1.Sell 2.Buy
    char        ordr_dstcd              [  1]   ;   // New or Amend or Cancel.  1.New  2.Amend  3.Cancel - 2. 정정은 안함
    char        acno                    [ 12]   ;   // Account Number
    char        ordr_qanty              [ 10]   ;   // Order Quantity ex) 0000000040
    char        ordr_prc                [ 11]   ;   // Order Price    ex) 00001383.20
    char        ofpr_dstcd              [  1]   ;   // Order Type - T : 시장가 
                                                    // T.시장가 (T.가격제한시장가)
                                                    // 2.지정가
                                                    // I.조건부지정가 (I.조건부지정가)- 장중 미체결주문이 장종료 동시호가에 시장가로 자동 전환되는 호가유형. (I.조건부지정가)
                                                    // W.최유리지정가 (W.가격제한 최유리지정가)  - 가격 입력 불가. 상대 최우선 매수/도 호가 자동셋팅(KRX)
    char        ctrcg_cndn_dstcd        [  1]   ;   // 체결조건구분코드(0.FAS 3.FAK(IOC) 4.FOK)
    char        ip                      [ 12]   ;   // Order Identification Info. IP Information - 183.51.94.54
    char        pt                      [  2]   ;   // Program Trading Type Code - 무조건 '00'
    char        area                    [ 20]   ;   // Customer Use Area - space
    char        algo                    [  1]   ;   // 알고리즘 전략구분코드.   1.일반호가  2.알고리즘호가  3.고속 알고리즘호가  // * 무조건 '2' 정정 및 취소는 SPACE로 입력
    char        subid                   [  2]   ;   // 거래자 SubID. 부여받은 알고리즘 Sub ID - 무조건 space
    char        group_no                [  2]   ;   // 호가그룹번호 - 무조건 space
    char        defend_cd               [  1]   ;   // 자전거래방지코드 - 무조건 '0'
} EUGMSG_ORD;

//------------------------------------------------------------------------------
//  SMB와 주문, 주문확인 및 체결을 처리하는 Structure
//-------------------------------------------------------------------------------
typedef struct {
    char    smb_MsgType         [  1];  // TAG-35   메세지유형       : 'D'-신규, 'F'-취소, '3'-거부, '8'-주문확인 및 체결"
    char    smb_SenderCompID    [ 50];  // TAG-49   송신회사ID       :
    char    smb_TargetCompID    [ 50];  // TAG-56   수신회사ID       :
    char    smb_SenderSubID     [ 50];  // TAG-50   송신종ID         :
    char    smb_TargetSubID     [ 50];  // TAG-57   수신종ID         :
    char    smb_DeliverToCompID [ 50];  // TAG-128  배달대상회사ID   :
    char    smb_DeliverToSubID  [ 50];  // TAG-129  배달대상종ID     :
    char    smb_OrdStatus       [  1];  // TAG-39   주문상태         : '0'-NEW
                                        //                           : '1'-Partially filled
                                        //                           : '2'-Filled
                                        //                           : '4'-Canceled
                                        //                           : '8'-Rejected"
    char    smb_ExecType        [  1];  // TAG-150  거래유형         : '0'-New
                                        //                           : '1'-Partially Filled
                                        //                           : '2'-Filled
                                        //                           : '4'-Canceled
                                        //                           : '5'-정정
                                        //                           : '6'-취소대기
                                        //                           : '8'-Rejected(거절)
                                        //                           : 'C'-주문만료(Expired)
                                        //                           : 'E'-정정대기
                                        //                           : 'F'-Filled"
    char    smb_Account         [ 30];  // TAG-1    계좌             : SMB로부터 부여받은 ID
    char    smb_ClOrdID         [ 24];  // TAG-11   회원처리항목1    : 주문번호(일자8자리 + 일련번호 10자리 + 전략번호 4자리 + 2자리 공백)
    char    smb_OrigClOrdID     [ 24];  // TAG-41   회원처리항목2    : 원주문번호
    char    smb_OrdID           [ 30];  // TAG-37   주문번호         : SMB로부터 수신받은 주문번호
    char    smb_OrdStatusReqID  [ 30];  // TAG-790  주문상태요청자ID : 'FXREQSPOTSTATUS208123'
    char    smb_NoPartyIDs      [  3];  // TAG-453  상대ID수         : Always  1(Number of Parties)
    char    smb_PartyID         [ 10];  // TAG-448  상대ID           : Counterparty SMBS code. SMBS code is 3 digit code defined by SMBS)
    char    smb_PartyRole       [  2];  // TAG-452  상대역활코드     : Always 35((Liquidity provider). The Liquidity provider is used to include the meaning of the Counterparty.)
    char    smb_NoPartySubIDs   [  3];  // TAG-802  상대부ID수       : Always  1(Number of Sub Parties)
    char    smb_PartySubID      [ 50];  // TAG-523  상대부ID         : Counterparty BIC code
    char    smb_PartySubIDType  [  2];  // TAG-803  상대부ID유형     : Always 16(BIC code). BIC Code : Bank Identification Code - SWIFT managed
    char    smb_Currency        [  3];  // TAG-15   통화코드         : Primary Currency
    char    smb_OrderQty        [ 30];  // TAG-38   주문수량         : USD/KRW, CNH/KRW Currency는 1,000,000(min) OTHER Currency 100,000
    char    smb_CumQty          [ 30];  // TAG-14   누적체결수량     :
    char    smb_LastQty         [ 30];  // TAG-32   체결수량         :
    char    smb_LeavesQty       [ 30];  // TAG-151  주문잔여수량     :
    char    smb_ExecID          [ 30];  // TAG-17   채결ID           : SMB로부터 수신받은 체결번호
    char    smb_LastPx          [ 30];  // TAG-31   체결가격         :
    char    smb_Price           [ 30];  // TAG-44   주문가격         :
    char    smb_Side            [  1];  // TAG-54   매매구분         : '1'-BUY, '2'-SELL
    char    smb_OrdType         [  1];  // TAG-40   주문유형         : Always 2('1'-시장가, '2'-지정가, '3'-예약주문")
    char    smb_Symbol          [  7];  // TAG-55   상품코드         : [Primary Currency]/[Counter Currency]
    char    smb_CxlRejResponseTo[  1];  // TAG-434  취소거부요청구분 : Must be 1(OrderCancelRequest)
    char    smb_CxlRejReason    [  1];  // TAG-102  취소거부사유     : '0'-Too laste to Cancel
                                        //                           : '1'-Unknown order
                                        //                           : '3'-Order already in Pending Cancel or Pending Replace status
                                        //                           : '6'-Duplication ClOrdID
    char    smb_TimeInForce     [  1];  // TAG-59   체결조건         : '0'-For Day
                                        //                           : '1'-For Good Till Cancel
                                        //                           : '3'-For Immediate Or Cancel(IOC)
                                        //                           : '4'-For Fill or Kill(FOK)
                                        //                           : '6'-For Good Till Date(GTD)
    char    smb_Text            [200];  // TAG-58   Text             : 거부사유등의 내용.
    char    smb_TransactTime    [ 30];  // TAG-60   주문일시         : '20180425-12:05:14.256'
    char    smb_ValueDate       [  8];  // TAG-64   결제일자         : Specific date of trade settlement in YYYYMMDD format.
    char    smb_SettType        [  3];  // TAG-9063 결제유형         : Always SP (Spot). Tenor Code
    char    smb_AggressorIndicator[1];  // TAG-1057                  : 'Y'-Match aggressor, 'N'-Resting at match

  //JPM 추가 start
    char    jpm_FixingDate        [ 10];  // TAG-6203  // N Fixing date in ISO 8  yyyy-mm-dd
    char    jpm_LastSpotRate      [ 15];  // TAG-194   Spot execution price.
    char    jpm_LastForwardPoints [ 15];  // TAG-195  The forward points of the near leg for swaps. See Appendix B for all-in rate calculations.
  //JPM 추가 end
    char    smb_filler          [  5];  // Filler        45->5:JPM

    char    smb_EOF             [  1];  // '0x00'
} SMB_ST;                               //                           : '99(Other)'

typedef struct
{
    //int                 auto_run_flag[ACC_NO_CNT];  /* 0:not run, 1:run  => 전략안에서만 쓰는애..abtg안에 상태구분코드로 처리할수 있을거같음*/
    ABTG_SET        Abtg_01[ACC_NO_CNT]; //현선물차익거래전략정보 
} ABTG;  // --> 위치.. shm_memory.h 

typedef struct {
    char    sf_mrgn_stus_dstcd            [   1];   // 현선물차익거래상태구분코드
    char    proc_id                       [   5];   // 프로세스ID
    char    sf_mrgn_rule_id               [   5];   // 현선물차익거래룰ID
    char    exch_dstcd                    [   1];   // 거래소구분코드- S:SMB
    char    fx_fcm_acno                   [  20];   // FXFCM계좌번호
    char    fx_prdct_cd                   [   6];   // FX상품코드
    char    fx_ofpr_dstcd                 [   1];   // FX호가구분코드 - 2.지정가(만 사용)
    char    fx_base_ordr_unit_qty         [  10];   // FX기본주문단위수량
    char    fx_max_ordr_unit_qty          [  10];   // FX최대주문단위수량
    char    fcm_id                        [   3];   // FCMID
    char    fcm_acno                      [  20];   // FCM계좌번호
    char    items_cd                      [  13];   // 종목코드
    char    ofpr_dstcd                    [   1];   // 호가구분코드 - T.시장가(만 사용)
    char    ftrs_base_ordr_unit_qty       [  10];   // 선물기본주문단위수량
    char    ftrs_max_ordr_unit_qty        [  10];   // 선물최대주문단위수량
    char    mrgn_drct_dstcd               [   1];   // 차익방향구분코드 - 1.매수, 2.매도, 3.양방향
    char    ordr_prity_dstcd              [   1];   // 주문우선순위구분코드 - 1.동시, 2.선물
    char    basis_calc_dstcd              [   1];   // 베이시스계산구분코드 - 1.Swap, 2.이론스프레드(딜러수기입력)
    char    basis_calc_sprd               [  11];   // 베이시스계산스프레드
    char    ordr_intval_ms                [  10];   // 주문간격밀리초값
    char    ent_sprd_dstcd                [   1];   // 진입스프레드구분코드 - 1.절대
    char    lqdt_start_sprd_dstcd         [   1];   // 청산개시스프레드구분코드(1차청산, 청산은 옵션) - 1.절대 , 9.미사용
    char    lqdt_cmpl_sprd_dstcd          [   1];   // 강제청산스프레드구분코드(2차청산, 청산은 옵션) - 1.절대 , 9.미사용
    char    ent_sprd                      [   9];   // 진입스프레드값
    char    lqdt_sprd                     [   9];   // 청산스프레드값
    char    lqdt_start_sprd               [   9];   // 청산개시스프레드값(1차청산)
    char    lqdt_cmpl_sprd                [   9];   // 강제청산스프레드값(2차청산)
    char    ent_sched_hms                 [   6];   // 진입예정시각
    char    lqdt_start_sched_hms          [   6];   // 청산개시예정시각(1차청산)
    char    lqdt_cmpl_sched_hms           [   6];   // 강제청산예정시각(2차청산)
    char    last_lqdt_end_sched_hms       [   6];   // 최종종료예정시간(최종청산)
    char    ref_no                        [  15];   // 참조번호(1 구분자 + 8 주문일자 + 6 일런번호)
    char    pay_dt                        [   8];   // 선물결제일자 (Swap Point 조회용)
} ABTG_SET_MSG;
#define ABTG_SET_MSG_SZ sizeof(ABTG_SET_MSG)

typedef struct {
    char    sf_mrgn_stus_dstcd            [   1];   // 현선물차익거래상태구분코드
    char    proc_id                       [   5];   // 프로세스ID
    char    sf_mrgn_rule_id               [   5];   // 현선물차익거래룰ID
    char    exch_dstcd                    [   1];   // 거래소구분코드- S:SMB
    char    fx_fcm_acno                   [  20];   // FXFCM계좌번호
    char    fx_prdct_cd                   [   6];   // FX상품코드
    char    fx_ofpr_dstcd                 [   1];   // FX호가구분코드 - 2.지정가(만 사용)
    long    fx_base_ordr_unit_qty         		;   // FX기본주문단위수량
    long    fx_max_ordr_unit_qty          		;   // FX최대주문단위수량
    char    fcm_id                        [   3];   // FCMID
    char    fcm_acno                      [  20];   // FCM계좌번호
    char    items_cd                      [  13];   // 종목코드
    char    ofpr_dstcd                    [   1];   // 호가구분코드 - T.시장가(만 사용)
    long    ftrs_base_ordr_unit_qty       		;   // 선물기본주문단위수량
    long    ftrs_max_ordr_unit_qty        		;   // 선물최대주문단위수량
    char    mrgn_drct_dstcd               [   1];   // 차익방향구분코드 - 1.매수, 2.매도, 3.양방향
    char    ordr_prity_dstcd              [   1];   // 주문우선순위구분코드 - 1.동시, 2.선물
    char    basis_calc_dstcd              [   1];   // 베이시스계산구분코드 - 1.Swap, 2.이론스프레드(딜러수기입력)
    char    basis_calc_sprd               [  11];   // 베이시스계산스프레드
    int     ordr_intval_ms                      ;   // 주문간격밀리초값
    char    ent_sprd_dstcd                [   1];   // 진입스프레드구분코드 - 1.절대
    char    lqdt_start_sprd_dstcd         [   1];   // 청산개시스프레드구분코드(1차청산, 청산은 옵션) - 1.절대 , 9.미사용
    char    lqdt_cmpl_sprd_dstcd          [   1];   // 강제청산스프레드구분코드(2차청산, 청산은 옵션) - 1.절대 , 9.미사용
    double  ent_sprd                            ;   // 진입스프레드값
    double  lqdt_sprd                           ;   // 청산스프레드값
    double  lqdt_start_sprd                     ;   // 청산개시스프레드값(1차청산)
    double  lqdt_cmpl_sprd                      ;   // 강제청산스프레드값(2차청산)
    char    ent_sched_hms                 [   6];   // 진입예정시각
    char    lqdt_start_sched_hms          [   6];   // 청산개시예정시각(1차청산)
    char    lqdt_cmpl_sched_hms           [   6];   // 강제청산예정시각(2차청산)
    char    last_lqdt_end_sched_hms       [   6];   // 최종종료예정시간(최종청산)
    char    ref_no                        [  15];   // 참조번호(1 구분자 + 8 주문일자 + 6 일런번호)
    char    pay_dt                        [   8];   // 선물결제일자 (Swap Point 조회용)
} ABTG_SET;
#define ABTG_SET_SZ sizeof(ABTG_SET)

/* 현선물차익거래전략주문정보 Shm */
typedef struct
{
	char	ordr_dt          	[  8];  // 주문일자
	int 	ftrs_fin_ordr_no	     ;  // 선물최종주문번호
	int     fx_fin_ordr_no  	     ;  // 현물최종주문번호
	int     abtg_fin_grp_no 	     ;  // 전략최종그룹번호
} SHM_ORDR_INFO;

/*************************************************************************
    End of Program (strategy.h)
*************************************************************************/
#endif
