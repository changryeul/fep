#define		_GLOBAL
/*------------------------------------------------------------------------
#   Module	: 선물옵션 주문응답/체결 수신 (TCP client)
#   File	: pw_4100_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	15

#ifdef SAM_USE
#define		WR_CNT			OFW_CNT(0,0)
#define		OUT_NAME		OFN(D_K,P_K,0)
#else
#define		WR_CNT			ODW_CNT(0,0)
#define		OUT_NAME		ODN(D_K,P_K,0)
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, PortNo, DataCnt, PktType, DataSize, MaxCnt, RetryCnt;
char	ApType[10], IpAddr[20], ErrCd[8];
char	RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];

TCP_MESSAGE		*R_Pkt = (TCP_MESSAGE *)RecvPkt;
TCP_HEAD		*S_Pkt = (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_4100_TR (void);
void	Init_Parameters (void);
void	Connection (void);
int		Receive_Packet (void);
void	Send_Packet (void);
int		Check_Header (void);
int		Write_Data (void);
void	Register_Signal (void);
void	Catch_Signal (int);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);

	PW_4100_TR ();

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_4100_TR (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

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
	PortNo = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));
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

	while (START_S < JOB_END)
	{
		Stat_Save ();

		rt = Receive_Packet ();

		if (rt == NOTOK)
			break;
		else if (rt == FAIL)
			continue;

		Send_Packet ();
	}

	return;
}	/* End of PW_4100_TR ()	*/

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
	S_K = 0;
	RetryCnt = 0;
	DataCnt = 0;

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	sprintf (ApType, "JC%-4.4s%c%c",
		_Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
	LtoU (ApType, strlen (ApType));

#ifdef SAM_USE
	DataSize = OFS(D_K,P_K,0);
#else
	DataSize = ODS(D_K,P_K,0);
#endif

	MaxCnt = TCP_DATA_LEN / (HEAD_SIZE + DataSize);
	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
		TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
	PortNo = TCP2_PORT_NO;

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
	char	error_flag;

	error_flag = OFF;
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

	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt ||
		recv_len > TCP_MESSAGE_MAX_LEN)
	{
		Log (USR_ERROR, "invalid length[%d,%d]", recv_len, rt);
		PktType = T_LNTH;
		DataCnt = 0;
		Send_Packet ();
		return (NOTOK);
	}

	rt = Check_Header ();

	if (rt != OK)
	{
		if (memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD,
			strlen (TCP_DATA_CD)) == 0)
		{
			PktType = T_DAOK;
			DataCnt = 0;
			Send_Packet ();
		}

		sleep (3);
		return (NOTOK);
	}

	if (memcmp (R_Pkt->Head.MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) == 0 ||
		memcmp (R_Pkt->Head.MsgType, TCP_RSOK_CD, strlen (TCP_RSOK_CD)) == 0)
		return (OK);
	else if (memcmp (R_Pkt->Head.MsgType, TCP_STOK_CD,
		strlen (TCP_STOK_CD)) == 0)
	{
		PktType = T_RSND;
		Send_Packet ();
	}
	else if (memcmp (R_Pkt->Head.MsgType, TCP_POLL_CD,
		strlen (TCP_POLL_CD)) == 0)
		PktType = T_POOK;
	else
	{
		PktType = T_DAOK;

		rt = Write_Data ();

		if (rt != OK)
		{
			sleep (3);
			DataCnt = 0;
			Send_Packet ();
			return (NOTOK);
		}
	}

	return (OK);
}	/* Receive_Packet ()	*/

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
	int		rt;
	char	t_time[12];

	memset (SendPkt, 0, sizeof (SendPkt));

	if (PktType == T_POOK || PktType == T_DAOK || PktType == T_LNTH)
		memcpy (SendPkt, RecvPkt, TCP_HEAD_LEN);
	else
		memset (SendPkt, ' ', TCP_HEAD_LEN);

	S_Pkt->Stx[0] = STX;
	ItoAf (TCP_HEAD_LEN, S_Pkt->Length, sizeof (S_Pkt->Length));
	memcpy (S_Pkt->ApType, ApType, sizeof (S_Pkt->ApType));
	memcpy (S_Pkt->Date, DATE_CURR, sizeof (S_Pkt->Date));
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Time, t_time, sizeof (S_Pkt->Time));
	ItoAf (DataCnt, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	switch (PktType)
	{
		case	T_LINK:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_LINK_CD, sizeof (S_Pkt->MsgType));
			ItoAf (0, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case	T_STRT:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_STRT_CD, sizeof (S_Pkt->MsgType));
			ItoAf (0, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case	T_RSND:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_RSND_CD, sizeof (S_Pkt->MsgType));
			ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case	T_POOK:
			memcpy (S_Pkt->ResponseCode, ErrCd, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_POOK_CD, sizeof (S_Pkt->MsgType));
			ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case    T_LNTH:
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
			memcpy (S_Pkt->MsgType+2, "OK", 2);
			ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case	T_DAOK:
			memcpy (S_Pkt->ResponseCode, ErrCd, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_DAOK_CD, sizeof (S_Pkt->MsgType));
			ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
			break;
		case	T_STOP:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_STOP_CD, sizeof (S_Pkt->MsgType));
			ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
		default:
			break;
	}

	rt = Select_Send (Sockfd, SendPkt, TCP_HEAD_LEN);

	if (rt != OK)
	{
		TCP2_CON_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	return;
}	/* Send_Packet ()	*/

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
	int		val, i, len, next, data_cnt, data_len;
	char	*data = (char *)R_Pkt->Data;

	memset (ErrCd, '0', sizeof (ErrCd));

	/* Stx (0x02)	*/
	if (R_Pkt->Head.Stx[0] != STX)
	{
		Log (USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Head.Stx[0], STX);
		memcpy (ErrCd, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Head.Length, sizeof (R_Pkt->Head.Length));

	if (val != strlen (RecvPkt) ||
		(memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) == 0 &&
		strlen (RecvPkt) <= TCP_HEAD_LEN))
	{
		Log (USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		memcpy (ErrCd, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return (NOTOK);
	}

	data_len = val - TCP_HEAD_LEN;

	/* ApType (업무구분식별자)	*/
	if (memcmp (R_Pkt->Head.ApType, ApType, sizeof (R_Pkt->Head.ApType)) != 0)
	{
		Log (USR_ERROR, "TH.ApType [%.8s:%s]", R_Pkt->Head.ApType, ApType);
		memcpy (ErrCd, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (R_Pkt->Head.ResponseCode,
		sizeof (R_Pkt->Head.ResponseCode)) != 0)
	{
		Log (USR_ERROR, "TH.ResponseCode [%.4s:%04d]",
			R_Pkt->Head.ResponseCode, 0);
		memcpy (ErrCd, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if ((memcmp (R_Pkt->Head.MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0 &&
		memcmp (R_Pkt->Head.MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0 &&
		memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) != 0 &&
		memcmp (R_Pkt->Head.MsgType, TCP_RSOK_CD, strlen (TCP_RSOK_CD)) != 0 &&
		memcmp (R_Pkt->Head.MsgType, TCP_POLL_CD, strlen (TCP_POLL_CD)) != 0) ||
		(PktType == T_LINK &&
		memcmp (R_Pkt->Head.MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0) ||
		(PktType == T_STRT &&
		memcmp (R_Pkt->Head.MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0))
	{
		Log (USR_ERROR, "TH.MsgType [%.4s]", R_Pkt->Head.MsgType);
		memcpy (ErrCd, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return (NOTOK);
	}

	if (memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) == 0)
	{
		/* SeqNo (일련번호)	*/
		val = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));

		if (val != INT_SEQ + 1)
		{
			Log (USR_ERROR, "TH.SeqNo [%d:%d]", val, INT_SEQ + 1);
			memcpy (ErrCd, ERR_HEAD_T8, strlen (ERR_HEAD_T8));
			return (NOTOK);
		}

		/* DataCnt (한 packet 내 blocking된 data 건수)	*/
		data_cnt = next = 0;

		DataCnt = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));

		for (i = 0; i < DataCnt; i ++)
		{
			pTcp_Data_Head = (TCP_DATA_HEAD *)(data+next);
			len = AtoIf (pTcp_Data_Head->Length,
				sizeof (pTcp_Data_Head->Length));
			next += len;

			if (next > data_len || (i == DataCnt - 1 && next < data_len) ||
				len == 0 || len <= HEAD_SIZE)
				break;

			data_cnt ++;
		}

		if (DataCnt != data_cnt || DataCnt > 30)
		{
			Log (USR_ERROR, "TH.DataCnt [%d:%d]", DataCnt, data_cnt);
			memcpy (ErrCd, ERR_HEAD_T9, strlen (ERR_HEAD_T9));
			return (NOTOK);
		}
	}

	return (OK);
}	/* End of Check_Header ()	*/

/*************************************************************************
	Function		: . Write_Data
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . write received data to file
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Write_Data (void)
/*----------------------------------------------------------------------*/
{
	int				i, next, seq, d_cnt, rt, len, f_size, rec_size;
	char			m_time[24], w_buf[FILE_BUF_LEN];
	char			*data = (char *)R_Pkt->Data;
	BUFF_RW_HEAD	f_head;

	d_cnt = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));
	f_size = sizeof (BUFF_RW_HEAD);
	rec_size = f_size + DataSize + 1;

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	memset (w_buf, ' ', sizeof (w_buf));

	for (i = 0, next = 0; i < d_cnt; i ++)
	{
		pTcp_Data_Head = (TCP_DATA_HEAD *)(data+next);

		seq = AtoIf (pTcp_Data_Head->DataSeq, sizeof (pTcp_Data_Head->DataSeq));

		if (seq != INT_SEQ + i + 1)
		{
			Log (USR_ERROR, "data header (DataSeq) [%d:%d]",
				seq, INT_SEQ + i + 1);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D2, strlen (ERR_HEAD_D2));
			return (NOTOK);
		}

		len = AtoIf (pTcp_Data_Head->Length, sizeof (pTcp_Data_Head->Length));

		if (len <= HEAD_SIZE || len > HEAD_SIZE + DataSize)
		{
			Log (USR_ERROR, "data header (Length) [%d:%d]",
				len, HEAD_SIZE + DataSize);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D1, strlen (ERR_HEAD_D1));
			return (NOTOK);
		}

		memcpy (f_head.DataHeader, data+next, HEAD_SIZE);
		ItoAf (INT_SEQ + i + 1, f_head.If_Seq, sizeof (f_head.If_Seq));
		memcpy (f_head.ApType, ApType, strlen (ApType));
		ItoAf (0, f_head.ResponseCode, sizeof (f_head.ResponseCode));
		memcpy (f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
		memcpy (f_head.RecvTime2, &m_time[sizeof(f_head.RecvTime1)],
			sizeof (f_head.RecvTime2));

		memcpy (&w_buf[rec_size*i], &f_head, f_size);
		memcpy (&w_buf[rec_size*i+f_size], data+next+HEAD_SIZE,
			len - HEAD_SIZE);
		w_buf[rec_size*i+f_size+DataSize] = '\n';

		next += len;
	}

	SYS_NO = 0;

#ifdef SAM_USE
	rt = F_W (TS_W1_1, w_buf, d_cnt);
#else
	rt = DSHM_W (TS_W1_1, w_buf, d_cnt);
#endif

	if (rt != d_cnt)
	{
#ifdef SAM_USE
		Log (SAM_FATAL, "file write[%s]", OUT_NAME);
#else
		Log (DSH_FATAL, "DSHM write[%s]", OUT_NAME);
#endif
		memcpy (S_Pkt->ResponseCode, ERR_FILE_WRITE, strlen (ERR_FILE_WRITE));
		return (NOTOK);
	}

	INT_SEQ += d_cnt;
	Log (USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	Set_TR_Time ();

	ItoAf (d_cnt, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	return (OK);
}	/* End of Write_Data ()	*/

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
	End of Program (pw_4100_tr.c)
*************************************************************************/
