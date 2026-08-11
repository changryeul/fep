/*------------------------------------------------------------------------
#   Module  : Shared Memory - Daemon Info (sub - DAEMON)
#   File	: py_4030_cm.c
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

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Info_Daemon (void);
static void		Disp_Daemon (void);
static void		Change_Daemon (void);

/*----------------------------------------------------------------------*/
void	py_4030_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	Dk = -1;
    job_end = 1;

    initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Mem_SHM (0, -1);
	Info_Daemon ();

	Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
	keypad (stdscr, TRUE);

    while (job_end)
    {
		Disp_Msg ("Enter:Retry I:Info C:Change Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);

	    switch (getch ())
	    {
			case	'i':
			case	'I':										/* Info	*/
				Info_Daemon ();
			    break;
			case	'c':
			case	'C':									/* Change	*/
				Change_Daemon ();
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
				Disp_Daemon ();
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
	Disp_Title ();
	attron (A_BOLD);
	mvaddstr (0, 18, "[4030] Daemon Info - sub (DAEMON)");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	mvaddstr (2, 2, "ºÎ¹®     (JA, JB, ...)");
	Draw_Underline (2, 7, 2);

	mvaddstr (2, 27, "D_K                process_no");
	mvaddstr (3, 5, "process_path");
	mvaddstr (4, 7,
		"process_id                             start_time        end_time");
	mvaddstr (5, 9, "               system_status     process_status");
	mvaddstr (6, 2, "start_FIFO_name                         exit_FIFO_name");
	mvaddstr (7, 1, "daemon_FIFO_name");
#if defined ISAM_INCL
	mvaddstr (8, 4, "cisam_count        c_count        CShmsize");
#endif
	mvaddstr (9, 2, "process_count        p_count         Shmsize");
	mvaddstr (10, 5, "file_count        f_count        FShmsize");
	mvaddstr (11, 5, "dshm_count        d_count        DShmsize");
	mvaddstr (12, 4, "accno_count        a_count        AShmsize");
	mvaddstr (15, 5, "tcp1_count       t1_count       T1Shmsize");
	mvaddstr (16, 5, "tcp2_count       t2_count       T2Shmsize");
	mvaddstr (17, 4, "udpip_count        u_count        UShmsize");
	mvaddstr (18, 3, "sisetr_count        s_count        SShmsize");
	mvaddstr (20, 3, "process_info");
	mvaddstr (21, 5, "data_count           date");
	mvaddstr (22, 6,
		"date_flag    (1:D~D 2:D~D+1 3:D+1~D+1 4:D-1~D 5:D-1~D-1)");
	mvaddstr (23, 3, "compact_days");
	mvaddstr (24, 8, "shm_log      w_cnt               r_cnt");

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
				buf[1] < 'A' || buf[1] > 'W')
            {
                Disp_Msg ("Incorrect Daemon ID (Esc:Cancel)");
                continue;
            }

			Dk = buf[1] - 'A';

			if (INFO(Dk).process_id[0] == 0)
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
	MvAddNum (2, 31, 2, Dk);
	MvAddStr (4, 57, 4, DAEMON(Dk).start_time);
	MvAddStr (4, 73, 4, DAEMON(Dk).end_time);
	MvAddStr (6, 18, 7, DAEMON(Dk).start_FIFO_name);
	MvAddStr (6, 57, 12, DAEMON(Dk).exit_FIFO_name);
	MvAddStr (7, 18, 12, DAEMON(Dk).daemon_FIFO_name);
	MvAddNum (23, 16, 2, DAEMON(Dk).compact_days);
	MvAddNum (24, 16, 1, DAEMON(Dk).shm_log);
	MvAddNum (24, 27, 10, DAEMON(Dk).w_cnt);
	MvAddNum (24, 47, 10, DAEMON(Dk).r_cnt);

	if (Dk + 'a' != 'z')
	{
		MvAddNum  (2, 57, 5, DAEMON(Dk).process_no);
		MvAddStr  (3, 18, 60, DAEMON(Dk).process_path);
		MvAddStr  (4, 18, 20, DAEMON(Dk).process_id);
		MvAddNum  (5, 38,  1, DAEMON(Dk).system_status);
		MvAddNum  (5, 57,  1, DAEMON(Dk).process_status);
#if defined ISAM_INCL
		MvAddNum  (8, 16,  3, DAEMON(Dk).cisam_count);
		MvAddNum  (8, 31,  3, DAEMON(Dk).c_count);
		MvAddLong (8, 47, 10, DAEMON(Dk).CShmsize);
#endif
		MvAddNum  (9, 16,  3, DAEMON(Dk).process_count);
		MvAddNum  (9, 31,  3, DAEMON(Dk).p_count);
		MvAddLong (9, 47, 10, DAEMON(Dk).Shmsize);
		MvAddNum  (10, 16,  3, DAEMON(Dk).file_count);
		MvAddNum  (10, 31,  3, DAEMON(Dk).f_count);
		MvAddLong (10, 47, 10, DAEMON(Dk).FShmsize);
		MvAddNum  (11, 16,  3, DAEMON(Dk).dshm_count);
		MvAddNum  (11, 31,  3, DAEMON(Dk).d_count);
		MvAddLong (11, 47, 10, DAEMON(Dk).DShmsize);
		MvAddNum  (12, 16,  3, DAEMON(Dk).accno_count);
		MvAddNum  (12, 31,  3, DAEMON(Dk).a_count);
		MvAddLong (12, 47, 10, DAEMON(Dk).AShmsize);
		MvAddNum  (15, 16,  3, DAEMON(Dk).tcp1_count);
		MvAddNum  (15, 31,  3, DAEMON(Dk).t1_count);
		MvAddLong (15, 47, 10, DAEMON(Dk).T1Shmsize);
		MvAddNum  (16, 16,  3, DAEMON(Dk).tcp2_count);
		MvAddNum  (16, 31,  3, DAEMON(Dk).t2_count);
		MvAddLong (16, 47, 10, DAEMON(Dk).T2Shmsize);
		MvAddNum  (17, 16,  3, DAEMON(Dk).udpip_count);
		MvAddNum  (17, 31,  3, DAEMON(Dk).u_count);
		MvAddLong (17, 47, 10, DAEMON(Dk).UShmsize);
		MvAddNum  (18, 16,  3, DAEMON(Dk).sisetr_count);
		MvAddNum  (18, 31,  3, DAEMON(Dk).s_count);
		MvAddLong (18, 47, 10, DAEMON(Dk).SShmsize);
		MvAddStr  (20, 16, 40, DAEMON(Dk).process_info);
		MvAddNum  (21, 16,  2, DAEMON(Dk).data_count);
		MvAddStr  (21, 31,  8, DAEMON(Dk).date);
		MvAddNum  (22, 16,  1, DAEMON(Dk).date_flag);
	}
	attroff (A_UNDERLINE);

	return;
}

/*----------------------------------------------------------------------*/
static void		Change_Daemon (void)
/*----------------------------------------------------------------------*/
{
	int 	rt, item, x, y, l;
	char	buf[10];

	attron (A_REVERSE);
	mvaddstr (2, 45, "1");
	mvaddstr (4, 45, "2");
	mvaddstr (4, 63, "3");
	mvaddstr (5, 23, "4");
	mvaddstr (5, 41, "5");
	mvaddstr (21, 25, "7");
	mvaddstr (22, 5, "8");
	mvaddstr (23, 2, "9");
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
				x = 2;	y= 57;	l = 5;
			}
			else if (item == 2)
			{
				x = 4;	y= 57;	l = 4;
			}
			else if (item == 3)
			{
				x = 4;	y= 73;	l = 4;
			}
			else if (item == 4)
			{
				x = 5;	y= 38;	l = 1;
			}
			else if (item == 5)
			{
				x = 5;	y= 57;	l = 1;
			}
			else if (item == 7)
			{
				x = 21;	y= 31;	l = 8;
			}
			else if (item == 8)
			{
				x = 22;	y= 16;	l = 1;
			}
			else if (item == 9)
			{
				x = 23;	y= 16;	l = 2;
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
		else if (rt == -4 && item != 8)						/* Space	*/
			Disp_Msg ("You cannot input SPACE. (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			rt = Yes_No ("Are you sure to change?", NO);
			if (rt == YES)
			{
				if (item == 1)
					DAEMON(Dk).process_no = atoi (buf);
				else if (item == 2)
				{
					memcpy (DAEMON(Dk).start_time, buf, 4);
					memcpy (INFO(Dk).start_time, buf, 4);
					Disp_Msg ("INFO.start_time also changed");
					sleep (1);
				}
				else if (item == 3)
				{
					memcpy (DAEMON(Dk).end_time, buf, 4);
					memcpy (INFO(Dk).end_time, buf, 4);
					Disp_Msg ("INFO.end_time also changed");
					sleep (1);
				}
				else if (item == 4)
					DAEMON(Dk).system_status = atoi (buf);
				else if (item == 5)
					DAEMON(Dk).process_status = atoi (buf);
				else if (item == 7)
				{
					memcpy (DAEMON(Dk).date, buf, 8);
					memcpy (INFO(Dk).date, buf, 8);
					Disp_Msg ("INFO.date also changed");
					sleep (1);
				}
				else if (item == 8)
				{
					DAEMON(Dk).date_flag = atoi (buf);
					INFO(Dk).date_flag = atoi (buf);
					Disp_Msg ("INFO.date_flag also changed");
					sleep (1);
				}
				else if (item == 9)
				{
					DAEMON(Dk).compact_days = atoi (buf);
					INFO(Dk).compact_days = atoi (buf);
					Disp_Msg ("INFO.compact_days also changed");
					sleep (1);
				}
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
	End of Program (py_4030_cm.c)
*************************************************************************/
