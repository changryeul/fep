#define		_GLOBAL
/*--------------------------------------------------------
#	Module	: Server 선물업틱 call매수, 다운틱 put매수
#	File	: pa_5020_mp.c
--------------------------------------------------------*/
/*------------------------------------------------------*/
/*------------------------------------------------------*/

/*--------------------------------------------------------
	Header Files
--------------------------------------------------------*/
#include	"fep_fepp.h"
#include 	"pa_struct.h"

#define		DATA_SIZE	2048
#include 	"buf_struct.h"

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
char				W_DFmt[4096];
char				Order_St[1024];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];
struct pollfd		Poll[10];
int		InPollCnt, PollCnt;

int		OD_SEQ;
int		Set_Flag;

/*--------------------------------------------------------
	Function Prototypes
--------------------------------------------------------*/
void	PA_5020_MP(void);
int		Init_Parameters(void);
void	Fifo_Event_Rtn(void);
void	Check_Proc_Status(int);
int		Od_Write_Data(int, int, int, int, int);

int		Auto_Logic(int);
void    File_Event_Rtn (void);
void	Set_In_Param(void);			// 500200(기동Setting);
void	Set_In_Stop(int, int);		// 500300(종료요청 수신), 500410(강제종료 송신)
void	Set_Response(void);			// 100120(응답확인/거부), 100140(체결/확인/거부)

/*--------------------------------------------------------*/
int		main(int argc, char *argv[])
/*--------------------------------------------------------*/
{
	Init_Proc(argc, argv);
	PA_5020_MP();
	Exit_Process();
}	/* End of main */

/*--------------------------------------------------------*/
void		PA_5020_MP(void)
/*--------------------------------------------------------*/
{
	int		i, j, rt, bi_y, rc, ju_edt;
	int		as_p, o_price, o_ticks01, o_ticks03;
	int		call_jan, call_ticks, put_jan, put_ticks, fu_jan, jm_su;
	int		call_price, put_price, fu_price, for_cnt, op_gbn;
	int		f_rp_c, f_rp_p, ju_gbn, o_rp;
	char	t_time[12];
	
	Set_Flag = 0;
	sleep(5);			// Client로부터 Parameter를 받을 시간을 준다.
	rt = Init_Parameters();
	
Log (USR_OK, "Ok 01");
	if (rt != OK)
	{
		Exit_Process();
		sleep(1);
	}
	
	while (START_S < JOB_END)
	{
Log (USR_OK, "Ok 02");
		Stat_Save();
		
		while (START_S < JOB_END)
		{
Log (USR_OK, "Ok 03");
/* **************************************************************************************************** */
/* 자동로직 시작 2021.11.02																					*/
/* **************************************************************************************************** */
			if (WR_CNT > RD_CNT)
			{
Log (USR_OK, "Ok 04");
				File_Event_Rtn();		// 응답/체결 처리
Log (USR_OK, "Ok 05");
				if (Set_Flag > 0)
					PollCnt = InPollCnt + 2;
				else
					PollCnt = 2;
				continue;
			}
			else
				break;
		}
		
		rt = poll(Poll, PollCnt, TIME_OUT * 1000);
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
				SLog(TCP_ERROR, "poll timeout (no response)");
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
		
		switch (i)
		{
			case FIFO_EVENT:
				Fifo_Event_Rtn();
				break;
			case FILE_EVENT:			//File Event, 응답/체결처리, 설정/종료수신등
				File_Event_Rtn();
				if (Set_Flag > 0)
					PollCnt = InPollCnt + 2;
				else
					PollCnt = 2;
				
				while(START_S < JOB_END)
				{
					if (WR_CNT > RD_CNT)
					{
						File_Event_Rtn();
						if (Set_Flag > 0)
							PollCnt = InPollCnt + 2;
						else
							PollCnt = 2;
						continue;
					}
					else
						break;
				}
				break;
				
			case 2:			// 첫번째 시장신호(ex, 채권)
			case 3:			// 두번째 시장신호(ex, 금융파생)
				rt = Auto_Logic(i-2);
				if (rt)
				{
					rt = Od_Write_Data(1, 1, 1, 2, 1);		// mk_gbn : 시장구분 (1~10),
															// nmc_gbn : 1:신규, 2:정정, 3:취소,  
															// p_flag : write falg 1:채권, 2:파생시장주문
															// mm_gbn : 1(매도), 2(매수)
															// arry : 주문낼 대상의 arry(수신받은 종목중 몇번째)
				}
				else
					return;

				break;
				
			default:
				SLog(USR_ERROR, "event case error[%d,%d]", i, PollCnt);
				return;
				break;
		}
	}
	
	Check_Proc_Status(0);
	Check_Proc_Status(1);
	
}	/* End of PA_5020_MP () */

