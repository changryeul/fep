#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: FX 주문 세션 (거래원) — 주문송신 + 체결수신  [VX-4b~4f]
#	File	: pf_1100_ts.c
#	System	: Connect to FX Venue(s) — SMB_ST / FIX-flat 고정 1024B
#
#	고성능 OMS 의 FX 주문 경로. 입력 트리거(FIFO)로 들어온 주문요청을
#	SMB_ST 신규주문('D')/취소('F')로 만들어 거래원으로 송신하고 체결통지
#	('8')를 수신·기록한다. 거래원은 가상거래원(mock) 또는 실 거래원.
#
#	전송수단(env VX_FX_TRANSPORT):
#	  tcp(기본) : mock_fx_venue 와 TCP (VX-4a~4e 테스트 배선)
#	  msgq      : 실 연동과 동일한 SysV 메시지큐 (win mon/fep 대면).  [VX-4f]
#	              VX_FX_MSGQ="excode:ord_key:exe_key:exe_mtype[,...]"
#	              주문 → ord_queue(msgsnd), 체결 ← exe_queue(msgrcv, mtype 데뮉스)
#	공통:
#	  VX_FX_ORDER_FIFO : 주문요청 트리거 FIFO ("EXCODE,ACCT,SYMBOL,SIDE,QTY,PX,CLORDID"
#	                     / 취소 "CXL,EXCODE,CLORDID,ORIGCLORDID")
#	  (tcp) VX_FX_VENUES="excode:host:port[,...]" (단일 VX_FX_HOST/PORT 폴백)
------------------------------------------------------------------------*/
#include	"pa_struct.h"
#include	"fep_fepp.h"
#include	"fx.h"

#define		DATA_SIZE		100
#include	"buf_struct.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<errno.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/select.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<fcntl.h>

#define		SMB_SZ		((int)sizeof(SMB_ST))
#define		MAX_VENUE	8

/* SysV msgq 메시지 봉투(mtext = SMB_ST) */
typedef struct { long mtype; SMB_ST body; } QMSG;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
typedef struct {
	char	excode;				/* 거래원 코드 (J/N/E/S/C/B..)     */
	/* tcp */
	int		fd;
	char	host[64];
	int		port;
	char	acc[8192];			/* 수신 프레임 누적 버퍼(tcp)      */
	int		alen;
	/* msgq */
	long	ord_key, exe_key;	/* 주문/체결 큐 키(hex)            */
	int		ord_qid, exe_qid;	/* 큐 id                            */
	long	exe_mtype;			/* 체결큐 데뮉스 mtype(거래원 구분)*/
} VENUE_T;

VENUE_T	Venue[MAX_VENUE];
int		VenueCnt     = 0;
int		FifoFd       = -1;
int		OrdSeq       = 500001;
int		TransportMsgq = 0;		/* 0=tcp, 1=msgq */

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PF_1100_TS (void);
int		Init_Parameters (void);
int		Venue_Connect_All (void);
int		Open_Order_Fifo (void);
void	On_Order_Request (char *line);
void	On_Venue_Recv (VENUE_T *v);
int		Venue_Send (VENUE_T *v, SMB_ST *o);
void	Drain_Msgq_Execs (void);
void	Log_Exec (char excode, SMB_ST *e);

/* SMB_ST char 필드(널 미종료) → 로그용 널종료(뒤 공백/0 제거) */
static void field (char *dst, const char *src, int n)
{
	int i;
	memcpy (dst, src, n); dst[n] = 0;
	for (i = n - 1; i >= 0 && (dst[i] == ' ' || dst[i] == 0); i--) dst[i] = 0;
}
static void setf (char *dst, int n, const char *src)
{ memset (dst, 0, n); strncpy (dst, src, n); }

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PF_1100_TS ();
	Exit_Process ();
	return (0);
}

