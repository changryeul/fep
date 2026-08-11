static char sqla_program_id[292] = 
{
 '\xac','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x4a','\x41','\x76','\x69','\x4f','\x56','\x45','\x70','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x8','\x0','\x57','\x51','\x43','\x4c','\x49','\x41','\x50','\x49','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0'
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {'N',' ',' ',' '}};


static const short sqlIsLiteral   = SQL_IS_LITERAL;
static const short sqlIsInputHvar = SQL_IS_INPUT_HVAR;


#line 1 "wqcliapi.sqc"
#include "mds2.h"
#include "wfaapi.h"
#include "wgate.h"
#include "comrdb.h"


/*
EXEC SQL INCLUDE SQLCA;
*/

/* SQL Communication Area - SQLCA - structures and constants */
#include "sqlca.h"
struct sqlca sqlca;


#line 6 "wqcliapi.sqc"


char	tbuf[1024];
char	compid[32];
char	o_tf[8];	// timeinforce
char	o_ot[8];	// ordertype
AG_DQUE		qin;

struct _WGVAL {
	char			useyn[1];
	int				ordcnt;
	WGKEY			wgkey;
	WG_QUOT_REQ		qreq;
	WG_QUOT			quot;
} wgval;


/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 22 "wqcliapi.sqc"

	char	v_sendcompid		[50+1];
	char	v_trsmt_taget_id	[50+1];
	char	v_trsmt_taget_sub_id[50+1];
	char	v_recv_taget_id		[50+1];
	char	c_fcm_acno			[20+1];
	char	c_fx_prdct_cd		[ 6+1];
	char	c_tnr_cd			[ 1+1];
	char	c_spot_ymd			[ 8+1];
	char	c_expir_ymd			[ 8+1];
	char	v_telgm_dmnd_dstic	[ 1+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 33 "wqcliapi.sqc"


int sendquotreq();
int sendorder();
void *handler_quot(void *argv);
void _set_pad(char *p, int len);

void exitproc(int signo)
{
	int		rc, slen;
	char	sbuf[1024];
	WGCOM		*c=sbuf;
	WG_LOGOUT	*p=c->data;

	memset(sbuf, 0x00, sizeof(sbuf));
	
	if (wgval.useyn[0] != '1')	{
		l_db2disconnect();
		exit(0);
	}

	switch (signo)
	{
	case SIGINT : // RFS 해제 요청	
//		if (wgval.useyn[0] == '0')		break;

		memset(sbuf, 0x00, sizeof(sbuf));
		memcpy(sbuf, tbuf, sizeof(WGCOM));
		c->msgtp[0] = 'L';
		strcpy(p->text, "I'm done.");
		
		slen=sizeof(WGCOM)+sizeof(WG_LOGOUT);
		_set_pad(sbuf, slen);
		rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, 1024, sbuf, slen);
		if (rc < 0)	printf("agdque_wrn error : %d\n", rc);
		else		printf("\nsend unsubscribe packet ok..!!!! [%d]\n", rc);

		break;

	default : 
		break;
	}

	l_db2disconnect();

	exit(0) ;
}

void _set_pad(char *p, int len)
{
	int		ii;

	for (ii=0; ii<len; ii++) {
		if (*(p+ii) == 0x00)	*(p+ii)=' ';
	}
	*(p+len) = 0x00;

	return ;
}

