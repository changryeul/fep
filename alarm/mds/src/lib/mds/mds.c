//
// Common schema interface initializer
// Set procedure for market data cooking
//
#include "mymq.h"
#include "context.h"
#define	_SCHEMA_H_
#include "mdfold.h"

#include "axsnd.h"

//void push_marketquot(MARKET *market, char *msgb, int msgl, int zdiv); 
void push_marketquot(MARKET *market, MDQUOT *quot, int zdiv);
//
// folder_init()
// Initialize shared memory
//
static int folder_init(MARKET *market)
{

	MDARCH	*arch;
	MDFOLD *_folder, *folder;
	INDEX	*indx;
	MDMSTR	mstr;
	MDQUOT	quot;
	MDBOOK	book;
	MDTRAD	trad;
	uint32_t ymd, hms;
	int 	nRet = 0;

	if (mds_shminit(market, sizeof(MDFOLD)) != 0)
		return(-1);

	if (market->whoami != I_AM_COOKER)
		return(0);

	arch = market->arch;				// archive for shared memory
	mds_log(market, LOG_MUST, "arch->nrec  [%d]", arch->nrec );
	//if (arch->nrec > 0)
	//	return(0);

	mds_time(market, 0, &ymd, &hms, NULL, NULL);	// get local date & time

	arch->nrec = 0;
	arch->drec = 0;
	arch->vrec = 0;
	indx = market->indx;			// indexes for shared memory's folder
	_folder = market->fold;
	
	memset(&mstr, 0, sizeof(MDMSTR));
	while (arch->vrec < 100)//arch->mrec)
	{

		if (loadmaster(market, &mstr) != 1)
			break;
		sprintf(indx[arch->nrec].symb, mstr.symb);
	  	indx[arch->nrec++].indx = arch->vrec;
	  	folder = &_folder[arch->vrec++];
	  	memset(folder, 0, sizeof(MDFOLD));
	 
	  	strcpy(folder->symb, mstr.symb);
	  	memcpy(&folder->mstr, &mstr, sizeof(MDMSTR));
	  	strcpy(folder->quot.symb, mstr.symb);
	  	strcpy(folder->book.symb, mstr.symb);
	  	strcpy(folder->trad.symb, mstr.symb);
	  	memset(&quot, 0, sizeof(MDQUOT));
	  	memset(&book, 0, sizeof(MDBOOK));
	  	memset(&trad, 0, sizeof(MDTRAD));
	  	strcpy(quot.symb, mstr.symb);
	  	strcpy(book.symb, mstr.symb);
	  	strcpy(trad.symb, mstr.symb);
	  	trad.xymd = ymd;
	  	trad.xhms = HMS(HOUR(hms), MINUTE(hms), 0);	
	}
	memset(&mstr, 0, sizeof(MDMSTR));

	
	mds_log(market, LOG_MUST, "Load %d/%d instruments to shared memory", arch->nrec, arch->mrec);
//	nRet = initBasePriceData();
//	if (nRet ==0 )
//		mds_log(market, LOG_MUST, "InitBase Data OK");
//	else
//	{
//		mds_log(market, LOG_MUST, "InitBase Data Fail [%d]", nRet);
//		return -1;
//	}
//	if (initSwapPriceData() == 0)
//		mds_log(market, LOG_MUST, "initSwapPriceData Data OK");	
//	else
//	{
//		mds_log(market, LOG_MUST, "initSwapPriceData Data Fail [%d]", nRet);
//		return -1;
//	}
//	
//	if (initMarkupPriceData() == 0)
//		mds_log(market, LOG_MUST, "initMarkupPriceData Data OK");	
//	else
//	{
//		mds_log(market, LOG_MUST, "initMarkupPriceData Data Fail [%d]", nRet);
//		return -1;
//	}
//	if (initPriceOrign() == 0)
//		mds_log(market, LOG_MUST, "initPriceOrign Data OK");	
//	else
//	{
//		mds_log(market, LOG_MUST, "initPriceOrign Data Fail [%d]", nRet);
//		return -1;
//	}
//	
//	
	
	nRet = InitMarketDate(market);
    if (nRet == 0)
        mds_log(market, LOG_MUST, "InitMarketDate OK");    
    else
    {
       mds_log(market, LOG_MUST, "InitMarketDate [%d]", nRet);
       return -1;
    }
	while (arch->vrec < 100)//arch->mrec)
	{
		nRet = loadData(market, &mstr);
		if (nRet != 1)
		{
			mds_log(market, LOG_MUST, "loadData Break [%d]", nRet);
			break;
		}
	}
	
	
	return(0);
}

//
// folder_roff()
// Offset of a specified data on folder
//
static void *folder_roff(MARKET *market, void *folder, int dbid)
{
	MDFOLD	*fold = folder;

	switch (dbid)
	{
	case MSTR: return((char *)&fold->mstr);
	case QUOT: return((char *)&fold->quot);
	case BOOK: return((char *)&fold->book);
	case INTR: return((char *)&fold->trad);
	default:   return(NULL);
	}
}

//
// folder_clear()
// Clear folder
//
static void folder_clear(MARKET *market, void *folder)
{
	MDFOLD	*fold = folder;
	char	*buf;
	int	size;

	buf  = (char *)&fold->quot;
	size = sizeof(MDQUOT) - sizeof(fold->quot.symb);
	buf += sizeof(fold->quot.symb);
	memset(buf, 0, size);

	buf  = (char *)&fold->book;
	size = sizeof(MDBOOK) - sizeof(fold->book.symb);
	buf += sizeof(fold->book.symb);
	memset(buf, 0, size);
	
	buf  = (char *)&fold->trad;
	size = sizeof(MDTRAD) - sizeof(fold->trad.symb);
	buf += sizeof(fold->trad.symb);
	memset(buf, 0, size);

	mds_upsert(market, folder, QUOT, &fold->quot);
	mds_upsert(market, folder, BOOK, &fold->book);
}

//
// folder_trad()
// Synchronize shared memroty's market data to ISAM file
// Update 1 minute candle data on shared memory
//
//static void folder_trad(MARKET *market, MDFOLD *fold)
//{
//}

//
// folder_sync()
// Synchronize shared memroty's market data to ISAM file
//
static void folder_sync(MARKET *market, void *folder, int what)
{
}

//struct push_allq {
//	char	type		[2];			/* must be 'FC'					*/
//	char	code		[20];			/* realtime symbol				*/
//	char	root		[10];			/* root symbol					*/
//	char	pind		[1];			/* price indicator				*/
//	char	kymd		[8];			/* korean trading time			*/
//	char	khms		[6];			/* korean trading time			*/
//	char	zdiv		[ 1];			/* no of decimal				*/
//	char	cpask		[1];			/* 매도호가 색상('+', '-', ' ')	*/
//	char	pask		[20];			/* best ask						*/
//	char	cpbid		[1];			/* 매수호가 색상('+', '-', ' ')	*/
//	char	pbid		[20];			/* best bid						*/
//	char	spread		[6];			/* spread						*/
//	char	paskf1		[20];			/* tenner ask						*/
//	char	pbidf1		[20];			/* tenner bid						*/
//	char	spreadf1	[6];			/* spread							*/
//	char	paskf2		[20];			/* tenner ask						*/
//	char	pbidf2		[20];			/* tenner bid						*/
//	char	spreadf2	[6];			/* spread							*/
//	char	paskf3		[20];			/* tenner ask						*/
//	char	pbidf3		[20];			/* tenner bid						*/
//	char	spreadf3	[6];			/* spread							*/
//	char	paskf4		[20];			/* tenner ask						*/
//	char	pbidf4		[20];			/* tenner bid						*/
//	char	spreadf4	[6];			/* spread							*/
//	char	paskf5		[20];			/* tenner ask						*/
//	char	pbidf5		[20];			/* tenner bid						*/
//	char	spreadf5	[6];			/* spread							*/
//	char	paskf6		[20];			/* tenner ask						*/
//	char	pbidf6		[20];			/* tenner bid						*/
//	char	spreadf6	[6];			/* spread							*/
//	char	paskf7		[20];			/* tenner ask						*/
//	char	pbidf7		[20];			/* tenner bid						*/
//	char	spreadf7	[6];			/* spread							*/
//	char	paskf8		[20];			/* tenner ask						*/
//	char	pbidf8		[20];			/* tenner bid						*/
//	char	spreadf8	[6];			/* spread							*/
//	char	paskf9		[20];			/* tenner ask						*/
//	char	pbidf9		[20];			/* tenner bid						*/
//	char	spreadf9	[6];			/* spread							*/
//	char	paskf10		[20];			/* tenner ask						*/
//	char	pbidf10		[20];			/* tenner bid						*/
//	char	spreadf10	[6];			/* spread							*/
//	char	paskf11		[20];			/* tenner ask						*/
//	char	pbidf11		[20];			/* tenner bid						*/
//	char	spreadf11	[6];			/* spread							*/
//	char	paskf12		[20];			/* tenner ask						*/
//	char	pbidf12		[20];			/* tenner bid						*/
//	char	spreadf12	[6];			/* spread							*/
//	char	bymd		[8];			/* business date 				*/
//	char	seqn		[4];			/* sequence in 1 second 		*/	
//};

