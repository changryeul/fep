/*------------------------------------------------------------------------
#	Module	: Main Menu
#	File	: py_main_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		xloc, yloc, zloc, sub_flag, Rows;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Clear_Screen (void);
void			Disp_Head (void);
void			Exec_Sub_Rtn (void);

/*----------------------------------------------------------------------*/
int		main (void)
/*----------------------------------------------------------------------*/
{
	WINDOW	*wp;

	Setsigfatal ();
	wp = initscr ();

	if (wp->_maxy < 30 || wp->_maxx < 80)
	{
		refresh ();
		endwin ();
		printf ("Please change screen rows or columns. (%dx%d -> 30x80)\n",
			wp->_maxy, wp->_maxx);
		printf ("30 lines and 80 columns recommended.\n");
		exit (OK);
	}

	Rows = wp->_maxy;
	newwin (Rows, 80, 0, 0);

	Register_Signal ();

	sub_flag = 0;
	xloc = yloc = zloc = 1;

	Disp_Head ();
	Disp_Scr ();

	while (xloc != 0)
	{
		keypad (stdscr, TRUE);
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

		switch (getch ()) 
		{
			case	KEY_RIGHT:
			case	'r':
				if (sub_flag == 0)
				{
					xloc ++;		yloc = 1;
					if (xloc > 4)   xloc = 1;
					Disp_Scr ();
				}
				else
				{
					sub_flag = 0;
					zloc = 1;
					Disp_Scr ();
				}
				break;
			case	KEY_LEFT:
			case	'l':
				if (sub_flag == 0)
				{
					xloc --;		yloc = 1;
					if (xloc < 1)   xloc = 4;
					Disp_Scr ();
				}
				else
				{
					sub_flag = 0;
					zloc = 1;
					Disp_Scr ();
				}
				break;
			case	KEY_UP:
			case	'u':
				if (sub_flag == 0)
				{
					yloc --;
					if (yloc < 1)
					{
						if (xloc == 1)		yloc = 4;	/* Proc/Data	*/
						else if (xloc == 2)	yloc = 7;	/* 시세, 계좌	*/
						else if (xloc == 3)	yloc = 6;	/* Process Stat	*/
						else				yloc = 5;	/* System/Misc	*/
					}
					Disp_Scr ();
				}

				break;
			case	KEY_DOWN:
			case	'd':
				if (sub_flag == 0)
				{
					yloc ++;
					if ((xloc == 1 && yloc > 4) || (xloc == 2 && yloc > 7) ||
						(xloc == 3 && yloc > 6) || (xloc == 4 && yloc > 5))
						yloc = 1;
					Disp_Scr ();
				}

				break;
			case	'\n':										/* Run	*/
				Exec_Sub_Rtn ();
				sub_flag = 0;
				break;
			case	'q':
			case	'Q':										/* Quit	*/
				xloc = 0;
				break;
	   }
	}

	refresh ();
	endwin ();

	exit (OK);
}

/*----------------------------------------------------------------------*/
void	Disp_Head (void)
/*----------------------------------------------------------------------*/
{
	Disp_Title ();
	attron (A_BOLD);
	mvaddstr (0, 18, "[main] Top Menu");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 2, "Proc/Data          시세               Process Status     System & Misc.");
	Draw_Line (3, 0, 79);
	Draw_Line (Rows - 2, 0, 79);
}