__inline int getinput(char *istr, int sz)
{
	memset(istr, 0x00, sz);
	fgets(istr, sz, stdin);

	if (istr[0] == '\n')    istr[0] = 0;
	else if (istr[strlen(istr)-1] == '\n')  istr[strlen(istr)-1] = 0;

	return (strlen(istr));	
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		pos, idx=1, seq=0;
	int		rc;
	int		slen, rwno[2];
	char	sbuf[1024], rbuf[1024], stemp[32], msg[128];
	char	dstcd[1+1];

	if (argc < 6)
	{
		printf("\n  %s [sendercompid] [symbol] [dstcd] [ordtype] [timeinforce]\n", argv[0]);
		printf("      dstcd       : 1-TD, 2-TOM, 3-현물환, 4-선물환, 5-스왑, 6-바로환전, 7-NDF \n");
		printf("      ordtype     : 1-Market, 2-Limit \n");
		printf("      timeinforce : 0-Day, 3-IOC, 4-FOK \n");
		printf("  ex) %s TTESTT-TRD01 USDJPY 3 2 3 \n\n\n", argv[0]);
		exit(0);
	}

	signal(SIGINT, (void *)exitproc);

	memset(sbuf, 0x00, sizeof(sbuf));
	memset(rbuf, 0x00, sizeof(rbuf));
	memset(tbuf, 0x00, sizeof(tbuf));

	memset(&wgval, 0x00, sizeof(wgval));

	l_db2connect();

	memset(v_sendcompid,		0x00, sizeof(v_sendcompid));
	memset(v_trsmt_taget_id,	0x00, sizeof(v_trsmt_taget_id));
	memset(v_trsmt_taget_sub_id,0x00, sizeof(v_trsmt_taget_sub_id));
	memset(v_telgm_dmnd_dstic,	0x00, sizeof(v_telgm_dmnd_dstic));
	memset(v_recv_taget_id, 	0x00, sizeof(v_recv_taget_id));
	memset(c_fcm_acno,			0x00, sizeof(c_fcm_acno));
	memset(c_fx_prdct_cd,		0x00, sizeof(c_fx_prdct_cd));
	memset(c_tnr_cd,		    0x00, sizeof(c_tnr_cd));
	memset(c_spot_ymd,		    0x00, sizeof(c_spot_ymd));
	memset(c_expir_ymd,		    0x00, sizeof(c_expir_ymd));

	// sendercompid
	strcpy(v_sendcompid, argv[1]);
	l_rtrim(v_sendcompid);

	// symbol
	strcpy(c_fx_prdct_cd, argv[2]);
	l_rtrim(c_fx_prdct_cd);
	
	// 1-TD, 2-TOM, 3-현물환, 4-선물환, 5-스왑, 6-바로환전, 7-NDF
	strcpy(c_tnr_cd, argv[3]);
	
	memset(o_ot, 0x00, sizeof(o_ot));
	memset(o_tf, 0x00, sizeof(o_tf));
	o_ot[0] = argv[4][0];
	o_tf[0] = argv[5][0];

//////////////////////////////////////////////////////////

/*
EXEC SQL
	SELECT	TRSMT_TAGET_ID, TELGM_DMND_DSTIC, FCM_ACNO, RECV_TAGET_ID
	  INTO	:v_trsmt_taget_id, :v_telgm_dmnd_dstic, :c_fcm_acno, :v_recv_taget_id
	  FROM	INST1.TSKEIAM87
	 WHERE	TRSMT_TAGET_ID = :v_sendcompid
	;
*/

{
#line 168 "wqcliapi.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 168 "wqcliapi.sqc"
  sqlaaloc(2,1,1,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 51;
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)v_sendcompid;
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 168 "wqcliapi.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 168 "wqcliapi.sqc"
  sqlaaloc(3,4,2,0L);
    {
      struct sqla_setdata_list sql_setdlist[4];
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 51;
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)v_trsmt_taget_id;
#line 168 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 168 "wqcliapi.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 2;
#line 168 "wqcliapi.sqc"
      sql_setdlist[1].sqldata = (void*)v_telgm_dmnd_dstic;
#line 168 "wqcliapi.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 168 "wqcliapi.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 21;
#line 168 "wqcliapi.sqc"
      sql_setdlist[2].sqldata = (void*)c_fcm_acno;
#line 168 "wqcliapi.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 168 "wqcliapi.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 51;
#line 168 "wqcliapi.sqc"
      sql_setdlist[3].sqldata = (void*)v_recv_taget_id;
#line 168 "wqcliapi.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 168 "wqcliapi.sqc"
      sqlasetdata(3,0,4,sql_setdlist,0L,0L);
    }
#line 168 "wqcliapi.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 168 "wqcliapi.sqc"
  sqlastop(0L);
}

