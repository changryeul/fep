#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: common library functions
#	File	: py_libs_cm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"py_cm.h"

#if defined __hpux
#include	<sys/syscall.h>
#include	<sys/pstat.h>
#elif defined sun
#include	<stdlib.h>
#include	<dirent.h>
#include	<limits.h>
#include	<sys/syscall.h>
#include	<sys/procfs.h>
#elif defined _AIX
#include	<procinfo.h>
#endif

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#if defined __hpux
#define		BURST	((size_t)500)
#elif defined _AIX
#define		BURST	200
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
extern int		Dk, Rows;
extern u_char	SwitchFlag, DisplayFlag, MonitorFlag, PageFlag, TimeFlag;

#if defined sun
int				Exist_Flag;
#endif

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	MvAddStr (int, int, int, char *);
void	MvAddNum (int, int, int, int);
void	MvAddLong (int, int, int, long);
void	Align_Num (int, int, char *);
int		Chk_Digit (char *);
int		Chk_Digit_Alpha (char *);
int		Chk_Dir (char *);
void	Clear_Lines (int, int);
void	Create_Dir (char *);
void	Disp_Msg (char *);
void	Disp_Title (void);
void	Draw_Box (int, int, int, int);
void	Draw_Underline (int, int, int);
void	Draw_Underbar (int, int, int);
void	Draw_Line (int, int, int);
void	Draw_Thin_Line (int, int, int);
void	Get_Data (char *, char *, char *);
int		Get_String (int, int, int, char *);
int		Yes_No (char *, int);
void	Setsigfatal (void);
void	End_Routine (int);
void	Sub_SHM (void);
void    Mem_SHM (int, int);
void	Sise_SHM (void);
char	*Shm_Attach (key_t, int *);
int		SHM_Creat (key_t, size_t);
char	*SHM_Attach (int);
char	*LtoU (char *, int);
char	*UtoL (char *, int);
int		AtoIf (char *, int);
void	Exit_Process (void);
void	SHM_Detach (char *);
void	Register_Signal (void);
void	Catch_Signal (int);
int		Trefresh (int);
int		Disp_IO (void);
void	G_Time (void);
void	Change_File_Cnt (void);
void	Change_If_Seq (void);
void	Help_Proc (int);
int		Change_Interval (void);
pid_t	Check_Proc (char *);

#if defined __hpux
pid_t	Check_Proc_HPUX (char *);
#elif defined sun
pid_t	Check_Proc_SUN (char *);
int		Make_List (char *, char *);
#elif defined _AIX
pid_t	Check_Proc_AIX (char *);
#endif

/*----------------------------------------------------------------------*/
void	MvAddStr (int y, int x, int l, char *s)
/*----------------------------------------------------------------------*/
{
	char	b[80];

	memset (b, 0, sizeof (b));
	if (s[0] == 0)
		memset (b, ' ', l);
	else
		memcpy (b, s, l);
	mvaddstr (y, x, b);

	return;
}

/*----------------------------------------------------------------------*/
void	MvAddNum (int y, int x, int l, int n)
/*----------------------------------------------------------------------*/
{
	char	b[20];

	memset (b, 0, sizeof (b));
	sprintf (b, "%*d", l, n);
	mvaddstr (y, x, b);

	return;
}

/*----------------------------------------------------------------------*/
void	MvAddLong (int y, int x, int l, long n)
/*----------------------------------------------------------------------*/
{
	char	b[20];

	memset (b, 0, sizeof (b));
	sprintf (b, "%*ld", l, n);
	mvaddstr (y, x, b);

	return;
}

/*----------------------------------------------------------------------*/
/*	숫자로 이루어진 문자열을 우측 정렬하고 1000단위마다 comma로 구분	*/
/*----------------------------------------------------------------------*/
void	Align_Num (num, len, conv)
int		num;									/* Number to display	*/
int		len;									/* Lenth of the string	*/
char	*conv;									/* Converted string		*/
{
	int		comma_cnt, comma_pos, spc_cnt, i;
	char	str[20], tmp[20];

	comma_cnt = comma_pos = spc_cnt = 0;
	memset (str, 0, sizeof (str));
	memset (tmp, 0, sizeof (tmp));

	sprintf (str, "%d", num);
	comma_cnt = strlen (str) / 3;
	comma_pos = strlen (str) % 3;

	if (comma_pos == 0)
	{
		comma_cnt -= 1;
		comma_pos += 3;
	}

	if (comma_cnt + strlen (str) > len)
	{
		for (i = 0; i < len - strlen (str); i ++)
		{
			strncat (tmp, " ", 1);
		}
		for (i = 0; i < strlen (str); i ++)
		{
			strncat (tmp, &str[i], 1);
		}
	}
	else
	{
		spc_cnt = len - strlen (str) - comma_cnt;
		for (i = 0; i < spc_cnt; i ++)
		{
			strncat (tmp, " ", 1);
		}

		for (i = 0; i < strlen (str); i ++)
		{
			strncat (tmp, &str[i], 1);
			if (comma_cnt > 0)
			{
				if (i == comma_pos - 1 && i != strlen (str) - 1)
				{
					strncat (tmp, ",", 1);
					comma_pos += 3;
				}
			}
		}
	}

	memcpy (conv, tmp, len);

	return;
}	/* End of Align_Num ()	*/

/*----------------------------------------------------------------------*/
/*	문자열에 숫자 이외의 것이 있는지 check								*/
/*----------------------------------------------------------------------*/
int		Chk_Digit (char *str)
{
	int		i;

	for (i = 0; i < strlen (str); i ++)
	{
		if (!isdigit (str[i]))
		{
			if (i == 0 && str[i] == '-' && strlen (str) > 1)
				continue;

			return (0);							/* 숫자 이외의 값 있음	*/
		}
	}

	return (1);											/* 숫자만 있음	*/
}

/*----------------------------------------------------------------------*/
/*	숫자 (0 - 9)와 영문자 (a - z, A - Z)이외의 것이 있는지 check		*/
/*----------------------------------------------------------------------*/
int		Chk_Digit_Alpha (char *str)
{
	int		i;

	for (i = 0; i < strlen (str); i ++)
	{
		if (!isdigit (str[i]) && !isalpha (str[i]))
			return (0);								/* 다른 문자 존재	*/
	}

	return (1);									/* 숫자와 영문자만 있음	*/
}	/* End of Chk_Digit_Alpha ()	*/

/*----------------------------------------------------------------------*/
/*	디렉토리 존재 여부 check											*/
/*----------------------------------------------------------------------*/
int		Chk_Dir (char *dir_path)
{
	int				cpid, status;
	struct stat		statbuf;

	if (stat (dir_path, &statbuf) == NULL)
		return (0);

	switch (cpid = fork ())
	{
		case	-1:
			return (-1);
		case	0:
			return (1);
		default:
			while (cpid != wait (&status))
			{
				;
			}
	}
	status = status >> 8;

	return (status);
}	/* End of Chk_Dir ()	*/

