#include "mds.h"

/******************************************************************************
* Function Name : GetPriceToDate(char* sbGubn, char *symbol, char* Expiredate)
* Description   : 입력일자의 시세정보를 반환합니다. 
******************************************************************************/
double GetPriceToDate(char* sbGubn, char *symbol, char* Expiredate)
{
	MARKET	*market, m;
	MDFOLD  *folder;
	char	esym[24], exnm[20];
	int		len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		//	sprintf(content->errm, "[%s]거래소를 찾을 수 없습니다.", exnm);
		mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
		return(-1);
	}

	if (folder == NULL)
	{
	//  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
		mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
		return(-1);
	}
	
	MDMSTR  *mstr;
	MDQUOT  *quot;
	char    form[16], value[16];
	char    editb[64];
	char    pind;
	int     exch_id, price;
	int     diff, rate;
	int     rc;
	int		ii;
	double	dval;
	//struct q_price QuotData; 
	
	mstr = &folder->mstr;
	quot = &folder->quot;
	char 	cToday[8];
	
	
	sprintf(cToday, "%08d" ,folder->quot.kymd);
	
	int diffDate = GetDiffDate(cToday ,Expiredate);
	//mds_log(&m, LOG_MUST, "diffdate = %d", diffDate);	
	int Basediff =0; //기준환율과 일자차이	
	int Tnrindex = 0;
	
  

	if (atoi(Expiredate) == atoi(mstr->expiredateSpot))  //spot시세 
	{
		if (sbGubn[0] == '1') //매수 
			return quot->spotdata.bidlast;
		else if (sbGubn[0] == '2') //매도 
			return quot->spotdata.offerlast;

	}
	else if (atoi(Expiredate) == atoi(mstr->expireToday))  //Today시세
	{
		Basediff = 0; 
		Tnrindex = 7;
	}
	else if (atoi(Expiredate) == atoi(mstr->expireTom))  //Tom시세
	{
		Basediff = 0; 
		Tnrindex = 8;
	}
	else if (mstr->expireNday1W >= diffDate)   //1Week 시세 
	{
		Basediff = mstr->expireNday1W - diffDate; 
		Tnrindex = 0;
	}
	else if (mstr->expireNday1M > diffDate) 
	{
		Basediff =  mstr->expireNday1M - diffDate; 
		Tnrindex = 1;
	}
	else if (mstr->expireNday2M > diffDate) 
	{
		Basediff = mstr->expireNday2M - diffDate; 
		Tnrindex = 2;
	}
	else if (mstr->expireNday3M > diffDate) 
	{
		Basediff = mstr->expireNday3M - diffDate; 
		Tnrindex = 3;
	}
	else if (mstr->expireNday6M > diffDate) 
	{
		Basediff = mstr->expireNday6M - diffDate; 
		Tnrindex = 4;
	}
	else if (mstr->expireNday9M > diffDate)  
	{
		Basediff = mstr->expireNday9M - diffDate; 
		Tnrindex = 5;
	}
	else if (mstr->expireNday12M > diffDate) 
	{
		Basediff = mstr->expireNday12M - diffDate; 
		Tnrindex = 6;
	}
	else if  (mstr->expireNday12M < diffDate) 
		return -1;  //가능기간 초과 ..
		

	
	if (sbGubn[0] == '2') //매수 
		return quot->foworddata[Tnrindex].bidlast - (quot->foworddata[Tnrindex].donedaybidswap * Basediff* mstr->swappmul);
	else if (sbGubn[0] == '1') //매도 
		return quot->foworddata[Tnrindex].offerlast - (quot->foworddata[Tnrindex].donedayofferswap * Basediff* mstr->swappmul);
		
	


}



/******************************************************************************
* Function Name : GetQuotToDate(char *symbol, char* Expiredate, S_SENDQUOT* rtQuot)
* Description   : SPOT시세를 rtQuot에 반환합니다.  
******************************************************************************/
int GetQuotToDate(char *symbol, char* Expiredate, S_SENDQUOT* rtQuot)
{
	MARKET	*market, m;
	MDFOLD  *folder;
	
	
	char	esym[24], exnm[20];
	int		len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	
	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");
	
	folder = pibo_getfolder2(exnm, esym, &market); 
	if (market == NULL)
	{
	 	mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
		return -2;
	}

	if (folder == NULL)
	{
		mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
		return -3;
	}
	
	MDMSTR  *mstr;
	MDQUOT  *quot;
	MDBOOK	*book;
	
	char    form[16], value[16];
	char    editb[64];
	char    pind;
	int     exch_id, price;
	int     diff, rate;
	int     rc;
	int		ii;
	double	dval;
	double  a,b;
	mstr = &folder->mstr;
	quot = &folder->quot;
	book = &folder->book;
	char 	cToday[8];

	
	sprintf(cToday, "%08d" ,folder->quot.kymd);
	
	int diffDate = GetDiffDate(cToday ,Expiredate);
	int Basediff =0; //기준환율과 일자차이	
	int Tnrindex = 0;

	
	sprintf(rtQuot->symb, "%s", symbol);	
	rtQuot->sseq	= quot->sseq;
	rtQuot->seqn	= quot->seqn;
	rtQuot->tymd	= quot->tymd;
	rtQuot->xymd	= quot->xymd;
	rtQuot->xhms	= quot->xhms;
	rtQuot->kymd	= quot->kymd;
	rtQuot->khms	= quot->kymd;
	memcpy(&rtQuot->pricedata , &quot->spotdata, sizeof(quot->spotdata));
	return 0;

}





