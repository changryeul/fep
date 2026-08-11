#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifndef CMD_H
#define	CMD_H	1

#include "log.h"
#include "dll.h"

#define	CMD_EXTENAL_COMMAND_ON		0x00000001
#define	CMD_EXTENAL_COMMAND_OFF		0x00000000
#define	CMD_EXIT					-9999
#define	CMD_HELP					-9998
#define	CMD_USAGE					-9997

typedef struct _cmd_
{
	char	cmd[ 8192];
	int		argc;
	char	**argv;
	char	prompt[ 512];					/* 프롬프트 */
	int		c_sz;
	int		external;					/* 외부 명령어 실행  0: 실행 안함, 1: 무조건 실행,  2: 실행 여부 문의 */
	int		internal;					/* cmd 내부명령어 실행 여부 0: 실행 안함, 1: 실행 */
	int		timeout;					/* 명령어 대기 타임아웃 */
	DLL		*cmd_tbl;					/* command table */
	DLL		*int_tbl;					/* internal command table */
	DLL		*history;					/* command history */
	int		max_hist;					/* 명령어 저장 갯수 */
	void	*old_sighandler;			/* for Cmd_SetSignal */
	int		(* prompt_action)( struct _cmd_ *cmd, char *prompt);	/* prompt setting user function) */
	LOG		*log;						/* for command log - 각 명령어를 로그로 기록 */
}	CMD;

typedef struct _cmd_tbl_
{
	int			f_no;
	const char	*cmd;
	int			(* act)( int argc, char **argv);
	const char	*usage;
	const char	*comment;
}	CMD_TBL;

typedef struct _cmd_int_tbl_
{
	int			f_no;
	const char	*cmd;
	int			(* act)( CMD *cmd, int argc, char **argv);
	const char	*usage;
	const char	*comment;
}	CMD_INT_TBL;

#endif

/***** Module : cmd.c *****/
CMD*        Cmd_Open( CMD_TBL *tbl);
int         Cmd_Close( CMD *cmd);
int         Cmd_Main( CMD *cmd);

void*       Cmd_SetSignal( CMD *cmd, int sig_id);
int         Cmd_SetPromptAction( CMD *cmd, int (*prompt_action)( CMD *cmd, char *prompt));
int         Cmd_SetPrompt( CMD *cmd, char *prompt);
char*       Cmd_GetPrompt( CMD *cmd);
int         Cmd_ExecCommand( CMD *cmd);
void        Cmd_SigHandler( int sig_no);
int         Cmd_CommandLog( void *ptr, char *msg, int sz);
int         Cmd_SetLog( CMD *cmd, char *log_file_name);
int         Cmd_RunCommand( CMD *cmd, char *command);
int         Cmd_GetCommand( CMD *cmd, char *command);
int         Cmd_PutCommand( CMD *cmd, char *command);
int         Cmd_GetArgument( CMD *cmd, char *command);
char*       Cmd_GetArgumentSub( CMD *cmd, char *cp);
int         Cmd_RunExtCommand( CMD *cmd);
int         Cmd_GetInternal( CMD *cmd);
int         Cmd_IntSet( CMD *cmd, int argc, char *argv[]);
int         Cmd_IntSleep( CMD *cmd, int argc, char *argv[]);
int         Cmd_IntUntil( CMD *cmd, int argc, char *argv[]);
int         Cmd_IntHelp( CMD *cmd, int argc, char *argv[]);
int         Cmd_IntHelpOld( CMD *cmd, int argc, char *argv[]);
int         Cmd_IntHistory( CMD *cmd, int argc, char *argv[]);
int         Cmd_GetPrevCommand( CMD *cmd, int pos);
int         Cmd_GetPrevHistCommand( CMD *cmd, char *str);
int         Cmd_GetFirstCommand( CMD *cmd, int pos);
int         Cmd_EditCommand( CMD *cmd);
int         Cmd_Display( CMD *cmd);
int         Cmd_HelpCommand( CMD *cmd, char *command);
char*       Cmd_ConvertComment( CMD *cmd, const char *comment);
int         Cmd_AtoI( char *str, int sz);
int         Cmd_Set( CMD *cmd, int opt);

