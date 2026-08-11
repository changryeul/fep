/************************************************************************************************
 *
 *
 *
 *
 *
************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "func.h"
#include "task.h"

/*************************************************************************************************
 *
*************************************************************************************************/
typedef struct	_stloader_keyword_
{
	int		no;												/* key number						*/
	char	word[ 32];										/* key word							*/
	char	comment[ 512];
}	FUNC_KEYWORD;

FUNC_KEYWORD	FuncKeyTable[ 32] = 
{
	{	1,		"typedef",	""},
	{	2,		"struct",	""},
	{	-1,		"",			""}
};

/************************************************************************************************
 *
************************************************************************************************/
FUNC   *Func_Open( char *file_name);

int Func()
{
	int			rtn, pos;
	FUNC		*fn;
	char		*f_name, buf[ 512];
	FILE		*ofp, *ifp;
	struct stat	stat_buf;

	f_name = Param.out_file;

	if( f_name[ 0] == 0)
	{
		ofp = stdout;
	}
	else
	{
		sprintf( buf, "a+");

		rtn = stat( f_name, &stat_buf);
		if( rtn == 0)
		{
			printf( "Output file already exist. name=[%s] \n", f_name);
			printf( "input action (a)ppend/(c)ancel/(d)elete/(o)verwrite (default=append): ");
			fgets( buf, 512, stdin);
			switch( buf[ 0])
			{
				case 'o':
					printf( "overwrite file. name=[%s]\n", f_name);
					sprintf( buf, "w+");
					break;
				case 'd':
					unlink( f_name);
				case 'c':
					return -1;
				case 'a':
				default :
					printf( "append file. name=[%s]\n", f_name);
					sprintf( buf, "a+");
					break;
			}
		}

		ofp = fopen( f_name, buf);
		if( ofp == NULL)
		{
			LogErr( "fopen error. name=[%s]", f_name);
			return -1;
		}
	}

	pos = 0;
	f_name = Param.file_list[ pos];
	while( f_name != NULL)
	{
		printf( "loading file. name=[%s] \n", f_name);
		fn = Func_Open( f_name);
		if( fn == NULL)
		{
			LogCri( "Func_Open error. name=[%s]", f_name);
			break;
		}

		rtn = Func_PrintFile( fn, ofp);

		Func_Close( fn);

		pos++;
		f_name = Param.file_list[ pos];
	}

	/*
	fn = Func_Open( "test.h");
	if( fn == NULL)
	{
		return -1;
	}

	Func_Close( fn);
	*/

	
	/*
	fp = fopen( "test.c", "a+");
	Func_PrintFile( fn, stdout);
	Func_PrintFile( fn, fp);
	fclose( fp);
	*/

	return 1;

}

FUNC	*Func_Open( char *file_name)
{
	int		rtn;
	FUNC	*fn;

	fn = malloc( sizeof( FUNC));
	if( fn == NULL)
	{
		LogErr( "malloc error. FUNC sz=[%d]", sizeof( FUNC));
		goto error_1;
	}
	memset( fn, 0, sizeof( FUNC));

	memcpy( fn->f_name, file_name, strlen( file_name) +1);

	fn->func = Dll_Open( 0);
	if( fn->func == NULL)
	{
		LogCri( "Dll_Open error. fn->func");
		goto error_2;
	}

	rtn = Func_LoadFile( fn);
	if( rtn < 0)
	{
		goto error_2;
	}

	return fn;

	error_3:
		Dll_Close( fn->func);
	error_2:
		free( fn);
	error_1:
		return NULL;
}

int Func_Close( FUNC *fn)
{
	int				pos;
	FUNC_DEF		*func;

	func = Dll_GetFirstPtr( fn->func);
	while( func != NULL)
	{
		while( pos < func->cnt)
		{
			if( func->arg[ pos] != NULL)	free( func->arg[ pos]);
			pos++;
		}
		if( func->arg != NULL)  free( func->arg);
		if( func->name != NULL) free( func->name);
		if( func->type != NULL) free( func->type);
		func = Dll_GetNextPtr( fn->func);
	}

	if( fn->fp != NULL)	fclose( fn->fp);
	fn->fp = NULL;
	free( fn);

	return 1;
}

