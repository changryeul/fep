#ifndef		__PA_STRUCT_H
#define		__PA_STRUCT_H
/*------------------------------------------------------------------------
#	Module	: structures - 선물옵션 주문체결
#	File	: pa_struct.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define KRX_HEAD_LEN					(sizeof (KRX_HEADER))	/* 82	*/
#define SEARCH_HEADER_LEN				(sizeof (SEARCH_HEADER))/* 50	*/

/* 선물옵션 주문/체결/장운영 header (82 bytes)  */
typedef struct {
	char    BeginString[8];         /* 전문유형(API프로토콜의버젼)      */
	char    BodyLength[6];          /* 메시지길이(Body전체길이)         */
	char    MsgType[11];            /* 메시지타입                       */
                                    /* 세션메시지타입 및 TR Code        */
                                    /* "S"+"CH"+Type(3)+일련번호(5)     */
	char    MsgSeqNum[11];          /* 일련번호                         */
                                    /* 주문요청 Body첫번째항목 일련번호 */
                                    /* 주문응답 채널 최종 처리 일련번호 */
	char    SenderCompID[5];        /* 회원번호                         */
	char    DeliverToCompID[10];    /* 연계시도착회원사번호             */
	char    OnBehalfOfCompID[10];   /* 회신시송신회원사번호             */
	char    SendingTime[17];        /* 전송일시(YYYYMMDDHHMMSSMS)       */
	char    DataCnt[3];             /* 데이터건수                       */
	char    Encrypt[1];             /* 암호화유무                       */
}   KRX_HEADER;