struct push_q {
	char	type[2];			/* must be 'FA'					*/
	char	code[20];			/* realtime symbol				*/
	char	root[10];			/* root symbol					*/
	char	pind[1];			/* price indicator				*/
	char	kymd[8];			/* korean trading time			*/
	char	khms[6];			/* korean trading time			*/
	char	xymd[8];			/* trading time					*/
	char	xhms[6];			/* exchange trading time		*/
	char	copen[1];			/* 시가 색상 ('+', '-', ' ')	*/
	char	open[20];			/* open price					*/
	char	chigh[1];			/* 고가 색상 ('+', '-', ' ')	*/
	char	high[20];			/* high price					*/
	char	clow[1];			/* 저가 색상 ('+', '-', ' ')	*/
	char	low[20];				/* low price					*/
	char	clast[1];			/* 현재가 색상 ('+', '-', ' ')	*/
	char	last[20];			/* last price					*/
	char	sign[1];			/* change sign					*/
	char	cdiff[1];			/* 대비 색상 ('+', '-', ' ')	*/
	char	diff[20];			/* net change					*/
	char	crate[1];			/* 등락율 색상 ('+', '-', ' ')	*/
	char	rate[6];			/* change rate					*/
	char	cpask[1];			/* Avail매도호가 색상('+', '-', ' ')	*/
	char	pask[20];			/* Avail ask						*/
	char	cpbid[1];			/* Avail매수호가 색상('+', '-', ' ')	*/
	char	pbid[20];			/* Avail ask						*/
	char	spread[6];			/* Availspread						*/
	char	bestcpask[1];			/* best매도호가 색상('+', '-', ' ')	*/
	char	bestpask[20];			/* best ask						*/
	char	bestcpbid[1];			/* best매수호가 색상('+', '-', ' ')	*/
	char	bestpbid[20];			/* best ask						*/
	char	bestspread[6];			/* bestspread						*/
	char	vask[20];						/* ask  size						*/
	char	vbid[20];				/* bid size						*/
	char	bestvask[20];		/* Best ask size						*/
	char	bestvbid[20];		/* Best size						*/
	char	seqn[4];			/* sequence in 1 second 		*/
	char	bymd		[8];			/* business date 				*/
	char	zdiv  [ 1];			/* no of decimal				*/
	char	zCdiv[ 1];			/* no of Customer decimal				*/
	char  onedaybidswap[26];
	char  onedayaskswap[26];
	char  spotbidmarkup1[26];
	char  spotbidmarkup2[26];
	char  spotbidmarkup3[26];
	char  spotaskmarkup1[26];
	char  spotaskmarkup2[26];
	char  spotaskmarkup3[26];	
	char  swapbidmarkup1[26];
	char  swapbidmarkup2[26];
	char  swapbidmarkup3[26];
	char  swapaskmarkup1[26];
	char  swapaskmarkup2[26];
	char  swapaskmarkup3[26];	
	char  onedaybidmarkup1[26];
	char  onedaybidmarkup2[26];
	char  onedaybidmarkup3[26];
	char  onedayaskmarkup1[26];
	char  onedayaskmarkup2[26];
	char  onedayaskmarkup3[26];	
};


//시장 실시간 
struct push_mq {
	char	type[2];			/* must be 'FC'					*/
	char	code[20];			/* realtime symbol				*/
	char	root[10];			/* root symbol					*/
	char	pind[1];			/* price indicator				*/
	char	kymd[8];			/* korean trading time			*/
	char	khms[6];			/* korean trading time			*/
	char	xymd[8];			/* trading time					*/
	char	xhms[6];			/* exchange trading time		*/
	char	copen[1];			/* 시가 색상 ('+', '-', ' ')	*/
	char	open[20];			/* open price					*/
	char	chigh[1];			/* 고가 색상 ('+', '-', ' ')	*/
	char	high[20];			/* high price					*/
	char	clow[1];			/* 저가 색상 ('+', '-', ' ')	*/
	char	low[20];				/* low price					*/
	char	clast[1];			/* 현재가 색상 ('+', '-', ' ')	*/
	char	last[20];			/* last price					*/
	char	sign[1];			/* change sign					*/
	char	cdiff[1];			/* 대비 색상 ('+', '-', ' ')	*/
	char	diff[20];			/* net change					*/
	char	crate[1];			/* 등락율 색상 ('+', '-', ' ')	*/
	char	rate[6];			/* change rate					*/
	char	cpask[1];			/* Avail매도호가 색상('+', '-', ' ')	*/
	char	pask[20];			/* Avail ask						*/
	char	cpbid[1];			/* Avail매수호가 색상('+', '-', ' ')	*/
	char	pbid[20];			/* Avail ask						*/
	char	spread[6];			/* Availspread						*/
	char	bestcpask[1];			/* best매도호가 색상('+', '-', ' ')	*/
	char	bestpask[20];			/* best ask						*/
	char	bestcpbid[1];			/* best매수호가 색상('+', '-', ' ')	*/
	char	bestpbid[20];			/* best ask						*/
	char	bestspread[6];			/* bestspread						*/
	char	vask[20];						/* ask  size						*/
	char	vbid[20];				/* bid size						*/
	char	bestvask[20];		/* Best ask size						*/
	char	bestvbid[20];		/* Best size						*/
	char	seqn[4];			/* sequence in 1 second 		*/
	char	bymd		[8];			/* business date 				*/
	char	zdiv  [ 1];			/* no of decimal				*/
	char	zCdiv[ 1];			/* no of Customer decimal				*/
	char	cdate[1];			/* 시가 색상 ('+', '-', ' ')	*/

};



#define	RTS_DEPTH	5
struct push_d {
	char	type[2];			/* must be 'FB'					*/
	char	code[20];			/* realtime symbol				*/
	char	root[10];			/* root symbol					*/
	char	pind[1];			/* price indicator				*/
	char	khms[6];			/* korea trading time			*/
	char	xhms[6];			/* exchange time				*/
	struct	{
		char	cpask[1];		/* 매수호가 색상('+', '-', ' ')	*/
		char	pask[20];		/* ask price					*/
		char	vask[20];		/* ask size						*/
		char	vaskdisp[15];		/* ask disp						*/
		char	cpbid[1];		/* 매수호가 색상('+', '-', ' ')	*/
		char	pbid[20];		/* bid price					*/
		char	vbid[20];		/* bid size						*/
		char	vbiddisp[15];		/* bid size						*/
	} book[RTS_DEPTH];
	char  onedaybidswap[26];
	char  onedayaskswap[26];
	char  spotbidmarkup1[26];
	char  spotbidmarkup2[26];
	char  spotbidmarkup3[26];
	char  spotaskmarkup1[26];
	char  spotaskmarkup2[26];
	char  spotaskmarkup3[26];	
	char  swapbidmarkup1[26];
	char  swapbidmarkup2[26];
	char  swapbidmarkup3[26];
	char  swapaskmarkup1[26];
	char  swapaskmarkup2[26];
	char  swapaskmarkup3[26];		
	char  onedaybidmarkup1[26];
	char  onedaybidmarkup2[26];
	char  onedaybidmarkup3[26];
	char  onedayaskmarkup1[26];
	char  onedayaskmarkup2[26];
	char  onedayaskmarkup3[26];	
	char  bidspotprice[26];    //BID Spot가격 . 주문시 필요해서 추가
	char  askspotprice[26];	   //ASK Spot가격 . 주문시 필요해서 추가 
	char  zCdiv[ 1];			/* no of Customer decimal				*/
	char  bidopenprice[26];			/* BIDopen price					*/
	char  bidhighprice[26];			/* BIDhigh price					*/
	char  bidlowprice[26];			/* BIDlow price					*/
	char  askopenprice[26];			/* ASKopen price					*/
	char  askhighprice[26];			/* ASKhigh price					*/
	char  asklowprice[26];			/* ASKlow price					*/	
};

