#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Server 장개시전 전략(2010.09)
#	File	: pa_5010_mp.c
------------------------------------------------------------------------*/
/* -------------------------------------------------------------------- */
/* 입력항목 : 1. 행사가 From ~ To.										*/
/*				 예, 25000, 26000 250씩 더해서 총 5개 행사가			*/
/*					 25000,25250,25500,25750,25600 						*/
/* -------------------------------------------------------------------- */
/* System Risk 관리(공용, 한 Packet 기준) 								*/
/* - 선물수량한도 :    50계약, 선물금액한도 : 75억						*/
/* - 옵션수량한도 : 1,000계약, 옵션금액한도 :  3억						*/
/* -------------------------------------------------------------------- */

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"
#include	"pa_struct.h"

#define		DATA_SIZE	420
#include	"buf_struct.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		DATA_TIME	60 * 1000
#define		READ_MAX	1

#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
char	Order_Ohd[83] = {"KMAPv1.0000300TCHODR00000           00021                                         "};

/* ############################ # ************************************* */
/* 1. 종목코드(12)              # 1. 매도매수 구분 1:매도, 2:매수       */
/* 2. 계좌번호12자리중 6자리    # 2. 호가수량(10) Long 예)"0000000010"  */
/* 3. 일자 (8)                  # 3. 호가가격(11) Flat 예)"00000012.21" */
/* 4. 회원사처리항목 (6)        # 4. 주문시각 (9) 예) "101010888"       */
/* ############################ # ************************************* */

char    O_Order_Dat[301] = {"00000000000TCHODR1000110002100999                    ############*1000999######**********/*******.**20000000000000    0001031     00            41114101000001172021101020/#######         ######                                                                                                           "};
char    F_Order_Dat[301] = {"00000000000TCHODR1000110002100999                    ############*1000999######**********/*******.**20000000000000    0001031     00            41114101000001172021101020/#######         ######                                                                                                           "};
/* 호가가격 부터 취소시 초기화 처리 */

int		Pk_mp;
int		In_Strategy, ApCh;

int		y_bs, max_su, max_oticks, tot_strike;
int		ju_biyul, inbuf, trigger, Op_bi_su, Fu_bi_su;	/* 100 곱해서 수신 받는다 */
int		call_seq[10], put_seq[10];

char				ApType[10], Cli_handler[5];
char				W_DFmt[512];
char				F_Order_St[512], O_Order_St[512];
FILE_BUFF_FORMAT	W_Fmt, R_Fmt[READ_MAX];
struct pollfd       Poll[3];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PA_5010_MP (void);
int		Init_Parameters (void);
void	Check_Proc_Status (void);
int		Write_Data (int, int);

/* ****************** */
/* 1 : 신규, 2 : 정정 */
/* 1 : 매도, 2 : 매수 */
/* 0~1 : Sub-set 일련번호 */
/* 0 : 첫번째 종목, 1 : 두번째 종목 */
/* 가격 : 예, 27610					*/
/* 수량 :							*/
/* ******************************** */
void	Mem_Write_Data_F (int, int, int, int, int, int);
void	Mem_Write_Data_O (int, int, int, int, int, int);
void	Poll_Wait (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PA_5010_MP ();
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PA_5010_MP (void)
/*----------------------------------------------------------------------*/
{
	int		i, j, rt, bi_y, rc, ju_edt;
	int		as_p, o_price, o_ticks01, o_ticks03;
	int		call_jan, call_ticks, put_jan, put_ticks, fu_jan, jm_su;
	int		call_price, put_price, fu_price, for_cnt, op_gbn;
	int		f_rp_c, f_rp_p, ju_gbn, o_rp;
	char	t_time[12];

	rt = Init_Parameters ( );

	if (rt == OK)
		rt = Write_Data (2,1);

	if (rt != OK)
	{
		rt = Write_Data (1,2);
		if (rt != OK)
			Exit_Process ();	

		rt = Write_Data (10,1);
		if (rt != OK)
			Exit_Process ();	

		sleep (1);
	}

	while (START_S != JOB_END)
	{
		Stat_Save ();

		while (START_S != JOB_END)
		{
			Poll_Wait ( );

			if ((START_S == JOB_END) ||
				(AtoIf (Shm_Futures[0].Futures_CURR.tot_con_qty, 7) <= 0))
				break;
/* *********************************************************************************** */
/* 자동로직 시작																	   */
/* *********************************************************************************** */

			/* 여기서 부터 시세에 의한 처리 */
			rc = Shm_Db[0].recv_seq;
			/* ju_gbn, 0:break 1:신규주문(선/옵) 5:정정주문(선/옵)	*/
			ju_gbn = 1;
			op_gbn = 9;

			/* 선물 시세시 tot_strike만큼 값처리를 한다 */
			if (rc == 999)
			{
				j = i = 0;
				ju_gbn = 1;
				for_cnt = tot_strike;
			}
			else
			{
				for_cnt = 1;

				for (i = 0; i < tot_strike; i++)
				{
					if ((Shm_Db[0].Ju_S_History[ApCh].sub_set[i].ju_set[0].Jong_Seq == rc)	||
						(Shm_Db[0].Ju_S_History[ApCh].sub_set[i].ju_set[1].Jong_Seq == rc))
					{
						if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run == 9)
							ju_gbn = 0;
						else if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run == 0)
							ju_gbn = 1;
						else
							ju_gbn = 5;

						if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].ju_set[0].Jong_Seq == rc)
							op_gbn = 0;
						else
							op_gbn = 1;
							
						j = i;

						break;
					}

					if (i >= tot_strike - 1)
						ju_gbn = 0;
				}

				if (ju_gbn == 0)
					break;

			}
Log (USR_OK, "TEST01 ju_gbn[%d] rc[%d] j[%d] for_cnt[%d]", ju_gbn, rc, j, for_cnt);

			/* 1. 나갈 sub_set이 있냐 */
			/* 2. 수정할 sub_set이 있냐. */
			if (ju_gbn == 1)
			{
				for (j = j; j < for_cnt; j ++)
				{
Log (USR_OK, "TEST02 j[%d] sub_auto_run[%d]",
	j, Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run);
Log (USR_OK, "TEST03 call_seq[%d] put_seq[%d]", call_seq[j], put_seq[j]);

					if (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run == 0)
					{
						if ((AtoIf (Shm_Options[call_seq[j]].Options_CURR.tot_con_qty, 8) > 0)	&&
							(AtoIf (Shm_Options[put_seq[j]].Options_CURR.tot_con_qty, 8) > 0))
						{
							Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hmd =
								(AtoIf (Shm_Options[call_seq[j]].Options_A0.striking_price, 11) +
								 AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid, 5)  -
								 AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid, 5))		-
								(AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid, 5) - y_bs);

							Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hms =
								(AtoIf (Shm_Options[call_seq[j]].Options_A0.striking_price, 11) +
								 AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid, 5)  -
								 AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid, 5))		-
								(AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid, 5) - y_bs);

Log (USR_OK, "TEST04 j[%d] hmd[%d] hms[%d] trigger[%d] y_bs[%d]",
	j, Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hmd,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hms, trigger, y_bs);
Log (USR_OK, "TEST05 c_st_p[%11.11s] p_st_p[%11.11s] c_b_p[%5.5s] c_s_p[%5.5s] p_b_p[%5.5s] p_s_p[%5.5s] f_p[%5.5s]",
	Shm_Options[call_seq[j]].Options_A0.striking_price,
	Shm_Options[put_seq[j]].Options_A0.striking_price,
	Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid,
	Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid,
	Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid,
	Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid,
	Shm_Futures[0].Futures_CURR.buy_1_prmbid);

							if (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hmd - trigger > 0)
							{
								o_price = (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hmd - inbuf) / 2;
								if (o_price < 0)
								{
									Log (USR_OK, "HMD o_price[%d] Not Case j[%d]", o_price, j);
									Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run = 9;

									/* 정상종료 처리 */
									for (i = 0; i < tot_strike; i ++)
									{
										if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run != 9)
											break;

										if (i >= tot_strike - 1)
										{
											rt = Write_Data (1,2);
											if (rt != OK)
												Exit_Process ();

											rt = Write_Data (19,1);
											if (rt != OK)
											{
												Log (USR_ERROR, "OK Write Err 19");
												Exit_Process ();
											}
										}
									}

									break;
								}

								if (max_oticks > (o_price / 1))
									o_ticks01 =  (o_price / 1) / 1;
								else
									o_ticks01 = max_oticks;

								if (max_oticks > (o_price / 5))
									o_ticks03 =  (o_price / 5) / 1;
								else
									o_ticks03 = max_oticks;

