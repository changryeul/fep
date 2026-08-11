/*------------------------------------------------------------------------
#   Module  : 종목별 계좌정보
#   File	: py_2070_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			Arry, In_Arry, roop, FO_Flag;
char		ItemCd[15];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Sise (void);
static void		Disp_Sise (void);

/*----------------------------------------------------------------------*/
void	py_2070_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

    Sub_SHM ();
    Mem_SHM (1, 0);
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
	mvaddstr (0, 18, "[2070] 시세 - 체결 (C0)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "상품   (1:선물 2:옵션 3:ELW)");
	Draw_Underline (2, 5, 1);
	mvaddstr (3, 0, "순번");
	Draw_Underline (3, 5, 3);

	mvaddstr (3, 9, "종목코드");

	mvaddstr (3, 31, "보유건수 [  ]  현재가격 [   .   ]");

	for(i = 0; i < 16; i++)
	{
		mvaddstr (5+i, 0,
"계좌번호[         ] 평가손익[         ] 종목수량[       ] 매입단가[   .   ] RISK[ ][ ]");
	}

	for(i = 0; i < 16; i++)
	{
		mvaddstr (22+i, 0,
		"ApType[  ]  실질손익[           ]  수수료[           ]  평가손익[           ]");
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

			if ((strlen (buf) != 1) ||
				(buf[0] != '1' && buf[0] != '2' && buf[0] != '3'))
			{
				Disp_Msg ("상품구분 오류 (Esc:Cancel)");
				continue;
			}

			FO_Flag = AtoIf (buf, 1);
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
	int	s_k, i, ym, pr, pr1, pr2, arp, skip;

	G_Time ();

	if (FO_Flag == 1)										/* 선물	*/
	{
		s_k = AtoIf(Shm_Futures[0].Futures_A0.cnt,
					sizeof(Shm_Futures[0].Futures_A0.cnt));
	}
	else if (FO_Flag == 2)									/* 옵션 */
	{
		s_k = AtoIf(Shm_Options[0].Options_A0.cnt, 
					sizeof(Shm_Options[0].Options_A0.cnt));;
	}
	else													/* ELW	*/
	{
		s_k = Shm_Elw[0].total_elw_cnt;
	}

	if (roop == 1)
		if (Arry >= s_k || Arry < 0)
			Arry = 0;
		else
			Arry += 1;
	else if (roop == -1)
		if (Arry > s_k || Arry <= 0)
			Arry = s_k - 1;
		else
			Arry -= 1;
	else
		Arry = 0;

	Arry = (Arry + In_Arry) % s_k;
	if (Arry < 0)
		Arry += s_k;
	else if (Arry > s_k)
		Arry -= s_k;

	attron (A_BOLD);
	
	MvAddNum (3, 5, 3, Arry);								/* 순번 */

	if (FO_Flag == 1)	                                 	/* 선물	*/
	{
   		MvAddStr (3, 18,  8, Shm_Futures[Arry].Futures_A0.item_code+3);
   		MvAddNum (3, 41,  2, Shm_Futures[Arry].Futures_CURR.check_cnt);
		pr = AtoIf(Shm_Futures[Arry].Futures_CURR.crprc,
				sizeof(Shm_Futures[Arry].Futures_CURR.crprc));
   		MvAddNum (3, 56,  3, pr / 100);
   		MvAddNum (3, 60,  3, (pr % 100)*10);
	}
	else if (FO_Flag == 2)									/* 옵션	*/
	{
   		MvAddStr (3, 18,  8, Shm_Options[Arry].Options_A0.item_code+3);
   		MvAddNum (3, 41,  2, Shm_Options[Arry].Options_CURR.check_cnt);
		pr = AtoIf(Shm_Options[Arry].Options_CURR.crprc,
					sizeof(Shm_Options[Arry].Options_CURR.crprc));
   		MvAddNum (3, 56,  3, pr / 100);
   		MvAddNum (3, 60,  3, (pr % 100)*10);
	}
	else
	{
   		MvAddStr (3, 18, 12, Shm_Elw[Arry].Elw_Master.stock_code);
   		MvAddStr (4, 18,  9, Shm_Elw[Arry].Elw_Master.compress_code);
   		MvAddNum (3, 41,  2, Shm_Elw[Arry].Elw_Curr.check_cnt);
   		MvAddStr (3, 56,  7, Shm_Elw[Arry].Elw_Curr.current_price+2);
	}

/* MM은 안하고 10개만 한다
	for (i = 0; i < ACC_NO_CNT; i++)
*/
	skip = 0;
	for (i = 0; i < 20; i++)
	{
		if (FO_Flag == 3)
		{
			if ((ACCNO(0,i+ACC_NO_CNT).acc_no[0] == NULL) ||
				(i >= 10) )
				break;

   			MvAddStr (5+i, 9,  9, ACCNO(0,i+ACC_NO_CNT).acc_no);
		}
		else
		{
			if (i == 4 || i == 9 || i ==16 || i == 17)
			{
				skip += 1;
				continue;
			}

			if (ACCNO(0,i).acc_no[0] == NULL)
				break;

   			MvAddStr (5+i-skip, 9,  9, ACCNO(0,i).acc_no);
		}


		if (FO_Flag == 1)									/* 선물	*/
		{
   			MvAddNum (5+i-skip, 29,  9, 
				Shm_Futures[Arry].Futures_CURR.acc_ver_prft[i]);
   			MvAddNum (5+i-skip, 49,  7, 
				Shm_Futures[Arry].Futures_CURR.getcnt[i]);
			pr = Shm_Futures[Arry].Futures_CURR.get_avr_price_jisu[i];
		}
		else if (FO_Flag == 2)								/* 옵션	*/
		{
   			MvAddNum (5+i-skip, 29,  9, 
				Shm_Options[Arry].Options_CURR.acc_ver_prft[i]);
   			MvAddNum (5+i-skip, 49,  7, 
				Shm_Options[Arry].Options_CURR.getcnt[i]);
			pr = Shm_Options[Arry].Options_CURR.get_avr_price_jisu[i];

		}
		else
		{
   			MvAddNum (5+i, 29,  9, 
				Shm_Elw[Arry].Elw_Curr.acc_ver_prft[i]);
   			MvAddNum (5+i, 49,  7, 
				Shm_Elw[Arry].Elw_Curr.getcnt[i]);
		}

		if (FO_Flag != 3)
		{
   			MvAddNum (5+i-skip, 67,  3, pr / 1000); 
	   		MvAddNum (5+i-skip, 71,  3, pr % 1000); 
			MvAddNum (5+i-skip, 81,  1, ACCNO(0,i).risk_flag); 
  			MvAddNum (5+i-skip, 84,  1, ACCNO(0,i).etc_risk_flag); 

			/* 계좌정보 */
			arp = ACCNO(0,i).acc_real_prft;
			MvAddStr (22+i-skip, 7,  2, ACCNO(0,i).aptype_code);

			MvAddStr (22+i-skip, 7,  2, ACCNO(0,i).aptype_code);
			MvAddNum (22+i-skip, 21, 11, arp -
									ACCNO(0,i).acc_fee/10 +
									ACCNO(0,i).acc_ver_prft);
	
			MvAddNum (22+i-skip, 42, 11, ACCNO(0,i).acc_fee);
			MvAddNum (22+i-skip, 65, 11, ACCNO(0,i).acc_ver_prft);
		}
		else														/* ELW	*/
		{
	   		MvAddNum (5+i, 67,  7, Shm_Elw[Arry].Elw_Curr.get_avr_price_jisu[i]); 
   			MvAddNum (5+i, 81,  1, ACCNO(0,i+ACC_NO_CNT).risk_flag); 
   			MvAddNum (5+i, 84,  1, ACCNO(0,i+ACC_NO_CNT).etc_risk_flag); 

			/* 계좌정보 */
			arp = ACCNO(0,i+ACC_NO_CNT).acc_real_prft;
			MvAddStr (22+i, 7,  2, ACCNO(0,i+ACC_NO_CNT).aptype_code);

			MvAddStr (22+i, 7,  2, ACCNO(0,i+ACC_NO_CNT).aptype_code);
			MvAddNum (22+i, 21, 11, arp -
									ACCNO(0,i+ACC_NO_CNT).acc_fee/10 +
									ACCNO(0,i+ACC_NO_CNT).acc_ver_prft);
	
			MvAddNum (22+i, 42, 11, ACCNO(0,i+ACC_NO_CNT).acc_fee);
			MvAddNum (22+i, 65, 11, ACCNO(0,i+ACC_NO_CNT).acc_ver_prft);
		}
	}

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2070_cm.c)
*************************************************************************/
