#ifndef FX_QUOTE_H
#define FX_QUOTE_H

typedef struct _mat_quote_
{
	char	symbol		[ 7];
	double	bidprc;
	double	askprc;
}	MAT_QUOTE;


/* ascii 전문이 일반적이나, 시스템 내부시세이므로 binary 전문사용 (필요시 ascii로 변경해도 무방) */
typedef struct {
	char	type		[ 2];	/* FA,FB SWAP rate 구분 위해                                             */
	char    excode      [ 1];	/* SMB/KMB/EBS/CMB/BEST/ZCUST/MATCHING                                  */
	char    bidex       [ 1];	/* BID원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
	char    askex       [ 1];	/* ASK원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
	char    symb        [ 7];	/* root symbol                                                          */
	char	id			[32];	/* 호가 id                                                              */
	char    date        [ 8];	/* 수신일자 YYYYMMDD (서버시간)                                         */
	char    time        [ 9];	/* 수신시간 HHMMSSSSS                                                   */
	double  usdbid			;	/* Current USDKRW BID                                                   */
 	double  usdask			;	/* Current USDKRW OFFER                                                 */
	double  bidprc          ;	/* Price of the MarketData Entry                                        */
	double  askprc			;	/* Price of the MarketData Entry                                        */
	double  bidqty          ;	/* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
	double  askqty			;	/* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
	double  midprc          ;	/* 중간가                                                               */
	double  fillprc         ;	/* 체결가                                                               */
	time_t	ctime			;	/* time_t convert                                                       */
	time_t	price_time		;	/* 시세 유효 시간                                                       */
	char	filler     [112];	/* 256 byte 맞춤                                                        */
} MATSISE;

typedef struct {
	char	type		[ 2];	/* FA,FB SWAP rate 구분 위해 FB는 거름                                  */
	char    excode      [ 1];	/* SMB/KMB/EBS/CMB/BEST/ZCUST                                           */
	char    bidex       [ 1];	/* BID원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
	char    offerex     [ 1];	/* ASK원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
	char    symb        [ 7];	/* root symbol                                                          */
	char	id			[32];	/* 호가 id                                                              */
	char    date        [ 8];	/* 수신일자 YYYYMMDD (서버시간)                                         */
	char    time        [ 9];	/* 수신시간 HHMMSSSSS                                                   */
	double  usdbid          ;	/* Current USDKRW BID                                                   */
 	double  usdoffer        ;	/* Current USDKRW OFFER                                                 */
	double  bidprc          ;	/* Price of the MarketData Entry                                        */
	double  offerprc        ;	/* Price of the MarketData Entry                                        */
	double  bidqty          ;	/* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
	double  offerqty        ;	/* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
	double  midprc          ;	/* 중간가                                                               */
	double  fillprc         ;	/* 체결가                                                               */
} APSISE;
#define     SZ_APSISE       sizeof(APSISE)


typedef struct _sise_entry_
{
	char	MDEntry_Type	[ 2];		/*  269: 0 (bid) or 1 (offer) */
	char	MDEntry_Px		[16];		/*  270: Price of the MarketData Entry. */
	char	MDEntry_Size	[16];		/*  271: Quantity of the MarketData Entry.  Always “0” For USD/KRW, CNH/KRW */
	char	MDEntry_Date	[ 8];		/*  272: Specific date of trade settlement in YYYYMMDD format. */
	char	Quote_EntryID	[32];		/*  299: Unique identifier of this MarketData Snapshot. 299=USDKRW20180325000029883 */
	char	Sett_Type		[ 2];		/* 9063: Always SP (SPOT) Tenor Code. 9063=SP */
	char	MDBest_Px		[16];		/* 9064: Best Price of the MarketData Entry. Only USD/KRW, CNH/KRW. Not tradeable price, reference only 9064=1107.4 */
	char	MDBest_Size		[16];		/* 9065: Best Quantity of the MarketData Entry.  Only USD/KRW, CNH/KRW. Not tradeable quantity, reference only. 9065=1000000 */
}	SISE_ENTRY;

/* FX quote struct */
#define FX_QUOTE_MDENTRY_NUM    2

typedef struct fx_quote_t
{ 
	char	Msg_Type		[ 4];		/*  35: Message type. */
	char	Sender_CompID	[15];		/*  49: Message sender identifier. */
	char	Sending_Time	[24];		/*  52: Sending time (GMT). Timestamp should include milliseconds (YYYYMMDD-HH:MM:SS.sss). */
	char	Symb			[ 8];		/*  55: Primary Currency/Counter Currency */
	char	No_MDEntries	[ 4];		/* 268: Number of entries in the MarketData message. */
	SISE_ENTRY	entry[ FX_QUOTE_MDENTRY_NUM];
} FX_QUOTE_T;
#define FX_QUOTE_SIZE   sizeof(FX_QUOTE_T)

#endif

