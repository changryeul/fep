#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: FX 주문 세션 (거래원 TCP) — 주문송신 + 체결수신  [VX-4b]
#	File	: pf_1100_ts.c
#	System	: Connect to FX Venue (SMB_ST / FIX-flat 고정 1024B)
#
#	고성능 OMS 의 FX 주문 경로. 입력 트리거(FIFO)로 들어온 주문요청을
#	SMB_ST 신규주문('D')으로 만들어 거래원(venue) TCP 로 송신하고, 같은
#	소켓으로 체결통지('8')를 수신·기록한다. 거래원은 가상거래원
#	(test/integ/mock/mock_fx_venue) 또는 실 거래원. (SHM 원장 적재는 후속)
#
#	접속 대상/트리거는 1차 슬라이스에서 환경변수로 배선(정식 tcp2.ini/입력
#	FIFO 통합은 후속):
#	  VX_FX_HOST(기본 127.0.0.1) VX_FX_PORT(기본 19100)
#	  VX_FX_ORDER_FIFO : 주문요청 트리거 FIFO 경로(harness 가 mkfifo)
#	  주문요청 1줄 = "ACCT,SYMBOL,SIDE,QTY,PX,CLORDID"
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

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		VenueFd = -1;			/* 거래원 TCP 소켓            */
int		FifoFd  = -1;			/* 주문요청 트리거 FIFO       */
int		OrdSeq  = 500001;		/* ClOrdID 미지정 시 자동번호 */
char	VenueHost[64];
int		VenuePort;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PF_1100_TS   (void);
int		Init_Parameters (void);
int		Venue_Connect (void);
int		Open_Order_Fifo (void);
void	On_Order_Request (char *line);
void	On_Venue_Recv (void);

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
	if (Init_Parameters () == NOTOK)	return;
	if (Venue_Connect ()   == NOTOK)	return;
	if (Open_Order_Fifo () == NOTOK)	return;

	Log (USR_OK, "FX order session ready: venue[%s:%d] SMB_ST=%d",
		  VenueHost, VenuePort, SMB_SZ);

	while (START_S != JOB_END)
	{
		fd_set	rf;
		struct timeval tv;
		int		maxfd, rt;

		FD_ZERO (&rf);
		if (FifoFd  >= 0) FD_SET (FifoFd,  &rf);
		if (VenueFd >= 0) FD_SET (VenueFd, &rf);
		maxfd = (FifoFd > VenueFd ? FifoFd : VenueFd);
		tv.tv_sec = 1; tv.tv_usec = 0;

		rt = select (maxfd + 1, &rf, NULL, NULL, &tv);
		if (rt < 0) { if (SYS_NO == EINTR) continue; break; }
		if (rt == 0) continue;					/* timeout → START_S 재확인 */

		/* 거래원 → 체결통지 수신 */
		if (VenueFd >= 0 && FD_ISSET (VenueFd, &rf))
			On_Venue_Recv ();

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
	char *h = getenv ("VX_FX_HOST");
	char *p = getenv ("VX_FX_PORT");
	memset (VenueHost, 0, sizeof (VenueHost));
	strncpy (VenueHost, (h && *h) ? h : "127.0.0.1", sizeof (VenueHost) - 1);
	VenuePort = (p && *p) ? atoi (p) : 19100;
	Log (USR_OK, "Init: venue target [%s:%d]", VenueHost, VenuePort);
	return (OK);
}