struct	push_g {
	char	type[2];			/* must be 'C'			*/
	char	code[12];			/* realtime symbol		*/
	char	delt[9];			/* 옵션민감도 : 델타		*/
	char	thet[9];			/*              세타		*/
	char	vega[9];			/*	        베가		*/
	char	gama[9];			/*              감마		*/
	char	rho[9];				/*              로우		*/
};

static char color(double val, double base)
{
	double diff;
	diff = val -base;
	if ((diff > -0.0000001) && (diff < 0.0000001))
		 return '3';
	if (val > base) return '2';
    else if (val < base) return '5';
	return '3';
}

static char color_ccvol(int dirf)
{
	if (dirf == '+')
		return '2';
	else if (dirf == '-')
		return '5';
	return '3';
}

/////////////////////////////////////////////////////////////////////////////////////////
// Push real-time market data
//
// Push message type : 
// 	1st char : data format indicator (Here : 0)
// 	2nd char : event indicator (data type)
//
// PUSH-TYPE	EVENT-BIT	EVENT_NAME		Market Data
//	A	0x00000001	PUSH_QUOT		QUOT
//	B	0x00000002	PUSH_BOOK		Market BOOK
//	C	0x00000002	PUSH_MARKET		Matket Data
////////////////////////////////////////////////////////////////////////////////////////

