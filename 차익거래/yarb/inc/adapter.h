#ifndef __ADAPTER_H__
#define __ADAPTER_H__
#include "arb.h"

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
    char    smb_PartyRole       [  2];  // TAG-452  상대역활코드     : Always 35((Liquidity provider). The Liquidity provider is used to include themeaning of the Counterparty.)
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
    char    smb_Text            [140];  // TAG-58   Text             : 거부사유등의 내용.
    char    smb_BidQuoteId      [ 30];  // TAG- 299   QuoteId        : smbText200-> 140 으로 줄이고..  quoteId 30 추가
    char    smb_AskQuoteId      [ 30];  // TAG- 299   QuoteId        : smbText200-> 140 으로 줄이고..  quoteId 30 추가
	
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
} SMB_ORD;                               //                           : '99(Other)'


//---------------------------------------------------------------------------
// FTRS YSK Ver. KRX Format

typedef struct {
    // --------------------------------------------------------------------------------
    // YSK FEP Data Header 정보
    // --------------------------------------------------------------------------------
    char        sLength                 [ 4]; // length field를 제외항 단일 DateSize
    char        sSeqNo                  [ 8]; // data 전문별 일련번호
    char        sDate                   [ 8]; // 주문일자
    char        sTime                   [ 6]; // 주문시간
    char        sSndTp                  [ 2]; // 송신처+LINT구분: 72 (파생)
    char        sMediaType              [ 3]; // 매체(채널)구분 : 102 (정규장_영업사원), 612 (야간장_영업사원)
    char        sSystemType             [ 2]; // 시스템구분( 00:정상 01:비상 02:백업)
    char        sBpIp                   [15]; // BP IP
    char        sFiller1                [ 1]; // filler1
    char        sCustLoginId            [ 8]; // Customer Login ID(HTS ID)
    char        sLoanDate               [ 8]; // 대출일자
    char        sOnOffGb                [ 2]; // Online/Offline
    char        sOrderType              [ 2]; // 주문종류
    char        sFiller2                [ 9]; // filler2
    char        sRespTime               [ 9]; // 거래소 응답시간(HHMMSSsss)
    char        sRpCode                 [ 4]; // 거래소 응답코드
    char        sRecvTime               [ 9]; // Agent, 채널 데이터 수신 시간(HHMMSSsss)
} YSKMSG_HDR;
#define YSKMSG_HDR_SZ sizeof(YSKMSG_HDR)

