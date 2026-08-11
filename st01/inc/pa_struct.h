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

/* ******************************************************************** */
/* ************************* 현물_파생,Start ************************** */
/* 	(파생_사용) TTRODP11301 : 정상
	(파생_사용) TTRODP11321 : 거부
	(파생_사용) TTRODP11303 : 자동취소
		   		TTRODP11305 : 경쟁대량정상
		   		TTRODP11322 : 경쟁대량거부
		   		TTRODP11306 : 경쟁대량자동취소
		   		TTRODP11308 : Buy-In 자동매수
		   		TTRODP11310 : 프로그램매매호가효력정지
		   		TTRODP11311 : 실시간가격제한지정가전환(접수즉시전환)
		   		TTRODP11312 : 실시간가격제한지정가전환(접수후전환)

*/
/* KRX 주문/체결/장운영 header (82 bytes)  */
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
	char    Encrypt[1];             /* 암호화유무(Y/N)                  */
}   KRX_HEADER;

/* 주문/체결/장운영 Body 공통 앞부분 (24 bytes)
   거의 모든 주문채결 TR이 이 3개 필드로 시작 */
typedef struct {
	char	DataSeq[11];			/* 데이터 일련번호						*/
	char	Transaction_Code[11];	/* TR코드 (TTRTDP42301 등)				*/
	char	Megrp_no[2];			/* ME그룹번호							*/
}	KRX_BODY_COMMON;

/* Header + Body 공통부 통합 (106 bytes) */
typedef struct {
	KRX_HEADER		Header;			/* 82 bytes */
	KRX_BODY_COMMON	Body;			/* 24 bytes */
}	KRX_MSG_COMMON;

/* KRX 차세대 Session_송신 */
typedef struct {
    KRX_HEADER  Header;
    char        Data[41];			/* 로그온:I/F프로세스정보(10)+I/F프로세스고유번호(30)+암호화 적용여부(1,Y/N) */
}   KRX_SESSION_FMT;

/* KRX 차세대 Session_수신 */
typedef struct {
    KRX_HEADER  Header;
    char        Data[116];			/* 체결수신은 ME그룹 필요 더 길다 */
}   KRX_R_SESSION_FMT;

