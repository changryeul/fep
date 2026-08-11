/*#####################################################################
// VER1 :
■ TODO : ApiOrdGate(API)에 적용된 각종 관리기준들 적용 필요
	- (채널) 주문금액 (최저/최대)						> mst.mxordvol
	- (세션) 주문한도초과 관리							> req.trdstat
	- (고객) 거래가능 상품 (현재 API는 USDKRW만 가능)	> 고객별 등록된 수수료 정보로 판단
	- (채널) 지원 주문타입 (지정가, 시장가만 가능)		> 하드코딩
	- (채널) 초당 최대 주문 건수						> mast.mxordcnt
	- (채널) 초당 최대 시세 건수						> req.quotinsec
	- (채널) 주문접수 가능시간							> mds.fold.trdt
	- (공통) 주문번호 채번								> oracle.seq
	- (채널) 자동햇지주문 여부							> mast.autohedg
	- (공통) 즉시 체결 여부								> define
	- (공통) IOC/FOK 주문처리 (잔량 취소처리 기능 추가)	> 하드코딩
	- (공통) 주문가격호가의 유효시간					> mast.quotvaltm
#####################################################################*/

#ifndef	__WGATE__
#define	__WGATE__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <math.h>

//################################################################################################
// DEFINE
//################################################################################################

#define		WG_LOG_DIR			"/fslog/kei/wfg"
#define		WG_DAT_DIR			"/fsfile/kei/wfg"

#define		DF_AGR_JNL_PATH		"/fsfile/kei/fxwin/jnl"
#define		DF_AGR_JNL_NAME		"AGR_JNL_EXC"

#define		WG_SHMKEY			0x17001700					// TODO : 임시 define

#define		WGDQ_RCVRQ			"WGRCVRQ"					// Quote Request : FIX  -> Gate
#define		WGDQ_SNDRS			"WGSNDRS"					// Quote Feeding : Gate -> FIX
#define		WGSEM_RCVRQ			0xff160043
#define		WGSEM_SNDRS			0xff160044

#define		SQL_CNT				sqlca.sqlerrd[2]
#define		SQL_NODATA			100

#define		DF_OFF				0
#define		DF_ON				1
#define		DF_ALLOC			2		// 할당 Flag
/*
#define     APLOG_EMERG     0  // system is unusable
#define     APLOG_ALERT     1  // action must be taken immediately
#define     APLOG_CRIT      2  // critical conditions
#define     APLOG_ERR       3  // error conditions
#define     APLOG_WARNING   4  // warning conditions
#define     APLOG_NOTICE    5  // normal but significant condition
#define     APLOG_INFO      6  // informational
#define     APLOG_DEBUG     7  // debug-level messages
*/

#define		DF_C_OFF			'0'
#define		DF_C_ON				'1'

#define		MX_WGREQ			200		// request 마다 쓰레드 기동 : 각 쓰레드가 시세 DQ 읽어서 처리
										// DQ 동시 READ 가능한 idx의 최대 수는 세마포어의 세트 최대치인 SEMMSL=250(현재 설정) 임.
										// -> 멀티캐스트로 수신하면 문제 없음.

#define		DF_MX_SYMB			30
#define		DF_USRKRW			"USDKRW"
#define		DF_USDJPY			"USDJPY"

#define		API_BUY				'1'
#define		API_SEL				'2'

#define		API_ORDTP_MARKET	'1'
#define		API_ORDTP_LIMIT		'2'

#define		API_ORDTF_DAY		'0'
#define		API_ORDTF_IOC		'3'
#define		API_ORDTF_FOK		'4'

#define		API_ORDST_NEW		'0'
#define		API_ORDST_PFIL		'1'
#define		API_ORDST_FILL		'2'
#define		API_ORDST_CNCL		'4'
#define		API_ORDST_REJT		'8'

#define		API_EXST_NEW		'0'
#define		API_EXST_CNCL		'4'
#define		API_EXST_REJT		'8'
#define		API_EXST_FILL		'F'

#define		DF_FXSPT			'S'
#define		DF_FXFWD			'F'
#define		DF_FXSWP			'W'

