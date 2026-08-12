#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: FX 주문 세션 (거래원 TCP) — 주문송신 + 체결수신  [VX-4b/4c]
#	File	: pf_1100_ts.c
#	System	: Connect to FX Venue(s) (SMB_ST / FIX-flat 고정 1024B)
#
#	고성능 OMS 의 FX 주문 경로. 입력 트리거(FIFO)로 들어온 주문요청을
#	SMB_ST 신규주문('D')으로 만들어 거래원(venue) TCP 로 송신하고, 같은
#	소켓으로 체결통지('8')를 수신·기록한다. 거래원은 가상거래원
#	(test/integ/mock/mock_fx_venue) 또는 실 거래원. (SHM 원장 적재는 후속)
#
#	[VX-4c] 다거래원: 여러 거래원(JPM/NH/EBS..)에 동시 접속, 주문요청의
#	excode 로 대상 거래원 라우팅. 거래원 추가 = cfg/vexch.ini 블록 1개
#	→ harness 가 VX_FX_VENUES 로 배선(정식 tcp2.ini 통합은 후속).
#
#	환경변수(1차 슬라이스 배선):
#	  VX_FX_VENUES : "excode:host:port[,excode:host:port...]" (다거래원)
#	  (미설정 시) VX_FX_HOST/VX_FX_PORT/VX_FX_EXCODE 단일 거래원 폴백
#	  VX_FX_ORDER_FIFO : 주문요청 트리거 FIFO 경로(harness 가 mkfifo)
#	  주문요청 1줄 = "EXCODE,ACCT,SYMBOL,SIDE,QTY,PX,CLORDID"
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
#include	<fcntl.h>

#define		SMB_SZ		((int)sizeof(SMB_ST))
#define		MAX_VENUE	8

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
typedef struct {
	char	excode;				/* 거래원 코드 (J/N/E..)      */
	int		fd;					/* TCP 소켓                   */
	char	host[64];
	int		port;
	char	acc[8192];			/* 수신 프레임 누적 버퍼      */
	int		alen;
} VENUE_T;

VENUE_T	Venue[MAX_VENUE];
int		VenueCnt = 0;
int		FifoFd   = -1;			/* 주문요청 트리거 FIFO       */
int		OrdSeq   = 500001;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PF_1100_TS (void);
int		Init_Parameters (void);
int		Venue_Connect_All (void);
int		Open_Order_Fifo (void);
void	On_Order_Request (char *line);
void	On_Venue_Recv (VENUE_T *v);

/* SMB_ST char 필드(널 미종료) → 로그용 널종료(뒤 공백/0 제거) */
static void field (char *dst, const char *src, int n)
{
	int i;
	memcpy (dst, src, n); dst[n] = 0;
	for (i = n - 1; i >= 0 && (dst[i] == ' ' || dst[i] == 0); i--) dst[i] = 0;
}
/* 고정폭 필드 세팅(0 패딩) */
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
}	/* End of main () */

/*----------------------------------------------------------------------*/
void	PF_1100_TS (void)
/*----------------------------------------------------------------------*/
{
	if (Init_Parameters ()  == NOTOK)	return;
	if (Venue_Connect_All() == NOTOK)	return;
	if (Open_Order_Fifo ()  == NOTOK)	return;

	Log (USR_OK, "FX order session ready: venues=%d SMB_ST=%d", VenueCnt, SMB_SZ);

	while (START_S != JOB_END)
	{
		fd_set	rf;
		struct timeval tv;
		int		maxfd = -1, rt, i;

		FD_ZERO (&rf);
		if (FifoFd >= 0) { FD_SET (FifoFd, &rf); maxfd = FifoFd; }
		for (i = 0; i < VenueCnt; i++)
			if (Venue[i].fd >= 0)
			{ FD_SET (Venue[i].fd, &rf); if (Venue[i].fd > maxfd) maxfd = Venue[i].fd; }
		tv.tv_sec = 1; tv.tv_usec = 0;

		rt = select (maxfd + 1, &rf, NULL, NULL, &tv);
		if (rt < 0) { if (SYS_NO == EINTR) continue; break; }
		if (rt == 0) continue;

		/* 거래원 → 체결통지 수신 */
		for (i = 0; i < VenueCnt; i++)
			if (Venue[i].fd >= 0 && FD_ISSET (Venue[i].fd, &rf))
				On_Venue_Recv (&Venue[i]);

		/* 주문요청 트리거 FIFO → 주문 송신 */
		if (FifoFd >= 0 && FD_ISSET (FifoFd, &rf))
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
	}
	return;
}	/* End of PF_1100_TS () */

