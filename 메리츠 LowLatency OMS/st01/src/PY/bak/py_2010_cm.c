/*------------------------------------------------------------------------
#   Module  : 시세 - 호가 (B6)
#   File	: py_2010_cm.c
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
void	py_2010_cm (void)
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
	mvaddstr (0, 18, "[2010] 시세 - 호가 (B6)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "상품   (1:선물 2:옵션)    종목코드");
	Draw_Underline (2, 5, 1);
	Draw_Underline (2, 35, 8);

    mvaddstr (5, 0,
        "응답시간 [          ]  체결시간 [          ]");
	mvaddstr (6, 0,
		"종목코드                  한글명");
	mvaddstr (7, 0, "상한가     .      하한가     .");

	mvaddstr (8, 0, "매도1우선호가    .        매수1우선호가    .");
	mvaddstr (9, 0, "매도2우선호가    .        매수2우선호가    .");
	mvaddstr (10, 0, "매도3우선호가    .        매수3우선호가    .");
	mvaddstr (11, 0, "매도4우선호가    .        매수4우선호가    .");
	mvaddstr (12, 0, "매도5우선호가    .        매수5우선호가    .");

	mvaddstr (14, 0, "매도총호가수량            매수총호가수량");
	mvaddstr (15, 0, "매도1우선호가수량         매수1우선호가수량");
	mvaddstr (16, 0, "매도2우선호가수량         매수2우선호가수량");
	mvaddstr (17, 0, "매도3우선호가수량         매수3우선호가수량");
	mvaddstr (18, 0, "매도4우선호가수량         매수4우선호가수량");
	mvaddstr (19, 0, "매도5우선호가수량         매수5우선호가수량");

	mvaddstr (21, 0, "매도총호가건수            매수총호가건수");
	mvaddstr (22, 0, "매도1우선호가건수         매수1우선호가건수");
	mvaddstr (23, 0, "매도2우선호가건수         매수2우선호가건수");
	mvaddstr (24, 0, "매도3우선호가건수         매수3우선호가건수");
	mvaddstr (25, 0, "매도4우선호가건수         매수4우선호가건수");
	mvaddstr (26, 0, "매도5우선호가건수         매수5우선호가건수");

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
					if (memcmp (buf, Shm_Futures[Sk].Futures_A0.item_code+3,
						sizeof (Shm_Futures[Sk].Futures_A0.item_code) - 4) == 0)
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
					if (memcmp (buf, Shm_Options[Sk].Options_A0.item_code+3,
						sizeof (Shm_Options[Sk].Options_A0.item_code) - 4) == 0)
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
	SIF_B6	fms_h0;
	SIO_B6	opt_h0;

	G_Time ();

	attron (A_REVERSE);

	if (FO_Flag == '1')											/* 선물	*/
	{
		mvaddstr (3, 0, "지수선물 호가 (Futures_B6)");
		memcpy (fms_h0.item_code, Shm_Futures[Sk].Futures_B6.item_code,
			sizeof (SIF_B6));
	}
	else														/* 옵션	*/
	{
		mvaddstr (3, 0, "지수옵션 호가 (Options_B6)");
		memcpy (opt_h0.item_code, Shm_Options[Sk].Options_B6.item_code,
			sizeof (SIO_B6));
	}

	attroff (A_REVERSE);

	attron (A_BOLD);

	if (FO_Flag == '1')											/* 선물	*/
	{
		MvAddStr (5, 10, 10, Shm_Futures[0].ordertime);
		MvAddStr (5, 33, 10, Shm_Futures[0].chetime);
		MvAddStr (6, 9, 8, fms_h0.item_code+3);
		MvAddStr (6, 33, 30, Shm_Futures[Sk].Futures_A0.kor_nm);
		MvAddStr (7, 7, 4, Shm_Futures[Sk].Futures_A0.hlprc+6);
		MvAddStr (7, 12, 2, Shm_Futures[Sk].Futures_A0.hlprc+10);
		MvAddStr (7, 25, 4, Shm_Futures[Sk].Futures_A0.llprc+6);
		MvAddStr (7, 30, 2, Shm_Futures[Sk].Futures_A0.llprc+10);

		/* 우선호가 */
		MvAddStr (8, 14, 3, fms_h0.sel_1_prmbid);
		MvAddStr (8, 18, 2, fms_h0.sel_1_prmbid+3);
		MvAddStr (8, 40, 3, fms_h0.buy_1_prmbid);
		MvAddStr (8, 44, 2, fms_h0.buy_1_prmbid+3);
		MvAddStr (9, 14, 3, fms_h0.sel_2_prmbid);
		MvAddStr (9, 18, 2, fms_h0.sel_2_prmbid+3);
		MvAddStr (9, 40, 3, fms_h0.buy_2_prmbid);
		MvAddStr (9, 44, 2, fms_h0.buy_2_prmbid+3);
		MvAddStr (10, 14, 3, fms_h0.sel_3_prmbid);
		MvAddStr (10, 18, 2, fms_h0.sel_3_prmbid+3);
		MvAddStr (10, 40, 3, fms_h0.buy_3_prmbid);
		MvAddStr (10, 44, 2, fms_h0.buy_3_prmbid+3);
		MvAddStr (11, 14, 3, fms_h0.sel_4_prmbid);
		MvAddStr (11, 18, 2, fms_h0.sel_4_prmbid+3);
		MvAddStr (11, 40, 3, fms_h0.buy_4_prmbid);
		MvAddStr (11, 44, 2, fms_h0.buy_4_prmbid+3);
		MvAddStr (12, 14, 3, fms_h0.sel_5_prmbid);
		MvAddStr (12, 18, 2, fms_h0.sel_5_prmbid+3);
		MvAddStr (12, 40, 3, fms_h0.buy_5_prmbid);
		MvAddStr (12, 44, 2, fms_h0.buy_5_prmbid+3);
		/* 총 호가수량 */
		MvAddStr (14, 15, sizeof(fms_h0.sel_tot_best_qty),
                                        fms_h0.sel_tot_best_qty);
        MvAddStr (14, 41, sizeof(fms_h0.buy_tot_best_qty),
                                        fms_h0.buy_tot_best_qty);
        /* 호가수량 */
        MvAddStr (15, 18, sizeof(fms_h0.sel_1_prmbid_qty),
                                        fms_h0.sel_1_prmbid_qty);
        MvAddStr (15, 44, sizeof(fms_h0.buy_1_prmbid_qty),
                                        fms_h0.buy_1_prmbid_qty);
        MvAddStr (16, 18, sizeof(fms_h0.sel_2_prmbid_qty),
                                        fms_h0.sel_2_prmbid_qty);
        MvAddStr (16, 44, sizeof(fms_h0.buy_2_prmbid_qty),
                                        fms_h0.buy_2_prmbid_qty);
        MvAddStr (17, 18, sizeof(fms_h0.sel_3_prmbid_qty),
                                        fms_h0.sel_3_prmbid_qty);
        MvAddStr (17, 44, sizeof(fms_h0.buy_3_prmbid_qty),
                                        fms_h0.buy_3_prmbid_qty);
        MvAddStr (18, 18, sizeof(fms_h0.sel_4_prmbid_qty),
                                        fms_h0.sel_4_prmbid_qty);
        MvAddStr (18, 44, sizeof(fms_h0.buy_4_prmbid_qty),
                                        fms_h0.buy_4_prmbid_qty);
        MvAddStr (19, 18, sizeof(fms_h0.sel_5_prmbid_qty),
                                        fms_h0.sel_5_prmbid_qty);
        MvAddStr (19, 44, sizeof(fms_h0.buy_5_prmbid_qty),
                                        fms_h0.buy_5_prmbid_qty);
        /* 총 호가건수 */
        MvAddStr (21, 15, sizeof(fms_h0.sel_tot_best_cnt),
                                        fms_h0.sel_tot_best_cnt);
        MvAddStr (21, 41, sizeof(fms_h0.buy_tot_best_cnt),
                                        fms_h0.buy_tot_best_cnt);
        /* 호가건수 */
        MvAddStr (22, 18, sizeof(fms_h0.sel_1_best_cnt),
                                        fms_h0.sel_1_best_cnt);
        MvAddStr (22, 44, sizeof(fms_h0.buy_1_best_cnt),
                                        fms_h0.buy_1_best_cnt);
        MvAddStr (23, 18, sizeof(fms_h0.sel_2_best_cnt),
                                        fms_h0.sel_2_best_cnt);
        MvAddStr (23, 44, sizeof(fms_h0.buy_2_best_cnt),
                                        fms_h0.buy_2_best_cnt);
        MvAddStr (24, 18, sizeof(fms_h0.sel_3_best_cnt),
                                        fms_h0.sel_3_best_cnt);
        MvAddStr (24, 44, sizeof(fms_h0.buy_3_best_cnt),
                                        fms_h0.buy_3_best_cnt);
        MvAddStr (25, 18, sizeof(fms_h0.sel_4_best_cnt),
                                        fms_h0.sel_4_best_cnt);
        MvAddStr (25, 44, sizeof(fms_h0.buy_4_best_cnt),
                                        fms_h0.buy_4_best_cnt);
        MvAddStr (26, 18, sizeof(fms_h0.sel_5_best_cnt),
                                        fms_h0.sel_5_best_cnt);
        MvAddStr (26, 44, sizeof(fms_h0.buy_5_best_cnt),
                                        fms_h0.buy_5_best_cnt);
	}
	else														/* 옵션	*/
	{
		MvAddStr (5, 10, 10, Shm_Options[0].ordertime);
		MvAddStr (5, 33, 10, Shm_Options[0].chetime);
        MvAddStr (6, 9, 8, opt_h0.item_code+3);
        MvAddStr (6, 33, 30, Shm_Options[Sk].Options_A0.kor_nm);
        MvAddStr (7, 7, 4, Shm_Options[Sk].Options_A0.hlprc+6);
        MvAddStr (7, 12, 2, Shm_Options[Sk].Options_A0.hlprc+10);
        MvAddStr (7, 25, 4, Shm_Options[Sk].Options_A0.llprc+6);
        MvAddStr (7, 30, 2, Shm_Options[Sk].Options_A0.llprc+10);

        /* 우선호가 */
        MvAddStr (8, 14, 3, opt_h0.sel_1_prmbid);
        MvAddStr (8, 18, 2, opt_h0.sel_1_prmbid+3);
        MvAddStr (8, 40, 3, opt_h0.buy_1_prmbid);
        MvAddStr (8, 44, 2, opt_h0.buy_1_prmbid+3);
        MvAddStr (9, 14, 3, opt_h0.sel_2_prmbid);
        MvAddStr (9, 18, 2, opt_h0.sel_2_prmbid+3);
        MvAddStr (9, 40, 3, opt_h0.buy_2_prmbid);
        MvAddStr (9, 44, 2, opt_h0.buy_2_prmbid+3);
        MvAddStr (10, 14, 3, opt_h0.sel_3_prmbid);
        MvAddStr (10, 18, 2, opt_h0.sel_3_prmbid+3);
        MvAddStr (10, 40, 3, opt_h0.buy_3_prmbid);
        MvAddStr (10, 44, 2, opt_h0.buy_3_prmbid+3);
        MvAddStr (11, 14, 3, opt_h0.sel_4_prmbid);
        MvAddStr (11, 18, 2, opt_h0.sel_4_prmbid+3);
        MvAddStr (11, 40, 3, opt_h0.buy_4_prmbid);
        MvAddStr (11, 44, 2, opt_h0.buy_4_prmbid+3);
        MvAddStr (12, 14, 3, opt_h0.sel_5_prmbid);
        MvAddStr (12, 18, 2, opt_h0.sel_5_prmbid+3);
        MvAddStr (12, 40, 3, opt_h0.buy_5_prmbid);
        MvAddStr (12, 44, 2, opt_h0.buy_5_prmbid+3);
        /* 총 호가수량 */
        MvAddStr (14, 15, sizeof(opt_h0.sel_tot_best_qty), 
										opt_h0.sel_tot_best_qty);
        MvAddStr (14, 41, sizeof(opt_h0.buy_tot_best_qty), 
										opt_h0.buy_tot_best_qty);
        /* 호가수량 */
        MvAddStr (15, 18, sizeof(opt_h0.sel_1_prmbid_qty), 
										opt_h0.sel_1_prmbid_qty);
        MvAddStr (15, 44, sizeof(opt_h0.buy_1_prmbid_qty), 
										opt_h0.buy_1_prmbid_qty);
        MvAddStr (16, 18, sizeof(opt_h0.sel_2_prmbid_qty), 
										opt_h0.sel_2_prmbid_qty);
        MvAddStr (16, 44, sizeof(opt_h0.buy_2_prmbid_qty), 
										opt_h0.buy_2_prmbid_qty);
        MvAddStr (17, 18, sizeof(opt_h0.sel_3_prmbid_qty), 
										opt_h0.sel_3_prmbid_qty);
        MvAddStr (17, 44, sizeof(opt_h0.buy_3_prmbid_qty), 
										opt_h0.buy_3_prmbid_qty);
        MvAddStr (18, 18, sizeof(opt_h0.sel_4_prmbid_qty), 
										opt_h0.sel_4_prmbid_qty);
        MvAddStr (18, 44, sizeof(opt_h0.buy_4_prmbid_qty), 
										opt_h0.buy_4_prmbid_qty);
        MvAddStr (19, 18, sizeof(opt_h0.sel_5_prmbid_qty), 
										opt_h0.sel_5_prmbid_qty);
        MvAddStr (19, 44, sizeof(opt_h0.buy_5_prmbid_qty), 
										opt_h0.buy_5_prmbid_qty);
        /* 총 호가건수 */
        MvAddStr (21, 15, sizeof(opt_h0.sel_tot_best_cnt), 
										opt_h0.sel_tot_best_cnt);
        MvAddStr (21, 41, sizeof(opt_h0.buy_tot_best_cnt), 
										opt_h0.buy_tot_best_cnt);
        /* 호가건수 */
        MvAddStr (22, 18, sizeof(opt_h0.sel_1_best_cnt), 
										opt_h0.sel_1_best_cnt);
        MvAddStr (22, 44, sizeof(opt_h0.buy_1_best_cnt), 
										opt_h0.buy_1_best_cnt);
        MvAddStr (23, 18, sizeof(opt_h0.sel_2_best_cnt), 
										opt_h0.sel_2_best_cnt);
        MvAddStr (23, 44, sizeof(opt_h0.buy_2_best_cnt), 
										opt_h0.buy_2_best_cnt);
        MvAddStr (24, 18, sizeof(opt_h0.sel_3_best_cnt), 
										opt_h0.sel_3_best_cnt);
        MvAddStr (24, 44, sizeof(opt_h0.buy_3_best_cnt), 
										opt_h0.buy_3_best_cnt);
        MvAddStr (25, 18, sizeof(opt_h0.sel_4_best_cnt), 
										opt_h0.sel_4_best_cnt);
        MvAddStr (25, 44, sizeof(opt_h0.buy_4_best_cnt), 
										opt_h0.buy_4_best_cnt);
        MvAddStr (26, 18, sizeof(opt_h0.sel_5_best_cnt), 
										opt_h0.sel_5_best_cnt);
        MvAddStr (26, 44, sizeof(opt_h0.buy_5_best_cnt), 
										opt_h0.buy_5_best_cnt);
	}

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2010_cm.c)
*************************************************************************/