/*----------------------------------------------------------------------*/
/*	Delete lines from line 1 to line 2									*/
/*----------------------------------------------------------------------*/
void	Clear_Lines (int y1, int y2)
{
	int		l, c;

	for (l = y1; l <= y2; l ++)
	{
		for (c = 0; c < 80; c ++)
		{
			mvaddch (l, c, ' ');
		}
	}

	return;
}	/* End of Clear_Lines ()	*/

/*----------------------------------------------------------------------*/
/*	Create directory													*/
/*----------------------------------------------------------------------*/
void	Create_Dir (char *dir_path)
{
	execl ("/bin/mkdir", "mkdir", dir_path, (char *)0);

	return;
}	/* End of Create_Dir ()	*/

/*----------------------------------------------------------------------*/
/*	Print messages into log file for debugging							*/
/*----------------------------------------------------------------------*/
void	Debug (f_name, f_line, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
char	*f_name;
int		 f_line;
char	*fmt;
{
	char		date_time[30], dt_buf[20], file[128], buf[128];
	FILE		*dfp, *fp;
	time_t		tloc;
	struct tm	*tp;

	time (&tloc);
	tp = localtime (&tloc);

	memset (dt_buf, NULL, sizeof dt_buf);
	strftime (dt_buf, 15, "%Y%m%d%H%M%S", tp);
	sprintf (date_time, "%.2s:%.2s:%.2s", dt_buf+8, dt_buf+10, dt_buf+12);

	sprintf (file, "%s/%.8s", (char *)getenv ("_PY_LOG"), dt_buf);
	if (Chk_Dir (file) == 1)
		Create_Dir (file);

	sprintf (file, "%s/%.8s/py_main_cm", (char *)getenv ("_PY_LOG"), dt_buf);

	if ((dfp = fopen (file, "ab")) != (FILE *) NULL)
	{
#if defined __hpux
		fp = popen ("who -mR", "r");
#elif defined sun || _AIX
		fp = popen ("who -m", "r");
#endif
		fgets (buf, sizeof (buf), fp);
		pclose (fp);
		buf[strlen(buf)-1] = NULL;

		fprintf (dfp, "[%s,", date_time);
		fprintf (dfp, "%-10.10s,%4d,", f_name, f_line);
		fprintf (dfp, "%s]", buf);
		fprintf (dfp, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		fprintf (dfp, "\n");
		fflush (dfp);
		fclose (dfp);
		fflush (stdout);
	}

	return;
}	/* End of Debug ()	*/

/*----------------------------------------------------------------------*/
/*	Display message														*/
/*----------------------------------------------------------------------*/
void	Disp_Msg (char *msg)
{
	int		i;

	attron (A_REVERSE);
	for (i = 0; i < 80; i ++)
	{
		mvaddch (Rows - 1, i, ' ');
	}
	mvaddstr (Rows - 1, 0, "☞ ");
	mvaddstr (Rows - 1, 3, msg);
	attroff (A_REVERSE);
	refresh ();

	return;
}	/* End of Disp_Msg ()	*/

/*----------------------------------------------------------------------*/
/*	Display title														*/
/*----------------------------------------------------------------------*/
void	Disp_Title (void)
{
	int			i;
	char		date_time[20], dt_buf[20];
	time_t		tloc;
	struct tm	*tp;

	time (&tloc);
	tp = localtime (&tloc);

	memset (dt_buf, NULL, sizeof dt_buf);
	strftime (dt_buf, 18, "%Y%m%d%a%H%M%S", tp);
	sprintf (date_time, "%.11s-%.6s", dt_buf, dt_buf+11);
												/* yyyymmddwww-hhmmss	*/

	attron (A_REVERSE);
	for (i = 0; i < 80; i ++)
		mvaddch (0, i, ' ');

	attroff (A_REVERSE);
	attron (A_REVERSE);
	mvaddstr (0, 0, date_time);
	attroff (A_REVERSE);
	refresh ();

	return;
}

/*----------------------------------------------------------------------*/
/*	Draw box lines														*/
/*----------------------------------------------------------------------*/
void	Draw_Box (int y1, int x1, int y2, int x2)
{
	int	i;

	for (i = x1+2; i < x2; i = i+2)	{ move (y1, i); addstr (B_2); }
	for (i = x1+2; i < x2; i = i+2)	{ move (y2, i); addstr (B_2); }
	for (i = y1 + 1; i < y2; i ++)	{ move (i, x2); addstr (B_1); }
	for (i = y1 + 1; i < y2; i ++)	{ move (i, x1); addstr (B_1); }
	move (y1, x1);	addstr (B_HOME);
	move (y1, x2);	addstr (B_PGUP);
	move (y2, x2);	addstr (B_PGDN);
	move (y2, x1);	addstr (B_END);

	return;
}	/* End of Draw_Box ()	*/

/*----------------------------------------------------------------------*/
/*	화면에 밑줄이 그어진 입력 field를 display함							*/
/*----------------------------------------------------------------------*/
void	Draw_Underline (y, x, l)
int		y;											/* Y Position		*/
int		x;											/* X Position		*/
int		l;											/* Length of string	*/
{
	int		i;

	attron (A_UNDERLINE);
	for (i = x; i < x + l; i ++)
	{
		mvaddch (y, i, ' ');
	}
	attroff (A_UNDERLINE);

	return;
}	/* End of Draw_Underline ()	*/

/*----------------------------------------------------------------------*/
void	Draw_Underbar (y, x, l)
/*----------------------------------------------------------------------*/
int		y;											/* Y Position		*/
int		x;											/* X Position		*/
int		l;											/* Length of string	*/
{
	int		i;

	for (i = x; i < x + l; i ++)
		mvaddch (y, i, '_');

	return;
}	/* End of Draw_Underline ()	*/

/*----------------------------------------------------------------------*/
/*	Draw horizontal lines												*/
/*----------------------------------------------------------------------*/
void	Draw_Line (int y, int x1, int x2)
{
	int	i;

	for (i = x1; i < x2; i = i + 2)
	{
		move (y, i);
		addstr (B_2);
	}

	return;
}	/* End of Draw_Line ()	*/

/*----------------------------------------------------------------------*/
/*	Draw thin line horizontally											*/
/*----------------------------------------------------------------------*/
void	Draw_Thin_Line (int y, int x1, int x2)
{
	int	i;

	for (i = x1; i < x2; i = i + 2)
	{
		move (y, i);
		addstr ("--");
	}

	return;
}	/* End of Draw_Thin_Line ()	*/

/*----------------------------------------------------------------------*/
/* Get string															*/
/*----------------------------------------------------------------------*/
int		Get_String (y, x, l, b)
int		y;									/* Y Position				*/
int		x;									/* X Position				*/
int		l;									/* Length of string			*/
char	*b;									/* Buffer for input data	*/
{
	int		i, j, bs_cnt;

	move (y, x);
	attron (A_UNDERLINE);

	bs_cnt = 0;
	keypad (stdscr, TRUE);
	for (i = x; i <= x + l; i ++)
	{
		b[i-x] = mvgetch (y, i);

		if (b[i-x] == KEY_ESC)								/* Esc key	*/
		{
			mvaddstr (y, i, "  ");
			attroff (A_UNDERLINE);
			refresh ();
			endwin ();
			return (1);
		}
		else if (b[i-x] == KEY_BS)					/* Backspace key	*/
		{
			b[i-x] = NULL;
			if (i == x)
				i --;
			else
				i -= 2;
			mvaddch (y, i + 1, ' ');
			bs_cnt ++;
		}
		else if (b[i-x] == KEY_DEL)						/* Delete key	*/
		{
			b[i-x] = NULL;
			for (j = x; j < x + l; j ++)
			{
				mvaddch (y, j, ' ');
			}
			attroff (A_UNDERLINE);
			return (-5);
		}
		else if (b[i-x] == '\n')							/* Enter	*/
		{
			b[i-x] = NULL;
			if (i == x)
			{
				attroff (A_UNDERLINE);
				return (-1);
			}
			/* 첫 자가 space이면 재 입력 요구	*/
			else if (b[0] == ' ')
			{
				attroff (A_UNDERLINE);
				return (-4);
			}
			/* 특수문자 입력불가	*/
			else if (strchr (b, '|') || strchr (b, '!') || strchr (b, '@') ||
				strchr (b, '#') || strchr (b, '$') || strchr (b, '%') ||
				strchr (b, '^') || strchr (b, '&') || strchr (b, '*') ||
				strchr (b, '-') || strchr (b, '+') || strchr (b, '=') ||
				strchr (b, '\\') || strchr (b, '~') || strchr (b, '`') ||
				strchr (b, '\'') || strchr (b, '"') || strchr (b, ':') ||
				strchr (b, ';') || strchr (b, '}') || strchr (b, '{') ||
				strchr (b, '[') || strchr (b, ']') || strchr (b, '?') ||
				strchr (b, '<') || strchr (b, '>') || strchr (b, ',') ||
				strchr (b, '\t') || strchr (b, ' '))
			{
				attroff (A_UNDERLINE);
				return (-2);
			}
			break;
		}
		else if (i == x)
		{
			for (j = x + 1; j < x + l; j ++)
			{
				mvaddch (y, j, ' ');
			}
		}

		if (i == x + l)								/* 입력 범위 넘음	*/
		{
			attroff (A_UNDERLINE);
			mvaddch (y, x + l, ' ');
			return (-3);
		}
	}
	attroff (A_UNDERLINE);

	return (OK);
}	/* End of Get_String ()	*/

/*----------------------------------------------------------------------*/
/* Ask yes or no														*/
/*----------------------------------------------------------------------*/
int		Yes_No (char *msg, int yn_flag)
{
	keypad (stdscr, TRUE);

	while (1)
	{
		Disp_Msg (msg);
		attron (A_REVERSE);
		if (yn_flag == YES)
			mvaddstr (Rows - 1, strlen (msg) + 4, "(Y/n/q) ");
		else if (yn_flag == NO)
			mvaddstr (Rows - 1, strlen (msg) + 4, "(y/N/q) ");
		else
			mvaddstr (Rows - 1, strlen (msg) + 4, "(y/n/Q) ");
		attroff (A_REVERSE);

		switch (getch ())
		{
			case	'y':
			case	'Y':
				return (YES);
			case	'n':
			case	'N':
				return (NO);
			case	'q':
			case	'Q':
				return (QUIT);
			case	'\n':
				if (yn_flag == YES)
					return (YES);
				else if (yn_flag == NO)
					return (NO);
				else
					return (QUIT);
				break;
			default:
				Disp_Msg ("Enter Y[y], N[n] or Q[q].");
				sleep (1);
		}
	}
}	/* End of Yes_No ()	*/

/*-----------------------------------------------------------------------*/
void	Sub_SHM (void)
/*-----------------------------------------------------------------------*/
{
	key_t	shm_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		shm_key += 0x01000000L;

	Shmptr = Shm_Attach (shm_key, &SHM_Shmid);
	SHM_All_Daemon_Info = (ALL_DAEMON_INFO *)Shmptr;

	return;
}	/* End of Sub_SHM ()	*/

/*-----------------------------------------------------------------------*/
void	Mem_SHM (int flag, int dk)
/*-----------------------------------------------------------------------*/
{
	int		i, j;
	char	key[12];
	key_t	base_key, shm_key;

	base_key = BASE_SHM_KEY;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		base_key += 0x01000000L;

	for (i = 0; i < Process_Count; i ++)
	{
		Shmsize = 0;
		sprintf (key, "0x00%02d0000", i + 1);
		shm_key = base_key + strtol (key, NULL, 16);

		if ((flag == 1 && i == dk) || (flag == 0 && INFO(i).process_id[0] != 0))
		{
			Shmsize = sizeof (PROCESS_INFO) * INFO(i).process_count +
				sizeof (FILE_INFO) * INFO(i).file_count +
				sizeof (DSHM_INFO) * INFO(i).dshm_count +
#if defined ISAM_INCL
				sizeof (CISAM_INFO) * INFO(i).cisam_count +
#endif
				sizeof (TCP1_INFO) * INFO(i).tcp1_count +
				sizeof (TCP2_INFO) * INFO(i).tcp2_count +
				sizeof (UDPIP_INFO) * INFO(i).udpip_count +
				sizeof (SISETR_INFO) * INFO(i).sisetr_count +
				sizeof (ACCNO_INFO) * INFO(i).accno_count;

			if (!Shmsize)
				continue;

			Shmsize += sizeof (SUB_DAEMON_INFO);

			if (INFO(i).data_count != 0)
				Shmsize += SHM_DATA_SIZE * INFO(i).data_count;

			Shmptr = Shm_Attach (shm_key, &Mem_Shmid[i]);
			SHM_Mem[i] = (char *)Shmptr;

			Shm_Mem[i].Daemon = (SUB_DAEMON_INFO *)Shmptr;
			Shmptr += sizeof (SUB_DAEMON_INFO);
			Shm_Mem[i].Proc = (PROCESS_INFO *)Shmptr;
			Shmptr += sizeof (PROCESS_INFO) * INFO(i).process_count;
			Shm_Mem[i].File = (FILE_INFO *)Shmptr;
			Shmptr += sizeof (FILE_INFO) * INFO(i).file_count;
			Shm_Mem[i].DShm = (DSHM_INFO *)Shmptr;
			Shmptr += sizeof (DSHM_INFO) * INFO(i).dshm_count;
#if defined ISAM_INCL
			Shm_Mem[i].Cisam = (CISAM_INFO *)Shmptr;
			Shmptr += sizeof (CISAM_INFO) * INFO(i).cisam_count;
#endif
			Shm_Mem[i].Tcp1 = (TCP1_INFO *)Shmptr;
			Shmptr += sizeof (TCP1_INFO) * INFO(i).tcp1_count;
			Shm_Mem[i].Tcp2 = (TCP2_INFO *)Shmptr;
			Shmptr += sizeof (TCP2_INFO) * INFO(i).tcp2_count;
			Shm_Mem[i].Udpip = (UDPIP_INFO *)Shmptr;
			Shmptr += sizeof (UDPIP_INFO) * INFO(i).udpip_count;
			Shm_Mem[i].Sisetr = (SISETR_INFO *)Shmptr;
			Shmptr += sizeof (SISETR_INFO) * INFO(i).sisetr_count;
			Shm_Mem[i].Accno = (ACCNO_INFO *)Shmptr;
			Shmptr += sizeof (ACCNO_INFO) * INFO(i).accno_count;

			for (j = 0; j < INFO(i).data_count; j ++)
			{
				Data_Ptr[j] = Shmptr;
				Shmptr += SHM_DATA_SIZE;
			}
		}
	}

	return;
}	/* End of Mem_SHM ()	*/

/*************************************************************************
	Function	   : . attach sise SHM
	Parameters IN  : .
	Parameters OUT : .
	Return Code	   : . void
*************************************************************************/
/*-----------------------------------------------------------------------*/
void	Sise_SHM (void)
/*-----------------------------------------------------------------------*/
{
	key_t	k;

	if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
		k = 0x01000000L;
	else
		k = 0x00000000L;

	/* 지수선물	*/
	Shm_Futures = (SHM_FUTURES *)Shm_Attach (F_SHM_KEY + k, &SISE_F_Shmid);

	/* 지수옵션	*/
	Shm_Options = (SHM_OPTIONS *)Shm_Attach (O_SHM_KEY + k, &SISE_O_Shmid);

	/* 유가증권	*/
	Shm_Stock = (SHM_STOCK *)Shm_Attach (S_SHM_KEY + k, &SISE_S_Shmid);

	/* ELW	*/
	Shm_Elw = (SHM_ELW *)Shm_Attach (E_SHM_KEY + k, &SISE_E_Shmid);
	
	/* 지수 */
	Shm_Jisu = (SHM_JISU *)Shm_Attach (J_SHM_KEY + k, &SISE_J_Shmid);

	/* BACKOFFICE	*/
	Shm_BackOffice = (SHM_BACKOFFICE *)Shm_Attach (B_SHM_KEY + k, &SISE_B_Shmid);

	/* DB처리용	*/
	Shm_Db = (SHM_DB *)Shm_Attach (DB_SHM_KEY + k, &SISE_DB_Shmid);

	/* ELW_DB처리용	*/
	Shm_Elw_Db = (SHM_ELW_DB *)Shm_Attach (DB_ELW_SHM_KEY + k, &SISE_ELW_DB_Shmid);

	return;
}	/* End of Sise_SHM ()	*/

/*-----------------------------------------------------------------------*/
void	Setsigfatal (void)
/*-----------------------------------------------------------------------*/
{
	signal (SIGINT, End_Routine);
	signal (SIGKILL, End_Routine);
	signal (SIGQUIT, End_Routine);
	signal (SIGILL, End_Routine);
	signal (SIGTERM, End_Routine);
	signal (SIGBUS, End_Routine);
	signal (SIGSEGV, End_Routine);
	signal (SIGHUP, End_Routine);
	signal (SIGUSR1, End_Routine);

	return;
}	/* End of Setsigfatal () */

/*-----------------------------------------------------------------------*/
void	End_Routine (int p_signo)
/*-----------------------------------------------------------------------*/
{
	switch (p_signo) 
	{
		case	SIGINT:
			Debug (PGMLIN, "signal caught [%d:SIGINT]", p_signo);
			break;
		case	SIGKILL:
			Debug (PGMLIN, "signal caught [%d:SIGKILL]", p_signo);
			break;
		case	SIGILL:
			Debug (PGMLIN, "signal caught [%d:SIGILL]", p_signo);
			break;
		case	SIGQUIT:
			Debug (PGMLIN, "signal caught [%d:SIGQUIT]", p_signo);
			break;
		case	SIGTERM:
			Debug (PGMLIN, "signal caught [%d:SIGTERM]", p_signo);
			break;
		case	SIGBUS:
			Debug (PGMLIN, "signal caught [%d:SIGBUS]", p_signo);
			break;
		case	SIGSEGV:
			Debug (PGMLIN, "signal caught [%d:SIGSEGV]", p_signo);
			break;
		case	SIGHUP:
			Debug (PGMLIN, "signal caught [%d:SIGHUP]", p_signo);
			break;
		case	SIGUSR1:
			Debug (PGMLIN, "signal caught [%d:SIGUSR1]", p_signo);
			break;
		default:
			Debug (PGMLIN, "signal caught [%d:XXXXXXX]", p_signo);
			break;
	}
	
	Exit_Process ();

	return;
}	/* End of End_Routine () */

/*----------------------------------------------------------------------*/
char	*Shm_Attach (key_t p_key, int *p_id)
/*----------------------------------------------------------------------*/
{
	char	*ptr = (char *)-1;

	*p_id = SHM_Creat (p_key, 0);
	if (*p_id == -1)
	{
		Debug (PGMLIN, "ERROR: Shm_Attach: cannot create SHM [%#x] {%d:%s}",
			p_key, SYS_NO, SYS_STR);
		Exit_Process ();
	}

	ptr = SHM_Attach (*p_id);
	if (ptr == (char *)-1)
	{
		Debug (PGMLIN, "ERROR: Shm_Attach: cannot attach SHM {%d:%s}",
			SYS_NO, SYS_STR);
		Exit_Process ();
	}

	return (ptr);
}	/* End of Shm_Attach () */

/*-----------------------------------------------------------------------*/
int		SHM_Creat (key_t p_shmkey, size_t p_shmsize)
/*-----------------------------------------------------------------------*/
{
	int     rt;

	rt = shmget (p_shmkey, p_shmsize, 0664 | IPC_CREAT);

	return (rt);
}	/* End of SHM_Creat () */

/*-----------------------------------------------------------------------*/
char	*SHM_Attach (int p_shmid)
/*-----------------------------------------------------------------------*/
{
	char    *rt;

	rt = shmat (p_shmid, (char *)0, 0);

	return (rt);
}	/* End of SHM_Attach () */

/*-----------------------------------------------------------------------*/
char	*LtoU (char *p_ltoubuf, int p_len)
/*-----------------------------------------------------------------------*/
{
	int     i;

	for (i = 0; i < p_len; i ++)
	  *(p_ltoubuf+i) = (char)toupper (*(unsigned char *)(p_ltoubuf+i));

	return (p_ltoubuf);
}	/* End of LtoU ()	*/

/*----------------------------------------------------------------------*/
char	*UtoL (char *p_utolbuf, int p_len)
/*----------------------------------------------------------------------*/
{
	int		i;

	for (i = 0; i < p_len; i ++) 
		*(p_utolbuf+i) = (char)tolower (*(unsigned char *)(p_utolbuf+i));

	return (p_utolbuf);
}	/* End of UtoL ()	*/

/*----------------------------------------------------------------------*/
int		AtoIf (char *p_ascii, int p_len)
/*----------------------------------------------------------------------*/
{
	int		i, j, jj;

	j = jj = 0;

	for (i = 0; i < p_len; i ++)
	{
		for (j = 0; j < 10; j ++)
		{
			if (*(p_ascii+i) == ('0' + j))
				break;
		}

		if (j < 10)
			jj = jj * 10 + j;
	}

	return (jj);
}	/* End of AtoIf ()	*/

/*-----------------------------------------------------------------------*/
void	Exit_Process (void)
/*-----------------------------------------------------------------------*/
{
	register	i;

	SHM_Detach ((char *)SHM_All_Daemon_Info);

	for (i = 0; i < Process_Count; i ++)
		SHM_Detach ((char *)SHM_Mem[i]); 

	refresh ();
	endwin ();

	exit (OK);
}	/* End of Exit_Process ()	*/

/*-----------------------------------------------------------------------*/
void	SHM_Detach (char *p_shmptr)
/*-----------------------------------------------------------------------*/
{
	if (p_shmptr != NULL)
		shmdt (p_shmptr);

	return;
}	/* End of SHM_Detach ()	*/

/*----------------------------------------------------------------------*/
void	Register_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = SA_RESTART;
	act.sa_handler = Catch_Signal;

	if (sigaction (SIGPIPE, &act, NULL) < 0)
	{
		Debug (PGMLIN, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		Disp_Msg ("SIGPIPE error");
		sleep (1);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Debug (PGMLIN, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		Disp_Msg ("SIGTERM error");
		sleep (1);
		return;
	}
}   /* End of Register_Signal ()	*/

/*----------------------------------------------------------------------*/
void	Catch_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	char	msgbuf[80];

	Debug (PGMLIN, "signal (%d) occurred", signo);

	memset (msgbuf, 0, sizeof (msgbuf));
	sprintf (msgbuf, "signal (%d) occurred", signo);
	Disp_Msg (msgbuf);

	Exit_Process ();
}	/* End of Catch_Signal ()	*/

/*-----------------------------------------------------------------------*/
int		Trefresh (int timeout)
/*-----------------------------------------------------------------------*/
{
	int				rt, ret, c1, c2, flag;
	struct pollfd	fds;

	fds.fd = 0;
	fds.events = POLLIN;
	
	rt = poll (&fds, 1, timeout * 1000);
	if (rt > 0)
	{
		if (fds.revents & POLLIN)
		{				
			switch (getch ())
			{
				case	'\r':									/* CR	*/
				case	'\n':									/* LF	*/
					flag = 0;
					break;
				case 	KEY_ESC:
				case 	KEY_LEFT:
				case 	KEY_RIGHT:
				case 	'1':
					flag = -1;
					break;
				case	'p':
				case	'P':								/* Pause	*/
					Disp_Msg ("< Paused! > Enter:Retry");
					keypad (stdscr, TRUE);
					mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);
					c1 = getch ();
					if (c1 == 't' || c1 == 'T' ||
						c1 == KEY_LEFT || c1 == KEY_RIGHT)
						flag = -1;
					else
					{
						Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
						flag = 0;
					}
					break;
				case	'F':
				case	'f':					/* Change file R/W cnt	*/
					flag = 2;
					break;
				case	'I':
				case	'i':				/* Change process I/F seq	*/
					flag = 3;
					break;
				case	'Q':
				case	'q':									/* Quit	*/
					flag = 4;
					break;
				case	'A':
				case	'a':						/* Process or File	*/
					flag = 5;
					SwitchFlag = (SwitchFlag + 1) % 2;
					PageFlag = TimeFlag = MonitorFlag = 0;
					break;
				case	'J':
				case	'j':						/* adjust interval	*/
					flag = 6;
					break;
				case	'T':						/* if_seq/tr_e_tm	*/
				case	't':
					flag = 7;
					TimeFlag = (TimeFlag + 1) % 2;
					break;
				case	KEY_UP:
				case	'u':
					flag = 8;
					PageFlag = 0;
					break;
				case	KEY_DOWN:
				case	'd':
					flag = 9;
					PageFlag = 1;
					break;
				case	'M':
				case	'm':								/* monitor	*/
					flag = 10;
					MonitorFlag = 1;
					break;
				case	'Z':
				case	'z':							/* in/out file	*/
					while (1)
					{
						ret = Disp_IO ();
						if (ret == 0)
						{
							Disp_Msg ("Enter:Retry Esc:Cancel");
							keypad (stdscr, TRUE);
							mvcur (stdscr->_cury, stdscr->_curx, Rows - 1, 79);
							c1 = getch ();
							if (c1 == 't' || c1 == 'T' ||
								c1 == KEY_LEFT || c1 == KEY_RIGHT)
								flag = -1;
							else if (c1 == '\r' || c1 == '\n')
								continue;
							else
							{
								Clear_Lines (1, Rows - 2);
								Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
								flag = 0;
							}
							break;
						}
						else
						{
							Clear_Lines (1, Rows - 2);
							Disp_Msg ("Enter:Retry P:Pause F:ChgFile I:ChgProc A:Proc/File Z:I/O M:Monitor Q:Quit");
							flag = 0;
							break;
						}
					}
					break;
				default:
					flag = 1;
					break;
			}
		}
	}
	else if (rt == 0)
		flag = 0;
	else
		flag = -1;

	return (flag);
}	/* End of Trefresh ()	*/

/*----------------------------------------------------------------------*/
int		Disp_IO (void)
/*----------------------------------------------------------------------*/
{
	int 	pk, rt, i;
	char	buf[12], proc_name[12];

	Clear_Lines (Rows - 7, Rows - 2);
	Draw_Box (Rows - 7, 0, Rows - 2, 66);

	attron (A_REVERSE);
	mvaddstr (Rows - 6, 2,
		"  IFN         Process       OFN                                 ");
	mvaddstr (Rows - 5, 2, "1");
	mvaddstr (Rows - 4, 2, "2");
	mvaddstr (Rows - 3, 2, "3");
	mvaddstr (Rows - 5, 28, "1");
	mvaddstr (Rows - 4, 28, "2");
	mvaddstr (Rows - 3, 28, "3");
	mvaddstr (Rows - 5, 41, "4");
	mvaddstr (Rows - 4, 41, "5");
	mvaddstr (Rows - 3, 41, "6");
	mvaddstr (Rows - 5, 54, "7");
	mvaddstr (Rows - 4, 54, "8");
	mvaddstr (Rows - 3, 54, "9");
	attroff (A_REVERSE);

	Draw_Underline (Rows - 5, 16, 10);
	Disp_Msg ("Input <Process ID> (e.g. pa_1111_ts, Esc:Cancel)");

	while (1)
	{
		memset (&buf, 0, sizeof (buf));
		Draw_Underline (Rows - 5, 16, 10);

		rt = Get_String (Rows - 5, 16, 10, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Process ID> (e.g. pa_1111_ts, Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 10 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return (-1);
		else
		{
			memset (proc_name, 0, sizeof (proc_name));
			sprintf (proc_name, "%.10s", buf);
			proc_name[10] = 0;
			break;
		}
	}

	for (pk = 0; pk < DAEMON(Dk).p_count; pk ++)
	{
		if (memcmp (PROC(Dk,pk).process_id, proc_name, 10) == 0)
		{
			MvAddStr (Rows - 5, 16, 10, PROC(Dk,pk).process_id);
			attron (A_BOLD);

			for (i = 0; i < 3; i ++)
			{
				if (PROC(Dk,pk).in_d[i] != 0)
					MvAddStr (Rows - 5 + i, 4, 10, IDN(Dk,pk,i));
				else if (PROC(Dk,pk).in_f[i] != 0)
					MvAddStr (Rows - 5 + i, 4, 10, IFN(Dk,pk,i));
			}

			for (i = 0; i < 9; i ++)
			{
				if (PROC(Dk,pk).out_d[i] != 0)
					MvAddStr (Rows - 5 + i % 3, 30 + (i / 3) * 13,
						10, ODN(Dk,pk,i));
				else if (PROC(Dk,pk).out_f[i] != 0)
					MvAddStr (Rows - 5 + i % 3, 30 + (i / 3) * 13,
						10, OFN(Dk,pk,i));
			}

			attroff (A_BOLD);
			break;
		}

		if (pk == DAEMON(Dk).p_count - 1)
		{
			Disp_Msg ("▶▶▶ Process Unregistered ◀◀◀");
			sleep (1);
			return (NOTOK);
		}
	}

	return (OK);
}

/*-----------------------------------------------------------------------*/
void	G_Time (void)
/*-----------------------------------------------------------------------*/
{
	char		p_time[20];
	time_t		sys_time;
	struct tm	*dt;

	time (&sys_time);
	dt = localtime (&sys_time);

	sprintf (p_time, "%02d%02d%02d", dt->tm_hour, dt->tm_min, dt->tm_sec);
	attron (A_REVERSE);
	mvaddstr (0, 12, p_time);
	attroff (A_REVERSE);

	return;
}	/* End of G_Time ()	*/

/*----------------------------------------------------------------------*/
void	Change_File_Cnt (void)
/*----------------------------------------------------------------------*/
{
	int 	j, k, rt, seq, dk;
	char	buf[10], file_name[11], rw_flag[3];

	seq = 0;

	Clear_Lines (Rows - 6, Rows - 2);
	Draw_Box (Rows - 6, 0, Rows - 2, 50);

	attron (A_BOLD);
	mvaddstr (Rows - 5, 6, "File:            (pa_6101_dd, ...)");
	mvaddstr (Rows - 4, 6, "W/R :    (W1, SM, R1, R2, ..., R9)");
	mvaddstr (Rows - 3, 6, "Seq :");
	attroff (A_BOLD);

	Draw_Underline (Rows - 5, 12, 10);
	Draw_Underline (Rows - 4, 12, 2);
	Draw_Underline (Rows - 3, 12, 8);

	Disp_Msg ("Input <File Name>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 5, 12, 10);
		rt = Get_String (Rows - 5, 12, 10, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <File Name>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 10 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			memset (file_name, 0, sizeof (file_name));
			sprintf (file_name, "%.10s", buf);
			file_name[10] = '\n';
			break;
		}
	}

	dk = file_name[1] - 'a';

	Disp_Msg ("Input <Read/Write Flag>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 4, 12, 2);
		rt = Get_String (Rows - 4, 12, 2, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Read/Write Flag>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 2 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			LtoU (buf, 2);
			if (memcmp (buf, "R1", 2) != 0 && memcmp (buf, "R2", 2) != 0 &&
				memcmp (buf, "R3", 2) != 0 && memcmp (buf, "R4", 2) != 0 &&
				memcmp (buf, "R5", 2) != 0 && memcmp (buf, "R6", 2) != 0 &&
				memcmp (buf, "R7", 2) != 0 && memcmp (buf, "R8", 2) != 0 &&
				memcmp (buf, "R9", 2) != 0 && memcmp (buf, "SM", 2) != 0 &&
				memcmp (buf, "W1", 2) != 0)
				Disp_Msg ("Input CORRECT <Read/Write Flag>. (Esc:Cancel)");
			else
			{
				attron (A_UNDERLINE);
				mvaddstr (Rows - 4, 12, buf);
				attroff (A_UNDERLINE);
				memset (rw_flag, 0, sizeof (rw_flag));
				memcpy (rw_flag, buf, 2);
				break;
			}
		}
	}

	Disp_Msg ("Input <R/W Sequence>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 3, 12, 8);
		rt = Get_String (Rows - 3, 12, 8, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <R/W Sequence>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input less than 9 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			if (Chk_Digit (buf) == 0)
				Disp_Msg ("Numbers only. (Esc:Cancel)");
			else
			{
				seq = atoi (buf);
				break;
			}
		}
	}

	rt = Yes_No ("Are you sure to change?", NO);
	if (rt == YES)
	{
		for (j = 0; j < DAEMON(dk).d_count; j ++)
		{
			if (memcmp (DSHM(dk,j).data_name, file_name, 10) == 0)
			{
				k = AtoIf (&rw_flag[1], 1) - 1;

				if (rw_flag[0] == 'R')
				{
					DSHM(dk,j).r_cnt[k] = seq;
					Debug (PGMLIN, "DSHM Cnt changed[%.10s,%d,%s]",
						file_name, seq, rw_flag);
					Disp_Msg ("▶▶▶ Changed ◀◀◀");
					sleep (1);
					return;
				}
				else if (rw_flag[0] == 'S')
				{
					DSHM(dk,j).sm_r_cnt = seq;
					Debug (PGMLIN, "DSHM Cnt changed[%.10s,%d,%s]",
						file_name, seq, rw_flag);
					Disp_Msg ("▶▶▶ Changed ◀◀◀");
					sleep (1);
					return;
				}
				else if (rw_flag[0] == 'W')
				{
					DSHM(dk,j).w_cnt[k] = seq;
					Debug (PGMLIN, "DSHM Cnt changed[%.10s,%d,%s]",
						file_name, seq, rw_flag);
					Disp_Msg ("▶▶▶ Changed ◀◀◀");
					sleep (1);
					return;
				}
			}
		}

		if (j == DAEMON(dk).d_count)
		{
			for (j = 0; j < DAEMON(dk).f_count; j ++)
			{
				if (memcmp (FILEM(dk,j).file_name, file_name, 10) == 0)
				{
					k = AtoIf (&rw_flag[1], 1) - 1;

					if (rw_flag[0] == 'R')
					{
						FILEM(dk,j).r_cnt[k] = seq;
						Debug (PGMLIN, "File Cnt changed[%.10s,%d,%s]",
							file_name, seq, rw_flag);
						Disp_Msg ("▶▶▶ Changed ◀◀◀");
						sleep (1);
						return;
					}
					else if (rw_flag[0] == 'W')
					{
						FILEM(dk,j).w_cnt[k] = seq;
						Debug (PGMLIN, "File Cnt changed[%.10s,%d,%s]",
							file_name, seq, rw_flag);
						Disp_Msg ("▶▶▶ Changed ◀◀◀");
						sleep (1);
						return;
					}
				}
			}
		}
		Disp_Msg ("▶▶▶ Not Changed ◀◀◀");
	}
	else
		Disp_Msg ("▶▶▶ Cancelled ◀◀◀");
	sleep (1);

	return;
}

/*----------------------------------------------------------------------*/
void	Change_If_Seq (void)
/*----------------------------------------------------------------------*/
{
	int 	j, rt, seq, dk;
	char	buf[10], proc_name[11];

	seq = 0;

	Clear_Lines (Rows - 5, Rows - 2);
	Draw_Box (Rows - 5, 0, Rows - 2, 50);

	attron (A_BOLD);
	mvaddstr (Rows - 4, 6, "Process:            (pa_1111_ts, ...)");
	mvaddstr (Rows - 3, 6, "I/F Seq:");
	attroff (A_BOLD);

	Draw_Underline (Rows - 4, 15, 10);
	Draw_Underline (Rows - 3, 15, 8);

	Disp_Msg ("Input <Process Name>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 4, 15, 10);
		rt = Get_String (Rows - 4, 15, 10, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Process Name>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input 10 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			memset (proc_name, 0, sizeof (proc_name));
			sprintf (proc_name, "%.10s", buf);
			proc_name[10] = '\n';
			break;
		}
	}

	dk = proc_name[1] - 'a';

	Disp_Msg ("Input <Interface Sequence>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 3, 15, 8);
		rt = Get_String (Rows - 3, 15, 8, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Input <Interface Sequence>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input less than 9 characters. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			if (Chk_Digit (buf) == 0)
				Disp_Msg ("Numbers only. (Esc:Cancel)");
			else
			{
				seq = atoi (buf);
				break;
			}
		}
	}

	rt = Yes_No ("Are you sure to change?", NO);
	if (rt == YES)
	{
		for (j = 0; j < DAEMON(dk).p_count; j ++)
		{
			if (memcmp (PROC(dk,j).process_id, proc_name, 10) == 0)
			{
				PROC(dk,j).if_seq = seq;
				Debug (PGMLIN, "IfSeq changed[%.10s,%d]", proc_name, seq);
				Disp_Msg ("▶▶▶ Changed ◀◀◀");
				sleep (1);
				return;
			}
		}
		Disp_Msg ("▶▶▶ Not Changed ◀◀◀");
	}
	else
		Disp_Msg ("▶▶▶ Cancelled ◀◀◀");
	sleep (1);

	return;
}

/*----------------------------------------------------------------------*/
void	Help_Proc (int flag)
/*----------------------------------------------------------------------*/
{
	attron (A_BOLD);
	mvaddstr (2, 38, "HELP");
	attroff (A_BOLD);

	if (flag == 1)
	{
		mvaddstr (4, 12, "PID ▶ process ID");
		mvaddstr (5, 12, "CMD ▶ process name (PROC.process_id)");
		mvaddstr (6, 14, "e ▶ exist flag (process 실제 기동 시 '*'로 표시)");
		mvaddstr (7, 14, "P ▶ process status (PSTAT)");
		mvaddstr (8, 14, "L ▶ line status (LSTAT)");
		mvaddstr (9, 14, "N ▶ network status (NSTAT)");
		mvaddstr (10, 14,
			"s ▶ PROC.start status (0:init 1:start 2:end 3:stop)");
		mvaddstr (11, 14, "p ▶ PROC.process_status (1:run 2:stop 9:not run)");
		mvaddstr (12, 14, "l ▶ LINE_GUBUN (0:main[primary] 1:backup)");
		mvaddstr (14, 12, "STM ▶ PROC.start_time (hhmm)");
		mvaddstr (15, 12, "ETM ▶ PROC.end_time (hhmm)");
		mvaddstr (16, 11, "TOUT ▶ PROC.timeout");
		mvaddstr (17, 11, "DLAY ▶ PROC.delay (millisec)");
		attron (A_BOLD);
		mvaddstr (19, 12, "BOLD");
		attroff (A_BOLD);
		mvaddstr (19, 17, "type으로 표시된 것은 PROC.process_no가 0인 경우임");
	}
	else if (flag == 2)
	{
		mvaddstr (4, 12, "Cmd ▶ process name (PROC.process_id)");
		mvaddstr (6, 12, "Pid ▶ process ID (PROC.process_no)");
		mvaddstr (8, 14, "S ▶ PROC.start status");
		mvaddstr (9, 19, "(0:init 1:start 2:end 3:stop)");
		mvaddstr (11, 14, "P ▶ PROC.process_status");
		mvaddstr (12, 19, "(1:run 2:stop 9:not run)");
		mvaddstr (14, 14, "L ▶ LINE_GUBUN (0:main 1:backup)");
	}
	else
	{
		mvaddstr (4, 12, "Pid ▶ pid");
		mvaddstr (6, 14, "P ▶ O:run .:stop");
		mvaddstr (8, 12, "Cmd ▶ process name");
		mvaddstr (10, 11, "STRT ▶ start time");
		mvaddstr (12, 12, "END ▶ end time");
		mvaddstr (14, 14, "S ▶ system status (0:off 1:on)");
	}
	Draw_Line (Rows - 2, 0, 79);

	return;
}

/*----------------------------------------------------------------------*/
int		Change_Interval (void)
/*----------------------------------------------------------------------*/
{
	int 	rt, tout;
	char	buf[10];

	tout = 10;

	Clear_Lines (Rows - 2, Rows - 2);

	attron (A_BOLD);
	mvaddstr (Rows - 2, 0, "update interval:");
	attroff (A_BOLD);

	Draw_Underline (Rows - 2, 17, 2);

	Disp_Msg ("Enter <update interval in sec>. (Esc:Cancel)");
	while (1)
	{
		memset (&buf, 0, sizeof buf);
		Draw_Underline (Rows - 2, 17, 2);
		rt = Get_String (Rows - 2, 17, 2, buf);
		if (rt == -1)											/* skip	*/
			Disp_Msg ("Enter <update interval in sec>. (Esc:Cancel)");
		else if (rt == -3)						/* Over the input limit	*/
			Disp_Msg ("Input one or two digits. (Esc:Cancel)");
		else if (rt == -4)									/* Space	*/
			Disp_Msg ("You cannot input SPACE (Esc:Cancel)");
		else if (rt == 1)										/* Esc	*/
			return;
		else
		{
			if (Chk_Digit (buf) == 0)
				Disp_Msg ("Numbers only. (Esc:Cancel)");
			else
			{
				tout = atoi (buf);
				break;
			}
		}
	}

	return (tout);
}	/* End of Change_Interval ()	*/

/*----------------------------------------------------------------------*/
pid_t	Check_Proc (char *pname)
/*----------------------------------------------------------------------*/
{
	pid_t	rt;

#if defined __hpux
	rt = Check_Proc_HPUX (pname);
#elif defined sun
	rt = Check_Proc_SUN (pname);
#elif defined _AIX
	rt = Check_Proc_AIX (pname);
#endif

	return (rt);
}	/* End of Check_Proc ()	*/

#if defined __hpux
/*************************************************************************
	Function		: . check process - hpux
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_HPUX (char *proc_name)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	struct pst_status	pst[BURST];

	idx = 0;
	uid = getuid ();

	while ((cnt = pstat_getproc (pst, sizeof (pst[0]), BURST, idx)) > 0)
	{
		for (i = 0; i < cnt; i ++)
		{
			if (uid != pst[i].pst_uid)
				continue;

			if (memcmp (proc_name, pst[i].pst_cmd, strlen (proc_name)) == 0)
				return (pst[i].pst_pid);
		}
		idx = pst[cnt-1].pst_idx + 1;
	}

	return (0);
}	/* End of Check_Proc_HPUX ()	*/
#endif

#if defined sun
/*************************************************************************
	Function		: . check process - sun
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_SUN (char *proc_name)
/*----------------------------------------------------------------------*/
{
	pid_t			rt;
	char			*directory="/proc/";	
	DIR				*dp;
	struct dirent	*dirp;
	struct stat		statbuf;

	Exist_Flag = 0;

	if (lstat (directory, &statbuf) < 0)
	{
		Debug (PGMLIN, "Check_Proc_SUN:lstat {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	if (S_ISDIR (statbuf.st_mode) == 0)
	{
		Debug (PGMLIN, "Check_Proc_SUN:S_ISDIR {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	if ((dp = opendir (directory)) == NULL)
	{
		Debug (PGMLIN, "Check_Proc_SUN:opendir {%d:%s}", SYS_NO, SYS_STR);
		return (0);
	}

	while ((dirp = readdir (dp)) != NULL) 
	{
		if (!strcmp (dirp->d_name, ".") || !strcmp (dirp->d_name, ".."))
			continue;
		if (Make_List (dirp->d_name, proc_name) == OK)
		{
			closedir (dp);
			rt = atoi (dirp->d_name);
			return (rt);
		}
	}
	closedir (dp);

	return (0);
}	/* End of Check_Proc_SUN ()	*/

/*************************************************************************
	Function		: . make process list
	Parameters IN	: . pid			: process pid
					  . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . int (0: run, -1: not run)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Make_List (char *pid, char *proc_name)
/*----------------------------------------------------------------------*/
{
	int 		fd;
	char		proc_path[PATH_MAX];
	prpsinfo_t	prpinfo;
  
	sprintf (proc_path, "/proc/%s", pid);
	if ((fd = open (proc_path, O_RDONLY)) == -1)
		return (NOTOK);

	if (ioctl (fd, PIOCPSINFO, &prpinfo) == -1)
	{
		close (fd);
		return (NOTOK);
	}

	if (getuid () != prpinfo.pr_uid)
	{
		close (fd);
		return (NOTOK);
	}

	if (memcmp (prpinfo.pr_psargs, proc_name, strlen (proc_name)) == 0)
	{
		close (fd);
		Exist_Flag ++;
		return (Exist_Flag == 1 ? OK : NOTOK);
	}

	close (fd);

	return (NOTOK);
}	/* End of Make_List ()	*/
#endif

#if defined _AIX
/*************************************************************************
	Function		: . check process - aix
	Parameters IN	: . proc_name	: process name
	Parameters OUT	: .
	Return Code		: . 0:process not run, !=0:process pid
*************************************************************************/
/*----------------------------------------------------------------------*/
pid_t	Check_Proc_AIX (char *proc_name)
/*----------------------------------------------------------------------*/
{
	int					i, cnt, idx, uid;
	pid_t				pid, next_pid;
	struct procsinfo	pi[BURST];

	idx = 0;
	uid = getuid ();

	memset (pi, 0, sizeof (pi));
	next_pid = 0;

	while (1)
	{
		pid = next_pid;

		cnt = getprocs (pi, sizeof (pi[0]), NULL, 0, &pid, BURST);
		if (cnt <= 0)
			break;

		for (i = 0; i < cnt; i ++)
		{
			if (uid != pi[i].pi_uid)
				continue;

			if (memcmp (proc_name, pi[i].pi_comm, strlen (proc_name)) == 0)
				return (pi[i].pi_pid);
		}
		next_pid = pid;
	}

	return (0);
}	/* End of Check_Proc_AIX ()	*/
#endif

/*************************************************************************
	End of Program (py_libs_cm.c)
*************************************************************************/
