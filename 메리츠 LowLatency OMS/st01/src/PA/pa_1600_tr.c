#define		_GLOBAL
/*------------------------------------------------------------------------
#			Module : 장운영정보 수신 (TCP client), async
#			File : pa_1600_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	35

#ifdef	SAM_USE
#define		WR_CNT		OFW_CNT(0,0)
#define		WR_CNT2		OFW_CNT(1,0)
#define		OUT_NAME	OFN(D_K,P_K,0)
#define		OUT_NAME2	OFN(D_K,P_K,1)
#else
#define		WR_CNT		ODW_CNT(0,0)
#define		WR_CNT2		ODW_CNT(1,0)
#define		OUT_NAME	ODN(D_K,P_K,0)
#define		OUT_NAME2	ODN(D_K,P_K,1)
#endif

#if	defined A1601
#define	BIZ_TYPE "J681"
#elif	defined A2601
#define	BIZ_TYPE "J682"
#endif
/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd,	PortNo, DataCnt, PktType, DataSize, MaxCnt, RetryCnt;
char	ApType[10],	IpAddr[20], ErrCd[8];
char	RecvPkt[TCP_BUFF_MAX_LEN],	SendPkt[TCP_BUFF_MAX_LEN];

TCP_MESSAGE		*R_Pkt	= (TCP_MESSAGE *)RecvPkt;
TCP_HEAD		*S_Pkt	= (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1600_TR	(void);
void	Init_Parameters	(void);
void	Connection	(void);
int		Receive_Packet	(void);
void	Send_Packet	(void);
int		Check_Header	(void);
int		Write_Data	(void);
void	Register_Signal	(void);
void	Catch_Signal	(int);
void	Set_Socket_Linger	(void);

/*----------------------------------------------------------------------*/
int		main	(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc	(argc, argv);

	PA_1600_TR ();

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main () */

/*----------------------------------------------------------------------*/
void	PA_1600_TR (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	Register_Signal ();
	Init_Parameters ();

#ifdef	HOLIDAY_CHECK
	while (1)
	{
		char	 t_time[12], dt[20];
		time_t	 t = time(NULL);
		struct	tm tm, *tp;

		Get_Time	(t_time);

		memset	(dt, 0, sizeof (dt));
		sprintf	(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s", DAEMON(D_K).date,
			DAEMON(D_K).date+4, DAEMON(D_K).date+6, t_time, t_time+2, t_time+4);
		strptime	(dt, "%Y-%m-%d %H:%M:%s", &tm);
		//t	= mktime (&tm);
		tp	= localtime (&t);

		//if	(tp->tm_wday == 0 || tp->tm_wday == 6) /* sun, sat */
		if	(tp->tm_wday == 4 || tp->tm_wday == 6) /* sun, sat */
		{
			Log (USR_OK, "it's weekend. sleeping...[%d]", tp->tm_wday);
			sleep (60);
			continue;
		}
		else
                break;
/*
			if (memcmp (Shm_Risk[0].business_day, DAEMON(D_K).date, 8) == 0)
                break;
            else
            {
                Log (USR_OK, "system Day와 business Day가 다르다...[%8.8s][%8.8s]",
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
		rt	= Receive_Packet ();

		if	(rt == FAIL)
			continue;
		else	if (rt == NOTOK)
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
		rt	= Receive_Packet ();

		if	(rt == FAIL)
			continue;
		else	if (rt == NOTOK)
			return;
		else
			break;
	}

	while (START_S < JOB_END)
	{
		Stat_Save	();

		rt	= Receive_Packet ();

		if	(rt == NOTOK)
			break;
		else	if (rt == FAIL)
			continue;

/*
		PktType	= T_LINK;
		Send_Packet	();
*/
	}

	return;
}	/* End of PA_1600_TR () */

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . initiate the global variables
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
		TIME_OUT	= TCP_TIME_OUT;

Log	(USR_OK, "TIME_OUT[%d]", TIME_OUT);
	sprintf (ApType, "%-2.2s%-4.4s%c%c",
		_Exe_Name,	_Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
	LtoU (ApType, strlen (ApType));

#ifdef	SAM_USE
	DataSize = OFS(D_K,P_K,0);
#else
	DataSize = ODS(D_K,P_K,0);
#endif

	MaxCnt = TCP_DATA_LEN / (HEAD_SIZE + DataSize);
	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
		TCP2_IP2(D_K,P_K,S_K),	TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
	PortNo = TCP2_PORT_NO;

	return;
}	/* End of Init_Parameters () */

/*************************************************************************
	Function		: . Connection
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . connect to server
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Connection (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	while (START_S != END)
	{
		Sockfd	= Socket ();

		if	(Sockfd < 0)
		{
			TCP2_LINE_ST = OFF;
			Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
				Sockfd, SYS_NO, SYS_STR);
			sleep (5);
			continue;
		}

		TCP2_LINE_ST	= ON;
		Log	(USR_OK, "socket created:Sockfd[%d]", Sockfd);
		Log	(USR_OK, "connecting to %s:%d", IpAddr, PortNo);

		rt	= Connect (Sockfd, IpAddr, PortNo);

		if	(rt < 0)
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

		if	(PktType == T_STRT)
			TCP2_CON_STA = ON;

		TCP2_NET_STA(S_K)	= ON;
		Log	(USR_OK, "connected to %s:%d", IpAddr, PortNo);
		break;
	}

	Set_Socket_Linger ();

	return;
}	/* End of Connection () */

/*************************************************************************
	Function		: . Receive_Packet
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . int (0:success, -1:failure, 1:interrupted)
	Comment		 : . receive Receive_Packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet	(void)
/*----------------------------------------------------------------------*/
{
	int		rt, recv_len;
	char error_flag;

	error_flag = OFF;
	memset (RecvPkt, 0, sizeof (RecvPkt));

	rt = Select_Receive (Sockfd, RecvPkt);

	if (rt == OK)
		return	(NOTOK);
	else if (rt == NOTOK)
	{
		if	(SYS_NO == EINTR)
			return (FAIL);

		return	(NOTOK);
	}

	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt ||
		recv_len	> TCP_MESSAGE_MAX_LEN)
	{
		Log	(USR_ERROR, "invalid length[%d,%d]", recv_len, rt);
		PktType	= T_LNTH;
		DataCnt	= 0;
		sleep	(3);
		return	(NOTOK);
	}

	rt = Check_Header ();

	if (rt != OK)
		return	(NOTOK);

	if (memcmp (R_Pkt->Head.DataCnt, "91", 2) == 0)		// T_LIOK(91)
	{
		return	(OK);
	}
	if (memcmp (R_Pkt->Head.DataCnt, "92", 2) == 0)		// T_RSOK(92,개시응답)
	{
		PktType	= T_RSND;          // Recovery 요구(93)
		Send_Packet	();
	}
	else if (memcmp (R_Pkt->Head.DataCnt, "98", 2) == 0)		// POLL요청
	{
		PktType	= T_POOK;
		Send_Packet	();
	}
	else
	if ((memcmp (R_Pkt->Head.DataCnt, "01", 2) >= 0) &&		 // T_DATA(01~08)
		(memcmp	(R_Pkt->Head.DataCnt, "08", 2) <= 0))
	{
		rt	= Write_Data ();
		if	(rt != OK)
		{
			Log	(USR_ERROR, "Write_Data Error");
			sleep (3);
			return (NOTOK);
		}
	}
	else
	{
		Log	(USR_ERROR, "Else Case Data R_Pkt->Head.DataCnt[%2.2s]", R_Pkt->Head.DataCnt);
		return	(NOTOK);
	}

	return (OK);
}	/* Receive_Packet () */

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char t_time[12];

	memset (SendPkt, 0, sizeof (SendPkt));
	memset (SendPkt, 0x20, TCP_HEAD_LEN);

	/* 1. STX */
	S_Pkt->Stx[0] = STX;
	/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
	/* 3. 업무식별코드 */