#line 168 "wqcliapi.sqc"

	if (SQLCODE != 0)
	{
		printf("[%d:%s] select error.. (%s)\n", SQLCODE, SQLERRM, v_sendcompid);
		exitproc(0);
	}
	l_rtrim(v_trsmt_taget_id);
	l_rtrim(v_trsmt_taget_sub_id);
	l_rtrim(v_telgm_dmnd_dstic);
	l_rtrim(c_fcm_acno);
	l_rtrim(v_recv_taget_id);

	if (v_telgm_dmnd_dstic[0] == DF_CLITP_FXG) {
		strcpy(v_trsmt_taget_sub_id, v_trsmt_taget_id);
		strcpy(v_trsmt_taget_id, "BLP_RFS_BETA");
	}

	time_t	tt;
	struct tm *tm;

	tt = time(NULL);
	tm = localtime(&tt);

	memset(&wgval.wgkey, ' ', sizeof(WGKEY));
	memcpy(wgval.wgkey.targetcompid, v_recv_taget_id, strlen(v_recv_taget_id));
	memcpy(wgval.wgkey.sendcompid, v_trsmt_taget_id, strlen(v_trsmt_taget_id));
	memcpy(wgval.wgkey.sendsubid, v_trsmt_taget_sub_id, strlen(v_trsmt_taget_sub_id));
	sprintf(wgval.wgkey.reqID, "T_%02d.%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

//	printf("WGKEY [%s:%s:%s:%s]\n", wgval.wgkey.targetcompid, wgval.wgkey.sendcompid, wgval.wgkey.sendsubid, wgval.wgkey.reqID);


/*
EXEC SQL
	SELECT	CASE WHEN LENGTH(TRIM(EXPIR_YMD)) = 0 THEN TO_CHAR(SYSDATE, 'YYYYMMDD') ELSE TRIM(EXPIR_YMD) END
	  INTO	:c_expir_ymd
	  FROM	INST1.TSKEIMU55
	 WHERE	FX_PRDCT_CD = :c_fx_prdct_cd
	   AND	TNR_CD = DECODE(:c_tnr_cd, '1','00',  '2','01',  '3','02', '4','10', '5','10', '6','02', 'N/A') -- // 1-TD, 2-TOM, 3-현물환, 4-선물환, 5-스왑, 6-바로환전, 7-NDF
	ORDER BY REGI_YMD DESC, NTHNO DESC
	FETCH FIRST 1 ROW ONLY
	;
*/

{
#line 207 "wqcliapi.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 207 "wqcliapi.sqc"
  sqlaaloc(2,2,3,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)c_fx_prdct_cd;
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 207 "wqcliapi.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 2;
#line 207 "wqcliapi.sqc"
      sql_setdlist[1].sqldata = (void*)c_tnr_cd;
#line 207 "wqcliapi.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 207 "wqcliapi.sqc"
      sqlasetdata(2,0,2,sql_setdlist,0L,0L);
    }
#line 207 "wqcliapi.sqc"
  sqlaaloc(3,1,4,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)c_expir_ymd;
#line 207 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 207 "wqcliapi.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 207 "wqcliapi.sqc"
  sqlacall((unsigned short)24,2,2,3,0L);
#line 207 "wqcliapi.sqc"
  sqlastop(0L);
}

#line 207 "wqcliapi.sqc"

	if (SQLCODE != 0 || IsEmpty(c_expir_ymd[0]))
	{
		printf("[%d:%s] select c_expir_ymd error.. \n", SQLCODE, SQLERRM);
		exitproc(0);
	}


/*
EXEC SQL
	SELECT	EXPIR_YMD
	  INTO	:c_spot_ymd
	  FROM	INST1.TSKEIMU55
	 WHERE	FX_PRDCT_CD = :c_fx_prdct_cd
	   AND	TNR_CD = '02' -- // 1-TD, 2-TOM, 3-현물환, 4-선물환, 5-스왑, 6-바로환전, 7-NDF
	ORDER BY REGI_YMD DESC, NTHNO DESC
	FETCH FIRST 1 ROW ONLY
	;
*/

