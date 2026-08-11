#define	 _GLOBAL
/*------------------------------------------------------------------------
#	Module : Online 송신 (TCP client), FEP 주문송신접속, async
#	File : pa_1100_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_SIZE	261
#include	"buf_struct.h"

#define	 TCP_TIME_OUT 30

#define	 FIFO_EVENT  0
#define	 SOCKET_EVENT 1
#define	 FILE_EVENT  2

#define	 P_TYPE   PS_R_1

#ifdef	SAM_USE
#define	 WR_CNT   W_CNT(0,0)
#define	 RD_CNT   R_CNT(0,0)
#define	 IN_NAME   IFN(D_K,P_K,0)
#else
#define	 WR_CNT   IDW_CNT(0,0)
#define	 RD_CNT   IDR_CNT(0,0)
#define	 IN_NAME   IDN(D_K,P_K,0)
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, PortNo, DataCnt, PollCnt, PktType, MaxCnt;
int		LegSize, DataSize, RetryCnt, f_len;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];
char	IpAddr[20], DeviceSendFlag;
char	Tcp_DataHeader[100];

FILE_BUFF_FORMAT	R_Buf[1];

struct	pollfd Poll[3];
TCP_HEAD	 *R_Pkt = (TCP_HEAD *)RecvPkt;
TCP_MESSAGE	 *S_Pkt = (TCP_MESSAGE *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1100_TS (void);
void	Init_Parameters (void);
void	Connection (void);
void	Communicate_Routine (void);
void	Fifo_Event_Rtn (void);
int		Receive_Packet	(void);
int		Check_Header (void);
void	File_Event_Rtn (void);
void	Send_Packet (void);
void	Register_Signal (void);
void	Catch_Signal (int);
void	Set_Socket_Linger (void);
int		Chk_Risk_All (char *);
void	Set_Band_Unit	(int, double);
int     Cross_Chk (int, int, int, int, double);

/*----------------------------------------------------------------------*/
int	 main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);

	PA_1100_TS ();

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main () */

/*----------------------------------------------------------------------*/
void	PA_1100_TS (void)
/*----------------------------------------------------------------------*/
{
	int  i, rt;

	Register_Signal ();
	Init_Parameters ();

#ifdef  HOLIDAY_CHECK
    while (1)
    {
        char     t_time[12], dt[20];
        time_t   t = time(NULL);
        struct  tm tm, *tp;

        Get_Time    (t_time);

        memset  (dt, 0, sizeof (dt));
        sprintf (dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
            DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
        strptime    (dt, "%Y-%m-%d %H:%M:%s", &tm);
        //t   = mktime (&tm);
        tp  = localtime (&t);

        if  (tp->tm_wday == 0 || tp->tm_wday == 6) /* sun, sat */
        {
            SLog (USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
            sleep (60);
            continue;
        }
        else
		{
            SLog (USR_OK, "it's not weekend. not sleeping...[%d]", tp->tm_wday);
			break;
		}
/*
			if (memcmp (Shm_Risk[0].business_day, DAEMON(D_K).date, 8) == 0)
				break;
			else
			{
				SLog (USR_OK, "system Day와 business Day가 다르다...[%8.8s][%8.8s]",
					DAEMON(D_K).date, Shm_Risk[0].business_day);
				sleep (600);
				continue;
			}
*/
    }
#endif

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
}	/* End of PA_1100_TS () */

/*************************************************************************
	Function  : . Init_Parameters
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . initiate the global variables
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

#ifdef	SAM_USE
	LegSize = TCP_DATA_HEAD_LEN + IFS(D_K,P_K,0);
	DataSize = IFS(D_K,P_K,0);
#else
	LegSize = TCP_DATA_HEAD_LEN + IDS(D_K,P_K,0);
	DataSize = IDS(D_K,P_K,0);
#endif

/*	메리츠증권은 async로 사용하고 1건씩 처리한다.(복수건은 sync일때만)
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
}	/* End of Init_Parameters () */

/*************************************************************************
	Function  : . Connection
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . connect to server
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Connection (void)
/*----------------------------------------------------------------------*/
{
	int  rt;

	while (START_S != END)
	{
		Sockfd = Socket ();

		if (Sockfd < 0)
		{
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
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
}	/* End of Connection () */

/*************************************************************************
	Function  : . Communicate_Routine
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . communicate routine
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Communicate_Routine (void)
/*----------------------------------------------------------------------*/
{
	int  rt, i;
	char t_time[12];

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
					if (MAX_MICHE < (WR_CNT - RD_CNT))
					{
						SLog (SAM_ERROR, "01 Memory Not Read So Delay WR_CNT[%d] RD_CNT[%d]", WR_CNT, RD_CNT);
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
						Log (TCP_ERROR, "socket disconnected[%#06x]", Poll[i].revents);
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
					SLog (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					SLog (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
 
				continue;
			}
		}

		switch (i)
		{
			case FIFO_EVENT:
				Fifo_Event_Rtn ();
				break;
			case SOCKET_EVENT:
				rt = Receive_Packet ();
				if (rt == NOTOK)
					return;
				break;
			case FILE_EVENT:
				File_Event_Rtn ();

				while (START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						if (MAX_MICHE < (WR_CNT - RD_CNT))
						{
							SLog (SAM_ERROR, "02 Memory Not Read So Delay WR_CNT[%d] RD_CNT[%d]", WR_CNT, RD_CNT);
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
}	/* End of Communicate_Routine () */

/*************************************************************************
	Function  : . Fifo_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn () */

/*************************************************************************
	Function  : . Receive_Packet
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . int (0:success, -1:failure, 1:interrupted)
	Comment   : . receive Receive_Packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int	 Receive_Packet (void)
/*----------------------------------------------------------------------*/
{
	int  rt, recv_len;

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
	if ((INT_SEQ != AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo))) &&
		(PROC(D_K,P_K).counter_seq < AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo))) )
		PROC(D_K,P_K).counter_seq = AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo));

	return (OK);
}	/* Receive_Packet () */

/*************************************************************************
	Function  : . Check_Header
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . int (0:success, -1:failure, 1:error write)
	Comment   : . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int	 Check_Header (void)
/*----------------------------------------------------------------------*/
{
	int  i, next, rt, val;

	/* Stx (0x02) */
	if (R_Pkt->Stx[0] != STX)
	{
		SLog (USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Stx[0], STX);
		return (NOTOK);
	}

	/* Length (packet 총 길이) */
	val = AtoIf (R_Pkt->Length, sizeof (R_Pkt->Length));
	if (val != strlen (RecvPkt)-5)
	{
		SLog (USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		return (NOTOK);
	}

	/* Seq 체크 */
	if (PktType != T_LINK)
	{
		if (INT_SEQ != AtoIf (R_Pkt->SeqNo, sizeof (R_Pkt->SeqNo)))
		{
			/* 202111 추가 */
			/* 92 RELINK시 FEP의 앞의 Seq를 우리 Seq로 처리한다(FEP Seq를 따른다) */
			if (PktType == T_STRT)
			{
				INT_SEQ = AtoIf (R_Pkt->SeqNo, sizeof(R_Pkt->SeqNo));
				RD_CNT  = INT_SEQ;
				SLog (USR_OK, "Update RD_CNT[%d]", RD_CNT);
			}
#if 0
			else
			{
				SLog (USR_ERROR, "TH.SeqNo Rcv[%.8s] INT_SEQ[%d]", R_Pkt->SeqNo, INT_SEQ);
				return (NOTOK);
			}
#endif
		}
	}

	/* ApType (업무구분식별자) 
	if (memcmp (R_Pkt->ApType, ApType, sizeof (R_Pkt->ApType)) != 0)
	{
		SLog (USR_ERROR, "TH.ApType [%.8s:%s]", R_Pkt->ApType, ApType);
		return (NOTOK);
	}
*/

	/* ResponseCode (응답코드) */
/*
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0 &&
		PktType != T_POLL)
*/
	if (AtoIf (R_Pkt->ResponseCode, sizeof (R_Pkt->ResponseCode)) != 0)
	{
		SLog (USR_ERROR, "TH.ResponseCode [%.3s:%03d]", R_Pkt->ResponseCode, 0);
		return (NOTOK);
	}

#if	0
//	메리츠는 운영코드를 사용하지 않음
	/* MsgType (운영코드) */
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
	if ((PktType == T_LINK && memcmp (R_Pkt->DataCnt, "91", 2) != 0) ||
		(PktType == T_STRT && memcmp (R_Pkt->DataCnt, "92", 2) != 0) ||
		(PktType == T_POLL && memcmp (R_Pkt->DataCnt, "98", 2) != 0))
	{
		SLog (USR_ERROR, "TH.Rcv Data [%s]", R_Pkt);
		return (NOTOK);
	}

	return (OK);
}	/* End of Check_Header () */

/*************************************************************************
	Function  : . File_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . read file and send data
*************************************************************************/
/*----------------------------------------------------------------------*/
void	File_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int  rt, ow_seq;
	char err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];

	memset (R_Buf, 0, sizeof (FILE_BUFF_FORMAT));

#if	(0)
/*	Test SLog */
SLog	(USR_OK, "Data Read");
#endif

#ifdef	SAM_USE
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

	rt = Chk_Risk_All (R_Buf[0].Data);
	if (rt < 0)
    {
        memset (w_buf, 0x20, sizeof (w_buf));
		memcpy (w_buf, R_Buf[0].Seq, sizeof(BUFF_RW_HEAD));

		memset (err_cd, 0, sizeof(err_cd));
        if (abs(rt) > 9999)
            sprintf (err_cd, "%04d", abs(rt)/10);
        else
            sprintf (err_cd, "%04d", abs(rt));

		/* Data Header (20 Byte) 100120(정합성오류) */
		memcpy (&w_buf[sizeof(BUFF_RW_HEAD)],		"100120",		 6);
		memcpy (&w_buf[sizeof(BUFF_RW_HEAD)+10],	err_cd,			 4);
		memcpy (&w_buf[sizeof(BUFF_RW_HEAD)+14], &R_Buf[0].Data[200+46], 5);
		/* Order Data */
		memcpy (&w_buf[sizeof(BUFF_RW_HEAD)+SEARCH_HEADER_LEN], R_Buf[0].Data, DATA_SIZE);
        w_buf[sizeof (BUFF_RW_HEAD)+ODS(D_K,P_K,0)] = '\n';

/*
		ow_seq = ((AtoIf(&R_Buf[0].Data[15], 2) -1) * 10) + AtoIf(&R_Buf[0].Data[17], 2);
*/
		ow_seq = ((AtoIf(&R_Buf[0].Data[200+47], 2) -1) * 10) + AtoIf(&R_Buf[0].Data[200+49], 2);

		/* 전략에게 오류전달 */
        rt = DSHM_W (ow_seq*10, w_buf, 1);
        if (rt != 1)
        {
            SLog (SAM_FATAL, "Dshm Error write[%s] rt[%d]", w_buf, rt);
			Dshm_Add_Count (TS_W1_1, 1);
            return ;
        }
        SLog (USR_OK, "Dshm write OK[%s][%d]", w_buf, strlen(w_buf));

		/* Client에게 오류전달 */
        w_buf[sizeof (BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';
        rt = F_W (TS_W1_1, w_buf, 1);
        if (rt != 1)
        {
            SLog (SAM_FATAL, "file write[%s] rt[%d]", w_buf, rt);
            return ;
        }
        SLog (USR_OK, "File write OK[%s][%d]", w_buf, strlen(w_buf));

        SLog (USR_OK, "Err F_W Done.");
        Dshm_Add_Count (TS_W1_1, 1);

        if (WR_CNT <= RD_CNT)
        {
            PktType = T_POLL;
            Send_Packet ();
        }
	}
    else
    {
        PktType = T_DATA;
        Send_Packet ();
        Dshm_Add_Count (PS_R_1, 1);
    }

/*
	PktType = T_DATA;
	Send_Packet ();
*/

	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));
#if	defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
	}

	return;
}	/* File_Event_Rtn () */

