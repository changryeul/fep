#define		_GLOBAL
	
/*------------------------------------------------------------------------
#	System	: Connect to KRX, RDS수신(KRX자료기준 일괄송신)
#	Author	: PSH 
#	Module	: 일괄송신(RDS수신) Sync
#	File	: pa_1300_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"

/* ********************************************** */
/*
	TRDESP50101 : 채권종목정보,			   1173
	TRDESP50102 : KTS종목정보,			    350
	-------------------------------------------
	나머지는 처리하지 않는다.
	단, 
	TRDESP01901 : 가격단위규칙,				 81,	이것은 일단 처리안하는데 필요하면 하시면됨.
*/
/* ********************************************** */

#if defined A1201 || A1202
/* 주문 송신 Size 정의 */
/* 채권일반_254, 채권조성_255 */
#define		DATA_SIZE		400			  /* Header(82) + Data(318)_가변포함 */
#elif defined A1601
#define		DATA_SIZE		100			  /* Data(100)_가변포함 */
#elif defined A1602
#define		DATA_SIZE		150			  /* Data(148)_가변포함 */
#endif
#include	"buf_struct.h"


/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DEVICE_TIME		3  * 1000						/*  3 sec	*/
#define		MAIN_TIME		15 * 1000		/* Heart Beat 간격 15 sec	*/
#define		FOREVER_TIME	30 * 1000						/* 30 sec	*/

#define		FIFO_EVENT		0
#define		SOCKET_EVENT	1

#define		MAX_CNT			1
#define		RESP_GAP		1000

#define		NO_TIME			"0830"		// 채권정규시장은 09시 ~ 15시 까지, 시간외 없음

#ifdef  SAM_USE
#define		WR_CNT			W_CNT(0,0)
#define		RD_CNT			R_CNT(0,0)
#define		IN_NAME			IFN(D_K,P_K,0)
#else
#define		WR_CNT			IDW_CNT(0,0)
#define		RD_CNT			IDR_CNT(0,0)
#define  IN_NAME   IDN(D_K,P_K,0)
#endif

#define     PORT_NO         TCP2_PORT_NO

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		ConnectRetryCnt;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int		back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag;
char	DataBuff[4096], IpAddr[20], ApType[10];