Log (USR_OK, "hmd max_oticks[%d] o_price[%d] o_ticks01[%d] o_ticks03[%d]",
	max_oticks, o_price, o_ticks01, o_ticks03);

								/* 콜잔량 누적하기 */
								if (AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_5_prmbid, 5) >= 300)
								{
									call_ticks = o_ticks03;
									call_price = - (call_ticks * 5) +
												 AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid, 5);
								}
								else
								{
									call_ticks = o_ticks01;
									call_price = - call_ticks +
												 AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid, 5);
									if (call_price < 1)
										call_price = 1;

									if (call_price > 300 && call_price % 5)
										call_price = call_price + (5 - (call_price % 5));
								}

								call_jan = 0;
								for (i = 0; i < call_ticks + 1; i ++)
								{
									if (i == 0)
										call_jan = AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid_qty, 7);
									else if (i == 1)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_2_prmbid_qty, 7);
									else if (i == 2)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_3_prmbid_qty, 7);
									else if (i == 3)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_4_prmbid_qty, 7);
									else if (i == 4)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_5_prmbid_qty, 7);
									else
										break;
								}
								call_jan = call_jan / 5;

Log (USR_OK, "TEST_hmd call_jan[%d] call_seq[%d] call_ticks[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	call_jan, call_seq[j], call_ticks,
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_1_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_2_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_3_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_4_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.buy_5_prmbid_qty, 7));
								
								/* 풋잔량 누적하기 */
								if (AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid, 5) >= 300)
								{
									put_ticks = o_ticks03;
									put_price = (put_ticks * 5) +
												 AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid, 5);
								}
								else
								{
									put_ticks = o_ticks01;
									put_price = put_ticks +
												 AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid, 5);

									if (put_price >= 300 && put_price % 5)
										put_price = put_price - (put_price % 5);
								}

								put_jan = 0;
								for (i = 0; i < put_ticks + 1; i ++)
								{
									if (i == 0)
										put_jan = AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid_qty, 7);
									else if (i == 1)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_2_prmbid_qty, 7);
									else if (i == 2)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_3_prmbid_qty, 7);
									else if (i == 3)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_4_prmbid_qty, 7);
									else if (i == 4)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_5_prmbid_qty, 7);
									else
										break;
								}
								put_jan = put_jan / 5;

Log (USR_OK, "TEST_hmd put_jan[%d] put_seq[%d] put_ticks[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	put_jan, put_seq[j], put_ticks,
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_1_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_2_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_3_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_4_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.sel_5_prmbid_qty, 7));
								
								fu_jan = 0;
								/* 선물 잔량누적하기 */
								for (i = 0; i < o_ticks03 + 1; i ++)
								{
									if (i == 0)
										fu_jan = AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid_qty, 6);
									else if (i == 1)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.sel_2_prmbid_qty, 6);
									else if (i == 2)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.sel_3_prmbid_qty, 6);
									else if (i == 3)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.sel_4_prmbid_qty, 6);
									else if (i == 4)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.sel_5_prmbid_qty, 6);
									else
										break;
								}

								fu_price =  (o_ticks03 * 5) +
											AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid, 5);

Log (USR_OK, "TEST_hmd fu_jan[%d] o_ticks03[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	fu_jan, o_ticks03,
	AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid_qty, 7),
	AtoIf (Shm_Futures[0].Futures_CURR.sel_2_prmbid_qty, 7),
	AtoIf (Shm_Futures[0].Futures_CURR.sel_3_prmbid_qty, 7),
	AtoIf (Shm_Futures[0].Futures_CURR.sel_4_prmbid_qty, 7),
	AtoIf (Shm_Futures[0].Futures_CURR.sel_5_prmbid_qty, 7));
								
								if (put_jan > call_jan)
									jm_su = call_jan;
								else
									jm_su = put_jan;

								if (jm_su > fu_jan)
									jm_su = fu_jan;

								if (jm_su > max_su)
									jm_su = max_su;

								jm_su = (jm_su * ju_biyul) / 100;

								if (jm_su <= 0)
								{
									Log (USR_OK, "HMD jm_su Not Case j[%d] jm_su[%d]", j, jm_su);
									Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run = 9;

									/* 정상종료 처리 */
									for (i = 0; i < tot_strike; i ++)
									{
										if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run != 9)
											break;

										if (i >= tot_strike - 1)
										{
											rt = Write_Data (1,2);
											if (rt != OK)
												Exit_Process ();

											rt = Write_Data (19,1);
											if (rt != OK)
											{
												Log (USR_ERROR, "OK Write Err 19");
												Exit_Process ();
											}
										}
									}
									break;
								}
Log (USR_OK, "TEST_hmd jm_su[%d] call_jan[%d] max_su[%d] put_jan[%d] fu_jan[%d] ju_biyul[%d] E[%1.1s][%1.1s][%1.1s]",
	jm_su, call_jan, max_su, put_jan, fu_jan, ju_biyul,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].End_Flag);

								if (put_jan > call_jan)
								{
									/* call, futures, put순 */
									Mem_Write_Data_O (1, 1, j, 0, call_price, jm_su * 5);
									Mem_Write_Data_F (1, 2, j, 2, fu_price, jm_su);
									Mem_Write_Data_O (1, 2, j, 1, put_price, jm_su * 5);
								}
								else
								{
									/* put, futures, call순 */
									Mem_Write_Data_O (1, 2, j, 1, put_price, jm_su * 5);
									Mem_Write_Data_F (1, 2, j, 2, fu_price, jm_su);
									Mem_Write_Data_O (1, 1, j, 0, call_price, jm_su * 5);
								}
Log (USR_OK, "TEST E[%1.1s][%1.1s][%1.1s]",
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].End_Flag);

							}
							else if (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hms + trigger < 0)
							{
								o_price = (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hms + inbuf) / 2;
								o_price = -1 * o_price;

								if (o_price < 0)
								{
									Log (USR_OK, "HMS o_price[%d] Not Case j[%d]", o_price, j);
									Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run = 9;

									/* 정상종료 처리 */
									for (i = 0; i < tot_strike; i ++)
									{
										if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run != 9)
											break;

										if (i >= tot_strike - 1)
										{
											rt = Write_Data (1,2);
											if (rt != OK)
												Exit_Process ();

											rt = Write_Data (19,1);
											if (rt != OK)
											{
												Log (USR_ERROR, "OK Write Err 19");
												Exit_Process ();
											}
										}
									}

									break;
								}

								if (max_oticks > (o_price / 1))
									o_ticks01 =  (o_price / 1);
								else
									o_ticks01 = max_oticks;

								if (max_oticks > (o_price / 5))
									o_ticks03 =  (o_price / 5);
								else
									o_ticks03 = max_oticks;

Log (USR_OK, "hms max_oticks[%d] o_price[%d] o_ticks01[%d] o_ticks03[%d] inbuf[%d]",
	max_oticks, o_price, o_ticks01, o_ticks03, inbuf);

								/* 콜잔량 누적하기 */
								if (AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid, 5) >= 300)
								{
									call_ticks = o_ticks03;
									call_price = (call_ticks * 5) +
												 AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid, 5);
								}
								else
								{
									call_ticks = o_ticks01;
									call_price = call_ticks +
												 AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid, 5);

									if (call_price >= 300 && call_price % 5)
										call_price = call_price - (call_price % 5);
								}

								call_jan = 0;
								for (i = 0; i < call_ticks + 1; i ++)
								{
									if (i == 0)
										call_jan = AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid_qty, 7);
									else if (i == 1)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_2_prmbid_qty, 7);
									else if (i == 2)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_3_prmbid_qty, 7);
									else if (i == 3)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_4_prmbid_qty, 7);
									else if (i == 4)
										call_jan += AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_5_prmbid_qty, 7);
									else
										break;
								}
								call_jan = call_jan / 5;
								
