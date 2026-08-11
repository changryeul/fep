#define		_GLOBAL
	
/*------------------------------------------------------------------------
#	System	: Connect to KRX
#	Author	: PSH 
#	Module	: 주문송신
#	File	: pa_2100_ts.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"

/* 주문 송신 Size 정의 */
#define		DATA_SIZE		450			/* data Header(100) + Data(294) */
										/* 주문응답(318)과 같이 사용	*/
#include	"buf_struct.h"

#define		MAX_LOAD_CNT	(SHM_DATA_SIZE / sizeof(DATA_FORMAT))
/* 55바이트 업무헤더 추가시 */
#define     ADD_HEADER_SIZE	YSK_DATA_HEAD_LEN

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	25

#define		DEVICE_TIME		3 * 1000						/*  3 sec	*/
#define		MAIN_TIME		5 * 1000		/* Heart Beat 간격 5 sec	*/
#define		DATA_TIME		15 * 1000		/* Heart Beat 3회  15 sec	*/
#define		FOREVER_TIME	60 * 1000						/* 60 sec	*/
#define		LIVE_TIME		25 * 1000		/* FEP LIVE timeout 25 sec	*/

#define		FIFO_EVENT		0
#define		DATA_EVENT		1
#define		SOCKET_EVENT	2

#define		MAX_CNT			1			/* 파생의 경우 최대 8 */
#define		RESP_GAP		1000

#define		NO_TIME			"0830"		// 채권정규시장은 09시 ~ 15시 까지, 시간외 없음

#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define		IN_NAME   IDN(D_K,P_K,0)
#endif

#define		FWR_CNT			OFW_CNT(0,0)

#define     PORT_NO         TCP2_PORT_NO

/* 체결Process가 인터페이스종료(TCHEDP99000) 수신후 종료되면 주문송신도 종료 */
#if defined	A2101
#define     CHK_PROC		"pa_2201_tr"
#elif defined	A2102
#define     CHK_PROC		"pa_2202_tr"
#elif defined	A3101
#define     CHK_PROC		"pa_3201_tr"
#else		/* A3102 */
#define     CHK_PROC		"pa_3202_tr"
#endif

/*
KRX 주문 서비스 업무흐름도
0101 : 호가접수개시전, 			세션Continue,	개시retry 
0004 : 데이터 일련번호 오류, 	세션Close, 		처음부터 다시 
0090 : 시스템오류, 				세션Close,		처음부터 다시
0013 : 호가접수정지중, 			세션Close,		처음부터 다시

기타
0020 : TPS허용건수 초과,		1초간 주문sleep
0102 : 매매거래시간 종류 후 (체결회선)
COD사용안함

기타2
주문프로세스는 체결프로세스가 체겨라감을 한 후 LogOut
*/

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
time_t	LiveTimer;
int		ConnectRetryCnt;
int     Sockfd, ErrCd, SendLen, MsgLen, RecvLen, ReTrCode;
int     PollCnt, FirstSeq, TimeOut, Pk;
int		back_int_seq, back_rd_cnt;
double  RTime = 0, STime = 0;
double  SendMsec, RecvMsec, RespMsec;
char    DeviceSendFlag, LogOnFlag, OpenFlag, DataBuff[YSK_TCP_MSG_LEN], IpAddr[20];


DATA_FORMAT					*SHM_DATA;
FILE_BUFF_FORMAT			R_Fmt[MAX_CNT], W_Fmt[MAX_CNT];

YSK_TCP_MESSAGE				M_Fmt;
YSK_TCP_HEAD				*H_Fmt = &M_Fmt.Head;		// 세션 포맷 (50)
YSK_JUMUN_FMT				*J_Q_Fmt;					// YSK 주문포맷(max 6)

struct pollfd		Poll[3];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_2100_TS (void);
void	Init_Parameters (void);
void	Fifo_Event_Rtn (void);
void	Socket_Event_Rtn (void);
void	Data_Event_Rtn (void);
void	Device_Open (int);
void	Device_Close (void);
void	Device_Write (void);
int		Device_Read (void);
#if	0
void    Line_Change (void);
#endif
void	Time_Out_Rtn (void);
int		Analyze_Data (void);
void 	Write_Response_Data (int);
int		Make_Send_Msg (int);
int		Make_Data_Block (int *);
void    Get_Msec (double *);
void	Log_Out (void);
void	Fep_Err_Msg (void);
int     Chk_Risk_All (char *);
unsigned char* EncryptAndMakeSendPacket(const void*, size_t, int*);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_2100_TS ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_2100_TS (void)
/*----------------------------------------------------------------------*/
{
	int		rt, i;

	Init_Parameters ();

	while (START_S != END)
	{
		Stat_Save ();
#if	0	/* 제거 */
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

#if	1	/* HONG : 장종료후 CPU 부하 상승으로 임시 추가 검토해 보세요 */
			sleep(1);
#endif
            continue;
        }
        else                                            /* 정상주문시간 */
#endif
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
/*                 Time_Out_Rtn => Device_Write()후 ON(TR_POLL)			*/
/*                 Data_Event_Rtn => ON.                                */
/*                 Make_Send_Msg (TR_DATA) => Device_Write()후 ON.      */
/* 2025 : Data_Event_Rtn & Make_Send_Msg 싱크시에만 유효, 어싱크는 유요하지 않음 */
/* -------------------------------------------------------------------- */

			/* 개시를 해야만 주문을 낼 수 있음, 정규장 열리기 전에는 개시응답을 정상으로 안준다. 개시만 retry */
			if (OpenFlag == OFF)					// 개시가 안된경우
			{
				if (WR_CNT > RD_CNT)				/* 개시가 안된 경우에 주문 전문을 거부 처리 */
				{
					Data_Event_Rtn ();
					continue;
				}

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
                	PollCnt = 2;
                	TimeOut = DEVICE_TIME;			// 3초
				}
				else								// 로그온은 된경우
				{
					Device_Open (TR_LINK);

					if (OpenFlag == OFF)			// 아직 로그온이 안된경우
					{
						PollCnt = 2;
						TimeOut = DEVICE_TIME;		// 3초
					}
					else
					{
						PollCnt = 3;
						TimeOut = MAIN_TIME;		// 5초
					}
				}
			}
			else								// 개시가 된 경우
			{
#if	0	/* 제거 */
               	if (DeviceSendFlag == ON)			// DATA를 송신한경우, LogOut , 응답받고 종료처리하자. DeviceSendFlag은 LogOut만 ON
               	{
					PollCnt = 2;
                   	TimeOut = DATA_TIME; 			// 15초
               	}
               	else 								// DATA를 송신한경우, 주문/HeartBeat/KillSwitch , Async라 송수신 가능하게
#endif
               	{
                   	PollCnt = 3;
                   	TimeOut = MAIN_TIME;			// 5초

                   	if (WR_CNT > RD_CNT)			// HeartBeat/KillSwitch를 보냈더라도 주문이 있으면 먼저 처리하자.
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
            case    DATA_EVENT:
                Data_Event_Rtn ();
                break;
            case    SOCKET_EVENT:
                Socket_Event_Rtn ();
                break;
            default:
                Log (USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				Device_Close ();
                Exit_Process ();
                break;
        }
    }

    Device_Close ();

	return;
}	/* End of PA_2100_TS ()	*/

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
#if 1   /* HONG */
	Sise_SHM_Detach();
