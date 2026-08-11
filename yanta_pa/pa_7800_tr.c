#define		_GLOBAL
	
/*------------------------------------------------------------------------
#	System	: Connect to KRX
#	Author	: PSH 
#	Module	: 일괄송신
#	File	: pa_7800_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"
#include    "INISAFENet.h"
/*
#include    "fep_fepj.h"
#include    "krx_struct.h"
*/

/* ********************************************** */
/*
	일괄송신(KRX기준) 중 2개만 저장하고 나머진 버림.
	TRDESP50101 : 채권종목정보 (1,173 byte),	7801
	TRDESP50102 : KTS종목정보  (  350 byte),	7802
*/
/* ********************************************** */

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define     TCP_TIME_OUT    35

#define		DEVICE_TIME		3  * 1000						/*  3 sec	*/
#define		MAIN_TIME		35 * 1000		/* Heart Beat 간격 30 sec	*/
#define		FOREVER_TIME	60 * 1000						/* 60 sec	*/

#define		FIFO_EVENT		0
#define     SOCKET_EVENT    1

#define		MAX_CNT			7				// RDS(일괄송신)는 묶음처리됨.

#define		NO_TIME			"0600"			// RDS(일괄송신)는 야간배치이기 때문에 의미없음

#define     PORT_NO         TCP2_PORT_NO

/* 암복호화 추가 */
#define KRX_INITECH_CONF_PATH "/user/fxa/fep/krx/INISAFE_Net_for_C_v7.2.47_64/conf/INISAFENet.cnf"

// 전역 정의 필요
net_ctx         *EnCtx = NULL;

unsigned char   *sinitout = NULL;
unsigned char   *supdateout = NULL;

unsigned char   *cinitout = NULL;
unsigned char   *cupdateout = NULL;
unsigned char   *cfinalout = NULL;

int             cinitoutl = 0;
int             cupdateoutl = 0;
int             cfinaloutl = 0;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		ConnectRetryCnt, D_End;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int		back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag;
char	DataBuff[4096], IpAddr[20], ApType[10];

