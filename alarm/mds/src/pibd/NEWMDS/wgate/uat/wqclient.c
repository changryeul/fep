#include "mds2.h"
#include "wfaapi.h"
#include "wgate.h"

char	tbuf[1024];
char	compid[32];
char	obuf[1024];			// send order temp buffer
AG_DQUE		qin, qsise;

WGCOM		 *tc;
WG_QUOT_REQ  *t;

struct _WGCLI {
	int		seq		;
	char	useyn	[  1];
	char	reqID	[ 64];		// API:내부생성, RFS/RFQ:Tag117=QuoteID (Tag131=QuoteReqID, Tag262=MDReqID)
	char	quotID	[ 32];		// 발송한 last QuoteID (Tag117=QuoteID)
	char	bid		[ 16];		// 고객제공환율
	char	ask		[ 16];
} wgcli;

int setquotreq(char *pbuf);
int setorder(char *sbuf);
void *handler_quot(void *argv);
void _set_pad(char *p, int len);

void exitproc(int signo)
{
	int		rc, dlen;
	char	sbuf[1024];
	WGCOM		*c=sbuf;
	WG_QUOT_RES *p=c->data;

	memset(sbuf, 0x00, sizeof(sbuf));
	
	if (wgcli.useyn[0] == '0')	exit(0);

	switch (signo)
	{
	case SIGINT : // RFS 해제 요청	
		memset(sbuf, 0x00, sizeof(sbuf));
		memcpy(sbuf, tbuf, sizeof(sbuf));
//		memset(c->wgkey.sendsubid, ' ', sizeof(c->wgkey.sendsubid));
//		memset(c->wgkey.keysymb,   ' ', sizeof(c->wgkey.keysymb));
		c->msgtp[0] = 'S';
		p->tag35[0] = 'b';
		
		dlen=sizeof(WGCOM)+sizeof(WG_QUOT_RES);
		_set_pad(sbuf, dlen);
		rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, 1024, sbuf, dlen);
		if (rc < 0) {
			printf("agdque_wrn error : %d\n", rc);
			exit(0);
		}
		printf("\nsend unsubscribe packet ok..!!!! [%d]\n", rc);
		
		exit(0);

		break;

	default : 
		break;
	}

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
	char	sbuf[1024], rbuf[1024], stemp[32];

	pthread_t	tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	if (argc < 3)	
	{
		printf("\n\t %s [compid qidx] \n", argv[0]);
		exit(0);
	}

	strcpy(compid, argv[1]);
	idx = atoi(argv[2]);

	signal(SIGINT, (void *)exitproc);

	memset(sbuf, 0x00, sizeof(sbuf));
	memset(rbuf, 0x00, sizeof(rbuf));
	memset(tbuf, 0x00, sizeof(tbuf));

	memset(&wgcli, 0x00, sizeof(wgcli));

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

	int		dsiz = 1024;

	tc = tbuf;
	t  = (WG_QUOT_REQ *)tc->data;

	if ((rc=setquotreq(sbuf)) < 0) {
		printf("setinput error : %d\n", rc);
		exit(0);
	}

	slen = sizeof(WGCOM) + sizeof(WG_QUOT_REQ);
	// QUOTE REQUEST SEND
	_set_pad(sbuf, slen);
	rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, slen);
	if (rc < 0) {
		printf("agdque_wrn error : %d\n", rc);
		exit(0);
	}
	wgcli.useyn[0] = '1';
	memcpy(tbuf, sbuf, sizeof(tbuf));

	printf("\nsend quote request ok..!!!! [%d]\n", rc);

	// QUOT RECEIVE THREAD START !!!...................
	pthread_create(&tid, &attr, handler_quot, (void *)&idx);
sleep(1);

printf("wait for quot..\n");
while (1) {
	if (wgcli.useyn[0] == '0')	exitproc(0);
	if (strlen(wgcli.quotID) > 0)	break;
	usleep(200000);
}
//////////////////////////////////////////////////// SEND ORDER
	memset(sbuf, 0x00, sizeof(sbuf));
	if ((rc=setorder(sbuf)) < 0)
	{
		printf("setorder error : %d\n", rc);
		exit(0);
	}

	// ORDER SEND
	slen = sizeof(WGCOM) + sizeof(WG_ORD);
	_set_pad(sbuf, slen);
	rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, slen);
	if (rc < 0) {
		printf("agdque_wrn error : %d\n", rc);
		exit(0);
	}
	wgcli.useyn[0] = '1';

	printf("\nsend order ok..!!!! [%d]\n", rc);

	// WAIT FOR ORDER EXECUTION REPORT
	while (1) {
		if (wgcli.useyn[0] == '0')	break;

		sleep(1);

		if (seq == wgcli.seq)	continue;
		seq = wgcli.seq;

		printf("curr bid/ask [%f : %f] [%s]\n", wgcli.bid, wgcli.ask, wgcli.quotID);
	}

	exitproc(0);
}