/*************************************************************************
	Function  : . Send_Packet
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . int (0:success, -1:failure)
	Comment   : . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int  i, rt, next;
	char t_time[12];

	memset (SendPkt, 0, sizeof (SendPkt));
	memset (SendPkt, 0x20, TCP_HEAD_LEN);

	/* 1. STX */
	S_Pkt->Head.Stx[0] = STX;
	/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
	/* 3. 업무식별코드 */
#if	defined A1101
	memcpy (S_Pkt->Head.ApType, "A301", sizeof (S_Pkt->Head.ApType));
#elif	defined A1102
	memcpy (S_Pkt->Head.ApType, "A311", sizeof (S_Pkt->Head.ApType));
#elif	defined A1111
	memcpy (S_Pkt->Head.ApType, "C301", sizeof (S_Pkt->Head.ApType));
#elif	defined A1112
	memcpy (S_Pkt->Head.ApType, "C311", sizeof (S_Pkt->Head.ApType));
#elif	defined A2101
	memcpy (S_Pkt->Head.ApType, "B301", sizeof (S_Pkt->Head.ApType));
#elif	defined A2102
	memcpy (S_Pkt->Head.ApType, "B311", sizeof (S_Pkt->Head.ApType));
#endif
	/* 4. 송수신 구분 S:send, R:Recv */
	memcpy (S_Pkt->Head.SR_gbn, "S", sizeof (S_Pkt->Head.SR_gbn));
	/* 5. 처리일자 */
	memcpy (S_Pkt->Head.Date, DATE_CURR,sizeof (S_Pkt->Head.Date));
	/* 6. 처리시각 (HHMMSS) */
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Head.Time, t_time, sizeof (S_Pkt->Head.Time));
	/* 7. 응답코드 '000'이외 모두 에러 */
	ItoAf (0, S_Pkt->Head.ResponseCode, sizeof (S_Pkt->Head.ResponseCode));
	/* 8. '000' Queue입력구분 */
	ItoAf (0, S_Pkt->Head.Q_gbn,  sizeof (S_Pkt->Head.Q_gbn));
	/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
	ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo,  sizeof (S_Pkt->Head.SeqNo));
	/* 10. Data건수
	   01~08:DATA전문 (기본:01)
	      91:운영전문,
	      92:개시전문,
	      93:일련번호요청전문,
	      98:POLL전문*/
	/* 11. 최종일련번호, 전문일련번호와 동일 */
	ItoAf (INT_SEQ + 1, S_Pkt->Head.Last_SeqNo, sizeof (S_Pkt->Head.Last_SeqNo));
	/* 12. 서버구분, '1'(정상주문)  */
	memcpy (S_Pkt->Head.Server_gbn, "1",sizeof (S_Pkt->Head.Server_gbn));
	/* 13. 지점번호, SPACE(FEP담당자가 SPACE하라고함) */
	memcpy (S_Pkt->Head.Branch, "   ", sizeof (S_Pkt->Head.Branch));
	/* 14. 물리적지점번호 (8706) */
	memcpy (S_Pkt->Head.Sv_No, "8706", sizeof (S_Pkt->Head.Sv_No));
	/* 15. HTS단말ID, 단말ID */
	memcpy (S_Pkt->Head.Hts_Id, "        ", sizeof (S_Pkt->Head.Hts_Id));
	/* 16. 사번 or 로그인 ID, 사용자ID */
	memcpy (S_Pkt->Head.User_Id, "        ",sizeof (S_Pkt->Head.User_Id));
	/* 17. Filler */
	memcpy (S_Pkt->Head.Filler, "        ", sizeof (S_Pkt->Head.Filler));

	/* TR에 따라 다시 처리 해야하는 항목 2.전문길이, 9,11일련번호, 10.Data건수(전문구분)  */
	if ( PktType == T_DATA )
	{
		/* 2.  전문길이 80(통신헤더)+70(데이터헤더)+261(KRX주문포맷)-5 => 406 Byte */
		ItoAf (TCP_HEAD_LEN+TCP_DATA_HEAD_LEN+sizeof(KRX_JUMUN_DATA)-5,
			S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
		/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
		ItoAf (INT_SEQ + 1, S_Pkt->Head.SeqNo,  sizeof (S_Pkt->Head.SeqNo));
		/* 11. 최종일련번호, 전문일련번호와 동일 */
		ItoAf (INT_SEQ + 1, S_Pkt->Head.Last_SeqNo, sizeof (S_Pkt->Head.Last_SeqNo));
		/* 10. Data건수(주문송신) */
		memcpy (S_Pkt->Head.DataCnt, "01",   sizeof (S_Pkt->Head.DataCnt));

		/* 통신Header(80)+DataHeader(70)+Data(261) */
		memcpy (S_Pkt->Data_Head.Filler, Tcp_DataHeader, TCP_DATA_HEAD_LEN);
		//memcpy (S_Pkt->Data,    &R_Buf[f_len], DataSize);
		memcpy (S_Pkt->Data,    R_Buf[0].Data, DataSize);
	}
	else
	{
		switch (PktType)
		{
			case T_LINK:
				/* 2. 전문길이 80-5 => 75 Byte */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 최초접속시 0 */
				ItoAf (0, S_Pkt->Head.SeqNo,    sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 최초접속시 0 */
				ItoAf (0, S_Pkt->Head.Last_SeqNo,  sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 최초접속 */
				memcpy (S_Pkt->Head.DataCnt, "91",  sizeof (S_Pkt->Head.DataCnt));

				break;
			case T_STRT:
				/* 2. 전문길이 80-5 => 75 Byte */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo,   sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.Last_SeqNo, sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 재접속 */
				memcpy (S_Pkt->Head.DataCnt, "92",  sizeof (S_Pkt->Head.DataCnt));
				break;
			case T_POLL:
				/* 2. 전문길이 80-5 => 75 Byte */
				ItoAf (TCP_HEAD_LEN-5, S_Pkt->Head.Length, sizeof (S_Pkt->Head.Length));
				/* 9. 전문일련번호; TCP port (접속시는 PORT_NO), 재접속 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.SeqNo,   sizeof (S_Pkt->Head.SeqNo));
				/* 11. 최종일련번호, 전문일련번호와 동일, 재접속시 현재 SEQ */
				ItoAf (INT_SEQ, S_Pkt->Head.Last_SeqNo, sizeof (S_Pkt->Head.Last_SeqNo));
				/* 10. Data건수(주문송신), 재접속 */
				memcpy (S_Pkt->Head.DataCnt, "98",  sizeof (S_Pkt->Head.DataCnt));
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
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}", rt, SYS_NO, SYS_STR);
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
}	/* Send_Packet () */

/*************************************************************************
	Function  : . Register_Signal
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . register signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction act;

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
}	/* End of Register_Signal () */

/*************************************************************************
	Function  : . Catch_Signal
	Parameters IN : . signo : signal number
	Parameters OUT : .
	Return Code  : . void
	Comment   : . catch signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	SLog (PRO_WARN, "signal (%d) occurred", signo);

#if	0
	if (signo == SIGTERM)
	{
	 PktType = T_STOP;
	 Send_Packet ();
	}
#endif

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of Catch_Signal () */

/*************************************************************************
	Function  : . Set_Socket_Linger
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . set linger option on socket
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Socket_Linger (void)
/*----------------------------------------------------------------------*/
{
	int    rt;
	struct linger ling;

	   /* close () returns after discarding any unsent data */
	ling.l_onoff = 1;
	ling.l_linger = 0;

	rt = setsockopt (Sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof (ling));
	if (rt < 0)
		Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}

/*************************************************************************
    Function        : . Chk_Risk_All
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . set sise data SHM (±aº≫/E￡°¡/A¼°a)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Chk_Risk_All (char *p_buf)
/*----------------------------------------------------------------------*/
{
    int     i, j, rt, tick_diff;
	char	m_time[24], Wdt[30], W3_Fmt[512], fifo_name[100];
	int		mm_flag, order_flag;
	double	mk_price, od_price, hl_lmt_price, order_quantity;
	int		mk_gbn, acc_seq, item_seq, risk_gbn;
	int		risk_mk_gbn;
	int		getcnt;
	double	in_std_price;

	KRX_JUMUN_DATA *dat	=	(KRX_JUMUN_DATA *)&p_buf[0];

	mk_gbn   = AtoIf (dat->MembershipItem+35, 2);	// 시장구분
	item_seq = AtoIf (dat->MembershipItem+37, 5);   // 종목seq
	acc_seq  = AtoIf (dat->MembershipItem+42, 2);   // 계좌seq
/*
11              00  코스피200선물      
12              01  코스피200콜옵션    
13              02  코스피200풋옵션    
14              03  주식선물           
15              04  주식콜옵션         
16              05  주식풋옵션         
20              06  코스닥150선물      
44              07  코스닥150콜옵션    
45              08  코스닥150풋옵션    
46              09  KRX300선물         
51              10  현물주식           
34              11  미니코스피200선물  
35              12  미니코스피200콜옵션
36              13  미니코스피200풋옵션
*/
	risk_mk_gbn   = AtoIf (dat->MembershipItem+51, 2);	// 리스크시장구분

#if     defined A1101||A1102			// 유가증권	
	if ((memcmp (Shm_Risk[0].Mst_Acc[acc_seq].mk_gbn, "1", 1) != 0)	||
		(memcmp (dat->account_number, Shm_Risk[0].Mst_Acc[acc_seq].acc_no, 12) != 0))
#elif     defined A1111||A1112			// 코스닥	
	if ((memcmp (Shm_Risk[0].Mst_Acc[acc_seq].mk_gbn, "1", 1) != 0)	||
		(memcmp (dat->account_number, Shm_Risk[0].Mst_Acc[acc_seq].acc_no, 12) != 0))
#elif     defined A2101||A2102			// 파생
	if ((memcmp (Shm_Risk[0].Mst_Acc[acc_seq].mk_gbn, "2", 1) != 0)	||
		(memcmp (dat->account_number, Shm_Risk[0].Mst_Acc[acc_seq].acc_no, 12) != 0))
#endif
    {
        SLog (USR_ERROR, "9999.Account Number No Match Error AccIdx[%d] Input[%12.12s] Accno[%12.12s]",
            acc_seq, dat->account_number,
            Shm_Risk[0].Mst_Acc[acc_seq].acc_no);
        return -9999;
    }

/* ************************************************************************************ */

    /* 매도,매수 */
    mm_flag = AtoIf (dat->ask_bid_type_code, 1);        // 1:매도, 2:매수

    /* 주문Flag 1:신규, 2:정정, 3:취소*/
    order_flag = AtoIf (dat->modify_or_cancel_type_code, 1);

#if     defined A1101||A1102||A1111||A1112			// 현물	
    /* 주문가격 */
	/* 시장가, 조건부지정가, 최유리지정가, 최우선지정가는 시장가, 나머지는 주문단가 */
    if ((memcmp (dat->order_type_code, "1", 1) == 0)	||	// 시장가
        (memcmp (dat->order_type_code, "I", 1) == 0)	||	// 조건부 지정가
        (memcmp (dat->order_type_code, "X", 1) == 0)	||	// 최유리 지정가
        (memcmp (dat->order_type_code, "Y", 1) == 0))		// 최우선 지정가
    {
        if (mm_flag == 1)
            od_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_l_lmt;
        else
            od_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
    }
    else
        od_price = AtoDf (dat->order_price, 11);
#elif     defined A2101||A2102			// 파생
	/* 파생은 시장가/최유리지정가이면 매도매수 상관없이 상한가로 한도를 크게잡는다 */
	if ((memcmp (dat->order_type_code, "T", 1) == 0)    ||   // 시장가(1대신 T)
		(memcmp (dat->order_type_code, "W", 1) == 0))        // 최유리 지정가(X대신 W)
		od_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;	// 상한가
	else
        od_price = AtoDf (dat->order_price, 11);
#endif

    /* 현재가, 0이면 마스터기준가로 */
    if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].crprc > 0)
        mk_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].crprc;
    else
        mk_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_stdard_price;

#if defined A2101||A2102                // 파생
	/* 20200706 스프레드는 가격이 0.00 이면 0.01로 변경해서 계산하게 한다,
     * 0으로 4칙연산을 하면 프로세스가 죽어버리는 경우가 있기때문에 불가피하게 처리
     * 스프레드종목들의 최소공배수는 1이다. */
	if (fabs(od_price - 0) < 0.0001	&&
		(memcmp (dat->MembershipItem+45, "1", 1) == 0))
		od_price = 1;

	if (fabs(mk_price - 0) < 0.0001	&&
		(memcmp (dat->MembershipItem+45, "1", 1) == 0))
		od_price = 1;
#endif

	/* 상하한가 */
    if (memcmp (dat->ask_bid_type_code, "1", 1) == 0)   // 매도
    {
        if (fabs(Shm_Risk[0].S_Sise[mk_gbn][item_seq].realtime_lprc) - 0 > 0.0001)  // >실시간하한가
            hl_lmt_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].realtime_lprc;
        else
            hl_lmt_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_l_lmt;
    }
    else
    if (memcmp (dat->ask_bid_type_code, "2", 1) == 0)   // 매수
    {
        if (fabs(Shm_Risk[0].S_Sise[mk_gbn][item_seq].realtime_hprc) - 0 > 0.0001)  // >실시간하한가
            hl_lmt_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].realtime_hprc;
        else
            hl_lmt_price = Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_h_lmt;
    }

    /* 주문수량 */
    order_quantity = AtoDf (dat->order_quantity, 10);

    /* ***************************************************************************** */
	/* 착오방지, 종목유형코드, 패킷사이즈(거래소요청사항) 체크 시작 */
    /* 1. 종목마스터 SHM 존재 체크 */
