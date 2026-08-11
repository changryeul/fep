#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module : 주문응답/체결 수신 (queue)
#            금융 회원처리호가/체결 수신
#	File : pa_4200_qr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include    "pa_struct.h"
#include    "fx.h"

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
#define     DATA_SIZE       1024
#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
char    ApType[10];
char    RecvPkt[TCP_BUFF_MAX_LEN];

FILE_BUFF_FORMAT	W_Fmt;
SMB_ST				*R_Pkt = (SMB_ST *)RecvPkt;      /* 1024 */
FILE_DATA_HEAD		File_Data_Head;     // 20

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Catch_Signal		(int);
int		Check_Header		(void);
void	PA_4200_QR			(void);
void	Init_Parameters		(void);
int		MakeQueue			(key_t);
int		Receive_Packet		(int);
int		ReceiveQueue		(int, long, char *);
void	Register_Signal		(void);
int		Write_Data			(void);

/*----------------------------------------------------------------------*/
int		main	(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc	(argc, argv);
	PA_4200_QR ();
	Exit_Process ();
}	/* End of main () */

/*----------------------------------------------------------------------*/
void	PA_4200_QR (void)
/*----------------------------------------------------------------------*/
{
	key_t	new_key;
	int		msqid;
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

	// Get msg q ID
	new_key = 0x22000025;	// 금융 체결 수신

//	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
//		new_key += 0x01000000L;

	msqid = MakeQueue (new_key);
	if (msqid < 0)
	{
		Log(USR_ERROR, "msgget msqid[%d]", msqid);
		Exit_Process ();
	}
	else
		Log (USR_OK, "msgget: succeeded: msqid = %d new_key [%#x]", msqid, new_key);


	while (START_S < JOB_END)
	{
#if	0	/* HONG */
		Stat_Save	();
#endif /* HONG */

		rt	= Receive_Packet (msqid);

		if	(rt == NOTOK)
			break;
		else	if (rt == FAIL)
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
	sprintf (ApType, "%-2.2s%-4.4s%c%c",
		_Exe_Name,	_Exe_Name+3, _Exe_Name[8], _Exe_Name[9] == 's' ? 'R' : 'S');
	LtoU (ApType, strlen (ApType));

	return;
}	/* End of Init_Parameters () */

/*************************************************************************
	Function		: . Receive_Packet
	Parameters IN : .
	Parameters OUT : .
	Return Code		: . int (0:success, -1:failure, 1:interrupted)
	Comment		 : . receive Receive_Packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet	(int msqid)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	error_flag;

	error_flag = OFF;
	memset (RecvPkt, 0, sizeof (RecvPkt));
	rt = ReceiveQueue(msqid, 0L, RecvPkt);
	if (rt < 0)
	{
		Log (USR_ERROR, "receive fail {%d:%s} rt[%d]", SYS_NO, SYS_STR, rt);
		return	(NOTOK);
	}

	RecvPkt[rt] = 0x00;

	Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	rt = Check_Header ();

	if (rt != OK)
		return	(NOTOK);

	rt	= Write_Data ();
	if	(rt != OK)
	{
		Log	(USR_ERROR, "Write_Data Error");
		sleep (3);
		return (NOTOK);
	}

	return (OK);
}	/* Receive_Packet () */

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
char    U_ErrMsg[1024];

memset (U_ErrMsg, 0, sizeof(U_ErrMsg));

/* Length (사이즈), Response of LogOn & HeartBit & Data Error Response */
if (strlen (RecvPkt) != sizeof(SMB_ST))
{
	SLog (USR_ERROR, "invalid length[%d:%d]", strlen (RecvPkt), sizeof(SMB_ST));
	return (NOTOK);
}

/* ResponseCode (응답코드) */

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
    int         d_cnt = 1, rt;
	int         f_gbn;
	char        m_time[24];
    BUFF_RW_HEAD f_head;


	if (memcmp (R_Pkt->smb_Symbol, "USD/KRW", 7) != 0
	 && memcmp (R_Pkt->smb_Symbol, "USD/KRO", 7) != 0
	 && memcmp (R_Pkt->smb_Symbol, "JPY/KRO", 7) != 0
	 && memcmp (R_Pkt->smb_Symbol, "CNH/KRO", 7) != 0)
	{
		f_gbn = 9;
	}
	else
	if (memcmp (R_Pkt->smb_MsgType, "D", 1) == 0   ||		// 신규주문거부
		memcmp (R_Pkt->smb_MsgType, "9", 1) == 0   ||		// 취소주문거부
		memcmp (R_Pkt->smb_MsgType, "F", 1) == 0   ||		// 취소주문거부
		memcmp (R_Pkt->smb_MsgType, "5", 1) == 0   )		// FIX연결 끊어짐
	{
		f_gbn = 1;
	}
	else
	if (memcmp (R_Pkt->smb_MsgType, "8", 1) == 0   )
	{
		if (memcmp (R_Pkt->smb_OrdStatus, "0", 1) == 0)		// 확인
			f_gbn = 1;
		else
		if (memcmp (R_Pkt->smb_OrdStatus, "1", 1) == 0
		 || memcmp (R_Pkt->smb_OrdStatus, "2", 1) == 0)
		{
			if (memcmp (R_Pkt->smb_ExecType, "1", 1) == 0
			 || memcmp (R_Pkt->smb_ExecType, "2", 1) == 0
			 || memcmp (R_Pkt->smb_ExecType, "F", 1) == 0)	// 체결
				f_gbn = 2;
			else
				f_gbn = 9;
		}
		else
		if (memcmp (R_Pkt->smb_OrdStatus, "4", 1) == 0)		// 접수이후 주문취소
			f_gbn = 1;
		else
		if (memcmp (R_Pkt->smb_OrdStatus, "8", 1) == 0)		// 접수이후 주문거부
			f_gbn = 1;
		else
			f_gbn = 9;
	}

	Log (USR_OK, "SYMBOL[%.7s] gbn[%d]", R_Pkt->smb_Symbol, f_gbn);
	if (f_gbn != 1 && f_gbn != 2)
		return (OK);

	/* ************************************ */
    /* 2501 : 체결전문                      */
    /* ************************************ */
    memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
    memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);

    ItoAf (INT_SEQ + 1, W_Fmt.If_Seq,			sizeof (W_Fmt.If_Seq));

    memcpy (W_Fmt.ApType,		ApType,			sizeof (W_Fmt.ApType));
    memcpy (W_Fmt.ResponseCode,	RES_NORMAL, 	strlen (RES_NORMAL));
    memcpy (W_Fmt.RecvTime1,	m_time,			sizeof (W_Fmt.RecvTime1));
    memcpy (W_Fmt.RecvTime2,    &m_time[sizeof(W_Fmt.RecvTime1)],
												sizeof (W_Fmt.RecvTime2));

    ItoAf (DATA_SIZE, File_Data_Head.Length,	sizeof (File_Data_Head.Length));
    ItoAf (INT_SEQ + 1, File_Data_Head.DataSeq,	sizeof (File_Data_Head.DataSeq));
    memcpy (File_Data_Head.ResponseCode,	RES_NORMAL,	strlen (RES_NORMAL));
    memcpy (File_Data_Head.LineFlag,		_Exe_Name+4,	3);

    memcpy (W_Fmt.DataHeader,	&File_Data_Head,sizeof (HEAD_SIZE));

    /* 2501 : 체결정보 체결전문, 원장에서 체결만 수신 */
	memcpy (W_Fmt.Data,		R_Pkt,	sizeof (SMB_ST));
    W_Fmt.LineFeed[0] = '\n';
	
	SYS_NO = 0;

