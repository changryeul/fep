#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: TCP/IP 업무접속 Master
#	File	: pw_1000_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	10
#define		PORT_NO			TCP1_PORT(D_K,P_K)
#define		S_CT(pos)		TCP1_S_CT(D_K,P_K,pos)
#define		S_ST(pos)		TCP1_S_ST(D_K,P_K,pos)
#define		SPORT(pos)		TCP1_SPORT(D_K,P_K,pos)

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Sockfd, Newfd;
char		RecvPkt[TCP_MESSAGE_MAX_LEN], SendPkt[TCP_MESSAGE_MAX_LEN];
char		ClntIp[20], TmpIp[20];
TCP_HEAD	*S_Pkt = (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_1000_TR (void);
void    Init_Parameters (void);
void	Device_Open (void);
void	Communicate_Routine (void);
int		Send_Packet (void);
int		Check_Header (void);
short	Minimize_Port_Index (void);
void 	Register_Parent_Signal (void);
void 	Catch_Parent_Signal (int);
void 	Register_Child_Signal (void);
void 	Catch_Child_Signal (int);
int		Check_Client (char *);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PW_1000_TR ();

	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_1000_TR (void)
/*----------------------------------------------------------------------*/
{
	u_short		clnt_port;
	int			pid, rt;

	memset (TmpIp, 0, sizeof (TmpIp));

	Init_Parameters ();
	Log (USR_OK, "PORT_NO[%d]", PORT_NO);

	Register_Parent_Signal ();
	Device_Open ();

	TCP1_NET_STA = ON;

	while (1)
	{
		Log (USR_OK, "accepting ...");
		memset (ClntIp, 0, sizeof (ClntIp));

		Newfd = Accept (Sockfd, ClntIp, &clnt_port);

		if (Newfd < 0)
		{
			if (SYS_NO == ECONNABORTED)
			{
				usleep (100000);
				continue;
			}

			Log (TCP_ERROR, "accept[%d] {%d:%s}", Newfd, SYS_NO, SYS_STR);
			return;
		}

		Log (USR_OK, "accept[%d,%d]", Sockfd, Newfd);

		pid = fork ();

		if (pid == 0)
		{
			Register_Child_Signal ();
			Communicate_Routine ();
			close (Newfd);
			exit (OK);
		}
		else if (pid < 0)
		{
			Log (SYS_ERROR, "fork failure {%d:%s}", SYS_NO, SYS_STR);
			usleep (100000);
			return;
		}

		close (Newfd);
	}

	return;
}	/* End of PW_1000_TR ()	*/

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . initialize parameters
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	signal (SIGCHLD, SIG_IGN);

	SYS_NO = 0;

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	return;
}	/* End of Init_Parameters ()	*/