KRX_HEADER					Header_Fmt;			// KRX Header (82)
KRX_JUMUN_R_FMT				Reply;				// 82+ 4+11 응답
FILE_DATA_HEAD				File_Data_Head;		// 20
KRX_R_SESSION_FMT			KR_Fmt;				// KRX 세션 포맷 (82+116)
struct pollfd		Poll[2];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7800_TR (void);
void	Init_Parameters (void);
void	Fifo_Event_Rtn (void);
void	Socket_Event_Rtn (void);
void	Device_Open (int);
void	Device_Close (void);
void	Device_Write (void);
int		Device_Read (void);
void    Line_Change (void);
void	Time_Out_Rtn (void);
int		Analyze_Data (void);
void	Write_Data (int, int);
int		Make_Send_Msg (int);
void    Get_Msec (double *);
void	Log_Out (void);
void	Err_Msg (void);
void	Check999 (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7800_TR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7800_TR (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;

	Init_Parameters ();

	while (START_S != END)
	{
		Stat_Save ();

		if (OpenFlag == OFF)				// 개시가 안된경우
		{
			if (LogOnFlag == OFF)				// 로그온이 안된경우 로그온 시도

				Device_Open (TR_LOON);

			if (LogOnFlag == OFF)				// 아직 로그온이 안된경우
			{
#if 0
2025 포트가 없다. 안한다.
				ConnectRetryCnt ++;
				if (ConnectRetryCnt >= 3)
				{
					Line_Change ();
					ConnectRetryCnt = 0;
				}
#endif

				PollCnt = 1;
				TimeOut = DEVICE_TIME;			// 3초
			}
#if 0
			else								// 개시가 된 경우
			{
				PollCnt = 2;
				TimeOut = MAIN_TIME;			// 35초
			}
#endif
			else								// 로그온은 된경우
			{
				Device_Open (TR_LINK);

				if (OpenFlag == OFF)			// 아직 로그온이 안된경우
				{
					PollCnt = 1;
					TimeOut = DEVICE_TIME;		// 3초
				}
				else
				{
					PollCnt = 2;
					TimeOut = MAIN_TIME;		// 35초
				}
			}
		}
		else									// 개시가 된 경우
		{
			PollCnt = 2;
			TimeOut = MAIN_TIME;				// 35초
		}

        rt = poll (Poll, PollCnt, TimeOut);
        if (rt < 0)
        {
            if (SYS_NO == EINTR)
                Log (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
            else
                Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);

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
            case    FIFO_EVENT:
                Fifo_Event_Rtn ();
                break;
            case    SOCKET_EVENT:
                Socket_Event_Rtn ();
                break;
            default:
                Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    }

    Device_Close ();

	return;
}	/* End of PA_7800_TR ()	*/

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
    D_End = ErrCd = 0;
	ConnectRetryCnt = 0;

	if (TIME_OUT == 0)
        TIME_OUT = TCP_TIME_OUT;
    Log (USR_OK, "TIME_OUT[%d]", TIME_OUT);

    S_K = TCP2_LINE_GU;

    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log (TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    //TCP2_PROC_ST = ON;
	TCP2_PROC_ST1(MAIN) = ON;
    TCP2_PROC_ST1(BACKUP) = ON;

    //TCP2_LINE_ST = OFF;
    TCP2_LINE_ST1(MAIN) = OFF;
    TCP2_LINE_ST1(BACKUP) = OFF;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU (ApType, strlen (ApType));

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
/*
    Poll[2].fd = INPUT_FD;			//	Input있으면
    Poll[2].events = POLLIN;		//	Input있으면
*/

	/* Log */
	Log (USR_OK, "DSHM t_seq [%d]", PROC(D_K,P_K).tr_seq);
	Log (USR_OK, "DSHM curr_tr[%11.11s]", PROC(D_K,P_K).curr_tr);

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
	int		rval, rt, cnt, r_meg_no, r_meg_seq;
	char	t_time[12];

	rval = Device_Read ();

	if (rval < 0)
		return;

	DeviceSendFlag = OFF;
	ErrCd = 0;

	memset (&Header_Fmt, 0, KRX_HEAD_LEN);
	memcpy (&Header_Fmt, DataBuff, KRX_HEAD_LEN);

	ReTrCode = Analyze_Data ();

	rt = 0;
	/* ************************************ */
	/* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
	/* ************************************ */
	switch (ReTrCode)
	{
		case    TR_DATA:
            rt = Make_Send_Msg (RP_DATA);           // TR_DATA,RP_DATA,RP_POLL,RP_STOP
            memset (DataBuff, 0, sizeof (DataBuff));
            memcpy (DataBuff, &KR_Fmt, SendLen);
            Device_Write ();
            Log (USR_OK, "DATA Reply OK!!");

/*
			INT_SEQ = 0;
			PROC(D_K,P_K).tr_seq = INT_SEQ;
			//memcpy (PROC(D_K,P_K).curr_tr,	Header_Fmt.MsgType,		11);
*/

            break;
	
		case	RP_DATA:
			/* Seq Check */
			/* 모든 Data는 +1로 수신받는다, 아니면 로그찍고 종료 */
 			if (FirstSeq == 9999999)
				break;

			if (INT_SEQ+1 != FirstSeq)
			{
				TCP2_LINE_ST = OpenFlag = OFF;

				Log (USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
					FirstSeq, INT_SEQ);

				sleep (3);							// 3초후 종료
				Device_Close ();
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

			/* TR 구분 */
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TRDESP50101",	11) == 0)		// RDS01 (채권종목정보)
			{
				if (memcmp (&DataBuff[KRX_HEAD_LEN-1], "Y", 1) == 0)
					Write_Data (1, 1);			// pa_7801_dd(1400)
				else
					Write_Data (1, 0);			// pa_7801_dd(1400)
			}
			else
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TRDESP50102",	11) == 0)		// RDS02 (KTS종목정보)
			{
				if (memcmp (&DataBuff[KRX_HEAD_LEN-1], "Y", 1) == 0)
					Write_Data (2, 1);			// pa_7802_dd(350)
				else
					Write_Data (2, 0);			// pa_7802_dd(350)
			}
			else
			{
				if (memcmp (&DataBuff[KRX_HEAD_LEN-1], "Y", 1) == 0)
					Check999 (1);
				else
					Check999 (0);
				// Skip Data는 SEQ만 증가시켜서 Skip시킨다.
				memcpy (PROC(D_K,P_K).curr_tr,  Header_Fmt.MsgType,     11);
				Log (USR_OK, "Data Skip tr_seq[%d], TR[%11.11s]", PROC(D_K,P_K).tr_seq, PROC(D_K,P_K).curr_tr);
			}

			rt = Make_Send_Msg (RP_DATA);			// TR_DATA,RP_DATA,RP_POLL,RP_STOP
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &KR_Fmt, SendLen);
			Device_Write ();
			Log (USR_OK, "DATA Reply OK!!");

			break;

		case	RP_POLL:
			rt = Make_Send_Msg (TR_POLL);
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &KR_Fmt, SendLen);
			Device_Write ();

			break;

		case	RP_LOON:
			if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000",	4) != 0)
			{
				Log (USR_ERROR, "LOGON Error [%4.4s]", &DataBuff[KRX_HEAD_LEN]);
				sleep (3);							// 3초후 종료
				Device_Close ();
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

			break;

		case	RP_STOP:
			Log (USR_OK, "LOGOUT OK Done");

			sleep (3);							// 3초후 종료
			Device_Close ();

			PROC(D_K,P_K).start_status = JOB_END;
			Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			break;

		default:
			break;
	}

	if (D_End == 1)
	{
//		INT_SEQ = 0;			// 배치는 TR별로 처리하는데 TR의 종료(999..9)면 Seq를 초기화한다.
		D_End = 0;
	}

	return;
}	/* End of Socket_Event_Rtn ()	*/

/*************************************************************************
    Function        : .  Free_All
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : .
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Free_All (ctxf)
/*----------------------------------------------------------------------*/
{
    if (cinitout)
    {
        INL_Free_Buf(cinitout);
        cinitout = NULL;
    }
    if (cupdateout)
    {
        INL_Free_Buf(cupdateout);
        cupdateout = NULL;
    }
    if (cfinalout)
{
        INL_Free_Buf(cfinalout);
        cfinalout = NULL;
    }
    if (sinitout)
    {
        INL_Free_Buf(sinitout);
        sinitout = NULL;
    }
    if (supdateout)
    {
        INL_Free_Buf(supdateout);
        supdateout = NULL;
    }

    if (ctxf && EnCtx)
    {
        INL_Free_Ctx(EnCtx);
        EnCtx = NULL;
    }
}

/*************************************************************************
	Function		: .  Device_Open
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Svm Line Status Set & TCPIP Poll fd set & LOGON
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Device_Open (int tr_code)
/*----------------------------------------------------------------------*/
{
	int     rt, rval;

	if (tr_code == TR_LOON)
	{
		Sockfd = Socket ();

		if (Sockfd < 0)
		{
			Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
				Sockfd, SYS_NO, SYS_STR);
			return;
		}

		Log (USR_OK, "socket created:Sockfd[%d]", Sockfd);
		Log (USR_OK, "connecting to %s:%d", IpAddr, PORT_NO);

		rt = Connect (Sockfd, IpAddr, PORT_NO);

		if (rt < 0)
		{
			Log (TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
			close (Sockfd);
			return;
		}

		/* 20251013 Add */
sleep (5);
		if (Handshake() < 0)
		{
			close (Sockfd);
			return;
		}
		/* 20251013 Add */

		rt = Make_Send_Msg (TR_LOON);
		memset (DataBuff, 0, sizeof (DataBuff));
		memcpy (DataBuff, &KR_Fmt, SendLen);
		Device_Write ();
		Log (USR_OK, "send LOGON request");

		rval = Device_Read ();
		if (rval < 0 ||	
			memcmp(&DataBuff[8+6], "SCHLIR00000", 11) != 0)
		{
			Log (USR_ERROR, "LOGON response recv error");
			close (Sockfd);
			return;
		}

		if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)
		{
			Log (USR_OK, "LOGON success");
		}
		else
		{
		   ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], 4);
		   Err_Msg ();

		   close (Sockfd);
		   return;
		}

		LogOnFlag = ON;
		TCP2_NET_STA(S_K) = ON;
		TCP2_LINE_ST = OpenFlag = ON;
		ConnectRetryCnt = 0;


		Poll[1].fd = Sockfd;
		Poll[1].events = POLLIN;
		Log (TCP_OK, "TCP Connect & LOGON OK");
	}
	else if (tr_code == TR_LINK)
    {
        while (START_S != END)
        {
            rt = Make_Send_Msg (TR_LINK);
            memset (DataBuff, 0, sizeof (DataBuff));
            memcpy (DataBuff, &KR_Fmt, SendLen);
            Device_Write ();
            Log (USR_OK, "send LINK request");

            rval = Device_Read ();
            if (rval < 0 ||
                memcmp(&DataBuff[8+6], "SCHOPR00000", 11) != 0)
            {
                Log (USR_ERROR, "LINK response recv error");
                close (Sockfd);
                break;
            }

            memset (&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
            memcpy (&KR_Fmt, DataBuff, RecvLen);

            FirstSeq = AtoIf (KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

            if (memcmp(KR_Fmt.Data, "0000", 4) == 0)
            {
                /* Seq 체크  */
                if (INT_SEQ != FirstSeq)
                {
                    TCP2_LINE_ST = OpenFlag = OFF;

                    Log (USR_ERROR, "PR_LINK recv:invalid KRX SEQ [%d] INT_SEQ[%d]",
                        FirstSeq, INT_SEQ);

                    sleep (3);                          // 3초후 종료
                    Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
                }

                TCP2_NET_STA(S_K) = ON;
                TCP2_LINE_ST = OpenFlag = ON;
                ConnectRetryCnt = 0;

                Log (TCP_OK, "LINK OK");

                break;
            }
            else
            {
                Log (USR_ERROR, "PR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ[%d] INT_SEQ[%d>",
                    KR_Fmt.Data, FirstSeq, INT_SEQ);
                Log (USR_ERROR, "PR_LINK recv:invalid ME_G[%d:%d:%d:%d:%d:%d:%d:%d:%d:%d]",
                    INT_MEG_SEQ(0), INT_MEG_SEQ(1), INT_MEG_SEQ(2),INT_MEG_SEQ(3),INT_MEG_SEQ(4),
                    INT_MEG_SEQ(5), INT_MEG_SEQ(6), INT_MEG_SEQ(7),INT_MEG_SEQ(8),INT_MEG_SEQ(9));

                sleep (3);                          // 3초후 종료
                Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}
        }   // End Of While
    }

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
    Log (TCP_OK, "TCP device close");

    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;

	/* 2025 암호화 초기화 작업 */
//  INL_CtxFree(EnCtx);
    Free_All(1);
    INL_Cleanup(CLIENT_CTX);

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

//    rt = Select_Send (Sockfd, DataBuff, strlen (DataBuff));
	rt = Select_Send (Sockfd, DataBuff, SendLen);

    if (rt != OK)
    {
        Log (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

//    Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
	Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);
	Get_Msec (&SendMsec);

	/* 현물과 다른부분 */
#if 0
   	if (memcmp (J_Q_Fmt.Header.MsgType, "TCHODR00000", 11) == 0)	/* 주문 */
    {
        STime = AtoIf (R_Fmt[0].RecvTime2, 2) * 60 * 60 +
                AtoIf (R_Fmt[0].RecvTime2+2, 2) * 60 +
                AtoIf (R_Fmt[0].RecvTime2+4, 2) +
                AtoIf (R_Fmt[0].RecvTime2+6, 6) / 1000000.0;

        Log (TCP_OK, "Inside Calculation Time [%.06f] [%5.5s]", 
			SendMsec - STime, R_Fmt[0].Data+185);		 /* 회원사 처리항목 */
    }
#endif

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

    rt = Select_Receive_Krx (Sockfd, DataBuff, KRX_HEAD_LEN);

    if (rt <= 0)
    {
        Device_Close ();
        return (NOTOK);
    }

	RecvLen = rt;	
   	//Log (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
	Log (TCP_OK, "TCP RD [%85.85s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);

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
		case    1:
            if (TimeOut == FOREVER_TIME)
                Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
/* 
일괄송신은 KRX가 아닌 증권사에서 POLL을 송신한다. 그래서 이부분을 미사용한다.2025
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                Log (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }
			rt = Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &Reply, sizeof (KRX_JUMUN_R_FMT));
			Device_Write ();
*/

            break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/

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
	//FirstSeq = AtoIf (Header_Fmt.MsgSeqNum, sizeof (Header_Fmt.MsgSeqNum));
	FirstSeq = AtoIf (&Header_Fmt.MsgSeqNum[4],		7);

	/* 2025 로직 변경 */
	if (memcmp (PROC(D_K,P_K).curr_tr,    Header_Fmt.MsgType,     11) != 0)
	{
		INT_SEQ = 0;
		PROC(D_K,P_K).tr_seq = INT_SEQ;
		memcpy (PROC(D_K,P_K).curr_tr,    Header_Fmt.MsgType,     11);
	}

	if (memcmp(Header_Fmt.MsgType, "SCHLIR00000", 11) == 0)				// LogOn응답
	{
		Log (USR_OK, "LogOn응답(SCHLIR00000): [%4.4s] <%d:%d:%d>",
			&DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_LOON);
	}
	else if (memcmp(Header_Fmt.MsgType, "SCHLOR00000", 11) == 0)		// LogOut응답
	{
		Log (USR_OK, "LogOut응답(SCHLOR00000): [%4.4s] <%d:%d:%d>",
			&DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_STOP);
	}
	else if (memcmp(Header_Fmt.MsgType, "SCHHEQ00000", 11) == 0)		// HeartBeat응답
	{
		Log (USR_OK, "회선시험응답(SCHHER00000): [%4.4s] <%d:%d:%d>",
			&DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_POLL);
	}
	else if (FirstSeq == 9999999)			// if 문의 순서를 꼭 지켜야 업무처리가 문제없음.(순서 지켜야함)
	{
		Log (USR_OK, "Data 99999999999 수신 INT_SEQ[%d]", INT_SEQ);
		return (TR_DATA);
	}
	else if ((memcmp(Header_Fmt.MsgType, "TRDESP50101", 11) == 0)	||	// Data, TRDESP50101 (RDS01)
			 (memcmp(Header_Fmt.MsgType, "TRDESP50102", 11) == 0) 	)	//		 TRDESP50102 (RDS02)
	{
		Log (USR_OK, "DATA수신(TRDESP50101/2): <%d:%d>", FirstSeq, INT_SEQ);
		return (RP_DATA);
	}
	else
	{
		Log (USR_OK, "Else Case 수신(Skip Data): <%d:%d> Tr[%11.11s]", FirstSeq, INT_SEQ, Header_Fmt.MsgType);
		return (RP_DATA);
	}

}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function		: . Check999
	Parameters IN	: . p_flag	: write file
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void 	Check999 (int YN)
/*----------------------------------------------------------------------*/
{
	int     rt, seq, i, for_i, d_size, d_seq, body_len;
    char    m_time[24];
	char	w_data[2048];

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	for_i	=	AtoIf (Header_Fmt.DataCnt,		sizeof (Header_Fmt.DataCnt));
	body_len = AtoIf (Header_Fmt.BodyLength,    sizeof (Header_Fmt.BodyLength));


	if (YN)			// 암호화("Y")
	{
		/* ******************************************************************** */
		/* 20251013, START
		 * 암복호화하는 로직을 함수로 사용하지 않고 직접 사용(n건이라) */
		int				result = 0;
		unsigned char	*dec_data = NULL;
		int				dec_len = 0;

		// -------------------------------------------------------
		// 1. 복호화 호출
		//    - 헤더(82바이트)는 제외
		//    - body_len은 암호화된 데이터부의 길이
		// -------------------------------------------------------
		result = INL_Decrypt(EnCtx,
							 (unsigned char*)&DataBuff[82],		// 암호화된 본문 시작 위치
							 body_len,						// 암호문 길이
							 &dec_data,						// 복호화 결과 버퍼 (라이브러리에서 할당)
							 &dec_len);						// 복호화 결과 길이

		if (result != 0) {
			Log (USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
			if (dec_data != NULL) INL_Free_Buf(dec_data);
			sleep(10);
			return;
		}

		// -------------------------------------------------------
		// 2. skip
		// 3. 복호화된 데이터 사용
		// -------------------------------------------------------
		Log (USR_OK, "복호화 성공! 복호화된 길이 = [%d]", dec_len);
		Log (USR_OK, "복호화된 데이터: [%s]", dec_data);

				
		/* 암호화된 DATA의 사이즈 정보 */
		d_size	 = dec_len / for_i;
	Log (USR_OK, "dec_len[%d] d_size[%d] for_i[%d]", dec_len, d_size, for_i);

		/* n건 수신시 암호화된 데이터의 사이즈가 모두 동일한지는 알지 못한다. 일단 테스트 해보고 수정하든하자 */ 
		for (i = 0; i < for_i; i++)
		{
			/* 20251013, END */
			/* ******************************************************************** */

			/* INT_SEQ 초기화 */ 
			d_seq = AtoIf (&dec_data[5+i*d_size], 6);			// 11자리중 뒤 6자리만
	Log (USR_OK, "dec_data[%20.20s] d_seq[%d]", &dec_data[5+i*d_size], d_seq);
			if ((i == for_i-1)	&&	// 마지막 DATA 인데 "99999999999"면 INT_SEQ를 초기화한다.(& 업무기마감처리)
				(d_seq == 999999))
				D_End = 1;

			if (INT_SEQ+1 != d_seq	&& D_End == 0)							// 마감이 아니면서 seq가 다름
			{
				Log (USR_ERROR, "i[%d] PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", i, d_seq, INT_SEQ);
				// -------------------------------------------------------
				// 4. 메모리 해제
				// -------------------------------------------------------
				INL_Free_Buf(dec_data);
	sleep(10);
				return;
			}

			if (D_End == 0)
			{
				INT_SEQ += 1;		// 9999..99는 SEQ증가 안시킴
				PROC(D_K,P_K).tr_seq = INT_SEQ;
				memcpy (PROC(D_K,P_K).curr_tr,	Header_Fmt.MsgType,		11);
//				Log (USR_OK, "SEQ Add");
			}

			Set_TR_Time ();
		}

		// -------------------------------------------------------
		// 4. 메모리 해제
		// -------------------------------------------------------
		INL_Free_Buf(dec_data);
	}
	else				// 평문수신 ('N' or Not 'Y')
	{
		d_size = body_len / for_i;

Log (USR_OK, "평문 d_size[%d] body_len[%d] for_i[%d]", d_size, body_len, for_i);

		for (i = 0; i < for_i; i++)
		{
			/* 20251013, END */
			/* ******************************************************************** */

			/* INT_SEQ 초기화 */ 
			d_seq = AtoIf (&DataBuff[82+5 + i*d_size], 6);
			if ((i == for_i-1)	&&	// 마지막 DATA 인데 "99999999999"면 INT_SEQ를 초기화한다.(& 업무기마감처리)
				(d_seq == 999999))
				D_End = 1;

			if (INT_SEQ+1 != d_seq	&& D_End == 0)							// 마감이 아니면서 seq가 다름
			{
				Log (USR_ERROR, "i[%d] PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", i, d_seq, INT_SEQ);
sleep(10);
				return;
			}

			if (D_End == 0)
			{
				INT_SEQ += 1;		// 9999..99는 SEQ증가 안시킴
				PROC(D_K,P_K).tr_seq = INT_SEQ;
				memcpy (PROC(D_K,P_K).curr_tr,	Header_Fmt.MsgType,		11);
//				Log (USR_OK, "SEQ Add");
			}

			Set_TR_Time ();
		}
	}


}	// End of Check999


/*************************************************************************
	Function		: . Write_Data
	Parameters IN	: . p_flag	: write file
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void 	Write_Data (int p_flag, int YN)
/*----------------------------------------------------------------------*/
{
	int     rt, seq, i, for_i, d_size, d_seq, body_len;
    char    m_time[24];
	char	w_data[2048];

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	for_i	=	AtoIf (Header_Fmt.DataCnt,		sizeof (Header_Fmt.DataCnt));

	/* 암호화된 DATA의 사이즈 정보 */
	body_len = AtoIf (Header_Fmt.BodyLength,	sizeof (Header_Fmt.BodyLength));
	d_size	 = AtoIf (Header_Fmt.BodyLength,	sizeof (Header_Fmt.BodyLength)) / for_i;

	if (YN)
	{

			/* ******************************************************************** */
			/* 20251013, START
			 * 암복호화하는 로직을 함수로 사용하지 않고 직접 사용(n건이라) */
			int				result = 0;
			unsigned char	*dec_data = NULL;
			int				dec_len = 0;

			// -------------------------------------------------------
			// 1. 복호화 호출
			//    - 헤더(82바이트)는 제외
			//    - body_len은 암호화된 데이터부의 길이
			// -------------------------------------------------------
			result = INL_Decrypt(EnCtx,
								 (unsigned char*)&DataBuff[82],		// 암호화된 본문 시작 위치
								 body_len,						// 암호문 길이
								 &dec_data,						// 복호화 결과 버퍼 (라이브러리에서 할당)
								 &dec_len);						// 복호화 결과 길이

			if (result != 0) {
				Log (USR_ERROR, "ERROR: INL_Decrypt failed with code %d\n", result);
				if (dec_data != NULL) INL_Free_Buf(dec_data);
				return;
			}

			// -------------------------------------------------------
			// 2. skip
			// 3. 복호화된 데이터 사용
			// -------------------------------------------------------
			Log (USR_OK, "복호화 성공! 복호화된 길이 = %d\n", dec_len);
			Log (USR_OK, "복호화된 데이터: %.*s\n", dec_len, dec_data);

				
			/* 20251013, END */
			/* ******************************************************************** */

		d_size	= 	dec_len / for_i;

		/* n건 수신시 암호화된 데이터의 사이즈가 모두 동일한지는 알지 못한다. 일단 테스트 해보고 수정하든하자 */ 
		for (i = 0; i < for_i; i++)
		{
			/* INT_SEQ 초기화 */ 
			d_seq = AtoIf (&dec_data[5 + i*d_size], 6);			// 11자리중 뒤 6자리만
			if ((i == for_i-1)	&&	// 마지막 DATA 인데 "99999999999"면 INT_SEQ를 초기화한다.(& 업무기마감처리)
				(d_seq == 999999))
				D_End = 1;

			if (INT_SEQ+1 != d_seq	&& D_End == 0)							// 마감이 아니면서 seq가 다름
			{
				Log (USR_ERROR, "PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", d_seq, INT_SEQ);
				// -------------------------------------------------------
				// 4. 메모리 해제
				// -------------------------------------------------------
				INL_Free_Buf(dec_data);

				return;
			}

			if (D_End == 0)
			{
				INT_SEQ += 1;		// 9999..99는 SEQ증가 안시킴
				PROC(D_K,P_K).tr_seq = INT_SEQ;
				memcpy (PROC(D_K,P_K).curr_tr,	Header_Fmt.MsgType,		11);
//				Log (USR_OK, "SEQ Add");
			}

			if (p_flag)		// 0은 처리배제
			{
				memset (w_data, 0x20,           sizeof (w_data));
				memset (&File_Data_Head, ' ',   sizeof (HEAD_SIZE));

				/* Header 50 byte */
				ItoAf (d_seq,			&w_data[0],     8);                     // Seq(8)
				ItoAf (d_seq,			&w_data[8],     8);                     // IF_Seq(8)
				memcpy (&w_data[16],    ApType,         8);                     // ApType(8)
				memcpy (&w_data[24],    RES_NORMAL,     strlen (RES_NORMAL));   // "0000"(4)
				memcpy (&w_data[28],    m_time,         10);                    // RecvTime1(10)
				memcpy (&w_data[38],    &m_time[10],    12);                    // RecvTime2(12)

				/* Heaer 20 byte */
				ItoAf (d_size,		File_Data_Head.Length,	sizeof (File_Data_Head.Length));    // 4
				ItoAf (d_seq,		File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));   // 8
				memcpy (File_Data_Head.ResponseCode,    RES_NORMAL,		strlen (RES_NORMAL));	// 4
				memcpy (File_Data_Head.LineFlag,        _Exe_Name+4,	3);						// 3

				memcpy (&w_data[50],	&File_Data_Head,	sizeof (HEAD_SIZE));

				/* Data */
				memcpy (&w_data[70],	&dec_data[i*d_size],	d_size);	// 복호화된 DATA SIZE
				w_data[70 + OFS(D_K,P_K,p_flag-1)] = '\n';

				rt = F_W (p_flag * 10, (void *)w_data, 1);
				if (rt != 1)
				{
					Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
					// -------------------------------------------------------
					// 4. 메모리 해제
					// -------------------------------------------------------
					INL_Free_Buf(dec_data);

					return;
				}

				Log (USR_OK, "file write[%s:%d:%d]",
					OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);
			}

			Set_TR_Time ();
		}

		// -------------------------------------------------------
		// 4. 메모리 해제
		// -------------------------------------------------------
		INL_Free_Buf(dec_data);

	}
	else
	{
		/* n건 수신시 암호화된 데이터의 사이즈가 모두 동일한지는 알지 못한다. 일단 테스트 해보고 수정하든하자 */ 
		for (i = 0; i < for_i; i++)
		{
			/* INT_SEQ 초기화 */ 
			//d_seq = AtoIf (&dec_data[5 + i*d_size], 6);			// 11자리중 뒤 6자리만
			d_seq = AtoIf (&DataBuff[82+5 + i*d_size], 6);			// 11자리중 뒤 6자리만
			if ((i == for_i-1)	&&	// 마지막 DATA 인데 "99999999999"면 INT_SEQ를 초기화한다.(& 업무기마감처리)
				(d_seq == 999999))
				D_End = 1;

			if (INT_SEQ+1 != d_seq	&& D_End == 0)							// 마감이 아니면서 seq가 다름
			{
				Log (USR_ERROR, "PR_DATA Data Seq ERROR DataSeq[%d] INT_SEQ[%d]", d_seq, INT_SEQ);
				return;
			}

			if (D_End == 0)
			{
				INT_SEQ += 1;		// 9999..99는 SEQ증가 안시킴
				PROC(D_K,P_K).tr_seq = INT_SEQ;
				memcpy (PROC(D_K,P_K).curr_tr,	Header_Fmt.MsgType,		11);
//				Log (USR_OK, "SEQ Add");
			}

			if (p_flag)		// 0은 처리배제
			{
				memset (w_data, 0x20,           sizeof (w_data));
				memset (&File_Data_Head, ' ',   sizeof (HEAD_SIZE));

				/* Header 50 byte */
				ItoAf (d_seq,			&w_data[0],     8);                     // Seq(8)
				ItoAf (d_seq,			&w_data[8],     8);                     // IF_Seq(8)
				memcpy (&w_data[16],    ApType,         8);                     // ApType(8)
				memcpy (&w_data[24],    RES_NORMAL,     strlen (RES_NORMAL));   // "0000"(4)
				memcpy (&w_data[28],    m_time,         10);                    // RecvTime1(10)
				memcpy (&w_data[38],    &m_time[10],    12);                    // RecvTime2(12)

				/* Heaer 20 byte */
				ItoAf (d_size,		File_Data_Head.Length,	sizeof (File_Data_Head.Length));    // 4
				ItoAf (d_seq,		File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));   // 8
				memcpy (File_Data_Head.ResponseCode,    RES_NORMAL,		strlen (RES_NORMAL));	// 4
				memcpy (File_Data_Head.LineFlag,        _Exe_Name+4,	3);						// 3

				memcpy (&w_data[50],	&File_Data_Head,	sizeof (HEAD_SIZE));

				/* Data */
				//memcpy (&w_data[70],	&dec_data[i*d_size],	d_size);	// 복호화된 DATA SIZE
				memcpy (&w_data[70],	&DataBuff[i*d_size],    d_size);    // 복호화된 DATA SIZE
				w_data[70 + OFS(D_K,P_K,p_flag-1)] = '\n';

				rt = F_W (p_flag * 10, (void *)w_data, 1);
				if (rt != 1)
				{
					Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
					return;
				}

				Log (USR_OK, "file write[%s:%d:%d]",
					OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);
			}

			Set_TR_Time ();
		}
	}

	return;
}	/* End of Write_Data ()	*/

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
	int		i, rt, datacnt;
	char	d_time[18];

	rt = 0;
	memset (&KR_Fmt,		0,	sizeof (KRX_R_SESSION_FMT));
	memset (&KR_Fmt, 0x20, sizeof (KRX_HEADER));

	memcpy (KR_Fmt.Header.BeginString,	"KMAPv2.0",	8);
	ItoAf (0, KR_Fmt.Header.BodyLength,	sizeof (KR_Fmt.Header.BodyLength));
	ItoAf (0, KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));		// 아래에서 Update
	memcpy (KR_Fmt.Header.SenderCompID,	TCP_COMPANY, strlen (TCP_COMPANY));
	Get_DateMilliTime (d_time);
	memcpy (KR_Fmt.Header.SendingTime,	d_time,	sizeof (KR_Fmt.Header.SendingTime));
	ItoAf (0, KR_Fmt.Header.DataCnt,	sizeof (KR_Fmt.Header.DataCnt));
	memcpy (KR_Fmt.Header.Encrypt,		"N",		1);		// 초기는 N, 주문만 Y
	SendLen = KRX_HEAD_LEN;

	switch (tr_code)
	{
		/* 2025 Add */
		case    TR_HSI:     /* handshake init */
            memcpy (KR_Fmt.Header.MsgType,  "SCHLIQ00101",      11);
            memcpy (KR_Fmt.Data, "0000", 4);                // INL_Handshake_Init() return value
            memcpy (&KR_Fmt.Data[4], (char *)cinitout, cinitoutl);
            MsgLen = cinitoutl + 4;
            ItoAf (MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSU:     /* handshake update */
            memcpy (KR_Fmt.Header.MsgType,  "SCHLIQ00103",      11);
            memcpy (KR_Fmt.Data, "0000", 4);                // INL_Handshake_Update() return value
            memcpy (&KR_Fmt.Data[4], (char *)cupdateout, cupdateoutl);
            MsgLen = cupdateoutl + 4;
            ItoAf (MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_HSF:     /* handshake final */
            memcpy (KR_Fmt.Header.MsgType,  "SCHLIQ00105",      11);
            memcpy (KR_Fmt.Data, "0000", 4);                // INL_Handshake_Final() return value
            memcpy (&KR_Fmt.Data[4], (char *)cfinalout, cfinaloutl);
            MsgLen = cfinaloutl + 4;
            ItoAf (MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
		/* 2025 Add */
		case    TR_LOON:    /* 로그온 요청 */
            memcpy (KR_Fmt.Header.MsgType,  "SCHLIQ00000",      11);
            ItoAf (INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
            memset (KR_Fmt.Data,    0x20,   41);                        // Logon Size
            memcpy (KR_Fmt.Data,            LOGON_ID(D_K,P_K),  10);
            memcpy (&KR_Fmt.Data[10],       LOGON_PW(D_K,P_K),  strlen(LOGON_PW(D_K,P_K)));
            memcpy (&KR_Fmt.Data[40],       "Y",                 1);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf (MsgLen, KR_Fmt.Header.BodyLength,    sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
            break;
        case    TR_LINK:    /* 업무개시 요청 */
Log (USR_OK, "업무개시 요청");
            memcpy (KR_Fmt.Header.MsgType,  "SCHOPQ00000",      11);
            ItoAf (INT_SEQ, KR_Fmt.Header.MsgSeqNum,    sizeof (KR_Fmt.Header.MsgSeqNum));
			memcpy (KR_Fmt.Data, "0000", 4);
			if (memcmp (PROC(D_K,P_K).curr_tr, "           ", 11) > 0)
			{
				memcpy (&KR_Fmt.Data[4],	PROC(D_K,P_K).curr_tr,	11);
				ItoAf  (PROC(D_K,P_K).tr_seq,	&KR_Fmt.Data[15],	11);
			}
			else
			{
				memcpy (&KR_Fmt.Data[4], "           ",	11);
				ItoAf (INT_SEQ, &KR_Fmt.Data[15],		11);
			}
/*
			memcpy (&KR_Fmt.Data[4], "           ",	11);
			ItoAf (INT_SEQ, &KR_Fmt.Data[15],		11);
*/

            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf (MsgLen, KR_Fmt.Header.BodyLength, sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;

            break;		
		case	TR_POLL: 	/* 회선시험 요청 */
			memcpy (KR_Fmt.Header.MsgType,	"SCHHER00000",  11);
            ItoAf (INT_SEQ, KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy (KR_Fmt.Data,			"0000",			 4);
            MsgLen  = strlen(KR_Fmt.Data);
            ItoAf (MsgLen,	KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
            SendLen += MsgLen;
			break;
		case	TR_LOOU:	/* 로그아웃 요청 */
			memcpy (KR_Fmt.Header.MsgType,	"SCHLOR00000", 	11);
			ItoAf (INT_SEQ,	KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
			MsgLen	= strlen(KR_Fmt.Data);				
			ItoAf (MsgLen,	KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	TR_DATA:	/* 미처리 데이터, DATA응답만 보낸다. */
		case	RP_DATA:	/* 로그아웃 요청 */
			memcpy (KR_Fmt.Header.MsgType,	Header_Fmt.MsgType, 	11);
			ItoAf (INT_SEQ,	KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
            memcpy (KR_Fmt.Data,		"0000",			4);
			ItoAf (INT_SEQ,	&KR_Fmt.Data[4],			11);
            MsgLen  = strlen(KR_Fmt.Data);
			ItoAf (MsgLen,	KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		default:
			break;
	}

	return (rt);
}	/* End of Make_Send_Msg ()	*/

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
	memcpy (DataBuff, &KR_Fmt, SendLen);
	Device_Write ();
    Log (USR_OK, "send LOGOUT request");

	rval = Device_Read ();
	if (rval < 0)
    {
        Log (USR_ERROR, "LOGOUT response recv error");
        close (Sockfd);
        return;
    }

   	if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)
	{
        Log (USR_OK, "LOGOUT success");

		Device_Close ();
        TCP2_NET_STA(S_K) = END;
	}
   	else 
	{
		ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], 4); 
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
	char    t_time[12];

	memset (t_time, 0, sizeof (t_time));

	switch (ErrCd)
	{
		case	0:	/* 정상	*/
			break;
		case	1:	/* 사용자검증(ID,PASSWORD)오류 */
			Log (USR_ERROR, "사용자검증(ID,PASSWORD)오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	2:	/* 세션메시지전문수순오류 */
			Log (USR_ERROR, "세션메시지전문수순오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	3:	/* 헤더회원번호오류 */
			Log (USR_ERROR, "헤더회원번호오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	4:	/* 데이터일련번호오류 */
			Log (USR_ERROR, "데이터일련번호오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	5:	/* 메시지길이오류 */
			Log (USR_ERROR, "메시지길이오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	6:	/* 업무기마감오류 */
			Log (USR_ERROR, "업무기마감오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	9:	/* 시스템오류 */
			Log (USR_ERROR, "시스템오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case  101:	/* 호가접수개시전 */
			Log (USR_ERROR, "호가접수개시전[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			memset (t_time, 0, sizeof (t_time));
			Get_Time (t_time);

			if (memcmp (t_time, NO_TIME, 4) < 0)		/* hhmm	*/
				sleep (10);
			sleep (2);
			break;
		case  102:	/* 매매거래시간종료후 */
			Log (USR_ERROR, "매매거래시간종료후[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			sleep (2);
			break;
		case  103:	/* 호가접수일시중지 */
			Log (USR_ERROR, "호가접수일시중지[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			FirstSeq --;
			sleep (2);
			break;
		default:
			Log (USR_ERROR,
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
    Function        : . Line_Change
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . change lines
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Line_Change (void)
/*----------------------------------------------------------------------*/
{
    TCP2_LINE_GU = TCP2_LINE_GU + 1;
    S_K = TCP2_LINE_GU % 2;
    TCP2_LINE_GU = S_K;
    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log (TCP_OK, "line changed to %s (%s:%d)",
        S_K == 0 ? "main" : "backup", IpAddr, PORT_NO);

    return;
}   /* End of Line_Change ()    */

/*************************************************************************
    Function        : .  Handshake
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int
    Comment         : .
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Handshake ()
/*----------------------------------------------------------------------*/
{
    int         result;
    int         hl;

    /* 202509 암호화 초기화 함수 호출 */
    result = INL_Initialize(CLIENT_CTX, KRX_INITECH_CONF_PATH, NULL);
    if (result != 0)
    {
        Log (TCP_ERROR, "INL_Initialize Failed. [%d:%s]", result, INL_Error_String(result));
        Free_All(1);
        return(-1);
    }

    result = INL_New_Ctx(CLIENT_CTX, &EnCtx);
    if (result != 0)
    {
        Log (TCP_ERROR, "INL_CtxNew Failed. [%d:%s]", result, INL_Error_String(result));
        Free_All(1);
        return(-1);
    }

    /* InitHandShake */
    result = INL_Handshake_Init(EnCtx, NULL, 0,  &cinitout, &cinitoutl);
    if (result != 0)
    {
        Log (TCP_ERROR, "Client Init HandShake Failed. [%d:%s]", result, INL_Error_String(result));
        Free_All(1);
        return(-1);
    }

	/* client init msg send */
    result = Make_Send_Msg (TR_HSI);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, &KR_Fmt, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake init request");

    /* server init msg recv */
    result = Device_Read ();
    if (result < 0)
    {
        Log (TCP_ERROR, "No Handshake init response from Server[%d]", result);
        Free_All(1);
        return(-1);
    }

    hl = RecvLen - sizeof(KRX_HEADER) - 4;
    if (memcmp(&DataBuff[8+6], "SCHLIQ00102", 11) != 0 ||
        memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4) != 0)
    {
        Log (TCP_ERROR, "Handshake init response error[%.11s:%.4s]",
            &DataBuff[8+6], &DataBuff[sizeof(KRX_HEADER)]);
        Free_All(1);
        return(-1);
    }

    sinitout = (unsigned char*)malloc(hl+1);
    if(!sinitout)
    {
        Log (TCP_ERROR, "sinitout malloc fail size = [%d]", hl);
        Free_All(1);
        return(-1);
    }

    memset(sinitout, '\0', hl+1);
    memcpy(sinitout, &DataBuff[sizeof(KRX_HEADER) + 4], hl);

    /* UpdateHandShake */
    result = INL_Handshake_Update(EnCtx, sinitout, hl, &cupdateout, &cupdateoutl);
	if (result != 0)
    {
        Log (TCP_ERROR, "Client Update HandShake Failed. [%d:%s]", result, INL_Error_String(result));
        Free_All(1);
        return(-1);
    }

    result = Make_Send_Msg (TR_HSU);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, &KR_Fmt, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake update request");

    result = Device_Read ();
    if (result < 0)
    {
        Log (TCP_ERROR, "No Handshake update  response from Server[%d]", result);
        Free_All(1);
        return(-1);
    }

    hl = RecvLen - sizeof(KRX_HEADER) - 4;
    if (memcmp(&DataBuff[8+6], "SCHLIQ00104", 11) != 0 ||
        memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4) != 0)
    {
        Log (TCP_ERROR, "Handshake update response error[%.11s:%.4s]",
            &DataBuff[8+6], &DataBuff[sizeof(KRX_HEADER)]);
        Free_All(1);
        return(-1);
    }

    supdateout = (unsigned char*)malloc(hl+1);
    if(!supdateout)
    {
        Log (TCP_ERROR, "supdateout malloc fail size = [%d]", hl);
        Free_All(1);
        return(-1);
    }

	memset(supdateout, '\0', hl+1);
    memcpy(supdateout, &DataBuff[sizeof(KRX_HEADER) + 4], hl);

    /* FinalHandShake */
    result= INL_Handshake_Final(EnCtx, supdateout, hl, &cfinalout, &cfinaloutl);
    if (result != 0)
    {
        Log (TCP_ERROR, "Client Final HandShake Failed. [%d:%s]", result, INL_Error_String(result));
        Free_All(1);
        return(-1);
    }

    result = Make_Send_Msg (TR_HSF);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, &KR_Fmt, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake final request");

    Free_All(0);

    return(0);
}

/*************************************************************************
	End of Program (pa_7800_tr.c)
*************************************************************************/