typedef struct {
    //------------------------------------------------------------------------------
    //  유안타 주문 - New Order Signle, Order Replace Request, Order Cancel Request
    //-------------------------------------------------------------------------------
 
	char        DataSeq                     [11];   // 01.메시지 일련번호                 - 주문번호 넣어도 OK
    char        Transaction_Code            [11];   // 02.트랜잭션코드                    - 신규 TCHODR10001
    char        Megrp_no                    [2 ];   // 03.ME그룹번호(2025신규)            - 00
    char        Board_Id                    [2 ];   // 04.보드ID                          - G1-일반
    char        MembershipNo                [5 ];   // 05.거래소가 부여한 회원번호        - 00024 자기거래(상품)지점
    char        BranchNo                    [5 ];   // 06.회원이 거래소에 신고한 지점번호 - 09999 자기거래(상품)지점
    char        OrderNo                     [10];   // 07.주문ID                          - "%0*lld"
    char        OriginalOrderNo             [10];   // 08.원주문ID                        - SPACE
    char        ItemCode                    [12];   // 09.종목코드                        -
    char        TradeFlag                   [1 ];   // 10.매도매수구분코드                - 1.매도 2.매수 
    char        New_Modify_Cancel_gbn       [1 ];   // 11.정정취소구분코드                - 1.신규 2.정정 3.취소
    char        AccountNo                   [12];   // 12.계좌번호                        
    char        Order_Quantity              [10];   // 13.호가수량                        - "%0*lld"    
    char        Order_Price                 [11];   // 14.호가가격(Float)                 - %0*.2f  (가격없는 호가 유형 0)
    char        Order_Type                  [1 ];   // 15.호가유형코드 :15                - T:가격제한시장가 2:지정가
    char        Order_Condition             [1 ];   // 16.호가조건코드 :16                - 0:FAS 3:FAK(IOC) 4:FOK
    char        Min_Che_Cnt                 [10];   // 17.최소체결수량(2025신규)          - 0
    char        Mm_Order_Type_No            [1 ];   // 18.시장조성자호가구분코드          - 0: 일반 
    char        Treasury_Stock_Id           [5 ];   // 19.자사주신고서ID                  - 0:해당없음 (파생 0 사용)
    char        Treasury_Stock_Method       [1 ];   // 20.자사주매매방법코드              - 0:해당없음 (파생 0 사용)
    char        Ask_Type                    [2 ];   // 21.매도유형코드                    - 00:해당없음 (파생 00 사용)
    char        Credit_Type                 [2 ];   // 22.신용구분코드                    - 10:보통(일반) - 파생은 10 사용
												    //                                    - 정정취소호가는 SPACE								 
    char        Trust_Principal_Type        [2 ];   // 23.위탁자기구분코드                - 31 호가입력화원의 자기거래 
    char        Trust_Company_No            [5 ];   // 24.위탁사번호                      - Space (위탁사)
    char        Program_Trading_Type        [2 ];   // 25.PT구분코드                      - 00 일반 
    char        Substitute_AccNo            [12];   // 26.대용주권계좌번호                - Space 
    char        Account_Type                [2 ];   // 27.계좌구분코드                    - 41: 파생자기일반계좌 
    char        Account_Margin_Type         [2 ];   // 28.계좌증거금유형코드              - 11 : 사후증거금 일반
    char        Country_Code                [3 ];   // 29.국가코드                        - 410 : 한국
    char        Investor_Type               [4 ];   // 30.투자자구분코드                  - 1000 : 금융투자회사
    char        Foreign_Investor_Type       [2 ];   // 31.외국인투자자구분코드            - 00 외국인 아님
    char        Order_Media_Type            [1 ];   // 32.주문매체구분코드                - 9.기타 
    char        Order_Identi                [12];   // 33.주문자식별정보                  - IP - 일단 Space
    char        Mac_Addr                    [12];   // 34.MAC주소                         - MAC 주소 - Space
    char        Order_Date                  [8 ];   // 35.호가일자                        - YYYYMMDD * 파생야간은 RDS 파생종목정보 영업일자
    char        Member_Send_Time            [9 ];   // 36.회원사주문시각                  - Space 
    char        MembershipItem              [60];   // 37.회원사용영역                    - 사용자영역
                                                    // memcpy(mabuf.host,      "X ", sizeof(mabuf.host));    // X: FX 일반주문  Y: FX 자동주문
                                                    // memcpy(mabuf.trd_type,  "0 ", sizeof(mabuf.trd_type));// 0: 거래유형(IAS사용)
                                                    // memcpy(mabuf.stgy_cd,   "0 ", sizeof(mabuf.stgy_cd)); // 0: 일반주문     L: 야간시장 주문 (CME)
                                                    // memcpy(ykord.MembershipItem,  mabuf.host, sizeof(ykord.MembershipItem));
    char        Algo_Stgy_Type              [1 ];   // 38.알고리즘전략구분코드            - 1 일반호가 2 알고리즘호가: 정정 및 취소 Space
    char        Trdr_Id                     [6 ];   // 39.거래자ID                        - SPACE
    char        Ord_Grp_No                  [2 ];   // 40.호가그룹번호                    - SPACE
    char        Smp_Cd                      [1 ];   // 41.자전거래방지코드                - 0
    char        Ord_Cond_Prc                [11];   // 42.호가조건가격                    - 0
    char        Trd_Mkt_Choic_Tp_Cd         [1 ];   // 43.거래시장선택구분코드            - 1 해당있음(거래시장 선택)-파생
    char        Srt_Sell_Id                 [10];   // 44.공매도ID                        - SPACE 파생시장 호가의 경우 공매도 허용대상이 아니므로 공매도ID SPACE로 입력
} YSKMSG_ORD;
#define YSKMSG_ORD_SZ sizeof(YSKMSG_ORD)

static inline void set_str_lpad_space( char *dst, size_t len, char *src )
{
     memset(dst, ' ', len);
     if (!src) return;

     size_t n = strlen(src);
     if (n> len)  n = len;
     memcpy(dst, src , n);
}

static inline void set_str_rpad_space( char *dst, size_t len, char *src )
{
     memset(dst, ' ', len);
     if (!src) return;

     size_t n = strlen(src);
     if (n> len)  n = len;
     memcpy(dst+len-n, src, n);
}

static inline void set_num_rpad_zero( char *dst, size_t len, char *numtxt )
{
     memset(dst, '0', len);
     if (!numtxt) return;

     size_t n = strlen(numtxt);
     if (n> len)  n = len;
     memcpy(dst+len-n, numtxt, n);
}

static inline void set_code1( char *dst, char *code1, char default_c)
{
    *dst = (code1 && code1[0]) ? code1[0]: default_c;
}

// ------------------------------------------------------------
// adapter_send_leg_spot
//  기능 : SPOT 레그 주문 전송(데모: 즉시체결 가정)
//  IN   : qty_m, side(+1/-1), ordtype, px
//  OUT  : 0=OK
// ------------------------------------------------------------
int adapter_send_spot(int ord_type,int qty_m, int side, int tif_type, double px);

// ------------------------------------------------------------
// adapter_send_leg_fut
//  기능 : FUT 레그 주문 전송(데모: 즉시체결 가정)
//  IN   : qty_ct, side(+1/-1), ordtype, px
//  OUT  : 0=OK
// ------------------------------------------------------------
int adapter_send_fut(int ord_type,int qty_ct, int side, int tif_type, double px);

// ------------------------------------------------------------
// adapter_on_fill_event
//  기능 : 실제 체결 이벤트 훅(데모에서는 미사용)
//  IN   : leg, filled_qty, filled_px
//  OUT  : 0
// ------------------------------------------------------------
int adapter_on_fill_event(int leg, long filled_qty, double filled_px);

#endif