/******************************************************************************
* Function Name : GetCodePind(char *symbol)
* Description   : 종목 소수점 자리수 정보 조회 
******************************************************************************/
int  GetCodePind(char *symbol)
{
	MARKET	*market, m;
	MDFOLD  *folder;
	char	esym[24], exnm[20];
	int		len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		mds_log(&m, LOG_ERROR, "Exchange is not found.  Exchange='%s'", exnm);
		return 0;
	}

  if (folder == NULL)
  {
  //  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
		mds_log(&m, LOG_ERROR,  "Symbol is not found.  esym='%s' for '%s' ", esym, exnm);
    return 0;
  }
  

  return folder->mstr.zdiv;
  
}

/******************************************************************************
* Function Name : GetCodeInfo(char *symbol, S_CODEINFO* sCodeinfo)
* Description   : 종목 기본 정보 조회 
******************************************************************************/
int  GetCodeInfo(char *symbol, S_CODEINFO* sCodeinfo)
{
	MARKET	*market, m;
  	MDFOLD  *folder;
	char	esym[24], exnm[20];
	int		len;
 	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
	//	sprintf(content->errm, "[%s]거래소를 찾을 수 없습니다.", exnm);
		mds_log(&m, LOG_ERROR, "Exchange is not found.  Exchange='%s'", exnm);
		return -1;
	}
	
	if (folder == NULL)
	{
	//  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
		mds_log(&m, LOG_ERROR,  "Symbol is not found.  esym='%s' for '%s' ", esym, exnm);
		return -2;
	}

	sCodeinfo->nSwapPind  = folder->mstr.swapzdiv;
	if (memcmp(symbol,"USDKRW", 6) != 0)
	{
		sCodeinfo->nPind = folder->mstr.zdiv;
		sCodeinfo->nCpind=  folder->mstr.zCustdiv;	
	}else
	{
		sCodeinfo->nPind = folder->mstr.zdiv;
		sCodeinfo->nCpind = folder->mstr.zCustdiv;		
	}
  
	sCodeinfo->dSwapAdjust= folder->mstr.swappmul;
	sCodeinfo->nAmountUnit= folder->mstr.nAmountUnit;
	sprintf(sCodeinfo->dispUnit, "%s", folder->mstr.dispUnit);
	sprintf(sCodeinfo->spotExpireDate, "%s", folder->mstr.expiredateSpot);
	
	
	return 0;
  
}





// 가격 수동 입력.  수기 SMBS 초기가격은 이곳을 통해 생성후 입력이 된다. 
int CustomPriceSetting(int iSource, int iPriceType, char *symbol, char* date, char* time, char* bidPrice, char* offerPrice)
{
//	if (iSource != 2)
//		return 0;
	MARKET	*market, m;
	char	exnm[40];
	int		rc;	

	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	if ((symbol==NULL) || (date==NULL) || (time==NULL) || (bidPrice==NULL) || (offerPrice==NULL) )
	{
		mds_log(&m, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&m, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-1);
	}

	if (strlen(symbol) < 6 )
	{
		mds_log(&m, LOG_MUST, "SymbolCode is Too Short '%s'\n", symbol);
		return(-1);	
	}
	
	
	struct cmbsquote	quote;
	memset(&quote ,0,sizeof(quote));
	quote.type = 'Q';
	sprintf(quote.symb, "%.*s", sizeof(quote.symb), symbol);
	sprintf(quote.ccy1, "%.3s", &quote.symb[0]);
	sprintf(quote.ccy2, "%.3s", &quote.symb[3]);
	sprintf(quote.date         , "%.*s", sizeof(quote.date         ), date);
	sprintf(quote.time         , "%.*s", sizeof(quote.time         ), time);
	sprintf(quote.cMDReqID     , "%.*s", sizeof(quote.cMDReqID     ), "");
	sprintf(quote.bidprice     , "%.*s", sizeof(quote.bidprice     ), bidPrice);
	sprintf(quote.bidQty       , "%.*s", sizeof(quote.bidQty       ), "0");
	sprintf(quote.biddate      , "%.*s", sizeof(quote.biddate      ), date);
	sprintf(quote.bidQuoteID   , "%.*s", sizeof(quote.bidQuoteID   ), "");
	sprintf(quote.bidSettType  , "%.*s", sizeof(quote.bidSettType  ), "");
	sprintf(quote.bidBestPx    , "%.*s", sizeof(quote.bidBestPx    ), bidPrice);
	sprintf(quote.bidBestSize  , "%.*s", sizeof(quote.bidBestSize  ), "0");
	sprintf(quote.offerprice   , "%.*s", sizeof(quote.offerprice   ), offerPrice);
	sprintf(quote.offerQty     , "%.*s", sizeof(quote.offerQty     ), "0");
	sprintf(quote.offerdate    , "%.*s", sizeof(quote.offerdate    ), date);
	sprintf(quote.offerQuoteID , "%.*s", sizeof(quote.offerQuoteID ), "");
	sprintf(quote.offerSettType, "%.*s", sizeof(quote.offerSettType), "");
	sprintf(quote.offerBestPx  , "%.*s", sizeof(quote.offerBestPx  ), offerPrice);
	sprintf(quote.offerBestSize, "%.*s", sizeof(quote.offerBestSize), "0");
	mds_log(&m, LOG_MUST, "[%-15s][%4d] CustomPriceSetting. [%s]  [%s] [%s] [%s]  ", __FUNCTION__, __LINE__,symbol, date, time,bidPrice, offerPrice);
	SendAndSavePriceData(market,iSource, &quote, sizeof(quote)); 
	mds_log(&m, LOG_MUST, "[%-15s][%4d] CustomPriceSetting. ", __FUNCTION__, __LINE__);
	mds_close(market);
	return 0;
}



