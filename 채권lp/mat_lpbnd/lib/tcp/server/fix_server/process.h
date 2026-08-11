/** ***************************************************************************
**  @file       main.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  메인 모듈 관련 헤더
**  프로그램 사용 파라메터 정의
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "tcp.h"

#ifndef PROCESS_H
#define	PROCESS_H	1

#define	MAX_CLIENT		1

#define	FIX_STAT_LINK	0
#define	FIX_STAT_POLL	1
#define	FIX_STAT_DATA	2

typedef struct _fix_send_
{
	int		rst;						/* recv stat */
	int		sst;						/* send stat */
	char	recv[ 65535];				/* recv data buffer */
	int		rsz;						/* recv size */
	char	send[ 65535];				/* send data buffer */
	int		ssz;						/* send size */
	int		seq;						/* sequence number */
	TCP		*client;					/* tcp/ip client pointer */
}	FIX_COMM;


typedef struct _fix_header_
{
	char	body_length		[  4];					/* 메세자 길이 */
	char	message_type	[  4];					/* 메세지 타입 */
	char	response_code	[  4];					/* 응답코드 */
	char	trade_date		[  8];					/* 기준일자 */
	char	sequence_number	[  8];					/* 일련번호 */
	char	filler			[ 22];					/* 필러 */
}	FIX_COMM_HEAD;


#endif	/* PROCESS_H */

extern int		Continue;
extern PARAM	*Param;
extern CFG		*Cfg;
extern char		Recv[ 65535];
extern char		Send[ 65535];