#if     defined A1101||A1102    // 유가증권
	if (mk_gbn == 5)
#elif   defined A1111||A1112    // 코스닥
	if (mk_gbn == 6)
#elif   defined A2101||A2102    // 파생
	if (mk_gbn == 1 || mk_gbn == 2 || mk_gbn == 3 || mk_gbn == 4 ||
		mk_gbn == 8 || mk_gbn == 9 || mk_gbn == 10 || mk_gbn == 11 || mk_gbn == 12)
#endif
	{
		if ((memcmp (dat->issue_code,
			 Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_cd, sizeof (dat->issue_code)) != 0))
		{
			SLog (USR_ERROR, "1.Item_Cd Error Input[%12.12s], m_item[%12.12s] issue_code[%12.12s], mk_gbn[%d] idx[%d]",
					dat->issue_code,
					Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_cd,
					dat->issue_code, mk_gbn, item_seq);
			return -101;
		}
	}
	else
	{
        SLog (USR_ERROR, "0.item_sect Error Input mk_gbn[%d]", mk_gbn);
        return -102;
    }

	
    /* 2. 팻킷사이즈 체크 */
    //if ( (strlen(p_buf) - 1) != sizeof(KRX_JUMUN_DATA) )    // 수신자료에 마지막 Enter 뺀다.    
    if ( (strlen(p_buf) - 1) < sizeof(KRX_JUMUN_DATA) )    // 수신자료에 마지막 Enter 뺀다.    
	{
        SLog (USR_ERROR, "2.Packet Size Error p_buf[%d] KRX_JUMUN_DATA[%d]", strlen(p_buf) -1,  sizeof(KRX_JUMUN_DATA));
        return -200;
    }

    /* 3.사번체크 */
    /* 4.주문일자 체크 */
    if (memcmp (Shm_Risk[0].business_day, dat->order_date, sizeof(dat->order_date)) != 0)
    {
        SLog (USR_ERROR, "4.Business Day Error p_buf[%8.8s] db_day[%8.8s]", dat->order_date, Shm_Risk[0].business_day);
        return -400;
    }

    /* 5. 장운영체크 */
#if     defined A1101||A1102 || A1111||A1112    // 유가증권, 코스닥
    if (memcmp (Shm_Risk[0].open_market_info[mk_gbn], "G1", 2) != 0 &&
        memcmp (Shm_Risk[0].open_market_info[mk_gbn], "G2", 2) != 0)
#elif   defined A2101||A2102    // 파생
    if (memcmp (Shm_Risk[0].open_market_info[mk_gbn], "G1", 2) != 0)
#endif
    {
        SLog (USR_ERROR, "5.Open Market Info Error[%2.2s]", Shm_Risk[0].open_market_info[mk_gbn]);
        return -500;
    }

	/* 6. 당사발행상품여부 체크 */
    /* 7. 종목 매매금지 Check   */
	// N:정상, 이외불가
    if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].dont_trade   ||	// 내부 주문불가 설정
        (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_stat, "N", 1) != 0)  )   // 시세종목 주문불가
    {
        SLog (USR_ERROR, "7.Item Don't Trade of This Accno Error item_seq[%d] acc_seq[%d] mk_gbn[%d]",
            item_seq, acc_seq, mk_gbn);
        return -700;
    }

    /* 8. 최대주문번호 체크, 현물/파생 각각 3천만건씩 할당 받음 */
#if     defined A1101||A1102 || A1111||A1112    // 현물
    if ((memcmp (dat->order_identification, "1530000000", 10) < 0)  ||
        (memcmp (dat->order_identification, "1559999999", 10) > 0)  )
#elif   defined A2101||A2102    // 파생
    if ((memcmp (dat->order_identification, "1580000000", 10) < 0)  ||
        (memcmp (dat->order_identification, "1609999999", 10) > 0)  )
#endif
    {
        SLog (USR_ERROR, "8.Order No Error input[%10.10s]", dat->order_identification);
        return -800;
    }

#if     defined A1101||A1102 || A1111||A1112    // 현물
    /* 9. 주문가격 Check (신규/정정 가격이 0일경우) */
    if (order_flag == 1 ||  order_flag == 2)        // 1:신규, 2:정정
    {
        if ((od_price <= 0) &&
            (memcmp (dat->order_type_code, "2",            1) == 0))     // 지정가
        {
            SLog (USR_ERROR, "9.Order Price Error price[%lf] type[%1.1s]",
                    od_price, dat->order_type_code);
            return -901;
        }
    }