//스왑환율을 저장한다.iSource => 1: From CMBS 2: From 수기입력
int SwapPriceSetting(int iSource, S_SENDSWAP swapquot)
{
	MARKET	*market, mc;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	

	swapquot.symb[6] = 0x00;

	//if ((symbol==NULL) || (date==NULL) || (time==NULL) || (bidPrice==NULL) || (offerPrice==NULL) )
	if (strlen(swapquot.symb) ==0 )
	{
		mds_log(&mc, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return(-2);
	}
	
	
	MDFOLD	*folder;//, *rfolder; 
	//MDFOLD	*krwfolder;
	MDQUOT	*mdquot, mdq;
	MDMSTR	*m;
	struct	q_data	qd;
	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, swapquot.symb)) == NULL)
	{ 
			mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, swapquot.symb);
		  return(NULL);
	}

	//1.메모리에 값 반영. 
	mdquot = &folder->quot;
	m = &folder->mstr;
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 스왑환율 메모리저장. [%s]", __FUNCTION__, __LINE__, swapquot.symb);
	memcpy(&mdq, mdquot, sizeof(MDQUOT));
	m->pmul = 1;
	mdq.tymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.seqn++;
	mdq.xymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.xhms = str2i(swapquot.time, sizeof(swapquot.time))* 1000;
	mdq.kymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.khms = str2i(swapquot.time, sizeof(swapquot.time))* 1000;

	struct q_price* q = mdq.swapdata;


	mdq.swapdata[0].bidlast  	 = str2f(swapquot.swapbid1W		,FlotePoint);//	,sizeof(swapquot.swapbid1W	));
	mdq.swapdata[0].bidbest    = str2f(swapquot.swapbid1W		,FlotePoint);//		,sizeof(swapquot.swapbid1W	));
	mdq.swapdata[0].offerlast  = str2f(swapquot.swapoffer1W	,FlotePoint);//		,sizeof(swapquot.swapoffer1W));
	mdq.swapdata[0].offerbest  = str2f(swapquot.swapoffer1W	,FlotePoint);//		,sizeof(swapquot.swapoffer1W));
	mdq.swapdata[1].bidlast    = str2f(swapquot.swapbid1M		,FlotePoint);//		,sizeof(swapquot.swapbid1M	));
	mdq.swapdata[1].bidbest    = str2f(swapquot.swapbid1M		,FlotePoint);//		,sizeof(swapquot.swapbid1M	));
	mdq.swapdata[1].offerlast  = str2f(swapquot.swapoffer1M	,FlotePoint);//		,sizeof(swapquot.swapoffer1M));
	mdq.swapdata[1].offerbest  = str2f(swapquot.swapoffer1M	,FlotePoint);//		,sizeof(swapquot.swapoffer1M));	
	mdq.swapdata[2].bidlast    = str2f(swapquot.swapbid2M		,FlotePoint);//		,sizeof(swapquot.swapbid1M	));
	mdq.swapdata[2].bidbest    = str2f(swapquot.swapbid2M		,FlotePoint);//		,sizeof(swapquot.swapbid1M	));
	mdq.swapdata[2].offerlast  = str2f(swapquot.swapoffer2M	,FlotePoint);//		,sizeof(swapquot.swapoffer1M));
	mdq.swapdata[2].offerbest  = str2f(swapquot.swapoffer2M	,FlotePoint);//		,sizeof(swapquot.swapoffer1M));		
	mdq.swapdata[3].bidlast    = str2f(swapquot.swapbid3M		,FlotePoint);//		,sizeof(swapquot.swapbid3M	));
	mdq.swapdata[3].bidbest    = str2f(swapquot.swapbid3M		,FlotePoint);//		,sizeof(swapquot.swapbid3M	));
	mdq.swapdata[3].offerlast  = str2f(swapquot.swapoffer3M	,FlotePoint);//		,sizeof(swapquot.swapoffer3M));
	mdq.swapdata[3].offerbest  = str2f(swapquot.swapoffer3M	,FlotePoint);//		,sizeof(swapquot.swapoffer3M));
	mdq.swapdata[4].bidlast		 = str2f(swapquot.swapbid6M		,FlotePoint);//		,sizeof(swapquot.swapbid6M	));
	mdq.swapdata[4].bidbest    = str2f(swapquot.swapbid6M		,FlotePoint);//		,sizeof(swapquot.swapbid6M	));
	mdq.swapdata[4].offerlast  = str2f(swapquot.swapoffer6M	,FlotePoint);//		,sizeof(swapquot.swapoffer6M));
	mdq.swapdata[4].offerbest  = str2f(swapquot.swapoffer6M	,FlotePoint);//		,sizeof(swapquot.swapoffer6M));	
	mdq.swapdata[5].bidlast    = str2f(swapquot.swapbid9M		,FlotePoint);//		,sizeof(swapquot.swapbid9M	));
	mdq.swapdata[5].bidbest    = str2f(swapquot.swapbid9M		,FlotePoint);//		,sizeof(swapquot.swapbid9M	));
	mdq.swapdata[5].offerlast  = str2f(swapquot.swapoffer9M	,FlotePoint);//		,sizeof(swapquot.swapoffer9M));
	mdq.swapdata[5].offerbest  = str2f(swapquot.swapoffer9M	,FlotePoint);//		,sizeof(swapquot.swapoffer9M));	
	mdq.swapdata[6].bidlast		 = str2f(swapquot.swapbid12M	,FlotePoint);//	  ,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[6].bidbest    = str2f(swapquot.swapbid12M	,FlotePoint);//		,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[6].offerlast  = str2f(swapquot.swapoffer12M,FlotePoint);//		,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[6].offerbest  = str2f(swapquot.swapoffer12M,FlotePoint);//		,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[7].bidlast	   = (str2f(swapquot.swapbidTOM	,FlotePoint)+ str2f(swapquot.swapbidSpot	,FlotePoint)) ;//	  ,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[7].bidbest    = (str2f(swapquot.swapbidTOM	,FlotePoint)+ str2f(swapquot.swapbidSpot	,FlotePoint)) ;//		,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[7].offerlast  = (str2f(swapquot.swapofferTOM,FlotePoint)+str2f(swapquot.swapofferSpot,FlotePoint))  ;//		,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[7].offerbest  = (str2f(swapquot.swapofferTOM,FlotePoint)+str2f(swapquot.swapofferSpot,FlotePoint))  ;//		,sizeof(swapquot.swapoffer12M));	
	mdq.swapdata[8].bidlast	   = str2f(swapquot.swapbidSpot	,FlotePoint);//	  ,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[8].bidbest    = str2f(swapquot.swapbidSpot	,FlotePoint);//		,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[8].offerlast  = str2f(swapquot.swapofferSpot,FlotePoint);//		,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[8].offerbest  = str2f(swapquot.swapofferSpot,FlotePoint);//		,sizeof(swapquot.swapoffer12M));	
	mdq.swapdata[9].bidlast		=0;// str2f(swapquot.swapbidSpot	,FlotePoint);//	  ,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[9].bidbest    = 0;//str2f(swapquot.swapbidSpot	,FlotePoint);//		,sizeof(swapquot.swapbid12M	));
	mdq.swapdata[9].offerlast  = 0;//str2f(swapquot.swapofferSpot,FlotePoint);//		,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[9].offerbest  = 0;//str2f(swapquot.swapofferSpot,FlotePoint);//		,sizeof(swapquot.swapoffer12M));	
		
	mdq.foworddata[0].donedaybidswap    = (mdq.swapdata[0].bidlast    ) / ( m->expireNday1W -m->expireNdaySpot);
	mdq.foworddata[0].donedayofferswap  = (mdq.swapdata[0].offerlast ) / ( m->expireNday1W -m->expireNdaySpot);
	mdq.foworddata[1].donedaybidswap    = (mdq.swapdata[1].bidlast   - mdq.swapdata[0].bidlast    ) / ( m->expireNday1M -m->expireNday1W);
	mdq.foworddata[1].donedayofferswap  = (mdq.swapdata[1].offerlast - mdq.swapdata[0].offerlast) / ( m->expireNday1M -m->expireNday1W);
	mdq.foworddata[2].donedaybidswap    = (mdq.swapdata[2].bidlast   - mdq.swapdata[1].bidlast    ) / ( m->expireNday2M -m->expireNday1M);
	mdq.foworddata[2].donedayofferswap  = (mdq.swapdata[2].offerlast - mdq.swapdata[1].offerlast) / ( m->expireNday2M -m->expireNday1M);
	mdq.foworddata[3].donedaybidswap    = (mdq.swapdata[3].bidlast   - mdq.swapdata[2].bidlast    ) / ( m->expireNday3M -m->expireNday2M);
	mdq.foworddata[3].donedayofferswap  = (mdq.swapdata[3].offerlast - mdq.swapdata[2].offerlast) / ( m->expireNday3M -m->expireNday2M);	
	mdq.foworddata[4].donedaybidswap    = (mdq.swapdata[4].bidlast   - mdq.swapdata[3].bidlast    ) / ( m->expireNday6M -m->expireNday3M);
	mdq.foworddata[4].donedayofferswap  = (mdq.swapdata[4].offerlast - mdq.swapdata[3].offerlast) / ( m->expireNday6M -m->expireNday3M);
	mdq.foworddata[5].donedaybidswap    = (mdq.swapdata[5].bidlast   - mdq.swapdata[4].bidlast    ) / ( m->expireNday9M -m->expireNday6M);
	mdq.foworddata[5].donedayofferswap  = (mdq.swapdata[5].offerlast - mdq.swapdata[4].offerlast) / ( m->expireNday9M -	m->expireNday6M);
	mdq.foworddata[6].donedaybidswap    = (mdq.swapdata[6].bidlast   - mdq.swapdata[5].bidlast    ) / ( m->expireNday12M -	m->expireNday9M);
	mdq.foworddata[6].donedayofferswap  = (mdq.swapdata[6].offerlast - mdq.swapdata[5].offerlast) / ( m->expireNday12M -	m->expireNday9M);
	mdq.foworddata[7].donedaybidswap    = 0;
	mdq.foworddata[7].donedayofferswap  = 0;
	mdq.foworddata[8].donedaybidswap    = 0;
	mdq.foworddata[8].donedayofferswap  = 0;
	mdq.foworddata[9].donedaybidswap    = 0;
	mdq.foworddata[9].donedayofferswap  = 0;


	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 스왑환율 메모리저장완료. [%s]", __FUNCTION__, __LINE__, swapquot.symb);	

	//2. DB에 저장 
	S_SENDQUOT sendquot;
	memset(&sendquot, 0, sizeof(S_SENDQUOT));
	sendquot.xymd = mdq.xymd;
	sendquot.xhms = mdq.xhms;
	sendquot.kymd = mdq.kymd;
	sendquot.khms = mdq.khms;
	int nRet = 0;
	for (count = 0; count < MAX_TENNER-1; count++)
	{
		sprintf(sendquot.symb, "%.*s%.*s" , SYMB_LEN,swapquot.symb,SYMB_SUBLEN,subSwapcode[count]);
		memcpy(&sendquot.pricedata, 	&mdq.swapdata[count], sizeof(mdq.swapdata[count]));	
		nRet = INSERTTICKDATA(market,&sendquot, sizeof(S_SENDQUOT),m->zdiv, iSource);
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] INSERTTICKDATA. [%d]", __FUNCTION__, __LINE__, nRet);
	}
		
	memcpy(mdquot, &mdq, sizeof(MDQUOT));
	mds_close(market);
	return nRet;
}

