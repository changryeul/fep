#ifndef ORDER_H
#define	ORDER_H	1

/*
#include "/sw/fxwin/src/wfrx/inc/glb/ordfld.h"
*/

/* 
 * 구분코드 						서브코드
 * T001 - 주문/정정/취소 		
 * T002 - 체결						체결시세 - 0001=기준/상대,0002=USD/기준,0003=USD/상대
 * T003 - 주문/정정/취소 확인
 */
typedef struct _order_head_
{
	char	Code				[4  ];			/* 구분코드 */
	char	SubCode				[4  ];			/* 서브코드 */
	char	DataCount			[2  ];			/* 연속데이타 번호 1 ~ n - 단일데이타 = 0 */
	char	DataTotal			[2  ];			/* 전체 데이타 갯수 n - 단일데이타 = 0 */
	char	DataSize			[4  ];			/* 데이타길이 - 헤더제외 */
	char	Filler				[16 ];			/* filler - total size = 32 */
}	ORDER_HEAD;

/* -------------------------------------------------------- */
/*  E_MSG : 처리결과 정보                                   */
/* -------------------------------------------------------- */
typedef struct _E_MSG
{
    char    pname       [ 30];      /* 프로세스 name 16->30*/
    char    ltyp        [  2];      /* Language type    */
    char    mtyp        [  1];      /* Message type  S:FXON H:HOST   */
    char    rtyp        [  1];      /* Error Code type  M:메시지 그외:code */
    char    code        [ 12];      /* Error Code 10-->12      */
    char    mesg        [256];      /* Error Message    */
}	MAT_E_MSG;
#define MATE_MSG_SZ    (sizeof(E_MSG))


/**********************************************************************************/
/* 수수료 계산 I/F                                                                */
/**********************************************************************************/
typedef struct _splt_in_st_
{
	char	s_csac_idnt_no		[ 30+1];		/* 고객번호                       */  
	char	s_csac_orgn_gb		[  1+1];		/* 원천구분                       */
	char	s_pair_id			[  7+1];		/* 통화페어                       */
	char	s_sett_type			[  3+1];		/* FX상품구분코드                 */
												/* : SPT,FWD */
	char	s_tnr_ptrn_dcd      [  1+1];		/* 테너유형구분코드(S:표준,U:비표준) */
	char	s_tnr_id			[  3+1];		/* 테너ID                         */
	char	s_bysel_dcd			[  1+1];		/* 매입매도구분코드               */
												/* (1-BUY(Sell&Buy), 2-SELL(Buy&Sell)) */
	char	s_expi_fnsh_ymd		[  8+1];		/* 만기종료년월일                 */
	char	s_expi_sttg_ymd		[  8+1];		/* 만기시작년월일                 */
	char	s_ordn_prc_cncd		[  1+1];		/* 주문가격조건코드 */
												/* (1-시장가,2-지정가,3-예약주문,4-MAR) */
	double	d_fx_ordn_prc;						/* 주문가격                       */
	double	d_bid_usd_prc;						/* BID USDKRW 가격                */
	double	d_ask_usd_prc;						/* ASK USDKRW 가격                */
	double	d_bid_std_prc;						/* BID 비재정 가격                */
	double	d_ask_std_prc;						/* ASK 비재정 가격                */
	double	d_bid_fnl_prc;						/* BID 재정 가격                  */
	double	d_ask_fnl_prc;						/* ASK 재정 가격                  */
	double	d_fx_ordn_amt;						/* 주문수량                       */
	double	d_usd_amt;							/* USD 환산주문수량               */
}	SPLIT_IN_ST;                                   