/* KRX 대체거래소 제도변경분 반영 - 주문요청 : 호가입력 (261->294 bytes), 202504 update  */
typedef struct {
	char	DataSeq							[11];	/* 메시지 일련번호          					*/
	char 	Transaction_Code                [11];	/* 트랜잭션코드									*/
													/*  신규호가 :  TCHODR10001
														정정호가 :  TCHODR10002
														취소호가 :  TCHODR10003
														경쟁대량신규호가 :  TCHAOR10001
														경쟁대량취소호가 :  TCHAOR10003 			*/
	char	Megrp_no						[2 ];	/* ME그룹번호(2025신규)							*/
													/* 회원사는 '00'으로 거래소 전달				*/
	char 	Board_Id                        [2 ];	/* 보드ID				 				 		*/
	char 	MembershipNo					[5 ];	/* 거래소가 부여한 회원번호				 		*/	/* 정합성 체크 대상 */
													/* 정정/취소 호가인 경우 원호가와 동일한 		*/
													/* 값이 입력되어야 함(원호가 정합성 체크)		*/
	char 	BranchNo	                    [5 ];	/* 회원이 거래소에 신고한 지점번호		 		*/	/* 정합성 체크 대상	*/
													/* 정정/취소 호가인 경우 원호가와 동일한 		*/
													/* 값이 입력되어야 함(원호가 정합성 체크)		*/
	char 	OrderNo				            [10];	/* 주문ID										*/
													/* * 문자를 포함
													   * 종목+회원+지점+주문ID가 Key (Unique)
													   * * 주문ID 사용불가영역 :
													   *  9000000001 ~ 9999999999
													   *  M000000001 ~ M999999999(추가)
													   *  C000000001 ~ C999999999(추가)
													   *  K000000001 ~ K999999999(추가)
													   *  (비상주문시 자동생성되는 주문ID영역으로 회원사에서 주문ID 사용하면 안됨) */
	char 	OriginalOrderNo				    [10];	/* 원주문ID										*/
													/* * 신규호가는 SPACE로 입력			 		*/
	char 	ItemCode                        [12];	/* 종목코드 									*/
													/* 현물/파생/채권/REPO 통합상품의 종목 코드(ISIN종목코드) */
	char 	TradeFlag		                [1 ];	/* 매도매수구분코드								*/	/* 정합성 체크 대상 */
													/* 1 매도
													   2 매수
													   * 정정/취소 호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크) */
	char 	New_Modify_Cancel_gbn		   [1 ];	/* 정정취소구분코드 							*/
													/* 1 신규
													   2 정정
													   3 취소 */
	char 	AccountNo	                    [12];	/* 계좌번호										*/	/* 정합성 체크 대상 */
													/* * 12자리 구성방법 제한없음. 12자리를 회원사별로 사용
													   * 정정/취소 호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크) */
	char 	Order_Quantity                  [10];	/* 호가수량 */
	char 	Price                     		[11];	/* 호가가격(Float)								*/
													/* * 가격이 없는 호가유형인 경우 0으로 입력
													   * 시간외종가 호가의 경우 0으로 입력
													   * * 취소호가는 0으로 입력
													   * * 경쟁대량매매는 0으로 입력				*/
	char 	Order_Type		                [1 ];	/* 호가유형코드									*/
													/* 1 시장가 (Market)
													   2 지정가 (Limit)
													   4 스톱지정가(StopLimit)
													   M 중간가(Midpoint)

													   T 가격제한시장가
													   W 가격제한최유리지정가
													   I 조건부지정가 (Limit To Market)
													   V 경쟁대량 (현물만 해당, 값 변경 : 3->V)
													   X 최유리지정가
													   Y 최우선지정가 (현물만 해당)

													   * 취소호가는 SPACE로 입력
													   * * 코넥스 경매매는 2:지정가만 가능
													   * * 파생시장 1,X 불가 (T,W 으로 대체)		*/
	char 	Order_Condition		            [1 ];	/* 호가조건코드									*/
													/* 0 일반(FAS)
													   3 FAK(IOC)
													   4 FOK

													   * 취소호가는 SPACE로 입력
													   * 코넥스 경매매는 0: 일반만 가능				*/
	char	Min_Che_Cnt						[10];	/* 최소체결수량(2025신규)						*/
													/* 회원사는 '0'으로 거래소 전달					*/
	char 	Mm_Order_Type_No                [1 ];	/* 시장조성자호가구분코드						*/	/* 정합성 체크 대상	*/
													/* 1) 현물
													     0 : 일반
													     1 : LP호가
													     2 : MM호가
													   2) 파생상품
													     0 : 일반
														 2 : MM호가

														* 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
													    * 취소호가는 0으로 입력
													    * 경쟁대량매매호가는 0으로 입력
													    * 코넥스 경매매호가는 0으로 입력			*/
	char 	Treasury_Stock_Id				 [5 ];	/* 자사주신고서ID						*/
													/* 0 해당없음(0    )
													   N 자사주직접일반(N    )
													   S 자사주직접스탁옵션(S    )
													   1 ~ 99999 자사주신탁(신고서일련번호)

													   * 파생상품은 0 해당없음 사용
													   * 유가증권은 자사주신탁일경우 신청서 입력시마다 발행되는 자사주신고서ID를 입력
													   * 정정, 취소호가는 SPACE로 입력
													   * 정정호가의 경우 원호가의 정보와 동일한 것으로 인정함
													   * 경쟁대량매매호가는 SPACE로 입력
													   * 코넥스 경매매는 0: 해당없음만 입력 */
	char 	Treasury_Stock_Method			[1 ];	/* 자사주매매방법코드						*/
													/* 0 해당없음
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
													   * 코넥스 경매매는 0: 해당없음만 입력			*/
	char 	Ask_Type						[2 ];	/* 매도유형코드									*/
													/* 00 해당없음
													   01 일반매도
													   02 차입증권매도
													   06 기타매도

													   * 매수호가 및 파생상품은 00 해당없음 사용
													   * 공매도 가격제한 대상 : 02 차입증권매도
													   * 정정, 취소호가는 SPACE로 입력
													   * 정정호가의 경우 원호가의 정보와 동일한 것으로 인정함

													   * 03, 04, 05 삭제 --> 06으로 통합
													   * 06 명칭변경
													   * 코넥스 경매매는 02: 차입증권매도 입력불가	*/
	char 	Credit_Type						[2 ];	/* 신용구분코드									*/
													/* 10 보통(일반)
													   21 자기융자매수
													   22 자기융자매도상환
													   23 자기대주매도
													   24 자기대주매수상환
													   31 유통융자매수
													   32 유통융자매도상환
													   33 유통대주매도
													   34 유통대주매수상환
													   41 CFD연계매매

													   * 파생상품은 10 보통 사용
													   * 정정, 취소호가는 SPACE로 입력
													   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char 	Trust_Principal_Type			[2 ];	/* 위탁자기구분코드								*/
													/* 1)현물
													     10 위탁매매
													     30 자기매매

													   2)파생상품
													     11 위탁거래
														 12 위탁주선거래
														 13 회원의 위탁거래
														 31 호가입력회원의 자기거래

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
													   * 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)	*/
	char 	Trust_Company_No[5 ];					/* 위탁사번호									*/
													/* 파생 : 회원의 위탁거래(위탁자기구분코드=13인 경우 위탁사 번호 입력,
															  위탁사가 없는 경우(위탁자기구분코드 11,12,31) SPACE로 입력
													   * 현물 : 위탁사가 없는 경우 SPACE로 입력
													   * 정정, 취소호가는 SPACE로 입력
													   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char 	Program_Trading_Type			[2 ];	/* PT구분코드									*/
													/* 1) 현물
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
														* 현물 공매도가격제한 예외 적용 대상
														  (증권그룹 ST,SC,RT,MF,IF,FS,DR에 대해)
														  11,12,15,16,19,21,23,25
														* 공매도금지시장조치시 호가입력제한 예외 적용 대상 : 21, 22, 23, 25
														* 정정/취소호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
														  --> 단, 파생상품은 원호가와 다른 값을 입력할 수 있음
														* 코넥스는 경매매/일반매매 모두 00: 일반만 입력가능 */
	char 	Substitute_AccNo				[12];	/* 대용주권계좌번호				*/
													/* * 현물은 SPACE로 입력
													   * 정정, 취소호가는 SPACE로 입력
													   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char	Account_Type					[2 ];	/* 계좌구분코드							*/
												/* 1) 현물
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
													17 : 외국 금융투자업자 계좌(외국인)(신설)

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
	char	Account_Margin_Type			[2 ];	/* 계좌증거금유형코드				*/
												/* 1)현물
												    00 일반
												    01 동결계좌
												    02 Buy-In매수전용계좌

												   2)파생상품
													10 사전증거금 일반
													11 사후증거금 일반
													12 사후증거금 할인

												   * 정정, 취소호가는 SPACE로 입력
												   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함 */
	char	Country_Code				[3 ];	/* 국가코드							*/
												/* 대한민국 410						*/
												/* * 국가코드표 참조 ISO3166-1적용 (Number-3) 없으면 거부
												    - ISO에 없는 코드 추가 : 997, 998, 999 (대한민국국민인해외영주권자, 국제기구, 기타)
												    - 기존코드에는 분류되어 있으나 ISO에는 구분없는 코드 신규 생성 : 991, 992 (말레이지아 라부안, 채널아일랜드)

												    * 정정, 취소호가는 SPACE로 입력 */
	char	Investor_Type				[4 ];	/* 투자자구분코드					*/
												/* 1000 금융투자회사
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
												   * 1000, 5000, 7000 : 명칭변경 	*/
	char	Foreign_Investor_Type		[2 ];	/* 외국인투자자구분코드				*/
												/* 1) 현물
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
	char	Order_Mesia_Type				[1 ];	/* 주문매체구분코드					*/
												/* 1 영업점단말 : 주문용단말, 비상주문단말, LP주문
												   2 유선단말 : 음성전화
												   3 무선단말 : 휴대폰, 스마트폰, PDA 등
												   4 HTS : 고객PC, 객장/사이버룸 단말 등
												   8 채권딜러서버
												   9 기타 : 특정위탁자전용주문(통칭 DMA), 반대매매, 선물옵션 주문, K-BLOX 등 */
	char	Order_Identi					 [12];	/* 주문자식별정보				*/
												/* 공인IP, 사설IP, 단말고유번호, 전화번호, 가입자번호, 기계번호
												 * 전화번호의 경우 "-" 제외
												 * IP의 경우 "." 제외
												 * 반대매매주문의 경우에는 "BANDAE",
												    ARS주문의 경우에는 "ARS", 
												    대량거래(K-BLOX협상)의 경우에만 "BLOCK" 으로 입력 
												 * 선물옵션기본예탁금액 관련하여 회원의 고객주문 직권취소의 경우에는 "OVER"
												 * Kill Switch 입력 시 "KILLSWITCH" */
	char	Mac_Addr						[12];	/* MAC주소							*/
												/* * PC, 스마트폰, 태블릿PC 등 주문 단말에 동시 장착된 유무선 LAN 카드의 MAC 주소 (LAN 카드가 복수인 경우
														1. 유선→무선,
														2. 오름차순으로 정렬하여 순서가 가장 빠른 주소
															* 무선단말중 LAN카드 미장착 등 MAC 입력이 불가능한 경우 : “000000000000”
															* 통칭 DMA : 실제 위탁자(자기 포함)의 IP, MAC 주소 입력이 원칙임.
															  단, 위탁자(자기 포함) IP, MAC 주소 수집이 불가능한 경우,
															  ‘통칭 DMA 서버’를 통해서  혹은 다른 수단으로 수집이 가능한 IP, MAC 주소 반드시 입력
															* Mac주소의 경우 : 하이픈(‘-’) 또는 콜론(‘:’) 문자 제외
															* 반대매매주문의 경우에는 "BANDAE",
															  ARS주문의 경우에는 "ARS", 
															  대량거래(K-BLOX협상)의 경우에만 "BLOCK" 으로 입력 
															* 선물옵션기본예탁금액 관련하여 회원의 고객주문 직권취소의 경우에는 "OVER"
															* Kill Switch 입력 시 "KILLSWITCH" */
	char	Order_Date					[8 ];	/* 호가일자							*/
												/* YYYYMMDD							*/
												/* 파생야간은 RDS 파생종목정보 영업일자	*/
	char	Member_Send_Time			[9 ];	/* 회원사주문시각					*/
												/* 회원 대외계 시스템(FEP)에 도달한 시각
												   (HHMMSSsss : 1/1000초까지 표시)	*/
	char    MembershipItem				[60];	/* 회원사용영역             		*/
	char	Algo_Stgy_Type				[1 ];	/* 알고리즘전략구분코드				*/
												/*  1 : 일반호가
													2 : 알고리즘호가
													3 : 고속 알고리즘호가
													* 정정 및 취소는 SPACE로 입력	*/
/* 2025 신규 */
	char	Trdr_Id						[6 ];	/* 거래자ID							*/
												/* 일괄호가취소, 자전거래방지 등의 기능을 제공하기 위해 투자자에게 제공하는 ID
												   * 해당사항 없는 경우 SPACE로 입력
												   * 경쟁대량매매호가는 SPACE로 입력
												   * 취소호가는 SPACE로 입력
												   * 정정호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)	*/
	char	Ord_Grp_No					[2 ];	/* 호가그룹번호						*/
												/* 거래자ID를 부여받은 투자자가 사전에 그룹핑을 위해 지정한 호가의 그룹번호
													(숫자, 대문자, 소문자 : 00 ~ 99, aa ~ ZZ) 지정한 그룹번호에 해당하는 호가를 일괄 취소한다
												   * 해당사항 없는 경우 SPACE로 입력
												   * 경쟁대량매매호가는 SPACE로 입력
												   * 정정 및 취소는 SPACE로 입력	*/
	char	Smp_Cd						[1 ];	/* 자전거래방지코드					*/
												/* 0 : 해당사항없음(선행/후행에 관계없이 자전거래방지 미적용)
												   1 : 기존호가 취소
												   2 : 신규호가 취소
												   3 : 양방향 호가 취소

												   * 경쟁대량매매호가는 0 으로 입력
												   * 정정 및 취소는 SPACE로 입력
												   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함	*/
	char	Ord_Cond_Prc				[11];	/* 호가조건가격						*/
												/* * 유가/코스닥 StopLimit 호가유형만 값 입력(그 외는 0입력)
												   * 전환 후 정정호가는 0으로 입력(전환 후 조건가격 정정불가)
												   * 취소호가는 0으로 입력			*/
	char	Trd_Mkt_Choic_Tp_Cd			[1 ];	/* 거래시장선택구분코드				*/
												/* [유가/코스닥]
												   0 : 해당없음 (거래시장 미선택)
												   1 : 해당있음 (거래시장 선택)
												   * 위탁자/회원사 등이 거래시장을 선택한 경우 '해당있음',
													 그렇지 않은 경우 '해당없음' 설정
													[ETF, 코넥스, 파생]
													 - '해당있음(1)'로 설정해서 송부*/
	char	Srt_Sell_Id					[10];	/* 공매도ID							*/
												/* 공매도 거래자에게 부여되는 고유 번호	*/
}	KRX_JUMUN_DATA;

