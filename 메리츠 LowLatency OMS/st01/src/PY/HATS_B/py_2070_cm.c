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
int			Arry, In_Arry, roop;
char		FO_Flag, ItemCd[12];

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
	mvaddstr (2, 0, "상품   (1:선물 2:옵션)");
	Draw_Underline (2, 5, 1);
	mvaddstr (3, 0, "순번");
	Draw_Underline (3, 5, 3);

	mvaddstr (5, 0,
		"종목코드                  한글명");

	mvaddstr (7, 0,
	"보유건수 [  ]  현재가격 [   .   ]");
	mvaddstr (9, 0,
	"계좌별 평가손익/보유종목수량/평균매입단가");

	for(i = 0; i < 12; i++)
	{
		mvaddstr (10+i, 0,
"계좌번호[         ] 평가손익[         ] 종목수량[       ] 매입단가[   .   ] RISK[ ][ ]");
	}

	for(i = 0; i < 12; i++)
	{
		mvaddstr (23+i, 0,
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

			if (strlen (buf) != 1 || (buf[0] != '1' && buf[0] != '2'))
			{
				Disp_Msg ("상품구분 오류 (Esc:Cancel)");
				continue;
			}

			FO_Flag = buf[0];
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
	int	s_k, i, pr, pr1, pr2, arp;

	SIF_G7	fms_c0;
	SIO_G7	opt_c0;

	G_Time ();

	if (FO_Flag == '1')										/* 선물	*/
	{
		s_k = AtoIf(Shm_Futures[0].Futures_A0.cnt,
					sizeof(Shm_Futures[0].Futures_A0.cnt));
	}
	else
	{
		s_k = AtoIf(Shm_Options[0].Options_A0.cnt, 
					sizeof(Shm_Options[0].Options_A0.cnt));;
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

	if (FO_Flag == '1')                                 	/* 선물	*/
	{
   		MvAddStr (5, 9,  8, Shm_Futures[Arry].Futures_A0.item_code+3);
   		MvAddStr (5, 33,  30, Shm_Futures[Arry].Futures_A0.kor_nm);
   		MvAddNum (7, 10,  2, Shm_Futures[Arry].Futures_CURR.check_cnt);
		pr = AtoIf(Shm_Futures[Arry].Futures_CURR.crprc,
				sizeof(Shm_Futures[Arry].Futures_CURR.crprc));
   		MvAddNum (7, 25,  3, pr / 100);
   		MvAddNum (7, 29,  3, (pr % 100)*10);
	}
	else
	{
   		MvAddStr (5, 9,  8, Shm_Options[Arry].Options_A0.item_code+3);
   		MvAddStr (5, 33,  30, Shm_Options[Arry].Options_A0.kor_nm);
   		MvAddNum (7, 10,  2, Shm_Options[Arry].Options_CURR.check_cnt);
		pr = AtoIf(Shm_Options[Arry].Options_CURR.crprc,
					sizeof(Shm_Options[Arry].Options_CURR.crprc));
   		MvAddNum (7, 25,  3, pr / 100);
   		MvAddNum (7, 29,  3, (pr % 100)*10);
	}

/* MM은 안하고 10개만 한다
	for (i = 0; i < ACC_NO_CNT; i++)
*/
	for (i = 0; i < 10; i++)
	{
		if (ACCNO(0,i).acc_no[0] == NULL)
			break;

   		MvAddStr (10+i, 9,  9, ACCNO(0,i).acc_no);

		if (FO_Flag == '1')									/* 선물	*/
		{
   			MvAddNum (10+i, 29,  9, 
				Shm_Futures[Arry].Futures_CURR.acc_ver_prft[i]);
   			MvAddNum (10+i, 49,  7, 
				Shm_Futures[Arry].Futures_CURR.getcnt[i]);
			pr = Shm_Futures[Arry].Futures_CURR.get_avr_price_jisu[i];
		}
		else 												/* 옵션	*/
		{
   			MvAddNum (10+i, 29,  9, 
				Shm_Options[Arry].Options_CURR.acc_ver_prft[i]);
   			MvAddNum (10+i, 49,  7, 
				Shm_Options[Arry].Options_CURR.getcnt[i]);
			pr = Shm_Options[Arry].Options_CURR.get_avr_price_jisu[i];
		}

   		MvAddNum (10+i, 67,  3, pr / 1000); 
   		MvAddNum (10+i, 71,  3, pr % 1000); 
   		MvAddNum (10+i, 81,  1, ACCNO(0,i).risk_flag); 
   		MvAddNum (10+i, 84,  1, ACCNO(0,i).etc_risk_flag); 

		/* 계좌정보 */
		arp = ACCNO(0,i).acc_real_prft;

/*
		MvAddNum (23+i,  7,  2, i+31);
		aptype_code
*/
		MvAddStr (23+i, 7,  2, ACCNO(0,i).aptype_code);

		MvAddStr (23+i, 7,  2, ACCNO(0,i).aptype_code);
		MvAddNum (23+i, 21, 11, arp -
								ACCNO(0,i).acc_fee/10 +
								ACCNO(0,i).acc_ver_prft);

		MvAddNum (23+i, 42, 11, ACCNO(0,i).acc_fee);
		MvAddNum (23+i, 65, 11, ACCNO(0,i).acc_ver_prft);
	}

/*
memcpy(ACCNO(0,6).aptype_code, "37", 2);
*/

	/* MM 개별표시 */
   	MvAddStr (10+10, 9,  9, ACCNO(0,11).acc_no);
   	MvAddStr (10+11, 9,  9, ACCNO(0,15).acc_no);

	if (FO_Flag == '1')									/* 선물	*/
	{
   		MvAddNum (10+10, 29,  9, 
			Shm_Futures[Arry].Futures_CURR.acc_ver_prft[11]);
   		MvAddNum (10+10, 49,  7, 
			Shm_Futures[Arry].Futures_CURR.getcnt[11]);
		pr1 = Shm_Futures[Arry].Futures_CURR.get_avr_price_jisu[11];

   		MvAddNum (10+11, 29,  9, 
			Shm_Futures[Arry].Futures_CURR.acc_ver_prft[15]);
   		MvAddNum (10+11, 49,  7, 
			Shm_Futures[Arry].Futures_CURR.getcnt[15]);
		pr2 = Shm_Futures[Arry].Futures_CURR.get_avr_price_jisu[15];
	}
	else 												/* 옵션	*/
	{
   		MvAddNum (10+10, 29,  9, 
			Shm_Options[Arry].Options_CURR.acc_ver_prft[11]);
   		MvAddNum (10+10, 49,  7, 
			Shm_Options[Arry].Options_CURR.getcnt[11]);
		pr1 = Shm_Options[Arry].Options_CURR.get_avr_price_jisu[11];

   		MvAddNum (10+11, 29,  9, 
			Shm_Options[Arry].Options_CURR.acc_ver_prft[15]);
   		MvAddNum (10+11, 49,  7, 
			Shm_Options[Arry].Options_CURR.getcnt[15]);
		pr2 = Shm_Options[Arry].Options_CURR.get_avr_price_jisu[15];
	}

   	MvAddNum (10+10, 67,  3, pr1 / 1000); 
   	MvAddNum (10+10, 71,  3, pr1 % 1000); 
   	MvAddNum (10+10, 81,  1, ACCNO(0,11).risk_flag); 
   	MvAddNum (10+10, 84,  1, ACCNO(0,11).etc_risk_flag); 

   	MvAddNum (10+11, 67,  3, pr2 / 1000); 
   	MvAddNum (10+11, 71,  3, pr2 % 1000); 
   	MvAddNum (10+11, 81,  1, ACCNO(0,15).risk_flag); 
   	MvAddNum (10+11, 84,  1, ACCNO(0,15).etc_risk_flag); 

	/* 계좌정보 */
	arp = ACCNO(0,11).acc_real_prft;

	MvAddStr (23+10, 7,  2, ACCNO(0,11).aptype_code);

	MvAddStr (23+10, 7,  2, ACCNO(0,11).aptype_code);
	MvAddNum (23+10, 21, 11, arp -
							ACCNO(0,11).acc_fee/10 +
							ACCNO(0,11).acc_ver_prft);

	MvAddNum (23+10, 42, 11, ACCNO(0,11).acc_fee);
	MvAddNum (23+10, 65, 11, ACCNO(0,11).acc_ver_prft);

	arp = ACCNO(0,15).acc_real_prft;

	MvAddStr (23+11, 7,  2, ACCNO(0,15).aptype_code);

	MvAddStr (23+11, 7,  2, ACCNO(0,15).aptype_code);
	MvAddNum (23+11, 21, 11, arp -
							ACCNO(0,15).acc_fee/10 +
							ACCNO(0,15).acc_ver_prft);

	MvAddNum (23+11, 42, 11, ACCNO(0,15).acc_fee);
	MvAddNum (23+11, 65, 11, ACCNO(0,15).acc_ver_prft);

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2070_cm.c)
*************************************************************************/