void *handler_quot(void *argv)
{
	int		ii, rtn, dlen, idx, pos;
	int		rwno[2];
	char	rbuf[1024], sbuf[1024], *aphome;
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

//			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, WG_KEYLEN) != 0)	break;
//			if (memcmp(e->symb, t->symb, sizeof(e->symb)) != 0)	break;
			
			if (str2i(e->rejcd, sizeof(e->rejcd)) != 0)
			{
				printf("(%d) RECV REJ [%.24s] stat[%.1s:%.1s] rej[%.8s:%s]\n", rwno[0], e->clordid, e->ordstatus, e->exectype, e->rejcd, e->rejtext);
				wgcli.useyn[0] = '0';
				printf("thread exit ...........\n");
				pthread_exit(NULL);
			}
			else printf("(%d) RECV [%.24s] exc[%.10s:%.16s:%.16s] stat[%.1s:%.1s] rej[%.8s:%s]\n", rwno[0], e->ordid, e->lastqty, e->execid, e->lastpx, e->ordstatus, e->exectype, e->rejcd, e->rejtext);
			
			break;

		case MSG_QUOTE :
			q = (WG_QUOT     *)rc->data;

			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, WG_KEYLEN) != 0)	break;
			if (memcmp(q->symb, t->symb, sizeof(q->symb)) != 0)	break;
			if (memcmp(q->reqID, t->reqID, sizeof(q->reqID)) != 0)	break;
/*
			printf("[%.*s] rcv=[%.*s/%.*s/%.*s/%.*s/%.*s] [%.*s/%.*s/%.*s/%.*s]\n", sizeof(q->quotID), q->quotID,
				sizeof(q->msgtp) ,q->msgtp, sizeof(q->sendcompid) ,q->sendcompid, 
				sizeof(q->sendsubid) ,q->sendsubid, sizeof(q->stype) ,q->stype, sizeof(q->symb) ,q->symb, 
				sizeof(q->bid), q->bid, sizeof(q->ask), q->ask, sizeof(q->bid2), q->bid2, sizeof(q->ask2), q->ask2);
*/
			//wgcli.bid = str2d(q->bid, sizeof(q->bid));
			//wgcli.ask = str2d(q->ask, sizeof(q->ask));
			memcpy(wgcli.bid, q->bid, sizeof(q->bid));
			memcpy(wgcli.ask, q->ask, sizeof(q->ask));
			memcpy(wgcli.quotID, q->quotID, sizeof(q->quotID));
			(wgcli.seq)++;
			break;

		case MSG_QTRES :
			r = (WG_QUOT_RES *)rc->data;

			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, WG_KEYLEN) != 0)	break;
			if (memcmp(r->reqID, t->reqID, sizeof(r->reqID)) != 0)	break;

			printf("(%d) RECV : close request.. [%.8s:%.*s]\n", rwno[0], r->rejcd, sizeof(r->rejtext), r->rejtext);
			wgcli.useyn[0] = '0';
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

