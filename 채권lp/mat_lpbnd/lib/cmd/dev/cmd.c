#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "cfg.h"
#include "cmd.h"

CMD_INT_TBL	CmdInternalCommand[] = 
{
	{	100,	".help",		Cmd_IntHelp,		"[command]",		"this help message"	},
	{	101,	".history",		Cmd_IntHistory,		"none",				"display history"	},
	{	102,	".quit",		Cmd_IntQuit,		"none",				"quit program"		},
	{	103,	".exit",		Cmd_IntQuit,		"none",				"exit program"		},
	{	-1,		"\0",			NULL,				"\0",				"\0"				}
};

CMD*	Cmd_Open( CMD_TBL *tbl)
{
	CMD			*cmd;
	CMD_TBL		*cp;
	CMD_INT_TBL	*icp;

	cmd = malloc( sizeof( CMD));
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

	cmd->prompt = malloc( 7);
	if( cmd->prompt == NULL)
	{
		LogErr( "malloc error. size=[%d]", sizeof( CMD));
		goto error_1;
	}
	sprintf( cmd->prompt, "[cmd] ");

	/* get command table */
	cmd->cmd_tbl = Dll_Open( 0);
	if( cmd->cmd_tbl == NULL)
	{
		LogCri( "command table Dll_Open error.");
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
		LogDel( "cp_addr=[%p] cp->f_no=[%d] cp->cmd=[%s] cp->act=[%p] cp->usage=[%s]", 
				cp, cp->f_no, cp->cmd, cp->act, cp->usage);
		Dll_Add( cmd->cmd_tbl, cp, sizeof( CMD_TBL));
		cp++;
	}

	/* get internal command table */
	cmd->cmd_int_tbl = Dll_Open( 0);
	if( cmd->cmd_int_tbl == NULL)
	{
		LogCri( "command internal table Dll_Open error.");
		goto error_4;
	}
	icp = &CmdInternalCommand[ 0];
	while( icp->f_no > 0)
	{
		LogDel( "icp         = [%p]", icp);
		LogDel( "icp->f_no   = [%d]", icp->f_no);
		LogDel( "icp->cmd    = [%s]", icp->cmd);
		LogDel( "icp->act    = [%p]", icp->act);
		LogDel( "icp->usage  = [%s]", icp->usage);
		LogDel( "icp_addr=[%p] icp->f_no=[%d] icp->cmd=[%s] icp->act=[%p] icp->usage=[%s]", 
				icp, icp->f_no, icp->cmd, icp->act, icp->usage);
		Dll_Add( cmd->cmd_int_tbl, icp, sizeof( CMD_INT_TBL));
		icp++;
	}

	cmd->external = 0;

	return cmd;

	error_x:
		Dll_Close( cmd->cmd_tbl);
	error_4:
		free( cmd->prompt);
	error_3:
		Dll_Close( cmd->history);
	error_2:
		free( cmd);
	error_1:
		return NULL;
}

Cmd_Close( CMD *cmd)
{
	if( cmd->prompt != NULL)	free( cmd->prompt);
	if( cmd->history != NULL)	Dll_Close( cmd->history);
	free( cmd);

	return 1;
}

Cmd_Main( CMD *cmd)
{
	int			i, rtn, run_flag;
	CMD_TBL		*cp;

	/*
	signal( SIGINT, Cmd_SigHandler);
	*/

	while( 1)
	{
		run_flag = 0;

		cmd->c_sz = Cmd_GetCommand( cmd, cmd->cmd);
		LogDel( "cmd->c_sz=[%d]", cmd->c_sz);
		if( cmd->c_sz < 0)
		{
			LogCri( "get command error.");
			return -1;
		}
		else if( cmd->c_sz == 0) 
		{
			return 0;
		}
		if( cmd->argc <= 0) return 0;

		LogDel( "run command. cmd=[%s]", cmd->cmd);
		rtn = Cmd_ExecCommand( cmd);
		if( rtn < 0)
		{
			LogCri( "Cmd_ExecCommand error. rtn=[%d]", rtn);
			return rtn;
		}
		LogDel( "run command OK. rtn=[%d]", rtn);
	}
}

Cmd_SetPrompt( CMD *cmd, char *prompt)
{
	int		rtn;
	int		sz;

	if( prompt == NULL) return 0;

	sz = strlen( prompt);
	if( sz <= 0) return 0;

	if( cmd->prompt != NULL) free( cmd->prompt);
	cmd->prompt = malloc( sz +1);
	if( cmd->prompt == NULL)
	{
		LogErr( "malloc error. cmd->prompt sz=[%d]", sz +1);
		return -1;
	}
	memcpy( cmd->prompt, prompt, sz +1);

	return sz;
}

