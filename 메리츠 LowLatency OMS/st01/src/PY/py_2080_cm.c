/*------------------------------------------------------------------------
#   Module  : 미체결 내역조회
#   File	: py_2080_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			Arry, In_Arry, roop, APTYPE;
char		ItemCd[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Sise (void);
static void		Disp_Sise (void);

/*----------------------------------------------------------------------*/
void	py_2080_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

    Sub_SHM ();
    Mem_SHM (0, -1);
    Sise_SHM ();

	Info_Sise ();

	Disp_Sise ();
	Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		roop = 0;
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
				roop = 1;

				Disp_Sise();
				Disp_Msg ("Enter:Retry I:Info Q:Quit ↑:Up ↓:Down");
				break;
			case	KEY_DOWN:
				roop = -1;

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
	int	i;

	Disp_Title ();
	attron (A_BOLD);
	mvaddstr (0, 18, "[2080] 시세 - 체결 (C0)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 90);
	mvaddstr (2, 0, "ApType     [파생(0:전체, 31~50:개별), ELW(100:전체, 61~70:개별)]");
	Draw_Underline (2, 7, 3);

	mvaddstr (4, 0, "보유건수 [    ]");

	mvaddstr (6, 0, "  Seq    Accno      Code    Ord_No  Oord_No Flag do.su Cnt  Jan_Su  Price  Jo Yu J_Seq");
	Draw_Line (7, 0, 90);

	for(i = 0; i < 28; i++)
	{
		mvaddstr (8+i, 0,
"[    ][         ][        ][       ][       ][ ] [  ][     ][     ][      ][ ][ ][    ]");
	}

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

	Disp_Msg ("<ApType> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 7, 3);
		rt = Get_String (2, 7, 3, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("<ApType> 입력 (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			attron (A_UNDERLINE);
			mvaddstr (2, 7, buf);
			attroff (A_UNDERLINE);

			APTYPE = AtoIf(buf, 3);
            if (APTYPE != 0 && APTYPE != 100 &&
				(APTYPE < 31 || APTYPE > 70))
            {
                Disp_Msg ("ApType 오류 (Esc:Cancel)");
                continue;
            }
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	int	j, i, tot_mcnt, elw_tot_mcnt, prt;

	tot_mcnt = elw_tot_mcnt = 0;

	prt = j = 0;
	if (APTYPE >= 61 || APTYPE == 100)								/* ELW	*/
	{
		for (i = 0; i < 10; i++)
			elw_tot_mcnt = elw_tot_mcnt + ACCNO(0,i+ACC_NO_CNT).miche_cnt;

		for (i = 0; i < 10000; i ++)
		{
			if (APTYPE == 100)
			{
				if (Shm_Elw_Db[0].E_MiChe[i].Jan_Cnt > 0)
				{
					if (prt >= 0 && prt < 28)
					{
/* park */
						MvAddNum (j+8,  1, 4, i);
						MvAddStr (j+8,  7, 9, Shm_Elw_Db[0].E_MiChe[i].AccountNo);
						MvAddStr (j+8, 18, 8, Shm_Elw_Db[0].E_MiChe[i].Item_Cd);
						MvAddStr (j+8, 28, 7, Shm_Elw_Db[0].E_MiChe[i].OrderNo);
						MvAddStr (j+8, 37, 7, Shm_Elw_Db[0].E_MiChe[i].OriginalOrderNo);
						MvAddStr (j+8, 46, 1, Shm_Elw_Db[0].E_MiChe[i].PriceFlag);
						MvAddStr (j+8, 50, 2, Shm_Elw_Db[0].E_MiChe[i].TradeFlag);
						MvAddStr (j+8, 54, 5, Shm_Elw_Db[0].E_MiChe[i].Order_Cnt+3);
						MvAddStr (j+8, 61, 5, Shm_Elw_Db[0].E_MiChe[i].Order_Jan_Cnt+3);
						MvAddStr (j+8, 68, 6, Shm_Elw_Db[0].E_MiChe[i].Order_Price+3);
						MvAddStr (j+8, 76, 1, Shm_Elw_Db[0].E_MiChe[i].JumunFlag);
						MvAddStr (j+8, 79, 1, Shm_Elw_Db[0].E_MiChe[i].OrderType);
						MvAddNum (j+8, 82, 4, Shm_Elw_Db[0].E_MiChe[i].Item_Seq);
						j ++;
					}
					prt = prt + 1;
				}
			}
			else
			{
				if ((Shm_Elw_Db[0].E_MiChe[i].Jan_Cnt > 0) &&
					(memcmp (Shm_Elw_Db[0].E_MiChe[i].AccountNo, ACCNO(0,APTYPE-ELW_ACC_NO_BASE+ACC_NO_CNT).acc_no, 9) == 0))
				{
					if (prt >= 0 && prt < 28)
					{
						MvAddNum (j+8,  1, 4, i);
						MvAddStr (j+8,  7, 9, Shm_Elw_Db[0].E_MiChe[i].AccountNo);
						MvAddStr (j+8, 18, 8, Shm_Elw_Db[0].E_MiChe[i].Item_Cd);
						MvAddStr (j+8, 28, 7, Shm_Elw_Db[0].E_MiChe[i].OrderNo);
						MvAddStr (j+8, 37, 7, Shm_Elw_Db[0].E_MiChe[i].OriginalOrderNo);
						MvAddStr (j+8, 46, 1, Shm_Elw_Db[0].E_MiChe[i].PriceFlag);
						MvAddStr (j+8, 50, 2, Shm_Elw_Db[0].E_MiChe[i].TradeFlag);
						MvAddStr (j+8, 54, 5, Shm_Elw_Db[0].E_MiChe[i].Order_Cnt+3);
						MvAddStr (j+8, 61, 5, Shm_Elw_Db[0].E_MiChe[i].Order_Jan_Cnt+3);
						MvAddStr (j+8, 68, 6, Shm_Elw_Db[0].E_MiChe[i].Order_Price+3);
						MvAddStr (j+8, 76, 1, Shm_Elw_Db[0].E_MiChe[i].JumunFlag);
						MvAddStr (j+8, 79, 1, Shm_Elw_Db[0].E_MiChe[i].OrderType);
						MvAddNum (j+8, 82, 4, Shm_Elw_Db[0].E_MiChe[i].Item_Seq);
						j ++;
					}
					prt = prt + 1;
				}
			}

			if ((APTYPE == 100) && (prt >= elw_tot_mcnt))
				break;

			if ((APTYPE != 100) && (prt >= ACCNO(0,APTYPE-ELW_ACC_NO_BASE+ACC_NO_CNT).miche_cnt))
				break;
		}
	}
	else
	{
		for (i = 0; i < 20; i++)
			tot_mcnt = tot_mcnt + ACCNO(0,i).miche_cnt;

		for (i = 0; i < 10000; i ++)
		{
			if (APTYPE == 0)
			{
				if (Shm_Db[0].F_MiChe[i].Jan_Cnt > 0)
				{
					if (prt >= 0 && prt < 28)
					{
						MvAddNum (j+8,  1, 4, i);
						MvAddStr (j+8,  7, 9, Shm_Db[0].F_MiChe[i].AccountNo);
						MvAddStr (j+8, 18, 8, Shm_Db[0].F_MiChe[i].Item_Cd);
						MvAddStr (j+8, 28, 7, Shm_Db[0].F_MiChe[i].OrderNo);
						MvAddStr (j+8, 37, 7, Shm_Db[0].F_MiChe[i].OriginalOrderNo);
						MvAddStr (j+8, 46, 1, Shm_Db[0].F_MiChe[i].PriceFlag);
						MvAddStr (j+8, 50, 2, Shm_Db[0].F_MiChe[i].TradeFlag);
						MvAddStr (j+8, 54, 5, Shm_Db[0].F_MiChe[i].Order_Cnt+3);
						MvAddStr (j+8, 61, 5, Shm_Db[0].F_MiChe[i].Order_Jan_Cnt+3);
						MvAddStr (j+8, 68, 6, Shm_Db[0].F_MiChe[i].Order_Price+3);
						MvAddStr (j+8, 76, 1, Shm_Db[0].F_MiChe[i].JumunFlag);
						MvAddStr (j+8, 79, 1, Shm_Db[0].F_MiChe[i].OrderType);
						j ++;
					}
					prt = prt + 1;
				}
			}
			else
			{
				if ((Shm_Db[0].F_MiChe[i].Jan_Cnt > 0) &&
					(memcmp (Shm_Db[0].F_MiChe[i].AccountNo, ACCNO(0,APTYPE-ACC_NO_BASE).acc_no, 9) == 0))
				{
					if (prt >= 0 && prt < 28)
					{
						MvAddNum (j+8,  1, 4, i);
						MvAddStr (j+8,  7, 9, Shm_Db[0].F_MiChe[i].AccountNo);
						MvAddStr (j+8, 18, 8, Shm_Db[0].F_MiChe[i].Item_Cd);
						MvAddStr (j+8, 28, 7, Shm_Db[0].F_MiChe[i].OrderNo);
						MvAddStr (j+8, 37, 7, Shm_Db[0].F_MiChe[i].OriginalOrderNo);
						MvAddStr (j+8, 46, 1, Shm_Db[0].F_MiChe[i].PriceFlag);
						MvAddStr (j+8, 50, 2, Shm_Db[0].F_MiChe[i].TradeFlag);
						MvAddStr (j+8, 54, 5, Shm_Db[0].F_MiChe[i].Order_Cnt+3);
						MvAddStr (j+8, 61, 5, Shm_Db[0].F_MiChe[i].Order_Jan_Cnt+3);
						MvAddStr (j+8, 68, 6, Shm_Db[0].F_MiChe[i].Order_Price+3);
						MvAddStr (j+8, 76, 1, Shm_Db[0].F_MiChe[i].JumunFlag);
						MvAddStr (j+8, 79, 1, Shm_Db[0].F_MiChe[i].OrderType);
						j ++;
					}
					prt = prt + 1;
				}
			}

			if ((APTYPE == 0) && (prt >= tot_mcnt))
				break;

			if ((APTYPE != 0) && (prt >= ACCNO(0,APTYPE-ACC_NO_BASE).miche_cnt))
				break;
		}
	}

	MvAddNum (4, 10, 4, prt);

	return;
}

/*************************************************************************
	End of Program (py_2080_cm.c)
*************************************************************************/