/***********************************************************
	Function : . File_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code : . void
	Comment : . read file and send data
***********************************************************/
/*--------------------------------------------------------*/
void	File_Event_Rtn (void) /*--------------------------------------------------------*/
{
	int rt, R_Cnt;
	char tmp[128];
	
	R_Cnt = DSHM_R (TS_W1_1, (void *)R_Fmt, 1);
Log( USR_OK, "    Read File R_Fmt[%s]", R_Fmt[0].Data);
	if (R_Cnt < 0)
	{
		Log(SAM_FATAL, "cannot read file[%s,%d:%s] W[%d]R[%d]",
			IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, RD_CNT);
		sleep(1);
		Exit_Process();
	}
	else if (R_Cnt == 0)
		return;
	
	if (memcmp(R_Fmt[0].Data, "500100", 6) == 0)				// 자동기동 Setting
		Set_In_Param();
	else if (memcmp(R_Fmt[0].Data, "500300", 6) == 0)		// 자동종료요청
	{
		if (Set_Flag == 0)
			Set_In_Stop(0,0);		// 감소없이 종료
		else
			Set_In_Stop(0,1);		// 감소시키고 종료
	}
	else if ((memcmp(R_Fmt[0].Data, "100120", 6) == 0) ||		// 응답(100120), 체결(100140)
			 (memcmp(R_Fmt[0].Data, "100140", 6) == 0))
		Set_Response();
	else
	{
		Log(USR_ERROR, "Other Tr Code Check Plz [%20.20s]", R_Fmt[0].Data);
	}
	
	Dshm_Add_Count(TS_W1_1, 1);
	
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

/*--------------------------------------------------------*/
int Auto_Logic(int fd_gbn)
/*--------------------------------------------------------*/
{
	int		rt = 0;
	int		curr_key, be_curr_key;
	/* **************************************************************************** */
	/* 채권시장																		*/
	/* 1. 조건은 체결(A3/G7)에만 반응한다.											*/
	/* 2. 샘플전략은 채권의 체결수량이 50계약 이상 && 체결틱이 상승 => 매수			*/
	/* 				   체결수량이 50계약 이상 && 체결틱이 하락 => 매도				*/
	/* 3. 주문은 1회만 처리하고 종료한다. 											*/
	/* 4. 전략설정시 3개의 종목을 받는데 첫번째는 조건이 되는 종목이고				*/
	/*						 두번째는 주문대상이 되는 종목이다						*/
	/* 5. 또한 화면(전략기동)에서도 3종목은 순서대로 받기로 하였다.						*/
	/* **************************************************************************** */
	
	if (AtoIf(&Shm_Note[0].G7.bond_accum_exec_vol[7], 8) <= 0)	// 장개시전에는 주문처리 안함.
		return (0);
	
	/* 선물의 체결수량(50이상)과 UpTick or DownTic 체크 */
	if (Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].HogaLastGbn == 0)	// 0:A3 or G7, 1:B6
	{
		// 현재체결가
		curr_key	= Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry_Key;
		// 직전체결가
		be_curr_key	= Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].Befor_CURR_Arry_Key;

		/* 체결수량 체크 */
		if (memcmp(&Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].volume[2], "00000050", 8) > 0)
		{
			/* Up or Down 체크 */
			if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
					   Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) > 0)	//UP
			{
				rt = 1;			// 첫번째종목 매수
			}
			else
			if (memcmp(Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[curr_key].crprc,
					   Shm_Note[Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[0].item_seq].CURR_Arry[be_curr_key].crprc, 10) < 0)	//Down
			{
				rt = 2;			// 두번째종목 매수
			}
		}
	}
	
	return (rt);
	
}	/* End of Auto_Logic() */

