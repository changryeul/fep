/*------------------------------------------------------------------------
#   Module  : Shared Memory - Process Info
#   File	: py_4010_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int			Dk, Pk;
extern int	Rows;
char		ProcName[20];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
void			Info_Proc (void);
void			Disp_Proc (void);
void			Change_Proc (void);

/*----------------------------------------------------------------------*/
void	py_4010_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;

	Dk = Pk = 0;
	job_end = 1;
	memset (ProcName, 0, sizeof (ProcName));

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Mem_SHM (0, -1);
	Info_Proc ();

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
				Info_Proc ();
				break;
			case	'c':
			case	'C':									/* Change	*/
				Change_Proc ();
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
				Disp_Proc ();
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
    mvaddstr (0, 18, "[4010] Process Info");
	attroff (A_BOLD);
    mvaddstr (1, 2,
		"process_id             D_K     P_K      type    process_status");
	Draw_Underline (1, 13, 10);

	attron (A_BOLD);
    mvaddstr (9, 0, "[FILE]");
    mvaddstr (9, 10, "FN         FS  FC");
    mvaddstr (9, 36, "FN         FS  FC");
    mvaddstr (9, 62, "FN         FS  FC");
	attroff (A_BOLD);

	mvaddstr (2, 2,
		"start_status    process_no          start_time       end_time");
	mvaddstr (3, 2, "                               out_d");
	mvaddstr (4, 2,
"process_type      if_seq           date           timeout       data_flag");
	mvaddstr (5, 2,
"tr_s_tm         tr_e_tm         error_cd       error_tm         data_cnt");
	mvaddstr (6, 2,
		"process_info                                           in_FIFO_fd");
	mvaddstr (7, 2,
		"fifo_f                                                       in_d");
	mvaddstr (8, 2, "counter_seq                       delay");

	mvaddstr (10, 3, "IF1                       IF2                       IF3");
#if defined ISAM_INCL
	mvaddstr (11, 3, "IC1                       IC2                       IC3");
	mvaddstr (12, 3, "OC1                       OC2                       OC3");
#endif
	mvaddstr (13, 3, "OF1                       OF2                       OF3");
	mvaddstr (14, 3, "OF4                       OF5                       OF6");
	mvaddstr (15, 3, "OF7                       OF8                       OF9");

	return;
}

/*----------------------------------------------------------------------*/
void	Info_Proc (void)
/*----------------------------------------------------------------------*/
{
	char	buf[80];
	int 	rt;

	Clear_Lines (1, Rows - 2);
	Disp_Scr ();

	Disp_Msg ("Input <Process ID>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (1, 13, 10);
		rt = Get_String (1, 13, 10, buf);
		
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Process ID>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 10 characters only. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			if (strlen (buf) != 10)
			{
				Disp_Msg ("Input 10 characters only. (Esc:Cancel)");
				continue;
			}

			memcpy (ProcName, buf, 10);
			UtoL( ProcName, 10 );
			Dk = ProcName[1] - 'a';

			if (INFO(Dk).process_id[0] == 0)
			{
				sprintf (buf,
					"P%c daemon not registered. (Esc:Cancel)", Dk + 'A');
				Disp_Msg (buf);
			}
			else
			{
				for (Pk = 0; Pk < DAEMON(Dk).p_count; Pk ++) 
				{
					if (memcmp (ProcName, PROC(Dk,Pk).process_id, 10) == 0)
					{
						Disp_Proc ();
						return;
					}

					if (Pk == DAEMON(Dk).p_count - 1)
					{
						Dk = Pk = 0;
						sprintf (buf,
							"%s not registered. (Esc:Cancel)", ProcName);
						Disp_Msg (buf);
						break;
					}
				}
 			}
		}
	}

	return;
}

