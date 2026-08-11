#define		_GLOBAL
	
/*------------------------------------------------------------------------
#	System	: Connect to KRX
#	Author	: PSH 
#	Module	: 주문송신
#	File	: pa_1100_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"
/*
#include    "fep_fepj.h"
#include    "krx_struct.h"
*/

/* 주문 송신 Size 정의 */
/* 채권일반_254, 채권조성_255 */
#define		DATA_SIZE		400			  /* Header(81) + Data(319)_가변포함 */
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DEVICE_TIME		3 * 1000						/*  3 sec	*/
#define		MAIN_TIME		5 * 1000		/* Heart Beat 간격  5 sec	*/
#define		DATA_TIME		15 * 1000		/* Heart Beat 3회  15 sec	*/
#define		FOREVER_TIME	60 * 1000						/* 60 sec	*/

#define		FIFO_EVENT		0
#define		SOCKET_EVENT	1
#define		DATA_EVENT		2

#define		MAX_CNT			1
#define		RESP_GAP		1000

#define		NO_TIME			"0830"		// 채권정규시장은 09시 ~ 15시 까지, 시간외 없음

#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define     PORT_NO         TCP2_PORT_NO

#define     CHK_PROC		"pa_1401_tr"					/* 체결Process가 인터페이스종료(TCHEDP99000) 수신후 종료되면 주문송신도 종료 */
#if 0
2025 미사용
#define     CHK_PROC_01		"pa_1601_tr"					/* 장운영 수신, 장중 */
#define     CHK_PROC_02		"pa_1701_tr"					/* RDS 	  수신, 18~21 */
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[4096], IpAddr[20];

