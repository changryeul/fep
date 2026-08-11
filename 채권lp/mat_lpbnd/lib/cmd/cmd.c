#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "etc.h"
#include "cfg.h"
#include "cmd.h"
#include "log.h"

CMD_INT_TBL	CmdInternalCommand[] = 
{
	{	100,	".set",			Cmd_IntSet,			"[command [value]]",		"command tool 관련 내부 변수 관리"},
	{	101,	".sleep",		Cmd_IntSleep,		"sec",						"인수(초) 만큼 sleep"	},
	{	102,	".until",		Cmd_IntUntil,		"[[[[[YYYY]MM]DD]hh]mm]ss",	"인수(시간)까지 sleep"	},
	{	198,	".help",		Cmd_IntHelp,		"[command]",				"도움말 보기"	},
	{	199,	".history",		Cmd_IntHistory,		"none",						"이전 명령어 조회"	},
	{	200,	".h",			Cmd_IntHistory,		"none",						"이전 명령어 조회"	},
	{	-1,		"\0",			NULL,				"\0",						"\0"				}
};

CMD*	Cmd_Open( CMD_TBL *tbl)
{
	CMD			*cmd;
	CMD_TBL		*cp;
	CMD_INT_TBL	*ip;

	cmd = ( CMD *)malloc( sizeof( CMD));
	if( cmd == NULL)
	{
		LogErr( "malloc error. size=[%d]", sizeof( CMD));
		goto error_1;
	}
	memset( cmd, 0, sizeof( CMD));

	cmd->history = Dll_Open( 1);
	if( cmd->history == NULL)
	{
		LogCri( "Dll_Open error.");
		goto error_2;
	}

	sprintf( cmd->prompt, "[cmd] ");

	cmd->cmd_tbl = Dll_Open( 0);
	if( cmd->cmd_tbl == NULL)
	{
		LogCri( "command table Dll_Open error.");
		goto error_4;
	}

	cmd->int_tbl = Dll_Open( 0);
	if( cmd->int_tbl == NULL)
	{
		LogCri( "internal command table Dll_Open error.");
		goto error_4;
	}

	cp = tbl;
	while( cp->f_no > 0)
	{
		LogDel( "cp         = [%p]", cp);
		LogDel( "cp->f_no   = [%d]", cp->f_no);
		LogDel( "cp->cmd    = [%s]", cp->cmd);
		LogDel( "cp->act    = [%p]", cp->act);
		LogDel( "cp->usage  = [%s]", cp->usage);
		LogDel( "addr=[%p] f_no=[%d] cmd=[%s] act=[%p] usage=[%s]", cp, cp->f_no, cp->cmd, cp->act, cp->usage);
		Dll_Add( cmd->cmd_tbl, cp, sizeof( CMD_TBL));
		cp++;
	}

	ip = CmdInternalCommand;
	while( ip->f_no > 0)
	{
		LogDel( "ip         = [%p]", ip);
		LogDel( "ip->f_no   = [%d]", ip->f_no);
		LogDel( "ip->cmd    = [%s]", ip->cmd);
		LogDel( "ip->act    = [%p]", ip->act);
		LogDel( "ip->usage  = [%s]", ip->usage);
		LogDel( "addr=[%p] f_no=[%d] cmd=[%s] act=[%p] usage=[%s]", ip, ip->f_no, ip->cmd, ip->act, ip->usage);
		Dll_Add( cmd->int_tbl, ip, sizeof( CMD_INT_TBL));
		ip++;
	}

	/* default setting */
	cmd->external = 0;
	cmd->internal = 1;

	return cmd;

#if 0
	error_x:
		Dll_Close( cmd->cmd_tbl);
#endif
	error_4:
	/* error_3: */
		Dll_Close( cmd->history);
	error_2:
		free( cmd);
	error_1:
		return NULL;
}

int Cmd_Close( CMD *cmd)
{
	if( cmd->int_tbl != NULL)	Dll_Close( cmd->int_tbl);
	if( cmd->cmd_tbl != NULL)	Dll_Close( cmd->cmd_tbl);
	if( cmd->history != NULL)	Dll_Close( cmd->history);
	if( cmd->log != NULL)	
	{
		Log_Usr( cmd->log, "Cmd log end pid=[%d]", getpid());
		Log_Close( cmd->log);
	}
	free( cmd);

	return 1;
}

