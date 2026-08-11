/** ***************************************************************************
**	@file		msg.c
**	@date		2022/08/23
**	@author		최동춘
**	@version	V2.0.20220823
**	@brif		
**	로그 파일 관리 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "log.h"
#include "msg.h"
#include "cfg.h"
#include "etc.h"


MSG				MsgBuf =
{
	"",									/* pname */
	0,									/* pid */
	"/kmbapp/fep/cfg/message.cfg",		/* fname */
	0,									/* fd */
	NULL,								/* cfg */
	"",									/* message */
	0									/* sz */
};
MSG				*MsgPtr = &MsgBuf;

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	메세지를 사용하기위한 초기화 작업
**  환경변수 $FEP_CFG_DIR 사용 - message.cfg 정보를 로드 
***************************************************************************** */
MSG	*Msg_Open( char *pname, pid_t pid)
{
	MSG		*msg;
	char	cfg_name[ 512];

	msg = MsgPtr;		/* 전역변수로 선언한 msg 포인터 사용 */

	sprintf( cfg_name, "%s/%s", getenv( "FEP_CFG_DIR"), "message.cfg");
	msg->cfg = Cfg_Open( cfg_name);
	if( msg->cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}
	LogDbg( "Cfg_Open success.  name=[%s] cfg=[%p]", cfg_name, msg->cfg);

	memcpy( msg->pname, pname, strlen( pname) +1);
	msg->pid = pid;
	Cfg_Get( msg->cfg, "msg_name", msg->fname, 512);

	if( msg->fd <= 0)
	{
		msg->fd = open( msg->fname, O_RDWR);
		if( msg->fd < 0)
		{
			LogErr( "msg_file open error. name=[%s]", msg->fname);
			goto error;
		}
	}

	Msg_Print( msg);

	return msg;

	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_Close( MSG *msg)
{
	if( msg->fd > 0)			
	{
		close( msg->fd);
		msg->fd = -1;
	}
	if( msg->cfg != NULL)		
	{
		Cfg_Close( msg->cfg);
		msg->cfg = NULL;
	}
	msg = NULL;

	return 1;
}

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_Send( const char *file, const char *func, int line, int level, MSG *msg, char *code, char *format, ...)
{
	va_list		args;
	int			sz = 0, s_sz;
	MCI_PUSH_HDR	*head = ( MCI_PUSH_HDR *)msg->message;
	char			*msg_sz = &msg->message[ sizeof( MCI_PUSH_HDR)];
	char			*data = msg_sz +6;
	char			*code_ptr = "";

	if( msg == NULL)
	{
		printf( "Msg_Send error. Message not opened.  MsgPtr=[%p]\n", msg);
		return -1;
	}

	LogDbg( "Mag_Send ... msg=[%p] code=[%s]", msg, code);
	if( code == NULL) code = code_ptr;

	va_start( args, format);
	sz += vsnprintf( data, MSG_BUFF_SZ, format, args);
	va_end( args);

	LogDbg( "DATA SIZE = [%d]", sz);

	if( !memcmp( code, "O2", 2))
	{
		code_ptr = Cfg_GetFirstNamePtr( msg->cfg, "O2");
		while( code_ptr != NULL)
		{
			LogDbg( "message send to = [%s]", code_ptr);
			s_sz = Msg_MakeHead( msg, code_ptr, sz);
			if( s_sz < 0)
			{
				LogCri( "Msg_MakeHead error. msg=[%p] code=[%s] sz=[%d] s_sz=[%d]", msg, code, sz, s_sz);
				return s_sz;
			} 
			Msg_Write( msg, msg->message, s_sz);
			code_ptr = Cfg_GetNextNamePtr( msg->cfg, "O2");
		}

		return sz;
	}
	else
	{
		s_sz = Msg_MakeHead( msg, code, sz);
		if( s_sz < 0)
		{
			LogCri( "Msg_MakeHead error. msg=[%p] code=[%s] sz=[%d] s_sz=[%d]", msg, code, sz, s_sz);
			return s_sz;
		} 
	}

	Msg_Write( msg, msg->message, s_sz);

	return sz;
}

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_SendData( MSG *msg)
{
	Msg_Write( msg, msg->message, msg->sz);

	return msg->sz;
}

/** ***************************************************************************
**	@fn			int	Msg_MakeHead( MSG *msg, char *code, int sz)
**	@param		MSG *msg - message 구조체
**	@param		char *code - null이면 전체 메세지 혹은 code(2)+id(10) 
**	@param		int sz - 데이타 메세지 길이
**	@return		data 전체 길이
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_MakeHead( MSG *msg, char *code, int sz)
{
	int				s_sz;
	MCI_PUSH_INFO_HDR	*message = ( MCI_PUSH_INFO_HDR *)msg->message;
	MCI_PUSH_HDR		*head	= ( MCI_PUSH_HDR *)msg->message;
	char				*d_seq	= &msg->message[ sizeof( MCI_PUSH_HDR)];
	char				*data	= d_seq +sizeof( message->d_seq);
	char				id[ 32];

	struct timeval	tv;
	struct tm		*tp;
	char			cur_time[ 32];

	gettimeofday( &tv, NULL);
	tp = localtime( &tv.tv_sec);
	sprintf( cur_time, "%02d%02d%02d%03d", tp->tm_hour, tp->tm_min, tp->tm_sec, tv.tv_usec / 1000);

	/* d_seq size를 더하여 쓰기 */
	sz += sizeof( message->d_seq);


	if( code[0] == 0)		/* 전체 메세지 */
	{
		memcpy( head->d_pbcode, 		"C1", 			sizeof( head->d_pbcode));
		memcpy( head->d_company, 		"9999999",		sizeof( head->d_company));
		memset( head->d_reservkey, 		0x20,			sizeof( head->d_reservkey));
	}
	else					/* 대상 ID 메세지 */
	{
		memcpy( head->d_pbcode, 		code, 			sizeof( head->d_pbcode));
		memset( head->d_company, 		0x20, 			sizeof( head->d_company));
		sprintf( id, "%-*.*s", sizeof( head->d_reservkey), sizeof( head->d_reservkey), &code[ 2]);
		memcpy( head->d_reservkey, 		id,				sizeof( head->d_reservkey));
	}

	memcpy( head->d_media_tp, 		"D", 			sizeof( head->d_media_tp));
	memcpy( head->d_stime, 			cur_time, 		sizeof( head->d_stime));
	ItoA  ( head->d_size,			sz,				sizeof( head->d_size));
	memset( d_seq, 					0x30, 			6);

	s_sz = sizeof( MCI_PUSH_HDR) + sz;
	return s_sz;
}

