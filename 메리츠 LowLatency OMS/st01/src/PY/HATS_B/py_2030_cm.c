/*------------------------------------------------------------------------
#   Module  : 시세 - 체결 (CHE_ARRY)
#   File	: py_2030_cm.c
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
void	py_2030_cm (void)
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
				if (In_Arry >= 99 || In_Arry < 0)
					In_Arry = 0;
				else
					In_Arry += 1;

				Disp_Sise();
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			case	KEY_DOWN:
				if (In_Arry > 99 || In_Arry <= 0)
					In_Arry = 99;
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
	mvaddstr (0, 18, "[2030] 시세 - 체결 (C0)");
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
	mvaddstr (7, 0, "체결시각   :  :  -   체결수량");
	mvaddstr (8, 0, "상한가     .      하한가     .");
	mvaddstr (9, 0, "시가      .       고가    .         저가    .");
	mvaddstr (10, 0,
		"누적체결수량              누적거래대금(천원)");

	mvaddstr (12, 0, "현재가    .");

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

			In_Arry = AtoIf(buf, 3);
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	SIF_A3	sif_a3;
	SIO_A3	sio_a3;

	G_Time ();

	attron (A_REVERSE);

	if (FO_Flag == '1')											/* 선물	*/
	{
		mvaddstr (4, 0, "지수선물 체결가 (Futures_C0)");
		Curr_Arry = Shm_Futures[Sk].CHE_Arry_Key;

		Curr_Arry = (Curr_Arry + In_Arry) % 100;
		if (Curr_Arry < 0)
			Curr_Arry += 100;
		else if (Curr_Arry > 100)
			Curr_Arry -= 100;

		memcpy (sif_a3.tr_gbn, 
				Shm_Futures[Sk].Futures_CHE_Arry[Curr_Arry].tr_gbn, 
				sizeof (SIF_A3));
	}
	else														/* 옵션	*/
	{
		mvaddstr (4, 0, "지수옵션 체결가 (Options_C0)");
		Curr_Arry = Shm_Options[Sk].CHE_Arry_Key;

		Curr_Arry = (Curr_Arry + In_Arry) % 100;
		if (Curr_Arry < 0)
			Curr_Arry += 100;
		else if (Curr_Arry > 100)
			Curr_Arry -= 100;

		memcpy (sio_a3.tr_gbn, 
				Shm_Options[Sk].Options_CHE_Arry[Curr_Arry].tr_gbn,
				sizeof (SIO_A3));
	}

	attroff (A_REVERSE);

	attron (A_BOLD);
	
	MvAddNum (3, 5, 3, In_Arry);								/* 순번 */

	if (FO_Flag == '1')											/* 선물	*/
	{
		MvAddStr (5, 10, 10, Shm_Futures[0].ordertime);
		MvAddStr (5, 33, 10, Shm_Futures[0].chetime);
		MvAddStr (6, 9, 8, sif_a3.item_code);
		MvAddStr (6, 33, 30, Shm_Futures[Sk].Futures_A0.kor_nm);
		MvAddStr (7, 9, 2, sif_a3.chekyul_tm);
		MvAddStr (7, 12, 2, sif_a3.chekyul_tm+2);
		MvAddStr (7, 15, 2, sif_a3.chekyul_tm+4);
		MvAddStr (7, 18, 2, sif_a3.chekyul_tm+6);
		MvAddStr (7, 30, 6, sif_a3.chekyul_qty);
		MvAddStr (8, 7, 4, Shm_Futures[Sk].Futures_A0.hlprc+6);
		MvAddStr (8, 12, 2, Shm_Futures[Sk].Futures_A0.hlprc+10);
		MvAddStr (8, 25, 4, Shm_Futures[Sk].Futures_A0.llprc+6);
		MvAddStr (8, 30, 2, Shm_Futures[Sk].Futures_A0.llprc+10);
		MvAddStr (9, 7, 3, sif_a3.oprc);
		MvAddStr (9, 11, 2, sif_a3.oprc+3);
		MvAddStr (9, 23, 3, sif_a3.hprc);
		MvAddStr (9, 27, 2, sif_a3.hprc+3);
		MvAddStr (9, 41, 3, sif_a3.lprc);
		MvAddStr (9, 45, 2, sif_a3.lprc+3);
		MvAddStr (10, 13, 7, sif_a3.tot_con_qty);
		MvAddStr (10, 45, 11, sif_a3.tot_con_amt);

		/* 현재가 */
		MvAddStr (12, 7, 3, sif_a3.crprc);
		MvAddStr (12, 11, 2, sif_a3.crprc+3);
	}
	else														/* 옵션	*/
	{
		MvAddStr (5, 10, 10, Shm_Options[0].ordertime);
		MvAddStr (5, 33, 10, Shm_Options[0].chetime);
        MvAddStr (6, 9, 8, sio_a3.item_code);
        MvAddStr (6, 33, 30, Shm_Options[Sk].Options_A0.kor_nm);
		MvAddStr (7, 9, 2, sio_a3.chekyul_tm);
		MvAddStr (7, 12, 2, sio_a3.chekyul_tm+2);
		MvAddStr (7, 15, 2, sio_a3.chekyul_tm+4);
		MvAddStr (7, 18, 2, sio_a3.chekyul_tm+6);
        MvAddStr (8, 7, 4, Shm_Options[Sk].Options_A0.hlprc);
        MvAddStr (8, 12, 2, Shm_Options[Sk].Options_A0.hlprc+4);
        MvAddStr (8, 25, 4, Shm_Options[Sk].Options_A0.llprc);
        MvAddStr (8, 30, 2, Shm_Options[Sk].Options_A0.llprc+4);
        MvAddStr (9, 7, 3, sio_a3.oprc);
        MvAddStr (9, 11, 2, sio_a3.oprc+3);
        MvAddStr (9, 23, 3, sio_a3.hprc);
        MvAddStr (9, 27, 2, sio_a3.hprc+3);
        MvAddStr (9, 41, 3, sio_a3.lprc);
        MvAddStr (9, 45, 2, sio_a3.lprc+3);
        MvAddStr (10, 13, 8, sio_a3.tot_con_qty);
        MvAddStr (10, 45, 11, sio_a3.tot_con_amt);

        /* 현재가 */
        MvAddStr (12, 7, 3, sio_a3.crprc);
        MvAddStr (12, 11, 2, sio_a3.crprc+3);
	}

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2030_cm.c)
*************************************************************************/