enum {
	REQTP_ABT	= '1',		// 차익거래 API
	REQTP_API	= '2',		// 일반 API
	REQTP_RFS	= '3',		// RFS
	REQTP_RFQ	= '4'		// RFQ
} REQTP;

#define		STR_REQTP(tp)		(tp=='1' ? "ABT" : tp=='2' ? "API" : tp=='3' ? "RFS" : tp=='4' ? "RFQ" : "N/A")

#define		STR_ORDTP(tp)		(tp=='1' ? "MARKET" : tp=='2' ? "LIMIT" : "N/A")
#define		STR_ORDTF(tp)		(tp=='0' ? "DAY" : tp=='3' ? "IOC" : tp=='4' ? "FOK" : "N/A")
#define		STR_ORDST(tp)		(tp=='0' ? "NEW" : tp=='1' ? "PFIL" : tp=='2' ? "FILL" : tp=='4' ? "CNCL" : tp=='8' ? "REJT" : "N/A")
#define		STR_EXCST(tp)		(tp=='0' ? "NEW" : tp=='4' ? "CNCL" : tp=='8' ? "REJT" : tp=='F' ? "FILL" : "N/A")
#define		STR_STYPE(tp)		(tp=='S' ? "SPOT" : tp=='F' ? "FWD" : tp=='W' ? "SWAP" : "N/A")

enum {
	LV_MUST		= 0,
	LV_ERR		= 3,
	LV_WARN		= 4,
	LV_INFO		= 6,
	LV_DEBUG	= 7
};

enum {
	GB_ORDNO		= 1,
	GB_EXCNO
};

enum {
	TRD_OFF			= 0,
	TRD_ON			= 1
};

#define		BIDPOS				0
#define		ASKPOS				1

#define		SPT_CUR				0					// 0
#define		SPT_USD 			SPT_CUR+1			// 1

#define		SWP_NEAR			SPT_CUR				// 0
#define		SWP_FAR				SWP_NEAR+1			// 1
#define		SWP_USD_NEAR		SWP_FAR+1			// 2
#define		SWP_USD_FAR			SWP_USD_NEAR+1		// 3

#define		MX_REQTP			5			// REQ타입 최대 수 : '1'=ABT(차익), '2'=API, '3'=RFS, '4'=RFQ

#define		GETBID_NEAR(p)		(p->bsmkup[ASKPOS].orgprc + p->bsmkup[BIDPOS].spotmkup + p->bsmkup[BIDPOS].swapmkup + p->bsmkup[ASKPOS].swap)
#define		GETASK_NEAR(p)		(p->bsmkup[BIDPOS].orgprc + p->bsmkup[ASKPOS].spotmkup + p->bsmkup[ASKPOS].swapmkup + p->bsmkup[BIDPOS].swap)

#define		GETBID(p)			(p->bsmkup[BIDPOS].orgprc + p->bsmkup[BIDPOS].spotmkup + p->bsmkup[BIDPOS].swapmkup + p->bsmkup[BIDPOS].swap)
#define		GETASK(p)			(p->bsmkup[ASKPOS].orgprc + p->bsmkup[ASKPOS].spotmkup + p->bsmkup[ASKPOS].swapmkup + p->bsmkup[ASKPOS].swap)

#define		IsRFS(q)			(q->req.reqtp[0] == REQTP_RFS || q->req.reqtp[0] == REQTP_RFQ)
#define		IsEmpty(c)			(c == ' ' || c == 0x00)


// 거의 변동이 없기 때문에 CUST.new.cfg 파일에서 읽어오지 않음.
#define		DF_MDS_EXNM		"CUST"		// "SMBS"
#define		DF_MDS_PORT		30028		// 30020
#define		DF_RFQ_PORT		30050		// RFQ 호가 수신포트
#define		DF_MDS_MCAST	"227.10.20.30"		// 개발2용.   개발1, 운영은 227.10.20.10