Log (USR_OK, "TEST_hms call_jan[%d] call_seq[%d] call_ticks[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	call_jan, call_seq[j], call_ticks,
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_1_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_2_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_3_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_4_prmbid_qty, 7),
	AtoIf (Shm_Options[call_seq[j]].Options_CURR.sel_5_prmbid_qty, 7));
								
								/* 풋잔량 누적하기 */
								if (AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_5_prmbid, 5) >= 300)
								{
									put_ticks = o_ticks03;
									put_price = - (put_ticks * 5) +
												 AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid, 5);
								}
								else
								{
									put_ticks = o_ticks01;
									put_price = - put_ticks +
												 AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid, 5);
									if (put_price < 1)
										put_price = 1;

									if (put_price > 300 && put_price % 5)
										put_price = put_price + (5 - (put_price % 5));
								}

								put_jan = 0;
								for (i = 0; i < put_ticks + 1; i ++)
								{
									if (i == 0)
										put_jan = AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid_qty, 7);
									else if (i == 1)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_2_prmbid_qty, 7);
									else if (i == 2)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_3_prmbid_qty, 7);
									else if (i == 3)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_4_prmbid_qty, 7);
									else if (i == 4)
										put_jan += AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_5_prmbid_qty, 7);
									else
										break;
								}
								put_jan = put_jan / 5;

Log (USR_OK, "TEST_hms put_jan[%d] put_seq[%d] put_ticks[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	put_jan, put_seq[j], put_ticks,
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_1_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_2_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_3_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_4_prmbid_qty, 7),
	AtoIf (Shm_Options[put_seq[j]].Options_CURR.buy_5_prmbid_qty, 7));
								
								fu_jan = 0;
								/* 선물 잔량누적하기 */
								for (i = 0; i < o_ticks03 + 1; i ++)
								{
									if (i == 0)
										fu_jan = AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid_qty, 6);
									else if (i == 1)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.buy_2_prmbid_qty, 6);
									else if (i == 2)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.buy_3_prmbid_qty, 6);
									else if (i == 3)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.buy_4_prmbid_qty, 6);
									else if (i == 4)
										fu_jan += AtoIf (Shm_Futures[0].Futures_CURR.buy_5_prmbid_qty, 6);
									else
										break;
								}

								fu_price =  - (o_ticks03 * 5) +
											AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid, 5);

Log (USR_OK, "TEST_hms fu_jan[%d] o_ticks03[%d] 01[%d] 02[%d] 03[%d] 04[%d] 05[%d]",
	fu_jan, o_ticks03,
	AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid_qty, 6),
	AtoIf (Shm_Futures[0].Futures_CURR.buy_2_prmbid_qty, 6),
	AtoIf (Shm_Futures[0].Futures_CURR.buy_3_prmbid_qty, 6),
	AtoIf (Shm_Futures[0].Futures_CURR.buy_4_prmbid_qty, 6),
	AtoIf (Shm_Futures[0].Futures_CURR.buy_5_prmbid_qty, 6));
								
								if (put_jan > call_jan)
									jm_su = call_jan;
								else
									jm_su = put_jan;

								if (jm_su > fu_jan)
									jm_su = fu_jan;

								if (jm_su > max_su)
									jm_su = max_su;

								jm_su = (jm_su * ju_biyul) / 100;

								if (jm_su <= 0)
								{
									Log (USR_OK, "HMS jm_su Not Case j[%d] jm_su[%d]", j, jm_su);
									Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run = 9;

									/* 정상종료 처리 */
									for (i = 0; i < tot_strike; i ++)
									{
										if (Shm_Db[0].Ju_S_History[ApCh].sub_set[i].sub_auto_run != 9)
											break;

										if (i >= tot_strike - 1)
										{
											rt = Write_Data (1,2);
											if (rt != OK)
												Exit_Process ();

											rt = Write_Data (19,1);
											if (rt != OK)
											{
												Log (USR_ERROR, "OK Write Err 19");
												Exit_Process ();
											}
										}
									}
									break;
								}

Log (USR_OK, "TEST_hms jm_su[%d] call_jan[%d] max_su[%d] put_jan[%d] fu_jan[%d] ju_biyul[%d] E[%1.1s][%1.1s][%1.1s]",
	jm_su, call_jan, max_su, put_jan, fu_jan, ju_biyul,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].End_Flag);

								if (put_jan > call_jan)
								{
									/* call, futures, put순 */
									Mem_Write_Data_O (1, 2, j, 0, call_price, jm_su * 5);
									Mem_Write_Data_F (1, 1, j, 2, fu_price, jm_su);
									Mem_Write_Data_O (1, 1, j, 1, put_price, jm_su * 5);
								}
								else
								{
									/* put, futures, call순 */
									Mem_Write_Data_O (1, 1, j, 1, put_price, jm_su * 5);
									Mem_Write_Data_F (1, 1, j, 2, fu_price, jm_su);
									Mem_Write_Data_O (1, 2, j, 0, call_price, jm_su * 5);
								}

Log (USR_OK, "TEST E[%1.1s][%1.1s][%1.1s]",
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].End_Flag,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].End_Flag);

							}
							else
							{
								Log (USR_OK, "Not HMD, HMS Case j[%d]", j);
							}
						}
					}
				}
			}

			/* 선물 자동정정 로직 */
			if (rc == 999)
			{
				for (j = 0; j < tot_strike; j ++)
				{
					if ((Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run != 0)	&&
					    (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run != 9)	&&
					    (memcmp (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].End_Flag, "1", 1) == 0))
					{
						f_rp_p = f_rp_c = 0;

						/* 선물 매도 정정 */
						if (memcmp (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].TradeFlag, "1", 1) == 0)
						{
							if (Fu_bi_su >
								AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid_qty,
									sizeof(Shm_Futures[0].Futures_CURR.buy_1_prmbid_qty))   )
							{
								/* 호가가격 계산 */
								f_rp_p = AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid,
											sizeof(Shm_Futures[0].Futures_CURR.buy_1_prmbid));
							}
							/* 선물매수1호가 + 0.05 < 주문대기가격 */
							else if (AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid,
										sizeof(Shm_Futures[0].Futures_CURR.buy_1_prmbid)) + 5	<
									 AtoIf (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price, 9))
							{
								/* 호가가격 계산 */
								f_rp_p = AtoIf (Shm_Futures[0].Futures_CURR.buy_1_prmbid,
									sizeof(Shm_Futures[0].Futures_CURR.buy_1_prmbid)) + 5;
							}
							else
								break;

							if ((AtoIf(Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price, 6) * 100 +
								 AtoIf(&Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price[7], 2))
								== f_rp_p)
								break;

							Mem_Write_Data_F (2, 1, j, 2, f_rp_p, 0);
						}
						/* 선물 매수 정정 */
						else
						if (memcmp (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].TradeFlag, "2", 1) == 0)
						{
							if (Fu_bi_su >
								AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid_qty,
									sizeof(Shm_Futures[0].Futures_CURR.sel_1_prmbid_qty))	)
							{
								f_rp_c = AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid,
									sizeof(Shm_Futures[0].Futures_CURR.sel_1_prmbid));
							}
							/* 선물매도1호가 - 0.05 > 주문대기가격 */
							else if (AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid,
										sizeof(Shm_Futures[0].Futures_CURR.sel_1_prmbid)) - 5	>
									 AtoIf (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price, 9))
							{
								f_rp_c = AtoIf (Shm_Futures[0].Futures_CURR.sel_1_prmbid,
											sizeof(Shm_Futures[0].Futures_CURR.sel_1_prmbid)) - 5;
							}
							else
								break;

							if ((AtoIf(Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price, 6) * 100 +
								 AtoIf(&Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Order_Price[7], 2))
								== f_rp_c)
								break;

							Mem_Write_Data_F (2, 2, j, 2, f_rp_c, 0);
						}
					}
				}
			}
			/* 옵션 자동정정로직 */
			else
			{
				for (j = 0; j < tot_strike; j ++)
				{
					if ((Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run != 0)	&&
						(Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run != 9)	)
					{
						for (i = 0; i < 2; i++)
						{
							if ((Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Jong_Seq == rc)	&&
								(memcmp (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].End_Flag, "1", 1) == 0))
							{
								/* 옵션이 매도인 경우 */
								if (memcmp (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].TradeFlag, "1", 1) == 0)
								{
									/* 정정가격 */
									if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) > 300)
										bi_y = 5;
									else
										bi_y = 1;

									/* 내호가가 최우선 호가가 아닌경우 */
									if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) < 
										AtoIf (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price, 6) * 100 +
										AtoIf (&Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price[7], 2))
									{
										/* 호가에 빈칸이 있는지 */
										if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) - 
											AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5)	> bi_y)
										{
											o_rp = AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) - bi_y;
										}
										/* 상대잔량이 충분하면 내최우선호가로 */
										else
										{
											/* 상대잔량이 Set값보다 적으면 내호가 - bi_y */
											if (AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid_qty, 7)	<
												(Op_bi_su *
												 (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Tot_Order_Cnt -
												  Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Nu_Che_Cnt)) / 10 )
											{
												o_rp = AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) - bi_y;
											}
											/* 상대잔량이 충분하면 내최우선호가로 */
											else
											{
												o_rp = AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5);
											}
										}
									}
									/* 내호가가 취우선 1호가인 경우 */
									else
									{
										/* 빈칸이 있으면 Skip */
										if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) -
											AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5)	> bi_y)
											break;

										/* 상대잔량이 Set값보다 적으면 내호가 - bi_y */
										if (AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid_qty, 7)	<
											(Op_bi_su *
											 (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Tot_Order_Cnt -
											  Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Nu_Che_Cnt)) /10  )
										{
											o_rp = AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) - bi_y;
										}
										/* 상대잔량이 충분하면 내최우선 호가로 */
										else
											break;
									}
								}
								/* 옵션이 매수인 경우 */
								else
								{
									/* 정정가격 */
									if (AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5) > 300)
										bi_y = 5;
									else
										bi_y = 1;

									/* 내호가가 최우선 호가가 아닌경우 */
									if (AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5)	>
										AtoIf (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price, 6) * 100 +
										AtoIf (&Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price[7], 2))
									{
										/* 호가에 빈칸이 있는지 */
										if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) - 
											AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5)	> bi_y)
										{
											o_rp = AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5) + bi_y;
										}
										/* 상대잔량이 충분하면 내최우선호가로 */
										else
										{
											/* 상대잔량이 Set값보다 적으면 내호가 - bi_y */
											if (AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid_qty, 7)	<
												(Op_bi_su *
												 (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Tot_Order_Cnt -
												  Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Nu_Che_Cnt)) / 10 )
											{
												o_rp = AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5) + bi_y;
											}
											/* 상대잔량이 충분하면 내최우선호가로 */
											else
											{
												o_rp = AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5);
											}
										}
									}
									/* 내호가가 취우선 1호가인 경우 */
									else
									{
										/* 빈칸이 있으면 Skip */
										if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid, 5) -
											AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5)	> bi_y)
											break;

										/* 상대잔량이 Set값보다 적으면 내호가 - bi_y */
										if (AtoIf (Shm_Options[rc].Options_CURR.sel_1_prmbid_qty, 7)	<
											(Op_bi_su *
											 (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Tot_Order_Cnt -
											  Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Nu_Che_Cnt)) /10  )
										{
											o_rp = AtoIf (Shm_Options[rc].Options_CURR.buy_1_prmbid, 5) + bi_y;
										}
										/* 상대잔량이 충분하면 내최우선 호가로 */
										else
											break;
									}
								}

								/* 정정주문의 가격이 기 존재하는 가격과 동일하면 주문나가지 않음 */
								if ((AtoIf(Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price, 6) * 100 +
									 AtoIf(&Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price[7], 2))
									!= o_rp)
								{
									/* 매매구분과 수량은 0으로 처리한다 */
									Mem_Write_Data_O (2, 0, j, i, o_rp, 0);
								}

								/* for문 종료 처리 */
								j = tot_strike;

								break;
							}
						}
					}
				}
			}

			break;
		}

		Check_Proc_Status ();
	}

}	/* End of PA_5010_MP ()	*/

