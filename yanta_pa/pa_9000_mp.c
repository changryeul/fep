#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 전략기동/종료
#	File	: pa_9000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	2048
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1
#define		KRX_PROCNT	4
#define     PLAY_TIME	"0739"
#define     AUTO_PLAY_TIME	"081000"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int					R_Cnt, RR;
char				ApType[10];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_9000_MP (int);
void    Init_Parameters (int);
void	Analyze_Data (void);
void	Start_Client (char *);
void	Write_Auto_Data (int, char *, int);
void	Stop_Client (char *, int);
void	Auto_Stop_Client (char *);
void	Write_Data (int);
void	Write_Auto_Fifo (int);
void	Stop_All_Client (char *, int);
void	Acc_Search (void);
void	Acc_Update (char *);

#define FLAG_FILE "/tmp/pa_9001_mp_rundate.txt"
int is_first_today()
{
	FILE *fp=NULL;
	char buf[32], today[32];
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	sprintf(today, "%04d-%02d-%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
	fp = fopen(FLAG_FILE, "r");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		fclose(fp);

		if (strcmp(buf, today) == 0) {
			return(0);
		}
	}

	fp = fopen(FLAG_FILE, "w");
	if (fp) {
		fprintf(fp, "%s", today);
		fclose(fp);
	}
	return(1);	
}

void
Clear_Pipo()
{
	char cmd[2048], fifo_name[256];
	int  ii, FIFO_fd[MAX_AUTO_PROC];

	if (is_first_today()) {
		Log (USR_OK, "Clear_Pipo OK");
		sprintf(cmd, "/bin/rm -f %s/PA/pa_75*", _FEP_FIFO); 
		system(cmd);
	}
}

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_9000_MP (argc);

	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_9000_MP (int argc)
/*----------------------------------------------------------------------*/
{
	int		rt, DelayFlag;
	char	m_time[24];

	DelayFlag = OFF;
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();

			if (DelayFlag == OFF)
			{
				memset (m_time, 0, sizeof(m_time));
				Get_Time (m_time);

				/* Acc정보 초기화 작업용 Data 요구처리 */
				if (memcmp (m_time, "0900", 4) < 0)		// 9시이후에는 초기화못하게
					Init_Parameters (argc);
Log (USR_OK, "Init_P OK");
				DelayFlag = ON;
			}

			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

			R_Cnt = F_R (PS_R_1, (void *)R_Fmt, READ_MAX);

			if (R_Cnt < 0)
			{
				Log (SAM_FATAL, "cannot read File[%s,%d:%s]",
					IFN(D_K,P_K,0), SYS_NO, SYS_STR);
				sleep (1);
				Exit_Process ();
			}
			else if (R_Cnt == 0)
				break;

			Log (USR_OK, "RD [%s:%d][%d]",
				IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
			Analyze_Data ();
		}

		rt = Poll_File (DATA_TIME);

		if (rt == 1)
			Log (USR_OK, "poll timeout <%d>", INT_SEQ);
		else if (rt == -1)
			continue;
	}
}	/* End of PA_9000_MP ()	*/

