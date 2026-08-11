#ifndef TASK_H
#define	TASK_H	1

#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "cfg.h"
#include "etc.h"
#include "tcp.h"

typedef struct _param_
{
	char	cfg_name[ 512];
	char	log_name[ 512];		/* log file name - output file name */
	char	bat_name[ 512];		/* batch command file */
	char	addr[ 32];
	int		port;
	int		interval;			/* data receive interval */
}   PARAM;

typedef struct _client_
{
	TCP		*tcp;
	char	recv[ 8192];
	char	send[ 8192];
	int		r_sz;
	int		s_sz;
	int		stat;
	int		seq;
}	CLIENT;

typedef struct _send_packet_
{
    char    STX        [1  ];   /* STX:0x02 */
    char    TELG_LNGT  [6  ];   /* 전문길이 */
    char    MASTER     [1  ];   /* MASTER   */
    char    TELG_FG    [2  ];   /* 전문구분 */
    char    TELG_SEQ   [6  ];   /* 전문번호 */
    char    TELG_SND_DT[14 ];   /* 전송일시 */
    char    RESP_CD    [2  ];   /* 응답코드 */
    char    RESV1      [10 ];   /* 예약1    */
    char        pgmd[1024];     /* PGM DATA     */
}   SEND_PACKET;

extern PARAM		*Param;
extern CMD			*Cmd;
extern CMD_TBL		CmdTable[];

extern CFG			*Cfg;

#endif /* TASK_H */

/***** Module : main.c *****/
int         main( int argc, char *argv[]);
int         GetOption( int argc, char *argv[]);
int         InitProcess( int argc, char *argv[]);
int         CommandProcess( int argc, char *argv[]);
int         BatchProcess( int argc, char *argv[]);
int         TermProcess( int argc, char *argv[]);
int         ParamPrint( PARAM *param);