int Cmd_Main( CMD *cmd)
{
	int			rtn;

	cmd->c_sz = Cmd_GetCommand( cmd, cmd->cmd);
	LogDel( "cmd->c_sz=[%d]", cmd->c_sz);
	if( cmd->c_sz < 0)
	{
		LogCri( "get command error.");
		return -1;
	}
	else 
	if( cmd->c_sz == 0) 
	{
		return 0;
	}

	if( cmd->argc <= 0) return 0;

	rtn = Cmd_ExecCommand( cmd);
	if( rtn < 0)
	{
		if( rtn == CMD_HELP)	
		{
			Cmd_HelpCommand( cmd, cmd->argv[ 0]);
			return 0;
		}
		return rtn;
	}
	return rtn;
}

void* Cmd_SetSignal( CMD *cmd, int sig_id)
{
#if 0
	if( sig_id <= 0)
	{
		cmd->old_sighandler = signal( SIGINT, Cmd_SigHandler);
		return cmd->old_sighandler;
	}

	return signal( sig_id, Cmd_SigHandler);
#endif
	return NULL;
}

int Cmd_SetPromptAction( CMD *cmd, int (*prompt_action)( CMD *cmd, char *prompt))
{
	int		rtn;

	cmd->prompt_action = prompt_action;
	rtn = cmd->prompt_action( cmd, cmd->prompt);

	return rtn;
}

int Cmd_SetPrompt( CMD *cmd, char *prompt)
{
	int		sz;

	sz = strlen( prompt);
	if( sz <= 0) return 0;

	memcpy( cmd->prompt, prompt, sz +1);

	return sz;
}

char *Cmd_GetPrompt( CMD *cmd)
{
	return cmd->prompt;
}

int Cmd_ExecCommand( CMD *cmd)
{
	int			rtn = 0;
	int			run_flag;
	CMD_TBL		*cp;
	CMD_INT_TBL	*ip;

	run_flag = 0;

	LogDel( "run command. argv[0]=[%s]", cmd->argv[ 0]);
	cp = ( CMD_TBL *)Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		if( cp->f_no < 0) break;
		LogDel( "addr=[%p] f_no=[%d] cmd=[%s] act=[%p] usage=[%s] argv[0]=[%s]", 
				cp, cp->f_no, cp->cmd, cp->act, cp->usage, cmd->argv[ 0]);
		if( memcmp( cp->cmd, cmd->argv[ 0], strlen( cp->cmd) +1) == 0)
		{
			rtn = cp->act( cmd->argc, cmd->argv);
			if( rtn < 0)
			{
				/*
				LogDel( "function=[%p] return [%d].", cp->act, rtn);
				LogCri( "command error. cmd=[%s] rtn=[%d]", cmd->argv[ 0], rtn);
				*/
				switch( rtn)
				{
					case CMD_HELP:
						Cmd_IntHelp( cmd, cmd->argc, cmd->argv);
						return 0;
					case CMD_USAGE:
						Cmd_HelpCommand( cmd, cmd->argv[ 0]);
						return 0;
					default:
					case CMD_EXIT:
						return rtn;
				}
			}
			run_flag = 1;
			break;
		}
		cp = ( CMD_TBL *)Dll_GetNextPtr( cmd->cmd_tbl);
	}

	if( cmd->internal && run_flag == 0)
	{
		ip = ( CMD_INT_TBL *)Dll_GetFirstPtr( cmd->int_tbl);
		while( ip != NULL)
		{
			if( ip->f_no < 0) break;
			LogDel( "addr=[%p] f_no=[%d] cmd=[%s] act=[%p] usage=[%s] argv[0]=[%s]", 
					ip, ip->f_no, ip->cmd, ip->act, ip->usage, cmd->argv[ 0]);
			if( memcmp( ip->cmd, cmd->argv[ 0], strlen( ip->cmd) +1) == 0)
			{
				rtn = ip->act( cmd, cmd->argc, cmd->argv);
				if( rtn < 0)
				{
					return rtn;
					/*
					LogDel( "function=[%p] return [%d].", ip->act, rtn);
					LogCri( "command error. cmd=[%s] rtn=[%d]", cmd->argv[ 0], rtn);
					*/
				}
				run_flag = 2;
				break;
			}
			ip = ( CMD_INT_TBL *)Dll_GetNextPtr( cmd->int_tbl);
		}
	}

	if( cmd->external && run_flag == 0)
	{
		rtn = Cmd_RunExtCommand( cmd);
		if( rtn < 0)
		{
			LogDel( "external command rtn=[%d]", rtn);
			return -1;
		}
		run_flag = 3;
	}

	if( run_flag == 0)
	{
		printf( "command not found\n");
	}

	if( cmd->prompt_action != NULL)
	{
		cmd->prompt_action( cmd, cmd->prompt);
	}

	return rtn;
}