/** ***************************************************************************
**	@fn			int	Msg_MakeHead( MSG *msg, char *code, int sz)
**	@param		MSG *msg - message 구조체
**	@param		char *code - null이면 전체 메세지 혹은 code(2)+id(10) 
**	@param		int sz - 데이타 메세지 길이
**	@return		data 전체 길이
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_Write( MSG *msg, char *rec, int sz)
{
	int		rtn;
	off_t	seek;

	LogDbg( "msg=[%p] rec=[%p] sz=[%d]", msg, rec, sz);
	LogDec( rec, sz);

	seek = lseek( msg->fd, 0, SEEK_END);
	if( seek < (off_t)0)
	{
		LogErr( "lseek error. fd=[%d] offset=[0] whence=[SEEK_END]", msg->fd);
		return -1;
	}

	rtn = write( msg->fd, rec, sz);
	if( rtn < sz)
	{
		LogErr( "write error. fd=[%d] data=[%p] sz=[%d]", msg->fd, rec, sz);
		return -1;
	}

	rtn = write( msg->fd, "\n", 1);
	if( rtn < 1)
	{
		LogErr( "write error. fd=[%d] data=[\\n] sz=[1]", msg->fd);
		return -1;
	}

	return sz +1;
}

#if 0
/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_Head( MSG *msg, const char *file, const char *func, int line, int level)
{
	int				sz = 0;
	int				sz_hd = 0;
	time_t			cur_time;
	struct tm		*tp;
	struct timeval	tv;
	char			lv[32] = "DMALWECB U     \0";
	int				color[16] = { 37, 37, 36, 32, 33, 31, 31, 31, 31, 37, 37};

	sz_hd = MSG_HEAD_SIZE;
	time( &cur_time);
	tp = localtime( &cur_time);
	gettimeofday( &tv, NULL);

	sz += sprintf( &msg->buf[ sz], "%02d:%02d:%02d-%06d ", tp->tm_hour, tp->tm_min, tp->tm_sec, tv.tv_usec);
	if( msg->head & MSG_HEAD_COLOR)
	{
		sz_hd += 9;
		sz += sprintf( &msg->buf[ sz], "\033[%dm%c\033[0m ", color[ level], lv[ level]);
	}
	else
	{
		sz += sprintf( &msg->buf[ sz], "%c ", lv[ level]);
	}
	sz += sprintf( &msg->buf[ sz], "%s:%s(%d)  ", file, func, line);
	if( msg->head & MSG_HEAD_BELL)
	{
		if( level == 7) msg->buf[ sz++] = '';
	}
	if( sz <= sz_hd)
	{
		memset( &msg->buf[ sz], 0x20, sz_hd - sz);
		sz = sz_hd;
	}

	return sz;
}

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int Msg_Tail( MSG *msg, char *tail, int sz)
{
	if( msg == NULL)
	{
		msg = MsgPtr = Msg_Open( NULL);
		if( msg == NULL)
		{
			printf( "Msg_Open error. err=[%d:%s]\n", errno, strerror( errno));
			return -1;
		}
	}

	msg->tail = ( char *)realloc( msg->tail, sz +1);
	if( msg->tail == NULL)
	{
		printf( "Msg_Tail: realloc error. Err=[%d:%s]\n", errno, strerror( errno));
		return -errno;
	}

	memcpy( msg->tail, tail, sz);
	msg->tail[ sz] = 0;

	return 1;
}

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int Msg_Out( MSG *msg, char *data, int sz)
{
	int		rtn;

	if( msg->f_name == NULL)
	{
		rtn = write( 1, data, sz);
		return rtn;
	}

	rtn = Msg_FileCheck( msg);
	if( rtn < 0) return -1;
	if( msg->fd < 0) return 0;

	rtn = write( msg->fd, data, sz);
	if( rtn < 0)
	{
		printf( "msg write error. fd=[%d] err=[%d:%s]\n", msg->fd, errno, strerror( errno));
	}

	return rtn;
}

#endif

/** ***************************************************************************
**	@fn			MSG *Msg_Open( char *f_name)
**	@param		char *f_name - 파일이름 NULL:stdout
**	@return		MSG 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	로그 파일을 사용하기위한 초기화 작업
**	AP에서 MsgOpen(헤더에 define 되어있는)을 호출하지 않으면 
**	전역 변수 MsgPtr을 사용 - stdout으로 로그 출력
**	MsgOpen을 AP에서 호출하면 그때부터 f_name에 로그 출력
***************************************************************************** */
int	Msg_Print( MSG *msg)
{
	LogDbg( "msg->pname            = [%s]", msg->pname);
	LogDbg( "msg->pid              = [%d]", msg->pid);
	LogDbg( "msg->fname            = [%s]", msg->fname);
	LogDbg( "msg->fd               = [%d]", msg->fd);
	LogDbg( "msg->message          = [%s]", msg->message);
	LogDbg( "msg->sz               = [%d]", msg->sz);
}


