static char sqla_program_id[292] = 
{
 '\xad','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x5a','\x42','\x6f','\x5a','\x4b','\x64','\x4b','\x6f','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x9','\x0','\x57','\x51','\x43','\x4c','\x49','\x48','\x4f','\x47','\x41','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
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


#line 1 "wqclihoga.sqc"

#include <curses.h>
#include <term.h>
#include <termios.h>

#include "mds2.h"
#include "wfaapi.h"
#include "wgate.h"

int		doneflag=0;
char	tbuf[1024];
char	compid[32];
char	obuf[1024];			// send order temp buffer
AG_DQUE		qin, qout;

WGCOM		 *tc;
WG_QUOT_REQ  *t;

WINDOW	*wquot;
WINDOW	*winput;
WINDOW	*wrcv;

struct _WGCLI {
	int		seq		;
	char	useyn	[  1];
	char	reqID	[ 64];		// API:내부생성, RFS/RFQ:Tag117=QuoteID (Tag131=QuoteReqID, Tag262=MDReqID)
	char	quotID	[ 32];		// 발송한 last QuoteID (Tag117=QuoteID)
	char	bid		[ 16];		// 고객제공환율
	char	ask		[ 16];
	char	bid2	[ 16];		// 고객제공환율
	char	ask2	[ 16];
} wgcli;

int setquotreq(char *pbuf);
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *win);
void closeterm();
void *handler_quot(void *argv);
void _set_pad(char *p, int len);

void exitproc(int exitval)
{
	int		rc, dlen;
	char	sbuf[1024];
	WGCOM		*c=sbuf;
	WG_QUOT_RES *p=c->data;

	if (wgcli.useyn[0] != '1')	{
		endwin();
		exit(0);
	}
	
	if (!doneflag)
	{
		memset(sbuf, 0x00, sizeof(sbuf));
		memcpy(sbuf, tbuf, sizeof(sbuf));

		c->msgtp[0] = 'L';
//		p->tag35[0] = 'b';
		
		dlen=sizeof(WGCOM)+sizeof(WG_QUOT_RES);
		_set_pad(sbuf, dlen);
		rc = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, 1024, sbuf, dlen);
		if (rc < 0) {
			printf("agdque_wrn error : %d\n", rc);
			exit(0);
		}
		printf("\nsend unsubscribe packet ok..!!!! [%d]\n", rc);
	}

	endwin();
	
	exit(0);
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

void init_scr()
{
	initscr();
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE,  COLOR_WHITE);
	init_pair(3, COLOR_WHITE, COLOR_RED);
//	curs_set(2);		// cursor invisible, normal, very_visible
//	noecho();
//	keypad(stdscr, TRUE);

	return ;
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		pos, idx, seq=0;
	int		ii,  dlen, rtn;
	int		slen, rwno[2];
	char	sbuf[1024], rbuf[1024], stemp[32];
	WGCOM		*c;
	WGKEY		*k=&c->wgkey;
	WG_ORD_EXC	*p=c->data;

	if (argc < 3)
	{
		printf("\n\t %s [compid qidx] \n", argv[0]);
		exit(0);
	}

	strcpy(compid, argv[1]);
	idx = atoi(argv[2]);

	signal(SIGINT,  (void *)exitproc);
	signal(SIGQUIT, (void *)exitproc);

	memset(sbuf, 0x00, sizeof(sbuf));
	memset(rbuf, 0x00, sizeof(rbuf));
	memset(tbuf, 0x00, sizeof(tbuf));

	memset(&wgcli, 0x00, sizeof(wgcli));

	AG_DQUECLS(&qin);
	AG_DQUECLS(&qout);

	qin.xha |= AQ_XHA_OPN;
	qin.xha |= AG_XAS_LIV;
	
	int		dsiz = 1024;

	tc = tbuf;
	t  = (WG_QUOT_REQ *)tc->data;

////////////////////////////
// READ DQ seq init
	rtn = agdque_at (&qin, WG_DAT_DIR, NULL, WGDQ_SNDRS, WGSEM_SNDRS, sizeof(rbuf));
	if (rtn < 0)
	{
		printf("\n agdque_at error.. [%d]\n", rtn);
		pthread_exit(NULL);
	}
	memset(&rwno, 0x00, sizeof(rwno));

//	printf("set aid[%d] read no [%d -> %d]\n", idx, qsise.hdr.rno[idx], qout.hdr.wno);
	rwno[0] = qin.hdr.wno;
	rtn = agdque_rwi_io (&qin, idx, &rwno);
	if (rtn < 0)
	{
		printf("\n agdque_rwi_io error.. [%d]\n", rtn);
		pthread_exit(NULL);
	}
	qin.xha |= AQ_XHA_OPN;
	qin.xha |= AG_XAS_LIV;

////////////////////////////
	if ((rtn=setquotreq(sbuf)) < 0) {
		printf("setinput error : %d\n", rtn);
		exit(0);
	}
	
	// QUOTE REQUEST SEND
	dlen = sizeof(WGCOM) + sizeof(WG_QUOT_REQ);
	_set_pad(sbuf, dlen);
	rtn = agdque_wrn (&qin, WG_DAT_DIR, NULL, WGDQ_RCVRQ, WGSEM_RCVRQ, dsiz, sbuf, dlen);
	if (rtn < 0) {
		printf("agdque_wrn error : %d\n", rtn);
		exit(0);
	}
	wgcli.useyn[0] = '1';
	memcpy(tbuf, sbuf, sizeof(tbuf));

