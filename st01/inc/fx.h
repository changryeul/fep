#ifndef		__FX_H
#define		__FX_H

typedef struct {
	char        excode          [ 1];       // 'J'P/'N'H/E'BS/'C'MB/'B'EST/'Z'CUST
	char        bidex           [ 1];       // BID원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB, 'H':HAND
	char        offerex         [ 1];       // ASK원천 : 'S':SMB, 'K':KMB, 'E':EBS, 'C':CMB, 'H':HAND
	char        symb            [ 7];       // root symbol
	char        date            [ 8];       // 수신일자 YYYYMMDD (서버시간)
	char        time            [ 9];       // 수신시간 HHMMSSSSS
	double      usdbid          ;           // Current USDKRW BID
	double      usdoffer        ;           // Current USDKRW OFFER
	double      bidprc          ;           // Price of the MarketData Entry
	double      offerprc        ;           // Price of the MarketData Entry
	double      bidqty          ;           // Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
	double      offerqty        ;           // Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW
	double      bidbest         ;           // NotUse : Only USD/KRW, CNH/KRW. Not tradeable price, reference only
	double      offerbest       ;           // NotUse : Only USD/KRW, CNH/KRW. Not tradeable price, reference only
    char        bid_quote_id      [ 30];
    char        ask_quote_id      [ 30];
}	CO_B6FX;

//------------------------------------------------------------------------------
//  SMB와 주문, 주문확인 및 체결을 처리하는 Structure
//  /user/fxa/win/src/inc/glb/ordfld.h SMB_ST
//-------------------------------------------------------------------------------
typedef struct {
	char    smb_MsgType         [  1];  // TAG-35   메세지유형       : 'D'-신규, 'F'-취소, '3'-거부, '8'-주문확인 및 >체결"
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
	char    smb_ClOrdID         [ 24];  // TAG-11   회원처리항목1    : 주문번호
										// array index 18 ~ 21		 : 전략룰번호
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
										//                           : '99(Other)'
	char    smb_TimeInForce     [  1];  // TAG-59   체결조건         : '0'-For Day
										//                           : '1'-For Good Till Cancel
										//                           : '3'-For Immediate Or Cancel(IOC)
										//                           : '4'-For Fill or Kill(FOK)
										//                           : '6'-For Good Till Date(GTD)
	char    smb_Text            [140];  // TAG-58   Text             : 거부사유등의 내용.
	char    smb_BidQuoteId      [ 30];  // TAG-117  QuoteId          ;  by lcr smb_Text 200-> 160 , QuoteId 40 add
	char    smb_AskQuoteId      [ 30];  // TAG-117  QuoteId          ;  by lcr smb_Text 200-> 160 , QuoteId 40 add
	char    smb_TransactTime    [ 30];  // TAG-60   주문일시         : '20180425-12:05:14.256'
	char    smb_ValueDate       [  8];  // TAG-64   결제일자         : Specific date of trade settlement in YYYYMMDD format.
	char    smb_SettType        [  3];  // TAG-9063 결제유형         : Always SP (Spot). Tenor Code
	char    smb_AggressorIndicator[1];  // TAG-1057                  : 'Y'-Match aggressor, 'N'-Resting at match
	//JPM 추가 start
	char  jpm_FixingDate        [ 10];  // TAG-6203  // N Fixing date in ISO 8  yyyy-mm-dd
	char    jpm_LastSpotRate        [ 15];  // TAG-194   Spot execution price.
	char    jpm_LastForwardPoints   [ 15];  // TAG-195  The forward points of the near leg for swaps. See Appendix B for all-in rate calculations.
	//JPM 추가 end
	char    smb_filler          [  5];  // Filler        45->5:JPM
	char    smb_EOF             [  1];  // '0x00'
} SMB_ST;

#endif
