static char sqla_program_id[292] = 
{
 '\xad','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x67','\x41','\x6d','\x58','\x52','\x4e','\x49','\x6f','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x9','\x0','\x57','\x51','\x43','\x4c','\x49','\x45','\x4e','\x54','\x33','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
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


#line 1 "wqclient3.sqc"
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


#line 6 "wqclient3.sqc"


char	tbuf[1024];
char	obuf[1024];			// send order temp buffer
char	compid[32];
AG_DQUE		qin, qsise;

WGCOM		 *tc;
WG_QUOT_REQ  *t;

struct _WGVAL {
	char			useyn[1];
	WGKEY			wgkey;
	WG_QUOT_REQ		qreq;
	WG_QUOT			quot;
	MDFOLD			*fold;
} wgval;


/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 24 "wqclient3.sqc"

	char	v_trsmt_taget_id	[50+1];
	char	v_trsmt_taget_sub_id[50+1];
	char	v_recv_taget_id		[50+1];
	char	c_fcm_acno			[20+1];

/*
EXEC SQL END DECLARE SECTION;
*/

#line 29 "wqclient3.sqc"


int sendquotreq();
int sendorder();
void *handler_quot(void *argv);
void _set_pad(char *p, int len);

void exitproc(int signo)
{
	int		rc, slen;
	char	sbuf[1024];
	WGCOM		*c=sbuf;
	WG_QUOT_RES *p=c->data;

	memset(sbuf, 0x00, sizeof(sbuf));

	switch (signo)
	{
	case SIGINT : // RFS 해제 요청	
		if (wgval.useyn[0] == '0')		break;

		memset(sbuf, 0x00, sizeof(sbuf));
		memcpy(sbuf, tbuf, sizeof(sbuf));
		c->msgtp[0] = 'S';
		p->tag35[0] = 'b';
		
		slen=sizeof(WGCOM)+sizeof(WG_QUOT_RES);
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
		if (*(p+ii) == 0x00)	*(p+ii)=0x20;
	}
	*(p+len) = 0x00;

	return ;
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

	if (argc < 4)	
	{
		printf("\n\t %s [compid symb qidx] \n", argv[0]);
		exit(0);
	}

	signal(SIGINT, (void *)exitproc);

	memset(sbuf, 0x00, sizeof(sbuf));
	memset(rbuf, 0x00, sizeof(rbuf));
	memset(tbuf, 0x00, sizeof(tbuf));

	memset(&wgval, 0x00, sizeof(wgval));

	l_db2connect();

	memset(v_trsmt_taget_id,	0x00, sizeof(v_trsmt_taget_id));
	memset(v_trsmt_taget_sub_id,0x00, sizeof(v_trsmt_taget_sub_id));
	memset(v_recv_taget_id, 	0x00, sizeof(v_recv_taget_id));
	memset(c_fcm_acno,			0x00, sizeof(c_fcm_acno));

	strcpy(v_trsmt_taget_id, argv[1]);
	idx = atoi(argv[2]);
	
	l_rtrim(v_trsmt_taget_id);


/*
EXEC SQL
	SELECT	FCM_ACNO, RECV_TAGET_ID
	  INTO	:c_fcm_acno, :v_recv_taget_id
	  FROM	INST1.TSKEIAM87
	 WHERE	TRSMT_TAGET_ID = :v_trsmt_taget_id
	;
*/

{
#line 125 "wqclient3.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 125 "wqclient3.sqc"
  sqlaaloc(2,1,1,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 51;
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqldata = (void*)v_trsmt_taget_id;
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 125 "wqclient3.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 125 "wqclient3.sqc"
  sqlaaloc(3,2,2,0L);
    {
      struct sqla_setdata_list sql_setdlist[2];
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 21;
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqldata = (void*)c_fcm_acno;
#line 125 "wqclient3.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 125 "wqclient3.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 51;
#line 125 "wqclient3.sqc"
      sql_setdlist[1].sqldata = (void*)v_recv_taget_id;
#line 125 "wqclient3.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 125 "wqclient3.sqc"
      sqlasetdata(3,0,2,sql_setdlist,0L,0L);
    }
#line 125 "wqclient3.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 125 "wqclient3.sqc"
  sqlastop(0L);
}

#line 125 "wqclient3.sqc"

	if (SQLCODE != 0)
	{
		printf("[%d:%s] select error.. \n", SQLCODE, SQLERRM);
		exitproc(0);
	}
	l_rtrim(c_fcm_acno);
	l_rtrim(v_recv_taget_id);

	memset(&wgval.wgkey, ' ', sizeof(WGKEY));
	memcpy(wgval.wgkey.targetcompid, v_recv_taget_id, strlen(v_recv_taget_id));
	memcpy(wgval.wgkey.sendcompid, v_trsmt_taget_id, strlen(v_trsmt_taget_id));
//	wgval.wgkey.sendsubid
	memcpy(wgval.wgkey.keysymb, argv[2], strlen(argv[2]));

	printf("WGKEY [%s:%s:%s]\n", v_recv_taget_id, v_trsmt_taget_id, argv[2]);
	
	if ((wgval.fold = mds_getfold("CUST", wgval.wgkey.keysymb, msg)) == NULL)
	{
		printf("mds_getfold error.. [%s]\n", msg);
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
	printf("set aid[%d] read no [%d -> %d]\n", idx, qsise.hdr.rno[idx], qsise.hdr.wno);
	qsise.xha |= AQ_XHA_OPN;
	qsise.xha |= AG_XAS_LIV;

////////////////////////////
	// SEND QUOTE REQUEST
	if ((rc=sendquotreq()) < 0) {
		printf("setinput error : %d\n", rc);
		exit(0);
	}

	printf("\nsend quote request ok..!!!! [%d]\n", rc);

	pthread_t	tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	// QUOT RECEIVE THREAD START !!!...................
	pthread_create(&tid, &attr, handler_quot, (void *)&idx);
	usleep(100000);

	while (1) {
		if (wgval.quot.quotID[0] == 0x00 || wgval.quot.quotID[0] == ' ') {sleep(1);		continue;}
		printf("Enter to order..");
		if ((rc=sendorder(sbuf)) < 0)
		{
			printf("sendorder error : %d\n", rc);
			break;
		}
		getchar();
	}

	exitproc(0);
}

int sendquotreq()
{
	int		rc, dsiz=1024, slen;
	char	sbuf[1024], stemp[128];
	time_t	tt;
	struct tm *tm;
	WG_QUOT_REQ *p;
	
	WGCOM	*c=&sbuf;
	WGKEY	*k=&c->wgkey;
	p = (WG_QUOT_REQ *)c->data;

	tt = time(NULL);
	tm = localtime(&tt);

// Quote 요청/해제 구조체
	c->msgtp[0] = MSG_QTREQ;
	memcpy(&c->wgkey, &wgval.wgkey, sizeof(WGKEY));

/*
char	tag35		[  2];		// 35(MsgType) // (R = QuoteRequest, Z = QuoteCancel, V = MarketDataRequest)
char	subtp		[  1];		// 1:subscribe,  2:unsubscribe
char	reqID		[ 64];		// 131(QuoteReqID) // API:내부생성, RFS/RFQ:Tag117=QuoteID (Tag131=QuoteReqID, Tag262=MDReqID)
char	account		[ 20];		// Dealer ID
char	stype		[  1];		// 167(SecurityType) // (S:FXSPOT,F:FXFWD,W:FXSWAP)
char	symb		[  6];		// PK // WGKEY + SYMBOL
char	setldate	[  8];		// 64(FutSettDate) // 결제일(YYYYMMDD) (swap 인 경우 near)
char	setldate2	[  8];		// 193(FutSettDate2) // SWAP 인 경우 far 결제일(YYYYMMDD)
char	side		[  1];		// 54(Side) // '0'=2way, '1'=buy, '2'=sell
char	orderqty	[ 16];		// 38(OrderQty) // RFS, RFQ 인 경우 필수
char	currency	[  3];		// 15(Currency)
char	echoTags	[128];		// echo tags to send back
*/
	p->tag35[0] = 'V';
	p->subtp[0] = '1';
	sprintf(p->reqID, "T_%02d.%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	memcpy(p->account, c_fcm_acno, strlen(c_fcm_acno));
	p->stype[0] = 'S';
	memcpy(p->symb, wgval.wgkey.keysymb, sizeof(p->symb));
	memcpy(p->setldate, wgval.fold->spotdate, sizeof(p->setldate));
	p->side[0] = '1';
	memcpy(p->orderqty, "100", 3);
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

	return 0;
}

int sendorder()
{
	int		rc, slen, dsiz=1024;
	char	sbuf[1024], stemp[128];
	time_t	tt;
	struct tm *tm;

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
	
	sprintf(stemp, "%02d%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	memcpy(p->clordid, stemp, strlen(stemp));
	p->stype[0] = wgval.qreq.stype[0];
	memcpy(p->symb, wgval.qreq.symb, sizeof(p->symb));
	memcpy(p->setldate, wgval.qreq.setldate, sizeof(p->setldate));
	memcpy(p->price, wgval.quot.ask, sizeof(p->price));
	p->side[0] = wgval.qreq.side[0];
	memcpy(p->orderqty, wgval.qreq.orderqty, sizeof(p->orderqty));
	memcpy(p->currency, wgval.qreq.currency, sizeof(p->currency));
	p->ordtype[0] = '1';
	p->timeinforce[0] = '0';

	// ORDER SEND
	slen = sizeof(WGCOM) + sizeof(WG_ORD);
	_set_pad(sbuf, slen);
	rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, slen);
	if (rc < 0)
		printf("agdque_wrn error : %d\n", rc);
	else
		printf("send order ok..!!!! [%d]\n", rc);

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

	WGCOM		*rc=rbuf;

	WG_QUOT		*q;
	WG_QUOT_RES	*r;
	WG_ORD_EXC	*e;

//	printf("wait for data ...\n");

	// 시세 수신
	while (1) {

		rtn = agdque_rdn (&qsise, WG_DAT_DIR, NULL, WGDQ_SNDRS, WGSEM_SNDRS, rbuf, sizeof(rbuf), idx, (-1), &rwno);
		if (rtn < 0)	{
			printf("agdque_rdn error : %d\n", rtn);
			break;
		}

		switch (rbuf[0]) {

		case MSG_ORDEXC :
			e = (WG_ORD_EXC  *)rc->data;

			if (memcmp(&rc->wgkey, &wgval.wgkey, sizeof(WGKEY)) != 0)		continue;

			if (str2i(e->rejcd, sizeof(e->rejcd)) != 0)
			{
				printf("(%d) RECV REJ [%.24s] stat[%.1s:%.1s] rej[%.8s:%s]\n", rwno[0], e->clordid, e->ordstatus, e->exectype, e->rejcd, e->rejtext);
				wgval.useyn[0] = '0';
				printf("thread exit ...........\n");
				pthread_exit(NULL);
			}
			else printf("(%d) RECV [%.24s] exc[%.10s:%.16s:%.16s] stat[%.1s:%.1s] rej[%.8s:%s]\n", rwno[0], e->ordid, e->lastqty, e->execid, e->lastpx, e->ordstatus, e->exectype, e->rejcd, e->rejtext);
			
			break;

		case MSG_QUOTE :
			q = (WG_QUOT     *)rc->data;

			if (memcmp(&rc->wgkey, &wgval.wgkey, sizeof(WGKEY)) != 0)		continue;

			memcpy(&wgval.quot, (char *)q, sizeof(WG_QUOT));

			break;

		case MSG_QTRES :
			r = (WG_QUOT_RES *)rc->data;

			printf("(%d) RECV : close request.. [%.8s:%.*s]\n", rwno[0], r->rejcd, sizeof(r->rejtext), r->rejtext);
			wgval.useyn[0] = '0';
			printf("thread exit ...........\n");
			pthread_exit(NULL);
			break;

		default :
			break;
		}
	}

	printf("receive quote done...........\n");

	pthread_exit(NULL);
}