/***********************************************************
	Function 		: . Check_Proc_Status
	Parameters IN 	: .
	Parameters OUT 	: .
	Return Code 	: . void
	Comment 		: . check validity of Send_Proc status
***********************************************************/
/*--------------------------------------------------------*/
void	Check_Proc_Status(int mk)
/*--------------------------------------------------------*/	  
{
	int rt;
	
#if 0
	if (TCP1_NSTAT(D_K,Pk_mp[mk]) != ON)
	{
		Log(USR_ERROR, "check process status[%.10s]", OFN(D_K,P_K,mk));
		
		sleep(1);
	}
#endif
	
	return;
}	/* End of Check_Proc_Status() */

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
	int 	i, rt;
	
	SEARCH_HEADER		*hd;
	KRX_JUMUN_DATA		*KrxOrder;
	
	Poll[0].fd = START_FD;		// of Daemon
	Poll[0].events = POLLIN;
	Poll[1].fd = INPUT_FD; 		// of SelfFile
	Poll[1].events = POLLIN;
	
	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;
	
	memset(ApType, 0, sizeof(ApType));
	sprintf(ApType, "%-2.2s%-5.5s", _Exe_Name, _Exe_Name+3);
	LtoU(ApType, strlen(ApType));
	
	/* Process 증가될때마다 수정검증을 해야됨. */
	OD_SEQ = ((AtoIf(&ApType[3], 2) -1) * 10) + AtoIf(&ApType[5], 2) -1;
	
	/* SendProcess 회선 Set */
	for(i = 0; i < 2; i++)
	{
		for(Pk_mp[i] = 0; Pk_mp[i] < DAEMON(D_K).p_count; Pk_mp[i] ++)
		{
			if (memcmp(PROC(D_K,Pk_mp[i]).process_id, ODN(D_K,P_K,i), 10) == 0)
				break;
			
			if (Pk_mp[i] == DAEMON(D_K).p_count - 1)
			{
				Log(USR_FATAL, "unregistered process[%s]", ODN(D_K,P_K,i));
				TCP1_NET_STA = OFF;
				Exit_Process();
			}
		}
	}
	
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
	int		i, rt, arry, mk_gbn, item_seq;
	int 	return_flag = 0;
	char 	fifo_name[256];
	char	err_no[10], head_size[10];
	
// 요기서부터 코딩, 202111031
	IN_SAMPLE01			*dat;
	
	dat = (IN_SAMPLE01 *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];
	/* 전략전달중 공통(윗부분)을 Set하고 시장 FD값을 설정한다 */
	if ((memcmp(dat->next_flag, "O", 1) == 0) ||		// 1개 Struct
		(memcmp(dat->next_flag, "S", 1) == 0))		// Struct 시작
	{
		Set_Flag = 0;
		/* 사용될 총 종목 수량 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt	= AtoIf(dat->tot_item_cnt, 2);
		/* 사용될 총 시장 수 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].use_market_cnt	= AtoIf(dat->use_market_cnt, 2);
		/* 1회 주문시 주문 수량 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_order_cnt	= AtoIf(dat->tot_order_cnt, 2);
		/* 주문낼 현물 계좌번호 */
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, dat->st_accno, sizeof(dat->st_accno));
		/* 주문낼 파생 게좌번호 */
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].dv_accno, dat->dv_accno, sizeof(dat->dv_accno));
		
		/* 시장별 fd값 설정 */
		InPollCnt = AtoIf(dat->use_market_cnt, 2);
		
