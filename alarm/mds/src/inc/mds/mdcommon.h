#ifndef	__MDCOMMON_H__
#define	__MDCOMMON_H__

#define BOOK_LEVEL 10
#define	DISPLAYBOOK_LEVEL 5
#define	SYMB_LEN	7
#define SYMB_SUBLEN	3
#define SYMB_RLEN	SYMB_LEN + SYMB_SUBLEN	
#define MAX_TENNER	10
//가격원천 정의
#define	SOURCE_SMBS        0
#define	SOURCE_CMBS        1
#define	SOURCE_HANDWRTING  2
#define	SOURCE_INITPRICE   3
#define	SOURCE_STOP			   4
#define SOURCE_MARKET	5

#define MARKUPGROUPCNT 4  //마크업 그룹개수 
#define MARKUPEMP 500   //마크업 내부
#define MARKUPPRI 200   //마크업 개인
#define MARKUPENT 100   //마크업 기업
#define MARKUPENT2 50  //마크업 기관
#define MARKUPINDEXEMP  0 //마크업 내부
#define MARKUPINDEXPRI  1 //마크업 개인
#define MARKUPINDEXENT  2 //마크업 기업
#define MARKUPINDEXENT2 3 //마크업 기관
#define MARKUPINDEXALL  4 //마크업 0

#define INDEX1W	  0	
#define INDEX1M	  1	
#define INDEX2M	  2	
#define INDEX3M	  3	
#define INDEX6M	  4	
#define INDEX9M	  5	
#define INDEX12M  6	
#define INDEXTOD  7
#define INDEXTOM  8
#define INDEXSPOT 9


#define CHECKDIFFRATE     10 //전틱대비 이격 버리는 RATE           100/10 = 10%
#define CEHCKSTOPDIFFRATE 2 //전일대비 체크 RATE 차이날경우 시세중단. = 100/2 = 50%
		
//SWAP데이터 
typedef	struct	{
	char	symb						[ 7];
	char	date						[ 8];
	char	time						[ 6];
	char	swapbidTOM	 		[26];
	char	swapofferTOM 		[26];
	char	swapbidSpot	 		[26];
	char	swapofferSpot 	[26];
	char	swapbid1W				[26];
	char	swapoffer1W			[26];
	char	swapbid1M				[26];
	char	swapoffer1M			[26];
	char	swapbid2M				[26];
	char	swapoffer2M			[26];
	char	swapbid3M				[26];
	char	swapoffer3M			[26];
	char	swapbid6M				[26];
	char	swapoffer6M			[26];
	char	swapbid9M				[26];
	char	swapoffer9M			[26];
	char	swapbid12M			[26];
	char	swapoffer12M		[26];
} S_SENDSWAP;

//MARKUP데이터 
typedef	struct	{
	char	symb						[ 7];
	char	date						[ 8];
	char	time						[ 6];
	char	markUpbidTOD	 		[26];
	char	markUpofferTOD 		[26];
	char	markUpbidTOM	 		[26];
	char	markUpofferTOM 		[26];
	char	markUpbidSpot	 		[26];
	char	markUpofferSpot 	[26];
	char	markUpbidSMBSSpot	 		[26];
	char	markUpofferSMBSSpot 	[26];
	char	markUpbid1W				[26];
	char	markUpoffer1W			[26];
	char	markUpbid1M				[26];
	char	markUpoffer1M			[26];
	char	markUpbid2M				[26];
	char	markUpoffer2M			[26];
	char	markUpbid3M				[26];
	char	markUpoffer3M			[26];
	char	markUpbid6M				[26];
	char	markUpoffer6M			[26];
	char	markUpbid9M				[26];
	char	markUpoffer9M			[26];
	char	markUpbid12M			[26];
	char	markUpoffer12M		[26];
} S_SENDMARKUP;


////////////////////////////////
//  호가 정보 
////////////////////////////////

typedef	struct {
	double	 dspotbidMarkup;			  // bid   Spot마크업 
	double	 dspotofferMarkup;			// offer Spot마크업
	double	 dbidMarkup;			      // bid   TNR마크업 
	double	 dofferMarkup;			    // offer TNR마크업
	double	 donedaybidMarkup;			// 테너기준 1일당 bid마크업
	double	 donedayofferMarkup;		// 테너기준 1일당 offer마크업
}ONEDAYMARKUPDATA;

typedef	struct {
	double	   spotask;			// 매도spot호가
	double	   pask;			// 매도호가
	double		 vask;			// 매도호가 잔량
	
} ASK;

