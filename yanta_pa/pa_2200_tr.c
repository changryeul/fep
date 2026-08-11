#define		_GLOBAL
	
/*------------------------------------------------------------------------
#	System	: Connect to KRX
#	Author	: PSH 
#	Module	: 주문수신
#	File	: pa_2200_tr.c
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

/* ********************************************** */
/*
	TTRMIP31301 : 공개장운영,				 91,	O
	TTRMIP32301 : 주식종목정보 공개,		146
	TTRMIP32303 : 회원 제재/해제 공개,		 63
	TTRMIP32304 : 결제적용기준환율 공개,	 53
	TTRMIP31302 : 기준가정보,				 71
	TTRMIP31303 : 임의종료,					131
	TTRMIP31304 : 종목마감,					105
	TTRMIP31306 : 배분정보,					 49
	TTRMIP31307 : VI(변동성완화장치),		117
	TTRMIP31308 : 실시간가격제한,			 70
	TTRMIP31309 : 가격제한폭확대발동,		 84
*/
/* ********************************************** */

//#if defined A2201
#define		DATA_SIZE		450			  /* Header(100) + Data(318) */
//#elif defined A2202
//#define		DATA_SIZE		350			  /* Header(100) + Data(233) */
//#endif
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	35

#define		DEVICE_TIME		3  * 1000						/*  3 sec	*/
#define		MAIN_TIME		50 * 1000		/* Heart Beat 간격 25 sec	*/
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
#define		IN_NAME			IDN(D_K,P_K,0)
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
//KRX_NOTE_ALL_JUMUN_Q_FMT	J_Q_Fmt;			// KRX 주문포맷(max 6) (82+300)
//KRX_NOTE_JUMUN_R_FMT		J_R_Fmt;			// KRX 세션 응답 포맷 (82+4+11)
//KRX_NOTE_JUMUN_S_FMT		Re_W_Fmt;			// 내부 주문응답 송신
//KRX_HEADER					Header_Fmt;			// KRX Header (82)
//KRX_R_SESSION_FMT			KR_Fmt;				// KRX 세션 포맷 (82+116)
FILE_DATA_HEAD				File_Data_Head;		// 20
struct pollfd		Poll[2];

YSK_TCP_MESSAGE				M_Fmt;
YSK_TCP_HEAD				*H_Fmt = &M_Fmt.Head;
#if defined A2201 || A2202 || A3201 || A3202
YSK_SETTLE_RESP_FMT			*R_Fmt;
#elif defined A2203 || A2204 || A3203 || A3204
YSK_SETTLE_FMT				*R_Fmt;
#endif


/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_2200_TR (void);
void	Init_Parameters (void);
void	Fifo_Event_Rtn (void);
void	Socket_Event_Rtn (void);
void	Device_Open (int);
void	Device_Close (void);
void	Device_Write (void);
int		Device_Read (void);
#if	0
void    Line_Change (void);
#endif
void	Time_Out_Rtn (void);
int		Analyze_Data (void);
void 	Write_Data (int);
int		Make_Send_Msg (int);
void    Get_Msec (double *);
void	Log_Out (void);
void	Fep_Err_Msg (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_2200_TR ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_2200_TR (void)
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
#if	0
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
}	/* End of PA_2200_TR ()	*/

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

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;
	Log (USR_OK, "TIME_OUT[%d]", TIME_OUT);

    S_K = 0;

    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log (TCP_OK, "IP[%s] port[%d]", IpAddr, PORT_NO);

    TCP2_PROC_ST = ON;

    TCP2_LINE_ST = OFF;

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
	int		ii, datacnt;
	int		body_len;
	char	t_time[12];

	rval = Device_Read ();

	if (rval < 0)
		return;