/* KRX 차세대 매매 - 주문요청 : 호가입력 (300->261 bytes)  */
typedef struct {
	char	DataSeq[11];					/* 메시지 일련번호          */
	char 	transaction_code                [11];	/* 장개시전 협의거래신규:TCHODR10001 	 */
																/* 장개시전 협의거래취소:TCHODR10003	 */
	char 	board_id                        [2 ];	/* 별첨_보드ID 매핑 참고 				 */
	char 	member_number                   [5 ];	/* 거래소가 부여한 회원번호				 */
																/* 정정/취소 호가인 경우 원호가와 동일한 */
																/* 값이 입력되어야 함(원호가 정합성 체크)*/
	char 	branch_number                   [5 ];	/* 회원이 거래소에 신고한 지점번호		 */
																/* 정정/취소 호가인 경우 원호가와 동일한 */
																/* 값이 입력되어야 함(원호가 정합성 체크)*/
	char 	order_identification            [10];	/* * 문자를 포함
																   * 종목+회원+지점+주문ID가 Key (Unique)
																   * * 주문ID 사용불가영역 :
																   *  9000000001 ~ 9999999999
																   *   (비상주문시 자동생성되는 주문ID영역으로 회원사에서 주문ID 사용하면 안됨) */
	char 	original_order_identification   [10];	/* * 신규호가는 SPACE로 입력			 */
	char 	issue_code                      [12];	/* 현물/파생/채권/REPO 통합상품의 종목 코드(ISIN종목코드) */
	char 	ask_bid_type_code               [1 ];	/* 1 매도
																   2 매수

																   * 정정/취소 호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크) */
	char 	modify_or_cancel_type_code      [1 ];	/* 1 신규
																   2 정정
																   3 취소 */
	char 	account_number                  [12];	/* * 12자리 구성방법 제한없음. 12자리를 회원사별로 사용

																   * 정정/취소 호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크) */
	char 	order_quantity                  [10];	/* 호가수량 */
	char 	order_price                     [11];	/* * 가격이 없는 호가유형인 경우 0으로 입력
																   * 시간외종가 호가의 경우 0으로 입력
																   * * 취소호가는 0으로 입력
																   * * 경쟁대량매매는 0으로 입력 */
	char 	order_type_code                 [1 ];	/* 1 시장가 (Market)
																   2 지정가 (Limit)
																   T 가격제한시장가
																   W 가격제한최유리지정가
																   I 조건부지정가 (Limit To Market)
																   V 경쟁대량 (현물만 해당, 값 변경 : 3->V)
																   X 최유리지정가
																   Y 최우선지정가 (현물만 해당)

																   * 취소호가는 SPACE로 입력
																   * * 코넥스 경매매는 2:지정가만 가능
																   * * 파생시장 1,X 불가 (T,W 으로 대체) */
	char 	order_condition_code            [1 ];	/* 0 일반 (FAS)
																   3 FAK(IOC)
																   4 FOK

																   * 취소호가는 SPACE로 입력
																   * * 코넥스 경매매는 0: 일반만 가능 */
	char 	mm_ord_tp_no                    [11];	/* 1) 현물
																       0 : 일반
																           1 : LP호가
															               2 : MM호가
														               2) 파생상품
														                   0 : 일반
													                       1~ : 시장조성자호가구분번호

												                       * 파생
												                         - 시장조성호가의 경우
												                           1부터 순차적으로 번호를 부여하고, 양방향호가의 경우 동일 번호를 부여하는 것이 원칙이나 해당 처리가 가능한 시스템을 구비하지 못한 회원사의 경우  시장조성호가에 대해 번호 증가없이 1로만 입력할 수 있음
												                       
												                        * 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
																          --> 파생의 경우 원호가가 0 아닌 값인 경우, 정정호가도 0 아닌 값이어야 하고 원호가가 0 인 경우, 정정호가도 0 이어야 함
																        * 취소호가는 0으로 입력
																        * 경쟁대량매매호가는 0으로 입력
																        * 코넥스 경매매호가는 0으로 입력 */
	char 	treasury_stock_statement_identification    [5 ];	/* 0 해당없음(0    )
																   N 자사주직접일반(N    )
																   S 자사주직접스탁옵션(S    )
																   1 ~ 99999 자사주신탁(신고서일련번호)

																   * 파생상품은 0 해당없음 사용
																   * 유가증권은 자사주신탁일경우 신청서 입력시마다 발행되는 자사주신고서ID를 입력
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정호가의 경우 원호가의 정보와 동일한 것으로 인정함
																   * 경쟁대량매매호가는 SPACE로 입력
																   * 코넥스 경매매는 0: 해당없음만 입력 */
	char 	treasury_stock_trading_method_code         [1 ];	/* 0 해당없음
																   1 자사주일반
																   2 한은등 자사주
																   3 정부등 자사주

																   * 파생상품은 0 해당없음 사용
																   * 2,3 : 장종료후시간외대량 매수만 가능, 당일거래(매매거래의 종류) 관련 사용
																    -> 자사주일반은 시간외대량 매수 불가 (매도만 가능)
																   * 3 : 장종료후시간외대량 매수시 상/하한가 제한 없음
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정호가의 경우 원호가의 정보와 동일한 것으로 인정함
																   * 경쟁대량매매호가는 SPACE로 입력
																   * 코넥스 경매매는 0: 해당없음만 입력 */

	char 	ask_type_code                              [2 ];	/* 00 해당없음
																   01 일반매도
																   02 차입증권매도
																   06 기타매도

																   * 매수호가 및 파생상품은 00 해당없음 사용
																   * 공매도 가격제한 대상 : 02 차입증권매도
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정호가의 경우 원호가의 정보와 동일한 것으로 인정함
																   *
																   * 03, 04, 05 삭제 --> 06으로 통합
																   * 06 명칭변경
																   * 코넥스 경매매는 02: 차입증권매도 입력불가 */
	char 	credit_type_code                           [2 ];	/* 10 보통(일반)
																   21 자기융자매수
																   22 자기융자매도상환
																   23 자기대주매도
																   24 자기대주매수상환
																   31 유통융자매수
																   32 유통융자매도상환
																   33 유통대주매도
																   34 유통대주매수상환

																   * 파생상품은 10 보통 사용
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char 	trust_principal_type_code                  [2 ];	/* 1)현물
																   10 위탁매매
																   30 자기매매

																   2)파생상품
																     11 위탁거래
																     12 위탁주선거래
																     31 호가입력회원의 자기거래
																     32 비회원의 자기거래

																   3)채권
																     10 위탁매매
																     30 자기매매
																     33 매도대행사 반대매도

																   * 현물
																     - 자기매매 : 증권업자가 자기계정으로 매매하는 경우
																     - 위탁매매 : 자기매매가 아닌 경우
																   * 파생
																     - 신탁업 및 집합투자업을 겸영하는 투자매매업자의 신탁거래 및 집합투자거래는 위탁거래로 입력함
																   * 취소호가는 SPACE로 입력
																   * 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
																  
																   * 21, 22, 23, 33 : 삭제
																   * 11, 32 : 명칭변경 */
	char 	trust_company_number                       [5 ];	/* * 파생상품은 SPACE로 입력
																   * 현물 : 위탁사가 없는 경우 SPACE로 입력
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char 	program_trading_type_code                  [2 ];	/* 1) 현물
																    00 일반
																   (10 차익거래)
																    11 지수차익거래,
																    12 주식차익거래,
																    13 ETF차익거래이면서 지수비차익거래인 경우
																    14 ETF차익거래이면서 지수비차익거래가 아닌 경우
																    15 DR차익거래
																    16 KDR차익거래
																    17 ETN차익거래이면서 지수비차익거래인 경우
																    18 ETN차익거래이면서 지수비차익거래가 아닌 경우
																    19 섹터지수차익거래
																   (20 헤지거래)
																    21 ELW헤지거래
																    22 선물헤지거래
																    23 ETF 헤지거래
																    24 장외파생상품 헤지거래
																    25 ETN 헤지거래
																   (30 비차익거래)
																    31 지수비차익거래
																   (40 설정거래)
																    41 ETF설정거래이면서 지수비차익거래인 경우
																    42 ETF설정거래이면서 지수비차익거래가 아닌 경우

                                                                   2) 파생상품
			                                                        00 일반
			                                                        10 차익거래(주식 및 주가지수)
			                                                        20 헤지거래(주식 및 주가지수)
																                                                                                                                         * 현물 Sidecar 대상 : 11,13,17,31,41
                                                                   * 현물 공매도가격제한 예외 적용 대상 : 11,12,13,14,15,16,17,18,19,21,22,23,25
																   * 공매도금지시장조치시 호가입력제한 예외 적용 대상 : 21, 22, 23, 25
																   * 정정/취소호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
																     --> 단, 파생상품은 원호가와 다른 값을 입력할 수 있음
																   * 코넥스는 경매매/일반매매 모두 00: 일반만 입력가능 */
	char 	substitute_stock_certificate_account_number[12];	/* * 현물은 SPACE로 입력
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char	account_type_code                          [2 ];	/* 1) 현물
																	00 : 일반
																	01 : 대표투자자계좌(외국인)
																	02 : 증권회사통합계좌(외국인)
																	03 : FP계좌-복수일임형랩계좌
																	04 : FP계좌-단일일임형랩계좌
																	05 : FP아닌일임형랩계좌
																	06 : Buy-In매수전용계좌
																	07:  복수신탁계좌-대표계좌 이용
																	08:  단일신탁계좌-대표계좌미이용
																	09 : 기타
																	10: 시장조성헤지계좌(파생연계)
																	11: 시장조성계좌(현물)
																	12: 비과세해외주식투자전용ETF계좌
																	13 : 소액투자전용계좌(코넥스)
																	14 : 외국인통합계좌
																	15 : 차익거래전용계좌

																   2) 파생
																	31 : 위탁일반계좌
																	41 : 자기일반계좌
																	42 : 시장조성계좌
																	51 : 헤지,차익거래계좌
																	52 : 헤지전용계좌
																	61 : 알고리즘거래계좌
																	63 : 알고리즘거래 일반최종투자자
																	64 : 알고리즘거래 배분최종투자자
																	72 : 비알고리즘거래 일반최종투자자
																	73 : 비알고리즘거래 배분최종투자자 

																   3) 채권
																	00 : 일반
																	01 : 대표투자자계좌(외국인)
																	02 : 증권회사통합계좌(외국인)
																	03 : FP계좌-복수일임형랩계좌
																	04 : FP계좌-단일일임형랩계좌
																	05 : FP아닌일임형랩계좌
																	07:  복수신탁계좌-대표계좌 이용(일반채권,KTS)
																	08:  단일신탁계좌-대표계좌미이용(일반채권,KTS)
																	90 : 소액시장공동계좌주문

																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함
																   * 현물 : 01, 02인 경우 ID있는 외국인 이어야 함 */
	char	account_margin_type_code                   [2 ];	/* 1)현물
																   00 일반
																   01 동결계좌
																   02 Buy-In매수전용계좌

																   2)파생상품
																   10 사전증거금 일반
																   11 사후증거금 일반
																   12 사후증거금 할인

																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char	country_code                               [3 ];	/* * 국가코드표 참조 ISO3166-1적용 (Number-3) 없으면 거부
																    - ISO에 없는 코드 추가 : 997, 998, 999 (대한민국국민인해외영주권자, 국제기구, 기타)
																    - 기존코드에는 분류되어 있으나 ISO에는 구분없는 코드 신규 생성 : 991, 992 (말레이지아 라부안, 채널아일랜드)

																     * 정정, 취소호가는 SPACE로 입력 */
	char	investor_type_code                         [4 ];	/* 1000 금융투자회사
																   2000 보험회사
																   3000 자산운용회사 및 투자회사
																   3100 사모펀드
																   4000 은행 (자산운용회사의 신탁재산은 자산운용회사로 분류)
																   5000 기타금융
																   6000 연금, 기금 및 공제회
																   7000 국가, 지자체 국제기구 및 공익기관
																   7100 기타법인
																   8000 개인

																   * 7100 : 기존 7000번 분류를 세분화하여 추가확정 
																   * 파생 : 3100 사용불가
																     --> 기타 투자자분류코드(예 : 3000, 7100 등)에 맞게 입력함
																   * 정정, 취소호가는 SPACE로 입력
																   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함
																   *
																   * 1500, 6100 : 삭제
																   * 1000, 7000 : 명칭변경
																   * 1000, 5000, 7000 : 명칭변경 */
	char	foreign_investor_type_code                 [2 ];	/* 1) 현물
																   00 외국인아님
																   (10 외국인(거주))
																     11 ID있는외국인 (거주)
																     12 ID없는외국인 (거주)
																   (20 외국인(비거주))
																     21 ID있는외국인 (비거주)
																     22 ID없는외국인 (비거주)

																    2) 파생상품
																     00 외국인아님
																     10 외국인(거주)
																     20 외국인(비거주)

																    * 정정, 취소호가는 SPACE로 입력
																    * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char	ord_media_tp_cd                            [1 ];	/* 1 영업점단말 : 주문용단말, 비상주문단말, LP주문
																   2 유선단말 : 음성전화
																   3 무선단말 : 휴대폰, 스마트폰, PDA 등
																   4 HTS : 고객PC, 객장/사이버룸 단말 등
																   8 채권딜러서버
																   9 기타 : 특정위탁자전용주문(통칭 DMA), 반대매매, 선물옵션 주문, K-BLOX 등 */
	char	order_identification_information           [12];	/* 공인IP, 사설IP, 단말고유번호, 전화번호, 가입자번호, 기계번호

																   * 전화번호의 경우 "-" 제외
																   * IP의 경우 "." 제외
																   * 반대매매주문의 경우에는 "BANDAE",
																     ARS주문의 경우에는 "ARS", 
																     대량거래(K-BLOX협상)의 경우에만
																     "BLOCK" 으로 입력 
																   * 선물옵션기본예탁금액 관련하여 회원의 고객주문 직권취소의 경우에는 "OVER"
																   * Kill Switch 입력 시 "KILLSWITCH" */
	char	mac_addr                                   [12];	/* * PC, 스마트폰, 태블릿PC 등 주문 단말에 동시 장착된 유무선 LAN 카드의 MAC 주소 (LAN 카드가 복수인 경우 1. 유선→무선, 2. 오름차순으로 정렬하여 순서가 가장 빠른 주소
																   * 무선단말중 LAN카드 미장착 등 MAC 입력이 불가능한 경우 : “000000000000”
																   * 통칭 DMA : 실제 위탁자(자기 포함)의 IP, MAC 주소 입력이 원칙임. 단, 위탁자(자기 포함) IP, MAC 주소 수집이 불가능한 경우, ‘통칭 DMA 서버’를 통해서  혹은 다른 수단으로 수집이 가능한 IP, MAC 주소 반드시 입력
																   * Mac주소의 경우 : 하이픈(‘-’) 또는 콜론(‘:’) 문자 제외
																   * 반대매매주문의 경우에는 "BANDAE",
																     ARS주문의 경우에는 "ARS", 
																     대량거래(K-BLOX협상)의 경우에만
																     "BLOCK" 으로 입력 
																   * 선물옵션기본예탁금액 관련하여 회원의 고객주문 직권취소의 경우에는 "OVER"
																   * Kill Switch 입력 시 "KILLSWITCH" */
	char	order_date                                 [8 ];	/* YYYYMMDD */
	char	member_firm_order_time                     [9 ];	/* 회원 대외계 시스템(FEP)에 도달한 시각
																   (HHMMSSsss : 1/1000초까지 표시) */
	char    MembershipItem[60];             /* 회원사용영역             */
	char	Program_Order_Declare_cd[1];	/* 프로그램호가신고구분코드	*/
											/* 0:해당없음				*/
}	KRX_JUMUN_DATA;