/*************************************************************************
	Function        : . Poll_Wait
	Parameters IN   : .
	Parameters OUT  : .
	Return Code     : . void
	Comment         : . Sise Poll Check
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Poll_Wait (void)
/*----------------------------------------------------------------------*/
{
	int i, rt;

	while (1)
	{
		rt = poll (Poll, 3, DATA_TIME);
	
		if (rt > 0)
		{
            for (i = 0; i < 3; i ++)
            {
                if (Poll[i].revents & POLLIN)
                {
                    Poll[i].revents = 0;
                    return;
                }

                if (Poll[i].revents & POLLHUP)
                {
                    Log (SYS_ERROR, "poll hangup[%d,%d]", i, 4);
                    continue;
                }
            }
		}
		else if (rt == 0)
       	{
			Log (USR_OK, "poll timeout");
			Check_Proc_Status ();
       	}
       	else
        {
			if (SYS_NO == EINTR)
				Log (SYS_OK, "poll interrupted {%d:%s}", SYS_NO, SYS_STR);
			else
				Log (SYS_ERROR, "poll failure {%d:%s}", SYS_NO, SYS_STR);
		}
	}

	return ;
}	/* End of Poll_Wait ( ) */

/*************************************************************************
    Function        : . Check_Proc_Status
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . check validity of Send_Proc status
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_Proc_Status (void)
/*----------------------------------------------------------------------*/
{
	int	rt;

	if (TCP1_NSTAT(D_K,Pk_mp) != ON)
	{
		Log (USR_ERROR, "check process status[%.10s]", OFN(D_K,P_K,0));

		rt = Write_Data (3,2);
		if (rt != OK)
			Exit_Process ();

		sleep (1);
	}

	return ;
}   /* End of Check_Proc_Status ()  */

/*************************************************************************
    Function        : . Init_Parameters
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . All Program Parameters Init
*************************************************************************/
/*----------------------------------------------------------------------*/
int    Init_Parameters ()
/*----------------------------------------------------------------------*/
{
	int			i, j, rt, fp, strike_price;
	char		item_code[20], memberitem_f[20], memberitem_o[20], tmp[128];
	char		d_time[16], head_size[10], cli_orsnd[21], buf[128], path[128];
	FILE		*r_fp;

	SEARCH_HEADER			*hd;
	CLI_AUTO_START_IN_DATA	*dat;
	KRX_JUMUN_DATA			*KrxOrder;

	Poll[0].fd = START_FD;
	Poll[0].events = POLLIN;
	Poll[1].fd = INPUT_FD;
	Poll[1].events = POLLIN;
	Poll[2].fd = INPUT_FD2;
	Poll[2].events = POLLIN;

	if (WRITE_CNT < READ_CNT)
	{
		Log (USR_ERROR, "Count Error File[%s] W[%d] R[%d]",
			IFN(D_K,P_K,0), WRITE_CNT, READ_CNT);
		Exit_Process ();
	}

	if ( (WRITE_CNT - READ_CNT) > 1)
		READ_CNT = WRITE_CNT - 1;

	/* ******************************************** */
	/* 전일자료 읽어서 처리하기(ATM)				*/
	/* ******************************************** */
	memset (buf, 0, sizeof(buf) );
	memset (path, 0, sizeof(path) );

	sprintf (path, "%s%s1/utl/overfile/overatm.dat",
		(char *)getenv("_FEP_HOME"), (char *)getenv("_FEP_SYSTEM"));

	if ((r_fp = fopen (path, "r")) == NULL)
	{
		printf ("\033[5mcannot open:[%s]\033[0m\n", path);
		exit (1);
	}

	fgets (buf, sizeof (buf), r_fp);

	/* 전일 베이시스 */
	y_bs =			AtoIf (buf+33,  10);
	if (memcmp (buf+33, "-", 1) == 0)
		y_bs = -1 * y_bs;

Log (USR_OK, "File Read [%s][%d]", buf, strlen(buf));
Log (USR_OK, "전일 합성베이시스[%d]", y_bs);

	fclose (r_fp);
	/* ******************************************** */
	/* 전일자료 읽어서 처리하기(ATM)				*/
	/* ******************************************** */

	rt = F_R (PS_R_1, (void *)&R_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "cannot read File[%s,%d:%s] W[%d] R[%d]",
			IFN(D_K,P_K,0), SYS_NO, SYS_STR, WRITE_CNT, READ_CNT);
		return (NOTOK);
	}
	else
	{
		Add_Count (PS_R_1, 1);
		INT_SEQ ++;
		Log (USR_OK, "File Read [%s] Write[%d] Read[%d]",
            &R_Fmt[0].Data, IFW(D_K,P_K,0,0), IFR(D_K,P_K,0,0));
	}

	dat = (CLI_AUTO_START_IN_DATA *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];

	/* 채널 찾기 */
	ApCh = AtoIf (_Exe_Name+4, 2) - ACC_NO_BASE;

	/* ************************************ */
	/* 장개시전 주문 Memory Struct 초기화	*/
	/* ************************************ */

	/* 1. 총 행사가 수량 */
	Shm_Db[0].Ju_S_History[ApCh].strike_cnt = tot_strike = AtoIf(dat->Int_In[0], 10);

	/* 2. 시작 주문행사가 */
	call_seq[0] = call_seq[0] = put_seq[1] = put_seq[1] = 999;

	for (j = 0; j < tot_strike; j ++)
	{
		strike_price = AtoIf(dat->Int_In[1], 10) + (j * 250);

		for (i = 0; i < AtoIf(Shm_Options[0].Options_A0.cnt, 5); i++)
		{
			if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "2", 1) == 0)	&&
				(memcmp (Shm_Options[i].Options_A0.gubun_code, "1", 1) == 0)	&&
				(AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price)	)
			{
				call_seq[j] = i;
				break;
			}

			if (i >= AtoIf(Shm_Options[0].Options_A0.cnt, 5) - 1)
			{
				Log (USR_ERROR, "Call 행사 찾기 실패 j[%d] i[%d]", j, i);
				Exit_Process ();
			}
		}
	}

	for (j = 0; j < tot_strike; j ++)
	{
		strike_price = AtoIf(dat->Int_In[1], 10) + (j * 250);

		for (i = 0; i < AtoIf(Shm_Options[0].Options_A0.cnt, 5); i++)
		{
			if ((memcmp (Shm_Options[i].Options_A0.item_code+3, "3", 1) == 0)	&&
				(memcmp (Shm_Options[i].Options_A0.gubun_code, "1", 1) == 0)	&&
				(AtoIf (Shm_Options[i].Options_A0.striking_price, 11) == strike_price)	)
			{
				put_seq[j] = i;
				break;
			}

			if (i >= AtoIf(Shm_Options[0].Options_A0.cnt, 5) - 1)
			{
				Log (USR_ERROR, "Put 행사 찾기 실패 j[%d] i[%d]", j, i);
				Exit_Process ();
			}
		}
	}

	/* 3, Max 수량 */
	max_su =		AtoIf(dat->Int_In[2], 10);

	/* 4, Max Oticks */
	max_oticks =	AtoIf(dat->Int_In[3], 10);

	/* 5, 주문비율(100) */
	ju_biyul =		AtoIf(dat->Int_In[4], 10);

	/* 6, 버퍼(100) */
	inbuf =			AtoIf(dat->Int_In[5], 10);
	
	/* 7, Trigger(100) */
	trigger =		AtoIf(dat->Int_In[6], 10);
