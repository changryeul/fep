#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Online 송신 (TCP client), FEP 주문송신접속, async
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
#define		TCP_TIME_OUT	30

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
int		LegSize, DataSize, RetryCnt, f_len;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char	IpAddr[20], DeviceSendFlag, R_Buf[FILE_BUF_LEN];
char	Tcp_DataHeader[100];

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

	sprintf (ApType, "%-2.2s%-4.4s%c%c",
		_Exe_Name, _Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
	LtoU (ApType, strlen (ApType));

	/* BUFF_RW_HEAD(70) */
	f_len = sizeof (BUFF_RW_HEAD);

#ifdef SAM_USE
	LegSize = TCP_DATA_HEAD_LEN + IFS(D_K,P_K,0);
	DataSize = IFS(D_K,P_K,0);
#else
	LegSize = TCP_DATA_HEAD_LEN + IDS(D_K,P_K,0);
	DataSize = IDS(D_K,P_K,0);
#endif

/* 메리츠증권은 async로 사용하고 1건씩 처리한다.(복수건은 sync일때만)
	MaxCnt = TCP_DATA_LEN / LegSize;

	if (MaxCnt > 8)
		MaxCnt = 8;
*/
	MaxCnt = 1;

	/* 송신시 데이터헤더 70바이트를 SPACE 처리한다. 협의시 수정 가능 */
	memset (Tcp_DataHeader, 0x20, sizeof(Tcp_DataHeader));

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
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			SLog (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
				Sockfd, SYS_NO, SYS_STR);
			sleep (5);
			continue;
		}

		TCP2_LINE_ST = ON;
		SLog (USR_OK, "socket created:Sockfd[%d]", Sockfd);
		SLog (USR_OK, "connecting to %s:%d", IpAddr, PortNo);

		rt = Connect (Sockfd, IpAddr, PortNo);

		if (rt < 0)
		{
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			TCP2_NET_STA(S_K) = OFF;
			SLog (TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
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

					SLog (TCP_OK, "port changed to %s (%s:%d)",
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
		SLog (USR_OK, "connected to %s:%d", IpAddr, PortNo);
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
					if (MAX_POLL < (WR_CNT - RD_CNT))
					{
						SLog (SAM_ERROR, "01 WR_CNT[%d] RD_CNT[%d]",
							WR_CNT, RD_CNT);
						sleep(60);
						Exit_Process ();
					}
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
						SLog (TCP_ERROR, "socket disconnected[%#06x]",
							Poll[i].revents);
						return;
					}

					SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
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
					SLog (TCP_ERROR, "poll timeout (no response)");
					return;
				}

				PktType = T_POLL;
				Send_Packet ();
				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					SLog (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					SLog (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

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
						if (MAX_POLL < (WR_CNT - RD_CNT))
						{
							SLog (SAM_ERROR, "02 WR_CNT[%d] RD_CNT[%d]",
								WR_CNT, RD_CNT);
							sleep(60);
							Exit_Process ();
						}
						File_Event_Rtn ();
						continue;
					}
					else
						break;
				}
				break;
			default:
				SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
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
	SLog (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt)
	{
		SLog (USR_ERROR, "invalid length[%d:%d,%d]", recv_len, TCP_HEAD_LEN, rt);
		return (NOTOK);
	}

	rt = Check_Header ();

	if (rt != OK)
	{
		sleep (3);
		return (NOTOK);
	}

	/* 상대방 seq 기록 */
	if ((INT_SEQ != AtoIf (R_Pkt->SeqNo,	sizeof (R_Pkt->SeqNo)))	&&
		(PROC(D_K,P_K).counter_seq < AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo))) )
		PROC(D_K,P_K).counter_seq =
					AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));

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
	int		i, next, rt, val;

	/* Stx (0x02)	*/
	if (R_Pkt->Stx[0] != STX)
	{
		SLog (USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Stx[0], STX);
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Length, sizeof (R_Pkt->Length));
	if (val != strlen (RecvPkt)-5)
	{
		SLog (USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		return (NOTOK);
	}

	/* Seq 체크 */
	if (PktType != T_LINK)
	{
		if (INT_SEQ < AtoIf (R_Pkt->SeqNo,	sizeof (R_Pkt->SeqNo)))
		{
			SLog (USR_ERROR, "TH.SeqNo Rcv[%.8s:%s] INT_SEQ[%d]", R_Pkt->SeqNo, INT_SEQ);
			return (NOTOK);
		}
	}

	/* ApType (업무구분식별자)	
	if (memcmp (R_Pkt->ApType, ApType, sizeof (R_Pkt->ApType)) != 0)
	{
		SLog (USR_ERROR, "TH.ApType [%.8s:%s]", R_Pkt->ApType, ApType);
		return (NOTOK);
	}
*/

	/* ResponseCode (응답코드)	*/
/*
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0 &&
		PktType != T_POLL)
*/
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0)
	{
		SLog (USR_ERROR, "TH.ResponseCode [%.4s:%04d]", R_Pkt->ResponseCode, 0);
		return (NOTOK);
	}

