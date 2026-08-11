#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <termios.h>

#if 1
#include <endian.h>
#else
#define __BYTE_ORDER	__BIG_ENDIAN
#endif

#ifndef WIN_H
#define	WIN_H


#define WIN_ATTR_BLACK				0x0
#define WIN_ATTR_RED				0x1
#define WIN_ATTR_GREEN				0x2
#define WIN_ATTR_YELLOW				0x3
#define WIN_ATTR_BLUE				0x4
#define WIN_ATTR_MAGENTA			0x5
#define WIN_ATTR_CYAN				0x6
#define WIN_ATTR_WHITE				0x7

#define	WIN_AFBLA					0x0					/* Attribute Foreground Black */
#define	WIN_AFRED					0x1
#define	WIN_AFGRE					0x2
#define	WIN_AFYEL					0x3
#define	WIN_AFBLU					0x4
#define	WIN_AFMAG					0x5
#define	WIN_AFCYA					0x6
#define	WIN_AFWHI					0x7
#define	WIN_AFBBLA					0x8					/* Attribute Foreground Bright Black */
#define	WIN_AFBRED					0x9
#define	WIN_AFBGRE					0xa
#define	WIN_AFBYEL					0xb
#define	WIN_AFBBLU					0xc
#define	WIN_AFBMAG					0xd
#define	WIN_AFBCYA					0xe
#define	WIN_AFBWHI					0xf

#define	WIN_ABBLA					0x00				/* Attribute Background Black */
#define	WIN_ABRED					0x10
#define	WIN_ABGRE					0x20
#define	WIN_ABYEL					0x30
#define	WIN_ABBLU					0x40
#define	WIN_ABMAG					0x50
#define	WIN_ABCYA					0x60
#define	WIN_ABWHI					0x70
#define	WIN_ABBBLA					0x80				/* Attribute Background Bright Black */
#define	WIN_ABBRED					0x90
#define	WIN_ABBGRE					0xa0
#define	WIN_ABBYEL					0xb0
#define	WIN_ABBBLU					0xc0
#define	WIN_ABBMAG					0xd0
#define	WIN_ABBCYA					0xe0
#define	WIN_ABBWHI					0xf0

/*
#define	WIN_ATTR_BLACK				0x0
#define	WIN_ATTR_RED				0x1
#define	WIN_ATTR_GREEN				0x2
#define	WIN_ATTR_YELLOW				0x3
#define	WIN_ATTR_BLUE				0x4
#define	WIN_ATTR_MAGENTA			0x5
#define	WIN_ATTR_CYAN				0x6
#define	WIN_ATTR_WHITE				0x7

#define	WIN_ATTR_BACK_BLACK			0x00
#define	WIN_ATTR_BACK_RED			0x10
#define	WIN_ATTR_BACK_GREEN			0x20
#define	WIN_ATTR_BACK_YELLOW		0x30
#define	WIN_ATTR_BACK_BLUE			0x40
#define	WIN_ATTR_BACK_MAGENTA		0x50
#define	WIN_ATTR_BACK_CYAN			0x60
#define	WIN_ATTR_BACK_WHITE			0x70

#define	WIN_ATTR_B_BLACK			0x00
#define	WIN_ATTR_B_RED				0x10
#define	WIN_ATTR_B_GREEN			0x20
#define	WIN_ATTR_B_YELLOW			0x30
#define	WIN_ATTR_B_BLUE				0x40
#define	WIN_ATTR_B_MAGENTA			0x50
#define	WIN_ATTR_B_CYAN				0x60
#define	WIN_ATTR_B_WHITE			0x70
*/

#define WIN_KEY_F1					0x001b4f50
#define WIN_KEY_F2					0x001b4f51
#define WIN_KEY_F3					0x001b4f52
#define WIN_KEY_F4					0x001b4f53
#define WIN_KEY_F5					0x5b31357e
#define WIN_KEY_F6					0x5b31377e
#define WIN_KEY_F7					0x5b31387e
#define WIN_KEY_F8					0x5b31397e
#define WIN_KEY_F9					0x5b32307e
#define WIN_KEY_F10					0x5b32317e
#define WIN_KEY_F11					0x5b32337e
#define WIN_KEY_F12					0x5b32347e

#define WIN_KEY_SF1					0x5b32337e
#define WIN_KEY_SF2					0x5b32347e
#define WIN_KEY_SF3					0x5b32357e
#define WIN_KEY_SF4					0x5b32367e
#define WIN_KEY_SF5					0x5b32387e
#define WIN_KEY_SF6					0x5b32397e
#define WIN_KEY_SF7					0x5b33317e
#define WIN_KEY_SF8					0x5b33327e
#define WIN_KEY_SF9					0x5b33337e
#define WIN_KEY_SF10				0x5b33347e
#define WIN_KEY_SF11				0x5b32337e
#define WIN_KEY_SF12				0x5b32347e

