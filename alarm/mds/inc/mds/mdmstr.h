
#ifndef __MDMSTR_H__
#define __MDMSTR_H__
//#include "mds.h"
//#include "mdfold.h"

//
// Master schema
//
#ifdef __cplusplus
extern "C" {
#endif


/*-------------- Market Data Feed Symbol Master Format ----------------*/
typedef struct {
	char fx_prdct_cd				[ 6];	/* 통화쌍							 */ 
	char cust_repsnt_yn	        	[ 1];   /* 고객거래여부                        */  
	char pinc				      	[26];   /* minimum price incremnet      */
	char fill						[ 2];   /* Filler                        */           	
	char expir_nc_way_dstcd       	[ 1];   /* 만기일수계산방법구분코드                 */ 
	char pind			        	[10];   /* 환율표시자리수                       */  
	char big_exrt_repsnt_nocip    	[10];   /* 큰환율표시자리수                     */ 
	char big_exrt_repsnt_situ     	[26];   /* 큰환율표시위치                       */  
	char fincl_exrt_calc_dstcd    	[ 1];   /* 재정환율계산구분코드                   */ 
	char ordr_rcept_abil_ttm      	[ 6];   /* 주문접수가능시각						*/
} MD_MSTR;


 
typedef	struct	{
	char	symb[SYMB_LEN+SYMB_SUBLEN];	/* Symbol(PK)	   		*/
	char	isym[SYMB_LEN];	/* internal symbol		*/
	int	 	seqn;			/* sequence number(internal)	*/
	char	inrt[SYMB_LEN];	/* internal root symbol			*/
	char	root[SYMB_LEN];	/* root symbol			*/
	double	 pmul;			/* price multipler		*/
	double	 swappmul;			/* price swap multipler		*/
	int	 	xdiv;			/* denominator			*/
	int	 	ydiv;			/* price numerator  		*/
	int	 	zdiv;			/* no of Employ decimals		*/
	int	 	zCustdiv;			/* no of decimals		*/
	int	 	swapzdiv;	/* no of Swap decimals		*/
	int	 	pind;			/* price indicator(internal)	*/
	int	 	swappind;			/* price indicator(internal)	*/
	int	 	aval;			/* price adjust	value(internal)	*/
	int	 	sect;			/* product sector(internal)	*/
	int	 	styp;			/* instrument(stock) type	*/
	double	  base;		/* prior settlement price	*/
	int		pricestat;	/* 현재 가격원천 */
	int 	exrt;     /* 재정환율여부  */
	int	  	jchk;			/* symbol check			*/
	int	 	exid;			/* exchange id			*/
	int		  iNowVirtualAmoutType; /* 현재적용 가상잔량 기준  0 : SMBS, 1: CMBS*/
	int		  iVirtualBidAmoutType; /*기준 Bid가상잔량 산출기준 0:비율 , 1: 고정 */
	double	dBidAMountRate; /*가상잔량 시장대비 Bid비율 */
	double	dBidMinAmount;  /*가상잔량 시장대비 Bid비율적용 최저수치 */
	double  dBidMaxAmount;	 /*가상잔량 시장대비 Bid비율적용 최대수치 */
	double	dBidBaseAmount; /*잔량이 0인경우 CMBS Bid적용수치  */	
	int		  iVirtualAskAmoutType; /*기준 Ask가상잔량 산출기준 0:비율 , 1: 고정 */
	double	dAskAMountRate; /*가상잔량 시장대비 Ask가비율 */
	double	dAskMinAmount;  /*가상잔량 시장대비 비율적용 Ask가최저수치 */
	double  dAskMaxAmount;	 /*가상잔량 시장대비 비율적용 Ask가최대수치 */
	double	dAskBaseAmount; /*잔량이 0인경우 CMBS Ask가적용수치  */	
	int     bUseBeatAmount; /*best 잔량 적용여부				*/
	int		  iVirtualAmoutType; /*가상잔량 산출기준 0:비율 , 1: 고정 */
	double	dVirtualOfferAmout[10];	/*가상잔량 비율 또는 고정수치 	1~9호가 */
	double	dVirtualBidAmout[10];		/*가상잔량 비율 또는 고정수치 	1~9호가 */
	char	exnm[7+1];		/* short exchange name		*/
	char  expiredateSpot[8+1];	/* Expiore Date   */
	char  expireToday[8+1];	/* Expiore Date   */
	char  expireTom[8+1];	/* Expiore Date   */
	char  expiredate1W[8+1];	/* Expiore Date   */
	char  expiredate1M[8+1];	/* Expiore Date   */
	char  expiredate2M[8+1];	/* Expiore Date   */
	char  expiredate3M[8+1];	/* Expiore Date   */
	char  expiredate6M[8+1];	/* Expiore Date   */
	char  expiredate9M[8+1];	/* Expiore Date   */
	char  expiredate12M[8+1];	/* Expiore Date   */
	int   expireNdaySpot;	/* Expiore Date RemainDay  */
	int	  	expireNToday;	/* Expiore Date RemainDay  */ 
	int		expireNTom;	/* Expiore Date RemainDay  */   	
	int   	expireNday1W;	/* Expiore Date RemainDay  */
	int   	expireNday1M;	/* Expiore Date RemainDay  */
	int   	expireNday2M;	/* Expiore Date RemainDay  */
	int   	expireNday3M;	/* Expiore Date RemainDay  */
	int   	expireNday6M;	/* Expiore Date RemainDay  */
	int   	expireNday9M;	/* Expiore Date RemainDay  */ 
	int   	expireNday12M;	/* Expiore Date RemainDay  */
	int  	nAmountUnit;	/* 잔량 기본 표시 단위 잔량/nAmountUnit */
	int   	nOrderrecy_ttm;/* 주문수량회복시간 */
	char  	dispUnit[1+1];			/* 쟌량표시값 					*/
	
	char	 clrs[SYMB_LEN];	/* clearing symbol		*/
	char	 cfic[7+1];		/* CFI code			*/
	char	 enam[127+1];		/* symbol name(english,full)	*/
	char	 snam[127+1];		/* symbol name( ", short)	*/
	char	 knam[127+1];		/* symbol name(korean) 		*/
	int	 	 feed;			/* data feeder id number	*/
	double	 adjv;			/* adjust value				*/
	double	 tval;			/* tick value				*/
	double	 pinc;			/* minimum price increment	*/
	int	 	 trdf;			/* tradable flag		*/
	uint32_t pymd;			/* previous trading day		*/
	uint32_t tymd;			/* current trading day		*/
	uint32_t zymd;			/* expire day(last trading date)*/
	int      smbstime;		/*smbs시작시간 */
	int		 cmbstime;		/*cmbs시작시간 */
	int		 stoptime;      /* 종료시간     */
	
} MDMSTR;



typedef	struct	{
	char	symb[SYMB_LEN+SYMB_SUBLEN];	/* Symbol(PK)	   		*/
	char	isym[SYMB_LEN];	/* internal symbol		*/
	int	 	seqn;			/* sequence number(internal)	*/
	char	inrt[SYMB_LEN];	/* internal root symbol			*/
	char	root[SYMB_LEN];	/* root symbol			*/
	char	gsym[SYMB_LEN];	/* security group code		*/
	char	clrs[SYMB_LEN];	/* clearing symbol		*/
	char	cfic[7+1];		/* CFI code			*/
	char	enam[127+1];		/* symbol name(english,full)	*/
	char	snam[127+1];		/* symbol name( ", short)	*/
	char	knam[127+1];		/* symbol name(korean) 		*/
	char	ecym[15+1];		/* contract year-month(english)	*/
	char	kcym[15+1];		/* contract year-month(korean)	*/
	int	 	exid;			/* exchange id			*/
	char	exnm[7+1];		/* short exchange name		*/
	int	 	stat;			/* trading status		*/
	int	 	trdf;			/* tradable flag		*/
	int	 	unpd;			/* underlying product		*/
					/* CME : 2=Agriculture		*/
					/*       4=Currency		*/
					/*	 5=Queity		*/
					/*      12=Other		*/
					/*	14=Interest Rate        */
					/*      15=FX Cash              */
					/*      16=Energy		*/
					/*      17=Metal		*/
	char	 unps[SYMB_LEN];	/* underlying product symbol	*/
	double	 pmul;			/* price multipler		*/
	int	 	xdiv;			/* denominator			*/
	int	 	ydiv;			/* price numerator  		*/
	int	 	zdiv;			/* no of decimals		*/
	int	 	pind;			/* price indicator(internal)	*/
	int	 	aval;			/* price adjust	value(internal)	*/
	int	 	sect;			/* product sector(internal)	*/
	int	 	styp;			/* instrument(stock) type	*/
							/* 'F':Future 'O':Option	*/
	char	 curr[3][3+1];		/* trading, settle, strike	*/
	uint32_t exym;			/* expired year & month(YYYYMM00)*/
	uint32_t lymd;			/* listed date			*/
	uint32_t zymd;			/* expire day(last trading date)*/
	uint32_t jjis;			/* day to last trading day	*/
	uint32_t minv;			/* minimum trading volume	*/
	uint32_t maxv;			/* maximum trading volume	*/
	int	 	csiz;			/* contract size(unit)		*/
	int	 	ndpt;			/* market depth level		*/
	double	adjv;			/* adjust value				*/
	double	tval;			/* tick value				*/
	double	pinc;			/* minimum price increment	*/
	double	uplp;			/* upper limit price		*/
	double	dnlp;			/* lower limit price		*/
	double	base;			/* base price			*/
	double	clos;			/* previous close price		*/
	double		hipr;			/* Contract lifetime high price */
	uint32_t 	hidy;			/* Contract lifetime high day	*/
	double		lopr;			/* Contract lifetime low price	*/
	uint32_t	lody;			/* Contract lifetime low day	*/
	double		hian;			/* annual high price		*/
	uint32_t 	hdan;			/* annual high day		*/
	double		loan;			/* annual low price		*/
	uint32_t 	ldan;			/* annual low day		*/
	int  	opts;         		/* option style        		*/
						/* 'A': american style 		*/
						/* 'E': europe style   		*/
						/* 'X': unknown       		*/
	int	corp; 	         	/* 'C': call 'P': put		*/
	int	atmf;				/* 0:Future 1:ATM, 2:ITM, 3:OTM		*/
								/* conversion right : 230 	*/
	double   strk; 	         	/* strike price        		*/
	double   conv;          	/* conversion ratio    		*/
	double   impv;          	/* implied volatility  		*/
	double   levg;          	/* leverage Factor     		*/
	double   gear;          	/* Gearing             		*/
	double   delt;          	/* Delta               		*/
	double	 gama;			/* Gamma			*/
	double	 vega;			/* Vega				*/
//	double	 teta;			/* Theta			*/
	uint32_t	xxxx;
	uint32_t	kfrhm;		/* trading start time		*/
	struct	 previous {		/* previous quote		*/
		 int	  sect;		/* 전일정산가구분코드 			*/
		 uint32_t symd;		/* most recent settlement date	*/
		 uint32_t pymd;		/* prior settlement date	*/
		 double	  base;		/* prior settlement price	*/
		 double	  open;		/* open price			*/
		 double	  high;		/* high price			*/
		 double	  low;		/* low price			*/
		 double	  last;		/* last price			*/
		 double	  setp;		/* settlement price		*/
		 int	  sign;		/* change sign			*/
		 double	  diff;		/* net change			*/
		 double	  rate;		/* change rate			*/
		 double   tvol;		/* trading volume		*/
		 uint32_t opin;		/* open interest		*/
		 uint32_t cvol;		/* cleared volume		*/
		 uint32_t uymd;		/* updated day by settlement	*/
		 uint32_t uhms;		/* update time by settlement	*/
		 double   upvo;		/* */
		 double   dnvo;		/* */
		 uint32_t upno;		/* */
		 uint32_t dnno;		/* */
	} p;
	struct	trading_hour {
		uint32_t frhm;		/* trading start hh:mm		*/
		uint32_t tohm;		/* trading end hh:mm		*/
		uint32_t hfhm;		/* trading halt time 		*/
		uint32_t hthm;		/* trading halt time 		*/
		uint32_t fwdy;		/* trading start day of week	*/
		uint32_t twdy;		/* trading end day of week	*/
		uint32_t tfhm;		/* T session start time		*/
	} session;			/*				*/
	int	 feed;			/* data feeder id number	*/
	uint32_t pymd;			/* previous trading day		*/
	uint32_t tymd;			/* current trading day		*/
	uint32_t oymd;			/* market open day		*/
	uint32_t cymd;			/* market close day		*/
	uint32_t uymd;			/* updated day			*/
	uint32_t uhms;			/* update time			*/
	uint32_t xage;			/* age counts			*/
	int	 jchk;			/* symbol check			*/
	int	 sind;
	int	 sad1;
	int	 sad2;
	int	 sfmt;
	int	 sdiv;
	double	bidswap[12];		/* swap point bid */
	double  offerswap[12];		/* swap point offer */
} MDMSTR__BACK;

/* exchange-id definition */
#define	EXID_CME	1	/* US : Chicago Mercantile Exchange	*/
#define	EXID_CBOT	2	/* US : Chicago Board of Trade		*/
#define	EXID_NYMEX	3	/* US : NewYork Merchantile Exchange	*/
#define	EXID_EUREX	4	/* EU : EUREX				*/
#define	EXID_SGX	5	/* SG : Singapore Exchange		*/
#define	EXID_HKFE	6	/* HK : Hong Kong Futures Exchange	*/
#define	EXID_OSE	7	/* JP : Osaka Securities Exchange	*/
#define	EXID_TSE	8	/* JP : Tokyo Stock Exchange		*/
#define	EXID_LIFFE	9	/* UK : London Int. Financial Future Ex	*/
#define	EXID_TIFFE	10	/* JP : Tokyo international Financial	*/
#define	EXID_ICE	11	/* US : International commodity Ex 	*/
#define	EXID_SFE	12	/* US : Sidney Futures Ex 		*/
#define	EXID_ETC	99	/* etc							*/

/* sector */
/* --------------------------------------------------
   10: Foreign exchange(Currency)   : Currency(통화)
   20: Interest rate (Financial)    : Iterest(금리)
   30: Index(Equity)            : Index(지수)
   40: Commodity (Agriculture, ...) : Commodity(농축산물)
   50: Metals               : Metals(귀금속)
   51: Metals               : Metals(비철금속)
   60: Energy               : Energy(에너지)
   90: Etc commodity
-------------------------------------------------- */
#define	SECT_ETC	90	/* Etc... 	*/

/* jchk : symbol check bits */
#define	JCHK_LM	0x01	/* Lead month contract				*/
#define	JCHK_CD	0x02	/* lead month code((ex. ES.1)		*/

#ifdef	_SCHEMA_H_
#define OF_MSTR(member)    ((long) &(((MDMSTR*)0)->member))
#define SZ_MSTR(member)    sizeof(((MDMSTR*)0)->member)

struct	keydesc mdmstr_key[] = {
	{ ISNODUPS, 1, 
	  {{ OF_MSTR(symb), SZ_MSTR(symb), CHARTYPE }}},
	{       -1, 0,
	  {{               0,               0,        0 }}},
};
#endif

#ifdef __cplusplus
}
#endif

#endif 
