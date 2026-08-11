#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module : 주문응답/체결 수신 (TCP client), async
#            금융파생(IMECO) 회원처리호가/체결 수신
#	File : pa_2200_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	35

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd,	PortNo, DataCnt, PktType, DataSize, MaxCnt, RetryCnt, W_Flag;
char	ApType[10],	IpAddr[20], ErrCd[8];
char	RecvPkt[TCP_BUFF_MAX_LEN],	SendPkt[TCP_BUFF_MAX_LEN];
char	TrCode[5];

FILE_DATA_HEAD		File_Data_Head;     // 20

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_2200_TR			(void);
void	Init_Parameters		(void);
void	Connection			(void);
int		Receive_Packet		(void);
void	Send_Packet			(void);
int		Write_Data			(void);
void	Register_Signal		(void);
void	Catch_Signal		(int);
void	Set_Socket_Linger	(void);

/*----------------------------------------------------------------------*/
int		main	(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc	(argc, argv);

	memset(TrCode, 0x00, sizeof(TrCode));
	if (argc == 2)
	{
		if (memcmp(argv[1], "A0", 2) == 0)
			memcpy(TrCode, "REQJM", 5);
	}

	PA_2200_TR ();

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main () */

/*----------------------------------------------------------------------*/
void	PA_2200_TR (void)
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
		strptime	(dt, "%Y-%m-%d %H:%M:%S", &tm);
		//t	= mktime (&tm);
		tp	= localtime (&t);

		if	(tp->tm_wday == 0 || tp->tm_wday == 6) /* sun, sat */
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

sleep (5);
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

	while (START_S < JOB_END)
	{
		Stat_Save	();

		rt	= Receive_Packet ();

		if	(rt == NOTOK)			// NOTOK:-1
			break;
		else	if (rt == FAIL)		// FAIL:1
			continue;
	}

	return;
}	/* End of PA_2200_TR () */

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

	//MaxCnt = TCP_DATA_LEN / (HEAD_SIZE + DataSize);
	MaxCnt = 1;

	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
		TCP2_IP2(D_K,P_K,S_K),	TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
	PortNo = TCP2_PORT_NO;

	Log (USR_OK, "IpAddr[%s] PortNo[%d]", IpAddr, PortNo);

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
			TCP2_CON_STA = OFF;
			TCP2_LINE_ST = OFF;
			Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}", Sockfd, SYS_NO, SYS_STR);
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
			sleep (5);

			Exit_Process ();
		}

		TCP2_CON_STA		= ON;
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
	int		rt;
	char	error_flag;

	error_flag = OFF;
	memset (RecvPkt, 0, sizeof (RecvPkt));

	rt = Select_Receive_Imeco_Sise (Sockfd, RecvPkt);

	if (rt == OK)
		return	(NOTOK);
	else if (rt == NOTOK)
	{
		if	(SYS_NO == EINTR)
			return (FAIL);

		return	(NOTOK);
	}

	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	/* ***
	   수신은 3개 가능, 송신은 3개 가능 
	   수신 : LOGOK/LOGER/RESHB/시세TR(A0/A3/G7...)
	   송신 : LOGIN/REQHB/REQRL
		오류코드는 체크할 필요 없음
	*** */
	W_Flag = 0;
	if (memcmp (&RecvPkt[10],	"LOGOK",	5) == 0)				// LOG ON, OK
	{
		if (TrCode[0] == 0x00)
			PktType = T_DATA;		// REQRL 요청
		else
			PktType = T_RSND;		// 재전송 요청
        Send_Packet ();
	}
	else if (memcmp (&RecvPkt[10],	"LOGER",	5) == 0)			// LOG ON, ERROR
	{
		Log (USR_ERROR, "Check Plz!!  Recv LOGIN ERROR RecvPkt[%s][%d]", RecvPkt, strlen(RecvPkt));
		return (NOTOK);
	}
	else if (memcmp (&RecvPkt[10],	"REQHB",	5) == 0)			// HeartBeat, OK
	{
		PktType = T_POOK;
        Send_Packet ();
Log (USR_OK, "Recv LIVE REQHB");
	}
	else if ((memcmp (&RecvPkt[10],	"A0",	2) == 0)	)			// DATA
	{
		W_Flag = 1;		// 7201dd
	}
	else if	((memcmp (&RecvPkt[10], "A3",   2) == 0)	||		// A3:173
			 (memcmp (&RecvPkt[10], "G7",   2) == 0)	||		// G7:431
			 (memcmp (&RecvPkt[10], "B6",   2) == 0)	)		// B6:324
	{
		W_Flag = 2;		// 7202dd
	}
	else if	((memcmp (&RecvPkt[10], "H3",   2) == 0)	||			// 정산가					,미사용
			 (memcmp (&RecvPkt[10], "A6",   2) == 0)	||			// 종목마감					,미사용
			 (memcmp (&RecvPkt[10], "A7",   2) == 0)	||			// 장운영TS(종목장운영정보) ,미사용
			 (memcmp (&RecvPkt[10], "R1",   2) == 0)	)			// 장운영TS+호가			,미사용
	{
		W_Flag = 3;
	}
	else
	{
		Log	(USR_OK, "Else Case Receive RecvPkt[%5.5s]", &RecvPkt[10]);
		return	(OK);
	}

	if (W_Flag > 0)
	{
		rt  = Write_Data ();
		if  (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error");
			sleep (3);
			return (NOTOK);
		}
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
	char	d_time[18];

	memset (SendPkt, 0, sizeof (SendPkt));

    memset (d_time, 0, sizeof (d_time));
    Get_DateMilliTime (d_time);		// yyyymmddhhmmssmmm

	memset (SendPkt, 0,	sizeof(SendPkt));

	/* 1. BodyLength, Header사이즈를 제외한 Body부 사이즈만 세팅 */
	/* 2. TR Code */
	if ( PktType == T_LINK )                                    // LogOn
	{
		memcpy (&SendPkt[0],	"0000000080",	10);		
        memcpy (&SendPkt[10],	"LOGIN",		 5);
	}
    else if ( PktType == T_POOK ) 
	{
		memcpy (&SendPkt[0],	"0000000040",	10);			// No Body
        //memcpy (&SendPkt[10],	"REQHB",		 5);			// Heartbeat
        memcpy (&SendPkt[10],	"RESHB",		 5);			// Heartbeat 응답
Log (USR_OK, "OK 01");
	}
    else if ( PktType == T_DATA ) 
	{
		memcpy (&SendPkt[0],	"0000000040",	10);			// No Body
        memcpy (&SendPkt[10],	"REQRL",		 5);			// Data 요청
	}
    else if ( PktType == T_RSND ) 
	{
		memcpy (&SendPkt[0],	"0000000040",	10);			// No Body
        memcpy (&SendPkt[10],	TrCode,			 5);			// 재전송 요청
	}

	/* 3. Seq, Seq는 0으로 */
	memcpy (&SendPkt[15],		"0000000000",	10);			// 항상 0
	/* 4. 에러코드 */
	memcpy (&SendPkt[25],		"00000",		 5);			// 항상 0
	/* 5. 송신시간 */
	memcpy (&SendPkt[30],		d_time,			17);			// 17자리만 갖고옴(ms)
	memcpy (&SendPkt[30+17],	"   ",			 3);			// 나머지는 Space

	/* Data 부 */
	if ( PktType == T_LINK )                                    // LogOn
	{
		memcpy (&SendPkt[50],	"KBFG2               ",	20);			// ID:KBFG2
		memcpy (&SendPkt[70],	"KBFG2               ",	20);			// PW:KBFG2
	}

	rt = Select_Send (Sockfd, SendPkt, strlen (SendPkt));
	if (rt != OK)
	{
		TCP2_CON_STA	= OFF;
		close	(Sockfd);
		Log (TCP_ERROR, "TCP SD ERROR[%s](%d)<%d> {%d:%s}", SendPkt, strlen (SendPkt), INT_SEQ, SYS_NO, SYS_STR);
		sleep (5);
		Exit_Process ();
	}

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	return;
}	/* Send_Packet () */

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
	int			i, next, seq, rt;
	char		m_time[24];
	char		media_gbn[2];
	char		w_data[2048];
	BUFF_RW_HEAD f_head;

	/* ************************************ */
    /* 1201 : 82 + 4+11 + 회원처리호가전문  */
    /* 1401 : 82 + 체결전문                 */
    /* ************************************ */
    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);
    memset (w_data,	0x20,			sizeof (w_data));
    memset (&File_Data_Head, ' ',	sizeof (HEAD_SIZE));

	/* Header 50 byte */
    ItoAf (INT_SEQ + 1,		&w_data[0],		8);						// Seq(8)
    ItoAf (INT_SEQ + 1,		&w_data[8],		8);						// IF_Seq(8)
    memcpy (&w_data[16],	ApType,			8);						// ApType(8)
    memcpy (&w_data[24],	RES_NORMAL, 	strlen (RES_NORMAL));	// "0000"(4)
    memcpy (&w_data[28],	m_time,			10);					// RecvTime1(10)
    memcpy (&w_data[38],	&m_time[10],	12);					// RecvTime2(12)

	/* Heaer 20 byte */
	ItoAf (OFS(D_K,P_K,W_Flag-1),	File_Data_Head.Length,	sizeof (File_Data_Head.Length));	// 4
    ItoAf (INT_SEQ + 1,				File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));	// 8
    memcpy (File_Data_Head.ResponseCode,	RES_NORMAL,	strlen (RES_NORMAL));					// 4
    memcpy (File_Data_Head.LineFlag,		_Exe_Name+4,	3);									// 3

    memcpy (&w_data[50],	&File_Data_Head,	sizeof (HEAD_SIZE));

	/* Data */
	memcpy (&w_data[70],	&RecvPkt[50],		AtoIf(&RecvPkt[5], 5) - 40);		// DATA SIZE
    w_data[70+OFS(D_K,P_K,W_Flag-1)] = '\n';
	
	SYS_NO = 0;

	if (W_Flag == 1)
		rt	= F_W (TS_W1_1, (void *)w_data, 1);
	else if (W_Flag == 2)
		rt	= F_W (TS_W2_1, (void *)w_data, 1);
	else if (W_Flag == 3)
		rt	= F_W (TS_W3_1, (void *)w_data, 1);

	if (rt != 1)
	{
		Log	(SAM_FATAL, "file write[%s]", OFN(D_K,P_K,W_Flag-1));
		return	(NOTOK);
	}

	INT_SEQ += 1;
	Log	(USR_OK, "data write[%s:%d:%d]", OFN(D_K,P_K,W_Flag-1), OFW_CNT(0,W_Flag-1), 1);
	Log (USR_OK, "Write ok[%s][%d]", w_data, strlen(w_data));
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
	End of Program (pa_2200_tr.c)
*************************************************************************/