/*
	if (memcmp (dat->Int_In[6], "-", 1) == 0)
		trigger = -1 * trigger;
*/

	/* 8, 옵션정정(100) */
	Op_bi_su =		AtoIf(dat->Int_In[7], 10);

	/* 9, 선물정정(100) */
	Fu_bi_su =		AtoIf(dat->Int_In[8], 10);

	for (j = 0; j < tot_strike; j++)
	{
		/* sub-set 주문상황	*/
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].sub_auto_run = 0;

		/* hms, hmd 초기화 */
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hmd = 0;
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].hms = 0;

		for (i = 0; i < 3; i++)
		{
			Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Jong_Seq = 0;
			Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Tot_Order_Cnt = 0;
			Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Nu_Che_Cnt = 0;

			/* Sub 옵션종목 종료여부 구분(0:주문대상, 1:주문종료)	*/
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].End_Flag, "0", 1);

			/* 종목코드 */
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Item_Cd, "        ", 8);
			/* 주문번호	*/
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].OrderNo, "       ",  7);
			/* 원주문번호 */
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].OriginalOrderNo,
																			   "       ",  7);
			/* 매도매수구분 */
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].TradeFlag, " ",      1);
			/* 주문수량 */
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Cnt, "        ", 8);
			/* 주문가격 */
			memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[i].Order_Price, "         ", 9);
		}

		/* 종목 Seq */
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].Jong_Seq = call_seq[j];
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].Jong_Seq = put_seq[j];
		Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Jong_Seq = 0;

		/* 종목 코드 */
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[0].Item_Cd,
							Shm_Options[call_seq[j]].Options_A0.item_code,
											sizeof(Shm_Options[0].Options_A0.item_code));
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[1].Item_Cd,
							Shm_Options[put_seq[j]].Options_A0.item_code,
											sizeof(Shm_Options[0].Options_A0.item_code));
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[j].ju_set[2].Item_Cd,
									Shm_Futures[0].Futures_A0.item_code,
											sizeof(Shm_Futures[0].Futures_A0.item_code));
	}

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

	memset (Cli_handler, ' ', sizeof (Cli_handler));
	memcpy (Cli_handler, &R_Fmt[0].DataHeader[16], 4);

	/* 체결에서 File Write 할때 사용한다 */
	memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[0].Client_Moniter_Key, &R_Fmt[0].DataHeader[16], 4);
	memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[1].Client_Moniter_Key, &R_Fmt[0].DataHeader[16], 4);

	hd = (SEARCH_HEADER *)R_Fmt[0].Data;
	dat = (CLI_AUTO_START_IN_DATA *)&R_Fmt[0].Data[sizeof(SEARCH_HEADER)];

	/* 전략번호 */
	In_Strategy = 0;
	In_Strategy = AtoIf (hd->ApType_Cd+3, 2) - 1;

	/* Future, Options Order Spec Set */
	memset (F_Order_St, ' ', sizeof(F_Order_St) );
	memset (O_Order_St, ' ', sizeof(O_Order_St) );
	memset (memberitem_f, ' ', sizeof(memberitem_f) );
	memset (memberitem_o, ' ', sizeof(memberitem_o) );
	sprintf (memberitem_f, "DFP%2.2sM%2.2s", hd->ApType_Cd+1, hd->ApType_Cd+3);
	sprintf (memberitem_o, "DOP%2.2sM%2.2s", hd->ApType_Cd+1, hd->ApType_Cd+3);

	KrxOrder = (KRX_JUMUN_DATA *)F_Order_Dat;				/* KRX Order Data(300)  */

	/* Futures Item_Code Set */
	memcpy (KrxOrder->ItemCode, Shm_Futures[0].Futures_A0.item_code,
		sizeof(KrxOrder->ItemCode));

	memcpy (KrxOrder->AccountNo+3,									/* KRX Order AccountNo  */
		ACCNO(D_K, AtoIf(hd->ApType_Cd+1, 2) - ACC_NO_BASE).acc_no, sizeof(KrxOrder->AccountNo) - 3);

	memcpy (KrxOrder->MembershipItem, memberitem_f, 8);	/* KRX Order MembershipItem	*/

	Get_DateTime (d_time);
	memcpy (KrxOrder->Order_Date, d_time,	8);				/* KRX Order Date(8)    */

	/* 취소주문 발생 여부(01 => 취소주문 미처리)	*/
	memcpy (KrxOrder->MembershipItem+8, "01", 2);   /* KRX Order MembershipItem */

	/* System구분 + 몇번째 자동 주문번호 인지 일련번호	*/
	memcpy (KrxOrder->MembershipItem+10, "B", 1);   /* KRX Order MembershipItem */
	memcpy (KrxOrder->MembershipItem+11, _Exe_Name+7, 1);   /* KRX Order MembershipItem */

	/* Order Struct 구성 */
	memset (head_size, ' ', sizeof (head_size));

	memcpy (&R_Fmt[0].ApType, ApType, 8);
	memcpy (&F_Order_St, &R_Fmt,	sizeof (BUFF_RW_HEAD) );
	memcpy (&O_Order_St, &R_Fmt,	sizeof (BUFF_RW_HEAD) );
	sprintf (head_size, "%04d", TCP_DATA_HEAD_LEN + ODS(D_K,P_K,0));
	memcpy (&F_Order_St[50], head_size, 4);
	memcpy (&O_Order_St[50], head_size, 4);

	memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD)],
		Order_Ohd, KRX_HEAD_LEN);							/* KRK Order Header(82) */
	memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN],
		KrxOrder, sizeof(KRX_JUMUN_DATA) );					/* KRX Order Data(300)  */

	memcpy (KrxOrder->MembershipItem, memberitem_o, 8);	/* KRX Order MembershipItem	*/
	memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD)],
		Order_Ohd, KRX_HEAD_LEN);							/* KRK Order Header(82) */
	memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN],
		KrxOrder, sizeof(KRX_JUMUN_DATA) );					/* KRX Order Data(300)  */

	/* Client에 후처리 주문처리를 위해 HATS_H만 대상    */
	memset (cli_orsnd, ' ', sizeof (cli_orsnd));
	sprintf (cli_orsnd, "000050050000005%2.2s%2.2s ",
		&memberitem_f[3], &memberitem_f[6]);

	memset (W_DFmt, ' ', sizeof (W_DFmt));
	memcpy (&W_DFmt, &R_Fmt,  sizeof (BUFF_RW_HEAD) );
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD) - 4], "FFFF",  4 );
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD)],
		cli_orsnd, sizeof (SEARCH_HEADER));					/* SEARCH_HEADER (20)   */
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD) + sizeof (SEARCH_HEADER)],
		Order_Ohd, KRX_HEAD_LEN);							/* KRK Order Header(82) */

	while (1)
	{
		rt = read (Poll[1].fd, tmp, sizeof (tmp));
#if defined __linux
        if (rt == 0 || errno == EAGAIN)
#else
        if (rt == 0)
#endif
			break;
		else if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;

			Log (FIF_ERROR, "Poll:cannot read FIFO {%d:%s}",
		 		SYS_NO, SYS_STR);
			break;
		}
	}

	/* Client 회선 Check Key */
	for (Pk_mp = 0; Pk_mp < DAEMON(D_K).p_count; Pk_mp ++)
	{
		if (memcmp (PROC(D_K,Pk_mp).process_id, OFN(D_K,P_K,0), 10) == 0)
			break;

		if (Pk_mp == DAEMON(D_K).p_count - 1)
		{
			Log (USR_FATAL, "unregistered process[%s]", OFN(D_K,P_K,0));
			TCP1_NET_STA = OFF;
			Exit_Process ();
		}
	}

	return (OK);
}   /* End of Init_Parameters ()    */