int Func_LoadFile( FUNC *fn)
{
	int		rtn;
	char 	*p;

	LogDel( "c source file open. name=[%s]", fn->f_name);

	if( fn->fp == NULL)
	{
		fn->fp = fopen( fn->f_name, "r");
		if( fn->fp == NULL)
		{
			LogErr( "file open error. name=[%s]", fn->f_name);
			return -1;
		}
		fn->lin = 0;
		fn->col = 0;
	}

	rtn = Func_Start( fn);

	if( fn->fp != NULL) fclose( fn->fp);
	fn->fp = NULL;

	return 1;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetLine( FUNC *fn, const char *call)
{
	char	*p;

	p = fgets( fn->rec, ST_BUF_SZ, fn->fp);
	if( p == NULL)
	{
		LogDel( "End of file. call=[%s] name=[%s] fp=[%p]", call, fn->f_name, fn->fp);
		return -1;
	}

	fn->lin++;
	fn->col = 0;

	LogDel( "line %-20s line=[%d]", call, fn->lin);
	LogDel( "[%s]", fn->rec);
	return fn->lin;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetChar( FUNC *fn, const char *call)
{
	int		rtn, ch;

	ch = fn->rec[ fn->col];
	if( ch == 0)
	{
		rtn = Func_GetLine( fn, call);
		if( rtn < 0) return -1;
		fn->col = 0;
		ch = fn->rec[ fn->col];
	}

	LogDel( "char %-20s pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
			call, fn->lin, fn->col, fn->rec[ fn->col], fn->rec[ fn->col], fn->rec[ fn->col]);

	fn->col++;
	if( fn->col >= ST_BUF_SZ) return -1;

	return ch;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_PutChar( FUNC *fn, int c, const char *call)
{
	fn->col--;
	fn->rec[ fn->col] = c;
	LogDel( "putc %-20s pos=[%3d:%3d] char=[%c][0x%02x][%3d]", call, fn->lin, fn->col, c, c, c);

	return c;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_PutWord( FUNC *fn, int c, const char *call)
{
	int		rtn, ch;

	fn->word[ fn->w_cnt] = c;
	fn->w_cnt++;
	if( fn->w_cnt >= 512) return -1;
	fn->word[ fn->w_cnt] = 0;

	LogDel( "word %-20s pos=[%3d] word=[%d:%s]", call, fn->w_cnt, fn->w_cnt, fn->word);

	return fn->w_cnt;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_Push( FUNC *fn, const char *call)
{
	int		pos = 0;

	if( fn->stack == NULL)
	{
		fn->stack = malloc( sizeof( char *));
		if( fn->stack == NULL)
		{
			LogErr( "malloc error. stack size=[%d]", sizeof( char *));
			return -1;
		}
		fn->stack[ fn->s_cnt] = NULL;
	}

	if( fn->w_cnt <= 0) return 0;

	fn->stack = realloc( fn->stack, sizeof( char *) * ( fn->s_cnt +2));
	if( fn->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}

	fn->stack[ fn->s_cnt] = malloc( fn->w_cnt +1);
	if( fn->stack[ fn->s_cnt] == NULL)
	{
		LogErr( "malloc error. stack data sz=[%d]", fn->w_cnt);
		return -1;
	}

	fn->word[ fn->w_cnt] = 0;
	memcpy( fn->stack[ fn->s_cnt], fn->word, fn->w_cnt +1);
	fn->stack[ fn->s_cnt][ fn->w_cnt] = 0;
	LogDel( "push %-20s stack=[%p] stack[%d]=[%p] data=[%s] sz=[%d]", 
			call, fn->stack, fn->s_cnt, fn->stack[ fn->s_cnt], fn->stack[ fn->s_cnt], fn->w_cnt);

	fn->s_cnt++;
	fn->stack[ fn->s_cnt] = NULL;

	pos = fn->s_cnt -1;
	while( pos >= 0) 
	{
		LogDel( "stack[%2d]=[%s]", pos, fn->stack[ pos]);
		pos--;
	}

	fn->w_cnt = 0;
	fn->word[ fn->w_cnt] = 0;

	return fn->w_cnt;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_PushRec( FUNC *fn, char *rec, int sz, const char *call)
{
	int		pos = 0;

	if( fn->stack == NULL)
	{
		fn->stack = malloc( sizeof( char *));
		if( fn->stack == NULL)
		{
			LogErr( "malloc error. stack size=[%d]", sizeof( char *));
			return -1;
		}
		fn->stack[ fn->s_cnt] = NULL;
	}

	fn->stack = realloc( fn->stack, sizeof( char *) * ( fn->s_cnt +2));
	if( fn->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}

	fn->stack[ fn->s_cnt] = malloc( sz +1);
	if( fn->stack[ fn->s_cnt] == NULL)
	{
		LogErr( "malloc error. stack data sz=[%d]", fn->w_cnt);
		return -1;
	}

	memcpy( fn->stack[ fn->s_cnt], rec, sz +1);
	fn->stack[ fn->s_cnt][ sz] = 0;
	LogDel( "p__r %-20s stack=[%p] stack[%d]=[%p] data=[%s] sz=[%d]", 
			call, fn->stack, fn->s_cnt, fn->stack[ fn->s_cnt], fn->stack[ fn->s_cnt], sz);

	fn->s_cnt++;
	fn->stack[ fn->s_cnt] = NULL;

	pos = fn->s_cnt -1;
	while( pos >= 0) 
	{
		LogDel( "stack[%2d]=[%s]", pos, fn->stack[ pos]);
		pos--;
	}

	return fn->w_cnt;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_Pop( FUNC *fn, char *data, int sz, const char *call)
{
	int		pos = 0;
	int		s_sz;

	if( fn->stack == NULL) return 0;
	if( fn->s_cnt <= 0) return 0;

	fn->s_cnt--;
	if( fn->s_cnt < 0) 
	{
		LogDel( "pop  %-20s data=[0:]", call);
		return 0;
	}
	s_sz = strlen( fn->stack[ fn->s_cnt]);
	if( s_sz >sz) s_sz = sz -1;
	memcpy( data, fn->stack[ fn->s_cnt], s_sz);
	data[ s_sz] = 0;

	free( fn->stack[ fn->s_cnt]);
	fn->stack[ fn->s_cnt] = NULL;

	fn->stack = realloc( fn->stack, sizeof( char *) * ( fn->s_cnt +2));
	if( fn->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}

	LogDel( "pop  %-20s data=[%d:%s]", call, s_sz, data);

	return s_sz;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetStackCnt( FUNC *fn)
{
	return fn->s_cnt;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_StackClear( FUNC *fn, const char *call)
{
	int pos;

	LogDel( "stack clear. cnt=[%d] call=[%s]", fn->s_cnt, call);

	fn->w_cnt = 0;
	fn->word[ fn->w_cnt] = 0;

	if( fn->s_cnt <= 0) return 1;

	pos = fn->s_cnt -1;
	while( pos >= 0)
	{
		LogDel( "stack clear pos=[%d] stack=[%p]", pos, fn->stack[ pos]);
		if( fn->stack[ pos] != NULL) free( fn->stack[ pos]);
		pos--;
	}

	fn->s_cnt = 0;
	fn->stack = realloc( fn->stack, sizeof( char *) * 2);
	if( fn->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}
	fn->stack[ fn->s_cnt] = NULL;

	return 1;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_Start( FUNC *fn)
{
	int				rtn, c;
	int				stat = 0;
	void			*ptr;

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1   :
				return 0;
			case '(' :
				Func_PutChar( fn, c, __FUNCTION__);
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( stat < 1) break;
				Func_Push( fn, __FUNCTION__);
				Func_GetType( fn);
				Func_StackClear( fn, __FUNCTION__);
				stat = 0;
				break;
			case '#' :
				Func_SkipEol( fn);
				break;
			case '\"':
			case '\'':
				Func_GetString( fn, c);
				Func_StackClear( fn, __FUNCTION__);
				break;
			case ';' :
				stat = 0;
				Func_StackClear( fn, __FUNCTION__);
				break;
			case '/' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_CommentPlus( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
				}
				break;
			case '*' :
				LogDel( "char=[%c]", fn->word[ fn->w_cnt -1]);
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_Comment( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
					Func_Push( fn, __FUNCTION__);
				}
				break;
			default  :
				Func_PutWord( fn, c, __FUNCTION__);
				stat = 1;
				break;
		}
	}

	error_1:
		LogDel( "Syntax error. line=[%d] column=[%d]", fn->lin, fn->col);
		return -1;
	
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int Func_SkipEol( FUNC *fn)
{
	int		c, rtn;

	LogDel( "Func_SkipEol ... ");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( fn->rec[ fn->col])
		{
			case -1   :
				return 0;
			case '\r':
			case '\n':
				return 1;
			default  :
				break;
		}
	}
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int Func_SkipBrace( FUNC *fn)
{
	int		c, rtn;
	int		cnt = 0;

	LogDel( "Func_SkipBrace ... ");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1  :
				return 0;
			case '\"':
			case '\'':
				Func_GetString( fn, c);
				Func_StackClear( fn, __FUNCTION__);
				break;
			case '{' :
				cnt++;
				LogDel( "Func_SkipBrace cnt=[%d]", cnt);
				break;
			case '}' :
				cnt--;
				LogDel( "Func_SkipBrace cnt=[%d]", cnt);
				if( cnt < 0) return 1;
				break;
			default  :
				break;
		}
	}
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetType( FUNC *fn)
{
	int			rtn, c, sz, pos = 0;
	int			stat = 0;
	char		rec[ 512];
	FUNC_DEF	*func;

	func = malloc( sizeof( FUNC_DEF));
	if( func == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( FUNC_DEF));
		goto error_1;
	}
	memset( func, 0, sizeof( FUNC_DEF));
	memset( rec, 0, 512);

	sz = Func_Pop( fn, rec, 512, __FUNCTION__);
	if( sz < 0)
	{
		goto clear;
	}

	func->type = malloc( sz +1);
	if( func->type == NULL)
	{
		goto clear;
	}
	memcpy( func->type, rec, sz +1);
	LogDel( "GetType type=[%s]", func->type);
	
	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1   :
				return 0;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( stat < 1) break;
				Func_Push( fn, __FUNCTION__);
				stat = 2;
				break;
			case '=' :
			case ';' :
				goto clear;
			case '(' :
				LogDel( "stat=[%d]", stat);
				if( stat < 1)
				{
					Func_PushRec( fn, func->type, strlen( func->type), __FUNCTION__);

					if( func->type != NULL)
					{
						free( func->type);
						func->type = NULL;
					}
				}
				else
				{
					Func_Push( fn, __FUNCTION__);
				}
				Func_PutChar( fn, c, __FUNCTION__);
				rtn = Func_GetName( fn, func);
				if( rtn <= 0) goto clear;
				rtn = Func_GetComment( fn, func);
				Dll_Add( fn->func, func, sizeof( FUNC_DEF));
				return 1;
			case '{' :
				Func_Push( fn, __FUNCTION__);
				Func_StackClear( fn, __FUNCTION__);
				goto clear;
				break;
			case '/' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_CommentPlus( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
				}
				break;
			case '*' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_Comment( fn);
				}
				else
				{
					sz = strlen( func->type);
					func->type = realloc( func->type, sz +2);
					if( func->type == NULL)
					{
						LogErr( "realloc error. func->type sz=[%d]", sz +2);
						return -1;
					}
					func->type[ sz] = c;
					func->type[ sz +1] = 0;
					LogDel( "GetType type=[%s]", func->type);
				}
				break;
			default  :
				Func_PutWord( fn, c, __FUNCTION__);
				stat = 1;
				break;
		}
	}

	return 1;

	clear:
		if( func->arg != NULL)
		{
			pos = func->cnt -1;
			while( pos >= 0)
			{
				if( func->arg[ pos] != NULL) free( func->arg[ pos]);
				pos--;
			}
			free( func->arg);
		}
		if( func->name != NULL) free( func->name);
		if( func->type != NULL) free( func->type);
		free( func);
		return 0;

	error_3:
		free( func->type);
	error_2:
		free( func);
	error_1:
		return -1;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetName( FUNC *fn, FUNC_DEF *func)
{
	int			sz, c, pos = 0;
	int			stat = 0;
	char		rec[ 512];

	sz = Func_Pop( fn, rec, 512, __FUNCTION__);
	if( sz <= 0)
	{
		LogCri( "Func_Pop error. at=[%d:%d]", fn->lin, fn->col);
		goto error_2;
	}

	func->name = malloc( sz +1);
	if( func->name == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz +1);
		goto error_2;
	}
	memcpy( func->name, rec, sz +1);

	LogDel( "GetName type=[%s]", func->type);
	LogDel( "GetName name=[%s]", func->name);

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1   :
				return 0;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( stat < 1) break;
				Func_Push( fn, __FUNCTION__);
				stat++;
				break;
			case ';' :
				Func_PutChar( fn, c, __FUNCTION__);
				return 0;
			case '\"':
			case '\'':
				Func_GetString( fn, c);
				Func_StackClear( fn, __FUNCTION__);
				break;
			case '(' :
				Func_GetArg( fn, func);
				stat = 3;
				break;
			case '{' :
				if( stat < 3)
				{
					LogCri( "syntax error. stat=[%d] at=[%d:%d]", stat, fn->lin, fn->col);
					return -1;
				}
				Func_SkipBrace( fn);
				return 1;
			case '/' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_CommentPlus( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
				}
				break;
			case '*' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_Comment( fn);
				}
				else
				{
					sz = strlen( func->type);
					func->type = realloc( func->type, sz +2);
					if( func->type == NULL)
					{
						LogErr( "realloc error. func->type sz=[%d]", sz +2);
						return -1;
					}
					func->type[ sz] = c;
					func->type[ sz +1] = 0;
					LogDel( "GetType type=[%s]", func->type);
				}
				break;
			default  :
				Func_PutWord( fn, c, __FUNCTION__);
				stat = 1;
				break;
		}
	}

	return 1;

	error_3:
		free( func->name);
	error_2:
		free( func);
	error_1:
		return -1;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetComment( FUNC *fn, FUNC_DEF *func)
{
	char 	*ptr;
	int		sz = 0;

	LogDel( "fn->cmt      = [%p]", fn->cmt);
	if( fn->cmt == NULL) return 0;

	LogDel( "fn->cmt      = [%s]", fn->cmt);

	ptr = strstr( fn->cmt, "@brief");
	if( ptr == NULL) 
	{
		free( fn->cmt);
		fn->cmt = NULL;
		fn->c_sz = 0;
		return 0;
	}

	LogDel( "ptr      = [%s]", ptr);

	/* get start */
	while( *ptr != '\n') 
	{	
		ptr++;
		if( *ptr == 0)
		{
			free( fn->cmt);
			fn->cmt = NULL;
			fn->c_sz = 0;
			return 0;
		}
	}

	LogDel( "ptr      = [%s]", ptr);
	ptr++;

	/* remove start charector */
	while( 1)
	{
		switch( *ptr)
		{
			case ' ':
			case '\t':
			case '*':
				ptr++;
				continue;
			default:
				break;
		}
		break;
	}

	func->cmt = malloc( sz +1);
	/* get comment */
	while( *ptr != '\n')
	{
		func->cmt[ sz] = *ptr;
		sz++;
		ptr++;
		if( *ptr == 0) break;
		func->cmt = realloc( func->cmt, sz +1);

	}
	func->cmt[ sz] = 0;

	free( fn->cmt);
	fn->cmt = NULL;
	fn->c_sz = 0;

	return sz;
}

/************************************************************************************************
 *
************************************************************************************************/
int Func_GetArg( FUNC *fn, FUNC_DEF *func)
{
	int		rtn, c, sz;
	int		stat = 0;
	char	rec[ 512];

	LogDel( "GetArg ... cnt=[%d]", func->cnt);
	func->arg = realloc( func->arg, sizeof( char *) * (func->cnt +2));
	if( func->arg == NULL)
	{
		LogErr( "realloc error. func->arg sz=[%d]", sizeof( char *) * (func->cnt +2));
		return -1;
	}
	func->arg[ func->cnt] = NULL;

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1   :
				return 0;
			case '\"':
			case '\'':
				Func_GetString( fn, c);
				Func_StackClear( fn, __FUNCTION__);
				break;
			case '{' :
				LogCri( "syntax error. at=[%d:%d]", fn->lin, fn->col);
				return -1;
				break;
			case '/' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_CommentPlus( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
				}
				break;
			case '*' :
				if( fn->word[ fn->w_cnt -1] == '/') 
				{
					fn->w_cnt--;
					Func_Comment( fn);
				}
				else
				{
					Func_PutWord( fn, c, __FUNCTION__);
				}
				break;
			case ';' :
				return 0;
			case '(' :
				rtn = Func_PutWord( fn, c, __FUNCTION__);
				rtn = Func_GetBracketData( fn);
				if( rtn < 0) return -1;
				break;
			case ')' :
			case ',' :
				Func_Push( fn, __FUNCTION__);
				sz = Func_Pop( fn, rec, 512, __FUNCTION__);
				if( sz <= 0) return 1;
				func->arg[ func->cnt] = malloc( sz +1);
				if( func->arg[ func->cnt] == NULL)
				{
					LogErr( "malloc error. func->arg[%d] sz=[%d]", func->cnt, sz +1);
					return -1;
				}
				memcpy( func->arg[ func->cnt], rec, sz +1);
				func->cnt++;
				func->arg = realloc( func->arg, sizeof( char *) * ( func->cnt +2));
				if( func->arg == NULL)
				{
					LogErr( "realloc error. func->arg sz=[%d]", sizeof( char *) * (func->cnt +2));
					return -1;
				}
				stat = 0;
				if( c == ')') return 1;
				break;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( stat == 0) break;
				Func_PutWord( fn, c, __FUNCTION__);
				break;
			default  :
				Func_PutWord( fn, c, __FUNCTION__);
				stat = 1;
				break;
		}
	}
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Func_GetString( FUNC *fn, int ch)
{
	int		rtn, c;

	LogDel( "GetString start");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case 0   :
				return 0;
			case '\\':
				Func_GetByte( fn);
				break;
			case '\"':
			case '\'':
				if( c == ch) return 1;
				LogCri( "syntax error. at=[%d:%d]", fn->lin, fn->col);
				return -1;
			default:
				rtn = Func_PutWord( fn, c, __FUNCTION__);
				if( rtn < 0) return -1;
				break;
		}
	}
}