void Cmd_SigHandler( int sig_no)
{
	printf( "signal %d received.\n", sig_no);
	signal( sig_no, Cmd_SigHandler);
	return;
}

int Cmd_CommandLog( void *ptr, char *msg, int sz)
{
	CMD		*cmd = ( CMD *)ptr;

	Log_Out( cmd->log, msg, sz);

	return 1;
}

int Cmd_SetLog( CMD *cmd, char *log_file_name)
{
	time_t	cur_time;

	if( cmd->log != NULL)
	{
		Log_Close( cmd->log);
	}

	cmd->log = Log_Open( log_file_name);
	if( cmd->log == NULL)
	{
		LogCri( "Log_Open( log_file_name=[%s]) error.", log_file_name);
		return -1;
	}

	Log_Func( cmd->log, cmd, Cmd_CommandLog);

	time( &cur_time);
	Log_Usr( cmd->log, "Cmd log start. pid=[%d] at=[%s]", getpid(), TtoS( cur_time));

	return 1;
}

int Cmd_RunCommand( CMD *cmd, char *command)
{
	int		rtn, sz;

	if( cmd->argv != NULL)
	{
		free( cmd->argv[ 0]);
		free( cmd->argv);
		cmd->argv = NULL;
	}
	cmd->argc = 0;

	sz = strlen( command);
	if( sz <= 1) return sz;

	if( command[ 0] == '\\') 
	{
		rtn = Cmd_GetInternal( cmd);
		return rtn;
	}

	sz = Cmd_PutCommand( cmd, command);

	Cmd_GetArgument( cmd, command);

	rtn = Cmd_ExecCommand( cmd);
	if( rtn < 0)
	{
		LogCri( "Cmd_ExecCommand error. rtn=[%d]", rtn);
		return rtn;
	}

	return sz;
}

int Cmd_GetCommand( CMD *cmd, char *command)
{
	int		rtn, sz;
	char	*p;

	if( cmd->argv != NULL)
	{
		/* free( cmd->argv[ 0]); 20140112 - space + command 면 SIGABRT 발생해서 막음 */
		free( cmd->argv);
		cmd->argv = NULL;
	}
	cmd->argc = 0;

	printf( "%s", cmd->prompt);
	fflush( stdout);

	p = fgets( command, 8192, stdin);
	if( p == NULL) return -1;

	sz = strlen( command);
	if( sz <= 1) return sz;

	if( cmd->log != NULL)	Log_Usr( cmd->log, "command=[%6d:%-.*s]", getpid(), sz -1, command);

	if( command[ 0] == '\\') 
	{
		rtn = Cmd_GetInternal( cmd);
		return rtn;
	}

	sz = Cmd_PutCommand( cmd, command);

	Cmd_GetArgument( cmd, command);

	return sz;
}

int Cmd_PutCommand( CMD *cmd, char *command)
{
	int		sz;
	char	*p;

	sz = strlen( command);
	p = ( char *)Dll_Add( cmd->history, command, sz);
	if( p == NULL)
	{
		LogMsg( "history add error. cmd=[%d:%s]", sz, command);
	}

	return sz;
}