DATA_FORMAT			*SHM_DATA;
FILE_BUFF_FORMAT	R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];
KRX_JUMUN_Q_FMT		J_Q_Fmt;			// KRX 주문포맷(max 6) (82+294)
KRX_JUMUN_R_FMT		J_R_Fmt;			// KRX 세션 응답 포맷 (82+4+11)
KRX_SESSION_FMT		S_Fmt;				// KRX 세션 포맷 (82+40)
KRX_JUMUN_S_FMT		Re_W_Fmt;			// 내부 주문응답 송신
struct pollfd		Poll[3];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1100_TS (void);
void	Init_Parameters (void);
void	Fifo_Event_Rtn (void);
void	Socket_Event_Rtn (void);
void	Data_Event_Rtn (void);
void	Device_Open (void);
void	Device_Close (void);
void	Device_Write (void);
int		Device_Read (void);
void	Time_Out_Rtn (void);
void	Jang_End_Time_Process (void);
int		Analyze_Data (void);
void	Jang_End_Time_File_Rtn (void);
void 	Write_Response_Data (int);
int		Make_Send_Msg (int);
int		Make_Data_Block (void);
void    Get_Msec (double *);
void	Log_Out (void);
void	Err_Msg (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1100_TS ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1100_TS (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;

	Init_Parameters ();

	while (START_S != END)
	{
		Stat_Save ();

		if (TCP2_NET_STA(S_K) != END && TCP2_NSTAT(D_K,Pk,S_K) == END)	
		{ 										   /* 장운영 정보 Check */
			Log_Out ();
		}

        if (TCP2_NET_STA(S_K) == END || TCP2_NET_STA(S_K) == JOB_STOP)
        {                                               /* 종료/중지    */
            if (LogOnFlag == ON && TCP2_NET_STA(S_K) == END)
			{
                Device_Close ();
			}

            Jang_End_Time_Process ();           /* 장종료 후 data 처리  */
            continue;
        }
        else                                            /* 정상주문시간 */
        {
/* -------------------------------------------------------------------- */
/* MAIN, BACKUP, S_K 사용예 									 		*/
/* MAIN, BACKUP은 Memory상에 1개 이상의 복수건 정의시(Socket, Netstat등)*/
/* 메모리의 번지수에 정의된 값이 상이함. 이에 Process가 복수건의 Socket */
/* 처리를 하는 Logic에서 사용.(Tcp2.ini 참조)						 	*/
/* S_K는 MAIN, BACKUP의 정의된 값중 현재 사용되는 Socket의 대상을 지정  */
/* - 주목적:															*/
/*  복수건의 Socket등을 사용시 사용 Socket을 편리하게 활용하기 위한 값  */
/* - 사용차이: 														  	*/
/*  MAIN, BACKUP: Memory에 Status 변경시 사용 (Netstat, Tcp2_stat등)	*/
/*  S_K: 참조하여 사용만 함											  	*/
/* -------------------------------------------------------------------- */
/* LogOnFlag: TCP Connect & LOGON check Flag                            */
/*            Init_Parameter => OFF.                                    */
/*            Device_Open    => ON.                                     */
/*            Device_Close   => OFF.                                    */
/* -------------------------------------------------------------------- */
/* OpenFlag: 업무개시 Flag, DATA 전송업무 가능 상태                     */
/*             Init_Parameter => OFF.                                   */
/*             Socket_Event_rtn => ON.                                  */
/*             Device_Close => OFF.                                     */
/* -------------------------------------------------------------------- */
/* DeviceSendFlag: DATA 전송 check Flag 							    */		
/*				   Init_Parameter => OFF.                               */
/*         		   Socket_Event_Rtn => OFF.                             */
/*                 Time_Out_Rtn => Device_Write()후 ON(TR_LINK||TR_TEST)*/
/*                 Data_Event_Rtn => ON.                                */
/*                 Make_Send_Msg (TR_DATA) => Device_Write()후 ON.      */
/* 2025 : Data_Event_Rtn & Make_Send_Msg 싱크시에만 유효, 어싱크는 유요하지 않음 */
/* -------------------------------------------------------------------- */

			if (OpenFlag == OFF)
			{
            	if (LogOnFlag == OFF)
                	Device_Open ();

          		if (LogOnFlag == OFF)
				{
                	PollCnt = 1;
                	TimeOut = DEVICE_TIME;
				}
				else
				{
					PollCnt = 2;
					TimeOut = FOREVER_TIME; 
				}
			}
			else
			{
               	if (DeviceSendFlag == ON)
               	{
                   	PollCnt = 2;
                   	TimeOut = DATA_TIME; 
               	}
               	else 
               	{
                   	PollCnt = 3;
                   	TimeOut = MAIN_TIME;

                   	if (WR_CNT > RD_CNT)
                   	{
                       	Data_Event_Rtn ();
                       	continue;
                   	}
               	}
        	}
		}

        rt = poll (Poll, PollCnt, TimeOut);

        if (rt < 0)
        {
            if (SYS_NO == EINTR)
                SLog (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                SLog (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

            continue;
        }
        else if (rt == 0)
        {
            Time_Out_Rtn ();
            continue;
        }

        for (i = 0; i < PollCnt; i ++)
        {
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
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    SOCKET_EVENT:
                Socket_Event_Rtn ();
                break;
            case    DATA_EVENT:
                Data_Event_Rtn ();
                break;
            default:
                SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    }

    Device_Close ();

	return;
}	/* End of PA_1100_TS ()	*/

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
    DeviceSendFlag = OFF;
    LogOnFlag = OFF;
    OpenFlag = OFF;
    ErrCd = 0;

    S_K = TCP2_LINE_GU;

	L_K = 0;

    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    SLog (TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    TCP2_PROC_ST = ON;
    TCP2_LINE_ST = OFF;

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

    if (PROC(D_K,P_K).data)
        SHM_DATA = (DATA_FORMAT *)PROC(D_K,P_K).data;

	/* 장운영정보 process key 구함 */
#if 0
2025 미사용
	for (Pk = 0; Pk < DAEMON(D_K).p_count; Pk ++)
	{
		if (memcmp (PROC(D_K,Pk).process_id, CHK_PROC, 10) == 0)
			break;

		if (Pk == DAEMON(D_K).p_count - 1)
		{
			Log (USR_FATAL, "unregistered process[%s]", CHK_PROC);
			Exit_Process ();
		}
	}
#endif

    if (DELAY_TIME == 0)
        DELAY_TIME = RESP_GAP;

	return;
}	/* End of Init_Parameters ()	*/

/*************************************************************************
	Function		: . Fifo_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . 업무 통제 FIFO SIGNAL GET & No Action
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
	Function		: . Socket_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . TCP Data Recv & Response Action
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Socket_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rval, rt, cnt;
	char	t_time[12];

	rval = Device_Read ();

	if (rval < 0)
		return;

	DeviceSendFlag = OFF;

	memset (&J_R_Fmt, 0, sizeof (KRX_JUMUN_R_FMT));
	memcpy (&J_R_Fmt, DataBuff, RecvLen);

	ReTrCode = Analyze_Data ();
	switch (ReTrCode)
	{
		case	TR_LINK:	/* 업무개시 요청 */
            TCP2_NET_STA(S_K) = ON;
			TCP2_LINE_ST = OpenFlag = ON;

            if (FirstSeq > INT_SEQ + LOAD_CNT) 
			{
				TCP2_LINE_ST = OpenFlag = OFF;

                SLog (USR_ERROR, "TR_LINK recv:invalid KRX SEQ <%d:%d:%d>",
                    FirstSeq, INT_SEQ, LOAD_CNT);

				rt = Make_Send_Msg (ER_LINK);
			}
			else if (FirstSeq < INT_SEQ)
			{
				SLog (USR_WARN, "TR_LINK recv:check KRX SEQ <%d:%d:%d>",
				FirstSeq, INT_SEQ, LOAD_CNT);

				RD_CNT = INT_SEQ = FirstSeq;
				PROC(D_K,P_K).counter_seq = FirstSeq;

				LOAD_CNT = 0;
				rt = Make_Send_Msg (RP_LINK);
			}
			else
            {
				Write_Response_Data (FirstSeq - INT_SEQ);
				memset ((char *)SHM_DATA, 0, SHM_DATA_SIZE);

				LOAD_CNT = 0;
				rt = Make_Send_Msg (RP_LINK);
			}
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &S_Fmt, SendLen);
			Device_Write ();

			break;
		case	RP_DATA:	/* 주문응답 */
			if (FirstSeq < INT_SEQ || FirstSeq > INT_SEQ + LOAD_CNT) 
			{
				SLog (USR_FATAL, "RP_DATA recv:invalid KRX SEQ <%d:%d:%d>",
					FirstSeq, INT_SEQ, LOAD_CNT);
				Exit_Process ();
			}

			Write_Response_Data (LOAD_CNT);
			LOAD_CNT = 0;
			memset ((char *)SHM_DATA, 0, SHM_DATA_SIZE);

			if (ErrCd != 0) Err_Msg();

			break;
		case	RP_TEST:
			break;
		default:
			break;
	}

	return;
}	/* End of Socket_Event_Rtn ()	*/

/*************************************************************************
	Function		: . Data_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . SHM Data Processing
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Data_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	tmp[128];

	ErrCd = 0;
	memset (DataBuff, 0, sizeof (DataBuff));

	rt = Make_Send_Msg (TR_DATA);
	if (rt == 0)
	{
		memcpy (DataBuff, &J_Q_Fmt, SendLen);
		Device_Write ();

		if (LogOnFlag == ON)
			Set_TR_Time ();

		DeviceSendFlag = ON;
	}

	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		if (rt == 0)
			break;
		else if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;
			SLog (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}",
				SYS_NO, SYS_STR);
			break;
		}
	}

	return;
}	/* End of Data_Event_Rtn ()	*/


/*************************************************************************
	Function		: .  Device_Open
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Svm Line Status Set & TCPIP Poll fd set & LOGON
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Open (void)
/*----------------------------------------------------------------------*/
{
	int     rt, rval;

	Sockfd = Socket ();

	if (Sockfd < 0)
	{
		SLog (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
			Sockfd, SYS_NO, SYS_STR);
		return;
	}

	SLog (USR_OK, "socket created:Sockfd[%d]", Sockfd);
	SLog (USR_OK, "connecting to %s:%d", IpAddr, PORT_NO);

	rt = Connect (Sockfd, IpAddr, PORT_NO);

	if (rt < 0)
	{
		SLog (TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
		close (Sockfd);
		return;
	}

	rt = Make_Send_Msg (TR_LOON);
	memset (DataBuff, 0, sizeof (DataBuff));
	memcpy (DataBuff, &S_Fmt, SendLen);
	Device_Write ();
	SLog (USR_OK, "send LOGON request");

	rval = Device_Read ();
	if (rval < 0)
   	{
        SLog (USR_ERROR, "LOGON response recv error");
        close (Sockfd);
        return;
   	}

	if (memcmp(DataBuff[80], "0000", 4) == 0)
	{
		SLog (USR_OK, "LOGON success");
	}
    else
	{
       ErrCd = AtoIf (DataBuff[80], 4);
       Err_Msg ();

       close (Sockfd);
       return;
	}

    LogOnFlag = ON;

    Poll[1].fd = Sockfd;
    Poll[1].events = POLLIN;
    SLog (TCP_OK, "TCP Connect & LOGON OK");

	return;
}	/* End of Device_Open ()	*/

/*************************************************************************
	Function		: .  Device_Close
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Svm Line Status Set
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Close (void)
/*----------------------------------------------------------------------*/
{
    close (Sockfd);
    SLog (TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

	return;
}	/* End of Device_Close ()	*/

/*************************************************************************
	Function		: .  Device_Write
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Tcpip Data Send
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Write (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

    rt = Select_Send (Sockfd, DataBuff, strlen (DataBuff));

    if (rt != OK)
    {
        SLog (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

    SLog (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
	Get_Msec (&SendMsec);

/* 현물과 다른부분 */
   	if (memcmp (J_Q_Fmt.Header.MsgType, "TCHODR00000", 11) == 0)	/* 주문 */
    {
        STime = AtoIf (R_Fmt[0].RecvTime2, 2) * 60 * 60 +
                AtoIf (R_Fmt[0].RecvTime2+2, 2) * 60 +
                AtoIf (R_Fmt[0].RecvTime2+4, 2) +
                AtoIf (R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

        Log (TCP_OK, "Inside Calculation Time [%.06f] [%5.5s]", 
			SendMsec - STime, R_Fmt[0].Data+185);		 /* 회원사 처리항목 */
    }


	return;
}	/* End of Device_Write ()	*/

/*************************************************************************
	Function		: .  Device_Read
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . Tcpip Data Recv
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Device_Read (void)
/*----------------------------------------------------------------------*/
{
    int     rt;
    char    m_time[24];

    memset (DataBuff, 0, sizeof (DataBuff));
	RecvLen = 0;

    rt = Select_Receive_Krx (Sockfd, DataBuff, sizeof(KRX_HEADER));

    if (rt <= 0)
    {
        Device_Close ();
        return (NOTOK);
    }

	RecvLen = rt;
   	SLog (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);

    if (DeviceSendFlag == ON && SendLen > sizeof (KRX_HEADER))
    {
		Get_Msec (&RecvMsec);
        RespMsec = RecvMsec - SendMsec;

        if (DELAY_TIME != 0 && RespMsec > DELAY_TIME / 1000.0)
            SLog (TCP_ERROR, "DELAYED [%.06f:%.06f]",
                RespMsec, DELAY_TIME / 1000.0);
        else
	    {
   			if (memcmp (DataBuff+14, "TCHODR00000", 11) == 0)	/* 메시지타입: 주문 */
            {
                RTime = AtoIf (DataBuff+68, 2) * 60 * 60 +		/* 전송일시 : 시 */
                    AtoIf (DataBuff+70, 2) * 60 +				/* 전송일시 : 분 */ 
					AtoIf (DataBuff+72, 2) +					/* 전송일시 : 초 */
                    AtoIf (DataBuff+74, 3) / 1000.0;			/* 전송일시 : MS */

                if (RTime > RecvMsec)
                    SLog (TCP_OK, "Data response [%3.3s][%.06f][+%.03f]",
                        DataBuff+77, RespMsec, RTime - RecvMsec);	/* 데이터건수 */
                else
                    SLog (TCP_OK, "Data response [%3.3s][%.06f][%.03f]",
                        DataBuff+77, RespMsec, RecvMsec - RTime);	/* 데이터건수 */
            }
            else
                SLog (TCP_OK, "response [%.06f]", RespMsec);
        }
    }

	return (OK);
}	/* End of Device_Read ()	*/

/*************************************************************************
	Function		: .  Time_Out_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Timeout Control
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	memset (DataBuff, 0, sizeof (DataBuff));
	
	switch (PollCnt)
	{
		case	2:
			if (OpenFlag == ON) 
			{
				SLog (TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
				Device_Close ();
			}
			else
			{
				SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
			}

			break;
		case	3:
			rt = Make_Send_Msg (TR_TEST);
			memcpy (DataBuff, &S_Fmt, SendLen);
			Device_Write ();
			DeviceSendFlag = ON;
			break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/

/*************************************************************************
	Function		: .  Jang_End_Time_Process
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . 장종료 후 발생한 주문 data 처리
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Jang_End_Time_Process (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;

    if (TCP2_NET_STA(S_K) != JOB_STOP)
        TCP2_NET_STA(S_K) = END;

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;
    TimeOut = FOREVER_TIME;

    PollCnt = 2;

    while (START_S != END)
    {
        Stat_Save ();

        if (PollCnt == 2)
        {
            if (WR_CNT > RD_CNT)
            {
                Jang_End_Time_File_Rtn ();
                continue;
            }
        }

        rt = poll (Poll, PollCnt, TimeOut);

        if (rt < 0)
        {
            if (SYS_NO == EINTR)
                SLog (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                SLog (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
            return;
        }
        else if (rt == 0)
        {
            SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
            return;
        }

        for (i = 0; i < PollCnt; i ++)
        {
            if (Poll[i].revents & POLLHUP)
            {
                SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
                return;
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
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    1:
                Jang_End_Time_File_Rtn ();
                break;
            default:
                SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
	}

	return;
}	/* End of Jang_End_Time_Process ()	*/

/*************************************************************************
	Function		: . Analyze_Data
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int TR_TYPE
	Comment			: . TCP Recv Data Analysis And TR_CODE Return
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
	FirstSeq = AtoIf (J_R_Fmt.Header.MsgSeqNum, 
		sizeof (J_R_Fmt.Header.MsgSeqNum));

	if (memcmp(J_R_Fmt.Header.MsgType, "SCHOPQ00000", 11) == 0)
	{
		SLog (USR_OK, "업무개시요청(SCHOPQ00000): <%d:%d:%d>",
			FirstSeq, INT_SEQ, LOAD_CNT);
		return (TR_LINK);
	else if (memcmp(J_R_Fmt.Header.MsgType, "SCHHER00000", 11) == 0)
	{
		SLog (USR_OK, "회선시험응답(SCHHER00000): <%d:%d:%d>",
			FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_TEST);
	}
	else if (memcmp(J_R_Fmt.Header.MsgType, "TCHODR00000", 11) == 0)
	{
		SLog (USR_OK, "주문응답(TCHODR00000): <%d:%d:%d>",
			FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function		: .  Jang_End_Time_File_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . 장종료후 주문 DATA 처리
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Jang_End_Time_File_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i, cnt;
	char	tmp[128];

	while (WRITE_CNT > READ_CNT)
	{
		Stat_Save ();

		memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
		memset (W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

		cnt = DSHM_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
		if (cnt < 0 || cnt > MAX_CNT)
		{
			SLog (SAM_FATAL, "DSHM_R(PS_R_1)[%s]", IDN(D_K,P_K,0));
			Exit_Process ();
		}
		else if (cnt == 0)
			return;

		memcpy (W_Fmt, R_Fmt, sizeof (FILE_BUFF_FORMAT) * cnt);

		for (i = 0; i < cnt; i ++)
		{
			if (TCP2_NET_STA(S_K) == JOB_STOP)
				memcpy (W_Fmt[i].ResponseCode, ERR_JANG_STOP,
					strlen (ERR_JANG_STOP));
			else
				memcpy (W_Fmt[i].ResponseCode, ERR_JANG_END,
					strlen (ERR_JANG_END));

            memcpy (W_Fmt[i].RecvTime1, "000.000000", 10);
		}

		rt = DSHM_W (TS_W1_1, (void *)W_Fmt, cnt);
		if (rt < 0)
		{
			SLog (SAM_FATAL, "file write[%s]", ODN(D_K,P_K,0));
			Exit_Process ();
		}

		Dshm_Add_Count (PS_R_1, cnt);

		if (TCP2_NET_STA(S_K) == JOB_STOP)
			SLog (USR_WARN, "write JANGSTOP data[%s:%d:%d]",
				ODN(D_K,P_K,0), ODW(D_K,P_K,0,0), cnt);
		else
			SLog (USR_WARN, "write JANGEND data[%s:%d:%d]",
				ODN(D_K,P_K,0), ODW(D_K,P_K,0,0), cnt);
	}

	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		if (rt == 0)
			break;
		else if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			SLog (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}",
				SYS_NO, SYS_STR);
			break;
		}
	}

	return;
}	/* End of Jang_End_Time_File_Rtn ()	*/

/*************************************************************************
	Function		: . Write_Respose_Data
	Parameters IN	: . d_cnt	: write count
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . 정상 주문에 대한 응답 file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void 	Write_Response_Data (int d_cnt)
/*----------------------------------------------------------------------*/
{
	int		i, rt;
    char    resp_time[12], order_reply[100];

	if (d_cnt <= 0)
		return;
	
	memset (W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
	memset (order_reply, ' ', sizeof(order_reply));

	for (i = 0; i < d_cnt; i ++)
	{
		memset (&Re_W_Fmt, ' ', sizeof(Re_W_Fmt));

		ItoAf (SHM_DATA[i].seq, W_Fmt[i].If_Seq, sizeof (W_Fmt[i].If_Seq));
		memcpy (W_Fmt[i].ApType, SHM_DATA[i].buf.ApType,
			sizeof (SHM_DATA[i].buf.ApType));

		/* ------------------------------------------------------------ */
		/* 주문응답(RP_DATA)인 경우, 									*/
		/*   ErrCd가 0000(정상),0102(매매시간종료)인 경우 Read_Cnt 증가,*/
		/*   이외 ErrCd의 경우 RETRY 처리.                            	*/
		/* ------------------------------------------------------------ */
		if (ReTrCode == RP_DATA)
		{
			if (ErrCd == 0)
			{
				ErrCd = AtoIf (J_R_Fmt.ReplyData[i].ErrCode,
					sizeof (J_R_Fmt.ReplyData[i].ErrCode));

				/* KRX Reply (24) */
				memcpy (order_reply, J_R_Fmt.ReplyData[i],
					sizeof(KRX_JUMUN_R_DATA));
			}

			if (ErrCd == 0) 
			{
				memcpy (W_Fmt[i].ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
			}
			else if (ErrCd == 102)
			{
            	W_Fmt[i].ResponseCode[0] = 'K';
            	memcpy (W_Fmt[i].ResponseCode+1, "102", 3);
			}
			else 
			{
				break;
			}
		}
		else 
		{
			memcpy (W_Fmt[i].ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
		}
		
        /* set KRX response time    */
        sprintf (resp_time, "%010.06f", RespMsec);
        memcpy (W_Fmt[i].RecvTime1, resp_time, sizeof (W_Fmt[i].RecvTime1));

		memcpy (W_Fmt[i].RecvTime2, SHM_DATA[i].buf.RecvTime2,
			sizeof (SHM_DATA[i].buf.RecvTime2));

		memcpy (W_Fmt[i].DataHeader, SHM_DATA[i].buf.DataHeader, HEAD_SIZE);

		/* DATA 조합(81+24+300) */
		/* KRX Header (81) */
		memcpy (&Re_W_Fmt.Header, SHM_DATA[i].buf.Data, sizeof(KRX_HEADER));
        ItoAf (SHM_DATA[i].seq, &W_Fmt[i].Data[25], 11);	/* 헤더의 일련번호 */

		/* KRX Reply (24) */
		memcpy (&Re_W_Fmt.ReplyData, order_reply, sizeof(KRX_JUMUN_R_DATA));
        ItoAf (SHM_DATA[i].seq, &W_Fmt[i].Data[85], 11);	/* 바디의 일련번호 */

		/* KRX Order (300) */
		memcpy (&Re_W_Fmt.JumunData,
			SHM_DATA[i].buf.Data[&sizeof(KRX_HEADER)], DATA_SIZE);

		memcpy (W_Fmt[i].Data, Re_W_Fmt, sizeof(Re_W_Fmt));

		W_Fmt[i].LineFeed[0] = '\n';

		if (DR_FLAG == 2 && memcmp (W_Fmt[i].ResponseCode,
			KRX_DATA_LOSS, strlen (KRX_DATA_LOSS)) != 0)
		{
			DR_FLAG = 3;
			SLog (USR_OK, "KRX data loss send OK: DR flag[%d]", DR_FLAG);
		}
	}

	rt = DSHM_W (TS_W1_1, (void *)W_Fmt, i);
	if (rt != i)
	{
		SLog (SAM_FATAL, "file write[%s]", ODN(D_K,P_K,0));
		Exit_Process ();
	}

	Dshm_Add_Count (PS_R_1, i);
	INT_SEQ += FirstSeq - INT_SEQ;

	SLog (USR_OK, "write response data[%s:%d:%d]",
		ODN(D_K,P_K,0), ODW(D_K,P_K,0,0), i);

	return;
}	/* End of Write_Response_Data ()	*/

/*************************************************************************
	Function		: . Make_Send_Msg
	Parameters IN	: . TR_TYPE
	Parameters OUT	: .
	Return Code		: . int (-1:no data, 0:success)
	Comment			: . Send data header making
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Make_Send_Msg (int tr_code)
/*----------------------------------------------------------------------*/
{
	int		rt, datacnt;
	char	d_time[18];

	rt = 0;
	memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));

	memcpy (S_Fmt.Header.BeginString, "KMAPv2.0", 8);
	ItoAf (0, S_Fmt.Header.BodyLength, sizeof (S_Fmt.Header.BodyLength));
	ItoAf (0, S_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
	memcpy (S_Fmt.Header.SenderComp, TCP_COMPANY, strlen (TCP_COMPANY));
	Get_DateMilliTime (d_time);
	memcpy (S_Fmt.Header.SendingTime, d_time, sizeof (S_Fmt.Header.SendingTime));
	ItoAf (0, S_Fmt.Header.DataCnt, sizeof (S_Fmt.Header.DataCnt));
	SendLen = sizeof (KRX_HEADER);

	switch (tr_code)
	{
		case	TR_LOON:	/* 로그온 요청 */
			memcpy (S_Fmt.Header.MsgType, 	"SCHLIQ00000", 	11);
			memcpy (S_Fmt.Data,     LOGON_ID(D_K,P_K), 10);
			memcpy (S_Fmt.Data[10], LOGON_PW(D_K,P_K), 30);
			MsgLen	= strlen(S_Fmt.Data);				
			ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	RP_LINK:	/* 업무개시 응답 */
			memcpy (S_Fmt.Header.MsgType, 	"SCHOPR00000", 	11);
            memcpy (S_Fmt.Header.MsgSeqNum, J_R_Fmt.Header.MsgSeqNum, sizeof (S_Fmt.Header.MsgSeqNum));
			memcpy (S_Fmt.Data, "0000", 4);
			memcpy (S_Fmt.Data[4], "          ", 11);
			ItoAf (INT_SEQ, S_Fmt.Data[15], 10);
			MsgLen	= strlen(S_Fmt.Data);				
			ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	ER_LINK:	/* 업무개시 Eror응답 */
			memcpy (S_Fmt.Header.MsgType, 	"SCHOPR00000", 	11);
            memcpy (S_Fmt.Header.MsgSeqNum, J_R_Fmt.Header.MsgSeqNum, 
					sizeof (S_Fmt.Header.MsgSeqNum));
			memcpy (S_Fmt.Data, "0004", 4);
			memcpy (S_Fmt.Data[4], "          ", 11);
			ItoAf (INT_SEQ, S_Fmt.Data[15], 10);
			MsgLen	= strlen(S_Fmt.Data);				
			ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	TR_TEST: 	/* 회선시험 요청 */
			memcpy (S_Fmt.Header.MsgType, 		"SCHHEQ00000", 	11);
			ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum,
					sizeof (S_Fmt.Header.MsgSeqNum));
			MsgLen	= strlen(S_Fmt.Data);				
			ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	TR_LOOU:	/* 로그아웃 요청 */
			memcpy (S_Fmt.Header.MsgType, 		"SCHLOQ00000", 	11);
			ItoAf (INT_SEQ, S_Fmt.Header.MsgSeqNum,
					sizeof (S_Fmt.Header.MsgSeqNum));
			MsgLen	= strlen(S_Fmt.Data);				
			ItoAf (MsgLen, S_Fmt.Header.BodyLength, sizeof(S_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	TR_DATA: 	/* 주문요청 */
			memset (&J_Q_Fmt, 0, sizeof (KRX_JUMUN_Q_FMT));
			memcpy (J_Q_Fmt.Header, S_Fmt.Header, sizeof (KRX_HEADER));

			memcpy (J_Q_Fmt.Header.MsgType, 		"TCHODR00000", 	11);
			ItoAf (INT_SEQ + 1, J_Q_Fmt.Header.MsgSeqNum,
				sizeof (J_Q_Fmt.Header.MsgSeqNum));

			datacnt = Make_Data_Block ();
			if (datacnt <= 0 || datacnt > MAX_CNT)
			{
				rt = -1;
				break;
			}
			MsgLen	= datacnt * sizeof (KRX_JUMUN_DATA); 				
			ItoAf (MsgLen, J_Q_Fmt.Header.BodyLength, 
					sizeof(J_Q_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			ItoAf (datacnt, J_Q_Fmt.Header.DataCnt,
				sizeof (J_Q_Fmt.Header.DataCnt));
			break;
		default:
			break;
	}

	return (rt);
}	/* End of Make_Send_Msg ()	*/

/*************************************************************************
	Function		: . Make_Data_Block
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int
						정상: KRX send record count (1 ~ MAX_CNT)
						0: no data
	Comment			: . 1) If Memory Data Exist -> Memory Data Send
						2) Data File Read & Time Out Check
							Time Out Check -> PS_EW  Write -> PS_R_1 Seq Add
							KRX Send Data  -> Seq Set & Memory Load
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Make_Data_Block (void)
/*----------------------------------------------------------------------*/
{
	int		r_cnt, t_cnt, i, j, rt;

	i = t_cnt = 0;
	memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
	memset (W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

	r_cnt = DSHM_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
	if (r_cnt < 0 || r_cnt > MAX_CNT)
	{
		SLog (SAM_FATAL, "DSHM_R(PS_R_1)[%s]", IDN(D_K,P_K,0));
		Exit_Process ();
	}
	else if (r_cnt == 0)
	{
		return (0);
	}

	for (i = 0; i < r_cnt; i ++)
	{
		LOAD_CNT ++;
		SHM_DATA[i].seq = INT_SEQ + LOAD_CNT;
		memcpy (SHM_DATA[i].buf.Seq, R_Fmt[i].Seq,
			sizeof (FILE_BUFF_FORMAT));
		ItoAf (SHM_DATA[i].seq, J_Q_Fmt.JumunData[i].DataSeq,
			sizeof (KRX_JUMUN_DATA.DataSeq));
		memcpy (J_Q_Fmt.JumunData[i].Data,
				&R_Fmt[i].Data[sizeof(KRX_HEADER)+
				sizeof(KRX_JUMUN_DATA.DataSeq)],
				sizeof(KRX_JUMUN_DATA_ST));
	}

	return (LOAD_CNT);
}	/* End of Make_Data_Block ()	*/

/*************************************************************************
	Function		: . get time to the unit of msec (millisecond)
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . double
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Get_Msec (double *msec)
/*----------------------------------------------------------------------*/
{
   struct timeval  tv;

   gettimeofday (&tv, NULL);
   *msec = tv.tv_sec + tv.tv_usec * 1e-6;

   return;
}	/* End of Get_Msec ()	*/

/*************************************************************************
	Function		: . Log_Out
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Log_Out (void)
/*----------------------------------------------------------------------*/
{
    int     rt, rval;

	rt = Make_Send_Msg (TR_LOOU);
	memset (DataBuff, 0, sizeof (DataBuff));
	memcpy (DataBuff, &S_Fmt, SendLen);
	Device_Write ();
    SLog (USR_OK, "send LOGOUT request");

	rval = Device_Read ();
	if (rval < 0)
    {
        SLog (USR_ERROR, "LOGOUT response recv error");
        close (Sockfd);
        return;
    }

   	if (memcmp(DataBuff[80], "0000", 4) == 0)
	{
        SLog (USR_OK, "LOGOUT success");

		Device_Close ();
        TCP2_NET_STA(S_K) = END;
	}
   	else 
	{
		ErrCd = AtoIf (DataBuff[80], 4); 
		Err_Msg ();
	}

	return;
}	/* End of Log_Out ()	*/

/*************************************************************************
	Function		: . Err_Msg
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Err_Msg (void)
/*----------------------------------------------------------------------*/
{
	switch (ErrCd)
	{
		case	0:	/* 정상	*/
			break;
		case	1:	/* 사용자검증(ID,PASSWORD)오류 */
			SLog (USR_ERROR, "사용자검증(ID,PASSWORD)오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	2:	/* 세션메시지전문수순오류 */
			SLog (USR_ERROR, "세션메시지전문수순오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	3:	/* 헤더회원번호오류 */
			SLog (USR_ERROR, "헤더회원번호오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	4:	/* 데이터일련번호오류 */
			SLog (USR_ERROR, "데이터일련번호오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	5:	/* 메시지길이오류 */
			SLog (USR_ERROR, "메시지길이오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	6:	/* 업무기마감오류 */
			SLog (USR_ERROR, "업무기마감오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	9:	/* 시스템오류 */
			SLog (USR_ERROR, "시스템오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case  101:	/* 호가접수개시전 */
			SLog (USR_ERROR, "호가접수개시전[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			memset (t_time, 0, sizeof (t_time));
			Get_Time (t_time);

			if (memcmp (t_time, NO_TIME, 4) < 0)		/* hhmm	*/
				sleep (10);
			sleep (2);
			break;
		case  102:	/* 매매거래시간종료후 */
			SLog (USR_ERROR, "매매거래시간종료후[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			sleep (2);
			break;
		case  103:	/* 호가접수일시중지 */
			SLog (USR_ERROR, "호가접수일시중지[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			sleep (2);
			break;
		default:
			SLog (USR_ERROR,
				"RP_DATA recv:unknown 거부사유코드[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
	}

	if (ErrCd == 102)	/* 매매거래시간종료후 */ /* 2 ?? */
	{
		ErrCd = 0;
		Log_Out();
	}
} /* End of Err_Msg ()	*/
/*************************************************************************
	End of Program (pa_1100_ts.c)
*************************************************************************/