/*----------------------------------------------------------------------*/
int		Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	char *vs = getenv ("VX_FX_VENUES");
	VenueCnt = 0;
	memset (Venue, 0, sizeof (Venue));

	if (vs && *vs)								/* 다거래원: "J:host:port,N:host:port" */
	{
		char tmp[512], *save1, *tok;
		strncpy (tmp, vs, sizeof (tmp) - 1); tmp[sizeof(tmp)-1] = 0;
		for (tok = strtok_r (tmp, ",", &save1); tok && VenueCnt < MAX_VENUE;
			 tok = strtok_r (NULL, ",", &save1))
		{
			char *save2, *e = strtok_r (tok, ":", &save2);
			char *h = strtok_r (NULL, ":", &save2);
			char *p = strtok_r (NULL, ":", &save2);
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
	if (VenueCnt == 0)							/* 단일 거래원 폴백 */
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
	for (i = 0; i < VenueCnt; i++)
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

/*----------------------------------------------------------------------*/
int		Open_Order_Fifo (void)
/*----------------------------------------------------------------------*/
{
	char *path = getenv ("VX_FX_ORDER_FIFO");
	if (!path || !*path)
	{ Log (USR_OK, "no VX_FX_ORDER_FIFO (트리거 없음, 수신 전용 동작)"); FifoFd = -1; return (OK); }
	FifoFd = open (path, O_RDWR | O_NONBLOCK);	/* O_RDWR: writer 부재에도 select 가능 */
	if (FifoFd < 0)
	{ Log (USR_ERROR, "order fifo open fail [%s] {%d:%s}", path, SYS_NO, SYS_STR); return (NOTOK); }
	Log (USR_OK, "order trigger fifo [%s] fd=%d", path, FifoFd);
	return (OK);
}

/* 주문요청 1줄("EXCODE,ACCT,SYMBOL,SIDE,QTY,PX,CLORDID") → 해당 거래원에 SMB_ST 'D' */
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
		for (i = 0; i < VenueCnt; i++)
			if (Venue[i].excode == excode && Venue[i].fd >= 0) { vidx = i; break; }
		if (vidx < 0) { Log (USR_ERROR, "미등록 거래원 excode=%c (취소 ClOrdID=%s)", excode, clord); return; }
		memset (&o, 0, SMB_SZ);
		o.smb_MsgType[0] = 'F';					/* 취소 */
		setf (o.smb_SenderCompID, 50, "FEP");
		setf (o.smb_TargetCompID, 50, "SMB");
		setf (o.smb_ClOrdID, 24, clord);
		setf (o.smb_OrigClOrdID, 24, origclord);
		if (write (Venue[vidx].fd, &o, SMB_SZ) != SMB_SZ)
		{ Log (USR_ERROR, "cancel write fail venue=%c {%d:%s}", excode, SYS_NO, SYS_STR); return; }
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

	for (i = 0; i < VenueCnt; i++)
		if (Venue[i].excode == excode && Venue[i].fd >= 0) { vidx = i; break; }
	if (vidx < 0)
	{ Log (USR_ERROR, "미등록 거래원 excode=%c → cfg/vexch.ini 블록 추가 필요 (ClOrdID=%s)", excode, clord); return; }

	memset (&o, 0, SMB_SZ);
	o.smb_MsgType[0] = 'D';						/* 신규 */
	setf (o.smb_SenderCompID, 50, "FEP");
	setf (o.smb_TargetCompID, 50, "SMB");
	setf (o.smb_ClOrdID, 24, clord);
	setf (o.smb_Account, 30, acct[0] ? acct : "FXACCT0001");
	setf (o.smb_Currency, 3, "USD");
	setf (o.smb_Symbol, 7, sym[0] ? sym : "USD/KRW");
	o.smb_Side[0]    = sidebuf[0];
	o.smb_OrdType[0] = '2';						/* 지정가 */
	setf (o.smb_OrderQty, 30, qty[0] ? qty : "1000000");
	setf (o.smb_Price, 30, px[0] ? px : "1385.50");
	o.smb_TimeInForce[0] = '3';					/* IOC */

	if (write (Venue[vidx].fd, &o, SMB_SZ) != SMB_SZ)
	{ Log (USR_ERROR, "order write fail venue=%c {%d:%s}", excode, SYS_NO, SYS_STR); return; }
	Log (USR_OK, "FX ORDER send venue=%c ClOrdID=%s Symbol=%s Side=%c Qty=%s Px=%s",
		 excode, clord, sym[0]?sym:"USD/KRW", o.smb_Side[0], qty[0]?qty:"1000000", px[0]?px:"1385.50");
}

/* 거래원 소켓 → SMB_ST 체결통지 수신·기록(고정 프레임 누적, venue 별 버퍼) */
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
		SMB_ST *e = (SMB_ST *) v->acc;
		char cl[25], oc[25], oid[31], eid[31], cum[31], lpx[31];
		field (cl,  e->smb_ClOrdID,     24);
		field (oc,  e->smb_OrigClOrdID, 24);
		field (oid, e->smb_OrdID,   30);
		field (eid, e->smb_ExecID,  30);
		field (cum, e->smb_CumQty,  30);
		field (lpx, e->smb_LastPx,  30);
		Log (USR_OK, "FX EXEC recv venue=%c MsgType=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrigClOrdID=%s OrdID=%s ExecID=%s CumQty=%s LastPx=%s",
			 v->excode, e->smb_MsgType[0], e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oc, oid, eid, cum, lpx);
		if (getenv ("VX_TEST") != NULL)
			Log (USR_OK, "VX_TEST exec venue=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrigClOrdID=%s OrdID=%s CumQty=%s",
				 v->excode, e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oc, oid, cum);
		memmove (v->acc, v->acc + SMB_SZ, v->alen - SMB_SZ);
		v->alen -= SMB_SZ;
	}
}
