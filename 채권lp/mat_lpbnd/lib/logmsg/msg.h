/** ***************************************************************************
**  @file		msg.h
**  @date		2022/09/22
**  @author		최동춘
**  @version	V2.0.20220823
**  @brif		
**	로그 파일 관리 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#include "log.h"
#include "cfg.h"

#ifndef MSG_H
#define	MSG_H 1

/********** define **********/
#define		MSG_BUFF_SZ		2000
#define     MsgSend( code, format, args...)    Msg_Send( __FILE__, __FUNCTION__, __LINE__, 1, MsgPtr, code, ( char *)format, ##args)


/** ***************************************************************************
**  @struct		MSG
**  @brief		메세지 DATA 구조체
***************************************************************************** */
typedef struct _MCI_PUSH_HDR_
{
    char d_pbcode		[ 2];   /* 실시간 전송 O2=(dhkim),O3(특정ID전송),C1(전체전송) */
    char d_company		[ 7];   /* 전송대상(회사코드) 9999999 - 전체, space - 특정인 */
    char d_reservkey	[10];   /* 전송대상(ID, 특정인) space - 전체 */
    char d_media_tp		[ 1];   /* 매체구분 : D-단말, A-API, X-All D - 단말로 */
    char d_stime		[ 9];   /* 데이터 전송시간(HHMMSSmsc)                   */
    char d_size			[ 4];   /* 실시간 데이터부의 길이(헤더부 제외길이,000#) */
}	MCI_PUSH_HDR;

typedef struct _MCI_PUSH_INFO_HDR_
{
    MCI_PUSH_HDR    mci_push_hdr;
    char d_seq     [6];     /* 전송데이터의 seq (00000#)                    */
    char d_pbdata  [2000+1];/* 실시간 데이터                                */  
}	MCI_PUSH_INFO_HDR;



/** ***************************************************************************
**  @struct		MSG
**  @brief		메세지 관리 구조체
***************************************************************************** */
typedef struct _msg_
{
	char		pname[ 64];				/** @var process name */
	pid_t		pid;					/** @var process id */
	char		fname[ 512];			/** @var message write file name - full path */
	int			fd;						/** @var message file descripter */
	CFG			*cfg;					/** @var config file pointer */
	char		message[ 2048];			/** @var message data */
	int			sz;						/** @var data size */
}	MSG;

extern MSG	*MsgPtr;





#endif

/***** Module : msg.c *****/
MSG*        Msg_Open( char *pname, pid_t pid);
int         Msg_Close( MSG *msg);
int         Msg_Send( const char *file, const char *func, int line, int level, MSG *msg, char *code, char *format, ...);
int         Msg_SendData( MSG *msg);
int         Msg_MakeHead( MSG *msg, char *code, int sz);
int         Msg_Write( MSG *msg, char *rec, int sz);
int         Msg_Head( MSG *msg, const char *file, const char *func, int line, int level);
int         Msg_Tail( MSG *msg, char *tail, int sz);
int         Msg_Out( MSG *msg, char *data, int sz);
int         Msg_Print( MSG *msg);