/*************************************************************************
	Function        : . Init_Parameters
	Parameters IN   : .
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Init_Parameters (int argc)
/*----------------------------------------------------------------------*/
{
	int     i, j, k;

	RR = 0;
	/* ******************** */
	/* 기초자료 초기화 작업 */
	/* -------------------- */
	/* 시세초기화는 기본dd에서 한다.										*/
	/* 단, 시세에서 주문관련된건 여기서 한다.(auto_use)			*/
	/* 여기서는 shm_memory의 MK_PREMATCH(미체결), RISK(리스크) 처리를 한다. */
	/* ******************************************************************** */

	/* 시장별 거래관련 초기화 */
	/* 내부거래불가종목 초기화 */
#if	0
	for (i = 0; i < SHM_MAX_NOTE; i++)		// 채권KTS
	{
		Shm_Risk[0].S_Sise[0][i].auto_use = 0;
		Shm_Risk[0].S_Sise[0][i].dont_trade = 0;
	}
#endif
	for (i = 0; i < SHM_MAX_DEV_FIF; i++)	// 금융파생
	{
		Shm_Risk[0].S_Sise[RISK_MK_FF][i].auto_use = 0;
		Shm_Risk[0].S_Sise[RISK_MK_FF][i].dont_trade = 0;
	}

	for (i = 0; i < SHM_MAX_DEV_FIF; i++)	// 금융파생(야)
	{
		Shm_Risk[0].S_Sise[RISK_MK_FF_N][i].auto_use = 0;
		Shm_Risk[0].S_Sise[RISK_MK_FF_N][i].dont_trade = 0;
	}

	for (i = 0; i < SHM_MAX_FX; i++)	// FX
	{
		Shm_Risk[0].FX_Sise[0][i].auto_use = 0;
		Shm_Risk[0].FX_Sise[0][i].dont_trade = 0;
	}

	if (Shm_FinFut != NULL)
	{
		for (i = 0; i < SHM_MAX_DEV_FIF; i++)	// 금융파생
		{
			for (j = 0; j < MAX_AUTO_PROC; j++)
			{
				Shm_FinFut[i].auto_use[j] = 0;
			}
		}
		Log (USR_OK, "[파생]: Shm_FinFut auto_use clear");
	}

	if (Shm_FinFut_N != NULL)
	{
		for (i = 0; i < SHM_MAX_DEV_FIF_N; i++)	// 금융파생(야)
		{
			for (j = 0; j < MAX_AUTO_PROC; j++)
			{
				Shm_FinFut_N[i].auto_use[j] = 0;
			}
		}
		Log (USR_OK, "[파생(야)]: Shm_FinFut_N auto_use clear");
	}

	if (Shm_FX != NULL)
	{
		for (i = 0; i < SHM_MAX_FX; i++)	// FX
		{
			for (j = 0; j < MAX_AUTO_PROC; j++)
			{
				Shm_FX[i].auto_use[j] = 0;
			}
		}
		Log (USR_OK, "[FX]: Shm_FX auto_use clear");
	}

	/* 시장별 거래관련 초기화 */

	/* 미체결 내역 초기화 */ 
	for (i = 0; i < RISK_MK_CNT; i++)		
	{
		for (j = 0; j < ACC_NO_CNT; j ++)		
		{
			Shm_Mk_PreMatch[0].MeChe_Cnt[i][j] = 0;

			for (k = 0; k < MAX_MICHE; k ++)		
				Shm_Mk_PreMatch[0].F_MiChe[i][j][k].Jan_Cnt = 0;
		}
	}

	for (j = 0; j < FX_ACC_NO_CNT; j ++)		
	{
		Shm_Mk_PreMatch[0].FX_MeChe_Cnt[0][j] = 0;

		for (k = 0; k < MAX_MICHE; k ++)		
			Shm_Mk_PreMatch[0].FX_MiChe[i][j][k].Jan_Cnt = 0;
	}
	
	/* ************* */
	/* 리스크 초기화 */
	memset (&Shm_Risk[0].ProFit[0][0][0].item_getcnt,  0,
								sizeof (PROFIT) * RISK_MK_CNT * SHM_MAX_DEV_FIF * ACC_NO_CNT);

	/* 메인서버와 백업서버가 있으면 나눠서 사용한다. */
	long		dual_atv_no[2];					// 백업서버 여부

	if ((memcmp ((char *)getenv ("_FEP_DIV"), "REAL1", 5) == 0)	||		// 백업서버이거나 테스트 확인
		(memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0))
	{
		dual_atv_no[0] = 0;
		dual_atv_no[1] = 0;
	}
	else																// DR
	{
		dual_atv_no[0] = MK01_ORDER_NO_BACK_GAP;		// 100,000,000
		dual_atv_no[1] = MK02_ORDER_NO_BACK_GAP;		//      40,000
	}

	/* 파라메타로 주문번호 초기화 여부를 판단해서 처리 */ 
	if (argc == 1 ||      /* 초기화 처리 */
		Shm_Risk[0].Order_No[1][0].START_JMNO != MK02_ORDER_NO_BASE + dual_atv_no[0])
	{
		/* 주문번호 초기화 */
		for (i = 0; i < MAX_AUTO_PROC; i++)		// MAX_AUTO_PROC (40)
		{
			/* 1번 전략서버는 채번 range를 할당받고 10만번미만은 Client가 채번영역으로 사용하게 둔다 */ 
			/* 채권 */                                /* 200,000,000 */       /* 1,000,000 */ 
			Shm_Risk[0].Order_No[0][i].START_JMNO	= MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + dual_atv_no[0];      // 0:200,000,000
			Shm_Risk[0].Order_No[0][i].USED_JMNO	= MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + dual_atv_no[0];      // 0:200,000,000
			Shm_Risk[0].Order_No[0][i].END_JMNO	    = MK01_ORDER_NO_BASE + (MK01_ORDER_NO_GAP*i) + MK01_ORDER_NO_GAP + dual_atv_no[0] - 1;  // 0:200,999,999
			/* 금융파생 */
			Shm_Risk[0].Order_No[1][i].START_JMNO	= MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + dual_atv_no[1];      // 0:200,000,000
			Shm_Risk[0].Order_No[1][i].USED_JMNO	= MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + dual_atv_no[1];      // 0:200,000,000
			Shm_Risk[0].Order_No[1][i].END_JMNO	    = MK02_ORDER_NO_BASE + (MK02_ORDER_NO_GAP*i) + MK02_ORDER_NO_GAP + dual_atv_no[1] - 1;  // 0:200,999,999

			Log (USR_OK, "[파생    ]:i[%d] Start[%ld] END[%ld]", i, Shm_Risk[0].Order_No[0][i].START_JMNO, Shm_Risk[0].Order_No[0][i].END_JMNO);
			Log (USR_OK, "[파생(야)]:i[%d] Start[%ld] END[%ld]", i, Shm_Risk[0].Order_No[1][i].START_JMNO, Shm_Risk[0].Order_No[1][i].END_JMNO);
		}
	}

	/* 받아오는 메모리의 계좌정보 초기화 */
	for (i = 0; i < RISK_MK_CNT; i++)
	{
		for (j = 0; j < ACC_NO_CNT; j ++)
		{
			/* 1회 한도 설정값 */
			memcpy (Shm_Risk[0].O_M_Fund[i][j].qty_gbn,		" ", 1);
			memcpy (Shm_Risk[0].O_M_Fund[i][j].money_gbn,	" ", 1);
			memcpy (Shm_Risk[0].O_M_Fund[i][j].tick_gbn,	" ", 1);
			Shm_Risk[0].O_M_Fund[i][j].qty = 0;
			Shm_Risk[0].O_M_Fund[i][j].money = 0;
			Shm_Risk[0].O_M_Fund[i][j].tick = 0;

			/* 누적 한도 설정값 */
			memcpy (Shm_Risk[0].T_M_Fund[i][j].do_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].do_money_gbn," ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].su_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_M_Fund[i][j].su_money_gbn," ", 1);
			Shm_Risk[0].T_M_Fund[i][j].do_cnt	= 0;
			Shm_Risk[0].T_M_Fund[i][j].do_money	= 0;
			Shm_Risk[0].T_M_Fund[i][j].su_cnt	= 0;
			Shm_Risk[0].T_M_Fund[i][j].su_money	= 0;

			/* 누적 한도 사용값 */
			memcpy (Shm_Risk[0].T_S_Fund[i][j].do_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].do_money_gbn," ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].su_cnt_gbn,	" ", 1);
			memcpy (Shm_Risk[0].T_S_Fund[i][j].su_money_gbn," ", 1);
			Shm_Risk[0].T_S_Fund[i][j].do_cnt	= 0;
			Shm_Risk[0].T_S_Fund[i][j].do_money	= 0;
			Shm_Risk[0].T_S_Fund[i][j].su_cnt	= 0;
			Shm_Risk[0].T_S_Fund[i][j].su_money	= 0;
		}
	}

	/* 잔고정보 초기화 */
	for (i = 0; i < RISK_MK_CNT; i++)
	{
		for (j = 0; j < SHM_MAX_NOTE; j ++)
		{
			for (k = 0; k < ACC_NO_CNT; k ++)
			{
				Shm_Risk[0].ProFit[i][j][k].item_getcnt		= 0;
				Shm_Risk[0].ProFit[i][j][k].item_get_avg_price	= 0;
//				Shm_Risk[0].ProFit[i][j][k].item_getmoney	= 0;
//				Shm_Risk[0].ProFit[i][j][k].item_tot_sugum	= 0;
//				Shm_Risk[0].ProFit[i][j][k].item_cha_sugum	= 0;
//				Shm_Risk[0].ProFit[i][j][k].item_borrow_cnt = 0;
//				Shm_Risk[0].ProFit[i][j][k].item_totsu_che	= 0;
//				Shm_Risk[0].ProFit[i][j][k].item_totdo_che	= 0;

				Shm_Risk[0].ProFit[i][j][k].item_su_michecnt	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_su_miche_gum	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_do_michecnt	= 0;
				Shm_Risk[0].ProFit[i][j][k].item_do_miche_gum	= 0;
			}	
		}	
	}	

	/* 배치정보 수신,처리건수 초기화 */
	for (i = 0; i < 7; i++)
	{
		Shm_Risk[0].Batch_Cnt[i].W_cnt = 0;
		Shm_Risk[0].Batch_Cnt[i].R_cnt = 0;
	}

	return;
}	/* End of Init_Parameters ()    */

