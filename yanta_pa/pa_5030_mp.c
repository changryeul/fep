#define		_GLOBAL
/*--------------------------------------------------------
#	Module	: 현선물차익거래(arb) 
#	File	: pa_5030_mp.c
--------------------------------------------------------*/
/*------------------------------------------------------*/
/*------------------------------------------------------*/

/*--------------------------------------------------------
	Header Files
--------------------------------------------------------*/
#include	"fep_fepp.h"
#include 	"pa_struct.h"
#include    "arb.h"

#define		DATA_SIZE	  2048
#define     ARB_STOP_CD   99
#include 	"buf_struct.h"
#include	"shm_memory.h"

/* 외부 소스 */
#ifndef _OMS_SOURCE_
#define		_OMS_SOURCE_	1
#endif /* _OMS_SOURCE_ */
/*--------------------------------------------------------
	FOR BLP
--------------------------------------------------------*/
int			Continue = 1;

/*--------------------------------------------------------
	Constants and Structures
--------------------------------------------------------*/
#define		FIFO_EVENT 	0
#define		FILE_EVENT	1

#define		TCP_TIME_OUT 30
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

#ifdef  SAM_USE
#define  WR_CNT   W_CNT(0,0)
#define  RD_CNT   R_CNT(0,0)
#else
#define  WR_CNT   IDW_CNT(0,0)
#define  RD_CNT   IDR_CNT(0,0)
#endif

/*--------------------------------------------------------
	Global Variables
--------------------------------------------------------*/
char    Order_Dat[261] = {"00000000001TCHODR10001G10001002999                    KR70059300031100000000182400000001230000007050020000000000000    0011030     00            000041010000011720172121276805CAE61CDC202110261317010280101                                                        0"};

int					Pk_mp[2];
char				ApType[10], Cli_handler[5];
char				W_DFmt[1024];
char				Order_St[1024];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];
struct pollfd		Poll[4];
int		InPollCnt, PollCnt;

int		OD_SEQ;
int		Set_Flag;
int     succ_flag;

/*--------------------------------------------------------
	Function Prototypes
--------------------------------------------------------*/
void	PA_5030_MP(void);
int		Init_Parameters(void);
void	Fifo_Event_Rtn(void);

void    File_Event_Rtn (void);
void	Set_In_Param(void);			// 500200(기동Setting);
void	Set_In_Stop(int, int);		// 500300(종료요청 수신), 500410(강제종료 송신)
void    Term_Process(int);
void    Pop_Fifo(int idx);

/*--------------------------------------------------------*/
int		main(int argc, char *argv[])
/*--------------------------------------------------------*/
{
	Init_Proc(argc, argv);
	PA_5030_MP();
	Exit_Process();
}	/* End of main */

