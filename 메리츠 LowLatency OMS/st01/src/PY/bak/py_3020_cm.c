/*------------------------------------------------------------------------
#   Module  : Process Status (super daemons)
#	File	: py_3020_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int	Rows;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static void		Disp_Data (void);
static void		Disp_Msg_2 (char *);

/*----------------------------------------------------------------------*/
void	py_3020_cm (void)
/*----------------------------------------------------------------------*/
{
	int		job_end;
	char	msg[80];

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Mem_SHM (0, -1);

	Disp_Scr ();
	Disp_Data ();
	Disp_Msg_2 ("Enter:Retry H:Help Q:Quit");
	keypad (stdscr, TRUE);

	while (job_end)
	{
		Disp_Msg_2 ("Enter:Retry H:Help Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);
		noecho ();

		switch (getch ())
		{
			case	'h':
			case	'H':										/* Help	*/
				Clear_Lines (1, Rows - 2);
				Help_Proc (3);
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
			case	'\n':									/* Retry	*/
				clear ();
				Disp_Scr ();
				Disp_Data ();
				Disp_Msg_2 ("Enter:Retry H:Help Q:Quit");
				break;
			default:
				clear ();
				Disp_Scr ();
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg_2 ("Enter:Retry H:Help Q:Quit");
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
	mvaddstr (0, 18, "[3020] Process - Super Daemons");
	attroff (A_BOLD);

	attron (A_UNDERLINE);
	mvaddstr (1, 0, "    PID P CMD                 STRT END    S Info                               ");
	attroff (A_UNDERLINE);

	return;
}	/* End of Disp_Scr ()	*/

/*----------------------------------------------------------------------*/
static void		Disp_Data (void)
/*----------------------------------------------------------------------*/
{
	int			i, cnt, cnt_run, cnt_not_run;
	pid_t		pid;
	char		msg[80], proc_name[20];
	register	dk;

	i = cnt = cnt_run = cnt_not_run = 0;

	for (dk = 0; dk < Process_Count; dk ++) 
	{
		if (INFO(dk).process_id[0] == 0)
			continue;

		cnt ++;

		memset (proc_name, 0, sizeof (proc_name));
		memcpy (proc_name, DAEMON(dk).process_id,
			strlen (DAEMON(dk).process_id));

		pid = Check_Proc (proc_name);
		sprintf (msg, "%7d %c %-12s        %-4s %-4s   %d 부문 daemon - %s",
			pid, pid == 0 ? '.' : 'O', DAEMON(dk).process_id,
			DAEMON(dk).start_time, DAEMON(dk).end_time,
			DAEMON(dk).system_status, DAEMON(dk).process_info);
		if (pid == 0)
		{
			attron (A_BOLD);
			mvaddstr (i + 2, 0, msg);
			attroff (A_BOLD);
			cnt_not_run ++;
		}
		else
		{
			mvaddstr (i + 2, 0, msg);
			cnt_run ++;
		}
		i ++;
	}

	attron (A_UNDERLINE);
	mvaddstr (i + 3, 0, "    PID P CMD                               Info                               ");
	attroff (A_UNDERLINE);
	i ++;

	sprintf (proc_name, "%s", "pz_procchk_mp");
	pid = Check_Proc (proc_name);
	sprintf (msg, "%7d %c %-33.33s %-35.35s",
		pid, pid == 0 ? '.' : 'O', proc_name, "process/disk check");
	if (pid == 0)
	{
		attron (A_BOLD);
		mvaddstr (i + 3, 0, msg);
		attroff (A_BOLD);
		cnt_not_run ++;
	}
	else
	{
		mvaddstr (i + 3, 0, msg);
		cnt_run ++;
	}
	cnt ++;
	i ++;

	sprintf (proc_name, "%s", "pz_fepp_mp");
	pid = Check_Proc (proc_name);
	sprintf (msg, "%7d %c %-33.33s %-35.35s",
		pid, pid == 0 ? '.' : 'O', proc_name, "최상위 daemon");
	if (pid == 0)
	{
		attron (A_BOLD);
		mvaddstr (i + 3, 0, msg);
		attroff (A_BOLD);
		cnt_not_run ++;
	}
	else
	{
		mvaddstr (i + 3, 0, msg);
		cnt_run ++;
	}
	cnt ++;
	i ++;

	memset (msg, 0, sizeof msg);
	sprintf (msg, "Run:%2d,", cnt_run);
	mvaddstr (Rows - 1, 54, msg);

	memset (msg, 0, sizeof msg);
	sprintf (msg, "Stop:%2d,", cnt_not_run);
	attron (A_BOLD);
	mvaddstr (Rows - 1, 62, msg);
	attroff (A_BOLD);

	memset (msg, 0, sizeof msg);
	sprintf (msg, "Total:%2d", cnt);
	attron (A_UNDERLINE);
	mvaddstr (Rows - 1, 71, msg);
	attroff (A_UNDERLINE);

	return;
}

/*----------------------------------------------------------------------*/
static void		Disp_Msg_2 (char *msg)
/*----------------------------------------------------------------------*/
{
	int		i;

	attron (A_REVERSE);
    for (i = 0; i < 38; i ++)
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
	End of Program (py_3020_cm.c)
*************************************************************************/