/*----------------------------------------------------------------------*/
int		Venue_Connect (void)
/*----------------------------------------------------------------------*/
{
	struct sockaddr_in sa;
	VenueFd = socket (AF_INET, SOCK_STREAM, 0);
	if (VenueFd < 0) { Log (USR_ERROR, "socket fail {%d:%s}", SYS_NO, SYS_STR); return (NOTOK); }
	memset (&sa, 0, sizeof (sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr (VenueHost);
	sa.sin_port = htons ((unsigned short) VenuePort);
	if (connect (VenueFd, (struct sockaddr *) &sa, sizeof (sa)) < 0)
	{
		Log (USR_ERROR, "venue connect fail [%s:%d] {%d:%s}",
			  VenueHost, VenuePort, SYS_NO, SYS_STR);
		close (VenueFd); VenueFd = -1; return (NOTOK);
	}
	Log (USR_OK, "venue connected [%s:%d] fd=%d", VenueHost, VenuePort, VenueFd);
	return (OK);
}

/*----------------------------------------------------------------------*/
int		Open_Order_Fifo (void)
/*----------------------------------------------------------------------*/
{
	char *path = getenv ("VX_FX_ORDER_FIFO");
	if (!path || !*path)
	{ Log (USR_OK, "no VX_FX_ORDER_FIFO (트리거 없음, 수신 전용 동작)"); FifoFd = -1; return (OK); }
	/* O_RDWR 로 열어 writer 부재에도 select 가능(EOF 방지) */
	FifoFd = open (path, O_RDWR | O_NONBLOCK);
	if (FifoFd < 0)
	{ Log (USR_ERROR, "order fifo open fail [%s] {%d:%s}", path, SYS_NO, SYS_STR); return (NOTOK); }
	Log (USR_OK, "order trigger fifo [%s] fd=%d", path, FifoFd);
	return (OK);
}

/* 주문요청 1줄("ACCT,SYMBOL,SIDE,QTY,PX,CLORDID") → SMB_ST 'D' 송신 */
/*----------------------------------------------------------------------*/
void	On_Order_Request (char *line)
/*----------------------------------------------------------------------*/
{
	SMB_ST	o;
	char	acct[31], sym[8], sidebuf[8], qty[31], px[31], clord[25];
	char	*tok;
	int		f = 0;

	acct[0]=sym[0]=sidebuf[0]=qty[0]=px[0]=clord[0]=0;
	for (tok = strtok (line, ","); tok; tok = strtok (NULL, ","), f++)
	{
		switch (f) {
		case 0: strncpy (acct, tok, 30);    break;
		case 1: strncpy (sym,  tok, 7);     break;
		case 2: strncpy (sidebuf, tok, 1);  break;
		case 3: strncpy (qty,  tok, 30);    break;
		case 4: strncpy (px,   tok, 30);    break;
		case 5: strncpy (clord, tok, 24);   break;
		}
	}
	if (!clord[0]) snprintf (clord, sizeof (clord), "FEPFX%010d", OrdSeq++);
	if (!sidebuf[0]) sidebuf[0] = '1';

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

	if (write (VenueFd, &o, SMB_SZ) != SMB_SZ)
	{ Log (USR_ERROR, "order write fail {%d:%s}", SYS_NO, SYS_STR); return; }
	Log (USR_OK, "FX ORDER send ClOrdID=%s Symbol=%s Side=%c Qty=%s Px=%s",
		  clord, sym[0]?sym:"USD/KRW", o.smb_Side[0], qty[0]?qty:"1000000", px[0]?px:"1385.50");
}

/* 거래원 소켓 → SMB_ST 체결통지 수신·기록(고정 프레임 누적) */
/*----------------------------------------------------------------------*/
void	On_Venue_Recv (void)
/*----------------------------------------------------------------------*/
{
	static char	acc[8192];
	static int	alen = 0;
	int		rt;

	rt = recv (VenueFd, acc + alen, sizeof (acc) - alen, 0);
	if (rt <= 0)
	{
		Log (USR_ERROR, "venue recv %s {%d:%s}", rt==0?"closed":"fail", SYS_NO, SYS_STR);
		close (VenueFd); VenueFd = -1; alen = 0; return;
	}
	alen += rt;
	while (alen >= SMB_SZ)
	{
		SMB_ST *e = (SMB_ST *) acc;
		char cl[25], oid[31], eid[31], cum[31], lpx[31];
		field (cl,  e->smb_ClOrdID, 24);
		field (oid, e->smb_OrdID,   30);
		field (eid, e->smb_ExecID,  30);
		field (cum, e->smb_CumQty,  30);
		field (lpx, e->smb_LastPx,  30);
		Log (USR_OK, "FX EXEC recv MsgType=%c ExecType=%c OrdStatus=%c ClOrdID=%s OrdID=%s ExecID=%s CumQty=%s LastPx=%s",
			  e->smb_MsgType[0], e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oid, eid, cum, lpx);
		if (getenv ("VX_TEST") != NULL)
			Log (USR_OK, "VX_TEST exec ExecType=%c OrdStatus=%c ClOrdID=%s OrdID=%s CumQty=%s",
				 e->smb_ExecType[0], e->smb_OrdStatus[0], cl, oid, cum);
		memmove (acc, acc + SMB_SZ, alen - SMB_SZ);
		alen -= SMB_SZ;
	}
}
