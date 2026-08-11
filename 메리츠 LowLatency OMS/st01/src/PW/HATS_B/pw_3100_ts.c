#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Online 송신 (TCP client)
#	File	: pw_3100_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	60

#define		FIFO_EVENT		0
#define		SOCKET_EVENT	1
#define		FILE_EVENT		2

#define		P_TYPE			TS_R1_1

#ifdef SAM_USE
#define		WR_CNT			W_CNT(0,0)
#define		RD_CNT			R_CNT(0,0)
#define		IN_NAME			IFN(D_K,P_K,0)
#else
#define		WR_CNT			IDW_CNT(0,0)
#define		RD_CNT			IDR_CNT(0,0)
#define		IN_NAME			IDN(D_K,P_K,0)
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, PortNo, DataCnt, PollCnt, PktType, MaxCnt;
int		LegSize, RetryCnt, Line;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char	IpAddr[20], DeviceSendFlag, R_Buf[FILE_BUF_LEN];

struct pollfd	Poll[3];
TCP_HEAD		*R_Pkt = (TCP_HEAD *)RecvPkt;
TCP_MESSAGE		*S_Pkt = (TCP_MESSAGE *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_3100_TS (void);
void	Init_Parameters (void);
void	Connection (void);
void	Communicate_Routine (void);
void	Fifo_Event_Rtn (void);
int  	Receive_Packet (void);
int		Check_Header (void);
void	File_Event_Rtn (void);
void	Send_Packet (void);
void	Register_Signal (void);
void	Catch_Signal (int);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);

	PW_3100_TS ();

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_3100_TS (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt;

	Register_Signal ();
	Init_Parameters ();

	PktType = T_LINK;
	Connection ();
	Send_Packet ();

	while (1)
	{
		rt = Receive_Packet ();

		if (rt == FAIL)
			continue;
		else if (rt == NOTOK)
			return;
		else
			break;
	}

	close (Sockfd);
	PortNo = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));
	PktType = T_STRT;
	Connection ();
	Send_Packet ();

	while (1)
	{
		rt = Receive_Packet ();

		if (rt == FAIL)
			continue;
		else if (rt == NOTOK)
			return;
		else
			break;
	}

	Communicate_Routine ();

	return;
}	/* End of PW_3100_TS ()	*/

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . initiate the global variables
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	int     rt;

	SYS_NO = 0;
	S_K = 0;
	RetryCnt = 0;
	DeviceSendFlag = OFF;

	Poll[0].fd = START_FD;
	Poll[0].events = POLLIN;
	Poll[2].fd = INPUT_FD;
	Poll[2].events = POLLIN;

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	sprintf (ApType, "JC%-4.4s%c%c",
		_Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
	LtoU (ApType, strlen (ApType));

#ifdef SAM_USE
	LegSize = HEAD_SIZE + IFS(D_K,P_K,0);
#else
	LegSize = HEAD_SIZE + IDS(D_K,P_K,0);
#endif

	MaxCnt = TCP_DATA_LEN / LegSize;

	if (MaxCnt > 8)
		MaxCnt = 8;

	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
		TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
	PortNo = TCP2_PORT_NO;

	Line = AtoIf (_Exe_Name+5, 2) - ACC_NO_BASE;

	return;
}	/* End of Init_Parameters ()	*/

