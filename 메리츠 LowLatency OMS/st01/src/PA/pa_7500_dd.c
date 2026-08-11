#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: 시세 수신 (UDP)
#	File	: pa_7500_dd.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

/* 기본정보, 유가증권/코스닥(800), ELW(910) 기본 */
#if defined	A7501
#define     DATA_SIZE       910		/* 유가증권(800), ELW(910) 기본 */
#elif defined	A7502
#define     DATA_SIZE       160		/* S1(160) 기본 */
#elif defined	A7601
#define     DATA_SIZE       800		/* 코스닥(800) 기본 */
/* 유가증권 */
#elif defined A7511||A7512
#define     DATA_SIZE       160		/* A3011/A4011/M4011	*/
#elif defined A7521||A7522
#define     DATA_SIZE       800		/* B6011/B7011/B8011	*/
/* 코스닥 */
#elif defined A7611||A7612
#define     DATA_SIZE       160		/* A3012/A4012/M4012	*/
#elif defined A7621||A7622
#define     DATA_SIZE       560		/* B6012/B8012	*/
/* ELW */
#elif defined A7711||A7712
#define     DATA_SIZE       160		/* A3021	*/
#elif defined A7721||A7722
#define     DATA_SIZE       800		/* B6021	*/
#elif defined A7001
#define     DATA_SIZE       50		/* B6021	*/
#endif
#include    "buf_struct.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#define		DATA_TIME       60 * 1000
#define     S_H_SIZE        17                      /* TR(5) + Code(12) */
KS_EXPCODE	Key;
int			Che_Gbn;
char		ApType[10];
FILE_BUFF_FORMAT    W_Fmt;
BUFF_RW_HEAD        f_head;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_7500_DD (void);
void    Set_Sise (char *);
void	Microsec_Sleep (int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_7500_DD ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_7500_DD (void)
/*----------------------------------------------------------------------*/
{
	int				idx, rt, len, R_Cnt, i, g_p;
	double			stock_jasan, one_jisu, tot_jasan, hap_cnt, jisu_tot;
	char 			m_time[24], path[100], f_buf[100];
	char            W2_Fmt[2048];
	FILE			*fp;
	FILE_BUFF_FORMAT    R_Fmt[1];

/* 기초정보 재기동시 초기화 작업 */
#if defined A7501
	Shm_Stock[0].total_item_cnt = 0;
    Shm_Jisu[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[5] = 0;		// 조회시 수신받은 seq가 Max를 넘는지 체크

	/* A0 초기화 */
	for (i = 0; i < SHM_MAX_STOCK; i++)
	{
		memset (Shm_Stock[i].A0.tr_gbn, 	0, sizeof (STOCK_A0011));
		memset (Shm_Stock[i].ELW_A1.tr_gbn, 0, sizeof (ELW_A1011));
		memset (Shm_Stock[i].ETN_A1.tr_gbn, 0, sizeof (STOCK_A1041));
	}

	/* Key 초기화 */
	memset (Shm_Item[0].S_Key,		0,	sizeof (KS_EXPCODE) * SHM_MAX_STOCK);
	memset (Shm_Item[0].J_Key,		0,	sizeof (KS_EXPCODE) * SHM_MAX_JISU);
#elif defined A7502	// S1, 초기화 할것은 없다.
	sleep (10);		// A0011이 처리될 시간을 준다, 이게 실행된건 재기동시 이기 때문이다.
#elif defined A7601
    Shm_Kosdaq[0].total_item_cnt = 0;
	Shm_Risk[0].Max_Seq[6] = 0;		// 조회시 수신받은 seq가 Max를 넘는지 체크

    /* A0 초기화 */
    for (i = 0; i < SHM_MAX_KOSDAQ; i++)
    {
        memset (Shm_Kosdaq[i].A0.tr_gbn, 0, sizeof (STOCK_A0011));
	}

	memset (Shm_Item[0].K_Key,      0,  sizeof (KS_EXPCODE) * SHM_MAX_KOSDAQ);
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

			if ((memcmp (R_Fmt[0].Data, "A0", 2) != 0)	&&
				(memcmp (R_Fmt[0].Data, "A1", 2) != 0)	)
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

		rt = Poll_File (60*1000);

		if (rt == 1)
			SLog (USR_OK, "poll timeout <%d>", INT_SEQ);
		else if (rt == -1)
			continue;
	}

	return;
}	/* End of PA_7500_DD ()	*/

/*************************************************************************
    Function        : . Set_Sise
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Sise (char *p_buf)
/*----------------------------------------------------------------------*/
{
    int         idx, rt, ii;
	char		item_cd[20];
    char        TrCode[10];

/* *********************************** */
/* 시장별, TR별 종목 일련번호 갖고오기 */
/* *********************************** */
#if defined A7501
	if (memcmp (p_buf, "A0011", 5) == 0)
	{
		idx = AtoIf (&p_buf[S_H_SIZE], sizeof (Shm_Stock[0].A0.seq_no));
		memcpy (Shm_Stock[idx].A0.tr_gbn, p_buf, sizeof (STOCK_A0011));

		if (Shm_Stock[0].total_item_cnt < idx)
			Shm_Stock[0].total_item_cnt = idx;

		/* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
		if (memcmp (&p_buf[5], "999999999999", sizeof (Shm_Stock[0].A0.item_code)) == 0)
		{
			Shm_Risk[0].Max_Seq[5] = Shm_Stock[0].total_item_cnt;		// 조회시 수신받은 seq가 Max를 넘는지 체크
			for (ii = 1; ii < Shm_Stock[0].total_item_cnt; ii++)
			{
				Shm_Risk[0].S_Sise[5][ii].crprc =
					(double)AtoIf(Shm_Stock[ii].A0.stprc, sizeof (Shm_Stock[0].A0.stprc));
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[5][ii].m_item_cd,
					Shm_Stock[ii].A0.item_code, sizeof (Shm_Stock[0].A0.item_code));
				/* 상한가 */
				Shm_Risk[0].S_Sise[5][ii].m_h_lmt =
					(double)AtoDf (Shm_Stock[ii].A0.high_limit_price, sizeof (Shm_Stock[0].A0.high_limit_price));
				/* 하한가 */
				Shm_Risk[0].S_Sise[5][ii].m_l_lmt =
					(double)AtoDf (Shm_Stock[ii].A0.low_limit_price, sizeof (Shm_Stock[0].A0.low_limit_price));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[5][ii].m_item_stat,
					Shm_Stock[ii].A0.trade_stop, sizeof (Shm_Stock[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */ 
				memcpy (Shm_Risk[0].S_Sise[5][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[5][ii].m_stdard_price =
					(double)AtoDf (Shm_Stock[ii].A0.stprc, sizeof (Shm_Stock[0].A0.stprc));
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만, (현물용)  */
				memcpy (Shm_Risk[0].S_Sise[5][ii].nb_shares_stock_1per,
					&Shm_Stock[ii].A0.listed_stock[3], 10);
				/* 시장가호가조건코드 (현물용)*/
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_1 = AtoIf (&Shm_Stock[ii].A0.market_order[4], 1);
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_2 = AtoIf (&Shm_Stock[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_I = AtoIf (&Shm_Stock[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) */
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_X = AtoIf (&Shm_Stock[ii].A0.advantage_order[4], 1);
				/* 최우선지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_Y = AtoIf (&Shm_Stock[ii].A0.first_order[4], 1);

				/* 시장가 호가조건코드(1대신 파생용) 
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_T = AtoIf (&Shm_Stock[ii].A0.market_order[4], 1);	*/
				/* 최유리지정가 호가조건코드(X대신 파생용) 
				Shm_Risk[0].S_Sise[5][ii].m_cds_gbn_W = AtoIf (&Shm_Stock[ii].A0.advantage_order[4], 1);	*/

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[5][ii].m_max_qty = (double)AtoDf(&Shm_Stock[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부 */
				memcpy (Shm_Risk[0].S_Sise[5][ii].m_clearance_gbn,
					Shm_Stock[ii].A0.clear_gubun, 1);
				/* 시가기준가종목 */
				memcpy (Shm_Risk[0].S_Sise[5][ii].m_start_price_gbn,
					Shm_Stock[ii].A0.open_st_price, 1);
				/* 최고호가가격 */
				Shm_Risk[0].S_Sise[5][ii].m_top_price = (double)AtoDf(Shm_Stock[ii].A0.high_bid, 9);
				/* 최저호가가격 */
				Shm_Risk[0].S_Sise[5][ii].m_lowest_price = (double)AtoDf(Shm_Stock[ii].A0.low_bid, 9);
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[5][ii].m_multiplier = 1;
				/* 거래종료일, 파생만 있다 
				memcpy (Shm_Risk[0].S_Sise[5][ii].m_closeday,
					Shm_Stock[ii].A0.delist_date, 8);		*/
				/* 기초자산전일종가.. 이건 위탁만 조사.. 뺀다. 
				memcpy (Shm_Risk[0].S_Sise[5][ii].basic_asset_price,
					Shm_Stock[ii].A0., 8);		*/
				/* 증권 그룹ID (현물용) */
                memcpy (Shm_Risk[0].S_Sise[5][ii].m_group_id,
                                      Shm_Stock[ii].A0.group_id, 2); 
				/* *************************************** */

				Shm_Item[0].S_Key[ii].idx = ii;
				memcpy (Shm_Item[0].S_Key[ii].expcode, Shm_Stock[ii].A0.item_code,
													sizeof (Shm_Stock[0].A0.item_code));
			}
			qsort (Shm_Item[0].S_Key, Shm_Stock[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
		}
	}
	else if ((memcmp (p_buf, "A1011", 5) == 0) &&
			 (memcmp (&p_buf[5], "999999999999", 12) != 0))
	{
		memset (&Key, 0, sizeof (Key));
		sprintf (Key.expcode, "%-12.12s", &p_buf[5]);

		rt = 0;
		rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char*)&Key);
		if (rt > 0)	//	A1의 위치를 A0에 넣고, A0의 위치를 A에 넣는다.
			memcpy (Shm_Stock[rt].ELW_A1.tr_gbn, p_buf, sizeof (ELW_A1011));
		else
			Log (USR_ERROR, "A0에 A1011의 종목코드가 존재하지 않는다. ELW_ITEM_CD[%12.12s]", &p_buf[5]);
	}
	else if ((memcmp (p_buf, "A1041", 5) == 0) &&
			 (memcmp (&p_buf[5], "999999999999", 12) != 0))
	{
		memset (&Key, 0, sizeof (Key));
		sprintf (Key.expcode, "%-12.12s", &p_buf[5]);

		rt = 0;
		rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char*)&Key);
		if (rt > 0)	//	A1의 위치를 A0에 넣고, A0의 위치를 A에 넣는다.
			memcpy (Shm_Stock[rt].ETN_A1.tr_gbn, p_buf, sizeof (STOCK_A1041));
		else
			Log (USR_ERROR, "A0에 A1041의 종목코드가 존재하지 않는다. ETN_ITEM_CD[%12.12s]", &p_buf[5]);
	}
#elif defined A7601
	if (memcmp (p_buf, "A0012", 5) == 0)
	{
		idx = AtoIf (&p_buf[S_H_SIZE], sizeof (Shm_Kosdaq[0].A0.seq_no));
		memcpy (Shm_Kosdaq[idx].A0.tr_gbn, p_buf, sizeof (STOCK_A0011));

		if (Shm_Kosdaq[0].total_item_cnt < idx)
			Shm_Kosdaq[0].total_item_cnt = idx;

		/* 시세마지막을 수신하면 종목코드 q sort처리하여 Key보관 */
		if (memcmp (&p_buf[5], "999999999999", sizeof (Shm_Kosdaq[0].A0.item_code)) == 0)
		{
			Shm_Risk[0].Max_Seq[6] = Shm_Kosdaq[0].total_item_cnt;		// 조회시 수신받은 seq가 Max를 넘는지 체크
			for (ii = 1; ii < Shm_Kosdaq[0].total_item_cnt; ii++)
			{
				Shm_Risk[0].S_Sise[6][ii].crprc =
					(double)AtoIf(Shm_Kosdaq[ii].A0.stprc, sizeof (Shm_Kosdaq[0].A0.stprc));
				/* *************************************** */
				/* 한도체크용 STANDARD_SISE_FORMAT Setting */
				/* 종목코드 */
				memcpy (Shm_Risk[0].S_Sise[6][ii].m_item_cd,
					Shm_Kosdaq[ii].A0.item_code, sizeof (Shm_Kosdaq[0].A0.item_code));
				/* 상한가 */
				Shm_Risk[0].S_Sise[6][ii].m_h_lmt =
					(double)AtoDf (Shm_Kosdaq[ii].A0.high_limit_price, sizeof (Shm_Kosdaq[0].A0.high_limit_price));
				/* 하한가 */
				Shm_Risk[0].S_Sise[6][ii].m_l_lmt =
					(double)AtoDf (Shm_Kosdaq[ii].A0.low_limit_price, sizeof (Shm_Kosdaq[0].A0.low_limit_price));
				/* 종목거래가능 여부 */
				memcpy (Shm_Risk[0].S_Sise[6][ii].m_item_stat,
					Shm_Kosdaq[ii].A0.trade_stop, sizeof (Shm_Kosdaq[0].A0.trade_stop));
				/* 종목장운영정보에 의한 종목거래가능 여부, 초기화만 */ 
				memcpy (Shm_Risk[0].S_Sise[6][ii].item_stat, " ", 1);
				/* 기준가격 */
				Shm_Risk[0].S_Sise[6][ii].m_stdard_price =
					(double)AtoDf (Shm_Kosdaq[ii].A0.stprc, sizeof (Shm_Kosdaq[0].A0.stprc));
				/* 상장주식수의 1%, 상장수 15자리중 1%에 해당하는 자리수만 (현물용)  */
				memcpy (Shm_Risk[0].S_Sise[6][ii].nb_shares_stock_1per,
					&Shm_Kosdaq[ii].A0.listed_stock[3], 10);
				/* 시장가호가조건코드 (현물용)*/
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_1 = AtoIf (&Shm_Kosdaq[ii].A0.market_order[4], 1);
				/* 지정가호가조건코드 */
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_2 = AtoIf (&Shm_Kosdaq[ii].A0.limits_order[4], 1);
				/* 조건부지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_I = AtoIf (&Shm_Kosdaq[ii].A0.condition_order[4], 1);
				/* 최유리지정가 호가조건코드(현물용) */
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_X = AtoIf (&Shm_Kosdaq[ii].A0.advantage_order[4], 1);
				/* 최우선지정가 호가조건코드 */
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_Y = AtoIf (&Shm_Kosdaq[ii].A0.first_order[4], 1);

				/* 시장가 호가조건코드(1대신 파생용) 
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_T = AtoIf (&Shm_Kosdaq[ii].A0.market_order[4], 1);	*/
				/* 최유리지정가 호가조건코드(X대신 파생용) 
				Shm_Risk[0].S_Sise[6][ii].m_cds_gbn_W = AtoIf (&Shm_Kosdaq[ii].A0.advantage_order[4], 1);	*/

				/* 상한수량, 16자리인데 종목수량은 10이라 10자리만 처리한다. */
				Shm_Risk[0].S_Sise[6][ii].m_max_qty = (double)AtoDf(&Shm_Kosdaq[ii].A0.hi_limit_cnt[6], 10);
				/* 정리매매 여부(현물용) */
				memcpy (Shm_Risk[0].S_Sise[6][ii].m_clearance_gbn,
					Shm_Kosdaq[ii].A0.clear_gubun, 1);
				/* 시가기준가종목(현물용) */
				memcpy (Shm_Risk[0].S_Sise[6][ii].m_start_price_gbn,
					Shm_Kosdaq[ii].A0.open_st_price, 1);
				/* 최고호가가격 */
				Shm_Risk[0].S_Sise[6][ii].m_top_price = (double)AtoDf(Shm_Kosdaq[ii].A0.high_bid, 9);
				/* 최저호가가격 */
				Shm_Risk[0].S_Sise[6][ii].m_lowest_price = (double)AtoDf(Shm_Kosdaq[ii].A0.low_bid, 9);
				/* 승수, 현물은 없으니 1로 */
				Shm_Risk[0].S_Sise[6][ii].m_multiplier = 1;
				/* 거래종료일, 파생만 있다 
				memcpy (Shm_Risk[0].S_Sise[6][ii].m_closeday,
					Shm_Kosdaq[ii].A0.delist_date, 8);		*/
				/* 기초자산전일종가.. 이건 위탁만 조사.. 뺀다. 
				memcpy (Shm_Risk[0].S_Sise[6][ii].basic_asset_price,
					Shm_Kosdaq[ii].A0., 8);		*/
				/* 증권 그룹ID (현물용) */
                memcpy (Shm_Risk[0].S_Sise[6][ii].m_group_id,
                                      Shm_Kosdaq[ii].A0.group_id, 2); 
				/* *************************************** */

				Shm_Item[0].K_Key[ii].idx = ii;
				memcpy (Shm_Item[0].K_Key[ii].expcode, Shm_Kosdaq[ii].A0.item_code,
													sizeof (Shm_Kosdaq[0].A0.item_code));
			}
			qsort (Shm_Item[0].K_Key, Shm_Kosdaq[0].total_item_cnt, sizeof (KS_EXPCODE), CmpExpcode);
		}
	}
#elif defined A7502
	if (memcmp (p_buf, "S1011", 5) == 0)
	{
		memset (&Key, 0, sizeof (Key));
		sprintf (Key.expcode, "%-12.12s", &p_buf[5]);
		rt = 0;
		rt = Key_Search (MK_KOSPI, KEY_EXPCODE, (char*)&Key);
		if (rt > 0)
		{
			idx = rt;
			memcpy (Shm_Stock[idx].S1.tr_gbn, p_buf, sizeof (STOCK_S1011));
		}
		else
			Log (USR_ERROR, "S1 Itemcode Not Search [%12.12s]", &p_buf[5]);
	}
#endif

    return;
}   /* End of Set_Sise ()   */

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
	End of program (pa_7500_dd.c)
*************************************************************************/ 