{
#line 222 "wqcliapi.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 222 "wqcliapi.sqc"
  sqlaaloc(2,1,5,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 7;
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)c_fx_prdct_cd;
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 222 "wqcliapi.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 222 "wqcliapi.sqc"
  sqlaaloc(3,1,6,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqldata = (void*)c_spot_ymd;
#line 222 "wqcliapi.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 222 "wqcliapi.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 222 "wqcliapi.sqc"
  sqlacall((unsigned short)24,3,2,3,0L);
#line 222 "wqcliapi.sqc"
  sqlastop(0L);
}

#line 222 "wqcliapi.sqc"

	if (SQLCODE != 0 || IsEmpty(c_spot_ymd[0]))
	{
		printf("[%d:%s] select c_spot_ymd error.. \n", SQLCODE, SQLERRM);
		exitproc(0);
	}
////////////////////////////
// WRITE DQ init

	AG_DQUECLS(&qin);

	qin.xha |= AQ_XHA_OPN;
	qin.xha |= AG_XAS_LIV;

////////////////////////////
// READ DQ seq init
	int		rtn;

	pthread_t	tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	// QUOT RECEIVE THREAD START !!!...................
	pthread_create(&tid, &attr, handler_quot, (void *)&idx);
	sleep(1);

////////////////////////////
	// SEND QUOTE REQUEST
	if ((rc=sendquotreq()) < 0) {
		printf("setinput error : %d\n", rc);
		exit(0);
	}
	printf("\nsend quote request ok..!!!! [%d]\n", rc);

	while (1) {
		if (wgval.useyn[0] == '9')	{
			printf("\nREQUEST IS DONE >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
			sleep(1);
			break;
		}

		if (wgval.quot.quotID[0] == 0x00 || wgval.quot.quotID[0] == ' ') {sleep(1);		continue;}
		printf("\n\nEnter to order..");
		fflush(stdout);
		getchar();

		if ((rc=sendorder(sbuf)) < 0)
		{
			printf("sendorder error : %d\n", rc);
			break;
		}
	}

	exitproc(0);
}

int sendquotreq()
{
	int		rc, dsiz=1024, slen;
	char	sbuf[1024], stemp[128];
	WG_QUOT_REQ *p;

	memset(sbuf, 0x00, sizeof(sbuf));

	WGCOM	*c=&sbuf;
	WGKEY	*k=&c->wgkey;
	p = (WG_QUOT_REQ *)c->data;

// Quote 요청/해제 구조체
	c->msgtp[0] = MSG_QTREQ;
	memcpy(&c->wgkey, &wgval.wgkey, sizeof(WGKEY));

/*
char	tag35		[  2];		// 35(MsgType) // (R = QuoteRequest, Z = QuoteCancel, V = MarketDataRequest)
char	subtp		[  1];		// 1:subscribe,  2:unsubscribe
char	account		[ 20];		// Dealer ID
char	stype		[  1];		// 167(SecurityType) // (S:SPOT,F:FWD,W:SWAP,I:바로환전)
char	symb		[  6];		// PK // WGKEY + SYMBOL
char	setldate	[  8];		// 64(FutSettDate) // 결제일(YYYYMMDD) (swap 인 경우 near)
char	setldate2	[  8];		// 193(FutSettDate2) // SWAP 인 경우 far 결제일(YYYYMMDD)
char	side		[  1];		// 54(Side) // '0'=2way, '1'=buy, '2'=sell
char	orderqty	[ 16];		// 38(OrderQty) // RFS, RFQ 인 경우 필수
char	currency	[  3];		// 15(Currency)
char	echoTags	[128];		// echo tags to send back
*/

	if (strncmp(wgval.wgkey.sendcompid, "BLP_RFS_BETA", 12) == 0)	p->tag35[0] = 'R';
	else															p->tag35[0] = 'V';

	p->subtp[0] = '1';
	memcpy(p->account, c_fcm_acno, strlen(c_fcm_acno));

	// 1-TD, 2-TOM, 3-현물환, 4-선물환, 5-스왑, 6-바로환전, 7-NDF
	if (c_tnr_cd[0] <= '3')			p->stype[0] = 'S';
	else if (c_tnr_cd[0] == '5')	p->stype[0] = 'W';
	else if (c_tnr_cd[0] == '6')	p->stype[0] = 'I';
	else							p->stype[0] = 'F';

	memcpy(p->symb, c_fx_prdct_cd, sizeof(p->symb));

	if (c_tnr_cd[0] == '5') {
		printf("near (%.8s) : ", c_spot_ymd);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->setldate, stemp, sizeof(p->setldate));
		else									memcpy(p->setldate, c_spot_ymd, sizeof(p->setldate));

		printf("far  (%.8s) : ", c_expir_ymd);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->setldate2, stemp, sizeof(p->setldate2));
		else									memcpy(p->setldate2, c_expir_ymd, sizeof(p->setldate2));
	}
	else
	{
		printf("setledate (%.8s) : ", c_expir_ymd);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->setldate, stemp, sizeof(p->setldate));
		else									memcpy(p->setldate, c_expir_ymd, sizeof(p->setldate));
	}

	p->side[0] = '1';

	printf("orderqty (100) : ");
	if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->orderqty, stemp, sizeof(p->orderqty));
	else									memcpy(p->orderqty, "100", 3);

	memcpy(p->currency, p->symb, sizeof(p->currency));

	slen = sizeof(WGCOM) + sizeof(WG_QUOT_REQ);
	// QUOTE REQUEST SEND
	_set_pad(sbuf, slen);
	rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, slen);
	if (rc < 0) {
		printf("agdque_wrn error : %d\n", rc);
		exit(0);
	}
	memcpy(&wgval.qreq, (char *)p, sizeof(WG_QUOT_REQ));
	wgval.useyn[0] = '1';
	
	memset(tbuf, 0x00, sizeof(tbuf));
	memcpy(tbuf, sbuf, sizeof(WGCOM)+sizeof(WG_QUOT_REQ));

	return 0;
}

