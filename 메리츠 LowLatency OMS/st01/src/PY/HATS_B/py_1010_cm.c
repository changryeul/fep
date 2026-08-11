/*------------------------------------------------------------------------
#   Module  : Sequence
#   File	: py_1010_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Dk, Tout;
extern int	Rows;
u_char		SwitchFlag, PageFlag, TimeFlag, MonitorFlag;
char		ChkTime[8], ProcId[12];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_List (char *);
static void		Cu_Job (char *);
int				Chk_Korean (char *, int);
void			Proc_Monitor (void);

/*----------------------------------------------------------------------*/
void	py_1010_cm (char *sub)
/*----------------------------------------------------------------------*/
{
	char	msg[80];

	SwitchFlag = PageFlag = TimeFlag = MonitorFlag = 0;
	Tout = 10;
	Dk = sub[0] - 'a';
	memset (ChkTime, 0, sizeof (ChkTime));

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Mem_SHM (1, Dk);
	Disp_Title ();

	attron (A_BOLD);
	if (memcmp (sub, "a1", 2) == 0)
		mvaddstr (0, 18, "[1010]선물옵션주문체결(PA):주문,응답,체결");
	else if (memcmp (sub, "a5", 2) == 0)
		mvaddstr (0, 18, "[1010]선물옵션주문체결(PA):Auto주문");
	else if (memcmp (sub, "a6", 2) == 0)
		mvaddstr (0, 18, "[1010]선물옵션주문체결(PA):조회");
	else if (memcmp (sub, "a7", 2) == 0)
		mvaddstr (0, 18, "[1010]선물옵션주문체결(PA):시세,한도");
	attroff (A_BOLD);

	if (INFO(Dk).process_id[0] == 0)
	{
		Draw_Line (1, 0, 79);
		sprintf (msg, "P%c daemon not registered.", Dk + 'A');
		Disp_Msg (msg);
		sleep (1);
		refresh ();
		endwin ();
		return;
	}

	Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
	Cu_Job (sub);

	keypad (stdscr, TRUE);

	while (1)
	{
		switch (Trefresh (Tout)) 
		{
			case	0:
			case	7:								/* T:if_seq/tr_e_tm	*/
			case	8:
			case	9:
				Clear_Lines (1, Rows - 2);
				Cu_Job (sub);
				break;
			case	1:
				break;
			case	2:
				Change_File_Cnt ();
				Clear_Lines (1, Rows - 2);
				Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
				Cu_Job (sub);
				break;
			case	3:
				Change_If_Seq ();
				Clear_Lines (1, Rows - 2);
				Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
				Cu_Job (sub);
				break;
			case	4:
				Exit_Process ();
			case	5:
				Clear_Lines (1, Rows - 2);
				Cu_Job (sub);
				break;
			case	6:
				Tout = Change_Interval ();
				Clear_Lines (1, Rows - 2);
				Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
				Cu_Job (sub);
				break;
			case	10:
				if (SwitchFlag == 1)
					Proc_Monitor ();
				Clear_Lines (1, Rows - 2);
				Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
				Cu_Job (sub);
				break;
			default:
				refresh ();
				endwin ();
				return;
		}
	}
}