/******************************************************************************
 *
******************************************************************************/
int Func_GetBracketData( FUNC *fn)
{
	int		rtn, c;

	LogDel( "GetString start");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case 0   :
				return 0;
			case '(' :
				rtn = Func_PutWord( fn, c, __FUNCTION__);
				rtn = Func_GetBracketData( fn);
				break;
			case ')' :
				rtn = Func_PutWord( fn, c, __FUNCTION__);
				return 1;
			default:
				rtn = Func_PutWord( fn, c, __FUNCTION__);
				if( rtn < 0) return -1;
				break;
		}
	}
}

/******************************************************************************
 *
******************************************************************************/
int Func_GetByte( FUNC *fn)
{
	int					rtn;
	int					c;
	int					o = 0, x = 0;

	LogDel( "GetChar start");


	c = Func_GetChar( fn, "get char");
	LogDel( "111");
	switch( c)
	{
		case 0x00:  /* null */
			return 1;
		case 'a' : break; /* bell	*/
		case 'b' : break; /* back space */
		case 'f' : break; /* form feed */
		case 'n' : break; /* line feed */
		case 'r' : break; /* carriage return */
		case 't' : break; /* horizontal tab */
		case 'v' : break; /* veritical tab */
		case '\\': break; /* back slash */
		case '\'': break; /* single quote */
		case '\"': break; /* double quote */
		case '?' : break; /* question mark */
		case '$' : break; /* question mark */
		case 'x' :
			break;
		case 'B' :
			break;
		default:
			break;
	}

	return 1;
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int Func_Comment( FUNC *fn)
{
	int		rtn, c, p;

	LogDel( "Func_Comment ... ");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( c)
		{
			case -1  :
				return -1;
			case '/' :
				if( p == '*') return 1;
				break;
			case '*' :
				p = '*';
				break;
			default  :
				p = 0;
				break;
		}
		if( fn->cmt == NULL)
		{
			fn->cmt = malloc( fn->c_sz +2);
		}
		else
		{
			fn->cmt = realloc( fn->cmt, fn->c_sz +2);
		}
		fn->cmt[ fn->c_sz++] = c;
		fn->cmt[ fn->c_sz  ] = 0;
	}
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int Func_CommentPlus( FUNC *fn)
{
	int		rtn, c, p;

	LogDel( "Func_CommentPlus ... ");

	while( 1)
	{
		c = Func_GetChar( fn, __FUNCTION__);
		switch( fn->rec[ fn->col])
		{
			case -1  :
				return -1;
			case '\n':
				return 1;
			default  :
				break;
		}
	}
}

int Func_Print( FUNC *fn)
{
	int				pos = 0;
	char			*ptr;
	FUNC_DEF		*func;

/*
	printf( "-------------------------------------------------------------------------------\n");
	func = Dll_GetFirstPtr( fn->func);
	while( func != NULL)
	{
		printf( "type            = [%s]\n", func->type);
		printf( "name            = [%s]\n", func->name);
		printf( "cnt             = [%d]\n", func->cnt);
		pos = 0;
		while( pos < func->cnt)
		{
			printf( "    arg[%2d]     = [%s]\n", pos, func->arg[ pos]);
			pos++;
		}
		func = Dll_GetNextPtr( fn->func);
	}
	printf( "-------------------------------------------------------------------------------\n");
*/

	func = Dll_GetFirstPtr( fn->func);
	while( func != NULL)
	{
		if( func->type == NULL)	printf( "%-11s ", "int");
		else					printf( "%-11s ", func->type);

		printf( "%s( ", func->name);

		pos = 0;
		while( pos < func->cnt)
		{
			printf( "%s", func->arg[ pos]);
			pos++;
			if( pos < func->cnt) printf( ", ");
		}
		printf( ");\n");

		func = Dll_GetNextPtr( fn->func);
	}
	
	return 1;
}

int Func_PrintFile( FUNC *fn, FILE *fp)
{
	int				pos = 0;
	int				col = 0;
	char			*ptr;
	FUNC_DEF		*func;

	fprintf( fp, "/***** Module : %s *****/\n", fn->f_name);

	func = Dll_GetFirstPtr( fn->func);
	while( func != NULL)
	{
		if( func->type == NULL)	col += fprintf( fp, "%-11s ", "int");
		else					col += fprintf( fp, "%-11s ", func->type);

		col += fprintf( fp, "%s(", func->name);

		pos = 0;
		while( pos < func->cnt)
		{
			col += fprintf( fp, " %s", func->arg[ pos]);
			pos++;
			if( pos < func->cnt) col += fprintf( fp, ",");
		}
		col += fprintf( fp, ");");

		if( func->cmt != NULL)
		{
			for( col; col <= FUNC_CMT_POS; col++) fprintf( fp, " ");
			fprintf( fp, "/* %s */", func->cmt);
		}

		fprintf( fp, "\n");
		col = 0;

		func = Dll_GetNextPtr( fn->func);
	}
	fprintf( fp, "\n");
	
	return 1;
}