/*************************************************************************
	Function		: . Connection
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . connect to server
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Connection (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	while (START_S != END)
	{
		Sockfd = Socket ();

		if (Sockfd < 0)
		{
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
				Sockfd, SYS_NO, SYS_STR);
			sleep (5);
			continue;
		}

		TCP2_LINE_ST = ON;
		Log (USR_OK, "socket created:Sockfd[%d]", Sockfd);
		Log (USR_OK, "connecting to %s:%d", IpAddr, PortNo);

		rt = Connect (Sockfd, IpAddr, PortNo);

		if (rt < 0)
		{
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			TCP2_NET_STA(S_K) = OFF;
			Log (TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
			close (Sockfd);

			if (PktType == T_STRT)
				Exit_Process ();
			else
			{
				if (RetryCnt > 5)
				{
					S_K = (S_K + 1) % 2;
					sprintf (IpAddr, "%d.%d.%d.%d",
						TCP2_IP1(D_K,P_K,S_K), TCP2_IP2(D_K,P_K,S_K),
						TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
					PortNo = TCP2_PORT_NO;
					RetryCnt = 0;

					Log (TCP_OK, "port changed to %s (%s:%d)",
						S_K == 0 ? "main" : "backup", IpAddr, PortNo);
				}

				RetryCnt ++;
				sleep (3);
				continue;
			}
		}

		if (PktType == T_STRT)
			TCP2_CON_STA = ON;

		TCP2_NET_STA(S_K) = ON;
		Log (USR_OK, "connected to %s:%d", IpAddr, PortNo);
		break;
	}

	Set_Socket_Linger ();

	return;
}	/* End of Connection ()	*/

/*************************************************************************
	Function		: . Communicate_Routine
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . communicate routine
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Communicate_Routine (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;
	char	t_time[12];

	Poll[1].fd = Sockfd;
	Poll[1].events = POLLIN;

	while (START_S < JOB_END)
	{
		Stat_Save ();

		if (DeviceSendFlag == ON)
			PollCnt = 2;
		else
		{
			while (START_S < JOB_END)
			{
				if (WR_CNT > RD_CNT)
				{
					File_Event_Rtn ();
					continue;
				}
				else
					break;
			}

			PollCnt = 3;
		}

		rt = poll (Poll, PollCnt, TIME_OUT * 1000);

		if (rt > 0)
		{
			for (i = 0; i < PollCnt; i ++)
			{
				if (Poll[i].revents & POLLIN)
				{
					Poll[i].revents = 0;
					break;
				}

				if (Poll[i].revents & POLLHUP)
				{
					if (i == SOCKET_EVENT)
					{
						Log (TCP_ERROR, "socket disconnected[%#06x]",
							Poll[i].revents);
						return;
					}

					Log (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
					continue;
				}

			}
		}
		else
		{
			if (rt == 0)
			{
				if (PollCnt == 2)
				{
					Log (TCP_ERROR, "poll timeout (no response)");
					return;
				}

				PktType = T_POLL;
				Send_Packet ();
				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					Log (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

				continue;
			}
		}

		switch (i)
		{
			case	FIFO_EVENT:
				Fifo_Event_Rtn ();
				break;
			case	SOCKET_EVENT:
				rt = Receive_Packet ();
				if (rt == NOTOK)
					return;
				break;
			case	FILE_EVENT:
				File_Event_Rtn ();

				while (START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						File_Event_Rtn ();
						continue;
					}
					else
						break;
				}
				break;
			default:
				Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}

	return;
}	/* End of Communicate_Routine ()	*/

/*************************************************************************
	Function		: . Fifo_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char	tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn ()	*/

/*************************************************************************
	Function		: . Receive_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, 1:interrupted)
	Comment			: . receive Receive_Packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, recv_len;

	memset (RecvPkt, 0, sizeof (RecvPkt));

	rt = Select_Receive (Sockfd, RecvPkt);

	if (rt == OK)
		return (NOTOK);
	else if (rt == NOTOK)
	{
		if (SYS_NO == EINTR)
			return (FAIL);

		return (NOTOK);
	}

	DeviceSendFlag = OFF;
	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt)
	{
		Log (USR_ERROR, "invalid length[%d:%d,%d]", recv_len, TCP_HEAD_LEN, rt);
		return (NOTOK);
	}

	if (memcmp (R_Pkt->MsgType, TCP_POOK_CD, strlen (TCP_POOK_CD)) == 0)
	{
		if (INT_SEQ != AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo)))
			PROC(D_K,P_K).counter_seq =
				AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));
	}

	rt = Check_Header ();

	if (rt == OK)
	{
		if (memcmp (R_Pkt->MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) == 0)
			INT_SEQ = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));
	}
	else
	{
		sleep (3);
		return (NOTOK);
	}

	return (OK);
}	/* Receive_Packet ()	*/

