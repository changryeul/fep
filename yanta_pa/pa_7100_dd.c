#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 현물시세수신 DD
#	File	: pa_7100_dd.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A7102
#define     DATA_SIZE       700
#elif defined A7103 || A7203 || A7303
#define     DATA_SIZE       100
#elif defined A7201 || A7301
#define     DATA_SIZE       1400
#elif defined A7202 || A7302
#define     DATA_SIZE       500
#elif defined A7181 || A7801
#define     DATA_SIZE       1200
#elif defined A7182 || A7802
#define     DATA_SIZE       350
#endif

#include    "buf_struct.h"
/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define            DATA_TIME       60 * 1000
/* 시세 Seq 위치 Size */
#define		S_H_SIZE		5						/* TR(5) 			*/
/* 기본마스터 Seq 위치 Size */
#define		M_H_SIZE		27	/* TR(5) + SEQ(8) + JCNT(6) + DATE(8) 	*/
/* HONG 시세 종목코드 위치 Size */
#define		I_H_SIZE		17	/* TR(5) + SEQ(8) + BID(2) + SID(2) 	*/

#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#define		DATA_RD			F_R(PS_R_1, (void *)R_Fmt, 1);
#define		ADD_CNT			Add_Count(PS_R_1, 1);
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define     IN_NAME			IDN(D_K,P_K,0)
#define		DATA_RD			DSHM_R(PS_R_1, (void *)R_Fmt, 1);
#define		ADD_CNT			Dshm_Add_Count (PS_R_1, 1)
#endif

#if defined A7181||A7182||A7102||A7103||A7203||A7801||A7802
KS_NOTE_EXPCODE		Key;
#elif defined A7201 || A7202 || A7203 || A7301 || A7302 || A7303
KS_EXPCODE			Key;
#endif

#if defined A7201 || A7202 || A7203
#define	RISK_MK_IDX	RISK_MK_FF
#elif defined A7301 || A7302 || A7303
#define	RISK_MK_IDX	RISK_MK_FF_N
#endif

int			Che_Gbn, Idx;
int			FIFO_fd[MAX_AUTO_PROC];
char		ApType[10];
char		d_time[18];
FILE_BUFF_FORMAT		W_Fmt;
BUFF_RW_HEAD        	f_head;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7100_DD (void);
void	Set_Sise (char *);
void	Write_Read_Fifo (int, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7100_DD ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7100_DD (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt, len, R_Cnt;
	char 			m_time[24];
	char			fifo_name[100], bumun[4];
	char			W2_Fmt[2048];
	FILE_BUFF_FORMAT    R_Fmt[1];

	Get_DateMilliTime (d_time);

/* 기초정보 재기동시 초기화 작업 */
#if defined A7201
	Shm_FinFut[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[RISK_MK_IDX] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_DEV_FIF; i++)
	{
		memset (Shm_FinFut[i].A0.tr_gbn, 0, sizeof (CO_A001F));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].D_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_DEV_FIF);
#elif defined A7301
	Shm_FinFut_N[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[RISK_MK_IDX] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_DEV_FIF_N; i++)
	{
		memset (Shm_FinFut_N[i].A0.tr_gbn, 0, sizeof (CO_A001F));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].D_N_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_DEV_FIF_N);
#elif defined A7181
	Shm_Note[2].total_item_cnt = 0;	// RDS01의 총 종목수량은 KTS의 [2]에서 관리한다.

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_RDS01; i++)
	{
		memset (Shm_Rds01[i].seq_no, 0, sizeof (CO_A001_RDS01));
	}

	/* Key 초기화 , 미사용
	memset (Shm_Item[0].D_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_DEV_FIF);	*/
#elif defined A7182
	if (Shm_Note[2].total_item_cnt <= 0)
	{
		Log (USR_OK, "채권종목정보 배치처리 전입니다. 60초 sleep ");
		sleep (60);
		return;
	}
	else
	{
		sleep (60);		// RDS01 처리이후 60초대기후(처리할 시간을 주자) 처리
	}

	Shm_Note[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[0] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크, 미사용

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_NOTE; i++)
	{
		memset (Shm_Note[i].A0.seq_no, 0, sizeof (CO_A001_RDS02));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].N_Key,	0,	sizeof (KS_NOTE_EXPCODE) * SHM_MAX_NOTE);
#endif
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

/* ************************************************************************ */
/* 전략기동중인 시장+종목을 수신했을때 전략에 전달용(Signal용)		*/
/* ************************************************************************ */
#if defined A7202||A7302
	for (i = 0; i < MAX_AUTO_PROC; i++)
	{
		memset (fifo_name, 0, sizeof(fifo_name));
		sprintf (fifo_name, "%s/PA/pa_75%1d%02dmp1", _FEP_FIFO, i/10+1, i%10+1);

		/* 전략전달용 FIFO */
		FIFO_fd[i] = open (fifo_name, O_RDWR|O_NDELAY);
		if (FIFO_fd[i] < 0)
			SLog (FIF_FATAL, "cannot open FIFO[%s][%d][%d:%s]", fifo_name, FIFO_fd, SYS_NO, SYS_STR);
		Log (USR_OK, "fifo_name[%s][%d] FIFO_fd[%d]", fifo_name, strlen(fifo_name), FIFO_fd[i]);
	}
#endif

	sleep(3);

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (WR_CNT > RD_CNT)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

			R_Cnt = DATA_RD;
			if (R_Cnt < 0)
			{
				Log (SAM_FATAL, "cannot read file[%s,%d:%s]",
					IN_NAME, SYS_NO, SYS_STR);
				sleep (1);
				Exit_Process ();
			}
			else if (R_Cnt == 0)
				break;

			Che_Gbn = Idx = 0;
			Set_Sise (R_Fmt[0].Data);

			/* 자동전략 처리위한 전달 */
			if (Che_Gbn > 0)
			{
				for (i = 0; i < MAX_AUTO_PROC; i++)
				{
					/* 기동중인 자동이 해당종목을 설정했을때 전달 */
#if defined A7202
					if (Shm_FinFut[Idx].auto_use[i] != 0)
#elif defined A7302
					if (Shm_FinFut_N[Idx].auto_use[i] != 0)
#endif
					{
						Write_Read_Fifo (1, i);
					}
				}
			}

			Set_TR_Time ();
			INT_SEQ ++;

			ADD_CNT;
		}

        rt = Poll_File (DATA_TIME);

        if (rt == 1)
            Log (USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;
	}

	return;
}	/* End of PA_7100_DD ()	*/