#define WIN_KEY_INS					0x1b5b327e
#define WIN_KEY_HOME				0x1b5b317e
#define WIN_KEY_PGUP				0x1b5b357e
#define WIN_KEY_DEL					0x1b5b337e
#define WIN_KEY_END					0x1b5b347e
#define WIN_KEY_PGDN				0x1b5b367e

#define WIN_KEY_TAB					0x00000009
#define WIN_KEY_ESC					0x0000001b
#define WIN_KEY_BACKSPACE			0x00000008
#define WIN_KEY_ENTER				0x0000000d
#define WIN_KEY_SPACE				0x00000020

#define WIN_KEY_UP					0x001b5b41
#define WIN_KEY_DOWN				0x001b5b42
#define WIN_KEY_LEFT				0x001b5b44
#define WIN_KEY_RIGHT				0x001b5b43


#define	WIN_CSI						"\e["
#define	WIN_ATTR_FORE_COLOR			0
#define	WIN_ATTR_BACK_COLOR			1
#define	WIN_ATTR_BRIGHT				2
#define	WIN_ATTR_REVERSE			3
#define	WIN_ATTR_BOLD				4
#define	WIN_ATTR_ITALIC				5
#define	WIN_ATTR_UNDERLINE			6
#define	WIN_ATTR_CONCEAL			7
#define	WIN_ATTR_CROSSED			8
#define	WIN_ATTR_DELETE				9
#define	WIN_ATTR_BLINK				10		/* 1=BLINK,2=SLOW,3=FAST */
#define	WIN_ATTR_MESSAGE			"70010100000"
#define	WIN_ATTR_MAP				"70000000000"
#define	WIN_ATTR_EDIT				"70010000000"
#define	WIN_ATTR_MENU				"70010000000"

typedef struct _window_
{
	int				out;
	int				in;
	char			*buf_ptr;
	int				buf_sz;
	int				buf_pos;
	char			attr[ 32];
	int				curx;
	int				cury;
	int				sz_x;
	int				sz_y;
	int				key;
	struct termios	tty_attr;
	struct termios	old_tty_attr;
	void			(*old_sigwin)( int sigid);
}	WIN;

#endif

extern WIN	*__Win;

/***** Module : win.c *****/
void        WinSigSize( int sigid);                                         /* Win */
int         WinInit();                                                      /* Win */
int         WinEnd();                                                       /* Win */
WIN*        Win_Open( char *tty_name);                                      /* Win */
int         Win_Close( WIN *win);                                           /* Win */
int         Win_Output( WIN *win, int desc);                                /* Win */
int         Win_SetTty( WIN *win);                                          /* Win */
int         Win_UnsetTty( WIN *win);                                        /* Win */
int         Win_GetKey( WIN *win, int timeout);                             /* Win */
int         Win_Flush( WIN *win);                                           /* Win */
int         Win_PutString( WIN *win, char *data);                           /* string 출력 */
int         Win_PutChar( WIN *win, char data);                              /* char 출력 */
int         Win_PutXYA( WIN *win, int x, int y, char *attr, char *data);    /* Win */
int         Win_PrintNXYA( WIN *win, int sz, int x, int y, char *attr, char *format, ...);/* Win */
int         Win_PrintXYA( WIN *win, int x, int y, char *attr, char *format, ...);/* Win */
int         Win_PrintXY( WIN *win, int x, int y, char *format, ...);        /* Win */
int         Win_Print( WIN *win, char *format, ...);                        /* Win */
int         Win_GotoXY( WIN *win, int x, int y);                            /* Win */
int         Win_CursorOn( WIN *win);                                        /* Win */
int         Win_CursorOff( WIN *win);                                       /* Win */
int         Win_Attr( WIN *win, char *attr);                                /* Win */
int         Win_SetBuffer( WIN *win, char *buffer, int sz);                 /* Win */
int         Win_GetBuffer( WIN *win);                                       /* Win */
int         Win_PutMessage( WIN *win, char *string, int sz);                /* Win */
int         Win_GetXY( WIN *win, int *x, int *y);                           /* Win */
int         Win_GetSize( WIN *win, int *x, int *y);                         /* Win */
int         Win_Message( WIN *win, char *msg, int sz);                      /* Win */
int         Win_MessageXYZ( WIN *win, int x, int y, int z, char *msg, int sz);/* Win */