int setquotreq(char *pbuf)
{
	char	stemp[128];
	time_t	tt;
	struct tm *tm;
	WG_QUOT_REQ *p;
	
	WGCOM	*c=pbuf;
	WGKEY	*k=&c->wgkey;
	p = (WG_QUOT_REQ *)c->data;

	tt = time(NULL);
	tm = localtime(&tt);

// Quote 요청/해제 구조체
	c->msgtp[0] = MSG_QTREQ;
	p->tag35[0] = 'R';
	p->reqtp[0] = REQTP_RFS;

	sprintf(p->reqID, "T_%02d.%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	strcpy(k->sendcompid, "BLP_RFS_BETA");
	strcpy(k->sendsubid, compid);
	strcpy(k->targetcompid, "KBET_RFS_BETA");

	printf("\n========= SET REQ ==========\n");
/*
	printf("reqtp ('1'=ABT(차익) '2'=API '3'=RFS '4'=RFQ) : ");
	scanf("%s", stemp);
	if (stemp[0] != '1' && stemp[0] != '2' && stemp[0] != '3' && stemp[0] != '4')
	{ printf("\tinput error..\n"); return (-1); }
	p->reqtp[0] = stemp[0];
	if (p->reqtp[0] == '1' || p->reqtp[0] == '2')	p->tag35[0] = 'V';
	else											p->tag35[0] = 'R';
*/
	printf("stype (S:FXSPOT,F:FXFWD,W:FXSWAP) : ");
	scanf("%s", stemp);
	if (stemp[0] != 'S' && stemp[0] != 'F' && stemp[0] != 'W')
	{ printf("\tinput error..\n"); return (-2); }
	p->stype[0] = stemp[0];

	printf("symb : ");
	scanf("%s", stemp);
	if (strlen(stemp) != sizeof(p->symb))
	{ printf("\tinput error..\n"); return (-3); }
	memcpy(p->symb, stemp, sizeof(p->symb));
	memcpy(p->currency, p->symb, 3);
memcpy(k->keysymb, p->symb, sizeof(k->keysymb));

	printf("setldate : ");
	scanf("%s", stemp);
	if (strlen(stemp) != sizeof(p->setldate))
	{ printf("\tinput error..\n"); return (-4); }
	memcpy(p->setldate, stemp, sizeof(p->setldate));

	if (p->stype[0] == 'W')
	{
		printf("setldate for far : ");
		scanf("%s", stemp);
		if (strlen(stemp) != sizeof(p->setldate2))
		{ printf("\tinput error..\n"); return (-5); }
		memcpy(p->setldate2, stemp, sizeof(p->setldate2));
	}

	printf("side ('0'=2way,'1'=buy,'2'=sell) : ");
	scanf("%s", stemp);
	if (stemp[0] != '0' && stemp[0] != '1' && stemp[0] != '2')
	{ printf("\tinput error..\n"); return (-6); }
	p->side[0] = stemp[0];

	printf("qty : ");
	scanf("%s", stemp);
	memcpy(p->orderqty, stemp, strlen(stemp));

memcpy(wgcli.reqID, p->reqID, sizeof(p->reqID));
wgcli.useyn[0] = '1';

	printf("make input packet done..\n");

	return 0;
}

int setorder(char *pbuf)
{
	char	stemp[128];
	time_t	tt;
	struct tm *tm;

	WGCOM	*c=pbuf;
	WGKEY	*k=&c->wgkey;
	WG_ORD	*p=c->data;

	tt = time(NULL);
	tm = localtime(&tt);

	memcpy(pbuf, tbuf, sizeof(WGCOM));

	c->msgtp[0] = MSG_ORDER;
/*
	strcpy(k->sendcompid, "BLP_RFS_BETA");
	strcpy(k->sendsubid, compid);
	strcpy(k->targetcompid, "KBET_RFS_BETA");
*/
	printf("\n========= SET ORD ==========\n");
	printf("quotID [%s]  bid:ask [%s:%s] \n", wgcli.quotID, wgcli.bid, wgcli.ask);
	
	printf("side ('1'=BUY '2'=SELL) : ");
	scanf("%s", stemp);
	if (stemp[0] != '1' && stemp[0] != '2')
	{ printf("\tinput error..\n"); return (-1); }
	p->side[0] = stemp[0];

	printf("qty : ");
	scanf("%s", stemp);
	memcpy(p->orderqty, stemp, strlen(stemp));

	printf("ordtype ('1'=Market '2'=Limit) : ");
	scanf("%s", stemp);
	if (stemp[0] != '1' && stemp[0] != '2')
	{ printf("\tinput error..\n"); return (-1); }
	p->ordtype[0] = stemp[0];

	printf("timeinforce ('0'=Day '3'=IOC '4'=FOK) : ");
	scanf("%s", stemp);
	if (stemp[0] != '0' && stemp[0] != '3' && stemp[0] != '4')
	{ printf("\tinput error..\n"); return (-1); }
	p->timeinforce[0] = stemp[0];

	sprintf(stemp, "%02d%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	memcpy(p->clordid, stemp, strlen(stemp));

	p->tag35[0] = 'D';
	memcpy(p->symb,  t->symb,  sizeof(p->symb));
	//	char	account		[ 20];		// Dealer ID
	//	char	ordid		[ 24];		// TAG-37 주문번호         : 데몬에서 채번한 KB주문번호
	//	char	origordid	[ 24];		// TAG-37 내부원주문번호   : 취소주문(35=F)인 경우 고객이 보낸 KB원주문번호
	memcpy(p->stype, t->stype, sizeof(p->stype));
	memcpy(p->setldate, t->setldate, sizeof(p->setldate));
	//	char	setldate2	[  8];		// SWAP 인 경우 far 결제일(YYYYMMDD)
	memcpy(p->currency, t->symb, sizeof(p->currency));

	memcpy(p->quotID, wgcli.quotID, sizeof(p->quotID));
	if (p->side[0] == '1')	memcpy(p->price, wgcli.ask, sizeof(p->price));
	else					memcpy(p->price, wgcli.bid, sizeof(p->price));

//p->price[6]='9';
	printf("make order packet done..\n");

	return 0;
}