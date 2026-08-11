#ifndef	_FUNC_H_
#define	_FUNC_H_	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dll.h"

#ifndef		MAX
#define		MAX( x, y)		(( x > y ) ? x : y)
#define		MIN( x, y)		(( x < y ) ? x : y)
#endif

#define		ST_BUF_SZ		8192
#define		FUNC_CMT_POS	75

typedef struct _func_arg_
{
	char	*type;
	char	*name;
}	FUNC_ARG;

typedef struct _func_def_
{
	char		*type;
	char		*name;
	FUNC_ARG	**arg;
	char		*cmt;
	int			cnt;
}	FUNC_DEF;

typedef struct _func_loader_
{
	char			f_name[ 512];
	FILE			*fp;
	char			rec[ 8192];
	int				lin;
	int				col;

	char			word[ 512];
	int				w_cnt;

	char			**stack;
	int				s_cnt;

	DLL				*func;							/* FUNC_DEF */

	char			*cmt;							/* comment temp */
	int				c_sz;

}	FUNC;

#endif	/* _FUNC_H_ */

/********************************************************************************************************
* function define
********************************************************************************************************/
/***** Module : func.c *****/
int         Func();
FUNC*       Func_Open( char *file_name);
int         Func_Close( FUNC *fn);
int         Func_LoadFile( FUNC *fn);
int         Func_GetLine( FUNC *fn, const char *call);
int         Func_GetChar( FUNC *fn, const char *call);
int         Func_PutChar( FUNC *fn, int c, const char *call);
int         Func_PutWord( FUNC *fn, int c, const char *call);
int         Func_Push( FUNC *fn, const char *call);
int         Func_PushRec( FUNC *fn, char *rec, int sz, const char *call);
int         Func_Pop( FUNC *fn, char *data, int sz, const char *call);
int         Func_GetStackCnt( FUNC *fn);
int         Func_StackClear( FUNC *fn, const char *call);
int         Func_Start( FUNC *fn);
int         Func_SkipEol( FUNC *fn);
int         Func_SkipBrace( FUNC *fn);
int         Func_GetType( FUNC *fn);
int         Func_GetName( FUNC *fn, FUNC_DEF *func);
int         Func_GetComment( FUNC *fn, FUNC_DEF *func);
int         Func_GetArg( FUNC *fn, FUNC_DEF *func);
int         Func_GetString( FUNC *fn, int ch);
int         Func_GetBracketData( FUNC *fn);
int         Func_GetByte( FUNC *fn);
int         Func_Comment( FUNC *fn);
int         Func_CommentPlus( FUNC *fn);
int         Func_Print( FUNC *fn);
int         Func_PrintFile( FUNC *fn, FILE *fp);