Cmd_ExecCommand( CMD *cmd)
{
	int				rtn;
	CMD_TBL			*cp;
	CMD_INT_TBL		*icp;

	LogDel( "run command. argv[0]=[%s]", cmd->argv[ 0]);
	cp = Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		if( cp->f_no < 0) break;
		LogDel( "cp_addr=[%p] cp->f_no=[%d] cp->cmd=[%s] cp->act=[%p] cp->usage=[%s] argv[0]=[%s]", 
				cp, cp->f_no, cp->cmd, cp->act, cp->usage, cmd->argv[ 0]);
		if( memcmp( cp->cmd, cmd->argv[ 0], strlen( cmd->argv[ 0])) == 0)
		{
			rtn = cp->act( cmd->argc, cmd->argv);
			if( rtn < 0)
			{
				LogDel( "function=[%p] return [%d].", cp->act, rtn);
				LogCri( "command error. cmd=[%s] rtn=[%d]", cmd->argv[ 0], rtn);
			}
			return rtn;
		}
		cp = Dll_GetNextPtr( cmd->cmd_tbl);
	}

	LogDel( "run internal command. argv[0]=[%s]", cmd->argv[ 0]);
	icp = Dll_GetFirstPtr( cmd->cmd_int_tbl);
	while( icp != NULL)
	{
		if( icp->f_no < 0) break;
		LogDel( "icp_addr=[%p] icp->f_no=[%d] icp->cmd=[%s] icp->act=[%p] icp->usage=[%s] argv[0]=[%s]", 
				icp, icp->f_no, icp->cmd, icp->act, icp->usage, cmd->argv[ 0]);
		if( memcmp( icp->cmd, cmd->argv[ 0], strlen( cmd->argv[ 0])) == 0)
		{
			rtn = icp->act( cmd, cmd->argc, cmd->argv);
			if( rtn < 0)
			{
				LogDel( "function=[%p] return [%d].", cp->act, rtn);
				LogCri( "command error. cmd=[%s] rtn=[%d]", cmd->argv[ 0], rtn);
			}
			return rtn;
		}
		cp = Dll_GetNextPtr( cmd->cmd_tbl);
	}

	rtn = Cmd_RunExtCommand( cmd);
	if( rtn < 0)
	{
		LogDel( "external command rtn=[%d]", rtn);
	}
	return rtn;
}

void Cmd_SigHandler( int sig_no)
{
	printf( "signal %d received.\n", sig_no);
	return;
}