typedef	struct {
	double	 spotbid;			// 매수spot호가
	double	 pbid;			// 매수호가
	double 	 vbid;			// 매수호가 잔량
	
} BID;
typedef	struct {
	char 		    basesubcode[3];
	char 		     difftnrday[4];      //테너종목과 차이일자
	char 		     QuotimeStamp[30];  //해당시세 TimeStamp
	char 		     filler			[2];
	double           bidswapperday;		//1일당 Swap이자
	double           offerswapperday;		//1일당 Swap이자
	ONEDAYMARKUPDATA onedayMarkup[MARKUPGROUPCNT];
	double	         cask;			// 매도체결량 
	double 	         cbid;		// 매수체결량
	double 	         bidSpotPrice		;		  //bid spotprice
	double	askSpotPrice		;		  //ask spotprice
	double	bidFowardPoint	;			//bidFowardPoint
	double	askFowardPoint	;			//askFowardPoint
	double	swapAdjestValue ;			//스왑조정계수 
	double	bidMarkup				;			//bidMarkup
	double	askMarkup				;			//askMarkup
	double	bidSpotMarkup		;			//bidSpotMarkup
	double	askSpotMarkup		;			//askSpotMarkup
	int			CustPind				;			//고객 소수점자리수
	int			EmpPind					;			//직원 소수점자리수
	int			OrderAbleStat		;			// 0:주문가능 , 1: 주문불가, 2: 주문불가 + 미체결 취소? 
	//double 	dTickSize				;			//가격최소단위

	ASK	 ask[BOOK_LEVEL];	// 매도호가
	BID	 bid[BOOK_LEVEL];	// 매수호가
	double  bidopenprice;			// BIDopen price
	double  bidhighprice;			// BIDhigh price
	double  bidlowprice;			// BIDlow price	
	double  askopenprice;			// ASKopen price
	double  askhighprice;			// ASKhigh price
	double  asklowprice;			// ASKlow price		
	
} BOOKSTRUCT;


typedef	struct {
	double	cask;			// 매도체결량 
	double 	cbid;		// 매수체결량
	ASK	 ask[BOOK_LEVEL];	// 매도호가
	BID	 bid[BOOK_LEVEL];	// 매수호가
} MDBOOKSTRUCT;



typedef	struct {
	int	 	 nPind;										//가격 소수점
	double spotBidPrice;						//SPOT 매수가겨
	double spotAskPrice;						//SPOT 매도가격
	double nearBidPrice;						//Near 매수가격
	double nearAskPrice;						//Near 매도가격
	double nearBidFowardPoint	;			// bidFowardPoint
	double nearAskFowardPoint	;			// askFowardPoint
	double farBidPrice;							//Far  매수가격
	double farAskPrice;							//Far	 매도가격
	double farBidFowardPoint	;			// bidFowardPoint
	double farAskFowardPoint	;			// askFowardPoint
	double swapSNB;									//Swap Sell and Buy 가격
	double swapBAS;									//Swap Buy  and Sell 가격
	double swapAdjestValue;					// 스왑조정계수 
	double	bidSpotMarkup		;			  //nearbidSpotMarkup
	double	askSpotMarkup		;			  //nearaskSpotMarkup
	double	nearbidSwapMarkup				;		//nearbidSwapMarkup
	double	nearaskSwapMarkup				;		//nearaskSwapMarkup
	double	farbidSwapMarkup				;		//farbidSwapMarkup
	double	faraskSwapMarkup				;		//faraskSwapMarkup
	
}QUOTMSTRUCT;