int initSwapPriceData()
{ 
	return initSwapPrice();
}

/******************************************************************************
* Function Name : MarkupSetting(S_SENDMARKUP markupData, int nMarkupType)
* Description   : 마크업 정보를 메모리에 입력 합니다.
******************************************************************************/
int MarkupSetting(S_SENDMARKUP markupData, int nMarkupType)
{
	MARKET	*market, mc;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	

	markupData.symb[6] = 0x00;

	//if ((symbol==NULL) || (date==NULL) || (time==NULL) || (bidPrice==NULL) || (offerPrice==NULL) )
	if (strlen(markupData.symb) ==0 )
	{
		mds_log(&mc, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return(-2);
	}
	
	
	MDFOLD	*folder;//, *rfolder; 
	//MDFOLD	*krwfolder;
	MDQUOT	*mdquot, mdq;
	MDMSTR	*m;
	struct	q_data	qd;
	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, markupData.symb)) == NULL)
	{ 
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, markupData.symb);
		return(NULL);
	}

	//1.메모리에 값 반영. 
	mdquot = &folder->quot;
	m = &folder->mstr;
	
	memcpy(&mdq, mdquot, sizeof(MDQUOT));
	m->pmul = 1;
//	mdq.tymd = str2i(markupData.time, sizeof(markupData.time));
	mdq.seqn++;