Cmd_GetCommand( CMD *cmd, char *command)
{
	int		rtn, sz;
	char	*p;

	if( cmd->argv != NULL)
	{
		free( cmd->argv[ 0]);
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

	if( command[ 0] == '\\') 
	{
		rtn = Cmd_GetInternal( cmd);
		return rtn;
	}

	sz = Cmd_PutCommand( cmd, command);

	Cmd_GetArgument( cmd, command);

	return sz;
}

Cmd_PutCommand( CMD *cmd, char *command)
{
	int		sz;
	char	*p;

	sz = strlen( command);
	p = Dll_Add( cmd->history, command, sz);
	if( p == NULL)
	{
		LogMsg( "history add error. cmd=[%d:%s]", sz, command);
	}

	return sz;
}

Cmd_GetArgument( CMD *cmd, char *command)
{
	int		c_pos = 0;
	int		start = 0;
	char	*cp;

	LogDel( "Cmd_GetArgument. command=[%s]", command);

	cp = malloc( strlen( command) +1);
	if( cp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", strlen( command));
		return -1;
	}
	memcpy( cp, command, strlen( command) +1);

	cmd->argv = malloc( sizeof( char *) + sizeof( char *));
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
			default  :
				if( start == 0)
				{
					cmd->argv = realloc( cmd->argv, sizeof( char *) * ( cmd->argc +2));
					if( cmd->argv == NULL)
					{
						LogErr( "malloc error. sz=[%d]", sizeof( char *) * ( cmd->argc +2));
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

Cmd_RunExtCommand( CMD *cmd)
{
	int		i, rtn, pos = 0;
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

	fclose( fp);

	return 1;
}

Cmd_GetInternal( CMD *cmd)
{
	int				rtn, pos;
	char			*p, *ap;
	CMD_INT_TBL		*ip;

	p = cmd->cmd;
	rtn = Cmd_GetArgument( cmd, p);

	if( memcmp( cmd->argv[ 0], "\\\\", 2) == 0)
	{
		LogDel( "Editing internal command...");
		rtn = Cmd_EditCommand( cmd);
	}
	else
	if( memcmp( cmd->argv[ 0], "\\", 1) == 0)
	{
		LogDel( "Run internal command...");
		rtn = Cmd_RunCommand( cmd);
	}

	return 1;




#if 0
	else if( memcmp( cmd->argv[ 0], "\\\\", 2) == 0)
	{
		ap = &cmd->argv[0][2];

		ip = &CmdInternalCommand[ 0];
		while( ip->f_no > 0)
		{
			if( memcmp( ip->cmd, ap, strlen( ap /* ip->cmd */)) == 0)
			{
				rtn = ip->act( cmd, cmd->argc, cmd->argv);
				break;
			}
			ip++;
		}
	}
	else
	if( memcmp( cmd->argv[ 0], "\\\0", 2) == 0)
	{
		rtn = Cmd_GetPrevCommand( cmd, 1);
		if( rtn <= 0)
		{
			printf( "Command not found.\n");
			return 0;
		}
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
			rtn = Cmd_GetPrevCommand( cmd, pos);
			rtn = Cmd_EditCommand( cmd);
			Cmd_PutCommand( cmd, cmd->cmd);
			cmd->c_sz = Cmd_GetArgument( cmd, cmd->cmd);
			rtn = Cmd_ExecCommand( cmd);
		}
	}
	else
	{
		LogDel( "unknown...");
	}
#endif

	cmd->argc = 0;
	return 0;
}

Cmd_EditCommand( CMD *cmd)
{
	char	buf[ 8192];

	printf( "     %s", cmd->cmd);
	printf( "edit:", cmd->cmd);
	fgets( buf, 8192, stdin);
	return 1;
}

Cmd_RunCommand( CMD *cmd)
{
	return 1;
}



Cmd_GetPrevCommand( CMD *cmd, int pos)
{
	int		sz, no = 0;
	char	*cp;

	cp = Dll_GetLastPtr( cmd->history);
	while( cp != NULL)
	{
		no++;
		if( no == pos) break;
		cp = Dll_GetPrevPtr( cmd->history);
	}

	sz = sprintf( cmd->cmd, "%s", cp);
	LogDel( "Cmd_GetPrevCommand cmd->cmd=[%s]", cmd->cmd);

	return sz;
}

Cmd_Display( CMD *cmd)
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
	cp = Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		LogDel( "    %3d %-10s %p", cp->f_no, cp->cmd, cp->act);
		cp = Dll_GetNextPtr( cmd->cmd_tbl);
	}

	LogDel( "history table [%d]", cmd->history->recs);
	hp = Dll_GetFirstPtr( cmd->history);
	while( hp != NULL)
	{
		LogDel( "    %s", hp);
		hp = Dll_GetNextPtr( cmd->history);
	}
}

/***************************************************************************************************
*
***************************************************************************************************/
Cmd_IntHelp( CMD *cmd, int argc, char *argv[])
{
	CMD_TBL			*cp;
	CMD_INT_TBL		*ip;

	LogDel( "not implement");
	printf( "fno command    usage                        comment\n");
	printf( "------------------------------------------------------------------------------------------------------\n");

	cp = Dll_GetFirstPtr( cmd->cmd_tbl);
	while( cp != NULL)
	{
		printf( "%3d ", cp->f_no);
		printf( "%-10s ", cp->cmd);
		printf( "%-30s", cp->usage);
		printf( "%s", cp->comment);
		printf( "\n");
		cp = Dll_GetNextPtr( cmd->cmd_tbl);
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

Cmd_IntHistory( CMD *cmd, int argc, char *argv[])
{
	int		cnt;
	char	*command;


	cnt = cmd->history->recs;
	if( cnt == 0)
	{
		printf( "No history found.\n");
		return 0;
	}


	command = Dll_GetFirstPtr( cmd->history);
	while( command != NULL)
	{
		printf( "    %3d %s", cnt--, command);
		command = Dll_GetNextPtr( cmd->history);
	}

	return 1;
}

Cmd_IntQuit( CMD *cmd, int argc, char *argv[])
{
	printf( "Program terminat by command ...\n");
	return -1;
}