/*----------------------------------------------------------------------*/
void	PF_1100_TS (void)
/*----------------------------------------------------------------------*/
{
	if (Init_Parameters ()  == NOTOK)	return;
	if (Venue_Connect_All() == NOTOK)	return;
	if (Open_Order_Fifo ()  == NOTOK)	return;

	Log (USR_OK, "FX order session ready: transport=%s venues=%d SMB_ST=%d",
		 TransportMsgq ? "msgq" : "tcp", VenueCnt, SMB_SZ);

	while (START_S != JOB_END)
	{
		fd_set	rf;
		struct timeval tv;
		int		maxfd = -1, rt, i;

		FD_ZERO (&rf);
		if (FifoFd >= 0) { FD_SET (FifoFd, &rf); maxfd = FifoFd; }
		if (!TransportMsgq)
			for (i = 0; i < VenueCnt; i++)
				if (Venue[i].fd >= 0)
				{ FD_SET (Venue[i].fd, &rf); if (Venue[i].fd > maxfd) maxfd = Venue[i].fd; }
		/* msgq 는 select 불가 → 짧은 타임아웃으로 체결큐 폴링 */
		tv.tv_sec = TransportMsgq ? 0 : 1;
		tv.tv_usec = TransportMsgq ? 200000 : 0;

		rt = select (maxfd + 1, &rf, NULL, NULL, &tv);
		if (rt < 0) { if (SYS_NO == EINTR) continue; break; }

		if (rt > 0 && FifoFd >= 0 && FD_ISSET (FifoFd, &rf))
		{
			char	fbuf[1024];
			int		n = read (FifoFd, fbuf, sizeof (fbuf) - 1);
			if (n > 0)
			{
				char *p = fbuf, *nl;
				fbuf[n] = 0;
				while ((nl = strchr (p, '\n')) != NULL)
				{ *nl = 0; if (*p) On_Order_Request (p); p = nl + 1; }
				if (*p) On_Order_Request (p);
			}
		}

		if (TransportMsgq)
			Drain_Msgq_Execs ();				/* 체결큐 msgrcv 드레인 */
		else if (rt > 0)
			for (i = 0; i < VenueCnt; i++)
				if (Venue[i].fd >= 0 && FD_ISSET (Venue[i].fd, &rf))
					On_Venue_Recv (&Venue[i]);
	}
	return;
}

/*----------------------------------------------------------------------*/
int		Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	char *tr = getenv ("VX_FX_TRANSPORT");
	char *mq = getenv ("VX_FX_MSGQ");
	char *vs = getenv ("VX_FX_VENUES");
	VenueCnt = 0;
	memset (Venue, 0, sizeof (Venue));
	TransportMsgq = (tr && !strcmp (tr, "msgq")) ? 1 : 0;

	if (TransportMsgq)							/* msgq: "excode:ord_key:exe_key:exe_mtype,..." */
	{
		char tmp[512], *save1, *tok;
		if (!mq || !*mq) { Log (USR_ERROR, "VX_FX_TRANSPORT=msgq 인데 VX_FX_MSGQ 없음"); return (NOTOK); }
		strncpy (tmp, mq, sizeof (tmp) - 1); tmp[sizeof(tmp)-1] = 0;
		for (tok = strtok_r (tmp, ",", &save1); tok && VenueCnt < MAX_VENUE;
			 tok = strtok_r (NULL, ",", &save1))
		{
			char *s2, *e = strtok_r (tok, ":", &s2);
			char *ok = strtok_r (NULL, ":", &s2);
			char *xk = strtok_r (NULL, ":", &s2);
			char *mt = strtok_r (NULL, ":", &s2);
			if (e && ok && xk && mt)
			{
				Venue[VenueCnt].excode    = e[0];
				Venue[VenueCnt].ord_key   = strtol (ok, NULL, 16);
				Venue[VenueCnt].exe_key   = strtol (xk, NULL, 16);
				Venue[VenueCnt].exe_mtype = atol (mt);
				Venue[VenueCnt].ord_qid = Venue[VenueCnt].exe_qid = -1;
				Venue[VenueCnt].fd = -1;
				VenueCnt++;
			}
		}
		{ int i; for (i = 0; i < VenueCnt; i++)
			Log (USR_OK, "Init: venue[%d] excode=%c ord_key=0x%lx exe_key=0x%lx exe_mtype=%ld",
				 i, Venue[i].excode, Venue[i].ord_key, Venue[i].exe_key, Venue[i].exe_mtype); }
		return (OK);
	}

	if (vs && *vs)								/* tcp 다거래원 */
	{
		char tmp[512], *save1, *tok;
		strncpy (tmp, vs, sizeof (tmp) - 1); tmp[sizeof(tmp)-1] = 0;
		for (tok = strtok_r (tmp, ",", &save1); tok && VenueCnt < MAX_VENUE;
			 tok = strtok_r (NULL, ",", &save1))
		{
			char *s2, *e = strtok_r (tok, ":", &s2);
			char *h = strtok_r (NULL, ":", &s2);
			char *p = strtok_r (NULL, ":", &s2);
			if (e && h && p)
			{
				Venue[VenueCnt].excode = e[0];
				strncpy (Venue[VenueCnt].host, h, sizeof (Venue[0].host) - 1);
				Venue[VenueCnt].port = atoi (p);
				Venue[VenueCnt].fd = -1;
				VenueCnt++;
			}
		}
	}
	if (VenueCnt == 0)							/* tcp 단일 폴백 */
	{
		char *h  = getenv ("VX_FX_HOST");
		char *p  = getenv ("VX_FX_PORT");
		char *ex = getenv ("VX_FX_EXCODE");
		Venue[0].excode = (ex && *ex) ? ex[0] : '?';
		strncpy (Venue[0].host, (h && *h) ? h : "127.0.0.1", sizeof (Venue[0].host) - 1);
		Venue[0].port = (p && *p) ? atoi (p) : 19100;
		Venue[0].fd = -1;
		VenueCnt = 1;
	}
	{ int i; for (i = 0; i < VenueCnt; i++)
		Log (USR_OK, "Init: venue[%d] excode=%c target[%s:%d]",
			 i, Venue[i].excode, Venue[i].host, Venue[i].port); }
	return (OK);
}