//FILE_BUFF_FORMAT			R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];
FILE_BUFF_FORMAT			W_Fmt;
//KRX_JUMUN_Q_FMT		J_Q_Fmt;			// KRX 주문포맷(max 6) (82+294)
KRX_NOTE_ALL_JUMUN_Q_FMT	J_Q_Fmt;			// KRX 주문포맷(max 6) (82+300)
KRX_NOTE_JUMUN_R_FMT		J_R_Fmt;			// KRX 세션 응답 포맷 (82+4+11)
KRX_NOTE_JUMUN_S_FMT		Re_W_Fmt;			// 내부 주문응답 송신
KRX_HEADER					Header_Fmt;			// KRX Header (82)
KRX_R_SESSION_FMT			KR_Fmt;				// KRX 세션 포맷 (82+116)
FILE_DATA_HEAD				File_Data_Head;		// 20
struct pollfd		Poll[2];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_1300_TR (void);
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
void 	Write_Data (int);
int		Make_Send_Msg (int);
void    Get_Msec (double *);
void	Log_Out (void);
void	Err_Msg (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_1300_TR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_1300_TR (void)
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
				ConnectRetryCnt ++;
				if (ConnectRetryCnt >= 3)
				{
					Line_Change ();
					ConnectRetryCnt = 0;
				}

				PollCnt = 1;
				TimeOut = DEVICE_TIME;			// 3초
			}
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
					TimeOut = MAIN_TIME;		// 15초
				}
			}
		}
		else									// 개시가 된 경우
		{
			PollCnt = 2;
			TimeOut = MAIN_TIME;				// 5초
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
            default:
                SLog (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
                Exit_Process ();
                break;
        }
    }

    Device_Close ();

	return;
}	/* End of PA_1300_TR ()	*/

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
	ConnectRetryCnt = 0;

    S_K = TCP2_LINE_GU;

    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    SLog (TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

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
    Poll[2].fd = INPUT_FD;
    Poll[2].events = POLLIN;

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

// RecvLen
	if (rval < 0)
		return;

	DeviceSendFlag = OFF;
	ErrCd = 0;

	memset (&Header_Fmt, 0, KRX_HEAD_LEN);
	memcpy (&Header_Fmt, DataBuff, KRX_HEAD_LEN);

	ReTrCode = Analyze_Data ();

	/* ************************************ */
	/* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
	/* ************************************ */
	switch (ReTrCode)
	{
		case	RP_DATA:
			/* Seq Check */
			/* 모든 Data는 +1로 수신받는다, 아니면 로그찍고 종료 */
			if (INT_SEQ+1 != FirstSeq)
			{
				TCP2_LINE_ST = OpenFlag = OFF;

				SLog (USR_ERROR, "PR_DATA recv:Header invalid KRX SEQ [%d] INT_SEQ[%d]",
					FirstSeq, INT_SEQ);

				sleep (3);							// 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

			/* ME그룹 Seq 체크 및 저장 */
			/* 대량은 01로 주고등등 안쓰는 데이터도 우선 받아서 seq처리는 한다 */
			r_meg_no = AtoIf (&DataBuff[KRX_HEAD_LEN+11+11],	 2);				// ME그룹번호
			r_meg_seq = AtoIf (&DataBuff[KRX_HEAD_LEN],			11);				// ME그룹번호별 일련번호

			if (INT_MEG_SEQ(r_meg_no-1) + 1 == r_meg_seq)
				INT_MEG_SEQ(r_meg_no-1) = r_meg_seq;
			else
			{
				TCP2_LINE_ST = OpenFlag = OFF;

				SLog (USR_ERROR, "PR_DATA recv:Data invalid r_meg_no [%d], r_meg_seq[%d] !=  INT_MEG_SEQ[%d]",
						r_meg_no, r_meg_seq, INT_MEG_SEQ(r_meg_no-1));

				sleep (3);							// 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

#if defined A1201 || A1202
			/* TR 구분 */
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRTDP42301", 11) == 0)				// 채권 체결결과(일반,조성) 317
			{
				Write_Data (2);			// pa_1401_mp
			}
			else
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRODP4130",  10) == 0	||			// 채권 일반처리호가 291 (1신규,2정정,3취소)
				memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRMOP4130",  10) == 0	||			// 채권 조성처리호가 295 (1신규,2정정,3취소)
				memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRKOP1130",  10) == 0	)			// Kill Switch 응답 161 (1정상,2거부)
			{
				Write_Data (1);			// pa_1201_mp
			}
			else
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TCHEDP99000", 11) == 0	)			// 체결 인터페이스 종료 153
			{
				SLog (USR_OK, "체결 인터페이스 종료");
				INT_SEQ++;

				TCP2_NET_STA(MAIN) = END;
				TCP2_NET_STA(BACKUP) = END;

				if (TCP2_LINE_ST == ON || OpenFlag == ON)
					Device_Close ();

				SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
				sleep (60);
				break;
			}
			else
			{
				INT_SEQ++;
				SLog (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
								&DataBuff[KRX_HEAD_LEN+11], &DataBuff[KRX_HEAD_LEN], RecvLen);
				break;
			}
#elif defined A1601 || A1602

/*
	장운영데이터는 장운영 프로세스 번호#1, #2로 분리하여 송신
	장운영#1 : 공개장운영(공개장운영, 공개정보, 인터페이스 종료)
	장운영#2 : 종목마감  (종목마감, 기준가결정, 임의종료, 인터페이스 종료)

    TTRMIP31301 : 공개장운영,                91,    O
    TTRMIP32301 : 주식종목정보 공개,        146
    TTRMIP32303 : 회원 제재/해제 공개,       63
    TTRMIP32304 : 결제적용기준환율 공개,     53
    TTRMIP31302 : 기준가정보,                71
    TTRMIP31303 : 임의종료,                 131
    TTRMIP31304 : 종목마감,                 105
    TTRMIP31306 : 배분정보,                  49
    TTRMIP31307 : VI(변동성완화장치),       117
    TTRMIP31308 : 실시간가격제한,            70
    TTRMIP31309 : 가격제한폭확대발동,        84
*/
			/* TR 구분 */
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRMIP31301", 11) == 0)				// 공개장운영, 91
			{
				Write_Data (1);			// pa_1601_mp
			}
			else																		// 나머지는 2번으로, 처리안함
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRMIP3",	   7) == 0)				// 공개장운영, 91
			{
				Write_Data (2);			// pa_1602_mp
			}
			else
			if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TCHEDP99001", 11) == 0	)			// 체결 인터페이스 종료 153
			{
				Write_Data (2);			// pa_1602_mp
				SLog (USR_OK, "장운영 인터페이스 종료");
				INT_SEQ++;

				TCP2_NET_STA(MAIN) = END;
				TCP2_NET_STA(BACKUP) = END;

				if (TCP2_LINE_ST == ON || OpenFlag == ON)
					Device_Close ();

				SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
				sleep (60);
				break;
			}
			else
			{
				INT_SEQ++;
				SLog (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
								&DataBuff[KRX_HEAD_LEN+11], &DataBuff[KRX_HEAD_LEN], RecvLen);
				break;
			}
#endif

			break;
		case	RP_POLL:
		case	RP_STOP:
			break;
		default:
			break;
	}

	return;
}	/* End of Socket_Event_Rtn ()	*/

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
		memcpy (DataBuff, &KR_Fmt, SendLen);
		Device_Write ();
		SLog (USR_OK, "send LOGON request");

		rval = Device_Read ();
		if (rval < 0 ||	
			memcmp(&DataBuff[8+6], "SCHLIR00000", 11) != 0)
		{
			SLog (USR_ERROR, "LOGON response recv error");
			close (Sockfd);
			return;
		}

		if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)
		{
			SLog (USR_OK, "LOGON success");
		}
		else
		{
		   ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], 4);
		   Err_Msg ();

		   close (Sockfd);
		   return;
		}

		LogOnFlag = ON;

		Poll[1].fd = Sockfd;
		Poll[1].events = POLLIN;
		SLog (TCP_OK, "TCP Connect & LOGON OK");
	}
	else if (tr_code == TR_LINK)
	{
		while (START_S != END)
		{
			rt = Make_Send_Msg (TR_LINK);
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &KR_Fmt, SendLen);
			Device_Write ();
			SLog (USR_OK, "send LINK request");

			rval = Device_Read ();
			if (rval < 0 ||
				memcmp(&DataBuff[8+6], "SCHOPR00000", 11) != 0)
			{
				SLog (USR_ERROR, "LINK response recv error");
				close (Sockfd);
				break;
			}

			memset (&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));
			memcpy (&KR_Fmt, DataBuff, RecvLen);

			FirstSeq = AtoIf (KR_Fmt.Header.MsgSeqNum, sizeof (KR_Fmt.Header.MsgSeqNum));

			if (memcmp(KR_Fmt.Data, "0000", 4) == 0)
			{
				/* Seq 체크	 */
				if (INT_SEQ != FirstSeq)
				{
					TCP2_LINE_ST = OpenFlag = OFF;

					SLog (USR_ERROR, "PR_LINK recv:invalid KRX SEQ [%d] INT_SEQ[%d]",
						FirstSeq, INT_SEQ);

					sleep (3);							// 3초후 종료
					Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
				}

				TCP2_NET_STA(S_K) = ON;
				TCP2_LINE_ST = OpenFlag = ON;
				ConnectRetryCnt = 0;

				SLog (TCP_OK, "LINK OK");

				break;
			}
			else
			{
				SLog (USR_ERROR, "PR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ[%d] INT_SEQ[%d>",
					KR_Fmt.Data, FirstSeq, INT_SEQ);
				SLog (USR_ERROR, "PR_LINK recv:invalid ME_G[%d:%d:%d:%d:%d:%d:%d:%d:%d:%d]",
					INT_MEG_SEQ(0), INT_MEG_SEQ(1), INT_MEG_SEQ(2),INT_MEG_SEQ(3),INT_MEG_SEQ(4),
					INT_MEG_SEQ(5), INT_MEG_SEQ(6), INT_MEG_SEQ(7),INT_MEG_SEQ(8),INT_MEG_SEQ(9));

				sleep (3);							// 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}
		}	// End Of While
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
   	SLog (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);

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
                SLog (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
        case    3:
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                SLog (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }

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
	FirstSeq = AtoIf (Header_Fmt.MsgSeqNum, sizeof (Header_Fmt.MsgSeqNum));

	if (memcmp(Header_Fmt.MsgType, "SCHHEQ00000", 11) == 0)			// HeartBeat
	{
		SLog (USR_OK, "회선시험요청(SCHHEQ00000): [%4.4s] <%d:%d:%d>",
			&DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_POLL);
	}
	else if (memcmp(Header_Fmt.MsgType, "SCHLOQ00000", 11) == 0)		// LogOut
	{
		SLog (USR_OK, "LogOut요청(SCHLOQ00000): [%4.4s] <%d:%d:%d>",
			&DataBuff[KRX_HEAD_LEN], FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_STOP);
	}
	else if (memcmp(Header_Fmt.MsgType, "TCHODR00000", 11) == 0)		// Data,(회원처리호가/체결/인터페이스종료(TCHEDP99000))
	{
		SLog (USR_OK, "DATA수신(TCHODR00000): <%d:%d> 거부사유코드[%4.4s]",
			FirstSeq, INT_SEQ, &DataBuff[KRX_HEAD_LEN]);
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function		: . Write_Data
	Parameters IN	: . p_flag	: write file
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void 	Write_Data (int p_flag)
/*----------------------------------------------------------------------*/
{
	int     rt, seq;
    char    m_time[24];

	/* ************************************ */
	/* 1201 : 82 + 4+11 + 회원처리호가전문	*/
	/* 1401 : 82 + 체결전문					*/
	/* ************************************ */
    memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
    memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);

	ItoAf (INT_SEQ + 1, W_Fmt.If_Seq,	sizeof (W_Fmt.If_Seq));

	memcpy (W_Fmt.ApType, ApType,		sizeof (W_Fmt.ApType));
	memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (W_Fmt.RecvTime1, m_time,	sizeof (W_Fmt.RecvTime1));
	memcpy (W_Fmt.RecvTime2,	&m_time[sizeof(W_Fmt.RecvTime1)],
		sizeof (W_Fmt.RecvTime2));

	ItoAf (HEAD_SIZE + DATA_SIZE, File_Data_Head.Length,
		sizeof (File_Data_Head.Length));
	ItoAf (INT_SEQ + 1, File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));
	memcpy (File_Data_Head.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (File_Data_Head.LineFlag, _Exe_Name+4, 3);

	memcpy (W_Fmt.DataHeader, &File_Data_Head,	sizeof (HEAD_SIZE));

#if defined A1201 || A1202
	if (p_flag == 1)
	{
		/* 1201 : 82 + 4+11 + 회원처리호가 */
		/* 82 KRX_HEAD_LEN	*/
		memcpy (W_Fmt.Data,						DataBuff,					KRX_HEAD_LEN);
		/* 4+11 (0) */
		memcpy (&W_Fmt.Data[KRX_HEAD_LEN],		"000000000000000",			15);
		/* 회원처리호가 */
		memcpy (&W_Fmt.Data[KRX_HEAD_LEN+15],	&DataBuff[KRX_HEAD_LEN],	RecvLen - KRX_HEAD_LEN);
	}
	else
	{
		/* 1401 : 체결정보 82+체결전문 */
		memcpy (W_Fmt.Data,						DataBuff,					RecvLen);
	}
#elif defined A1601 || A1602
	/* 1601/1602 : 82 + 장운영정보수신전문 */
	memcpy (W_Fmt.Data,							DataBuff,					RecvLen);
#endif
	W_Fmt.LineFeed[0] = '\n';

	rt = F_W (p_flag * 10, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		SLog (SAM_FATAL, "file write[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	if (rt != 1)
    {
        Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
        return;
    }

    Log (USR_OK, "file write[%s:%d:%d]",
        OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

	INT_SEQ += rt;

	SLog (USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
	Set_TR_Time ();

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
	memset (&KR_Fmt, 0, sizeof (KRX_R_SESSION_FMT));

	memcpy (KR_Fmt.Header.BeginString,	"KMAPv2.0",	8);
	ItoAf (0, KR_Fmt.Header.BodyLength,	sizeof (KR_Fmt.Header.BodyLength));
	ItoAf (0, KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
	memcpy (KR_Fmt.Header.SenderCompID,	TCP_COMPANY, strlen (TCP_COMPANY));
	Get_DateMilliTime (d_time);
	memcpy (KR_Fmt.Header.SendingTime,	d_time,	sizeof (KR_Fmt.Header.SendingTime));
	ItoAf (0, KR_Fmt.Header.DataCnt,	sizeof (KR_Fmt.Header.DataCnt));
	memcpy (KR_Fmt.Header.Encrypt,		"N",		1);		// 초기는 N, 주문만 Y
	SendLen = KRX_HEAD_LEN;

	switch (tr_code)
	{
		case	TR_LOON:	/* 로그온 요청 */
			memcpy (KR_Fmt.Header.MsgType,	"SCHLIQ00000",		11);
			memcpy (KR_Fmt.Data,			LOGON_ID(D_K,P_K),	10);
			memcpy (&KR_Fmt.Data[10],		LOGON_PW(D_K,P_K),	30);
			memcpy (&KR_Fmt.Data[40],		"Y", 				 1);
			MsgLen	= strlen(KR_Fmt.Data);
			ItoAf (MsgLen, KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	TR_LINK:	/* 업무개시 요청 */
			memcpy (KR_Fmt.Header.MsgType, 	"SCHOPQ10000",		11);
			ItoAf (INT_SEQ, KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));

			/* DATA Set */
			/* 거부사유코드 */
			memcpy (KR_Fmt.Data, "0000", 4);

			if (INT_SEQ == 0)	// 최초시 모두 0 
			{
				memset(&KR_Fmt.Data[4], '0', 2+110);
			}
			else
			{
				ItoAf (INT_MEG_SEQ(0), &KR_Fmt.Data[4+2], 11);
				ItoAf (INT_MEG_SEQ(1), &KR_Fmt.Data[4+2+(1*11)], 11);
				ItoAf (INT_MEG_SEQ(2), &KR_Fmt.Data[4+2+(2*11)], 11);
				ItoAf (INT_MEG_SEQ(3), &KR_Fmt.Data[4+2+(3*11)], 11);
				ItoAf (INT_MEG_SEQ(4), &KR_Fmt.Data[4+2+(4*11)], 11);
				ItoAf (INT_MEG_SEQ(5), &KR_Fmt.Data[4+2+(5*11)], 11);
				ItoAf (INT_MEG_SEQ(6), &KR_Fmt.Data[4+2+(6*11)], 11);
				ItoAf (INT_MEG_SEQ(7), &KR_Fmt.Data[4+2+(7*11)], 11);
				ItoAf (INT_MEG_SEQ(8), &KR_Fmt.Data[4+2+(8*11)], 11);
				ItoAf (INT_MEG_SEQ(9), &KR_Fmt.Data[4+2+(9*11)], 11);
			}

			MsgLen	= strlen(KR_Fmt.Data);				
			ItoAf (MsgLen, KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	RP_POLL: 	/* 회선시험 응답 */
			memcpy (KR_Fmt.Header.MsgType, 		"SCHHER00000", 	11);
			ItoAf (INT_SEQ,	KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
			memcpy (KR_Fmt.Data, "0000", 4);
			MsgLen	= strlen(KR_Fmt.Data);				
			ItoAf (MsgLen,	KR_Fmt.Header.BodyLength,	sizeof(KR_Fmt.Header.BodyLength));
			SendLen += MsgLen;
			break;
		case	RP_LOOU:	/* 로그아웃 응답 */
			memcpy (KR_Fmt.Header.MsgType, 		"SCHLOR00000", 	11);
			ItoAf (INT_SEQ,	KR_Fmt.Header.MsgSeqNum,	sizeof (KR_Fmt.Header.MsgSeqNum));
			MsgLen	= strlen(KR_Fmt.Data);				
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
    SLog (USR_OK, "send LOGOUT request");

	rval = Device_Read ();
	if (rval < 0)
    {
        SLog (USR_ERROR, "LOGOUT response recv error");
        close (Sockfd);
        return;
    }

   	if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)
	{
        SLog (USR_OK, "LOGOUT success");

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
	End of Program (pa_1300_tr.c)
*************************************************************************/