// 공통
		for(i = 0; i < InPollCnt; i++)
		{
			memset(fifo_name, 0, sizeof(fifo_name));
			if (memcmp(dat->market_gbn[i], "10", 2) == 0)
				sprintf(fifo_name, "%s/PA/pa_7912_ur1", _FEP_FIFO);
			else
				sprintf(fifo_name, "%s/PA/pa_7%1.1s11_ur1", _FEP_FIFO, dat->market_gbn[i]+1);
			
			Poll[i+2].fd = open(fifo_name, O_RDWR|O_NDELAY);
			Poll[i+2].events = POLLIN;
			if (Poll[i+2].fd < 0)
			{
				Log(FIF_FATAL, "cannot open i{%d] FIFO[%s][%d][%d:%s]",
					i, fifo_name, Poll[i+2].fd, SYS_NO, SYS_STR);
				return_flag = -1;
			}
		}
		Log(USR_OK, "Poll Cnt = [%d]", InPollCnt);
		
		if (memcmp(dat->next_flag, "O", 1) == 0)
			Set_Flag = 1;
		else
		{
			if (return_flag == 0)
				return;
		}
	}
	else
	if ((memcmp(dat->next_flag, "N", 1) == 0) ||		// Struct 계속
		(memcmp(dat->next_flag, "E", 1) == 0))		// Struct 종료
	{
		/* Arry 번호, 순번 */
		arry = AtoIf(dat->Condition.arry_no, 3);
		/* 시장구분 */
		mk_gbn = AtoIf(dat->Condition.market_gbn, 2);
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn = mk_gbn;
		/* 종목seq */
		item_seq = AtoIf(dat->Condition.item_seq, 5);
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq = item_seq;
		/* 종목코드 정합성 체크 */
		if (memcmp(Shm_Risk[0].S_Sise[mk_gbn][item_seq].m_item_cd,
				  dat->Condition.item_code, 12) != 0)
		{
			return_flag = -2;
		}
		memcpy(Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_code,
			   dat->Condition.item_code, 12);
		/*타켓인지 참조인지 여부 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].target_gbn = 
			AtoIf(dat->Condition.target_gbn, 1);
		/* 1회 주문수량 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt = 
			AtoIf(dat->Condition.order_cnt, 8);
		/* 주문타입 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type = 
			AtoIf(dat->Condition.order_type, 1);
		/* 주문가격구분 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn = 
			AtoIf(dat->Condition.order_price_gbn, 1);
		/* 주문가격조건 */
		Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition = 
			AtoIf(dat->Condition.order_price_condition, 1);
			
		if (memcmp(dat->next_flag, "E", 1) == 0)
			Set_Flag = 1;
		else
		{
			if (return_flag == 0)
				return;
		}
	}
	
	/* ********************************************************* */
	/* 정상여부 응답 처리 */
	memset(W_DFmt, 0x20, sizeof(W_DFmt));
	memcpy(&W_DFmt, &R_Fmt, sizeof(BUFF_RW_HEAD) + 20);
	/* 기동 Set 오류 */
	if (return_flag < 0)
	{
		// 종료후 Client에 Error 송신
		memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500120", 6);		/* SEARCH_HEADER(20), Tr Code */
	}
	else
	{
		// Client에 정상기동 (50021) 송신
		memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)], "500110", 6);		/* SEARCH_HEADER(20), Tr Code */
	}
	memset(err_no, 0, sizeof(err_no));
	sprintf(err_no, "%04d", -1*return_flag);
	memcpy(&W_DFmt[sizeof(BUFF_RW_HEAD)+10], err_no, 4);	// SEARCH_HEADER(20), ErrCode(0000:Normal) */
	W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';
	
	rt = F_W(TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log(SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
		Exit_Process();
	}
	
	Log(USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
	/* ********************************************************** */
	
	/* 시장별 시세의 auto_use를 증가시켜준다. 시세신호를 받기위해서 */
	if (Set_Flag == 1 && return_flag == 0) 	/* Client의 Set처리가 정상적으로 끝났으면 */
	{
		for(i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++)
		{
			mk_gbn 	 = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn;
			item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq;
			Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use++;
		}
		
		/* 9001이 Set 한다.
		Shm_Strrg[0].auto_run_flag[OD_SEQ] = 1;
		*/
	}
	else if (return_flag < 0)
	{
		Set_In_Stop(1, 0); 		//감소없이 종료
	}
	
	return;
}	/* End of Set_In_Param */


/***********************************************************
	Function 		: . Set_In_Stop
	Parameters IN 	: . tr_gbn   => 0:500310(정상종료), 1:500410(강제종료)
					: . min_flag => 0:감소없이, 1:감소하고
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_In_Stop (int tr_gbn, int min_flag)
/*--------------------------------------------------------*/
{
	int		i, rt, mk_gbn, item_seq;
	char	err_no[10], fifo_name[128];

	/* ************************************************** */
	/* 정상여부 응답 처리 */
	memset (W_DFmt, 0x20, sizeof (W_DFmt));
	memcpy (&W_DFmt, &R_Fmt, sizeof (BUFF_RW_HEAD)+20);
	if (tr_gbn == 0)
		memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)], "500310", 6);		/* SEARCH_HEADER(20), TrCode (500310:정상종료)	*/
	else
		memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)], "500410", 6);		/* SEARCH_HEADER(20), TrCode (500410:강제종료)	*/
	memset (err_no, 0, sizeof (err_no));
	sprintf (err_no, "%04d", 0);
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)+10], err_no, 4);			/* SEARCH_HEADER(20), ErrCode(0000:Normal)		*/
	W_DFmt[sizeof(BUFF_RW_HEAD) + DATA_SIZE] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,1));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,OD_SEQ+2), OFW(D_K,P_K,OD_SEQ+2,0), rt);
	/* ************************************************** */

	/* 시장별 시세의 auto_use를 증가 시켜준다. 시세신호를 받기위해서 */
	if (min_flag == 1)
	{
		for (i = 0; i < Shm_Strrg[0].SamPle_St01[OD_SEQ].tot_item_cnt; i++)
		{
			mk_gbn	 = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].market_gbn;
			item_seq = Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[i].item_seq;

			if (Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use < 0)
				Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use = 0;
			else
				Shm_Risk[0].S_Sise[mk_gbn][item_seq].auto_use--;
		}
	}

	/* 전략이 종료처리를 위한 Set을 한다 */
	Shm_Strrg[0].auto_run_flag[OD_SEQ] = 0;
	PROC(D_K,P_K).start_status = JOB_END;
	PROC(D_K,P_K).process_status = 9;

	/* Daemon에게 즉각적인 종료요청을 위해 signal 주기 */
	sprintf (fifo_name, "%s/PA/%s", _FEP_FIFO, INFO(D_K).daemon_FIFO_name);
	DFIFD(D_K) = open (fifo_name, O_RDWR | O_NDELAY);

	if (DFIFD(D_K) == -1)
	{
		Log (FIF_FATAL, "cannot open daemon FIFO[%s] {%d:%s}",
			fifo_name, SYS_NO, SYS_STR);
		exit (FAIL);
	}

	write (DFIFD(D_K), "1", 1);
}	/* End of Set_In_Stop () */