/*--------------------------------------------------------*/
void		PA_5030_MP(void)
/*--------------------------------------------------------*/
{
	int		i, j, rt, bi_y, rc, ju_edt;
	int		as_p, o_price, o_ticks01, o_ticks03;
	int		call_jan, call_ticks, put_jan, put_ticks, fu_jan, jm_su;
	int		call_price, put_price, fu_price, for_cnt, op_gbn;
	int		f_rp_c, f_rp_p, ju_gbn, o_rp;
	char	t_time[12];
	
	Set_Flag = 0;
//	Log(USR_OK, "start ..... sleep 5");
//	sleep(5);			// Client로부터 Parameter를 받을 시간을 준다.
	rt = Init_Parameters();
	Log(USR_OK, "Init_Parameters rt = [%d]", rt);
	
	if (rt != OK)
	{
		//Exit_Process();
		Term_Process(9);
		sleep(1);
	}
	/*
	Log(USR_OK, "[%s:%d] Call Set_In_Param ... ", __FUNCTION__, __LINE__);
	Set_In_Param();
	*/
	
	PollCnt = 4;
	Log(USR_OK, "TEST R_CNT=[%d] RD_CNT=[%d]", WR_CNT, RD_CNT);

	while (START_S < JOB_END)
	{
		Stat_Save();

//		while (START_S < JOB_END)
		while (1)
		{
//			Log(USR_OK, "START_S=[%d] JOB_END=[%d]", START_S, JOB_END);
//			Log(USR_OK, "R_CNT=[%d] RD_CNT=[%d]", WR_CNT, RD_CNT);
			if (WR_CNT > RD_CNT)
			{
				File_Event_Rtn();		// 응답/체결 처리
				continue;
			}
			else
				break;
		}
		/*
		PollCnt = InPollCnt +2;
		Log(USR_OK, "poll ... PollCnt=[%d] TIME_OUT=[%d]", PollCnt, TIME_OUT * 1000);
		*/
		//Log (USR_OK, "PollCnt[%d]", PollCnt);
		rt = poll(Poll, PollCnt, 5000);
		//Log(USR_OK, "poss ... rt=[%d]", rt);
		if (rt > 0)
		{
			for (i = 0; i < PollCnt; i++)
			{	
				if (Poll[i].revents & POLLIN)
				{
					Poll[i].revents = 0;
					break;
				}
				
				if (Poll[i].revents & POLLHUP)
				{
					SLog(SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
					continue;
				}
		
			}
		}
		else
		{
			if (rt == 0)
			{
				//SLog(TCP_ERROR, "poll timeout (no response)");
				rc = Arb_Process(NULL);
				if (rc == ARB_STOP_CD)
				{
					market_auto_use_init();
				 	Log(USR_OK, "Arb 강제 전략 종료 .. rc=[%d]", rc);
				 	Term_Process(9);
				}
				continue;
			}
			else
			{
				if (SYS_NO == EINTR)
					SLog(SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
				else
					SLog(SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
				
				continue;
			}
		}
		
		//Log(USR_OK, "switch ... i=[%d]", i);
		switch (i)
		{
			case FIFO_EVENT:
				Fifo_Event_Rtn();
				break;
			case FILE_EVENT:			//File Event, 응답/체결처리, 설정/종료수신등
				while(START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						File_Event_Rtn();
						continue;
					}
					else
						break;
				}
				break;
			case 2:			
				//Log (USR_OK, "fut poll Evnet check");
				rc = Arb_Feed();
				Pop_Fifo(2); // 데이터 Read
				if (rc == ARB_STOP_CD)
				{
				 	Log(USR_OK, "Arb 강제 전략 종료 .. rc=[%d]", rc);
				 	Term_Process(9);
				}
				break;
			case 3:		
				//Log (USR_OK, "spot poll Event check");
				rc = Arb_Feed();
				Pop_Fifo(3); // 데이터 Read
				if (rc == ARB_STOP_CD)
				{
				 	Log(USR_OK, "Arb 강제 전략 종료 .. rc=[%d]", rc);
				 	Term_Process(9);
				}
				break;
			default:
				SLog(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}
	
	
}	/* End of PA_5030_MP () */

/***********************************************************
	Function : . File_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code : . void
	Comment : . read file and send data
***********************************************************/
/*--------------------------------------------------------*/
void	File_Event_Rtn (void)
/*--------------------------------------------------------*/
{
	int rt, R_Cnt;
	char tmp[128];
	int fut_side = 0, spot_side = 0;
	int fut_ord_id = 0, fut_fill_qty = 0;
	long spot_ord_id = 0, spot_fill_qty = 0;
	double fut_fill_px = 0, spot_fill_px = 0;
	int rc = 0;

//	Log(USR_OK, "File_Event_Rtn start ..." );
//	sleep(1);

	R_Cnt = DSHM_R(PS_R_1, (void *)R_Fmt, 1);

	Log(USR_OK, "WR_CNT=[%d] RD_CNT=[%d]", WR_CNT, RD_CNT);
	Log(USR_OK, "R_Fmt=[%s]", R_Fmt[0].Data );

//	Log(USR_OK, "    TS_R1_1 R_Fmt=[%s]", R_Fmt );
//	Log(USR_OK, "    TS_R1_1 R_Cnt=[%d]", R_Cnt );
	if (R_Cnt < 0)
	{
		Log(SAM_FATAL, "cannot read file[%s,%d:%s] W[%d]R[%d]",
			IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
		sleep(1);
		Exit_Process();
	}
	else if (R_Cnt == 0)
		return;
	
	Dshm_Add_Count(TS_W1_1, 1);
//Log (USR_OK, "5030 Wcnt[%d] Rcnt[%d]", WR_CNT, RD_CNT);
	
	if (memcmp(R_Fmt[0].Data, "500100", 6) == 0)             // 자동기동 Setting
	{		   
		Log(USR_OK, "[ARB] 현선차익거래전략구동 Data=[%.2048s]", (char *)R_Fmt[0].Data);
		Set_In_Param();

	}
	else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)		// 자동종료요청
	{
		Log(USR_OK, "[ARB] 자동종료요청 수신..");
		if (succ_flag == 1) // Arb_Init 정상처리시에만 call
			market_auto_use_init();
		else
			Log(USR_OK, "succ_flag = [%d]", succ_flag);

		Term_Process(9);

	}
	else if (memcmp(R_Fmt[0].Data, "100120", 6) == 0) // 응답(100120)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0) // 시장구분 - '01' 선물(파생) 
		{
		    IMECO_SETTLE_RESP_DATA *i = (IMECO_SETTLE_RESP_DATA *)&R_Fmt[0].Data[40];

			Log(USR_OK, "선물 주문응답전문 : [%.121s]", (char *)i);
			Log(USR_OK, "MsgType [%.*s]", (int)sizeof(i->Order_Message_Type), i->Order_Message_Type);
			if (memcmp(i->Order_Message_Type, "R", 1) == 0) // R-Reject, 
			{
				 fut_ord_id   = AtoIf(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
			  	 Log(USR_OK, "선물 주문거부응답 fut_ord_id [%ld]", fut_ord_id);
				 rc = Arb_Fut_Exec(-1, 0, fut_ord_id, 0, 0); 
				 if (rc == ARB_STOP_CD)
				 {
					market_auto_use_init();
				 	Log(USR_OK, "Arb_Fut_Exec 강제 전략 종료 .. 선물 주문 거부 rc=[%d]", rc);
				 	Term_Process(9);
				 }
			}
			else if (memcmp(i->Order_Message_Type, "A", 1) == 0) // A-Expired(KRX Auto Cancel)
			{
				 fut_ord_id   = AtoIf(i->OrderNo, sizeof(i->OrderNo)); // 주문번호
			  	 Log(USR_OK, "선물 KRX Auto Cancel 응답 fut_ord_id [%ld]", fut_ord_id);
				 rc = Arb_Fut_Exec(-2, 0, fut_ord_id, 0, 0); 
				 if (rc == ARB_STOP_CD)
				 {
					market_auto_use_init();
				 	Log(USR_OK, "Arb_Fut_Exec 강제 전략 종료 .. rc=[%d]", rc);
				 	Term_Process(9);
				 }
			} else {
				Log(USR_OK, "INCORRECT CHECK CASE!!! Order_Message_Type[%.1s] rc=[%d]", i->Order_Message_Type, rc);
			}
		}
		else if (memcmp(R_Fmt[0].Data+6, "04", 2) == 0) // 시장구분 - '04' 현물 
		{
			SMB_ST *s = (SMB_ST *)&R_Fmt[0].Data[20];
			Log(USR_OK, "현물 주문응답 전문 : [%.*s]", sizeof(SMB_ST), (char*)s);
			Log(USR_OK, "MsgType [%.*s]", (int)sizeof(s->smb_MsgType), s->smb_MsgType);
			if (memcmp(s->smb_MsgType, "3", 1) == 0 || memcmp(s->smb_MsgType, "8", 1) == 0  ) // '3'-거부, '8'-주문확인 및 체결
			{
				// 거부/취소 떨어지면 양쪽 수량 맞을때까지 retry...
				if ((memcmp(s->smb_OrdStatus, "8", 1) == 0 && memcmp(s->smb_ExecType, "8", 1) == 0) || 
					(memcmp(s->smb_OrdStatus, "4", 1) == 0 && memcmp(s->smb_ExecType, "4", 1) == 0)
				    ) // '8'-거부
				{
					spot_ord_id   = AtoLf(s->smb_ClOrdID + 8, 10);  // 주문번호
			  	 	Log(USR_OK, "현물 거부응답..spot_ord_id [%ld]", spot_ord_id);
					rc = Arb_Spot_Exec(-1, 0, spot_ord_id, 0, 0);
					if (rc == ARB_STOP_CD)
					{
						market_auto_use_init();
						Log(USR_OK, "Arb_Spot_Exec 강제 전략 종료.. 현물 주문 거부 rc=[%d]", rc);
						Term_Process(9);
					}

				}
			} else {
				Log(USR_OK, "INCORRECT CHECK CASE!!! smb_MsgType[%.1s] rc=[%d]", s->smb_MsgType, rc);
			}
		}
	}
	else if (memcmp(R_Fmt[0].Data, "100140", 6) == 0)// 체결(100140)
	{
		if (memcmp(R_Fmt[0].Data+6, "01", 2) == 0) // 시장구분 - '01' 선물(파생) 
		{
			IMECO_SETTLE_DATA *i = (IMECO_SETTLE_DATA *)&R_Fmt[0].Data[40]; // oms헤더(20byte) + 유진헤더(20byte)
			Log(USR_OK, "선물 체결 전문 : [%.*s]", sizeof(IMECO_SETTLE_DATA), (char*)i);
			if (memcmp(i->Order_Message_Type, "T", 1) == 0) // T-Trade '체결' 
			{
				fut_ord_id   = AtoIf(i->OrderNo, sizeof(i->OrderNo));               // 주문번호
			    fut_side	 = AtoIf(i->TradeFlag, sizeof(i->TradeFlag));           // 매도매수구분코드 - 1:Sell, 2.Buy 
				fut_fill_qty = AtoIf(i->Trading_Volumn, sizeof(i->Trading_Volumn)); // 체결수량 - 계약단위
				fut_fill_px  = AtoDf(i->Trading_price, sizeof(i->Trading_price));   // 체결가격				
				rc = Arb_Fut_Exec(1, fut_side, fut_ord_id, fut_fill_qty, fut_fill_px);
				if (rc == ARB_STOP_CD)
				{
					market_auto_use_init();
					Log(USR_OK, "Arb_Fut_Exec 강제 전략 종료.. 선물 주문 거부 rc=[%d]", rc);
					Term_Process(9);
				}

			} else {
				Log(USR_OK, "INCORRECT CHECK CASE!!! Order_Message_Type[%.1s] rc=[%d]", i->Order_Message_Type, rc);
			}
		}
		else if (memcmp(R_Fmt[0].Data+6, "04", 2) == 0) // 시장구분 - '04' 현물 
		{
			SMB_ST *s = (SMB_ST *)&R_Fmt[0].Data[20]; // oms헤더(20byte)
			Log(USR_OK, "현물 체결 전문 : [%.*s]", sizeof(SMB_ST), (char*)s);
			if (memcmp(s->smb_MsgType, "8", 1) == 0 ) // '8'-주문확인 및 체결 
			{
				spot_ord_id   = AtoLf(s->smb_ClOrdID + 8, 10);             // 주문번호
				spot_side     = AtoIf(s->smb_Side, sizeof(s->smb_Side));                       // 매매구분 - 1:Buy, 2:Sell
				spot_fill_qty = AtoLf(s->smb_LastQty, sizeof(s->smb_LastQty)) / 1000000L;      // 체결수량 - arb에서 수량을 M 단위로 관리하므로 백만으로 나눈 후 전송 
				spot_fill_px  = AtoDf(s->smb_LastPx, sizeof(s->smb_LastPx));                   // 체결가격
				rc = Arb_Spot_Exec(1, spot_side, spot_ord_id, spot_fill_qty, spot_fill_px);
				if (rc == ARB_STOP_CD)
				{
					market_auto_use_init();
					Log(USR_OK, "Arb_Spot_Exec 강제 전략 종료.. 현물 주문 거부 rc=[%d]", rc);
					Term_Process(9);
				}

			} else {
				Log(USR_OK, "INCORRECT CHECK CASE!!! smb_MsgType[%.1s] rc=[%d]", s->smb_MsgType, rc);
			}
		}
	}
	else
	{
		Log(USR_ERROR, "Other Tr Code Check Plz [%20.20s]", R_Fmt[0].Data);
	}
	
	while(1)
	{
		rt = read(INPUT_FD, tmp, sizeof(tmp));
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
	}
	
	return;
}	/* File_Event_Rtn() */

/***********************************************************
	Function 		: . Init_Parameters
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . All Program Parameters Init
***********************************************************/
/*--------------------------------------------------------*/
int		Init_Parameters()
/*--------------------------------------------------------*/
{
	char 	fifo_name[256];
	int 	return_flag = 0;
	int 	i, rt;
	
	/* attach daemon SHM (INFO) : ALL_DAEMON_INFO */
    Sub_SHM();

	Log(USR_OK, "Init_Parameters ...ARB ");
	
	Poll[0].fd = START_FD;		// of Daemon
	Poll[0].events = POLLIN;
	Poll[1].fd = INPUT_FD; 		// of SelfFile
	Poll[1].events = POLLIN;

	/* 선물 */
	memset(fifo_name, 0, sizeof(fifo_name));

	sprintf(fifo_name, "%s/PA/pa_55%5.5s1", _FEP_FIFO, _Exe_Name+5);  // 선물시세수신

	Poll[2].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[2].events = POLLIN;
	Log(USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[2].fd);

	if (Poll[2].fd < 0)
	{
		Log(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
			i, fifo_name, Poll[2].fd, SYS_NO, SYS_STR);
		return_flag = -1;
	}
	
	// 기동시 2로 변경
	// 종료시 0으로 변경해야함
	PROC(D_K,P_K).tr_seq = 2; // tr_seq : 시장구분 - 1:채권 , 2:선물, 3:채권/선물 둘다 수신필요시
	Log (USR_OK, "tr_seq[%d]", PROC(D_K,P_K).tr_seq);

	/* 현물 */
	memset(fifo_name, 0, sizeof(fifo_name));
	sprintf(fifo_name, "%s/PA/pa_7402_ur1", _FEP_FIFO);  // 현물시세수신
	Poll[3].fd = open(fifo_name, O_RDWR|O_NDELAY);
	Poll[3].events = POLLIN;
	Log (USR_OK, "_Exe_Name [%s] fifo_name[%s] fd[%d]", _Exe_Name, fifo_name, Poll[3].fd);

	if (Poll[3].fd < 0)
	{
		Log(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
			i, fifo_name, Poll[3].fd, SYS_NO, SYS_STR);
		return_flag = -1;
	}

	/* 전략 전달용 시그널 설정 */
	Log(USR_OK, "    START_FD =[%d]", START_FD);
	Log(USR_OK, "    DTART_FD =[%d]", DTART_FD);
	Log(USR_OK, "    INPUT_FD =[%d]", INPUT_FD);
	Log(USR_OK, "    INPUT_FD2=[%d]", INPUT_FD2);

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;
	
	memset(ApType, 0, sizeof(ApType));
	sprintf(ApType, "%-2.2s%-5.5s", _Exe_Name, _Exe_Name+3);
	LtoU(ApType, strlen(ApType));
	Log(USR_OK, "ApType=[%s]", ApType);
	
	/* Process 증가될때마다 수정검증을 해야됨. */
	OD_SEQ = ((AtoIf(&ApType[3], 2) -1) * 10) + AtoIf(&ApType[5], 2) -1;
	Log(USR_OK, "OD_SEQ=[%d]", OD_SEQ);

	Log(USR_OK, "Init_Parameters ... END");
	
	// 2025114 추가
	Log(USR_OK, "1 WR_CNT [%d] RD_CNT[%d]", WR_CNT, RD_CNT);
	if (WR_CNT > RD_CNT) RD_CNT = WR_CNT -1;

	Log(USR_OK, "2 WR_CNT [%d] RD_CNT[%d]", WR_CNT, RD_CNT);

	return 0;
}	/* End of Init_Parameters() */

/***********************************************************
	Function 		: . Set_In_Param
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_In_Param(void)
/*--------------------------------------------------------*/
{
	int		i, rt, arry;
	char 	fifo_name[256];
	char	err_no[10], head_size[10];
	
	IN_SAMPLE01			*dat;
	unsigned char		*ptr = ( char *)dat;
	
	Log(USR_OK, "Set_In_Param ...ARB ");
	dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];
	//Log(USR_OK, "dat->next_flag=[%s][%s]", dat->next_flag, (char *)dat);

	succ_flag = 0;

	rt = Arb_Init(ApType, (char *)dat);
	if (rt != 0)
	{
	    Log(USR_OK, "****Arb_Init Error [%d]********", rt);
		Term_Process(9);
	}
	
	succ_flag = 1;

//	Log(USR_OK, "R_Fmt=[%s]", (char *)&R_Fmt[0]);

	return;
}	/* End of Set_In_Param */


/***********************************************************
	Function 		: . Set_In_Stop
	Parameters IN 	: . tr_gbn   => 0:500310(정상종료), 1:500410(강제종료)
					: . min_flag => 0:감소없이, 1:감소하고
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: 미사용 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_In_Stop (int tr_gbn, int min_flag)
/*--------------------------------------------------------*/
{
	int		i, rt;
	char	err_no[10], fifo_name[128];

	/* ************************************************** */
	/* 정상여부 응답 처리 */
	memset(W_DFmt, 0x20, sizeof (W_DFmt));
	memcpy(&W_DFmt, &R_Fmt, sizeof (BUFF_RW_HEAD)+20);
	if (tr_gbn == 0)
		memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)], "500310", 6);		/* SEARCH_HEADER(20), TrCode (500310:정상종료)	*/
	else
		memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)], "500410", 6);		/* SEARCH_HEADER(20), TrCode (500410:강제종료)	*/

	memset(err_no, 0, sizeof (err_no));
	sprintf(err_no, "%04d", 0);

	memcpy(&W_DFmt[sizeof (BUFF_RW_HEAD)+10], err_no, 4);			/* SEARCH_HEADER(20), ErrCode(0000:Normal)		*/
	W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';

	rt = F_W(TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
		Exit_Process ();
	}

	Log(USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);

	/* 전략이 종료처리를 위한 Set을 한다 */
	Shm_Strrg[0].auto_run_flag[OD_SEQ] = 0;
