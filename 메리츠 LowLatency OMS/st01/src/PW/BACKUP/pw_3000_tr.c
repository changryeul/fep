#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Online 업무 수신
#	File	: pw_3000_tr.c
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
int		Sockfd, PktType, DataSize, MaxCnt;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];

TCP_MESSAGE		*R_Pkt = (TCP_MESSAGE *)RecvPkt;
TCP_HEAD		*S_Pkt = (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_3000_TR (int, char **);
void	Init_Parameters (void);
int		Receive_Packet (void);
int		Send_Packet (void);
int		Write_Data (void);
int		Check_Header (void);
void	Register_Signal (void);
void	Catch_Signal (int);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PW_3000_TR (argc, argv);

	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_3000_TR (int argc, char **argv)
/*----------------------------------------------------------------------*/
{
	int		rt;

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

	rt = Send_Packet ();

	if (rt == NOTOK)
		return;

	while (START_S < JOB_END)
	{
		Stat_Save ();

		rt = Receive_Packet ();

		if (rt == NOTOK)
			break;
		else if (rt == FAIL)
			continue;

		rt = Send_Packet ();

		if (rt == NOTOK)
			break;
	}

	return;
}	/* End of PW_3000_TR ()	*/

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

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

#ifdef SAM_USE
    DataSize = OFS(D_K,P_K,0);
#else
    DataSize = ODS(D_K,P_K,0);
#endif
	MaxCnt = FILE_BUF_LEN / (sizeof (FILE_RW_HEAD) + DataSize + 1);

	return;
}  /* End of Init_Parameters () */

/*************************************************************************
	Function		: . Receive_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, 1:interrupted)
	Comment			: . receive packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, recv_len, seq;

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

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt ||
		recv_len > TCP_MESSAGE_MAX_LEN)
	{
		Log (USR_ERROR, "invalid length[%d,%d]", recv_len, rt);
		PktType = T_LNTH;
		Send_Packet ();
		return (NOTOK);
	}

	if (memcmp (R_Pkt->Head.MsgType, TCP_STOP_CD, strlen (TCP_STOP_CD)) == 0)
	{
		Log (USR_OK, "received STOP request");
		TCP1_NET_STA = END;
		close (Sockfd);
		Exit_Process ();
	}
	else if (memcmp (R_Pkt->Head.MsgType,
		TCP_POLL_CD, strlen (TCP_POLL_CD)) == 0)
	{
		seq = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));

		if (seq != INT_SEQ)
			Log (USR_WARN, "POLL:SeqNo [%d:%d]", seq, INT_SEQ);

		PktType = T_POOK;
	}
	else if (memcmp (R_Pkt->Head.MsgType,
		TCP_DATA_CD, strlen (TCP_DATA_CD)) == 0)
		PktType = T_DAOK;
	else
	{
		Log (USR_ERROR, "TCP header (MsgType) [%.4s:%s]",
			R_Pkt->Head.MsgType, TCP_DATA_CD);
		memcpy (R_Pkt->Head.ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		PktType = T_EROR;
		Send_Packet ();
		return (NOTOK);
	}

	return (OK);
}	/* Receive_Packet ()	*/

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
	int		rt;
	char	error_flag, t_time[12];

	error_flag = OFF;
	memset (SendPkt, 0, sizeof (SendPkt));

	if (PktType == T_STOK)
		memset (SendPkt, ' ', TCP_HEAD_LEN);
	else
		memcpy (SendPkt, RecvPkt, TCP_HEAD_LEN);

	if (PktType != T_EROR)
	{
		S_Pkt->Stx[0] = STX;
		ItoAf (TCP_HEAD_LEN, S_Pkt->Length, sizeof (S_Pkt->Length));
		memcpy (S_Pkt->ApType, ApType, sizeof (S_Pkt->ApType));
		memcpy (S_Pkt->Date, DATE_CURR, sizeof (S_Pkt->Date));
		memset (t_time, 0, sizeof (t_time));
		Get_Time (t_time);
		memcpy (S_Pkt->Time, t_time, sizeof (S_Pkt->Time));
		ItoAf (0, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));
	}

	switch (PktType)
	{
		case	T_STOK:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_STOK_CD, sizeof (S_Pkt->MsgType));
			break;
		case	T_POOK:
			memcpy (S_Pkt->MsgType, TCP_POOK_CD, sizeof (S_Pkt->MsgType));
			break;
		case    T_LNTH:
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
			memcpy (S_Pkt->MsgType+2, "OK", 2);
			break;
		case	T_DAOK:
			rt = Check_Header ();

			if (rt == OK)
			{
				rt = Write_Data ();

				if (rt != OK)
					error_flag = ON;
			}
			else
				error_flag = ON;

			memcpy (S_Pkt->MsgType, TCP_DAOK_CD, sizeof (S_Pkt->MsgType));
			break;
		default:
			break;
	}

	if (PktType != T_EROR)
		ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));

	rt = Select_Send (Sockfd, SendPkt, TCP_HEAD_LEN);

	if (rt != OK)
		return (NOTOK);

	if (PktType == T_STOK)
		TCP1_NET_STA = ON;
	else
		Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	if (PktType == T_LNTH || error_flag == ON)
		return (NOTOK);

	return (OK);
}	/* Send_Packet ()	*/

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
	Set_TR_Time ();
	Log (USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	ItoAf (d_cnt, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	return (OK);
}	/* End of Write_Data ()	*/

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

	/* Stx (0x02)	*/
	if (R_Pkt->Head.Stx[0] != STX)
	{
		Log (USR_ERROR, "TCP header (Stx) [%#04x:%#04x]",
			R_Pkt->Head.Stx[0], STX);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Head.Length, sizeof (R_Pkt->Head.Length));
	if (val != strlen (RecvPkt) || strlen (RecvPkt) <= TCP_HEAD_LEN)
	{
		Log (USR_ERROR, "TCP header (Length) [%d:%d]", val, strlen (RecvPkt));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return (NOTOK);
	}

	/* ApType (업무구분식별자)	*/
	if (memcmp (R_Pkt->Head.ApType, ApType, sizeof (R_Pkt->Head.ApType)) != 0)
	{
		Log (USR_ERROR, "TCP header (ApType) [%.8s:%s]",
			R_Pkt->Head.ApType, ApType);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (R_Pkt->Head.ResponseCode,
		sizeof (R_Pkt->Head.ResponseCode)) != 0)
	{
		Log (USR_ERROR, "TCP header (ResponseCode) [%.4s:%04d]",
			R_Pkt->Head.ResponseCode, 0);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if (memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) != 0)
	{
		Log (USR_ERROR, "TCP header (MsgType) [%.4s:%s]",
			R_Pkt->Head.MsgType, TCP_DATA_CD);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return (NOTOK);
	}

	/* SeqNo (일련번호)	*/
	val = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));

	if (val != INT_SEQ + 1)
	{
		Log (USR_ERROR, "TCP header (SeqNo) [%d:%d]", val, INT_SEQ + 1);
		ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T8, strlen (ERR_HEAD_T8));
		return (NOTOK);
	}

	/* DataCnt (한 packet 내 blocking된 data 건수)	*/
	val = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));

	if (val < 1 || val > MaxCnt)
	{
		Log (USR_ERROR, "TCP header (DataCnt) [%d:%d]", val, MaxCnt);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T9, strlen (ERR_HEAD_T9));
		return (NOTOK);
	}

	return (OK);
}	/* End of Check_Header ()	*/

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
	End of Program (pw_3000_tr.c)
*************************************************************************/
