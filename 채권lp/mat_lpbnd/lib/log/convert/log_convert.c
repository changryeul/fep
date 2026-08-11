/** ***************************************************************************
**  @file       log_convert.c
**  @date       2025/01/02
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  LogXxx 형신의 로그 function call을 APLog function으로 변경
**  LogDbg( "rtn=[%d]", rtn)
**  ---> APLog( log_name, APLOG_DEBUG, "rtn=[%d]"" %s(%d)", rtn, __FUNCTION__, __LINE__);
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

/********** USER FUNCTION **********/
typedef struct _log_convert_table_
{
	char	*str;
	char	*conv;
	int		type;
}	LC_TABLE;

LC_TABLE	LogTable[ 64] =
{
	{	"Hex",		NULL,		0		},
	{	"Dec",		NULL,		0		},
	{	"Cha",		NULL,		0		},
	{	"Dump",		NULL,		0		},
	{	"Dddd",		NULL,		0		},
	{	"Dde",		NULL,		0		},
	{	"SetDump",	NULL,		0		},
	{	"Usr",		NULL,		0		},
	{	"Bel",		NULL,		0		},
	{	"Cri",		NULL,		0		},
	{	"CRI",		NULL,		0		},
	{	"Err",		NULL,		0		},
	{	"ERR",		NULL,		0		},
	{	"War",		NULL,		0		},
	{	"Lib",		NULL,		0		},
	{	"Msg",		NULL,		0		},
	{	"App",		NULL,		0		},
	{	"Dbg",		NULL,		0		},
	{	"Dev",		NULL,		0		},
	{	"Dmp",		NULL,		0		},
	{	"Dbg",		NULL,		0		},
	{	"Trc",		NULL,		0		},
	{	"Tst",		NULL,		0		},
	{	"Del",		NULL,		0		},
	{	"Ddd",		NULL,		0		},
	{	"Rem",		NULL,		0		},
	{	"Noh",		NULL,		0		},
	{	"Raw",		NULL,		0		},
	{	"Ra2",		NULL,		0		},
	{	"RAW",		NULL,		0		},
	{	"Dat",		NULL,		0		},
	{	"MEM",		NULL,		0		},
	{	"Open",		NULL,		0		},
	{	"File",		NULL,		0		},
	{	"Mem",		NULL,		0		},
	{	"Close",	NULL,		0		},
	{	"Func",		NULL,		0		},
	{	"Type",		NULL,		0		},
	{	"Size",		NULL,		0		},
	{	"Level",	NULL,		0		},
	{	"GetFd",	NULL,		0		},
	{	"Name",		NULL,		0		},
	{	"Dec",		NULL,		0		}
};
/***********************************/

/********** USER DEFINE VALIABLE **********/
/******************************************/

/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      char *o_file - output file name
**  @param      char *i_file - input file name
**  @return     LC_CONV* - 성공
**  @retval     NULL - 실패
**  @brief      
**  LOG_CONV struct alloc
**  LogXxx -> APLog convert
***************************************************************************** */
int LC_Process( char *o_name, char *i_name)
{
	int			rtn;
	LOG_CONV	*lc;

	lc = LC_Open( o_name, i_name);
	if( lc == NULL)
	{
		LogCri( "LC_Open error. o_name=[%s] i_name=[%s]", o_name, i_name);
		goto error_1;
	}

	while( 1)
	{
		rtn = LC_GetLine( lc);
		if( rtn <= 0) break;

		rtn = LC_CheckLine( lc);
		if( rtn == 0)
		{
			rtn = LC_PutLine( lc);
		}
	}

	LC_Close( lc);
	return 1;

	error_1:
		LC_Close( lc);
		return -1;
}




/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      char *o_file - output file name
**  @param      char *i_file - input file name
**  @return     LC_CONV* - 성공
**  @retval     NULL - 실패
**  @brief      
**  LOG_CONV struct alloc
**  LogXxx -> APLog convert
***************************************************************************** */
LOG_CONV *LC_Open( char *o_name, char *i_name)
{
	LOG_CONV	*lc;

	lc = malloc( sizeof( LOG_CONV));
	if( lc == NULL)
	{
		LogErr( "malloc error. sz=[%ld]", sizeof( LOG_CONV));
		goto error_1;
	}

	memset( lc, 0x00, sizeof( LOG_CONV));

	/* input open */
	if( i_name != NULL)
	{
		lc->i_ptr = fopen( i_name, "r");
		if( lc->i_ptr == NULL)
		{
			LogErr( "fopen error. name=[%s]", i_name);
			goto error_1;
		}
	}
	else
	{
		lc->i_ptr = stdin;
	}
	/* output open */
	if( o_name != NULL)
	{
		lc->o_ptr = fopen( o_name, "w+");
		if( lc->o_ptr == NULL)
		{
			LogErr( "fopen error. name=[%s]", o_name);
			goto error_2;
		}
	}
	else
	{
		lc->o_ptr = stdout;
	}

	return lc;


	error_2:
		fclose( lc->i_ptr);
	error_1:
		return NULL;
}

/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      LC_CONV* 
**  @param      int
**  @return     + - 성공
**  @retval     - - 실패
**  @brief      
**  LogXxx -> APLog convert
***************************************************************************** */
int LC_Close( LOG_CONV *lc)
{
	if( lc->i_ptr != stdin)		fclose( lc->i_ptr);
	if( lc->o_ptr != stdout)	fclose( lc->o_ptr);
	if( lc != NULL) free( lc);

	return 1;
}

/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      LC_CONV* 
**  @return     + - 성공
**  @retval     - - 실패
**  @brief      
**  LogXxx -> APLog convert
***************************************************************************** */
int LC_GetLine( LOG_CONV *lc)
{
	int		sz;
	char 	*ptr;

	ptr = fgets( lc->i_buf[ lc->i_cnt], 8192, lc->i_ptr);
	if( ptr == NULL)
	{
		LogMsg( "End of file. line=[%d] pos=[%d]", lc->line, lc->pos);
		return 0;
	}
	LogRaw( "%s", lc->i_buf[ lc->i_cnt]);
	sz = strlen( lc->i_buf[ lc->i_cnt]);
	LogDbg( "sz=[%d]", sz);
	return sz;
}

/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      LC_CONV* 
**  @return     + - 성공
**  @retval     - - 실패
**  @brief      
**  LogXxx -> APLog convert
***************************************************************************** */
int LC_PutLine( LOG_CONV *lc)
{
	int		rtn;

	rtn = fputs( lc->i_buf[ lc->i_cnt], lc->o_ptr);
	if( rtn <= 0)
	{
		LogErr( "fputs error. line=[%d] pos=[%d] data=[%.*.s", 
				lc->line, lc->pos, strlen( lc->i_buf[ lc->i_cnt]), lc->i_buf[ lc->i_cnt]);
		return 0;
	}
	return rtn;
}

/** ***************************************************************************
**  @fu         int LC_( int argc, char *argv[])
**  @param      LC_CONV* 
**  @return     + - 성공
**  @retval     - - 실패
**  @brief      
**  LogXxx -> APLog convert
***************************************************************************** */
int LC_CheckLine( LOG_CONV *lc)
{
	char	*ptr;

	ptr = strstr( lc->i_buf[ lc->i_cnt], "Log");
	if( ptr == NULL) return 0;

	return  0;
}