void push_quot_filter(MARKET *market, MDQUOT *quot, int zdiv, double cvol)
{
#if 0
	mds_log(market, LOG_MUST, "push_quot_filter 392 ");
	MDFOLD	*fbase = 0;
	MDFOLD	*folder = (MDFOLD *)(((char*)quot) - (char*)(&fbase->quot));
	MDMSTR	*mstr = &folder->mstr;
	struct	pushdata pushdata;
	char	*push_b = pushdata.pushmsg.msgb;
	struct	push_q *push_q = (struct push_q *)push_b;
	int	push_l = 0;
	char	form[40], diff[40];

	sprintf(form, "%%.%df",  zdiv);
	sprintf(diff, "%%+.%df", zdiv);
	
	memset(&pushdata, 0x00, sizeof(struct pushdata));
	pushdata.mkid = market->exid;
	pushdata.pushmsg.mask = PUSH_QUOT;
	pushdata.pushmsg.type = 'A';
	sprintf(pushdata.pushmsg.symb, quot->symb);

	memset(push_q, 0, sizeof(struct push_q));
	sprintf(push_q->type, "%cA", market->xchg->push_id);
	strcpy(push_q->code, pushdata.pushmsg.symb);	// real symbol
	strcpy(push_q->root, folder->mstr.inrt);
	sprintf(push_q->pind, "%c", folder->mstr.pind);	// kor time
	if (quot->tick)
		push_q->tick[0] = '1';			// with tick
	else
		push_q->tick[0] = '0';			// no tick
	sprintf(push_q->kymd, "%08d", quot->kymd);	// date
	
	sprintf(push_q->khms, "%06d", quot->khms/1000);	// kor time
	sprintf(push_q->xhms, "%06d", quot->xhms/1000);
		
	sprintf(push_q->xymd, "%08d", quot->xymd);	// date

	sprintf(push_q->copen, "%c", color(quot->bidopen, mstr->base));
	sprintf(push_q->open, form, quot->bidopen);	// open
	sprintf(push_q->chigh, "%c", color(quot->bidhigh, mstr->base));
	sprintf(push_q->high, form, quot->bidhigh);	// high
	sprintf(push_q->clow , "%c", color(quot->bidlow , mstr->base));
	sprintf(push_q->low,  form, quot->bidlow);		// low
	sprintf(push_q->clast, "%c", color(quot->bidlast , mstr->base));
	sprintf(push_q->last, form, quot->bidlast);	// last
	push_q->sign[0] = quot->bidsign | '0';		// change sign
	sprintf(push_q->cdiff, "%c", color(quot->biddiff, 0));
	sprintf(push_q->diff, diff, quot->biddiff);	// net change
	sprintf(push_q->crate, "%c", color(quot->bidrate, 0));
	sprintf(push_q->rate, "%.2f", quot->bidrate);	// % change

	sprintf(push_q->ccvol, "%c", color_ccvol(quot->side));
	sprintf(push_q->cvol, "%.f", cvol);
//	sprintf(push_q->tvol, "%.f", quot->tvol);
//	sprintf(push_q->tamt, "%.f", quot->tamt);
	push_q->side[0] = quot->side;
	sprintf(push_q->cpask, "%c", color(quot->pask, folder->mstr.base));
	sprintf(push_q->pask, form, quot->pask);
	sprintf(push_q->cpbid, "%c", color(quot->pbid, folder->mstr.base));
	sprintf(push_q->pbid, form, quot->pbid);
	sprintf(push_q->pvol, "%.f", folder->mstr.p.tvol);
//	if (folder->mstr.p.tvol != 0)
//		sprintf(push_q->vrat, "%.2f", quot->tvol * 100. / folder->mstr.p.tvol);
//	else
//		sprintf(push_q->vrat, "%.2f", 0.);

	sprintf(push_q->bvol, "%.f", quot->bvol);
	sprintf(push_q->svol, "%.f", quot->svol);
	sprintf(push_q->bymd, "%08d", quot->tymd);	// business date
	sprintf(push_q->seqn, "%04d", quot->sseq);
	sprintf(push_q->bamt, form, quot->bamt);
	sprintf(push_q->samt, form, quot->samt);
	sprintf(push_q->zdiv, "%01d", zdiv);

	sprintf(push_q->vask, "%d", folder->book.ask[0].vask);	// best ask volume
	sprintf(push_q->vbid, "%d", folder->book.bid[0].vbid);	// best bid volume

	push_l = sizeof(struct push_q);
	if (push_l > MAX_PUSH_LEN)
		push_l = MAX_PUSH_LEN;
	pushdata.pushmsg.msgl = push_l;
	myrq_push(&pushdata);
#endif
}
static void push_quot(MARKET *market, MDQUOT *quot, int zdiv)
{
	MDFOLD	*fbase = 0;
	MDFOLD	*folder = (MDFOLD *)(((char*)quot) - (char*)(&fbase->quot));
	MDMSTR	*mstr = &folder->mstr;
	struct	pushdata pushdata;
	char	*push_b = pushdata.pushmsg.msgb;
	
	struct	push_q *push_q = (struct push_q *)push_b;
	int	push_l = 0, allpush_l = 0;
	
	char	form[40], diff[40];
//	sprintf(form, "%%.%df",  zdiv);
	//sprintf(diff, "%%+.%df", zdiv);
	sprintf(diff, "%%.%df", zdiv);
	sprintf(form, "%%.%df",  8);
//	sprintf(diff, "%%+.%df", 8);
	//if ((quot->sseq %2) == 0)
	//	return;
	memset(&pushdata, 0x00, sizeof(struct pushdata));
	pushdata.mkid = market->exid;
	
	pushdata.pushmsg.mask = PUSH_QUOT;
	pushdata.pushmsg.type = 'A';
	
	sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&quot->symb,SYMB_SUBLEN,subCodeSpot[0]);
	memset(push_q, 0, sizeof(struct push_q));
	sprintf(push_q->type, "%cA", market->xchg->push_id);
	strcpy(push_q->code, pushdata.pushmsg.symb);	// real symbol
	strcpy(push_q->root, folder->mstr.inrt);
	sprintf(push_q->pind, "%c", folder->mstr.pind);	// kor time
	sprintf(push_q->xymd, "%08d", quot->xymd);	// date
	sprintf(push_q->kymd, "%08d", quot->kymd);	// date
	sprintf(push_q->khms, "%06d", quot->khms/1000);	// kor time
	sprintf(push_q->xhms, "%06d", quot->xhms/1000);
	/*
	sprintf(push_q->copen, "%c", color(quot->spotdata.bidopen, mstr->base));
	sprintf(push_q->open, form, quot->spotdata.bidopen);	// open
	sprintf(push_q->chigh, "%c", color(quot->spotdata.bidhigh, mstr->base));
	sprintf(push_q->high, form, quot->spotdata.bidhigh);	// high
	sprintf(push_q->clow , "%c", color(quot->spotdata.bidlow , mstr->base));
	sprintf(push_q->low,  form, quot->spotdata.bidlow);		// low
	sprintf(push_q->clast, "%c", color(quot->spotdata.bidlast , mstr->base));
	sprintf(push_q->last, form, quot->spotdata.bidlast);	// last
	push_q->sign[0] = quot->spotdata.bidsign | '0';		// change sign
	
	sprintf(push_q->cdiff, "%c", color(quot->spotdata.biddiff, 0));
	sprintf(push_q->diff, form, quot->spotdata.biddiff);	// net change
	sprintf(push_q->crate, "%c", color(quot->spotdata.bidrate, 0));
	sprintf(push_q->rate, "%.2f", quot->spotdata.bidrate);	// % change
	//sprintf(push_q->pvol, "%.f", folder->mstr.p.tvol);
	*/
	//대비는 mid값 기준으로
	
	sprintf(push_q->copen, "%c", color(quot->spotdata.midopen,quot->spotdata.midbase));
	sprintf(push_q->open, form, quot->spotdata.midopen);	// open
	sprintf(push_q->chigh, "%c", color(quot->spotdata.bidhigh, quot->spotdata.midbase));
	sprintf(push_q->high, form, quot->spotdata.midhigh);	// high
	sprintf(push_q->clow , "%c", color(quot->spotdata.midlow , quot->spotdata.midbase));
	sprintf(push_q->low,  form, quot->spotdata.midlow);		// low
	sprintf(push_q->clast, "%c", color(quot->spotdata.midlast , quot->spotdata.midbase));
	sprintf(push_q->last, form, quot->spotdata.midlast);	// last
	push_q->sign[0] = quot->spotdata.midsign | '0';		// change sign
	sprintf(push_q->cdiff, "%c", color(quot->spotdata.middiff, 0));
	sprintf(push_q->diff, diff, quot->spotdata.middiff);	// net change

	sprintf(push_q->crate, "%c", color(quot->spotdata.midrate, 0));
	sprintf(push_q->rate, "%.2f", quot->spotdata.midrate);	// % change
	
	
	sprintf(push_q->cpask, "%d", quot->spotdata.offersign);//color(quot->spotdata.offerlast, folder->mstr.base));
	sprintf(push_q->pask, form, quot->spotdata.offerlast);
	sprintf(push_q->cpbid, "%d", quot->spotdata.bidsign);
	sprintf(push_q->pbid, form, quot->spotdata.bidlast);
	sprintf(push_q->spread, form, quot->spotdata.offerlast - quot->spotdata.bidlast);	// last
	
	sprintf(push_q->bestcpask, "%d", quot->spotdata.offersign);
	sprintf(push_q->bestpask, form, quot->spotdata.offerbest);
	sprintf(push_q->bestcpbid, "%d", quot->spotdata.bidsign);
	sprintf(push_q->bestpbid, form, quot->spotdata.bidbest);
	sprintf(push_q->bestspread, form, quot->spotdata.offerbest - quot->spotdata.bidbest);	// best
	
	
	sprintf(push_q->vask, "%.0f", quot->spotdata.offervol );	// 
	sprintf(push_q->vbid, "%.0f", quot->spotdata.bidvol );	// 
	sprintf(push_q->bestvask, "%.0f", quot->spotdata.offerbestvol );	// best
	sprintf(push_q->bestvbid, "%.0f", quot->spotdata.bidbestvol );	// best
	

	sprintf(push_q->bymd, "%08d", quot->tymd);	// business date
	sprintf(push_q->seqn, "%04d", quot->sseq);
	sprintf(push_q->zdiv, "%01d", zdiv);
	sprintf(push_q->zCdiv, "%01d", folder->mstr.zCustdiv);
	
	sprintf(push_q->onedaybidswap,    "%.8f", quot->spotdata.donedaybidswap* mstr->swappmul);	
	sprintf(push_q->onedayaskswap,    "%.8f", quot->spotdata.donedayofferswap* mstr->swappmul);	
	
	sprintf(push_q->spotbidmarkup1, "%.8f", quot->markupdata[9].markupSet[0].bidMarkup* mstr->swappmul);	
	sprintf(push_q->spotbidmarkup2, "%.8f", quot->markupdata[9].markupSet[1].bidMarkup* mstr->swappmul);	
	sprintf(push_q->spotbidmarkup3, "%.8f", quot->markupdata[9].markupSet[2].bidMarkup* mstr->swappmul);		
	sprintf(push_q->spotaskmarkup1, "%.8f", quot->markupdata[9].markupSet[0].offerMarkup* mstr->swappmul);	
	sprintf(push_q->spotaskmarkup2, "%.8f", quot->markupdata[9].markupSet[1].offerMarkup* mstr->swappmul);	
	sprintf(push_q->spotaskmarkup3, "%.8f", quot->markupdata[9].markupSet[2].offerMarkup* mstr->swappmul);	
	
	sprintf(push_q->swapbidmarkup1, "%.8f", 0.00000000);	
	sprintf(push_q->swapbidmarkup2, "%.8f", 0.00000000);	
	sprintf(push_q->swapbidmarkup3, "%.8f", 0.00000000);		
	sprintf(push_q->swapaskmarkup1, "%.8f", 0.00000000);	
	sprintf(push_q->swapaskmarkup2, "%.8f", 0.00000000);	
	sprintf(push_q->swapaskmarkup3, "%.8f", 0.00000000);	
	
	sprintf(push_q->onedaybidmarkup1, "%.8f", quot->spotdata.onedayMarkup[0].donedaybidMarkup* mstr->swappmul);	
	sprintf(push_q->onedaybidmarkup2, "%.8f", quot->spotdata.onedayMarkup[1].donedaybidMarkup* mstr->swappmul);	
	sprintf(push_q->onedaybidmarkup3, "%.8f", quot->spotdata.onedayMarkup[2].donedaybidMarkup* mstr->swappmul);		
	sprintf(push_q->onedayaskmarkup1, "%.8f", quot->spotdata.onedayMarkup[0].donedayofferMarkup* mstr->swappmul);	
	sprintf(push_q->onedayaskmarkup2, "%.8f", quot->spotdata.onedayMarkup[1].donedayofferMarkup* mstr->swappmul);	
	sprintf(push_q->onedayaskmarkup3, "%.8f", quot->spotdata.onedayMarkup[2].donedayofferMarkup* mstr->swappmul);	

	push_l = sizeof(struct push_q);
	if (push_l > MAX_PUSH_LEN)
		push_l = MAX_PUSH_LEN;
	pushdata.pushmsg.msgl = push_l;
	myrq_push(&pushdata);
	
	int i = 0;
	for ( i = 0; i < MAX_TENNER -1 ; i++)
	{
		
		sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&quot->symb,SYMB_SUBLEN,subFowardode[i]);
		sprintf(push_q->type, "%cA", market->xchg->push_id);
		strcpy(push_q->code, pushdata.pushmsg.symb);	// real symbol
		sprintf(push_q->copen, "%c", color(quot->foworddata[i].bidopen, mstr->base));
		sprintf(push_q->open, form, quot->foworddata[i].bidopen);	// open
		sprintf(push_q->chigh, "%c", color(quot->foworddata[i].bidhigh, mstr->base));
		sprintf(push_q->high, form, quot->foworddata[i].bidhigh);	// high
		sprintf(push_q->clow , "%c", color(quot->foworddata[i].bidlow , mstr->base));
		sprintf(push_q->low,  form, quot->foworddata[i].bidlow);		// low
		sprintf(push_q->clast, "%c", color(quot->foworddata[i].bidlast , mstr->base));
		sprintf(push_q->last, form, quot->foworddata[i].bidlast);	// last
		push_q->sign[0] = quot->foworddata[i].bidsign | '0';		// change sign
		sprintf(push_q->cdiff, "%c", color(quot->foworddata[i].middiff, 0));
		sprintf(push_q->diff, form, quot->foworddata[i].middiff);	// net change
		sprintf(push_q->crate, "%c", color(quot->foworddata[i].midrate, 0));
		sprintf(push_q->rate, "%.2f", quot->foworddata[i].midrate);	// % change
		sprintf(push_q->cpask, "%c", color(quot->foworddata[i].offerlast, folder->mstr.base));
		sprintf(push_q->pask, form, quot->foworddata[i].offerlast);
		sprintf(push_q->cpbid, "%c", color(quot->foworddata[i].bidlast, folder->mstr.base));
		sprintf(push_q->pbid, form, quot->foworddata[i].bidlast);
		sprintf(push_q->spread, form, quot->foworddata[i].bidlast - quot->foworddata[i].offerlast);	// last
		sprintf(push_q->onedaybidswap,    "%.8f", quot->foworddata[i].donedaybidswap* mstr->swappmul);	
		
		sprintf(push_q->swapbidmarkup1, "%.8f", quot->markupdata[i].markupSet[0].bidMarkup* mstr->swappmul);	
		sprintf(push_q->swapbidmarkup2, "%.8f", quot->markupdata[i].markupSet[1].bidMarkup* mstr->swappmul);	
		sprintf(push_q->swapbidmarkup3, "%.8f", quot->markupdata[i].markupSet[2].bidMarkup* mstr->swappmul);		
		sprintf(push_q->swapaskmarkup1, "%.8f", quot->markupdata[i].markupSet[0].offerMarkup* mstr->swappmul);	
		sprintf(push_q->swapaskmarkup2, "%.8f", quot->markupdata[i].markupSet[1].offerMarkup* mstr->swappmul);	
		sprintf(push_q->swapaskmarkup3, "%.8f", quot->markupdata[i].markupSet[2].offerMarkup* mstr->swappmul);	
			
		sprintf(push_q->onedaybidmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_q->onedaybidmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_q->onedaybidmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedaybidMarkup* mstr->swappmul);	

		sprintf(push_q->onedayaskswap,    "%.8f", quot->foworddata[i].donedayofferswap* mstr->swappmul);	
		sprintf(push_q->onedayaskmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_q->onedayaskmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_q->onedayaskmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedayofferMarkup* mstr->swappmul);	
				
		push_l = sizeof(struct push_q);
		if (push_l > MAX_PUSH_LEN)
			push_l = MAX_PUSH_LEN;
		pushdata.pushmsg.msgl = push_l;
		myrq_push(&pushdata);
	}



}