/* **************************** KRX 주문 ****************************** */
typedef struct {
	KRX_HEADER		Header;
	KRX_JUMUN_DATA	JumunData[6];
}	KRX_JUMUN_Q_FMT;
/* **************************** KRX 주문 ****************************** */

/* **************************** KRX 응답 ****************************** */
/* KRX 차세대 매매 - 주문응답 : 호가입력, 대량호가입력 (24 bytes)   */
typedef struct {
	char	ErrCode[4];                     /* 거부 사유코드            */
	char	DataSeq[11];                    /* 일련번호                 */
	char	ReceivingTime[9];               /* 메시지 도달시간          */
}	KRX_JUMUN_R_DATA;

/* 거래소 응답 Format	*/
typedef struct {
	KRX_HEADER          Header;
	KRX_JUMUN_R_DATA    ReplyData[6];
}	KRX_JUMUN_R_FMT;
/* **************************** KRX 응답 ****************************** */

/* ************************** 내부응답처리 **************************** */
/* KRX 응답 업무계 송신(82+24+300)*n */
typedef struct {
	KRX_HEADER          Header;
	KRX_JUMUN_R_DATA    ReplyData;
	KRX_JUMUN_DATA      JumunData;
}	KRX_JUMUN_S_FMT;
/* ************************** 내부응답처리 **************************** */