#define		DF_AGR_CH		"5"			// #define     CH_SPI       '5'    // client user (Server API) 		// win/src/inc/com/comdef.h
#define		DF_FXGO_ID		"BLP_RFS_BETA"

//################################################################################################
// BUSINESS DEFINE
//################################################################################################
#define		DF_RFS_SEC			60			// RFS 요청 시 시세 제공 시간 (현재 StartFX에서 현/선물은 1분, 스왑은 10분적용)
#define		DF_RFQ_SEC			10			// RFQ 요청 시 시세 제공 후 주문유효시간 (데몬에서 종료거부를 주지만. 방어로직 필요)
#define		DF_MAXFEED			3			// Request별 초당 최대 전송건수 (FXGO 기준 3건/초)
#define		DF_GRACEPERIOD		1			// 호가제공시간과 고객주문시간의 유효시간 갭 (1초?)
#define		DF_AUTOHEDGE_MIN	1000000		// 오토헷지 대상 최소 금액

#define		DF_MX_ORDTIM		232900		// TODO : 주문가능시간 관리(?)
#define		DF_MI_ORDVOL		1000
#define		DF_MX_ORDVOL		1000000
#define		DF_MX_ORDSEC		2			// 세션별 초당 최대 주문건수 (추후 세션별로 관리하기 위해 별도 변수로 분리)

#define		DF_REJ_KBEMAL		" : helpdesk@kb.com"

enum {
//------------------------------------------------------------------
// FXGO 거부코드
//------------------------------------------------------------------
	// 35=b 거부코드
	ERR_FXGO_b_SYMB		=   1,	// Unknown Symbol Unsupported Product (Category F)
	ERR_FXGO_b_LMT		=  17,	// Insufficient Credit Limit Credit (Category A)
	ERR_FXGO_b_OTHR		=  99,	// Other Exceptional (Category G)
	ERR_FXGO_b_PRC		= 106,	// No Available Prices Pricing outage (Category B)
	ERR_FXGO_b_REGULA	= 107,	// Failed Regulatory Requirements Regulatory (Category C)
	ERR_FXGO_b_RSKLMT	= 108,	// Risk Limits Constraints (or “Risk Limit Breached”) Risk and capital constraints (Category D)
	ERR_FXGO_b_DATA		= 109,	// Failed Static Data Checks Static Data (Category E)
	
	// 35=8 거부코드
	ERR_FXGO_8_STALE	=   8,	// Last Look Latency (Category A-2)
	ERR_FXGO_8_PRC		=  19,	// Pricing/Liquidity Unavailable (Category B)
	ERR_FXGO_8_LMT		=  25,	// Insufficient Credit Limit, Credit (Category C)
	ERR_FXGO_8_ETC		=  99,	// Exceptional (Category E)
	ERR_FXGO_8_LAST		= 100,	// Last Look (Category A-1)
	ERR_FXGO_8_DATA		= 101,	// Failed static data checks, Static Data (Category D)

//------------------------------------------------------------------
// 자체 거부코드 (일반 API도 동일하게 적용)
//------------------------------------------------------------------
	// SYSTEM
	ERR_SYS_FAULT		= (-1000),
	ERR_SYS_CLOSE		= (-1001),
	// QUOTE REQUEST      
	ERR_REQ_DUP			= (-1100),
	ERR_REQ_USER		= (-1101),
	ERR_REQ_MAX			= (-1102),
	ERR_REQ_SYMB		= (-1103),
	ERR_REQ_DATA		= (-1104),
	// ORDER CHECK        
	ERR_ORD_STALE		= (-1200),
	ERR_ORD_PRC			= (-1201),
	ERR_ORD_DATA		= (-1202),
	// EXECUTION REJECT   
	ERR_EXC_LMT			= (-1300),
	ERR_EXC_ETC			= (-1301)
};

typedef struct {
	int		errno_kb;
	int		errno_fxgo;
	char	errmsg[128];
} ERRDEF;