/*----------------------------------------------------------------------*/
void	Disp_Proc (void)
/*----------------------------------------------------------------------*/
{
	int		i;

	G_Time ();

	if (PROC(Dk,Pk).type == TY_TRS1)						/* TCP 1	*/
	{
		attron (A_BOLD);
		mvaddstr (17, 0, "[TCP1]");
		attroff (A_BOLD);

		mvaddstr (17, 9,
			"LG   PT   NS     PORT        IP    .   .   .     P_ST");
		mvaddstr (18, 9, "S_ST");
		mvaddstr (19, 9, "SPORT");
		mvaddstr (20, 9, "S_CT");
		mvaddstr (21, 9, "client ip                       port");
	}
	else if (PROC(Dk,Pk).type == TY_TRS2)					/* TCP 2	*/
	{
		attron (A_BOLD);
		mvaddstr (17, 0, "[TCP2]");
		mvaddstr (17, 11, "ID  PORT IP              P L N");
		attroff (A_BOLD);

		mvaddstr (17, 47, "l.t2.line_gubun    l.t2.l");
		mvaddstr (18, 7, "P               .   .   .");
		mvaddstr (19, 7, "B               .   .   .");
	}
	else if (PROC(Dk,Pk).type == TY_URS)					/* UDPIP	*/
	{
		attron (A_BOLD);
		mvaddstr (17, 0, "[UDP]");
		mvaddstr (17, 7, "IP              PORT");
		attroff (A_BOLD);
	}

	attron (A_UNDERLINE);
	MvAddStr (1, 13, 10, PROC(Dk,Pk).process_id);
	MvAddNum (1, 29, 2, Dk);
	MvAddNum (1, 37, 3, Pk);
	MvAddNum (1, 47, 1, PROC(Dk,Pk).type);
	MvAddNum (1, 65, 1, PROC(Dk,Pk).process_status);

	MvAddNum (2, 15, 1, PROC(Dk,Pk).start_status);
	MvAddNum (2, 29, 7, PROC(Dk,Pk).process_no);
	MvAddStr (2, 49, 4, PROC(Dk,Pk).start_time);
	MvAddStr (2, 64, 4, PROC(Dk,Pk).end_time);

	MvAddNum (3, 39, 3, PROC(Dk,Pk).out_d[0]);
	MvAddNum (3, 43, 3, PROC(Dk,Pk).out_d[1]);
	MvAddNum (3, 47, 3, PROC(Dk,Pk).out_d[2]);
	MvAddNum (3, 51, 3, PROC(Dk,Pk).out_d[3]);
	MvAddNum (3, 55, 3, PROC(Dk,Pk).out_d[4]);
	MvAddNum (3, 59, 3, PROC(Dk,Pk).out_d[5]);
	MvAddNum (3, 63, 3, PROC(Dk,Pk).out_d[6]);
	MvAddNum (3, 67, 3, PROC(Dk,Pk).out_d[7]);
	MvAddNum (3, 71, 3, PROC(Dk,Pk).out_d[8]);

	MvAddStr (4, 15, 3, PROC(Dk,Pk).process_type);
	MvAddNum (4, 27, 8, PROC(Dk,Pk).if_seq);
	MvAddStr (4, 42, 8, PROC(Dk,Pk).date);
	MvAddNum (4, 60, 4, PROC(Dk,Pk).timeout);
	MvAddNum (4, 76, 1, PROC(Dk,Pk).data_flag);

	MvAddStr (5, 10, 6, PROC(Dk,Pk).tr_s_tm);
	MvAddStr (5, 26, 6, PROC(Dk,Pk).tr_e_tm);
	MvAddStr (5, 43, 4, PROC(Dk,Pk).error_cd);
	MvAddStr (5, 58, 6, PROC(Dk,Pk).error_tm);
	MvAddNum (5, 75, 3, PROC(Dk,Pk).data_cnt);

	MvAddStr (6, 15, 40, PROC(Dk,Pk).process_info);
	MvAddNum (6, 68, 3, IFIFD(Dk,Pk,0));
	MvAddNum (6, 72, 3, IFIFD(Dk,Pk,1));
	MvAddNum (6, 76, 3, IFIFD(Dk,Pk,2));

	MvAddStr (7, 9, 11, PROC(Dk,Pk).fifo_f[0]);
	MvAddStr (7, 21, 11, PROC(Dk,Pk).fifo_f[1]);
	MvAddStr (7, 33, 11, PROC(Dk,Pk).fifo_f[2]);
	MvAddNum (7, 68, 3, PROC(Dk,Pk).in_d[0]);
	MvAddNum (7, 72, 3, PROC(Dk,Pk).in_d[1]);
	MvAddNum (7, 76, 3, PROC(Dk,Pk).in_d[2]);

	MvAddNum (8, 14, 8, PROC(Dk,Pk).counter_seq);
	MvAddNum (8, 42, 4, PROC(Dk,Pk).delay);

	for (i = 0; i < 3; i ++)
	{
		if (PROC(Dk,Pk).in_d[i] != 0)
		{
			MvAddNum (10, 7 + i * 26, 2, PROC(Dk,Pk).in_d[i]);
			MvAddStr (10, 10 + i * 26, 10, IDN(Dk,Pk,i));
			MvAddNum (10, 21 + i * 26, 4, IDS(Dk,Pk,i));
			MvAddNum (10, 26 + i * 26, 1, IDC(Dk,Pk,i));
		}
		else if (PROC(Dk,Pk).in_f[i] != 0)
		{
			MvAddNum (10, 7 + i * 26, 2, PROC(Dk,Pk).in_f[i]);
			MvAddStr (10, 10 + i * 26, 10, IFN(Dk,Pk,i));
			MvAddNum (10, 21 + i * 26, 4, IFS(Dk,Pk,i));
			MvAddNum (10, 26 + i * 26, 1, IFC(Dk,Pk,i));
		}

#if defined ISAM_INCL
		if (PROC(Dk,Pk).in_c[i] != 0)
		{
			MvAddNum (11, 7 + i * 26, 2, PROC(Dk,Pk).in_c[i]);
			MvAddStr (11, 10 + i * 26, 10, ICN(Dk,Pk,i));
			MvAddNum (11, 21 + i * 26, 4, ICS(Dk,Pk,i));
		}

		if (PROC(Dk,Pk).out_c[i] != 0)
		{
			MvAddNum (12, 7 + i * 26, 2, PROC(Dk,Pk).out_c[i]);
			MvAddStr (12, 10 + i * 26, 10, OCN(Dk,Pk,i));
			MvAddNum (12, 21 + i * 26, 4, OCS(Dk,Pk,i));
		}
#endif
	}

	for (i = 0; i < 9; i ++)
	{
		if (PROC(Dk,Pk).out_d[i] != 0)
		{
			MvAddNum (13 + i / 3, 7 + (i % 3) * 26, 2, PROC(Dk,Pk).out_d[i]);
			MvAddStr (13 + i / 3, 10 + (i % 3) * 26, 10, ODN(Dk,Pk,i));
			MvAddNum (13 + i / 3, 21 + (i % 3) * 26, 4, ODS(Dk,Pk,i));
			MvAddNum (13 + i / 3, 26 + (i % 3) * 26, 1, ODC(Dk,Pk,i));
		}
		else if (PROC(Dk,Pk).out_f[i] != 0)
		{
			MvAddNum (13 + i / 3, 7 + (i % 3) * 26, 2, PROC(Dk,Pk).out_f[i]);
			MvAddStr (13 + i / 3, 10 + (i % 3) * 26, 10, OFN(Dk,Pk,i));
			MvAddNum (13 + i / 3, 21 + (i % 3) * 26, 4, OFS(Dk,Pk,i));
			MvAddNum (13 + i / 3, 26 + (i % 3) * 26, 1, OFC(Dk,Pk,i));
		}
	}

	if (PROC(Dk,Pk).type == TY_TRS1)						/* TCP 1	*/
	{
		MvAddNum (17, 12, 1, PROC(Dk,Pk).l.t1.line_gubun);
		MvAddNum (17, 17, 1, PROC(Dk,Pk).l.t1.port_type);
		MvAddNum (17, 22, 1, PROC(Dk,Pk).l.t1.network_status);

		if (PROC(Dk,Pk).l.t1.line_gubun == 1 ||
			PROC(Dk,Pk).l.t1.line_gubun == 2)
		{
			if (PROC(Dk,Pk).l.t1.port_type == 0)			/* master	*/
			{
				MvAddNum (17, 31, 5, TCP1_PORT(Dk,Pk));
				MvAddNum (17, 41, 3, TCP1_IP(Dk,Pk,0));
				MvAddNum (17, 45, 3, TCP1_IP(Dk,Pk,1));
				MvAddNum (17, 49, 3, TCP1_IP(Dk,Pk,2));
				MvAddNum (17, 53, 3, TCP1_IP(Dk,Pk,3));
				MvAddNum (17, 63, 1, TCP1_P_ST(Dk,Pk));
				MvAddNum (18, 14, 1, TCP1_S_ST(Dk,Pk,0));
				MvAddNum (18, 16, 1, TCP1_S_ST(Dk,Pk,1));
				MvAddNum (18, 18, 1, TCP1_S_ST(Dk,Pk,2));
				MvAddNum (18, 20, 1, TCP1_S_ST(Dk,Pk,3));
				MvAddNum (18, 22, 1, TCP1_S_ST(Dk,Pk,4));
				MvAddNum (18, 24, 1, TCP1_S_ST(Dk,Pk,5));
				MvAddNum (18, 26, 1, TCP1_S_ST(Dk,Pk,6));
				MvAddNum (18, 28, 1, TCP1_S_ST(Dk,Pk,7));
				MvAddNum (18, 30, 1, TCP1_S_ST(Dk,Pk,8));
				MvAddNum (19, 15, 5, TCP1_SPORT(Dk,Pk,0));
				MvAddNum (19, 21, 5, TCP1_SPORT(Dk,Pk,1));
				MvAddNum (19, 27, 5, TCP1_SPORT(Dk,Pk,2));
				MvAddNum (19, 33, 5, TCP1_SPORT(Dk,Pk,3));
				MvAddNum (19, 39, 5, TCP1_SPORT(Dk,Pk,4));
				MvAddNum (19, 45, 5, TCP1_SPORT(Dk,Pk,5));
				MvAddNum (19, 51, 5, TCP1_SPORT(Dk,Pk,6));
				MvAddNum (19, 57, 5, TCP1_SPORT(Dk,Pk,7));
				MvAddNum (19, 63, 5, TCP1_SPORT(Dk,Pk,8));
				MvAddNum (20, 14, 5, TCP1_S_CT(Dk,Pk,0));
				MvAddNum (20, 20, 5, TCP1_S_CT(Dk,Pk,1));
				MvAddNum (20, 26, 5, TCP1_S_CT(Dk,Pk,2));
				MvAddNum (20, 32, 5, TCP1_S_CT(Dk,Pk,3));
				MvAddNum (20, 38, 5, TCP1_S_CT(Dk,Pk,4));
				MvAddNum (20, 44, 5, TCP1_S_CT(Dk,Pk,5));
				MvAddNum (20, 50, 5, TCP1_S_CT(Dk,Pk,6));
				MvAddNum (20, 56, 5, TCP1_S_CT(Dk,Pk,7));
				MvAddNum (20, 62, 5, TCP1_S_CT(Dk,Pk,8));
			}
			else
			{
				if (PROC(Dk,Pk).process_status == 1)		/* service	*/
				{
					MvAddNum (17, 63, 1, TCP1_P_ST(Dk,Pk));
					MvAddNum (18, 14, 1,
						TCP1_S_ST(Dk,Pk,PROC(Dk,Pk).l.t1.port_type-1));
					MvAddNum (19, 15, 5,
						TCP1_SPORT(Dk,Pk,PROC(Dk,Pk).l.t1.port_type-1));
					MvAddNum (20, 14, 5,
						TCP1_S_CT(Dk,Pk,PROC(Dk,Pk).l.t1.port_type-1));
				}
				else
				{
					if (PROC(Dk,Pk).l.t1.line_gubun != 0)
						MvAddNum (19, 15, 5,
							TCP1('w'-'a',PROC(Dk,Pk).l.t1.line_gubun-1).
							service_port_no[PROC(Dk,Pk).l.t1.port_type-1]);

					MvAddStr (21, 19, 15, PROC(Dk,Pk).ip);
					MvAddNum (21, 46, 5, PROC(Dk,Pk).port);
				}
			}
		}
	}
	else if (PROC(Dk,Pk).type == TY_TRS2)					/* TCP 2	*/
	{
		if (PROC(Dk,Pk).l.t2.l[0] != 0)
		{
			MvAddNum (17, 63, 1, TCP2_LINE_GUBUN(Dk,Pk));
			MvAddNum (17, 73, 3, PROC(Dk,Pk).l.t2.l[0]);
			MvAddNum (18, 10, 3, TCP2_ID(Dk,Pk,0));
			MvAddNum (18, 14, 5, TCP2_PORT(Dk,Pk,0));
			mvaddstr (18, 20, "   .   .   .");
			MvAddNum (18, 20, 3, TCP2_IP1(Dk,Pk,0));
			MvAddNum (18, 24, 3, TCP2_IP2(Dk,Pk,0));
			MvAddNum (18, 28, 3, TCP2_IP3(Dk,Pk,0));
			MvAddNum (18, 32, 3, TCP2_IP4(Dk,Pk,0));
			MvAddNum (18, 36, 1, TCP2_PSTAT(Dk,Pk,0));
			MvAddNum (18, 38, 1, TCP2_LSTAT(Dk,Pk,0));
			MvAddNum (18, 40, 1, TCP2_NSTAT(Dk,Pk,0));
		}

		if (PROC(Dk,Pk).l.t2.l[1] != 0)
		{
			MvAddNum (17, 77, 3, PROC(Dk,Pk).l.t2.l[1]);
			MvAddNum (19, 10, 3, TCP2_ID(Dk,Pk,1));
			MvAddNum (19, 14, 5, TCP2_PORT(Dk,Pk,1));
			mvaddstr (19, 20, "   .   .   .");
			MvAddNum (19, 20, 3, TCP2_IP1(Dk,Pk,1));
			MvAddNum (19, 24, 3, TCP2_IP2(Dk,Pk,1));
			MvAddNum (19, 28, 3, TCP2_IP3(Dk,Pk,1));
			MvAddNum (19, 32, 3, TCP2_IP4(Dk,Pk,1));
			MvAddNum (19, 36, 1, TCP2_PSTAT(Dk,Pk,1));
			MvAddNum (19, 38, 1, TCP2_LSTAT(Dk,Pk,1));
			MvAddNum (19, 40, 1, TCP2_NSTAT(Dk,Pk,1));
		}
	}
	else if (PROC(Dk,Pk).type == TY_URS)					/* UDPIP	*/
	{
		for (i = 0; i < 5; i ++)
		{
			if(UDP_PORT(Dk,Pk,i) != 0)
			{
				mvaddstr (18+i, 7, "   .   .   .   :");
				MvAddNum (18+i, 7, 3, UDP_IP1(Dk,Pk,i));
				MvAddNum (18+i, 11, 3, UDP_IP2(Dk,Pk,i));
				MvAddNum (18+i, 15, 3, UDP_IP3(Dk,Pk,i));
				MvAddNum (18+i, 19, 3, UDP_IP4(Dk,Pk,i));
				MvAddNum (18+i, 23, 5, UDP_PORT(Dk,Pk,i));
			}
		}
	}

	attroff (A_UNDERLINE);

	return;
}