// RecvLen

	DeviceSendFlag = OFF;
	ErrCd = 0;

    memset (&M_Fmt, 0, YSK_TCP_MSG_LEN);
    memcpy (&M_Fmt, DataBuff, RecvLen);

	ReTrCode = Analyze_Data ();

	datacnt = AtoIf (H_Fmt->DataCnt, sizeof (H_Fmt->DataCnt));

	/* ************************************ */
	/* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
	/* ************************************ */
	switch (ReTrCode)
	{
		case	RP_DATA:
			ErrCd = AtoIf (H_Fmt->ResponseCode, 2);
			if (ErrCd != 0) Fep_Err_Msg();

			/* Seq Check */
			/* 모든 Data는 +1로 수신받는다, 아니면 로그찍고 종료 */
			if (INT_SEQ+1 != FirstSeq)
			{
				TCP2_LINE_ST = OpenFlag = OFF;

				Log (USR_ERROR, "RP_DATA recv:Header invalid YSK SEQ [%d] INT_SEQ[%d]",
					FirstSeq, INT_SEQ);

				sleep (3);							// 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

#if defined A2201 || A2202 || A3201 || A3202
			R_Fmt = (YSK_SETTLE_RESP_FMT *)M_Fmt.Data;
#elif defined A2203 || A2204 || A3203 || A3204
			R_Fmt = (YSK_SETTLE_FMT *)M_Fmt.Data;
#endif
			
			for (ii = 0; ii < datacnt; ii++)
			{
				/* TR 구분 */
#if defined A2201 || A2202 || A3201 || A3202
				if ((memcmp(R_Fmt->Header.sRpCode, "0000", sizeof(R_Fmt->Header.sRpCode)) != 0)
				 || (memcmp(R_Fmt->SettleRespData.TrCode,	"TTRODP113",		9) == 0))	// 100+318
				{
					Write_Data (0);			// pa_2201_mp
				}
#elif defined A2203 || A2204 || A3203 || A3204
				if (memcmp(R_Fmt->SettleData.TrCode,	"TTRODP113",		9) == 0)	// 100+318
				{
					Write_Data (0);			// pa_2201_mp
				}
				else if (memcmp(R_Fmt->SettleData.TrCode,		"TTRTDP21301",		11) == 0)	// 체결결과 100+233
				{
					Write_Data (1);			// pa_2401_mp
				}
#elif defined A1601

/* ************************************************************************ 
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

 ************************************************************************ */


				/* TR 구분 */
				/* 수신받은 모든 데이터는 업무계로 흘린다. 202510 요청에 따름 */
				if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRMIP3",	   7) == 0)				// 공개장운영, 91
				{
					Write_Data (1);			// pa_1601_mp
				}
				else
				if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TCHEDP99001", 11) == 0	)			// 체결 인터페이스 종료 153
				{
					Write_Data (1);			// pa_1601_mp
					Log (USR_OK, "장운영 인터페이스 종료");

					TCP2_NET_STA(MAIN) = END;
					TCP2_NET_STA(BACKUP) = END;

					if (TCP2_LINE_ST == ON || OpenFlag == ON)
						Device_Close ();

					Log (USR_OK, "poll timeout <%d>", INT_SEQ);
					sleep (60);
					break;
				}
				else
				{
					INT_SEQ++;
					Log (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
									&DataBuff[KRX_HEAD_LEN+11], &DataBuff[KRX_HEAD_LEN], RecvLen);
					break;
				}
/*
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
				Log (USR_OK, "장운영 인터페이스 종료");

				TCP2_NET_STA(MAIN) = END;
				TCP2_NET_STA(BACKUP) = END;

				if (TCP2_LINE_ST == ON || OpenFlag == ON)
					Device_Close ();

				Log (USR_OK, "poll timeout <%d>", INT_SEQ);
				sleep (60);
				break;
			}
			else
			{
				INT_SEQ++;
				Log (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
								&DataBuff[KRX_HEAD_LEN+11], &DataBuff[KRX_HEAD_LEN], RecvLen);
				break;
			}
*/
#endif
				else
				{
#if defined A2201 || A2202 || A3201 || A3202
					Log (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
									R_Fmt->SettleRespData.TrCode, (char *)R_Fmt, AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
#elif defined A2203 || A2204 || A3203 || A3204
					Log (USR_WARN, "PR_DATA recv Not Define DATA Skip [%11.11s], [%s][%d]",
									R_Fmt->SettleData.TrCode, (char *)R_Fmt, AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
#endif
				}

#if defined A2201 || A2202 || A3201 || A3202
				R_Fmt = (YSK_SETTLE_RESP_FMT *)((char *)R_Fmt + AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
#elif defined A2203 || A2204 || A3203 || A3204
				R_Fmt = (YSK_SETTLE_FMT *)((char *)R_Fmt + AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
#endif

				INT_SEQ += 1;
			}

			break;
		case	RP_POLL:
			Make_Send_Msg (RP_POLL);
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &M_Fmt, SendLen);
			Device_Write ();
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
	YSK_TCP_HEAD	*rhfmt;
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

		LogOnFlag = ON;

		Poll[1].fd = Sockfd;
		Poll[1].events = POLLIN;
		Log (TCP_OK, "TCP Connect & LOGON OK");
	}
	else if (tr_code == TR_LINK)
	{
#if	0
		while (START_S != END)
#endif
		{
			rt = Make_Send_Msg (TR_LINK);
			memset (DataBuff, 0, sizeof (DataBuff));
			memcpy (DataBuff, &M_Fmt, SendLen);
			Device_Write ();
			Log (USR_OK, "send LINK request");

			rhfmt = (YSK_TCP_HEAD *)DataBuff;
			rval = Device_Read ();
			if (rval < 0
			 || (memcmp (rhfmt->TrCode, "LIOK", sizeof(rhfmt->TrCode)) != 0
			  && memcmp (rhfmt->TrCode, "REOK", sizeof(rhfmt->TrCode)) != 0))
			{
				Log (USR_ERROR, "LINK response recv error");
				close (Sockfd);
//				break;
				return;
			}

			memset (H_Fmt, 0, YSK_TCP_HEAD_LEN);
			memcpy (H_Fmt, DataBuff, RecvLen);

			FirstSeq = AtoIf (H_Fmt->SeqNo, sizeof (H_Fmt->SeqNo));

			Log (USR_OK, "업무개시응답(LIOK/REOK): [%2.2s] <%d:%d:%d>",
				H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT);

			if (memcmp(H_Fmt->ResponseCode, "00", 2) == 0)
			{
				/* Seq 체크	 */
				if (INT_SEQ != FirstSeq)
				{
					TCP2_LINE_ST = OpenFlag = OFF;

					Log (USR_ERROR, "PR_LINK recv:invalid YSK SEQ [%d] INT_SEQ[%d]",
						FirstSeq, INT_SEQ);

					sleep (3);							// 3초후 종료
					Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
				}

				TCP2_NET_STA(S_K) = ON;
				TCP2_LINE_ST = OpenFlag = ON;
				ConnectRetryCnt = 0;
				IF_MEG_SEQ(D_K,P_K,S_K) = 1;					/* LINK 연결 성공 */

				Log (TCP_OK, "LINK OK");

//				break;
				return;
			}
			else
			{
				ErrCd = AtoIf (H_Fmt->ResponseCode, 2);
				Fep_Err_Msg ();

                if (ErrCd == 0)                     // 호가접수 개시전
                {
                    Log (USR_OK, "호가접수전 개시요청 3초후 Retry!!!");
                    sleep (3);                      // 3초 쉬었다가 재개시 시도

//                  break;
					return;
				}

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
    Log (TCP_OK, "TCP device close");

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
	YSK_TCP_HEAD	*h_fmt = (YSK_TCP_HEAD *)DataBuff;
	int		rt;

//    rt = Select_Send (Sockfd, DataBuff, strlen (DataBuff));
    rt = Select_Send (Sockfd, DataBuff, SendLen);

    if (rt != OK)
    {
        Log (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

//    Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
	if (memcmp(h_fmt->TrCode, "LIVE", sizeof(h_fmt->TrCode)) != 0)      // HeartBeat
    	Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);

	Get_Msec (&SendMsec);

	/* 현물과 다른부분 */
#if 0
   	if (memcmp (J_Q_Fmt.Header.MsgType, "TCHTDP00000", 11) == 0)	/* 주문 */
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
	YSK_TCP_HEAD	*h_fmt = (YSK_TCP_HEAD *)DataBuff;
    int     rt;
    char    m_time[24];

    memset (DataBuff, 0, sizeof (DataBuff));
	RecvLen = 0;

    rt = Select_Receive_YSK (Sockfd, DataBuff);

    if (rt <= 0)
    {
        Device_Close ();
        return (NOTOK);
    }

	RecvLen = rt;
//   	Log (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, strlen (DataBuff), INT_SEQ);
	if (memcmp(h_fmt->TrCode, "LIVE", sizeof(h_fmt->TrCode)) != 0)      // HeartBeat
	{
   		Log (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);
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
		case    1:
            if (TimeOut == FOREVER_TIME)
                Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
        case    3:
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                Log (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
#if	0
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
#endif
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
	FirstSeq = AtoIf (H_Fmt->SeqNo, sizeof (H_Fmt->SeqNo));

	if (memcmp(H_Fmt->TrCode, "LIVE", sizeof(H_Fmt->TrCode)) == 0)      // HeartBeat
	{
//		Log (USR_OK, "회선시험요청[%2.2s] <%d:%d:%d>", H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_POLL);
	}
	else if (memcmp(H_Fmt->TrCode, "DATA", sizeof(H_Fmt->TrCode)) == 0)
	{
		Log (USR_OK, "DATA수신[%2.2s] <%d:%d:%d>",
			H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT);
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function		: . Write_Data
	Parameters IN	: . 
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
    memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);

	ItoAf (OFW(D_K,P_K,p_flag,0) + 1,		W_Fmt.Seq,		sizeof (W_Fmt.Seq));
	ItoAf (INT_SEQ + 1,			W_Fmt.If_Seq,	sizeof (W_Fmt.If_Seq));
	memcpy (W_Fmt.ApType,		ApType,		sizeof (W_Fmt.ApType));
	memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (W_Fmt.RecvTime1,	m_time,		sizeof (W_Fmt.RecvTime1));
	memcpy (W_Fmt.RecvTime2,	&m_time[sizeof(W_Fmt.RecvTime1)], sizeof (W_Fmt.RecvTime2));

#if	0
	ItoAf (HEAD_SIZE + DATA_SIZE, File_Data_Head.Length,
		sizeof (File_Data_Head.Length));
	ItoAf (INT_SEQ + 1, File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));
	memcpy (File_Data_Head.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (File_Data_Head.LineFlag, _Exe_Name+4, 3);
#endif

	memcpy (W_Fmt.DataHeader, &File_Data_Head,	sizeof (HEAD_SIZE));

	/* 2201 : 4+11 + 회원처리호가, 거래소에서 4+11+회원처리호가 수신 */
	/* 2401 : 체결정보 체결전문,   거래소에서 체결만 수신 */
	/* 2601/2602 : 장운영정보수신전문, 거래소에서 장운영정보만 수신 */
#if defined A2201 || A2202 || A3201 || A3202
	memcpy (W_Fmt.Data,			(char *)R_Fmt,		AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
	W_Fmt.LineFeed[0] = '\n';
#elif defined A2203 || A2204 || A3203 || A3204
	memcpy (W_Fmt.Data,			(char *)R_Fmt,		AtoIf (R_Fmt->Header.sLength, sizeof(R_Fmt->Header.sLength)) + sizeof(R_Fmt->Header.sLength));
	if (p_flag == 0)
		W_Fmt.LineFeed[0] = '\n';
	else
		W_Fmt.Data[OFS(D_K,P_K,p_flag)] = '\n';
#else
	memcpy (W_Fmt.Data,			&DataBuff[KRX_HEAD_LEN],	RecvLen - KRX_HEAD_LEN);
	W_Fmt.LineFeed[0] = '\n';
#endif

	if (p_flag == 0)
		rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
	else
		rt = F_W (TS_W2_1, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write[%s:%d]", OFN(D_K,P_K,p_flag), rt);
		Exit_Process ();
	}

    Log (USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,p_flag), OFW(D_K,P_K,p_flag,0), rt);

//	INT_SEQ += rt;

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
	SendLen = YSK_TCP_HEAD_LEN;

	switch (tr_code)
	{
		case	TR_LINK:	/* 업무개시 요청 */
			memset (H_Fmt, 0x20, YSK_TCP_HEAD_LEN);

			ItoAf (YSK_TCP_HEAD_LEN - 4,    H_Fmt->Length,          sizeof (H_Fmt->Length));
			if (S_K == 0)
			{
#if 0
				if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
					H_Fmt->Process.Target[0] = '1';
				else
					H_Fmt->Process.Target[0] = '0';
#endif
					H_Fmt->Process.Target[0] = '1';
			}
			else
				H_Fmt->Process.Target[0] = '1';
			ItoAf (PORT_NO,             H_Fmt->Process.Port,    sizeof (H_Fmt->Process.Port));
			if (IF_MEG_SEQ(D_K,P_K,S_K) == 0)
				memcpy (H_Fmt->TrCode,  "LINK",                 sizeof (H_Fmt->TrCode));
			else
				memcpy (H_Fmt->TrCode,  "RELI",                 sizeof (H_Fmt->TrCode));
			memcpy (H_Fmt->ResponseCode,"00",                   sizeof (H_Fmt->ResponseCode));
			ItoAf (INT_SEQ,             H_Fmt->SeqNo,           sizeof (H_Fmt->SeqNo));
			Get_DateMicroTime (d_time);
			memcpy (H_Fmt->Time,        d_time,                 sizeof (H_Fmt->Time));
			memcpy (H_Fmt->DataCnt,     "00",                   sizeof (H_Fmt->DataCnt));
			H_Fmt->Ack[0] = '0';

			break;
		case	RP_POLL: 	/* 회선시험 응답 */
			memset (H_Fmt, 0x20, YSK_TCP_HEAD_LEN);

			ItoAf (YSK_TCP_HEAD_LEN - 4,    H_Fmt->Length,          sizeof (H_Fmt->Length));
			if (S_K == 0)
			{
#if 0
				if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
					H_Fmt->Process.Target[0] = '1';
				else
					H_Fmt->Process.Target[0] = '0';
#endif
					H_Fmt->Process.Target[0] = '1';
			}
			else
				H_Fmt->Process.Target[0] = '1';
			ItoAf (PORT_NO,             H_Fmt->Process.Port,    sizeof (H_Fmt->Process.Port));
			memcpy (H_Fmt->TrCode,      "LIVE",                 sizeof (H_Fmt->TrCode));
			memcpy (H_Fmt->ResponseCode,"00",                   sizeof (H_Fmt->ResponseCode));
			ItoAf (INT_SEQ,             H_Fmt->SeqNo,           sizeof (H_Fmt->SeqNo));
			Get_DateMicroTime (d_time);
			memcpy (H_Fmt->Time,        d_time,                 sizeof (H_Fmt->Time));
			memcpy (H_Fmt->DataCnt,     "00",                   sizeof (H_Fmt->DataCnt));
			H_Fmt->Ack[0] = '0';

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
#if	1
	Device_Close ();
	TCP2_NET_STA(S_K) = END;
#else
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
#endif

	return;
}	/* End of Log_Out ()	*/

/*************************************************************************
	Function		: . Fep_Err_Msg
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fep_Err_Msg (void)
/*----------------------------------------------------------------------*/
{
	char    t_time[12];

	memset (t_time, 0, sizeof (t_time));

	switch (ErrCd)
	{
		case	0:	/* 정상	*/
			break;
		/* KRX Return */
		case	1:
			Log (USR_ERROR, "FEP : 전문일련번호오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	2:
			Log (USR_ERROR, "FEP : 온라인 개시 이전 접수[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	3:
			Log (USR_ERROR, "FEP : 장 종료 후 주문 접수[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		/* FEP Return   */
		case	4:
			Log (USR_ERROR, "FEP : FEP 장애(READ & WRITE)[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	5:
			Log (USR_ERROR, "FEP : 전문처리 수순 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	6:
			Log (USR_ERROR, "FEP : Ack 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	7:
			Log (USR_ERROR, "FEP : Null in Dat[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		/* FEP Return, Protocol Error   */
		case	91:
			Log (USR_ERROR, "FEP : 전문길이 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	92:
			Log (USR_ERROR, "FEP : 업무식별코드 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	93:
			Log (USR_ERROR, "FEP : TR-CODE 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	94:
			Log (USR_ERROR, "FEP : 응답코드 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	95:
			Log (USR_ERROR, "FEP : 전문일련번호 형식 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	96:
			Log (USR_ERROR, "FEP : 처리시간 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	97:
			Log (USR_ERROR, "FEP : DATA 건수 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case	98:
			Log (USR_ERROR, "FEP : 재처리 횟수 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
#if	0
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
#endif
		default:
			Log (USR_ERROR, "FEP : unknown 거부사유코드[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
	}

	if (ErrCd == 102)	/* 매매거래시간종료후 */ /* 2 ?? */
	{
		ErrCd = 0;
		Log_Out();
	}
} /* End of Fep_Err_Msg ()	*/

#if	0
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
#endif

/*************************************************************************
	End of Program (pa_2200_tr.c)
*************************************************************************/
