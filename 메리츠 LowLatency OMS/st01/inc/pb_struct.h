#ifndef		__PB_STRUCT_H
#define		__PB_STRUCT_H
/*------------------------------------------------------------------------
#   System  : HATS
#	Module	: structures - 선물옵션 주문체결
#	File	: pb_struct.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
/* 선물옵션 주문/체결/장운영 header (40 bytes)	*/
typedef struct {
	char	Length[4];							/* 전송길이				*/
	char	NodeId[6];							/* 접속 ID				*/
	char	IfTrCode[4];						/* TR code				*/
	char	Gubun[1];							/* 구분					*/
	char	SeqNo[9];							/* 일련번호				*/
	char	DataCount[2];						/* 처리건수				*/
	char	ErrCode[3];							/* 거부코드				*/
	char	Time[8];							/* 처리시각 (HHMMSSss)	*/
	char	Filler[3];							/* 예비					*/
}	FOT_HEADER;

/* 선물옵션 주문내역/체결내역 재송요구 (40 bytes)	*/
typedef struct {
	char	Length[4];				/* 전송길이							*/
	char	NodeId[6];				/* 접속 ID							*/
	char	IfTrCode[4];			/* TR code (AW54/AW55)				*/
	char	Gubun[1];				/* 구분								*/
	char	OrigNodeId[6];			/* 원주문 접속 ID					*/
	char	SeqFrom[9];				/* 주문일련번호/체결통지번호 from	*/
	char	SeqTo[9];				/* 주문일련번호/체결통지번호 to		*/
	char	Filler[1];				/* 예비								*/
}	FOT_AW_HEADER;

/* 선물옵션 주문 (158 bytes)	*/
typedef struct {
	char	DataSeq[9];								/* data 일련번호	*/
	char	Data[149];								/* data				*/
}	FOT_JUMUN_DATA;

typedef struct {
    char    Sequence[9];					/* 주문일련번호					*/
	char	ItemCode[8];					/* 종목코드						*/
	char	BranchCode[3];					/* 증권사지점코드				*/
	char    OrderNumber[7];                 /* 주문번호						*/
	char    PriceFlag[2];                   /* 호가구분                 	*/
	char    TradeFlag[1];                   /* 매도매수구분					*/
	char    OrderType[1];                   /* 호가유형						*/
	char	OrderQuantity[8];				/* 호가주문수량					*/
	char	PriceSign[1];					/* 호가가격부호					*/
	char	Price[9];						/* 호가가격						*/
	char	TrustSelfGubun[1];				/* 위탁자기구분					*/
	char    MktAsstBidGubun[9];             /* 시장조성호가구분         	*/
	char	NationCode[2];					/* 국가코드						*/
	char	InvestorCode[2];				/* 투자자코드					*/
	char	OrderTrustMethod[1];			/* 호가수탁방법					*/
	char	TradeType[1];					/* 계좌구분(거래유형)			*/
	char	OriginalAcceptNo[7];			/* 원주문번호 					*/
	char	AccountNo[9];					/* 계좌번호						*/
	char	MembershipItem[30];				/* 회원처리항목					*/
	char	DylinkAccountNo[12];			/* 주식연결계좌번호 			*/
	char	JumunjaApinfo[12];				/* 주문자식별정보				*/
	char	JumunFlag[1];					/* 주문조건						*/
	char	JumunGubun[1];					/* 주문구분						*/
	char	PromiseOtherComNo[3];			/* 협의대량거래래 상대회원번호 	*/
	char	PromiseOtherAccNo[9];			/* 협의대량거래래 상대계좌번호 	*/
	char	PromiseTime[4];					/* 협의대량거래래 협의완료시각 	*/
	char	Filler[5];						/* FILLER						*/
}	FOT_JUMUN_DATA_ST;

typedef struct {
	FOT_HEADER		Header;
	FOT_JUMUN_DATA	JumunData[6];
}	FOT_JUMUN_FMT;

/* 선물옵션 체결 (170 bytes)	*/
typedef struct {
	char	TrCode[4];									/* TR code		*/
	char	Gubun[1];									/* 연속구분		*/
	char	DataSeq[9];									/* 체결통지번호	*/
	char	Data[156];									/* data			*/
}	FOT_SETTLE_DATA;

/* 체결내역 (170 bytes)	*/
typedef struct {
	char	TrCode[4];						/* Tr Code					*/
	char	Serial[1];						/* 연속구분					*/
	char	ConclusionNo[9];				/* 체결통지번호				*/
	char	ItemCode[8];					/* 종목코드					*/
	char	BranchNo[3];					/* 지점번호					*/
	char	OrderNo[7];						/* 주문번호    				*/
	char	MembershipNo[3];				/* 회원번호					*/
	char	ConsNo[9];						/* 약정번호					*/
	char	TradeFlag[1];					/* 매도매수구분				*/
	char	OrderType[1];					/* 주문유형					*/
	char	ConsSign[1];					/* 약정가격부호				*/
	char	ConsPrice[9];					/* 약정가격					*/
	char	ConsQuantity[8];				/* 약정수량					*/
	char	ConsTime[8];					/* 약정시각					*/
	char	ConsPrice1[9];					/* 최근월물 약정가격		*/
	char	ConsPrice2[9];					/* 차근월물 약정가격		*/
	char	AccountNo[9];					/* 계좌번호					*/
	char	MembershipItem[30];				/* 회원처리항목				*/
	char	Filler[41];						/* FILLER					*/
}	FOT_SETTLE_DATA_ST;