int Cmd_GetArgument( CMD *cmd, char *command)
{
	int		start = 0;
	char	*cp;

	LogDel( "Cmd_GetArgument. command=[%s]", command);

	cp = ( char *)malloc( strlen( command) +1);
	if( cp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", strlen( command));
		return -1;
	}
	memcpy( cp, command, strlen( command) +1);

	cmd->argv = ( char **)malloc( sizeof( char *) + sizeof( char *));
	if( cmd->argv == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( char *));
		return -1;
	}
	cmd->argc = 0;

	while( *cp != 0)
	{
		switch( *cp)
		{
			case 0x00:
				cmd->argv[ cmd->argc] = NULL;
				return cmd->argc;
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
			case 0x09:  /* horizontal tab */
			case 0x20:  /* space */
				*cp = 0;
				start = 0;
				break;
			case '\"':
			case '\'':
				if( start == 0)	
				{	
					cp = Cmd_GetArgumentSub( cmd, cp);
					if( cp == NULL) return -1;
				}
				break;
			default  :
				if( start == 0)
				{
					cmd->argv = ( char **)realloc( cmd->argv, sizeof( char *) * ( cmd->argc +2));
					if( cmd->argv == NULL)
					{
						LogErr( "realloc error. ptr=[%p] sz=[%d]", cmd->argv, sizeof( char *) * ( cmd->argc +2));
						return -1;
					}
					cmd->argv[ cmd->argc] = cp;
					LogDel( "Cmd_GetArgument cmd->argc=[%d] cmd->argv[%d]=[%p:%s]", 
							cmd->argc, cmd->argc, cmd->argv[ cmd->argc], cmd->argv[ cmd->argc]);
					cmd->argc++;
				}
				start = 1;
				break;
		}
		cp++;
	}

	cmd->argv[ cmd->argc] = NULL;
	return cmd->argc;
}

char *Cmd_GetArgumentSub( CMD *cmd, char *cp)
{
	char	c;

	c = *cp;
	*cp = 0;
	cp++;

	cmd->argv = ( char **)realloc( cmd->argv, sizeof( char *) * ( cmd->argc +2));
	if( cmd->argv == NULL)
	{
		LogErr( "realloc error. ptr=[%p] sz=[%d]", cmd->argv, sizeof( char *) * ( cmd->argc +2));
		return NULL;
	}
	cmd->argv[ cmd->argc] = cp;
	LogDel( "Cmd_GetArgument cmd->argc=[%d] cmd->argv[%d]=[%p:%s]", 
			cmd->argc, cmd->argc, cmd->argv[ cmd->argc], cmd->argv[ cmd->argc]);
	cmd->argc++;

	while( *cp != 0)
	{
		LogDel( "[%c] == [%c]\n", *cp, c);
		if( *cp == c) 
		{
			*cp = 0;
			return cp;
		}
		cp++;
	}

	return NULL;
}

int Cmd_RunExtCommand( CMD *cmd)
{
	int		i, pos = 0;
	FILE	*fp;
	char	*p, run_cmd[ 8192], line[ 8192];

	for( i = 0; cmd->argv[ i] != NULL; i++)
		pos += sprintf( &run_cmd[ pos], "%s ", cmd->argv[ i]);

	if( cmd->external == 2)
	{
		printf( "external command \"%s\" excute? (Yes/No):", run_cmd);
		p = fgets( line, 8192, stdin);
		if( p == NULL) return 0;
		if( toupper( line[ 0]) != 'Y') return 0;
	}

	fp = popen( run_cmd, "r");
	if( fp == NULL)
	{
		LogErr( "popen error. cmd=[%s]", run_cmd);
		return -1;
	}

	while( 1)
	{
		p = fgets( line, 8192, fp);
		if( p == NULL) break;
		printf( "%s", line);
	}

	pclose( fp);

	return 1;
}