/* **************************** KRX 주문 ****************************** */
typedef struct {
	KRX_HEADER		Header;
	KRX_JUMUN_DATA	JumunData[6];
}	KRX_JUMUN_Q_FMT;
/* **************************** KRX 주문 ****************************** */

/* **************************** KRX 응답 ****************************** */
/* KRX 차세대 매매 - 주문응답 : 호가입력, 대량호가입력 (15 bytes)   */
typedef struct {
	char	ErrCode[4];                     /* 거부 사유코드            */
	char	DataSeq[11];                    /* 일련번호                 */
/*	char	ReceivingTime[9];                  메시지 도달시간				*/
}	KRX_JUMUN_R_DATA;

/* 거래소 응답 Format	*/
typedef struct {
	KRX_HEADER          Header;
	KRX_JUMUN_R_DATA    ReplyData[1];		/* 6 -> 1 */
}	KRX_JUMUN_R_FMT;
/* **************************** KRX 응답 ****************************** */

/* ************************** 내부응답처리 **************************** */
/* KRX 응답 업무계 송신(82+15+294)*n */
typedef struct {
	KRX_HEADER          Header;
	KRX_JUMUN_R_DATA    ReplyData;
	KRX_JUMUN_DATA      JumunData;
}	KRX_JUMUN_S_FMT;
/* ************************** 내부응답처리 **************************** */

/* ********************** 회원체결처리.Start ************************** */
/* **************************** KRX 체결 ****************************** */
/* 체결 (233 bytes)	2025 */
typedef struct {
    char    DataSeq[11];                        /* 일련번호                 */
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
	char	Session_Id[2];					/* 세션ID					*/
    char    Trading_Date[8];                /* 체결일자                 */
    char    Trading_Time[9];                /* 체결시각                 */
    char    Nearby_Trading_Price[11];       /* 근월물체결가격           */
    char    Future_Trading_Price[11];       /* 원월물체결가격           */
    char    TradeFlag[1];		            /* 매도매수구분코드         */
    char    AccountNo[12];                  /* 계좌번호                 */
    char    Mm_Order_Type_No[1];            /* 시장조성자 호가구분번호  */
    char    Trust_Company_No[5];			/* 위탁사번호               */
    char    Substitute_AccNo[12];	        /* 대용주권 계좌번호        */
    char    MembershipItem[60];             /* 회원사용영역             */
	char	Last_Ask_Bid_Type[1];			/* 최종매도매수구분코드		*/
											/* 체결호가 중 후행호가(Taker)의 매도매수 구분 표기
											   1 : 후행호가가 매도
											   2 : 후행호가가 매수
											   SPACE : 후행호가 구분 없음	*/
											/* NXT는 다르게 사용
												0: Maker
												1: Taker
												2: 일반(단일가)			*/
}	KRX_SETTLE_DATA;		/* TTRTDP21301(체결결과) */

typedef struct {
	KRX_HEADER          Header;
	KRX_SETTLE_DATA     SettleData[15];
}	KRX_SETTLE_FMT;
/* ************************* 회원체결처리.End ***************************** */