/*----------------------------------------------------------------------*/
static void		Disp_List (char *sub)
/*----------------------------------------------------------------------*/
{
	int		pk, fk, i, line, x, y, cnt, id_from, id_to, id_comp;
	int		start, end, max_cnt, disp_cnt;
	char	proc_name[20], file_name[20];
	P_FMT	p[128];
	F_FMT	f[128];

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

	if (SwitchFlag == 1)									/* process	*/
	{
		cnt = 0;
		memset (&p, 0, sizeof (p));

		attron (A_UNDERLINE);
		if (TimeFlag == 0)
		{
			mvaddstr (1, 0, "Proc      IfSeq Info                   ");
			mvaddstr (1, 40, "Proc      IfSeq Info                   ");
		}
		else
		{
			mvaddstr (1, 0, "Proc    TR_E_TM Info                   ");
			mvaddstr (1, 40, "Proc    TR_E_TM Info                   ");
		}
		attroff (A_UNDERLINE);

		for (pk = 0; pk < DAEMON(Dk).p_count; pk ++)
		{
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

			if (MonitorFlag == 1 &&
				memcmp (PROC(Dk,pk).process_id, ProcId, 10) == 0)
			{
				attron (A_BLINK);
				MvAddStr (1, 70, 10, ProcId);
				attroff (A_BLINK);

				if (memcmp (PROC(Dk,pk).tr_e_tm, ChkTime, 6) != 0)
					beep ();

				memcpy (ChkTime, PROC(Dk,pk).tr_e_tm, 6);
			}

			if (PROC(Dk,pk).process_status == 1)
				p[cnt].exist_flag = '*';
			else
				p[cnt].exist_flag = ' ';

			if (PROC(Dk,pk).process_no == 0)
				p[cnt].run_flag = 0;
			else
				p[cnt].run_flag = 1;

			memcpy (p[cnt].p_name, PROC(Dk,pk).process_id, 10);
			memcpy (p[cnt].tr_e_tm, PROC(Dk,pk).tr_e_tm, 6);
			p[cnt].seq = IF_SEQ(Dk,pk);
			memcpy (p[cnt].p_info, PROC(Dk,pk).process_info,
				strlen (PROC(Dk,pk).process_info));
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
			if (i == cnt)
				break;

			if (Chk_Korean (p[i].p_info, 23) == -1)
				p[i].p_info[22] = ' ';

			if (TimeFlag == 0)
			{
				if (p[i].run_flag == 0)
				{
					attron (A_BOLD);
					mvprintw (y + 2, x, "%7.7s%c%7d %-23.23s",
						p[i].p_name+3, p[i].exist_flag, p[i].seq, p[i].p_info);
					attroff (A_BOLD);
				}
				else
					mvprintw (y + 2, x, "%7.7s%c%7d %-23.23s",
						p[i].p_name+3, p[i].exist_flag, p[i].seq, p[i].p_info);
			}
			else
			{
				if (p[i].run_flag == 0)
				{
					attron (A_BOLD);
					mvprintw (y + 2, x, "%7.7s%c %6s %-23.23s", p[i].p_name+3,
						p[i].exist_flag, p[i].tr_e_tm, p[i].p_info);
					attroff (A_BOLD);
				}
				else
					mvprintw (y + 2, x, "%7.7s%c %6s %-23.23s", p[i].p_name+3,
						p[i].exist_flag, p[i].tr_e_tm, p[i].p_info);
			}

			y ++;

			if (y == line)
			{
				x += 40;
				y = 0;
			}
		}
	}
	else														/* file	*/
	{
		cnt = 0;
		memset (&f, 0, sizeof (f));

		for (fk = 0; fk < DAEMON(Dk).d_count; fk ++)
		{
			if (strlen (sub) > 1)
			{
				memset (file_name, 0, sizeof (file_name));
				memcpy (file_name, DSHM(Dk,fk).data_name, 10);
				if (strlen (sub) == 6)
					id_comp = AtoIf (DSHM(Dk,fk).data_name+4, 2);
				else if (strlen (sub) == 8)
					id_comp = AtoIf (DSHM(Dk,fk).data_name+5, 3);
				else
					id_comp = AtoIf (DSHM(Dk,fk).data_name+3, 2);

				if ((strlen (sub) == 6 || strlen (sub) == 8) &&
					file_name[3] != sub[1])
					continue;

				if (id_comp < id_from || id_comp > id_to)
					continue;
			}

			f[cnt].f_key = fk;
			memcpy (f[cnt].f_name, DSHM(Dk,fk).data_name, 10);
			f[cnt].sm_rcnt = DSHM(Dk,fk).sm_r_cnt;
			f[cnt].rcnt[0] = DSHM(Dk,fk).r_cnt[0];
			f[cnt].rcnt[1] = DSHM(Dk,fk).r_cnt[1];
			f[cnt].wcnt = DSHM(Dk,fk).w_cnt[0];
			f[cnt].f_cnt = DSHM(Dk,fk).fifo_count;
			f[cnt].rec_sz = DSHM(Dk,fk).data_size;
			memcpy (f[cnt].f_info, DSHM(Dk,fk).info, strlen (DSHM(Dk,fk).info));

			if (f[cnt].wcnt != f[cnt].rcnt[0])
				f[cnt].pile_flag = '*';
			else
				f[cnt].pile_flag = ' ';

			cnt ++;
		}

		for (fk = 0; fk < DAEMON(Dk).f_count; fk ++)
		{
			if (strlen (sub) > 1)
			{
				memset (file_name, 0, sizeof (file_name));
				memcpy (file_name, FILEM(Dk,fk).file_name, 10);
				if (strlen (sub) == 6)
					id_comp = AtoIf (FILEM(Dk,fk).file_name+4, 2);
				else if (strlen (sub) == 8)
					id_comp = AtoIf (FILEM(Dk,fk).file_name+5, 3);
				else
					id_comp = AtoIf (FILEM(Dk,fk).file_name+3, 2);

				if ((strlen (sub) == 6 || strlen (sub) == 8) &&
					file_name[3] != sub[1])
					continue;

				if (id_comp < id_from || id_comp > id_to)
					continue;
			}

			f[cnt].f_key = fk;
			memcpy (f[cnt].f_name, FILEM(Dk,fk).file_name, 10);
			f[cnt].rcnt[0] = FILEM(Dk,fk).r_cnt[0];
			f[cnt].rcnt[1] = FILEM(Dk,fk).r_cnt[1];
			f[cnt].wcnt = FILEM(Dk,fk).w_cnt[0];
			f[cnt].f_cnt = FILEM(Dk,fk).fifo_count;
			f[cnt].rec_sz = FILEM(Dk,fk).record_size;
			memcpy (f[cnt].f_info, FILEM(Dk,fk).file_info,
				strlen (FILEM(Dk,fk).file_info));
			f[cnt].type = 1;							/* 1:SAM file	*/

			if (f[cnt].wcnt != f[cnt].rcnt[0])
				f[cnt].pile_flag = '*';
			else
				f[cnt].pile_flag = ' ';

			cnt ++;
		}

		if (cnt <= 54)
		{
			y = 0;
			max_cnt = Rows - 3;

			attron (A_UNDERLINE);
			mvaddstr (1, 0,"File/DSHM    W1      SM      R1      R2   FK FC Size Info[FK:FileKey FC:FifoCnt]");
			attroff (A_UNDERLINE);

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

			for (i = start; i < end; i ++)
			{
				if (i == cnt)
					break;

				if (Chk_Korean (f[i].f_info, 27) == -1)
					f[i].f_info[26] = ' ';

				if (f[i].pile_flag == '*')
					attron (A_BOLD);

				if (f[i].type == 0)
					mvprintw (y + 2, 0,
						"%7.7s %7d %7d %7d%c%7d %4d  %d %4d %-27.27s",
						f[i].f_name+3, f[i].wcnt, f[i].sm_rcnt, f[i].rcnt[0],
						f[i].pile_flag, f[i].rcnt[1], f[i].f_key, f[i].f_cnt,
						f[i].rec_sz, f[i].f_info);
				else
					mvprintw (y + 2, 0,
						"%7.7s %7d       - %7d%c%7d %4d  %d %4d %-27.27s",
						f[i].f_name+3, f[i].wcnt, f[i].rcnt[0],
						f[i].pile_flag, f[i].rcnt[1], f[i].f_key, f[i].f_cnt,
						f[i].rec_sz, f[i].f_info);

				if (f[i].pile_flag == '*')
					attroff (A_BOLD);

				y ++;
			}
		}
		else
		{
			x = y = 0;
			max_cnt = (Rows - 3) * 2;

			attron (A_UNDERLINE);
			mvaddstr (1, 0, "File           W1     R1     R2     R3");
			mvaddstr (1, 42, "File           W1     R1     R2     R3");
			attroff (A_UNDERLINE);

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
				if (i == cnt)
					break;

				if (f[i].pile_flag == '*')
					attron (A_BOLD);

				mvprintw (y + 2, x, "%10.10s %6d %6d%c%6d %6d",
					f[i].f_name, f[i].wcnt, f[i].rcnt[0], f[i].pile_flag,
					f[i].rcnt[1], f[i].rcnt[2]);

				if (f[i].pile_flag == '*')
					attroff (A_BOLD);

				y ++;

				if (y == line)
				{
					x += 42;
					y = 0;
				}
			}
		}
	}

	mvdelch (Rows - 1, 79);

	return;
}