/* **************************** KRX 체결 ****************************** */
/* 선물옵션 체결 (300->242 bytes)	*/
typedef struct {
    char    Seq[11];                        /* 일련번호                 */
    char    TrCode[11];                     /* TR code                  */
	char	Megrp_no[2];					/* ME그룹번호				*/
	char	Board_id[2];					/* 보드ID					*/
    char    MembershipNo[5];                /* 회원번호                 */
    char    BranchNo[5];                    /* 지점번호                 */
    char    OrderNo[10];                    /* 주문번호                 */
    char    OriginalOrderNo[10];            /* 원주문번호               */
    char    ItemCode[12];                   /* 종목코드(표준종목코드)   */
    char    Trading_No[11];                 /* 체결번호                 */
    char    Trading_price[11];              /* 체결가격                 */
    char    Trading_Volumn[10];             /* 체결수량                 */
    char    Trading_Type_Code[2];           /* 체결유형코드             */
    char    Trading_Date[8];                /* 체결일자                 */
    char    Trading_Time[9];                /* 체결시각                 */
    char    Nearby_Trading_Price[11];       /* 근월물체결가격           */
    char    Future_Trading_Price[11];       /* 원월물체결가격           */
    char    Ask_Bid_Type_Code[1];           /* 매도매수구분코드         */
    char    Account_No[12];                 /* 계좌번호                 */
    char    Mm_Order_Type_No[11];           /* 시장조성자 호가구분번호  */
    char    Trust_Company_No[5];            /* 위탁사번호               */
    char    Substitute_Stock_No[12];        /* 대용주권 계좌번호        */
    char    MembershipItem[60];             /* 회원사용영역             */
}	KRX_SETTLE_DATA;		// TTRTDP21301(체결결과)

