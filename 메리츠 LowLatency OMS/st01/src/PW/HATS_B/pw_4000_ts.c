#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Online 업무 송신
#	File	: pw_4000_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    10

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
int		Sockfd, DataCnt, PollCnt, PktType, MaxCnt, LegSize;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char	DeviceSendFlag, DataSendFlag, R_Buf[FILE_BUF_LEN];

struct pollfd	Poll[3];
TCP_HEAD		*R_Pkt = (TCP_HEAD *)RecvPkt;
TCP_MESSAGE		*S_Pkt = (TCP_MESSAGE *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_4000_TS (int, char **);
void	Init_Parameters (void);
void	Communicate_Routine (void);
void	Fifo_Event_Rtn (void);
int		Receive_Packet (void);
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
	PW_4000_TS (argc, argv);

	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_4000_TS (int argc, char **argv)
/*----------------------------------------------------------------------*/
{
	int		i, rt, seq;

	Register_Signal ();
	Init_Parameters ();

	if (argc != 3)
	{
		Log (USR_FATAL, "invalid arguments number[%d]", argc);
		Exit_Process ();
	}

	Sockfd = AtoIf (argv[1], strlen (argv[1]));
	PROC(D_K,P_K).l.t1.line_gubun = AtoIf (argv[2], 1);
	PROC(D_K,P_K).l.t1.port_type = AtoIf (&argv[2][2], 1);
	Log (USR_OK, "Sockfd[%d] line_gubun[%d] port_type[%d]",
		Sockfd, PROC(D_K,P_K).l.t1.line_gubun, PROC(D_K,P_K).l.t1.port_type);

	Set_Socket_Linger ();

	PktType = T_STOK;
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

	if (memcmp (R_Pkt->MsgType, TCP_RSND_CD, strlen (TCP_RSND_CD)) == 0)
	{
		seq = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));

		if (seq > WR_CNT || seq < 0)
		{
			PROC(D_K,P_K).counter_seq = seq;
			Log (USR_ERROR, "RSND:invalid SeqNo[%d:%d]", seq, WR_CNT);
			PktType = T_RSOK;
			Send_Packet ();
			sleep (3);
			return;
		}

		Log (USR_OK, "received RSND request[%d]", seq);
		RD_CNT = seq;
		INT_SEQ = seq;
	}
	else
		return;

	Communicate_Routine ();

	return;
}	/* End of PW_4000_TS ()	*/

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
	SYS_NO = 0;
	DeviceSendFlag = OFF;
	DataSendFlag = OFF;

	Poll[0].fd = START_FD;
	Poll[0].events = POLLIN;
	Poll[2].fd = INPUT_FD;
	Poll[2].events = POLLIN;

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

#ifdef SAM_USE
	LegSize = HEAD_SIZE + IFS(D_K,P_K,0);
#else
	LegSize = HEAD_SIZE + IDS(D_K,P_K,0);
#endif

#ifdef BLOCK_MAX
	MaxCnt = BLOCK_MAX;
#else
	MaxCnt = (TCP_DATA_LEN / LegSize);