/*************************************************************************
    Function        : . Mem_Write_Data_O
    Parameters IN   : . 6개의 입력값을 수신받는다.
						1. 1 : 신규, 2 : 정정 
						2. 1 : 매도, 2 : 매수 0 : (정정시 기존거 수렴)
						3. 0~9 : Sub-set 일련번호 
						4. 0 : 첫번째 종목(콜), 1 : 두번째 종목(풋) 2 : 선물
						5. 가격 예, 27610
						6. 수량, 0 : (정정시 기존거 수렴)
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . check validity of Send_Proc status
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Mem_Write_Data_O (int nm_gbn, int mm_gbn, int ss_cnt, int position, int i_p, int i_s)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	mic_time[24];
	char	cli_orsnd[21], sub_ap_dat[10], t_time[12];

Log (USR_OK, "O nm_gbn[%d] mm_gbn[%d] ss_cnt[%d] position[%d] i_p[%d] i_s[%d]",
	nm_gbn, mm_gbn, ss_cnt, position, i_p, i_s);

	/* 회원사처리항목에 신규주문의 포지션위치를 넣어준다 */
	ItoAf (ss_cnt, &O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 199], 2);

	/* 정정취소 구분코드 (1:신규, 2:정정) */
	ItoAf (nm_gbn, &O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 66], 1);

	/* 주문가격 */
	ItoAf ( i_p / 100, &O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89], 8);
	ItoAf ( i_p % 100, &O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89 + 9], 2);

	if (nm_gbn == 1)
	{
		/* ********************************** */
		/* 옵션 신규주문 Packet 만들어서 송신 */
		/* TRcoed, 정정취소 구분코드, 원주문번호, 회원사항목 초기화 */
		/* ********************************** */
		/* 신규주문 재처리 */
		/* TR code 변경 TCHODR10001 */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 11],
									"TCHODR10001", 11);
	
		/* 원주문번호 (주문번호 -> 원주문번호) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 43], "      ", 6);

		/* 신규주문시 특정항목 초기화 처리(44) */
		/* 자사주신고서 ID, 자사주매매방법코드, 매도유형코드, 신용구분코드 	*/
		/* 위탁사번호(정정주문시 Space)	*/
		/* 대용주권계좌번호부터 (12+2+2+3+4+2=25) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 113],
			"0    0001031     00            4111410100000" , 44);
		/* 신규주문 재처리 */

		/* 종목코드 */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 53],
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_A0.item_code, 12);
		
		/* 매매구분 */
		ItoAf (mm_gbn, &O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], 1);

		/* 호가수량 Setting */
		ItoAf ( i_s, &O_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10);

		/* 회원사처리항목 자동취소 옵션취소주문 발생 여부(01 => 취소주문 미처리) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 195], "01", 2);
	}
	/* 옵션 정정 */
	else
	{
		/* ****************** */
		/* 옵션 정정주문 Send */
		/* ****************** */
		/* Options Order Spec Set(정정주문 발주)	*/
	
		/* TR code 변경 TCHODR10001 -> TCHODR10003  */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 11],
									"TCHODR10002", 11);
	
		/* 종목코드	*/
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 53], 
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_A0.item_code, 12);

		/* 매매구분 */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], 
			Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].TradeFlag, 1);

		/* 호가수량 Setting */
		ItoAf ( Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Tot_Order_Cnt -
				Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Nu_Che_Cnt,
			&O_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10);

		/* 정정주문시 특정항목 Space 처리	*/
		/* 자사주신고서 ID, 자사주매매방법코드, 매도유형코드, 신용구분코드 	*/
		/* 5 + 1 + 2 + 2 = 10 (정정, 취소시에는 Space로 처리)	*/
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 113], "          ", 10);
		/* 위탁사번호(정정주문시 Space)	*/
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 125], "     ", 5);
		/* 대용주권계좌번호부터 (12+2+2+3+4+2=25) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 132],
										"                         ", 25);

		/* 원주문번호 (주문번호 -> 원주문번호) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 43],
			Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].OrderNo, 6);

		/* 회원사처리항목 자동취소 옵션취소주문 발생 여부(01 => 취소주문 미처리) */
		memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 195], "01", 2);
	}

	/* 회원사 주문시각  */
	memset (t_time, 0, sizeof (t_time));
	Get_Time (t_time);
	memcpy (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 178], t_time, 9);

	/* 옵션수량 1,000계약(공용) */
	if (AtoIf (&O_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10) > MAX_OP_SU)
	{
		rt = Write_Data (1,2);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 04 (1,2)");
			Exit_Process ();
		}

		rt = Write_Data (14,1);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 04 (14,1)");
			Exit_Process ();
		}

		return;
	}

	/* 옵션금액 3억(공용) */
	if ((AtoIf (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89], 8)
		 * AtoIf (&O_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10))
		> MAX_OP_GUM)
	{
		rt = Write_Data (1,2);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 05 (1,2)");
			Exit_Process ();
		}

		rt = Write_Data (16,1);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 05 (16,1)");
			Exit_Process ();
		}

		return;
	}

	/* ApType별 주문번호 채번(10만번대만 사용, 6자리만 쓴다) */
	ACCNO(D_K,ApCh).js_order_no_band_b++;
	ItoAf (ACCNO(D_K, ApCh).js_order_no_band_b,
		&O_Order_St[sizeof(BUFF_RW_HEAD)+sizeof(SEARCH_HEADER)+KRX_HEAD_LEN+13], 6);

	memset (mic_time, 0, sizeof (mic_time));
	Get_MicroTime (mic_time);
	memcpy (&O_Order_St[sizeof(BUFF_RW_HEAD) - 12 - 20], &mic_time[10], 12);

	O_Order_St[sizeof(BUFF_RW_HEAD)+ODS(D_K,P_K,0)] = '\n';

	/* Shw Write */
	rt = DSHM_W (TS_W1_1, (void *)O_Order_St, READ_MAX);

	Log (USR_OK, "DSHM write OK");

	if (rt < 0)
	{
		Log (DSH_FATAL, "DSHM write[%s]", ODN(D_K,P_K,0));

		rt = Write_Data (1,2);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 6 (1,2)");
			Exit_Process ();
		}

		sleep (1);
	}

	/* Client 후처리 로직 추가	*/
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + sizeof (SEARCH_HEADER)],
		&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN],
		sizeof(KRX_JUMUN_DATA));
	W_DFmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

/* ************************************************************************************* */
/* 자동록직 종료                                                                         */
/* ************************************************************************************* */
#if (1)
/* Test Log */
	if (memcmp (&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], "1", 1) == 0)
	{
		Log (USR_OK, "매매구분[%.1s] 주문수량[%.10s] 주문가격[%.11s] 시장가격,수량[%.5s][%.7s] 주문번호[%6.6s]",
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65],
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79],
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89],
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.buy_1_prmbid,
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.buy_1_prmbid_qty,
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 33]);

		if (AtoIf(Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.buy_1_prmbid+3, 2) >=
			AtoIf(&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 98], 2)	)
			Log (USR_OK, "유효주문수량[%7.7s] 주문수량[%10.10s]", 
				Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.buy_1_prmbid_qty,
				&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79]);
		else
			Log (USR_OK, "유효주문수량[00000]"); 
	}
	else
	{
		Log (USR_OK, "매매구분[%.1s] 주문수량[%.10s] 주문가격[%.11s] 시장가격,수량[%.5s][%.6s] 주문번호[%6.6s]",
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65],
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79],
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89],
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.sel_1_prmbid,
			Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.sel_1_prmbid_qty,
			&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 33]);

		if (AtoIf(Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.sel_1_prmbid+3, 2) <=
			AtoIf(&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 98], 2)	)
			Log (USR_OK, "유효주문수량[%.7s] 주문수량[%.10s]", 
				Shm_Options[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].Jong_Seq].Options_CURR.sel_1_prmbid_qty,
				&O_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79]);
		else
			Log (USR_OK, "유효주문수량[00000]"); 
	}
