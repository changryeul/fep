#ifndef	_MDF_H_
#define	_MDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/msg.h>

#define	SYMB_SZ	7

/*------------------------------------------------------------------*/
/* Start Option format												*/
/*------------------------------------------------------------------*/
typedef struct {
	long	mtyp;				// message type
	int	styp;				// security type	
	int	dtyp;				// data type			
	short	exid;				// exchange id
	char	symb[SYMB_SZ];			// symbol
	int	xdiv;				// 
	int	ydiv;				//
	int	zdiv;				// no of decimal point
} MDFH;

// feed format
struct	master {
	char	type[1];			// 'M'
	char	symb[SYMB_SZ];			// ticke rsymbol
	char	seqn[4];			// symbol sequence
	char	root[12];			// root symbol
	char	gsym[12];			// security group
	char	clrs[12];			// clearing symbol
	char	cfic[6];			// CFI code
	char	enam[128];			// english name
	char	snam[128];			// english short name
	char	knam[129];			// hangul name
	char	exnm[8];			// exchange name
	char	stat[2];			// trading status
	char	unpd[2];			// underlying product
	char	unps[12];			// underlying product symbol
	char	pmul[12];			// price multiplier
	char	xdiv[6];			// price denominator
	char	ydiv[6];			// price numerator
	char	zdiv[6];			// no of decimals
	char	styp[1];			// instrument type : 'F': Future 'O': Option
	char	curr[3][3];			// currency : trading, settle, strike price
	char	exym[8];			// expired year month : YYYYMM00
	char	lymd[8];			// listed date
	char	zymd[8];			// expire dat : last trading day
	char	jjis[4];			// days to last trading day
	char	minv[6];			// minimum trading volume
	char	maxv[6];			// maximum trading volume
	char	csiz[6];			// contract size
	char	ndpt[2];			// market book depth
	char	pinc[12];			// minimum price increment
	char	uplp[12];			// upper limit price
	char	dnlp[12];			// lower limit price
	char	base[12];			// prior settlement price
	char	opts[1];			// option : Style
	char	corp[1];			//          Call or Put
	char	strk[12];			//          Strike Price
	char	feed[4];			// feed number
	char	symd[8];			// prior settlement date
	char	setp[12];			//       settlement price
	char	opin[9];			// open interest
	char	cvol[9];			// cleared volume
};

struct	quote {
	char	type[1];			// 'Q'
	char	symb[SYMB_SZ];			// ticke rsymbol
	char	cMDReqID			[20];    
	char    bidprice         	[12];   /* bid                          */      
	char	bidQty						[12];
	char	biddate						[12];
	char	bidQuoteEntryID		[12];
	char	bidSettType				[12];
	char	bidBestPx					[12];
	char	bidBestSize				[12];
	char	offerprice         	[12];   /* bid                          */      
	char	offerQty						[12];
	char	offerdate						[12];
	char	offerQuoteEntryID		[12];
	char	offerSettType				[12];
	char	offerBestPx					[12];
	char	offerBestSize				[12];
};

struct	books {
	char	type[1];			// 'B'
	char	symb[SYMB_SZ];			// ticke rsymbol
	char	tymd[8];			// trading day
	char	xymd[8];			// date
	char	xhms[6];			// time
	char	kymd[8];			// KOR date
	char	khms[6];			// KOR time
	char	dept[2];			// market depth level
	char	dumy[3];			// dumy
	struct	{
		char pask[12];			// offer price
		char vask[8];			// offer size
		char nask[6];			// no of offer orders
		char pbid[12];			// bid price
		char vbid[8];			// bid size
		char nbid[6];			// no of buy orders
	} book[1];				// depth : *dept
};

struct	settle {
	char	type[1];			// 'S'
	char	symb[SYMB_SZ];			// symbol
	char	tymd[8];			// trrading day
	char	setp[12];			// settlement price
	char	diff[12];			// net change
	char	tvol[9];			// estimated volume
	char	opin[9];			// open interest
};

struct	status {
	char	type[1];			// 'G'
	char	gsym[12];			// security group 
	char	symb[512];			// symbol list
	char	stat[2];			// status code
};
/*------------------------------------------------------------------*/
/* End Option format												*/
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/* Start Futures format												*/
/*------------------------------------------------------------------*/
#define MD_MAX_DEPTH	5
#define	MD_IMAX_DEPTH	10
#define	MD_CMAX_DEPTH	20
#define MD_MAX_IMPLY	2

/*---------------- Market Data Feed Header Format ------------------*/
typedef struct	{			/* Market Data Transaction Header		*/
	char	trxc[3];		/* Transaction Code						*/
							/* T10 : Market Symbol Master					*/
							/* T21 : Market Trade(realtime)-체결	*/
							/* T22 : Market Trade(snapshot)-체결	*/
							/* T31 : Market Depth(realtime)-호가	*/
							/* T32 : Market Depth(snapshot)-호가	*/
							/* T40 : Market SettlePrice-정산가(?)	*/
	char	exch[3];		/* Exchange Code						*/
							/* E01 : CME Group						*/
							/*		(include CME/CBOT/NYMEX)		*/
							/* E02 : Singapore Exchange     		*/
							/* E03 : Hong Kong Exchage    			*/
							/* E04 : European Exchange     			*/
							/* E05 : Intercontinental Exchange		*/
	char	sndf[1];		/* DATA SOURCE							*/
	char	fill[1];		/* filler								*/
	char	date[8];		/* Sending Date (KST)					*/
	char	time[12];		/* Sending Time (KST)					*/
	char	code[16];		/* Market Stock Code 					*/
	char	mseq[10];		/* Message Sequence						*/
} MD_HEAD, *Md_Head;
#define	MD_HEAD_SZ	sizeof(MD_HEAD)

