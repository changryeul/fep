/*------------------------------------------------------------------------
#	Module	: System & Misc. - Control Daemon
#	File	: py_4040_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int			Dk;
extern int	Rows;
char		DaemonType, RunFlag, ReloadFlag, SubName[2];

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
static void		Disp_Scr (void);
static int		Get_Scr (void);
static void		Start_Proc (void);

/*----------------------------------------------------------------------*/
void	py_4040_cm (void)
/*----------------------------------------------------------------------*/
{
	int		key, job_end;

	job_end = 1;

	initscr ();
	newwin (Rows, 80, 0, 0);

	Sub_SHM ();
	Disp_Scr ();

	if (Get_Scr () == 0)
		Start_Proc ();

	Disp_Msg ("Enter:Retry Q:Quit");
	keypad (stdscr, TRUE);

    while (job_end)
    {
		Disp_Msg ("Enter:Retry Q:Quit");
		mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);
	    key = getch ();
		clear ();
		Disp_Scr ();

	    switch (key)
	    {
			case 	KEY_ESC:
			case	KEY_LEFT:
			case	KEY_RIGHT:
			case	'1':
				job_end = 0;
				refresh ();
				endwin ();
                break;
			case	'q':
			case	'Q':										/* Quit	*/
				Exit_Process ();
			case	'\n':									/* Retry	*/
				Clear_Lines (2, Rows - 3);
				Disp_Scr ();
				if (Get_Scr () == 0)
				{
					Start_Proc ();
				}
				Disp_Msg ("Enter:Retry Q:Quit");
                break;
			default:
				Disp_Msg ("You entered wrong key.");
				sleep (1);
				Disp_Msg ("Enter:Retry Q:Quit");
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
	mvaddstr (0, 18, "[4040] Control Daemon");
	attroff (A_BOLD);
	Draw_Line (1, 0, 79);
	Draw_Line (Rows - 2, 0, 79);

	mvaddstr (3, 5, "Daemon Type :    (1:super daemon  2:sub daemon)");
	Draw_Underline (3, 19, 1);
	mvaddstr (5, 8, "Sub Name :     (pa, pb, ...)");
	Draw_Underline (5, 19, 2);
	mvaddstr (7, 5, "Run or Stop :    (r:run  s:stop)");
	Draw_Underline (7, 19, 1);
	mvaddstr (9, 6, "SHM reload : -  (y:yes  n:no)");
	Draw_Underline (9, 19, 1);
	Draw_Line (Rows - 2, 0, 79);

	return;
}

/*----------------------------------------------------------------------*/
static int	Get_Scr (void)
/*----------------------------------------------------------------------*/
{
	char	buf[4];
	int		rt;

	rt = DaemonType = RunFlag = ReloadFlag = 0;
	memset (SubName, 0, sizeof (SubName));

	Disp_Msg ("Input <Daemon Type>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (3, 19, 1);
		rt = Get_String (3, 19, 1, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Daemon Type>. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
		{
			Disp_Scr ();
			return (-1);
		}
		else
		{
			if (strlen (buf) != 1 || (buf[0] != '1' && buf[0] != '2'))
			{
				Disp_Msg ("Incorrect Daemon Type (Esc:Cancel)");
				continue;
			}
			DaemonType = buf[0];
			break;
		}
	}

	if (DaemonType == '2')
	{
		Disp_Msg ("Input <Sub Name>. (Esc:Cancel)");
		while (1)
		{
			memset (&buf, 0, sizeof buf);
			Draw_Underline (5, 19, 2);
			rt = Get_String (5, 19, 2, buf);
			if (rt == -1)										/* skip	*/
				Disp_Msg ("Input <Sub Name>. (Esc:Cancel)");
			else if (rt == -4)								/* Space	*/
				Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
			else if (rt == 1)									/* Esc	*/
			{
				Disp_Scr ();
				return (-1);
			}
			else
			{
				UtoL (buf, 2);

				if (strlen (buf) != 2 || buf[0] != 'p' ||
					buf[1] < 'a' || buf[1] > 'w')
				{
					Disp_Msg ("Incorrect Sub Name (Esc:Cancel)");
					continue;
				}
				memcpy (SubName, buf, 2);
				Dk = SubName[1] - 'a';
				break;
			}
		}
	}

	Disp_Msg ("Select <Run or Stop>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (7, 19, 1);
		rt = Get_String (7, 19, 1, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Select <Run or Stop>. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
		{
			Disp_Scr ();
			return (-1);
		}
		else
		{
			UtoL (buf, 1);

			if (strlen (buf) != 1 || (buf[0] != 'r' && buf[0] != 's'))
			{
				Disp_Msg ("Input 'r' or 's'. (Esc:Cancel)");
				continue;
			}
			RunFlag = buf[0];
			break;
		}
	}

	if (DaemonType == '2' && RunFlag == 'r')
	{
		Disp_Msg ("Reload SHM ? (Esc:Cancel)");
		while (1)
		{
			memset (&buf, 0, sizeof buf);
			Draw_Underline (9, 19, 1);
			rt = Get_String (9, 19, 1, buf);
			if (rt == -1)										/* skip	*/
				Disp_Msg ("Select <Yes or No>. (Esc:Cancel)");
			else if (rt == -4)								/* Space	*/
				Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
			else if (rt == 1)									/* Esc	*/
			{
				Disp_Scr ();
				return (-1);
			}
			else
			{
				UtoL (buf, 1);

				if (strlen (buf) != 1 || (buf[0] != 'y' && buf[0] != 'n'))
				{
					Disp_Msg ("Input 'y' or 'n'. (Esc:Cancel)");
					continue;
				}
				ReloadFlag = buf[0];
				break;
			}
		}
	}

	return (0);
}

/*----------------------------------------------------------------------*/
static void		Start_Proc (void)
/*----------------------------------------------------------------------*/
{
	int 	rt;
	pid_t	pid;
	char	msg[80], path[256], p_name[20];

	if (DaemonType == '1')
		sprintf (p_name, "%s", "pz_fepp_mp");
	else
		sprintf (p_name, "%.2s_daemon_mp", SubName);

	pid = Check_Proc (p_name);

	if (DaemonType == '1')
	{
		if (RunFlag == 's')
		{
			if (pid == 0)
			{
				sprintf (msg, "%s not run !", p_name);
				Disp_Msg (msg);
				sleep (1);
			}
			else
			{
				rt = kill (pid, SIGUSR2);
				if (rt == -1)
				{
					sprintf (msg, "ERROR:kill failure [%d]", pid);
					Disp_Msg (msg);
					return;
				}
				sprintf (msg, "%s stopped !", p_name);
				Disp_Msg (msg);
				sleep (1);
			}
		}
		else
		{
			if (pid == 0)
			{
				sprintf (path, "%s", (char *)getenv ("_P_BIN"));
				rt = chdir (path);
				if (rt != 0)
				{
					sprintf (msg, "ERROR:chdir (%s)", path);
					Disp_Msg (msg);
					sleep (1);
					return;
				}

				rt = system (p_name);
				if (rt < 0)
				{
					sprintf (msg, "ERROR:system (%s)", p_name);
					Disp_Msg (msg);
					sleep (1);
					return;
				}
				sprintf (msg, "%s started !", p_name);
				Disp_Msg (msg);
				sleep (1);
				Debug (PGMLIN, "%s", msg);
			}
			else
			{
				sprintf (msg, "%s already run !", p_name);
				Disp_Msg (msg);
				sleep (1);
			}
		}
	}
	else
	{
		if (RunFlag == 's')
		{
			if (pid == 0)
			{
				sprintf (msg, "%s not run !", p_name);
				Disp_Msg (msg);
				sleep (1);
			}
			else
			{
				INFO(Dk).process_status = 0;
				rt = kill (pid, SIGTERM);
				if (rt == -1)
				{
					sprintf (msg, "ERROR:kill failure [%d]", pid);
					Disp_Msg (msg);
					sleep (1);
					return;
				}
				sprintf (msg, "%s stopped !", p_name);
				Disp_Msg (msg);
				sleep (1);
				Debug (PGMLIN, "%s", msg);
			}
		}
		else
		{
			if (pid == 0)
			{
				if (ReloadFlag == 'y')
				{
					INFO(Dk).system_status = 0;

					sprintf (path, "%s", (char *)getenv ("_P_BIN"));
					rt = chdir (path);
					if (rt != 0)
					{
						sprintf (msg, "ERROR:chdir (%s)", path);
						Disp_Msg (msg);
						sleep (1);
						return;
					}

					rt = system (p_name);
					if (rt < 0)
					{
						sprintf (msg, "ERROR:system (%s)", p_name);
						Disp_Msg (msg);
						sleep (1);
						return;
					}
					sprintf (msg, "%s started (SHM reloaded) !", p_name);
					Disp_Msg (msg);
					sleep (1);
					Debug (PGMLIN, "%s", msg);
				}
				else
				{
					INFO(Dk).system_status = 1;
					INFO(Dk).process_status = 1;
					INFO(Dk).process_no = 0;
					sprintf (msg, "%s started (current SHM used) !", p_name);
					Disp_Msg (msg);
					sleep (1);
					Debug (PGMLIN, "%s", msg);
				}
			}
			else
			{
				sprintf (msg, "%s already run !", p_name);
				Disp_Msg (msg);
				sleep (1);
			}
		}
	}

	return;
}

/*************************************************************************
	End of Program (py_4040_cm.c)
*************************************************************************/