int Cmd_GetInternal( CMD *cmd)
{
	int				rtn, pos, sz;
	char			*p;

	p = cmd->cmd;
	rtn = Cmd_GetArgument( cmd, p);

	if( memcmp( cmd->argv[ 0], "\\\0", 2) == 0)
	{
		rtn = Cmd_GetPrevCommand( cmd, 1);
		if( rtn <= 0)
		{
			printf( "Command not found.\n");
			return 0;
		}
		sz = strlen( cmd->cmd);
		printf( "[%-.*s]\n", sz -1, cmd->cmd);
		if( cmd->log != NULL) Log_Usr( cmd->log, "history=[%6d:%-.*s]", getpid(), sz -1, cmd->cmd);
		Cmd_PutCommand( cmd, cmd->cmd);
		cmd->c_sz = Cmd_GetArgument( cmd, cmd->cmd);
		rtn = Cmd_ExecCommand( cmd);
		/*
		rtn = Cmd_EditCommand( cmd);
		*/
	}
	else
	if( memcmp( cmd->argv[ 0], "\\", 1) == 0)
	{
		pos = atoi( &cmd->argv[ 0][1]);
		if( pos > 0)
		{
			LogDel( "history command... pos=[%d]", pos);
			rtn = Cmd_GetFirstCommand( cmd, pos);
			rtn = Cmd_EditCommand( cmd);
			sz = strlen( cmd->cmd);
			printf( "[%-.*s]\n", sz -1, cmd->cmd);
			if( cmd->log != NULL) Log_Usr( cmd->log, "history=[%6d:%-.*s]", getpid(), sz -1, cmd->cmd);
			Cmd_PutCommand( cmd, cmd->cmd);
			cmd->c_sz = Cmd_GetArgument( cmd, cmd->cmd);
			rtn = Cmd_ExecCommand( cmd);
		}
		else
		{
			rtn = Cmd_GetPrevHistCommand( cmd, &cmd->argv[ 0][ 1]);
			if( rtn <= 0)
			{
				printf( "Command not found.\n");
				return 0;
			}
			rtn = Cmd_EditCommand( cmd);
			sz = strlen( cmd->cmd);
			printf( "[%-.*s]\n", sz -1, cmd->cmd);
			if( cmd->log != NULL) Log_Usr( cmd->log, "history=[%6d:%-.*s]", getpid(), sz -1, cmd->cmd);
			Cmd_PutCommand( cmd, cmd->cmd);
			cmd->c_sz = Cmd_GetArgument( cmd, cmd->cmd);
			rtn = Cmd_ExecCommand( cmd);
		}
	}
	else
	{
		LogDel( "unknown...");
	}

	cmd->argc = 0;
	return rtn;
}

int Cmd_IntSet( CMD *cmd, int argc, char *argv[])
{
	if( argc < 2)
	{
		printf( "prompt           = %-20.20s프롬프트\n", cmd->prompt);
		printf( "internal         = %d                 내부명령어 - 0: 실행 안함, 1: 실행\n", cmd->internal);
		printf( "external         = %d                 외부명령어 - 0: 실행 안함, 1: 실행,  2: 실행 여부 문의\n", cmd->external);
		printf( "timeout          = %d                 명령어 대기 타임아웃\n", cmd->timeout);
		printf( "max_hist         = %-4d              명령어 저장 갯수\n", cmd->max_hist);
		return 0;
	}

	if( !memcmp( argv[1], "prompt", 7))					Cmd_SetPrompt( cmd, argv[ 2]); 
	else if( !memcmp( argv[1], "internal", 9))			cmd->internal = atoi( argv[ 2]);
	else if( !memcmp( argv[1], "external", 9))			cmd->external = atoi( argv[ 2]);
	else if( !memcmp( argv[1], "timeout", 8))			cmd->timeout = atoi( argv[ 2]);
	else if( !memcmp( argv[1], "max_hist", 9))			cmd->max_hist = atoi( argv[ 2]);

	printf( "prompt           = %-20.20s프롬프트\n", cmd->prompt);
	printf( "internal         = %d                 내부명령어 - 0: 실행 안함, 1: 실행\n", cmd->internal);
	printf( "external         = %d                 외부명령어 - 0: 실행 안함, 1: 실행,  2: 실행 여부 문의\n", cmd->external);
	printf( "timeout          = %d                 명령어 대기 타임아웃\n", cmd->timeout);
	printf( "max_hist         = %-4d              명령어 저장 갯수\n", cmd->max_hist);

	return 1;
}