/*************************************************************************
	Function		: . Set_Sise
	Parameters IN	: . p_buf	: received data
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Sise (char *p_buf)
/*----------------------------------------------------------------------*/
{
	int		ii, s_k, idx, rt, sign;
	char	m_time[24], TrCode[10];

    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);

#if defined A7201
	if (memcmp (p_buf, "A006F", 5) == 0)
    {

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[M_H_SIZE], "999999999999", sizeof (Shm_FinFut[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[RISK_MK_IDX] = Shm_FinFut[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_FinFut[0].total_item_cnt; ii++)
            {
				/* 현재가, 11자리, 첫째자리는 부호 */
				if (memcmp (Shm_FinFut[ii].A0.base_prc, "-", 1) == 0)
					sign = -1;
				else
					sign = 1;
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].crprc = sign * (double)AtoDf (&Shm_FinFut[ii].A0.base_prc[1], sizeof (Shm_FinFut[0].A0.base_prc)-1);

				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* *************************************** */
				/* 기준가격, 소수점으로 수신, 1번은 부호 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_stdard_price = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].crprc;
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_item_cd,
					Shm_FinFut[ii].A0.item_code, sizeof (Shm_FinFut[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_item_stat,
					Shm_FinFut[ii].A0.trade_susp_yn, sizeof (Shm_FinFut[0].A0.trade_susp_yn));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
//				memcpy (Shm_Risk[0].S_Sise[1][ii].item_stat, " ", 1);
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[1][ii].nb_shares_stock_1per,
					&Shm_FinFut[ii].A0.listed_stock[3], 10);	*/
				/* 지정가호가조건코드, 지정가호가취소조건코드 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_2 = AtoIf (&Shm_FinFut[ii].A0.lim_ord_cancel_cond_cd[4], 1);
				/* 조건부지정가 호가조건코드, 조건부지정가호가취소조건코드 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_I = AtoIf (&Shm_FinFut[ii].A0.cond_lim_ord_cancel_cond_cd[4], 1);
				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_T = AtoIf (&Shm_FinFut[ii].A0.mkt_ord_cancel_cond_cd[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_W = AtoIf (&Shm_FinFut[ii].A0.best_lim_ord_cancel_cond_cd[4], 1); 
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_1 = AtoIf (&Shm_FinFut[ii].A0.mkt_ord_cancel_cond_cd[4], 1);	*/
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_X = AtoIf (&Shm_FinFut[ii].A0.best_lim_ord_cancel_cond_cd[4], 1);	*/

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_max_qty = (double)AtoDf(&Shm_FinFut[ii].A0.up_qty[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_clearance_gbn, Shm_FinFut[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_start_price_gbn, Shm_FinFut[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_top_price = (double)AtoDf(Shm_FinFut[ii].A0.high_bid, 11);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_lowest_price = (double)AtoDf(Shm_FinFut[ii].A0.low_bid, 11);	*/

				/* 가격제한 최종단계 */
				if (memcmp (Shm_FinFut[ii].A0.prc_lmt_finl_stg, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg1_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg1_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg1_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg1_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg1_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg1_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_FinFut[ii].A0.prc_lmt_finl_stg, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg2_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg2_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg2_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg2_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg2_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg2_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_FinFut[ii].A0.prc_lmt_finl_stg, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg3_up[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg3_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg3_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut[ii].A0.prc_lmt_stg3_lo[1], sizeof (Shm_FinFut[0].A0.prc_lmt_stg3_up)-1);
					if (memcmp (Shm_FinFut[ii].A0.prc_lmt_stg3_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				/* 승수 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_multiplier = AtoIf (&Shm_FinFut[ii].A0.trade_mult[1],		sizeof (Shm_FinFut[0].A0.trade_mult)-1);
				/* 행사가격 18자리중 12자리(소수점2자리)까지 처리함, 2025 확인필요. 이전에는 17이였음 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price = AtoDf (&Shm_FinFut[ii].A0.strike_prc[6],	sizeof (Shm_FinFut[0].A0.strike_prc)-6);
				if (memcmp (Shm_FinFut[ii].A0.strike_prc, "-", 1) == 0)
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price * -1;
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_closeday, Shm_FinFut[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[1][ii].basic_asset_price, Shm_FinFut[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_group_id, Shm_FinFut[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].D_Key[ii].idx = ii;
                memcpy (Shm_Item[0].D_Key[ii].expcode, Shm_FinFut[ii].A0.item_code,
                                                    sizeof (Shm_FinFut[0].A0.item_code));
            }
            qsort (Shm_Item[0].D_Key, Shm_FinFut[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
		else
		{
			idx = AtoIf (&p_buf[S_H_SIZE], sizeof (Shm_FinFut[0].A0.seq_no));
			memcpy (Shm_FinFut[idx].A0.tr_gbn, p_buf, sizeof (CO_A001F));

			if (Shm_FinFut[0].total_item_cnt < idx) Shm_FinFut[0].total_item_cnt = idx;
		}
    }
#elif defined A7301
	if (memcmp (p_buf, "A006V", 5) == 0)
    {
        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[M_H_SIZE], "999999999999", sizeof (Shm_FinFut_N[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[RISK_MK_IDX] = Shm_FinFut_N[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_FinFut_N[0].total_item_cnt; ii++)
            {
				/* 현재가, 11자리, 첫째자리는 부호 */
				if (memcmp (Shm_FinFut_N[ii].A0.base_prc, "-", 1) == 0)
					sign = -1;
				else
					sign = 1;
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].crprc = sign * (double)AtoDf (&Shm_FinFut_N[ii].A0.base_prc[1], sizeof (Shm_FinFut_N[0].A0.base_prc)-1);

				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* *************************************** */
				/* 기준가격, 소수점으로 수신, 1번은 부호 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_stdard_price = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].crprc;
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_item_cd,
					Shm_FinFut_N[ii].A0.item_code, sizeof (Shm_FinFut_N[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_item_stat,
					Shm_FinFut_N[ii].A0.trade_susp_yn, sizeof (Shm_FinFut_N[0].A0.trade_susp_yn));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
//				memcpy (Shm_Risk[0].S_Sise[1][ii].item_stat, " ", 1);
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[1][ii].nb_shares_stock_1per,
					&Shm_FinFut[ii].A0.listed_stock[3], 10);	*/
				/* 지정가호가조건코드, 지정가호가취소조건코드 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_2 = AtoIf (&Shm_FinFut_N[ii].A0.lim_ord_cancel_cond_cd[4], 1);
				/* 조건부지정가 호가조건코드, 조건부지정가호가취소조건코드 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_I = AtoIf (&Shm_FinFut_N[ii].A0.cond_lim_ord_cancel_cond_cd[4], 1);
				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_T = AtoIf (&Shm_FinFut_N[ii].A0.mkt_ord_cancel_cond_cd[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_cds_gbn_W = AtoIf (&Shm_FinFut_N[ii].A0.best_lim_ord_cancel_cond_cd[4], 1); 
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_1 = AtoIf (&Shm_FinFut[ii].A0.mkt_ord_cancel_cond_cd[4], 1);	*/
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_X = AtoIf (&Shm_FinFut[ii].A0.best_lim_ord_cancel_cond_cd[4], 1);	*/

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_max_qty = (double)AtoDf(&Shm_FinFut_N[ii].A0.up_qty[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_clearance_gbn, Shm_FinFut[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_start_price_gbn, Shm_FinFut[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_top_price = (double)AtoDf(Shm_FinFut[ii].A0.high_bid, 11);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_lowest_price = (double)AtoDf(Shm_FinFut[ii].A0.low_bid, 11);	*/

				/* 가격제한 최종단계 */
				if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_finl_stg, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg1_up[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg1_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg1_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg1_lo[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg1_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg1_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_finl_stg, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg2_up[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg2_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg2_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg2_lo[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg2_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg2_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_finl_stg, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg3_up[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg3_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg3_up, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = (double)AtoDf (&Shm_FinFut_N[ii].A0.prc_lmt_stg3_lo[1], sizeof (Shm_FinFut_N[0].A0.prc_lmt_stg3_up)-1);
					if (memcmp (Shm_FinFut_N[ii].A0.prc_lmt_stg3_lo, "-", 1) == 0)
						Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_l_lmt * -1;
				}
				/* 승수 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_multiplier = AtoIf (&Shm_FinFut_N[ii].A0.trade_mult[1],		sizeof (Shm_FinFut_N[0].A0.trade_mult)-1);
				/* 행사가격 18자리중 12자리(소수점2자리)까지 처리함, 2025 확인필요. 이전에는 17이였음 */
				Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price = AtoDf (&Shm_FinFut_N[ii].A0.strike_prc[6],	sizeof (Shm_FinFut_N[0].A0.strike_prc)-6);
				if (memcmp (Shm_FinFut_N[ii].A0.strike_prc, "-", 1) == 0)
					Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price = Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_striking_price * -1;
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[RISK_MK_IDX][ii].m_closeday, Shm_FinFut_N[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[1][ii].basic_asset_price, Shm_FinFut[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_group_id, Shm_FinFut[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].D_N_Key[ii].idx = ii;
                memcpy (Shm_Item[0].D_N_Key[ii].expcode, Shm_FinFut_N[ii].A0.item_code,
                                                    sizeof (Shm_FinFut_N[0].A0.item_code));
            }
            qsort (Shm_Item[0].D_N_Key, Shm_FinFut_N[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
		else
		{
			idx = AtoIf (&p_buf[S_H_SIZE], sizeof (Shm_FinFut_N[0].A0.seq_no));
			memcpy (Shm_FinFut_N[idx].A0.tr_gbn, p_buf, sizeof (CO_A001F));

			if (Shm_FinFut_N[0].total_item_cnt < idx) Shm_FinFut_N[0].total_item_cnt = idx;
		}
    }
#elif defined A7102 || A7103
    /* 수신시세 => Memory 시세에 Set */
    if (memcmp (p_buf, "A301K", 5) == 0)
    {
        Che_Gbn = 1;
        /* Idx 만드는 방법 */
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].A3.seq_no));
        memcpy (Shm_Note[Idx].A3.tr_gbn, p_buf,         sizeof (CO_A301K));

        /* 공용현재가_체결가, 0:0만(부호) 7.2 */
        Shm_Risk[0].S_Sise[0][Idx].crprc =
            (double)AtoDf (Shm_Note[Idx].A3.crprc,      sizeof (Shm_Note[0].A3.crprc));
    }
    else if (memcmp (p_buf, "G701K", 5) == 0)
    {
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].G7.seq_no));
        Shm_Note[Idx].HogaLastGbn = 0;
        memcpy (Shm_Note[Idx].G7.tr_gbn, p_buf,         sizeof (CO_G701K));

        /* 공용 현재가 */
        Shm_Risk[0].S_Sise[0][Idx].crprc =
            (double)AtoDf (Shm_Note[Idx].G7.crprc,      sizeof (Shm_Note[0].G7.crprc));
        /* 공용 매도1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
            (double)AtoDf (Shm_Note[Idx].G7.ask1_price, sizeof (Shm_Note[0].G7.ask1_price));
        /* 공용 매수1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].buy_1_price =
            (double)AtoDf (Shm_Note[Idx].G7.bid1_price, sizeof (Shm_Note[0].G7.bid1_price));
    }
    else if (memcmp (p_buf, "B601K", 5) == 0)
    {
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_Note[0].B6.seq_no));
        Shm_Note[Idx].HogaLastGbn = 1;
        memcpy (Shm_Note[Idx].B6.tr_gbn, p_buf,         sizeof (CO_B601K));

        /* 공용 매도1호가가격 */
        Shm_Risk[0].S_Sise[0][Idx].sell_1_price =
            (double)AtoDf (Shm_Note[Idx].B6.ask1_price, sizeof (Shm_Note[0].B6.ask1_price));
        /* 공용 매수1호가가격 */
        Shm_Risk[0].S_Sise[1][Idx].buy_1_price =
            (double)AtoDf (Shm_Note[Idx].B6.bid1_price, sizeof (Shm_Note[0].B6.bid1_price));
    }
    else if (memcmp (p_buf, "M401K", 5) == 0)
    {
        memcpy (Shm_Note[0].M4.tr_gbn, p_buf,           sizeof (CO_M401K));
    }
    else if (memcmp (p_buf, "A701K", 5) == 0)
    {
        memcpy (Shm_Note[0].A7.tr_gbn, p_buf,           sizeof (CO_A701A));
    }
    else
    {
        Log (USR_OK, "Other TrCode[%s][%d]", p_buf,     sizeof(p_buf));
        Che_Gbn = -1;
    }
#elif defined A7202 || A7203
    /* 수신시세 => Memory 시세에 Set */
    if (memcmp (p_buf, "A306F", 5) == 0)
    {
#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			memcpy (Shm_FinFut[Idx].A3.tr_gbn, p_buf,       sizeof (CO_A301F));

			/* 공용 현재가 */
			if (memcmp (Shm_FinFut[Idx].A3.crprc, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].crprc =
				sign *
				(double)AtoDf (&Shm_FinFut[Idx].A3.crprc[1],sizeof (Shm_FinFut[0].A3.crprc) - 1);
			/* 공용 실시간상한가가격 */
			if (memcmp (Shm_FinFut[Idx].A3.dyn_upper_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_hprc =
				(double)AtoDf (&Shm_FinFut[Idx].A3.dyn_upper_limit[1],
															sizeof (Shm_FinFut[0].A3.dyn_upper_limit) - 1);
			/* 공용 실시간하한가가격 */
			if (memcmp (Shm_FinFut[Idx].A3.dyn_lower_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_lprc =
				(double)AtoDf (&Shm_FinFut[Idx].A3.dyn_lower_limit[1],
															sizeof (Shm_FinFut[0].A3.dyn_lower_limit) - 1);
		}
    }
    else if (memcmp (p_buf, "G706F", 5) == 0)
    {
		CO_G701F *dat = (CO_G701F *)p_buf;

#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			Shm_FinFut[Idx].HogaLastGbn = 0;
			memcpy(Shm_FinFut[Idx].G7.tr_gbn, p_buf,     sizeof(CO_G701F));

			memcpy(Shm_FinFut[Idx].B6.ask1_price       , dat->ask1_price       , sizeof(dat->ask1_price  ));
			memcpy(Shm_FinFut[Idx].B6.bid1_price       , dat->bid1_price       , sizeof(dat->bid1_price  ));

			memcpy(Shm_FinFut[Idx].B6.ask1_qty         , dat->ask1_qty         , sizeof(dat->ask1_qty    ));
			memcpy(Shm_FinFut[Idx].B6.bid1_qty         , dat->bid1_qty         , sizeof(dat->bid1_qty    ));

			memcpy(Shm_FinFut[Idx].B6.ask1_ord_qty     , dat->ask1_ord_qty     , sizeof(dat->ask1_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.bid1_ord_qty     , dat->bid1_ord_qty     , sizeof(dat->bid1_ord_qty));

			memcpy(Shm_FinFut[Idx].B6.ask2_price       , dat->ask2_price       , sizeof(dat->ask2_price  ));
			memcpy(Shm_FinFut[Idx].B6.bid2_price       , dat->bid2_price       , sizeof(dat->bid2_price  ));

			memcpy(Shm_FinFut[Idx].B6.ask2_qty         , dat->ask2_qty         , sizeof(dat->ask2_qty    ));
			memcpy(Shm_FinFut[Idx].B6.bid2_qty         , dat->bid2_qty         , sizeof(dat->bid2_qty    ));

			memcpy(Shm_FinFut[Idx].B6.ask2_ord_qty     , dat->ask2_ord_qty     , sizeof(dat->ask2_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.bid2_ord_qty     , dat->bid2_ord_qty     , sizeof(dat->bid2_ord_qty));

			memcpy(Shm_FinFut[Idx].B6.ask3_price       , dat->ask3_price       , sizeof(dat->ask3_price  ));
			memcpy(Shm_FinFut[Idx].B6.bid3_price       , dat->bid3_price       , sizeof(dat->bid3_price  ));

			memcpy(Shm_FinFut[Idx].B6.ask3_qty         , dat->ask3_qty         , sizeof(dat->ask3_qty    ));
			memcpy(Shm_FinFut[Idx].B6.bid3_qty         , dat->bid3_qty         , sizeof(dat->bid3_qty    ));

			memcpy(Shm_FinFut[Idx].B6.ask3_ord_qty     , dat->ask3_ord_qty     , sizeof(dat->ask3_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.bid3_ord_qty     , dat->bid3_ord_qty     , sizeof(dat->bid3_ord_qty));

			memcpy(Shm_FinFut[Idx].B6.ask4_price       , dat->ask4_price       , sizeof(dat->ask4_price  ));
			memcpy(Shm_FinFut[Idx].B6.bid4_price       , dat->bid4_price       , sizeof(dat->bid4_price  ));

			memcpy(Shm_FinFut[Idx].B6.ask4_qty         , dat->ask4_qty         , sizeof(dat->ask4_qty    ));
			memcpy(Shm_FinFut[Idx].B6.bid4_qty         , dat->bid4_qty         , sizeof(dat->bid4_qty    ));

			memcpy(Shm_FinFut[Idx].B6.ask4_ord_qty     , dat->ask4_ord_qty     , sizeof(dat->ask4_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.bid4_ord_qty     , dat->bid4_ord_qty     , sizeof(dat->bid4_ord_qty));

			memcpy(Shm_FinFut[Idx].B6.ask5_price       , dat->ask5_price       , sizeof(dat->ask5_price  ));
			memcpy(Shm_FinFut[Idx].B6.bid5_price       , dat->bid5_price       , sizeof(dat->bid5_price  ));

			memcpy(Shm_FinFut[Idx].B6.ask5_qty         , dat->ask5_qty         , sizeof(dat->ask5_qty    ));
			memcpy(Shm_FinFut[Idx].B6.bid5_qty         , dat->bid5_qty         , sizeof(dat->bid5_qty    ));

			memcpy(Shm_FinFut[Idx].B6.ask5_ord_qty     , dat->ask5_ord_qty     , sizeof(dat->ask5_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.bid5_ord_qty     , dat->bid5_ord_qty     , sizeof(dat->bid5_ord_qty));

			memcpy(Shm_FinFut[Idx].B6.total_ask_qty    , dat->total_ask_qty    , sizeof(dat->total_ask_qty));
			memcpy(Shm_FinFut[Idx].B6.total_bid_qty    , dat->total_bid_qty    , sizeof(dat->total_bid_qty));

			memcpy(Shm_FinFut[Idx].B6.valid_ask_ord_qty, dat->valid_ask_ord_qty, sizeof(dat->valid_ask_ord_qty));
			memcpy(Shm_FinFut[Idx].B6.valid_bid_ord_qty, dat->valid_bid_ord_qty, sizeof(dat->valid_bid_ord_qty));

			/* 공용 현재가 */
			if (memcmp (Shm_FinFut[Idx].G7.crprc, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].crprc =
				sign *
				(double)AtoDf (&Shm_FinFut[Idx].G7.crprc[1],sizeof (Shm_FinFut[0].G7.crprc) - 1);
			/* 공용 실시간상한가가격 */
			if (memcmp (Shm_FinFut[Idx].G7.dyn_upper_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_hprc =
				(double)AtoDf (&Shm_FinFut[Idx].G7.dyn_upper_limit[1],
															sizeof(Shm_FinFut[0].G7.dyn_upper_limit) - 1);
			/* 공용 실시간하한가가격 */
			if (memcmp (Shm_FinFut[Idx].G7.dyn_lower_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_lprc =
				(double)AtoDf (&Shm_FinFut[Idx].G7.dyn_lower_limit[1],
															sizeof(Shm_FinFut[0].G7.dyn_lower_limit) - 1);
			/* 공용 매도1호가가격 */
			if (memcmp (Shm_FinFut[Idx].G7.ask1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].sell_1_price =
				(double)AtoDf (&Shm_FinFut[Idx].G7.ask1_price[1],
															sizeof(Shm_FinFut[0].G7.ask1_price) - 1);
			/* 공용 매수1호가가격 */
			if (memcmp (Shm_FinFut[Idx].G7.bid1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].buy_1_price =
				(double)AtoDf (&Shm_FinFut[Idx].G7.bid1_price[1],
															sizeof(Shm_FinFut[0].G7.bid1_price) - 1);
			
		}
    }
    else if (memcmp (p_buf, "B606F", 5) == 0)
    {
#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			Shm_FinFut[Idx].HogaLastGbn = 1;
			memcpy (Shm_FinFut[Idx].B6.tr_gbn, p_buf, sizeof (CO_B601F));

			/* 공용 매도1호가가격 */
			if (memcmp (Shm_FinFut[Idx].B6.ask1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].sell_1_price =
				(double)AtoDf (&Shm_FinFut[Idx].B6.ask1_price[1],
															sizeof(Shm_FinFut[0].B6.ask1_price) - 1);
			/* 공용 매수1호가가격 */
			if (memcmp (Shm_FinFut[Idx].B6.bid1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].buy_1_price =
				(double)AtoDf (&Shm_FinFut[Idx].B6.bid1_price[1],
															sizeof(Shm_FinFut[0].B6.bid1_price) - 1);
		}
    }
    else if (memcmp (p_buf, "M406F", 5) == 0)
    {
		memcpy (Shm_FinFut[0].M4.tr_gbn, p_buf, sizeof (CO_M401F));
    }
    else
    {
        Log (USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }
#elif defined A7302 || A7303
    /* 수신시세 => Memory 시세에 Set */
    if (memcmp (p_buf, "A306V", 5) == 0)
    {
#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF_N, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			memcpy (Shm_FinFut_N[Idx].A3.tr_gbn, p_buf,       sizeof (CO_A301F));

			/* 공용 현재가 */
			if (memcmp (Shm_FinFut_N[Idx].A3.crprc, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].crprc =
				sign *
				(double)AtoDf (&Shm_FinFut_N[Idx].A3.crprc[1],sizeof (Shm_FinFut_N[0].A3.crprc) - 1);
			/* 공용 실시간상한가가격 */
			if (memcmp (Shm_FinFut_N[Idx].A3.dyn_upper_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_hprc =
				(double)AtoDf (&Shm_FinFut_N[Idx].A3.dyn_upper_limit[1],
															sizeof (Shm_FinFut_N[0].A3.dyn_upper_limit) - 1);
			/* 공용 실시간하한가가격 */
			if (memcmp (Shm_FinFut_N[Idx].A3.dyn_lower_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_lprc =
				(double)AtoDf (&Shm_FinFut_N[Idx].A3.dyn_lower_limit[1],
															sizeof (Shm_FinFut_N[0].A3.dyn_lower_limit) - 1);
		}
    }
    else if (memcmp (p_buf, "G706V", 5) == 0)
    {
		CO_G701F *dat = (CO_G701F *)p_buf;
#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF_N, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE],                  sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			Shm_FinFut_N[Idx].HogaLastGbn = 0;
			memcpy(Shm_FinFut_N[Idx].G7.tr_gbn, p_buf, sizeof (CO_G701F));

			memcpy(Shm_FinFut_N[Idx].B6.ask1_price       , dat->ask1_price       , sizeof(dat->ask1_price  ));
			memcpy(Shm_FinFut_N[Idx].B6.bid1_price       , dat->bid1_price       , sizeof(dat->bid1_price  ));

			memcpy(Shm_FinFut_N[Idx].B6.ask1_qty         , dat->ask1_qty         , sizeof(dat->ask1_qty    ));
			memcpy(Shm_FinFut_N[Idx].B6.bid1_qty         , dat->bid1_qty         , sizeof(dat->bid1_qty    ));

			memcpy(Shm_FinFut_N[Idx].B6.ask1_ord_qty     , dat->ask1_ord_qty     , sizeof(dat->ask1_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.bid1_ord_qty     , dat->bid1_ord_qty     , sizeof(dat->bid1_ord_qty));

			memcpy(Shm_FinFut_N[Idx].B6.ask2_price       , dat->ask2_price       , sizeof(dat->ask2_price  ));
			memcpy(Shm_FinFut_N[Idx].B6.bid2_price       , dat->bid2_price       , sizeof(dat->bid2_price  ));

			memcpy(Shm_FinFut_N[Idx].B6.ask2_qty         , dat->ask2_qty         , sizeof(dat->ask2_qty    ));
			memcpy(Shm_FinFut_N[Idx].B6.bid2_qty         , dat->bid2_qty         , sizeof(dat->bid2_qty    ));

			memcpy(Shm_FinFut_N[Idx].B6.ask2_ord_qty     , dat->ask2_ord_qty     , sizeof(dat->ask2_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.bid2_ord_qty     , dat->bid2_ord_qty     , sizeof(dat->bid2_ord_qty));

			memcpy(Shm_FinFut_N[Idx].B6.ask3_price       , dat->ask3_price       , sizeof(dat->ask3_price  ));
			memcpy(Shm_FinFut_N[Idx].B6.bid3_price       , dat->bid3_price       , sizeof(dat->bid3_price  ));

			memcpy(Shm_FinFut_N[Idx].B6.ask3_qty         , dat->ask3_qty         , sizeof(dat->ask3_qty    ));
			memcpy(Shm_FinFut_N[Idx].B6.bid3_qty         , dat->bid3_qty         , sizeof(dat->bid3_qty    ));

			memcpy(Shm_FinFut_N[Idx].B6.ask3_ord_qty     , dat->ask3_ord_qty     , sizeof(dat->ask3_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.bid3_ord_qty     , dat->bid3_ord_qty     , sizeof(dat->bid3_ord_qty));

			memcpy(Shm_FinFut_N[Idx].B6.ask4_price       , dat->ask4_price       , sizeof(dat->ask4_price  ));
			memcpy(Shm_FinFut_N[Idx].B6.bid4_price       , dat->bid4_price       , sizeof(dat->bid4_price  ));

			memcpy(Shm_FinFut_N[Idx].B6.ask4_qty         , dat->ask4_qty         , sizeof(dat->ask4_qty    ));
			memcpy(Shm_FinFut_N[Idx].B6.bid4_qty         , dat->bid4_qty         , sizeof(dat->bid4_qty    ));

			memcpy(Shm_FinFut_N[Idx].B6.ask4_ord_qty     , dat->ask4_ord_qty     , sizeof(dat->ask4_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.bid4_ord_qty     , dat->bid4_ord_qty     , sizeof(dat->bid4_ord_qty));

			memcpy(Shm_FinFut_N[Idx].B6.ask5_price       , dat->ask5_price       , sizeof(dat->ask5_price  ));
			memcpy(Shm_FinFut_N[Idx].B6.bid5_price       , dat->bid5_price       , sizeof(dat->bid5_price  ));

			memcpy(Shm_FinFut_N[Idx].B6.ask5_qty         , dat->ask5_qty         , sizeof(dat->ask5_qty    ));
			memcpy(Shm_FinFut_N[Idx].B6.bid5_qty         , dat->bid5_qty         , sizeof(dat->bid5_qty    ));

			memcpy(Shm_FinFut_N[Idx].B6.ask5_ord_qty     , dat->ask5_ord_qty     , sizeof(dat->ask5_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.bid5_ord_qty     , dat->bid5_ord_qty     , sizeof(dat->bid5_ord_qty));

			memcpy(Shm_FinFut_N[Idx].B6.total_ask_qty    , dat->total_ask_qty    , sizeof(dat->total_ask_qty));
			memcpy(Shm_FinFut_N[Idx].B6.total_bid_qty    , dat->total_bid_qty    , sizeof(dat->total_bid_qty));

			memcpy(Shm_FinFut_N[Idx].B6.valid_ask_ord_qty, dat->valid_ask_ord_qty, sizeof(dat->valid_ask_ord_qty));
			memcpy(Shm_FinFut_N[Idx].B6.valid_bid_ord_qty, dat->valid_bid_ord_qty, sizeof(dat->valid_bid_ord_qty));

			/* 공용 현재가 */
			if (memcmp (Shm_FinFut_N[Idx].G7.crprc, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;

			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].crprc =
				sign *
				(double)AtoDf (&Shm_FinFut_N[Idx].G7.crprc[1],sizeof (Shm_FinFut_N[0].G7.crprc) - 1);
			/* 공용 실시간상한가가격 */
			if (memcmp (Shm_FinFut_N[Idx].G7.dyn_upper_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_hprc =
				(double)AtoDf (&Shm_FinFut_N[Idx].G7.dyn_upper_limit[1],
															sizeof(Shm_FinFut_N[0].G7.dyn_upper_limit) - 1);
			/* 공용 실시간하한가가격 */
			if (memcmp (Shm_FinFut_N[Idx].G7.dyn_lower_limit, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].realtime_lprc =
				(double)AtoDf (&Shm_FinFut_N[Idx].G7.dyn_lower_limit[1],
															sizeof(Shm_FinFut_N[0].G7.dyn_lower_limit) - 1);
			/* 공용 매도1호가가격 */
			if (memcmp (Shm_FinFut_N[Idx].G7.ask1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].sell_1_price =
				(double)AtoDf (&Shm_FinFut_N[Idx].G7.ask1_price[1],
															sizeof(Shm_FinFut_N[0].G7.ask1_price) - 1);
			/* 공용 매수1호가가격 */
			if (memcmp (Shm_FinFut_N[Idx].G7.bid1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].buy_1_price =
				(double)AtoDf (&Shm_FinFut_N[Idx].G7.bid1_price[1],
															sizeof(Shm_FinFut_N[0].G7.bid1_price) - 1);
		}
    }
    else if (memcmp (p_buf, "B606V", 5) == 0)
    {
#if	1	/* HONG 시세 전문에 seq가 없으므로 종목코드로 Idx를 구한다 */
        Che_Gbn = 1;
		memcpy(Key.expcode, &p_buf[I_H_SIZE],			sizeof (Key.expcode));
		Idx = Key_Search (DEV_MK_FIF_N, 0, (char *)&Key);

		if (Idx != NOTOK)
#else
        Che_Gbn = 1;
        Idx = AtoIf (&p_buf[S_H_SIZE], sizeof(Shm_FinFut[0].A3.seq_no));
#endif
		{
			Shm_FinFut_N[Idx].HogaLastGbn = 1;
			memcpy (Shm_FinFut_N[Idx].B6.tr_gbn, p_buf, sizeof (CO_B601F));

			/* 공용 매도1호가가격 */
			if (memcmp (Shm_FinFut_N[Idx].B6.ask1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].sell_1_price =
				(double)AtoDf (&Shm_FinFut_N[Idx].B6.ask1_price[1],
															sizeof(Shm_FinFut_N[0].B6.ask1_price) - 1);
			/* 공용 매수1호가가격 */
			if (memcmp (Shm_FinFut_N[Idx].B6.bid1_price, "-", 1) == 0)
				sign = -1;
			else
				sign = 1;
			Shm_Risk[0].S_Sise[RISK_MK_IDX][Idx].buy_1_price =
				(double)AtoDf (&Shm_FinFut_N[Idx].B6.bid1_price[1],
															sizeof(Shm_FinFut_N[0].B6.bid1_price) - 1);
		}
    }
    else if (memcmp (p_buf, "M406V", 5) == 0)
    {
        memcpy (Shm_FinFut_N[0].M4.tr_gbn, p_buf, sizeof (CO_M401K));
    }
    else
    {
        Log (USR_OK, "Other TrCode[%s][%d]", p_buf, sizeof(p_buf));
        Che_Gbn = -1;
    }
#elif defined A7181
	CO_A001_RDS01 *dat = (CO_A001_RDS01 *)p_buf;

	for (ii = 1; ii < SHM_MAX_RDS01; ii++)
	{
		/*	값이 비여있다는 것은 "해당종목코드가 없었고 신규로 등록해야한다는 것임"	*/
		if (memcmp (Shm_Rds01[ii].item_code, "000000000000", 12) <= 0)
		{
			Shm_Note[2].total_item_cnt++;
			//Shm_Risk[0].Max_Seq[1] = Shm_Note[2].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크, 999..99 안줄지도 모르니
			memcpy (Shm_Rds01[ii].seq_no, p_buf, sizeof (CO_A001_RDS01));
			break;
		}

		/* 중복된 값(종목정보)이 있으면 나간다 */
		if (memcmp (Shm_Rds01[ii].item_code, dat->item_code, 12) == 0)
		{
			memcpy (Shm_Rds01[ii].seq_no, p_buf, sizeof (CO_A001_RDS01));
			return;
		}

		if (ii >= SHM_MAX_RDS01 - 1)
		{
			Log (USR_ERROR, "NOTE SHM COUNT ERROR!!! CHECK PLZ!!! ii[%d], SHM_MAX_NOTE_RDS01[%d]", ii, SHM_MAX_RDS01);
			return;
		}
	}
#elif defined A7182
	//idx = AtoIf (&p_buf[0], sizeof (Shm_Note[0].A0.seq_no));
	/* 기접수 처리여부, 있으면 업어친다(나중거로) */

	CO_A001_RDS02 *dat = (CO_A001_RDS02 *)p_buf;

	for (ii = 1; ii < SHM_MAX_NOTE; ii++)
	{
		/* 값이 비여있으면 나간다 */
		if (memcmp (Shm_Note[ii].A0.item_code, "000000000000", 12) <= 0)
		{
			Shm_Note[0].total_item_cnt++;
			Shm_Risk[0].Max_Seq[1] = Shm_Note[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크, 999..99 안줄지도 모르니
			memcpy (Shm_Note[ii].A0.seq_no, p_buf, sizeof (CO_A001_RDS02));
			break;
		}

		/* 중복된 값(종목정보)이 있으면 나간다 */
		if (memcmp (Shm_Note[ii].A0.item_code, dat->item_code, 12) == 0)
		{
			memcpy (Shm_Note[ii].A0.seq_no, p_buf, sizeof (CO_A001_RDS02));
			return;
		}

		if (ii >= SHM_MAX_NOTE - 1)
		{
			Log (USR_ERROR, "NOTE SHM COUNT ERROR!!! CHECK PLZ!!! ii[%d], SHM_MAX_NOTE[%d]", ii, SHM_MAX_NOTE);
			return;
		}
	}

	/* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
	if (memcmp (Shm_Note[ii].A0.item_code, "999999999999", 12) == 0)
	{
		Shm_Risk[0].Max_Seq[1] = Shm_Note[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크

		for (ii = 1; ii < Shm_Note[0].total_item_cnt; ii++)
		{
			Shm_Risk[0].S_Sise[0][ii].crprc = (double)AtoDf (&Shm_Note[ii].A0.base_prc[1],	sizeof (Shm_Note[0].A0.base_prc) - 1);
			/* *************************************** */
			/* 한도체크용 STANDARD_SISE_FORMAT Setting */
			/* 종목코드 */
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_item_cd,	Shm_Note[ii].A0.item_code,		sizeof (Shm_Note[0].A0.item_code));
			/* 종목거래가능 여부 */
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_item_stat,	Shm_Note[ii].A0.trade_susp_yn,	sizeof (Shm_Note[0].A0.trade_susp_yn));
			/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
//				memcpy (Shm_Risk[0].S_Sise[0][ii].item_stat, " ", 1);
			/* 기준가격, 양의수만사용(1)+ 소수점 미만 둘째 자리의 값은 0으로 고정(소수점 1자리까지만 사용) */
			Shm_Risk[0].S_Sise[0][ii].m_stdard_price = Shm_Risk[0].S_Sise[0][ii].crprc;
			/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
			memcpy (Shm_Risk[0].S_Sise[0][ii].nb_shares_stock_1per,
				&Shm_Note[ii].A0.listed_stock[3], 10);	*/
/* 지정가만 사용 가능, 조건코드 없음 */
			/* 지정가호가조건코드, 지정가호가취소조건코드 
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_2 = AtoIf (&Shm_Note[ii].A0.lim_ord_cancel_cond_cd[4], 1);	*/
			/* 조건부지정가 호가조건코드, 조건부지정가호가취소조건코드 
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_I = AtoIf (&Shm_Note[ii].A0.cond_lim_ord_cancel_cond_cd[4], 1);	*/
			/* 시장가 호가조건코드(1대신 파생용) 
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_T = AtoIf (&Shm_Note[ii].A0.mkt_ord_cancel_cond_cd[4], 1); 	*/
			/* 최유리지정가 호가조건코드(X대신 파생용) 
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_W = AtoIf (&Shm_Note[ii].A0.best_lim_ord_cancel_cond_cd[4], 1); */
			/* 시장가호가조건코드 (현물용)
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_1 = AtoIf (&Shm_Note[ii].A0.mkt_ord_cancel_cond_cd[4], 1);	*/
			/* 최유리지정가 호가조건코드(현물용) 
			Shm_Risk[0].S_Sise[0][ii].m_cds_gbn_X = AtoIf (&Shm_Note[ii].A0.best_lim_ord_cancel_cond_cd[4], 1);	*/

			/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. , 채권은 없음
			Shm_Risk[0].S_Sise[0][ii].m_max_qty = (double)AtoDf(&Shm_Note[ii].A0.up_qty[6], 10);	*/
			/* 정리매매 여부(현물용)
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_clearance_gbn, Shm_Note[ii].A0.clear_gubun, 1);		*/
			/* 시가기준가종목(현물용)
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_start_price_gbn, Shm_Note[ii].A0.open_st_price, 1);		*/
			/* 최고호가가격(현물용) 
			Shm_Risk[0].S_Sise[0][ii].m_top_price = (double)AtoDf(Shm_Note[ii].A0.high_bid, 9);	*/
			/* 최저호가가격(현물용) 
			Shm_Risk[0].S_Sise[0][ii].m_lowest_price = (double)AtoDf(Shm_Note[ii].A0.low_bid, 9);	*/

			/* 가격제한 최종단계, 채권은 없음, 상하한가가 있음 */
			/* 2025 7181 읽어서 상하한가 처리하는 방식 고려 */
//			if (memcmp (Shm_Note[ii].A0.prc_lmt_finl_stg, "001", 3) == 0)
//			{
//				/* 상한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_h_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg1_up[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg1_up, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_h_lmt = Shm_Risk[0].S_Sise[0][ii].m_h_lmt * -1;
//				/* 하한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_l_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg1_lo[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg1_lo, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_l_lmt = Shm_Risk[0].S_Sise[0][ii].m_l_lmt * -1;
//			}
//			else if (memcmp (Shm_Note[ii].A0.prc_lmt_finl_stg, "002", 3) == 0)
//			{
//				/* 상한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_h_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg2_up[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg2_up, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_h_lmt = Shm_Risk[0].S_Sise[0][ii].m_h_lmt * -1;
//				/* 하한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_l_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg2_lo[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg2_lo, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_l_lmt = Shm_Risk[0].S_Sise[0][ii].m_l_lmt * -1;
//			}
//			else if (memcmp (Shm_Note[ii].A0.prc_lmt_finl_stg, "003", 3) == 0)
//			{
//				/* 상한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_h_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg3_up[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg3_up, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_h_lmt = Shm_Risk[0].S_Sise[0][ii].m_h_lmt * -1;
//				/* 하한가 */
//				Shm_Risk[0].S_Sise[0][ii].m_l_lmt = (double)AtoDf(&Shm_Note[ii].A0.prc_lmt_stg3_lo[3], 9)/100;
//				if (memcmp (Shm_Note[ii].A0.prc_lmt_stg3_lo, "-", 1) == 0)
//					Shm_Risk[0].S_Sise[0][ii].m_l_lmt = Shm_Risk[0].S_Sise[0][ii].m_l_lmt * -1;
//			}

			/* 승수, 채권은 확인필요 2025 우선 1,000원으로 */
			Shm_Risk[0].S_Sise[0][ii].m_multiplier = 1000;
			/* 행사가격 18자리중 12자리(소수점2자리)까지 처리함, 2025 채권확인필요. 이전에는 17이였음 
			Shm_Risk[0].S_Sise[0][ii].m_striking_price = AtoDf (Shm_Note[ii].A0.strike_prc, 12)/100;	*/
			/* 거래종료일, (파생용) 
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_closeday, Shm_Note[ii].A0.delist_date, 8); */
			/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
			memcpy (Shm_Risk[0].S_Sise[0][ii].basic_asset_price, Shm_Note[ii].A0., 8);      */
			/* 증권 그룹ID (현물용)
			memcpy (Shm_Risk[0].S_Sise[0][ii].m_group_id, Shm_Note[ii].A0.group_id, 2); */
			/* *************************************** */

			Shm_Item[0].N_Key[ii].idx = ii;
			memcpy (Shm_Item[0].N_Key[ii].expcode, Shm_Note[ii].A0.board_id,
							sizeof (Shm_Note[0].A0.board_id) + sizeof (Shm_Note[0].A0.item_code));
		}
		qsort (Shm_Item[0].N_Key, Shm_Note[0].total_item_cnt, sizeof (KS_NOTE_EXPCODE), CmpExpcode);
	}
#elif defined A7801
	CO_A001_RDS01 *dat = (CO_A001_RDS01 *)p_buf;

	/* 영업일자/채권유형코드/채권분류코드/소매채권분류코드가 일치하는 것만 Write */
	if ((memcmp (dat->biz_date,				d_time,	8) == 0)		&&
		(memcmp (dat->bond_type_cd,			"GB",	2) == 0)		&&
		((memcmp (dat->bond_class_cd,	"111100",	6) == 0)	||
		 (memcmp (dat->bond_class_cd,	"111900",	6) == 0))		&&
		(memcmp (dat->retail_bond_class_cd, "GA",	2) == 0))
	{
		rt = F_W (PS_R_1, (void *)R_Fmt, 1);

		if (rt != 1)
		{
			Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
			return;
		}

		Log (USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
	}
	else
		Log (USR_OK, "file SKIP[%s]", dat.seq_no);
#elif defined A7802
	CO_A001_RDS02 *dat = (CO_A001_RDS02 *)p_buf;

	/* 영업일자/보드아이디가 일치하는 것만 Write */
	if ((memcmp (dat->biz_date,	d_time,	8) == 0)	&&
		(memcmp (dat->board_id, "G1",	2) == 0))
	{
		rt = F_W (PS_R_1, (void *)R_Fmt, 1);

		if (rt != 1)
		{
			Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
			return;
		}

		Log (USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
	}
	else
		Log (USR_OK, "file SKIP[%s]", dat.seq_no);
#endif

	return;
}	/* End of Set_Sise ()	*/

/*************************************************************************
    Function        : . Write_Read_Fifo
    Parameters IN   : . 1 : A3/G7체결, 2 : B6호가
    Parameters OUT  : .
    Comment         : . write to FIFO
*************************************************************************/
void	Write_Read_Fifo (int c0h0, int ii)
{
	int     rt;
	char    tmp[128];

	rt = write (FIFO_fd[ii], "1", 1);

	if (rt < 0)
	SLog (FIF_FATAL, "cannot write FIFO[%d][%d:%s]",
		FIFO_fd[ii], SYS_NO, SYS_STR);

#if	0	/* FIFO를 read 하면 다른 process가 poll을 못 받음	*/
	while (1)
	{
		rt = read (FIFO_fd, tmp, sizeof(tmp));
#if defined __linux
		if (rt == 0 || errno == EAGAIN)
#else
		if (rt == 0)
#endif
			break;
	}
#endif

	return;
}   /* End of Write_Read_Fifo ()    */

/*************************************************************************
	End of program (pa_7100_dd.c)
*************************************************************************/ 