/*----------------------------------------------------------------------*/
void	Change_Proc (void)
/*----------------------------------------------------------------------*/
{
	char	buf[20];
	int 	i, rt, item, x, y, l, cnt;
	u_long	ul;

	cnt = 0;

	attron (A_REVERSE);
	mvaddstr (1, 49, "1");
	mvaddstr (2, 1, "2");
	mvaddstr (2, 17, "3");
	mvaddstr (2, 37, "4");
	mvaddstr (2, 54, "5");
	mvaddstr (4, 19, "6");
	mvaddstr (4, 36, "7");
	mvaddstr (4, 51, "8");

	if (PROC(Dk,Pk).type == TY_TRS2)
	{
		mvaddstr (17, 46, "9");
		mvaddstr (18, 8, "10");
		mvaddstr (19, 8, "11");
	}
	else if (PROC(Dk,Pk).type == TY_URS)
	{
		for (i = 0; i < 5; i ++)
		{
			if(UDP_PORT(Dk,Pk,i) != 0)
				MvAddNum (18+i, 5, 2, 9+i);
			else
			{
				MvAddNum (18+i, 5, 2, 9+i);
				break;
			}
		}
	}

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
				x = 1;	y= 65;	l = 1;
			}
			else if (item == 2)
			{
				x = 2;	y= 15;	l = 1;
			}
			else if (item == 3)
			{
				x = 2;	y= 29;	l = 7;
			}
			else if (item == 4)
			{
				x = 2;	y= 49;	l = 4;
			}
			else if (item == 5)
			{
				x = 2;	y= 64;	l = 4;
			}
			else if (item == 6)
			{
				x = 4;	y= 27;	l = 8;
			}
			else if (item == 7)
			{
				x = 4;	y= 42;	l = 8;
			}
			else if (item == 8)
			{
				x = 4;	y= 60;	l = 4;
			}
			else if (item == 9 && PROC(Dk,Pk).type == TY_TRS2)
			{
				x = 17;	y= 63;	l = 1;
			}
			else if (item == 10 && PROC(Dk,Pk).type == TY_TRS2)
			{
				x = 18;	y= 14;	l = 5;
			}
			else if (item == 11 && PROC(Dk,Pk).type == TY_TRS2)
			{
				x = 19;	y= 15;	l = 5;
			}
			else if (item == 9 && PROC(Dk,Pk).type == TY_URS)
			{
				x = 18;	y= 7;	l = 15;
			}
			else if (item == 10 && PROC(Dk,Pk).type == TY_URS)
			{
				x = 19;	y= 7;	l = 15;
			}
			else if (item == 11 && PROC(Dk,Pk).type == TY_URS)
			{
				x = 20;	y= 7;	l = 15;
			}
			else if (item == 12 && PROC(Dk,Pk).type == TY_URS)
			{
				x = 21;	y= 7;	l = 15;
			}
			else if (item == 13 && PROC(Dk,Pk).type == TY_URS)
			{
				x = 22;	y= 7;	l = 15;
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
		if (PROC(Dk,Pk).type != TY_URS)
		{
			if ((item == 10 || item == 11) && cnt >= 2)
				break;
		}
		else
		{
			if ((item >= 9 && item <= 13) && cnt >= 2)
				break;
		}

		memset (&buf, 0, sizeof buf);

		Draw_Underline (x, y, l);
		rt = Get_String (x, y, l, buf);

		if (PROC(Dk,Pk).type != TY_URS)
		{
			if (item == 10 || item == 11)
			{
				y += (l + 1);
				l = 15;
				cnt ++;
			}
		}
		else
		{
			if (item >= 9 && item <= 13)
			{
				y += (l + 1);
				l = 5;
				cnt ++;
			}
		}

		if (rt == -1)											/* skip	*/
		{
			if (PROC(Dk,Pk).type != TY_URS)
			{
				if (item == 10)
				{
					mvaddstr (18, 20, "   .   .   .");
					attron (A_UNDERLINE);
					if (cnt == 1)
						MvAddNum (18, 14, 5, TCP2_PORT(Dk,Pk,0));
					else if (cnt == 2)
					{
						MvAddNum (18, 20, 3, TCP2_IP1(Dk,Pk,0));
						MvAddNum (18, 24, 3, TCP2_IP2(Dk,Pk,0));
						MvAddNum (18, 28, 3, TCP2_IP3(Dk,Pk,0));
						MvAddNum (18, 32, 3, TCP2_IP4(Dk,Pk,0));
					}
					attroff (A_UNDERLINE);
				}
				else if (item == 11)
				{
					mvaddstr (19, 20, "   .   .   .");
					attron (A_UNDERLINE);
					if (cnt == 1)
						MvAddNum (19, 14, 5, TCP2_PORT(Dk,Pk,1));
					else if (cnt == 2)
					{
						MvAddNum (19, 20, 3, TCP2_IP1(Dk,Pk,1));
						MvAddNum (19, 24, 3, TCP2_IP2(Dk,Pk,1));
						MvAddNum (19, 28, 3, TCP2_IP3(Dk,Pk,1));
						MvAddNum (19, 32, 3, TCP2_IP4(Dk,Pk,1));
					}
					attroff (A_UNDERLINE);
				}
				else
					Disp_Msg ("Input the value that you want to set. (Esc:Cancel)");
			}
			else
			{
				if (item == 9)
				{
					mvaddstr (18, 7, "   .   .   .   :");
					attron (A_UNDERLINE);
					if (UDP_PORT(Dk,Pk,0) != 0)
					{
						if (cnt == 1)
						{
							MvAddNum (18, 7, 3, UDP_IP1(Dk,Pk,0));
							MvAddNum (18, 11, 3, UDP_IP2(Dk,Pk,0));
							MvAddNum (18, 15, 3, UDP_IP3(Dk,Pk,0));
							MvAddNum (18, 19, 3, UDP_IP4(Dk,Pk,0));
						}
						else if (cnt == 2)
							MvAddNum (18, 23, 5, UDP_PORT(Dk,Pk,0));
					}
					attroff (A_UNDERLINE);
				}
				else if (item == 10)
				{
					mvaddstr (19, 7, "   .   .   .   :");
					attron (A_UNDERLINE);
					if (UDP_PORT(Dk,Pk,1) != 0)
					{
						if (cnt == 1)
						{
							MvAddNum (19, 7, 3, UDP_IP1(Dk,Pk,1));
							MvAddNum (19, 11, 3, UDP_IP2(Dk,Pk,1));
							MvAddNum (19, 15, 3, UDP_IP3(Dk,Pk,1));
							MvAddNum (19, 19, 3, UDP_IP4(Dk,Pk,1));
						}
						else if (cnt == 2)
							MvAddNum (19, 23, 5, UDP_PORT(Dk,Pk,1));
					}
					attroff (A_UNDERLINE);
				}
				else if (item == 11)
				{
					mvaddstr (20, 7, "   .   .   .   :");
					attron (A_UNDERLINE);
					if (UDP_PORT(Dk,Pk,2) != 0)
					{
						if (cnt == 1)
						{
							MvAddNum (20, 7, 3, UDP_IP1(Dk,Pk,2));
							MvAddNum (20, 11, 3, UDP_IP2(Dk,Pk,2));
							MvAddNum (20, 15, 3, UDP_IP3(Dk,Pk,2));
							MvAddNum (20, 19, 3, UDP_IP4(Dk,Pk,2));
						}
						else if (cnt == 2)
							MvAddNum (20, 23, 5, UDP_PORT(Dk,Pk,2));
					}
					attroff (A_UNDERLINE);
				}
				else if (item == 12)
				{
					mvaddstr (21, 7, "   .   .   .   :");
					attron (A_UNDERLINE);
					if (UDP_PORT(Dk,Pk,3) != 0)
					{
						if (cnt == 1)
						{
							MvAddNum (21, 7, 3, UDP_IP1(Dk,Pk,3));
							MvAddNum (21, 11, 3, UDP_IP2(Dk,Pk,3));
							MvAddNum (21, 15, 3, UDP_IP3(Dk,Pk,3));
							MvAddNum (21, 19, 3, UDP_IP4(Dk,Pk,3));
						}
						else if (cnt == 2)
							MvAddNum (21, 23, 5, UDP_PORT(Dk,Pk,3));
					}
					attroff (A_UNDERLINE);
				}
				else if (item == 13)
				{
					mvaddstr (22, 7, "   .   .   .   :");
					attron (A_UNDERLINE);
					if (UDP_PORT(Dk,Pk,0) != 4)
					{
						if (cnt == 1)
						{
							MvAddNum (22, 7, 3, UDP_IP1(Dk,Pk,4));
							MvAddNum (22, 11, 3, UDP_IP2(Dk,Pk,4));
							MvAddNum (22, 15, 3, UDP_IP3(Dk,Pk,4));
							MvAddNum (22, 19, 3, UDP_IP4(Dk,Pk,4));
						}
						else if (cnt == 2)
							MvAddNum (22, 23, 5, UDP_PORT(Dk,Pk,4));
					}
					attroff (A_UNDERLINE);
				}
				else
					Disp_Msg ("Input the value that you want to set. (Esc:Cancel)");
			}
		}
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
					PROC(Dk,Pk).process_status = atoi (buf);
				else if (item == 2)
					PROC(Dk,Pk).start_status = atoi (buf);
				else if (item == 3)
					PROC(Dk,Pk).process_no = atoi (buf);
				else if (item == 4)
					memcpy (PROC(Dk,Pk).start_time, buf, 4);
				else if (item == 5)
					memcpy (PROC(Dk,Pk).end_time, buf, 4);
				else if (item == 6)
					PROC(Dk,Pk).if_seq = atoi (buf);
				else if (item == 7)
					memcpy (PROC(Dk,Pk).date, buf, 8);
				else if (item == 8)
					PROC(Dk,Pk).timeout = atoi (buf);
				else if (item == 9 && PROC(Dk,Pk).type == TY_TRS2)
					PROC(Dk,Pk).l.t2.line_gubun = atoi (buf);
				else if ((item == 10 || item == 11) &&
					PROC(Dk,Pk).type == TY_TRS2)
				{
					if (cnt == 1)
						TCP2_PORT(Dk,Pk,item-10) = atoi (buf);
					else if (cnt == 2)
					{
						ul = inet_addr (buf);
						if (ul == -1)
						{
							Disp_Msg ("¢º¢º¢º malformed IP address ¢¸¢¸¢¸");
							sleep (1);
							break;
						}
						else
							memcpy (&TCP2_IP1(Dk,Pk,item-10), &ul, sizeof (ul));
					}

					if (cnt != 2)
					{
						Disp_Msg ("Input the value. (Esc:Cancel)");
						continue;
					}
				}
				else if ((item >= 9 && item <= 13) &&
					PROC(Dk,Pk).type == TY_URS)
				{
					if (cnt == 1)
					{
						ul = inet_addr (buf);
						if (ul == -1)
						{
							Disp_Msg ("¢º¢º¢º malformed IP address ¢¸¢¸¢¸");
							sleep (1);
							break;
						}
						else
							memcpy (&UDP_IP1(Dk,Pk,item-9), &ul, sizeof (ul));
					}
					else if (cnt == 2)
						UDP_PORT(Dk,Pk,item-9) = atoi (buf);

					if (cnt != 2)
					{
						Disp_Msg ("Input the value. (Esc:Cancel)");
						continue;
					}
				}

				Disp_Msg ("¢º¢º¢º Changed ¢¸¢¸¢¸");
				Debug (PGMLIN, "item %d changed [%s]", item, buf);
			}
			else
			{
				if (PROC(Dk,Pk).type != TY_URS)
				{
					if (item == 10)
					{
						mvaddstr (18, 20, "   .   .   .");
						attron (A_UNDERLINE);
						if (cnt == 1)
							MvAddNum (18, 14, 5, TCP2_PORT(Dk,Pk,0));
						else if (cnt == 2)
						{
							MvAddNum (18, 20, 3, TCP2_IP1(Dk,Pk,0));
							MvAddNum (18, 24, 3, TCP2_IP2(Dk,Pk,0));
							MvAddNum (18, 28, 3, TCP2_IP3(Dk,Pk,0));
							MvAddNum (18, 32, 3, TCP2_IP4(Dk,Pk,0));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else if (item == 11)
					{
						mvaddstr (19, 20, "   .   .   .");
						attron (A_UNDERLINE);
						if (cnt == 1)
							MvAddNum (19, 14, 5, TCP2_PORT(Dk,Pk,1));
						else if (cnt == 2)
						{
							MvAddNum (19, 20, 3, TCP2_IP1(Dk,Pk,1));
							MvAddNum (19, 24, 3, TCP2_IP2(Dk,Pk,1));
							MvAddNum (19, 28, 3, TCP2_IP3(Dk,Pk,1));
							MvAddNum (19, 32, 3, TCP2_IP4(Dk,Pk,1));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else
						Disp_Msg ("¢º¢º¢º Cancelled ¢¸¢¸¢¸");
				}
				else
				{
					if (item == 9)
					{
						mvaddstr (18, 7, "   .   .   .   :");
						attron (A_UNDERLINE);
						if (UDP_PORT(Dk,Pk,0) != 0)
						{
							if (cnt == 1)
							{
								MvAddNum (18, 7, 3, UDP_IP1(Dk,Pk,0));
								MvAddNum (18, 11, 3, UDP_IP2(Dk,Pk,0));
								MvAddNum (18, 15, 3, UDP_IP3(Dk,Pk,0));
								MvAddNum (18, 19, 3, UDP_IP4(Dk,Pk,0));
							}
							else if (cnt == 2)
								MvAddNum (18, 23, 5, UDP_PORT(Dk,Pk,0));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else if (item == 10)
					{
						mvaddstr (19, 7, "   .   .   .   :");
						attron (A_UNDERLINE);
						if (UDP_PORT(Dk,Pk,1) != 0)
						{
							if (cnt == 1)
							{
								MvAddNum (19, 7, 3, UDP_IP1(Dk,Pk,1));
								MvAddNum (19, 11, 3, UDP_IP2(Dk,Pk,1));
								MvAddNum (19, 15, 3, UDP_IP3(Dk,Pk,1));
								MvAddNum (19, 19, 3, UDP_IP4(Dk,Pk,1));
							}
							else if (cnt == 2)
								MvAddNum (19, 23, 5, UDP_PORT(Dk,Pk,1));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else if (item == 11)
					{
						mvaddstr (20, 7, "   .   .   .   :");
						attron (A_UNDERLINE);
						if (UDP_PORT(Dk,Pk,2) != 0)
						{
							if (cnt == 1)
							{
								MvAddNum (20, 7, 3, UDP_IP1(Dk,Pk,2));
								MvAddNum (20, 11, 3, UDP_IP2(Dk,Pk,2));
								MvAddNum (20, 15, 3, UDP_IP3(Dk,Pk,2));
								MvAddNum (20, 19, 3, UDP_IP4(Dk,Pk,2));
							}
							else if (cnt == 2)
								MvAddNum (20, 23, 5, UDP_PORT(Dk,Pk,2));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else if (item == 12)
					{
						mvaddstr (21, 7, "   .   .   .   :");
						attron (A_UNDERLINE);
						if (UDP_PORT(Dk,Pk,3) != 0)
						{
							if (cnt == 1)
							{
								MvAddNum (21, 7, 3, UDP_IP1(Dk,Pk,3));
								MvAddNum (21, 11, 3, UDP_IP2(Dk,Pk,3));
								MvAddNum (21, 15, 3, UDP_IP3(Dk,Pk,3));
								MvAddNum (21, 19, 3, UDP_IP4(Dk,Pk,3));
							}
							else if (cnt == 2)
								MvAddNum (21, 23, 5, UDP_PORT(Dk,Pk,3));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else if (item == 13)
					{
						mvaddstr (22, 7, "   .   .   .   :");
						attron (A_UNDERLINE);
						if (UDP_PORT(Dk,Pk,0) != 4)
						{
							if (cnt == 1)
							{
								MvAddNum (22, 7, 3, UDP_IP1(Dk,Pk,4));
								MvAddNum (22, 11, 3, UDP_IP2(Dk,Pk,4));
								MvAddNum (22, 15, 3, UDP_IP3(Dk,Pk,4));
								MvAddNum (22, 19, 3, UDP_IP4(Dk,Pk,4));
							}
							else if (cnt == 2)
								MvAddNum (22, 23, 5, UDP_PORT(Dk,Pk,4));
						}
						attroff (A_UNDERLINE);

						if (cnt != 2)
						{
							Disp_Msg ("Input the value. (Esc:Cancel)");
							continue;
						}
					}
					else
						Disp_Msg ("¢º¢º¢º Cancelled ¢¸¢¸¢¸");
				}
			}

			sleep (1);
			break;
		}
	}

	return;
}

/*************************************************************************
	End of Program (py_4010_cm.c)
*************************************************************************/