int Cmd_IntSleep( CMD *cmd, int argc, char *argv[])
{
	int			sleep_time;

	if( argc < 2)
	{
		return 0;
	}

	sleep_time = atoi( argv[ 1]);

	sleep( sleep_time);

	return 1;
}

int Cmd_IntUntil( CMD *cmd, int argc, char *argv[])
{
	time_t		cur_time, until_time;
	struct tm	tm_buf;
	char		*ptr;
	int			sz;

	if( argc < 2)
	{
		return 0;
	}

	time( &cur_time);
	localtime_r( &cur_time, &tm_buf);

	ptr = argv[ 1];
	sz = strlen( argv[ 1]);



	switch( sz)
	{
		case 14:
			tm_buf.tm_year = Cmd_AtoI( ptr, 4) - 1900;
			ptr += 2;
		case 10:
			tm_buf.tm_mon = Cmd_AtoI( ptr, 2) -1;
			ptr += 2;
		case 8:
			tm_buf.tm_mday = Cmd_AtoI( ptr, 2);
			ptr += 2;
		case 6:
			tm_buf.tm_hour = Cmd_AtoI( ptr, 2);
			ptr += 2;
		case 4:
			tm_buf.tm_min = Cmd_AtoI( ptr, 2);
			ptr += 2;
		case 2:
			tm_buf.tm_sec = Cmd_AtoI( ptr, 2);
			break;
	}

	until_time = mktime( &tm_buf);
	time( &cur_time);
	if( until_time - cur_time <= 0) return 0;

	printf( "sleep %ld second(s).\n", until_time - cur_time);
	sleep( until_time - cur_time);

	return 1;
}

int Cmd_IntHelp( CMD *cmd, int argc, char *argv[])
{
	char			*ptr, rec[ 512];
	int				cmd_len = 10;
	int				len, i;
	CMD_TBL			*cp;
	CMD_INT_TBL		*ip;

	LogDel( "not implement");
	printf( "------------------------------------------------------------------------------------------------------\n");
	printf( "명령어     인수(argument)\n");
	printf( "           명령어 설명\n");
	printf( "------------------------------------------------------------------------------------------------------\n");

	cp = ( CMD_TBL *)Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		len = printf( "%s", cp->cmd);

		memcpy( rec, cp->usage, strlen( cp->usage) +1);

		ptr = strtok( rec, "\n");
		while( ptr != NULL)
		{
			for( i = len; i < cmd_len; i++) printf( " ");
			len = 0;
			printf( "%s\n", ptr);
			ptr = strtok( NULL, "\n");
			if( ptr == NULL) break;
		}

		memcpy( rec, cp->comment, strlen( cp->comment) +1);
		ptr = strtok( rec, "\n");
		while( ptr != NULL)
		{
			for( i = 0; i < cmd_len; i++) printf( " ");
			printf( "%s\n", ptr);
			ptr = strtok( NULL, "\n");
			if( ptr == NULL) break;
		}

		cp = ( CMD_TBL *)Dll_GetNextPtr( cmd->cmd_tbl);
	}
	printf( "--[ internal command ]--------------------------------------------------------------------------------\n");

	ip = &CmdInternalCommand[ 0];
	while( ip->f_no > 0)
	{
		printf( "%-10s ", ip->cmd);
		printf( "%-30s", ip->usage);
		printf( "%s", ip->comment);
		printf( "\n");
		ip++;
	}
	printf( "------------------------------------------------------------------------------------------------------\n");
	printf( " \\              - repeat previous command\n");
	printf( " \\ + number     - history command 실행\n"); 
	printf( " \\ + string     - matching command 실행\n");
	printf( "------------------------------------------------------------------------------------------------------\n");

	return 0;
}