#endif

	/* ************************************************ */
	/* sub_set의 최초신규주문시에는 다음의 처리를 한다	*/
	/* 1. 신규주문 송신횟수 증가. 						*/
	/* 2. 해당 sub_set의 매매 결정.						*/
	/* 3. 주문번호 Set									*/
	/* ************************************************ */

	if (nm_gbn == 1)
	{
		/* 주문번호 */
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].OrderNo,
			&O_Order_St[sizeof(BUFF_RW_HEAD)+sizeof(SEARCH_HEADER)+KRX_HEAD_LEN+13], 6);
	}
	else
	{
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].End_Flag, "5", 1);
	}

Log (USR_OK, "TEST ApCh[%d] sub_set[%d] position[%d] 주문번호[%6.6s][%6.6s]",
	ApCh, ss_cnt, position,
	Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[position].OrderNo,
	&O_Order_St[sizeof(BUFF_RW_HEAD)+sizeof(SEARCH_HEADER)+KRX_HEAD_LEN+13]);

	return ;
}   /* End of Mem_Write_Data_O ()  */

/*************************************************************************
    Function        : . Mem_Write_Data_F
    Parameters IN   : . 6개의 입력값을 수신받는다.
						1. 0 : 신규, 1 : 정정 
						2. 1 : 매도, 2 : 매수 0 : (정정시 기존거 수렴)
						3. 0~9 : Sub-set 일련번호 
						4. 0 : 첫번째 종목(콜), 1 : 두번째 종목(풋) 2 : 선물
						5. 가격 예, 27610
						6. 수량, 0 : (정정시 기존거 수렴)
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . check validity of Send_Proc status
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Mem_Write_Data_F (int nm_gbn, int mm_gbn, int ss_cnt, int position, int i_p, int i_s)
/*----------------------------------------------------------------------*/
{
	int		rt;
	char	mic_time[24], t_time[12];

Log (USR_OK, "F nm_gbn[%d] mm_gbn[%d] ss_cnt[%d] position[%d] i_p[%d] i_s[%d]",
	nm_gbn, mm_gbn, ss_cnt, position, i_p, i_s);

	/* ************** */
	/* 선물 주문 Send */
	/* ************** */
	/* Future Order Spec Set(정정주문 발주)	*/

	/* 회원사처리항목에 신규주문의 포지션위치를 넣어준다 */
	ItoAf (ss_cnt, &F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 199], 2);

	/* 종목코드 */
	memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 53],
		Shm_Futures[0].Futures_A0.item_code, 12);

	/* 정정취소 구분코드 (1:신규, 2:정정) */
	ItoAf (nm_gbn, &F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 66], 1);

	/* 호가가격 */
	ItoAf ( i_p  / 100, &F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89], 8);
	ItoAf ( i_p  % 100, &F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89 + 9], 2);

	if (nm_gbn == 1)
	{
		/* ************************************ */
		/* 선물 신규주문 Packet 만들어서 송신	*/		
		/* TRcoed, 정정취소 구분코드, 원주문번호, 회원사항목 초기화 */
		/* ************************************ */
		/* 신규주문 재처리 */
		/* TR code 변경 TCHODR10001 */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 11],
									"TCHODR10001", 11);

		/* 원주문번호 (주문번호 -> 원주문번호) */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 43], "      ", 6);

		/* 신규주문시 특정항목 초기화 처리(44) */
		/* 자사주신고서 ID, 자사주매매방법코드, 매도유형코드, 신용구분코드  */
		/* 위탁사번호(정정주문시 Space) */
		/* 대용주권계좌번호부터 (12+2+2+3+4+2=25) */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 113],
			"0    0001031     00            4111410100000" , 44);
		/* 신규주문 재처리 */

		/* 매매구분 */
		ItoAf (mm_gbn, &F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], 1);

		/* 호가수량 Setting */
		ItoAf ( i_s, &F_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10);

		/* 회원사처리항목 자동취소 옵션취소주문 발생 여부(01 => 취소주문 미처리) */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 195], "01", 2);
	}
	else
	{
		/* TR code 변경 TCHODR10001 -> TCHODR10003  */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 11], "TCHODR10002", 11);
	
		/* 매매구분 */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], 
			Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].TradeFlag, 1);

		/* 선물은 종목이 한종목인지라 종목코드 수정을 하지 않는다 */
		/* 매매구분도 수정하지 않는다 */
		/* 호가가격은 위에서 처리했음 */
		
		/* 호가수량 Setting */
		ItoAf ( Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Tot_Order_Cnt -
				Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Nu_Che_Cnt,
			&F_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10);
	
		/* 정정주문시 특정항목 Space 처리	*/
		/* 자사주신고서 ID, 자사주매매방법코드, 매도유형코드, 신용구분코드 	*/
		/* 5 + 1 + 2 + 2 = 10 (정정, 취소시에는 Space로 처리)	*/
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 113], "          ", 10);
		/* 위탁사번호(정정주문시 Space)	*/
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 125], "     ", 5);
		/* 대용주권계좌번호부터 (12+2+2+3+4+2=25) */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 132],
								"                         ", 25);
		
		/* 회원사 주문시각  */
		memset (t_time, 0, sizeof (t_time));
		Get_Time (t_time);
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 178], t_time, 9);

		/* 원주문번호 (주문번호 -> 원주문번호) */
		memcpy (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 43],
			Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].OrderNo, 6);
	}

	/* 선물수량 50계약(공용) */
	if (AtoIf (&F_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10) > MAX_FU_SU)
	{
		rt = Write_Data (1,2);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 07 (1,2)");
			Exit_Process ();
		}

		rt = Write_Data (13,1);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 07 (13,1)");
			Exit_Process ();
		}

		return;
	}

	/* 선물금액 75억(공용, 소수점 빼고 계산한다, 7천5백으로 비교) */
	if ((AtoIf (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89], 8)
		 * AtoIf (&F_Order_St[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN + 79], 10)	)
		> MAX_FU_GUM)
	{
		rt = Write_Data (1,2);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 08 (1,2)");
			Exit_Process ();
		}

		rt = Write_Data (15,1);
		if (rt != OK)
		{
			Log (USR_ERROR, "Write_Data Error 08 (15,1)");
			Exit_Process ();
		}

		return;
	}

	/* ApType별 주문번호 채번(10만번대만 사용, 6자리만 쓴다) */
	ACCNO(D_K,ApCh).js_order_no_band_b++;
	ItoAf (ACCNO(D_K,ApCh).js_order_no_band_b,
		&F_Order_St[sizeof(BUFF_RW_HEAD)+sizeof(SEARCH_HEADER)+KRX_HEAD_LEN+13], 6);

	memset (mic_time, 0, sizeof (mic_time));
	Get_MicroTime (mic_time);
	memcpy (&F_Order_St[sizeof(BUFF_RW_HEAD) - 12 - 20], &mic_time[10], 12);

	F_Order_St[sizeof(BUFF_RW_HEAD)+ODS(D_K,P_K,0)] = '\n';

	/* Shw Write */
	rt = DSHM_W (TS_W1_1, (void *)F_Order_St, READ_MAX);

	Log (USR_OK, "DSHM write OK");

	if (rt < 0)
	{
		Log (DSH_FATAL, "DSHM write[%s]", ODN(D_K,P_K,0));

		rt = Write_Data (1,2);
		if (rt != OK)
			Exit_Process ();

		rt = Write_Data (1,1);
		if (rt != OK)
			Exit_Process ();

		sleep (1);
	}

	/* Client 후처리 로직 추가	*/
	memcpy (&W_DFmt[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + sizeof (SEARCH_HEADER)],
		&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN],
		sizeof(KRX_JUMUN_DATA));
	W_DFmt[sizeof(BUFF_RW_HEAD)+OFS(D_K,P_K,0)] = '\n';

	rt = F_W (TS_W1_1, (void *)&W_DFmt, 1);
	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

	/* 자동정정 대상으로 설정    */
	if (Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].sub_auto_run == 0)
		Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].sub_auto_run = 1;

	if (nm_gbn == 1)
	{
		/* 주문번호 */
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].OrderNo,
			&F_Order_St[sizeof(BUFF_RW_HEAD)+sizeof(SEARCH_HEADER)+KRX_HEAD_LEN+13], 6);
	}
	else
	{
		memcpy (Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].End_Flag, "5", 1);
	}