/*************************************************************************
	Function		: . Analyze_Data
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . analyze and divide data
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt;
	char			m_time[24];
	SEARCH_HEADER	*i_hd;

	memset (m_time, 0, sizeof (m_time));
	memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));

	for (i = 0; i < R_Cnt; i++)
	{
		i_hd = (SEARCH_HEADER *)R_Fmt[i].Data;

		if (memcmp (i_hd->TrCode, "500100", 6) == 0)
		{	/* 개별 Client전략 기동	*/
			if ((memcmp (i_hd->ApType_Cd, "7010", 4) >= 0)	&&
				(memcmp (i_hd->ApType_Cd, "7999", 4) <= 0))
			{
				memcpy (&W_Fmt,	&R_Fmt[i], sizeof (FILE_BUFF_FORMAT));
				Start_Client (R_Fmt[i].Data);
			}
			else
			{	
				Log (USR_ERROR, "ApType_Cd error Input[%50.50s]", i_hd->ApType_Cd);
				memset (&W_Fmt, 0x20, sizeof (FILE_BUFF_FORMAT));
				memcpy (W_Fmt.Data, R_Fmt[i].Data,	sizeof (SEARCH_HEADER));
				memcpy (W_Fmt.Data, "500120",	6);
				Write_Data (-1);
			}
			
		}