typedef	struct {
	char symb							   [ 6+1];
	char bidspotbase				[26+1];
	char bidfowordbase1W     [26+1];
	char bidfowordbase1M     [26+1];
	char bidfowordbase2M     [26+1];
	char bidfowordbase3M     [26+1];
	char bidfowordbase6M     [26+1];
	char bidfowordbase9M     [26+1];
	char bidfowordbase12M    [26+1];
	char bidswapbase1W       [26+1];
	char bidswapbase1M       [26+1];
	char bidswapbase2M       [26+1];
	char bidswapbase3M       [26+1];
	char bidswapbase6M       [26+1];
	char bidswapbase9M       [26+1];
	char bidswapbase12M      [26+1];	
	char bidOpenprice       [26+1];
	char bidHighprice       [26+1];
	char bidLowprice        [26+1];

	
	char offerspotbase				[26+1];
	char offerfowordbase1W     [26+1];
	char offerfowordbase1M     [26+1];
	char offerfowordbase2M     [26+1];
	char offerfowordbase3M     [26+1];
	char offerfowordbase6M     [26+1];
	char offerfowordbase9M     [26+1];
	char offerfowordbase12M    [26+1];
	char offerswapbase1W       [26+1];
	char offerswapbase1M       [26+1];
	char offerswapbase2M       [26+1];
	char offerswapbase3M       [26+1];
	char offerswapbase6M       [26+1];
	char offerswapbase9M       [26+1];
	char offerswapbase12M      [26+1];
	char offerOpenprice       [26+1];
	char offerHighprice       [26+1];
	char offerLowprice        [26+1];
		
	char midspotbase				 [26+1];
	char midfowordbase1W     [26+1];
	char midfowordbase1M     [26+1];
	char midfowordbase2M     [26+1];
	char midfowordbase3M     [26+1];
	char midfowordbase6M     [26+1];
	char midfowordbase9M     [26+1];
	char midfowordbase12M    [26+1];
	char midswapbase1W       [26+1];
	char midswapbase1M       [26+1];
	char midswapbase2M       [26+1];
	char midswapbase3M       [26+1];
	char midswapbase6M       [26+1];
	char midswapbase9M       [26+1];
	char midswapbase12M      [26+1];
	char midOpenprice       [26+1];
	char midHighprice       [26+1];
	char midLowprice        [26+1];	
} BASEPRICEINPUT;


//가격원천 정보
typedef	struct {
	char fx_prdct_cd			[6+1];//종목코드            
	char prc_orign_dstic      [1+1];//시세원천구분        
	char smbs_aply_start_hms  [6+1];//smbs시작시간       
	char cmbs_aply_start_hms  [6+1];//cmbs시작시간       
	char exrt_ofer_dscn_hms   [6+1];//죵료시간 중단시간  
}PRICEORIGN;


//체결 정보
struct q_price{
	double      bidopen;
	double      bidhigh;
	double      bidlow;
	double      bidlast;
	double      bidbest;
	double			bidbase;
	double      bidvol;
	double      bidvirvol;
	double      bidbestvol;
	int         bidsign;
	double      biddiff;
	double      bidrate;
	double      bidopin;
	int         bidside;
	int         biddirf;
	int         bidtsid;
	int         bidstat;
	uint32_t		bidotim;
	uint32_t		bidhtim;
	uint32_t		bidltim;
	double      offeropen;
	double      offerhigh;
	double      offerlow;
	double      offerlast;
	double      offerbest;
	double			offerbase;
	double      offervol;
	double      offerbestvol;
	double      offervirvol;
	int         offersign;
	double      offerdiff;
	double      offerrate;
	double      offeropin;
	int         offerside;
	int         offerdirf;
	int         offertsid;
	int         offerstat;
	uint32_t		offerotim;
	uint32_t		offerhtim;
	uint32_t		offerltim;
	
	double      midopen;
	double      midhigh;
	double      midlow;
	double      midlast;
	double      midbest;
	double		midbase;
	double      midvol;
	int         midsign;
	double      middiff;
	double      midrate;
	double      midopin;
	int         midside;
	int         middirf;
	int         midtsid;
	int         midstat;
	uint32_t	midotim;
	uint32_t	midhtim;
	uint32_t	midltim;
	double		dspread;
	double		donedaybidswap;
	double		donedayofferswap;
	ASK	 		ask[BOOK_LEVEL];	// 매도호가
	BID	 		bid[BOOK_LEVEL];	// 매수호가
	ONEDAYMARKUPDATA onedayMarkup[MARKUPGROUPCNT];
};


typedef	struct {
	double	 offerMarkup;			// offer마크업
	double	 offerCMBSMarkup;			// offer마크업CMBS
	double	 offerSMBSMarkup;			// offer마크업SMBS
	double	 bidMarkup;			  // bid마크업
	double	 bidCMBSMarkup;			// offer마크업CMBS
	double	 bidSMBSMarkup;			// offer마크업SMBS
}MARKUPDATA;

struct q_Markup{
	MARKUPDATA			markupSet[MARKUPGROUPCNT];
};

////////////////////////////////
//  호가 정보 
////////////////////////////////
struct  bookmid {           /* Message Inbound      */
	char	PrdctCd           [ 7];	// 상품코드
	char	inExpireDate        [ 8];	// 만기일자
}; 