/* 회원처리호가(정상주문거부,정정취소 확인/거부) (318 bytes)	*/
/* ********************** 회원처리호가.Start ************************** */
typedef struct {
    char    DataSeq[11];                        /* 일련번호                 */
    char    TrCode[11];                     /* TR code                  */
	char	Megrp_no[2];					/* ME그룹번호				*/
	char	Board_id[2];					/* 보드ID					*/
    char    MembershipNo[5];                /* 회원번호                 */
    char    BranchNo[5];                    /* 지점번호                 */
    char    OrderNo[10];                    /* 주문번호                 */
    char    OriginalOrderNo[10];            /* 원주문번호               */
    char    ItemCode[12];                   /* 종목코드(표준종목코드)   */
    char    TradeFlag[1];		            /* 매도매수구분             */
    char    New_Modify_Cancel_gbn[1];       /* 정정취소구분코드         */
    char    AccountNo[12];                  /* 계좌번호                 */
    char    OrderQuantity[10];              /* 호가수량                 */
    char    Price[11];                      /* 호가가격                 */
    char    Order_Type[1];                  /* 호가유형                 */
    char    Order_Condition[1];             /* 호가조건                 */
	char	Min_Che_Cnt[10];				/* 최소체결수량				*/
    char    Mm_Order_Type_No[1];            /* 시장조성자 호가구분번호  */
    char    Treasury_Stock_Id[5];           /* 자사주신고서 ID          */
    char    Treasury_Stock_Method[1];       /* 자사주매매 방법코드      */
    char    Ask_Type[2];                    /* 매도유형코드             */
    char    Credit_Type[2];                 /* 신용구분코드             */
    char    Trust_Principal_Type[2];        /* 위탁 자기구분코드        */
    char    Trust_Company_No[5];			/* 위탁사번호               */
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
    char    Member_Send_Time[9];            /* 회원사 주문시각          */
    char    MembershipItem[60];             /* 회원사용영역             */
    char    Order_Accept_Time[9];     		/* 회원사 주문접수          */
    char    Real_Modify_Cancel_Cnt[10];     /* 실정정취소 호가수량      */
    char    Auto_Cancel_Process_Type[1];    /* 자동취소처리구분코드     */
    char    Order_Rejected_Reason[4];       /* 거부사유코드             */
	char	Program_Ord_Declare_Type_Code[1]; /* 프로그램호가신고구분코드 0 */
	char	Trdr_Id[6];						/* 거래자ID					*/
	char	Ord_Grp_No[2];					/* 호가그룹번호				*/
	char	Smp_Cd[1];						/* 자전거래방지코드			*/
	char	Ord_Cond_Prc[11];				/* 호가조건가격				*/
	char	Trd_Mkt_Choic_Tp_Cd[1];			/* 거래시장선택구분코드		*/
	char	Srt_Sell_Id[10];				/* 공매도 거래자에게 부여되는 고유 번호	*/
}	KRX_SETTLE_RESP_DATA;		/* TTRODP11301(정상), TTRODP11321(거부), TTRODP11303(자동취소) 등 */

typedef struct {
	KRX_HEADER              Header;
	KRX_SETTLE_RESP_DATA    SettleRespData[15];
}	KRX_SETTLE_RESP_FMT;
/* ********************** 회원처리호가.End ************************** */
/* ****************************************************************** */
/* ************************* 현물_파생,End ************************** */


/* ****************************************************************** */
/* ************************* 채권_Start ***************************** */
/* 채권 TR정리 */
/*
	< 채권일반호가입력 254 byte >
	TCHODR40001 : 신규주문
	TCHODR40002 : 정정주문
	TCHODR40003 : 취소주문						
	< 채권조성호가입력, 사용여부 협의내용 255 byte >
	TCHMOR40001 : 신규주문
	TCHMOR40001 : 정정주문
	TCHMOR40001 : 취소주문

	< 채권일반처리호가 291 byte >
	TTRODP41301 : 정상
	TTRODP41302 : 거부
	TTRODP41303 : 자동취소
	< 채권조성처리호가, 사용여부 협의내용 295 byte >
	TTRMOP41301 : 정상
	TTRMOP41302 : 거부
	TTRMOP41303 : 자동취소

	< 채권체결결과(일반,조성) 317 byte >
	TTRTDP42301 : 채권체결결과
*/

/* ******** 채권일반호가입력(254) *************2025 */
/* 일반채권(B), 소액채권(M), KTS(K)
	REPO(R)는 대상아님(장내채권 아닌듯)				*/
/* ************************************************ */
typedef struct {
	char DataSeq                 [11];  /* 메세지일련번호 */
	char Transaction_Code        [11];  /* 트랜잭션코드 */
	char Megrp_no                [2];   /* ME그룹번호 */
	char Undly_Asset_Mkt_Id      [3];   /* 시장ID */
	char Board_id                [2];   /* 보드ID */
	char MembershipNo            [5];   /* 회원번호 */
	char BranchNo                [5];   /* 지점번호 */
	char OrderNo                 [10];  /* 주문ID */
	char OriginalOrderNo         [10];  /* 원주문ID */
	char ItemCode                [12];  /* 종목코드 */
	char TradeFlag 			     [1];   /* 매도매수구분코드 */
	char New_Modify_Cancel_gbn   [1];   /* 정정취소구분코드 */
	char AccountNo               [12];  /* 계좌번호 */
	char OrderQuantity           [10];  /* 호가수량 */
	char Price                   [11];  /* 호가가격 */
	char Order_Type              [1];   /* 호가유형코드 */
	char Order_Condition	     [1];   /* 호가조건코드 */
	char Ask_Type                [2];   /* 매도유형코드 */
	char Trust_Principal_Type    [2];   /* 위탁자기구분코드 */
	char Trust_Company_No        [5];   /* 위탁사번호 */
	char Account_Type            [2];   /* 계좌구분코드 */
	char Country_Code            [3];   /* 국가코드 */
	char Investor_Type           [4];   /* 투자자구분코드 */
	char Filler                  [6];   /* 필러값 */
	char Foreign_Investor_Type   [2];   /* 외국인투자자구분코드 */
	char Sm_Bond_Part_Yn         [1];   /* 소액채권장종료매매참여여부 */
	char Tax_Exempt_Yn           [1];   /* 비과세여부 */
	char Order_Mesia_Type        [1];   /* 주문매체구분코드 */
	char Order_Identi            [12];  /* 주문자식별정보 */
	char Mac_Addr                [12];  /* MAC주소 */
	char Order_Date              [8];   /* 호가일자 */
	char Member_Send_Time        [9];   /* 회원사주문시각 */
	char MembershipItem          [60];  /* 회원사용영역 */
	char Trdr_Id                 [5];   /* 거래원번호 */
	char Mm_Order_Type_No        [11];  /* 시장조성자호가구분번호 */
}	KRX_NOTE_JUMUN_DATA;		/* TCHODR40001_신규, TCHODR40002_정정, TCHODR40003_취소 */


/* ******** 채권조성호가입력(255) *************2025 */
/* 일반채권(B), 소액채권(M), KTS(K)
	REPO(R)는 대상아님(장내채권 아닌듯)				*/