#if	defined A1601
	memcpy (S_Pkt->ApType, "J681", sizeof (S_Pkt->ApType));
#elif	defined A2601
	memcpy (S_Pkt->ApType, "J682", sizeof (S_Pkt->ApType));
#endif
	/* 4. 송수신 구분 S:send, R:Recv */
	memcpy (S_Pkt->SR_gbn, "R",    sizeof (S_Pkt->SR_gbn));
	/* 5. 처리일자 */
	memcpy (S_Pkt->Date, DATE_CURR,sizeof (S_Pkt->Date));
	/* 6. 처리시각 (HHMMSS) */
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (S_Pkt->Time, t_time,   sizeof (S_Pkt->Time));
	/* 7. 응답코드 '000'이외 모두 에러 */
	ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
	/* 8. '000' Queue입력구분 */
	ItoAf (0, S_Pkt->Q_gbn,        sizeof (S_Pkt->Q_gbn));
	/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
	/* 10. Data건수
		01~08:DATA전문 (기본:01)
		91:운영전문,
		92:개시전문,
		93:일련번호요청전문,
		98:POLL전문*/
	/* 11. 최종일련번호, 전문일련번호와 동일 */
	/* 12. 서버구분, '1'(정상주문)  */
	memcpy (S_Pkt->Server_gbn, "1",sizeof (S_Pkt->Server_gbn));
	/* 13. 지점번호, SPACE(FEP담당자가 SPACE하라고함)   */
	memcpy (S_Pkt->Branch, "   ",  sizeof (S_Pkt->Branch));
	/* 14. 물리적지점번호 (8706)    */
	memcpy (S_Pkt->Sv_No,  "8706", sizeof (S_Pkt->Sv_No));
	/* 15. HTS단말ID, 단말ID */
	memcpy (S_Pkt->Hts_Id, "        ", sizeof (S_Pkt->Hts_Id));
	/* 16. 사번 or 로그인 ID, 사용자ID */
	memcpy (S_Pkt->User_Id, "        ",sizeof (S_Pkt->User_Id));
	/* 17. Filler */
	memcpy (S_Pkt->Filler, "        ", sizeof (S_Pkt->Filler));


	switch (PktType)
	{
		case	T_LINK:
			/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
			ItoAf (TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
			/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
			ItoAf (0, S_Pkt->SeqNo,  sizeof (S_Pkt->SeqNo));
			/* 10. Data건수 */
			memcpy (S_Pkt->DataCnt, "91",   sizeof (S_Pkt->DataCnt));
			/* 11. 최종일련번호, 전문일련번호와 동일 */
			ItoAf (0, S_Pkt->Last_SeqNo, sizeof (S_Pkt->Last_SeqNo));
			break;
		case	T_STRT:
			/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
			ItoAf (TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
			/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
			ItoAf (INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
			/* 10. Data건수 */
			memcpy (S_Pkt->DataCnt, "92",   sizeof (S_Pkt->DataCnt));
			/* 11. 최종일련번호, 전문일련번호와 동일 */
			ItoAf (INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
			break;
		case	T_RSND:
			/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
			ItoAf (TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
			/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
			ItoAf (INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
			/* 10. Data건수 */
			memcpy (S_Pkt->DataCnt, "93",   sizeof (S_Pkt->DataCnt));
			/* 11. 최종일련번호, 전문일련번호와 동일 */
			ItoAf (INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
			break;
		case	T_POOK:
			/* 2. 전문길이 5 byte 제외, 각 전문에서 Set */
			ItoAf (TCP_HEAD_LEN-5, S_Pkt->Length, sizeof (S_Pkt->Length));
			/* 9. 전문일련번호; TCP port (접속시는 PORT_NO) */
			ItoAf (INT_SEQ, S_Pkt->SeqNo,   sizeof (S_Pkt->SeqNo));
			/* 10. Data건수 */
			memcpy (S_Pkt->DataCnt, "98",   sizeof (S_Pkt->DataCnt));
			/* 11. 최종일련번호, 전문일련번호와 동일 */
			ItoAf (INT_SEQ, S_Pkt->Last_SeqNo,  sizeof (S_Pkt->Last_SeqNo));
			break;
		default:
			break;
	}

	rt = Select_Send (Sockfd, SendPkt, TCP_HEAD_LEN);

	if (rt != OK)
	{
		TCP2_CON_STA	= OFF;
		close	(Sockfd);
		Exit_Process	();
	}

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	return;
}	/* Send_Packet () */

/*************************************************************************
	Function		: . Check_Header
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . int (0:success, -1:failure)
	Comment		 : . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Header	(void)
/*----------------------------------------------------------------------*/
{
	int		val, i, len, data_cnt, data_len;
	char *data = (char *)R_Pkt->Data;

	memset (ErrCd, '0', sizeof (ErrCd));

	/* Stx (0x02) */
	if (R_Pkt->Head.Stx[0] != STX)
	{
		Log	(USR_ERROR, "TH.Stx [%#04x:%#04x]", R_Pkt->Head.Stx[0], STX);
		memcpy	(ErrCd, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return	(NOTOK);
	}

	/* Length (packet 총 길이) */
	val = AtoIf (R_Pkt->Head.Length, sizeof (R_Pkt->Head.Length));

	if (val != strlen (RecvPkt)-5 ||
		strlen	(RecvPkt) < TCP_HEAD_LEN)
	{
		Log	(USR_ERROR, "TH.Length [%d:%d]", val, strlen (RecvPkt));
		memcpy	(ErrCd, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return	(NOTOK);
	}

	data_len = val - TCP_HEAD_LEN - 5;

	/* ResponseCode (응답코드) */
	if (AtoIf (R_Pkt->Head.ResponseCode,
		sizeof	(R_Pkt->Head.ResponseCode)) != 0)
	{
		Log	(USR_ERROR, "TH.ResponseCode [%.4s:%04d]",
			R_Pkt->Head.ResponseCode, 0);
		memcpy	(ErrCd, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return	(NOTOK);
	}

#if	0
//	메리츠는 운영코드를 사용하지 않음
	/* MsgType (운영코드) */
	if ((memcmp (R_Pkt->Head.MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0 &&
		memcmp	(R_Pkt->Head.MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0 &&
		memcmp	(R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) != 0 &&
		memcmp	(R_Pkt->Head.MsgType, TCP_RSOK_CD, strlen (TCP_RSOK_CD)) != 0 &&
		memcmp	(R_Pkt->Head.MsgType, TCP_POLL_CD, strlen (TCP_POLL_CD)) != 0) ||
		(PktType	== T_LINK &&
		memcmp	(R_Pkt->Head.MsgType, TCP_LIOK_CD, strlen (TCP_LIOK_CD)) != 0) ||
		(PktType	== T_STRT &&
		memcmp	(R_Pkt->Head.MsgType, TCP_STOK_CD, strlen (TCP_STOK_CD)) != 0))
	{
		Log	(USR_ERROR, "TH.MsgType [%.4s]", R_Pkt->Head.MsgType);
		memcpy	(ErrCd, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return	(NOTOK);
	}
#endif

	/* 접속/재접속 정상수신 체크 */
	if ((PktType == T_LINK && memcmp (R_Pkt->Head.DataCnt, "91", 2) != 0) ||
		(PktType	== T_STRT && memcmp (R_Pkt->Head.DataCnt, "92", 2) != 0) )
	{
		Log	(USR_ERROR, "TH.Rcv Data PktType[%d] [%s]", PktType, R_Pkt);
	}

	return (OK);
}	/* End of Check_Header () */

/*************************************************************************
	Function		: . Write_Data
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . int (0:success, -1:failure)
	Comment		 : . write received data to file
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Write_Data	(void)
/*----------------------------------------------------------------------*/
{
	int		  i, next, seq, d_cnt, rt, len, f_size, rec_size;
	int		  f_gbn;
	char		 m_time[24], w_buf[FILE_BUF_LEN];
	char		 *data = (char *)R_Pkt->Data;
	BUFF_RW_HEAD f_head;

	next = 0;
/*
	if  (memcmp (data+11+next, "TTRMIP31301", 11) != 0)
		return	(-1);
*/

	d_cnt = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));
	f_size = sizeof (BUFF_RW_HEAD);
	rec_size = f_size + DataSize + 1;
	len = strlen(data);

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	memset (w_buf, ' ', sizeof (w_buf));

	/* BUFF_RW_HEAD 70 byte */
    ItoAf (INT_SEQ + i + 1, f_head.If_Seq, sizeof (f_head.If_Seq));
    //memcpy (f_head.ApType, ApType, strlen (ApType));  SPARROW
    memcpy (f_head.ApType, ApType, sizeof (f_head.ApType));
    ItoAf (0, f_head.ResponseCode, sizeof (f_head.ResponseCode));
    memcpy (f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
    memcpy (f_head.RecvTime2, &m_time[sizeof(f_head.RecvTime1)],
        sizeof (f_head.RecvTime2));
    ItoAf (DataSize, f_head.DataHeader, 20);

    memcpy (&w_buf[0], &f_head, f_size);
    memcpy (&w_buf[f_size], data, len);
    if (DataSize > len)
        memset (&w_buf[f_size+(len)], 0x20, DataSize - (len));
    w_buf[f_size+DataSize] = '\n';


	SYS_NO = 0;

#ifdef	SAM_USE
	rt	= F_W (TS_W1_1, w_buf, d_cnt);
#else
	rt	= DSHM_W (TS_W1_1, w_buf, d_cnt);
#endif

	if (rt != d_cnt)
	{
#ifdef	SAM_USE
	Log	(SAM_FATAL, "file write[%s]", OUT_NAME);
#else
	Log	(DSH_FATAL, "DSHM write[%s]", OUT_NAME);
#endif
		memcpy	(S_Pkt->ResponseCode, ERR_FILE_WRITE, strlen (ERR_FILE_WRITE));
		return	(NOTOK);
	}

	INT_SEQ += d_cnt;
	Log	(USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	Set_TR_Time ();

	return (OK);
}	/* End of Write_Data () */

/*************************************************************************
	Function		: . Register_Signal
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . register signal
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
		Log	(SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log	(SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Signal () */

/*************************************************************************
	Function		: . Catch_Signal
	Parameters IN : . signo : signal number
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . catch signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred", signo);

#if 0
	if (signo == SIGTERM)
	{
		PktType	= T_STOP;
		Send_Packet	();
	}
#endif

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of Catch_Signal () */

/*************************************************************************
	Function		: . Set_Socket_Linger
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . void
	Comment		 : . set linger option on socket
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Socket_Linger (void)
/*----------------------------------------------------------------------*/
{
	int		  rt;
	struct linger ling;

			 /* close () returns after discarding any unsent data */
	ling.l_onoff = 1;
	ling.l_linger = 0;

	rt = setsockopt (Sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling,
		sizeof	(ling));

	if (rt < 0)
		Log	(TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}

/*************************************************************************
	End of Program (pa_1600_tr.c)
*************************************************************************/
