/*------------------------------------------------------------------------
#   Module  : 계좌 손익, 수수료, 평가금액
#   File	: py_2090_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
int			ApType, W_Key, St_No;
char		FO_Flag, ItemCd[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void     Info_Sise (void);
static void		Disp_Sise (void);
void			Change_Acc_Rtn_09 (void);

/*----------------------------------------------------------------------*/
void	py_2090_cm (void)
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
	Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg ("Enter:Retry C:Change I:Info Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

		switch (getch ())
		{
            case    'i':
            case    'I':                                        /* Info */
                Info_Sise ();
                break;
			case    'c':
			case    'C':                                    /* Change   */
				Change_Acc_Rtn_09 ();
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
	mvaddstr (0, 18, "[2090] 서버MM전략조회");
	attroff (A_BOLD);
    Draw_Line (1, 0, 79);
    mvaddstr (2, 0, "전략번호   (1/4/6/8)");
    Draw_Underline (2, 9, 1);
    mvaddstr (2, 21, "ApType    ");
    Draw_Underline (2, 28, 2);

	mvaddstr (3, 0, "ToTal [ ]");
	mvaddstr (4, 0, "AUTO [ ][ ][ ][ ] Recv_Seq [   ]총한도금액  [          ] 옵션주문현황 [  ]");
	mvaddstr (5, 0, "선물단위수량 [  ] 옵션종목수량 [  ]");

	mvaddstr (6, 0, "보유종목[선물] 현재가격(9(3)V9(3))");
	mvaddstr (7, 0,
"코드[        ] 평가액[           ] 수량[      ] 매/주/현[   .  ][   .  ][   .  ]");
	mvaddstr (8, 0, 
"End_Flag[ ] 총수량[      ] 주문금액[          ] 주문번호[       ][       ]");

	mvaddstr (10, 0, "보유종목[옵션] 현재가격(9(3)V9(3))");

	for (i = 0; i < 8; i++)
	{
		mvaddstr (i*2+11, 0,
"코드[        ] 평가액[           ] 수량[      ] 매/주/현[   .  ][   .  ][   .  ]");
		mvaddstr (i*2+12, 0, 
"End_Flag[ ] 총수량[      ] 주문금액[          ] 주문번호[       ][       ]");
	}

	mvaddstr (28, 0, "보유건수 [    ]");

	for (i = 0; i < 8; i++)
	{
		mvaddstr (i+29, 0,
"[    ][         ][        ][       ][       ][ ] [  ][        ][        ][         ][ ][ ]");
	}

	return;
}

/*----------------------------------------------------------------------*/
static void     Info_Sise (void)
/*----------------------------------------------------------------------*/
{
    int     i, rt;
    char    buf[80];

    Clear_Lines (2, Rows - 2);
    Disp_Scr ();

    Disp_Msg ("<전략번호> 입력 (Esc:Cancel)");
    while (1)
    {
        memset (&buf, 0, sizeof buf);
        Draw_Underline (2, 9, 1);
        rt = Get_String (2, 9, 1, buf);
        if (rt == -1)                                           /* skip */
            Disp_Msg ("<ApType> 입력 (Esc:Cancel)");
        else if (rt == -4)                                  /* Space    */
            Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
        else if (rt == 1)                                       /* Esc  */
            return;
        else
        {
            attron (A_UNDERLINE);
            mvaddstr (2, 9, buf);
            attroff (A_UNDERLINE);

			if ((strlen (buf) != 1) ||
				(buf[0] != '1' && buf[0] != '4' && buf[0] != '6' && buf[0] != '8'))
            {
                Disp_Msg ("전략번호 입력오류 (Esc:Cancel)");
                continue;
            }

            St_No = AtoIf(buf, 1);
            break;
		}
	}

    Disp_Msg ("<APTYPE> 입력 (Esc:Cancel)");
    while (1)
    {
        memset (&buf, 0, sizeof buf);
        Draw_Underline (2, 28, 2);
        rt = Get_String (2, 28, 2, buf);
        if (rt == -1)                                           /* skip */
            Disp_Msg ("<ApType> 입력 (Esc:Cancel)");
        else if (rt == -4)                                  /* Space    */
            Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
        else if (rt == 1)                                       /* Esc  */
            return;
        else
        {
            attron (A_UNDERLINE);
            mvaddstr (2, 28, buf);
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
void    Change_Acc_Rtn_09 (void)
/*----------------------------------------------------------------------*/
{
    char    buf[20];
    int     i, rt, item, x, y, l, cnt;
    u_long  ul;

    cnt = 0;

    attron (A_REVERSE);
    mvaddstr ( 3,  5,  "0");
    mvaddstr ( 4,  4,  "1");
    mvaddstr ( 4, 23,  "2");
    mvaddstr ( 4, 40,  "3");
    mvaddstr ( 4, 66,  "4");
    mvaddstr ( 5, 12,  "5");
    mvaddstr ( 5, 30,  "6");
    mvaddstr ( 8,  7,  "7");
    mvaddstr (12,  7,  "8");
    mvaddstr (14,  7,  "9");
    mvaddstr (16,  6, "10");
    mvaddstr (18,  6, "11");
    mvaddstr (20,  6, "12");
    mvaddstr (22,  6, "13");
    mvaddstr (24,  6, "14");
    mvaddstr (26,  6, "15");
    mvaddstr ( 4,  7, "20");
    mvaddstr ( 4, 10, "21");
    mvaddstr ( 4, 13, "22");

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
				case	0:
	                x = 3;  y=  7;  l = 1;
					break;
				case	1:
	                x = 4;  y=  6;  l = 1;
					break;
				case	2:
                	x = 4;  y= 25;  l = 2;
					break;
				case	3:
                	x = 4;  y= 42;  l = 10;
					break;
				case	4:
                	x = 4;  y= 68;  l = 2;
					break;
				case	5:
                	x = 5;  y= 14;  l = 2;
					break;
				case	6:
                	x = 5;  y= 32;  l = 2;
					break;
				case	7:
                	x = 8;  y= 9;  l = 1;
					break;
				case	8:
                	x = 12;  y= 9;  l = 1;
					break;
				case	9:
                	x = 14;  y= 9;  l = 1;
					break;
				case	10:
                	x = 16;  y= 9;  l = 1;
					break;
				case	11:
                	x = 18;  y= 9;  l = 1;
					break;
				case	12:
                	x = 20;  y= 9;  l = 1;
					break;
				case	13:
                	x = 22;  y= 9;  l = 1;
					break;
				case	14:
                	x = 24;  y= 9;  l = 1;
					break;
				case	15:
                	x = 26;  y= 9;  l = 1;
					break;
				case	20:
	                x = 4;  y=  9;  l = 1;
					break;
				case	21:
	                x = 4;  y= 12;  l = 1;
					break;
				case	22:
	                x = 4;  y= 15;  l = 1;
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
				if (St_No == 1)
				{
					switch(item)
					{
						case	0:
							Shm_Db[0].auto_run_cnt = AtoIf(buf, 1);
							break;
						case	1:
							Shm_Db[0].Ju_S_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	2:
							Shm_Db[0].recv_seq = AtoIf(buf, 2);
							break;
						case	5:
							Shm_Db[0].Ju_S_History[ApType].sub_set[0].sub_auto_run = AtoIf(buf, 2);
							break;
						case	6:
							Shm_Db[0].Ju_S_History[ApType].sub_set[1].sub_auto_run = AtoIf(buf, 2);
							break;
/*
						case	7:
							break;
*/
						case	8:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[0].ju_set[0].End_Flag, buf, 1);
							break;
						case	9:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[0].ju_set[1].End_Flag, buf, 1);
							break;
						case	10:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[0].ju_set[2].End_Flag, buf, 1);
							break;
						case	11:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[1].ju_set[0].End_Flag, buf, 1);
							break;
						case	12:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[1].ju_set[1].End_Flag, buf, 1);
							break;
						case	13:
							memcpy(Shm_Db[0].Ju_S_History[ApType].sub_set[1].ju_set[2].End_Flag, buf, 1);
							break;
						case	14:
							break;
						case	15:
							break;
						case	20:
							Shm_Db[0].Ju_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	21:
							Shm_Db[0].Ju_O_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	22:
							Shm_Db[0].Ju_A_History[ApType].auto_run = AtoIf(buf, 1);
							break;

						default	:
							break;
					}
				}
				if (St_No == 4)
				{
					switch(item)
					{
						case	0:
							Shm_Db[0].auto_run_cnt = AtoIf(buf, 1);
							break;
						case	1:
							Shm_Db[0].Ju_S_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	2:
							Shm_Db[0].recv_seq = AtoIf(buf, 2);
							break;
						case	3:
							Shm_Db[0].Ju_History[ApType].tot_money = AtoIf(buf, 10);
							break;
						case	4:
							Shm_Db[0].Ju_History[ApType].curr_op_or = AtoIf(buf, 2);
							break;
						case	5:
							Shm_Db[0].Ju_History[ApType].fu_dan_cnt = AtoIf(buf, 2);
							break;
						case	6:
							Shm_Db[0].Ju_History[ApType].tot_op_cnt = AtoIf(buf, 2);
							break;
						case	7:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[19].End_Flag, buf, 1);
							break;
						case	8:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[0].End_Flag, buf, 1);
							break;
						case	9:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[1].End_Flag, buf, 1);
							break;
						case	10:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[2].End_Flag, buf, 1);
							break;
						case	11:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[3].End_Flag, buf, 1);
							break;
						case	12:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[4].End_Flag, buf, 1);
							break;
						case	13:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[5].End_Flag, buf, 1);
							break;
						case	14:
						memcpy(Shm_Db[0].Ju_History[ApType].apno[6].End_Flag, buf, 1);
							break;
						case	15:
							memcpy(Shm_Db[0].Ju_History[ApType].apno[7].End_Flag, buf, 1);
							break;
						case	20:
							Shm_Db[0].Ju_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	21:
							Shm_Db[0].Ju_O_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	22:
							Shm_Db[0].Ju_A_History[ApType].auto_run = AtoIf(buf, 1);
							break;

						default	:
							break;
					}
				}
				else if (St_No == 6)
				{
					switch(item)
					{
						case	0:
							Shm_Db[0].auto_run_cnt = AtoIf(buf, 1);
							break;
						case	1:
							Shm_Db[0].Ju_S_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	2:
							Shm_Db[0].recv_seq = AtoIf(buf, 2);
							break;
						case	3:
							Shm_Db[0].Ju_O_History[ApType].tot_money = AtoIf(buf, 10);
							break;
						case	4:
							Shm_Db[0].Ju_O_History[ApType].curr_op_or = AtoIf(buf, 2);
							break;
						case	5:
							Shm_Db[0].Ju_O_History[ApType].fu_dan_cnt = AtoIf(buf, 2);
							break;
						case	6:
							Shm_Db[0].Ju_O_History[ApType].tot_op_cnt = AtoIf(buf, 2);
							break;
						case	7:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[19].End_Flag, buf, 1);
							break;
						case	8:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[0].End_Flag, buf, 1);
							break;
						case	9:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[1].End_Flag, buf, 1);
							break;
						case	10:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[2].End_Flag, buf, 1);
							break;
						case	11:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[3].End_Flag, buf, 1);
							break;
						case	12:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[4].End_Flag, buf, 1);
							break;
						case	13:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[5].End_Flag, buf, 1);
							break;
						case	14:
						memcpy(Shm_Db[0].Ju_O_History[ApType].apno[6].End_Flag, buf, 1);
							break;
						case	15:
							memcpy(Shm_Db[0].Ju_O_History[ApType].apno[7].End_Flag, buf, 1);
							break;
						case	20:
							Shm_Db[0].Ju_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	21:
							Shm_Db[0].Ju_O_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	22:
							Shm_Db[0].Ju_A_History[ApType].auto_run = AtoIf(buf, 1);
							break;

						default	:
							break;
					}
				}
				else if (St_No == 8)
				{
					switch(item)
					{
						case	0:
							Shm_Db[0].auto_run_cnt = AtoIf(buf, 1);
							break;
						case	1:
							Shm_Db[0].Ju_S_History[ApType].auto_run = AtoIf(buf, 1);
							break;
						case	2:
							Shm_Db[0].recv_seq = AtoIf(buf, 2);
							break;
						case	22:
							Shm_Db[0].Ju_A_History[ApType].auto_run = AtoIf(buf, 1);
							break;

						default	:
							break;
					}
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
static void		Disp_Sise (void)
/*----------------------------------------------------------------------*/
{
	int		i, prt, j, m, v, a01, b01;

	G_Time ();

	/* 초기화 */
	MvAddStr (3,  7,  1, " ");
	MvAddStr (4,  6,  1, " ");
	MvAddStr (4,  9,  1, " ");
	MvAddStr (4, 12,  1, " ");
	MvAddStr (4, 15,  1, " ");
	MvAddStr (4, 28,  2, "  ");
	MvAddStr (4, 45, 10, "          ");
	MvAddStr (4, 71,  2, "  ");
	MvAddStr (5, 14,  2, "  ");
	MvAddStr (5, 32,  2, "  ");

	MvAddNum (3,  7,  1, Shm_Db[0].auto_run_cnt);
	MvAddNum (4,  6,  1, Shm_Db[0].Ju_S_History[ApType].auto_run);
	MvAddNum (4,  9,  1, Shm_Db[0].Ju_History[ApType].auto_run);
	MvAddNum (4, 12,  1, Shm_Db[0].Ju_O_History[ApType].auto_run);
	MvAddNum (4, 15,  1, Shm_Db[0].Ju_A_History[ApType].auto_run);
	MvAddNum (4, 28,  3, Shm_Db[0].recv_seq);
	if (St_No == 1)
	{
		MvAddNum (5, 14,  2, Shm_Db[0].Ju_S_History[ApType].sub_set[0].sub_auto_run);
		MvAddNum (5, 32,  2, Shm_Db[0].Ju_S_History[ApType].sub_set[1].sub_auto_run);
	}
	else if (St_No == 4)
	{
		MvAddNum (4, 45, 10, Shm_Db[0].Ju_History[ApType].tot_money);
		MvAddNum (4, 71,  2, Shm_Db[0].Ju_History[ApType].curr_op_or);
		MvAddNum (5, 14,  2, Shm_Db[0].Ju_History[ApType].fu_dan_cnt);
		MvAddNum (5, 32,  2, Shm_Db[0].Ju_History[ApType].tot_op_cnt);
	}
	else if (St_No == 6)
	{
		MvAddNum (4, 45, 10, Shm_Db[0].Ju_O_History[ApType].tot_money);
		MvAddNum (4, 71,  2, Shm_Db[0].Ju_O_History[ApType].curr_op_or);
		MvAddNum (5, 14,  2, Shm_Db[0].Ju_O_History[ApType].fu_dan_cnt);
		MvAddNum (5, 32,  2, Shm_Db[0].Ju_O_History[ApType].tot_op_cnt);
	}
	else if (St_No == 8)
	{
		MvAddNum (4, 45, 10, Shm_Db[0].Ju_A_History[ApType].send_cnt);
		MvAddNum (4, 71,  2, Shm_Db[0].Ju_A_History[ApType].curr_s_cnt);
		MvAddNum (5, 14,  2, Shm_Db[0].Ju_A_History[ApType].ju_stat);
		MvAddNum (5, 32,  2, Shm_Db[0].Ju_A_History[ApType].che_cnt);
	}

	/* 선물정보 초기화 */
	MvAddStr (7,  5,  8, "        ");
	MvAddStr (7, 22, 10, "          ");
	MvAddStr (7, 40,  6, "      ");
	MvAddStr (7, 57,  3, "   ");
	MvAddStr (7, 61,  2, "  ");
	MvAddStr (7, 65,  3, "   ");
	MvAddStr (7, 69,  2, "  ");
	MvAddStr (7, 73,  3, "   ");
	MvAddStr (7, 77,  2, "  ");
	MvAddStr (8,  9,  1, " ");
	MvAddStr (8, 19,  6, "      ");
	MvAddStr (8, 36, 10, "          ");
	MvAddStr (8, 57,  7, "       ");
	MvAddStr (8, 66,  7, "       ");

	if (St_No == 4)
	{
		/* 종목정보 */
		MvAddStr (7,  5,  8, Shm_Db[0].Ju_History[ApType].apno[19].Item_Cd);
		/* 평가액 */
		MvAddNum (7, 22, 11,
		Shm_Db[0].Ju_History[ApType].apno[19].Che_Gum -
		  (AtoIf (Shm_Futures[0].Futures_CURR.crprc, 5) * Shm_Db[0].Ju_History[ApType].apno[19].Nu_Che_Cnt));
		/* 수량 */
		MvAddNum (7, 40,  6, Shm_Db[0].Ju_History[ApType].apno[19].Nu_Che_Cnt);

		/* 매입가(지수) */
		MvAddNum (7, 57,  3,
			(Shm_Db[0].Ju_History[ApType].apno[19].Che_Gum / 5 / Shm_Db[0].Ju_History[ApType].apno[19].Nu_Che_Cnt) / 100);
		/* 매입가(소수) */
		MvAddNum (7, 61,  2,
			(Shm_Db[0].Ju_History[ApType].apno[19].Che_Gum / Shm_Db[0].Ju_History[ApType].apno[19].Nu_Che_Cnt) % 100);
		/* 주문가(지수) */
		MvAddStr (7, 65,  3, Shm_Db[0].Ju_History[ApType].apno[19].Order_Price+3);
		/* 주문가(소수) */
		MvAddStr (7, 69,  2, Shm_Db[0].Ju_History[ApType].apno[19].Order_Price+7);
		/* 현재값(지수) */
		MvAddNum (7, 73,  3, AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5) / 100);
		/* 현재값(소수) */
		MvAddNum (7, 77,  2, AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5) % 100);

		/* End_Flag */
		MvAddStr (8,  9,  1, Shm_Db[0].Ju_History[ApType].apno[19].End_Flag);
		/* 총수량 */
		MvAddNum (8, 19,  6, Shm_Db[0].Ju_History[ApType].apno[19].Tot_Order_Cnt);
		/* 주문금액 */
		MvAddNum (8, 36, 10, Shm_Db[0].Ju_History[ApType].apno[19].Ju_Gum);
		/* 주문번호 */
		MvAddStr (8, 57,  7, Shm_Db[0].Ju_History[ApType].apno[19].OrderNo);
		/* 원주문번호 */
		MvAddStr (8, 66,  7, Shm_Db[0].Ju_History[ApType].apno[19].OriginalOrderNo);
	}
	else if (St_No == 6)
	{
		/* 종목정보 */
		MvAddStr (7,  5,  8, Shm_Db[0].Ju_O_History[ApType].apno[19].Item_Cd);
		/* 평가액 */
		MvAddNum (7, 22, 11,
		Shm_Db[0].Ju_O_History[ApType].apno[19].Che_Gum -
		  (AtoIf (Shm_Futures[0].Futures_CURR.crprc, 5) * Shm_Db[0].Ju_O_History[ApType].apno[19].Nu_Che_Cnt));
		/* 수량 */
		MvAddNum (7, 40,  6, Shm_Db[0].Ju_O_History[ApType].apno[19].Nu_Che_Cnt);

		/* 매입가(지수) */
		MvAddNum (7, 57,  3,
			(Shm_Db[0].Ju_O_History[ApType].apno[19].Che_Gum / Shm_Db[0].Ju_O_History[ApType].apno[19].Nu_Che_Cnt) / 100);
		/* 매입가(소수) */
		MvAddNum (7, 61,  2,
			(Shm_Db[0].Ju_O_History[ApType].apno[19].Che_Gum / Shm_Db[0].Ju_O_History[ApType].apno[19].Nu_Che_Cnt) % 100);
		/* 주문가(지수) */
		MvAddStr (7, 65,  3, Shm_Db[0].Ju_O_History[ApType].apno[19].Order_Price+3);
		/* 주문가(소수) */
		MvAddStr (7, 69,  2, Shm_Db[0].Ju_O_History[ApType].apno[19].Order_Price+7);
		/* 현재값(지수) */
		MvAddNum (7, 73,  3, AtoIf(Shm_Options[Shm_Db[0].Ju_O_History[ApType].apno[19].Jong_Seq].Options_CURR.crprc, 5) / 100);
		/* 현재값(소수) */
		MvAddNum (7, 77,  2, AtoIf(Shm_Options[Shm_Db[0].Ju_O_History[ApType].apno[19].Jong_Seq].Options_CURR.crprc, 5) % 100);

		/* End_Flag */
		MvAddStr (8,  9,  1, Shm_Db[0].Ju_O_History[ApType].apno[19].End_Flag);
		/* 총수량 */
		MvAddNum (8, 19,  6, Shm_Db[0].Ju_O_History[ApType].apno[19].Tot_Order_Cnt);
		/* 주문금액 */
		MvAddNum (8, 36, 10, Shm_Db[0].Ju_O_History[ApType].apno[19].Ju_Gum);
		/* 주문번호 */
		MvAddStr (8, 57,  7, Shm_Db[0].Ju_O_History[ApType].apno[19].OrderNo);
		/* 원주문번호 */
		MvAddStr (8, 66,  7, Shm_Db[0].Ju_O_History[ApType].apno[19].OriginalOrderNo);
	}

	a01 = b01 = 0;

	for (i = 0; i < 8; i++)
	{
		/* 초기화 */
		MvAddStr (i*2+11,  5,  8, "        ");
		MvAddStr (i*2+11, 22, 10, "          ");
		MvAddStr (i*2+11, 40,  6, "      ");
		MvAddStr (i*2+11, 57,  3, "   ");
		MvAddStr (i*2+11, 61,  2, "  ");
		MvAddStr (i*2+11, 65,  3, "   ");
		MvAddStr (i*2+11, 69,  2, "  ");
		MvAddStr (i*2+11, 73,  3, "   ");
		MvAddStr (i*2+11, 77,  2, "  ");

		MvAddStr (i*2+12,  9,  1, " ");
		MvAddStr (i*2+12, 19,  6, "      ");
		MvAddStr (i*2+12, 36, 10, "          ");
		MvAddStr (i*2+12, 57,  7, "       ");
		MvAddStr (i*2+12, 66,  7, "       ");

		a01 = b01 = 0;
		if (St_No == 1)
		{
			if (i == 0 || i == 1 || i == 2)
				a01 = 0;
			else if (i == 3 || i == 4 || i == 5)
				a01 = 1;
			else
				break;

			if (i == 0 || i == 3)
				b01 = 0;
			else if (i == 1 || i == 4)
				b01 = 1;
			else if (i == 2 || i == 5)
				b01 = 2;
			else
				break;

			/* 종목정보 */
			MvAddStr (i*2+11,  5,  8, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Item_Cd);
			/* 평가액 */
/*
			MvAddStr (i*2+11, 22, 10,
				(AtoIf (Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Order_Price, 9) -
				 AtoIf (Shm_Options[Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Jong_Seq].Options_CURR.crprc, 5))
			   * Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Nu_Che_Cnt);
*/
			/* 수량 */
			MvAddNum (i*2+11, 40,  6, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Nu_Che_Cnt);
			/* 주문가(지수) */
			MvAddStr (i*2+11, 65,  3, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Order_Price+3);
			/* 주문가(소수) */
			MvAddStr (i*2+11, 69,  2, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Order_Price+7);
			/* 현재값(지수) */
			MvAddNum (i*2+11, 73,  3, AtoIf(Shm_Options[Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Jong_Seq].Options_CURR.crprc, 5) / 100);
			/* 현재값(소수) */
			MvAddNum (i*2+11, 77,  2, AtoIf(Shm_Options[Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Jong_Seq].Options_CURR.crprc, 5) % 100);

			/* End_Flag */
			MvAddStr (i*2+12,  9,  1, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].End_Flag);
			/* 총수량 */
			MvAddNum (i*2+12, 19,  6, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].Tot_Order_Cnt);
			/* 주문번호 */
			MvAddStr (i*2+12, 57,  7, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].OrderNo);
			/* 원주문번호 */
			MvAddStr (i*2+12, 66,  7, Shm_Db[0].Ju_S_History[ApType].sub_set[a01].ju_set[b01].OriginalOrderNo);
		}
		else if (St_No == 4)
		{
			if (i < Shm_Db[0].Ju_History[ApType].tot_op_cnt)
			{
				/* 종목정보 */
				MvAddStr (i*2+11,  5,  8, Shm_Db[0].Ju_History[ApType].apno[i].Item_Cd);
				/* 평가액 */
				MvAddNum (i*2+11, 22, 10,
					Shm_Db[0].Ju_History[ApType].apno[i].Che_Gum -
					(AtoIf (Shm_Options[Shm_Db[0].Ju_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) * Shm_Db[0].Ju_History[ApType].apno[i].Nu_Che_Cnt));
				/* 수량 */
				MvAddNum (i*2+11, 40,  6, Shm_Db[0].Ju_History[ApType].apno[i].Nu_Che_Cnt);

				/* 매입가(지수) */
				MvAddNum (i*2+11, 57,  3,
					(Shm_Db[0].Ju_History[ApType].apno[i].Che_Gum / Shm_Db[0].Ju_History[ApType].apno[i].Nu_Che_Cnt) / 100);
				/* 매입가(소수) */
				MvAddNum (i*2+11, 61,  2,
					(Shm_Db[0].Ju_History[ApType].apno[i].Che_Gum / Shm_Db[0].Ju_History[ApType].apno[i].Nu_Che_Cnt) % 100);
				/* 주문가(지수) */
				MvAddStr (i*2+11, 65,  3, Shm_Db[0].Ju_History[ApType].apno[i].Order_Price+3);
				/* 주문가(소수) */
				MvAddStr (i*2+11, 69,  2, Shm_Db[0].Ju_History[ApType].apno[i].Order_Price+7);
				/* 현재값(지수) */
				MvAddNum (i*2+11, 73,  3, AtoIf(Shm_Options[Shm_Db[0].Ju_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) / 100);
				/* 현재값(소수) */
				MvAddNum (i*2+11, 77,  2, AtoIf(Shm_Options[Shm_Db[0].Ju_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) % 100);

				/* End_Flag */
				MvAddStr (i*2+12,  9,  1, Shm_Db[0].Ju_History[ApType].apno[i].End_Flag);
				/* 총수량 */
				MvAddNum (i*2+12, 19,  6, Shm_Db[0].Ju_History[ApType].apno[i].Tot_Order_Cnt);
				/* 주문금액 */
				MvAddNum (i*2+12, 36, 10, Shm_Db[0].Ju_History[ApType].apno[i].Ju_Gum);
				/* 주문번호 */
				MvAddStr (i*2+12, 57,  7, Shm_Db[0].Ju_History[ApType].apno[i].OrderNo);
				/* 원주문번호 */
				MvAddStr (i*2+12, 66,  7, Shm_Db[0].Ju_History[ApType].apno[i].OriginalOrderNo);
			}
		}
		else if (St_No == 6)
		{
			if (i < Shm_Db[0].Ju_O_History[ApType].tot_op_cnt + 1)
			{
				/* 종목정보 */
				MvAddStr (i*2+11,  5,  8, Shm_Db[0].Ju_O_History[ApType].apno[i].Item_Cd);
				/* 평가액 */
				MvAddNum (i*2+11, 22, 10,
					Shm_Db[0].Ju_O_History[ApType].apno[i].Che_Gum -
					(AtoIf (Shm_Options[Shm_Db[0].Ju_O_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) * Shm_Db[0].Ju_O_History[ApType].apno[i].Nu_Che_Cnt));
				/* 수량 */
				MvAddNum (i*2+11, 40,  6, Shm_Db[0].Ju_O_History[ApType].apno[i].Nu_Che_Cnt);

				/* 매입가(지수) */
				MvAddNum (i*2+11, 57,  3,
					(Shm_Db[0].Ju_O_History[ApType].apno[i].Che_Gum / Shm_Db[0].Ju_O_History[ApType].apno[i].Nu_Che_Cnt) / 100);
				/* 매입가(소수) */
				MvAddNum (i*2+11, 61,  2,
					(Shm_Db[0].Ju_O_History[ApType].apno[i].Che_Gum / Shm_Db[0].Ju_O_History[ApType].apno[i].Nu_Che_Cnt) % 100);
				/* 주문가(지수) */
				MvAddStr (i*2+11, 65,  3, Shm_Db[0].Ju_O_History[ApType].apno[i].Order_Price+3);
				/* 주문가(소수) */
				MvAddStr (i*2+11, 69,  2, Shm_Db[0].Ju_O_History[ApType].apno[i].Order_Price+7);
				/* 현재값(지수) */
				MvAddNum (i*2+11, 73,  3, AtoIf(Shm_Options[Shm_Db[0].Ju_O_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) / 100);
				/* 현재값(소수) */
				MvAddNum (i*2+11, 77,  2, AtoIf(Shm_Options[Shm_Db[0].Ju_O_History[ApType].apno[i].Jong_Seq].Options_CURR.crprc, 5) % 100);

				/* End_Flag */
				MvAddStr (i*2+12,  9,  1, Shm_Db[0].Ju_O_History[ApType].apno[i].End_Flag);
				/* 총수량 */
				MvAddNum (i*2+12, 19,  6, Shm_Db[0].Ju_O_History[ApType].apno[i].Tot_Order_Cnt);
				/* 주문금액 */
				MvAddNum (i*2+12, 36, 10, Shm_Db[0].Ju_O_History[ApType].apno[i].Ju_Gum);
				/* 주문번호 */
				MvAddStr (i*2+12, 57,  7, Shm_Db[0].Ju_O_History[ApType].apno[i].OrderNo);
				/* 원주문번호 */
				MvAddStr (i*2+12, 66,  7, Shm_Db[0].Ju_O_History[ApType].apno[i].OriginalOrderNo);
			}
		}
		else if (St_No == 8)
		{
			if ((i < Shm_Db[0].Ju_A_History[ApType].che_max)	&&
				(memcmp (Shm_Db[0].Ju_A_History[ApType].che_set[i].End_Flag, "0", 1) != 0))
			{
				/* 종목정보 */
				MvAddStr (i*2+11,  5,  8, Shm_Futures[0].Futures_CURR.item_code);
				/* 평가액 */
				MvAddNum (i*2+11, 22, 10,
					((Shm_Db[0].Ju_A_History[ApType].che_set[i].Che_Avg -
					  AtoIf (Shm_Futures[0].Futures_CURR.crprc, 5)) *
					 (Shm_Db[0].Ju_A_History[ApType].che_set[i].Order_Int -
					  Shm_Db[0].Ju_A_History[ApType].che_set[i].Che_Int)));
				/* 수량 */
				MvAddNum (i*2+11, 40,  6,
						Shm_Db[0].Ju_A_History[ApType].che_set[i].Order_Int -
						Shm_Db[0].Ju_A_History[ApType].che_set[i].Che_Int);

				/* 매입가(지수) */
				MvAddNum (i*2+11, 57,  3,
						Shm_Db[0].Ju_A_History[ApType].che_set[i].Che_Avg / 100);
				/* 매입가(소수) */
				MvAddNum (i*2+11, 61,  2,
						Shm_Db[0].Ju_A_History[ApType].che_set[i].Che_Avg % 100);

				/* 현재값(지수) */
				MvAddNum (i*2+11, 73,  3, AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5) / 100);
				/* 현재값(소수) */
				MvAddNum (i*2+11, 77,  2, AtoIf(Shm_Futures[0].Futures_CURR.crprc, 5) % 100);

				/* End_Flag */
				MvAddStr (i*2+12,  9,  1, Shm_Db[0].Ju_A_History[ApType].che_set[i].End_Flag);

				/* 주문번호 */
				MvAddStr (i*2+12, 57,  7, Shm_Db[0].Ju_A_History[ApType].che_set[i].OrderNo);
				/* 원주문번호 */
				MvAddStr (i*2+12, 66,  7, Shm_Db[0].Ju_A_History[ApType].che_set[i].OriginalOrderNo);
			}
		}
	}

	prt = j = 0;
	for (i = 0; i < 10000; i ++)
	{
		if ((Shm_Db[0].F_MiChe[i].Jan_Cnt > 0) &&
			(memcmp (Shm_Db[0].F_MiChe[i].AccountNo, ACCNO(0,ApType).acc_no, 9) == 0))
		{
			v = 0;
			if (St_No == 1)
			{
				break;
			}
			else if (St_No == 4)
			{
				for (m = 0; m < 20; m++)
				{
					if (memcmp (Shm_Db[0].F_MiChe[i].OrderNo, Shm_Db[0].Ju_History[ApType].apno[m].OrderNo, 6) == 0)
					{
						v = 1;
						break;
					}
				}
			}
			else if (St_No == 6)
			{
				for (m = 0; m < 20; m++)
				{
					if (memcmp (Shm_Db[0].F_MiChe[i].OrderNo, Shm_Db[0].Ju_O_History[ApType].apno[m].OrderNo, 6) == 0)
					{
						v = 1;
						break;
					}
				}
			}
			else if (St_No == 8)
			{
				for (m = 0; m < 3; m++)
				{
					if (memcmp (Shm_Db[0].F_MiChe[i].OrderNo, Shm_Db[0].Ju_A_History[ApType].nju_set[m].OrderNo, 6) == 0)
					{
						v = 1;
						break;
					}
				}
			}

			if ((prt >= 0 && prt < 8) && (v == 1))
			{
				MvAddNum (j+29,  1, 4, i);
				MvAddStr (j+29,  7, 9, Shm_Db[0].F_MiChe[i].AccountNo);
				MvAddStr (j+29, 18, 8, Shm_Db[0].F_MiChe[i].Item_Cd);
				MvAddStr (j+29, 28, 7, Shm_Db[0].F_MiChe[i].OrderNo);
				MvAddStr (j+29, 37, 7, Shm_Db[0].F_MiChe[i].OriginalOrderNo);
				MvAddStr (j+29, 46, 1, Shm_Db[0].F_MiChe[i].PriceFlag);
				MvAddStr (j+29, 50, 2, Shm_Db[0].F_MiChe[i].TradeFlag);
				MvAddStr (j+29, 54, 8, Shm_Db[0].F_MiChe[i].Order_Cnt);
				MvAddStr (j+29, 64, 8, Shm_Db[0].F_MiChe[i].Order_Jan_Cnt);
				MvAddStr (j+29, 74, 9, Shm_Db[0].F_MiChe[i].Order_Price);
				MvAddStr (j+29, 85, 1, Shm_Db[0].F_MiChe[i].JumunFlag);
				MvAddStr (j+29, 88, 1, Shm_Db[0].F_MiChe[i].OrderType);
				j ++;
			}
			prt = prt + 1;
		}
		
		if (prt >= ACCNO(0,ApType).miche_cnt)
			break;
	}

	MvAddNum (28, 10, 4, j);

	return;
}

/*************************************************************************
	End of Program (py_2090_cm.c)
*************************************************************************/
