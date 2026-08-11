/*------------------------------------------------------------------------
#   Module  : Shared Memory - Daemon Info (all - INFO)
#   File	: py_4020_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
extern int	Rows;
static int	Dk;
static int	Flag;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		All_Daemon (void);
static void		Info_Daemon (void);
static void		Disp_Daemon (void);
static void		Change_Daemon (void);

/*----------------------------------------------------------------------*/
void	py_4020_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	Dk = 25;
	Flag = 0;
	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Info_Daemon ();

	Disp_Msg ("Enter:Retry A:All I:Info C:Change Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg ("Enter:Retry A:All I:Info C:Change Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

		switch (getch ())
		{
			case	'a':
			case	'A':										/* All	*/
				All_Daemon ();
				Flag = 1;
				break;
			case	'i':
			case	'I':										/* Info	*/
				Info_Daemon ();
				Flag = 0;
				break;
			case	'c':
			case	'C':									/* Change	*/
				Change_Daemon ();
				Flag = 0;
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
				if (Flag == 0)
					Disp_Daemon ();
				else
					All_Daemon ();
				Disp_Msg ("Enter:Retry A:All I:Info C:Change Q:Quit");
				break;
			default:
				clear ();
				Disp_Scr ();
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg ("Enter:Retry A:All I:Info C:Change Q:Quit");
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
	mvaddstr (0, 18, "[4020] Daemon Info - all (INFO)");
	attroff (A_BOLD);

	Draw_Line (1, 0, 79);
	mvaddstr (2, 2, "ºÎ¹®     (PA, PB, ...)  D_K");
	Draw_Underline (2, 7, 2);

	mvaddstr (4, 9, "process_no                    process_id");
	mvaddstr (5, 9, "start_time                      end_time");
	mvaddstr (6, 6, "system_status                process_status");
	mvaddstr (7, 4, "start_FIFO_name                exit_FIFO_name");
	mvaddstr (8, 3, "daemon_FIFO_name                  check_status");
	mvaddstr (10, 6, "process_count                    file_count");
#if defined ISAM_INCL
	mvaddstr (12, 8, "accno_count                 cisam_count");
#else
	mvaddstr (12, 8, "accno_count                 udpip_count");
#endif
	mvaddstr (13, 9, "tcp1_count                    tcp2_count");
	mvaddstr (14, 9, "dshm_count                  sisetr_count");
	mvaddstr (15, 9, "data_count                          date");
	mvaddstr (16, 10,
		"date_flag    (1:D~D 2:D~D+1 3:D+1~D+1 4:D-1~D 5:D-1~D-1)");
	mvaddstr (17, 7, "compact_days");

	return;
}

/*----------------------------------------------------------------------*/
static void		All_Daemon (void)
/*----------------------------------------------------------------------*/
{
	int		i, y;
	char	buf[4];

	Clear_Lines (1, 28);
	G_Time ();

#if defined ISAM_INCL
	mvaddstr (1, 6,
"PID STRT END  S P PROC FILE ISAM  TCP1 TCP2 DC DATE     F  C");
#else
	mvaddstr (1, 6,
"PID STRT END  S P PROC FILE DSHM  TCP1 TCP2 DC DATE     F  C");
#endif

	for (i = 0, y = 2; i < Process_Count; i ++)
	{
		if (i + 'a' == 'x' || i + 'a' == 'y' ||
			i < 0 || i > 25 || (i + 'a' != 'z' && INFO(i).process_count == 0))
			continue;

		attron (A_BOLD);
		sprintf (buf, "P%c", i + 'A');
		mvaddstr (y, 0, buf);
		attroff (A_BOLD);

		attron (A_UNDERLINE);
		MvAddStr (y, 10, 4, INFO(i).start_time);
		MvAddStr (y, 15, 4, INFO(i).end_time);
		MvAddNum (y, 75, 2, INFO(i).compact_days);

		if (i + 'a' != 'z')
		{
			MvAddNum (y, 4, 5, INFO(i).process_no);
			MvAddNum (y, 20, 1, INFO(i).system_status);
			MvAddNum (y, 22, 1, INFO(i).process_status);
			MvAddNum (y, 25, 3, INFO(i).process_count);
			MvAddNum (y, 30, 3, INFO(i).file_count);
#if defined ISAM_INCL
			MvAddNum (y, 35, 3, INFO(i).cisam_count);
#else
			MvAddNum (y, 35, 3, INFO(i).dshm_count);
#endif
			MvAddNum (y, 52, 3, INFO(i).tcp1_count);
			MvAddNum (y, 57, 3, INFO(i).tcp2_count);
			MvAddNum (y, 61, 2, INFO(i).data_count);
			MvAddStr (y, 64, 8, INFO(i).date);
			MvAddNum (y, 73, 1, INFO(i).date_flag);
		}
		attroff (A_UNDERLINE);

		y ++;
	}

	mvaddstr (Rows - 5, 4,
		"STRT:start_time    END:end_time    S:system_status  P:process_status");
	mvaddstr (Rows - 4, 4, "PRO:process_count  FIL:file_count");
#if defined ISAM_INCL
	mvaddstr (Rows - 3, 4, "TCP1:tcp1_count TCP2:tcp2_count  ISAM:cisam_count");
#else
	mvaddstr (Rows - 3, 4, "TCP1:tcp1_count TCP2:tcp2_count  DSHM:dshm_count");
#endif
	mvaddstr (Rows - 2, 4, "DC:data_count      F:date_flag     C:compact_days");

	return;
}

/*----------------------------------------------------------------------*/
static void		Info_Daemon (void)
/*----------------------------------------------------------------------*/
{
	int 	rt;
	char	buf[80];

	Clear_Lines (2, Rows - 2);
	Disp_Scr ();

	Disp_Msg ("Input <Daemon ID> (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (2, 7, 2);
		rt = Get_String (2, 7, 2, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Daemon ID> (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			LtoU (buf, 2);
			attron (A_UNDERLINE);
			mvaddstr (2, 7, buf);
			attroff (A_UNDERLINE);

			if (strlen (buf) != 2 || buf[0] != 'P' ||
				buf[1] < 'A' || buf[1] > 'Z' || buf[1] == 'X' || buf[1] == 'Y')
            {
                Disp_Msg ("Incorrect Daemon ID (Esc:Cancel)");
                continue;
            }

			Dk = buf[1] - 'A';

			if (Dk + 'a' == 'z')
			{
				Disp_Daemon ();
				return;
			}
			else if (INFO(Dk).process_id[0] == 0)
			{
				sprintf (buf,
					"P%c daemon not registered. (Esc:Cancel)", Dk + 'A');
				Disp_Msg (buf);
			}
			else
			{
				Disp_Daemon ();
				return;
			}
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Daemon (void)
/*----------------------------------------------------------------------*/
{
	G_Time ();

	attron (A_UNDERLINE);
	MvAddNum (2, 30, 2, Dk);
	MvAddStr (5, 20, 4, INFO(Dk).start_time);
	MvAddStr (5, 50, 4, INFO(Dk).end_time);
	MvAddStr (7, 20, 7, INFO(Dk).start_FIFO_name);
	MvAddStr (7, 50, 12, INFO(Dk).exit_FIFO_name);
	MvAddStr (8, 20, 12, INFO(Dk).daemon_FIFO_name);
	MvAddNum (17, 20, 2, INFO(Dk).compact_days);

	if (Dk + 'a' != 'z')
	{
		MvAddNum (4, 20, 5, INFO(Dk).process_no);
		MvAddStr (4, 50, 20, INFO(Dk).process_id);
		MvAddNum (6, 20, 1, INFO(Dk).system_status);
		MvAddNum (6, 50, 1, INFO(Dk).process_status);
		MvAddNum (8, 50, 1, INFO(Dk).check_status);
		MvAddNum (10, 20, 3, INFO(Dk).process_count);
		MvAddNum (10, 50, 3, INFO(Dk).file_count);
		MvAddNum (12, 20, 3, INFO(Dk).accno_count);
#if defined ISAM_INCL
		MvAddNum (12, 50, 3, INFO(Dk).cisam_count);
#else
		MvAddNum (12, 50, 3, INFO(Dk).udpip_count);
#endif
		MvAddNum (13, 20, 3, INFO(Dk).tcp1_count);
		MvAddNum (13, 50, 3, INFO(Dk).tcp2_count);
		MvAddNum (14, 20, 3, INFO(Dk).dshm_count);
		MvAddNum (14, 50, 3, INFO(Dk).sisetr_count);
		MvAddNum (15, 20, 3, INFO(Dk).data_count);
		MvAddStr (15, 50, 8, INFO(Dk).date);
		MvAddNum (16, 20, 1, INFO(Dk).date_flag);
	}
	attroff (A_UNDERLINE);

	return;
}

/*----------------------------------------------------------------------*/
static void		Change_Daemon (void)
/*----------------------------------------------------------------------*/
{
	int 	rt, item, x, y, l;
	char	buf[20];

	attron (A_REVERSE);
	mvaddstr (4, 8, "1");
	mvaddstr (5, 8, "2");
	mvaddstr (5, 40, "3");
	mvaddstr (6, 5, "4");
	mvaddstr (6, 34, "5");
	mvaddstr (7, 3, "6");
	mvaddstr (7, 34, "7");
	mvaddstr (8, 2, "8");
	mvaddstr (8, 36, "9");
	mvaddstr (10, 4, "10");
	mvaddstr (10, 37, "11");
#if defined ISAM_INCL
	mvaddstr (12, 36, "15");
#endif
	mvaddstr (13, 7, "16");
	mvaddstr (13, 37, "17");
	mvaddstr (14, 7, "22");
	mvaddstr (14, 35, "23");
	mvaddstr (15, 7, "18");
	mvaddstr (15, 43, "19");
	mvaddstr (16, 8, "20");
	mvaddstr (17, 5, "21");
	attroff (A_REVERSE);

	Disp_Msg ("Select the item to change. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 1, 44, 2);
		rt = Get_String (Rows - 1, 44, 2, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Select the item to change. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input less than 3 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE. (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else if (Chk_Digit (buf) == 0)
			Disp_Msg ("Numbers only. (Esc:Cancel)");
		else
		{
			item = atoi (buf);

			if (item == 1)
			{
				x = 4;	y= 20;	l = 5;
			}
			else if (item == 2)
			{
				x = 5;	y= 20;	l = 4;
			}
			else if (item == 3)
			{
				x = 5;	y= 50;	l = 4;
			}
			else if (item == 4)
			{
				x = 6;	y= 20;	l = 1;
			}
			else if (item == 5)
			{
				x = 6;	y= 50;	l = 1;
			}
			else if (item == 6)
			{
				x = 7;	y= 20;	l = 7;
			}
			else if (item == 7)
			{
				x = 7;	y= 50;	l = 12;
			}
			else if (item == 8)
			{
				x = 8;	y= 20;	l = 12;
			}
			else if (item == 9)
			{
				x = 8;	y= 50;	l = 1;
			}
			else if (item == 10)
			{
				x = 10;	y= 20;	l = 3;
			}
			else if (item == 11)
			{
				x = 10;	y= 50;	l = 3;
			}
			else if (item == 12)
			{
				x = 11;	y= 20;	l = 3;
			}
			else if (item == 13)
			{
				x = 11;	y= 50;	l = 3;
			}
			else if (item == 14)
			{
				x = 12;	y= 20;	l = 3;
			}
			else if (item == 15)
			{
				x = 12;	y= 50;	l = 3;
			}
			else if (item == 16)
			{
				x = 13;	y= 20;	l = 3;
			}
			else if (item == 17)
			{
				x = 13;	y= 50;	l = 3;
			}
			else if (item == 18)
			{
				x = 15;	y= 20;	l = 3;
			}
			else if (item == 19)
			{
				x = 15;	y= 50;	l = 8;
			}
			else if (item == 20)
			{
				x = 16;	y= 20;	l = 1;
			}
			else if (item == 21)
			{
				x = 17;	y= 20;	l = 2;
			}
			else if (item == 22)
			{
				x = 14;	y= 20;	l = 3;
			}
			else if (item == 23)
			{
				x = 14;	y= 50;	l = 3;
			}
			else
			{
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
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input the value that you want to set. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("You inputted over the limit. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE. (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			rt = Yes_No ("Are you sure to change?", NO);
			if (rt == YES)
			{
				if (item == 1)
					INFO(Dk).process_no = atoi (buf);
				else if (item == 2)
					memcpy (INFO(Dk).start_time, buf, 4);
				else if (item == 3)
					memcpy (INFO(Dk).end_time, buf, 4);
				else if (item == 4)
					INFO(Dk).system_status = atoi (buf);
				else if (item == 5)
					INFO(Dk).process_status = atoi (buf);
				else if (item == 6)
					memcpy (INFO(Dk).start_FIFO_name, buf, 7);
				else if (item == 7)
					memcpy (INFO(Dk).exit_FIFO_name, buf, 12);
				else if (item == 8)
					memcpy (INFO(Dk).daemon_FIFO_name, buf, 12);
				else if (item == 9)
					INFO(Dk).check_status = atoi (buf);
				else if (item == 10)
					INFO(Dk).process_count = atoi (buf);
				else if (item == 11)
					INFO(Dk).file_count = atoi (buf);
				else if (item == 15)
#if defined ISAM_INCL
					INFO(Dk).cisam_count = atoi (buf);
#else
					INFO(Dk).udpip_count = atoi (buf);
#endif
				else if (item == 16)
					INFO(Dk).tcp1_count = atoi (buf);
				else if (item == 17)
					INFO(Dk).tcp2_count = atoi (buf);
				else if (item == 18)
					INFO(Dk).data_count = atoi (buf);
				else if (item == 19)
					memcpy (INFO(Dk).date, buf, 8);
				else if (item == 20)
					INFO(Dk).date_flag = atoi (buf);
				else if (item == 21)
					INFO(Dk).compact_days = atoi (buf);
				else if (item == 22)
					INFO(Dk).dshm_count = atoi (buf);
				else if (item == 23)
					INFO(Dk).sisetr_count = atoi (buf);

				Disp_Msg ("¢º¢º¢º Changed ¢¸¢¸¢¸");
				Debug (PGMLIN, "item %d changed [%s]", item, buf);
			}
			else
				Disp_Msg ("¢º¢º¢º Cancelled ¢¸¢¸¢¸");

			sleep (1);
			break;
		}
	}

	return;
}

/*************************************************************************
	End of Program (py_4020_cm.c)
*************************************************************************/