#elif   defined A2101||A2102                    // 파생
    /* 지정가인데 가격이 0이면 오류(스프레드는 0.00이 올수있다. 그리고 마이너스 가격도 있으니 제외)	*/
    if ((order_flag == 1 || order_flag == 2)				&&	// 신규,정정
        od_price < 0.0001									&&	// 0.00보다작으면
		(memcmp (dat->MembershipItem+45, "1", 1) != 0)		&&	// !스프레드
        (memcmp (dat->order_type_code, "2", 1) == 0))       	// 지정가
    {
        SLog (USR_ERROR, "9.Order Price Error price[%lf] type[%1.1s] order_flag[%d]",
                od_price, dat->order_type_code, order_flag);
        return -902;
    }
#endif

    /* 10. 주문수량 Check (신규/정정 수량 0일 경우) */
#if 0
/* 202112 */
    if (order_flag != 3 &&                      // '3' 취소아니면서(신규또는정정)
        order_quantity <= 0 )           // 주문수량 0보다 같거나 작으면
#endif
    if (order_quantity <= 0 )           // 주문수량 0보다 같거나 작으면
    {
        SLog (USR_ERROR, "10.Order Quantity Error qty[%lf]", order_quantity);
        return -1000;
    }

    /* 11. 프로그램매매구분코드 Check */
    /* 12. 원주문번호 Check, 정정취소시 원주문번호가 없으면 에러 */
    if ((order_flag == 2 || order_flag == 3)                                &&                      // 정정/취소
        (memcmp (dat->original_order_identification, "0000000000", 10) <= 0))   // 원주문번>호
	{
        SLog (USR_ERROR, "12.Order Quantity Error order_flag[%d] 원주문번호[%10.10s]", order_flag, dat->original_order_identification);
        return -1200;
    }

    /* 13. 취소가 아닐 경우에 호가유형코드, 호가조건코드 Check */
    if (order_flag != 3)        // 3:취소
    {
#if     defined A1101||A1102 || A1111||A1112    // 현물
        if (((memcmp (dat->order_type_code,      "1", 1) != 0)  &&          // 호가유형코드
             (memcmp (dat->order_type_code,      "2", 1) != 0)  &&
             (memcmp (dat->order_type_code,      "I", 1) != 0)  &&
             (memcmp (dat->order_type_code,      "X", 1) != 0)  &&
             (memcmp (dat->order_type_code,      "Y", 1) != 0)  )       ||

            ((memcmp (dat->order_condition_code, "0", 1) != 0) &&           // 호가조건코드
             (memcmp (dat->order_condition_code, "3", 1) != 0)  &&
             (memcmp (dat->order_condition_code, "4", 1) != 0)))
        {
            SLog (USR_ERROR, "13.Order Type, Order Condition Error Type[%1.1s] Condition[%1.1s]",
                    dat->order_type_code, dat->order_condition_code);
            return -1300;
        }
#elif   defined A2101||A2102                    // 파생
        if (((memcmp (dat->order_type_code,      "T", 1) != 0)  &&          // 호가유형코드
             (memcmp (dat->order_type_code,      "2", 1) != 0)  &&
             (memcmp (dat->order_type_code,      "I", 1) != 0)  &&
             (memcmp (dat->order_type_code,      "W", 1) != 0)  )       ||

            ((memcmp (dat->order_condition_code, "0", 1) != 0) &&           // 호가조건코드
             (memcmp (dat->order_condition_code, "3", 1) != 0)  &&
             (memcmp (dat->order_condition_code, "4", 1) != 0)))
        {
            SLog (USR_ERROR, "13.Order Type, Order Condition Error Type[%1.1s] Condition[%1.1s]",
				dat->order_type_code, dat->order_condition_code);
            return -1300;
		}

#endif
    }
	/* 착오방지, 종목유형코드, 패킷사이즈(거래소요청사항) 체크 종료 */
    /* ***************************************************************************** */

#if 0
/* 20211218 중복이라 제외 */
    /* ******************** */
    /* 14. 호가적합성 Check */
    /* 14-1-1. 종목마스터의 종목별 거래가능여부, 종목장운영의 거래가능여부 체크 */
    if (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_stat, "0", 1) != 0)	// 0:Normal, !0:Cannot
    {
        SLog (USR_ERROR, "14-1-1.Item Info Cannot Order Error[%1.1s]",
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_stat);
        return -1411;
    }
#endif

    /* ************************************** */
    /* 14.2 호가적합성 Check, 신규/정정일때만  */
    if (order_flag == 1 || order_flag == 2)     // 1:신규, 2:정정
    {
#if     defined A1101||A1102 || A1111||A1112    // 현물
        /* 14-2-1 호가단위 제한 Check */
        // 함수 사용, 호가단위 정상여부, 아래에서 처리(착오주문처리17)

        /* 14-2-2 가격제한폭 */
        // 이것이 아니고 => EW:주식워런트증권, SR:신주인수권증서, SW:신주인수권증권, EF(ETF), EN(ETN)
        // 정리매매가 아닐때
        // 호가타입이 '지정가' 또는 '조건부지정가' 인 경우 Check한다.
/* 체크로직이 뭔지 모르겠다.(미래에서도 모르겠다고 함, Skip)
        if ((memcmp (MarketGbn, "011D", 4) != 0) &&                         // 거래소신주인>수증권아니다
            (memcmp (MarketGbn, "012E", 4) != 0) &&                         // 코스닥신주인>수증권아니다
            (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_clearance_gbn, "N", 1) == 0) && // 정리매매 아니다.
            ((memcmp (dat->order_type_code, "2", 1) == 0) ||
             (memcmp (dat->order_type_code, "I", 1) == 0))      )               // 지정가 또는 조건부지정가
        {
            SLog (USR_ERROR, "14-2-2. Error item_sect[%4.4s]", MarketGbn);
            return -1422;
        }
*/

        /* 14-2-3 시가기준가종목 가격제한 체크 */
        // 시가기준가종목 가격제한 체크, 최고호가는 9자리고 주문가격은 11자리라 끊어서 비교>한다
        if ((memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_start_price_gbn, "Y", 1) == 0)  &&
            (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_clearance_gbn,   "N", 1) == 0))
		{
            if ((mm_flag == 1   &&  // 1:매도
                 (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_lowest_price > od_price))  ||
                (mm_flag == 2   &&  // 2:매수
                 (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_top_price < od_price)) )
            {
                SLog (USR_ERROR, "14-2-3. Error ask_bid[%d]", mm_flag);
                return -1423;
            }
        }

        // 14-2-4
        // 이것이 아니고 => EW:주식워런트증권, SR:신주인수권증서, SW:신주인수권증권
        // 정리매매가 아닐때
        // 호가타입이 '조건부지정가' 인 경우 Check한다.
		if ((memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "EW", 2) != 0	 &&
			 memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "EF", 2) != 0  &&
			 //memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "EN", 2) != 0  &&
			 memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "SR", 2) != 0  &&
			 memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "SW", 2) != 0)  &&
            (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_clearance_gbn, "N", 1) == 0) && // 정리매매 아니다.
            ((memcmp (dat->order_type_code, "2", 1) == 0) ||
             (memcmp (dat->order_type_code, "I", 1) == 0))      )               // 지정가 또는 조건부지정가
        {
            if (mm_flag == 1)   // 1:매도
            {
                // 20200708 로직추가, 상/하한가에 0이면 통과.신주인수권, ELW등 상하한가 없다.
                if (od_price < hl_lmt_price && hl_lmt_price != 0)   //하한가보다 작으면 오류 
				{
                    SLog (USR_ERROR, "14-2-4.11 start price check Error order.No[%10.10s] order_price[%lf] low_price[%lf]",
                            dat->order_identification, od_price, hl_lmt_price);
                    return -1424;
				}
			}
            else
            if (mm_flag == 2)   // 2:매수
            {
                // 20200708 로직추가, 상/하한가에 0이면 통과.신주인수권, ELW등 상하한가 없다.
                if (od_price > hl_lmt_price && hl_lmt_price != 0)   //상한가보다 크면 오류
                {
                    SLog (USR_ERROR, "14-2-4.12 start price check Error order.No[%10.10s] order_price[%lf] hi_low_price[%lf]",
                            dat->order_identification, od_price, hl_lmt_price);
                    return -1424;
                }
            }
        }
#elif  defined A2101||A2102             // 파생
        // 함수 사용, 호가단위 정상여부, 아래에서 처리(착오주문처리17)
#endif
    }

    /* 14-3 주식일때 호가종류 체크 */
#if     defined A1101||A1102 || A1111||A1112    // 현물
    // '1'일때 종목MAST 시장가 호가조건코드 항목
    // '2'일때 종목MAST 지정가 호가조건코드 항목
    // 'I'일때 종목MAST 조건부지정가 호가조건코드 항목
    // 'X'일때 종목MAST 최유리지정가 호가조검코드 항목
    // 'Y'일때 종목MAST 최유리지정가 호가조검코드 항목