typedef struct _prd_lst_ 
{
	int		n_trhs_srn;							/* 거래내역일련번호               */ 
												/* 재정통화Pair(EURKRW,JPYKRW,..) */ 
												/* 선물환, 현물환                 */
												/* 재정통화Pair:0, 비재정통화Pair: 1, USDKRW: 2 */
												/* USDKRW 현물환, 선물환, 비재정통화Pair(EURUSD, USDJPY, ...   )  */
												/* 현물환 -> 0 하나의 정보만 리턴 */
	char	s_cncr_pair_id		[  7+1];		/* 통화페어ID                     */
	char	s_sett_type			[  3+1];		/* 거래유형                       */
	char	s_tnr_ptrn_dcd      [  1+1];		/* 테너유형구분코드(S:표준,U:비표준) */
	char	s_tnr_id			[  3+1];		/* 테너ID                         */
	char	s_expi_fnsh_ymd		[  8+1];		/* 만기종료년월일                 */
	char	s_bysel_dcd			[  1+1];		/* 매입매도구분코드               */
	double	d_mrkt_spt_prc;						/* 시장SPOT가격                   */
	double	d_mrkt_swap_prc;					/* 시장SWAP가격         (SWAP포인 트) */
	double	d_fx_mrkt_prc;						/* FX시장가격                     */
	double	d_cvr_spr;							/* cover dealer 스프레드(cover 마 진)     */
	double	d_fx_cvr_prc;						/* cover dealer 가격    (cover 마 진가격) */
	double	d_sls_spr;							/* corp dealer 스프레드 (corp 마진)       */
	double	d_orcy_spr;							/* 당사스프레드 (본점마진)        */
	double	d_fx_orcy_prc;						/* FX당사가격   (본점가격)        */
	double	d_cus_spr;							/* 고객스프레드 (영업점마진)      */
	double	d_fx_csac_prc;						/* FX고객가격                     */
}	SPLIT_PRD_LIST;                              

typedef struct _split_put_st_
{
	char	s_pair_id			[  7+1];		/* 통화페어                       */ 
	int		n_rec_cnt;							/* 레코드 건수 1, 3               */
	SPLIT_PRD_LIST	rec			[    3];
} SPLIT_OUT_ST;
/**********************************************************************************/

/**********************************************************************************/
/* 주문 FORM                                                                      */
/**********************************************************************************/
typedef struct _order_recv_
{
	char	MsgType				[1  ];			/* 주문타입                       */
												/* D-신규,G-정정,F-취소           */
	char	CustID				[30 ];			/* 고객번호 (내부사용자ID)        */
	char	FundNO				[12 ];			/* 펀드 번호                      */
	char	OrgnGb				[1  ];			/* 원천구분                       */
												/* (1-고객거래,2-내부거래,3-대행거래) */
	char	PuCd				[5  ];			/* PU 코드                        */
	char	BkNo				[20 ];			/* 북번호                         */
	char	ClOrdID				[11 ];			/* 주문번호                       */
	char	OrigClOrdID			[11 ];			/* 원주문번호                     */
	char	Currency			[3  ];			/* 기준통화코드(정보성)           */
	char	OrderQty			[23 ];			/* 주문수량                       */
	char	OrdType				[1  ];			/* 주문유형 1:시장,2:지정,3:예약  */
												/* (1-시장가,2-지정가,3-예약주문) */
	char	Price				[20 ];			/* 주문가격                       */
												/* (SWAP인 경우는 SWAP_RATE)      */
	char	PriceSpr			[20 ];			/* 고객스프레드                   */
	char	SlipCmpPrice		[20 ];			/* 주문시점가격(시장가의 경우)    */
	char	SlipPip				[20 ];			/* 슬립피지허용가격(시장가만 의미있음) */
	char	Side				[1  ];			/* 매매구분                       */
												/* (1-BUY(Sell&Buy), 2-SELL(Buy&Sell)) */
	char	TimeInForce			[1  ];			/* 주문유효시간                   */
												/* 0-For Day                      */
												/* 1-For Good Till Cancel         */
												/* 3-For Immediate Or Cancel(IOC) */
												/* 4-For Fill or Kill(FOK)        */
												/* 6-For Good Till Date(GTD)      */
	char	TransactTime		[20 ];			/* 처리시각                       */
	char	SettType			[3  ];			/* FX상품구분코드                 */
												/* : SPT,FWD,SWP */
												/* MAR일경우 시장가로 체결 */
	char	TnrId				[3  ];			/* 테너 ID                        */
												/* TOD,TOM.SPT,S/N,W01,M01,M02,M03,M06,M09,Y01 */
	char	TnrPtrnDcd			[1  ];			/* 테너유형구분코드               */
												/* S:표준,U:비표준,F:만기선택선물환 */
	char	Symbol				[7  ];			/* FX상품코드     : USD/KRW       */
	char	ValueDate1			[8  ];			/* 결제시작일자                   */
	char	ValueDate2			[8  ];			/* 결제종료일자                   */

	char	NearSettType		[3  ];			/* 근일물상품구분코드             */
												/* : 1-TOD,2-TOM, 3-SP(현물환),4-FWD,5-SWAP,8-MAR */
	char	NearTnrId			[3  ];			/* 테너ID                         */
												/* TOD,TOM.SPT,S/N,W01,M01,M02,M03,M06,M09,Y01 */
	char	NearTnrPtrnDcd		[1  ];			/* 테너유형구분코드               */
												/* S:표준,U:비표준,F:만기선택선물환 */
	char	NearLegSide			[1  ];			/* NEAR매매구분(1-Buy,2-Sell)     */
	char	NearLegSettlDate	[8  ];			/* NEAR-결제일자                  */
	char	NearLegPrice		[20 ];			/* FWD는 FWD환율(고객가격)        */
	char	NearLegPriceSprd	[20 ];			/* FWD는 FWD환율 스프레드         */

	char	FarSettType			[3  ];			/* 원일물상품구분코드             */
												/* : 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SW,6-바로   */
	char	FarTnrId			[3  ];			/* 테너ID                         */
												/* TOD,TOM.SPT,S/N,W01,M01,M02,M03,M06,M09,Y01 */
	char	FarTnrPtrnDcd		[1  ];			/* 테너유형구분코드               */
												/* S:표준,U:비표준,F:만기선택선물환 */
	char	FarLegSide			[1  ];			/* FAR매매구분(1-Buy, 2-Sell)     */
	char	FarLegSettlDate		[8  ];			/* FAR-결제일자                   */
	char	FarLegPrice			[20 ];			/* FWD는 FWD환율(고객가격)        */
	char	FarLegPriceSprd		[20 ];			/* FWD는 FWD환율 스프레드         */

	char	TranPtrnCd			[1  ];			/* 거래유형코드                   */
												/* (1-일반,2-MAR,3-RFQ)           */
												/* 1-일반,2-MAR,3-RFQ,4-RFS,5-예약(가격예약),6-예약(기간예약),7-일괄거래,9-재헷지 */
	char	GrpOrdnNo			[11 ];			/* 그룹주문번호                   */
	char	GrpOrdnCnt			[10 ];			/* 그룹주문건수                   */
	char	GrpOrdnSeq			[10 ];			/* 그룹주문순번                   */

	char	GrpFxPdcd			[3  ];			/* 그룹주문 상품코드 SPT,FWD      */
	char	TrdTypeDcd			[1  ];			/* 주문 타입                      */
												/* 1-일반,2-SPOT고정,3-체결가격상세 생성안함(대행및RFQ) */
	char	SpotPrc				[20 ];			/* TrdTypeDcd=2일경우 스팟가격 */
	char	SpotPrcSpr			[20 ];			/* TrdTypeDcd=2일경우 스팟스프레드 */
	char	UsdQty				[23 ];			/* 주문수량을 달러로 환산한 금액   */
	char	Filler				[574];			/* Filler                         */
	char	Eof					[1  ];			/* EOF 0x00                       */
}   ORDER_RECV;