/* ************************************************************************************* */
/* 자동록직 종료                                                                         */
/* ************************************************************************************* */
#if (1)
/* Test Log */
	if (memcmp (&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65], "1", 1) == 0)
	{
		Log (USR_OK, "매매구분[%.1s] 주문수량[%.10s] 주문가격[%.11s] 시장가격,수량[%.5s][%.6s] 주문번호[%6.6s]",
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65],
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79],
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89],
			Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.buy_1_prmbid,
			Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.buy_1_prmbid_qty,
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 33]);

		if (AtoIf(Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.buy_1_prmbid+3, 2) >=
			AtoIf(&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 98], 2)	)
			Log (USR_OK, "유효주문수량[%6.6s] 주문수량[%10.10s]", 
				Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.buy_1_prmbid_qty,
				&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79]);
		else
			Log (USR_OK, "유효주문수량[00000]"); 
	}
	else
	{
		Log (USR_OK, "매매구분[%.1s] 주문수량[%.10s] 주문가격[%.11s] 시장가격,수량[%.5s][%.6s] 주문번호[%6.6s]",
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 65],
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79],
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 89],
			Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.sel_1_prmbid,
			Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.sel_1_prmbid_qty,
			&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 33]);

		if (AtoIf(Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.sel_1_prmbid+3, 2) <=
			AtoIf(&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 98], 2)	)
			Log (USR_OK, "유효주문수량[%.6s] 주문수량[%.10s]", 
				Shm_Futures[Shm_Db[0].Ju_S_History[ApCh].sub_set[ss_cnt].ju_set[2].Jong_Seq].Futures_CURR.sel_1_prmbid_qty,
				&F_Order_St[sizeof (BUFF_RW_HEAD) + KRX_HEAD_LEN + 79]);
		else
			Log (USR_OK, "유효주문수량[00000]"); 
	}
#endif

	return ;
}   /* End of Mem_Write_Data_F ()  */

/**************************************************************************
    Function        : . Write_Data
    Parameters IN   : . egbn : Error_Flag
						 1 : 500330 (Auto Kill)
						 2 : 500110 (주문기동요청 응답)
						 3 : 500310 (Auto All Kill)
						10 : 입력값 오류
						13 : 선물계약수량초과
						14 : 옵션계약수량초과
						15 : 선물 주문금액 한도초과
						16 : 옵션 주문금액 한도초과
						19 : 주문횟수 만족(정상종료, 500210)
				      .	p_flag  : write flag
						1 : Client 후처리
						2 : pa_9001_mp(관리)
    Parameters OUT  : .
    Return Code     : . int
    Comment         : . write data to file
**************************************************************************/
/*-----------------------------------------------------------------------*/
int    Write_Data (int egbn, int p_flag)
/*-----------------------------------------------------------------------*/
{
	int     rt;
	char    m_time[24], str[10];
	char	cdata[30];

	memset (m_time, 0, sizeof (m_time));
	memset (cdata, ' ', sizeof (cdata));
	memset (str, ' ', sizeof (str));
	memset (&W_Fmt, ' ', sizeof (FILE_BUFF_FORMAT));
	Get_MicroTime (m_time);

	ItoAf (OFW_CNT(p_flag-1,0) + 1, W_Fmt.If_Seq, sizeof (W_Fmt.If_Seq));
	memcpy (W_Fmt.ApType, ApType, sizeof (W_Fmt.ApType));
	memcpy (W_Fmt.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (W_Fmt.RecvTime1, m_time, sizeof (W_Fmt.RecvTime1));
	memcpy (W_Fmt.RecvTime2, &m_time[sizeof(W_Fmt.RecvTime1)],
		sizeof (W_Fmt.RecvTime2));

	ItoAf (HEAD_SIZE + DATA_SIZE, Tcp_Data_Head.Length,
		sizeof (Tcp_Data_Head.Length));
	ItoAf (OFW_CNT(p_flag-1,0) + 1, Tcp_Data_Head.DataSeq,
		sizeof (Tcp_Data_Head.DataSeq));
	memcpy (Tcp_Data_Head.ResponseCode, RES_NORMAL, strlen (RES_NORMAL));
	memcpy (Tcp_Data_Head.LineFlag, Cli_handler, 4);

	memcpy (W_Fmt.DataHeader, &Tcp_Data_Head, sizeof (TCP_DATA_HEAD));

	/* 후처리 client */
	if (p_flag == 1)
	{
		/* 주문기동 응답 */
		if (egbn == 2)
		{
			sprintf (cdata, "00005001100000%5.5s %02d", _Exe_Name+3, In_Strategy+1);
			memcpy (W_Fmt.Data, cdata, sizeof (SEARCH_HEADER) + 2);

			/* Auto TOT Cnt - */
			if (AtoIf(ACCNO(D_K, ApCh).auto_use_cnt, 2) <= 0)
			{
				memcpy (ACCNO(D_K, ApCh).auto_use_cnt, "01", 2);
			}
			else
			{
				sprintf (str, "%02d",
					AtoIf(ACCNO(D_K, ApCh).auto_use_cnt, 2) + 1);
				memcpy (ACCNO(D_K, ApCh).auto_use_cnt, str, 2);
			}

			/* Auto Strategy Cnt - */
			if (AtoIf(&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2], 1) <= 0)
			{
				memcpy (&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2],
				"1", 1);
			}
			else
			{
				sprintf (str, "%d",
					AtoIf(&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2], 1) + 1);
				memcpy (&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2], str, 1);
			}
		}
		else
		{
			sprintf (cdata, "00005002200000%5.5s %02d", _Exe_Name+3, In_Strategy+1);
			memcpy (W_Fmt.Data, cdata, sizeof (SEARCH_HEADER) + 2);

			if (egbn == 10)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"입력값 오류로 인한 미기동 ", 26);
			else if (egbn == 13)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"선물계약 수량초과에 의한 종료", 29);
			else if (egbn == 14)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"옵션계약 수량초과에 의한 종료", 29);
			else if (egbn == 15)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"선물 주문금액 한도초과에 의한 종료", 34);
			else if (egbn == 16)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"옵션 주문금액 한도초과에 의한 종료", 34);
			else if (egbn == 19)
				memcpy (&W_Fmt.Data[sizeof (SEARCH_HEADER) + 10],
					"주문종료 조건 만족에 의한 종료처리", 34);
		}
			
	}
	/* pa_9001_mp(관리) */
	else
	{
		/* Auto Kill */
		if (egbn == 1)
		{
			sprintf (cdata, "00005003300000%5.5s %02d", _Exe_Name+3, In_Strategy+1);
			memcpy (W_Fmt.Data, cdata, sizeof (SEARCH_HEADER) + 2);
		}
		/* Auto All Kill */
		else if (egbn == 3)
		{
			sprintf (cdata, "00005003100000%5s %02d", _Exe_Name+3, In_Strategy+1);
			memcpy (W_Fmt.Data, cdata, sizeof (SEARCH_HEADER) + 2);

			/* Auto TOT Cnt - */
			if (AtoIf(ACCNO(D_K,ApCh).auto_use_cnt, 2) <= 0)
			{
				memcpy (ACCNO(D_K,ApCh).auto_use_cnt, "00", 2);
			}
			else
			{
				sprintf (str, "%02d",
					AtoIf(ACCNO(D_K,ApCh).auto_use_cnt, 2) - 1);
				memcpy (ACCNO(D_K,ApCh).auto_use_cnt, str, 2);
			}
	
			/* Auto Strategy Cnt - */
			if (AtoIf(&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2], 1)
			<= 0)
			{
				memcpy (&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2],
					"0", 1);
			}
			else
			{
				sprintf (str, "%d",
		AtoIf(&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2], 1) - 1);
				memcpy (&ACCNO(D_K,ApCh).auto_use_cnt[In_Strategy+2],
					str, 2);
			}
		}
	}

	W_Fmt.LineFeed[0] = '\n';

	rt = F_W (p_flag * 10, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
		return (NOTOK);
	}

	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

	return (OK);
}   /* End of Write_Data () */

/*************************************************************************
	End of Program (pa_5010_mp.c)
*************************************************************************/

