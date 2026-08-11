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
int			Sk, Curr_Arry, InDex, InDex;

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
				if (InDex >= 199 || InDex < 0)
					InDex = 0;
				else
					InDex += 1;

				clear ();
				Disp_Scr ();
				Disp_Sise();
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			case	KEY_DOWN:
				if (InDex > 199 || InDex <= 0)
					InDex = 199;
				else
					InDex -= 1;

				clear ();
				Disp_Scr ();
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
	mvaddstr (0, 18, "[2040] 현물, KOSPI 200 조회");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "일련번호");
	Draw_Underline (2,  9, 3);
	mvaddstr (3, 0, "종목코드               한글명");
	mvaddstr (5, 0, "체결가격               상한가               하한가");
	mvaddstr (6, 0, "    시가                 고가                 저가");
	mvaddstr (7, 0,
		"체결수량               누적호가수량");
	mvaddstr (8, 0, 
		"지수비율   .           누적거래대금(천원)");

	mvaddstr (10, 0, "[KOSPI 지수]");
	mvaddstr (11, 0,
		"업종코드         시각                부호   ('+':상승, '-':하락,' ':보합)");
	mvaddstr (12, 0, "지수        .    대비        .");
	mvaddstr (13, 0, "거래량(체결량)           (단위:천주)");
	mvaddstr (14, 0, "거래대금                 (단위:백만원)");

	mvaddstr (17, 0, "[KOSPI200 지수]");
	mvaddstr (18, 0,
		"업종코드         시각                부호   (('+':상승, '-':하락,' ':보합)");
	mvaddstr (19, 0, "지수        .    대비        .");
	mvaddstr (20, 0, "거래량(체결량)           (단위:천주)");
	mvaddstr (21, 0, "거래대금                 (단위:백만원)");

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

	Disp_Msg ("<일련번호> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 9, 3);
		rt = Get_String (2, 9, 3, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("<일련번호> 입력 (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			attron (A_UNDERLINE);
			mvaddstr (2, 9, buf);
			attroff (A_UNDERLINE);

			if ((AtoIf(buf, 3) < 0 && AtoIf(buf, 3) > 200))
			{
				Disp_Msg ("일련번호 입력 오류 (Esc:Cancel)");
				continue;
			}

			InDex = AtoIf(buf, 3);
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	G_Time ();

	attron (A_BOLD);
	
	/* 일련번호	*/
	MvAddNum (2,  9,  3, InDex);
	/* 종목코드	 */
	MvAddStr (3,  9, 12, Shm_Stock[InDex].Stock_A0.stock_code);
	/* 한글명	*/
	MvAddStr (3, 30, 40, Shm_Stock[InDex].Stock_A0.name);

	/* 체결가격	*/
	MvAddStr (5,  9,  9, Shm_Stock[InDex].Stock_A3.current_price);
	/* 상한가	*/
	MvAddStr (5, 30,  9, Shm_Stock[InDex].Stock_A0.high_limit_price);
	/* 하한가	*/
	MvAddStr (5, 51,  9, Shm_Stock[InDex].Stock_A0.low_limit_price);

	/* 시가	*/
	MvAddStr (6,  9,  9, Shm_Stock[InDex].Stock_A3.opening_price);
	/* 고가	*/
	MvAddStr (6,  9,  9, Shm_Stock[InDex].Stock_A3.highest_price);
	/* 저가	*/
	MvAddStr (6,  9,  9, Shm_Stock[InDex].Stock_A3.lowest_price);

	/* 체결수량	*/
	MvAddStr (7,  9,  9, Shm_Stock[InDex].Stock_A3.lowest_price);
	/* 누적호가수량	*/
	MvAddStr (7, 36, 12, Shm_Stock[InDex].Stock_A3.accum_trade_qty);
	/* 누적거래대금(천원)	*/
	MvAddStr (8, 42, 18, Shm_Stock[InDex].Stock_A3.accum_trade_amt);

	/* 지수비율	*/
	MvAddNum (8,  9,  2, Shm_Stock[InDex].kospi_bi/100);
	MvAddNum (8, 12,  2, Shm_Stock[InDex].kospi_bi%100);

	/* KOSPI 지수	*/
		/* 업종코드	*/
		MvAddStr (11,  9,  3, Shm_Jisu[0].Stock_D0.up_gbn);
		/* 시각	*/
		MvAddStr (11, 22,  6, Shm_Jisu[0].Stock_D0.time);
		/* 부호	*/
		MvAddStr (11, 42,  1, Shm_Jisu[0].Stock_D0.sign);

		/* 지수	*/
		MvAddStr (12,  6,  6, Shm_Jisu[0].Stock_D0.idx);
		MvAddStr (12, 13,  2, Shm_Jisu[0].Stock_D0.idx+6);
		/* 대비	*/
		MvAddStr (12, 23,  6, Shm_Jisu[0].Stock_D0.dif);
		MvAddStr (12, 30,  2, Shm_Jisu[0].Stock_D0.dif+6);

		/* 체결량(거래량)	*/
		MvAddStr (13, 15,  8, Shm_Jisu[0].Stock_D0.qty);

		/* 거래대금	*/
		MvAddStr (13, 15,  8, Shm_Jisu[0].Stock_D0.amt);

	/* KOSPI200 지수	*/
		/* 업종코드	*/
		MvAddStr (18,  9,  3, Shm_Jisu[0].Stock_D2.up_gbn);
		/* 시각	*/
		MvAddStr (18, 22,  6, Shm_Jisu[0].Stock_D2.time);
		/* 부호	*/
		MvAddStr (18, 42,  1, Shm_Jisu[0].Stock_D2.sign);

		/* 지수	*/
		MvAddStr (19,  6,  6, Shm_Jisu[0].Stock_D2.idx);
		MvAddStr (19, 13,  2, Shm_Jisu[0].Stock_D2.idx+6);
		/* 대비	*/
		MvAddStr (19, 23,  6, Shm_Jisu[0].Stock_D2.dif);
		MvAddStr (19, 30,  2, Shm_Jisu[0].Stock_D2.dif+6);

		/* 체결량(거래량)	*/
		MvAddStr (20, 15,  8, Shm_Jisu[0].Stock_D2.qty);

		/* 거래대금	*/
		MvAddStr (21, 15,  8, Shm_Jisu[0].Stock_D2.amt);

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2040_cm.c)
*************************************************************************/