//	mdq.xymd = str2i(markupData.date, sizeof(markupData.date));
//	mdq.xhms = str2i(markupData.time, sizeof(markupData.time))* 1000;
//	mdq.kymd = str2i(markupData.date, sizeof(markupData.date));
//	mdq.khms = str2i(markupData.time, sizeof(markupData.time))* 1000;

	//struct q_price* q = mdq.swapdata;

	mdq.markupdata[0].markupSet[nMarkupType].bidMarkup			  = str2f(markupData.markUpbid1W		,FlotePoint);//	
	mdq.markupdata[0].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid1W		,FlotePoint);//	
	mdq.markupdata[0].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer1W	,FlotePoint);//	
	mdq.markupdata[0].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer1W	,FlotePoint);//	
	mdq.markupdata[1].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid1M		,FlotePoint);//	
	mdq.markupdata[1].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid1M		,FlotePoint);//	
	mdq.markupdata[1].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer1M	,FlotePoint);//	
	mdq.markupdata[1].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer1M	,FlotePoint);//	
	mdq.markupdata[2].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid2M		,FlotePoint);//	
	mdq.markupdata[2].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid2M		,FlotePoint);//	
	mdq.markupdata[2].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer2M	,FlotePoint);//	
	mdq.markupdata[2].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer2M	,FlotePoint);//		
	mdq.markupdata[3].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid3M		,FlotePoint);//	
	mdq.markupdata[3].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid3M		,FlotePoint);//	
	mdq.markupdata[3].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer3M	,FlotePoint);//	
	mdq.markupdata[3].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer3M	,FlotePoint);//	
	mdq.markupdata[4].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid6M		,FlotePoint);//	
	mdq.markupdata[4].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid6M		,FlotePoint);//	
	mdq.markupdata[4].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer6M	,FlotePoint);//	
	mdq.markupdata[4].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer6M	,FlotePoint);//	
	mdq.markupdata[5].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid9M		,FlotePoint);//	
	mdq.markupdata[5].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid9M		,FlotePoint);//	
	mdq.markupdata[5].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer9M	,FlotePoint);//	
	mdq.markupdata[5].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer9M	,FlotePoint);//	
	mdq.markupdata[6].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbid12M	  ,FlotePoint);//	
	mdq.markupdata[6].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbid12M	 ,FlotePoint);//	
	mdq.markupdata[6].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpoffer12M,FlotePoint);//	
	mdq.markupdata[6].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpoffer12M,FlotePoint);//	
	mdq.markupdata[7].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbidTOD	,FlotePoint);//	
	mdq.markupdata[7].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbidTOD	,FlotePoint);//	
	mdq.markupdata[7].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpofferTOD,FlotePoint);//	
	mdq.markupdata[7].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpofferTOD,FlotePoint);//		
	mdq.markupdata[8].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbidTOM	 ,FlotePoint);//	
	mdq.markupdata[8].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbidTOM	 ,FlotePoint);//	
	mdq.markupdata[8].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpofferTOM,FlotePoint);//	
	mdq.markupdata[8].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpofferTOM,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].bidMarkup		    = str2f(markupData.markUpbidSMBSSpot	,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].bidCMBSMarkup	  = str2f(markupData.markUpbidSpot	,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].bidSMBSMarkup	  = str2f(markupData.markUpbidSMBSSpot	,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].offerMarkup		  = str2f(markupData.markUpofferSMBSSpot,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].offerCMBSMarkup  = str2f(markupData.markUpofferSpot,FlotePoint);//	
	mdq.markupdata[9].markupSet[nMarkupType].offerSMBSMarkup  = str2f(markupData.markUpofferSMBSSpot,FlotePoint);//		
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] mdq.markupdata[9].markupSet[%d].bidMarkup [%f] [%s]", __FUNCTION__, __LINE__, nMarkupType,mdq.markupdata[9].markupSet[nMarkupType].bidMarkup,markupData.symb);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] mdq.markupdata[9].markupSet[%d].offerMarkup [%f] [%s]", __FUNCTION__, __LINE__,nMarkupType, mdq.markupdata[9].markupSet[nMarkupType].offerMarkup,markupData.symb);	
	
	mdq.foworddata[0].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[0].markupSet[nMarkupType].bidMarkup   ) / ( m->expireNday1W -m->expireNdaySpot);
	mdq.foworddata[0].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[0].markupSet[nMarkupType].offerMarkup ) / ( m->expireNday1W -m->expireNdaySpot);
	mdq.foworddata[1].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[1].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[0].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday1M -m->expireNday1W);
	mdq.foworddata[1].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[1].markupSet[nMarkupType].offerMarkup - mdq.markupdata[0].markupSet[nMarkupType].offerMarkup) / ( m->expireNday1M -m->expireNday1W);
	mdq.foworddata[2].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[2].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[1].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday2M -m->expireNday1M);
	mdq.foworddata[2].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[2].markupSet[nMarkupType].offerMarkup - mdq.markupdata[1].markupSet[nMarkupType].offerMarkup) / ( m->expireNday2M -m->expireNday1M);
	mdq.foworddata[3].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[3].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[2].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday3M -m->expireNday2M);
	mdq.foworddata[3].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[3].markupSet[nMarkupType].offerMarkup - mdq.markupdata[2].markupSet[nMarkupType].offerMarkup) / ( m->expireNday3M -m->expireNday2M);
	mdq.foworddata[4].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[4].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[3].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday6M -m->expireNday3M);
	mdq.foworddata[4].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[4].markupSet[nMarkupType].offerMarkup - mdq.markupdata[3].markupSet[nMarkupType].offerMarkup) / ( m->expireNday6M -m->expireNday3M);
	mdq.foworddata[5].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[5].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[4].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday9M -m->expireNday6M);
	mdq.foworddata[5].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[5].markupSet[nMarkupType].offerMarkup - mdq.markupdata[4].markupSet[nMarkupType].offerMarkup) / ( m->expireNday9M -	m->expireNday6M);
	mdq.foworddata[6].onedayMarkup[nMarkupType].donedaybidMarkup	  = (mdq.markupdata[6].markupSet[nMarkupType].bidMarkup   - mdq.markupdata[5].markupSet[nMarkupType].bidMarkup    ) / ( m->expireNday12M -	m->expireNday9M);
	mdq.foworddata[6].onedayMarkup[nMarkupType].donedayofferMarkup  = (mdq.markupdata[6].markupSet[nMarkupType].offerMarkup - mdq.markupdata[5].markupSet[nMarkupType].offerMarkup) / ( m->expireNday12M -	m->expireNday9M);
	mdq.foworddata[7].onedayMarkup[nMarkupType].donedaybidMarkup  = 0;
	mdq.foworddata[7].onedayMarkup[nMarkupType].donedayofferMarkup  = 0;
	mdq.foworddata[8].onedayMarkup[nMarkupType].donedaybidMarkup  = 0;
	mdq.foworddata[8].onedayMarkup[nMarkupType].donedayofferMarkup  = 0;
	mdq.foworddata[9].onedayMarkup[nMarkupType].donedaybidMarkup  = 0;
	mdq.foworddata[9].onedayMarkup[nMarkupType].donedayofferMarkup  = 0;
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 0[%f]", __FUNCTION__, __LINE__, mdq.foworddata[0].onedayMarkup[nMarkupType].donedaybidMarkup);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 1[%f]", __FUNCTION__, __LINE__, mdq.foworddata[1].onedayMarkup[nMarkupType].donedaybidMarkup);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 2[%f]", __FUNCTION__, __LINE__, mdq.foworddata[2].onedayMarkup[nMarkupType].donedaybidMarkup);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 3[%f]", __FUNCTION__, __LINE__, mdq.foworddata[3].onedayMarkup[nMarkupType].donedaybidMarkup);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 4[%f]", __FUNCTION__, __LINE__, mdq.foworddata[4].onedayMarkup[nMarkupType].donedaybidMarkup);	
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 마크업 메모리저장완료. [%s]", __FUNCTION__, __LINE__, markupData.symb);	
	int nRet = 0;
	memcpy(mdquot, &mdq, sizeof(MDQUOT));
	mds_close(market);
	return nRet;
}