////////////////////////////

	WGCOM		*rc=rbuf;

	WG_QUOT		*q;
	WG_QUOT_RES	*r;
	WG_ORD_EXC	*e;

//	printf("wait for data ...\n");
	wclear(wquot);

	time_t	tv;
	struct tm *tm;

	init_scr();

//	wquot = subwin(stdscr, 5, 30, 0, 0);

//	bkgd(COLOR_PAIR(1));
//	wbkgd(wquot, COLOR_PAIR(1));
//	watron(wquot, COLOR_PAIR(3));
attron(COLOR_PAIR(2));
	move(0, 0);
	printw("PRD    : %.6s  %.8s.%.8s", t->symb, t->setldate, t->setldate2);
attroff(COLOR_PAIR(2));
	move(2, 0);
	addstr("TIME   : ");
	move(3, 0);
	addstr("BID    : ");
	move(4, 0);
//	watron(wquot, COLOR_PAIR(3));
	addstr("ASK    : ");
	move(5, 0);
//	watron(wquot, COLOR_PAIR(3));
	addstr("QUOTID : ");

	refresh();

	// 시세 수신
	while (1) {
		
		memset(rbuf, 0x00, sizeof(rbuf));
		
		rtn = agdque_rdn (&qin, WG_DAT_DIR, NULL, WGDQ_SNDRS, WGSEM_SNDRS, rbuf, sizeof(rbuf), idx, (-1), &rwno);
		if (rtn < 0)	{
			printf("agdque_rdn error : %d\n", rtn);
			break;
		}
 
		tv = time(NULL);
		tm = localtime(&tv);
	
		switch (rc->msgtp[0])
		{
		case MSG_ORDEXC :
			e = (WG_ORD_EXC  *)rc->data;
			
			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, sizeof(WGKEY)) != 0)	break;

			printf("RECV [%s] exc[%s:%s:%s] stat[%.1s:%.1s] rej[%.8s:%s]\n", e->ordid, e->lastqty, e->execid, e->lastpx, e->ordstatus, e->exectype, e->rejcd, e->rejtext);
			if (str2i(e->rejcd, sizeof(e->rejcd)) != 0)
			{
				doneflag=1;
				endwin();
				printf("\n receive reject....!!!! [%.8s:%s]\n", e->rejcd, e->rejtext);
				exit(0);
			}
			break;

		case MSG_QUOTE :
			q = (WG_QUOT     *)rc->data;

			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, sizeof(WGKEY)) != 0)	break;
/*
			printf("[%.*s] rcv=[%.*s/%.*s/%.*s/%.*s/%.*s] [%.*s/%.*s/%.*s/%.*s]\n", sizeof(q->quotID), q->quotID,
				sizeof(q->msgtp) ,q->msgtp, sizeof(q->sendcompid) ,q->sendcompid, 
				sizeof(q->sendsubid) ,q->sendsubid, sizeof(q->stype) ,q->stype, sizeof(q->symb) ,q->symb, 
				sizeof(q->bid), q->bid, sizeof(q->ask), q->ask, sizeof(q->bid2), q->bid2, sizeof(q->ask2), q->ask2);

			if (memcmp(q->reqtp, t->reqtp, sizeof(WGKEY)) != 0)	break;
*/
			//wgcli.bid = str2d(q->bid, sizeof(q->bid));
			//wgcli.ask = str2d(q->ask, sizeof(q->ask));
			memcpy(wgcli.bid, q->bid, sizeof(q->bid));
			memcpy(wgcli.ask, q->ask, sizeof(q->ask));
			
			memcpy(wgcli.bid2, q->bid2, sizeof(q->bid2));
			memcpy(wgcli.ask2, q->ask2, sizeof(q->ask2));
			memcpy(wgcli.quotID, q->quotID, sizeof(q->quotID));
			(wgcli.seq)++;

//attron(COLOR_PAIR(2));
	move(2, 10);
	printw("%02d:%02d.%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
attron(COLOR_PAIR(1));
	move(3, 10);
	printw("%.16s    %.16s", wgcli.bid, wgcli.bid2);
attron(COLOR_PAIR(3));
	move(4, 10);
	printw("%.16s    %.16s", wgcli.ask, wgcli.ask2);
attroff(COLOR_PAIR(3));
	move(5, 10);
	printw("%.32s", wgcli.quotID);

	refresh();
//attroff(COLOR_PAIR(2));
/*
			mvwprintw(wquot, 1, 1, "BID : %s", wgcli.bid);
			mvwprintw(wquot, 1, 20, "ASK : %s", wgcli.ask);
			mvwprintw(wquot, 1, 40, "QUOTID : %s", wgcli.quotID);
*/
			break;

		case MSG_QTRES :
			r = (WG_QUOT_RES *)rc->data;

			if (memcmp((char *)&rc->wgkey, (char *)&tc->wgkey, sizeof(WGKEY)) != 0)	break;

			endwin();
			refresh();
			printf("RECV : close request.. [%.8s:%.*s]\n", r->rejcd, sizeof(r->rejtext), r->rejtext);
			doneflag=1;
			wgcli.useyn[0] = '0';
			exitproc(0);
			break;

		default :
			break;
		}
	}

	printf("receive quote done...........\n");

	exitproc(0);
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
//	p->reqtp[0] = REQTP_RFS;

	sprintf(k->reqID, "T_%02d.%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	strcpy(k->targetcompid, "KBET_RFS_BETA");
	strcpy(k->sendcompid, compid);

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

memcpy(wgcli.reqID, k->reqID, sizeof(k->reqID));
wgcli.useyn[0] = '1';

	printf("make input packet done..\n");

	return 0;
}