/* 선물옵션 회원처리호가(정상주문거부,정정취소 확인/거부) (300->287 bytes)	*/
typedef struct {
    char    Seq[11];                        /* 일련번호                 */
    char    TrCode[11];                     /* TR code                  */
	char	Megrp_no[2];					/* ME그룹번호				*/
	char	Board_id[2];					/* 보드ID					*/
    char    MembershipNo[5];                /* 회원번호                 */
    char    BranchNo[5];                    /* 지점번호                 */
    char    OrderNo[10];                    /* 주문번호                 */
    char    OriginalOrderNo[10];            /* 원주문번호               */
    char    ItemCode[12];                   /* 종목코드(표준종목코드)   */
    char    TradeFlag[1];                   /* 매도매수구분             */
    char    New_Modify_Cancel_gbn[1];       /* 정정취소구분코드         */
    char    AccountNo[12];                  /* 계좌번호                 */
    char    OrderQuantity[10];              /* 호가수량                 */
    char    Price[11];                      /* 호가가격                 */
    char    Order_Type[1];                  /* 호가유형                 */
    char    Order_Condition[1];             /* 호가조건                 */
    char    Mm_Order_Type_No[11];           /* 지장조성자 호가구분번호  */
    char    Treasury_Stock_Id[5];           /* 자사주신고서 ID          */
    char    Treasury_Stock_Method[1];       /* 자사주매매 방법코드      */
    char    Ask_Type[2];                    /* 매도유형코드             */
    char    Credit_Type[2];                 /* 신용구분코드             */
    char    Trust_Principal_Type[2];        /* 위탁 자기구분코드        */
    char    Trust_Company_No[5];            /* 위탁사번호               */
    char    Program_Trading_Type[2];        /* PT구분코드               */
    char    Substitute_AccNo[12];           /* 대용주권계좌번호         */
    char    Account_Type[2];                /* 계좌구분코드             */
    char    Account_Margin_Type[2];         /* 계좌증거금 유형코드      */
    char    Country_Code[3];                /* 국가코드                 */
    char    Investor_Type[4];               /* 투자자구분코드           */
    char    Foreign_Investor_Type[2];       /* 외국인투자자 구분코드    */
    char    Order_Mesia_Type[1];            /* 주문매체구분코드         */
    char    Order_Identi[12];               /* 주문자식별정보           */
	char	Mac_Addr[12];					/* Mac 주소					*/
    char    Order_Date[8];                  /* 호가일자                 */
    char    Member_Order_Time[9];           /* 회원사 주문시각          */
    char    MembershipItem[60];             /* 회원사용영역             */
    char    Ord_Acpt_tm[9];           		/* 회원사 주문접수          */
    char    Real_Modify_Cancel_Cnt[10];     /* 실정정취소 호가수량      */
    char    Auto_Cancel_Process_Type[1];    /* 자동취소처리구분코드     */
    char    Order_Rejected_Reason[4];       /* 거부사유코드             */
	char	Program_Ord_Declare_Type_Code[1]; /* 프로그램호가신고구분코드 0 */
}	KRX_SETTLE_RESP_DATA;		// TTRODP11301(정상), TTRODP11321(거부)등

