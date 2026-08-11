typedef struct _order_recv_
{
	char MsgType			[1  ];			/* 주문타입                       */
											/* D-신규,G-정정,F-취소           */
	char CustID				[30 ];			/* 고객번호 (내부사용자ID)        */
	char OrgnGb				[1  ];			/* 원천구분                       */
											/* (1-고객거래,2-내부거래)        */
	char BkNo				[10 ];			/* 북번호                         */
	char ClOrdID			[24 ];			/* 주문번호                       */
	char OrigClOrdID		[24 ];			/* 원주문번호                     */
	char Currency			[3  ];			/* 기준통화코드(정보성)           */
	char OrderQty			[20 ];			/* 주문수량                       */
	char OrdType			[1  ];			/* 주문유형                       */
											/* (1-시장가,2-지정가,3-예약주문) */
	char Price				[20 ];			/* 주문가격                       */
											/* (SWAP인 경우는 SWAP_RATE)      */
	char SlipCmpPrice		[20 ];			/* 주문시점가격(시장가의 경우)    */
	char SlipPip			[20 ];			/* Slipage Pip(가격이격:체결범위) */
	char Side				[1  ];			/* 매매구분                       */
											/* (1-BUY(Sell &   Buy), 2-SELL(Buy & Sell)) */
	char TimeInForce		[1  ];			/* 주문유효시간                   */
											/* 0-For Day                      */
											/* 1-For Good Till Cancel         */
											/* 3-For Immediate Or Cancel(IOC) */
											/* 4-For Fill or Kill(FOK)        */
											/* 6-For Good Till Date(GTD)      */
	char TransactTime		[20 ];			/* 처리시각                       */
	char SettType			[1  ];			/* FX상품구분코드                 */
											/* : 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SWAP,8-MAR */
	char Symbol				[7  ];			/* FX상품코드     : USD/KRW       */
	char ValueDate1			[8  ];			/* 결제시작일자                   */
	char ValueDate2			[8  ];			/* 결제종료일자                   */
	char NearSettType		[1  ];			/* 근일물상품구분코드             */
											/* : 1-TOD,2-TOM, 3-SP(현물환),4-FWD,5-SWAP,8-MAR */
	char NearLegSide		[1  ];			/* NEAR매매구분(1-Buy,2-Sell)     */
	char NearLegSettlDate	[8  ];			/* NEAR-결제일자                  */
	char NearLegPrice		[20 ];			/* FWD는 FWD환율(고객가격)        */
	char NearLegPriceSprd	[20 ];			/* FWD는 FWD환율 스프레드         */
	char FarSettType		[1  ];			/* 원일물상품구분코드             */
											/* : 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SW,6-바로   */
	char FarLegSide			[1  ];			/* FAR매매구분(1-Buy, 2-Sell)     */
	char FarLegSettlDate	[8  ];			/* FAR-결제일자                   */
	char FarLegPrice		[20 ];			/* FWD는 FWD환율(고객가격)        */
	char FarLegPriceSprd	[20 ];			/* FWD는 FWD환율 스프레드         */
	char tran_ptrncd		[1  ];			/* 거래유형코드                   */
											/* (1-일반,2-MAR,3-RFQ)           */
	char Filler				[647];			/* Filler                         */
	char Eof				[1  ];			/* EOF                            */
}   ORDER_RECV;


typedef struct _order_send_
{
	char MsgType			[1	];			/* 메세지유형                     */
	char OrdStatus			[1	];			/* 주문상태                       */
	char ExecType			[1	];			/* 거래유형                       */
	char LgenNo				[30	];			/* 시장참여자ID및트레이더번호     */
	char BkNo				[10	];			/* 북번호                         */
	char ClOrdID			[24	];			/* 회원처리항목1                  */
	char OrigClOrdID		[24	];			/* 회원처리항목2                  */
	char Currency			[3	];			/* 통화코드                       */
	char OrderQty			[20	];			/* 주문수량                       */
	char CumQty				[20	];			/* 누적체결수량                   */
	char LastQty			[20	];			/* 체결수량                       */
	char LeavesQty			[20	];			/* 주문잔여수량                   */
	char OrdID				[30	];			/* 주문번호ID                     */
	char ExecID				[30	];			/* 체결ID                         */
	char Price				[20	];			/* 주문가격                       */
	char LastPx1			[20	];			/* 주문고객마진                   */
	char LastPx2			[20	];			/* 체결가격                       */
	char LastPx3			[20	];			/* 체결고객마진                   */
	char Side				[1	];			/* 매매구분                       */
	char TimeInForce		[1	];			/* 체결조건                       */
	char RefuslCd			[10	];			/* 거부코드                       */
	char Text				[200];			/* 내용                           */
	char TransactTime		[20	];			/* 주문일시                       */
	char SettType			[1	];			/* FX상품구분코드                 */
											/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SW, 8-마크업 */
	char Symbol				[7	];			/* FX상품코드     : USD/KRW       */
	char ValueDate1			[8	];			/* 결제시작일자                   */
	char ValueDate2			[8	];			/* 결제종료일자                   */
	char OfprKeyVal			[30	];			/* 거래호가번호                   */
											/* (호가KEY(체결호가KEY))         */
	char NearSettType		[1	];			/* NEAR-레크상품구분코드          */
											/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char NearLegSide		[1	];			/* NEAR-레그매수매도구분코드      */
	char NearLegSettlDate	[8	];			/* NEAR-레그결제년월일            */
	char NearLegPrice1		[20	];			/* NEAR-레그체결가격              */
	char NearLegPrice2		[20	];			/* NEAR-CV스프레드                */
	char NearLegPrice3		[20	];			/* NEAR-CO스프레드                */
	char NearLegPriceSprd	[20	];			/* NEAR-레그체결가격스프레드      */
	char FarSettType		[1	];			/* FAR-레크상품구분코드           */
											/* : 1-TOD, 2-TOM, 3-SP(현물환), 4-FWD, 5-SWAP */
	char FarLegSide			[1	];			/* FAR-레그매수매도구분코드       */
	char FarLegSettlDate	[8	];			/* FAR-레그결제년월일             */
	char FarLegPrice1		[20	];			/* FAR-레그체결가격               */
	char FarLegPrice2		[20	];			/* FAR-CV스프레드                 */
	char FarLegPrice3		[20	];			/* FAR-CO스프레드                 */
	char FarLegPriceSprd	[20	];			/* FAR-레그체결가격스프레드       */
	char filler				[1	];			/* Filler                         */
	char Eof				[1	];			/* 0x00                           */
}	ORDER_SEND;