/* ************************************************ */
/* 채권조성호가는 양방향주문이 가능함 */
/* 미사용, 협의시 사용 가능 */
typedef struct {
	char DataSeq [11];                     /* 메세지일련번호 */
	char Transaction_Code [11];            /* 트랜잭션코드 */
	char Megrp_no [2];                     /* ME그룹번호 */
	char Trd_Mkt_Choic_Tp_Cd [3];          /* 시장ID */
	char Board_id [2];                     /* 보드ID */
	char MembershipNo [5];                 /* 회원번호 */
	char BranchNo [5];                     /* 지점번호 */
	char OrderNo [10];                     /* 주문ID */
	char OriginalOrderNo [10];             /* 원주문ID */
	char ItemCode [12];                    /* 종목코드 */
	char TradeFlag [1];	   	               /* 매도매수구분코드 */
	char New_Modify_Cancel_gbn [1];        /* 정정취소구분코드 */
	char Bond_Offer_Type_Cd [1];           /* 채권호가종류코드 */
	char AccountNo [12];                   /* 계좌번호 */
	char Ask_Offer_Qty [10];               /* 매도호가수량 */
	char Ask_Offer_Prc [11];               /* 매도호가가격 */
	char Bid_Offer_Qty [10];               /* 매수호가수량 */
	char Bid_Offer_Prc [11];               /* 매수호가가격 */
	char Order_Type [1];                   /* 호가유형코드 */
	char Order_Condition [1];	           /* 호가조건코드 */
	char Investor_Type [4];                /* 투자자구분코드 */
	char Effect_Suspend_Resume_Cd [1];     /* 효력정지재개구분코드 */
	char Order_Mesia_Type [1];             /* 주문매체구분코드 */
	char Order_Identi [12];                /* 주문자식별정보 */
	char Mac_Addr [12];                    /* MAC주소 */
	char Order_Date [8];                   /* 호가일자 */
	char Member_Send_Time [9];             /* 회원사주문시각 */
	char MembershipItem [60];              /* 회원사용영역 */
	char Trdr_Id [5];                      /* 거래원번호 */
	char Mm_Order_Type_No [11];            /* 시장조성자호가구분번호 */
	char Account_Type [2];                 /* 계좌구분코드 */
}	KRX_LP_NOTE_JUMUN_DATA;		/* TCHMOR40001_신규, TCHMOR40002_정정, TCHMOR40003_취소 */

typedef struct {
	char DataSeq            [11];  /* 메세지일련번호 */
	char TrCode             [11]; /* 트랜잭션코드 */
	char MeGrpNo             [2]; /* ME그룹번호 */
	char MktId               [3]; /* 시장ID */
	char MembershipNo        [5]; /* 회원번호 */
	char AccountNo          [12]; /* 계좌번호 */
	char TaxExemptYn         [1]; /* 제재해제구분코드 */
	char OrderMesiaType      [1]; /* 주문매체구분코드 */
	char OrderIdenti        [12]; /* 주문자식별정보 */
	char MacAddr            [12]; /* MAC주소 */
	char SendDate            [8]; /* 신고일자 */
	char TradingTime         [9]; /* 신고시각 */
	char MembershipItem     [60]; /* 회원사용영역 */
}	KRX_NOTE_KILLSWITCH_DATA;

/* ******************** KRX NOTE 응답,거부,자동취소 ******************** */
/* 채권응답,거부,자동취소 (291 bytes)	*/
typedef struct {
	char DataSeq                 [11];  /* 메세지일련번호 */
	char TrCode                  [11];  /* 트랜잭션코드 */
	char Megrp_no                [2];   /* ME그룹번호 */
	char Undly_Asset_Mkt_Id      [3];   /* 시장ID */
	char Board_id                [2];   /* 보드ID */
	char MembershipNo            [5];   /* 회원번호 */
	char BranchNo                [5];   /* 지점번호 */
	char OrderNo                 [10];  /* 주문ID */
	char OriginalOrderNo         [10];  /* 원주문ID */
	char ItemCode                [12];  /* 종목코드 */
	char TradeFlag       		 [1];   /* 매도매수구분코드 */
	char New_Modify_Cancel_gbn   [1];   /* 정정취소구분코드 */
	char AccountNo               [12];  /* 계좌번호 */
	char OrderQuantity           [10];  /* 호가수량 */
	char Price                   [11];  /* 호가가격 */
	char Order_Yield             [13];  /* 호가수익률 */
	char Order_Type              [1];   /* 호가유형코드 */
	char Order_Condition	     [1];   /* 호가조건코드 */
	char Ask_Type                [2];   /* 매도유형코드 */
	char Trust_Principal_Type    [2];   /* 위탁자기구분코드 */
	char Trust_Company_No        [5];   /* 위탁사번호 */
	char Account_Type            [2];   /* 계좌구분코드 */
	char Country_Code            [3];   /* 국가코드 */
	char Investor_Type           [4];   /* 투자자구분코드 */
	char Filler                  [6];   /* 필러값 */
	char Foreign_Investor_Type   [2];   /* 외국인투자자구분코드 */
	char Sm_Bond_Part_Yn         [1];   /* 소액채권장종료매매참여여부 */
	char Tax_Exempt_Yn           [1];   /* 비과세여부 */
	char Order_Mesia_Type        [1];   /* 주문매체구분코드 */
	char Order_Identi            [12];  /* 주문자식별정보 */
	char Mac_Addr                [12];  /* MAC주소 */
	char Order_Date              [8];   /* 호가일자 */
	char Member_Send_Time        [9];   /* 회원사주문시각 */
	char MembershipItem          [60];  /* 회원사용영역 */
	char Order_Accept_Time       [9];   /* 호가접수시각 */
	char Trdr_Id                 [5];   /* 거래원번호 */
	char Real_Modify_Cancel_Cnt  [10];  /* 실정정취소호가수량 */
	char Auto_Cancel_Process_Type[1];   /* 자동취소처리구분코드 */
	char Order_Rejected_Reason   [4];   /* 호가거부사유코드 */
	char Mm_Order_Type_No        [11];  /* 시장조성자호가구분번호 */
}	KRX_NOTE_SETTLE_RESP_DATA;		/* TTRODP41301(정상), TTRODP41302(거부), TTRODP41303(자동취소)등 */