//마크업 그룹 3개를 로드 한다. 
int initMarkupPriceData()
{ 
	//내부거래
	if (initMarkupPrice(MARKUPEMP,0) != 0)
		return -1;
	//중소기업
	if (initMarkupPrice(MARKUPPRI,1) != 0)
		return -1;	
	//대기업
	if (initMarkupPrice(MARKUPENT,2) != 0)
		return -1;	
	return 0;
}

int initPriceOrign()
{
	MARKET mc;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	return roadPriceOrign(&mc);
	
}


int SetPriceOrign(PRICEORIGN* bp)
{
	
	MARKET	*market, mc;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0; 
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	bp->fx_prdct_cd[6] = 0x00;

	if (strlen(bp->fx_prdct_cd) ==0 )
	{
		mds_log(&mc, LOG_MUST, "SetPriceOrign : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return(-2);
	}

	MDFOLD	*folder;

	MDQUOT	*mdq;
	MDMSTR	*m;
	struct	q_data	qd;
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] SetPriceOrign [%s]", __FUNCTION__, __LINE__,bp->fx_prdct_cd);	
	/*-------------------------
	 * data set 
	 * -----------------------*/
	if ((folder = mds_getfolder(market, bp->fx_prdct_cd)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "등록된 종목정보가 없습니다. [%s]", bp->fx_prdct_cd);
		return(NULL);
	}

	m = &folder->mstr;	
	m->pricestat = atoi(bp->prc_orign_dstic);
	m->smbstime  = atoi(bp->smbs_aply_start_hms);
	m->cmbstime  = atoi(bp->cmbs_aply_start_hms);
	m->stoptime  = atoi(bp->exrt_ofer_dscn_hms);

	mds_close(market);
	return 0;
}