#define MD_TRXC_MAST	"T10"		/* Symbol Master				*/
#define MD_TRXC_OMAST	"T11"		/* Symbol Master				*/
#define	MD_TRXC_TRAD	"T21"		/* Market Trade					*/
#define MD_TRXC_DEPT	"T31"		/* Market Depth					*/
#define MD_TRXC_SETL	"T40"		/* SettlePrice					*/
#define MD_TRXC_STRD	"T22"		/* SnapShot Trade				*/
#define MD_TRXC_SDEP	"T32"		/* SnapShot Depth				*/

#define MD_EXCH_CME		"E01"		/* CME Group					*/
#define MD_EXCH_SGX		"E02"		/* SGX 							*/

#define MD_STAT_A1		"A1"		/* Calculated Price				*/




#define MD_MSTR_SZ	sizeof(MD_MSTR)

/*-------------- Market Data Feed Symbol Master Format ----------------*/
typedef struct {
	char	code[16];		// Market symbol code    
	char	ticd[8 ];		// Commodity(Ticker) code
	char	enam[60];		// Market symbol name    
	char	type[1 ];		// C : Call, P : Put     
	char	exch[3 ];		// Exchange section 
	char	curr[3 ];		// currency code
	char	prod[3 ];		// Product section
	char	pind[1 ];		// Price indicator
	char	ftdt[8 ];		// First trading date            
	char	ltdt[8 ];		// Last trading date(expire date)
	char	fddt[8 ];		// First Delivery date           
	char	lddt[8 ];		// Last  Delivery date           
	char	bsdt[8 ];		// Business date                 
	char	stdt[8 ];		// Settlement date               
	char	setp[15];		// Settlement price              
	char	matm[6 ];		// Maturity MonthYear            
	char	strp[20];		// Strike Price                  
	char	strc[3 ];		// Strike Currency               
	char	pinc[20];		// MinPriceIncrement             
	char	disp[20];		// DisplayFactor                 
	char	cvol[12];		// ClearedVolume                 
	char	oint[12];		// OpenInterestQty               
	char	ucod[16];		// Underlying Symbol Code        
	char	fill[25];		// FILLER                        
} MD_OMSTR;
#define MD_OMSTR_SZ	sizeof(MD_OMSTR)

// prod info
//   P10: Foreign exchange(Currency)  : Currency
//   P20: Interest rate (Financial)   : Iterest
//   P30: Index(Equity)               : Index
//   P40: Commodity (Agriculture, ...): Commodity
//   P50: Metals                      : Metals
//   P60: Energy                      : Energy
//   P80: Single Stock
//   P90: Etc commodity

// pind info 
//   '0' : 1/1          999999999   
//   '1' : 1/10         99999999.9  
//   '2' : 1/100        9999999.99  
//   '3' : 1/1000       999999.999  
//   '4' : 1/10000      99999.9999  
//   '5' : 1/100000     9999.99999  
//   '6' : 1/1000000    999.999999  
//   '7' : 1/10000000   99.9999999  
//   '8' : 1/100000000  9.99999999  
//   'A' : 1/2          99999999'9  
//   'B' : 1/4          99999999'9  
//   'C' : 1/8          99999999'9  
//   'D' : 1/16         9999999'99  
//   'E' : 1/32         9999999'99  
//   'F' : 1/64         9999999'99  
//   'G' : 1/128        999999'999  
//   'H' : 1/256        999999'999  
//   'I' : 0.5/32       999999'99.9 
//   'J' : 0.5/64       999999'99.9 
//   'K' : 0.25/32      999999'99.9

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef	struct	{			/* Market Trade Message					*/
	char	trdt[ 8]; 		/* Traded Date(KST)						*/
	char	trtm[12]; 		/* Traded Time(KST)						*/
	char	last[15]; 		/* Last Traded Price					*/
	char	tqty[ 9]; 		/* Last Traded Quantity					*/
	char	tvol[ 9]; 		/* Last Traded Total Volume				*/
	char	open[15]; 		/* Open Price             				*/
	char	high[15]; 		/* High Price							*/
	char	lowp[15]; 		/* Low Price							*/
	char	stat[ 2]; 		/* Market Status(?)						*/
	char	date[ 8];		/* Settle date							*/
} MD_TRADE, *Md_Trade;

#define	MD_TRADE_SZ	sizeof(MD_TRADE)
#define MDF_TRADE_SZ sizeof(MD_TRADE) - 15

/*-------------- Market Data Feed Depth Data Format ----------------*/
struct book {
	char	bidn[ 8];		/* Bid Number							*/
	char	bidq[ 8]; 		/* Bid Quantity							*/
	char	bidp[15]; 		/* Bid Price							*/
	char	askn[ 8];		/* Ask Number							*/
	char	askq[ 8]; 		/* Ask Quantity							*/
	char	askp[15]; 		/* Ask Number							*/
};

typedef	struct	{			/* Market Depth Message					*/
	char	dpdt[ 8]; 		/* Trading Depth Date(KST)				*/
	char	dptm[12]; 		/* Trading Depth Time(KST)				*/
	struct	book book[MD_MAX_DEPTH+MD_MAX_IMPLY];
} MD_DEPTH, *Md_Depth;

#define	MD_DEPTH_SZ	sizeof(MD_DEPTH)

typedef	struct	{			/* Market Depth Message					*/
	struct	book 	mdep [MD_CMAX_DEPTH];
	struct	book 	idep [MD_MAX_IMPLY];
} MD_BOOK, *Md_Book;

#define MD_BOOK_SZ	sizeof(MD_BOOK)

#define MD_EVNT_TRAD	1
#define	MD_EVNT_DEPT	2
#define MD_EVNT_SETL	4
/*------------------------------------------------------------------*/
/* End Futures format												*/
/*------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif
