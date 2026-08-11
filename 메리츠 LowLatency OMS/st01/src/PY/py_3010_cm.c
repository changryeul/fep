/*------------------------------------------------------------------------
#   Module  : Process Status
#	File	: py_3010_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
typedef struct {
	char	p_name[12];
	int		pk;
}	P_LIST;

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Dk;
extern int	Rows;
u_char		DisplayFlag, PageFlag;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Data (char *);
static void		Disp_Msg_2 (char *);

/*----------------------------------------------------------------------*/
void	py_3010_cm (char *sub)
/*----------------------------------------------------------------------*/
{
	int		job_end;
	char	msg[80];

	DisplayFlag = PageFlag = 0;
	job_end = 1;
	Dk = sub[0] - 'a';

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Mem_SHM (1, Dk);
	Disp_Title ();

	attron (A_BOLD);
	switch (sub[0])
	{
		case	'a':
			if (memcmp (sub, "a10", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PA):주문,응답,체결");
			else if (memcmp (sub, "b10", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PB):주문,응답,체결");
			else if (memcmp (sub, "a50", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PA):Auto주문");
			else if (memcmp (sub, "b50", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PB):Auto주문");
			else if (memcmp (sub+1, "60", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PA):조회");
			else if (memcmp (sub+1, "70", 2) == 0)
				mvaddstr (0, 18, "[3010]선물옵션주문체결(PA):시세,한도");
			break;
		case	'w':	mvaddstr (0, 18, "[3010]업무접속 (PW)");	break;
		default:	break;
	}
	attroff (A_BOLD);

	if (INFO(sub[0]-'a').process_id[0] == 0)
	{
		Draw_Line (1, 0, 79);
		sprintf (msg, "P%c daemon not registered.", sub[0] - 32);
		Disp_Msg (msg);
		sleep (1);
		refresh ();
		endwin ();
		return;
	}

	Disp_Data (sub);
	Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);
		noecho ();

		switch (getch ())
		{
			case	'h':
			case	'H':										/* Help	*/
				Clear_Lines (1, Rows - 2);
				Help_Proc (1);
				break;
			case	KEY_UP:
				PageFlag = 0;
				Clear_Lines (1, Rows - 2);
				Disp_Data (sub);
				Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
				break;
			case	KEY_DOWN:
				PageFlag = 1;
				Clear_Lines (1, Rows - 2);
				Disp_Data (sub);
				Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
				break;
			case 	KEY_ESC:
			case 	KEY_LEFT:
			case 	KEY_RIGHT:
				job_end = 0;
				refresh ();
				endwin ();
				break;
			case	'q':
			case	'Q':										/* Quit	*/
				Exit_Process ();
			case	'a':
			case	'A':
				DisplayFlag = (DisplayFlag + 1) % 2;
				Clear_Lines (1, Rows - 2);
				Disp_Data (sub);
				Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
				break;
			case	'\n':									/* Retry	*/
				Clear_Lines (1, Rows - 2);
				Disp_Data (sub);
				Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
				break;
			default:
				Clear_Lines (1, Rows - 2);
				Disp_Msg_2 ("You entered wrong key.");
				sleep (1);
				Disp_Msg_2 ("Enter:Retry H:Help A:Stat/Time Q:Quit");
				break;
		}
	}
	refresh ();
	endwin ();

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Data (char *sub)
/*----------------------------------------------------------------------*/
{
	int		i, x, y, tot_cnt, cnt, cnt_run, cnt_not_run, pk, sk;
	int		id_from, id_to, id_comp, start, end, max_cnt, disp_cnt, line;
	pid_t	pid;
	char	msg[80], exist_flag, proc_name[20];
	P_LIST	p[128];

	cnt = cnt_run = cnt_not_run = tot_cnt = 0;
	memset (p, 0, sizeof (p));

	G_Time ();

	attron (A_UNDERLINE);
	if (DisplayFlag == 0)
	{
		mvaddstr (1, 0, "       PID CMD       e p s P L N l TOUT");
		mvaddstr (1, 41, "       PID CMD       e p s P L N l TOUT");
	}
	else
	{
		mvaddstr (1, 0, "       PID CMD       e p STM  ETM  DLAY");
		mvaddstr (1, 41, "       PID CMD       e p STM  ETM  DLAY");
	}
	attroff (A_UNDERLINE);

	if (strlen (sub) > 1)
	{
		if (strlen (sub) == 6)
		{
			id_from = AtoIf (sub+2, 2);
			id_to = AtoIf (sub+4, 2);
		}
		else if (strlen (sub) == 8)
		{
			id_from = AtoIf (sub+2, 3);
			id_to = AtoIf (sub+5, 3);
		}
		else
		{
			id_from = AtoIf (sub+1, 2);
			id_to = AtoIf (sub+3, 2);
		}
	}

	for (pk = 0; pk < DAEMON(Dk).p_count; pk ++)
	{
		tot_cnt ++;

		if (strlen (sub) > 1)
		{
			memset (proc_name, 0, sizeof (proc_name));
			memcpy (proc_name, PROC(Dk,pk).process_id, 10);

			if ((strlen (sub) == 6 || strlen (sub) == 8) &&
				proc_name[3] != sub[1])
				continue;

			if (strlen (sub) == 6)
				id_comp = AtoIf (PROC(Dk,pk).process_id+4, 2);
			else if (strlen (sub) == 8)
				id_comp = AtoIf (PROC(Dk,pk).process_id+5, 3);
			else
				id_comp = AtoIf (PROC(Dk,pk).process_id+3, 2);

			if (id_comp < id_from || id_comp > id_to)
				continue;
		}

		memcpy (p[cnt].p_name, PROC(Dk,pk).process_id, 10);
		p[cnt].pk = pk;
		cnt ++;
	}

	x = y = 0;
	max_cnt = (Rows - 3) * 2;

	attron (A_REVERSE);

	if (PageFlag == 0)
	{
		mvaddstr (0, 73, "↓:next");

		if (cnt > max_cnt)
			disp_cnt = max_cnt;
		else
			disp_cnt = cnt;

		start = 0;
		end = disp_cnt;
	}
	else
	{
		mvaddstr (0, 73, "↑:prev");

		if (cnt > max_cnt)
		{
			disp_cnt = cnt - max_cnt;
			start = max_cnt;
			end = cnt;
		}
		else
			start = end = 0;
	}

	attroff (A_REVERSE);

	line = disp_cnt / 2;
	if (disp_cnt % 2 != 0)
		line ++;

	for (i = start; i < end; i ++)
	{
		memset (msg, 0, sizeof msg);

		pid = Check_Proc (p[i].p_name);
		if (pid == 0)
			exist_flag = ' ';
		else
			exist_flag = '*';

		if (DisplayFlag == 0)
		{
			if (PROC(Dk,p[i].pk).type == TY_TRS1)
			{
				sprintf (msg, "%10d %-10s%c %d %d - - %d - %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_status,
					TCP1_NSTAT(Dk,p[i].pk), PROC(Dk,p[i].pk).timeout);
			}
			else if (PROC(Dk,p[i].pk).type == TY_TRS2)
			{
				sk = TCP2_LINE_GUBUN(Dk,p[i].pk);
				sprintf (msg, "%10d %-10s%c %d %d %d %d %d %d %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_status,
					TCP2_PSTAT(Dk,p[i].pk,sk), TCP2_LSTAT(Dk,p[i].pk,sk),
					TCP2_NSTAT(Dk,p[i].pk,sk), sk, PROC(Dk,p[i].pk).timeout);
			}
			else
				sprintf (msg, "%10d %-10s%c %d %d - - - - %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_status, PROC(Dk,p[i].pk).timeout);
		}
		else
		{
			if (PROC(Dk,p[i].pk).type == TY_TRS1)
			{
				sprintf (msg, "%10d %-10s%c %d %4.4s %4.4s %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_time, PROC(Dk,p[i].pk).end_time,
					PROC(Dk,p[i].pk).delay);
			}
			else if (PROC(Dk,p[i].pk).type == TY_TRS2)
			{
				sprintf (msg, "%10d %-10s%c %d %4.4s %4.4s %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_time, PROC(Dk,p[i].pk).end_time,
					PROC(Dk,p[i].pk).delay);
			}
			else
				sprintf (msg, "%10d %-10s%c %d %4.4s %4.4s %4d",
					pid, PROC(Dk,p[i].pk).process_id, exist_flag,
					PROC(Dk,p[i].pk).process_status,
					PROC(Dk,p[i].pk).start_time, PROC(Dk,p[i].pk).end_time,
					PROC(Dk,p[i].pk).delay);
		}

		if (PROC(Dk,p[i].pk).process_no == 0)
		{
			attron (A_BOLD);
			mvaddstr (y + 2, x, msg);
			attroff (A_BOLD);
			cnt_not_run ++;
		}
		else
		{
			mvaddstr (y + 2, x, msg);
			cnt_run ++;
		}
		y ++;

		if (y == line)
		{
			y = 0;
			x += 41;
		}
	}

	memset (msg, 0, sizeof msg);
	sprintf (msg, "run:%2d,", cnt_run);
	mvaddstr (Rows - 1, 51, msg);

	memset (msg, 0, sizeof msg);
	sprintf (msg, "stop:%2d,", cnt_not_run);
	attron (A_BOLD);
	mvaddstr (Rows - 1, 58, msg);
	attroff (A_BOLD);

	memset (msg, 0, sizeof msg);
	sprintf (msg, "total:%2d(%3d)", cnt, tot_cnt);
	attron (A_UNDERLINE);
	mvaddstr (Rows - 1, 66, msg);
	attroff (A_UNDERLINE);

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Msg_2 (char *msg)
/*----------------------------------------------------------------------*/
{
	int		i;

	attron (A_REVERSE);
    for (i = 0; i < 50; i ++)
	{
		mvaddch (Rows - 1, i, ' ');
	}
	mvaddstr (Rows - 1, 0, "☞ ");
	mvaddstr (Rows - 1, 3, msg);
	attroff (A_REVERSE);
	refresh ();

	return;
}	/* End of Disp_Msg_2 ()	*/

/*************************************************************************
	End of Program (py_3010_cm.c)
*************************************************************************/