int Cmd_IntHelpOld( CMD *cmd, int argc, char *argv[])
{
	CMD_TBL			*cp;
	CMD_INT_TBL		*ip;

	LogDel( "not implement");
	printf( "fno command    usage                        comment\n");
	printf( "------------------------------------------------------------------------------------------------------\n");

	cp = ( CMD_TBL *)Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		printf( "%3d ", cp->f_no);
		printf( "%-10s ", cp->cmd);
		printf( "%-30s", cp->usage);
		printf( "%s", cp->comment);
		printf( "\n");
		cp = ( CMD_TBL *)Dll_GetNextPtr( cmd->cmd_tbl);
	}
	printf( "--[ internal command ]--------------------------------------------------------------------------------\n");

	ip = &CmdInternalCommand[ 0];
	while( ip->f_no > 0)
	{
		printf( "%3d ", ip->f_no);
		printf( "%-10s ", ip->cmd);
		printf( "%-30s", ip->usage);
		printf( "%s", ip->comment);
		printf( "\n");
		ip++;
	}
	printf( "------------------------------------------------------------------------------------------------------\n");
	printf( " \\              - repeat previous command\n");
	printf( " \\ + number     - history command edit\n"); 
	printf( " \\ + string     - matching command edit\n");
	printf( " \\\\             - previous command edit\n");
	printf( " \\\\ + command   - intenal command\n");
	printf( "------------------------------------------------------------------------------------------------------\n");

	return 0;
}

int Cmd_IntHistory( CMD *cmd, int argc, char *argv[])
{
	int		cnt = 1;
	char	*command;


	command = ( char *)Dll_GetFirstPtr( cmd->history);
	while( command != NULL)
	{
		printf( "    %3d %s", cnt++, command);
		command = ( char *)Dll_GetNextPtr( cmd->history);
	}

	return 1;
}

int Cmd_GetPrevCommand( CMD *cmd, int pos)
{
	int		sz;
	char	*cp;

	cp = ( char *)Dll_GetLastPtr( cmd->history);
	if( cp == NULL) return 0;

	sz = sprintf( cmd->cmd, "%s", cp);
	LogDel( "Cmd_GetNextCommand cmd->cmd=[%s]", cmd->cmd);

	return sz;
}

int Cmd_GetPrevHistCommand( CMD *cmd, char *str)
{
	int		sz;
	char	*cp;

	sz = strlen( str);
	if( sz <= 0) return 0;

	cp = ( char *)Dll_GetLastPtr( cmd->history);
	while( cp != NULL)
	{
		if( !memcmp( cp, str, sz)) break;
		cp = ( char *)Dll_GetPrevPtr( cmd->history);
	}
	if( cp == NULL) return 0;

	sz = sprintf( cmd->cmd, "%s", cp);
	LogDel( "Cmd_GetNextCommand cmd->cmd=[%s]", cmd->cmd);

	return sz;
}

int Cmd_GetFirstCommand( CMD *cmd, int pos)
{
	int		sz, no = 0;
	char	*cp;

	cp = ( char *)Dll_GetFirstPtr( cmd->history);
	while( cp != NULL)
	{
		no++;
		if( no == pos) break;
		cp = ( char *)Dll_GetNextPtr( cmd->history);
	}

	sz = sprintf( cmd->cmd, "%s", cp);
	LogDel( "Cmd_GetNextCommand cmd->cmd=[%s]", cmd->cmd);

	return sz;
}