/* 정상주문거부, 정정취소확인/거부 (170 bytes)	*/
typedef struct {
	char	TrCode[4];						/* Tr Code					*/
	char	Serial[1];						/* 연속구분					*/
	char	ConclusionNo[9];				/* 체결통지번호				*/
	char	ItemCode[8];					/* 종목코드					*/
	char	BranchNo[3];					/* 지점번호					*/
	char	OrderNo[7];						/* 주문번호					*/
	char	MembershipNo[3];				/* 회원번호					*/
	char	OriginalOrderNo[7];				/* 원주문번호				*/
	char	PriceFlag[2];					/* 호가구분					*/
	char	TradeFlag[1];					/* 매도매수구분				*/
	char	OrderType[1];					/* 주문유형					*/
	char	PriceSign[1];					/* 호가가격부호				*/
	char	Price[9];						/* 호가가격					*/
	char	PriceSign1[1];					/* 정정/취소가격부호		*/
	char	Price1[9];						/* 정정/취소가격			*/
	char	OrderQuantity[8];				/* 호가수량					*/
	char	OrderQuantity1[8];				/* 실정정/취소수량			*/
	char	AcceptTime[8];					/* 접수시각					*/
	char	DisqualifyReason[3];			/* 사유코드					*/
	char	AccountNo[9];					/* 계좌번호					*/
	char	MembershipItem[30];				/* 회원처리항목				*/
	char	AcceptTime1[8];					/* 정정/취소확인시각		*/
	char	Filler[30];						/* FILLER					*/
}	FOT_SETTLE_RESP_ST;

typedef struct {
	FOT_HEADER			Header;
	FOT_SETTLE_DATA		SettleData;
}	FOT_SETTLE_FMT;

/*----------------------------------------------------------------------*/
/* 선물옵션 주문, 주문내역재전송요구응답 (158 bytes)					*/
/*----------------------------------------------------------------------*/
typedef struct {
	char	DataSeq[9];									/* 주문일련번호	*/
	char	JongMokNo[8];								/* 종목코드		*/
	char	BranchNo[3];								/* 지점번호		*/
	char	OrderNo[7];									/* 주문번호		*/
	char	Data[131];
}	FOT_ORDER_DAT;

/*----------------------------------------------------------------------*/
/* 선물옵션 체결 (170 bytes)											*/
/*	CH01:체결  HO01:정상주문거부/정정취소확인/거부						*/
/*	CH02:체결내역재전송  HO02:정상주문거부/정정취소확인/거부 재전송		*/
/*----------------------------------------------------------------------*/
typedef struct {
	char	TrCode[4];										/* TR code	*/
	char	Gubun[1];										/* 연속구분	*/
	char	Data1[17];
	char	BranchNo[3];									/* 지점번호	*/
	char	OrderNo[7];										/* 주문번호	*/
	char	Data2[138];
}	FOT_SETTLE_DAT;

typedef struct {
	FOT_HEADER	Header;
	union {
		FOT_ORDER_DAT	OrderData;
		FOT_SETTLE_DAT	SettleData;
	}	d;
}	FOT_DIVIDE_FMT;

/* 선물옵션 장운영 (170 bytes)	*/
typedef struct {
	char	TrCode[4];										/* TR code	*/
	char	Gubun[1];										/* 연속구분	*/
	char	Data[165];
}	FOT_JANG_DATA;

typedef struct {
	FOT_HEADER		Header;
	FOT_JANG_DATA	JangData;
}	FOT_JANG_FMT;

/* 선물옵션 청산결제 header (40 bytes)	*/
typedef struct {
	char	Length[4];									/* 전송길이		*/
	char	NodeId[6];									/* 접속ID		*/
	char	IfTrCode[4];								/* TR code		*/
	char	Gubun[1];									/* 구분			*/
	char	SeqNo[9];									/* 일련번호		*/
	char	ErrCode[3];									/* 거부코드		*/
	char	Filler[13];									/* 예비			*/
}	GYU_HEADER;

/* 선물옵션 청산결제 (360 bytes)	*/
typedef struct {
	char	TrCode[4];										/* TR code	*/
	char	Data[356];
}	FOT_GYULJE_DATA;

typedef struct {
	GYU_HEADER			Header;
	FOT_GYULJE_DATA		GyuljeData;
}	FOT_GYULJE_FMT;

/* 선물옵션 감리자료요청 (700 bytes)	*/
typedef struct {
	char	Length[4];								/* 전송길이			*/
	char	TrCode[4];								/* TR code			*/
	char	Company[3];								/* 회원번호			*/
	char	Date[8];								/* 일자	(YYYYMMDD)	*/
	char	Time[8];								/* 시각 (HHMMSSss)	*/
	char	Filler[13];								/* 예비				*/
	char	Data[660];								/* data				*/
}	FOT_GYEJA_RECV_FMT;

/* 선물옵션 감리자료응답 (700 bytes)	*/
typedef struct {
	char	Length[4];								/* 전송길이			*/
	char	TrCode[4];								/* TR code			*/
	char	Company[3];								/* 회원번호			*/
	char	Date[8];								/* 일자	(YYYYMMDD)	*/
	char	Time[8];								/* 시각 (HHMMSSss)	*/
	char	Filler[13];								/* 예비				*/
	char	Data[660];								/* data				*/
}	FOT_GYEJA_SEND_FMT;

/*************************************************************************
	End of Program (pb_struct.h)
*************************************************************************/
#endif
