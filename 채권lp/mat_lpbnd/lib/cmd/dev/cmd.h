#ifndef CMD_H
#define	CMD_H	1

#include "log.h"
#include "dll.h"

#define	CMD_EXTENAL_COMMAND_ON		0x00000001
#define	CMD_EXTENAL_COMMAND_OFF		0x00000000

typedef struct _cmd_
{
	char	cmd[ 8192];
	int		argc;
	char	**argv;
	char	*prompt;					/* 프롬프트 */
	int		c_sz;
	int		external;					/* 외부 명령어 실행  0: 실행 안함, 1: 무조건 실행,  2: 실행 여부 문의 */
	int		timeout;					/* 명령어 대기 타임아웃 */
	DLL		*cmd_tbl;					/* command table */
	DLL		*cmd_int_tbl;				/* command table */
	DLL		*history;					/* command history */
	int		max_hist;					/* 명령어 저장 갯수 */
}	CMD;

typedef struct _cmd_tbl_
{
	int		f_no;
	char	*cmd;
	int		(* act)( int argc, char **argv);
	char	*usage;
	char	*comment;
}	CMD_TBL;

typedef struct _cmd_int_tbl_
{
	int		f_no;
	char	*cmd;
	int		(* act)( CMD *cmd, int argc, char **argv);
	char	*usage;
	char	*comment;
}	CMD_INT_TBL;

#endif

/*** Module : cmd.c ***/
CMD*	Cmd_Open( CMD_TBL *tbl);
int		Cmd_Close( CMD *cmd);
int		Cmd_Main( CMD *cmd);
int		Cmd_ExecCommand( CMD *cmd);
void 	Cmd_SigHandler( int sig_no);
int		Cmd_GetCommand( CMD *cmd, char *command);
int		Cmd_PutCommand( CMD *cmd, char *command);
int		Cmd_GetArgument( CMD *cmd, char *command);
int		Cmd_RunExtCommand( CMD *cmd);
int		Cmd_GetInternal( CMD *cmd);

int		Cmd_IntHelp( CMD *cmd, int argc, char *argv[]);
int		Cmd_IntHistory( CMD *cmd, int argc, char *argv[]);
int		Cmd_IntQuit( CMD *cmd, int argc, char *argv[]);
int		Cmd_GetPrevCommand( CMD *cmd, int pos);
int		Cmd_EditCommand( CMD *cmd);