/* 20211218 요청에 의해 로직 막음 
    if (memcmp (dat->order_type_code, "1", 1) == 0)         // 시장가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 7)
            {
				SLog (USR_ERROR, "14-3.1 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1431;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 7)
            {
                SLog (USR_ERROR, "14-3.2 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1432;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_1 != 7)
            {
                SLog (USR_ERROR, "14-3.3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1433;
            }
        }
	}
    else
    if (memcmp (dat->order_type_code, "2", 1) == 0)         // 지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-3.4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1434;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-3.5 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1435;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-3.6 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1436;
            }
        }
    }
    else
    if (memcmp (dat->order_type_code, "I", 1) == 0)         // 조건부지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
                SLog (USR_ERROR, "14-3.7 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                            dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1437;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
				SLog (USR_ERROR, "14-3.8 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1438;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
                SLog (USR_ERROR, "14-3.9 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1439;
            }
        }
    }
    else
    if (memcmp (dat->order_type_code, "X", 1) == 0)         // 최유리지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 7)
            {
                SLog (USR_ERROR, "14-40.X0 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1440;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 7)
            {
                SLog (USR_ERROR, "14-41.X3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1441;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_X != 7)
            {
                SLog (USR_ERROR, "14-42.X4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1442;
            }
        }
    }
    else
	if (memcmp (dat->order_type_code, "Y", 1) == 0)         // 최우선지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 7)
            {
                SLog (USR_ERROR, "14-43.Y0 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1443;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 7)
            {
                SLog (USR_ERROR, "14-44.Y3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1444;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_Y != 7)
            {
                SLog (USR_ERROR, "14-45.Y4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1445;
            }
        }
    }
    else
    {
        if (order_flag != 3)
        {
            SLog (USR_ERROR, "14-46.No Type call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                    dat->order_identification, dat->order_type_code, dat->order_condition_code);
            return -1446;
        }
    }
#elif  defined A2101||A2102             // 파생
    // 'T'일때 종목MAST 시장가 호가조건코드 항목, 현물 1과 동일
    // '2'일때 종목MAST 지정가 호가조건코드 항목
    // 'I'일때 종목MAST 조건부지정가 호가조건코드 항목
    // 'W'일때 종목MAST 최유리지정가 호가조검코드 항목, 현물 X과 동일
    if (memcmp (dat->order_type_code, "T", 1) == 0)         // 시장가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 7)
            {
                SLog (USR_ERROR, "14-47.T call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1447;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 7)
            {
                SLog (USR_ERROR, "14-48.13 T3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1448;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_T != 7)
            {
                SLog (USR_ERROR, "14-49.14 T4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1449;
            }
        }
    }
    else
    if (memcmp (dat->order_type_code, "2", 1) == 0)         // 지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-50.20 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1450;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-51.23 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1451;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_2 != 7)
            {
                SLog (USR_ERROR, "14-52.24 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1452;
            }
        }
    }
    else
    if (memcmp (dat->order_type_code, "I", 1) == 0)         // 조건부지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
                SLog (USR_ERROR, "14-53.I0 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                            dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1453;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
                SLog (USR_ERROR, "14-54.I3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1454;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_I != 7)
            {
                SLog (USR_ERROR, "14-55.I4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1455;
            }
        }
    }
    else
    if (memcmp (dat->order_type_code, "W", 1) == 0)         // 최유리지정가
    {
        if (memcmp (dat->order_condition_code, "0", 1) == 0)        // 일반
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 1   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 7)
            {
                SLog (USR_ERROR, "14-56.W0 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1456;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "3", 1) == 0)        // FAK(IOC)
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 2   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 3   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 7)
            {
                SLog (USR_ERROR, "14-57.W3 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1457;
            }
        }
        else
        if (memcmp (dat->order_condition_code, "4", 1) == 0)        // FOK
        {
            if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 4   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 5   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 6   &&
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_cds_gbn_W != 7)
            {
                SLog (USR_ERROR, "14-58.W4 call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                        dat->order_identification, dat->order_type_code, dat->order_condition_code);
                return -1458;
            }
        }
    }
    else
	{
        if (order_flag != 3)
        {
            SLog (USR_ERROR, "14-59.No Type call price check Error order.No[%10.10s] type[%1.1s] cds[%1.1s]",
                    dat->order_identification, dat->order_type_code, dat->order_condition_code);
            return -1459;
        }
    }
20211217 */
#endif

    /* 14-4 호가유형별 호가입력제한 체크 */
#if     defined A1101||A1102 || A1111||A1112    // 현물
    // 시가기준가결정종목경우 호가유형이 시장가, 조건부지정가 인 경우 호가입력제한 거부
    if (((memcmp (dat->order_type_code, "1", 1) == 0)   ||  // 시장가
         (memcmp (dat->order_type_code, "I", 1) == 0))  &&  // 조건부지정가
        (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_start_price_gbn, "Y", 1) == 0)  )
    {
        SLog (USR_ERROR, "14-60.호가입력제한 오류 Error order.No[%10.10s] order_type_code[%1.1s]",
                dat->order_identification, dat->order_type_code);
        return -1460;
    }
    // 14-42. 보드이벤트정보가 아래와 같을때 호가유형은  X:최유리(현물만), Y:최우선(현물만) 일수 없음
    if (((memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AA1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "BC1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AE1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AF1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AG1", 3) == 0))     &&
        ((memcmp (dat->order_type_code, "X", 1) == 0)   ||                  // 최유리
         (memcmp (dat->order_type_code, "Y", 1) == 0))  )                   // 최우선
    {
        SLog (USR_ERROR, "14-61.호가입력제한 오류2 Error order.No[%10.10s] order_type_code[%1.1s] sub_market_info[%3.3s]",
                dat->order_identification, dat->order_type_code, Shm_Risk[0].sub_market_info[mk_gbn]);
        return -1461;
    }