/* LP(조성) 채권응답,거부,자동취소 (295 bytes)	*/
/* 미사용, 협의시 사용가능 */
typedef struct {
	char DataSeq [11];                     /* 메세지일련번호 */
	char Transaction_Code [11];            /* 트랜잭션코드 */
	char Megrp_no [2];                     /* ME그룹번호 */
	char Trd_Mkt_Choic_Tp_Cd [3];          /* 시장ID */
	char Board_id [2];                     /* 보드ID */
	char MembershipNo [5];                 /* 회원번호 */
	char BranchNo [5];                     /* 지점번호 */
	char OrderNo [10];                     /* 주문ID */
	char OriginalOrderNo [10];             /* 원주문ID */
	char ItemCode [12];                    /* 종목코드 */
	char TradeFlag [1];					   /* 매도매수구분코드 */
	char New_Modify_Cancel_gbn [1];        /* 정정취소구분코드 */
	char Bond_Offer_Type_Cd [1];           /* 채권호가종류코드 */
	char AccountNo [12];                   /* 계좌번호 */
	char Ask_Offer_Qty [10];               /* 매도호가수량 */
	char Ask_Offer_Prc [11];               /* 매도호가가격 */
	char Ask_Offer_Yield [13];             /* 매도호가수익률 */
	char Bid_Offer_Qty [10];               /* 매수호가수량 */
	char Bid_Offer_Prc [11];               /* 매수호가가격 */
	char Bid_Offer_Yield [13];             /* 매수호가수익률 */
	char Order_Type [1];                   /* 호가유형코드 */
	char Order_Condition [1];			   /* 호가조건코드 */
	char Investor_Type [4];                /* 투자자구분코드 */
	char Effect_Suspend_Resume_Cd [1];     /* 효력정지재개구분코드 */
	char Order_Mesia_Type [1];             /* 주문매체구분코드 */
	char Order_Identi [12];                /* 주문자식별정보 */
	char Mac_Addr [12];                    /* MAC주소 */
	char Order_Date [8];                   /* 호가일자 */
	char Member_Send_Time [9];             /* 회원사주문시각 */
	char MembershipItem [60];              /* 회원사용영역 */
	char Order_Accept_Time [9];            /* 호가접수시각 */
	char Trdr_Id [5];                      /* 거래원번호 */
	char Auto_Cancel_Process_Type [1];     /* 자동취소처리구분코드 */
	char Order_Rejected_Reason [4];        /* 호가거부사유코드 */
	char Mm_Order_Type_No [11];            /* 시장조성자호가구분번호 */
	char Account_Type [2];                 /* 계좌구분코드 */
}	KRX_LP_NOTE_SETTLE_RESP_DATA;	/* TTRMOP41301(정상), TTRMOP41302(거부), TTRMOP41303(자동취소)등 */


/* ************************** KRX NOTE 체결 **************************** */
/* 채권체결 (317 bytes)	*/
typedef struct {
	char DataSeq                 [11];  /* 메세지일련번호 */
	char TrCode                  [11];  /* 트랜잭션코드 */
	char Megrp_no                [2];   /* ME그룹번호 */
	char Undly_Asset_Mkt_Id      [3];   /* 시장ID */
	char Board_id                [2];   /* 보드ID */
	char MembershipNo            [5];   /* 회원번호 */
	char BranchNo                [5];   /* 지점번호 */
	char OrderNo                 [10];  /* 주문ID */
	char OriginalOrderNo         [10];  /* 원주문ID */
	char ItemCode                [12];  /* 종목코드 */
	char Trading_No              [11];  /* 체결번호 */
	char Trading_Yield           [13];  /* 체결수익률 */
	char Trading_price           [11];  /* 체결가격 */
	char Trading_Volumn          [10];  /* 체결수량 */
	char Session_Id              [2];   /* 세션ID */
	char Trading_Date            [8];   /* 체결일자 */
	char Trading_Time            [9];   /* 체결시각 */
	char TradeFlag				 [1];   /* 매도매수구분코드 */
	char Ask_Type                [2];   /* 매도유형코드 */
	char AccountNo               [12];  /* 계좌번호 */
	char Bond_Order_Type         [1];   /* 채권호가종류코드 */
	char OrderQuantity           [10];  /* 호가수량 */
	char Price                   [11];  /* 호가가격 */
	char Order_Yield             [13];  /* 주문수익률 */
	char Trust_Principal_Type    [2];   /* 위탁자기구분코드 */
	char Trust_Company_No        [5];   /* 위탁사번호 */
	char Account_Type            [2];   /* 계좌구분코드 */
	char Investor_Type           [4];   /* 투자자구분코드 */
	char Filler                  [6];   /* 필러값 */
	char Foreign_Investor_Type   [2];   /* 외국인투자자구분코드 */
	char Order_Mesia_Type        [1];   /* 주문매체구분코드 */
	char Order_Identi            [12];  /* 주문자식별정보 */
	char Mac_Addr                [12];  /* MAC주소 */
	char Effect_Suspend_Resume   [1];   /* 효력정지재개구분코드 */
	char MembershipItem          [60];  /* 회원사용영역 */
	char Trdr_Id                 [5];   /* 거래원번호 */
	char Settle_Date             [8];   /* 결제일자 */
	char Mm_Order_Type_No        [11];  /* 시장조성자호가구분번호 */
	char Last_Ask_Bid_Type       [1];   /* 최종매도매수구분코드 */
}	KRX_NOTE_SETTLE_DATA;		/* TTRTDP42301(체결결과) */

/* **************************** KRX NOTE 주문 ************************** */
typedef struct {
	KRX_HEADER				Header;
	KRX_NOTE_JUMUN_DATA		JumunData[1];				/* 6 -> 1 */
}	KRX_NOTE_JUMUN_Q_FMT;		/* 주문 */

/* ************************** KRX NOTE 응답 **************************** */
/* KRX 차세대 매매 - 주문응답 : 호가입력, 대량호가입력 (15 bytes)   */
typedef struct {
	char	ErrCode[4];                     /* 거부 사유코드        */
	char	DataSeq[11];                    /* 일련번호             */
/*	char	ReceivingTime[9];                  메시지 도달시간			*/
}	KRX_NOTE_JUMUN_R_DATA;			/* 주문수신 from KRX */

/* 거래소 주문응답 Format	*/
typedef struct {
	KRX_HEADER          	Header;
	KRX_NOTE_JUMUN_R_DATA	ReplyData[1];	/* 6 -> 1 */
}	KRX_NOTE_JUMUN_R_FMT;			/* 응답,거부,자동거부 수신 from KRX */
/* ************************** KRX NOTE 응답 **************************** */

/* ************************** 내부응답처리 NOTE ************************ */
/* KRX 응답 업무계 송신(82+4+11+300)*n */
typedef struct {
	KRX_HEADER          		Header;		/* 82 */
	KRX_NOTE_JUMUN_R_DATA    	ReplyData;	/* 4+11 */
/*	KRX_NOTE_JUMUN_DATA      	JumunData;	   254 */
	char	R_Data[300];					/* 채권일반처리호가 291, 채권조성처리호가 295 */
											/* KillSwitch처리161(정상/거부) */
}	KRX_NOTE_JUMUN_S_FMT;

typedef struct {
	KRX_HEADER              	Header;
	KRX_NOTE_SETTLE_RESP_DATA	SettleRespData[15];
}	KRX_NOTE_SETTLE_RESP_FMT;
/* ************************** 내부응답처리 NOTE ************************ */
/* ************************** 체결 NOTE ************************ */
typedef struct {
    KRX_HEADER          Header;
    KRX_NOTE_SETTLE_DATA     SettleData[15];
}   KRX_NOTE_SETTLE_FMT;
/* ************************** 체결 NOTE ************************ */


/* 채권은 전송사이즈가 2개다 그래서 이렇게 처리한다. */
typedef struct {
	char DataSeq					[11];	/* 메세지일련번호	*/
	char Data						[289];	/* 300-11			*/
}	KRX_NOTE_ALL_JUMUN_DATA;		/* 주문 */

typedef struct {
	KRX_HEADER					Header;
	KRX_NOTE_ALL_JUMUN_DATA		JumunData[1];
}	KRX_NOTE_ALL_JUMUN_Q_FMT;		/* 주문 */
/* ****************************** 채권_End *************************** */