/**********************************************************************************/
/* 매칭 주문  FORM                                                                */
/**********************************************************************************/
typedef struct _order_
{
	char	MsgType				[1	];			/* 메세지유형                     */
	char	OrdStatus			[1	];			/* 주문상태                       */
												/* 0=New,1=Partially filled,2=Filled,4=Canceled,8=Rejected */
	char	CustID				[30 ];			/* 고객번호 (내부사용자ID)        */
	char	FundNO				[12 ];			/* 펀드 번호                      */
	char	ExecType			[1	];			/* 거래유형 @                     */
	char	OrgnGb				[1  ];			/* 원천구분                       */ /* internal */
												/* (1-고객거래,2-내부거래)        */
	char	LgenNo				[30	];			/* 시장참여자ID및트레이더번호     */
	char	BkNo				[20	];			/* 북번호                         */
	char	FxBkId				[20	];			/* 북번호                         */
	char	ClOrdID				[11	];			/* 회원처리항목1                  */
	char	OrigClOrdID			[11	];			/* 회원처리항목2                  */
	char	Currency			[3	];			/* 통화코드                       */
	char	OrdType				[1  ];			/* 주문유형                       */ /* internal */
												/* (1-시장가,2-지정가,3-예약주문) */
	double	SlipCmpPrice			;			/* 주문시점가격(시장가)           */ /* internal */
	double	SlipPip					;			/* Slipage Pip                    */ /* (가격이격:체결범위) */ /* internal */
	double	OrderQty				;			/* 주문수량                       */
	double	CumQty					;			/* 누적체결수량                   */
	double	LastQty					;			/* 체결수량                       */
	double	LeavesQty				;			/* 주문잔여수량                   */
	char	OrdID				[30	];			/* 주문번호ID                     */
	char	ExecID				[30	];			/* 체결ID                         */
	double	Price					;			/* 주문가격                       */
	double	PriceSpr			    ;			/* 고객스프레드                   */
	double	OrdSprPrc				;			/* 주문고객마진 안씀(?:20240108)  */
	double	LastPx					;			/* 체결가격                       */
	double	LastSprPx				;			/* 체결고객마진                   */
	char	Side				[1	];			/* 매매구분                       */
	char	TimeInForce			[1	];			/* 체결조건                       */
	char	RefuslCd			[10	];			/* 거부코드                       */
	char	Text				[200];			/* 내용                           */
	char	TransactTime		[20	];			/* 주문일시                       */
	char	SettType			[3	];			/* FX상품구분코드                 */
												/* : SPT,FWD,SWP */
	char	TnrId				[3  ];			/* 테너 ID                        */
												/* TOD,TOM.SPT,S/N,W01,M01,M02,M03,M06,M09,Y01 */
	char	TnrPtrnDcd			[1  ];			/* 테너유형구분코드               */
												/* S:표준,U:비표준,F:만기선택선물환 */
	char	Symbol				[7	];			/* FX상품코드     : USD/KRW       */
	char	ValueDate1			[8	];			/* 결제시작일자                   */
	char	ValueDate2			[8	];			/* 결제종료일자                   */
	char	OfprKeyVal			[60	];			/* 거래호가번호                   */
												/* (호가KEY(체결호가KEY))         */
	char	NearSettType		[3	];			/* NEAR-레크상품구분코드          */
												/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char	NearTnrId			[3  ];			/* 테너 ID                        */
	char	NearTnrPtrnDcd		[1  ];			/* 테너유형구분코드               */
	char	NearLegSide			[1	];			/* NEAR-레그매수매도구분코드      */
	char	NearLegSettlDate	[8	];			/* NEAR-레그결제년월일            */
	double	NearLegPrice			;			/* FWD는 FWD환율(고객가격)        */
	double	NearLegPriceSprd		;			/* FWD는 FWD환율 스프레드         */
	double	NearLegMktPrc			;			/* NEAR-레그체결가격              */
	double	NearLegCvPrc			;			/* NEAR-CV스프레드                */
	double	NearLegCoPrc			;			/* NEAR-CO스프레드                */
	double	NearLegCusPrc			;			/* NEAR-레그체결가격스프레드      */
	double	NearLegCusSpr			;			/* NEAR-Cu prc (spread)           */

	char	FarSettType			[3	];			/* FAR-레크상품구분코드           */
												/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char	FarTnrId			[3  ];			/* 테너 ID                        */
	char	FarTnrPtrnDcd		[1  ];			/* 테너유형구분코드               */
	char	FarLegSide			[1	];			/* FAR-레그매수매도구분코드       */
	char	FarLegSettlDate		[8	];			/* FAR-레그결제년월일             */
	double	FarLegPrice				;			/* FWD는 FWD환율(고객가격)        */
	double	FarLegPriceSprd			;			/* FWD는 FWD환율 스프레드         */
	double	FarLegMktPrc			;			/* FAR-레그체결가격               */
	double	FarLegCvPrc				;			/* FAR-CV스프레드                 */
	double	FarLegCoPrc				;			/* FAR-CO스프레드                 */
	double	FarLegCusPrc			;			/* FAR-레그체결가격스프레드       */
	double	FarLegCusSpr			;			/* FAR-Cu prc (spread)            */

	char	TranPtrnCd			[1  ];			/* 거래유형코드                   */
												/* (1-일반,2-MAR,3-RFQ)           */
	char	GrpOrdnNo			[11 ];			/* 그룹주문번호                   */
	char	GrpOrdnCnt			[10 ];			/* 그룹주문건수                   */
	char	GrpOrdnSeq			[10 ];			/* 그룹주문순번                   */

	char	GrpFxPdcd			[3  ];			/* 그룹주문 상품코드 SPT,FWD      */
	char	TrdTypeDcd			[1  ];			/* 주문 타입                      */
												/* 1-일반,2-SPOT고정,3-체결가격상세 생성안함(대행및RFQ) */
	double	SpotPrc				     ;			/* TrdTypeDcd=2일경우 스팟가격    */
	double	SpotPrcSpr			     ;			/* TrdTypeDcd=2일경우 스팟가격    */

	double	UsdQty				     ;			/* 주문수량을 달러로 환산한 금액   */
	char	filler				[589];			/* Filler                         */
	char	Eof					[1	];			/* 0x00                           */
}	ORDER;

