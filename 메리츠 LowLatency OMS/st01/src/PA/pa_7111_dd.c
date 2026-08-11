#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 현물시세수신 DD
#	File	: pa_7111_dd.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#if defined A7191||A7291||A7391||A7491	||	A7891||A7991||A7992 ||A6591||A6691
#define     DATA_SIZE       1300
#elif defined A7392||A7492
#define     DATA_SIZE       550
#elif defined A7996
#define     DATA_SIZE       320
#elif defined A7192||A7193||A7292||A7293 ||A7892||A7997 ||A6592||A6692
#define     DATA_SIZE       300
#endif
#include    "buf_struct.h"
/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define            DATA_TIME       60 * 1000
/* 시세 Seq 위치 Size */
#define		S_H_SIZE		17						/* TR(5) + Code(12) */
/* 기본마스터 Seq 위치 Size */
#define		M_H_SIZE		30	/* TR(5) + JCNT(5) + DATE(8) + Code(12)	*/
KS_EXPCODE	Key;

int			Che_Gbn;
char		ApType[10];
FILE_BUFF_FORMAT		W_Fmt;
BUFF_RW_HEAD        	f_head;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7111_DD (void);
void	Set_Sise (char *);
void	Microsec_Sleep (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7111_DD ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7111_DD (void)
/*----------------------------------------------------------------------*/
{
	int				i, rt, len, R_Cnt;
	char 			m_time[24];
	char			fifo_name[100], bumun[4];
	char			W2_Fmt[2048];
	FILE_BUFF_FORMAT    R_Fmt[1];

/* 기초정보 재기동시 초기화 작업 */
#if defined A7191
	Shm_Futures[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[1] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_FUTURES; i++)
	{
		memset (Shm_Futures[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].F_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_FUTURES);
#elif defined A7291
	Shm_Options[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[2] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_OPTIONS; i++)
	{
		memset (Shm_Options[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].O_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_OPTIONS);
#elif defined A7391
	Shm_SFutures[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[3] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_S_FUTURES; i++)
	{
		memset (Shm_SFutures[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].SF_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_S_FUTURES);
#elif defined A7491
	Shm_SOptions[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[4] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_S_OPTIONS; i++)
	{
		memset (Shm_SOptions[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].SO_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_S_OPTIONS);
#elif defined A7891
	Shm_K300[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[8] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_K300; i++)
	{
		memset (Shm_K300[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].K300_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_K300);
#elif defined A7991
	Shm_K150F[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[9] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_K150F; i++)
	{
		memset (Shm_K150F[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].K150F_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_K150F);
#elif defined A7992
	Shm_K150O[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[10] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_K150O; i++)
	{
		memset (Shm_K150O[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].K150O_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_K150O);
#elif defined A6591
	Shm_MF[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[11] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_MF; i++)
	{
		memset (Shm_MF[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].MF_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_MF);
#elif defined A6691
	Shm_MO[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[12] = 0;     // 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_MO; i++)
	{
		memset (Shm_MO[i].A0.tr_gbn, 0, sizeof (SIF_A0014));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].MO_Key,	0,	sizeof (KS_EXPCODE) * SHM_MAX_MO);
#else
	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));
#endif

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Stat_Save ();
			memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT));

			R_Cnt = F_R(PS_R_1, (void *)R_Fmt, 1);
			if (R_Cnt < 0)
        	{
            	Log (SAM_FATAL, "cannot read file[%s,%d:%s]",
                	IFN(D_K,P_K,0), SYS_NO, SYS_STR);
            	sleep (1);
            	Exit_Process ();
        	}
        	else if (R_Cnt == 0)
            	break;

			Set_Sise (R_Fmt[0].Data);

			Set_TR_Time ();
			INT_SEQ ++;

			if (memcmp (R_Fmt[0].Data, "A0", 2) != 0)
			{
				/* ********************************** */
				/* pa_7001_mp 통시세에 Write 해야한다 */
				memset (m_time, 0, sizeof (m_time));
				Get_MicroTime (m_time);

				/* File Write */
				memset (W2_Fmt, 0x20, sizeof (W2_Fmt));

				/* write to DD file */
				ItoAf (O_W_CNT1 + 1, f_head.If_Seq, sizeof (f_head.If_Seq));

				memcpy (f_head.ApType, ApType, sizeof (f_head.ApType));
				memcpy (f_head.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
				memcpy (f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
				memcpy (f_head.RecvTime2, m_time+10, sizeof (f_head.RecvTime2));

				memcpy (f_head.DataHeader, "                    ",  20);
				memcpy (&W2_Fmt, &f_head, sizeof (BUFF_RW_HEAD));

				/* Data Write */
				memcpy (&W2_Fmt[sizeof (BUFF_RW_HEAD)], R_Fmt[0].Data,
														strlen (R_Fmt[0].Data));

				W2_Fmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

				rt = F_W (TS_W1_1, (void *)&W2_Fmt, 1);
				if (rt != 1)
				{
					Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
					return;
				}
				/* ********************************** */
			}
			Add_Count(PS_R_1, 1);
		}

        rt = Poll_File (DATA_TIME);

        if (rt == 1)
            Log (USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;
	}

	return;
}	/* End of PA_7111_DD ()	*/

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
	int			ii, s_k, idx, rt;
	char		m_time[24], TrCode[10];

    memset (m_time, 0, sizeof (m_time));
    Get_MicroTime (m_time);

#if defined A7191
	if (memcmp (p_buf, "A0014", 5) == 0)
	{
		idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_Futures[0].A0.seq_no));
		memcpy (Shm_Futures[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

		if (Shm_Futures[0].total_item_cnt < idx)
			Shm_Futures[0].total_item_cnt = idx;

		/* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
		if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_Futures[0].A0.item_code)) == 0)
		{
			Shm_Risk[0].Max_Seq[1] = Shm_Futures[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
			for (ii = 1; ii < Shm_Futures[0].total_item_cnt; ii++)
			{
				Shm_Risk[0].S_Sise[1][ii].crprc = (double)AtoIf(&Shm_Futures[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_item_cd,
					Shm_Futures[ii].A0.item_code, sizeof (Shm_Futures[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_item_stat,
					Shm_Futures[ii].A0.trade_stop, sizeof (Shm_Futures[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[1][ii].item_stat, " ", 1);
				/* 기준가격(스프레드는 0임) */
				Shm_Risk[0].S_Sise[1][ii].m_stdard_price = 
					(double)AtoDf (&Shm_Futures[ii].A0.stprc[3], sizeof (Shm_Futures[0].A0.stprc)-3);
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[1][ii].nb_shares_stock_1per,
					&Shm_Futures[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_1 = AtoIf (&Shm_Futures[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_2 = AtoIf (&Shm_Futures[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_I = AtoIf (&Shm_Futures[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_X = AtoIf (&Shm_Futures[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용) 
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_Y = AtoIf (&Shm_Futures[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_T = AtoIf (&Shm_Futures[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[1][ii].m_cds_gbn_W = AtoIf (&Shm_Futures[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[1][ii].m_max_qty = (double)AtoDf(&Shm_Futures[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_clearance_gbn,
					Shm_Futures[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_start_price_gbn,
					Shm_Futures[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_top_price = (double)AtoDf(Shm_Futures[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[1][ii].m_lowest_price = (double)AtoDf(Shm_Futures[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_Futures[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_Futures[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_Futures[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_Futures[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_Futures[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_Futures[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_Futures[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[1][ii].m_h_lmt = (double)AtoDf(&Shm_Futures[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_h_lmt = Shm_Risk[0].S_Sise[1][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[1][ii].m_l_lmt = (double)AtoDf(&Shm_Futures[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_Futures[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[1][ii].m_l_lmt = Shm_Risk[0].S_Sise[1][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[1][ii].m_multiplier = AtoIf (&Shm_Futures[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_closeday,
					Shm_Futures[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[1][ii].basic_asset_price,
					Shm_Futures[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[1][ii].m_group_id,
					Shm_Futures[ii].A0.group_id, 2); */
				/* *************************************** */

				Shm_Item[0].F_Key[ii].idx = ii;
				memcpy (Shm_Item[0].F_Key[ii].expcode, Shm_Futures[ii].A0.item_code,
													sizeof (Shm_Futures[0].A0.item_code));
			}
			qsort (Shm_Item[0].F_Key, Shm_Futures[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
		}
	}
#elif defined A7291
	if (memcmp (p_buf, "A0034", 5) == 0)
	{
		idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_Options[0].A0.seq_no));
		memcpy (Shm_Options[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

		if (Shm_Options[0].total_item_cnt < idx)
			Shm_Options[0].total_item_cnt = idx;

		/* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
		if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_Options[0].A0.item_code)) == 0)
		{
			Shm_Risk[0].Max_Seq[2] = Shm_Options[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
			for (ii = 1; ii < Shm_Options[0].total_item_cnt; ii++)
			{
				Shm_Risk[0].S_Sise[2][ii].crprc = (double)AtoIf(&Shm_Options[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_item_cd,
					Shm_Options[ii].A0.item_code, sizeof (Shm_Options[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_item_stat,
					Shm_Options[ii].A0.trade_stop, sizeof (Shm_Options[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[2][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[2][ii].m_stdard_price =
					(double)AtoDf (&Shm_Options[ii].A0.stprc[3], sizeof (Shm_Options[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[2][ii].nb_shares_stock_1per,
					&Shm_Options[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_1 = AtoIf (&Shm_Options[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_2 = AtoIf (&Shm_Options[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_I = AtoIf (&Shm_Options[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_X = AtoIf (&Shm_Options[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용) 
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_Y = AtoIf (&Shm_Options[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_T = AtoIf (&Shm_Options[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[2][ii].m_cds_gbn_W = AtoIf (&Shm_Options[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[2][ii].m_max_qty = (double)AtoDf(&Shm_Options[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_clearance_gbn,
					Shm_Options[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_start_price_gbn,
					Shm_Options[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[2][ii].m_top_price = (double)AtoDf(Shm_Options[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[2][ii].m_lowest_price = (double)AtoDf(Shm_Options[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_Options[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[2][ii].m_h_lmt = (double)AtoDf(&Shm_Options[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_h_lmt = Shm_Risk[0].S_Sise[2][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[2][ii].m_l_lmt = (double)AtoDf(&Shm_Options[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_l_lmt = Shm_Risk[0].S_Sise[2][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_Options[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[2][ii].m_h_lmt = (double)AtoDf(&Shm_Options[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_h_lmt = Shm_Risk[0].S_Sise[2][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[2][ii].m_l_lmt = (double)AtoDf(&Shm_Options[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_l_lmt = Shm_Risk[0].S_Sise[2][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_Options[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[2][ii].m_h_lmt = (double)AtoDf(&Shm_Options[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_h_lmt = Shm_Risk[0].S_Sise[2][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[2][ii].m_l_lmt = (double)AtoDf(&Shm_Options[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_Options[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[2][ii].m_l_lmt = Shm_Risk[0].S_Sise[2][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[2][ii].m_multiplier = AtoIf (&Shm_Options[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_closeday,
					Shm_Options[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[2][ii].basic_asset_price,
					Shm_Options[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[2][ii].m_group_id,
					Shm_Options[ii].A0.group_id, 2); */
				/* *************************************** */

				Shm_Item[0].O_Key[ii].idx = ii;
				memcpy (Shm_Item[0].O_Key[ii].expcode, Shm_Options[ii].A0.item_code,
													sizeof (Shm_Options[0].A0.item_code));
			}
			qsort (Shm_Item[0].O_Key, Shm_Options[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
		}
	}
#elif defined A7391		// 개별선물 
	if (memcmp (p_buf, "A0015", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_SFutures[0].A0.seq_no));
        memcpy (Shm_SFutures[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_SFutures[0].total_item_cnt < idx)
            Shm_SFutures[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_SFutures[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[3] = Shm_SFutures[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_SFutures[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[3][ii].crprc = (double)AtoIf(&Shm_SFutures[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_item_cd,
					Shm_SFutures[ii].A0.item_code, sizeof (Shm_SFutures[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_item_stat,
					Shm_SFutures[ii].A0.trade_stop, sizeof (Shm_SFutures[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[3][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[3][ii].m_stdard_price =
					(double)AtoDf (&Shm_SFutures[ii].A0.stprc[3], sizeof (Shm_SFutures[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[3][ii].nb_shares_stock_1per,
					&Shm_SFutures[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_1 = AtoIf (&Shm_SFutures[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_2 = AtoIf (&Shm_SFutures[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_I = AtoIf (&Shm_SFutures[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_X = AtoIf (&Shm_SFutures[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용)
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_Y = AtoIf (&Shm_SFutures[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_T = AtoIf (&Shm_SFutures[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[3][ii].m_cds_gbn_W = AtoIf (&Shm_SFutures[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[3][ii].m_max_qty = (double)AtoDf(&Shm_SFutures[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_clearance_gbn,
					Shm_SFutures[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_start_price_gbn,
					Shm_SFutures[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[3][ii].m_top_price = (double)AtoDf(Shm_SFutures[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[3][ii].m_lowest_price = (double)AtoDf(Shm_SFutures[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_SFutures[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[3][ii].m_h_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_h_lmt = Shm_Risk[0].S_Sise[3][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[3][ii].m_l_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_l_lmt = Shm_Risk[0].S_Sise[3][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_SFutures[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[3][ii].m_h_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_h_lmt = Shm_Risk[0].S_Sise[3][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[3][ii].m_l_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_l_lmt = Shm_Risk[0].S_Sise[3][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_SFutures[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[3][ii].m_h_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_h_lmt = Shm_Risk[0].S_Sise[3][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[3][ii].m_l_lmt = (double)AtoDf(&Shm_SFutures[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_SFutures[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[3][ii].m_l_lmt = Shm_Risk[0].S_Sise[3][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[3][ii].m_multiplier = AtoIf (&Shm_SFutures[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_closeday,
					Shm_SFutures[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 주식선물만 처리한다. */
				/* 기초자산종목코드 A0에서 갖고와서 전일종가 넣기 */

				Shm_Risk[0].S_Sise[3][ii].sf_mk_gbn = 0;

				memset (&Key, 0, sizeof (Key));
				sprintf (Key.expcode, "%-12.12s", Shm_SFutures[ii].A0.underlying_code);
				rt = 0;
				rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char*)&Key);
				if (rt > 0) 
				{
					Shm_Risk[0].S_Sise[3][ii].sf_mk_gbn = 5;
					Shm_Risk[0].S_Sise[3][ii].basic_asset_price = 
						(double)AtoDf(Shm_Stock[ii].A0.prev_cprc, sizeof(Shm_Stock[0].A0.prev_cprc));
				}
				else
				{
					rt = 0;
					rt = Key_Search (MK_KOSDAQ, KEY_EXPCODE, (char*)&Key);
					if (rt > 0) 
					{
						Shm_Risk[0].S_Sise[3][ii].sf_mk_gbn = 6;
						Shm_Risk[0].S_Sise[3][ii].basic_asset_price = 
							(double)AtoDf(Shm_Kosdaq[ii].A0.prev_cprc, sizeof(Shm_Kosdaq[0].A0.prev_cprc));
					}
					else
						Log (USR_ERROR, "StockFutures Not Found Underlying_ITEM_CD[%12.12s] ii[%d]", Shm_SFutures[ii].A0.underlying_code, ii);
				}

				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[3][ii].m_group_id,
					Shm_SFutures[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].SF_Key[ii].idx = ii;
                memcpy (Shm_Item[0].SF_Key[ii].expcode, Shm_SFutures[ii].A0.item_code,
                                                    sizeof (Shm_SFutures[0].A0.item_code));
            }
            qsort (Shm_Item[0].SF_Key, Shm_SFutures[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A7491
	if (memcmp (p_buf, "A0025", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_SOptions[0].A0.seq_no));
        memcpy (Shm_SOptions[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_SOptions[0].total_item_cnt < idx)
            Shm_SOptions[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_SOptions[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[4] = Shm_SOptions[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_SOptions[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[4][ii].crprc = (double)AtoIf(&Shm_SOptions[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_item_cd,
					Shm_SOptions[ii].A0.item_code, sizeof (Shm_SOptions[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_item_stat,
					Shm_SOptions[ii].A0.trade_stop, sizeof (Shm_SOptions[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[4][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[4][ii].m_stdard_price =
					(double)AtoDf (&Shm_SOptions[ii].A0.stprc[3], sizeof (Shm_SOptions[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[4][ii].nb_shares_stock_1per,
					&Shm_SOptions[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_1 = AtoIf (&Shm_SOptions[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_2 = AtoIf (&Shm_SOptions[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_I = AtoIf (&Shm_SOptions[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_X = AtoIf (&Shm_SOptions[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용) 
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_Y = AtoIf (&Shm_SOptions[ii].A0.first_order[4], 1);		*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_T = AtoIf (&Shm_SOptions[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[4][ii].m_cds_gbn_W = AtoIf (&Shm_SOptions[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[4][ii].m_max_qty = (double)AtoDf(&Shm_SOptions[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_clearance_gbn,
					Shm_SOptions[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_start_price_gbn,
					Shm_SOptions[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[4][ii].m_top_price = (double)AtoDf(Shm_SOptions[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[4][ii].m_lowest_price = (double)AtoDf(Shm_SOptions[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_SOptions[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[4][ii].m_h_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_h_lmt = Shm_Risk[0].S_Sise[4][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[4][ii].m_l_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_l_lmt = Shm_Risk[0].S_Sise[4][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_SOptions[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[4][ii].m_h_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_h_lmt = Shm_Risk[0].S_Sise[4][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[4][ii].m_l_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_l_lmt = Shm_Risk[0].S_Sise[4][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_SOptions[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[4][ii].m_h_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_h_lmt = Shm_Risk[0].S_Sise[4][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[4][ii].m_l_lmt = (double)AtoDf(&Shm_SOptions[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_SOptions[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[4][ii].m_l_lmt = Shm_Risk[0].S_Sise[4][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[4][ii].m_multiplier = AtoIf (&Shm_SOptions[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_closeday,
					Shm_SOptions[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[4][ii].basic_asset_price,
					Shm_SOptions[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[4][ii].m_group_id,
					Shm_SOptions[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].SO_Key[ii].idx = ii;
                memcpy (Shm_Item[0].SO_Key[ii].expcode, Shm_SOptions[ii].A0.item_code,
                                                    sizeof (Shm_SOptions[0].A0.item_code));
            }
            qsort (Shm_Item[0].SO_Key, Shm_SOptions[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A7891
	if (memcmp (p_buf, "A0164", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_K300[0].A0.seq_no));
        memcpy (Shm_K300[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_K300[0].total_item_cnt < idx) Shm_K300[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_K300[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[8] = Shm_K300[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_K300[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[8][ii].crprc = (double)AtoIf(&Shm_K300[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_item_cd,
					Shm_K300[ii].A0.item_code, sizeof (Shm_K300[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_item_stat,
					Shm_K300[ii].A0.trade_stop, sizeof (Shm_K300[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[8][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[8][ii].m_stdard_price =
					(double)AtoDf (&Shm_K300[ii].A0.stprc[3], sizeof (Shm_K300[0].A0.stprc))/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[8][ii].nb_shares_stock_1per,
					&Shm_K300[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_1 = AtoIf (&Shm_K300[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_2 = AtoIf (&Shm_K300[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_I = AtoIf (&Shm_K300[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_X = AtoIf (&Shm_K300[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용)
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_Y = AtoIf (&Shm_K300[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_T = AtoIf (&Shm_K300[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[8][ii].m_cds_gbn_W = AtoIf (&Shm_K300[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[8][ii].m_max_qty = (double)AtoDf(&Shm_K300[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_clearance_gbn, Shm_K300[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_start_price_gbn, Shm_K300[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[8][ii].m_top_price = (double)AtoDf(Shm_K300[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[8][ii].m_lowest_price = (double)AtoDf(Shm_K300[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_K300[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[8][ii].m_h_lmt = (double)AtoDf(&Shm_K300[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_h_lmt = Shm_Risk[0].S_Sise[8][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[8][ii].m_l_lmt = (double)AtoDf(&Shm_K300[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_l_lmt = Shm_Risk[0].S_Sise[8][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K300[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[8][ii].m_h_lmt = (double)AtoDf(&Shm_K300[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_h_lmt = Shm_Risk[0].S_Sise[8][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[8][ii].m_l_lmt = (double)AtoDf(&Shm_K300[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_l_lmt = Shm_Risk[0].S_Sise[8][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K300[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[8][ii].m_h_lmt = (double)AtoDf(&Shm_K300[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_h_lmt = Shm_Risk[0].S_Sise[8][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[8][ii].m_l_lmt = (double)AtoDf(&Shm_K300[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_K300[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[8][ii].m_l_lmt = Shm_Risk[0].S_Sise[8][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[8][ii].m_multiplier = AtoIf (&Shm_K300[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_closeday, Shm_K300[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[8][ii].basic_asset_price, Shm_K300[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[8][ii].m_group_id,
					Shm_K300[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].K300_Key[ii].idx = ii;
                memcpy (Shm_Item[0].K300_Key[ii].expcode, Shm_K300[ii].A0.item_code,
                                                    sizeof (Shm_K300[0].A0.item_code));
            }
            qsort (Shm_Item[0].K300_Key, Shm_K300[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A7991
	if (memcmp (p_buf, "A0024", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_K150F[0].A0.seq_no));
        memcpy (Shm_K150F[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_K150F[0].total_item_cnt < idx) Shm_K150F[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_K150F[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[9] = Shm_K150F[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_K150F[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[9][ii].crprc = (double)AtoIf(&Shm_K150F[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_item_cd,
					Shm_K150F[ii].A0.item_code, sizeof (Shm_K150F[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_item_stat,
					Shm_K150F[ii].A0.trade_stop, sizeof (Shm_K150F[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[9][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[9][ii].m_stdard_price =
					(double)AtoDf (&Shm_K150F[ii].A0.stprc[3], sizeof (Shm_K150F[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[9][ii].nb_shares_stock_1per,
					&Shm_K150F[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_1 = AtoIf (&Shm_K150F[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_2 = AtoIf (&Shm_K150F[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_I = AtoIf (&Shm_K150F[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_X = AtoIf (&Shm_K150F[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용)
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_Y = AtoIf (&Shm_K150F[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_T = AtoIf (&Shm_K150F[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[9][ii].m_cds_gbn_W = AtoIf (&Shm_K150F[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[9][ii].m_max_qty = (double)AtoDf(&Shm_K150F[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_clearance_gbn, Shm_K150F[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_start_price_gbn, Shm_K150F[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[9][ii].m_top_price = (double)AtoDf(Shm_K150F[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[9][ii].m_lowest_price = (double)AtoDf(Shm_K150F[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_K150F[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[9][ii].m_h_lmt = (double)AtoDf(&Shm_K150F[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_h_lmt = Shm_Risk[0].S_Sise[9][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[9][ii].m_l_lmt = (double)AtoDf(&Shm_K150F[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_l_lmt = Shm_Risk[0].S_Sise[9][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K150F[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[9][ii].m_h_lmt = (double)AtoDf(&Shm_K150F[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_h_lmt = Shm_Risk[0].S_Sise[9][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[9][ii].m_l_lmt = (double)AtoDf(&Shm_K150F[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_l_lmt = Shm_Risk[0].S_Sise[9][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K150F[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[9][ii].m_h_lmt = (double)AtoDf(&Shm_K150F[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_h_lmt = Shm_Risk[0].S_Sise[9][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[9][ii].m_l_lmt = (double)AtoDf(&Shm_K150F[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_K150F[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[9][ii].m_l_lmt = Shm_Risk[0].S_Sise[9][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[9][ii].m_multiplier = AtoIf (&Shm_K150F[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_closeday, Shm_K150F[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[9][ii].basic_asset_price, Shm_K150F[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[9][ii].m_group_id,
					Shm_K150F[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].K150F_Key[ii].idx = ii;
                memcpy (Shm_Item[0].K150F_Key[ii].expcode, Shm_K150F[ii].A0.item_code,
                                                    sizeof (Shm_K150F[0].A0.item_code));
            }
            qsort (Shm_Item[0].K150F_Key, Shm_K150F[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A7992
	if (memcmp (p_buf, "A0174", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_K150O[0].A0.seq_no));
        memcpy (Shm_K150O[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_K150O[0].total_item_cnt < idx) Shm_K150O[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_K150O[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[10] = Shm_K150O[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_K150O[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[10][ii].crprc = (double)AtoIf(&Shm_K150O[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_item_cd,
					Shm_K150O[ii].A0.item_code, sizeof (Shm_K150O[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_item_stat,
					Shm_K150O[ii].A0.trade_stop, sizeof (Shm_K150O[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[10][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[10][ii].m_stdard_price =
					(double)AtoDf (&Shm_K150O[ii].A0.stprc[3], sizeof (Shm_K150O[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[10][ii].nb_shares_stock_1per,
					&Shm_K150O[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_1 = AtoIf (&Shm_K150O[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_2 = AtoIf (&Shm_K150O[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_I = AtoIf (&Shm_K150O[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_X = AtoIf (&Shm_K150O[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용) 
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_Y = AtoIf (&Shm_K150O[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_T = AtoIf (&Shm_K150O[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[10][ii].m_cds_gbn_W = AtoIf (&Shm_K150O[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[10][ii].m_max_qty = (double)AtoDf(&Shm_K150O[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_clearance_gbn, Shm_K150O[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_start_price_gbn, Shm_K150O[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[10][ii].m_top_price = (double)AtoDf(Shm_K150O[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[10][ii].m_lowest_price = (double)AtoDf(Shm_K150O[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_K150O[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[10][ii].m_h_lmt = (double)AtoDf(&Shm_K150O[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_h_lmt = Shm_Risk[0].S_Sise[10][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[10][ii].m_l_lmt = (double)AtoDf(&Shm_K150O[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_l_lmt = Shm_Risk[0].S_Sise[10][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K150O[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[10][ii].m_h_lmt = (double)AtoDf(&Shm_K150O[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_h_lmt = Shm_Risk[0].S_Sise[10][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[10][ii].m_l_lmt = (double)AtoDf(&Shm_K150O[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_l_lmt = Shm_Risk[0].S_Sise[10][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_K150O[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[10][ii].m_h_lmt = (double)AtoDf(&Shm_K150O[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_h_lmt = Shm_Risk[0].S_Sise[10][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[10][ii].m_l_lmt = (double)AtoDf(&Shm_K150O[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_K150O[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[10][ii].m_l_lmt = Shm_Risk[0].S_Sise[10][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[10][ii].m_multiplier = AtoIf (&Shm_K150O[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_closeday, Shm_K150O[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[10][ii].basic_asset_price, Shm_K150O[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[10][ii].m_group_id,
					Shm_K150O[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].K150O_Key[ii].idx = ii;
                memcpy (Shm_Item[0].K150O_Key[ii].expcode, Shm_K150O[ii].A0.item_code,
                                                    sizeof (Shm_K150O[0].A0.item_code));
            }
            qsort (Shm_Item[0].K150O_Key, Shm_K150O[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A6591
	if (memcmp (p_buf, "A0124", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_MF[0].A0.seq_no));
        memcpy (Shm_MF[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_MF[0].total_item_cnt < idx) Shm_MF[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_MF[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[11] = Shm_MF[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_MF[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[11][ii].crprc = (double)AtoIf(&Shm_MF[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_item_cd,
					Shm_MF[ii].A0.item_code, sizeof (Shm_MF[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_item_stat,
					Shm_MF[ii].A0.trade_stop, sizeof (Shm_MF[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[11][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[11][ii].m_stdard_price =
					(double)AtoDf (&Shm_MF[ii].A0.stprc[3], sizeof (Shm_MF[0].A0.stprc))/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[11][ii].nb_shares_stock_1per,
					&Shm_MF[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_1 = AtoIf (&Shm_MF[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_2 = AtoIf (&Shm_MF[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_I = AtoIf (&Shm_MF[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_X = AtoIf (&Shm_MF[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용)
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_Y = AtoIf (&Shm_MF[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_T = AtoIf (&Shm_MF[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[11][ii].m_cds_gbn_W = AtoIf (&Shm_MF[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[11][ii].m_max_qty = (double)AtoDf(&Shm_MF[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_clearance_gbn, Shm_MF[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_start_price_gbn, Shm_MF[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[11][ii].m_top_price = (double)AtoDf(Shm_MF[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[11][ii].m_lowest_price = (double)AtoDf(Shm_MF[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_MF[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[11][ii].m_h_lmt = (double)AtoDf(&Shm_MF[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_h_lmt = Shm_Risk[0].S_Sise[11][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[11][ii].m_l_lmt = (double)AtoDf(&Shm_MF[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_l_lmt = Shm_Risk[0].S_Sise[11][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_MF[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[11][ii].m_h_lmt = (double)AtoDf(&Shm_MF[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_h_lmt = Shm_Risk[0].S_Sise[11][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[11][ii].m_l_lmt = (double)AtoDf(&Shm_MF[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_l_lmt = Shm_Risk[0].S_Sise[11][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_MF[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[11][ii].m_h_lmt = (double)AtoDf(&Shm_MF[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_h_lmt = Shm_Risk[0].S_Sise[11][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[11][ii].m_l_lmt = (double)AtoDf(&Shm_MF[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_MF[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[11][ii].m_l_lmt = Shm_Risk[0].S_Sise[11][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[11][ii].m_multiplier = AtoIf (&Shm_MF[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_closeday, Shm_MF[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[11][ii].basic_asset_price, Shm_MF[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[11][ii].m_group_id,
					Shm_MF[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].MF_Key[ii].idx = ii;
                memcpy (Shm_Item[0].MF_Key[ii].expcode, Shm_MF[ii].A0.item_code,
                                                    sizeof (Shm_MF[0].A0.item_code));
            }
            qsort (Shm_Item[0].MF_Key, Shm_MF[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#elif defined A6691
	if (memcmp (p_buf, "A0134", 5) == 0)
    {
        idx = AtoIf (&p_buf[M_H_SIZE], sizeof (Shm_MO[0].A0.seq_no));
        memcpy (Shm_MO[idx].A0.tr_gbn, p_buf, sizeof (SIF_A0014));

        if (Shm_MO[0].total_item_cnt < idx) Shm_MO[0].total_item_cnt = idx;

        /* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
        if (memcmp (&p_buf[18], "999999999999", sizeof (Shm_MO[0].A0.item_code)) == 0)
        {
			Shm_Risk[0].Max_Seq[12] = Shm_MO[0].total_item_cnt;	// 조회시 수신받은 seq가 Max를 넘는지 체크
            for (ii = 1; ii < Shm_MO[0].total_item_cnt; ii++)
            {
				Shm_Risk[0].S_Sise[12][ii].crprc = (double)AtoIf(&Shm_MO[ii].A0.stprc[3], 9)/100;
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_item_cd,
					Shm_MO[ii].A0.item_code, sizeof (Shm_MO[0].A0.item_code));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_item_stat,
					Shm_MO[ii].A0.trade_stop, sizeof (Shm_MO[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */
				memcpy (Shm_Risk[0].S_Sise[12][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[12][ii].m_stdard_price =
					(double)AtoDf (&Shm_MO[ii].A0.stprc[3], sizeof (Shm_MO[0].A0.stprc)-3)/100;
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만(현물용)  
				memcpy (Shm_Risk[0].S_Sise[12][ii].nb_shares_stock_1per,
					&Shm_MO[ii].A0.listed_stock[3], 10);	*/
				/* 시장가호가조건코드 (현물용)
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_1 = AtoIf (&Shm_MO[ii].A0.market_order[4], 1);	*/
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_2 = AtoIf (&Shm_MO[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_I = AtoIf (&Shm_MO[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) 
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_X = AtoIf (&Shm_MO[ii].A0.advantage_order[4], 1);	*/
				/* 최우선지정가 호가조건코드(미사용) 
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_Y = AtoIf (&Shm_MO[ii].A0.first_order[4], 1);	*/

				/* 시장가 호가조건코드(1대신 파생용) */
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_T = AtoIf (&Shm_MO[ii].A0.market_order[4], 1); 
				/* 최유리지정가 호가조건코드(X대신 파생용) */
				Shm_Risk[0].S_Sise[12][ii].m_cds_gbn_W = AtoIf (&Shm_MO[ii].A0.advantage_order[4], 1); 

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[12][ii].m_max_qty = (double)AtoDf(&Shm_MO[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용)
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_clearance_gbn, Shm_MO[ii].A0.clear_gubun, 1);		*/
				/* 시가기준가종목(현물용)
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_start_price_gbn, Shm_MO[ii].A0.open_st_price, 1);		*/
				/* 최고호가가격(현물용) 
				Shm_Risk[0].S_Sise[12][ii].m_top_price = (double)AtoDf(Shm_MO[ii].A0.high_bid, 9);	*/
				/* 최저호가가격(현물용) 
				Shm_Risk[0].S_Sise[12][ii].m_lowest_price = (double)AtoDf(Shm_MO[ii].A0.low_bid, 9);	*/
				if (memcmp (Shm_MO[ii].A0.limit_price_last_stat, "001", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[12][ii].m_h_lmt = (double)AtoDf(&Shm_MO[ii].A0.hlprc01[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.hlprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_h_lmt = Shm_Risk[0].S_Sise[12][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[12][ii].m_l_lmt = (double)AtoDf(&Shm_MO[ii].A0.llprc01[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.llprc01_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_l_lmt = Shm_Risk[0].S_Sise[12][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_MO[ii].A0.limit_price_last_stat, "002", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[12][ii].m_h_lmt = (double)AtoDf(&Shm_MO[ii].A0.hlprc02[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.hlprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_h_lmt = Shm_Risk[0].S_Sise[12][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[12][ii].m_l_lmt = (double)AtoDf(&Shm_MO[ii].A0.llprc02[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.llprc02_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_l_lmt = Shm_Risk[0].S_Sise[12][ii].m_l_lmt * -1;
				}
				else if (memcmp (Shm_MO[ii].A0.limit_price_last_stat, "003", 3) == 0)
				{
					/* 상한가 */
					Shm_Risk[0].S_Sise[12][ii].m_h_lmt = (double)AtoDf(&Shm_MO[ii].A0.hlprc03[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.hlprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_h_lmt = Shm_Risk[0].S_Sise[12][ii].m_h_lmt * -1;
					/* 하한가 */
					Shm_Risk[0].S_Sise[12][ii].m_l_lmt = (double)AtoDf(&Shm_MO[ii].A0.llprc03[3], 9)/100;
					if (memcmp (Shm_MO[ii].A0.llprc03_sign, "-", 1) == 0)
						Shm_Risk[0].S_Sise[12][ii].m_l_lmt = Shm_Risk[0].S_Sise[12][ii].m_l_lmt * -1;
				}
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[12][ii].m_multiplier = AtoIf (&Shm_MO[ii].A0.trade_multi[4], 9);
				/* 거래종료일, (파생용) */
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_closeday, Shm_MO[ii].A0.delist_date, 8); 
				/* 기초자산전일종가.. (파생용) 이건 위탁만 조사.. 뺀다.
				memcpy (Shm_Risk[0].S_Sise[12][ii].basic_asset_price, Shm_MO[ii].A0., 8);      */
				/* 증권 그룹ID (현물용)
				memcpy (Shm_Risk[0].S_Sise[12][ii].m_group_id,
					Shm_MO[ii].A0.group_id, 2); */
				/* *************************************** */

                Shm_Item[0].MO_Key[ii].idx = ii;
                memcpy (Shm_Item[0].MO_Key[ii].expcode, Shm_MO[ii].A0.item_code,
                                                    sizeof (Shm_MO[0].A0.item_code));
            }
            qsort (Shm_Item[0].MO_Key, Shm_MO[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
        }
    }
#endif

	return;
}	/* End of Set_Sise ()	*/

/*************************************************************************
	Function		: . Microsec_Sleep
	Parameters IN	: . msec	: sleep time (in microsec)
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . sleep for microsec
*************************************************************************/
void	Microsec_Sleep (int msec)
{
	int				rt;
	struct timespec	ts;

	ts.tv_sec = 0;
	ts.tv_nsec = msec * 1000;

	rt = nanosleep (&ts, NULL);

	if (rt == -1)
		Log (SYS_ERROR, "nanosleep fail {%d:%s}", SYS_NO, SYS_STR);

	return;
}	/* End of Microsec_Sleep ()	*/

/*************************************************************************
	End of program (pa_7111_dd.c)
*************************************************************************/ 