#endif

	if (MaxCnt > 45)
		MaxCnt = 45;

	Log (USR_OK, "max block count [%d]", MaxCnt);

	return;
}	/* End of Init_Parameters ()	*/

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
			if (WR_CNT > RD_CNT)
			{
				File_Event_Rtn ();
				continue;
			}

			PollCnt = 3;
		}

		rt = poll (Poll, PollCnt, TIME_OUT * 1000);

		if (rt < 0)
		{
			Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
			continue;
		}
		else if (rt == 0)
		{
			if (PollCnt == 2)
			{
				Log (TCP_ERROR, "poll timeout (no response)");

				if (DataSendFlag == ON)
					RD_CNT -= DataCnt;

				return;
			}

			PktType = T_POLL;
			Send_Packet ();
			continue;
		}

		for (i = 0; i < PollCnt; i ++)
		{
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

		for (i = 0; i < PollCnt; i ++)
		{
			if (Poll[i].revents & POLLIN)
			{
				Poll[i].revents = 0;
				break;
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
	Return Code		: . int 
	Comment			: . receive data from socket and send response
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, recv_len, seq;

	memset (RecvPkt, 0, sizeof (RecvPkt));

	rt = Select_Receive (Sockfd, RecvPkt);

	if (rt == OK)
	{
		if (DataSendFlag == ON)
			RD_CNT -= DataCnt;

		return (NOTOK);
	}
	else if (rt == NOTOK)
	{
		if (SYS_NO == EINTR)
			return (FAIL);

		if (DataSendFlag == ON)
			RD_CNT -= DataCnt;

		return (NOTOK);
	}

	DeviceSendFlag = OFF;
	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	recv_len = strlen (RecvPkt);

	if (recv_len != TCP_HEAD_LEN || recv_len != rt)
	{
		Log (USR_ERROR, "invalid length[%d:%d,%d]",
			recv_len, TCP_HEAD_LEN, rt);

		if (PktType == T_DATA)
			RD_CNT -= DataCnt;

		return (NOTOK);
	}

	if (memcmp (R_Pkt->MsgType, TCP_POOK_CD, strlen (TCP_POOK_CD)) == 0)
	{
		seq = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));

		if (seq != INT_SEQ)
			Log (USR_WARN, "POOK:SeqNo [%d:%d]", seq, INT_SEQ);

		return (OK);
	}
	else if (memcmp (R_Pkt->MsgType, TCP_STOP_CD, strlen (TCP_STOP_CD)) == 0)
	{
		Log (USR_OK, "received STOP request");
		TCP1_NET_STA = END;
		close (Sockfd);
		Exit_Process ();
	}
	else if (memcmp (R_Pkt->MsgType, TCP_RSND_CD, strlen (TCP_RSND_CD)) == 0)
		return (OK);
	else
	{
		rt = Check_Header ();

		if (rt == OK)
		{
			Set_TR_Time ();
			INT_SEQ += DataCnt;
			DataSendFlag = OFF;
		}
		else
		{
			if (DataSendFlag == ON)
				RD_CNT -= DataCnt;

			return (NOTOK);
		}
	}

	return (OK);
}	/* Receive_Packet ()	*/

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
	int		val;

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Length, sizeof (R_Pkt->Length));
	if (val != strlen (RecvPkt))
	{
		Log (USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0)
	{
		Log (USR_ERROR, "TH.ResponseCode [%-4.4s:%04d]",
			R_Pkt->ResponseCode, 0);
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if (memcmp (R_Pkt->MsgType, TCP_DAOK_CD, strlen (TCP_DAOK_CD)) != 0 ||
		(DataSendFlag == OFF &&
		memcmp (R_Pkt->MsgType, TCP_DAOK_CD, strlen (TCP_DAOK_CD)) == 0))
	{
		Log (USR_ERROR, "TH.MsgType [%.4s:%s]", R_Pkt->MsgType, TCP_DAOK_CD);
		return (NOTOK);
	}

	/* DataCnt (한 packet 내 blocking된 data 건수)	*/
	val = AtoIf (R_Pkt->DataCnt, sizeof (R_Pkt->DataCnt));

	if (val < 0 || val > MaxCnt)
	{
		 Log (USR_ERROR, "TH.DataCnt [%d:%d]", val, MaxCnt);
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
	char	tmp[2];

	while (1)
	{
		rt = read (INPUT_FD, tmp, 1);

		if (rt == 0)
			break;
	}

	memset (R_Buf, 0, sizeof (R_Buf));

#ifdef SAM_USE
	DataCnt = F_R (P_TYPE, R_Buf, MaxCnt);
#else
	DataCnt = DSHM_R (P_TYPE, R_Buf, MaxCnt);
#endif

	if (DataCnt < 0 || DataCnt > MaxCnt)
	{
		Log (USR_FATAL, "cannot read file[%s]", IN_NAME);
		sleep (1);
		TCP1_NET_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}
	else if (DataCnt == 0)
		return;

	PktType = T_DATA;
	Send_Packet ();

	return;
}	/* File_Event_Rtn ()	*/

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt, data_len, next, f_len;
	char			t_time[12], recvtm[16];
	BUFF_RW_HEAD	*f_head;

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
	ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
	ItoAf (0, S_Pkt->Head.DataCnt, sizeof (S_Pkt->Head.DataCnt));

	switch (PktType)
	{
		case	T_STOK:
			memcpy (S_Pkt->Head.MsgType, TCP_STOK_CD,
				sizeof (S_Pkt->Head.MsgType));
			break;
		case	T_POLL:
			memcpy (S_Pkt->Head.MsgType, TCP_POLL_CD,
				sizeof (S_Pkt->Head.MsgType));
			break;
		case	T_RSOK:
			memcpy (S_Pkt->Head.ResponseCode,
				ERR_HEAD_T8, strlen (ERR_HEAD_T8));
			memcpy (S_Pkt->Head.MsgType, TCP_RSOK_CD,
				sizeof (S_Pkt->Head.MsgType));
			break;
		case	T_DATA:
			memcpy (S_Pkt->Head.MsgType, TCP_DATA_CD,
				sizeof (S_Pkt->Head.MsgType));

			for (i = 0, next = 0; i < DataCnt; i ++)
			{
				pTcp_Data_Head =
					(TCP_DATA_HEAD *)&R_Buf[(f_len+LegSize+1)*i+f_len];
				data_len = AtoIf (pTcp_Data_Head->Length,
					sizeof (pTcp_Data_Head->Length));
				memcpy (&S_Pkt->Data[next],
					&R_Buf[(f_len+LegSize+1)*i+f_len], data_len);

				pTcp_Data_Head = (TCP_DATA_HEAD *)&S_Pkt->Data[next];
				ItoAf (INT_SEQ + i + 1, pTcp_Data_Head->DataSeq,
					sizeof (pTcp_Data_Head->DataSeq));

				f_head = (BUFF_RW_HEAD *)&R_Buf[(f_len+LegSize+1)*i];

				if (i == 0)
					sprintf (recvtm, "%12.12s", f_head->RecvTime2);

#if defined PROC_CLI
				next += data_len;
#else
				memcpy (pTcp_Data_Head->ResponseCode, f_head->ResponseCode,
					sizeof (f_head->ResponseCode));
				next += data_len;
#endif
			}

			ItoAf (strlen (SendPkt), S_Pkt->Head.Length,
				sizeof (S_Pkt->Head.Length));
			ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo, sizeof (S_Pkt->Head.SeqNo));
			ItoAf (DataCnt, S_Pkt->Head.DataCnt, sizeof (S_Pkt->Head.DataCnt));
			break;
		default:
			break;
	}

	rt = Select_Send (Sockfd, SendPkt, strlen (SendPkt));

	if (rt != OK)
	{
		if (PktType == T_DATA)
			RD_CNT -= DataCnt;

		TCP1_NET_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}

	if (PktType == T_DATA)
		Log (TCP_OK, "TCP SD [%s][%s](%d)<%d>",
			recvtm, SendPkt, strlen (SendPkt), INT_SEQ);
	else
		Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	if (PktType == T_STOK)
		TCP1_NET_STA = ON;

	DeviceSendFlag = ON;

	if (PktType == T_DATA)
		DataSendFlag = ON;
	else
		DataSendFlag = OFF;

	return;
}	/* Send_Packet ()	*/

/*************************************************************************
	Function		: . Register_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register signals
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
	TCP1_NET_STA = OFF;
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
	End of Program (pw_4000_ts.c)
*************************************************************************/