#ifdef	SAM_USE
	if (f_gbn == 1)
		rt	= F_W (TS_W1_1, (void *)&W_Fmt, 1);
	else if (f_gbn == 2)
		rt	= F_W (TS_W2_1, (void *)&W_Fmt, 1);
#else
	if (f_gbn == 1)
		rt	= DSHM_W (TS_W1_1, (void *)&W_Fmt, 1);
	else if (f_gbn == 2)
		rt	= DSHM_W (TS_W2_1, (void *)&W_Fmt, 1);
#endif

	if (rt != d_cnt)
	{
#ifdef	SAM_USE
		if (f_gbn == 1)
			Log	(SAM_FATAL, "file write[%s]", OUT_NAME);
		else if (f_gbn == 2)
			Log	(SAM_FATAL, "file write[%s]", OUT_NAME2);
#else
		if (f_gbn == 1)
			Log	(DSH_FATAL, "DSHM write[%s]", OUT_NAME);
		else if (f_gbn == 2)
			Log	(DSH_FATAL, "DSHM write[%s]", OUT_NAME2);
#endif
		return	(NOTOK);
	}

	INT_SEQ += d_cnt;
	if (f_gbn == 1)
		Log	(USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	else if (f_gbn == 2)
		Log	(USR_OK, "data write[%s:%d:%d]", OUT_NAME2, WR_CNT2, d_cnt);
	Log (USR_OK, "Write ok[%s][%d]", W_Fmt.Data, strlen(W_Fmt.Data));
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

	Exit_Process ();
}	/* End of Catch_Signal () */

/*************************************************************************
	End of Program (pa_2200_tr.c)
*************************************************************************/