#if 0
// 메리츠는 운영코드를 사용하지 않음
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
		SLog (USR_ERROR, "TH.MsgType [%.4s]", R_Pkt->MsgType);
		return (NOTOK);
	}
#endif
	/* 접속/재접속 정상수신 체크 */
	if ( (PktType == T_LINK && memcmp (R_Pkt->DataCnt, "91", 2) != 0)	||
		 (PktType == T_STRT && memcmp (R_Pkt->DataCnt, "92", 2) != 0)	||
		 (PktType == T_POLL && memcmp (R_Pkt->DataCnt, "98", 2) != 0))
	{
		SLog (USR_ERROR, "TH.Rcv Data [%s]", R_Pkt);
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
SLog (USR_OK, "Data Read");
#endif

#ifdef SAM_USE
	DataCnt = F_R (P_TYPE, R_Buf, MaxCnt);
#else
	DataCnt = DSHM_R (P_TYPE, R_Buf, MaxCnt);
#endif

	if (DataCnt < 0 || DataCnt > MaxCnt)
	{
		SLog (SAM_FATAL, "cannot read file[%s]", IN_NAME);
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
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
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
	int		i, rt, next;
	char	t_time[12];

	memset (SendPkt, 0, sizeof (SendPkt));
	memset (SendPkt, 0x20, TCP_HEAD_LEN);

	/* 1. STX */
	S_Pkt->Head.Stx[0] = STX;
	/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
	/* 3. 업무식별코드 */
#if defined A1101
	memcpy (S_Pkt->Head.ApType, "A301", sizeof (S_Pkt->Head.ApType));
#elif defined A1102
	memcpy (S_Pkt->Head.ApType, "A311", sizeof (S_Pkt->Head.ApType));
#elif defined A2101
	memcpy (S_Pkt->Head.ApType, "B301", sizeof (S_Pkt->Head.ApType));
#elif defined A2102
	memcpy (S_Pkt->Head.ApType, "B311", sizeof (S_Pkt->Head.ApType));
#endif
	/* 4. 송수신 구분 S:send, R:Recv */
	memcpy (S_Pkt->Head.SR_gbn, "S",	sizeof (S_Pkt->Head.SR_gbn));
	/* 5. 처리일자 */
	memcpy (S_Pkt->Head.Date, DATE_CURR,sizeof (S_Pkt->Head.Date));
	/* 6. 처리시각 (HHMMSS) */
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Head.Time, t_time,	sizeof (S_Pkt->Head.Time));
	/* 7. 응답코드 '000'이외 모두 에러 */
	ItoAf (0, S_Pkt->Head.ResponseCode, sizeof (S_Pkt->Head.ResponseCode));
	/* 8. '000' Queue입력구분 */
	ItoAf (0, S_Pkt->Head.Q_gbn,		sizeof (S_Pkt->Head.Q_gbn));
	/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
	ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo,		sizeof (S_Pkt->Head.SeqNo));
	/* 10. Data건수
	   01~08:DATA전문 (기본:01)
       91:운영전문,
       92:개시전문,
       93:일련번호요청전문,
       98:POLL전문*/
	/* 11. 최종일련번호, 전문일련번호와 동일 */
	ItoAf (INT_SEQ + 1, S_Pkt->Head.Last_SeqNo,	sizeof (S_Pkt->Head.Last_SeqNo));
	/* 12. 서버구분, '1'(정상주문) 	*/
	memcpy (S_Pkt->Head.Server_gbn, "1",sizeof (S_Pkt->Head.Server_gbn));
	/* 13. 지점번호, SPACE(FEP담당자가 SPACE하라고함)	*/
	memcpy (S_Pkt->Head.Branch,	"   ",	sizeof (S_Pkt->Head.Branch));
	/* 14. 물리적지점번호 (8706)	*/
	memcpy (S_Pkt->Head.Sv_No,	"8706",	sizeof (S_Pkt->Head.Sv_No));
	/* 15. HTS단말ID, 단말ID */
	memcpy (S_Pkt->Head.Hts_Id, "        ",	sizeof (S_Pkt->Head.Hts_Id));
	/* 16. 사번 or 로그인 ID, 사용자ID */
	memcpy (S_Pkt->Head.User_Id, "        ",sizeof (S_Pkt->Head.User_Id));
	/* 17. Filler */
	memcpy (S_Pkt->Head.Filler, "        ",	sizeof (S_Pkt->Head.Filler));

	/* TR에 따라 다시 처리 해야하는 항목 2.전문길이, 9,11일련번호, 10.Data건수(전문구분)  */
	if ( PktType == T_DATA )
	{
		/* 2.  전문길이 80(통신헤더)+70(데이터헤더)+261(KRX주문포맷)-5 => 406 Byte */
		ItoAf (TCP_HEAD_LEN+TCP_DATA_HEAD_LEN+sizeof(KRX_JUMUN_DATA)-5,
				S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
		/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
		ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo,		sizeof (S_Pkt->Head.DataCnt));
		/* 11. 최종일련번호, 전문일련번호와 동일 */
		ItoAf (INT_SEQ + 1, S_Pkt->Head.Last_SeqNo,	sizeof (S_Pkt->Head.Last_SeqNo));
		/* 10. Data건수(주문송신) */
		memcpy (S_Pkt->Head.DataCnt, "01",			sizeof (S_Pkt->Head.DataCnt));

		/* 통신Header(80)+DataHeader(70)+Data(261) */
		memcpy (S_Pkt->Data_Head.Filler,	Tcp_DataHeader,	TCP_DATA_HEAD_LEN);
		memcpy (S_Pkt->Data,				&R_Buf[f_len],	DataSize);

		/* 주문번호 채번 */
#if defined A1101||A1102
		Shm_Risk[0].js_order_no_band ++;
		ItoAf (Shm_Risk[0].js_order_no_band, &S_Pkt->Data[34], 10);
#elif defined A2101||A2102
		Shm_Risk[0].dv_order_no_band ++;
		ItoAf (Shm_Risk[0].dv_order_no_band, &S_Pkt->Data[34], 10);
#endif
	}
	else
	{
		switch (PktType)
		{
			case	T_LINK:
				/* 2. 전문길이 80-5 => 75 Byte, 위에서 처리 */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 최초접속시 0 */
				ItoAf (0, S_Pkt->Head.SeqNo, 			sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 최초접속시 0 */
				ItoAf (0, S_Pkt->Head.Last_SeqNo,		sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 최초접속 */
				memcpy (S_Pkt->Head.DataCnt, "91",		sizeof (S_Pkt->Head.DataCnt));

				break;
			case	T_STRT:
				/* 2. 전문길이 80-5 => 75 Byte, 위에서 처리 */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, 		sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.Last_SeqNo,	sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 재접속 */
				memcpy (S_Pkt->Head.DataCnt, "92",		sizeof (S_Pkt->Head.DataCnt));
				break;
			case	T_POLL:
				/* 2. 전문길이 80-5 => 75 Byte, 위에서 처리 */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo, 		sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.Last_SeqNo,	sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 재접속 */
				memcpy (S_Pkt->Head.DataCnt, "98",		sizeof (S_Pkt->Head.DataCnt));
				break;
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
		SLog (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		TCP2_CON_STA = OFF;
		close (Sockfd);
		Exit_Process ();
	}
*/

	SLog (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

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
		SLog (SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		SLog (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
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
	SLog (PRO_WARN, "signal (%d) occurred", signo);

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
		SLog (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}

/*************************************************************************
	End of Program (pw_3100_ts.c)
*************************************************************************/