/* ********************************************************************** */
/* ****************************** IMECO_Start *************************** */
/* IMECO FEP Header (20 bytes) */
typedef struct {
    char            Length[4];                  /* Header를 제외한 Packet길이 (예, 20+80->80)   */
    char            MsgType[1];                 /* 업무구분식별자 (L:Logon, H:Heartbeat, D:주문,응답,체결)  */
    char            ResponseCode[4];            /* 응답코드 '0000'이외는 오류       */
    char            SeqNo[10];                  /* 전문일련번호; TCP port (접속응답)*/

    char            MsgCount[1];                /* '0' 상수로 사용                  */
}   IMECO_HEADER;

/* 112 byte */
typedef struct {
 	char	Order_Message_Type			[ 1];	/* ‘S’-Single Order			*/
	char	Board_id					[ 2];	/* KRX Board ID				*/
	char	OrderNo						[10];	/* Order ID					*/
	char	OriginalOrderNo				[10];	/* Amend/Cancel KRX Original ID, New Order Space Set	*/
	char	ItemCode					[12];	/* KRX ISIN Code			*/
    char    TradeFlag					[ 1];	/* 매도매수구분코드 1:매도,2:매수	*/
	char	New_Modify_Cancel_gbn		[ 1];	/* 정정취소구분코드 (1:신규,2:정정,3:취소)	*/
	char	AccountNo					[12];	/* Account Number			*/
	char	Order_Quantity				[10];	/*  Order Quantity			*/
	char	Price						[11];	/* Order Price				*/
	char	Order_Type					[ 1];	/* ‘T’-Market, ‘2’-Limit, ‘I’-Limit To Market, ‘W’-Most Advantage Limit	*/
												/*	2(O) 지정가 (Limit)                        
													T(O) 가격제한시장가 (파생은 2_시장가 대신 T)
													W(O) 가격제한최유리지정가 (파생은 X_최유리지정가 대신 W)
													I(O) 조건부지정가 (Limit To Market)

													1(X) 시장가 (Market)                       
													4(X) 스톱지정가(StopLimit)                 
													M(X) 중간가(Midpoint)                      
													V(X) 경쟁대량 (현물만 해당, 값 변경 : 3->V)
													X(X) 최유리지정가                          
													Y(X) 최우선지정가 (현물만 해당)	*/
	char	Order_Condition				[ 1];	/* ‘0’-FAS, ‘3’-IOC, ‘4’-FOK	*/
	char    Order_Identi				[12];	/* IP Information, 주문자식별정보	*/ 
												/* 공인IP, 사설IP, 단말고유번호, 전화번호, 가입자번호, 기계번호
												 * 전화번호의 경우 "-" 제외 
												 * IP의 경우 "." 제외
												 * 반대매매주문의 경우에는 "BANDAE",
													ARS주문의 경우에는 "ARS",
													대량거래(K-BLOX협상)의 경우에만 "BLOCK" 으로 입력
												 * 선물옵션기본예탁금액 관련하여 회원의 고객주문 직권취소의 경우에는 "OVER"
												 * Kill Switch 입력 시 "KILLSWITCH" */
	char    Program_Trading_Type		[ 2];   /* Program Trading Type Code
													PT구분코드					*/
												/* 1) 현물
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
													* 현물 공매도가격제한 예외 적용 대상
													  (증권그룹 ST,SC,RT,MF,IF,FS,DR에 대해)
													  11,12,15,16,19,21,23,25
													* 공매도금지시장조치시 호가입력제한 예외 적용 대상 : 21, 22, 23, 25
													* 정정/취소호가인 경우 원호가와 동일한 값이 입력되어야 함(원호가 정합성 체크)
													  --> 단, 파생상품은 원호가와 다른 값을 입력할 수 있음
													* 코넥스는 경매매/일반매매 모두 00: 일반만 입력가능 */
	char	MembershipItem				[20];	/*  Customer Use Area			*/
	char    Algo_Stgy_Type				[ 1];   /* 알고리즘전략구분코드             */
												/*  1 : 일반호가
													2 : 알고리즘호가
													3 : 고속 알고리즘호가
													* 정정 및 취소는 SPACE로 입력   */
	char	Trdr_Sub_Id					[ 2];	/* 부여받은 알고리즘 Sub ID		*/
	char	Ord_Grp_No					[ 2];	/* 그룹핑을 위해 지정한 호가의 그룹번호(숫자, 대문자, 소문자 : 00~99, aa~ZZ)	*/
	char	Smp_Cd						[ 1];	/* ‘0’-해당없음, ‘1’-기존호가 취소, ‘2’-신규호가 취소, ‘3’-양방향 호가 취소	*/
												/* * 경쟁대량매매호가는 0 으로 입력
                                                   * 정정 및 취소는 SPACE로 입력
                                                   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함   */
}	IMECO_JUMUN_DATA;

/* 회원처리호가(정상주문거부,정정취소 확인/거부) 121 byte */
typedef struct {
	char    Order_Message_Type			[ 1];	/* 회원처리호가 구분		*/
												/* ‘C’-(확인)Confirm,
												   ‘R’-(거부)Reject,
												   ‘A’-(자동취소)Expired(KRX Auto Cancel), 

												   'T'로 주문시 응답인듯
                                                   ‘I’ Change realtime price Limit(Switch right off receiving)
                                                   ‘P’-Change realtime price Limit((Switch after receiving) */
	char    OrderNo						[10];	/* 주문번호									*/
	char    OriginalOrderNo				[10];	/* 원주문번호								*/
    char    ItemCode                    [12];   /* KRX ISIN Code							*/
    char    TradeFlag		            [ 1];   /* 매도매수구분코드 1:매도,2:매수			*/
    char    New_Modify_Cancel_gbn       [ 1];   /* 정정취소구분코드 (1:신규,2:정정,3:취소)  */
    char    AccountNo                   [12];   /* Account Number							*/
    char    OrderQuantity              [10];   /* Order Quantity							*/
    char    Price                       [11];   /* Order Price								*/
    char    Order_Type                  [ 1];   /* ‘T’-Market, ‘2’-Limit, ‘I’-Limit To Market, ‘W’-Most Advantage Limit */
                                                /*  2(O) 지정가 (Limit)
                                                    T(O) 가격제한시장가
                                                    W(O) 가격제한최유리지정가
                                                    I(O) 조건부지정가 (Limit To Market)

                                                    1(X) 시장가 (Market)
                                                    4(X) 스톱지정가(StopLimit)
                                                    M(X) 중간가(Midpoint)
                                                    V(X) 경쟁대량 (현물만 해당, 값 변경 : 3->V)
                                                    X(X) 최유리지정가
                                                    Y(X) 최우선지정가 (현물만 해당)			*/
    char    Order_Condition		        [ 1];   /* ‘0’-FAS, ‘3’-IOC, ‘4’-FOK				*/
	char    Real_Modify_Cancel_Cnt		[10];	/* 실정정취소 호가수량						*/
	char    Auto_Cancel_Process_Type	[ 1];	/* 자동취소처리구분코드						*/
												/*  ‘0’-N/A,
													‘1’-Limit to Market Cancel,
													‘2’-All Cancel							*/
	char    Order_Rejected_Reason		[ 4];	/* 거부사유코드								*/
	char    Program_Trading_Type        [ 2];   /* Program Trading Type Code
                                                    PT구분코드								*/
	char    MembershipItem              [20];   /*  Customer Use Area						*/
	char	Order_Accept_Time			[ 9];	/* 호가접수시각 HHMMSSsss					*/
	char    Trdr_Sub_Id					[ 2];   /* 부여받은 알고리즘 Sub ID					*/
	char    Ord_Grp_No					[ 2];   /* 그룹핑을 위해 지정한 호가의 그룹번호(숫자, 대문자, 소문자 : 00~99, aa~ZZ)	*/
	char    Smp_Cd						[ 1];   /* ‘0’-해당없음, ‘1’-기존호가 취소, ‘2’-신규호가 취소, ‘3’-양방향 호가 취소		*/
                                                /* * 경쟁대량매매호가는 0 으로 입력
                                                   * 정정 및 취소는 SPACE로 입력
                                                   * 정정의 경우 원호가의 정보와 동일한 것으로 인정함   */
}	IMECO_SETTLE_RESP_DATA;