#elif  defined A2101||A2102             // 파생
    // 14-41. 단일가호가인 경우에는 최유리지정가호가 불가(취소호가는 가능)
    //     => board_event in ("AA1", "BC1", "AE1", "AF1", "AG1") 일때 ORD_TYPE 이 'W'이면 오류 (단일가호가 중 최유리지정가불가)
    if ((order_flag != 3)  &&
        ((memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AA1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "BC1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AE1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AF1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AG1", 3) == 0))     &&
        (memcmp (dat->order_type_code, "W", 1) == 0)    )                   // 최유리
    {
        SLog (USR_ERROR, "14-62.호가입력제한 오류1 Error order.No[%10.10s] order_type_code[%1.1s] sub_market_info[%3.3s]",
                dat->order_identification, dat->order_type_code, Shm_Risk[0].sub_market_info[mk_gbn]);
        return -1462;
    }
    // 14-42. 종가단일가(121)인 경우에는 조건부지정가호가 불가(최소호가는 가능)
    //     => board_event = 'BC1' 일때 ORD_TYPE 이 'I' 일때 오류 (종가단일가 중 조건부지정가 불가
    if ((order_flag != 3)   &&
        (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "BC1", 3) == 0)   &&
        (memcmp (dat->order_type_code, "I", 1) == 0)    )
    {
        SLog (USR_ERROR, "14-63.호가입력제한 오류2 Error order.No[%10.10s] order_type_code[%1.1s] sub_market_info[%3.3s]",
                dat->order_identification, dat->order_type_code, Shm_Risk[0].sub_market_info[mk_gbn]);
        return -1463;
    }
    // 14-43. 최종거래일인 경우에는 조건부지정가호가불가
    if ((memcmp (dat->order_type_code, "I", 1) == 0)    &&  // 조건부지정가
        (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_closeday, Shm_Risk[0].business_day, 8) <= 0))
    {
        SLog (USR_ERROR, "14-64.호가입력제한 오류3 Error order.No[%10.10s] closeday[%8.8s] business_day[%8.8s]",
                dat->order_identification,
                Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_closeday,
                Shm_Risk[0].business_day);
        return -1464;
    }
    // 14-44. 선물스프레드 종목인 경우
    //     => ORD_TYPE 'T', 'I', 'W' 만 가능
    //     => 단일가호가시간의 경우 주문불가(취소제외) -> board_event in ("AA1", "BC1", "AE1", "AF1", "AG1") 일때 주문불가
	if ((memcmp (dat->MembershipItem+45, "1", 1) == 0)	&&	// 스프레드
         (
          (order_flag != 3)  &&
          (memcmp (dat->order_type_code, "T", 1) != 0)  &&
          (memcmp (dat->order_type_code, "I", 1) != 0)  &&
          (memcmp (dat->order_type_code, "W", 1) != 0))     )
    {
        SLog (USR_ERROR, "14-65.호가입력제한 오류4 Error order.No[%10.10s] order_type_code[%1.1s] mk_gbn[%d]",
                dat->order_identification, dat->order_type_code, mk_gbn);
        return -1465;
    }
	if ((memcmp (dat->MembershipItem+45, "1", 1) == 0)	&&	// 스프레드
		(
          (order_flag != 3)  &&
//        ((memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AA1", 3) == 0)    ||     * 주식선물스프레드 예약 주문시 AA1에 주문 발주.
          ((memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "BC1", 3) == 0)    ||
           (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AE1", 3) == 0)    ||
           (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AF1", 3) == 0)    ||
           (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AG1", 3) == 0)))  )
    {
        SLog (USR_ERROR, "14-66.호가입력제한 오류5 Error order.No[%10.10s] order_type_code[%1.1s] sub_market_info[%3.3s] order_flag[%d]",
                dat->order_identification,
                dat->order_type_code,
                Shm_Risk[0].sub_market_info[mk_gbn],
                order_flag);
        return -1466;
    }
    // 14-46. IOC, FOK인경우
    //     => ORD_TYPE = 'I'  불가 (IOC, FOK 주문의 조건부지정가호가 불가)
    // 14-47.
    //     => borad_event in ("AA1", "BC1", "AE1", "AF1", "AG1") 일때 오류 (IOC, FOK 주문의 단일가호가 불가)
    if ((memcmp (dat->order_type_code, "I", 1) == 0)            &&
        ((memcmp (dat->order_condition_code, "3", 1) == 0)  ||      // FAK(IOC)
         (memcmp (dat->order_condition_code, "4", 1) == 0)) )       // FOK
    {
        SLog (USR_ERROR, "14-67.호가입력제한 오류6 Error order.No[%10.10s] order_type_code[%1.1s] order_condition_code[%1.1s]",
                dat->order_identification, dat->order_type_code, dat->order_condition_code);        return -1467;
    }
    if (((memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AA1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "BC1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AE1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AF1", 3) == 0)  ||
         (memcmp (Shm_Risk[0].sub_market_info[mk_gbn], "AG1", 3) == 0))     &&
        ((memcmp (dat->order_condition_code, "3", 1) == 0)  ||      // FAK(IOC)
         (memcmp (dat->order_condition_code, "4", 1) == 0)) )       // FOK
    {
        SLog (USR_ERROR, "14-68.호가입력제한 오류7 Error order.No[%10.10s] order_condition_code[%1.1s] sub_market_info[%3.3s]",
                dat->order_identification, dat->order_condition_code, Shm_Risk[0].sub_market_info[mk_gbn]);
        return -1468;
    }
    // 14-48. 조건부지정가 매수인 경우 상한가로 지정된 호가 (조건부지정가의 상한가 매수불가)    
	//        조건부지정가 매도인 경우 하한가로 지정된 호가 (조건부지정가의 하한가 매도불가)    
	if (((memcmp (dat->order_type_code, "I", 1) == 0)   &&
         ((mm_flag == 1 &&  od_price <= hl_lmt_price)   ||      // 하한가보다 작거나 같으면 오류
          (mm_flag == 2 &&  od_price >= hl_lmt_price)))	&&      // 상한가보다 크거나 같으면 오류        &&
        fabs(hl_lmt_price - 0) > 0.0001)
    {
        SLog (USR_ERROR, "14-69.호가입력제한 오류8 Error mm_flag[%d] order.No[%10.10s] order_condition_code[%1.1s] od_price[%lf] hl_lmt_price[%lf]",
                mm_flag, dat->order_identification, dat->order_condition_code, od_price, hl_lmt_price);
        return -1469;
    }

    /* 상하한가 체크, 지정가일경우만 체크, 조건부지정가는 상하한가 주문 불가이기 때문. */
    if (memcmp (dat->order_type_code, "2", 1) == 0)
    {
        if (mm_flag == 1)   // 1:매도
        {
            // 20200708 로직추가, 상/하한가에 0이면 통과.신주인수권, ELW등 상하한가 없다.
            if (od_price < hl_lmt_price && fabs(hl_lmt_price - 0) > 0.0001) //하한가보다 작으면 오류
            {
                SLog (USR_ERROR, "14-70.11 start price check Error order.No[%10.10s] order_price[%lf] low_price[%lf]",
                        dat->order_identification, od_price, hl_lmt_price);
                return -1470;
            }
        }
        else
        if (mm_flag == 2)   // 2:매수
        {
            // 20200708 로직추가, 상/하한가에 0이면 통과.신주인수권, ELW등 상하한가 없다.
            if (od_price > hl_lmt_price && fabs(hl_lmt_price - 0) > 0.0001) //상한가보다 크면 오류
            {
                SLog (USR_ERROR, "14-71.12 start price check Error order.No[%10.10s] order_price[%lf] hi_low_price[%lf]",
                        dat->order_identification, od_price, hl_lmt_price);
                return -1471;
            }
        }
    }
#endif

    /* 14-5 상한수량 Check (신규만) */
    // 상한수량은 16자리, 호가수량은 10자리 그래서 뒤 10자리만 체크한다.
    if (order_flag == 1)            // 1:신규
    {
        if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_max_qty < AtoDf (dat->order_quantity, 10))
        {
            SLog (USR_ERROR, "14-72.Max Quantity Over Error order.No[%10.10s] Max[%lf] input[%10.10s]",
                    dat->order_identification, Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_max_qty, dat->order_quantity);
            return -1472;
        }
    }

    /* 15. DW_OrdCtl_Check 업무계 실시간 제어 */
	/* 16. 한도체크, 아래에서 처리  */

    /* 17. 착오주문체크, 금액기준은 기준가로 하고 기준가가 0이면 전일종가로 함 */
    // 신규, 정정만 착오주문을 체크함.
    if (order_flag == 1 || order_flag == 2)         // 1:신규, 2:정정
    {
#if defined A1101||A1102 || A1111||A1112    // 현물
        // 17-1.현물 20억 초과여부
		if (mk_gbn == 5 || mk_gbn == 6)			// 유가증권 or 코스닥
        {
            if (2000000000 <                                        // 2,000,000,000
                (od_price * order_quantity) )
            {
                SLog (USR_ERROR, "17-1.MisTake Order Price 2,000,000,000 Over Error order.No[%10.10s] 기준Price[%lf] 주문가 [%lf] Qtt[%lf] mk_gbn[%d]",
                        dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1701;
            }

            /* 호가 정합성 체크, 현물 */
            if (memcmp (dat->MembershipItem+44, "0", 1) == 0)		// Normal, Not ELW/ETN/ETF
                tick_diff = Tick_Chk (mk_gbn, mk_price, od_price); // 5,6
            else
			if (memcmp (dat->MembershipItem+44, "1", 1) == 0)		// ELW/ETN/ETF
                tick_diff = Tick_Chk (7, mk_price, od_price); // 7.ELW/ETN/ETF
			else
            {
                SLog (USR_ERROR, "17-2.01 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
				tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1702;
			}

            if (tick_diff < 0)      // 호가가격 비정상 에러, Tick 체크 없음
            {
                SLog (USR_ERROR, "17-2.02 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1703;
            }
            else    // 틱 방향때문에 부호를 넣어준다.
            {
                if (mk_price > od_price)
                    tick_diff = -1 * tick_diff;         // 현재가보다 주문가가 더 낮으면 음>수
            }
        }

        // 17-1. 신주인수권, 20억 초과여부
        else
		if (memcmp (Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_group_id, "SW", 2) == 0)	// 신주인수권
        {
            if (2000000000 <                                        // 2,000,000,000
                (od_price * order_quantity) )
            {
                SLog (USR_ERROR, "17-1.MisTake Order Price 2,000,000,000 Over Error order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        dat->order_identification, mk_price, od_price, order_quantity);
                return -1704;
            }

            /* 호가 정합성 체크, 현재가대비 Tick 체크, 신주인수권/증서 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price); // 유가증권 신주인수권은 유가증권과 동일체크, 코스닥 신주인수권 코스닥과 동일체크

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-2 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1705;
            }
            else    // 틱 방향때문에 부호를 넣어준다.
            {
                if (mk_price > od_price)
                    tick_diff = -1 * tick_diff;             // 현재가보다 주문가가 더 낮으면 음수
            }

            /* 17-4. 호가수량이 1%넘는지 Check */
            if (memcmp (dat->order_quantity, Shm_Risk[0].S_Sise[mk_gbn][item_seq].nb_shares_stock_1per, 10) > 0)
            {
                SLog (USR_ERROR, "17-4. 1 Persent Over quantity Error input[%10.10s] 1persent[%10.10s]",
                        dat->order_quantity, Shm_Risk[0].S_Sise[mk_gbn][item_seq].nb_shares_stock_1per);
                return -1706;
            }
        }

#elif  defined A2101||A2102             // 파생
		/* 스프레드는 5개시장이 있는데 지수선물/미니선물/KRX300/코스닥150선물 4개는 본시장과 같이 체크하고 주식선물만 따로 처리한다 */
		if (mk_gbn == 1)				// 지수선물, 50틱/600계약
        {
            /* 호가 정합성 체크, 현재가대비 Tick 체크 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-01 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Std_Price[%lf] od_price[%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1701;
            }

			if (tick_diff > 50)
            {
                SLog (USR_ERROR, "17-02 Call Order Price Check Error 60tick Over tick_diff[%d] order.No[%10.10s] Price[%lf] od_price[%lf] Qtt[%lf] mk_gbn[%d]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1702;
            }

			if (memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
			{
				/* 17. 호가수량이 600계약 넘는지 Check */
				if (AtoDf (dat->order_quantity, 10) > 600)
				{
					SLog (USR_ERROR, "17-03. 600 Quantity Over Error input[%10.10s] mk_gbn[%d]", dat->order_quantity, mk_gbn);
					return -1703;
				}
			}
        }

		else
		if (mk_gbn == 11)				// 미니선물, 125틱/3000계약
        {
            /* 호가 정합성 체크, 현재가대비 Tick 체크 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-04 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Std_Price[%lf] od_price[%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1704;
            }

			if (tick_diff > 125)
            {
                SLog (USR_ERROR, "17-05 Call Order Price Check Error 125tick Over tick_diff[%d] order.No[%10.10s] Price[%lf] od_price[%lf] Qtt[%lf] mk_gbn[%d]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1705;
            }

			if (memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
			{
				/* 17. 호가수량이 3000계약 넘는지 Check */
				if (AtoDf (dat->order_quantity, 10) > 3000)
				{
					SLog (USR_ERROR, "17-06. 3000 Quantity Over Error input[%10.10s] mk_gbn[%d]", dat->order_quantity, mk_gbn);
					return -1706;
				}
			}
        }

        else
		if (mk_gbn == 9)			// KSQ150 선물, 50틱/300계약
        {
            /* 호가 정합성 체크, 현재가대비 Tick 체크 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-07 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1707;
            }

			if (tick_diff > 50)
            {
                SLog (USR_ERROR, "17-08 Call Order Price Check Error 50tick Over tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1708;
            }

			if (memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
			{
				/* 17. 호가수량이 300계약 넘는지 Check */
				if (AtoDf (dat->order_quantity, 10) > 300)
				{
					SLog (USR_ERROR, "17-09. 300 Quantity Over Error input[%10.10s] mk_gbn[%d]", dat->order_quantity, mk_gbn);
					return -1709;
				}
			}
		}

        else
		if (mk_gbn == 8)		// KRX300, 50틱/600계약
        {
            /* 호가 정합성 체크, 현재가대비 Tick 체크 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-10 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1710;
            }

			if (tick_diff > 50)
            {
                SLog (USR_ERROR, "17-11 Call Order Price Check Error 50tick Over tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1711;
            }

			if (memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
			{
				/* 17-4. 호가수량이 600계약 넘는지 Check */
				if (AtoDf (dat->order_quantity, 10) > 600)
				{
					SLog (USR_ERROR, "17-12. 600 Quantity Over Error input[%10.10s] mk_gbn[%d]", dat->order_quantity, mk_gbn);
					return -1712;
				}
			}
        }

        else	// 주식선물, 5억만
				// 주식선물스프레드, 50틱만
		if (mk_gbn == 3 &&									// 주식 선물(코스피)
			memcmp (dat->MembershipItem+46, "1", 1) == 0	&&	// 코스피종목
			memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
        {
            // 17. 주식선물, 5억 초과여부 
            if (500000000  < 
                (od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier) )
            {
                SLog (USR_ERROR, "17-13.MisTake Order Price 5억 Over Error order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1713;
            }

			/* 주식선물 스프레드가 아닌건 5억초과여부만 체크하지만 Tick_Chk를 통해서 호가가격이 정확하게 입력된건지 체크한다. 음수면 호가단위위반 */
            /* 호가 정합성 체크, 현재가대비 Tick 체크는 안함 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);
            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-14 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1714;
            }
        }

        else
		if (mk_gbn == 3 &&									// 주식 선물(코스닥)
			memcmp (dat->MembershipItem+46, "2", 1) == 0	&&	// 코스닥
			memcmp (dat->MembershipItem+45, "1", 1) != 0)		// !스프레드(45)
        {
            // 17. 주식선물, 5억 초과여부
            if (500000000 <
                (od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier) )
            {
                SLog (USR_ERROR, "17-15.MisTake Order Price 5억 Over Error order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1715;
            }

			/* 주식선물 스프레드가 아닌건 5억초과여부만 체크하지만 Tick_Chk를 통해서 호가가격이 정확하게 입력된건지 체크한다. 음수면 호가단위위반 */
            /* 호가 정합성 체크, 현재가대비 Tick 체크는 안함 */
            tick_diff = Tick_Chk (0, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-16 Call Order Price Check Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1716;
            }
        }

		else		// 주식선물스프레드, 50틱만
		if (mk_gbn == 3 &&									// 주식 선물스프레드(코스피)
			memcmp (dat->MembershipItem+45, "1", 1) == 0)		// 스프레드(45)
        {
            /* 호가 정합성 체크, 현재가대비 Tick 체크 */

			/* 스프레드는 근월물과 원월물의 기준가를 비교해서 작은가격의 호가단위에 해당하는 단위로 스프레드종목의 틱을 계산한다. */
			if (Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+4,  5)].m_stdard_price >
				Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+16, 5)].m_stdard_price)
				in_std_price = Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+16, 5)].m_stdard_price;
			else
				in_std_price = Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+4,  5)].m_stdard_price;

			if (memcmp (dat->MembershipItem+46, "1", 1) == 0)	// 기초자산이 코스피종목
				tick_diff = JS_Tick_Chk (3, in_std_price, mk_price, od_price);
			else												// 기초자산이 코스닥종목
				tick_diff = JS_Tick_Chk (0, in_std_price, mk_price, od_price);

            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-20 Call Order Price Check Error rt[%d] tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        rt, tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1720;
            }

			if (tick_diff > 50)
            {
                SLog (USR_ERROR, "17-21 Call Order Price Check 50tick Over Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1721;
            }
        }

        else
		if (mk_gbn == 2 || mk_gbn == 4 || mk_gbn == 10 || mk_gbn == 12)		// 2.지수옵션, 4:주식옵션, 10:KSD150옵션, 12:미니옵션
        {
			/* 지수옵션/미니옵션(20억), 주식옵션(2억), 코스닥150옵션(5억), 틱체크없음 */
			long	chk_gum;
			if (mk_gbn == 2 || mk_gbn == 12)	// 2:지수옵션, 12:미니옵션 20억
				chk_gum = 2000000000;
			else if (mk_gbn == 4)				// 4:주식옵션 2억
				chk_gum =  200000000;
			else if (mk_gbn == 10)				// 10:KSD150옵션 5억
				chk_gum =  500000000;

            if (chk_gum <
                (od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier) )
            {
                SLog (USR_ERROR, "17-30.MisTake Order Price 1 Time Money Over Error order.No[%10.10s] Price[%lf][%lf] Qtt[%lf] mk_gbn[%d]",
                        dat->order_identification, mk_price, od_price, order_quantity, mk_gbn);
                return -1730;
            }

            /* 호가 정합성 체크, 현재가대비 Tick 체크 */
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);
            if (tick_diff < 0)
            {
                SLog (USR_ERROR, "17-31 Call Order Price Check 호가정합성 Error tick_diff[%d] order.No[%10.10s] Price[%lf][%lf] Qtt[%lf]",
                        tick_diff, dat->order_identification, mk_price, od_price, order_quantity);
                return -1731;
            }
        }