/***********************************************************
	Function 		: . Set_Response
	Parameters IN 	: . 
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
void	Set_Response(void)
/*--------------------------------------------------------*/
{
	/* 여기에서 응답/체결을 받고 조건에 맞게 하시면 됩니다. */

	return;
}	/* End of Set_Response () */

/***********************************************************
	Function 		: . Od_Write_Data
	Parameters IN 	: . mk_gbn : 시장구분 (1~10)	1:채권, 2:금융파생
					: . nmc_gbn : 1:신규, 2:정정, 3:취소
					: . p_flag : write falg
                        1 : 일반
                        2 : LP
					: . mm_gbn : 1:매도, 2:매수
					: . arry : 주문낼 대상의 arry(수신받은 종목중 몇번째)
	Parameters OUT 	: .
	Return Code 	: . int
	Comment 		: . 
***********************************************************/
/*--------------------------------------------------------*/
int		Od_Write_Data(int mk_gbn, int nmc_gbn, int p_flag, int mm_gbn, int arry)
/*--------------------------------------------------------*/
{
	int		rt, o_gbn = 0;
	char	m_time[24], str[10];
	char	cdata[30];
	char	tmp[128];

	double	cal_price;

	memset (m_time, 0, sizeof (m_time));
	memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

	/* KRX 주문포맷 초기값을 복사해둔다 */
	memcpy (&Order_St[sizeof (BUFF_RW_HEAD)], Order_Dat, sizeof(KRX_JUMUN_DATA));

	/* KRX주문포맷 기본은 넣었고 필요한 항목을 Update 해준다 */
	if (mk_gbn == 1)			// 채권일반주문
	{
		/* ************************************************************************ */
		/* 변수값 넣기 */

		/* 주문번호 채번 */
		Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO++;		// 파생은 ....Order_No[1]....
		if (Shm_Risk[0].Order_No[0][OD_SEQ].START_JMNO > Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO	||
			Shm_Risk[0].Order_No[0][OD_SEQ].END_JMNO  <= Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO)
		{
			Log (USR_ERROR, "주문번호 사용 영역 FULL!!! jm_no[%ld]", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
			return (-1);
		}

		/* TR Code, 11 여기서는 전략상 신규주문만 넣기 때문에 수정하지 않음
		if (nmc_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10001", 11);
		else if (nmc_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10002", 11);
		else if (nmc_gbn == 3)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+11], "TCHODR10003", 11);	*/

		/* 보드ID도 넣어야 하는데 G1이외의 경우에 거래하면 안되니까 Default로 G1쓴다 */
	
		/* 주문ID, 34부터 */
		memset (tmp, 0, sizeof (tmp));
		sprintf (tmp, "%010ld", Shm_Risk[0].Order_No[0][OD_SEQ].USED_JMNO);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+34], tmp, 10);

		/* 신규라 원주문번호는 넣지 않는다. 44부터
 		   정정,취소는 내가냈던 주문번호를 보관하고 있다가 조건
		   보관은 응답/체결 수신시 하던가 주문낼시 하던가.... 결정
		   내가낸 주문에 대한 미체결관리를 해야 한다는 의미이다. 공유메모리 Struct를 만들어서 하면 된다. 
		if (nmc_gbn == 2 || nmc_gbn == 3)		// 정정, 취소
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+44], xxxxxxxxxx, 10);	*/

		/* 종목코드, 54부터 */
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+54],
				Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_item_cd, 12);

		/* 매도매수 구분, 66 */
		if (mk_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+66], "1", 1);
		else if (mk_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+66], "2", 2);
		else
		{
			Log (USR_ERROR, "매도매수구분 Error!!! mm_gbn[%d]", mm_gbn);
			return (-2);
		}
			
		/* 정정/취소구분, 67 */
		if (nmc_gbn == 1)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "1", 1);
		else if (nmc_gbn == 2)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "2", 1);
		else if (nmc_gbn == 3)
			memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+67], "3", 1);

		/* 계좌번호(현물), 68 */
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+68], Shm_Strrg[0].SamPle_St01[OD_SEQ].st_accno, 12);

		/* 호가수량, 80 */
		memset (tmp, 0, sizeof (tmp));
		sprintf (tmp, "%010d", Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_cnt);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+80], tmp, 10);

		/* 호가가격 , 90 */
		memset (tmp, 0, sizeof (tmp));
		if (mm_gbn == 1)		// 매도 
		{
			if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)	// 1:지정가
			{
				if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)		// 0:매도1호가(자기호가), 1:매수1호가(상대호가)
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
				}
				else
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
				}