/* ************************* 회원체결처리 ***************************** */
typedef struct {
	KRX_HEADER          Header;
	KRX_SETTLE_DATA     SettleData[15];
}	KRX_SETTLE_FMT;
/* ************************* 회원체결처리 ***************************** */

/* ******** 회원처리호가(정상주문거부, 정정취소확인/거부) ************* */
typedef struct {
	KRX_HEADER              Header;
	KRX_SETTLE_RESP_DATA    SettleRespData[15];
}	KRX_SETTLE_RESP_FMT;
/* ******** 회원처리호가(정상주문거부, 정정취소확인/거부) ************* */
/* **************************** KRX 체결 ****************************** */

/*----------------------------------------------------------------------*/
/* Server/Client I/O 정의  Struct                                       */
/*----------------------------------------------------------------------*/
/* Client 요청  내부 조회용 DATA_HEADER 20 Byte */
typedef struct {
	char	TrCode[6];							/* TrCode				*/
	char	Scr_key[4];							/* 화면키				*/
	char	ErrCode[4];							/* Error Code			*/
	char	ApType_Cd[5];						/* Aptype Code			*/
												/* Process 이름 (50101) */
								/* 자동기동/자동종료/강제종료시만 사용	*/ 
	char    Media_gbn[1];                       /* 매체구분(A/C/T)		*/
	char    Filler[30];                         /* 예비                 */
}	SEARCH_HEADER;

/*----------------------------------------------------------------------
 - 조회 TR
	700001 : A0배치정보 (종목배치 A0XXX의 KRX 포맷으로 내려감)
	700100 : LP자동전략 상태조회
	......

 - 전략처리 TR
	500901 : LP잔고
	500902 : LP이론가

 - 아침배치 TR
	900001 : 계좌정보
	900002 : 펀드별 한도관리
	900003 : 원장잔고
	900004 : 매매제한종목
	900005 : 증거금율
	900006 : 영업일자
	900010 : CD금리(IO미정)
	
---------------------------------------------------------------------- */

/* 700001 IO정하지 않음, TR로 인지하고 각 시장의 A0포맷으로 내려감 */