#endif
    }

	/* 18. RISK 오류방지 설정체크, 신규주문 처리한다. */
    // 1회 주문금액, (수량 or 수량%), (Tick범위 or 가격%), 미설정시 주문불가
	if (order_flag == 1)          // 신규 && 상품계좌만 처리한다.
	{
/* 20220204 */
#if     defined A1101||A1102			// 유가증권	
		if (memcmp (dat->ask_bid_type_code, "1", 1) == 0)	// 매도
		{
			/* 현물 공매도주문 체크 (잔고가 매도잔고 or 매도주문수량+매도미체결 > 잔고) */
			if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt < 0	||
				(int)order_quantity + Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt >
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt)
			{
                SLog (USR_ERROR, "18-01.공매도주문 오류 주문번호[%10.10s] 수량[%lf] 잔고[%d] 매도미체결수량[%d]",
                        dat->order_identification, order_quantity,
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt,
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt);
                return -1801;
            }
		}
#elif     defined A2101||A2102			// 파생	
		/* 1회한도체크 항목은 Tick/계약수/계약금액이다. 이중 하나도 체크가 안되어 있으면 오류(계좌별/시장별 운용상품분류코드단위) */
		if (memcmp (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].qty_gbn,   "Y", 1) != 0	&&	// 수량체크
			memcmp (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].money_gbn, "Y", 1) != 0	&&	// 금액체크
			memcmp (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].tick_gbn,  "Y", 1) != 0)		// 틱체크
		{
			SLog (USR_ERROR, "18-10.1 OneTime Chk Flag Error order.No[%10.10s]", dat->order_identification);
			return -1810;
		}

		// 1회 주문금액 체크
		if ((memcmp (Shm_Risk[0].O_M_Fund[acc_seq][acc_seq].money_gbn, "Y", 1) == 0)    &&  	// 1회 주문한도 설정되 있어야 체크
			(memcmp (dat->MembershipItem+45, "1", 1) != 0)								&&		// !스프레드(45)
			Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].money < (od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))
		{
			SLog (USR_ERROR, "18-11.1 OneTime Money Over Error order.No[%10.10s] Price[%lf] Qtt[%lf] mk_gbn[%d] M_order_money[%lf]",
					dat->order_identification, od_price, order_quantity, mk_gbn, Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].money);
			return -1811;
		}

        // 1회 주문수량 체크
        if ((memcmp (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].qty_gbn, "Y", 1) == 0)	&&		// 1회 주문수량한도 설정되 있어야 체크, !스프레드
            (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].qty < order_quantity))
        {
			SLog (USR_ERROR, "18-12.1 OneTime Qtt Over Error order.No[%10.10s] Qtt[%lf][%lf] mk_gbn[%d]",
					dat->order_identification,
					Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].qty, order_quantity, mk_gbn);
			return -1812;
        }

		// 1회 틱 체크
		if ((memcmp (Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].tick_gbn, "Y", 1) == 0)	&&     	// 1회 주문 Tick 체크, !스프레드
			(memcmp (dat->MembershipItem+45, "1", 1) != 0)								)		// !스프레드(45)
		{
            tick_diff = Tick_Chk (mk_gbn, mk_price, od_price);
			if (tick_diff > Shm_Risk[0].O_M_Fund[risk_mk_gbn][acc_seq].tick)			// 받음 TICK은 절대값
			{
				SLog (USR_ERROR, "18-13.1 OneTime Tick Check Over Error order.No[%10.10s] tick_diff[%d] Qtt[%lf] mk_gbn[%d]",
						dat->order_identification, tick_diff, order_quantity, mk_gbn);
				return -1813;
			}
		}
#endif

        /* 19. 누적 한도 Check */
        // 현물은 총주문금액, 파생은 총수량 체크
        // 정정주문은 미미하니 SKIP, 취소는 주문에서는 SKIP하고 체결쪽으로 확인되면 거기서 >빼준다.
        // 여기서는 신규만 한도를 잡아준다.

/* 20220204 */
#if     defined A1101||A1102
        // Risk
        if (mm_flag == 2)				// 2:매수
        {
			/* 로직 : 매수한도금액 < 상품주식 상품포지션 장부금액합 + 주문금액 + 당일 매수 주문금액 - 당일 차감 매수 금액 이면 오류 */
            if ((Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getmoney + 
				 (order_quantity * od_price) + 
				 Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_tot_sugum -
				 Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum) 
				>
                Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money)
            { 
                SLog (USR_ERROR, "19-03.Fund Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 설정금액[%lf] mk_gbn[%d] 장부합[%ld] 매수주문금액[%ld] 차감매수금액[%ld]",
                        dat->order_identification, order_quantity, order_quantity * od_price,
						Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money, mk_gbn,
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getmoney,
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_tot_sugum,
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_cha_sugum);
                return -1903;
            }
        }