/*************************************************************************
	Function		: . Check_Header
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, 1:error write)
	Comment			: . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Header (void)
/*----------------------------------------------------------------------*/
{
	int		data_len, i, next, rt, val;

	/* Stx (0x02)	*/
	if (R_Pkt->Stx[0] != STX)
	{
		Log (USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Stx[0], STX);
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Length, sizeof (R_Pkt->Length));
	if (val != strlen (RecvPkt))
	{
		Log (USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		return (NOTOK);
	}

	/* ApType (업무구분식별자)	*/
	if (memcmp (R_Pkt->ApType, ApType, sizeof (R_Pkt->ApType)) != 0)
	{
		Log (USR_ERROR, "TH.ApType [%.8s:%s]", R_Pkt->ApType, ApType);
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0 &&
		PktType != T_POLL)
	{
		Log (USR_ERROR, "TH.ResponseCode [%.4s:%04d]", R_Pkt->ResponseCode, 0);
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if ((memcmp (R_Pkt->MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0 &&
		memcmp (R_Pkt->MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0 &&
		memcmp (R_Pkt->MsgType, TCP_POOK_CD, strlen (TCP_POOK_CD)) != 0) ||
		(PktType == T_LINK &&
		memcmp (R_Pkt->MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0) ||
		(PktType == T_STRT &&
		memcmp (R_Pkt->MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0) ||
		(PktType == T_POLL &&
		memcmp (R_Pkt->MsgType, TCP_POOK_CD, strlen (TCP_POOK_CD)) != 0))
	{
		Log (USR_ERROR, "TH.MsgType [%.4s]", R_Pkt->MsgType);
		return (NOTOK);
	}

	return (OK);
}	/* End of Check_Header ()	*/

/*************************************************************************
	Function		: . File_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . read file and send data
*************************************************************************/
/*----------------------------------------------------------------------*/
void	File_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	tmp[128];

	memset (R_Buf, 0, sizeof (R_Buf));

#if (0)
/* Test Log	*/
Log (USR_OK, "Data Read");
#endif

#ifdef SAM_USE
	DataCnt = F_R (P_TYPE, R_Buf, MaxCnt);
#else
	DataCnt = DSHM_R (P_TYPE, R_Buf, MaxCnt);
#endif

	if (DataCnt < 0 || DataCnt > MaxCnt)
	{
		Log (SAM_FATAL, "cannot read file[%s]", IN_NAME);
		sleep (1);
		TCP2_CON_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}
	else if (DataCnt == 0)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));
		return;
	}


	PktType = T_DATA;
	Send_Packet ();

	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		if (rt == 0)
			break;
	}

	return;
}	/* File_Event_Rtn ()	*/

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt, data_len, next, f_len;
	char	t_time[12];

	/* BUFF_RW_HEAD(70) - HEAD_SIZE(20) = 50    */
	f_len = sizeof (BUFF_RW_HEAD) - HEAD_SIZE;

	memset (SendPkt, 0, sizeof (SendPkt));
	memset (SendPkt, ' ', TCP_HEAD_LEN);

	S_Pkt->Head.Stx[0] = STX;
	ItoAf (TCP_HEAD_LEN, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
	memcpy (S_Pkt->Head.ApType, ApType, sizeof (S_Pkt->Head.ApType));
	memcpy (S_Pkt->Head.Date, DATE_CURR, sizeof (S_Pkt->Head.Date));
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Head.Time, t_time, sizeof (S_Pkt->Head.Time));
	ItoAf (0, S_Pkt->Head.ResponseCode, sizeof (S_Pkt->Head.ResponseCode));
	ItoAf (0, S_Pkt->Head.DataCnt, sizeof (S_Pkt->Head.DataCnt));


	if ( PktType == T_DATA )
	{
		memcpy (S_Pkt->Head.MsgType, TCP_DATA_CD,
			sizeof (S_Pkt->Head.MsgType));

		for (i = 0, next = 0; i < DataCnt; i ++)
		{
			pTcp_Data_Head =
				(TCP_DATA_HEAD *)&R_Buf[(f_len+LegSize+1)*i+f_len];
			pJmData = 
				(KRX_JUMUN_DATA *)&R_Buf[(f_len+LegSize+1)*i+f_len+20+KRX_HEAD_LEN];

			data_len = AtoIf (pTcp_Data_Head->Length,
				sizeof (pTcp_Data_Head->Length));
			memcpy (&S_Pkt->Data[next],
				&R_Buf[(f_len+LegSize+1)*i+f_len], data_len);

			/* ApType별 주문번호 채번(10만번대만 사용하기 때문에 6자리만 쓴다) */
/*
			ACCNO(D_K, AtoIf(&pJmData->MembershipItem[3], 2) - ACC_NO_BASE).js_order_no_band ++;
			ItoAf (ACCNO(D_K,
			AtoIf(&pJmData->MembershipItem[3], 2) - ACC_NO_BASE).js_order_no_band,
					&S_Pkt->Data[next+20+82+33], 6);
*/
			ACCNO(D_K, Line).js_order_no_band ++;
			ItoAf (ACCNO(D_K, Line).js_order_no_band,
					&S_Pkt->Data[next+20+82+33], 6);

			pTcp_Data_Head = (TCP_DATA_HEAD *)&S_Pkt->Data[next];
			ItoAf (INT_SEQ + i + 1, pTcp_Data_Head->DataSeq,
				sizeof (pTcp_Data_Head->DataSeq));
			next += data_len;
		}

		ItoAf (strlen (SendPkt), S_Pkt->Head.Length,
			sizeof (S_Pkt->Head.Length));
		ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
		ItoAf (DataCnt, S_Pkt->Head.DataCnt, sizeof (S_Pkt->Head.DataCnt));
	}
	else
	{
		switch (PktType)
		{
			case	T_LINK:
				memcpy (S_Pkt->Head.MsgType, TCP_LINK_CD,
					sizeof (S_Pkt->Head.MsgType));
				ItoAf (0, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
				break;
			case	T_STRT:
				memcpy (S_Pkt->Head.MsgType, TCP_STRT_CD,
					sizeof (S_Pkt->Head.MsgType));
				ItoAf (0, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
				break;
			case	T_POLL:
				memcpy (S_Pkt->Head.MsgType, TCP_POLL_CD,
					sizeof (S_Pkt->Head.MsgType));
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
				break;
			case	T_STOP:
				memcpy (S_Pkt->Head.MsgType, TCP_STOP_CD,
					sizeof (S_Pkt->Head.MsgType));
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
			default:
				break;
		}
	}

	rt = Select_Send (Sockfd, SendPkt, strlen (SendPkt));
	if (rt != OK)
	{
		TCP2_CON_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}

/*
	select 처리 안하고 send처리 하기위함.
	rt = Sendn (Sockfd, SendPkt, strlen (SendPkt));
	if (rt <= 0)
	{
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		TCP2_CON_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}
*/

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	if (PktType == T_DATA)
	{
		Set_TR_Time ();
		INT_SEQ += DataCnt;
	}
	else
		DeviceSendFlag = ON;

	return;
}	/* Send_Packet ()	*/

/*************************************************************************
	Function		: . Register_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = Catch_Signal;

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
}	/* End of Register_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred", signo);

	if (signo == SIGTERM)
	{
		PktType = T_STOP;
		Send_Packet ();
	}

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of Catch_Signal ()	*/

/*************************************************************************
	Function		: . Set_Socket_Linger
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set linger option on socket
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Socket_Linger (void)
/*----------------------------------------------------------------------*/
{
	int				rt;
	struct linger	ling;

    /* close () returns after discarding any unsent data	*/
	ling.l_onoff = 1;
	ling.l_linger = 0;

	rt = setsockopt (Sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling,
		sizeof (ling));
	if (rt < 0)
		Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}

/*************************************************************************
	End of Program (pw_3100_ts.c)
*************************************************************************/