/* IMECO 체결 (121 bytes) 2025 */
typedef struct {
	char	Order_Message_Type			[ 1];	/* ‘T’-Trade				*/
	char	OrderNo						[10];	/* Order ID                 */
	char	ItemCode					[12];	/* KRX ISIN Code            */
	char	AccountNo					[12];	/* Account Number           */
	char	Trading_No					[11];	/* 체결번호					*/
	char	Trading_price				[11];	/* 체결가격					*/
    char	Trading_Volumn				[10];	/* 체결수량					*/
	char	Session_Id					[ 2];	/* 세션ID					*/
	char	Trading_Time				[ 9];	/* 체결시각					*/
	char	Nearby_Trading_Price		[11];	/* 근월물체결가격           */
    char	Future_Trading_Price		[11];	/* 원월물체결가격           */
	char	TradeFlag					[ 1];	/* 매도매수구분코드         */
	char	MembershipItem				[20];	/* Customer Use Area		*/
}	IMECO_SETTLE_DATA;

/* 주문의 거부는 주문회선으로 20만돌려준다. */
typedef struct {
    IMECO_HEADER      		Header;				/*  20 byte */
    IMECO_JUMUN_DATA  		JumunData[1];		/* 112 byte */
}   IMECO_JUMUN_Q_FMT;

typedef struct {
	IMECO_HEADER			Header;				/*  20 byte */
	IMECO_SETTLE_DATA		SettleData[1];		/* 121 byte */
}   IMECO_SETTLE_FMT;

/* ***
	주문 		 : IMECO_JUMUN_DATA			<...>	KRX_JUMUN_DATA
	회원처리호가 : IMECO_SETTLE_RESP_DATA	<...>	KRX_SETTLE_RESP_DATA
	체결 		 : IMECO_SETTLE_DATA		<...>	KRX_SETTLE_DATA
*** */

/* ******************************  IMECO_End  *************************** */
/* ********************************************************************** */


/*----------------------------------------------------------------------*/
/* Server/Client I/O 정의  Struct                                       */
/*----------------------------------------------------------------------*/
/* Client 요청  내부 조회용 DATA_HEADER 50 Byte */
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

/* YSK arb data head - yanta_pa port (byte-exact) */
typedef struct
{
    char  sLength               [ 4]; /* length field�� ������ ����DataSize        */
    char  sSeqNo                [ 8]; /* data������ �Ϸù�ȣ                       */
    char  sDate                 [ 8]; /* �ֹ�����                                  */
    char  sTime                 [ 6]; /* �ֹ��ð�                                  */
    char  sSndTp                [ 2]; /* �۽�ó+LINT����                           */
                                		/* �۽�ó���� : 72(�Ļ�)                    */
    char  sMediaType            [ 3]; /* ��ü(ä��)����                            */
                                		/* 102 ( ������_������� ) , 612 (�߰���_�������) */
    char  sSystemType           [ 2]; /* �ý��۱���(00:����,01:���,02:���)       */
    char  sBpIp                 [15]; /* BP IP                                     */
    char  sFiller1              [ 1]; /* filler1                                   */
    char  sCustLoginId			[ 8];	/* Customer Login ID(HTS ID) space			*/
    char  sLoanDate             [ 8]; /* ��������                                  */
    char  sOnOffGb              [ 2]; /* Online/Offline ����                       */
    char  sOrderType            [ 2]; /* �ֹ� ����                                 */
    char  sFiller2              [ 9]; /* filler2                                    */
    char  sRespTime             [ 9]; /* �ŷ��� ����ð�(HHMMSSsss)                */
    char  sRpCode               [ 4]; /* �ŷ��� �����ڵ�                           */
    char  sRecvTime             [ 9]; /* Agent, ä�� ������ ���� �ð�(HHMMSSsss)   */
} YSK_DATA_HEAD;

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
	char	Ask_Bid_Type_Code[2];	           	/* 매도매수구분         */	
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

KRX_JUMUN_DATA					JmData;
KRX_JUMUN_DATA					*pJmData;

KRX_NOTE_JUMUN_DATA				JmNoteData;
KRX_NOTE_JUMUN_DATA				*pJmNoteData;

KRX_LP_NOTE_JUMUN_DATA			JmLpNoteData;
KRX_LP_NOTE_JUMUN_DATA			*pJmLpNoteData;

IMECO_JUMUN_DATA				JmImecoData;
IMECO_JUMUN_DATA				*pJmImecoData;

#else

extern	KRX_JUMUN_DATA			JmData;
extern	KRX_JUMUN_DATA			*pJmData;

extern	KRX_NOTE_JUMUN_DATA		JmNoteData;
extern	KRX_NOTE_JUMUN_DATA		*pJmNoteData;

extern	KRX_LP_NOTE_JUMUN_DATA	JmLpNoteData;
extern	KRX_LP_NOTE_JUMUN_DATA	*pJmLpNoteData;

extern	IMECO_JUMUN_DATA		JmImecoData;
extern	IMECO_JUMUN_DATA		*pJmImecoData;

#endif

/*------------------------------------------------------------------------
	Compile-time size validation (krx_trcode.h provides STATIC_ASSERT)
------------------------------------------------------------------------*/
#ifdef __KRX_TRCODE_H
STATIC_ASSERT(sizeof(KRX_HEADER)      == 82,  KRX_HEADER_size_mismatch);
STATIC_ASSERT(sizeof(KRX_BODY_COMMON) == 24,  KRX_BODY_COMMON_size_mismatch);
STATIC_ASSERT(sizeof(KRX_MSG_COMMON)  == 106, KRX_MSG_COMMON_size_mismatch);
#endif

/*************************************************************************
	End of Program (pa_struct.h)
*************************************************************************/
#endif
