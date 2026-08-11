static char sqla_program_id[292] = 
{
 '\xac','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x5a','\x41','\x49','\x4d','\x4b','\x52','\x48','\x6f','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x8','\x0','\x4d','\x44','\x53','\x52','\x44','\x42','\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0'
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {'C',' ',' ',' '}};


static const short sqlIsLiteral   = SQL_IS_LITERAL;
static const short sqlIsInputHvar = SQL_IS_INPUT_HVAR;


#line 1 "mdsrdb.sqc"
//
// Common schema interface initializer
// Set procedure for market data cooking
//
#include	"mds2.h"
#define		_SCHEMA_H_
#include 	"comrdb.h"

#define		SQL_CNT		sqlca.sqlerrd[2]
#define		FlotePoint	8
// ★TODO-18 : 마크업구분코드. CMBS, Reuter, Manual 인 경우 '2'  그 외는 '1'
#define		GetFeeMrkup(excode)		((excode == 'C' || excode == 'R' || excode == 'H') ? '2' : '1')



/*
EXEC SQL INCLUDE SQLCA;
*/

/* SQL Communication Area - SQLCA - structures and constants */
#include "sqlca.h"
struct sqlca sqlca;


#line 15 "mdsrdb.sqc"


extern	MARKET	*market;

/*
//##### mds/src/lib/mds/mdsrdb.sqc
	int LoadMaster(MARKET *market);
	int INSERTTICKDATA(MARKET *market, MDSSISE *psise);
	int INSERTTICKDATA_CR(MARKET *market, MDFOLD *pfold, MDSSISE *pusdkrw);
*/

//###################################################################################
//##### mds/src/lib/mds/mdsrdb.sqc

/* 아래 함수의 합본임.
	loadmaster()
	InitMarketDate()
	initBasePriceData()
	SetBasePrice()
	reloadmaster()
	initSwapPriceData()
	initSwapPrice()
	initMarkupPriceData()
	initPriceOrign()
	roadPriceOrign()
	InitTrade()
	loadData()
	CustomPriceSetting() -> SendAndSavePriceData()
*/