/*----------------------------------------------------------------------*/
static void		Disp_Scr (void)
/*----------------------------------------------------------------------*/
{
	mvaddstr (2, 2, "Proc/Data          시세               Process Status     System & Misc.    ");
	if (xloc == 1)
	{
		attron (A_REVERSE);
		mvaddstr (2, 2, "Proc/Data         ");
		attroff (A_REVERSE);
		Clear_Screen ();

		mvaddstr (4, 2, "주문,응답,체결    ");
		mvaddstr (5, 2, "Auto주문          ");
		mvaddstr (6, 2, "조회              ");
		mvaddstr (7, 2, "시세,한도         ");

		attron (A_REVERSE);
		
		switch (yloc)
		{
			case	1:
				mvaddstr (4, 2, "주문,응답,체결    ");
				Disp_Msg ("py_1010_cm (Q:Quit)");
				break;
			case	2:
				mvaddstr (5, 2, "Auto주문          ");
				Disp_Msg ("py_1010_cm (Q:Quit)");
				break;
			case	3:
				mvaddstr (6, 2, "조회              ");
				Disp_Msg ("py_1010_cm (Q:Quit)");
				break;
			case	4:
				mvaddstr (7, 2, "시세,한도         ");
				Disp_Msg ("py_1010_cm (Q:Quit)");
				break;
		}

		attroff (A_REVERSE);
	}
	else if (xloc == 2)
	{
		attron (A_REVERSE);
		mvaddstr (2, 21, "시세              ");
		attroff (A_REVERSE);
		Clear_Screen ();

		mvaddstr (4, 21, "호가(B6)          ");
		mvaddstr (5, 21, "체결(A3/G7)       ");
		mvaddstr (6, 21, "최근체결내역      ");
		mvaddstr (7, 21, "최근시세내역      ");
		mvaddstr (8, 21, "현물정보조회      ");
		mvaddstr (9, 21, "계좌손익조회      ");
		mvaddstr (10, 21, "종목별계좌정보    ");

		attron (A_REVERSE);
		switch (yloc)
		{
			case	1:
				mvaddstr (4, 21, "호가(B6)          ");
				Disp_Msg ("py_2010_cm (Q:Quit)");
				break;
			case	2:
				mvaddstr (5, 21, "체결(A3/G7)       ");
				Disp_Msg ("py_2020_cm (Q:Quit)");
				break;
			case	3:
				mvaddstr (6, 21, "최근체결내역      ");
				Disp_Msg ("py_2030_cm (Q:Quit)");
				break;
			case	4:
				mvaddstr (7, 21, "최근시세내역      ");
				Disp_Msg ("py_2040_cm (Q:Quit)");
				break;
			case	5:
				mvaddstr (8, 21, "현물정보조회    ");
				Disp_Msg ("py_2050_cm (Q:Quit)");
				break;
			case	6:
				mvaddstr (9, 21, "계좌손익조회      ");
				Disp_Msg ("py_2060_cm (Q:Quit)");
				break;
			case	7:
				mvaddstr (10, 21, "종목별계좌정보    ");
				Disp_Msg ("py_2070_cm (Q:Quit)");
				break;
		}
		attroff (A_REVERSE);
	}
	else if (xloc == 3)
	{
		attron (A_REVERSE);
		mvaddstr (2, 40, "Process Status    ");
		attroff (A_REVERSE);
		Clear_Screen ();

		mvaddstr (4, 40, "주문,응답,체결    ");
		mvaddstr (5, 40, "Auto주문          ");
		mvaddstr (6, 40, "조회              ");
		mvaddstr (7, 40, "시세,한도         ");
		mvaddstr (8, 40, "업무접속      (PW)");
		mvaddstr (9, 40, "Super Daemons     ");

		attron (A_REVERSE);
		switch (yloc)
		{
			case	1:
				mvaddstr (4, 40, "주문,응답,체결    ");
				Disp_Msg ("py_3010_cm (Q:Quit)");
				break;
			case	2:
				mvaddstr (5, 40, "Auto주문          ");
				Disp_Msg ("py_3010_cm (Q:Quit)");
				break;
			case	3:
				mvaddstr (6, 40, "조회              ");
				Disp_Msg ("py_3010_cm (Q:Quit)");
				break;
			case	4:
				mvaddstr (7, 40, "시세,한도         ");
				Disp_Msg ("py_3010_cm (Q:Quit)");
				break;
			case	5:
				mvaddstr (8, 40, "업무접속      (PW)");
				Disp_Msg ("py_3010_cm (Q:Quit)");
				break;
			case	6:
				mvaddstr (9, 40, "Super Daemons     ");
				Disp_Msg ("py_3020_cm (Q:Quit)");
				break;
		}
		attroff (A_REVERSE);
	}
	else if (xloc == 4)
	{
		attron (A_REVERSE);
		mvaddstr (2, 59, "System & Misc.    ");
		attroff (A_REVERSE);
		Clear_Screen ();
		mvaddstr (4, 59, "Process Info      ");
		mvaddstr (5, 59, "Daemon Info - all ");
		mvaddstr (6, 59, "Daemon Info - sub ");
		mvaddstr (7, 59, "Control Daemon");
		mvaddstr (8, 59, "Quit");

		attron (A_REVERSE);
		switch (yloc)
		{
			case	1:
				mvaddstr (4, 59, "Process Info      ");
				Disp_Msg ("py_2020_cm (Q:Quit)");
				break;
			case	2:
				mvaddstr (5, 59, "Daemon Info - all ");
				Disp_Msg ("py_2030_cm (Q:Quit)");
				break;
			case	3:
				mvaddstr (6, 59, "Daemon Info - sub ");
				Disp_Msg ("py_2040_cm (Q:Quit)");
				break;
			case	4:
				mvaddstr (7, 59, "Control Daemon    ");
				Disp_Msg ("py_4020_cm (Q:Quit)");
				break;
			case	5:
				mvaddstr (8, 59, "Quit              ");
				Disp_Msg ("Quit the program.");
				break;
		}
		attroff (A_REVERSE);
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Sub_Menu (void)
/*----------------------------------------------------------------------*/
{
	sub_flag = 1;

	return;
}

/*----------------------------------------------------------------------*/
static void		Clear_Screen (void)
/*----------------------------------------------------------------------*/
{
	int		ln;

	ln = 3;

	while (ln < Rows - 3)
	{
		ln ++;
		mvaddstr (ln, 1, "                                                                              ");
	}

	return;
}

/*----------------------------------------------------------------------*/
void	Exec_Sub_Rtn (void)
/*----------------------------------------------------------------------*/
{
	register	i;

	refresh ();
	endwin ();

	if (xloc == 1)										/* Proc/Data	*/
	{
		switch (yloc)
		{
			case	1:								/* 주문,응답,체결	*/
				py_1010_cm ("a1049");	break;
			case	2:										/* Auto주문	*/
				py_1010_cm ("a5059");	break;
			case	3:											/* 조회	*/
				py_1010_cm ("a6069");	break;
			case	4:									/* 시세,한도	*/
				py_1010_cm ("a7099");	break;
			default:					break;
		}
	}
	else if (xloc == 2)											/* 시세	*/
	{
		switch (yloc)
		{
			case	1:											/* 호가	*/
				py_2010_cm ();
				break;
			case	2:											/* 체결	*/
				py_2020_cm ();
				break;
			case	3:										/* 최근체결	*/
				py_2030_cm ();
				break;
			case	4:										/* 최근시세	*/
				py_2040_cm ();
				break;
			case	5:										/* 현물정보	*/
				py_2050_cm ();
				break;
			case	6:										/* 계좌손익	*/
				py_2060_cm ();
				break;
			case	7:										/* 계좌정보	*/
				py_2070_cm ();
				break;
			default:
				break;
		}
	}
	else if (xloc == 3)								/* Process Status	*/
	{
		switch (yloc)
		{
			case	1:	py_3010_cm ("a1049");	break;
			case	2:	py_3010_cm ("a5059");	break;
			case	3:	py_3010_cm ("a6069");	break;
			case	4:	py_3010_cm ("a7099");	break;
			case	5:	py_3010_cm ("w");		break;
			case	6:	py_3020_cm ();			break;
			default:							break;
		}
	}
	else if (xloc == 4)								/* System & Misc.	*/
	{
		switch (yloc)
		{
			case	1:		py_4010_cm ();	break;
			case	2:		py_4020_cm ();	break;
			case	3:		py_4030_cm ();	break;
			case	4:		py_4040_cm ();	break;
			case	5:		xloc = 0;		return;
		}
	}

	SHM_Detach ((char *)SHM_All_Daemon_Info);

	for (i = 0; i < Process_Count; i ++)
		SHM_Detach ((char *)SHM_Mem[i]); 

	initscr ();
	newwin (Rows, 80, 0, 0);
	Disp_Head ();
	Disp_Scr ();

	if (zloc != 0)
		Disp_Sub_Menu ();

	return;
}

/*************************************************************************
	End of Program (py_main_cm.c)
*************************************************************************/