/*************************************************************************
	Function		: . Device_Open
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . open device (create socket, bind and listen)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Open (void)
/*----------------------------------------------------------------------*/
{
	int			rt;
	const int	on = 1;

	Sockfd = Socket ();

	if (Sockfd < 0)
	{
		Log (TCP_ERROR, "socket[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
		TCP1_NET_STA = OFF;
		Exit_Process ();
	}

	Log (USR_OK, "socket[%d]", Sockfd);

	rt = setsockopt (Sockfd, SOL_SOCKET, SO_REUSEADDR,
		(char *)&on, sizeof (on));

	if (rt < 0)
	{
		Log (TCP_ERROR, "setsockopt[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
		shutdown (Sockfd, SHUT_RDWR);
		TCP1_NET_STA = OFF;
		Exit_Process ();
	}

	rt = Bind (Sockfd, PORT_NO);

	if (rt < 0)
	{
		Log (TCP_ERROR, "bind[%d][%d] {%d:%s}", rt, PORT_NO, SYS_NO, SYS_STR);
		close (Sockfd);
		TCP1_NET_STA = OFF;
		Exit_Process ();
	}

	Log (USR_OK, "bind[%d]", Sockfd);

	Listen (Sockfd);
	Log (USR_OK, "listen[%d]", Sockfd);

	return;
}	/* Device_Open ()	*/

/*************************************************************************
	Function		: . Communicate_Routine
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . receive and send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Communicate_Routine (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	while (1)
	{
		memset (RecvPkt, 0, sizeof (RecvPkt));

		rt = Select_Receive (Newfd, RecvPkt);

		if (rt == OK)
			continue;
		else if (rt == NOTOK)
			return;
		else
			break;
	}

	Log (USR_OK, "TCP RD [%s](%d)", RecvPkt, strlen (RecvPkt));

	rt = Send_Packet ();

	if (rt == OK)
		Log (USR_OK, "TCP SD [%s](%d)", SendPkt, strlen (SendPkt));

	return;
}	/* End of Communicate_Routine ()	*/

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, port_idx;
	char	t_time[12];

	memset (SendPkt, 0, sizeof (SendPkt));
	memcpy (SendPkt, RecvPkt, TCP_HEAD_LEN);

	rt = Check_Header ();

	if (rt == OK)
	{
		while (1)
		{
			port_idx = Minimize_Port_Index ();

			if (port_idx != NOTOK)
				break;

			sleep (1);
		}

		S_CT(port_idx) ++;
		ItoAf (SPORT(port_idx), S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
	}

	S_Pkt->Stx[0] = STX;
	ItoAf (TCP_HEAD_LEN, S_Pkt->Length, sizeof (S_Pkt->Length));
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Time, t_time, sizeof (S_Pkt->Time));
	memcpy (S_Pkt->MsgType, TCP_LIOK_CD, sizeof (S_Pkt->MsgType));
	ItoAf (0, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	rt = Select_Send (Newfd, SendPkt, TCP_HEAD_LEN);

	return (rt);
}	/* End of Send_Packet ()	*/

/*************************************************************************
	Function		: . Check_Header
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Header (void)
/*----------------------------------------------------------------------*/
{
	int		rt, val;

	if (memcmp (TmpIp, ClntIp, strlen (ClntIp)) != 0)
	{
		rt = Check_Client (ClntIp);

		if (rt == NOTOK)
		{
			Log (TCP_ERROR, "%s not permitted to connect", ClntIp);
			memcpy (S_Pkt->ResponseCode, ERR_REJECT, strlen (ERR_REJECT));
			sleep (5);
			return (NOTOK);
		}

		memset (TmpIp, 0, sizeof (TmpIp));
		memcpy (TmpIp, ClntIp, strlen (ClntIp));
	}

	/* Stx (0x02)	*/
	if (S_Pkt->Stx[0] != STX)
	{
		Log (USR_ERROR, "header (Stx)[%#04x]", S_Pkt->Stx[0]);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (S_Pkt->Length, sizeof (S_Pkt->Length));
	if (val != strlen (RecvPkt) || strlen (RecvPkt) < TCP_HEAD_LEN)
	{
		Log (USR_ERROR, "header (Length)[%d:%d]", val, strlen (RecvPkt));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode)) != 0)
	{
		Log (USR_ERROR, "header (ResponseCode)[%-4.4s:%d]",
			S_Pkt->ResponseCode, 0);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if (memcmp (S_Pkt->MsgType, TCP_LINK_CD, sizeof (S_Pkt->MsgType)) != 0)
	{
		Log (USR_ERROR, "header (MsgType)[%-4.4s:%s]",
			S_Pkt->MsgType, TCP_LINK_CD);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return (NOTOK);
	}

	return (OK);
}	/* End of Check_Header ()	*/

/*************************************************************************
	Function		: . Minimize_Port_Index
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . short
	Comment			: . minimize port index
*************************************************************************/
/*----------------------------------------------------------------------*/
short	Minimize_Port_Index (void)
/*----------------------------------------------------------------------*/
{
	short	idx, sidx;
	int		min;
	pid_t	p_id;
	char	chk_proc[12];

	sidx = 0;
	min = S_CT(0);

	for (idx = 1; idx < 9; idx ++)
	{
		if (S_ST(idx) == ON && S_CT(idx) < min)
		{
			sidx = idx;
			min = S_CT(idx);
		}
	}

	sprintf (chk_proc, "%-6.6s%d_tr", _Exe_Name, sidx + 1);

	p_id = Check_Proc (chk_proc);

	if (p_id == 0)
	{
		S_ST(sidx) = 9;
		Log (USR_ERROR, "service process DOWN[%s]", chk_proc);
		return (NOTOK);
	}

	if (S_CT(sidx) > 1000)
	{
		for (idx = 0; idx < 9; idx ++)
			S_CT(idx) = 0;
	}

	return (sidx);
}	/* End of Minimize_Port_Index ()	*/

/*************************************************************************
	Function		: . Register_Parent_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register parent signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Parent_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = SA_RESTART;
	act.sa_handler = Catch_Parent_Signal;

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	act.sa_handler = SIG_IGN;

	if (sigaction (SIGPOLL, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGPOLL) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Parent_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Parent_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch parent signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Parent_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred to parent[%5d]", signo, getpid ());
	TCP1_NET_STA = OFF;
	close (Sockfd);
	close (Newfd);
	Exit_Process ();
}	/* End of Catch_Parent_Signal ()	*/

/*************************************************************************
	Function		: . Register_Child_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register child signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Child_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = Catch_Child_Signal;

	if (sigaction (SIGPIPE, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Child_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Child_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch child signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Child_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred to child[%5d]", signo, getpid ());
	TCP1_NET_STA = OFF;
	close (Newfd);
	exit (FAIL);
}	/* End of Catch_Child_Signal ()	*/

/*************************************************************************
	Function		: . Check_Client
	Parameters IN	: . client_ip	: client IP address
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . check client IP address
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Client (char *client_ip)
/*----------------------------------------------------------------------*/
{
	char	buf[128];
	char	*sp;
	FILE	*fp;

	sprintf (buf, "%s/clientip.ini", _FEP_CFG);

	if ((fp = fopen (buf, "r")) == 0)
	{
		Log (SAM_FATAL, "fopen failure[%s] {%d:%s}", buf, SYS_NO, SYS_STR);
		exit (FAIL);
	}

	while (1)
	{
		memset (buf, 0, sizeof (buf));

		sp = fgets (buf, sizeof (buf), fp);

		if (sp == NULL)
			break;

		buf[strlen(buf)-1] = 0;

		if (*buf == '#' || *buf == '\0' || *buf == '\t' || *buf == ' ')
			continue;

		if (memcmp (buf, client_ip, strlen (client_ip)) == 0)
		{
			fclose (fp);
			return (OK);
		}
	}

	fclose (fp);
	return (NOTOK);
}	/* End of Check_Client ()	*/

/*************************************************************************
	End of Program (pw_1000_tr.c)
*************************************************************************/