//	PROC(D_K,P_K).start_status = JOB_END;
	PROC(D_K,P_K).process_status = 9;

	/* Daemon에게 즉각적인 종료요청을 위해 signal 주기 */
	sprintf(fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open(fifo_name, O_RDWR | O_NDELAY);

	if (DFIFD(D_K) == -1)
	{
		Log(FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		exit(FAIL);
	}

	write(DFIFD(D_K), "1", 1);
	Log (USR_OK, "Arb 전략종료..");
}	/* End of Set_In_Stop () */

/*************************************************************************
    Function  : . Fifo_Event_Rtn
    Parameters IN : .
    Parameters OUT : .
    Return Code  : . void
    Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    char tmp[2];

    read(START_FD, tmp, 1);

    return;
}   /* End of Fifo_Event_Rtn () */

/*----------------------------------------------------------------------*/ 
void    Term_Process (int option) 
/*----------------------------------------------------------------------*/ 
{ 
	char	fifo_name[128];

    PROC(D_K,P_K).process_status = option;      /* 1:죽을시 재실행  9:kill */

	Log(USR_OK, "PROC(D_K,P_K).process_status = %d", PROC(D_K,P_K).process_status);
	PROC(D_K,P_K).tr_seq = 0;                   /* 시세 시그널 해제                 */
	// write(DTART_FD, "1", 1); /* process 관리 데몬에 signal 보냄 - 필요 없음(?) */ 
	
	Log (USR_OK, "Arb 전략종료..");

    /* Daemon에게 즉각적인 종료요청을 위해 signal 주기 */
	sprintf (fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);

	Log (USR_OK, "fifo_name[%s] [%s]", fifo_name, _Exe_Name);
	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);

	if (DFIFD(D_K) == -1)
	{
		Log(FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		exit(FAIL);
	}

	write(DFIFD(D_K), "1", 1);

	Exit_Process();
}

/***********************************************************
    Function : . File_Event_Rtn
    Parameters IN : .
    Parameters OUT : .
    Return Code : . void
    Comment : . read file and send data
***********************************************************/
/*--------------------------------------------------------*/
void    Pop_Fifo(int idx)
/*--------------------------------------------------------*/
{
    int     rt;
    char    tmp[ 64];

    while(1)
    {
        rt = read(Poll[idx].fd, tmp, sizeof(tmp));
        if (rt == 0 || errno == EAGAIN) break;
    }

}
/*************************************************************************
    End of Program (pa_5050_mp.c)
*************************************************************************/