/*----------------------------------------------------------------------*/
static void		Cu_Job (char *sub)
/*----------------------------------------------------------------------*/
{
	G_Time ();
	Disp_List (sub);
	refresh ();
}

/*------------------------------------------------------------------------
	Check korean/korean half, alpahnumeric
	Return	-1 = korean start
			 1 = korean end, space (0x20) or uppercase (A ~ Z)
			 0 = alpahnumeric (0 ~ 9, a ~ z)
------------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
int		Chk_Korean (char *str, int idx)
/*----------------------------------------------------------------------*/
{
	int		i, flag;
	char	ch;
	
	i = flag = 0;

	if (idx < 0) 
		return (flag);

	ch = *(str+idx);

	if (!(ch & 0x80))
		return (flag);

	while (i <= idx)
	{ 
		if (*str & 0x80)
			flag ^= 0x01;
		str ++;
		i ++;
	}

	if (flag)
		return (1);
	else
		return (-1);
}

/*----------------------------------------------------------------------*/
void	Proc_Monitor (void)
/*----------------------------------------------------------------------*/
{
	int 	pk, rt;
	char	buf[12];

	Clear_Lines (Rows - 2, Rows - 2);

	attron (A_BOLD);
	mvaddstr (Rows - 2, 7,
		"process:            (beeps when tr_e_tm has been changed)");
	attroff (A_BOLD);

	Draw_Underline (Rows - 2, 16, 10);
	Disp_Msg ("Input <process ID> (e.g. pa_1111_ts). (Esc:Cancel)");

	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 2, 16, 10);
		rt = Get_String (Rows - 2, 16, 10, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <process ID> (e.g. pa_1111_ts). (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 10 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
		{
			MonitorFlag = 0;
			return;
		}
		else
		{
			sprintf (ProcId, "%.10s", buf);
			break;
		}
	}

	for (pk = 0; pk < DAEMON(Dk).p_count; pk ++)
	{
		if (memcmp (PROC(Dk,pk).process_id, ProcId, 10) == 0)
		{
			MonitorFlag = 1;
			break;
		}

		if (pk == DAEMON(Dk).p_count - 1)
		{
			MonitorFlag = 0;
			Disp_Msg ("▶▶▶ Unregistered process ◀◀◀");
			sleep (1);
			return;
		}
	}

	sleep (1);

	return;
}

/*************************************************************************
	End of Program (py_1010_cm.c)
*************************************************************************/