int LoadMaster(MARKET *m)
{
	int		rtn, initflag=DF_OFF;
	INDEX	*indx, *pindx;
	MDARCH	*parch;
	MDFOLD	*fold, *pfold;
	MDSSISE	msise;
	uint32_t ymd, hms;


/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 54 "mdsrdb.sqc"

	char	o_usdpos				[ 1+1];
	char	o_fx_prdcd_cd			[ 6+1];
	char	o_exrt_repsnt_nocip		[10+1];
	char	o_ofpr_cust_rlnu_nocip	[10+1];
	char	o_swap_rate_nocip		[10+1];
	char	o_spt_ymd				[ 8+1];
	char	o_trdf					[ 1+1];
	//========================================
	// TODO : DECIMAL 타입을 인쿼리 함수 DOUBLE()로 형변환 하면 nn.n20 으로 떨어지지 않고 nn.n199999999997 과 같이 변환 됨.
	//        이런 현상으로 인해 double 형 변수에 직접 할당하지 못하고 char로 받아옴.
	//========================================
	char	v_baseymd				[ 8+1];
	char	v_bid_last_prc			[26+1];
	char	v_offer_last_prc		[26+1];
	char	v_mid_last_prc			[26+1];
	char	v_bid_open_prc			[26+1];
	char	v_bid_high_prc			[26+1];
	char	v_bid_low_prc			[26+1];
	char	v_offer_open_prc		[26+1];
	char	v_offer_high_prc		[26+1];
	char	v_offer_low_prc			[26+1];
	char	v_mid_open_prc			[26+1];
	char	v_mid_high_prc			[26+1];
	char	v_mid_low_prc			[26+1];
	char	v_fcm_id				[10+1];
	char	v_mrkup_dstcd			[ 1+1];

	char	v_biz_ymd				[ 8+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 83 "mdsrdb.sqc"


	parch	= (MDARCH *)m->arch;
	pindx	= (INDEX  *)m->indx;
	pfold	= (MDFOLD *)m->fold;

	rtn = l_db2connect();		// 메모리 초기화를 위한 임시 CONNECT
	if (rtn != 0)
	{
		mds_log(m, MLOG_ERROR, "[%s] l_db2connect failed : (%d:%s)",__func__, rtn, SQLERRM);
		return(-1);
	}

	// 영업일자 조회
	memset(v_biz_ymd, 0x00, sizeof(v_biz_ymd));


/*
EXEC SQL
	SELECT	거래기준년월일
	  INTO	:v_biz_ymd
	  FROM	INST1.TSKEIAM00
	WITH UR;
*/

{
#line 103 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 103 "mdsrdb.sqc"
  sqlaaloc(3,1,1,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 103 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 103 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_biz_ymd;
#line 103 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 103 "mdsrdb.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 103 "mdsrdb.sqc"
  sqlacall((unsigned short)24,1,0,3,0L);
#line 103 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 103 "mdsrdb.sqc"

	if (SQLCODE != 0)
	{
		mds_log(m, MLOG_ERROR,"[%s] select 거래기준년월일 error (%d:%s)", __FUNCTION__, SQLCODE, SQLERRM);
		return(-1);
	}

//	mds_time(m, 0, &ymd, &hms, NULL, NULL);	// get local date & time
//	parch->tymd = ymd;
	l_rtrim(v_biz_ymd);
	if (parch->tymd != atoi(v_biz_ymd))		initflag = DF_ON;		// 영업일자 넘어가면 메모리 초기화
	parch->tymd = atoi(v_biz_ymd);

	memset(v_fcm_id, 0x00, sizeof(v_fcm_id));
	memcpy(v_fcm_id, parch->xchg.exnm, sizeof(parch->xchg.exnm));
	l_rtrim(v_fcm_id);

	/*===============================================================
	 * 마스터메모리 적재 대상 통화 조회
	   > TODO : 원천(excode)별 대상 통화가 다를 시에는 TSKEIAM85 테이블에 KEY 추가필요
	   >>>> 원천별 대상통화 테이블(TSKEIAM88) 참조하여 trdf 세팅
	   >>>>  TSKEIAM88.TRNABIL_YN 컬럼 '0'=거래불가, '1'=거래가능
	===============================================================*/

/*
EXEC SQL DECLARE NEWMDS_C1 CURSOR FOR
	SELECT	 A.FX_PRDCT_CD
			,A.EXRT_REPSNT_NOCIP
			,A.OFPR_CUST_RLNU_NOCIP
			,A.SWAP_RATE_NOCIP
			,DECODE(A.BASE_CNCYCD, 'USD', 0, 1)
			,A.SPT_YMD
			,CASE WHEN :v_fcm_id NOT IN ('CUST', 'BEST')
					THEN NVL(DECODE(B.TRNABIL_YN, '0', '9', '0'), '9') ELSE '0' END		-- 초기 세팅을 '0' 으로
	  FROM	 INST1.TSKEIAM85 A
			,(SELECT FX_PRDCT_CD, TRNABIL_YN FROM INST1.TSKEIAM88 WHERE FCM_ID = :v_fcm_id) B
	 WHERE	A.GROUP_CO_CD = 'KB0'
	   AND	A.FINCL_YN    = '0'
	   AND	A.MSTR_USE_YN = '1'
	   AND  A.FX_PRDCT_CD = B.FX_PRDCT_CD(+)
	ORDER BY A.FX_PRDCT_CD
	WITH UR
	;
*/

#line 143 "mdsrdb.sqc"



/*
EXEC SQL OPEN NEWMDS_C1;
*/

{
#line 145 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 145 "mdsrdb.sqc"
  sqlaaloc(2,2,2,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 145 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 11;
#line 145 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fcm_id;
#line 145 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 145 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 11;
#line 145 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fcm_id;
#line 145 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 145 "mdsrdb.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 145 "mdsrdb.sqc"
  sqlacall((unsigned short)26,2,2,0,0L);
#line 145 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 145 "mdsrdb.sqc"

	if (SQLCODE != 0)
	{
		mds_log(m, MLOG_ERROR, "[%-15s][%4d] ERROR OPEN NEWMDS_C1 [%s]\n", __FUNCTION__, __LINE__, SQLERRM);
		
/*
EXEC SQL CLOSE NEWMDS_C1;
*/

{
#line 149 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 149 "mdsrdb.sqc"
  sqlacall((unsigned short)20,2,0,0,0L);
#line 149 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 149 "mdsrdb.sqc"

		return(-1);
	}

	// 현 상태 백업 용
	int trdf, pricestat;

	// update_master() 호출시 사용되는 msise 구조체 용 시간
	struct tm *tm;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);

	parch->nrec = 0;

	while (1)
	{
		memset(o_fx_prdcd_cd,			0x00, sizeof(o_fx_prdcd_cd));
		memset(o_exrt_repsnt_nocip,		0x00, sizeof(o_exrt_repsnt_nocip));
		memset(o_ofpr_cust_rlnu_nocip,	0x00, sizeof(o_ofpr_cust_rlnu_nocip));
		memset(o_swap_rate_nocip,		0x00, sizeof(o_swap_rate_nocip));
		memset(o_usdpos, 				0x00, sizeof(o_usdpos));
		memset(o_spt_ymd, 				0x00, sizeof(o_spt_ymd));
		memset(o_trdf, 					0x00, sizeof(o_trdf));

	
/*
EXEC SQL
		FETCH NEWMDS_C1
		INTO	 :o_fx_prdcd_cd
				,:o_exrt_repsnt_nocip
				,:o_ofpr_cust_rlnu_nocip
				,:o_swap_rate_nocip
				,:o_usdpos
				,:o_spt_ymd
				,:o_trdf
		;
*/

{
#line 184 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 184 "mdsrdb.sqc"
  sqlaaloc(3,7,3,0L);
    {
      struct sqla_setdata_list sql_setdlist[7];
#line 184 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 184 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)o_fx_prdcd_cd;
#line 184 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 11;
#line 184 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)o_exrt_repsnt_nocip;
#line 184 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 11;
#line 184 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)o_ofpr_cust_rlnu_nocip;
#line 184 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 11;
#line 184 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)o_swap_rate_nocip;
#line 184 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 2;
#line 184 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)o_usdpos;
#line 184 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 9;
#line 184 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)o_spt_ymd;
#line 184 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 2;
#line 184 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)o_trdf;
#line 184 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 184 "mdsrdb.sqc"
      sqlasetdata(3,0,7,sql_setdlist,0L,0L);
    }
#line 184 "mdsrdb.sqc"
  sqlacall((unsigned short)25,2,0,3,0L);
#line 184 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 184 "mdsrdb.sqc"

		if (SQLCODE == SQL_RC_W100)
		{
			if (SQL_CNT == 0)
			{
				mds_log(m, MLOG_WARNING, "[%s:%d] NOTFOUND FETCH NEWMDS_C1 [%s][%d]", __FUNCTION__, __LINE__, SQLERRM, SQL_CNT);
			}
			break;
		}
		else if (SQLCODE != 0)
		{
			mds_log(m, MLOG_ERROR, "[%s:%d] ERROR FETCH NEWMDS_C1 [%s]\n", __FUNCTION__, __LINE__, SQLERRM);
			
/*
EXEC SQL CLOSE NEWMDS_C1;
*/

{
#line 196 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 196 "mdsrdb.sqc"
  sqlacall((unsigned short)20,2,0,0,0L);
#line 196 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 196 "mdsrdb.sqc"

			return(-2);
		}

		l_rtrim(o_fx_prdcd_cd);
		l_rtrim(o_exrt_repsnt_nocip);
		l_rtrim(o_ofpr_cust_rlnu_nocip);
		l_rtrim(o_swap_rate_nocip);
		l_rtrim(o_spt_ymd);
		l_rtrim(o_trdf);

		fold = (MDFOLD *)&pfold[parch->nrec];
		indx = (INDEX  *)&pindx[parch->nrec];

		// 영업일 변경 시 메모리 초기화		
		if (initflag)	memset(fold, 0x00, sizeof(MDFOLD));

		strcpy(fold->symb, o_fx_prdcd_cd);
		fold->zdiv		= atoi(o_exrt_repsnt_nocip);
		fold->zCustdiv	= atoi(o_ofpr_cust_rlnu_nocip);
//		fold->swapzdiv	= atoi(o_swap_rate_nocip);		// quotalarm 으로 대체
		fold->quotalarm = DF_OFF;
		fold->usdpos	= atoi(o_usdpos);
		memcpy(fold->spotdate, o_spt_ymd, sizeof(fold->spotdate));
// ★TODO-15 : 24시간 가동하게 되면 초기화 시 영업일과 현재일자는 다를 수 있으므로 로직 체크 필요
		fold->tymd		= (uint32_t)parch->tymd;

		// 초기화 전 시세처리 LOCK 및 현 상태 백업
//		pricestat		= fold->pricestat;
		trdf			= fold->trdf;

//		fold->pricestat	= SISE_STOP;
		fold->trdf		= DF_OFF;

		// INDEX 정보 세팅
		strcpy(indx->symb, o_fx_prdcd_cd);
		indx->indx	= parch->nrec;
		fold->seqn	= parch->nrec;

		/*==============================================================
	 	 * 마스터메모리 데이터 초기화 (from TSKEIMU52 일데이터)
		===============================================================*/
		memset(v_baseymd,			0x00, sizeof(v_baseymd));
		memset(v_bid_last_prc,		0x00, sizeof(v_bid_last_prc));
		memset(v_offer_last_prc,	0x00, sizeof(v_offer_last_prc));
		memset(v_mid_last_prc,		0x00, sizeof(v_mid_last_prc));
		memset(v_mrkup_dstcd,		0x00, sizeof(v_mrkup_dstcd));

		// 전영업일자의 최종 환율정보 조회
	
/*
EXEC SQL
		SELECT	 REGI_YMD
				,MRKUP_DSTCD
				,BID_LAST_PRC
				,OFFER_LAST_PRC
				,(BID_LAST_PRC + OFFER_LAST_PRC)/2 --MID_LAST_PRC
		  INTO	 :v_baseymd
				,:v_mrkup_dstcd
				,:v_bid_last_prc
				,:v_offer_last_prc
				,:v_mid_last_prc
		  FROM	INST1.TSKEIMU52
		 WHERE	GROUP_CO_CD = 'KB0'
		   AND	FX_PRDCT_CD = :o_fx_prdcd_cd
		   AND	(BID_LAST_PRC <> 0 AND OFFER_LAST_PRC <> 0)
--		   AND	REGI_YMD    = (SELECT MAX(REGI_YMD) FROM INST1.TSKEIMU52 WHERE REGI_YMD < :v_biz_ymd)
		ORDER BY REGI_YMD DESC
		FETCH FIRST 1 ROWS ONLY		-- 어제일자 없으면 최종일자로 사용해도 되는지(?)
		WITH UR
		;
*/

{
#line 264 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 264 "mdsrdb.sqc"
  sqlaaloc(2,1,4,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)o_fx_prdcd_cd;
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 264 "mdsrdb.sqc"
  sqlaaloc(3,5,5,0L);
    {
      struct sqla_setdata_list sql_setdlist[5];
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_baseymd;
#line 264 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 2;
#line 264 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_mrkup_dstcd;
#line 264 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 27;
#line 264 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_bid_last_prc;
#line 264 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 27;
#line 264 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_offer_last_prc;
#line 264 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 27;
#line 264 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_mid_last_prc;
#line 264 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 264 "mdsrdb.sqc"
      sqlasetdata(3,0,5,sql_setdlist,0L,0L);
    }
#line 264 "mdsrdb.sqc"
  sqlacall((unsigned short)24,3,2,3,0L);
#line 264 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 264 "mdsrdb.sqc"

		if (SQLCODE != 0)
		{
			mds_log(m, MLOG_DEBUG, "[%s:%d] Get baseprc FROM TSKEIMU52 ERROR [%s][%s] \n", __FUNCTION__, __LINE__, o_fx_prdcd_cd, SQLERRM);
			// 전일종가 없을 시.. 첫시세 수신하면 전일종가 세팅 (updatemast)
			v_mrkup_dstcd[0]	= '1';
			sprintf(v_bid_last_prc,   "%f", 0.);
			sprintf(v_offer_last_prc, "%f", 0.);
			sprintf(v_mid_last_prc,   "%f", 0.);
		}
		mds_log(m, MLOG_BIZ, "[%s] 전일종가(%.8s) [%s:%.6s:%.1s] [%s/%s/%s]", __func__, v_baseymd, m->exnm, o_fx_prdcd_cd, o_trdf, v_bid_last_prc, v_offer_last_prc, v_mid_last_prc);

		// 2024.03.22) USDKRW 의 전일종가는 INST1.TSKEIMU59 테이블 값을 사용하는 것으로 재조회
		if (memcmp(fold->symb, DF_USDKRW, 6) == 0)
		{
			memset(v_mid_last_prc, 0x00, sizeof(v_mid_last_prc));

		
/*
EXEC SQL
			SELECT	MID_LAST_PRC
			  INTO	:v_mid_last_prc
			  FROM	INST1.TSKEIMU59
			 WHERE	GROUP_CO_CD    = 'KB0'
			   AND	FX_PRDCT_CD    = :o_fx_prdcd_cd
			   AND	FX_PRDCT_DSTCD ='3'
			   AND	TNR_CD         ='00'
			   AND	REGI_YMD = (SELECT MAX(REGI_YMD) FROM INST1.TSKEIMU52 WHERE REGI_YMD < :v_biz_ymd)
			WITH UR
			;
*/

{
#line 291 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 291 "mdsrdb.sqc"
  sqlaaloc(2,2,6,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)o_fx_prdcd_cd;
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 291 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 291 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_biz_ymd;
#line 291 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 291 "mdsrdb.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 291 "mdsrdb.sqc"
  sqlaaloc(3,1,7,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 27;
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mid_last_prc;
#line 291 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 291 "mdsrdb.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 291 "mdsrdb.sqc"
  sqlacall((unsigned short)24,4,2,3,0L);
#line 291 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 291 "mdsrdb.sqc"

			if (SQLCODE != 0)
			{
				mds_log(m, MLOG_DEBUG, "[%s:%d] SELECT PRC FROM TSKEIMU59 ERROR [%s][%s] \n", __FUNCTION__, __LINE__, o_fx_prdcd_cd, SQLERRM);
				// 전일종가 없을 시.. 첫시세 수신하면 전일종가 세팅 (updatemast)
				// continue;
			}
			mds_log(m, MLOG_BIZ, "[%s] USDKRW 전일종가 재조회 [%s:%.6s] [%s]", __func__, m->exnm, o_fx_prdcd_cd, v_mid_last_prc);
		}

		// 전일 최종 시세를 가져와 당일의 시/고/저/종 환율을 초기화 한다.
		fold->baseprc	= str2d(v_mid_last_prc, FlotePoint);	// premid
		fold->prebid	= str2d(v_bid_last_prc, FlotePoint);
		fold->preoffer	= str2d(v_offer_last_prc, FlotePoint);

		fold->bidlast = fold->bidopen = fold->bidhigh = fold->bidlow = fold->prebid;
		fold->offerlast = fold->offeropen = fold->offerhigh = fold->offerlow = fold->preoffer;
		fold->midopen = fold->midhigh = fold->midlow = fold->baseprc;

		// update_mast()을 호출을 위한 시세패킷 세팅
		memset(&msise, 0x00, sizeof(msise));

		// BEST, CUST 인경우는 실제 EXCODE가 아닌 'B', 'Z'로 초기화 됨
		if (parch->xchg.excode[0] == DF_EXCD_CUST)
		{
			// CUST 경우, 해당 통화 시세가 안내려오면 'Z'로 남아있어서 SpotMargin 계산이 되지 않아 'S'/'C'로 초기화 함.
			if (v_mrkup_dstcd[0] == '1')	msise.excode[0] = 'S';
			else							msise.excode[0] = 'C';
		}
		else
			memcpy (msise.excode, parch->xchg.excode, sizeof(msise.excode));

		memcpy (msise.symb,		fold->symb,			sizeof(msise.symb));
		sprintf(msise.date,		"%04d%02d%02d",		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		sprintf(msise.time,		"%02d%02d%02d000", 	tm->tm_hour, tm->tm_min, tm->tm_sec);

		// 당일 시세 있는 경우, 당일 최종 시세로 MDSSISE 패킷 생성
		memset(v_bid_last_prc,		0x00, sizeof(v_bid_last_prc));
		memset(v_offer_last_prc,	0x00, sizeof(v_offer_last_prc));
		memset(v_mid_last_prc,		0x00, sizeof(v_mid_last_prc));
		memset(v_bid_open_prc,		0x00, sizeof(v_bid_open_prc));
		memset(v_bid_high_prc,		0x00, sizeof(v_bid_high_prc));
		memset(v_bid_low_prc,		0x00, sizeof(v_bid_low_prc));
		memset(v_offer_open_prc,	0x00, sizeof(v_offer_open_prc));
		memset(v_offer_high_prc,	0x00, sizeof(v_offer_high_prc));
		memset(v_offer_low_prc,		0x00, sizeof(v_offer_low_prc));
		memset(v_mid_open_prc,		0x00, sizeof(v_mid_open_prc));
		memset(v_mid_high_prc,		0x00, sizeof(v_mid_high_prc));
		memset(v_mid_low_prc,		0x00, sizeof(v_mid_low_prc));

		// 당일 최종 환율정보 조회
		
/*
EXEC SQL
		SELECT	 BID_LAST_PRC
				,OFFER_LAST_PRC
				,MID_LAST_PRC
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE BID_OPEN_PRC   END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE BID_HIGH_PRC   END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE BID_LOW_PRC    END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE OFFER_OPEN_PRC END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE OFFER_HIGH_PRC END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE OFFER_LOW_PRC  END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE (BID_OPEN_PRC + OFFER_OPEN_PRC)/2 END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE (BID_HIGH_PRC + OFFER_HIGH_PRC)/2 END
				,CASE WHEN(RGST_TIME = '000000000'OR REGI_YMD != :v_biz_ymd)  THEN 0 ELSE (BID_LOW_PRC + OFFER_LOW_PRC)/2   END
		  INTO	 :v_bid_last_prc
		  		,:v_offer_last_prc
		  		,:v_mid_last_prc
				,:v_bid_open_prc
				,:v_bid_high_prc
				,:v_bid_low_prc
				,:v_offer_open_prc
				,:v_offer_high_prc
				,:v_offer_low_prc
				,:v_mid_open_prc
				,:v_mid_high_prc
				,:v_mid_low_prc
		  FROM	INST1.TSKEIMU52
		 WHERE	GROUP_CO_CD	= 'KB0'
--		   AND	FX_EXCODE	= :i_excode
		   AND	FX_PRDCT_CD = :o_fx_prdcd_cd
		   AND	REGI_YMD    = :v_biz_ymd
		WITH UR
		;
*/

{
#line 373 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 373 "mdsrdb.sqc"
  sqlaaloc(2,11,8,0L);
    {
      struct sqla_setdata_list sql_setdlist[11];
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 7;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)o_fx_prdcd_cd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 9;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_biz_ymd;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sqlasetdata(2,0,11,sql_setdlist,0L,0L);
    }
#line 373 "mdsrdb.sqc"
  sqlaaloc(3,12,9,0L);
    {
      struct sqla_setdata_list sql_setdlist[12];
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_bid_last_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_offer_last_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_mid_last_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_open_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_bid_high_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_low_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_offer_open_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_offer_high_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_low_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_mid_open_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_mid_high_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 27;
#line 373 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_mid_low_prc;
#line 373 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 373 "mdsrdb.sqc"
      sqlasetdata(3,0,12,sql_setdlist,0L,0L);
    }
#line 373 "mdsrdb.sqc"
  sqlacall((unsigned short)24,5,2,3,0L);
#line 373 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 373 "mdsrdb.sqc"


		// 당일 데이터가 없는 경우. 전일 최종 데이터로 시세패킷 생성
		if (SQLCODE == SQL_RC_W100)
		{
			if (SQL_CNT == 0)
			{
				mds_log(m, MLOG_BIZ, "[%s] 당일데이터 없음 [%s:%.6s]", __func__, m->exnm, o_fx_prdcd_cd);

				msise.bidprc		= fold->bidlast;
				msise.bidbestprc	= fold->bidlast;
				msise.offerprc		= fold->offerlast;
				msise.offerbestprc	= fold->offerlast;
			}
		}
		// 당일 데이터가 있는 경우(장중 재기동) 조회된 값으로 마스트 메모리 세팅
		else
		{
			mds_log(m, MLOG_BIZ, "[%s] 당일데이터 [%s:%.6s] [%s/%s/%s]", __func__, m->exnm, o_fx_prdcd_cd, v_bid_last_prc, v_offer_last_prc, v_mid_last_prc);
			// ★TODO-16 : 장중에 마스터메모리 다시 적재 시 전일대비 세팅 로직 검토
			fold->bidlast		= str2d(v_bid_last_prc,		FlotePoint);
			fold->offerlast		= str2d(v_offer_last_prc,	FlotePoint);
			
			fold->bidopen		= str2d(v_bid_open_prc,		FlotePoint);
			fold->bidhigh		= str2d(v_bid_high_prc,		FlotePoint);
			fold->bidlow		= str2d(v_bid_low_prc,		FlotePoint);
			fold->offeropen		= str2d(v_offer_open_prc,	FlotePoint);
			fold->offerhigh		= str2d(v_offer_high_prc,	FlotePoint);
			fold->offerlow		= str2d(v_offer_low_prc,	FlotePoint);
			fold->midlast		= str2d(v_mid_last_prc,		FlotePoint);
			fold->midopen		= str2d(v_mid_open_prc,		FlotePoint);
			fold->midhigh		= str2d(v_mid_high_prc,		FlotePoint);
			fold->midlow		= str2d(v_mid_low_prc,		FlotePoint);
			
			// 당일 최종 데이터로 시세패킷 생성
			msise.bidprc		= str2d(v_bid_last_prc,		FlotePoint);
			msise.bidbestprc	= str2d(v_bid_last_prc,		FlotePoint);
			msise.offerprc		= str2d(v_offer_last_prc,	FlotePoint);
			msise.offerbestprc	= str2d(v_offer_last_prc,	FlotePoint);
		}
		
		msise.bidqty		= 0;
		msise.bidbestqty	= 0;
		msise.offerqty		= 0;
		msise.offerbestqty	= 0;

		/*==============================================================
	 	 * 가상의 내부시세패킷(MDSSISE) 생성 후 update_mast()을 호출
	 	  > update_master() 는 장중 시세 수신 시 호출되는 함수
	 	  > 환율외의 나머지 필드의 값을 초기화
		===============================================================*/
		rtn = update_master(m, fold, &msise);
		if (rtn < 0)
		{
			mds_log(m, MLOG_WARNING, "[%s] UPDATE MAST SHM(FOLD) ERROR [%s][%s] \n", __func__, o_fx_prdcd_cd, SQLERRM);
			continue;
		}

		// ★TODO-11 : 통화별 시세전송 상태 구분 체크 및 세팅로직 체크 필요 : if (pfold->pricestat == SOURCE_STOP - 1) //시세정지
		// 최초 초기화시에는 거래정지 상태. 시세 수신 시에 update_master() 에서 trdf=1로 세팅. 중지이후 해제하였을 경우 시장가 체결방지.

		// 초기화 후 시세처리 LOCK 해제 : 최초 초기화가 아닌 경우는 기존 상태 복원
//		fold->pricestat	= (first ? SISE_MARKET : pricestat);
		// TSKEIAM88.TRNABIL_YN 세팅에 따라 거래불가 통화 세팅
		if (o_trdf[0] == '9')	fold->trdf = 9;
		else					fold->trdf = 0;		// ★★★★ 장중 재로딩 시에도 거래불가 상태로 초기화
//		else					fold->trdf = (first ? 0 : trdf);

		/*===============================================================
		 * 통화별 고객시세 처리 시간 정보 세팅 (CUST 마스트에서만 사용)
		===============================================================*/
/* CHECK : 커서 안에서 호출하면 -501 오류남. main() 으로 이동.

		if (m->excode[0] == DF_EXCD_CUST)
		{
			if (SetCustFeed(m, fold) < 0)
				mds_log(m, MLOG_WARNING, "[%s] [%s] SetCustFeed error [%.6s]..", __func__, m->exnm, fold->symb);
		}
*/
		mds_log(m, MLOG_BIZ, "## [%s] [%s] SETUP MASTER SHM for [%.6s:%d] DONE..", __func__, m->exnm, fold->symb, fold->trdf);
		
		// NEXT
		(parch->nrec)++;
	}

	// 24.07.08) 북 평가액 산출을 위해 SMBS 시세로 MU59 테이블의 현재가 Update 여부 flag. '1'로 초기화.
	// - (4181) 화면을 통해 17시 이후에 정산환율 수신하면 Update 중지('0'). (배치에서 flag 업데이트)
	parch->xchg.custflag = DF_C_ON;

	
/*
EXEC SQL CLOSE NEWMDS_C1;
*/

{
#line 462 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 462 "mdsrdb.sqc"
  sqlacall((unsigned short)20,2,0,0,0L);
#line 462 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 462 "mdsrdb.sqc"


	rtn = l_db2commit();
	if (rtn != 0)
	{
		mds_log(m, MLOG_ERROR, "[%s] l_db2commit failed : (%d:%s)",__func__, rtn, SQLERRM);
		return(NULL);
	}

	// INDEX 메모리 정렬 (SELECT 시 ORDER BY 했기 때문에 차이 없음) - 추후 제거 요망
	mds_shmidxsort(m);

	// cursor 안에서 통화쌍별로 SetCustFeed() 호출하면 -501 오류남.
	if (SetCustFeed_all(m) < 0)
	{
		mds_log(market, MLOG_WARNING, "[%s] SetCustFeed_all error..", __func__);
		return (-1);
	}
	
	rtn = l_db2disconnect();
	if (rtn != 0)
	{
		mds_log(m, MLOG_ERROR, "[%s] l_db2disconnect failed : (%d:%s)\n",__func__, rtn, SQLERRM);
		return (-1);
	}

	return (parch->nrec);

} /* End of int LoadMaster(MARKET *m) */


int InsertTrdTick(MDSSISE *psise)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 497 "mdsrdb.sqc"

	char	v_prc_orign_dstcd	[ 1+1];
	char	v_fx_prdct_cd		[ 6+1];
	char	v_trade_ymd			[ 8+1];
	char	v_trade_time		[ 9+1];
	char	v_trade_side		[ 1+1];
	char	v_trade_prc			[20+1];
	char	v_trade_vol			[20+1];
	char	v_fx_ex_quote_id	[30+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 506 "mdsrdb.sqc"


  //--------------------------------------------------------------------
  // 1. initialization
  //--------------------------------------------------------------------
	memset(v_prc_orign_dstcd,	0x00, sizeof(v_prc_orign_dstcd));
	memset(v_fx_prdct_cd,		0x00, sizeof(v_fx_prdct_cd));
	memset(v_trade_ymd,			0x00, sizeof(v_trade_ymd));
	memset(v_trade_time,		0x00, sizeof(v_trade_time));
	memset(v_trade_side,		0x00, sizeof(v_trade_side));
	memset(v_trade_prc,			0x00, sizeof(v_trade_prc));
	memset(v_trade_vol,			0x00, sizeof(v_trade_vol));
	memset(v_fx_ex_quote_id,	0x00, sizeof(v_fx_ex_quote_id));

	// ★TODO-18 : 마크업구분코드. CMBS, Reuter, Manual 인 경우 '2'  그 외는 '1'
	v_prc_orign_dstcd[0] = psise->excode[0];

	sprintf(v_fx_prdct_cd,	"%.6s%",	psise->symb);
	sprintf(v_trade_ymd,	"%.8s",		psise->date);
	sprintf(v_trade_time,	"%.9s",		psise->time);

	if (psise->bidprc > 0.)
	{
		v_trade_side[0] = '1';
		sprintf(v_trade_prc,	"%.*f",		pind, psise->bidprc);
		sprintf(v_trade_vol,	"%.f",		psise->bidqty);
	}
	else
	{
		v_trade_side[0] = '2';
		sprintf(v_trade_prc,	"%.*f",		pind, psise->offerprc);
		sprintf(v_trade_vol,	"%.f",		psise->offerqty);
	}
	sprintf(v_fx_ex_quote_id, "%s", psise->quotid);

	l_rtrim(v_prc_orign_dstcd);
	l_rtrim(v_fx_prdct_cd);
	l_rtrim(v_trade_ymd);
	l_rtrim(v_trade_time);
	l_rtrim(v_trade_side);
	l_rtrim(v_trade_prc);
	l_rtrim(v_trade_vol);
	l_rtrim(v_fx_ex_quote_id);

	//--------------------------------------------------------------------
	// 1. initialization
	//--------------------------------------------------------------------
	
/*
EXEC SQL 
	INSERT INTO INST1.TSKEIMU80 (
			 GROUP_CO_CD
			,PRC_ORIGN_DSTCD
			,FX_PRDCT_CD
			,TRADE_YMD
			,TRADE_TIME
			,TRADE_SIDE
			,TRADE_PRC
			,TRADE_VOL
			,FX_EX_QUOTE_ID
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
	) VALUES (
			'KB0'
			,:v_prc_orign_dstcd
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00)
			,:v_trade_time
			,:v_trade_side
			,:v_trade_prc
			,:v_trade_vol
			,:v_fx_ex_quote_id
			, SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
			,'KEI000'
	);
*/

{
#line 578 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 578 "mdsrdb.sqc"
  sqlaaloc(2,7,10,0L);
    {
      struct sqla_setdata_list sql_setdlist[7];
#line 578 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 578 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_prc_orign_dstcd;
#line 578 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 578 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 578 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 578 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_trade_time;
#line 578 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 2;
#line 578 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_trade_side;
#line 578 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 578 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_trade_prc;
#line 578 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 578 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_trade_vol;
#line 578 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 31;
#line 578 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_fx_ex_quote_id;
#line 578 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 578 "mdsrdb.sqc"
      sqlasetdata(2,0,7,sql_setdlist,0L,0L);
    }
#line 578 "mdsrdb.sqc"
  sqlacall((unsigned short)24,6,2,0,0L);
#line 578 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 578 "mdsrdb.sqc"


	if (SQLCODE != 0 && abs(SQLCODE) != 803)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU80 insert error [%.1s:%.6s] [%s]  \n", psise->excode, v_fx_prdct_cd, SQLERRM);
		return(-1);
	}

	return(0);
} /* End of int InsertTrdTick(MDSSISE *psise) */


int InsertTick(MDSSISE *psise, MDFOLD *pfold)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 594 "mdsrdb.sqc"

	char	v_mrkup_dstcd		[ 1+1];
	char	v_fx_prdct_cd		[ 6+1];
	char	v_rgst_date			[ 8+1];
	char	v_rgst_time			[ 9+1];
	char	v_bid_last_prc		[20+1];
	char	v_offer_last_prc	[20+1];
	char	v_mid_last_prc		[20+1];
	char	v_fx_ex_quote_id	[30+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 603 "mdsrdb.sqc"


  //--------------------------------------------------------------------
  // 1. initialization
  //--------------------------------------------------------------------
	memset(v_mrkup_dstcd,		0x00, sizeof(v_mrkup_dstcd));
	memset(v_fx_prdct_cd,		0x00, sizeof(v_fx_prdct_cd));
	memset(v_rgst_date,			0x00, sizeof(v_rgst_date));
	memset(v_rgst_time,			0x00, sizeof(v_rgst_time));
	memset(v_bid_last_prc,		0x00, sizeof(v_bid_last_prc));
	memset(v_offer_last_prc,	0x00, sizeof(v_offer_last_prc));
	memset(v_mid_last_prc,		0x00, sizeof(v_mid_last_prc));
	memset(v_fx_ex_quote_id,	0x00, sizeof(v_fx_ex_quote_id));

	// ★TODO-18 : 마크업구분코드. CMBS, Reuter, Manual 인 경우 '2'  그 외는 '1'
// #define		GetFeeMrkup(excode)		((excode == 'C' || excode == 'R' || excode == 'H') ? '2' : '1')
	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	sprintf(v_fx_prdct_cd,		"%.6s%",	psise->symb);
	sprintf(v_rgst_date,		"%.8s",		psise->date);
	sprintf(v_rgst_time,		"%.9s",		psise->time);
	sprintf(v_bid_last_prc,		"%.*f",		pind, psise->bidprc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, psise->offerprc);
	sprintf(v_mid_last_prc,		"%.*f",		pind, (psise->bidprc + psise->offerprc)/2);
	sprintf(v_fx_ex_quote_id,	"%.*s",		sizeof(psise->quotid), psise->quotid);

	l_rtrim(v_mrkup_dstcd);
	l_rtrim(v_fx_prdct_cd);
	l_rtrim(v_rgst_date);
	l_rtrim(v_rgst_time);
	l_rtrim(v_bid_last_prc);
	l_rtrim(v_offer_last_prc);
	l_rtrim(v_mid_last_prc);
	l_rtrim(v_fx_ex_quote_id);

	//--------------------------------------------------------------------
	// 1. initialization
	//--------------------------------------------------------------------
	
/*
EXEC SQL 
	INSERT INTO INST1.TSKEIMU50 (
			 GROUP_CO_CD
			,MRKUP_DSTCD --
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,MID_LAST_PRC --
			,FX_EX_QUOTE_ID --
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
	) VALUES (
			'KB0'
			,:v_mrkup_dstcd
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_rgst_date
			,:v_rgst_time
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_fx_ex_quote_id
			, SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
			,'KEI000'
	);
*/

{
#line 666 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 666 "mdsrdb.sqc"
  sqlaaloc(2,7,11,0L);
    {
      struct sqla_setdata_list sql_setdlist[7];
#line 666 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 666 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 666 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 666 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 666 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 666 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 666 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 666 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 666 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 666 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 666 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 666 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_mid_last_prc;
#line 666 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 31;
#line 666 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_fx_ex_quote_id;
#line 666 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 666 "mdsrdb.sqc"
      sqlasetdata(2,0,7,sql_setdlist,0L,0L);
    }
#line 666 "mdsrdb.sqc"
  sqlacall((unsigned short)24,7,2,0,0L);
#line 666 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 666 "mdsrdb.sqc"


	// TODO : RSGT_TIME이 msec 이어서 시세가 많은 경우 중복에러(-803) 이 발생함
	if (SQLCODE != 0 && abs(SQLCODE) != 803)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU50 insert error [%.1s:%.6s:%.9s] [%s]  \n", psise->excode, v_fx_prdct_cd, v_rgst_time, SQLERRM);
		return(-1);
	}

	return(0);
} /* End of int InsertTick(MDSSISE *psise) */

int InsertTick_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 682 "mdsrdb.sqc"

	char	v_fx_prdct_cd		[ 6+1];
	char	v_regi_ymd			[ 8+1];
	char	v_rgst_time			[ 9+1];
	char	v_mrkup_dstcd		[ 1+1];
	char	v_fx_prdct_base_cd	[ 6+1];
	char	v_bid_last_prc		[20+1];
	char	v_offer_last_prc	[20+1];
	char	v_usd_mrkup_dstcd	[ 1+1];
	char	v_usd_bid_prc		[20+1];
	char	v_usd_offer_prc		[20+1];
	char	v_fx_ex_quote_id	[30+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 694 "mdsrdb.sqc"

	double	d_bid_prc			= 0.0;
	double	d_offer_prc			= 0.0;

  //--------------------------------------------------------------------
  // 1. initialization
  //--------------------------------------------------------------------
	memset(v_fx_prdct_cd		, 0x00, sizeof(v_fx_prdct_cd));
	memset(v_regi_ymd			, 0x00, sizeof(v_regi_ymd));
	memset(v_rgst_time			, 0x00, sizeof(v_rgst_time));
	memset(v_mrkup_dstcd		, 0x00, sizeof(v_mrkup_dstcd));
	memset(v_fx_prdct_base_cd	, 0x00, sizeof(v_fx_prdct_base_cd));
	memset(v_bid_last_prc		, 0x00, sizeof(v_bid_last_prc));
	memset(v_offer_last_prc		, 0x00, sizeof(v_offer_last_prc));
	memset(v_usd_mrkup_dstcd	, 0x00, sizeof(v_usd_mrkup_dstcd));
	memset(v_usd_bid_prc		, 0x00, sizeof(v_usd_bid_prc));
	memset(v_usd_offer_prc		, 0x00, sizeof(v_usd_offer_prc));
	memset(v_fx_ex_quote_id		, 0x00, sizeof(v_fx_ex_quote_id));

	//	if (pfold->usdpos == 0)			// USD위치 ("0": CADKRW=USDKRW/USDCAD, "1":EURKRW=USDKRW*EURUSD)
	if (memcmp(pfold->symb, "USD", 3) == 0)
		memcpy(v_fx_prdct_cd, &pfold->symb[3], 3);
	else
		memcpy(v_fx_prdct_cd, &pfold->symb, 3);
	memcpy(&v_fx_prdct_cd[3], "KRW", 3);

	sprintf(v_regi_ymd,			"%.8s",		psise->date);
	sprintf(v_rgst_time,		"%.9s",		psise->time);

	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	// USDKRW 수신으로 재정통화계산 시에는 psise값은 미사용, 두 통화의 마스트메모리 값으로 계산함.
	if (memcmp(psise->symb, DF_USDKRW, 6) == 0)
	{
		d_bid_prc	= pfold->bidlast;
		d_offer_prc	= pfold->offerlast;
	}
	else
	{
		d_bid_prc	= psise->bidprc;
		d_offer_prc	= psise->offerprc;
	}

	// 시세정합성 점검
	if (d_bid_prc == 0. || d_offer_prc == 0. || pusdkrw->bidlast == 0. || pusdkrw->offerlast == 0.)
		return (0);

	sprintf(v_fx_prdct_base_cd,	"%.6s%",	pfold->symb);
	sprintf(v_bid_last_prc,		"%.*f",		pind, d_bid_prc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, d_offer_prc);

	// USDKRW
	v_usd_mrkup_dstcd[0] = GetFeeMrkup(pusdkrw->bidex[0]);

	sprintf(v_usd_bid_prc,		"%.*f",		pind, pusdkrw->bidlast);
	sprintf(v_usd_offer_prc,	"%.*f",		pind, pusdkrw->offerlast);
	sprintf(v_fx_ex_quote_id,	"%.*s",		sizeof(psise->quotid), psise->quotid);
	
	l_rtrim(v_fx_prdct_cd);
	l_rtrim(v_regi_ymd);
	l_rtrim(v_rgst_time);
	l_rtrim(v_mrkup_dstcd);
	l_rtrim(v_fx_prdct_base_cd);
	l_rtrim(v_bid_last_prc);
	l_rtrim(v_offer_last_prc);
	l_rtrim(v_usd_mrkup_dstcd);
	l_rtrim(v_usd_bid_prc);
	l_rtrim(v_usd_offer_prc);
	l_rtrim(v_fx_ex_quote_id);

	//--------------------------------------------------------------------
	// 1. initialization
	//--------------------------------------------------------------------
	
/*
EXEC SQL 
	INSERT INTO INST1.TSKEIMU70 (
			 GROUP_CO_CD
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,MRKUP_DSTCD
			,FX_PRDCT_BASE_CD
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,USD_MRKUP_DSTCD
			,USD_BID_PRC
			,USD_OFFER_PRC
			,FX_EX_QUOTE_ID
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
	) VALUES (
			'KB0'
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_mrkup_dstcd
			,:v_fx_prdct_base_cd
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_usd_mrkup_dstcd
			,:v_usd_bid_prc
			,:v_usd_offer_prc
			,:v_fx_ex_quote_id
			, SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
			,'KEI000'
	);
*/

{
#line 798 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 798 "mdsrdb.sqc"
  sqlaaloc(2,10,12,0L);
    {
      struct sqla_setdata_list sql_setdlist[10];
#line 798 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 798 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 798 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 10;
#line 798 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_rgst_time;
#line 798 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 2;
#line 798 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_mrkup_dstcd;
#line 798 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 7;
#line 798 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_fx_prdct_base_cd;
#line 798 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 798 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_bid_last_prc;
#line 798 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 798 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_offer_last_prc;
#line 798 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 2;
#line 798 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_usd_mrkup_dstcd;
#line 798 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 798 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_usd_bid_prc;
#line 798 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 798 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_usd_offer_prc;
#line 798 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 31;
#line 798 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_fx_ex_quote_id;
#line 798 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 798 "mdsrdb.sqc"
      sqlasetdata(2,0,10,sql_setdlist,0L,0L);
    }
#line 798 "mdsrdb.sqc"
  sqlacall((unsigned short)24,8,2,0,0L);
#line 798 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 798 "mdsrdb.sqc"


	// TODO : RSGT_TIME이 msec 이어서 시세가 많은 경우 중복에러(-803) 이 발생함
	if (SQLCODE != 0 && abs(SQLCODE) != 803)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU70 insert error [%.1s:%.6s:%.9s] [%s]  \n", psise->excode, v_fx_prdct_cd, v_rgst_time, SQLERRM);
		return(-1);
	}

	return(0);
} /* End of int InsertTick_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw) */

int UpsertMin(MDSSISE *psise, MDFOLD *pfold)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 814 "mdsrdb.sqc"

	char	v_mrkup_dstcd       [ 1+1];
	char	v_fx_prdct_cd       [ 6+1];
	char	v_regi_ymd          [ 8+1];
	char	v_rgst_time         [ 9+1];
	char	v_bid_last_prc      [20+1];
	char	v_offer_last_prc    [20+1];
	char	v_mid_last_prc      [20+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 822 "mdsrdb.sqc"


	memset(v_mrkup_dstcd   , 0x00, sizeof(v_mrkup_dstcd   ));
	memset(v_fx_prdct_cd   , 0x00, sizeof(v_fx_prdct_cd   ));
	memset(v_regi_ymd      , 0x00, sizeof(v_regi_ymd      ));
	memset(v_rgst_time     , 0x00, sizeof(v_rgst_time     ));
	memset(v_bid_last_prc  , 0x00, sizeof(v_bid_last_prc  ));
	memset(v_offer_last_prc, 0x00, sizeof(v_offer_last_prc));
	memset(v_mid_last_prc  , 0x00, sizeof(v_mid_last_prc  ));

	sprintf(v_fx_prdct_cd,	"%.6s%",		psise->symb);
	sprintf(v_regi_ymd,		"%.8s",			psise->date);
	sprintf(v_rgst_time,	"%.4s00000",	psise->time);

	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	sprintf(v_bid_last_prc,		"%.*f",		pind, psise->bidprc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, psise->offerprc);
	sprintf(v_mid_last_prc,		"%.*f",		pind, (psise->bidprc+psise->offerprc)/2);

	l_rtrim(v_mrkup_dstcd   );
	l_rtrim(v_fx_prdct_cd   );
	l_rtrim(v_regi_ymd      );
	l_rtrim(v_rgst_time     );
	l_rtrim(v_bid_last_prc  );
	l_rtrim(v_offer_last_prc);
	l_rtrim(v_mid_last_prc  );

	
/*
EXEC SQL 
	UPDATE INST1.TSKEIMU51
	SET	 GROUP_CO_CD        = 'KB0'
		,MRKUP_DSTCD        = :v_mrkup_dstcd
		,FX_PRDCT_CD        = :v_fx_prdct_cd
--		,REGI_YMD           = :v_regi_ymd
--		,RGST_TIME          = :v_rgst_time
		,BID_LAST_PRC       = :v_bid_last_prc
		,OFFER_LAST_PRC     = :v_offer_last_prc
		,BID_HIGH_PRC       = CASE WHEN BID_HIGH_PRC < :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_HIGH_PRC END
		,BID_LOW_PRC        = CASE WHEN BID_LOW_PRC  > :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_LOW_PRC  END
		,OFFER_HIGH_PRC     = CASE WHEN OFFER_HIGH_PRC < :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_HIGH_PRC END
		,OFFER_LOW_PRC      = CASE WHEN OFFER_LOW_PRC  > :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_LOW_PRC  END
		,MID_LAST_PRC       = :v_mid_last_prc
-- (2024.05.27) add column
		,MID_HIGH_PRC       = CASE WHEN MID_HIGH_PRC < :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_HIGH_PRC END
		,MID_LOW_PRC        = CASE WHEN MID_LOW_PRC  > :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_LOW_PRC  END
		,SYS_LAST_PRCSS_YMS = SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
		,SYS_LAST_UNO       = 'KEI000'
	WHERE	GROUP_CO_CD	='KB0'
	  AND	FX_PRDCT_CD	= :v_fx_prdct_cd
	  AND	REGI_YMD	= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
	  AND	RGST_TIME	=  :v_rgst_time
	;
*/

{
#line 873 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 873 "mdsrdb.sqc"
  sqlaaloc(2,19,13,0L);
    {
      struct sqla_setdata_list sql_setdlist[19];
#line 873 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 873 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 873 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 873 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 873 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_bid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_offer_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_bid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_offer_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_mid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_mid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_mid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_mid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 21;
#line 873 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_mid_last_prc;
#line 873 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 460; sql_setdlist[17].sqllen = 7;
#line 873 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)v_fx_prdct_cd;
#line 873 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 460; sql_setdlist[18].sqllen = 10;
#line 873 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)v_rgst_time;
#line 873 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 873 "mdsrdb.sqc"
      sqlasetdata(2,0,19,sql_setdlist,0L,0L);
    }
#line 873 "mdsrdb.sqc"
  sqlacall((unsigned short)24,9,2,0,0L);
#line 873 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 873 "mdsrdb.sqc"

	if (SQLCODE == 100) 
	{
	
/*
EXEC SQL 
		INSERT INTO INST1.TSKEIMU51 (
			 GROUP_CO_CD
			,MRKUP_DSTCD
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,BID_OPEN_PRC
			,BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_OPEN_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,MID_LAST_PRC
-- (2024.05.27) add column
			,MID_OPEN_PRC
			,MID_HIGH_PRC
			,MID_LOW_PRC
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
		) VALUES (
			 'KB0'
			,:v_mrkup_dstcd
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
			,'KEI000'
		);
*/

{
#line 918 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 918 "mdsrdb.sqc"
  sqlaaloc(2,15,14,0L);
    {
      struct sqla_setdata_list sql_setdlist[15];
#line 918 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 918 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 918 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 918 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 918 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 918 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 918 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_mid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_mid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_mid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 918 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_mid_last_prc;
#line 918 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 918 "mdsrdb.sqc"
      sqlasetdata(2,0,15,sql_setdlist,0L,0L);
    }
#line 918 "mdsrdb.sqc"
  sqlacall((unsigned short)24,10,2,0,0L);
#line 918 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 918 "mdsrdb.sqc"

		if (SQLCODE != 0)
		{
			mds_log(market, MLOG_ERROR, "TSKEIMU51 insert error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
			return(-1);
		}
	}
	else if (SQLCODE != 0)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU51 update error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}

	return (0);

} /* End of int UpsertMin(MDSSISE *psise) */


int UpsertMin_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw)
{
	int		pind = 8;


/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 940 "mdsrdb.sqc"

	char	v_mrkup_dstcd       [ 1+1];
	char	v_fx_prdct_cd       [ 6+1];
	char	v_regi_ymd          [ 8+1];
	char	v_rgst_time         [ 9+1];
	char	v_bid_last_prc      [20+1];
	char	v_offer_last_prc    [20+1];
	char	v_mid_last_prc      [20+1];
	char	v_usd_mrkup_dstcd	[ 1+1];
	char	v_usd_bid_last_prc	[20+1];
	char	v_usd_offer_last_prc[20+1];
	char	v_usd_mid_last_prc	[20+1];

	double	d_bid_high_prc		;
	double	d_bid_low_prc		;
	double	d_offer_high_prc	;
	double	d_offer_low_prc		;
	double	d_usd_bid_high_prc	;
	double	d_usd_bid_low_prc	;
	double	d_usd_offer_high_prc;
	double	d_usd_offer_low_prc	;

	sqlint32     i_bh                ;
	sqlint32     i_bl                ;
	sqlint32     i_oh                ;
	sqlint32     i_ol                ;
	sqlint32		i_usdpos			;

/*
EXEC SQL END DECLARE SECTION;
*/

#line 967 "mdsrdb.sqc"


	double  fn_bid              = 0.0;
	double  fn_offer            = 0.0;
	double	fn_cur_bid_high		= 0.0;
	double	fn_cur_offer_high	= 0.0;
	double	fn_cur_bid_low		= 0.0;
	double	fn_cur_offer_low	= 0.0;
	
	double  fn_baseprc          = 0.0;
	double  d_updwn_rato        = 0.0;
	double  d_mid_last_prc      = 0.0;
	double	d_bid_prc			= 0.0;
	double	d_offer_prc			= 0.0;
	double	d_usd_bid_prc		= 0.0;
	double	d_usd_offer_prc		= 0.0;

	memset(v_mrkup_dstcd   		, 0x00, sizeof(v_mrkup_dstcd   ));
	memset(v_fx_prdct_cd   		, 0x00, sizeof(v_fx_prdct_cd   ));
	memset(v_regi_ymd      		, 0x00, sizeof(v_regi_ymd      ));
	memset(v_rgst_time     		, 0x00, sizeof(v_rgst_time     ));
	memset(v_bid_last_prc  		, 0x00, sizeof(v_bid_last_prc  ));
	memset(v_offer_last_prc		, 0x00, sizeof(v_offer_last_prc));
	memset(v_mid_last_prc  		, 0x00, sizeof(v_mid_last_prc  ));
	memset(v_usd_mrkup_dstcd	, 0x00, sizeof(v_usd_mrkup_dstcd));
	memset(v_usd_bid_last_prc	, 0x00, sizeof(v_usd_bid_last_prc));
	memset(v_usd_offer_last_prc	, 0x00, sizeof(v_usd_offer_last_prc));
	memset(v_usd_mid_last_prc	, 0x00, sizeof(v_usd_mid_last_prc));

	d_bid_high_prc      	= 0.0;
	d_bid_low_prc       	= 0.0;
	d_offer_high_prc    	= 0.0;
	d_offer_low_prc     	= 0.0;
	d_usd_bid_high_prc		= 0.0;
	d_usd_bid_low_prc		= 0.0;
	d_usd_offer_high_prc	= 0.0;
	d_usd_offer_low_prc		= 0.0;

	i_bh = i_bl = i_oh = i_ol = i_usdpos = 0;

	i_usdpos = pfold->usdpos;			// USD위치 ("0": CADKRW=USDKRW/USDCAD, "1":EURKRW=USDKRW*EURUSD)

	if (i_usdpos == 0)		memcpy(v_fx_prdct_cd, &pfold->symb[3], 3);
	else 					memcpy(v_fx_prdct_cd, pfold->symb, 3);
	memcpy(&v_fx_prdct_cd[3], "KRW", 3);

	sprintf(v_regi_ymd,		"%.8s",			psise->date);
	sprintf(v_rgst_time,	"%.4s00000",	psise->time);

	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	// USDKRW 수신으로 재정통화계산 시에는 psise값은 미사용, 두 통화의 마스트메모리 값으로 계산함.
	if (memcmp(psise->symb, DF_USDKRW, 6) == 0)
	{
		d_bid_prc		= pfold->bidlast;
		d_offer_prc		= pfold->offerlast;
		d_usd_bid_prc	= psise->bidprc;	//pusdkrw->bidlast;
		d_usd_offer_prc	= psise->offerprc;	//pusdkrw->offerlast;
	}
	else
	{
		d_bid_prc		= psise->bidprc;
		d_offer_prc		= psise->offerprc;
		d_usd_bid_prc	= pusdkrw->bidlast;
		d_usd_offer_prc	= pusdkrw->offerlast;
	}

	// 시세정합성 점검
	if (d_bid_prc == 0. || d_offer_prc == 0. || pusdkrw->bidlast == 0. || pusdkrw->offerlast == 0.)
		return (0);

 	// 수신한 환율 기준으로 재정통화 환율계산
	fn_bid		= (i_usdpos == 0 ? d_usd_bid_prc/d_offer_prc : d_usd_bid_prc*d_bid_prc);
	fn_offer	= (i_usdpos == 0 ? d_usd_offer_prc/d_bid_prc : d_usd_offer_prc*d_offer_prc);
	fn_baseprc	= (i_usdpos == 0 ? pusdkrw->baseprc/pfold->baseprc : pusdkrw->baseprc*pfold->baseprc);

	// 중간값은 이종통화의 중간값을 저장
	d_mid_last_prc = (fn_bid + fn_offer)/2;
//	sprintf(v_mid_last_prc,		"%.*f",		pind, d_mid_last_prc);
	sprintf(v_mid_last_prc,		"%.*f",		pind, (d_bid_prc+d_offer_prc)/2);

	// USDKRW
	v_usd_mrkup_dstcd[0] = GetFeeMrkup(pusdkrw->bidex[0]);

	sprintf(v_bid_last_prc,		"%.*f",		pind, d_bid_prc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, d_offer_prc);

	sprintf(v_usd_bid_last_prc,		"%.*f",		pind, d_usd_bid_prc);
	sprintf(v_usd_offer_last_prc,	"%.*f",		pind, d_usd_offer_prc);
	sprintf(v_usd_mid_last_prc,		"%.*f",		pind, (d_usd_bid_prc+d_usd_offer_prc)/2);

	l_rtrim(v_mrkup_dstcd   );
	l_rtrim(v_fx_prdct_cd   );
	l_rtrim(v_regi_ymd      );
	l_rtrim(v_rgst_time     );
	l_rtrim(v_bid_last_prc  );
	l_rtrim(v_offer_last_prc);
	l_rtrim(v_mid_last_prc  );
	l_rtrim(v_usd_mrkup_dstcd);
	l_rtrim(v_usd_bid_last_prc);
	l_rtrim(v_usd_offer_last_prc);
	l_rtrim(v_usd_mid_last_prc);

	
/*
EXEC SQL
	SELECT  --CASE WHEN :i_usdpos = 0 THEN USD_BID_HIGH_PRC/BID_HIGH_PRC     ELSE USD_BID_HIGH_PRC*BID_HIGH_PRC     END
			-- 더블형의 DB계산값과 서버계산값이 달라서 쿼리로 계산하지 않고 서버에서 계산 후 비교
			 BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,USD_BID_HIGH_PRC
			,USD_BID_LOW_PRC
			,USD_OFFER_HIGH_PRC
			,USD_OFFER_LOW_PRC
	  INTO	 :d_bid_high_prc
			,:d_bid_low_prc
			,:d_offer_high_prc
			,:d_offer_low_prc
			,:d_usd_bid_high_prc
			,:d_usd_bid_low_prc
			,:d_usd_offer_high_prc
			,:d_usd_offer_low_prc
	 FROM	INST1.TSKEIMU71
	WHERE	GROUP_CO_CD ='KB0'
	  AND	FX_PRDCT_CD = :v_fx_prdct_cd
	  AND	REGI_YMD    = (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
	  AND	RGST_TIME   = :v_rgst_time
	;
*/

{
#line 1094 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1094 "mdsrdb.sqc"
  sqlaaloc(2,2,15,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 10;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_rgst_time;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 1094 "mdsrdb.sqc"
  sqlaaloc(3,8,16,0L);
    {
      struct sqla_setdata_list sql_setdlist[8];
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 480; sql_setdlist[0].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)&d_bid_high_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 480; sql_setdlist[1].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)&d_bid_low_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 480; sql_setdlist[2].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)&d_offer_high_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 480; sql_setdlist[3].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)&d_offer_low_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 480; sql_setdlist[4].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)&d_usd_bid_high_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 480; sql_setdlist[5].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)&d_usd_bid_low_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 480; sql_setdlist[6].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)&d_usd_offer_high_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 480; sql_setdlist[7].sqllen = 8;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)&d_usd_offer_low_prc;
#line 1094 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1094 "mdsrdb.sqc"
      sqlasetdata(3,0,8,sql_setdlist,0L,0L);
    }
#line 1094 "mdsrdb.sqc"
  sqlacall((unsigned short)24,11,2,3,0L);
#line 1094 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1094 "mdsrdb.sqc"

	if (SQLCODE == 100)
	{
		
/*
EXEC SQL 
		INSERT INTO INST1.TSKEIMU71 (
			 GROUP_CO_CD
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,MRKUP_DSTCD
			,USD_MRKUP_DSTCD
			,BID_OPEN_PRC
			,BID_HIGH_PRC
			,BID_LOW_PRC
			,BID_LAST_PRC
			,OFFER_OPEN_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,OFFER_LAST_PRC
			,MID_LAST_PRC
			,USD_BID_OPEN_PRC
			,USD_BID_HIGH_PRC
			,USD_BID_LOW_PRC
			,USD_BID_LAST_PRC
			,USD_OFFER_OPEN_PRC
			,USD_OFFER_HIGH_PRC
			,USD_OFFER_LOW_PRC
			,USD_OFFER_LAST_PRC
			,USD_MID_LAST_PRC
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
		) VALUES (
			 'KB0'
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_mrkup_dstcd
			,:v_usd_mrkup_dstcd
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_mid_last_prc
			, SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
			,'KEI000'
		);
*/

{
#line 1152 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1152 "mdsrdb.sqc"
  sqlaaloc(2,22,17,0L);
    {
      struct sqla_setdata_list sql_setdlist[22];
#line 1152 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 10;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_rgst_time;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 2;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_mrkup_dstcd;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 2;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_usd_mrkup_dstcd;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_mid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_usd_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_usd_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_usd_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_usd_bid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 460; sql_setdlist[17].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)v_usd_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 460; sql_setdlist[18].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)v_usd_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[19].sqltype = 460; sql_setdlist[19].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[19].sqldata = (void*)v_usd_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[19].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[20].sqltype = 460; sql_setdlist[20].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[20].sqldata = (void*)v_usd_offer_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[20].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[21].sqltype = 460; sql_setdlist[21].sqllen = 21;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[21].sqldata = (void*)v_usd_mid_last_prc;
#line 1152 "mdsrdb.sqc"
      sql_setdlist[21].sqlind = 0L;
#line 1152 "mdsrdb.sqc"
      sqlasetdata(2,0,22,sql_setdlist,0L,0L);
    }
#line 1152 "mdsrdb.sqc"
  sqlacall((unsigned short)24,12,2,0,0L);
#line 1152 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1152 "mdsrdb.sqc"

		if (SQLCODE != 0)
		{
			mds_log(market, MLOG_ERROR, "TSKEIMU71 insert error [%.1s:%.6s:%.6s] [%s] \n", psise->excode, psise->symb, v_fx_prdct_cd, SQLERRM);
			return(-1);
		}

	}
	if (SQLCODE != 0)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU71 select error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}

	/*===============================================================
	* 고가와 저가는 계산된 재정환율을 기준으로 비교 함
	* 계산된 재정환율이 고가를 갱신한 경우에 각 HIGH_PRC, USD_HIGH_PRC, HIGH_PRC_TIME 업데이트
	* 계산된 재정환율이 저가를 갱신한 경우에 각 LOW_PRC, USD_LOW_PRC, LOW_PRC_TIME 업데이트
	===============================================================*/
	fn_cur_bid_high		= (i_usdpos == 0 ? d_usd_bid_high_prc/d_offer_high_prc : d_usd_bid_high_prc*d_bid_high_prc);
	fn_cur_bid_low		= (i_usdpos == 0 ? d_usd_bid_low_prc/d_offer_low_prc   : d_usd_bid_low_prc*d_bid_low_prc);
	fn_cur_offer_high	= (i_usdpos == 0 ? d_usd_offer_high_prc/d_bid_high_prc : d_usd_offer_high_prc*d_offer_high_prc);
	fn_cur_offer_low	= (i_usdpos == 0 ? d_usd_offer_low_prc/d_bid_low_prc   : d_usd_offer_low_prc*d_offer_low_prc);

	// 소수점 이슈로 동일한 값을 계속 업데이트 하는 현상 제거를 위해 ULONG 으로 타입 변환
	ulong	ubid, ubid_cur_h, ubid_cur_l;
	ulong	uoffer, uoffer_cur_h, uoffer_cur_l;
	int		x = 6;	//pusdkrw->zCustdiv;		// 표시기준인 소수점 2자리만 비교하여 계산하면 오류성으로 보이는 데이터 많이 보임

	ubid         = (ulong)(pow(10, x)*fn_bid + 0.5);
	ubid_cur_h   = (ulong)(pow(10, x)*fn_cur_bid_high + 0.5);
	ubid_cur_l   = (ulong)(pow(10, x)*fn_cur_bid_low  + 0.5);

	uoffer       = (ulong)(pow(10, x)*fn_offer + 0.5);
	uoffer_cur_h = (ulong)(pow(10, x)*fn_cur_offer_high + 0.5);
	uoffer_cur_l = (ulong)(pow(10, x)*fn_cur_offer_low  + 0.5);

	if (ubid > ubid_cur_h)			i_bh = 1;
	else if (ubid < ubid_cur_l)		i_bl = 1;

	if (uoffer > uoffer_cur_h)		i_oh = 1;
	else if (uoffer < uoffer_cur_l)	i_ol = 1;

/*
	if (i_bh + i_bl + i_oh + i_ol)
	{
//		mds_log(market, MLOG_DEBUG, "Changed! [%s] [%.1s:%.6s] BID[%d:%d:%f] hl[%f:%f] ASK[%d:%d:%f] hl[%f:%f] NEW ! ", __func__, psise->excode, v_fx_prdct_cd, 
		mds_log(market, MLOG_DEBUG, "Changed! [%s] [%.1s:%.6s] BID[%d:%d:%u] hl[%u:%u] ASK[%d:%d:%u] hl[%u:%u] NEW ! ", __func__, psise->excode, v_fx_prdct_cd, 
			i_bh, i_bl,
//			fn_bid, fn_cur_bid_high, fn_cur_bid_low,  // bid DB 값
ubid, ubid_cur_h, ubid_cur_l,
			i_oh, i_ol,
//			fn_offer, fn_cur_offer_high, fn_cur_offer_low); // offer DB값
uoffer, uoffer_cur_h, uoffer_cur_l);
		mds_log(market, MLOG_DEBUG, "    [%s] [rcv %.6s:%s] [%.6s:%s:%s] [%.6s:%s:%s] Update ! ", __func__, psise->symb, v_rgst_time, 
			pfold->symb, v_bid_last_prc, v_offer_last_prc, pusdkrw->symb, v_usd_bid_last_prc, v_usd_offer_last_prc);
	}
*/


/*
EXEC SQL 
	UPDATE INST1.TSKEIMU71
	SET	 
--	   	 GROUP_CO_CD        = 'KB0'
		 MRKUP_DSTCD        = :v_mrkup_dstcd
		,USD_MRKUP_DSTCD    = :v_usd_mrkup_dstcd
		,FX_PRDCT_CD        = :v_fx_prdct_cd
--		,REGI_YMD           = :v_regi_ymd
--		,RGST_TIME          = :v_rgst_time
		,BID_LAST_PRC       = :v_bid_last_prc
		,OFFER_LAST_PRC     = :v_offer_last_prc
--		,BID_OPEN_PRC       = :v_bid_last_prc
--		,BID_HIGH_PRC       = DECODE(:i_bh, 1, :v_bid_last_prc, BID_HIGH_PRC)
--		,BID_LOW_PRC        = DECODE(:i_bl, 1, :v_bid_last_prc, BID_LOW_PRC )
		,BID_HIGH_PRC       = CASE WHEN ((:i_bh = 1 AND :i_usdpos = 1) OR (:i_oh = 1 AND :i_usdpos = 0)) THEN :v_bid_last_prc	ELSE BID_HIGH_PRC END
		,BID_LOW_PRC        = CASE WHEN ((:i_bl = 1 AND :i_usdpos = 1) OR (:i_ol = 1 AND :i_usdpos = 0)) THEN :v_bid_last_prc	ELSE BID_LOW_PRC  END
--		,OFFER_OPEN_PRC     = :v_offer_last_prc
--		,OFFER_HIGH_PRC     = DECODE(:i_oh, 1, :v_offer_last_prc, OFFER_HIGH_PRC)
--		,OFFER_LOW_PRC      = DECODE(:i_ol, 1, :v_offer_last_prc, OFFER_LOW_PRC )
		,OFFER_HIGH_PRC     = CASE WHEN ((:i_oh = 1 AND :i_usdpos = 1) OR (:i_bh = 1 AND :i_usdpos = 0)) THEN :v_offer_last_prc	ELSE OFFER_HIGH_PRC END
		,OFFER_LOW_PRC      = CASE WHEN ((:i_ol = 1 AND :i_usdpos = 1) OR (:i_bl = 1 AND :i_usdpos = 0)) THEN :v_offer_last_prc	ELSE OFFER_LOW_PRC  END
		,USD_BID_LAST_PRC   = :v_usd_bid_last_prc
		,MID_LAST_PRC       = :v_mid_last_prc
--		,USD_BID_OPEN_PRC   = :v_usd_bid_last_prc
		,USD_BID_HIGH_PRC   = DECODE(:i_bh, 1, :v_usd_bid_last_prc, USD_BID_HIGH_PRC)
		,USD_BID_LOW_PRC    = DECODE(:i_bl, 1, :v_usd_bid_last_prc, USD_BID_LOW_PRC )
--		,USD_OFFER_OPEN_PRC = :v_usd_offer_last_prc
		,USD_OFFER_HIGH_PRC = DECODE(:i_oh, 1, :v_usd_offer_last_prc, USD_OFFER_HIGH_PRC)
		,USD_OFFER_LOW_PRC  = DECODE(:i_ol, 1, :v_usd_offer_last_prc, USD_OFFER_LOW_PRC )
		,USD_OFFER_LAST_PRC = :v_usd_offer_last_prc
		,USD_MID_LAST_PRC   = :v_usd_mid_last_prc
		,SYS_LAST_PRCSS_YMS = SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
		,SYS_LAST_UNO       = 'KEI000'
	WHERE	GROUP_CO_CD	='KB0'
	  AND	FX_PRDCT_CD	= :v_fx_prdct_cd
	  AND	REGI_YMD	= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
	  AND	RGST_TIME	= :v_rgst_time
	;
*/

{
#line 1248 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1248 "mdsrdb.sqc"
  sqlaaloc(2,39,18,0L);
    {
      struct sqla_setdata_list sql_setdlist[39];
#line 1248 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 2;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_usd_mrkup_dstcd;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 7;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_fx_prdct_cd;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 496; sql_setdlist[5].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)&i_bh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 496; sql_setdlist[6].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 496; sql_setdlist[7].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)&i_oh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 496; sql_setdlist[8].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 496; sql_setdlist[10].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)&i_bl;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 496; sql_setdlist[11].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 496; sql_setdlist[12].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)&i_ol;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 496; sql_setdlist[13].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 496; sql_setdlist[15].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)&i_oh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 496; sql_setdlist[16].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 496; sql_setdlist[17].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)&i_bh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 496; sql_setdlist[18].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[19].sqltype = 460; sql_setdlist[19].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[19].sqldata = (void*)v_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[19].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[20].sqltype = 496; sql_setdlist[20].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[20].sqldata = (void*)&i_ol;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[20].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[21].sqltype = 496; sql_setdlist[21].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[21].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[21].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[22].sqltype = 496; sql_setdlist[22].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[22].sqldata = (void*)&i_bl;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[22].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[23].sqltype = 496; sql_setdlist[23].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[23].sqldata = (void*)&i_usdpos;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[23].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[24].sqltype = 460; sql_setdlist[24].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[24].sqldata = (void*)v_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[24].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[25].sqltype = 460; sql_setdlist[25].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[25].sqldata = (void*)v_usd_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[25].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[26].sqltype = 460; sql_setdlist[26].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[26].sqldata = (void*)v_mid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[26].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[27].sqltype = 496; sql_setdlist[27].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[27].sqldata = (void*)&i_bh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[27].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[28].sqltype = 460; sql_setdlist[28].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[28].sqldata = (void*)v_usd_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[28].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[29].sqltype = 496; sql_setdlist[29].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[29].sqldata = (void*)&i_bl;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[29].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[30].sqltype = 460; sql_setdlist[30].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[30].sqldata = (void*)v_usd_bid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[30].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[31].sqltype = 496; sql_setdlist[31].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[31].sqldata = (void*)&i_oh;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[31].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[32].sqltype = 460; sql_setdlist[32].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[32].sqldata = (void*)v_usd_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[32].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[33].sqltype = 496; sql_setdlist[33].sqllen = 4;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[33].sqldata = (void*)&i_ol;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[33].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[34].sqltype = 460; sql_setdlist[34].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[34].sqldata = (void*)v_usd_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[34].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[35].sqltype = 460; sql_setdlist[35].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[35].sqldata = (void*)v_usd_offer_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[35].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[36].sqltype = 460; sql_setdlist[36].sqllen = 21;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[36].sqldata = (void*)v_usd_mid_last_prc;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[36].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[37].sqltype = 460; sql_setdlist[37].sqllen = 7;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[37].sqldata = (void*)v_fx_prdct_cd;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[37].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[38].sqltype = 460; sql_setdlist[38].sqllen = 10;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[38].sqldata = (void*)v_rgst_time;
#line 1248 "mdsrdb.sqc"
      sql_setdlist[38].sqlind = 0L;
#line 1248 "mdsrdb.sqc"
      sqlasetdata(2,0,39,sql_setdlist,0L,0L);
    }
#line 1248 "mdsrdb.sqc"
  sqlacall((unsigned short)24,13,2,0,0L);
#line 1248 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1248 "mdsrdb.sqc"

	if (SQLCODE != 0)
	{
		if (abs(SQLCODE) == 420)
			mds_log(market, MLOG_WARNING, "%s,%s,%s,%s,%s,%s,%s"
			,v_fx_prdct_cd
			,v_bid_last_prc
			,v_offer_last_prc
			,v_mid_last_prc
			,v_usd_bid_last_prc
			,v_usd_offer_last_prc
			,v_usd_mid_last_prc );

		mds_log(market, MLOG_ERROR, "TSKEIMU71 update error [%.1s:%.6s:%.6s] [%s]\n", psise->excode, psise->symb, pfold->symb, SQLERRM);
		return(-1);
	}

	return (0);

} /* End of int UpsertMin_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw) */


int UpsertDay(MDSSISE *psise, MDFOLD *pfold)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 1274 "mdsrdb.sqc"

	char	v_mrkup_dstcd       [ 1+1];
	char	v_fx_prdct_cd       [ 6+1];
	char	v_regi_ymd          [ 8+1];
	char	v_rgst_time         [ 9+1];
	char	v_bid_last_prc      [20+1];
	char	v_offer_last_prc    [20+1];
	char	v_mid_last_prc      [20+1];
	char	v_prdat_cntst_prc   [20+1];
	char	v_updwn_rato        [ 8+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 1284 "mdsrdb.sqc"


	double	d_prdat_cntst_prc	= 0.;
	double	d_updwn_rato		= 0.;
	double	d_mid_last_prc		= 0.;

	memset(v_mrkup_dstcd    , 0x00,		sizeof(v_mrkup_dstcd    ));
	memset(v_fx_prdct_cd    , 0x00,		sizeof(v_fx_prdct_cd    ));
	memset(v_regi_ymd       , 0x00,		sizeof(v_regi_ymd       ));
	memset(v_rgst_time      , 0x00,		sizeof(v_rgst_time      ));
	memset(v_bid_last_prc   , 0x00,		sizeof(v_bid_last_prc   ));
	memset(v_offer_last_prc , 0x00,		sizeof(v_offer_last_prc ));
	memset(v_mid_last_prc   , 0x00,		sizeof(v_mid_last_prc   ));
	memset(v_prdat_cntst_prc, 0x00,		sizeof(v_prdat_cntst_prc));
	memset(v_updwn_rato     , 0x00,		sizeof(v_updwn_rato     ));
	
	sprintf(v_fx_prdct_cd,	"%.6s%",		psise->symb);
	sprintf(v_regi_ymd,		"%.8s",			psise->date);
	sprintf(v_rgst_time,	"%.4s00000",	psise->time);
	
	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	sprintf(v_bid_last_prc,		"%.*f",		pind, psise->bidprc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, psise->offerprc);

	d_mid_last_prc = (psise->bidprc + psise->offerprc)/2;
	sprintf(v_mid_last_prc,		"%.*f",		pind, d_mid_last_prc);
	if (d_mid_last_prc > 0.0)
	{
		d_prdat_cntst_prc	= d_mid_last_prc - pfold->baseprc;
		d_updwn_rato		= (d_prdat_cntst_prc / d_mid_last_prc) * 100.;
	}
	sprintf(v_prdat_cntst_prc,	"%.*f",		pind, d_prdat_cntst_prc);
	sprintf(v_updwn_rato,		"%.*f",		pind, d_updwn_rato);

	l_rtrim(v_mrkup_dstcd    );
	l_rtrim(v_fx_prdct_cd    );
	l_rtrim(v_regi_ymd       );
	l_rtrim(v_rgst_time      );
	l_rtrim(v_bid_last_prc   );
	l_rtrim(v_offer_last_prc );
	l_rtrim(v_mid_last_prc   );
	l_rtrim(v_prdat_cntst_prc);
	l_rtrim(v_updwn_rato     );


/*
EXEC SQL 
	UPDATE INST1.TSKEIMU52
	SET	 GROUP_CO_CD        = 'KB0'
		,MRKUP_DSTCD        = :v_mrkup_dstcd
		,FX_PRDCT_CD        = :v_fx_prdct_cd
--		,REGI_YMD           = :v_regi_ymd
		,RGST_TIME          = :v_rgst_time
		,BID_LAST_PRC       = :v_bid_last_prc
		,OFFER_LAST_PRC     = :v_offer_last_prc
--		,BID_OPEN_PRC       = :v_bid_last_prc
		,BID_HIGH_PRC       = CASE WHEN BID_HIGH_PRC < :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_HIGH_PRC END
		,BID_LOW_PRC        = CASE WHEN BID_LOW_PRC  > :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_LOW_PRC  END
--		,OFFER_OPEN_PRC     = :v_offer_last_prc
		,OFFER_HIGH_PRC     = CASE WHEN OFFER_HIGH_PRC < :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_HIGH_PRC END
		,OFFER_LOW_PRC      = CASE WHEN OFFER_LOW_PRC  > :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_LOW_PRC  END
		,MID_LAST_PRC       = :v_mid_last_prc
-- (2024.05.27) add column
		,MID_HIGH_PRC       = CASE WHEN MID_HIGH_PRC < :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_HIGH_PRC END
		,MID_LOW_PRC        = CASE WHEN MID_LOW_PRC  > :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_LOW_PRC  END
--		,OPEN_PRC_TIME      = SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
		,HIGH_PRC_TIME      = CASE WHEN BID_HIGH_PRC < :v_bid_last_prc OR OFFER_HIGH_PRC < :v_offer_last_prc THEN 
									SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9) ELSE HIGH_PRC_TIME END
		,LOW_PRC_TIME       = CASE WHEN BID_LOW_PRC  > :v_bid_last_prc OR OFFER_LOW_PRC  > :v_offer_last_prc THEN
									SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9) ELSE LOW_PRC_TIME  END
		,PRDAT_CNTST_PRC    = :v_prdat_cntst_prc
		,UPDWN_RATO         = :v_updwn_rato
		,SYS_LAST_PRCSS_YMS = SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
		,SYS_LAST_UNO       = 'KEI000'
	WHERE	GROUP_CO_CD	='KB0'
	  AND	FX_PRDCT_CD	= :v_fx_prdct_cd
	  AND	REGI_YMD	= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
	;
*/

{
#line 1360 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1360 "mdsrdb.sqc"
  sqlaaloc(2,25,19,0L);
    {
      struct sqla_setdata_list sql_setdlist[25];
#line 1360 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_mid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_mid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_mid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_mid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 460; sql_setdlist[17].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)v_mid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 460; sql_setdlist[18].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[19].sqltype = 460; sql_setdlist[19].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[19].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[19].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[20].sqltype = 460; sql_setdlist[20].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[20].sqldata = (void*)v_bid_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[20].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[21].sqltype = 460; sql_setdlist[21].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[21].sqldata = (void*)v_offer_last_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[21].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[22].sqltype = 460; sql_setdlist[22].sqllen = 21;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[22].sqldata = (void*)v_prdat_cntst_prc;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[22].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[23].sqltype = 460; sql_setdlist[23].sqllen = 9;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[23].sqldata = (void*)v_updwn_rato;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[23].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[24].sqltype = 460; sql_setdlist[24].sqllen = 7;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[24].sqldata = (void*)v_fx_prdct_cd;
#line 1360 "mdsrdb.sqc"
      sql_setdlist[24].sqlind = 0L;
#line 1360 "mdsrdb.sqc"
      sqlasetdata(2,0,25,sql_setdlist,0L,0L);
    }
#line 1360 "mdsrdb.sqc"
  sqlacall((unsigned short)24,14,2,0,0L);
#line 1360 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1360 "mdsrdb.sqc"

	if (SQLCODE == 100) 
	{
	
/*
EXEC SQL 
		INSERT INTO INST1.TSKEIMU52 (
			 GROUP_CO_CD
			,MRKUP_DSTCD
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,BID_OPEN_PRC
			,BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_OPEN_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,MID_LAST_PRC
-- (2024.05.27) add column
			,MID_OPEN_PRC
			,MID_HIGH_PRC
			,MID_LOW_PRC
			,OPEN_PRC_TIME
			,HIGH_PRC_TIME
			,LOW_PRC_TIME
			,PRDAT_CNTST_PRC
			,UPDWN_RATO
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
		) VALUES (
			 'KB0'
			,:v_mrkup_dstcd
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			,:v_prdat_cntst_prc
			,:v_updwn_rato
			, SUBSTR(HEX(CURRENT TIMESTAMP), 1, 20)
			,'KEI000'
		);
*/

{
#line 1415 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1415 "mdsrdb.sqc"
  sqlaaloc(2,17,20,0L);
    {
      struct sqla_setdata_list sql_setdlist[17];
#line 1415 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_mid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_mid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_mid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_mid_last_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_prdat_cntst_prc;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 9;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_updwn_rato;
#line 1415 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1415 "mdsrdb.sqc"
      sqlasetdata(2,0,17,sql_setdlist,0L,0L);
    }
#line 1415 "mdsrdb.sqc"
  sqlacall((unsigned short)24,15,2,0,0L);
#line 1415 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1415 "mdsrdb.sqc"

		if (SQLCODE != 0)
		{
			mds_log(market, MLOG_ERROR, "TSKEIMU52 insert error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
			return(-1);
		}
	}
	else if (SQLCODE != 0)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU52 update error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}

	return (0);

} /* End of int UpsertDay(MDFOLD *pfold, MDSSISE *psise) */


int UpsertDay_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw)
{
	int		pind = 8;
	

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 1437 "mdsrdb.sqc"

	char	v_mrkup_dstcd       [ 1+1];
	char	v_fx_prdct_cd       [ 6+1];
	char	v_regi_ymd          [ 8+1];
	char	v_rgst_time         [ 9+1];
	char	v_bid_last_prc      [20+1];
	char	v_offer_last_prc    [20+1];
	char	v_mid_last_prc      [20+1];
	char	v_prdat_cntst_prc   [20+1];
	char	v_updwn_rato        [ 8+1];
	char	v_usd_mrkup_dstcd	[ 1+1];
	char	v_usd_bid_last_prc	[20+1];
	char	v_usd_offer_last_prc[20+1];
	char	v_usd_mid_last_prc	[20+1];
	
	double	d_bid_high_prc		;
	double	d_bid_low_prc		;
	double	d_offer_high_prc	;
	double	d_offer_low_prc		;
	double	d_usd_bid_high_prc	;
	double	d_usd_bid_low_prc	;
	double	d_usd_offer_high_prc;
	double	d_usd_offer_low_prc	;
	
	sqlint32		i_bh				;
	sqlint32		i_bl				;
	sqlint32		i_oh				;
	sqlint32		i_ol				;
	sqlint32		i_usdpos			;

/*
EXEC SQL END DECLARE SECTION;
*/

#line 1466 "mdsrdb.sqc"


	double	fn_bid				= 0.0;
	double	fn_offer			= 0.0;
	double	fn_cur_bid_high		= 0.0;
	double	fn_cur_offer_high	= 0.0;
	double	fn_cur_bid_low		= 0.0;
	double	fn_cur_offer_low	= 0.0;

	double	fn_baseprc			= 0.0;
	double	d_prdat_cntst_prc	= 0.0;
	double	d_updwn_rato		= 0.0;
	double	d_mid_last_prc		= 0.0;
	double	d_bid_prc			= 0.0;
	double	d_offer_prc			= 0.0;
	double	d_usd_bid_prc		= 0.0;
	double	d_usd_offer_prc		= 0.0;

	memset(v_mrkup_dstcd    	, 0x00,	sizeof(v_mrkup_dstcd    ));
	memset(v_fx_prdct_cd    	, 0x00,	sizeof(v_fx_prdct_cd    ));
	memset(v_regi_ymd       	, 0x00,	sizeof(v_regi_ymd       ));
	memset(v_rgst_time      	, 0x00,	sizeof(v_rgst_time      ));
	memset(v_bid_last_prc   	, 0x00,	sizeof(v_bid_last_prc   ));
	memset(v_offer_last_prc 	, 0x00,	sizeof(v_offer_last_prc ));
	memset(v_mid_last_prc   	, 0x00,	sizeof(v_mid_last_prc   ));
	memset(v_prdat_cntst_prc	, 0x00,	sizeof(v_prdat_cntst_prc));
	memset(v_updwn_rato     	, 0x00,	sizeof(v_updwn_rato     ));
	memset(v_usd_mrkup_dstcd	, 0x00, sizeof(v_usd_mrkup_dstcd));
	memset(v_usd_bid_last_prc	, 0x00, sizeof(v_usd_bid_last_prc));
	memset(v_usd_offer_last_prc	, 0x00, sizeof(v_usd_offer_last_prc));
	memset(v_usd_mid_last_prc	, 0x00, sizeof(v_usd_mid_last_prc));
//	memset(v_prdct_cd    	    , 0x00,	sizeof(v_prdct_cd       ));

	d_bid_high_prc			= 0.0;
	d_bid_low_prc			= 0.0;
	d_offer_high_prc		= 0.0;
	d_offer_low_prc			= 0.0;
	d_usd_bid_high_prc		= 0.0;
	d_usd_bid_low_prc		= 0.0;
	d_usd_offer_high_prc	= 0.0;
	d_usd_offer_low_prc		= 0.0;

	i_bh = i_bl = i_oh = i_ol = i_usdpos = 0;

	i_usdpos = pfold->usdpos;			// USD위치 ("0": CADKRW=USDKRW/USDCAD, "1":EURKRW=USDKRW*EURUSD)

	if (i_usdpos == 0)		memcpy(v_fx_prdct_cd, &pfold->symb[3], 3);
	else					memcpy(v_fx_prdct_cd, pfold->symb, 3);
	memcpy(&v_fx_prdct_cd[3], "KRW", 3);

	sprintf(v_regi_ymd,		"%.8s",			psise->date);
	sprintf(v_rgst_time,	"%.4s00000",	psise->time);

	v_mrkup_dstcd[0] = GetFeeMrkup(pfold->bidex[0]);

	// 재정통화계산 (Bid/offer)
	//    이종통화(USDJPY)는 psise(recv@packet) 사용하며, USDKRW는 메모리값(latest price@memory)으로 사용 
	if (memcmp(psise->symb, DF_USDKRW, 6) == 0) // in case,  recv data : USDKRW
	{
		d_bid_prc		= pfold->bidlast;
		d_offer_prc		= pfold->offerlast;
		d_usd_bid_prc	= psise->bidprc;		//pusdkrw->bidlast;
		d_usd_offer_prc	= psise->offerprc;		//pusdkrw->offerlast;
	}
	else // in case,  recv data : USDJPY
	{
		d_bid_prc		= psise->bidprc;
		d_offer_prc		= psise->offerprc;
		d_usd_bid_prc	= pusdkrw->bidlast;
		d_usd_offer_prc	= pusdkrw->offerlast;
	}

	// 시세정합성 점검
	if (d_bid_prc == 0. || d_offer_prc == 0. || pusdkrw->bidlast == 0. || pusdkrw->offerlast == 0.)
		return (0);

	// 수신한 환율 기준으로 재정통화 환율계산
	fn_bid		= (i_usdpos == 0 ? d_usd_bid_prc/d_offer_prc : d_usd_bid_prc*d_bid_prc);
	fn_offer	= (i_usdpos == 0 ? d_usd_offer_prc/d_bid_prc : d_usd_offer_prc*d_offer_prc);
	fn_baseprc	= (i_usdpos == 0 ? pusdkrw->baseprc/pfold->baseprc : pusdkrw->baseprc*pfold->baseprc);

	// 중간값은 이종통화의 중간값을 저장
	d_mid_last_prc = (fn_bid + fn_offer)/2;
//	sprintf(v_mid_last_prc,		"%.*f",		pind, d_mid_last_prc);
	sprintf(v_mid_last_prc,		"%.*f",		pind, (d_bid_prc+d_offer_prc)/2);

	if (d_mid_last_prc <= 0.0)	return (-1);

	d_prdat_cntst_prc	= d_mid_last_prc - fn_baseprc;
	d_updwn_rato		= (d_prdat_cntst_prc / d_mid_last_prc) * 100.;
	sprintf(v_prdat_cntst_prc,	"%.*f",		pind, d_prdat_cntst_prc);
	sprintf(v_updwn_rato,		"%.*f",		pind, d_updwn_rato);

	// USDKRW
	v_usd_mrkup_dstcd[0] = GetFeeMrkup(pusdkrw->bidex[0]);

	sprintf(v_bid_last_prc,		"%.*f",		pind, d_bid_prc);
	sprintf(v_offer_last_prc,	"%.*f",		pind, d_offer_prc);

	sprintf(v_usd_bid_last_prc,		"%.*f",		pind, d_usd_bid_prc);
	sprintf(v_usd_offer_last_prc,	"%.*f",		pind, d_usd_offer_prc);
	sprintf(v_usd_mid_last_prc,		"%.*f",		pind, (d_usd_bid_prc+d_usd_offer_prc)/2);

	l_rtrim(v_mrkup_dstcd    );
	l_rtrim(v_fx_prdct_cd    );
	l_rtrim(v_regi_ymd       );
	l_rtrim(v_rgst_time      );
	l_rtrim(v_bid_last_prc   );
	l_rtrim(v_offer_last_prc );
	l_rtrim(v_mid_last_prc   );
	l_rtrim(v_prdat_cntst_prc);
	l_rtrim(v_updwn_rato     );
	l_rtrim(v_usd_mrkup_dstcd);
	l_rtrim(v_usd_bid_last_prc);
	l_rtrim(v_usd_offer_last_prc);
	l_rtrim(v_usd_mid_last_prc);

	
/*
EXEC SQL
	SELECT	-- CASE WHEN :i_usdpos = 0 THEN USD_BID_HIGH_PRC/BID_HIGH_PRC ELSE USD_BID_HIGH_PRC*BID_HIGH_PRC END
			-- 더블형의 DB계산값과 서버계산값이 달라서 쿼리로 계산하지 않고 서버에서 계산 후 비교
			 BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,USD_BID_HIGH_PRC
			,USD_BID_LOW_PRC
			,USD_OFFER_HIGH_PRC
			,USD_OFFER_LOW_PRC
	  INTO	 :d_bid_high_prc
			,:d_bid_low_prc
			,:d_offer_high_prc
			,:d_offer_low_prc
			,:d_usd_bid_high_prc
			,:d_usd_bid_low_prc
			,:d_usd_offer_high_prc
			,:d_usd_offer_low_prc
	  FROM	INST1.TSKEIMU72
	WHERE	GROUP_CO_CD	='KB0'
	  AND	FX_PRDCT_CD	= :v_fx_prdct_cd
	  AND	REGI_YMD	= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --:v_regi_ymd
	;
*/

{
#line 1606 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1606 "mdsrdb.sqc"
  sqlaaloc(2,1,21,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 1606 "mdsrdb.sqc"
  sqlaaloc(3,8,22,0L);
    {
      struct sqla_setdata_list sql_setdlist[8];
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 480; sql_setdlist[0].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)&d_bid_high_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 480; sql_setdlist[1].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)&d_bid_low_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 480; sql_setdlist[2].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)&d_offer_high_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 480; sql_setdlist[3].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)&d_offer_low_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 480; sql_setdlist[4].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)&d_usd_bid_high_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 480; sql_setdlist[5].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)&d_usd_bid_low_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 480; sql_setdlist[6].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)&d_usd_offer_high_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 480; sql_setdlist[7].sqllen = 8;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)&d_usd_offer_low_prc;
#line 1606 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1606 "mdsrdb.sqc"
      sqlasetdata(3,0,8,sql_setdlist,0L,0L);
    }
#line 1606 "mdsrdb.sqc"
  sqlacall((unsigned short)24,16,2,3,0L);
#line 1606 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1606 "mdsrdb.sqc"

	if (SQLCODE == 100) 
	{
		
/*
EXEC SQL
		INSERT INTO INST1.TSKEIMU72 (
			 GROUP_CO_CD
			,MRKUP_DSTCD
			,FX_PRDCT_CD
			,REGI_YMD
			,RGST_TIME
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,BID_OPEN_PRC
			,BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_OPEN_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,MID_LAST_PRC
			,USD_MRKUP_DSTCD
			,USD_BID_OPEN_PRC
			,USD_BID_HIGH_PRC
			,USD_BID_LOW_PRC
			,USD_BID_LAST_PRC
			,USD_OFFER_OPEN_PRC
			,USD_OFFER_HIGH_PRC
			,USD_OFFER_LOW_PRC
			,USD_OFFER_LAST_PRC
			,USD_MID_LAST_PRC
			,OPEN_PRC_TIME
			,HIGH_PRC_TIME
			,LOW_PRC_TIME
			,PRDAT_CNTST_PRC
			,UPDWN_RATO
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
		) VALUES (
			 'KB0'
			,:v_mrkup_dstcd
			,:v_fx_prdct_cd
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_usd_mrkup_dstcd
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_bid_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_offer_last_prc
			,:v_usd_mid_last_prc
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			, SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
			,:v_prdat_cntst_prc
			,:v_updwn_rato
			, SUBSTR(HEX(CURRENT TIMESTAMP), 1, 20)
			,'KEI000'
		);
*/

{
#line 1674 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1674 "mdsrdb.sqc"
  sqlaaloc(2,24,23,0L);
    {
      struct sqla_setdata_list sql_setdlist[24];
#line 1674 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_mid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 2;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_usd_mrkup_dstcd;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_usd_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_usd_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_usd_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_usd_bid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 460; sql_setdlist[17].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)v_usd_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 460; sql_setdlist[18].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)v_usd_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[19].sqltype = 460; sql_setdlist[19].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[19].sqldata = (void*)v_usd_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[19].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[20].sqltype = 460; sql_setdlist[20].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[20].sqldata = (void*)v_usd_offer_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[20].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[21].sqltype = 460; sql_setdlist[21].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[21].sqldata = (void*)v_usd_mid_last_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[21].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[22].sqltype = 460; sql_setdlist[22].sqllen = 21;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[22].sqldata = (void*)v_prdat_cntst_prc;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[22].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[23].sqltype = 460; sql_setdlist[23].sqllen = 9;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[23].sqldata = (void*)v_updwn_rato;
#line 1674 "mdsrdb.sqc"
      sql_setdlist[23].sqlind = 0L;
#line 1674 "mdsrdb.sqc"
      sqlasetdata(2,0,24,sql_setdlist,0L,0L);
    }
#line 1674 "mdsrdb.sqc"
  sqlacall((unsigned short)24,17,2,0,0L);
#line 1674 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1674 "mdsrdb.sqc"

		if (SQLCODE != 0)
		{
			mds_log(market, MLOG_ERROR, "TSKEIMU72 insert error [%.1s:%.6s:%.6s] [%s] \n", psise->excode, psise->symb, v_fx_prdct_cd, SQLERRM);
mds_log(market, MLOG_MUST, "data [%s,%s,%s] [%s,%s,%s,%s]"
,v_bid_last_prc
,v_offer_last_prc
,v_mid_last_prc
,v_usd_mrkup_dstcd
,v_usd_bid_last_prc
,v_usd_offer_last_prc
,v_usd_mid_last_prc
);
			return(-1);
		}
		return (0);
	}
	else if (SQLCODE != 0)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU72 select error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}

	/*===============================================================
	 * 고가와 저가는 계산된 재정환율을 기준으로 비교 함
	 * 계산된 재정환율이 고가를 갱신한 경우에 각 HIGH_PRC, USD_HIGH_PRC, HIGH_PRC_TIME 업데이트
	 * 계산된 재정환율이 저가를 갱신한 경우에 각 LOW_PRC, USD_LOW_PRC, LOW_PRC_TIME 업데이트
	 *-----------------------------------------------
	 **** fn_bid, fn_offer 값 계산 시 두 종목 중 하나는 마스터메모리 값을 쓰게 되는데, 
	 ****  이 때 update_master()의 메모리 update 시점이 일치하지 않으므로 계산된 값과 저장된 값이 다를 수 있음.
	===============================================================*/
	fn_cur_bid_high		= (i_usdpos == 0 ? d_usd_bid_high_prc/d_offer_high_prc : d_usd_bid_high_prc*d_bid_high_prc);
	fn_cur_bid_low		= (i_usdpos == 0 ? d_usd_bid_low_prc/d_offer_low_prc   : d_usd_bid_low_prc*d_bid_low_prc);
	fn_cur_offer_high	= (i_usdpos == 0 ? d_usd_offer_high_prc/d_bid_high_prc : d_usd_offer_high_prc*d_offer_high_prc);
	fn_cur_offer_low	= (i_usdpos == 0 ? d_usd_offer_low_prc/d_bid_low_prc   : d_usd_offer_low_prc*d_offer_low_prc);

	// 소수점 이슈로 동일한 값을 계속 업데이트 하는 현상 제거를 위해 ULONG 으로 타입 변환
	ulong	ubid, ubid_cur_h, ubid_cur_l;
	ulong	uoffer, uoffer_cur_h, uoffer_cur_l;
	int		x = 6;	//pusdkrw->zCustdiv;		// 표시기준인 소수점 2자리만 비교하여 계산하면 오류성으로 보이는 데이터 많이 보임

	ubid         = (ulong)(pow(10, x)*fn_bid + 0.5);
	ubid_cur_h   = (ulong)(pow(10, x)*fn_cur_bid_high + 0.5);
	ubid_cur_l   = (ulong)(pow(10, x)*fn_cur_bid_low  + 0.5);

	uoffer       = (ulong)(pow(10, x)*fn_offer + 0.5);
	uoffer_cur_h = (ulong)(pow(10, x)*fn_cur_offer_high + 0.5);
	uoffer_cur_l = (ulong)(pow(10, x)*fn_cur_offer_low  + 0.5);

	if (ubid > ubid_cur_h)			i_bh = 1;
	else if (ubid < ubid_cur_l)		i_bl = 1;

	if (uoffer > uoffer_cur_h)		i_oh = 1;
	else if (uoffer < uoffer_cur_l)	i_ol = 1;
/*
if (memcmp(psise->symb, "USDSEK", 6) == 0) {
mds_log(market, MLOG_DEBUG, "[%.6s:%d] curr bh=[%f/%f>%f] bl=[%f/%f>%f] oh=[%f/%f>%f] ol=[%f/%f>%f]", psise->symb, i_bh + i_bl + i_oh + i_ol,
d_usd_bid_high_prc, d_offer_high_prc, fn_cur_bid_high, d_usd_bid_low_prc, d_offer_low_prc, fn_cur_bid_low, 
d_usd_offer_high_prc, d_bid_high_prc, fn_cur_offer_high, d_usd_offer_low_prc, d_bid_low_prc, fn_cur_offer_low); }
*/

/*
	if (i_bh + i_bl + i_oh + i_ol)
	{
//		mds_log(market, MLOG_DEBUG, "Changed! [%s] [%.1s:%.6s] BID[%d:%d:%f] hl[%f:%f] ASK[%d:%d:%f] hl[%f:%f] NEW ! ", __func__, psise->excode, v_fx_prdct_cd, 
		mds_log(market, MLOG_DEBUG, "Changed! [%s] [%.1s:%.6s:%d] BID[%d:%d:%u] hl[%u:%u] ASK[%d:%d:%u] hl[%u:%u] NEW ! ", __func__, psise->excode, v_fx_prdct_cd, i_usdpos,
			i_bh, i_bl,
//			fn_bid, fn_cur_bid_high, fn_cur_bid_low,  // bid DB 값
ubid, ubid_cur_h, ubid_cur_l,
			i_oh, i_ol,
//			fn_offer, fn_cur_offer_high, fn_cur_offer_low); // offer DB값
uoffer, uoffer_cur_h, uoffer_cur_l);
		mds_log(market, MLOG_DEBUG, "    [%s] [rcv %.6s:%s] [%.6s:%s:%s:%s] [%.6s:%s:%s:%s] Update ! ", __func__, psise->symb, v_rgst_time, 
			pfold->symb, v_bid_last_prc, v_offer_last_prc, v_mid_last_prc, pusdkrw->symb, v_usd_bid_last_prc, v_usd_offer_last_prc, v_usd_mid_last_prc);
	}
*/
	
/*
EXEC SQL 
	UPDATE INST1.TSKEIMU72
	SET	 GROUP_CO_CD        = 'KB0'
		,MRKUP_DSTCD        = :v_mrkup_dstcd
		,FX_PRDCT_CD        = :v_fx_prdct_cd
--		,REGI_YMD           = :v_regi_ymd
		,RGST_TIME          = :v_rgst_time
		,BID_LAST_PRC       = :v_bid_last_prc
		,OFFER_LAST_PRC     = :v_offer_last_prc
--		,BID_OPEN_PRC       = :v_bid_last_prc
--		,BID_HIGH_PRC       = DECODE(:i_bh, 1, :v_bid_last_prc, BID_HIGH_PRC)
--		,BID_LOW_PRC        = DECODE(:i_bl, 1, :v_bid_last_prc, BID_LOW_PRC )
		,BID_HIGH_PRC       = CASE WHEN ((:i_bh = 1 AND :i_usdpos = 1) OR (:i_oh = 1 AND :i_usdpos = 0)) THEN :v_bid_last_prc	ELSE BID_HIGH_PRC END
		,BID_LOW_PRC        = CASE WHEN ((:i_bl = 1 AND :i_usdpos = 1) OR (:i_ol = 1 AND :i_usdpos = 0)) THEN :v_bid_last_prc	ELSE BID_LOW_PRC  END
--		,OFFER_OPEN_PRC     = :v_offer_last_prc
--		,OFFER_HIGH_PRC     = DECODE(:i_oh, 1, :v_offer_last_prc, OFFER_HIGH_PRC)
--		,OFFER_LOW_PRC      = DECODE(:i_ol, 1, :v_offer_last_prc, OFFER_LOW_PRC )
		,OFFER_HIGH_PRC     = CASE WHEN ((:i_oh = 1 AND :i_usdpos = 1) OR (:i_bh = 1 AND :i_usdpos = 0)) THEN :v_offer_last_prc	ELSE OFFER_HIGH_PRC END
		,OFFER_LOW_PRC      = CASE WHEN ((:i_ol = 1 AND :i_usdpos = 1) OR (:i_bl = 1 AND :i_usdpos = 0)) THEN :v_offer_last_prc	ELSE OFFER_LOW_PRC  END
		,MID_LAST_PRC       = :v_mid_last_prc
		,USD_MRKUP_DSTCD    = :v_usd_mrkup_dstcd
--		,USD_BID_OPEN_PRC   = :v_usd_bid_last_prc
		,USD_BID_HIGH_PRC   = DECODE(:i_bh, 1, :v_usd_bid_last_prc, USD_BID_HIGH_PRC)
		,USD_BID_LOW_PRC    = DECODE(:i_bl, 1, :v_usd_bid_last_prc, USD_BID_LOW_PRC )
		,USD_BID_LAST_PRC   = :v_usd_bid_last_prc  
--		,USD_OFFER_OPEN_PRC = :v_usd_offer_last_prc
		,USD_OFFER_HIGH_PRC = DECODE(:i_oh, 1, :v_usd_offer_last_prc, USD_OFFER_HIGH_PRC)
		,USD_OFFER_LOW_PRC  = DECODE(:i_ol, 1, :v_usd_offer_last_prc, USD_OFFER_LOW_PRC )
		,USD_OFFER_LAST_PRC = :v_usd_offer_last_prc
		,USD_MID_LAST_PRC   = :v_usd_mid_last_prc
--		,OPEN_PRC_TIME      = SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9)
		,HIGH_PRC_TIME      = CASE WHEN :i_bh + :i_oh > 0 THEN SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9) ELSE HIGH_PRC_TIME END
		,LOW_PRC_TIME       = CASE WHEN :i_bl + :i_ol > 0 THEN SUBSTR(HEX(CURRENT TIMESTAMP), 9, 9) ELSE LOW_PRC_TIME  END
		,PRDAT_CNTST_PRC    = :v_prdat_cntst_prc
		,UPDWN_RATO         = :v_updwn_rato
		,SYS_LAST_PRCSS_YMS = SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
		,SYS_LAST_UNO       = 'KEI000'
	WHERE	GROUP_CO_CD	='KB0'
	  AND	FX_PRDCT_CD	= :v_fx_prdct_cd
	  AND	REGI_YMD	= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --:v_regi_ymd
	;
*/

{
#line 1790 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1790 "mdsrdb.sqc"
  sqlaaloc(2,45,24,0L);
    {
      struct sqla_setdata_list sql_setdlist[45];
#line 1790 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 2;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_mrkup_dstcd;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 10;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_rgst_time;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 496; sql_setdlist[5].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)&i_bh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 496; sql_setdlist[6].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 496; sql_setdlist[7].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)&i_oh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 496; sql_setdlist[8].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 496; sql_setdlist[10].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)&i_bl;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 496; sql_setdlist[11].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 496; sql_setdlist[12].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)&i_ol;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 496; sql_setdlist[13].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 496; sql_setdlist[15].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)&i_oh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 496; sql_setdlist[16].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[17].sqltype = 496; sql_setdlist[17].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[17].sqldata = (void*)&i_bh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[17].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[18].sqltype = 496; sql_setdlist[18].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[18].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[18].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[19].sqltype = 460; sql_setdlist[19].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[19].sqldata = (void*)v_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[19].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[20].sqltype = 496; sql_setdlist[20].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[20].sqldata = (void*)&i_ol;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[20].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[21].sqltype = 496; sql_setdlist[21].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[21].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[21].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[22].sqltype = 496; sql_setdlist[22].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[22].sqldata = (void*)&i_bl;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[22].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[23].sqltype = 496; sql_setdlist[23].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[23].sqldata = (void*)&i_usdpos;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[23].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[24].sqltype = 460; sql_setdlist[24].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[24].sqldata = (void*)v_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[24].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[25].sqltype = 460; sql_setdlist[25].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[25].sqldata = (void*)v_mid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[25].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[26].sqltype = 460; sql_setdlist[26].sqllen = 2;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[26].sqldata = (void*)v_usd_mrkup_dstcd;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[26].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[27].sqltype = 496; sql_setdlist[27].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[27].sqldata = (void*)&i_bh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[27].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[28].sqltype = 460; sql_setdlist[28].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[28].sqldata = (void*)v_usd_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[28].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[29].sqltype = 496; sql_setdlist[29].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[29].sqldata = (void*)&i_bl;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[29].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[30].sqltype = 460; sql_setdlist[30].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[30].sqldata = (void*)v_usd_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[30].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[31].sqltype = 460; sql_setdlist[31].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[31].sqldata = (void*)v_usd_bid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[31].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[32].sqltype = 496; sql_setdlist[32].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[32].sqldata = (void*)&i_oh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[32].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[33].sqltype = 460; sql_setdlist[33].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[33].sqldata = (void*)v_usd_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[33].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[34].sqltype = 496; sql_setdlist[34].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[34].sqldata = (void*)&i_ol;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[34].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[35].sqltype = 460; sql_setdlist[35].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[35].sqldata = (void*)v_usd_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[35].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[36].sqltype = 460; sql_setdlist[36].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[36].sqldata = (void*)v_usd_offer_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[36].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[37].sqltype = 460; sql_setdlist[37].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[37].sqldata = (void*)v_usd_mid_last_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[37].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[38].sqltype = 496; sql_setdlist[38].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[38].sqldata = (void*)&i_bh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[38].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[39].sqltype = 496; sql_setdlist[39].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[39].sqldata = (void*)&i_oh;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[39].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[40].sqltype = 496; sql_setdlist[40].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[40].sqldata = (void*)&i_bl;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[40].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[41].sqltype = 496; sql_setdlist[41].sqllen = 4;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[41].sqldata = (void*)&i_ol;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[41].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[42].sqltype = 460; sql_setdlist[42].sqllen = 21;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[42].sqldata = (void*)v_prdat_cntst_prc;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[42].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[43].sqltype = 460; sql_setdlist[43].sqllen = 9;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[43].sqldata = (void*)v_updwn_rato;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[43].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[44].sqltype = 460; sql_setdlist[44].sqllen = 7;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[44].sqldata = (void*)v_fx_prdct_cd;
#line 1790 "mdsrdb.sqc"
      sql_setdlist[44].sqlind = 0L;
#line 1790 "mdsrdb.sqc"
      sqlasetdata(2,0,45,sql_setdlist,0L,0L);
    }
#line 1790 "mdsrdb.sqc"
  sqlacall((unsigned short)24,18,2,0,0L);
#line 1790 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1790 "mdsrdb.sqc"

	if (SQLCODE != 0)
	{
		if (abs(SQLCODE) == 420)
			mds_log(market, MLOG_ERROR, "%s,%s,%s,%s,%s,%s,%s,%s,%s"
					,v_fx_prdct_cd
					,v_bid_last_prc
					,v_offer_last_prc
					,v_mid_last_prc
					,v_usd_bid_last_prc  
					,v_usd_offer_last_prc
					,v_usd_mid_last_prc
					,v_prdat_cntst_prc
					,v_updwn_rato);
		mds_log(market, MLOG_ERROR, "TSKEIMU72 update error [%.1s:%.6s:%.6s] [%s]\n", psise->excode, psise->symb, pfold->symb, SQLERRM);
		return(-1);
	}

	return (0);

} /* End of int UpsertDay_Fincl(MDSSISE *psise, MDFOLD *pfold, MDFOLD *pusdkrw) */


void SetCustFeed_all(MARKET *m)
{
	int		ii;
	MDARCH	*parch;
	MDFOLD	*pfold;
	
	parch = (MDARCH *)m->arch;
	pfold = (MDFOLD *)m->fold;

//	while (pfold = mds_popfolder(m)))
	for (ii=0; ii<parch->nrec; ii++, pfold++)
	{
		SetCustFeed(m, pfold);

		// TSKEIAM97 테이블에서 관리하지 않는 통화코드 SKIP
		if (memcmp(pfold->symb, "CNHKRW", 6) == 0 || memcmp(pfold->symb, "MARKRW", 6) == 0)
		{
			if (pfold->cust.excode[0] == 0x00 || pfold->cust.excode[0] == ' ')
				pfold->cust.excode[0] = DF_EXCD_HAND;
			if (pfold->cust.feedtp[0] == 0x00 || pfold->cust.feedtp[0] == ' ')
				pfold->cust.feedtp[0] = DF_CFEED_HAND;
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// 2024.04.01
// 각 원천 데몬들이 각자의 시간관리를 하는 구조에서 CUST 데몬에서 일괄로 관리하는 구조로 변경 함.
// ★★★★★ 내용변경 시 API함수 mds_updt_custfeed() 도 같이 변경해줘야 함
//////////////////////////////////////////////////////////////////////////////////////////////////////
int SetCustFeed(MARKET *m, MDFOLD *pfold)
{
	int		chflag=0;		// 변경여부
	time_t	clock;
	struct	tm tm;
	uint32_t nextday;
	char	stime	[14+1];
	char	etime	[14+1];
	char	ctime	[14+1];
	char	cur_hms	[ 6+1];


/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 1857 "mdsrdb.sqc"

	char v_fx_prdct_cd			[ 6+1];
	char v_prc_orign_dstcd		[ 1+1];
	char v_fincl_anoun_stop_yn	[ 1+1];
	char v_fcm_id				[10+1];
	char v_start_hms			[ 6+1];
	char v_end_hms				[ 6+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 1864 "mdsrdb.sqc"


	// TSKEIAM97 테이블에서 관리하지 않는 통화코드 SKIP
	if (memcmp(pfold->symb, "CNHKRW", 6) == 0 || memcmp(pfold->symb, "MARKRW", 6) == 0)
		return (chflag);

	// 각 원천 데몬들이 각자의 시간관리를 하는 구조에서 CUST 데몬에서 일괄로 관리하는 구조로 변경 함.
	if (m->excode[0] != DF_EXCD_CUST)
	{
		mds_log(m, MLOG_DEBUG, "%s is just for the CUST mast [%s] !!!!!!  ", __func__,  m->exnm);
		return (chflag);
	}

	memset(v_fx_prdct_cd		, 0x00, sizeof(v_fx_prdct_cd		));
	memset(v_prc_orign_dstcd	, 0x00, sizeof(v_prc_orign_dstcd	));
	memset(v_fincl_anoun_stop_yn, 0x00, sizeof(v_fincl_anoun_stop_yn));
	memset(v_fcm_id				, 0x00, sizeof(v_fcm_id				));
	memset(v_start_hms			, 0x00, sizeof(v_start_hms			));
	memset(v_end_hms			, 0x00, sizeof(v_end_hms			));

	// 24.03.18) 화면(1801) 에서의 관리는 USDCNY 기준으로 하기 때문에 USDCNH 인 경우는 USDCNY를 카피해서 사용
	if (memcmp(pfold->symb, "USDCNH", 6) == 0)		// 화면에서 USDCNH로 관리되므로 USDCNY는 USDCNH 시간설정을 동일하게 처리.
		memcpy(v_fx_prdct_cd, "USDCNY", sizeof(pfold->symb));
	else
		memcpy(v_fx_prdct_cd, pfold->symb, sizeof(pfold->symb));

mds_log(m, MLOG_DEBUG, "%s (BEFORE) [%.1s:%.6s:%d] [%.1s:%.1s:%.1s:%.14s~%.14s]", __func__, m->excode, pfold->symb, pfold->trdf, 
				pfold->cust.excode, pfold->cust.feedtp, pfold->cust.finclstop, pfold->cust.stm, pfold->cust.etm);

/*
EXEC SQL
	SELECT	 FX_PRDCT_CD
			,PRC_ORIGN_DSTCD
			,FINCL_ANOUN_STOP_YN
			,PROR_FCM_ID
	  INTO	 :v_fx_prdct_cd
			,:v_prc_orign_dstcd
			,:v_fincl_anoun_stop_yn
			,:v_fcm_id
	  FROM	INST1.TSKEIAM97
	 WHERE	FX_PRDCT_CD = :v_fx_prdct_cd
	WITH UR
	;
*/

{
#line 1904 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1904 "mdsrdb.sqc"
  sqlaaloc(2,1,25,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1904 "mdsrdb.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 1904 "mdsrdb.sqc"
  sqlaaloc(3,4,26,0L);
    {
      struct sqla_setdata_list sql_setdlist[4];
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 2;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_prc_orign_dstcd;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 2;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_fincl_anoun_stop_yn;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 11;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_fcm_id;
#line 1904 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 1904 "mdsrdb.sqc"
      sqlasetdata(3,0,4,sql_setdlist,0L,0L);
    }
#line 1904 "mdsrdb.sqc"
  sqlacall((unsigned short)24,19,2,3,0L);
#line 1904 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1904 "mdsrdb.sqc"

	if (SQLCODE != 0)
	{
		mds_log(m, MLOG_ERROR, "TSKEIAM97 select error [%.1s:%.6s] [%s] \n", m->excode, pfold->symb, SQLERRM);
		return (-1);
	}

	l_rtrim(v_fx_prdct_cd		);
	l_rtrim(v_prc_orign_dstcd	);
	l_rtrim(v_fincl_anoun_stop_yn);
	l_rtrim(v_fcm_id			);

	if (pfold->cust.feedtp[0]  != v_prc_orign_dstcd[0])
	{
		mds_log(m, MLOG_BIZ, "[%s] 고객제공원천 for [%.6s] 변경됨 -> [%.1s]",__func__, pfold->symb, v_prc_orign_dstcd);

		// 원천변경 시 trdf 거래불가로 세팅
		if (v_prc_orign_dstcd[0] == DF_CFEED_HAND || v_prc_orign_dstcd[0] == DF_CFEED_STOP)
		{
			mds_log(m, MLOG_BIZ, "[%s] 원천변경으로 '거래불가'로 상태 변경 [%.6s]",__func__, pfold->symb);
			pfold->trdf = DF_OFF;
		}
		pfold->cust.feedtp[0]  = v_prc_orign_dstcd[0];
		chflag = 1;
	}
	if (pfold->cust.finclstop[0] != v_fincl_anoun_stop_yn[0])
	{
		mds_log(m, MLOG_BIZ, "[%s] 고객 재정환율제공 for [%.6s] 변경됨 -> [%.1s]",__func__, pfold->symb, v_fincl_anoun_stop_yn);
		pfold->cust.finclstop[0] = v_fincl_anoun_stop_yn[0];
		chflag = 1;
	}

	clock = time(0);
	localtime_r(&clock, &tm);

	// '가격원천구분코드'에 따른 분기처리
	switch (v_prc_orign_dstcd[0])
	{
	case DF_CFEED_STOP :		// 고객시세 : 제공중단
//		mds_log(m, MLOG_DEBUG, "[%s] [%.6s] 고객시세 중단",__func__, pfold->symb);
		break;

	case DF_CFEED_HAND :		// 고객시세 : 수기제공
//		mds_log(m, MLOG_DEBUG, "[%s] [%.6s] 고객시세 수기",__func__, pfold->symb);
		break;

	case DF_CFEED_PRIO :		// 고객시세 : 우선설정
//		mds_log(m, MLOG_DEBUG, "[%s] [%.6s] 고객시세 우선 [%s]",__func__, pfold->symb, v_fcm_id);

		if (v_fcm_id[0] == 0x00 || v_fcm_id[0] == ' ')
		{
			mds_log(m, MLOG_ERROR, "[%s] [%.6s] 고객시세 우선 원천코드 오류",__func__, pfold->symb);
			return (-2);
		}
		// ★★★★ TODO : EXNM의 첫글자와 EXCODE가 같다는 전제로 세팅함. (추후 다르게 세팅된 원천이 추가되면 코드 변경 필요)
		// 우선시세인 상태에서 원천만 변경될 수도 있기 때문에 UPDATE
		if (pfold->cust.excode[0] != v_fcm_id[0])
		{
			pfold->cust.excode[0] = v_fcm_id[0];
			chflag = 1;
		}
		break;

	case DF_CFEED_STND :		// 고객시세 : 기본설정(TSKEIAM98)

		memset(v_fcm_id		, 0x00, sizeof(v_fcm_id		));
		memset(v_start_hms	, 0x00, sizeof(v_start_hms	));
		memset(v_end_hms	, 0x00, sizeof(v_end_hms	));

		// 현재시간 기준으로 적용할 설정시간이 있는지 조회
	
/*
EXEC SQL
		SELECT	 FCM_ID
				,START_HMS
				,END_HMS
		  INTO	 :v_fcm_id
				,:v_start_hms
				,:v_end_hms
		FROM (
			SELECT	 FCM_ID, START_HMS, END_HMS
			  FROM	INST1.TSKEIAM98
			 WHERE	START_HMS < END_HMS
			   AND	(TO_CHAR(SYSDATE, 'HH24MISS') > START_HMS AND TO_CHAR(SYSDATE, 'HH24MISS') < END_HMS)
			   AND	FX_PRDCT_CD = :v_fx_prdct_cd
			UNION ALL
			SELECT	 FCM_ID, START_HMS, END_HMS
			  FROM	INST1.TSKEIAM98
			 WHERE	START_HMS > END_HMS
			   AND	(TO_CHAR(SYSDATE, 'HH24MISS') > START_HMS OR TO_CHAR(SYSDATE, 'HH24MISS') < END_HMS)
			   AND	FX_PRDCT_CD = :v_fx_prdct_cd
		);
*/

{
#line 1993 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 1993 "mdsrdb.sqc"
  sqlaaloc(2,2,27,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_fx_prdct_cd;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1993 "mdsrdb.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 1993 "mdsrdb.sqc"
  sqlaaloc(3,3,28,0L);
    {
      struct sqla_setdata_list sql_setdlist[3];
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 11;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fcm_id;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_start_hms;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 7;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_end_hms;
#line 1993 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 1993 "mdsrdb.sqc"
      sqlasetdata(3,0,3,sql_setdlist,0L,0L);
    }
#line 1993 "mdsrdb.sqc"
  sqlacall((unsigned short)24,20,2,3,0L);
#line 1993 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 1993 "mdsrdb.sqc"

		if (SQLCODE == 100)
		{
		// 현재 적용시간이 없으면 다음 설정시간대역 조회
		
/*
EXEC SQL
			SELECT	 FCM_ID
					,START_HMS
					,END_HMS
			  INTO	 :v_fcm_id
					,:v_start_hms
					,:v_end_hms
			  FROM	(	SELECT	 FCM_ID
			  					,START_HMS
								,END_HMS
								,CASE WHEN START_HMS > END_HMS THEN TO_CHAR(TO_NUMBER(END_HMS) + 240000) ELSE END_HMS END AS ETIME
						  FROM	INST1.TSKEIAM98
						 WHERE	FX_PRDCT_CD = :v_fx_prdct_cd
					)
			 WHERE	ETIME > TO_CHAR(SYSDATE, 'HH24MISS')
			ORDER BY ETIME
			FETCH FIRST 1 ROWS ONLY
			WITH UR;
*/

{
#line 2014 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 2014 "mdsrdb.sqc"
  sqlaaloc(2,1,29,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fx_prdct_cd;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 2014 "mdsrdb.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 2014 "mdsrdb.sqc"
  sqlaaloc(3,3,30,0L);
    {
      struct sqla_setdata_list sql_setdlist[3];
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 11;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_fcm_id;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 7;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_start_hms;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 7;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_end_hms;
#line 2014 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 2014 "mdsrdb.sqc"
      sqlasetdata(3,0,3,sql_setdlist,0L,0L);
    }
#line 2014 "mdsrdb.sqc"
  sqlacall((unsigned short)24,21,2,3,0L);
#line 2014 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 2014 "mdsrdb.sqc"

		}
		if (SQLCODE != 0)
		{		
			mds_log(m, MLOG_ERROR, "TSKEIAM98 select error [%.1s:%.6s] [%s] \n", m->excode, pfold->symb, SQLERRM);
//			mds_log(m, MLOG_MUST, "[%s] [%.6s] custfeed time reset [%.14s/%.14s]=>[9999/0000]", __func__, pfold->symb, pfold->cust.stm, pfold->cust.etm);
			if (memcmp(pfold->cust.stm, "99999999999999", sizeof(pfold->cust.stm)) || memcmp(pfold->cust.etm, "00000000000000", sizeof(pfold->cust.etm)))
			{
				memcpy(pfold->cust.stm, "99999999999999", sizeof(pfold->cust.stm));
				memcpy(pfold->cust.etm, "00000000000000", sizeof(pfold->cust.etm));
				mds_log(m, MLOG_BIZ, "[%s] 원천제공시간 정보없음. [%.6s]",__func__, pfold->symb);
				pfold->trdf = DF_OFF;
				chflag = 1;
			}
			break;
		}

		sprintf(ctime, "%04d%02d%02d%02d%02d%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		sprintf(cur_hms, "%02d%02d%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
		
		if (strncmp(v_start_hms, v_end_hms, sizeof(v_start_hms)) > 0)
		{
			if (strncmp(cur_hms, v_end_hms, sizeof(cur_hms)) < 0) {		// 일자가 넘어간 상태면 시작일자가 전일
				nextday = getnextday(-1);
				sprintf(stime, "%08d%.6s", nextday, v_start_hms);
				sprintf(etime, "%04d%02d%02d%.6s", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, v_end_hms);
			}
			else {														// 일자변경 전이면 종료일자가 익일
				nextday = getnextday(1);
				sprintf(etime, "%08d%.6s", nextday, v_end_hms);
				sprintf(stime, "%04d%02d%02d%.6s", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, v_start_hms);
			}
		}
		else
		{
			sprintf(stime, "%04d%02d%02d%.6s", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, v_start_hms);
			sprintf(etime, "%04d%02d%02d%.6s", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, v_end_hms);
		}

		if (memcmp(pfold->cust.stm, stime, sizeof(pfold->cust.stm)) != 0)
		{
			mds_log(m, MLOG_BIZ, "[%s] [%.6s] starttime 변경됨 [%.14s -> %.14s]",__func__, pfold->symb, pfold->cust.stm, stime);
			memcpy(pfold->cust.stm, stime, sizeof(pfold->cust.stm));
			chflag = 1;
		}
		if (memcmp(pfold->cust.etm, etime, sizeof(pfold->cust.etm)) != 0)
		{
			mds_log(m, MLOG_BIZ, "[%s] [%.6s] endtime 변경됨 [%.14s -> %.14s]",__func__, pfold->symb, pfold->cust.etm, etime);
			memcpy(pfold->cust.etm, etime, sizeof(pfold->cust.etm));
			chflag = 1;
		}
		if (memcmp(pfold->cust.excode, v_fcm_id, sizeof(pfold->cust.excode)) != 0)
		{
			mds_log(m, MLOG_BIZ, "[%s] [%.6s] 원천 변경됨 [%.1s -> %.1s]",__func__, pfold->symb, pfold->cust.excode, v_fcm_id);
			// ★★★★ TODO : EXNM의 첫글자와 EXCODE가 같다는 전제로 세팅함. (추후 다르게 세팅된 원천이 추가되면 코드 변경 필요)
			pfold->cust.excode[0] =  v_fcm_id[0];
			chflag = 1;
		}

		if (memcmp(pfold->cust.stm, ctime, sizeof(pfold->cust.stm)) > 0 || memcmp(pfold->cust.etm, ctime, sizeof(pfold->cust.etm)) < 0)
		{
			mds_log(m, MLOG_BIZ, "[%s] 원천제공시간 아님. [%.6s] [%.14s - %.14s]",__func__, pfold->symb, pfold->cust.stm, pfold->cust.etm);
			pfold->trdf = DF_OFF;
		}
		break;

	default :
		mds_log(market, MLOG_ERROR, "TSKEIAM98 v_prc_orign_dstcd error [%.1s:%.6s] [%.1s] \n", market->excode, pfold->symb, v_prc_orign_dstcd);
	}

	l_db2commit();

mds_log(m, MLOG_DEBUG, "%s (AFTER)  [%.1s:%.6s:%d] [%.1s:%.1s:%.1s:%.14s~%.14s]", __func__, m->excode, pfold->symb, pfold->trdf, 
				pfold->cust.excode, pfold->cust.feedtp, pfold->cust.finclstop, pfold->cust.stm, pfold->cust.etm);

	return (chflag);
}

//
// 딜러의 시장거래에 대한 북의 평가금액 계산을 위해 통화별 현재환율 Update (CUST가 아닌 원천.SMBS 수신 시 호출됨)
//
int UpsertMU59(MDSSISE *psise, MDFOLD *pfold)
{

/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 2097 "mdsrdb.sqc"

	sqlint32		i_cnt			;
	char	v_fx_prdct_cd	[ 6+1];
	char	v_rgst_time		[ 9+1];
	char	v_bid_last_prc	[20+1];
	char	v_offer_last_prc[20+1];
	char	v_mid_last_prc	[20+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 2104 "mdsrdb.sqc"


	memset(v_fx_prdct_cd		, 0x00,	sizeof(v_fx_prdct_cd    ));
	memset(v_rgst_time			, 0x00,	sizeof(v_rgst_time      ));
	memset(v_bid_last_prc		, 0x00,	sizeof(v_bid_last_prc   ));
	memset(v_offer_last_prc		, 0x00,	sizeof(v_offer_last_prc ));
	memset(v_mid_last_prc		, 0x00,	sizeof(v_mid_last_prc   ));

	if (psise->bidprc <= 0 || psise->offerprc <= 0)		return (0);

	sprintf(v_fx_prdct_cd,		pfold->symb);
	sprintf(v_bid_last_prc,		"%.*f",		pfold->zdiv, psise->bidprc);
	sprintf(v_offer_last_prc,	"%.*f",		pfold->zdiv, psise->offerprc);
	sprintf(v_mid_last_prc,		"%.*f",		pfold->zdiv, (psise->bidprc + psise->offerprc)/2);
	sprintf(v_rgst_time,		"%.6s",		psise->time);

/*
if(psise->bidprc<=0 || psise->offerprc<=0)
mds_log(market, MLOG_ERROR, "[%s:WARN] [%.1s:%.6s:%f:%f]\n", __func__, psise->excode, psise->symb, psise->bidprc, psise->offerprc);
*/
	l_rtrim(v_fx_prdct_cd    );
	l_rtrim(v_bid_last_prc   );
	l_rtrim(v_offer_last_prc );
	l_rtrim(v_mid_last_prc   );
	l_rtrim(v_rgst_time      );


/*
EXEC SQL 
	UPDATE	INST1.TSKEIMU59
	SET	 GROUP_CO_CD		= 'KB0'
--		,FX_PRDCT_CD		= :v_fx_prdct_cd
--		,FX_PRDCT_DSTCD		= '3'
--		,TNR_CD				= '00'
--		,REGI_YMD			= :v_rgst_date
		,RGST_TIME			= :v_rgst_time
		,BID_LAST_PRC		= :v_bid_last_prc
		,OFFER_LAST_PRC		= :v_offer_last_prc
		,MID_LAST_PRC		= :v_mid_last_prc
		,BID_HIGH_PRC		= CASE WHEN BID_HIGH_PRC < :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_HIGH_PRC END
		,BID_LOW_PRC		= CASE WHEN BID_LOW_PRC  > :v_bid_last_prc THEN :v_bid_last_prc ELSE BID_LOW_PRC  END
		,OFFER_HIGH_PRC     = CASE WHEN OFFER_HIGH_PRC < :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_HIGH_PRC END
		,OFFER_LOW_PRC      = CASE WHEN OFFER_LOW_PRC  > :v_offer_last_prc THEN :v_offer_last_prc ELSE OFFER_LOW_PRC  END
		,MID_HIGH_PRC       = CASE WHEN MID_HIGH_PRC < :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_HIGH_PRC END
		,MID_LOW_PRC        = CASE WHEN MID_LOW_PRC  > :v_mid_last_prc THEN :v_mid_last_prc ELSE MID_LOW_PRC  END
		,SYS_LAST_UNO		= 'SYSTEM'
		,SYS_LAST_PRCSS_YMS	= SUBSTR(HEX(CURRENT TIMESTAMP),1,20)
	WHERE	GROUP_CO_CD		='KB0'
	  AND	FX_PRDCT_DSTCD	= '3'
	  AND	FX_PRDCT_CD		= :v_fx_prdct_cd
	  AND	TRIM(TNR_CD)	= '00'
	  AND	REGI_YMD		= (SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
	;
*/

{
#line 2154 "mdsrdb.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 2154 "mdsrdb.sqc"
  sqlaaloc(2,17,31,0L);
    {
      struct sqla_setdata_list sql_setdlist[17];
#line 2154 "mdsrdb.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 10;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[0].sqldata = (void*)v_rgst_time;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[1].sqldata = (void*)v_bid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[2].sqldata = (void*)v_offer_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[3].sqldata = (void*)v_mid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[4].sqldata = (void*)v_bid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[5].sqldata = (void*)v_bid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[6].sqldata = (void*)v_bid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[7].sqltype = 460; sql_setdlist[7].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[7].sqldata = (void*)v_bid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[7].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[8].sqltype = 460; sql_setdlist[8].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[8].sqldata = (void*)v_offer_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[8].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[9].sqltype = 460; sql_setdlist[9].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[9].sqldata = (void*)v_offer_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[9].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[10].sqltype = 460; sql_setdlist[10].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[10].sqldata = (void*)v_offer_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[10].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[11].sqltype = 460; sql_setdlist[11].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[11].sqldata = (void*)v_offer_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[11].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[12].sqltype = 460; sql_setdlist[12].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[12].sqldata = (void*)v_mid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[12].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[13].sqltype = 460; sql_setdlist[13].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[13].sqldata = (void*)v_mid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[13].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[14].sqltype = 460; sql_setdlist[14].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[14].sqldata = (void*)v_mid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[14].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[15].sqltype = 460; sql_setdlist[15].sqllen = 21;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[15].sqldata = (void*)v_mid_last_prc;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[15].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[16].sqltype = 460; sql_setdlist[16].sqllen = 7;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[16].sqldata = (void*)v_fx_prdct_cd;
#line 2154 "mdsrdb.sqc"
      sql_setdlist[16].sqlind = 0L;
#line 2154 "mdsrdb.sqc"
      sqlasetdata(2,0,17,sql_setdlist,0L,0L);
    }
#line 2154 "mdsrdb.sqc"
  sqlacall((unsigned short)24,22,2,0,0L);
#line 2154 "mdsrdb.sqc"
  sqlastop(0L);
}

#line 2154 "mdsrdb.sqc"

	if (SQLCODE != 0) 
	{
//		mds_log(market, MLOG_DEBUG, "TSKEIMU59 update error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}
/*
	EXEC SQL 
		INSERT INTO INST1.TSKEIMU59 (
			 GROUP_CO_CD
			,FX_PRDCT_DSTCD
			,FX_PRDCT_CD
			,TNR_CD
			,REGI_YMD
			,RGST_TIME
			,BID_LAST_PRC
			,OFFER_LAST_PRC
			,MID_LAST_PRC
			,BID_OPEN_PRC
			,BID_HIGH_PRC
			,BID_LOW_PRC
			,OFFER_OPEN_PRC
			,OFFER_HIGH_PRC
			,OFFER_LOW_PRC
			,MID_OPEN_PRC
			,MID_HIGH_PRC
			,MID_LOW_PRC
--			,PRDAT_CNTST_PRC
--			,UPDWN_RATO
--			,TNR_BASE_YMD
--			,OFST
--			,SPTNODAY
--			,PDY_MID_LAST_PRC
			,SYS_LAST_PRCSS_YMS
			,SYS_LAST_UNO
		) VALUES (
			 'KB0'
			,'3'
			,:v_fx_prdct_cd
			,'00'
			,(SELECT 거래기준년월일 FROM INST1.TSKEIAM00) --v_regi_ymd
			,:v_rgst_time
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_bid_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_offer_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			,:v_mid_last_prc
			, SUBSTR(HEX(CURRENT TIMESTAMP), 1, 20) 
			,'SYSTEM'                               
		);
		if (SQLCODE != 0)
		{
			mds_log(market, MLOG_ERROR, "TSKEIMU59 insert error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
			return(-1);
		}
	}
	else if (SQLCODE != 0)
	{
		mds_log(market, MLOG_ERROR, "TSKEIMU59 update error [%.1s:%.6s] [%s] \n", psise->excode, psise->symb, SQLERRM);
		return(-1);
	}
*/
	return (0);

} /* End of int UpsertDay(MDFOLD *pfold, MDSSISE *psise) */

// END ##########################################################################################
//###############################################################################################
//###############################################################################################