#if 0
		else if ((memcmp (i_hd->TrCode, "500120", 6) == 0)	||
				 (memcmp (i_hd->TrCode, "500210", 6) == 0)	||
				 (memcmp (i_hd->TrCode, "500220", 6) == 0)	)
		{	/* 개별 Client전략 종료	*/
			memset (Cli_handler, ' ', sizeof (Cli_handler));
			memcpy (Cli_handler, &R_Fmt[i].DataHeader[16], 4);
			memcpy (Tcp_Data_Head.LineFlag, &R_Fmt[i].DataHeader[16], 4);
			Stop_Client (R_Fmt[i].Data, AtoIf(i_hd->TrCode, 6));
		}
		else if (memcmp (i_hd->TrCode, "500310", 6) == 0)
		{	/* 특정계좌의 전체 Client전략 종료(세션이상등)	*/
			Stop_All_Client (R_Fmt[i].Data, 1);
		}
		else if (memcmp (i_hd->TrCode, "500320", 6) == 0)
		{	/* 특정계좌의 전체 Client전략 종료(한도체크등)	*/
			Stop_All_Client (R_Fmt[i].Data, 2);
		}
		else if (memcmp (i_hd->TrCode, "500330", 6) == 0)
		{	/* Auto 개별 API 종료 통보 */
			Auto_Stop_Client (R_Fmt[i].Data);
		}
		else if (memcmp (i_hd->TrCode, "500999", 6) == 0)
		{	/* Account정보 초기화 */
			Acc_Update (R_Fmt[i].Data);
		}
#endif
		else if (memcmp (i_hd->TrCode, "500", 3) == 0)
		{
//			memcpy (&W_Fmt,	&R_Fmt[i], sizeof (FILE_BUFF_FORMAT));

			/* 전략에도 전달 추가 */
			int		target1;

			target1 = (AtoIf (&i_hd->ApType_Cd[1], 2) - 1) * 10;
			target1 = (AtoIf (&i_hd->ApType_Cd[3], 2))     + target1;
			rt = DSHM_W (target1*10, (void *)&R_Fmt, 1);     // 자동Process

			if (rt != 1)
			{
				SLog (SAM_FATAL, "DSHM write fail[%d]", target1*10);
				Exit_Process ();
			}
			Log (USR_OK, "DSHM write OK [%s]", R_Fmt);
		}
		else										/* TR code error	*/
		{	
			Log (USR_ERROR, "TR code error Input[%50.50s]", i_hd->TrCode);
			memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
			memcpy (W_Fmt.Data, R_Fmt[i].Data,	sizeof (SEARCH_HEADER));
			memcpy (W_Fmt.Data, "500120",	6);
			Write_Data (-1);
		}

		Add_Count (PS_R_1, 1);
		INT_SEQ ++;
		Set_TR_Time ();
	}

	return;
}	/* End of Analyze_Data ()	*/