int sendorder()
{
	int		rc, slen, dsiz=1024;
	char	sbuf[1024], stemp[128];
	time_t	tt;
	struct tm *tm;

	memset(sbuf, 0x00, sizeof(sbuf));

	WGCOM	*c=&sbuf;
	WGKEY	*k=&c->wgkey;
	WG_ORD	*p=c->data;

	tt = time(NULL);
	tm = localtime(&tt);

	c->msgtp[0] = MSG_ORDER;
	memcpy(&c->wgkey, &wgval.wgkey, sizeof(WGKEY));
	p->tag35[0] = 'D';
/*
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
*/
	memcpy(p->quotID, wgval.quot.quotID, sizeof(p->quotID));
	memcpy(p->account, wgval.qreq.account, sizeof(p->account));
	printf("account (%.*s) : ", sizeof(p->account), p->account);
	if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->account, stemp, strlen(stemp));

	sprintf(stemp, "%02d%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	memcpy(p->clordid, stemp, strlen(stemp));
	p->stype[0] = wgval.qreq.stype[0];
	memcpy(p->symb, wgval.qreq.symb, sizeof(p->symb));
//	memcpy(p->setldate, wgval.qreq.setldate, sizeof(p->setldate));

	printf("setldate (%.8s) : ", wgval.qreq.setldate);
	if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->setldate, stemp, sizeof(p->setldate));
	else									memcpy(p->setldate, wgval.qreq.setldate, sizeof(p->setldate));

	printf("compid (%.*s) : ", sizeof(k->sendcompid), k->sendcompid);
	if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(k->sendcompid, stemp, strlen(stemp));

	if (wgval.ordcnt%2 == 0) {
		p->side[0] = '1';		// BUY
		if (c_tnr_cd[0] == '5') {
			memcpy(p->price,  wgval.quot.bid,  sizeof(p->price));
			memcpy(p->price2, wgval.quot.ask2, sizeof(p->price2));
		} else
//memcpy(p->price, "1370.00", 7);
			memcpy(p->price,  wgval.quot.ask,  sizeof(p->price));
	} else {
		p->side[0] = '2';		// SEL
		if (c_tnr_cd[0] == '5') {
			memcpy(p->price,  wgval.quot.ask,  sizeof(p->price));
			memcpy(p->price2, wgval.quot.bid2, sizeof(p->price2));
		} else
//memcpy(p->price, "1390.00", 7);
			memcpy(p->price,  wgval.quot.bid,  sizeof(p->price));
	}

	memcpy(p->setldate2, wgval.quot.setldate2, sizeof(p->setldate2));

	memcpy(p->orderqty, wgval.qreq.orderqty, sizeof(p->orderqty));
	memcpy(p->currency, wgval.qreq.currency, sizeof(p->currency));
	p->ordtype[0]     = o_ot[0];	//API_ORDTP_MARKET;
	p->timeinforce[0] = o_tf[0];	//API_ORDTF_DAY;



/*
1. API-현물환(CNY/KRW)로  
 CNY기준 11,000 바이거래 
 
2. API-현물환(USD/CNY)로  
 USD기준 111 바이거래
*/
	   
	// 매매구분, 주문수량, 주문가격 수기 입력 (스왑제외)
	if (c_tnr_cd[0] != '5') {
		// BUY/SELL side input
		printf("side (%.*s) : ", sizeof(p->side), p->side);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->side, stemp, strlen(stemp));
		
		printf("기준통화currency (%.*s) : ", sizeof(p->currency), p->currency);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->currency, stemp, strlen(stemp));
						
		printf("orderqty (%.*s) : ", sizeof(p->orderqty), p->orderqty);
		if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->orderqty, stemp, strlen(stemp));
		
		// 지정가 주문인 경우 주문가격 수기 입력
		if (p->ordtype[0] == '2') {
			printf("price (%.*s) : ", sizeof(p->price), p->price);
			if (getinput(stemp, sizeof(stemp)) > 0)	memcpy(p->price, stemp, strlen(stemp));		
		}
	}

	// ORDER SEND
	slen = sizeof(WGCOM) + sizeof(WG_ORD);
	_set_pad(sbuf, slen);
	rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, slen);
	if (rc < 0)
		printf("agdque_wrn error : %d\n", rc);
	else
		printf("send order ok..!!!! [%d]\n", rc);

	wgval.ordcnt++;

	return 0;
}