int initBasePriceData()
{
	MARKET mc;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	return initBasePrice(&mc);
	
}


int SetBasePrice(BASEPRICEINPUT* bp)
{

	MARKET	*market, mc;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0; 
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	bp->symb[6] = 0x00;

	if (strlen(bp->symb) ==0 )
	{
		mds_log(&mc, LOG_MUST, "SetBasePrice : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return(-2);
	}
	MDFOLD	*folder;

	MDQUOT	*mdq;
	MDMSTR	*m;
	struct	q_data	qd;

	/*-------------------------
	 * data set 
	 * -----------------------*/
	if ((folder = mds_getfolder(market, bp->symb)) == NULL)
	{
			mds_log(&mc, LOG_MUST, "등록된 종목정보가 없습니다. [%s]", bp->symb);
		  return(NULL);
	}
	
	//1.메모리에 값 반영. 
	mdq = &folder->quot;
	m = &folder->mstr;
	
	mdq->spotdata.bidbase  		= str2f(bp->bidspotbase		,	FlotePoint);//,sizeof(bp->bidspotbase		));
	mdq->spotdata.offerbase  	= str2f(bp->offerspotbase	,	FlotePoint);//,sizeof(bp->offerspotbase	));
	mdq->spotdata.midbase  		= str2f(bp->midspotbase		,	FlotePoint);//,sizeof(bp->midspotbase		));

	mdq->spotdata.bidopen	= str2f(bp->bidOpenprice		,	FlotePoint);//,sizeof(bp->bidspotbase		));
	mdq->spotdata.bidhigh  	= str2f(bp->bidHighprice	,	FlotePoint);//,sizeof(bp->offerspotbase	));
	mdq->spotdata.bidlow	= str2f(bp->bidLowprice		,	FlotePoint);//,sizeof(bp->midspotbase		));
	
	mdq->spotdata.offeropen	= str2f(bp->offerOpenprice		,	FlotePoint);//,sizeof(bp->bidspotbase		));
	mdq->spotdata.offerhigh = str2f(bp->offerHighprice	,	FlotePoint);//,sizeof(bp->offerspotbase	));
	mdq->spotdata.offerlow	= str2f(bp->offerLowprice		,	FlotePoint);//,sizeof(bp->midspotbase		));

	
	int nRet = reloadmaster(market,m);
	if ( nRet != 1)
	{
			mds_log(&mc, LOG_MUST, "[%-15s][%4d] reloadmaster Fail . [%s] [%d]", __FUNCTION__, __LINE__, bp->symb, nRet);
		
	}

	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 전일종가 로드. [%s]", __FUNCTION__, __LINE__, bp->symb);
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 전일종가 로드. [%f]", __FUNCTION__, __LINE__, mdq->spotdata.midbase);
	mds_close(market);
	return 0;
}



int ReloadVirtualQty()
{
	MARKET	*market, mc;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0; 
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return -2;
	}
	MDFOLD	*folder;
	MDMSTR	*m;
	/*-------------------------
	 * data set 
	 * -----------------------*/
	if ((folder = mds_getfolder(market, "USDKRW")) == NULL)
	{
		mds_log(&mc, LOG_MUST, "등록된 종목정보가 없습니다. [%s]", "USDKRW");
		return -1;
	}

	//1.메모리에 값 반영. 
	m = &folder->mstr;	
	int nRet = reloadmaster(market,m);
	if ( nRet != 1)
	{
			mds_log(&mc, LOG_MUST, "[%-15s][%4d] reloadmaster Fail . [%s] [%d]", __FUNCTION__, __LINE__, "USDKRW", nRet);
		
	}
	mds_close(market);
	return 0;
}


	
int insertCalcCAmount(char *symbol, DATAFIELD df)
{
	
	MARKET	*market, mc;
	MDFOLD	*folder;
	char	exnm[40];
	int		rc;	
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");

	if (strlen(symbol) ==0 )
	{
		mds_log(&mc, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n","Custom");
		return(-2);
	}
	
	

	/*-------------------------
	 * data set USDKRW 만 가상잔량 반영으로 해당 폴더만 반영 
	 * -----------------------*/
	if ((folder = mds_getfolder(market, symbol)) == NULL)
	{ 
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, symbol);
		return(-3);
	}
	
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] insertCalcCAmount [%s]", __FUNCTION__, __LINE__, symbol);
	MDTRAD	*mdtrad;
	MDMSTR	*m;
	struct	q_data	qd;
	//1.메모리에 값 반영. 
	mdtrad = &folder->trad;
	m = &folder->mstr;
	int nEnd = mdtrad->nEnd;
									
  	mds_plussecound(m->nOrderrecy_ttm, df.exymd, df.xhms, &mdtrad->orderdata[nEnd].exymd	, &mdtrad->orderdata[nEnd].exhms	);		
	mdtrad->orderdata[nEnd].xhms		= df.xhms;
	//mdtrad->orderdata[nEnd].exymd		= df.exymd;
	//mdtrad->orderdata[nEnd].exhms		= df.exhms;
	mdtrad->orderdata[nEnd].nhoga		= df.nhoga;
	mdtrad->orderdata[nEnd].dAmount = df.dAmount;
	mdtrad->nEnd = mdtrad->nEnd+1;
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] xhms [%d] xhms [%d] xhms [%d] xhms [%d] ", __FUNCTION__, __LINE__, df.exymd, df.xhms, mdtrad->orderdata[nEnd].exymd	, mdtrad->orderdata[nEnd].exhms );
	CalcCAmount(folder);
	//잔량이 변경되어서 시세를 내려줘야 한다. 
	char	check[MAX_ISAM_F];
	memset(&check[0], 0, sizeof(check));
	check[BOOK] = X_PUSH;
	sendfolder(market, folder, check, SOURCE_HANDWRTING);
	mds_close(market);
	return 0;

}
	
	
//종목별 체결 수량 합산 
int CalcCAmount(MDFOLD	*folder)
{
	int ii = 0, jj=0;
	int  nStart= 0;
	int  nEnd= 0, nRealIndex =0;

	MARKET  *market2, mc;
	;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	
	uint32_t kymd =0;
	uint32_t khms=0;
	uint32_t exymd =0;
	uint32_t exhms=0;
	time_t  clock;
	time(&clock);
	mds_ktime(NULL, clock, &kymd, &khms);
	//mds_plussecound(50, kymd, khms, &exymd, &exhms);
	//
	MDMSTR *m = &folder->mstr;
	MDBOOK	*book = &folder->book;
	MDTRAD	*trad = &folder->trad;
	nStart = trad->nStart;
	nEnd   = trad->nEnd;
	
	if (nStart > nEnd) //한바퀴돌아서 처음부터 데이터 저장하는 경우 
	{
		nEnd = nEnd + MAX_SAVEORDER;		
	}
	double dTotalask =0;
	double dTotalbid =0;
	// 주문리스트 처음부터 끝까지 읽어온다. 
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] nStart [%d] nEnd [%d] ", __FUNCTION__, __LINE__, nStart, nEnd);	  
	for (ii = nStart; ii<nEnd; ii++)
	{
		nRealIndex = ii%MAX_SAVEORDER;
		//유효일자가 더 현재일자 보다 더크면 24시간을 더해서 계산한다. 
		exymd =trad->orderdata[nRealIndex].exymd;
		exhms = trad->orderdata[nRealIndex].exhms;		
		
		if (exymd > kymd)
			exhms = exhms + 240000;	
		// 현재시간이 주문 유효시간보다 작으면 유효한 주문이르로 가상체결량에 더해준다 
		//mds_log(&mc, LOG_MUST, "[%-15s][%4d] kymd [%d] khms [%d] exymd [%d] exhms [%d] ", __FUNCTION__, __LINE__, kymd, khms, exymd, exhms);	  	 
		if (exhms > khms)
		{
			if (trad->orderdata[nRealIndex].nhoga >= 0) 
			{
				dTotalask   = dTotalask + trad->orderdata[nRealIndex].dAmount;
			}else
			{
				dTotalbid   = dTotalbid + trad->orderdata[nRealIndex].dAmount;
			}
					
		}
		else		//아니면 다음번에 현재 위치에서부터 계산하도록 처음을 변경해준다. 
		{
				trad->nStart = nRealIndex; 		
		}
	}
	