ERRDEF	wgerror[] = {
	// SYSTEM ERROR
	{ ERR_SYS_FAULT,	ERR_FXGO_b_OTHR,	"system error"							},
	{ ERR_SYS_CLOSE,	ERR_FXGO_b_OTHR,	"market is closed"						},

	// QUOTE REQUEST
	{ ERR_REQ_DUP,		ERR_FXGO_b_OTHR,	"duplicate request error"				},
	{ ERR_REQ_USER,		ERR_FXGO_b_OTHR,	"client is not found"					},
	{ ERR_REQ_MAX,		ERR_FXGO_b_OTHR,	"no empty slot for FIX request"			},
	{ ERR_REQ_SYMB,		ERR_FXGO_b_SYMB,	"symbol is not found"					},
	{ ERR_REQ_DATA,		ERR_FXGO_b_DATA,	"wrong input data"						},

	// ORDER CHECK
	{ ERR_ORD_STALE,	ERR_FXGO_8_STALE,	"the quote is not available"			},
	{ ERR_ORD_PRC,		ERR_FXGO_8_PRC,		"the price is not available"			},
	{ ERR_ORD_DATA,		ERR_FXGO_8_DATA,	"wrong input data"						},
	
	// EXECUTION REJECT
	{ ERR_EXC_LMT,		ERR_FXGO_8_LMT,		"credit limit is exceeded"				},
	{ ERR_EXC_ETC,		ERR_FXGO_8_ETC,		"error occured during processing"		},
	{ 0,				0,					""										}
};

//################################################################################################
// SHARED MEMORY MAST STRUCTURE
//################################################################################################
#define		MX_THREAD		10

typedef struct {
	pthread_mutex_t		mutex;			// maxreqpos update 용
	char	bizdate		[ 8];			// 생성일자		>>>> 시스템일자와 다르면 mmap file 초기화. 같으면 continue. (req 별 쓰레드 생성)
	char	updtmkup	[ 6];			// m365 업데이트 시 'HHMMSS'로 마킹 (with mutex api 제공)
/*
	struct _HANDLER {
		pid_t	handler_ord	[MX_THREAD];
		pid_t	handler_exc	[MX_THREAD];
		pit_t	handler_rfq	[MX_THREAD];
	} handler;
*/
	int		mxreqpos	;				// WGREQ 최대 위치
} WGMST;

typedef struct {
	char	targetcompid	[ 32];		// PK // Tag56=TargetCompID (channel)
	char	sendcompid		[ 32];		// PK // Tag49=SenderCompID (channel)
	// ◆◆◆ FXGO : Liquidity Taker Deal Code(Tag448=PartyID) : FXGO에서 sendsubid 미사용이므로 DealCode값을 sendsubid로 사용
	char	sendsubid		[ 32];		// PK // 
	// ◆◆◆ FXGO : UUID (Tag803(PartySubIDType)='2' && Tag523(PartySubID)) : 사용자를 구분하는 UUID를 userid로 사용
	char	keysymb			[  6];		// PK // symb
//	char	userid			[ 32];		// PK // Tag803(PartySubIDType)='2' && Tag523(PartySubID)		// 추후 필요하면 주문필드에 추가
} WGKEY;