struct bookmod {
	char		symb				[ 7];			// 종목코드
	char		RealCode	  [13];			// 실시간종목코드
	char    pind      	[ 1];       	// price indicator
	char    cmidlast	  [ 1];       	// last price color (('+', '-', ' ')
	char    midlast	    [15];       	// last price
	char    cbidlast  	[ 1];       	// last price color (('+', '-', ' ')
	char    bidlast	    [15];       	// last price
	char    cofferlast	[ 1];       	// last price color (('+', '-', ' ')
	char    offerlast	  [15];       	// last price	
	char    cdiff	[ 1];       	// net change color
	char    diff	[15];       	// net change           
	char    crate	[ 1];       	// % change color
	char    rate	[12];       	// % change           
	char    copen	[ 1];       	// open price color
	char    open	[15];       	// open price 
	char    chigh	[ 1];       	// high price color
	char    high	[15];       	// high price 
	char    clow	[ 1];       	// low price color
	char    low		[15];       	// low price 
	char    pcls	[15];       	// prev clos price	
	char	difftnrday   [4];			//실시간테너종목과 치이일자
	char	swapperday   [15];    //1일당 swap이자
	char    htim				[ 6];       	// time(HHMMSS)
	struct	{
		char	cpask	[ 1];		// ask price color ('+', '-', ' ')
		char	pask	[15];		// ask price 
		char	vask	[12];		// ask volume 
		char	cpbid	[ 1];		// bid price color ('+', '-', ' ')
		char	pbid	[15];		// bid price 
		char	vbid	[12];		// bid volume 
	} book[5];
	char    bday	[ 8];       	// business day
};

struct bookmod2 {
	char		symb				[ 6];					// 종목코드
	char		RealCode	  [13];					// 실시간종목코드
	char    pind      	[ 1];       	// 직원용 소수점 자리수
	char    cpind      	[ 1];       	// 고객용 소수점자리수
	char    cmidlast	  [ 1];       	// 현재 Mid값 색상
	char    midlast	    [15];       	// 현재 MID값 
	char    cdiff				[ 1];       	// net change color
	char    diff				[15];       	// net change           
	char    crate				[ 1];       	// % change color
	char    rate				[12];       	// % change           
	char    pcls				[15];       	// prev clos price	
	char		difftnrday  [ 4];			//실시간테너종목과 차이일자
	char		swapperday  [15];    	//1일당 swap이자
	char    htim				[ 6];       	// time(HHMMSS)
	struct	{
		char	cpask	[ 1];		// ask price color 
		char	pask	[15];		// ask price 
		char	vask	[12];		// ask volume 
		char	vaskM [12];		// 단위표시 거래량 
		char	cpbid	[ 1];		// bid price color 
		char	pbid	[15];		// bid price 
		char	vbid	[12];		// bid volume 
		char	vbidM [12];		// 단위표시 거래량 
	} book[5];
	char    bday	[ 8];       	// business day
};



typedef	struct	{
	char    		symb[SYMB_RLEN];
	int					sseq;
	int					seqn;
	uint32_t    tymd;
	uint32_t    xymd;
	uint32_t    xhms;
	uint32_t    kymd;
	uint32_t    khms;
	struct      q_price     pricedata;
} S_SENDQUOT;



typedef	struct	{
	char    		symb[SYMB_RLEN]; //종목콬드
	int 			nPind;					 //직원용 pind
	int 			nCpind;					// 고객용 pind
	int				nSwapPind;			// Swap pind
	double			dSwapAdjust;				// Swap조정계수  (far- near) *dSwapAdjust
	int 			nAmountUnit;
	char			dispUnit[1+1];
	char			spotExpireDate[ 8];
} S_CODEINFO;

//만기일에 따른 호가 정보 
int GetBookToDate(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook, int nMarkupType, char ExpiredateType);

//만기일에따른 시세정보(현재는 Spot만 지원) 
int GetQuotToDate(char *symbol, char* Expiredate, S_SENDQUOT* rtQuot);
//만기일에 따른 현재가 
double GetPriceToDate(char * sbGubn, char *symbol, char* Expiredate);
//종목 소수점정보
int GetCodePind(char *symbol);
//스왑데이터 입력
int SwapPriceSetting(int iSource, S_SENDSWAP swapquot);
//시세원천변경, 시세수기입력 
int CustomPriceSetting(int iSource, int iPriceType, char *symbol, char* date, char* time, char* bidPrice, char* offerPrice);
//Swap데이터 정보 
int GetSwapPriceToDate(char *symbol, char* NearExpiredate, char* FarExpiredate, char* sLgenNo, QUOTMSTRUCT* rtQuot, int nMarkupType);

int GetW6112(char *symbol, char* NearExpiredate, char* sndb);
int  GetCodeInfo(char *symbol, S_CODEINFO* sCodeinfo);

double fround(double dbase, int nround);
#endif