int Cmd_EditCommand( CMD *cmd)
{
	int		i, pos;
	int		sz, buf_sz, u_flag, e_flag;
	char	buf[ 8192] , *p;


	return 1;

	while( 1)
	{
		sz = strlen( cmd->cmd);
		if( cmd->cmd[ sz -1] == '\n') sz -= 1;
		cmd->cmd[ sz] = 0;
		LogDel( "sz=[%d:%s]", sz, cmd->cmd);

		printf( "     %s\n", cmd->cmd);
		printf( "edit:");

		p = fgets( buf, 8192, stdin);
		if( p == NULL) return 1;
		buf_sz = strlen( buf);
		if( buf_sz <= 0) return 1;
		buf[ buf_sz] = 0;
		LogDel( "buf_sz=[%d]", buf_sz);

		if( buf_sz >= sz)
		{
			memset( &cmd->cmd[ sz], 0x20, buf_sz - sz);
		}

		pos = 0;
		u_flag = 0;
		for( i = 0; i < buf_sz; i++)
		{
			LogDel( "pos=[%d:%c] i=[%d:%c]", pos, cmd->cmd[ pos], i, buf[ i]);
			switch( buf[ i])
			{
				case ' ':
					if( u_flag == 0) 
					{
						if( i >= sz) cmd->cmd[ pos] = buf[ i];
						pos++;
						continue;
					}
					cmd->cmd[ pos++] = buf[ i];
					break;
				case '\r':
				case '\n':
				case '\0':
					e_flag = 1;
					break;
				default :
					u_flag = 1;
					cmd->cmd[ pos++] = buf[ i];
					break;
			}
			if( e_flag) break;
		}
		cmd->cmd[ pos] = 0;
		LogDel( "pos=[%d:%s] i=[%d:%s]", pos, cmd->cmd, i, buf);
	}


	return 1;
}

int Cmd_Display( CMD *cmd)
{
	int		i;
	CMD_TBL	*cp;
	char	*hp;

	LogDel( "cmd address      = [%p]", cmd);
	LogDel( "cmd              = [%s]", cmd->cmd);
	LogDel( "argc             = [%d]", cmd->argc);
	for( i = 0; i < cmd->argc; i++)
	{
		LogDel( "    argv[ %d]     = [%s]", i, cmd->argv[ i]);
	}
	LogDel( "prompt           = [%s]", cmd->prompt);
	LogDel( "c_sz             = [%d]", cmd->c_sz);
	LogDel( "external         = [%d]", cmd->external);
	LogDel( "timeout          = [%d]", cmd->timeout);

	LogDel( "command table [%d]", cmd->cmd_tbl->recs);
	cp = ( CMD_TBL *)Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		LogDel( "    %3d %-10s %p", cp->f_no, cp->cmd, cp->act);
		cp = ( CMD_TBL *)Dll_GetNextPtr( cmd->cmd_tbl);
	}

	LogDel( "history table [%d]", cmd->history->recs);
	hp = ( char *)Dll_GetFirstPtr( cmd->history);
	while( hp != NULL)
	{
		LogDel( "    %s", hp);
		hp = ( char *)Dll_GetNextPtr( cmd->history);
	}

	return 0;
}

int Cmd_HelpCommand( CMD *cmd, char *command)
{
	int			sz, cmp_sz;
	CMD_TBL		*cp;

	if( command == NULL)	
	{
		if( cmd->argv == NULL) return 0;
		command = cmd->argv[ 0];
	}

	sz = strlen( command);

	cp = ( CMD_TBL *)Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		cmp_sz = _Max( sz, strlen( cp->cmd));
		if( !memcmp( command, cp->cmd, cmp_sz)) break;
		cp = ( CMD_TBL *)Dll_GetNextPtr( cmd->cmd_tbl);
	}
	
	if( cp == NULL) return 0;

	printf( "USAGE: %s %s\n%-10s%s\n", cp->cmd, cp->usage, "",  Cmd_ConvertComment( cmd, cp->comment));
	return 1;
}

char *Cmd_ConvertComment( CMD *cmd, const char *comment)
{
	static char	cmt[ 8192];
	int			pos = 0;
	int			i, col = 10;

	while( *comment)
	{
		switch( *comment)
		{
			case '\n':
				cmt[ pos++] = *comment;
				for( i = 0; i < col; i++) cmt[ pos++] = ' ';
				break;
			default:
				cmt[ pos++] = *comment;
				break;
		}
		comment++;
	}

	return cmt;
}

int Cmd_AtoI( char *str, int sz)
{
	char	rec[ 512];

	memcpy( rec, str, sz);
	rec[ sz] = 0;

	return atoi( rec);
}

int Cmd_Set( CMD *cmd, int opt)
{
	switch( opt)
	{
		case CMD_EXTENAL_COMMAND_ON :
			cmd->external = 1;
			break;
		case CMD_EXTENAL_COMMAND_OFF :
			cmd->external = 0;
			break;
		default:
			break;
	}

	return 0;
}