void push_marketquot(MARKET *market, MDQUOT *quot, int zdiv) //(MARKET *market, char *msgb, int msgl, int zdiv) 
{
	MDFOLD	*fbase = 0;
	MDFOLD	*folder = (MDFOLD *)(((char*)quot) - (char*)(&fbase->quot));
	MDMSTR	*mstr = &folder->mstr;
	struct	pushdata pushdata;
	char	*push_b = pushdata.pushmsg.msgb;
	
	struct	push_mq *push_q = (struct push_q *)push_b;
	int	push_l = 0, allpush_l = 0;
	//struct cmbsquote	*quote = (struct cmbsquote *)msgb;

	
	//struct	pushdata pushdata;
	//char	*push_b = pushdata.pushmsg.msgb;
	
	//struct	push_mq *push_q = (struct push_mq *)push_b;

	char	form[40], diff[40];
	sprintf(form, "%%.%df",  zdiv);
	sprintf(diff, "%%.%df", zdiv);

	memset(&pushdata, 0x00, sizeof(struct pushdata));
	pushdata.mkid = market->exid;
	
	pushdata.pushmsg.mask = PUSH_MARKET;
	pushdata.pushmsg.type = 'C';
	
//sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,quote->symb,SYMB_SUBLEN,subCodeSpot[0]);
	sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&quot->symb,SYMB_SUBLEN,subCodeSpot[0]);
	//memset(push_q, 0, sizeof(struct push_q));
	memset(push_q, 0, sizeof(struct push_q));
	sprintf(push_q->type, "%cC", market->xchg->push_id);
	strcpy(push_q->code, pushdata.pushmsg.symb);	// real symbol
	//strcpy(push_q->root, folder->mstr.inrt);
	sprintf(push_q->pind, "%d", zdiv);	
//	sprintf(push_q->kymd, "%08d", str2i(quote->date, sizeof(quote->date)));	// date
//	sprintf(push_q->khms, "%06d", str2i(quote->time, sizeof(quote->time)));	// kor time
	sprintf(push_q->xymd, "%08d", quot->xymd);	// date
	sprintf(push_q->kymd, "%08d", quot->kymd);	// date
	sprintf(push_q->khms, "%06d", quot->khms/1000);	// kor time
	sprintf(push_q->xhms, "%06d", quot->xhms/1000);

	sprintf(push_q->copen, "%c", color(quot->spotdata.midopen,quot->spotdata.midbase));
	sprintf(push_q->open, form, quot->spotdata.midopen);	// open
	sprintf(push_q->chigh, "%c", color(quot->spotdata.bidhigh, quot->spotdata.midbase));
	sprintf(push_q->high, form, quot->spotdata.midhigh);	// high
	sprintf(push_q->clow , "%c", color(quot->spotdata.midlow , quot->spotdata.midbase));
	sprintf(push_q->low,  form, quot->spotdata.midlow);		// low
	sprintf(push_q->clast, "%c", color(quot->spotdata.midlast , quot->spotdata.midbase));
	sprintf(push_q->last, form, quot->spotdata.midlast);	// last
	push_q->sign[0] = quot->spotdata.midsign | '0';		// change sign
	sprintf(push_q->cdiff, "%c", color(quot->spotdata.middiff, 0));
	sprintf(push_q->diff, diff, quot->spotdata.middiff);	// net change

	sprintf(push_q->crate, "%c", color(quot->spotdata.midrate, 0));
	sprintf(push_q->rate, "%.2f", quot->spotdata.midrate);	// % change
	
	
	sprintf(push_q->cpask, "%d", quot->spotdata.offersign);//color(quot->spotdata.offerlast, folder->mstr.base));
	sprintf(push_q->pask, form, quot->spotdata.offerlast);
	sprintf(push_q->cpbid, "%d", quot->spotdata.bidsign);
	sprintf(push_q->pbid, form, quot->spotdata.bidlast);
	sprintf(push_q->spread, form, quot->spotdata.offerlast - quot->spotdata.bidlast);	// last
	
	sprintf(push_q->bestcpask, "%d", quot->spotdata.offersign);
	sprintf(push_q->bestpask, form, quot->spotdata.offerbest);
	sprintf(push_q->bestcpbid, "%d", quot->spotdata.bidsign);
	sprintf(push_q->bestpbid, form, quot->spotdata.bidbest);
	sprintf(push_q->bestspread, form, quot->spotdata.offerbest - quot->spotdata.bidbest);	// best
	
	
	sprintf(push_q->vask, "%.0f", quot->spotdata.offervol );	// 
	sprintf(push_q->vbid, "%.0f", quot->spotdata.bidvol );	// 
	sprintf(push_q->bestvask, "%.0f", quot->spotdata.offerbestvol );	// best
	sprintf(push_q->bestvbid, "%.0f", quot->spotdata.bidbestvol );	// best
	

	sprintf(push_q->bymd, "%08d", quot->tymd);	// business date
	sprintf(push_q->seqn, "%04d", quot->sseq);
	sprintf(push_q->zdiv, "%01d", zdiv);
	sprintf(push_q->zdiv, "%01d", zdiv);
	
	sprintf(push_q->cdate, "%c", color(quot->SMBSspotdate, str2i(mstr->expiredateSpot, sizeof(mstr->expiredateSpot))));
	
	

	

	push_l = sizeof(struct push_q);
	if (push_l > MAX_PUSH_LEN)
		push_l = MAX_PUSH_LEN;
	pushdata.pushmsg.msgl = push_l;
	myrq_push(&pushdata);

}



