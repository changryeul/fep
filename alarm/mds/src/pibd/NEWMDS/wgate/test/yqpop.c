static char sqla_program_id[292] = 
{
 '\xac','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x6a','\x41','\x54','\x6e','\x4b','\x58','\x45','\x6f','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x8','\x0','\x59','\x51','\x50','\x4f','\x50','\x20','\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
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


#line 1 "yqpop.sqc"
#include "mds2.h"
#include "wfaapi.h"
#include "../wgate.h"

#include "quotio.h"
#include "comdef.h"


/*
EXEC SQL INCLUDE SQLCA;
*/

/* SQL Communication Area - SQLCA - structures and constants */
#include "sqlca.h"
struct sqlca sqlca;


#line 8 "yqpop.sqc"
 

/*
char    s_Date                    [  8+1];  // 조회일자
char    s_fx_prdct_dstcd          [  1+1];  // FX상품구분
char    s_fx_prdct_cd             [  6+1];  // FX상품코드
char    s_ctyp                    [  1+1];  // 접속매체 구분
char    s_lgen_no                 [ 15+1];  // 유저아이디(직원) or 시장참여자번호(고객)
char    s_RFSFlag                 [  1+1];  // 밴드환율조회여부 1이아닌값 = 밴드아님, 1= 밴드환율조회
char    s_base_ccy                [  3+1];  // 기준통화  밴드일때만 입력
char    s_calcAmount              [ 15+1];  // 밴드환산  밴드일때만 입력
char    s_Fincl_Yn                [  1+1] ;  // 재정여부
*/
/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		rc;
	char	item[32];
	QUOT_PRICEST	mm;
	E_MSG			emsg;

	if (l_db2connect() != 0)
		exit(0);

	memset(&mm,   0x00, sizeof(mm));
	memset(&emsg, 0x00, sizeof(emsg));

/*
	strcpy(mm.s_Date          , "20240110"  );
	strcpy(mm.s_fx_prdct_dstcd, "3"         );
	strcpy(mm.s_fx_prdct_cd   , "USDKRW"    );
	strcpy(mm.s_ctyp          , "1"         );
	strcpy(mm.s_lgen_no       , "0000104041     ");
	strcpy(mm.s_RFSFlag       , "0"         );
	strcpy(mm.s_Fincl_Yn      , "0"         );
	printf("Date(%s) : ", mm.s_Date);

	scanf("%s", item);
	if (strlen(item))	strncpy(mm.s_Date, item, sizeof(mm.s_Date));

	printf("Product(%s) : ", mm.s_fx_prdct_cd);
	scanf("%s", item);
	if (strlen(item))	strncpy(mm.s_fx_prdct_cd, item, sizeof(mm.s_fx_prdct_cd));

	printf("Lgen(%s) : ", mm.s_lgen_no);
	scanf("%s", item);
	if (strlen(item))	strncpy(mm.s_lgen_no, item, sizeof(mm.s_lgen_no));
	rc = f_get_CurrentQuote(&mm, &emsg);
	if (rc < 0)
	{
		printf("f_get_CurrentQuote error [%.10s:%s]\n", emsg.code, emsg.mesg);
		exit(0);
	}
double  d_bidSpotPrice                   ;  // Spot가격
double  d_bidSpot_mrkup                  ;  // bid Spot가격마크업
double  d_bidSwapPrice                   ;  // bid Swap가격
double  d_bidSwap_mrkup                  ;  // bid Swap가격마크업
double  d_bidPrice                       ;  // bid 가격
double  d_bid_mrgn                       ;  // bid 마진
double  d_askSpotPrice                   ;  // Spot가격
double  d_askSpot_mrkup                  ;  // ask Spot가격마크업
double  d_askSwapPrice                   ;  // ask Swap가격
double  d_askSwap_mrkup                  ;  // ask Swap가격마크업
double  d_askPrice                       ;  // ask 가격
double  d_ask_mrgn                       ;  // ask 마진
	printf("bid[%f/%f/%f/%f/%f/%f] ask[%f/%f/%f/%f/%f/%f]\n"
	,mm.d_bidSpotPrice ,mm.d_bidSpot_mrkup ,mm.d_bidSwapPrice ,mm.d_bidSwap_mrkup ,mm.d_bidPrice ,mm.d_bid_mrgn 
	,mm.d_askSpotPrice ,mm.d_askSpot_mrkup ,mm.d_askSwapPrice ,mm.d_askSwap_mrkup ,mm.d_askPrice ,mm.d_ask_mrgn);
*/

#if 1
	int		seq=0, idx=0, first=1;
	char	rbuf[1024];

	memset(rbuf, 0x00, sizeof(rbuf));

	char	*dpath  = "/fsfxwin/wfa";
	char	*ddir	= "dat";
	char	*dfnam	= "YARIQ";
	int		semk	= 0x16000351;
	int		dsiz	= 1024;
	int		rwno[2];

	if (argc > 1)	idx = atoi(argv[1]);

	AG_DQUE dque;
	memset(&dque, 0x00, sizeof(dque));
	AG_DQUECLS(&dque);
	
	dque.xha |= AQ_XHA_OPN;
	dque.xha |= AG_XAS_LIV;
	dque.xha |= AG_XAS_CFM;  // 필요 ? 

	//int agdque_at (Ag_Dque dque, char *dpath, char *ddir, char *dfnam, int semk, int rsiz)
	rc = agdque_at (&dque, WG_DAT_DIR, NULL, DQ_RCVRQ, SEM_RCVRQ, sizeof(rbuf));
	//rc = agdque_at (&dque, dpath, ddir, dfnam, semk, dsiz);
	if (rc < 0)
	{
		printf("\n agdque_at error.. [%d]\n", rc);
		exit(0);
	}
	printf("\n agdque_at ok.. [%d]\n", rc);

	/*
	int agdque_at (Ag_Dque dque, char *dpath, char *ddir, char *dfnam, int semk, int rsiz)
	int agdque_rdnx (Ag_Dque dque, char *pbuf, int psiz, int aid, int *rwno)
	int agdque_wrnx (Ag_Dque dque, char *pbuf, int plen)
	int agdque_rwi_io (Ag_Dque dque, int aid, int *rwno)
	//dque->hdr.rno[idx] = dque->hdr.wno;
	*/
	printf("set aid[%d] read no [%d -> %d]\n", idx, dque.hdr.rno[idx], dque.hdr.wno);
/*
	memset(&rwno, 0x00, sizeof(rwno));
	
	printf("set aid[%d] read no [%d -> %d]\n", idx, dque.hdr.rno[idx], dque.hdr.wno);
	rwno[0] = dque.hdr.wno;
	rc = agdque_rwi_io (&dque, idx, &rwno);
	if (rc < 0)
	{
		printf("\n agdque_rwi_io error.. [%d]\n", rc);
		exit(0);		
	}
*/
	while (1)
	{
		rc = agdque_rdn (&dque, dpath, ddir, dfnam, semk, rbuf, sizeof(rbuf), idx, (-1), &rwno);
		//rc = agdque_rdn (&dque, WG_DAT_DIR, NULL, DQ_RCVRQ, SEM_RCVRQ, rbuf, sizeof(rbuf), idx, 1000000, &rwno);
		if (rc == 0)
		{
			continue;
		}
		if (rc < 0)
		{
			printf("agdque_rdn error [%d]\n", rc);
			break;
		}
		seq++;
		printf("read:%d:%d) [%d:%d] [%s]\n", seq, rc, rwno[0], rwno[1], rbuf);
		memset(rbuf, 0x00, sizeof(rbuf));
	}
//	printf("read:%d:%d) [%d:%d] [%s]\n", seq, rc, rwno[0], rwno[1], rbuf);
#endif

	l_db2commit();

	l_db2disconnect();

	exit(0);
}