#define		WG_KEYLEN		sizeof(WGKEY)	// 128 bytes
#define		MX_QUOTS		(DF_MAXFEED*DF_GRACEPERIOD + 5)
// SharedMemory : WGMST + WGREQ[]	// MAX = MX_WGREQ
typedef struct {
	WGKEY	wgkey		;

	int		useyn		;			// '1'=USE, '0'=N/A
//	pthread_mutex_t		mutex;		// for update "useyn" : 추가 삭제 시
//===================
	char	tag35		[  2];		// 수신한 Tag35
	char	reqID		[ 64];		// API:내부생성, RFS/RFQ:Tag117=QuoteID (Tag131=QuoteReqID, Tag262=MDReqID)
// 주문 시 상품의 환율에 포함된 spotmarkup, swapmarkup, swappoint, margin 을 모두 전달해야 하며, 재정인 경우는 두 쌍이 필요 함.
// 단 swap 상품인 경우는 near, far 두 상품의 최종 가격만 있으면 되며, 재정인 경우 각각의 near, far 모두 필요 함.
// RFS 인경우는 req 요청시 조회한 밴드정보와 마크업 값으로 계산하고, Stream 인 경우는 실시간으로 m365 조회하여 계산함.
	// SYMBOL
	char	stype		[  1];		// Tag167=SecurityType (S:FXSPOT,F:FXFWD,W:FXSWAP)
	char	symb		[  6];		// SYMBOL
	char	symb4r		[  6];		// SYMBOL for Realdata (재정인 경우는 usdkrw를 제외한 나머지 통화코드)
	char	todymd		[  8];		// TODAY
	char	tomymd		[  8];		// TOM
	char	sptymd		[  8];		// 주문통화의 스팟일자 (재정인 경우 포함)
	int		crosyn		;			// 재정통화여부
	int		usdpos		;			// 재정통화계산방식
	int		zCustdiv	;			// 재정통화의 소수점자리수

	// MARKUP INFO
	char	updtmkup	[  6];		// 'HHMMSS' : wgmst.updtmkup 과 다르면 setreq_user()호출로 markup 재설정 (각 쓰레드에서 수행)
	char	mkupgroup	[  3];		// 마크업 그룹정보 (밴드정보 포함?)
	double	margin		;			// 수수료는 최종 환율에 합산함.		(BUY기준으로 조회된 수수료임. SELL인 경우 뺴줘야 함)
	struct	_markup {
		char	symb		[  6];		// SYMBOL
		char	setldate	[  8];		// 결제일(YYYYMMDD)
		char	tnr_dstcd	[  2];		// 테너구분자 : TOD=00, TOM=01, SPT=00, 99
		struct {
			double	spotmkup	;		// spot markup
			double	swapmkup	;		// swap markup
			double	swap		;		// swap point
			double	orgprc		;		// 수신 원가 (SPOT가격)
			double	prc			;		// spotmarkup + swapmarkup + swappoint 까지 적용된 price
		} bsmkup	[2];				// bid:markup[0], ask:markup[1]
	} markup	[4];					// markup[0][1]:near, markup[2][3]:far

	// USER INFO
	struct	_user {		// 요청이 올 때마다 조회해야 하므로 DB에서 정보관리
		char	lgen_no		[ 10];		// LGEN_NO
		char	acno		[ 20];		// Dealer ID
		char	userid		[ 32];		// Tag803(PartySubIDType)='2' && Tag523(PartySubID)
		char	trdstat		[ 1];		// 거래상태 구분 ('1'=주문가능, '0'=주문불가) (관리자 중지, KILL SWITCH)
		char	lmtstat		[ 1];		// 주문한도 가능여부 ('1'=주문가능, '0'=한도초과)
		char	autohedge	[ 1];		// 자동햇지 구분 (1백만불 이상, 스팟 인 경우만)
		char	excutenow	[ 1];		// 즉시체결 여부
//		char	orgquot		[ 1];		// 마크업/수수료 없는 원가제공 여부
		struct	timeval	ordtm	[DF_MX_ORDSEC];	// 직전,전직전 주문시간
	} user;
 
	// RFS,RFQ INFO
	struct	_req {
		char	reqtp		[  1];		// '1'=ABT(차익), '2'=API, '3'=RFS, '4'=RFQ
		char	rfskey		[ 30];		// RFS/RFQ 키 (W6109A04 에서 채번)
		char	orderno		[ 10];		// 주문번호								// RFS,RFQ에서는 단수로 주문 관리 (복수주문 거부처리), API는 복수주문가능.
		double	orderqty	;			// 밴드구분 기준 값 (ex. 주문금액)
		char	currency	[  3];		// 주문통화
		char	side		[  1];		// '0'=2way, '1'=buy, '2'=sell
		char	etime		[  9];		// quote end time (HHMMssSSS)
	} req;

// RFS의 주문가격검증용, API의 현재가 조회용
	// QUOTE INFO
	int		quotseq		;				// 해당 request 를 통해 전송된 시세 건수

	struct	_quot {						// 고객에게 제공한 시세 (3초 분량) : 주문가격검증 및 체결보장을 위한 히스토리
		char	quotID	[ 32];			// 발송한 last QuoteID (Tag117=QuoteID)
		long	bid		;				// 소수점 이슈를 해결하기 위해 zCustdiv 만큼 10 거듭제곱한 값으로 저장
		long	ask		;
		long	bid2	;				// 소수점 이슈를 해결하기 위해 zCustdiv 만큼 10 거듭제곱한 값으로 저장
		long	ask2	;
		struct timeval	qtm	;			// 시세송신시간 (초당 건수관리를 위해)
	} quot	[MX_QUOTS];

	char	regtime		[  9];		// quote start time (HHMMssSSS)
	char	echoTags	[128];		// echo tags to send back

} WGREQ;