static void push_book(MARKET *market, MDBOOK *book, int zdiv)
{
	MDFOLD	*fbase = 0;
	MDFOLD	*folder = (MDFOLD *)(((char*)book) - (char*)(&fbase->book));
	MDQUOT	*quot = &folder->quot;
	MDMSTR	*mstr = &folder->mstr;
	struct	pushdata pushdata;
	char	*push_b = pushdata.pushmsg.msgb;
	struct	push_d *push_d = (struct push_d *)push_b;
	int	push_l = 0;
	char    form[16];
	int	ii;
	
	memset(&pushdata, 0x20, sizeof(struct pushdata));
	pushdata.mkid = market->exid;
	pushdata.pushmsg.mask = PUSH_BOOK;
	//sprintf(pushdata.pushmsg.symb, book->symb);
	sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&book->symb,SYMB_SUBLEN,subCodeSpot[0]);
	pushdata.pushmsg.type = 'B';

	memset(push_d, 0, sizeof(struct push_d));
	sprintf(push_d->type, "%cB", market->xchg->push_id);

	strcpy(push_d->code, pushdata.pushmsg.symb);
	
	strcpy(push_d->root, folder->mstr.inrt);
	sprintf(push_d->pind, "%d", folder->mstr.zdiv);	// 직원소수점 	
	sprintf(push_d->khms, "%06d", book->khms/1000);	// kor time
	sprintf(push_d->xhms, "%06d", book->xhms/1000);
	
	int	ibidzeroCount = 0;
	int iaskzeroCount = 0;	
	double dCask = book->spotdata.cask;
	double dCbid = book->spotdata.cbid;
	double dPask = book->spotdata.ask[0].pask;
	double dPbid = book->spotdata.bid[0].pbid;
//	sprintf(form, "%%*.%df", folder->mstr.zdiv);
	sprintf(form, "%%*.%df", 8);
	
	

	
	for (ii = 0; ii < BOOK_LEVEL; ii++)
	{
		if (ii < RTS_DEPTH)
		{
			//가능수량이 0보다 작은경우 해당 호가는 최소수량 설정
			if (book->spotdata.bid[ii].vbid -  dCbid < 0.00000001)
			{
			//	ibidzeroCount++;
				sprintf(push_d->book[ii].cpbid, "%d", quot->spotdata.bidsign);
				SET_STR(push_d->book[ii].pbid, form, sizeof(push_d->book[ii].pbid) ,book->spotdata.bid[ii].pbid);	
				SET_STR(push_d->book[ii].vbid,"%*d",	(int)sizeof(push_d->book[ii].vbid),		(int)folder->mstr.dBidBaseAmount);
				SET_STR(push_d->book[ii].vbiddisp, "%.1f%s",	folder->mstr.dAskBaseAmount / folder->mstr.nAmountUnit, folder->mstr.dispUnit);	
				

			}else
			{
				sprintf(push_d->book[ii].cpbid, "%d", quot->spotdata.bidsign);
				SET_STR(push_d->book[ii].pbid, form, sizeof(push_d->book[ii-ibidzeroCount].pbid) ,book->spotdata.bid[ii].pbid);	
				SET_STR(push_d->book[ii].vbid,"%*d",	(int)sizeof(push_d->book[ii-ibidzeroCount].vbid),		(int)(book->spotdata.bid[ii].vbid-  dCbid));
				SET_STR(push_d->book[ii].vbiddisp, "%.1f%s",	(book->spotdata.bid[ii].vbid-  dCbid) / folder->mstr.nAmountUnit, folder->mstr.dispUnit);	


			}
			dCbid = dCbid - book->spotdata.bid[ii].vbid;
			if (dCbid< 0.0000001)
				dCbid = 0;
		}
		
		
		if (ii < RTS_DEPTH)
		{
			//가능수량이 0보다 작은경우 해당 호가는 최소수량 설정
			if (book->spotdata.ask[ii].vask -  dCask < 0.00000001)
			{
			//	iaskzeroCount++;
				sprintf(push_d->book[ii].cpask, "%d", quot->spotdata.offersign);
				SET_STR(push_d->book[ii].pask, form, sizeof(push_d->book[ii].pask) ,book->spotdata.ask[ii].pask);	
				SET_STR(push_d->book[ii].vask, "%*d",	(int)sizeof(push_d->book[ii].vask),	 (int)folder->mstr.dAskBaseAmount);
				SET_STR(push_d->book[ii].vaskdisp, "%.1f%s", folder->mstr.dAskBaseAmount/ folder->mstr.nAmountUnit, folder->mstr.dispUnit);	
			}else
			{
				sprintf(push_d->book[ii].cpask, "%d", quot->spotdata.offersign);
				SET_STR(push_d->book[ii].pask, form, sizeof(push_d->book[ii].pask) ,book->spotdata.ask[ii].pask);	
				SET_STR(push_d->book[ii].vask, "%*d",	(int)sizeof(push_d->book[ii-iaskzeroCount].vask),	 (int)(book->spotdata.ask[ii].vask-  dCask));		
				SET_STR(push_d->book[ii].vaskdisp, "%.1f%s",(book->spotdata.ask[ii].vask-  dCask)/ folder->mstr.nAmountUnit, folder->mstr.dispUnit);	
					
			}
			dCask = dCask - book->spotdata.ask[ii].vask;
			if (dCask< 0.0000001)
				dCask = 0;
		}
	}

	sprintf(push_d->spotbidmarkup1, "%.8f", quot->markupdata[9].markupSet[0].bidMarkup* mstr->swappmul);	
	sprintf(push_d->spotbidmarkup2, "%.8f", quot->markupdata[9].markupSet[1].bidMarkup* mstr->swappmul);	
	sprintf(push_d->spotbidmarkup3, "%.8f", quot->markupdata[9].markupSet[2].bidMarkup* mstr->swappmul);		
	sprintf(push_d->spotaskmarkup1, "%.8f", quot->markupdata[9].markupSet[0].offerMarkup* mstr->swappmul);	
	sprintf(push_d->spotaskmarkup2, "%.8f", quot->markupdata[9].markupSet[1].offerMarkup* mstr->swappmul);	
	sprintf(push_d->spotaskmarkup3, "%.8f", quot->markupdata[9].markupSet[2].offerMarkup* mstr->swappmul);	
	
	sprintf(push_d->swapbidmarkup1, "%.8f",0.00000000 );	
	sprintf(push_d->swapbidmarkup2, "%.8f",0.00000000 );	
	sprintf(push_d->swapbidmarkup3, "%.8f",0.00000000 );		
	sprintf(push_d->swapaskmarkup1, "%.8f",0.00000000 );	
	sprintf(push_d->swapaskmarkup2, "%.8f",0.00000000 );	
	sprintf(push_d->swapaskmarkup3, "%.8f",0.00000000 );
	
		
	sprintf(push_d->onedaybidswap,    "%.8f", quot->spotdata.donedaybidswap* mstr->swappmul);	
	sprintf(push_d->onedaybidmarkup1, "%.8f", quot->spotdata.onedayMarkup[0].donedaybidMarkup* mstr->swappmul);	
	sprintf(push_d->onedaybidmarkup2, "%.8f", quot->spotdata.onedayMarkup[1].donedaybidMarkup* mstr->swappmul);	
	sprintf(push_d->onedaybidmarkup3, "%.8f", quot->spotdata.onedayMarkup[2].donedaybidMarkup* mstr->swappmul);	

	sprintf(push_d->onedayaskswap,    "%.8f", quot->spotdata.donedayofferswap* mstr->swappmul);	
	sprintf(push_d->onedayaskmarkup1, "%.8f", quot->spotdata.onedayMarkup[0].donedayofferMarkup* mstr->swappmul);	
	sprintf(push_d->onedayaskmarkup2, "%.8f", quot->spotdata.onedayMarkup[1].donedayofferMarkup* mstr->swappmul);	
	sprintf(push_d->onedayaskmarkup3, "%.8f", quot->spotdata.onedayMarkup[2].donedayofferMarkup* mstr->swappmul);	

	sprintf(push_d->bidspotprice, 	  "%.8f", dPbid);	
	sprintf(push_d->askspotprice, 	  "%.8f", dPask);	
	sprintf(push_d->zCdiv, 	   "%01d", folder->mstr.zCustdiv);
	
	sprintf(push_d->bidopenprice, "%.8f", quot->spotdata.bidopen);	
	sprintf(push_d->bidhighprice, "%.8f", quot->spotdata.bidhigh);	
	sprintf(push_d->bidlowprice, "%.8f",  quot->spotdata.bidlow);	
	sprintf(push_d->askopenprice, "%.8f", quot->spotdata.offeropen);	
	sprintf(push_d->askhighprice, "%.8f", quot->spotdata.offerhigh);	
	sprintf(push_d->asklowprice, "%.8f",  quot->spotdata.offerlow);	
		