#endif

	LiveTimer = 0;

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

    Poll[0].fd = START_FD;
    Poll[0].events = POLLIN;
    Poll[1].fd = INPUT_FD;
    Poll[1].events = POLLIN;

    if (PROC(D_K,P_K).data)
	{
        SHM_DATA = (DATA_FORMAT *)PROC(D_K,P_K).data;
Log (USR_OK, "SHM OK");
	}
	else
Log (USR_OK, "SHM NOTOK");

	/* 체결Process01 process key 구함 */
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
	int		rval, rt, cnt, mun;
	char	t_time[12];

	rval = Device_Read ();

	if (rval < 0)
		return;

//	DeviceSendFlag = OFF;

	memset (&M_Fmt, 0, YSK_TCP_MSG_LEN);
	memcpy (&M_Fmt, DataBuff, RecvLen);

	ReTrCode = Analyze_Data ();

	/* ************************************ */
	/* 여기서는 RP_DATA, RP_POLL 2개만 체크 */
	/* ************************************ */
	switch (ReTrCode)
	{
		case	RP_DATA:	/* 주문응답은 "0020"(TPS허용수량초과)빼고, 모두 거부 */
#if	0
			if (memcmp(S_Fmt.Data, "0020", 4) == 0)			// 0020 (TPS허용수량초과)
			{
				Log (USR_OK, "TCS초과 오류 수신 1초 sleep!!!");
				sleep (1);
				break;
			}
#endif

			ErrCd = AtoIf (H_Fmt->ResponseCode, 2);
			if (ErrCd != 0) Fep_Err_Msg();

			if (ErrCd == 1)							/* 전문일련번호오류		*/
			{
				LOAD_CNT = 0;						/* load data를 clear	*/
				Device_Close ();
				sleep (3);                          // 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}

			/* Seq 처리 후 종료 */
			if (FirstSeq > INT_SEQ)					// 수신받은 Seq가 나의 INT_SEQ보다 클수없다. 오류후 Process종료.
			{
				Device_Close ();

				Log (USR_ERROR, "RP_DATA recv:invalid YSK SEQ <%d:%d>",
					FirstSeq, INT_SEQ);
 
				sleep (3);                          // 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}
//			else if (FirstSeq <= INT_SEQ)
			else									// 주문에서 DATA를 수신했다는 것은 오류가 있었다는 것이다.
			{
//				Log (USR_WARN, "RP_DATA recv:Seq Change INT_SEQ[%d] => FirstSeq[%d]", INT_SEQ, FirstSeq);

				/* DATA오류를 받으면 해당 "Seq -1"까지 처리된 것이다 */  
				/* Seq BackUp 
				back_rd_cnt	 = RD_CNT;
				back_int_seq = INT_SEQ;
				*/

#if	0
				//RD_CNT = INT_SEQ = FirstSeq -1;					// 오류 Seq -1로 변경, 그래야 Next로 오류건을 처리할 수 있음
				mun = (FirstSeq - INT_SEQ) - 1;
				INT_SEQ = INT_SEQ - mun;
				RD_CNT  = RD_CNT  - mun;
				PROC(D_K,P_K).counter_seq = FirstSeq -1;		// 오류 Seq -1로 변경, 그래야 Next로 오류건을 처리할 수 있음,DR
#endif

				/* 나머지는 오류 처리 */
				Write_Response_Data (1);

				/* Seq 원복 
				RD_CNT  = back_rd_cnt;
				INT_SEQ = back_int_seq;
				*/

				Device_Close ();					// 소켓 종료
			}

			break;
		case	RP_RJCT:
			Device_Close ();
			sleep (3);                          	// 3초후 종료
			Exit_Process ();                    	// 종료, 운영자에게 알리기 위해서

			break;
		case	RP_POLL:
			LiveTimer = 0;
			/* Seq 처리 후 종료 */
			if (DeviceSendFlag == OFF)				/* 마지막 보낸 전문이 LIVE	*/
			{
				if (FirstSeq != INT_SEQ)			/* 수신받은 Seq가 나의 INT_SEQ 와 동일 해야 함	*/
				{
					Device_Close ();

					Log (USR_ERROR, "RP_POLL recv:invalid YSK LIVE SEQ <%d:%d>",
						FirstSeq, INT_SEQ);

					sleep (3);                      // 3초후 종료
					Exit_Process ();                // 종료, 운영자에게 알리기 위해서
				}
			}

			if (FirstSeq > INT_SEQ)					// 수신받은 Seq가 나의 INT_SEQ보다 클수없다. 오류후 Process종료.
			{
				Device_Close ();

				Log (USR_ERROR, "RP_POLL recv:invalid YSK SEQ <%d:%d>",
					FirstSeq, INT_SEQ);

				sleep (3);                          // 3초후 종료
				Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
			}
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
	if (rt == 0)			// 정상
	{
		// 2025 암복호화 때문에 여기서 하지않고 Make_Send_Msg에서 DataBuff에 선행해서 처리해둔다.
		//memcpy (DataBuff, &J_Q_Fmt, SendLen);
		Device_Write ();

//		Dshm_Add_Count (PS_R_1, 1);
		INT_SEQ++;

		if (LogOnFlag == ON)
		{
			DeviceSendFlag = ON;			/* 마지막 보낸 전문이 DATA	*/
			Set_TR_Time ();
		}
	}

	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		break;
/*
		if (rt == 0)
			break;
		else if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;
			Log (USR_OK, "Poll:cannot read FIFO {%d:%s}",
			//Log (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}",
				SYS_NO, SYS_STR);
			break;
		}
*/
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
void	Device_Open (int tr_code)
/*----------------------------------------------------------------------*/
{
	YSK_TCP_HEAD	*rhfmt;
	int     rt, rval, mun;

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

		Poll[2].fd = Sockfd;
		Poll[2].events = POLLIN;
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
				/* Seq 처리 */
				if (FirstSeq != INT_SEQ)
				{
					Device_Close ();

					Log (USR_ERROR, "TR_LINK recv:invalid SEQ <%d:%d>",
						FirstSeq, INT_SEQ);

					sleep (3);							// 3초후 종료
					Exit_Process ();                    // 종료, 운영자에게 알리기 위해서
				}
#if	0
				else if (FirstSeq < INT_SEQ)
				{
					Log (USR_WARN, "TR_LINK recv:Seq Change [%d] => [%d] SEQ <%d>",
						INT_SEQ, FirstSeq, FirstSeq);

					//RD_CNT = INT_SEQ = FirstSeq;                // 수신측 Seq로 변경
					mun = FirstSeq - INT_SEQ;
					INT_SEQ = INT_SEQ - mun;
					RD_CNT  = RD_CNT  - mun;
					PROC(D_K,P_K).counter_seq = FirstSeq;       // 수신측 Seq로 변경
				}
#endif

//				LOAD_CNT = 0;
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

				if (ErrCd == 0)						// 호가접수 개시전
				{
					Log (USR_OK, "호가접수전 개시요청 3초후 Retry!!!");
					sleep (3);						// 3초 쉬었다가 재개시 시도

//					break;
					return;
				}
#if	0
				/* 다른 오류는 Process를 죽여서 운영자에게 알려야함 */
				Log (USR_ERROR, "TR_LINK recv:invalid ResponseCode[%4.4s] KRX SEQ <%d:%d>",
					S_Fmt.Data, FirstSeq, INT_SEQ);
#endif
				Device_Close ();

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
    OpenFlag = OFF;
    TCP2_PROC_ST = OFF;
    TCP2_LINE_ST = OFF;
    TCP2_NET_STA(S_K) = OFF;

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
		return;
    }

	if (memcmp(h_fmt->TrCode, "LIVE", sizeof(h_fmt->TrCode)) != 0)		// HeartBeat
    	Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);
//	Get_Msec (&SendMsec);

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
	if (memcmp(h_fmt->TrCode, "LIVE", sizeof(h_fmt->TrCode)) != 0)		// HeartBeat
   		Log (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);

#if 0
    if (DeviceSendFlag == ON && SendLen > KRX_HEAD_LEN)
    {
		Get_Msec (&RecvMsec);
        RespMsec = RecvMsec - SendMsec;

        if (DELAY_TIME != 0 && RespMsec > DELAY_TIME / 1000.0)
            Log (TCP_ERROR, "DELAYED [%.06f:%.06f]",
                RespMsec, DELAY_TIME / 1000.0);
        else
	    {
			/* async에서는 주문전문이면 주문거부만 있다. (4+11) */
   			if (memcmp (DataBuff+14, "TCHODR00000", 11) == 0)	/* 메시지타입: 주문 */
            {
                RTime = AtoIf (DataBuff+68, 2) * 60 * 60 +		/* 전송일시 : 시 */
                    AtoIf (DataBuff+70, 2) * 60 +				/* 전송일시 : 분 */ 
					AtoIf (DataBuff+72, 2) +					/* 전송일시 : 초 */
                    AtoIf (DataBuff+74, 3) / 1000.0;			/* 전송일시 : MS */

                if (RTime > RecvMsec)
                    Log (TCP_OK, "Data response [%3.3s][%.06f][+%.03f]",
                        DataBuff+77, RespMsec, RTime - RecvMsec);	/* 데이터건수 */
                else
                    Log (TCP_OK, "Data response [%3.3s][%.06f][%.03f]",
                        DataBuff+77, RespMsec, RecvMsec - RTime);	/* 데이터건수 */
            }
            else
                Log (TCP_OK, "response [%.06f]", RespMsec);
        }
    }
#endif

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
#if	0
		case	2:
			if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
				Log (TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
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
#if 0
			if (OpenFlag == ON) { Log (TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
				Device_Close ();

				ConnectRetryCnt ++;
				
				if (ConnectRetryCnt >= 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
			}
			else
			{
				Log (USR_OK, "poll timeout <%d>", INT_SEQ);
			}
#endif

			break;
#endif
		case	3:
			if (LiveTimer == 0)
			{
				DeviceSendFlag = OFF;						/* 마지막 send 전문이 LIVE 인 경우	*/
				rt = Make_Send_Msg (TR_POLL);
				memcpy (DataBuff, &M_Fmt, SendLen);
				Device_Write ();
				LiveTimer = time(NULL);
				//DeviceSendFlag = ON;
			}

			if (time(NULL) - LiveTimer >= LIVE_TIME)
			{
				Device_Close ();
				Log (USR_ERROR, "LIVE timeout");
				sleep(3);
				Exit_Process();
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
/*
    1. LogOn      SCHLIR00000 : 87  ( 5) = 82 +4+1
    2. LIOK(주문) SCHOPR00000 : 108 (26) = 82 +4+11+11
    3. LIOK(체결) SCHOPR10000 : 198 (27) = 82 +4+2+11*10
    4. HeartBeat  SCHHER00000 : 86  ( 4) = 82 +4 (송수신 동일)
    5. LogOut     SCHLOQ00000 : 86  ( 4) = 82 +4
    6. 주문거부   TCHODR00000 : 97  (15) = 82 +4+11

    KRX_SESSION_FMT             S_Fmt    : 82 + Data(41)
    KRX_NOTE_ALL_JUMUN_Q_FMT    J_Q_Fmt  : 82 + 300
    KRX_NOTE_JUMUN_R_FMT        J_R_Fmt  : 82 + 4+11
*/

	FirstSeq = AtoIf (H_Fmt->SeqNo, sizeof (H_Fmt->SeqNo));

	if (memcmp(H_Fmt->TrCode, "LIVE", sizeof(H_Fmt->TrCode)) == 0)		// HeartBeat
	{
#if	0
		Log (USR_OK, "회선시험요청[%2.2s] <%d:%d:%d> SHM_DATA[0].seq:<%d>",
			H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT, SHM_DATA[0].seq);
#endif
		return (RP_POLL);
	}
	else if (memcmp(H_Fmt->TrCode, "DAOK", sizeof(H_Fmt->TrCode)) == 0)
	{
		Log (USR_OK, "주문응답[%2.2s] <%d:%d:%d> SHM_DATA[0].seq:<%d>",
			H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT, SHM_DATA[0].seq);
		return (RP_DATA);
	}
	else if (memcmp(H_Fmt->TrCode, "RJCT", sizeof(H_Fmt->TrCode)) == 0)
	{
		Log (USR_OK, "주문거부[%2.2s] <%d:%d:%d> SHM_DATA[0].seq:<%d>",
			H_Fmt->ResponseCode, FirstSeq, INT_SEQ, LOAD_CNT, SHM_DATA[0].seq);
		return (RP_RJCT);
	}
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function		: . Write_Respose_Data
	Parameters IN	: . d_cnt	: write count
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . 주문거부에 대한 값 돌려주기, file write
*************************************************************************/
/*----------------------------------------------------------------------*/
void 	Write_Response_Data (int d_cnt)
/*----------------------------------------------------------------------*/
{
	YSK_DATA_HEAD	*Header;
	int		i, jj, rt, cnt, err_seqno, idx;;
	int		find_err;
    char    resp_time[12];

	if (d_cnt <= 0)
		return;
	
#if	1
	for (i = 0; i < d_cnt; i ++)
	{
		err_seqno = AtoIf (H_Fmt->SeqNo, sizeof (H_Fmt->SeqNo));
		find_err = 0;

		/* 리턴받은 Error Data를 찾는다. 위에서 Seq는 처리해다.*/ 
		/* 오류코드만 받기때문에 주문전문을 알수없다. 그래서 seq로 찾아야 한다. */
		/* d_cnt 가 1인 경우만 아래가 성립됨 */
		if (LOAD_CNT >= MAX_LOAD_CNT)
		{
			for (idx = (LOAD_CNT - 1) % MAX_LOAD_CNT; idx >= 0; idx--)
			{
				if (SHM_DATA[idx].seq == err_seqno)
				{
					find_err = 1;
					break;
				}
			}

			if (find_err == 0 &&  LOAD_CNT % MAX_LOAD_CNT != 0 )
			{
				for (idx = MAX_LOAD_CNT - 1; idx > ((LOAD_CNT - 1) % MAX_LOAD_CNT); idx--)
				{
					if (SHM_DATA[idx].seq == err_seqno)
					{
						find_err = 1;
						break;
					}
				}
			}
		}
		else
		{
			for (idx = LOAD_CNT - 1; idx >= 0; idx--)
			{
				if (SHM_DATA[idx].seq == err_seqno)
				{
					find_err = 1;
					break;
				}
			}
		}

		if (find_err == 0)
		{
			Log (SAM_ERROR, "DSHM_R(PS_R_1)[%s] 거부처리 못했음", IDN(D_K,P_K,0));
			return;		// 돌아가서 정리하고 죽는다.
		}

		/* Space로 초기화 */
		memset (W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
		/* 1. Seq (8) */
		ItoAf (FWR_CNT + 1,				W_Fmt[i].Seq,				sizeof (W_Fmt[0].Seq));			// Data의 Seq
		/* 2. If_Seq (8) */
		memcpy (W_Fmt[i].If_Seq,		H_Fmt->SeqNo,				sizeof (W_Fmt[0].If_Seq));		// 헤더의 Seq
		
		/* 3. ApType (8) */
		memcpy (W_Fmt[i].ApType,		R_Fmt[i].ApType,			sizeof (W_Fmt[0].ApType));
		/* 4. ResponseCode (4), 전달은 정상으로 한다.(0000), 오류코드는 위에 */
		memcpy (W_Fmt[i].ResponseCode,	RES_NORMAL,					strlen (RES_NORMAL));
        /* 5. set KRX response time   (10)
        sprintf (resp_time, "%010.06f", RespMsec);
        memcpy (W_Fmt[i].RecvTime1, resp_time, sizeof (W_Fmt[i].RecvTime1));
		*/
		/* 6. RecvTime2 (12) */
		memcpy (W_Fmt[i].RecvTime2,		&H_Fmt->Time[8],	12);
		/* 7. DataHeader(20), Async라.. 알수없다 */
//		memcpy (W_Fmt[i].DataHeader,	R_Fmt[i].DataHeader,		HEAD_SIZE);
		/* 8. DATA 조합(4+11 +300) */
#if	1
		memcpy (W_Fmt[i].Data, SHM_DATA[idx].buf.Data, YSK_DATA_HEAD_LEN + sizeof(KRX_JUMUN_DATA));
#else
		/* Write Format : 4+11 + 55(SPACE) + 주문전문 */
		/* KRX Reply (4+11) */
		memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));
		cnt = DSHM_R (PS_R_1, (void *)R_Fmt, 1);
		if (cnt < 0 || cnt > 1)
		{
			Log (SAM_ERROR, "DSHM_R(PS_R_1)[%s] 거부처리 못했음", IDN(D_K,P_K,0));
			return;		// 돌아가서 정리하고 죽는다.
		}
		else if (cnt == 0)
			return;		// 돌아가서 정리하고 죽는다.

		memcpy (W_Fmt[i].Data, R_Fmt[i].Data, YSK_DATA_HEAD_LEN + sizeof(KRX_JUMUN_DATA));
#endif
		Header = (YSK_DATA_HEAD *)W_Fmt[i].Data;
		
		memcpy (Header->sRpCode, "Y0", 2);
		memcpy (&Header->sRpCode[2], H_Fmt->ResponseCode, sizeof(H_Fmt->ResponseCode));

		W_Fmt[i].LineFeed[0] = '\n';
	}
#else
	memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

	/* 리턴받은 Error Data를 찾는다. 위에서 Seq는 처리해다.*/ 
	/* 오류코드만 받기때문에 주문전문을 알수없다. 그래서 seq로 찾아야 한다. */
	cnt = DSHM_R (PS_R_1, (void *)R_Fmt, 1);
	if (cnt < 0 || cnt > 1)
	{
		/* Seq 원복 
		RD_CNT  = back_rd_cnt;
		INT_SEQ = back_int_seq;
		*/

		Log (SAM_ERROR, "DSHM_R(PS_R_1)[%s] 거부처리 못했음", IDN(D_K,P_K,0));
		return;		// 돌아가서 정리하고 죽는다.
	}
	else if (cnt == 0)
		return;		// 돌아가서 정리하고 죽는다.

	/* 주문거부는 수신받은 전문 "4+11+ 55(찾아서) + 주문전문"을 전달한다. */
	/* KRX Header 82는 제외 */
	/* Async로 1개만 처리한다 */
	for (i = 0; i < d_cnt; i ++)
	{
		/* Space로 초기화 */
		memset (W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

#if	0
		/* 1. Seq (8) */
		memcpy (W_Fmt[i].Seq,			&S_Fmt.Data[4],				sizeof (W_Fmt[0].Seq));			// Data의 Seq
#endif
		/* 2. If_Seq (8) */
		memcpy (W_Fmt[i].If_Seq,		S_Fmt.Header.MsgSeqNum,		sizeof (W_Fmt[0].If_Seq));		// 헤더의 Seq
		/* 3. ApType (8) */
		memcpy (W_Fmt[i].ApType,		R_Fmt[i].ApType,			sizeof (W_Fmt[0].ApType));
		/* 4. ResponseCode (4), 전달은 정상으로 한다.(0000), 오류코드는 위에 */
		memcpy (W_Fmt[i].ResponseCode,	RES_NORMAL,					strlen (RES_NORMAL));
        /* 5. set KRX response time   (10)
        sprintf (resp_time, "%010.06f", RespMsec);
        memcpy (W_Fmt[i].RecvTime1, resp_time, sizeof (W_Fmt[i].RecvTime1));
		*/
		/* 6. RecvTime2 (12) */
		memcpy (W_Fmt[i].RecvTime2,		&S_Fmt.Header.SendingTime[8],	9);							// 12자리중 9자리만
		/* 7. DataHeader(20), Async라.. 알수없다 */
		memcpy (W_Fmt[i].DataHeader,	R_Fmt[i].DataHeader,		HEAD_SIZE);
		/* 8. DATA 조합(4+11 +300) */

		/* Write Format : 4+11 + 55(SPACE) + 주문전문 */
		/* KRX Reply (4+11) */
		memcpy (W_Fmt[i].Data,			S_Fmt.Data,					sizeof(KRX_NOTE_JUMUN_R_DATA));	// 4+11
		/* Read Order Data (400 include tmp) */
		memcpy (&W_Fmt[i].Data[sizeof(KRX_NOTE_JUMUN_R_DATA)],		R_Fmt[i].Data,	310);			// 400중 310(0+주문번문)만 Copy
		W_Fmt[i].LineFeed[0] = '\n';
	}
#endif

	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);		// OFN1 = pa_1201_mp
    if (rt != 1)
    {
		/* Seq 원복 
		RD_CNT  = back_rd_cnt;
		INT_SEQ = back_int_seq;
		*/

		Log (SAM_FATAL, "file write fail[%s:%d]", OFN(D_K,P_K,0), rt);
		return;
    }

//	Dshm_Add_Count (PS_R_1, i);
//	INT_SEQ += FirstSeq - INT_SEQ;

	Log (USR_OK, "Write!! Error Response Data[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), i);

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
	int     msglen;
	int		rt, datacnt;
	char	d_time[23];

	rt = 0;
	SendLen = YSK_TCP_HEAD_LEN;

	switch (tr_code)
	{
		case	TR_LINK:	/* 업무개시 요청 */
			memset (H_Fmt, 0x20, YSK_TCP_HEAD_LEN);

			ItoAf (YSK_TCP_HEAD_LEN - 4,	H_Fmt->Length,			sizeof (H_Fmt->Length));
			if (S_K == 0)
			{
#if 0
				if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
					H_Fmt->Process.Target[0] = '1';
				else
					H_Fmt->Process.Target[0] = '1';
#endif
					H_Fmt->Process.Target[0] = '1';
			}
			else
				H_Fmt->Process.Target[0] = '1';
			ItoAf (PORT_NO,				H_Fmt->Process.Port,    sizeof (H_Fmt->Process.Port));
			if (IF_MEG_SEQ(D_K,P_K,S_K) == 0 && INT_SEQ == 0)
			{
				memcpy (H_Fmt->TrCode,	"LINK",					sizeof (H_Fmt->TrCode));
				ItoAf (0,				H_Fmt->SeqNo,			sizeof (H_Fmt->SeqNo));
			}
			else
			{
				memcpy (H_Fmt->TrCode,	"RELI",					sizeof (H_Fmt->TrCode));
				ItoAf (INT_SEQ,			H_Fmt->SeqNo,			sizeof (H_Fmt->SeqNo));
			}
			memcpy (H_Fmt->ResponseCode,"00",					sizeof (H_Fmt->ResponseCode));
			Get_DateMicroTime (d_time);
			memcpy (H_Fmt->Time,		d_time,					sizeof (H_Fmt->Time));
			memcpy (H_Fmt->DataCnt,		"00",					sizeof (H_Fmt->DataCnt));
			H_Fmt->Ack[0] =	'0';

			break;
		case	TR_POLL: 	/* 회선시험 요청 */
			memset (H_Fmt, 0x20, YSK_TCP_HEAD_LEN);

			ItoAf (YSK_TCP_HEAD_LEN - 4,	H_Fmt->Length,			sizeof (H_Fmt->Length));
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
			ItoAf (PORT_NO,				H_Fmt->Process.Port,    sizeof (H_Fmt->Process.Port));
			memcpy (H_Fmt->TrCode,		"LIVE",					sizeof (H_Fmt->TrCode));
			memcpy (H_Fmt->ResponseCode,"00",					sizeof (H_Fmt->ResponseCode));
			ItoAf (INT_SEQ,				H_Fmt->SeqNo,			sizeof (H_Fmt->SeqNo));
			Get_DateMicroTime (d_time);
			memcpy (H_Fmt->Time,		d_time,					sizeof (H_Fmt->Time));
			memcpy (H_Fmt->DataCnt,		"00",					sizeof (H_Fmt->DataCnt));
			H_Fmt->Ack[0] =	'0';

			break;
		case	TR_DATA: 	/* 주문요청 + Kill Swith */
			memset (H_Fmt, 0x20, YSK_TCP_HEAD_LEN);
			J_Q_Fmt = (YSK_JUMUN_FMT *)M_Fmt.Data;
			memset (J_Q_Fmt, 0x00, YSK_TCP_MSG_LEN - YSK_TCP_HEAD_LEN);

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
			ItoAf (PORT_NO,				H_Fmt->Process.Port,    sizeof (H_Fmt->Process.Port));
			memcpy (H_Fmt->TrCode,		"DATA",					sizeof (H_Fmt->TrCode));
			memcpy (H_Fmt->ResponseCode,"00",					sizeof (H_Fmt->ResponseCode));
			ItoAf (INT_SEQ + 1,			H_Fmt->SeqNo,			sizeof (H_Fmt->SeqNo));
			Get_DateMicroTime (d_time);
			memcpy (H_Fmt->Time,		d_time,					sizeof (H_Fmt->Time));
			H_Fmt->Ack[0] =	'0';

//			ItoAf (INT_SEQ+1, J_Q_Fmt.Header.MsgSeqNum,	sizeof (J_Q_Fmt.Header.MsgSeqNum));

			msglen = Make_Data_Block (&datacnt);
/*
			if (MsgLen == -2)		// Skip(REJC)
			{
				rt = -2;
				break;
			}
			else
*/
			if (msglen <= 0)
			{
				rt = -1;
				break;
			}

			/* 2025 암복호화 때문에 DataBuff에 직접 처리한다. */
			/*
			// body size ( 주문전문길이, 헤더(82)배제 )
			ItoAf (MsgLen,	J_Q_Fmt.Header.BodyLength,	sizeof(J_Q_Fmt.Header.BodyLength));	// Make_Data_Block에서 했음
			ItoAf (datacnt,	J_Q_Fmt.Header.DataCnt,		sizeof (J_Q_Fmt.Header.DataCnt));
			*/
			SendLen += msglen;
			ItoAf (SendLen - 4,			H_Fmt->Length,			sizeof (H_Fmt->Length));
			ItoAf (datacnt,				H_Fmt->DataCnt,			sizeof (H_Fmt->DataCnt));
			memcpy (DataBuff,			&M_Fmt,					SendLen);

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
int		Make_Data_Block (int *datacnt)
/*----------------------------------------------------------------------*/
{
	YSK_DATA_HEAD		*dataheader;
	int		r_cnt, t_cnt, i, j, rt, ow_seq;
	int		totlen = 0, datalen, shm_data_idx;
	char	err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];

	*datacnt = 0;
	i = t_cnt = 0;
	memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);
	memset (W_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * MAX_CNT);

	/* 송신 파일에는 KRX Header + 주문전문으로 받는다 */
	r_cnt = DSHM_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
	if (r_cnt < 0 || r_cnt > MAX_CNT)
	{
		Device_Close ();
		Log (SAM_FATAL, "DSHM_R(PS_R_1)[%s][%d]", IDN(D_K,P_K,0), r_cnt);
		Exit_Process ();
	}
	else if (r_cnt == 0)
	{
		return (0);
	}

/* SKIP CHECK 
	if ( memcmp (&R_Fmt[0].DataHeader[10],   "REJC", 4) == 0 )
	{
		return (-2);		// SKIP
	}
*/

	/* 주문거부는 수신받은 전문 "4+11+ 55(찾아서) + 주문전문"을 전달한다. */
	/* KRX Header 82는 제외 */
	J_Q_Fmt = (YSK_JUMUN_FMT *)M_Fmt.Data;
	for (i = 0; i < r_cnt; i ++)
	{
		Dshm_Add_Count (PS_R_1, 1);					/* 주문 재전송 없음	*/

		/* 수신받은 데이터 포맷은 "55Byte + 주문전문"	*/
		if ( (memcmp (&R_Fmt[i].Data[ADD_HEADER_SIZE+11], "TCHODR10001", 11) == 0)	||	/* 일반호가(294) */
			 (memcmp (&R_Fmt[i].Data[ADD_HEADER_SIZE+11], "TCHODR10002", 11) == 0)  ||
			 (memcmp (&R_Fmt[i].Data[ADD_HEADER_SIZE+11], "TCHODR10003", 11) == 0)  )
		{
			datalen	= ADD_HEADER_SIZE + sizeof(KRX_JUMUN_DATA);
		}
#if	0
		else
		if (memcmp (&R_Fmt[i].Data[ADD_HEADER_SIZE+11], "TCHKOR10001", 11) == 0)		/* Kill Switch , KTS만 가능 (147) */
		{
			Size_Len	= 147;					
		}
#endif
		else
		{
			Device_Close ();
			Log (USR_ERROR, "Undefined DataType Was Read [%s][%11.11s]", R_Fmt[i].Data, R_Fmt[i].Data);
			sleep (3);
			Exit_Process ();
		}

		/* 호가정합성, 착오매매, 회원사내부룰에 의한 주문정합성 체크로직(협의필요) */
		rt = Chk_Risk_All (R_Fmt[i].Data);
#if	1
		if (rt < 0 || OpenFlag != ON)
#else
		if (rt < 0 || TCP2_NET_STA(S_K) != ON)
#endif
		{
			/* Write Format : 4+11 + 55(SPACE) + 주문전문 */
			memset (w_buf, 0x20, sizeof (w_buf));
			memcpy (w_buf, R_Fmt[0].Seq, sizeof(BUFF_RW_HEAD));

			memset (err_cd, 0, sizeof(err_cd));
#if	1
			if (OpenFlag != ON)
			{
				Log (USR_WARN, "개시전 주문(거부처리)");
			}
#else
			if (TCP2_NET_STA(S_K) == OFF)
			{
				Log (USR_WARN, "개시전 주문(거부처리)");
			}
			else if (TCP2_NET_STA(S_K) == END)
			{
				Log (USR_WARN, "장 종료 후 주문(거부처리)");
			}
#endif
			else
			{
				if (abs(rt) > 9999)
					sprintf (err_cd, "%04d", abs(rt)/10);
				else
					sprintf (err_cd, "%04d", abs(rt));

				Log (USR_WARN, "정합성오류 err_cd[%4.4s]", err_cd);
			}
	// BUFF_RW_HEAD      : file buffer header (50 + 20 = 70 bytes)
	// SEARCH_HEADER_LEN : sizeof (SEARCH_HEADER)) = 50 bytes

//			memcpy (&w_buf[sizeof(BUFF_RW_HEAD)],	R_Fmt[0].Data,	DATA_SIZE);		// 주문값 돌려주기
			memcpy (&w_buf[sizeof(BUFF_RW_HEAD)],	R_Fmt[0].Data,	datalen);		// 주문값 돌려주기
			dataheader = (YSK_DATA_HEAD *)&w_buf[sizeof(BUFF_RW_HEAD)];
			memcpy (dataheader->sRpCode,			"REJC",			4);
			w_buf[sizeof (BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

			/* 오류전달 */
			rt = F_W (TS_W1_1, w_buf, 1);
			if (rt != 1)
			{
				Log (SAM_FATAL, "file write[%s] rt[%d]", w_buf, rt);
				return (-1);
			}
			Log (USR_OK, "Err F_W Done. File write OK[%s][%d]", w_buf, strlen(w_buf));

#if	0
			// 아래 for문을 처리하지 못하게 한다.
			r_cnt = 0;		
	/* 2025 암복호와 들어가면서 순서가 뒤로가야한다 
			Dshm_Add_Count (PS_R_1, 1);		// 정상송신시에는 송신후 카운드증가
	*/
#endif
			continue;
		}

		shm_data_idx = LOAD_CNT % MAX_LOAD_CNT;
		LOAD_CNT ++;
		//SHM_DATA[i].seq = INT_SEQ + LOAD_CNT;
		SHM_DATA[shm_data_idx].seq = INT_SEQ + 1;
		memcpy (SHM_DATA[shm_data_idx].buf.Seq, R_Fmt[i].Seq,				sizeof (FILE_BUFF_FORMAT));
/*
2025 암복호화때문에 로직 변경됨
*/
		memcpy (J_Q_Fmt, R_Fmt[i].Data, datalen);
		ItoAf (datalen - sizeof(J_Q_Fmt->Header.sLength), J_Q_Fmt->Header.sLength,	sizeof(J_Q_Fmt->Header.sLength));
		memset(J_Q_Fmt->Header.sSeqNo,		'0',		 sizeof(J_Q_Fmt->Header.sSeqNo));
		//ItoAf (SHM_DATA[i].seq, J_Q_Fmt.JumunData[i].DataSeq,	sizeof (J_Q_Fmt.JumunData[0].DataSeq));
		memset(J_Q_Fmt->JumunData.DataSeq,	'0',		 sizeof(J_Q_Fmt->JumunData.DataSeq));
		//ItoAf (SHM_DATA[i].seq, &R_Fmt[i].Data[0],				11);	// 일련번호 Update후 처리넘김
		ItoAf (INT_SEQ+1,					&R_Fmt[i].Data[0],		11);	// 일련번호 Update후 처리넘김

// 값 요청 때문에
Log (USR_OK, "R_Fmt[i].Data [%s](%d)", R_Fmt[i].Data, strlen(R_Fmt[i].Data));
#if	0
		/* 202509 거래소 암복호화 처리 추가, Start */
		// 구조체를 넘길 때는 pointer + 크기를 같이 넘기는 게 안전함
		unsigned char* ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
														   Size_Len,
														   &enc_len);
		if (ret_data)
		{
			// 암호화된 데이터부를 J_Q_Fmt.JumunData[0]에 복사
			// 2025 암보화된 자료는 사이즈가 더 커진다. OverFlow현상이 발생됨.
			memcpy (DataBuff,	&J_Q_Fmt.Header,	sizeof(KRX_HEADER));
			ItoAf  (enc_len,	&DataBuff[8],		6);
			memcpy (&DataBuff[sizeof(KRX_HEADER)],	ret_data,	enc_len);
			//memcpy(J_Q_Fmt.JumunData[0].DataSeq, ret_data, enc_len);

			// 이제 J_Q_Fmt = [Header(평문) + JumunData(암호문)] 구조체가 완성됨
			// 바로 송신 가능
			// send(fd, &J_Q_Fmt, sizeof(J_Q_Fmt.Header) + enc_len, 0);

			// 암호화 버퍼 해제
			INL_Free_Buf(ret_data);
			ret_data = NULL;
		}
		else
		{
			LOAD_CNT --;
			SHM_DATA[i].seq = INT_SEQ + LOAD_CNT;
			Log (USR_ERROR, "암호화 오류 error, 확인 필요");

			// 암호화 버퍼 해제(송신후 해야함)
			INL_Free_Buf(ret_data);
			ret_data = NULL;
			sleep (3);
			Exit_Process ();
		}

		/* 202509 거래소 암복호화 처리 추가, End */
#endif
		(*datacnt) ++;
		totlen += datalen;
		J_Q_Fmt = (YSK_JUMUN_FMT *)((char *)J_Q_Fmt + datalen);

	}	// 실제로는 1번만, n번 불가

	return (totlen);
}	/* End of Make_Data_Block ()	*/

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
#if 0
	/* 2025 고객사와 협의필요 (호가정합성/착오매매/내부규정) */
    int     i, j, rt, tick_diff;
    char    m_time[24], Wdt[30], W3_Fmt[512], fifo_name[100];
    int     mm_flag, order_flag;
    double  mk_price, od_price, hl_lmt_price, order_quantity;
    int     mk_gbn, acc_seq, item_seq, risk_gbn;
    int     risk_mk_gbn;
    int     getcnt;
    double  in_std_price;

    KRX_JUMUN_DATA *dat =   (KRX_JUMUN_DATA *)&p_buf[0];

    mk_gbn   = AtoIf (dat->MembershipItem+35, 2);   // 시장구분
    item_seq = AtoIf (dat->MembershipItem+37, 5);   // 종목seq
    acc_seq  = AtoIf (dat->MembershipItem+42, 2);   // 계좌seq

    risk_mk_gbn   = AtoIf (dat->MembershipItem+51, 2);  // 리스크시장구분
#endif
	return 0;
}
#if	0
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
#endif
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

	/* 2025 추가 */
	PROC(D_K,P_K).start_status = JOB_END;
	PROC(D_K,P_K).process_status = 2;
	write (DTART_FD, "1", 1);
	/* 2025 추가 */
#else
    int     rt, rval;

	rt = Make_Send_Msg (TR_LOOU);
	memset (DataBuff, 0, sizeof (DataBuff));
	memcpy (DataBuff, &S_Fmt, SendLen);
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

		/* 2025 추가 */
		PROC(D_K,P_K).start_status = JOB_END;
		PROC(D_K,P_K).process_status = 2;
		write (DTART_FD, "1", 1);
		/* 2025 추가 */
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

			FirstSeq --;
			memset (t_time, 0, sizeof (t_time));
			Get_Time (t_time);

			if (memcmp (t_time, NO_TIME, 4) < 0)		/* hhmm	*/
				sleep (10);

			sleep (2);
			break;
		case	3:
			Log (USR_ERROR, "FEP : 장 종료 후 주문 접수[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);

			FirstSeq --;

			sleep (2);

			ErrCd = 0;
#if	0	/* 제거 */
			Log_Out();
#endif
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
		case   91:
			Log (USR_ERROR, "FEP : 전문길이 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   92:
			Log (USR_ERROR, "FEP : 업무식별코드 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   93:
			Log (USR_ERROR, "FEP : TR-CODE 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   94:
			Log (USR_ERROR, "FEP : 응답코드 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   95:
			Log (USR_ERROR, "FEP : 전문일련번호 형식 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   96:
			Log (USR_ERROR, "FEP : 처리시간 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   97:
			Log (USR_ERROR, "FEP : DATA 건수 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		case   98:
			Log (USR_ERROR, "FEP : 재처리 횟수 오류[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
		default:
			Log (USR_ERROR, "FEP : unknown 거부사유코드[%d] <%d:%d:%d>",
				ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
			sleep (2);
			break;
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
	End of Program (pa_2100_ts.c)
*************************************************************************/