/* 700100 Input은 없고 TR로 인지한다	*/
typedef struct {
	char	ApType[5];					/* ApType (ex, 50101)	*/
	char	Str_No[4];					/* 전략번호				*/
	char	Item_Code[12];				/* 종목코드				*/
}	OUT_700100;

/* 700002 (시세기본정보), 시장코드(2)+종목코드(12) Input, Output은 해당종목의 A0(해외포함)	*/
/* 단, 해외시세는 Seq가 없어서 내부에서 만든 Seq를 추가로 줘야(공유)한다. 따라서 
       국내시세기본정보는 50(Header)+A0, 해외는 50+5(내부에서만든 종목일련번호)+A0(해외포맷) */
typedef struct {
	char	Mk_Gbn[2];					/* 01:지수선물, 02:지수옵션, 03:주식선물, 04:주식옵션, 05:유가증권, 06:코스닥, 08:KRX300, 09:Kosdaq150F, 10:Kosdaq150O	*/
	char	Item_Code[12];				/* 종목코드				*/
}	IN_700002;

/* 900001 900은 Set으로 Input만 있다(계좌정보)	*/
typedef struct {
	char	Accno_Seq[2];				/* 0부터 순차증가	*/
	char	Accno[12];					/* 주문나가는 계좌번호와 동일, 앞에 빈칸은 0으로 채움	*/
	char	SD_Gbn[1];					/* 1:현물, 2:파생	*/
}	IN_900001;

 
/* 900002 900은 Set으로 Input만 있다(펀드별 한도관리)	*/
typedef struct {
	char	Accno[12];					/* 주문나가는 계좌번호와 동일, 앞에 빈칸은 0으로 채움	*/
	char	Risk_Cd[2];					/* 운용상품분류코드	운용상품분류
											11	0	코스피200선물
											12	1	코스피200콜옵션
											13	2	코스피200풋옵션
											14	3	주식선물
											15	4	주식콜옵션
											16	5	주식풋옵션
								(미사용)	17		스타선물
								(미사용)	18		변동성지수선물
								(미사용)	19		섹터지수선물
											20	6	코스닥150선물
								(미사용)	21		국채선물(3년)
								(미사용)	22		국채선물(5년)
								(미사용)	23		국채선물(10년)
								(미사용)	24		통안증권금리선물
								(미사용)	25		원달러선물
								(미사용)	26		엔선물
								(미사용)	27		유로선물
								(미사용)	28		금선물
								(미사용)	29		돈육선물
								(미사용)	31		미국달러옵션
								(미사용)	32		미니금선물
								(미사용)	33		CME선물
											34	11	미니코스피200선물
											35	12	미니코스피200콜옵션
											36	13	미니코스피200풋옵션
								(미사용)	38		위안선물
								(미사용)	40		유로스톡스50선물
											44	7	코스닥150콜옵션
											45	8	코스닥150풋옵션
											46	9	KRX300선물
								(미사용)	49		위클리코스피200 콜옵션
								(미사용)	50		위클리코스피200 풋옵션
											51	10	현물주식
								(미사용)	61		현물채권
								(미사용)	70		해외주식		 */
	char	One_Limit_Cnt_Gbn[1];		/* 1회주문계약수 체크여부 0:미사용, 1:사용	*/
	char	One_Limit_Cnt[12];			/* 1회주문계약수			*/
	char	One_Limit_Gum_Gbn[1];		/* 1회주문금액 체크여부 0:미사용, 1:사용	*/
	char	One_Limit_Gum[12];			/* 1회주문금액				*/
	char	One_Limit_Tick_Gbn[1];		/* 1회주문틱 체크여부 0:미사용, 1:사용	*/
	char	One_Limit_Tick[8];			/* 1회주문틱				*/
	char	Tot_Limit_DoCnt_Gbn[1];		/* 누적매도계약수 체크여부 0:미사용, 1:사용	*/
	char	Tot_Limit_DoCnt[12];		/* 누적매도계약수			*/
	char	Tot_Limit_DoGum_Gbn[1];		/* 누적매도금액 체크여부 0:미사용, 1:사용	*/
	char	Tot_Limit_DoGum[12];		/* 누적매도금액			*/
	char	Tot_Limit_SuCnt_Gbn[1];		/* 누적매수계약수 체크여부 0:미사용, 1:사용	*/
	char	Tot_Limit_SuCnt[12];		/* 누적매수계약수			*/
	char	Tot_Limit_SuGum_Gbn[1];		/* 누적매수금액 체크여부 0:미사용, 1:사용	*/
	char	Tot_Limit_SuGum[12];		/* 누적매수금액			*/
}	IN_900002;