//	sprintf(push_d->fprc, form, book->fprc);
	push_l = sizeof(struct push_d);
	pushdata.pushmsg.msgl = push_l;
	
	
	int rc;
	rc = myrq_push(&pushdata);
	
	
	int i = 0;
	for ( i = MAX_TENNER -2; i < MAX_TENNER; i++)
	{

		sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&book->symb,SYMB_SUBLEN,subFowardode[i]);
		strcpy(push_d->code, pushdata.pushmsg.symb);
		for (ii = 0; ii < RTS_DEPTH; ii++)
		{
			SET_PRC(push_d->book[ii].pask, form, sizeof(push_d->book[ii].pask),book->foworddata[i].ask[ii].pask);	
			SET_PRC(push_d->book[ii].pbid, form, sizeof(push_d->book[ii].pbid),book->foworddata[i].bid[ii].pbid);			
		}
		
		sprintf(push_d->swapbidmarkup1, "%.8f", quot->markupdata[i].markupSet[0].bidMarkup* mstr->swappmul);	
		sprintf(push_d->swapbidmarkup2, "%.8f", quot->markupdata[i].markupSet[1].bidMarkup* mstr->swappmul);	
		sprintf(push_d->swapbidmarkup3, "%.8f", quot->markupdata[i].markupSet[2].bidMarkup* mstr->swappmul);		
		sprintf(push_d->swapaskmarkup1, "%.8f", quot->markupdata[i].markupSet[0].offerMarkup* mstr->swappmul);	
		sprintf(push_d->swapaskmarkup2, "%.8f", quot->markupdata[i].markupSet[1].offerMarkup* mstr->swappmul);	
		sprintf(push_d->swapaskmarkup3, "%.8f", quot->markupdata[i].markupSet[2].offerMarkup* mstr->swappmul);
				
		sprintf(push_d->onedaybidswap,    "%.8f", quot->foworddata[i].donedaybidswap* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedaybidMarkup* mstr->swappmul);	
	
		sprintf(push_d->onedayaskswap,    "%.8f", quot->foworddata[i].donedayofferswap* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedayofferMarkup* mstr->swappmul);		
	
		push_l = sizeof(struct push_d);
		if (push_l > MAX_PUSH_LEN)
			push_l = MAX_PUSH_LEN;
		pushdata.pushmsg.msgl = push_l;
		
		myrq_push(&pushdata);
	}
	
		
	
	for ( i = 0; i < MAX_TENNER-2; i++)
	{

		sprintf(pushdata.pushmsg.symb, "%.*s%.*s" , SYMB_LEN,&book->symb,SYMB_SUBLEN,subFowardode[i]);
		strcpy(push_d->code, pushdata.pushmsg.symb);
		for (ii = 0; ii < RTS_DEPTH; ii++)
		{
			SET_PRC(push_d->book[ii].pask, form, sizeof(push_d->book[ii].pask),book->foworddata[i].ask[ii].pask);	
			SET_PRC(push_d->book[ii].pbid, form, sizeof(push_d->book[ii].pbid),book->foworddata[i].bid[ii].pbid);			
		}
		
		sprintf(push_d->swapbidmarkup1, "%.8f", quot->markupdata[i].markupSet[0].bidMarkup* mstr->swappmul);	
		sprintf(push_d->swapbidmarkup2, "%.8f", quot->markupdata[i].markupSet[1].bidMarkup* mstr->swappmul);	
		sprintf(push_d->swapbidmarkup3, "%.8f", quot->markupdata[i].markupSet[2].bidMarkup* mstr->swappmul);		
		sprintf(push_d->swapaskmarkup1, "%.8f", quot->markupdata[i].markupSet[0].offerMarkup* mstr->swappmul);	
		sprintf(push_d->swapaskmarkup2, "%.8f", quot->markupdata[i].markupSet[1].offerMarkup* mstr->swappmul);	
		sprintf(push_d->swapaskmarkup3, "%.8f", quot->markupdata[i].markupSet[2].offerMarkup* mstr->swappmul);
				
		sprintf(push_d->onedaybidswap,    "%.8f", quot->foworddata[i].donedaybidswap* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedaybidMarkup* mstr->swappmul);	
		sprintf(push_d->onedaybidmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedaybidMarkup* mstr->swappmul);	
	
		sprintf(push_d->onedayaskswap,    "%.8f", quot->foworddata[i].donedayofferswap* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup1, "%.8f", quot->foworddata[i].onedayMarkup[0].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup2, "%.8f", quot->foworddata[i].onedayMarkup[1].donedayofferMarkup* mstr->swappmul);	
		sprintf(push_d->onedayaskmarkup3, "%.8f", quot->foworddata[i].onedayMarkup[2].donedayofferMarkup* mstr->swappmul);		
	
		push_l = sizeof(struct push_d);
		if (push_l > MAX_PUSH_LEN)
			push_l = MAX_PUSH_LEN;
		pushdata.pushmsg.msgl = push_l;
		
		myrq_push(&pushdata);
	}

}

//
// folder_push()
// Push real-time message 
//
static void folder_push(MARKET *market, void *folder, int event)
{
	MDFOLD	*fold = folder;
	switch (event)
	{
	case PUSH_QUOT: push_quot(market, &fold->quot, fold->mstr.zdiv); break;
	case PUSH_BOOK: push_book(market, &fold->book, fold->mstr.zdiv); break;
	//case PUSH_MARKET: push_marketquot(market, &fold->quot, fold->mstr.zdiv); break;
	
	}
}

//
// folder_seek()
// Seek all folders for product family 
//
static int folder_seek(MARKET *market, const char *prefix, WHERE *where)
{
	MDARCH	*arch = market->arch;
	INDEX	*indx, *e, *p;
	MDFOLD	*fold = market->fold;
	int	len = strlen(prefix);

	where->n_folder = 0;
	where->l_folder = NULL;
	if (len <= 0)
		return(0);
	if ((p = mds_shmseek(market, prefix)) == NULL)
		return(0);

	indx = market->indx;
	e = &indx[arch->nrec];
	for (; p < e && where->n_folder < MAX_SEEK_FOLD; p++)
	{
		if (strncmp(prefix, p->symb, len) != 0)
			break;
		where->p_folder[where->n_folder++] = &fold[p->indx];
		//if (where->l_folder == NULL && fold[p->indx].mstr.jchk & JCHK_LM)
		if (where->l_folder == NULL && fold[p->indx].mstr.jchk & JCHK_LM &&
			(strstr(fold[p->indx].symb, LM_SUFFIX) == NULL))
			where->l_folder = &fold[p->indx];
	}
	return(where->n_folder);
}

