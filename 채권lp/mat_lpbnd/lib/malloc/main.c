#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "malloc.h"
#include "cfg.h"
#include "cmd.h"
#include "map.h"
#include "task.h"

CFG				*Cfg;
extern CMD		*Cmd;
MAP				*Map;
extern CMD_TBL	CmdTable[];

char	CfgFilePath[ 512] = ".";
char	CfgFileName[ 512] = "packet.cfg";

PARAM	Param = 
{
	0,										/* argc */
	NULL,									/* argv */
	NULL,									/* envp */
	"./test.cfg"
};


char	*OptStr = "ho:";
char	Usage[] = 
"usage: %s -h -o out_file_name [header_file ... ]\n"
"\to: output file name (default:stdout);\n"
"\th: help message\n";

/***** Module : main.c *****/
int         main( int argc, char *argv[], char *envp[]);
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         MainProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);
int         getch();
int         getch_test();
int         get_winsize();

int main( int argc, char *argv[], char *envp[])
{
	int		i;
	int		rtn;

	rtn = GetOption( argc, argv);
	if( rtn <= 0)
	{
		printf( Usage, argv[ 0]);
		return -1;
	}

	Param.argc = argc;
	Param.argv = argv;
	Param.envp = envp;

	rtn = InitProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "InitProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	rtn = MainProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	rtn = TermProcess( argc, argv);
	if( rtn < 0)
	{
		LogCri( "MainProcess error. rtn=[%d]", rtn);
		return rtn;
	}

	return 1;
}

int GetOption( int argc, char *argv[])
{
	int		opt;
	char	*p;

	while( 1)
	{
		opt = getopt( argc, argv, OptStr);
		if( opt < 0) break;

		switch( opt)
		{
			case 'h' :
				return 0;
			default  :
				break;
		}
	}
	return 1;
}

int InitProcess( int argc, char *argv[])
{
	int		rtn;

	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. ptr=[%p]", CmdTable);
		return -1;
	}
	Cmd_SetPrompt( Cmd, "[map] ");

	ParamPrint( &Param);
	return 1;
}

int MainProcess( int argc, char *argv[])
{
	int		rtn = 0;

	while( rtn >= 0)
		rtn = Cmd_Main( Cmd);

	return 1;
}

int TermProcess( int argc, char *argv[])
{
	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
int ParamPrint( PARAM *param)
{
	int		i;

	/*
	LogDbg( "argc                      = [%d]", param->argc);
	for( i = 0; i < param->argc; i++)
		LogMsg( "argv[%2d]                  = [%s]", i, Param.argv[ i]);
	for( i = 0; param->envp[ i] != NULL; i++)
		LogMsg( "envp[%2d]                  = [%s]", i, Param.envp[ i]);
	*/

	return 1;
}

/*********************************************************************************************************
*
*********************************************************************************************************/
#include <unistd.h>
#include <termios.h>

int getch()
{
	int				c;
	struct termios	oldattr, newattr;

	tcgetattr( STDIN_FILENO, &oldattr);
	memcpy( &newattr, &oldattr, sizeof( struct termios));
	newattr.c_lflag &= ~(ICANON | ECHO);
	newattr.c_cc[VMIN] = 1;
	newattr.c_cc[VTIME] = 0;
	tcsetattr( STDIN_FILENO, TCSANOW, &newattr);
	c = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr);

	return c;
}

int getch_test()
{
	unsigned int	c;
	struct termios	oldattr, newattr, *termios_p;

	tcgetattr( STDIN_FILENO, &oldattr);
	memcpy( &newattr, &oldattr, sizeof( struct termios));


	termios_p = &newattr;
	termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_p->c_cflag &= ~(CSIZE | PARENB);
	termios_p->c_cflag |= CS8;

	
	newattr.c_cc[VMIN] = 1;
	newattr.c_cc[VTIME] = 0;

	tcsetattr( STDIN_FILENO, TCSANOW, &newattr);
	c = getchar();
	printf( "[%d]\n", c);
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr);

	return c;
}

int get_winsize()
{
	struct winsize w;

	ioctl( STDOUT_FILENO, TIOCGWINSZ, &w);

	printf( "lines    = [%d]\n", w.ws_row);
	printf( "columns  = [%d]\n", w.ws_col);
	printf( "xpixel   = [%d]\n", w.ws_xpixel);
	printf( "ypixel   = [%d]\n", w.ws_ypixel);

	return 1;
}