//	mds_log(&mc, LOG_MUST, "[%-15s][%4d] dTotalask [%f] dTotalbid [%f]", __FUNCTION__, __LINE__, dTotalask, dTotalbid);	  
	book->spotdata.cask = dTotalask;
	book->spotdata.cbid = dTotalbid;

			
	

}
	
	
double GetMid(double bidprice, double askprice, int zdiv)
{
	return (bidprice + askprice ) / 2;
	
}

int color(double val, double base)
{

	if (val > base) return 2;
    else if (val < base) return 5;
    else return 3;
}



// 통화코드의 가격 원천을 변경 
int SetPriceSource(char *symbol, int iSource)
{
	char	esym[24], exnm[20];
	MARKET	*market,mc;
	MDFOLD  *folder;
	MDMSTR  *mstr;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	int		len;
	int 	nRet;
	STR2S(exnm, "CMBS");
	STR2S(esym, symbol);
	esym[6] = 0x00;


	if (symbol==NULL)
	{
		mds_log(&mc, LOG_MUST, "SetPriceSource : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}
	
	if (strlen(symbol) < 6 )
	{
		mds_log(&mc, LOG_MUST, "SymbolCode is Too Short '%s'\n", symbol);
		return(-3);	
	}
	
	if ((iSource < SOURCE_SMBS) || (iSource > SOURCE_STOP))
	{
		mds_log(&mc, LOG_MUST, "Source vaoue is too big or too short '%d'\n", iSource);
		return(-3);		
	}
	
	
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}
	
	if (market == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d]거래소를 찾을 수 없습니다. [%s]", __FUNCTION__, __LINE__, exnm);
		return(-4);
	}
	
  if (folder == NULL)
  {
  	mds_log(&mc, LOG_MUST, "[%-15s][%4d]Symbol is not found. [%s]  ", __FUNCTION__, __LINE__, esym);
  	return(-5);
  }
  
	mstr = &folder->mstr;

	
	if (mstr == NULL)
	{
		mds_log(&market, LOG_MUST, "[%-15s][%4d]종목정보가 없습니다.", __FUNCTION__, __LINE__);
		return -6;
	}

	mstr->pricestat = iSource;
	
	// 이후 해당 시세원천에 맞게 처리 필요
	mds_close(market);
	return 0;
}