#elif     defined A2101||A2102
		int		su_jango, do_jango;
		double	su_jango_gum, do_jango_gum;
		if (mm_flag == 2)				// 2:매수
		{
			/* 매수잔고금액 */
			if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt > 0)
			{
				su_jango = Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt;
				su_jango_gum = (double)su_jango * Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price;
			}
			else
				su_jango = su_jango_gum = 0;
		}
		else							// 1:매도
		{
			/* 매도잔고금액 */
			if (Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt < 0)
			{
				do_jango = abs(Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_getcnt);
				do_jango_gum = (double)do_jango * Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_get_avg_price;
			}
			else
				do_jango = do_jango_gum = 0;
		}

		/* 누적은 체크유무와 상관없이 무조건 체크한다. */
		if (mk_gbn == 3 && (memcmp (dat->MembershipItem+45, "1", 1) != 0))		// 주식선물 && !스프레드
		{
			if (mm_flag == 2)				// 2:매수
			{
				if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
					su_jango_gum +																		// 매수잔고금액
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum +					// 매수미체결금액
					(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
				{
					SLog (USR_ERROR, "19-10.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고금액[%f] 미체결금액[%f]",
                        dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
						su_jango_gum, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum);
					return -1910;
				}
			}
			else							// 1:매도
			{
				if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
					do_jango_gum +																		// 매도잔고금액
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum +					// 매도미체결금액
					(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
				{
					SLog (USR_ERROR, "19-11.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고금액[%f] 미체결금액[%f]",
                        dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
						do_jango_gum, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum);
					return -1911;
				}
			}
		}
		else if (mk_gbn == 4 && (memcmp (dat->MembershipItem+45, "1", 1) != 0))		// 주식옵션 && !스프레드
		{
			if (mm_flag == 2)				// 2:매수
			{
				if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
					(su_jango * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier *	Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_striking_price) +// 매수잔고수량*거래승수*행사가
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum +					// 매수미체결금액
					(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
				{
					SLog (USR_ERROR, "19-12.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
                        dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
						su_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum);
					return -1912;
				}
			}
			else							// 1:매도
			{
				if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
					(do_jango * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier *	Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_striking_price) +// 매도잔고수량*거래승수*행사가
					Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum +					// 매도미체결금액
					(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
				{
					SLog (USR_ERROR, "19-13.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
                        dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
						do_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum);
					return -1913;
				}
			}
		}
		else		// 이외처리
		{
			if (memcmp (dat->MembershipItem+45, "1", 1) == 0)		// 모든 스프레드
			{
				/* 근월물 잔고수량, 매도면 근월물매수체크(반대로) */
				if (mm_flag == 1)				// 1:매도
				{
					if (Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_getcnt > 0)
						su_jango = Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_getcnt;
					else
						su_jango = 0;

					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
						su_jango +																					// 매수잔고수량
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_su_michecnt +	// 매수미체결수량
						order_quantity)																				// 주문수량
					{
						SLog (USR_ERROR, "19-14.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+4, 5)].m_multiplier,
							su_jango, Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_su_michecnt);
						return -1914;
					}
				}
				else							// 2:매수
				{
					/* 매도잔고금액 */
					if (Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_getcnt < 0)
						do_jango = abs(Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_getcnt);
					else
						do_jango = 0;

					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
						do_jango +																					// 매도잔고수량
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_do_michecnt +	// 매도미체결수량
						order_quantity)																				// 주문수량
					{
						SLog (USR_ERROR, "19-15.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+4, 5)].m_multiplier,
							do_jango, Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4, 5)][acc_seq].item_do_michecnt);
						return -1915;
					}
				}

				/* 원월물 잔고수량, 주문반향과 동일 */
				if (mm_flag == 2)				// 2:매수
				{
					if (Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_getcnt > 0)
						su_jango = Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_getcnt;
					else
						su_jango = 0;

					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
						su_jango +																					// 매수잔고수량
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_su_michecnt +	// 매수미체결수량
						order_quantity)																				// 주문수량
					{
						SLog (USR_ERROR, "19-16.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+16, 5)].m_multiplier,
							su_jango, Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_su_michecnt);
						return -1916;
					}
				}
				else							// 1:매도
				{
					/* 매도잔고금액 */
					if (Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_getcnt < 0)
						do_jango = abs(Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_getcnt);
					else
						do_jango = 0;

					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
						do_jango +																					// 매도잔고수량
						Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_do_michecnt +	// 매도미체결수량
						order_quantity)																				// 매도주문수량
					{
						SLog (USR_ERROR, "19-17.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][AtoIf (dat->MembershipItem+16, 5)].m_multiplier,
							do_jango, Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_do_michecnt);
						return -1917;
					}
				}
			}
			else if (mk_gbn == 2 || mk_gbn == 12 || mk_gbn == 10)		// 2.지수옵션, 12:미니옵션, 10:코스닥150옵션
			{
				if (mm_flag == 2)				// 2:매수
				{
					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
						su_jango_gum +																		// 매수잔고금액
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum +					// 매수미체결금액
						(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
					{
						SLog (USR_ERROR, "19-18.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
							su_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum);
						return -1918;
					}
				}
				else							// 1:매도
				{
					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
						do_jango_gum +																		// 매도잔고금액
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum +					// 매도미체결금액
						(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier))	// 주문금액
					{
						SLog (USR_ERROR, "19-19.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
							do_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum);
						return -1919;
					}
				}
			}
			else
			{
				if (mm_flag == 2)				// 2:매수
				{
					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].su_money <
						su_jango +																			// 매수잔고수량
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt +					// 매수미체결수량
						order_quantity)																		// 주문수량
					{
						SLog (USR_ERROR, "19-18.Fund 매수 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
							su_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt);
						return -1918;
					}
				}
				else							// 1:매도
				{
					if (Shm_Risk[0].T_M_Fund[risk_mk_gbn][acc_seq].do_money <
						do_jango +																			// 매도잔고수량
						Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt +					// 매도미체결수량
						order_quantity)																		// 주문수량
					{
						SLog (USR_ERROR, "19-19.Fund 매도 Tot Order Money Over Error 주문번호[%10.10s] 수량[%lf] 주문금액[%lf] 잔고수량[%d] 미체결금액[%f]",
							dat->order_identification, order_quantity, order_quantity * od_price * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier,
							do_jango, Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt);
						return -1919;
					}
				}
			}
		}
#endif
	}	/* End of 리스크오류방지(18) */

#if     defined A1101||A1102			// 유가증권	
	/* 자전거래 체크(30) */
	if (memcmp (dat->MembershipItem+45, "1", 1) != 0	&&		// 스프레드(45)
		order_flag != 3)	// 취소
	{
		rt = 0;
    	rt = Cross_Chk (acc_seq, mk_gbn, item_seq, mm_flag, od_price);

		if (rt < 0)
		{
			SLog (USR_ERROR, "30-01.Corss_Chk Error mm[%d] item_seq[%d] od_price[%f] acc_seq[%d]",
					mm_flag, item_seq, od_price, acc_seq);
			return -3001;
		}
	}
#endif

    /* ************************** */
    /* 한도 관리를 위한 가감 로직 */
    /* ************************** */
	if (order_flag == 1)			// 신규
	{
/* 20220204 */
#if     defined A1101||A1102
		/* ******** */
		/* 금액누적 */
		if (mm_flag == 2)					// 매수
		{
			/* 매수주문금액 누적 */
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_tot_sugum += (order_quantity * od_price);
		}
		else								// 매도
		{
			Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt += (int)order_quantity;
		}
		/* ******** */
#elif     defined A2101||A2102
		/* ******** */
		/* 수량누적 */
		/* 계좌 수량 누적 */
		if (memcmp (dat->MembershipItem+45, "1", 1) == 0)		// 스프레드, 수량만
		{
			if (mm_flag == 2)				// 2:매수
			{
				/* 근월물(반대방향) */
				Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4,  5)][acc_seq].item_do_michecnt += order_quantity;
				/* 원월물(반대방향) */
				Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+16, 5)][acc_seq].item_su_michecnt += order_quantity;
			}
			else
			{
				/* 근월물(반대방향) */
				Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4,  5)][acc_seq].item_su_michecnt += order_quantity;
				/* 원월물(반대방향) */
				Shm_Risk[0].ProFit[mk_gbn][AtoIf (dat->MembershipItem+4,  5)][acc_seq].item_do_michecnt += order_quantity;
			}
		}
		else													// !스프레드, 미체결수량 && 미체결금액 모두
		{
			if (mm_flag == 2)				// 2:매수
			{
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_michecnt += order_quantity;
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_su_miche_gum +=
						(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);	// 주문금액
			}
			else
			{
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_michecnt += order_quantity;
				Shm_Risk[0].ProFit[mk_gbn][item_seq][acc_seq].item_do_miche_gum +=
						(od_price * order_quantity * Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_multiplier);	// 주문금액
			}
		}
		/* ******** */
#endif
	}   // End of New Order

    return 0;

}   /* End of Chk_Risk () */

/*************************************************************************
    Function        : . Set ksp,ksd spread band unit
					  . 주식선물의 스프레드만 처리한다.
    Parameters IN   : . market_gbn  :  0 . 코스피 호가단위
                                       1 . 코스닥 호가단위
                        basic_asset_price  : 기초자산전일종가
    Parameters OUT  : .
    Return Code     : .
*************************************************************************/
/*----------------------------------------------------------------------*/
void     Set_Band_Unit (int market_gbn, double basic_asset_price)
/*----------------------------------------------------------------------*/
{
    int  i, sp_gbn;
    i       = 0;
    sp_gbn  = 0;

    if (market_gbn == 5)                                // 5:코스피
    {
        sp_gbn = 13; // 13:주식선물스프레드(코스피)
    }
    else if (market_gbn == 6)                           // 6:코스닥
    {
        sp_gbn = 14; // 14:주식선물스프레드(코스닥)
    }
    else
    {
        SLog (USR_ERROR, "Set_Band_Unit market_gbn error");
        return;
    }

    for (i = 0; i < Shm_Risk[0].Ho_Chk[market_gbn][0].hoga_depth; i++)
    {
        if (basic_asset_price >= Shm_Risk[0].Ho_Chk[market_gbn][i].band_price)
        {
            Shm_Risk[0].Ho_Chk[sp_gbn][0].band_price = Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit;
            Shm_Risk[0].Ho_Chk[sp_gbn][0].band_unit  = Shm_Risk[0].Ho_Chk[market_gbn][i].band_unit;
            break;
        }
    }

    return;
}

/*************************************************************************
    Function        : . Cross_Chk, Only ETN
    Parameters IN   : . 1. acc_seq
                      . 2. mk_gbn
                      . 3. item_seq
                      . 4. mm_flag
                      . 5. od_price
    Parameters OUT  : .  0 : OK
                        -1 : ERROR
    Return Code     : . int (0:success, 1:timeout, -1:failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Cross_Chk (int acc_seq, int mk_gbn, int item_seq, int mm_flag, double od_price)
/*----------------------------------------------------------------------*/
{
    int     i, chk_cnt;

    chk_cnt = 0;
    for (i = 0; i < MAX_MICHE; i++)
    {
        if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt > 0)
        {
            chk_cnt++;
            /* 미체결 내역중 시장구분/종목seq 가 같은것 찾기 */
            if (item_seq == Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq)
            {
                /* 매매구분이 반대인것 */
                if (mm_flag != AtoIf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag, 1))
                {
                    if (mm_flag == 1)    /* 매도주문이고 && 주문가격보다 미체결매수가격이 같거나 높으면 오류 */
                    {
                        if (od_price <= AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9))
                            return -1;
                    }
                    else                /* 매주문이고 && 주문가격보다 미체결매수가격이 같거나 높으면 오류 */
                    {
                        if (od_price >= AtoDf(Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price, 9))
                            return -1;
                    }
				}
            }

            if (chk_cnt >= Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq])
                break;
        }
    }

    return 0;
}   /* End of Cross_Chk ()  */

/*************************************************************************
	End of Program (pa_1100_ts.c)
*************************************************************************/