/**********************************************************************************/
/* 주문확인/체결  FORM                                                            */
/**********************************************************************************/
typedef struct _order_send_
{
	char	MsgType				[1	];			/* 메세지유형      '3'-거부(주문거부), '8'-주문확인 및 체결(체결 거부)  */
	char	OrdStatus			[1	];			/* 주문상태                       */
												/* 0:New, 1:Partially filled, 2:Filled, 4:Canceled, 8:Rejected */
												/* 5:정정 20240104 */
	char	ExecType			[1	];			/* 거래유형                       */
												/* 0:New, 1:Partially Filled, 2:Filled, 4:Canceled, 5:정정, */ 
												/* 6:취소대기, 8:거절, C:주문만료(Expired), E:정정대기, F:Filled */
	char	LgenNo				[16	];			/* 시장참여자ID및트레이더번호     */
	char	BkNo				[5	];			/* 북번호                         */
	char	FxBkId				[20	];			/* 북번호                         */
	char	ClOrdID				[11	];			/* 회원처리항목1                  */
	char	OrigClOrdID			[11	];			/* 회원처리항목2                  */
	char	Currency			[3	];			/* 통화코드                       */
	char	OrderQty			[23	];			/* 주문수량                       */
	char	CumQty				[23	];			/* 누적체결수량                   */
	char	LastQty				[23	];			/* 체결수량                       */
	char	LeavesQty			[23	];			/* 주문잔여수량                   */
	char	OrdID				[30	];			/* 주문번호ID                     */
	char	ExecID				[30	];			/* 체결ID                         */
	char	Price				[20	];			/* 주문가격                       */
	char	PriceSpr			[20	];			/* 주문가격고객스프레드           */
	char	LastPx				[20	];			/* 체결가격                       */
	char	LastSprPx			[20	];			/* 체결가격고객스프레드           */
	char	Side				[1	];			/* 매매구분                       */
	char	TimeInForce			[1	];			/* 체결조건                       */
	char	RefuslCd			[10	];			/* 거부코드                       */
	char	Text				[200];			/* 내용                           */
	char	TransactTime		[20	];			/* 주문일시                       */
	char	SettType			[3	];			/* FX상품구분코드                 */
												/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SW, 8-마크업 */
	char	TnrId				[3  ];			/* 테너 ID                        */
	char	TnrPtrnDcd			[1  ];			/* 테너유형구분                   */
	char	Symbol				[7	];			/* FX상품코드     : USD/KRW       */
	char	ValueDate1			[8	];			/* 결제시작일자                   */
	char	ValueDate2			[8	];			/* 결제종료일자                   */
	char	OfprKeyVal			[60	];			/* 거래호가번호                   */
												/* (호가KEY(체결호가KEY))         */
	char	NearSettType		[3	];			/* NEAR-레크상품구분코드          */
												/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char	NearTnrId			[3  ];			/* 테너 ID                        */
	char	NearTnrPtrnDcd		[1  ];			/* 테너 유형구분                  */
	char	NearLegSide			[1	];			/* NEAR-레그매수매도구분코드      */
	char	NearLegSettlDate	[8	];			/* NEAR-레그결제년월일            */
	char	NearLegMktPrc		[20	];			/* NEAR-레그체결가격 SPOT+SWAP  - 원가 */
	char	NearLegCvPrc		[20	];			/* NEAR-CV 가격                   */
	char	NearLegCoPrc		[20	];			/* NEAR-CO 가격                   */
	char	NearLegCusPrc		[20	];			/* NEAR-CU 가격                 - 마진가격 */
	char	NearLegCusSpr		[20	];			/* NEAR-CU스프레드                */

	char	FarSettType			[3	];			/* FAR-레크상품구분코드           */
												/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char	FarTnrId			[3  ];			/* 테너 ID                        */
	char	FarTnrPtrnDcd		[1  ];			/* 테너 유형구분                  */
	char	FarLegSide			[1	];			/* FAR-레그매수매도구분코드       */
	char	FarLegSettlDate		[8	];			/* FAR-레그결제년월일             */
	char	FarLegMktPrc		[20	];			/* FAR-레그체결가격 SPOT+SWAP    */
	char	FarLegCvPrc			[20	];			/* FAR-CV 가격                   */
	char	FarLegCoPrc			[20	];			/* FAR-CO 가격                   */
	char	FarLegCusPrc		[20	];			/* FAR-CU 가격                   */
	char	FarLegCusSpr		[20	];			/* FAR-CU스프레드                */

	char	filler				[168];			/* Filler                         */
	char	Eof					[1	];			/* 0x00                           */
}	ORDER_SEND;

#endif	/* ORDER_H */