/*
				cal_price = Hoga_Change (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn,
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq,
										 0,			// 나머진 0(Default), 주식선물만 사용, 0:코스피, 1:코스닥
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition,
										 cal_price);
*/
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
			}
			else	// 2.시장가, A0의 하한가격으로 주문
			{
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_l_lmt;
			}
		}
		else		// 매수
		{
			if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_type == 1)		//1: 지정가
			{
				if (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_gbn == 0)	// 0:매수1호가(자기호가), 1:매도1호가(상대호가)
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].buy_1_price;
				}
				else
				{
					cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].sell_1_price;
				}
					
/*
				cal_price = Hoga_Change (Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].market_gbn,
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq,
										 0,			// 나머진 0(Default), 주식선물만 사용, 0:코스피, 1:코스닥
										 Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].order_price_condition,
										 cal_price);
*/
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
			}
			else	// 2.시장가, A0의 상한가격으로 주문
			{
				cal_price = Shm_Risk[0].S_Sise[mk_gbn][Shm_Strrg[0].SamPle_St01[OD_SEQ].Condition[arry].item_seq].m_h_lmt;
			}
		}

		sprintf (tmp, "%011ld", (long)cal_price);
		memcpy (&Order_St[sizeof (BUFF_RW_HEAD)+90], tmp, 11);
			
		/* 호가유형코드, 여기서는 지정가로 처리했다. 상/하한가로 주문낼때도 지정가로 상하한가 가격으로 내면 된다. 나머지는 전략에 따라 사용하면 된다. */
		/* 호가조건코드, 여기서는 대상이 아니다. Default(0)사용, 수정필요없음 초기값이 0임	*/
		/* 시장조성자호가구분번호, 여기서는 Default(0) 사용, LP는 1:LP호가로 Set 필요 		*/
		/* 기타등등 전략 및 계좌상태 주문조건등에 따라 나머지 값들도 맞게 처리하면 됨...	*/

		/* ******************************************************************** */
		/* 261Byte의 KRX주문 포맷을 만들고 마지막으로 회원사처리항목 값 설정 */
		/* 회원사처리항목 60바이트중 앞 30바이트는 원장에서 요청한 값을 넣어야 하고, 뒤 30바이트는 아래의 조건에 맞게 Setting해야함 (Client도 동일)	*/
		/* 30			: A(서버자동주문), C(Client에서 낸 주문)				*/
		/* 31, 32		: ST(Strategy)											*/
		/* 33, 34		: 전략번호 01 ~ 99										*/
		/* 35, 36		: 시장구분												*/
		/* 				  1(지수선물) 2(지수옵션) 3(주식선물) 4(주식옵션)	
						  5(유가증권/ELW/ETF/ETN) 6(코스닥)
						  8(KRX300) 9(Kosdaq150 Futures) 10(Kosdaq150 Options)	*/
		/* 37,38,39,40,41 : A0의 Seq번호 Ex) 236 => (00236)						*/
		/* 42, 43		: 계좌번호 Seq											*/
		/* 44           : 0 (Default, 유가증권과 주식선물만 선택 나머진 0)					
						: 유가증권(5)일 경우 => 0(Normal), 1(ELW), 2(ETN), 3(ETF)
						: 주식선물(3)일 경우 => 해당종목이 코스피면 0, 코스닥이면 1	*/
		/* 45           : 스프레드여부, 0:normal, 1:스프레드					*/
		/* 46,47,48,49	: Process Nick Name
						  pa_50101mp => 0101, pa_50203mp => 0203				*/
		/* ******************************************************************** */
 
	}
	else		// 파생시장 
	{
		/* 파생시장에 맞게 KRX전문 채우기(위 현물 참조)	*/
	}

	/* *************************************************************** */
	/* 주문송신 처리 */
	memcpy (&W_Fmt, Order_St, sizeof (BUFF_RW_HEAD) + ODS(D_K,P_K,0));
	W_Fmt.LineFeed[0] = '\n';

	if (mk_gbn == 1 && p_flag == 1)					// 채권일반
	{
		o_gbn = 0;
		rt = DSHM_W (TS_W1_1, (void *)&W_Fmt, 1);
	}
	else									 
	if (mk_gbn == 1 && p_flag == 2)					// 채권LP
	{
		o_gbn = 1;
		rt = DSHM_W (TS_W2_1, (void *)&W_Fmt, 1);
	}
	else											// 금융파생
	{
		o_gbn = 2;
		rt = DSHM_W (TS_W3_1, (void *)&W_Fmt, 1);
	}

	if (rt != 1)
	{
		Log (SAM_FATAL, "shm write fail [%s]", ODN(D_K,P_K,o_gbn));
		return (NOTOK);
	}

	Log (SAM_OK, "file write [%s:%d:%d]", ODN(D_K,P_K,o_gbn), ODW(D_K,P_K,0,o_gbn), rt);

#if 0
	/* Client에 송신 */
	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail [%s]", ODN(D_K,P_K,0));
		return (NOTOK);
	}
		
	Log (USR_OK, "fail write [%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
#endif
	return (OK);
}	/* End of Write_Data() */

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

    read (START_FD, tmp, 1);

    return;
}   /* End of Fifo_Event_Rtn () */

/*************************************************************************
    End of Program (pa_5020_mp.c)
*************************************************************************/
