/** ***************************************************************************
**  @file       win.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  화면출력 제어 library
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#include <unistd.h>
#include <termios.h>
#include <signal.h>

#include "log.h"
#include "win.h"

WIN		*__Win = NULL;
static void	(*OldSigSize)( int sig_id);

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
void WinSigSize( int sigid)
{
	Win_GetSize( __Win, &__Win->sz_x, &__Win->sz_y);
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int WinInit()
{
	__Win = Win_Open( NULL);
	if( __Win == NULL)
	{
		LogCri( "WinInit error.");
		return -1;
	}

	Win_SetTty( __Win);
    Win_GetSize( __Win, &__Win->sz_x, &__Win->sz_y);
	OldSigSize = signal( SIGWINCH, WinSigSize);

	Win_Print( __Win, "%s%dJ", WIN_CSI, 2);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int WinEnd()
{
	signal( SIGWINCH, OldSigSize);
	Win_UnsetTty( __Win);
	Win_GotoXY( __Win, 0, __Win->sz_y);
	Win_Close( __Win);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
WIN *Win_Open( char *tty_name)
{
	WIN	*win;

	win = malloc( sizeof( WIN));
	if( win == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( WIN));
		return NULL;
	}
	memset( win, 0, sizeof( WIN));

	if( tty_name == NULL)
	{
		win->out = STDOUT_FILENO;
		win->in = STDIN_FILENO;
	}
	else
	{
	}

	/*
	Win_GetSize( win);
	Win_SetTty( win); 
	*/

	return win;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Close( WIN *win)
{
	free( win);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Output( WIN *win, int desc)
{
	win->out = desc;

	return desc;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_SetTty( WIN *win)
{
	struct termios	*termios_p;

	termios_p = &win->tty_attr;
	tcgetattr( win->in, &win->old_tty_attr);
	memcpy( termios_p, &win->old_tty_attr, sizeof( struct termios));

	termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_p->c_cflag &= ~(CSIZE | PARENB);
	termios_p->c_cflag |= CS8;

	
	termios_p->c_cc[VMIN] = 1;
	termios_p->c_cc[VTIME] = 0;

	tcsetattr( win->in, TCSANOW, termios_p);
#if 0
	c = getchar();
	printf( "[%d]\n", c);
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr);
#endif

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_UnsetTty( WIN *win)
{
	tcsetattr( win->in, TCSANOW, &win->old_tty_attr);
	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_GetKey( WIN *win, int timeout)
{
	int				rtn;
	unsigned char	c = 0;
	unsigned int	i;

	fd_set			rfds;
	struct timeval	_tv, *tv;

	c = 0;
	i = 0;
	FD_ZERO( &rfds);

	while( 1)
	{
		FD_SET( 0, &rfds);

		if( timeout == 0)	tv = NULL;
		else
		{
			tv = &_tv;
			tv->tv_sec  = timeout / 1000000;
			tv->tv_usec = timeout % 1000000;
		}

		rtn = select( 1, &rfds, NULL, NULL, tv);
		if( rtn < 0)
		{
			LogErr( "select error. rtn=[%d] timeout=[%d]", rtn, timeout);
			return -1;
		}
		else if( rtn == 0)
		{
			win->key = i;
			return i;
		}

		LogDel( "select rtn. rtn=[%d]", rtn);

		if( FD_ISSET( 0, &rfds))
		{
			read( 0, &c, 1);
			i <<= 8;
			i += c;
			timeout = 1;
		}
	}

	return i;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Flush( WIN *win)
{
	// fflush( win->out);

	return 1;
}


/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  string 출력
***************************************************************************** */
int Win_PutString( WIN *win, char *data)
{
	int			sz;

	sz = strlen( data);

	Win_PutMessage( win, data, sz);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  char 출력
***************************************************************************** */
int Win_PutChar( WIN *win, char data)
{
	Win_PutMessage( win, &data, 1);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_PutXYA( WIN *win, int x, int y, char *attr, char *data)
{
	int			sz;

	sz = strlen( data);

	Win_GotoXY( win, x, y);
	Win_Attr( win, attr);
	Win_PutMessage( win, data, sz);
	Win_Attr( win, 0);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_PrintNXYA( WIN *win, int sz, int x, int y, char *attr, char *format, ...)
{
	va_list		args;
	int			dat_sz;
	char		buf[ 8192];

	va_start( args, format);
	dat_sz = vsnprintf( buf, 8192, format, args);
	va_end( args);


	// if( x == 72)
	LogDel( "dat_sz=[%d] data=[%s]", dat_sz, buf);
	if( dat_sz >= sz)	buf[ sz] = 0;
	else
	{
		while( dat_sz < sz) buf[ dat_sz++] = ' ';
		buf[ sz] = 0;
	}

	Win_GotoXY( win, x, y);
	Win_Attr( win, attr);
	Win_PutMessage( win, buf, sz);
	Win_Attr( win, 0);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_PrintXYA( WIN *win, int x, int y, char *attr, char *format, ...)
{
	va_list		args;
	int			sz;
	char		buf[ 8192];

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	Win_GotoXY( win, x, y);
	Win_Attr( win, attr);
	Win_PutMessage( win, buf, sz);
	Win_Attr( win, 0);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_PrintXY( WIN *win, int x, int y, char *format, ...)
{
	va_list		args;
	int			sz;
	char		buf[ 8192];

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	Win_GotoXY( win, x, y);
	Win_PutMessage( win, buf, sz);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Print( WIN *win, char *format, ...)
{
	va_list		args;
	int			sz;
	char		buf[ 8192];

	va_start( args, format);
	sz = vsnprintf( buf, 8192, format, args);
	va_end( args);

	Win_PutMessage( win, buf, sz);

	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_GotoXY( WIN *win, int x, int y)
{
	win->curx = x;
	win->cury = y;

	LogDel( "win goto x=[%d:%d] y=[%d:%d]", x, __Win->sz_x, y, __Win->sz_y);
	Win_Print( win, "%s%d;%dH", WIN_CSI, win->cury, win->curx);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_CursorOn( WIN *win)
{
	Win_Print( win, "%s?25h", WIN_CSI);
	Win_Flush( win);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_CursorOff( WIN *win)
{
	Win_Print( win, "%s?25l", WIN_CSI);
	Win_Flush( win);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Attr( WIN *win, char *attr)
{
	int		bright = 0;

	if( attr == NULL)
	{
		Win_Print( win, "%s0m", WIN_CSI);
		return 0;
	}

	memcpy( win->attr, attr, sizeof( win->attr));

	LogDel( "--------------------------------------------------------");
	LogDel( "win->attr                 = [%s]", win->attr);
	LogDel( "win->attr.b.fore          = [%c]", win->attr[ WIN_ATTR_FORE_COLOR]);
	LogDel( "win->attr.b.back          = [%c]", win->attr[ WIN_ATTR_BACK_COLOR]);
	LogDel( "win->attr.b.blink         = [%c]", win->attr[ WIN_ATTR_REVERSE]);
	LogDel( "win->attr.b.blink         = [%c]", win->attr[ WIN_ATTR_BOLD]);
	LogDel( "win->attr.b.crossed       = [%c]", win->attr[ WIN_ATTR_ITALIC]);
	LogDel( "win->attr.b.conceal       = [%c]", win->attr[ WIN_ATTR_UNDERLINE]);
	LogDel( "win->attr.b.underline     = [%c]", win->attr[ WIN_ATTR_CONCEAL]);
	LogDel( "win->attr.b.italic        = [%c]", win->attr[ WIN_ATTR_CROSSED]);
	LogDel( "win->attr.b.bold          = [%c]", win->attr[ WIN_ATTR_DELETE]);
	LogDel( "win->attr.b.reverse       = [%c]", win->attr[ WIN_ATTR_BLINK]);

	if( win->attr[ WIN_ATTR_REVERSE] == '1')		Win_Print( win, "%s%dm", WIN_CSI, 7);
	if( win->attr[ WIN_ATTR_BOLD] == '1')			Win_Print( win, "%s%dm", WIN_CSI, 1);
	if( win->attr[ WIN_ATTR_ITALIC] == '1')			Win_Print( win, "%s%dm", WIN_CSI, 3);
	if( win->attr[ WIN_ATTR_UNDERLINE] == '1')		Win_Print( win, "%s%dm", WIN_CSI, 4);
	if( win->attr[ WIN_ATTR_CONCEAL] == '1')		Win_Print( win, "%s%dm", WIN_CSI, 8);
	if( win->attr[ WIN_ATTR_CROSSED] == '1')		Win_Print( win, "%s%dm", WIN_CSI, 9);
	if( win->attr[ WIN_ATTR_BLINK] != '0')			Win_Print( win, "%s%dm", WIN_CSI, 5);
	if( win->attr[ WIN_ATTR_BLINK] == '1')			Win_Print( win, "%s%dm", WIN_CSI, 6);

	if( win->attr[ WIN_ATTR_BRIGHT] == '1')			bright = 8;
	Win_Print( win, "%s38;5;%dm", WIN_CSI, win->attr[ WIN_ATTR_FORE_COLOR] - '0' + bright);
	Win_Print( win, "%s48;5;%dm", WIN_CSI, win->attr[ WIN_ATTR_BACK_COLOR] - '0' + bright);

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_SetBuffer( WIN *win, char *buffer, int sz)
{
	win->buf_ptr = buffer;
	win->buf_sz = sz;
	win->buf_pos = 0;

	return win->out;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_GetBuffer( WIN *win)
{
	int		rtn;

	rtn = win->buf_pos;
	win->buf_pos = 0;
	return rtn;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_PutMessage( WIN *win, char *string, int sz)
{
	if( win->buf_sz > 0)
	{
		if( win->buf_ptr != NULL)
		{
			if( sz + win->buf_pos > win->buf_sz) return 0;
			memcpy( &win->buf_ptr[ win->buf_pos], string, sz);
			win->buf_pos += sz;
			win->buf_ptr[ win->buf_pos] = 0;
			return sz;
		}
		return 0;
	}
	write( win->out, string, sz);
	return sz;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_GetXY( WIN *win, int *x, int *y)
{
	*x = win->curx;
	*y = win->cury;

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_GetSize( WIN *win, int *x, int *y)
{
	struct winsize		w;

	ioctl( STDOUT_FILENO, TIOCGWINSZ, &w);

	*x = w.ws_col;
	*y = w.ws_row;

	if( win != NULL)
	{
		win->sz_x = *x;
		win->sz_y = *y;
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_Message( WIN *win, char *msg, int sz)
{
	int		i = 0;
	char	buf[ 512];

	Win_GotoXY( __Win, 0, __Win->sz_y);
	Win_Attr( __Win, WIN_ATTR_MESSAGE);
	Win_PutMessage( __Win, " MESSAGE ", 9);
	Win_Attr( __Win, 0x00000000);
	Win_PutMessage( __Win, " ", 1);
	Win_PutMessage( __Win, msg, sz);
	i = __Win->sz_x - ( sz + 10);
	memset( buf, 0x20, i);
	buf[ i] = 0;
	Win_PutMessage( __Win, buf, i);
	Win_Flush( __Win);


	return 1;
}

/** ***************************************************************************
**  @func       int Win_( WIN *win, )
**  @param      WIN *win - WIN pointer
**  @return     성공 - +
**  @retval     실패 - -
**  @brief
**  Win
***************************************************************************** */
int Win_MessageXYZ( WIN *win, int x, int y, int z, char *msg, int sz)
{
	int		i = 0;
	char	buf[ 512];

	Win_GotoXY( win, x, y);
	Win_Attr( win, WIN_ATTR_MESSAGE);
	Win_PutMessage( win, " MESSAGE ", 9);
	Win_Attr( win, 0x00000000);
	Win_PutMessage( win, " ", 1);
	Win_PutMessage( win, msg, sz);
	i = z - ( sz + 10);
	memset( buf, 0x20, i);
	buf[ i] = 0;
	Win_PutMessage( win, buf, i);
	Win_Flush( win);


	return 1;
}