/*----------------------------------------------------------------------*/
int		Venue_Connect_All (void)
/*----------------------------------------------------------------------*/
{
	int i, ok = 0;

	if (TransportMsgq)							/* msgq: 큐 attach(없으면 생성) */
	{
		for (i = 0; i < VenueCnt; i++)
		{
			Venue[i].ord_qid = msgget ((key_t) Venue[i].ord_key, 0666 | IPC_CREAT);
			Venue[i].exe_qid = msgget ((key_t) Venue[i].exe_key, 0666 | IPC_CREAT);
			if (Venue[i].ord_qid < 0 || Venue[i].exe_qid < 0)
			{ Log (USR_ERROR, "venue[%c] msgget fail ord=0x%lx exe=0x%lx {%d:%s}",
				   Venue[i].excode, Venue[i].ord_key, Venue[i].exe_key, SYS_NO, SYS_STR); continue; }
			ok++;
			Log (USR_OK, "venue[%c] msgq ready ord_qid=%d exe_qid=%d (mtype=%ld)",
				 Venue[i].excode, Venue[i].ord_qid, Venue[i].exe_qid, Venue[i].exe_mtype);
		}
		if (ok == 0) { Log (USR_ERROR, "no venue msgq"); return (NOTOK); }
		return (OK);
	}

	for (i = 0; i < VenueCnt; i++)				/* tcp: connect */
	{
		struct sockaddr_in sa;
		int fd = socket (AF_INET, SOCK_STREAM, 0);
		if (fd < 0) { Log (USR_ERROR, "socket fail {%d:%s}", SYS_NO, SYS_STR); continue; }
		memset (&sa, 0, sizeof (sa));
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr (Venue[i].host);
		sa.sin_port = htons ((unsigned short) Venue[i].port);
		if (connect (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0)
		{
			Log (USR_ERROR, "venue[%c] connect fail [%s:%d] {%d:%s}",
				 Venue[i].excode, Venue[i].host, Venue[i].port, SYS_NO, SYS_STR);
			close (fd); Venue[i].fd = -1; continue;
		}
		Venue[i].fd = fd; ok++;
		Log (USR_OK, "venue[%c] connected [%s:%d] fd=%d",
			 Venue[i].excode, Venue[i].host, Venue[i].port, fd);
	}
	if (ok == 0) { Log (USR_ERROR, "no venue connected"); return (NOTOK); }
	return (OK);
}

/* 주문/취소 SMB_ST 를 거래원으로 송신 (전송수단 추상화) */
/*----------------------------------------------------------------------*/
int		Venue_Send (VENUE_T *v, SMB_ST *o)
/*----------------------------------------------------------------------*/
{
	if (TransportMsgq)
	{
		QMSG m; m.mtype = 1; memcpy (&m.body, o, SMB_SZ);
		if (msgsnd (v->ord_qid, &m, SMB_SZ, 0) < 0)
		{ Log (USR_ERROR, "msgsnd fail venue=%c {%d:%s}", v->excode, SYS_NO, SYS_STR); return (NOTOK); }
		return (OK);
	}
	if (write (v->fd, o, SMB_SZ) != SMB_SZ)
	{ Log (USR_ERROR, "write fail venue=%c {%d:%s}", v->excode, SYS_NO, SYS_STR); return (NOTOK); }
	return (OK);
}

/*----------------------------------------------------------------------*/
int		Open_Order_Fifo (void)
/*----------------------------------------------------------------------*/
{
	char *path = getenv ("VX_FX_ORDER_FIFO");
	if (!path || !*path)
	{ Log (USR_OK, "no VX_FX_ORDER_FIFO (트리거 없음, 수신 전용 동작)"); FifoFd = -1; return (OK); }
	FifoFd = open (path, O_RDWR | O_NONBLOCK);
	if (FifoFd < 0)
	{ Log (USR_ERROR, "order fifo open fail [%s] {%d:%s}", path, SYS_NO, SYS_STR); return (NOTOK); }
	Log (USR_OK, "order trigger fifo [%s] fd=%d", path, FifoFd);
	return (OK);
}

/* 주문요청 1줄 → 신규('D')/취소('F') SMB_ST 빌드·라우팅·송신 */
/*----------------------------------------------------------------------*/
void	On_Order_Request (char *line)
/*----------------------------------------------------------------------*/
{
	SMB_ST	o;
	char	*tok, *fld[10];
	char	acct[31], sym[8], sidebuf[8], qty[31], px[31], clord[25], origclord[25];
	int		nt = 0, i, vidx = -1;
	char	excode;

	for (tok = strtok (line, ","); tok && nt < 10; tok = strtok (NULL, ","))
		fld[nt++] = tok;
	if (nt == 0) return;

	/* --- 취소: "CXL,EXCODE,CLORDID,ORIGCLORDID" --- */
	if (!strcmp (fld[0], "CXL"))
	{
		if (nt < 4) { Log (USR_ERROR, "취소요청 필드부족 (CXL,EXCODE,CLORDID,ORIGCLORDID)"); return; }
		excode = fld[1][0];
		strncpy (clord, fld[2], 24); clord[24] = 0;
		strncpy (origclord, fld[3], 24); origclord[24] = 0;
		for (i = 0; i < VenueCnt; i++) if (Venue[i].excode == excode) { vidx = i; break; }
		if (vidx < 0) { Log (USR_ERROR, "미등록 거래원 excode=%c (취소 ClOrdID=%s)", excode, clord); return; }
		memset (&o, 0, SMB_SZ);
		o.smb_MsgType[0] = 'F';
		setf (o.smb_SenderCompID, 50, "FEP");
		setf (o.smb_TargetCompID, 50, "SMB");
		setf (o.smb_ClOrdID, 24, clord);
		setf (o.smb_OrigClOrdID, 24, origclord);
		if (Venue_Send (&Venue[vidx], &o) == OK)
			Log (USR_OK, "FX CANCEL send venue=%c ClOrdID=%s OrigClOrdID=%s", excode, clord, origclord);
		return;
	}

	/* --- 신규: "EXCODE,ACCT,SYMBOL,SIDE,QTY,PX,CLORDID" --- */
	acct[0]=sym[0]=sidebuf[0]=qty[0]=px[0]=clord[0]=0;
	excode = fld[0][0];
	if (nt > 1) { strncpy (acct, fld[1], 30); acct[30] = 0; }
	if (nt > 2) { strncpy (sym,  fld[2], 7);  sym[7]  = 0; }
	if (nt > 3) sidebuf[0] = fld[3][0];
	if (nt > 4) { strncpy (qty,  fld[4], 30); qty[30] = 0; }
	if (nt > 5) { strncpy (px,   fld[5], 30); px[30]  = 0; }
	if (nt > 6) { strncpy (clord, fld[6], 24); clord[24] = 0; }
	if (!clord[0]) snprintf (clord, sizeof (clord), "FEPFX%010d", OrdSeq++);
	if (!sidebuf[0]) sidebuf[0] = '1';

	for (i = 0; i < VenueCnt; i++) if (Venue[i].excode == excode) { vidx = i; break; }
	if (vidx < 0)
	{ Log (USR_ERROR, "미등록 거래원 excode=%c → cfg/vexch.ini 블록 추가 필요 (ClOrdID=%s)", excode, clord); return; }

	memset (&o, 0, SMB_SZ);
	o.smb_MsgType[0] = 'D';
	setf (o.smb_SenderCompID, 50, "FEP");
	setf (o.smb_TargetCompID, 50, "SMB");
	setf (o.smb_ClOrdID, 24, clord);
	setf (o.smb_Account, 30, acct[0] ? acct : "FXACCT0001");
	setf (o.smb_Currency, 3, "USD");
	setf (o.smb_Symbol, 7, sym[0] ? sym : "USD/KRW");
	o.smb_Side[0]    = sidebuf[0];
	o.smb_OrdType[0] = '2';
	setf (o.smb_OrderQty, 30, qty[0] ? qty : "1000000");
	setf (o.smb_Price, 30, px[0] ? px : "1385.50");
	o.smb_TimeInForce[0] = '3';

	if (Venue_Send (&Venue[vidx], &o) == OK)
		Log (USR_OK, "FX ORDER send venue=%c ClOrdID=%s Symbol=%s Side=%c Qty=%s Px=%s",
			 excode, clord, sym[0]?sym:"USD/KRW", o.smb_Side[0], qty[0]?qty:"1000000", px[0]?px:"1385.50");
}

/* 체결통지 SMB_ST 1건 기록 (tcp/msgq 공통) */
/*----------------------------------------------------------------------*/
void	Log_Exec (char excode, SMB_ST *e)
/*----------------------------------------------------------------------*/
{
	char cl[25], oc[25], oid[31], eid[31], cum[31], lpx[31];
	field (cl,  e->smb_ClOrdID,     24);
	field (oc,  e->smb_OrigClOrdID, 24);
	field (oid, e->smb_OrdID,   30);
	field (eid, e->smb_ExecID,  30);
	field (cum, e->smb_CumQty,  30);
	field (lpx, e->smb_LastPx,  30);
	Log (USR_OK, "FX EXEC recv venue=%c MsgType=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrigClOrdID=%s OrdID=%s ExecID=%s CumQty=%s LastPx=%s",
		 excode, e->smb_MsgType[0], e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oc, oid, eid, cum, lpx);
	if (getenv ("VX_TEST") != NULL)
		Log (USR_OK, "VX_TEST exec venue=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrigClOrdID=%s OrdID=%s CumQty=%s",
			 excode, e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oc, oid, cum);
}

/* tcp: 거래원 소켓 → SMB_ST 체결 수신(프레임 누적) */
/*----------------------------------------------------------------------*/
void	On_Venue_Recv (VENUE_T *v)
/*----------------------------------------------------------------------*/
{
	int rt = recv (v->fd, v->acc + v->alen, sizeof (v->acc) - v->alen, 0);
	if (rt <= 0)
	{
		Log (USR_ERROR, "venue[%c] recv %s {%d:%s}", v->excode, rt==0?"closed":"fail", SYS_NO, SYS_STR);
		close (v->fd); v->fd = -1; v->alen = 0; return;
	}
	v->alen += rt;
	while (v->alen >= SMB_SZ)
	{
		Log_Exec (v->excode, (SMB_ST *) v->acc);
		memmove (v->acc, v->acc + SMB_SZ, v->alen - SMB_SZ);
		v->alen -= SMB_SZ;
	}
}

/* msgq: 체결큐 msgrcv 드레인 (공유큐는 mtype 로 거래원 데뮉스) */
/*----------------------------------------------------------------------*/
void	Drain_Msgq_Execs (void)
/*----------------------------------------------------------------------*/
{
	int  i, j, done[MAX_VENUE], dn = 0;
	for (i = 0; i < VenueCnt; i++)
	{
		int  qid = Venue[i].exe_qid, seen = 0;
		QMSG m;
		if (qid < 0) continue;
		for (j = 0; j < dn; j++) if (done[j] == qid) { seen = 1; break; }
		if (seen) continue;
		done[dn++] = qid;
		while (msgrcv (qid, &m, SMB_SZ, 0, IPC_NOWAIT) > 0)
		{
			char ex = '?'; int k;
			for (k = 0; k < VenueCnt; k++)
				if (Venue[k].exe_mtype == m.mtype) { ex = Venue[k].excode; break; }
			Log_Exec (ex, &m.body);
		}
	}
}