/* 900003 900은 Set으로 Input만 있다(원장잔고)	*/
typedef struct {
	char	Accno[12];					/* 주문나가는 계좌번호와 동일, 앞에 빈칸은 0으로 채움	*/
	char	Mk_Gbn[2];					/* 01:지수선물, 02:지수옵션, 03:주식선물, 04:주식옵션, 05:유가증권, 06:코스닥, 08:KRX300, 09:Kosdaq150F, 10:Kosdaq150O	*/
	char	Item_Seq[9];				/* A0의 종목일련번호	*/
	char	JanGo[12];					/* 장부수량				*/
	char	JanGum[12];					/* 장부금액				*/
	char	Borrow_Cnt[12];				/* 차입수량				*/
	char	Total_Su_Che[12];			/* 매수누적체결수량		*/
	char	Total_Do_Che[12];			/* 매도누적체결수량		*/
}	IN_900003;

/* 900004 900은 Set으로 Input만 있다(매매제한종목)	*/
typedef struct {
	char	Mk_Gbn[2];					/* 01:지수선물, 02:지수옵션, 03:주식선물, 04:주식옵션, 05:유가증권, 06:코스닥, 08:KRX300, 09:Kosdaq150F, 10:Kosdaq150O	*/
	char	Item_Seq[9];				/* A0의 종목일련번호, "999999999999"이면 해당시장의 Dont_Trade초기화(0)	*/
	char	Dont_Trade[1];				/* 0:정상, 1:불가			*/
}	IN_900004;

/* 900005 900은 Set으로 Input만 있다(증거금율)	*/
typedef struct {
	char	Asset_Cd[2];				/* 기초자산코드(08인 KRX300만사용)	*/
	char	Indv_Rate[8];				/* 증거금율(소수점포함)	*/
}	IN_900005;

/* 900006 900은 Set으로 Input만 있다(영업일자)	*/
typedef struct {
	char	Business_Day[8];			/* 최근영업일		*/
}	IN_900006;

/* 900007 900은 Set으로 Input만 있다(CD금리)	*/
typedef struct {
	char	CD_Rate[8];					/* CD금리, 소수점으로 온다	*/
}	IN_900007;



/* 600020 미체결내역조회 Output 480 Byte	*/
typedef struct {
	char    Search_Gbn[1];                      /* 조회구분				*/
												/* 0:계좌전체 			*/
												/* 2:종목코드 일치      */
	char    AccountNo[9];                       /* 계좌번호             */
	char    Item_Cd[8];                         /* 종목코드             */
	char    Filler[462];                        /* 예비                 */
}	ACC_MICHE_SELECT_IN_DATA;

/* 600020 미체결내역조회 Output 480 Byte	*/
typedef struct {
	char	Next_Flag[1];						/* 처리구분 			*/
												/* S:start, E:End, M	*/
	char	Curr_Cnt[4];						/* 현재 Count			*/
	char    AccountNo[9];                       /* 계좌번호             */
	char    Item_Cd[8];                         /* 종목코드             */
	char	Avr_Price[17];						/* 평균단가(소수점 8) */
	char	TradeFlag[2];	                   	/* 매도매수구분         */	
												/* 01 ,신규매도 		*/
												/* 02 ,신규매수 		*/
												/* 03 ,전매도   		*/
												/* 04 ,환매수   		*/
												/* 05 ,최종만기매도 	*/
												/* 06 ,최종만기매수 	*/
												/* 07 ,권리행사 		*/
												/* 08 ,권리배정 		*/
												/* 09 ,옵션매도소멸 	*/
												/* 10 ,옵션매수소멸 	*/
	char	GetCnt[9];							/* 보유수량				*/
	char	Filler[430];						/* 예비					*/
}	ACC_JAN_SELECT_OUT_DATA;

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
#ifdef  _GLOBAL

KRX_JUMUN_DATA				JmData;
KRX_JUMUN_DATA				*pJmData;

#else

extern	KRX_JUMUN_DATA		JmData;
extern	KRX_JUMUN_DATA		*pJmData;

#endif

/*************************************************************************
	End of Program (pa_struct.h)
*************************************************************************/
#endif