void *handler_quot(void *argv)
{
	int		ii, rtn, dlen, idx, pos;
	int		rwno[2];
	char	rbuf[1024], *aphome;
	WG_ORD_EXC	*p;;
	pthread_t	tid;

	tid = pthread_self();
	pthread_detach(tid);

	idx = *((int *)argv);		// 주문 응답 수신처리와 분리하기 위해 idx +1 한다.
	
	AG_DQUE		qsise;
	
	WGCOM		*rc=rbuf;

	WG_QUOT		*q;
	WG_QUOT_RES	*r;
	WG_ORD_EXC	*e;

	memset(&qsise, 0x00, sizeof(qsise));
	AG_DQUECLS(&qsise);

	rtn = agdque_at (&qsise, WG_DAT_DIR, NULL, WGDQ_SNDRS, WGSEM_SNDRS, sizeof(rbuf));
	if (rtn < 0)
	{
		printf("\n agdque_at error.. [%d]\n", rtn);
		pthread_exit(NULL);
	}

	memset(&rwno, 0x00, sizeof(rwno));

	rwno[0] = qsise.hdr.wno;
	rtn = agdque_rwi_io (&qsise, idx, &rwno);
	if (rtn < 0)
	{
		printf("\n agdque_rwi_io error.. [%d]\n", rtn);
		pthread_exit(NULL);
	}
	printf("\n set aid[%d] read no [%d -> %d]\n", idx, qsise.hdr.rno[idx], qsise.hdr.wno);

	qsise.xha |= AQ_XHA_OPN;
	qsise.xha |= AG_XAS_LIV;

printf("(%s) start to wait for data ...(idx=%d)\n", __func__, idx);

	// 시세 수신
	while (1)
	{
		rtn = agdque_rdn(&qsise, WG_DAT_DIR, NULL, WGDQ_SNDRS, WGSEM_SNDRS, rbuf, sizeof(rbuf), idx, 100, &rwno);

		if (rtn == 0)	continue;
		else if (rtn < 0)	{
			printf("\n agdque_rdn error : %d\n", rtn);
			break;
		}

//printf("(%d) tp(%.1s) key (%.32s/%.32s/%.32s/%.6s)\n", rwno[0], rc->msgtp, rc->wgkey.targetcompid, rc->wgkey.sendcompid, rc->wgkey.sendsubid, rc->wgkey.keysymb);

		switch (rc->msgtp[0]) {

		case MSG_ORDEXC :
			e = (WG_ORD_EXC *)rc->data;

//			if (memcmp(&rc->wgkey, &wgval.wgkey, sizeof(WGKEY)) != 0)		continue;

			if (e->rejtext[0] != ' ')
				printf("\n (%d) RECV REJ [%.24s] stat[%s:%s] rej[%.8s:%s]\n", rwno[0], e->clordid, STR_ORDST(e->ordstatus[0]), STR_EXCST(e->exectype[0]), e->rejcd, e->rejtext);
			else
				printf("\n (%d) RECV [%.24s] exc[%.10s:%.16s:%.16s] stat[%s:%s] rej[%.8s:%.50s]\n", rwno[0], e->ordid, e->lastqty, e->execid, e->lastpx, STR_ORDST(e->ordstatus[0]), STR_EXCST(e->exectype[0]), e->rejcd, e->rejtext);

//			wgval.useyn[0] = '9';
//			pthread_exit(NULL);

			break;

		case MSG_QUOTE :
			q = (WG_QUOT *)rc->data;

//printf("(%d) Q [%.30s] [%.6s] [%.16s:%.16s] \n", rwno[0], q->quotID, q->symb, q->bid, q->ask);
//			if (memcmp(&rc->wgkey, &wgval.wgkey, sizeof(WGKEY)) != 0)		continue;

			memcpy(&wgval.quot, (char *)q, sizeof(WG_QUOT));

			break;

		case MSG_QTRES :
			r = (WG_QUOT_RES *)rc->data;

			printf("\n (%d) RECV : close request.. [%.8s:%.*s]\n", rwno[0], r->rejcd, sizeof(r->rejtext), r->rejtext);
			printf("\n thread exit ...........\n");

			wgval.useyn[0] = '9';
			pthread_exit(NULL);
			break;

		default :
			break;
		}
	}

	printf("\n receive quote done...........\n");

	pthread_exit(NULL);
}
