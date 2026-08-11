/*------------------------------------------------------------------------
#   Module  : 시세 - 체결 (H0)
#   File	: py_2020_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			Sk;
char		FO_Flag, ItemCd[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Sise (void);
static void		Disp_Sise (void);

/*----------------------------------------------------------------------*/
void	py_2020_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sise_SHM ();
	Info_Sise ();

	Disp_Sise ();
	Disp_Msg ("Enter:Retry I:Info Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg ("Enter:Retry I:Info Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

		switch (getch ())
		{
			case	'i':
			case	'I':										/* Info	*/
				Info_Sise ();
				break;
			case 	KEY_ESC:
			case 	KEY_LEFT:
			case 	KEY_RIGHT:
			case	'1':
				job_end = 0;
				refresh ();
				endwin ();
				break;
			case	'q':
			case	'Q':										/* Quit	*/
				Exit_Process ();
			case	'\n':									/* Retry	*/
				Disp_Sise ();
				Disp_Msg ("Enter:Retry I:Info Q:Quit");
				break;
			default:
				clear ();
				Disp_Scr ();
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg ("Enter:Retry I:Info Q:Quit");
				break;
		}
	}

	refresh ();
	endwin ();

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Scr (void)
/*----------------------------------------------------------------------*/
{
	Disp_Title ();
	attron (A_BOLD);
	mvaddstr (0, 18, "[2020] 시세 - 체결 (C0)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "상품   (1:선물 2:옵션)    종목코드");
	Draw_Underline (2, 5, 1);
	Draw_Underline (2, 35, 8);

	mvaddstr (5, 0,
		"체결시각 [  :  :  -  ]");
	mvaddstr (6, 0,
		"응답시간 [          ]  체결시간 [          ]");
	mvaddstr (7, 0,
		"종목코드                  한글명");
	mvaddstr (8, 0, "상한가     .      하한가     .      체결량 ");
	mvaddstr (9, 0, "시가      .       고가    .         저가    .");
	mvaddstr (10, 0,
		"누적체결수량              누적거래대금(천원)");

	mvaddstr (12, 0, "현재가    .");
	mvaddstr (13, 0, "매도1우선호가    .        매수1우선호가    .");
	mvaddstr (14, 0, "매도2우선호가    .        매수2우선호가    .");
	mvaddstr (15, 0, "매도3우선호가    .        매수3우선호가    .");
	mvaddstr (16, 0, "매도4우선호가    .        매수4우선호가    .");
	mvaddstr (17, 0, "매도5우선호가    .        매수5우선호가    .");

	mvaddstr (19, 0, "매도총호가수량            매수총호가수량");
	mvaddstr (20, 0, "매도1우선호가수량         매수1우선호가수량");
	mvaddstr (21, 0, "매도2우선호가수량         매수2우선호가수량");
	mvaddstr (22, 0, "매도3우선호가수량         매수3우선호가수량");
	mvaddstr (23, 0, "매도4우선호가수량         매수4우선호가수량");
	mvaddstr (24, 0, "매도5우선호가수량         매수5우선호가수량");

	mvaddstr (26, 0, "매도총호가건수            매수총호가건수");
	mvaddstr (27, 0, "매도1우선호가건수         매수1우선호가건수");
	mvaddstr (28, 0, "매도2우선호가건수         매수2우선호가건수");
	mvaddstr (29, 0, "매도3우선호가건수         매수3우선호가건수");
	mvaddstr (30, 0, "매도4우선호가건수         매수4우선호가건수");
	mvaddstr (31, 0, "매도5우선호가건수         매수5우선호가건수");

	return;
}

/*----------------------------------------------------------------------*/
static void		Info_Sise (void)
/*----------------------------------------------------------------------*/
{
	int 	i, rt;
	char	buf[80];

	Clear_Lines (2, Rows - 2);
	Disp_Scr ();

	Disp_Msg ("<상품구분> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 5, 1);
		rt = Get_String (2, 5, 1, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("<상품구분> 입력 (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			attron (A_UNDERLINE);
			mvaddstr (2, 5, buf);
			attroff (A_UNDERLINE);

			if (strlen (buf) != 1 || (buf[0] != '1' && buf[0] != '2'))
			{
				Disp_Msg ("상품구분 오류 (Esc:Cancel)");
				continue;
			}

			FO_Flag = buf[0];
			break;
		}
	}

	Disp_Msg ("<종목코드> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 35, 8);
		rt = Get_String (2, 35, 8, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("<종목코드> 입력 (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			attron (A_UNDERLINE);
			mvaddstr (2, 35, buf);
			attroff (A_UNDERLINE);

			if (FO_Flag == '1')									/* 선물	*/
			{
				for (Sk = 0; Sk < SHM_MAX_FUTURES; Sk ++)
				{
					if (memcmp (buf, Shm_Futures[Sk].Futures_FB.item_cd,
						sizeof (Shm_Futures[Sk].Futures_FB.item_cd)) == 0)
						break;
				}

				if (Sk == SHM_MAX_FUTURES)
				{
					Disp_Msg ("선물 종목코드 미존재 (Esc:Cancel)");
					continue;
				}
			}													/* 옵션	*/
			else
			{
				for (Sk = 0; Sk < SHM_MAX_OPTIONS; Sk ++)
				{
					if (memcmp (buf, Shm_Options[Sk].Options_OB.item_cd+3,
						sizeof (Shm_Options[Sk].Options_OB.item_cd) - 4) == 0)
						break;
				}

				if (Sk == SHM_MAX_OPTIONS)
				{
					Disp_Msg ("옵션 종목코드 미존재 (Esc:Cancel)");
					continue;
				}
			}

			memcpy (ItemCd, buf, 8);
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	FMS_C0	fms_c0;
	OPT_C0	opt_c0;

	G_Time ();

	attron (A_REVERSE);

	if (FO_Flag == '1')											/* 선물	*/
	{
		mvaddstr (3, 0, "지수선물 체결가 (Futures_C0)");
		memcpy (fms_c0.item_cd, Shm_Futures[Sk].Futures_C0.item_cd,
			sizeof (FMS_C0) - 4);
	}
	else														/* 옵션	*/
	{
		mvaddstr (3, 0, "지수옵션 체결가 (Options_C0)");
		memcpy (opt_c0.item_cd, Shm_Options[Sk].Options_C0.item_cd,
			sizeof (OPT_C0) - 4);
	}

	attroff (A_REVERSE);

	attron (A_BOLD);

	if (FO_Flag == '1')											/* 선물	*/
	{
		MvAddStr (5, 10, 2, fms_c0.chekyul_tm);
		MvAddStr (5, 13, 2, fms_c0.chekyul_tm+2);
		MvAddStr (5, 16, 2, fms_c0.chekyul_tm+4);
		MvAddStr (5, 19, 2, fms_c0.chekyul_tm+6);
		MvAddStr (6, 10, 10, Shm_Futures[0].ordertime);
		MvAddStr (6, 33, 10, Shm_Futures[0].chetime);
		MvAddStr (7, 9, 8, fms_c0.item_cd);
		MvAddStr (7, 33, 30, Shm_Futures[Sk].Futures_FB.kor_nm);
		MvAddStr (8, 7, 4, Shm_Futures[Sk].Futures_FB.hlprc);
		MvAddStr (8, 12, 2, Shm_Futures[Sk].Futures_FB.hlprc+4);
		MvAddStr (8, 25, 4, Shm_Futures[Sk].Futures_FB.llprc);
		MvAddStr (8, 30, 2, Shm_Futures[Sk].Futures_FB.llprc+4);
		MvAddStr (8, 43, 6, fms_c0.chekyul_qty);
		MvAddStr (9, 7, 3, fms_c0.oprc);
		MvAddStr (9, 11, 2, fms_c0.oprc+3);
		MvAddStr (9, 23, 3, fms_c0.hprc);
		MvAddStr (9, 27, 2, fms_c0.hprc+3);
		MvAddStr (9, 41, 3, fms_c0.lprc);
		MvAddStr (9, 45, 2, fms_c0.lprc+3);
		MvAddStr (10, 13, 7, fms_c0.tot_con_qty);
		MvAddStr (10, 45, 11, fms_c0.tot_con_amt);

		/* 현재가 */
		MvAddStr (12, 7, 3, fms_c0.crprc);
		MvAddStr (12, 11, 2, fms_c0.crprc+3);
		/* 우선호가 */
		MvAddStr (13, 14, 3, fms_c0.sel_1_prmbid);
		MvAddStr (13, 18, 2, fms_c0.sel_1_prmbid+3);
		MvAddStr (13, 40, 3, fms_c0.buy_1_prmbid);
		MvAddStr (13, 44, 2, fms_c0.buy_1_prmbid+3);
		MvAddStr (14, 14, 3, fms_c0.sel_2_prmbid);
		MvAddStr (14, 18, 2, fms_c0.sel_2_prmbid+3);
		MvAddStr (14, 40, 3, fms_c0.buy_2_prmbid);
		MvAddStr (14, 44, 2, fms_c0.buy_2_prmbid+3);
		MvAddStr (15, 14, 3, fms_c0.sel_3_prmbid);
		MvAddStr (15, 18, 2, fms_c0.sel_3_prmbid+3);
		MvAddStr (15, 40, 3, fms_c0.buy_3_prmbid);
		MvAddStr (15, 44, 2, fms_c0.buy_3_prmbid+3);
		MvAddStr (16, 14, 3, fms_c0.sel_4_prmbid);
		MvAddStr (16, 18, 2, fms_c0.sel_4_prmbid+3);
		MvAddStr (16, 40, 3, fms_c0.buy_4_prmbid);
		MvAddStr (16, 44, 2, fms_c0.buy_4_prmbid+3);
		MvAddStr (17, 14, 3, fms_c0.sel_5_prmbid);
		MvAddStr (17, 18, 2, fms_c0.sel_5_prmbid+3);
		MvAddStr (17, 40, 3, fms_c0.buy_5_prmbid);
		MvAddStr (17, 44, 2, fms_c0.buy_5_prmbid+3);
		/* 총 호가수량 */
		MvAddStr (19, 15, 6, fms_c0.sel_tot_best_qty);
		MvAddStr (19, 41, 6, fms_c0.buy_tot_best_qty);
		/* 호가수량 */
		MvAddStr (20, 18, 6, fms_c0.sel_1_prmbid_qty);
		MvAddStr (20, 44, 6, fms_c0.buy_1_prmbid_qty);
		MvAddStr (21, 18, 6, fms_c0.sel_2_prmbid_qty);
		MvAddStr (21, 44, 6, fms_c0.buy_2_prmbid_qty);
		MvAddStr (22, 18, 6, fms_c0.sel_3_prmbid_qty);
		MvAddStr (22, 44, 6, fms_c0.buy_3_prmbid_qty);
		MvAddStr (23, 18, 6, fms_c0.sel_4_prmbid_qty);
		MvAddStr (23, 44, 6, fms_c0.buy_4_prmbid_qty);
		MvAddStr (24, 18, 6, fms_c0.sel_5_prmbid_qty);
		MvAddStr (24, 44, 6, fms_c0.buy_5_prmbid_qty);
		/* 총 호가건수 */
		MvAddStr (26, 15, 5, fms_c0.sel_tot_best_cnt);
		MvAddStr (26, 41, 5, fms_c0.buy_tot_best_cnt);
		/* 호가건수 */
		MvAddStr (27, 18, 4, fms_c0.sel_1_best_cnt);
		MvAddStr (27, 44, 4, fms_c0.buy_1_best_cnt);
		MvAddStr (28, 18, 4, fms_c0.sel_2_best_cnt);
		MvAddStr (28, 44, 4, fms_c0.buy_2_best_cnt);
		MvAddStr (29, 18, 4, fms_c0.sel_3_best_cnt);
		MvAddStr (29, 44, 4, fms_c0.buy_3_best_cnt);
		MvAddStr (30, 18, 4, fms_c0.sel_4_best_cnt);
		MvAddStr (30, 44, 4, fms_c0.buy_4_best_cnt);
		MvAddStr (31, 18, 4, fms_c0.sel_5_best_cnt);
		MvAddStr (31, 44, 4, fms_c0.buy_5_best_cnt);
	}
	else														/* 옵션	*/
	{
		MvAddStr (5, 10, 2, opt_c0.chekyul_tm);
		MvAddStr (5, 13, 2, opt_c0.chekyul_tm+2);
		MvAddStr (5, 16, 2, opt_c0.chekyul_tm+4);
		MvAddStr (5, 19, 2, opt_c0.chekyul_tm+6);
		MvAddStr (6, 10, 10, Shm_Options[0].ordertime);
		MvAddStr (6, 33, 10, Shm_Options[0].chetime);
        MvAddStr (7, 9, 8, opt_c0.item_cd);
        MvAddStr (7, 33, 30, Shm_Options[Sk].Options_OB.kor_nm);
        MvAddStr (8, 7, 4, Shm_Options[Sk].Options_OB.hlprc);
        MvAddStr (8, 12, 2, Shm_Options[Sk].Options_OB.hlprc+4);
        MvAddStr (8, 25, 4, Shm_Options[Sk].Options_OB.llprc);
        MvAddStr (8, 30, 2, Shm_Options[Sk].Options_OB.llprc+4);
        MvAddStr (8, 43, 6, opt_c0.chekyul_qty);
        MvAddStr (9, 7, 3, opt_c0.oprc);
        MvAddStr (9, 11, 2, opt_c0.oprc+3);
        MvAddStr (9, 23, 3, opt_c0.hprc);
        MvAddStr (9, 27, 2, opt_c0.hprc+3);
        MvAddStr (9, 41, 3, opt_c0.lprc);
        MvAddStr (9, 45, 2, opt_c0.lprc+3);
        MvAddStr (10, 13, 8, opt_c0.tot_con_qty);
        MvAddStr (10, 45, 11, opt_c0.tot_con_amt);

        /* 현재가 */
        MvAddStr (12, 7, 3, opt_c0.crprc);
        MvAddStr (12, 11, 2, opt_c0.crprc+3);
        /* 우선호가 */
        MvAddStr (13, 14, 3, opt_c0.sel_1_prmbid);
        MvAddStr (13, 18, 2, opt_c0.sel_1_prmbid+3);
        MvAddStr (13, 40, 3, opt_c0.buy_1_prmbid);
        MvAddStr (13, 44, 2, opt_c0.buy_1_prmbid+3);
        MvAddStr (14, 14, 3, opt_c0.sel_2_prmbid);
        MvAddStr (14, 18, 2, opt_c0.sel_2_prmbid+3);
        MvAddStr (14, 40, 3, opt_c0.buy_2_prmbid);
        MvAddStr (14, 44, 2, opt_c0.buy_2_prmbid+3);
        MvAddStr (15, 14, 3, opt_c0.sel_3_prmbid);
        MvAddStr (15, 18, 2, opt_c0.sel_3_prmbid+3);
        MvAddStr (15, 40, 3, opt_c0.buy_3_prmbid);
        MvAddStr (15, 44, 2, opt_c0.buy_3_prmbid+3);
        MvAddStr (16, 14, 3, opt_c0.sel_4_prmbid);
        MvAddStr (16, 18, 2, opt_c0.sel_4_prmbid+3);
        MvAddStr (16, 40, 3, opt_c0.buy_4_prmbid);
        MvAddStr (16, 44, 2, opt_c0.buy_4_prmbid+3);
        MvAddStr (17, 14, 3, opt_c0.sel_5_prmbid);
        MvAddStr (17, 18, 2, opt_c0.sel_5_prmbid+3);
        MvAddStr (17, 40, 3, opt_c0.buy_5_prmbid);
        MvAddStr (17, 44, 2, opt_c0.buy_5_prmbid+3);
        /* 총 호가수량 */
        MvAddStr (19, 15, 6, opt_c0.sel_tot_best_qty);
        MvAddStr (19, 41, 6, opt_c0.buy_tot_best_qty);
        /* 호가수량 */
        MvAddStr (20, 18, 7, opt_c0.sel_1_prmbid_qty);
        MvAddStr (20, 44, 7, opt_c0.buy_1_prmbid_qty);
        MvAddStr (21, 18, 7, opt_c0.sel_2_prmbid_qty);
        MvAddStr (21, 44, 7, opt_c0.buy_2_prmbid_qty);
        MvAddStr (22, 18, 7, opt_c0.sel_3_prmbid_qty);
        MvAddStr (22, 44, 7, opt_c0.buy_3_prmbid_qty);
        MvAddStr (23, 18, 7, opt_c0.sel_4_prmbid_qty);
        MvAddStr (23, 44, 7, opt_c0.buy_4_prmbid_qty);
        MvAddStr (24, 18, 7, opt_c0.sel_5_prmbid_qty);
        MvAddStr (24, 44, 7, opt_c0.buy_5_prmbid_qty);
        /* 총 호가건수 */
        MvAddStr (26, 15, 5, opt_c0.sel_tot_best_cnt);
        MvAddStr (26, 41, 5, opt_c0.buy_tot_best_cnt);
        /* 호가건수 */
        MvAddStr (27, 18, 4, opt_c0.sel_1_best_cnt);
        MvAddStr (27, 44, 4, opt_c0.buy_1_best_cnt);
        MvAddStr (28, 18, 4, opt_c0.sel_2_best_cnt);
        MvAddStr (28, 44, 4, opt_c0.buy_2_best_cnt);
        MvAddStr (29, 18, 4, opt_c0.sel_3_best_cnt);
        MvAddStr (29, 44, 4, opt_c0.buy_3_best_cnt);
        MvAddStr (30, 18, 4, opt_c0.sel_4_best_cnt);
        MvAddStr (30, 44, 4, opt_c0.buy_4_best_cnt);
        MvAddStr (31, 18, 4, opt_c0.sel_5_best_cnt);
        MvAddStr (31, 44, 4, opt_c0.buy_5_best_cnt);
	}

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2020_cm.c)
*************************************************************************/