//
// folder_type()
// Return instrument type 
//
static void folder_type(void *folder, int *styp, int *corp, double *strk)
{
	MDFOLD	*fold = folder;
	MDMSTR	*mstr = &fold->mstr;
	
	*styp = mstr->styp;		// insrument type
	*corp = 0;
	*strk = 0.;


}

static int fetch_tick(MARKET *market, MDTICK *tick, int mode, int limit, int cont)
{
	MDFOLD	*fold;

	if ((fold = mds_getfolder(market, tick->symb)) == NULL)
		return(0);

	if (!cont && fold->quot.xymd == 0)		// fetch today data but not opened market
		return(0);
	if (!cont)
		tick->xymd = fold->quot.xymd;		// trading day = today
	else
	{
		// continuous access between trading days
		if (tick->xymd == 0)
			tick->xymd = YMD(9999, 99, 99);
	}
#if 0
	// 2017.07.21 del by bsj
	if (tick->xhms == 0)
	{
		tick->xhms = HMS(99, 99, 99);
		tick->seqn = UINT_MAX;
		//tick->seqn = 99;
	}
#endif
	return(mds_isfetch(market, TICK, tick, mode, limit, cont));
}

static int fetch_trad(MARKET *market, MDTRAD *trad, int mode, int limit, int cont)
{
#if 0
	MDFOLD	*fold;
	int	nnnn, many = 0;
	int	today = 0;

	if ((fold = mds_getfolder(market, trad->symb)) == NULL)
		return(0);

	if (!cont && fold->quot.xymd == 0)		// fetch today data but not opened market
		return(0);
	if (trad->xymd == 0)
	{
		trad->xymd = 99999999;
		trad->xhms = 999999;
	}

	if (!fold->trad.flush && fold->trad.xymd != 0)
	{
		if (trad->xymd > fold->trad.xymd && (mode == ISGREAT || mode == ISGTEQ))
			today = 1;
		else if (trad->xymd == fold->trad.xymd)
		{
			if (trad->xhms > fold->trad.xhms && (mode == ISGREAT || mode == ISGTEQ))
				today = 1;
			else if (trad->xhms == fold->trad.xhms && (mode == ISGTEQ))
				today = 1;
		}
	}
	if (today)
	{
		if (limit > 1)
			memcpy(&trad[1], &trad[0], sizeof(MDTRAD));
		memcpy(&trad[many], &fold->trad, sizeof(MDTRAD));
		many++;
		if (many >= limit)
			return(many);
		trad[many].xymd = trad[0].xymd;
		trad[many].xhms = trad[0].xhms;
	}
	nnnn = mds_isfetch(market, INTR, &trad[many], mode, limit-many, cont);
	if (nnnn < 0)
		return(nnnn);
	many += nnnn;
	return(many);
#endif
	return 0;
}

//static int fetch_heod(MARKET *market, MDHEOD *heod, int mode, int limit)
//{
//#if 0
//	MDFOLD	*fold;
//	int	nnnn, many = 0;
//	WHERE	where;
//	char	symb[SYMB_LEN];
//	int	today = 0;
//
//	if (heod->symb[4] == '0' && heod->symb[5] == '0')
//	{
//		// Continuous symbol : Get a lead month symbol
//		// Example) K101??000 (?? Expired year & month)
//		sprintf(symb, "%.4s", heod->symb);
//		folder_seek(market, symb, &where);
//		if ((fold = where.l_folder) == NULL)
//			return(0);
//	}
//	else if ((fold = mds_getfolder(market, heod->symb)) == NULL)
//		return(0);
//	if (heod->xymd == 0)
//		heod->xymd = 99999999;
//	
//	if (fold->quot.tymd != 0 && fold->quot.last != 0.)
//	{
//		if (heod->xymd == fold->quot.tymd && (mode == ISEQUAL || mode == ISGTEQ))
//			today = 1;
//		else if (heod->xymd >= fold->quot.tymd && mode == ISGTEQ)
//			today = 1;
//		else if (heod->xymd > fold->quot.tymd && mode == ISGREAT)
//			today = 1;
//	}
//	if (today)
//	{
//		if (limit > 1)
//			memcpy(&heod[1], &heod[0], sizeof(MDHEOD));
//		//heod[many].xymd = fold->quot.xymd;
//		heod[many].xymd = fold->quot.tymd;
//		heod[many].open = fold->quot.open;
//		heod[many].high = fold->quot.high;
//		heod[many].low  = fold->quot.low;
//		heod[many].last = fold->quot.last;
//		heod[many].clos = fold->quot.last;
//		heod[many].sign = fold->quot.sign;
//		heod[many].diff = fold->quot.diff;
//		heod[many].rate = fold->quot.rate;
//		heod[many].tvol = fold->quot.tvol;
//		heod[many].tamt = fold->quot.tamt;
//		heod[many].opin = fold->quot.opin;
//		heod[many].zdiv = fold->mstr.zdiv;
//		
//		// 20171030 add
//		uint32_t kymd, kfrhm, ktohm;
//		mds_kortime(market, heod[many].xymd, fold->mstr.session.frhm * 100, &kymd, &kfrhm);
//		mds_kortime(market, heod[many].xymd, fold->mstr.session.tohm * 100, &kymd, &ktohm);
//		heod[many].shms = kfrhm;
//		heod[many].ehms = ktohm;
//		//heod[many].ehms = fold->quot.khms;
//
//		many++;
//		if (many >= limit)
//			return(many);
//		heod[many].xymd = heod[0].xymd;
//
//		mode = ISGREAT;
//	}
//	nnnn = mds_isfetch(market, HEOD, &heod[many], mode, limit-many, 0);
//	if (nnnn < 0)
//		return(nnnn);
//	many += nnnn;
//	return(many);
//#endif
//	return (0);
//}

//
// prefetch()
// Fetch market data from ISAM file
//
static int prefetch(MARKET *market, int dbid, void *buf, int mode, int limit, int cont)
{
	switch (dbid)
	{
		case MSTR: return(mds_isfetch(market, MSTR, buf, mode, limit, 0));
		case QUOT: return(mds_isfetch(market, QUOT, buf, mode, limit, 0));
		case BOOK: return(mds_isfetch(market, BOOK, buf, mode, limit, 0));
		case TICK: return(fetch_tick(market, buf, mode, limit, cont));
		case INTR: return(fetch_trad(market, buf, mode, limit, cont));
	//	case HEOD: return(fetch_heod(market, buf, mode, limit));
		default:   break;
	}
	return(0);
}

//
// Functions to access market data on shared memory
//
static 	METHOD _func_ =  {
	folder_init,			// shm initializer
	folder_roff,			// seek data pointer on folder
	folder_sync,			// synchronize shared memory and file
	folder_push,			// push real-time data
	folder_clear,			// clear folder's data
	folder_seek,			// seek folder's
	folder_type,			// folder's instrument type
	prefetch			// fetch
};

//
// init()
// Set schema type & function procedures to access ISAM file & shared memory 
// for the specified market(domestic futures & options)
//
int mds_init(MARKET *market)
{
	MDCTX	*ctx = market->ctx;

	SELECT_CLR(market->dbis);
	SELECT_ON(market->dbis, MSTR);
	SELECT_ON(market->dbis, QUOT);
	SELECT_ON(market->dbis, TICK);
	SELECT_ON(market->dbis, INTR);
	SELECT_ON(market->dbis, HEOD);

//	ctx->schema  = schema;
	ctx->fsiz = sizeof(MDFOLD);
	ctx->func = &_func_;
	return(0);
}