//################################################################################################
// INTERFACE STRUCTURE
//################################################################################################
enum {
	MSG_QTREQ		= 'R',		// Quote Request(subscribe/unsubscribe)
	MSG_QTRES		= 'S',		// Quote Response
	MSG_QUOTE		= 'Q',		// Quote Feed

	MSG_ORDER		= 'O',		// New/Amend/Cncl Order
	MSG_ORDEXC		= 'E'		// Order Ack/Execution
} MSGTYPE;

//######################################################################################
// 통신패킷
//######################################################################################
typedef struct {
	char	msgtp		[  1];		// 'R' Quote Request(subscribe/unsubscribe)
									// 'S' Quote Response(reject)
									// 'Q' Quote Feed
									// 'O' New/Amend/Cncl Order
									// 'E' Order Ack/Execution
	WGKEY	wgkey		;
	char	data		[  1];		// data pointer for : WG_QUOT_REQ / WG_QUOT_RES / WG_QUOT / WG_ORD / WG_ORD_EXC
} WGCOM;

// Quote 요청/해제 구조체
typedef struct {
	char	tag35		[  2];		// 35(MsgType) // (R = QuoteRequest, Z = QuoteCancel, V = MarketDataRequest)
	char	reqtp		[  1];		// '1'=ABT(차익), '2'=API, '3'=RFS, '4'=RFQ
	char	subtp		[  1];		// 1:subscribe,  2:unsubscribe
	char	reqID		[ 64];		// 131(QuoteReqID) // API:내부생성, RFS/RFQ:Tag117=QuoteID (Tag131=QuoteReqID, Tag262=MDReqID)
	char	account		[ 20];		// Dealer ID
	char	stype		[  1];		// 167(SecurityType) // (S:FXSPOT,F:FXFWD,W:FXSWAP)
	char	symb		[  6];		// PK // WGKEY + SYMBOL
	char	setldate	[  8];		// 64(FutSettDate) // 결제일(YYYYMMDD) (swap 인 경우 near)
	char	setldate2	[  8];		// 193(FutSettDate2) // SWAP 인 경우 far 결제일(YYYYMMDD)
	char	side		[  1];		// 54(Side) // '0'=2way, '1'=buy, '2'=sell
// for FXGO RFS
	char	orderqty	[ 16];		// 38(OrderQty) // RFS, RFQ 인 경우 필수
	char	currency	[  3];		// 15(Currency)
	char	echoTags	[128];		// echo tags to send back
} WG_QUOT_REQ;
#define		SZ_WG_QUOT_REQ		sizeof(WG_QUOT_REQ)

// 거부 또는 (제공)종료인 경우 전송
typedef struct {
	char	tag35		[  2];		// 송/수신 Tag35
	char	reqID		[ 64];		// 131(QuoteReqID/R), 262(MDReqID/V)
	char	rejcd		[  8];		// 300(QuoteRejectReason) // 거부코드
	char	rejtext		[128];		// 58(Text) // 거부사유
	char	echoTags	[128];		// echo tags to send back
} WG_QUOT_RES;
#define		SZ_WG_QUOT_RES		sizeof(WG_QUOT_RES)

// TODO : FXGO에 전송하는 패킷 중 KB정보에 해당하는 TAG는 FIX엔진에서 ADD (Tag448=PartyID 등)
typedef struct {
	char	tag35		[  2];		// 송신 Tag35
	char	reqID		[ 64];		// 131(QuoteReqID/R), 262(MDReqID/V)
	char	quotID		[ 32];		// 117(QuoteID) // 데몬에서 채번한 uniq quote id
	char	account		[ 20];		// Dealer ID
	char	stype		[  1];		// 167(SecurityType) // (S:FXSPOT,F:FXFWD,W:FXSWAP)
	char	symb		[  6];		// PK // WGKEY + SYMBOL
	char	setldate	[  8];		// 64(FutSettDate) // 결제일(YYYYMMDD) (swap 인 경우 near)
	char	bid			[ 16];		// 132(BidPx) // bid 호가
	char	ask			[ 16];		// 133(OfferPx) // ask 호가
	// for SWAP
	char	setldate2	[  8];		// 193(FutSettDate2) // SWAP 인 경우 far 결제일(YYYYMMDD)
	char	bid2		[ 16];		// 6050(BidPx2) // SWAP 인 경우 far bid 호가
	char	ask2		[ 16];		// 6051(OfferPx2) // SWAP 인 경우 far ask 호가
	char	bidswap		[ 16];		// 642(BidForwardPoints2)   // FarBid-NearOffer
	char	askswap		[ 16];		// 643(OfferForwardPoints2) // FarOffer-NearBid
	char	timestamp	[ 12];		// 시세 생성시간(데몬) time_t 타입.
	char	echoTags	[128];		// echo tags to send back
} WG_QUOT;
#define		SZ_WG_QUOT			sizeof(WG_QUOT)

// TODO : FXGO에 전송하는 패킷 중 KB정보에 해당하는 TAG는 FIX엔진에서 ADD (Tag448=PartyID 등)
typedef struct {
	char	tag35		[  2];		// 수신 Tag35
	char	quotID		[ 32];		// RFS인 경우 주문에 연동된 시세패킷 QuoteID (Tag117=QuoteID)
	char	account		[ 20];		// Dealer ID
	char	userid		[ 32];		// Tag803(PartySubIDType)='2' && Tag523(PartySubID)

	char	clordid		[ 24];		// TAG-11 기관주문번호     : 기관 주문번호
	char	origclordid	[ 24];		// TAG-41 기관원주문번호   : 기관 원주문번호
	char	ordid		[ 24];		// TAG-37 주문번호         : 데몬에서 채번한 KB주문번호
	char	origordid	[ 24];		// TAG-37 내부원주문번호   : 취소주문(35=F)인 경우 고객이 보낸 KB원주문번호
	char	stype		[  1];		// Tag167=SecurityType or Tag9063=Tenor Code (S:FXSPOT,F:FXFWD,W:FXSWAP)
	char	symb		[  6];		// PK // WGKEY + SYMBOL
	char	setldate	[  8];		// 결제일(YYYYMMDD) (swap 인 경우 near)
	char	price		[ 16];		// 주문가격 (swap 인 경우 near)
	char	side		[  1];		// '0'=2way, '1'=buy, '2'=sell
	char	orderqty	[ 16];		// 밴드구분 기준 값 (ex. 주문금액) : RFS, RFQ 인 경우 필수
	char	currency	[  3];		// Primary Currency
	char	ordtype		[  1];		// 주문유형 : '1'-Market '2'-Limit
	char	timeinforce	[  1];		// 체결조건 : '0'-For Day
	char	setldate2	[  8];		// SWAP 인 경우 far 결제일(YYYYMMDD)
	char	price2		[ 16];		// SWAP 인 경우 far 환율
	char	echoTags	[128];		// echo tags to send back
} WG_ORD;
#define		SZ_WG_ORD			sizeof(WG_ORD)

// TODO : FXGO에 전송하는 패킷 중 KB정보에 해당하는 TAG는 FIX엔진에서 ADD (Tag448=PartyID 등)
typedef struct {
	char	tag35		[  2];		// 송신 Tag35
	char	quotID		[ 32];		// RFS인 경우 주문에 연동된 시세패킷 QuoteID (Tag117=QuoteID)
	char	account		[ 20];		// Dealer ID
	char	userid		[ 32];		// Tag803(PartySubIDType)='2' && Tag523(PartySubID)

	char	clordid		[ 24];		// TAG-11 기관주문번호     : 기관 주문번호
	char	ordid		[ 24];		// TAG-37 주문번호         : 데몬에서 채번한 KB주문번호
	char	origordid	[ 24];		// TAG-37 내부원주문번호   : 취소주문(35=F)인 경우 고객이 보낸 KB원주문번호
	char	stype		[  1];		// Tag167=SecurityType or Tag9063=Tenor Code (S:FXSPOT,F:FXFWD,W:FXSWAP)
	char	symb		[  6];		// PK // WGKEY + SYMBOL
	char	setldate	[  8];		// 결제일(YYYYMMDD) (swap 인 경우 near)

	char	side		[  1];		// '0'=2way, '1'=buy(sell&buy), '2'=sell(buy&sell)
	char	orderqty	[ 16];		// 밴드구분 기준 값 (ex. 주문금액) : RFS, RFQ 인 경우 필수
	char	currency	[  3];		// Primary Currency
	char	price		[ 16];		// 주문가격
	char	ordtype		[  1];		// 주문유형 : '1'-Market '2'-Limit
	char	timeinforce	[  1];		// 체결조건 : '0'-For Day

	char	lastqty		[ 16];		// TAG-32   체결수량         : 
	char	execid		[ 30];		// TAG-17   채결ID           : KB 체결번호 (주문거부인 경우는 시간으로 unique 번호채번하여 보냄)
	char	lastpx		[ 16];		// TAG-31   체결가격         : 
	char	ordstatus	[  1];		// TAG-39   주문상태         : '0'-NEW
	char	exectype	[  1];		// TAG-150  거래유형         : '0'-New
	char	commission	[ 16];		// TAG-12   수수료           : 
	char	setldate2	[  8];		// SWAP 인 경우 far 결제일(YYYYMMDD)
	char	lastpx2		[ 16];		// SWAP 인 경우 far 체결가격 (Tag 6160)
	char	rejcd		[  8];		// 300(QuoteRejectReason) // 거부코드
	char	rejtext		[128];		// 58(Text) // 거부사유
	char	echoTags	[128];		// echo tags to send back
} WG_ORD_EXC;
#define		SZ_WG_ORD_EXC		sizeof(WG_ORD_EXC)


#ifndef _AGRGATE_API_
#define _AGRGATE_API_
//--------------------------------------------------------------------------------
// ## 간단한 하나의 API 이므로 라이브러리로 만들지 않고 헤더에 정의함.
//
// 어그리게이터 데몬(wAgrGate) 마스트메모리에 365 갱신시간 업데이트
// - 호출된 시간으로 업데이트 됨
// - 정상 : return   0
// - 오류 : return < 0
//--------------------------------------------------------------------------------
static inline int wgate_updt_mkuptm(char *emsg)
{
	// SHARED MEMORY ATTACH
	int 	shmid;
	WGMST	*pmast;

	shmid = shmget(WG_SHMKEY, 0, 0);
	if (shmid < 0)	{
		sprintf(emsg, "(%s) create shared memory error (%d/%s)", __func__, errno, strerror(errno));
		return (-1);
	}

	pmast = (WGMST *)shmat(shmid, NULL, 0);
	if (pmast == NULL)	{
		sprintf(emsg, "(%s) shared memory attach error (%d/%s)", __func__, errno, strerror(errno));
		return (-2);
	}

	// MARKUP 갱신시간 마킹
	char	ctime[6+1];
	struct timeval	tmout, tv;
	struct tm *lt;

	gettimeofday(&tv, NULL);
	lt = localtime(&tv.tv_sec);

	sprintf(ctime, "%02d%02d%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
	memcpy(pmast->updtmkup, ctime, sizeof(pmast->updtmkup));
	
	shmdt(pmast);

	return (0);
}
#endif		//_AGRGATE_API_

#endif
