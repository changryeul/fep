/*------------------------------------------------------------------------
#   Module  : 계좌 손익, 수수료, 평가금액
#   File	: py_2050_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			ApType;
char		FO_Flag, ItemCd[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Sise (void);
static void		Disp_Sise (void);
void			Change_Acc_Rtn (void);

/*----------------------------------------------------------------------*/
void	py_2050_cm (void)
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
	Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg ("Enter:Retry C:Change I:Info Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

		switch (getch ())
		{
			case	'i':
			case	'I':										/* Info	*/
				Info_Sise ();
			 	break;
			case    'c':
			case    'C':                                    /* Change   */
				Change_Acc_Rtn ();
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
				refresh ();
				Disp_Sise ();
				Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
				break;
			default:
				clear ();
				Disp_Scr ();
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
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
	int		i;

	Disp_Title ();
	attron (A_BOLD);
	mvaddstr (0, 18, "[2050] 계좌 - 손익 (ACCNO)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 0, "ApType    ");
	Draw_Underline (2, 7, 2);

	mvaddstr (4, 0, "계좌설명 [        ] 계좌번호 [         ] RISK_FLAG [ ]  [ ]");

	mvaddstr (6, 0, "보유종목[선물] 현재가격(9(3)V9(3))");
	for (i = 0; i < 4; i++)
	{
		mvaddstr (i+ 7, 0,
"코드[        ] 평가액[           ] 수량[      ] 매입가[   .   ] 현재가[   .   ]");
	}

	mvaddstr (12, 0, "보유종목[옵션] 현재가격(9(3)V9(3))");
	for (i = 0; i < 10; i++)
	{
		mvaddstr (i+13, 0,
"코드[        ] 평가액[           ] 수량[      ] 매입가[   .   ] 현재가[   .   ]");
	}

	mvaddstr (24, 0, 
	"비밀번호 [        ] 사용자권한 [   ] 사용IP [   .   .   .   ]");

	mvaddstr (25, 0, 
	"계좌한도 [           ] 선물최대수량 [       ] 옵션최대수량 [       ]");

	mvaddstr (26, 0, 
	"자동번호 [       ] 자동번호B [       ] Cli번호 [       ]");

	mvaddstr (28, 0,
	"실질손익 [           ]");

	mvaddstr (29, 0, 
	"매매손익 [           ] 수수료 [           ] 평가손익 [           ]");

	mvaddstr (31, 0,
	"자동총가능전략 [  ][  ]");

	mvaddstr (32, 0,
	"가능전략 [                    ]");
	mvaddstr (33, 0,
	"사용전략 [                    ]");

    mvaddstr (35, 0,
	"매도건수 [         ][         ]  매도금액 [           ][           ]");
    mvaddstr (36, 0,
	"매수건수 [         ][         ]  매수금액 [           ][           ]");

	return;
}

/*----------------------------------------------------------------------*/
void    Change_Acc_Rtn (void)
/*----------------------------------------------------------------------*/
{
    char    buf[20];
    int     i, rt, item, x, y, l, cnt;
    u_long  ul;

    cnt = 0;

    attron (A_REVERSE);
    mvaddstr ( 4,  8,  "1");
    mvaddstr ( 4, 28,  "2");
    mvaddstr ( 4, 50,  "3");
    mvaddstr (24,  8,  "4");
    mvaddstr (24, 30,  "5");
    mvaddstr (24, 43,  "6");
    mvaddstr (25,  8,  "7");
    mvaddstr (25, 35,  "8");
    mvaddstr (25, 58,  "9");
    mvaddstr (26,  8, "10");
    mvaddstr (26, 28, "11");
    mvaddstr (26, 46, "12");
    mvaddstr (29,  8, "13");
    mvaddstr (29, 29, "14");
    mvaddstr (29, 52, "15");
    mvaddstr (31, 14, "16");
    mvaddstr (32,  8, "17");
/* MM 200809 */
    mvaddstr ( 4, 54, "18");
    mvaddstr (35,  8, "19");
    mvaddstr (35,  41,"20");
    mvaddstr (36,  8, "21");
    mvaddstr (36,  41,"22");

    attroff (A_REVERSE);

    Disp_Msg ("Select the item to change. (Esc:Cancel)");
    while (1)
    {
        memset (&buf, 0, sizeof buf);
        Draw_Underline (Rows - 1, 44, 2);
        rt = Get_String (Rows - 1, 44, 2, buf);
        if (rt == -1)                                           /* skip */
            Disp_Msg ("Select the item to change. (Esc:Cancel)");
        else if (rt == -3)                      /* Over the input limit */
            Disp_Msg ("Input less than 3 characters. (Esc:Cancel)");
        else if (rt == -4)                                  /* Space    */
            Disp_Msg ("You cannot input SPACE. (Esc:Cancel)");
        else if (rt == 1)                                       /* Esc  */
            return;
        else if (Chk_Digit (buf) == 0)
            Disp_Msg ("Numbers only. (Esc:Cancel)");
        else
        {
            item = atoi (buf);

			switch(item)
			{
				case	1:
	                x = 4;  y= 10;  l = 8;
					break;
				case	2:
                	x = 4;  y= 30;  l = 9;
					break;
				case	3:
                	x = 4;  y= 52;  l = 1;
					break;
				case	4:
                	x = 24;  y= 10;  l = 8;
					break;
				case	5:
                	x = 24;  y= 32;  l = 3;
					break;
				case	6:
                	x = 24;  y= 45;  l = 12;
					break;
				case	7:
                	x = 25;  y= 10;  l = 11;
					break;
				case	8:
                	x = 25;  y= 37;  l = 7;
					break;
				case	9:
                	x = 25;  y= 60;  l = 7;
					break;
				case	10:
                	x = 26;  y= 10;  l = 7;
					break;
				case	11:
                	x = 26;  y= 30;  l = 7;
					break;
				case	12:
                	x = 26;  y= 48;  l = 7;
					break;
				case	13:
                	x = 29;  y= 10;  l = 11;
					break;
				case	14:
                	x = 29;  y= 31;  l = 11;
					break;
				case	15:
                	x = 29;  y= 54;  l = 11;
					break;
				case	16:
                	x = 31;  y= 16;  l = 2;
					break;
				case	17:
                	x = 32;  y= 10;  l = 20;
					break;
				case	18:
                	x = 4;  y= 57;  l = 1;
					break;
				case	19:
                	x = 35;  y= 10;  l = 9;
					break;
				case	20:
                	x = 35;  y= 43;  l = 11;
					break;
				case	21:
                	x = 36;  y= 10;  l = 9;
					break;
				case	22:
                	x = 36;  y= 43;  l = 11;
					break;
				default	:
                	Disp_Msg ("Select the correct item. (Esc:Cancel)");
                	sleep (1);
                	continue;
            }
            break;
        }
    }

    Disp_Msg ("Input the value that you want to set. (Esc:Cancel)");
    while (1)
    {
        memset (&buf, 0, sizeof buf);

        Draw_Underline (x, y, l);
        rt = Get_String (x, y, l, buf);

        if (rt == -1)                                           /* skip */
        {
			Disp_Msg ("Input the value that you want to set. (Esc:Cancel)");
        }
        else if (rt == -3)                      /* Over the input limit */
			Disp_Msg ("You inputted over the limit. (Esc:Cancel)");
        else if (rt == -4)                                  /* Space    */
			Disp_Msg ("You cannot input SPACE. (Esc:Cancel)");
        else if (rt == 1)                                       /* Esc  */
			return;
        else
        {
            rt = Yes_No ("Are you sure to change?", NO);
            if (rt == YES)
            {
				switch(item)
				{
					case	1:
						memcpy(ACCNO(0,ApType).info, buf, 8);
						break;
					case	2:
						memcpy(ACCNO(0,ApType).acc_no, buf, 9);
						break;
					case	3:
						ACCNO(0,ApType).risk_flag = AtoIf(buf, 1);
						break;
					case	4:
						memcpy(ACCNO(0,ApType).login_pwd, buf, 8);
						break;
					case	5:
						memcpy(ACCNO(0,ApType).athrty, buf, 3);
						break;
					case	6:
						memcpy(ACCNO(0,ApType).ip_addr, buf, 12);
						break;
					case	7:
						ACCNO(0,ApType).acc_risk_max = atoi(buf);
						break;
					case	8:
						ACCNO(0,ApType).js_order_maxcnt = AtoIf(buf, 7);
						break;
					case	9:
						ACCNO(0,ApType).jo_order_maxcnt = AtoIf(buf, 7);
						break;
					case	10:
						ACCNO(0,ApType).js_order_no_band = AtoIf(buf, 7);
						break;
					case	11:
						ACCNO(0,ApType).js_order_no_band_b = AtoIf(buf, 7);
						break;
					case	12:
						ACCNO(0,ApType).acc_order_no = AtoIf(buf, 7);
						break;
					case	13:
						ACCNO(0,ApType).acc_real_prft = atoi(buf);
						break;
					case	14:
						ACCNO(0,ApType).acc_fee = atoi(buf);
						break;
					case	15:
						ACCNO(0,ApType).acc_ver_prft = atoi(buf);
						break;
					case	16:
						memcpy(ACCNO(0,ApType).auto_man_cnt, buf, 2);
						break;
					case	17:
						memcpy(ACCNO(0,ApType).auto_man_cnt+2, buf, 20);
						break;
					case	18:
						ACCNO(0,ApType).etc_risk_flag = AtoIf(buf, 1);
						break;
					case	19:
						ACCNO(0,ApType).medo_man_cnt = AtoIf(buf, 9);
						break;
					case	20:
						ACCNO(0,ApType).medo_man_money = AtoIf(buf, 11);
						break;
					case	21:
						ACCNO(0,ApType).mesu_man_cnt = AtoIf(buf, 9);
						break;
					case	22:
						ACCNO(0,ApType).mesu_man_money = AtoIf(buf, 11);
						break;
					default	:
						break;
				}

				Disp_Msg ("▶▶▶ Changed ◀◀◀");
				Debug (PGMLIN, "item %d changed [%s]", item, buf);
            }
			else
				Disp_Msg ("▶▶▶ Cancelled ◀◀◀");

            sleep (1);
            break;
        }
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

	Disp_Msg ("<APTYPE> 입력 (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 7, 2);
		rt = Get_String (2, 7, 2, buf);
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

			if (strlen (buf) != 2)
			{
				Disp_Msg ("ApType 입력 오류 (Esc:Cancel)");
				continue;
			}

			ApType = AtoIf(buf, 2) - 31;
			break;
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	int		i, j, b_avrp, pyprice, arp;

	G_Time ();

	attron (A_BOLD);

	/* 초기화 */
	MvAddStr (4, 10,  8, "        ");
	MvAddStr (4, 30,  9, "         ");
	MvAddStr (4, 52,  1, " ");
	MvAddStr (4, 57,  1, " ");

	MvAddStr (4, 10,  8, ACCNO(0,ApType).info);
	MvAddStr (4, 30,  9, ACCNO(0,ApType).acc_no);
	MvAddNum (4, 52,  1, ACCNO(0,ApType).risk_flag);
	MvAddNum (4, 57,  1, ACCNO(0,ApType).etc_risk_flag);

	j = 0;

	for (i = 0; i < 4; i++)
	{
		/* 초기화 */
		MvAddStr (j+7,  5, 8, "        ");
		MvAddStr (j+7, 22, 11,	"           ");
		MvAddStr (j+7, 40, 6,	"      ");
		MvAddStr (j+7, 55, 3, 	"   ");
		MvAddStr (j+7, 59, 3, 	"   ");
		MvAddStr (j+7, 71, 3, 	"   ");
		MvAddStr (j+7, 75, 3, 	"   ");

		if (Shm_Futures[i].Futures_CURR.getcnt[ApType] != 0)
		{
			b_avrp = Shm_Futures[i].Futures_CURR.get_avr_price_jisu[ApType];
			pyprice  = AtoIf(Shm_Futures[i].Futures_CURR.crprc, 5) * 10;

			MvAddStr (j+7,  5, 8, 
				Shm_Futures[i].Futures_A0.item_code+3);
			MvAddNum (j+7, 22, 11, 
				Shm_Futures[i].Futures_CURR.acc_ver_prft[ApType]);
			MvAddNum (j+7, 40, 6, 
				Shm_Futures[i].Futures_CURR.getcnt[ApType]);
			MvAddNum (j+7, 55, 3, b_avrp / 1000);
			MvAddNum (j+7, 59, 3, b_avrp % 1000);
			MvAddNum (j+7, 71, 3, pyprice / 1000);
			MvAddNum (j+7, 75, 3, pyprice % 1000);
			j ++;
		}
	}

	j = 0;

	for (i = 0; i < SHM_MAX_OPTIONS; i++)
	{
		/* 초기화 */
		MvAddStr (j+13,  5, 8, "        ");
		MvAddStr (j+13, 22, 11,	"           ");
		MvAddStr (j+13, 40, 6,	"      ");
		MvAddStr (j+13, 55, 3, 	"   ");
		MvAddStr (j+13, 59, 3, 	"   ");
		MvAddStr (j+13, 71, 3, 	"   ");
		MvAddStr (j+13, 75, 3, 	"   ");

		if (Shm_Options[i].Options_CURR.getcnt[ApType] != 0)
		{
			b_avrp = Shm_Options[i].Options_CURR.get_avr_price_jisu[ApType];
			pyprice  = AtoIf(Shm_Options[i].Options_CURR.crprc, 5) * 10;

			MvAddStr (j+13,  5, 8, 
				Shm_Options[i].Options_A0.item_code+3);
			MvAddNum (j+13, 22, 11, 
				Shm_Options[i].Options_CURR.acc_ver_prft[ApType]);
			MvAddNum (j+13, 40, 6, 
				Shm_Options[i].Options_CURR.getcnt[ApType]);
			MvAddNum (j+13, 55, 3, b_avrp / 1000);
			MvAddNum (j+13, 59, 3, b_avrp % 1000);
			MvAddNum (j+13, 71, 3, pyprice / 1000);
			MvAddNum (j+13, 75, 3, pyprice % 1000);
			j ++;
		}

		if (j >= 10)
			break;
	}

	/* 초기화 */
	MvAddStr (24, 10, 8, "        ");
	MvAddStr (24, 32, 3, "   ");
	MvAddStr (24, 45, 3, "   ");
	MvAddStr (24, 49, 3, "   ");
	MvAddStr (24, 53, 3, "   ");
	MvAddStr (24, 57, 3, "   ");

	MvAddStr (24, 10, 8, ACCNO(0,ApType).login_pwd);
	MvAddStr (24, 32, 3, ACCNO(0,ApType).athrty);
	MvAddStr (24, 45, 3, ACCNO(0,ApType).ip_addr);
	MvAddStr (24, 49, 3, ACCNO(0,ApType).ip_addr+3);
	MvAddStr (24, 53, 3, ACCNO(0,ApType).ip_addr+6);
	MvAddStr (24, 57, 3, ACCNO(0,ApType).ip_addr+9);

	/* 초기화 */
	MvAddNum (25, 10, 11, 0);
	MvAddNum (25, 37, 7, 0);
	MvAddNum (25, 60, 7, 0);

	MvAddNum (25, 10, 11, ACCNO(0,ApType).acc_risk_max);
	MvAddNum (25, 37, 7, ACCNO(0,ApType).js_order_maxcnt);
	MvAddNum (25, 60, 7, ACCNO(0,ApType).jo_order_maxcnt);

	/* 초기화 */
	MvAddNum (26, 10, 7, 0);
	MvAddNum (26, 30, 7, 0);
	MvAddNum (26, 48, 7, 0);

	MvAddNum (26, 10, 7, ACCNO(0,ApType).js_order_no_band);
	MvAddNum (26, 30, 7, ACCNO(0,ApType).js_order_no_band_b);
	MvAddNum (26, 48, 7, ACCNO(0,ApType).acc_order_no);

	/* 초기화 */
    MvAddNum (28, 10, 11, 0);
	MvAddNum (29, 10, 11, 0);
	MvAddNum (29, 31, 11, 0);
	MvAddNum (29, 54, 11, 0);

	arp = ACCNO(0,ApType).acc_real_prft;

    MvAddNum (28, 10, 11,   arp -
                            ACCNO(0,ApType).acc_fee/10 +
                            ACCNO(0,ApType).acc_ver_prft);

	MvAddNum (29, 10, 11, arp);
	MvAddNum (29, 31, 11, ACCNO(0,ApType).acc_fee);
	MvAddNum (29, 54, 11, ACCNO(0,ApType).acc_ver_prft);

	/* 자동 전체전략 */
	MvAddStr (31, 16, 2, ACCNO(0,ApType).auto_man_cnt);
	MvAddStr (31, 20, 2, ACCNO(0,ApType).auto_use_cnt);

	/* 자동 개별전략 */
	MvAddStr (32,  10, 20, ACCNO(0,ApType).auto_man_cnt+2);
	MvAddStr (33,  10, 20, ACCNO(0,ApType).auto_use_cnt+2);

	/* MM 200809 매도건수 */
    MvAddNum (35, 10, 9,   ACCNO(0,ApType).medo_man_cnt);
    MvAddNum (35, 21, 9,   ACCNO(0,ApType).medo_use_cnt);

	/* MM 200809 매도금액 */
    MvAddNum (35, 43, 11,   ACCNO(0,ApType).medo_man_money);
    MvAddNum (35, 56, 11,   ACCNO(0,ApType).medo_use_money);

	/* MM 200809 매수건수 */
    MvAddNum (36, 10, 9,   ACCNO(0,ApType).mesu_man_cnt);
    MvAddNum (36, 21, 9,   ACCNO(0,ApType).mesu_use_cnt);

	/* MM 200809 매수금액 */
    MvAddNum (36, 43, 11,   ACCNO(0,ApType).mesu_man_money);
    MvAddNum (36, 56, 11,   ACCNO(0,ApType).mesu_use_money);

	attroff (A_BOLD);

	return;
}

/*************************************************************************
	End of Program (py_2050_cm.c)
*************************************************************************/
