/*------------------------------------------------------------------------
#   Module  : 시세 - 호가 (CURR)
#   File	: py_2040_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			Sk, Curr_Arry, In_Arry;
char		FO_Flag, ItemCd[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Sise (void);
static void		Disp_Sise (void);

/*----------------------------------------------------------------------*/
void	py_2040_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sise_SHM ();
	Info_Sise ();

	Disp_Sise ();
	Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
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
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			case	KEY_UP:
				if (In_Arry >= 29 || In_Arry < 0)
					In_Arry = 0;
				else
					In_Arry += 1;

				Disp_Sise();
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			case	KEY_DOWN:
				if (In_Arry > 29 || In_Arry <= 0)
					In_Arry = 29;
				else
					In_Arry -= 1;

				Disp_Sise();
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			default:
				clear ();
				Disp_Scr ();
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
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
	mvaddstr (0, 18, "[2040] 시세 - 현재가 (CURR)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "상품   (1:선물 2:옵션)    종목코드");
	Draw_Underline (2, 5, 1);
	Draw_Underline (2, 35, 8);
	mvaddstr (3, 0, "순번");
	Draw_Underline (3, 5, 3);

	mvaddstr (5, 0,
		"응답시간 [          ]  체결시간 [          ]");
	mvaddstr (6, 0,
		"종목코드                  한글명");
	mvaddstr (8, 0, "상한가     .      하한가     .");
	mvaddstr (9, 0, "시가      .       고가    .         저가    .");
	mvaddstr (10, 0,
		"체결수량        누적호가수량         누적거래대금(천원)");

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
				for (Sk = 0; Sk < AtoIf(Shm_Futures[0].Futures_A0.cnt, 5); Sk ++)
				{
					if (memcmp (buf, Shm_Futures[Sk].Futures_A0.item_code+3,
						sizeof (Shm_Futures[Sk].Futures_A0.item_code) - 4) == 0)
						break;
				}

				if (Sk == AtoIf(Shm_Futures[0].Futures_A0.cnt, 5))
				{
					Disp_Msg ("선물 종목코드 미존재 (Esc:Cancel)");
					continue;
				}
			}													/* 옵션	*/
			else
			{
				for (Sk = 0; Sk < AtoIf(Shm_Options[0].Options_A0.cnt, 5); Sk ++)
				{
					if (memcmp (buf, Shm_Options[Sk].Options_A0.item_code+3,
						sizeof (Shm_Options[Sk].Options_A0.item_code) - 4) == 0)
						break;
				}

				if (Sk == AtoIf(Shm_Options[0].Options_A0.cnt, 5))
				{
					Disp_Msg ("옵션 종목코드 미존재 (Esc:Cancel)");
					continue;
				}
			}

			memcpy (ItemCd, buf, 8);
			break;
		}
	}

	Disp_Msg ("<순번> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (3, 5, 3);
		rt = Get_String (3, 5, 3, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("<순번> 입력 (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			attron (A_UNDERLINE);
			mvaddstr (3, 5, buf);
			attroff (A_UNDERLINE);

			if (strlen (buf) != 2)
			{
				Disp_Msg ("순번 오류 (Esc:Cancel)");
				continue;
			}

			In_Arry = AtoIf(buf, 2);
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	SIF_CURR	fms_curr;
	SIF_G7		fms_c0;
	SIO_CURR	opt_curr;
	SIO_G7		opt_c0;

	G_Time ();

	attron (A_REVERSE);

	if (FO_Flag == '1')										/* 선물	*/
	{
		mvaddstr (4, 0, "지수선물 현재가 (Futures_CURR)");
		Curr_Arry = Shm_Futures[Sk].CURR_Arry_Key;

		Curr_Arry = (Curr_Arry + In_Arry) % 30;
		if (Curr_Arry < 0)
			Curr_Arry += 30;
		else if (Curr_Arry > 30)
			Curr_Arry -= 30;

		memcpy (fms_curr.tr_gbn, 
				Shm_Futures[Sk].Futures_CURR_Arry[Curr_Arry].tr_gbn, 
				sizeof (SIF_G7));
	}
	else													/* 옵션	*/
	{
		mvaddstr (4, 0, "지수옵션 현재가 (Options_CURR)");
		Curr_Arry = Shm_Options[Sk].CURR_Arry_Key;

		Curr_Arry = (Curr_Arry + In_Arry) % 30;
		if (Curr_Arry < 0)
			Curr_Arry += 30;
		else if (Curr_Arry > 30)
			Curr_Arry -= 30;

		memcpy (opt_curr.tr_gbn, 
				Shm_Options[Sk].Options_CURR_Arry[Curr_Arry].tr_gbn,
				sizeof (SIO_G7));
	}

	attroff (A_REVERSE);

	attron (A_BOLD);
	
	MvAddNum (3, 5, 3, In_Arry);								/* 순번 */

	if (FO_Flag == '2')											/* 선물	*/
	{
		MvAddStr (5, 10, 10, Shm_Options[0].ordertime);
		MvAddStr (5, 33, 10, Shm_Options[0].chetime);
        MvAddStr (6, 9, 8, opt_curr.item_code+3);
        MvAddStr (6, 33, 30, Shm_Options[Sk].Options_A0.kor_nm);
        MvAddStr (8, 7, 4, Shm_Options[Sk].Options_A0.hlprc+6);
        MvAddStr (8, 12, 2, Shm_Options[Sk].Options_A0.hlprc+10);
        MvAddStr (8, 25, 4, Shm_Options[Sk].Options_A0.llprc+6);
        MvAddStr (8, 30, 2, Shm_Options[Sk].Options_A0.llprc+10);
        MvAddStr (9, 7, 3, opt_curr.oprc);
        MvAddStr (9, 11, 2, opt_curr.oprc+3);
        MvAddStr (9, 23, 3, opt_curr.hprc);
        MvAddStr (9, 27, 2, opt_curr.hprc+3);
        MvAddStr (9, 41, 3, opt_curr.lprc);
        MvAddStr (9, 45, 2, opt_curr.lprc+3);
		MvAddStr (10, 9, 7, opt_curr.chekyul_qty);
        MvAddStr (10, 29, 8, opt_curr.tot_con_qty);
        MvAddStr (10, 56, 11, opt_curr.tot_con_amt);

        /* 현재가 */
        MvAddStr (12, 7, 3, opt_curr.crprc);
        MvAddStr (12, 11, 2, opt_curr.crprc+3);
        /* 우선호가 */
        MvAddStr (13, 14, 3, opt_curr.sel_1_prmbid);
        MvAddStr (13, 18, 2, opt_curr.sel_1_prmbid+3);
        MvAddStr (13, 40, 3, opt_curr.buy_1_prmbid);
        MvAddStr (13, 44, 2, opt_curr.buy_1_prmbid+3);
        MvAddStr (14, 14, 3, opt_curr.sel_2_prmbid);
        MvAddStr (14, 18, 2, opt_curr.sel_2_prmbid+3);
        MvAddStr (14, 40, 3, opt_curr.buy_2_prmbid);
        MvAddStr (14, 44, 2, opt_curr.buy_2_prmbid+3);
        MvAddStr (15, 14, 3, opt_curr.sel_3_prmbid);
        MvAddStr (15, 18, 2, opt_curr.sel_3_prmbid+3);
        MvAddStr (15, 40, 3, opt_curr.buy_3_prmbid);
        MvAddStr (15, 44, 2, opt_curr.buy_3_prmbid+3);
        MvAddStr (16, 14, 3, opt_curr.sel_4_prmbid);
        MvAddStr (16, 18, 2, opt_curr.sel_4_prmbid+3);
        MvAddStr (16, 40, 3, opt_curr.buy_4_prmbid);
        MvAddStr (16, 44, 2, opt_curr.buy_4_prmbid+3);
        MvAddStr (17, 14, 3, opt_curr.sel_5_prmbid);
        MvAddStr (17, 18, 2, opt_curr.sel_5_prmbid+3);
        MvAddStr (17, 40, 3, opt_curr.buy_5_prmbid);
        MvAddStr (17, 44, 2, opt_curr.buy_5_prmbid+3);
        /* 총 호가수량 */
        MvAddStr (19, 15, 7, opt_curr.sel_tot_best_qty);
        MvAddStr (19, 41, 7, opt_curr.buy_tot_best_qty);
        /* 호가수량 */
        MvAddStr (20, 18, 7, opt_curr.sel_1_prmbid_qty);
        MvAddStr (20, 44, 7, opt_curr.buy_1_prmbid_qty);
        MvAddStr (21, 18, 7, opt_curr.sel_2_prmbid_qty);
        MvAddStr (21, 44, 7, opt_curr.buy_2_prmbid_qty);
        MvAddStr (22, 18, 7, opt_curr.sel_3_prmbid_qty);
        MvAddStr (22, 44, 7, opt_curr.buy_3_prmbid_qty);
        MvAddStr (23, 18, 7, opt_curr.sel_4_prmbid_qty);
        MvAddStr (23, 44, 7, opt_curr.buy_4_prmbid_qty);
        MvAddStr (24, 18, 7, opt_curr.sel_5_prmbid_qty);
        MvAddStr (24, 44, 7, opt_curr.buy_5_prmbid_qty);
        /* 총 호가건수 */
        MvAddStr (26, 15, 5, opt_curr.sel_tot_best_cnt);
        MvAddStr (26, 41, 5, opt_curr.buy_tot_best_cnt);
        /* 호가건수 */
        MvAddStr (27, 18, 4, opt_curr.sel_1_best_cnt);
        MvAddStr (27, 44, 4, opt_curr.buy_1_best_cnt);
        MvAddStr (28, 18, 4, opt_curr.sel_2_best_cnt);
        MvAddStr (28, 44, 4, opt_curr.buy_2_best_cnt);
        MvAddStr (29, 18, 4, opt_curr.sel_3_best_cnt);
        MvAddStr (29, 44, 4, opt_curr.buy_3_best_cnt);
        MvAddStr (30, 18, 4, opt_curr.sel_4_best_cnt);
        MvAddStr (30, 44, 4, opt_curr.buy_4_best_cnt);
        MvAddStr (31, 18, 4, opt_curr.sel_5_best_cnt);
        MvAddStr (31, 44, 4, opt_curr.buy_5_best_cnt);
	}
	else
	{
		MvAddStr (5, 10, 10, Shm_Futures[0].ordertime);
/*
		MvAddStr (5, 33, 10, Shm_Futures[0].chetime);
*/
        MvAddStr (5, 33, 8, fms_curr.chekyul_tm);

		MvAddStr (6, 9, 8, fms_curr.item_code+3);
		MvAddStr (6, 33, 30, Shm_Futures[Sk].Futures_A0.kor_nm);
		MvAddStr (8, 7, 4, Shm_Futures[Sk].Futures_A0.hlprc+6);
		MvAddStr (8, 12, 2, Shm_Futures[Sk].Futures_A0.hlprc+10);
		MvAddStr (8, 25, 4, Shm_Futures[Sk].Futures_A0.llprc+6);
		MvAddStr (8, 30, 2, Shm_Futures[Sk].Futures_A0.llprc+10);
		MvAddStr (9, 7, 3, fms_curr.oprc);
		MvAddStr (9, 11, 2, fms_curr.oprc+3);
		MvAddStr (9, 23, 3, fms_curr.hprc);
		MvAddStr (9, 27, 2, fms_curr.hprc+3);
		MvAddStr (9, 41, 3, fms_curr.lprc);
		MvAddStr (9, 45, 2, fms_curr.lprc+3);
		MvAddStr (10, 9, 6, fms_curr.chekyul_qty);
		MvAddStr (10, 29, 7, fms_curr.tot_con_qty);
		MvAddStr (10, 56, 11, fms_curr.tot_con_amt);

		/* 현재가 */
		MvAddStr (12, 7, 3, fms_curr.crprc);
		MvAddStr (12, 11, 2, fms_curr.crprc+3);
		/* 우선호가 */
		MvAddStr (13, 14, 3, fms_curr.sel_1_prmbid);
		MvAddStr (13, 18, 2, fms_curr.sel_1_prmbid+3);
		MvAddStr (13, 40, 3, fms_curr.buy_1_prmbid);
		MvAddStr (13, 44, 2, fms_curr.buy_1_prmbid+3);
		MvAddStr (14, 14, 3, fms_curr.sel_2_prmbid);
		MvAddStr (14, 18, 2, fms_curr.sel_2_prmbid+3);
		MvAddStr (14, 40, 3, fms_curr.buy_2_prmbid);
		MvAddStr (14, 44, 2, fms_curr.buy_2_prmbid+3);
		MvAddStr (15, 14, 3, fms_curr.sel_3_prmbid);
		MvAddStr (15, 18, 2, fms_curr.sel_3_prmbid+3);
		MvAddStr (15, 40, 3, fms_curr.buy_3_prmbid);
		MvAddStr (15, 44, 2, fms_curr.buy_3_prmbid+3);
		MvAddStr (16, 14, 3, fms_curr.sel_4_prmbid);
		MvAddStr (16, 18, 2, fms_curr.sel_4_prmbid+3);
		MvAddStr (16, 40, 3, fms_curr.buy_4_prmbid);
		MvAddStr (16, 44, 2, fms_curr.buy_4_prmbid+3);
		MvAddStr (17, 14, 3, fms_curr.sel_5_prmbid);
		MvAddStr (17, 18, 2, fms_curr.sel_5_prmbid+3);
		MvAddStr (17, 40, 3, fms_curr.buy_5_prmbid);
		MvAddStr (17, 44, 2, fms_curr.buy_5_prmbid+3);
		/* 총 호가수량 */
		MvAddStr (19, 15, 6, fms_curr.sel_tot_best_qty);
		MvAddStr (19, 41, 6, fms_curr.buy_tot_best_qty);
		/* 호가수량 */
		MvAddStr (20, 18, 6, fms_curr.sel_1_prmbid_qty);
		MvAddStr (20, 44, 6, fms_curr.buy_1_prmbid_qty);
		MvAddStr (21, 18, 6, fms_curr.sel_2_prmbid_qty);
		MvAddStr (21, 44, 6, fms_curr.buy_2_prmbid_qty);
		MvAddStr (22, 18, 6, fms_curr.sel_3_prmbid_qty);
		MvAddStr (22, 44, 6, fms_curr.buy_3_prmbid_qty);
		MvAddStr (23, 18, 6, fms_curr.sel_4_prmbid_qty);
		MvAddStr (23, 44, 6, fms_curr.buy_4_prmbid_qty);
		MvAddStr (24, 18, 6, fms_curr.sel_5_prmbid_qty);
		MvAddStr (24, 44, 6, fms_curr.buy_5_prmbid_qty);
		/* 총 호가건수 */
		MvAddStr (26, 15, 5, fms_curr.sel_tot_best_cnt);
		MvAddStr (26, 41, 5, fms_curr.buy_tot_best_cnt);
		/* 호가건수 */
		MvAddStr (27, 18, 4, fms_curr.sel_1_best_cnt);
		MvAddStr (27, 44, 4, fms_curr.buy_1_best_cnt);
		MvAddStr (28, 18, 4, fms_curr.sel_2_best_cnt);
		MvAddStr (28, 44, 4, fms_curr.buy_2_best_cnt);
		MvAddStr (29, 18, 4, fms_curr.sel_3_best_cnt);
		MvAddStr (29, 44, 4, fms_curr.buy_3_best_cnt);
		MvAddStr (30, 18, 4, fms_curr.sel_4_best_cnt);
		MvAddStr (30, 44, 4, fms_curr.buy_4_best_cnt);
		MvAddStr (31, 18, 4, fms_curr.sel_5_best_cnt);
		MvAddStr (31, 44, 4, fms_curr.buy_5_best_cnt);
	}

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2040_cm.c)
*************************************************************************/