/*************************************************************************
	Function        : . Start_Client
	Parameters IN   : . p_buf	: input data
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . start a client process
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Start_Client (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int						pk, w_flag, i, rt, flag, bin_type, m;
	int						auto_f_tcnt, auto_f_scnt, auto_m_tcnt, auto_m_scnt;
	char					pname[12], cmd[256], m_time[20], chk_proc[20];
	char					st_name[20], tg_name[10];
	int						search_seq, get_proc;
	SEARCH_HEADER			*i_hd;

	memset (m_time, 0, sizeof(m_time));
	Get_Time (m_time);

	i_hd = (SEARCH_HEADER *)p_buf;
	memset (st_name, 0, sizeof (st_name));
	sprintf(st_name, "pa_%4.4s_mp", i_hd->ApType_Cd);

	/* Process Name Search */
	get_proc = 0;
	search_seq = RR;
	for (i = 0; i < MAX_AUTO_PROC; i++)
	{
		if (search_seq >= MAX_AUTO_PROC)
			search_seq = 1;
		else
			search_seq++;

		memset (pname, 0, sizeof (pname));
		if (search_seq%10 == 0)
			sprintf (pname, "%2.2s_70%01d%02dmp", _SubSystem_Name, search_seq/10, 10);
		else
			sprintf (pname, "%2.2s_70%01d%02dmp", _SubSystem_Name, search_seq/10+1, search_seq%10);

		for (pk = 0; pk < DAEMON(D_K).p_count; pk ++)
		{
			if (memcmp (PROC(D_K,pk).process_id, pname, 10) == 0)
			{
				if (PROC(D_K,pk).process_status == 1)
//					continue;
					break;
				else 
				{
					get_proc = 1;
					/* Edit of Input File Count */
					IDR(D_K,pk,0,0) = IDW(D_K,pk,0,0);
					break;
				}
			}

			if (pk == DAEMON(D_K).p_count - 1)
			{
				Log (USR_ERROR, "Not Search process Name[%s] p_count[%d] pk[%d]", pname, DAEMON(D_K).p_count, pk);
				memcpy (W_Fmt.Data, "500120",	6);
				Write_Data (-1);
				return;
			}
		}
	
		if (get_proc == 0 && i >= MAX_AUTO_PROC-1)
		{
			Log (USR_ERROR, "Fail Auto Run, process Full Run input[%50.50s]", i_hd->TrCode);
			memcpy (W_Fmt.Data, "500120",	6);
			Write_Data (-1);
			return;
		}
	

		if (get_proc > 0)
		{
			RR = search_seq;
			break;
		}
	}

	Log (USR_OK, "cp [%s/%s %s/%s]", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);

	sprintf (cmd, "cp %s/%s %s/%s", (char *)getenv("_P_BIN"), st_name, (char *)getenv("_P_BIN"), pname);

	rt = system (cmd);

	if (rt < 0 && SYS_NO != 10)
	{
		Log (SYS_FATAL, "system call fail[%s] {%d:%s}",
			cmd, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	PROC(D_K,pk).start_status = JOB_INIT;
	PROC(D_K,pk).process_status = 1;
	PROC(D_K,pk).process_no = 0;
	write (DTART_FD, "1", 1);

	memcpy (W_Fmt.Data, "500110",		6);

	memset (tg_name, 0, sizeof (tg_name));
	sprintf (tg_name, "%5.5s", &pname[3]);
	memcpy (&W_Fmt.Data[14], tg_name,	5);
	Write_Data (1);

	/* 전략에도 전달 추가 */
	int		target;

	target = (AtoIf (&tg_name[1], 2) - 1) * 10;
	target = (AtoIf (&tg_name[3], 2))     + target;
	rt = DSHM_W (target*10, (void *)&R_Fmt, 1);     // 자동Process

	if (rt != 1)
	{
		SLog (SAM_FATAL, "DSHM write fail[%d]", target*10);
		Exit_Process ();
	}

	return;
}	/* End of Start_Client ()	*/

/**************************************************************************
	Function		: . Write_Data
	Parameters IN	: . p_flag	: write flag
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . write data to file
**************************************************************************/
/*-----------------------------------------------------------------------*/
void	Write_Data (int p_flag)
/*-----------------------------------------------------------------------*/
{
	int		rt;

	W_Fmt.LineFeed[0] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]", 
		OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

	return;
}	/* End of Write_Data ()	*/

/*************************************************************************
	End of Program (pa_9000_mp.c)
*************************************************************************/

