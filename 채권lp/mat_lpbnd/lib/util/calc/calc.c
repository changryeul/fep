#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "calc.h"

CALC_OPER	CalcOper[ 64] =
{
	{	"+", 		300,	300,		Calc_ActPlus},
	{	"-", 		300,	300,		Calc_ActMius},
	{	"*", 		400,	400,		Calc_ActMult},
	{	"/", 		400,	400,		Calc_ActDevi},
	{	"(", 		500,	200,		Calc_ActOpen},
	{	")", 		100,	500,		Calc_ActClos},
	{	"=", 		0,		300,		Calc_ActEqul},
	{	"", 		-1,		-1,			NULL		}
};

CALC *Calc_Open()
{
	CALC	*cal;

	cal = malloc( sizeof( CALC));
	if( cal == NULL)
	{
		LogErr( "malloc error. cal sz=[%d]", sizeof( CALC));
		return NULL;
	}
	memset( cal, 0, sizeof( CALC));

	return cal;
}

int Calc_Close( CALC *cal)
{
	int		cnt ;

	if( cal->ts_cnt > 0)
	{
		cnt  = cal->ts_cnt -1;
		while( cnt)
		{
			if( cal->ts[ cnt] != NULL) free( cal->ts[ cnt]);
			cnt--;
		}
		if( cal->ts != NULL) free( cal->ts);
	}

	if( cal->os_cnt > 0)
	{
		cnt  = cal->os_cnt -1;
		while( cnt)
		{
			if( cal->os[ cnt] != NULL) free( cal->os[ cnt]);
			cnt--;
		}
		if( cal->os != NULL) free( cal->os);
	}

	if( cal->input != NULL)	free( cal->input);

	free( cal);
}

int Calc_Process( CALC *cal, char *rec)
{
	int		sz, rtn;
	char	res[ 512];

	sz = strlen( rec) -1;

	LogDel( "i_sz=[%d] sz=[%d] i_cnt=[%d]", cal->i_sz, sz, cal->i_cnt);

	rec[ sz] = 0;

	cal->input = realloc( cal->input, cal->i_sz + sz +1);
	if( cal->input == NULL)
	{
		LogErr( "realloc error. input sz=[%d]", sz +1);
		return -1;
	}
	memcpy( &cal->input[ cal->i_sz], rec, sz +1);
	cal->i_sz += sz;

	rtn = Calc_Start( cal);

	LogDel( "Process end ... rtn = [%d]", rtn);

	if( rtn == 0)
	{
		Calc_GetOper( cal, res, 512, __FUNCTION__);
		Calc_GetToken( cal, res, 512, __FUNCTION__);
		/*
		printf( "%s", cal->input);
		*/
		printf( "%s\n", res);
	}
	/*
	Calc_Print( cal);
	*/

	return rtn;
}

int Calc_GetChar( CALC *cal, const char *call)
{
	int		rtn;

	if( cal->i_cnt >= cal->i_sz) return -1;

	rtn = cal->input[ cal->i_cnt];
	if( rtn == 0) return -1;

	cal->i_cnt++;
	LogDel( "getc %-20s data = [%c][%3d][%02x]", call, rtn, rtn, rtn);

	return rtn;
}

int Calc_PutWord( CALC *cal, int c, const char *call)
{
	cal->word[ cal->w_cnt] = c;
	cal->w_cnt++;
	cal->word[ cal->w_cnt] = 0;
	LogDel( "putw %-20s cnt = [%d] c = [%c][%3d][%02x] word = [%s]", call, cal->w_cnt, c, c, c, cal->word);
	if( cal->w_cnt >= 512) return -1;
	return 1;
}

int Calc_GetWord( CALC *cal, char *data, int sz, const char *call)
{
	int		cp_sz;

	if( cal->w_cnt <= 0) 
	{
		LogDel( "getw %-20s no data", call);
		data[ 0] = 0;
		return 0;
	}

	cal->word[ cal->w_cnt] = 0;

	cp_sz = cal->w_cnt;
	if( cp_sz >= sz) 
	{
		LogCri( "buffer too small. cp_sz=[%d] sz=[%d]", cp_sz, sz);
		return -1;
	}
	memcpy( data, cal->word, cp_sz +1);
	LogDel( "getw %-20s data = [%d:%s]", call, cp_sz, data);
	cal->w_cnt = 0;
	cal->word[ cal->w_cnt] = 0;

	return cp_sz;
}

int Calc_PutToken( CALC *cal, char *data, int sz, const char *call)
{
	int		pos;
	char	*ptr;

	LogDel( "putt %-20s data=[%s] sz=[%d]", call, data, sz);

	if( sz <= 0)
	{
		LogDel( "putw %-20s no data", call);
		return 0;
	}

	cal->ts = realloc( cal->ts, sizeof( char *) * ( cal->ts_cnt +2));
	if( cal->ts == NULL)
	{
		LogErr( "realloc error. ws sz = [%d]", sizeof( char *) * ( cal->ts_cnt +2));
		return -1;
	}

	ptr = malloc( sz +1);
	if( ptr == NULL)
	{
		LogErr( "malloc error. token sz=[%d]", cal->w_cnt +1);
		return -1;
	}

	memcpy( ptr, data, sz +1);
	ptr[ sz] = 0;
	cal->ts[ cal->ts_cnt] = ptr;


	pos = cal->ts_cnt;
	while( pos >= 0)
	{
		LogDel( "putt                      ts[%2d] = [%s]", pos, cal->ts[ pos]);
		pos--;
	}

	cal->ts_cnt++;

	return cal->ts_cnt;
}

int Calc_GetToken( CALC *cal, char *rec, int sz, const char *call)
{
	int		cp_sz;
	char	*ptr;

	if( cal->ts_cnt <= 0) 
	{
		LogDel( "gett %-20s no data.", call);
		rec[ 0] = 0;
		return 0;
	}

	cal->ts_cnt--;
	ptr = cal->ts[ cal->ts_cnt];

	cp_sz = strlen( ptr);
	if( cp_sz >= sz)
	{
		LogCri( "operland too small. call=[%s] cp_sz=[%d] sz=[%d]", call, cp_sz, sz);
		return -1;
	}
	memcpy( rec, ptr, cp_sz +1);

	free( ptr);
	cal->ts[ cal->ts_cnt] = NULL;

	cal->ts = realloc( cal->ts, sizeof( char *) * ( cal->ts_cnt +2));
	if( cal->ts == NULL)
	{
		LogErr( "realloc error. ws sz=[%d]", sizeof( char *) * ( cal->ts_cnt +2));
		return -1;
	}
	LogDel( "gett %-20s cnt=[%d] data=[%s]", call, cal->ts_cnt, rec);

	return cp_sz;
}

int Calc_PutOper( CALC *cal, char *oper, int sz, const char *call)
{
	int		pos;
	char	*ptr;

	cal->os = realloc( cal->os, sizeof( char *) * ( cal->os_cnt +2));
	if( cal->os == NULL)
	{
		LogErr( "realloc error. os sz=[%d]", sizeof( char *) * ( cal->os_cnt +2));
		return -1;
	}

	ptr = malloc( sz +1);
	if( ptr == NULL)
	{
		LogErr( "malloc error. oper sz=[%d]", sizeof( int));
		return -1;
	}

	memcpy( ptr, oper, sz +1);
	cal->os[ cal->os_cnt] = ptr;

	LogDel( "puto %-20s cnt=[%d] data=[%s] ptr=[%p]", call, cal->os_cnt, cal->os[ cal->os_cnt], cal->os[ cal->os_cnt]);

	pos = cal->os_cnt;
	while( pos >= 0)
	{
		LogDel( "puto                      os[%2d] = [%s]", pos, cal->os[ pos]);
		pos--;
	}

	cal->os_cnt++;
	cal->os[ cal->os_cnt] = NULL;
	return cal->os_cnt;
}

int Calc_GetOper( CALC *cal, char *rec, int sz, const char *call)
{
	int		cp_sz;
	char	*ptr;

	if( cal->os_cnt <= 0) 
	{
		LogDel( "geto %-20s no data.", call);
		rec[ 0] = 0;
		return 0;
	}

	LogDel( "geto %-20s cnt=[%d] ptr=[%p]", call, cal->os_cnt, cal->os[ cal->os_cnt]);

	cal->os_cnt--;

	ptr = cal->os[ cal->os_cnt];
	cp_sz = strlen( ptr);
	if( cp_sz >= sz)
	{
		LogCri( "operland too small. call=[%s] cp_sz=[%d] sz=[%d]", call, cp_sz, sz);
		return -1;
	}
	memcpy( rec, ptr, cp_sz);

	free( ptr);
	cal->os[ cal->os_cnt] = NULL;

	cal->os = realloc( cal->os, sizeof( char *) * ( cal->os_cnt +2));
	if( cal->os == NULL)
	{
		LogErr( "realloc error. os sz=[%d]", sizeof( char *) * ( cal->os_cnt +2));
		return -1;
	}
	LogDel( "geto %-20s cnt=[%d] data=[%s]", call, cal->os_cnt, rec);

	return cp_sz;
}

int Calc_CheckOper( CALC *cal, char *oper1, char *oper2)
{
	int			pos;
	CALC_OPER	*op, *op1, *op2;

	op = &CalcOper[ 0];
	while( op->s_prio >= 0)
	{
		if( !strncmp( oper1, op->oper, strlen( op->oper) +1))	op1 = op;
		if( !strncmp( oper2, op->oper, strlen( op->oper) +1))	op2 = op;
		op++;
	}
	return op1->s_prio - op2->e_prio;
}

int Calc_CalcOper( CALC *cal, char *oper)
{
	int			rtn;
	CALC_OPER	*optr;

	LogDel( "Calc_CalcOper ... oper=[%s]", oper);

	optr = &CalcOper[ 0];
	while( optr->s_prio >= 0)
	{
		if( !strncmp( oper, optr->oper, strlen( optr->oper) +1)) break;
		optr++;
	}
	if( optr->s_prio < 0) return -1;

	LogDel( "call oper=[%s] ptr=[%p]", oper, optr->act);
	rtn = optr->act( cal, oper);

	return rtn;
}

int Calc_ActPlus( CALC *cal, char *oper)
{
	int		rtn, sz;
	char	d[ 512], d1[ 512], d2[ 512];
	int		i, i1, i2;

	Calc_GetToken( cal, d1, 512, __FUNCTION__);
	Calc_GetToken( cal, d2, 512, __FUNCTION__);

	i1 = atoi( d1);
	i2 = atoi( d2);

	i = i2 + i1;

	sz = sprintf( d, "%d", i);

	rtn = Calc_PutToken( cal, d, sz, __FUNCTION__);

	return rtn;
}

int Calc_ActMius( CALC *cal, char *oper)
{
	int		rtn, sz;
	char	d[ 512], d1[ 512], d2[ 512];
	int		i, i1, i2;

	Calc_GetToken( cal, d1, 512, __FUNCTION__);
	Calc_GetToken( cal, d2, 512, __FUNCTION__);

	i1 = atoi( d1);
	i2 = atoi( d2);

	i = i2 - i1;

	sz = sprintf( d, "%d", i);

	rtn = Calc_PutToken( cal, d, sz, __FUNCTION__);

	return rtn;
}

int Calc_ActMult( CALC *cal, char *oper)
{
	int		rtn, sz;
	char	d[ 512], d1[ 512], d2[ 512];
	int		i, i1, i2;

	Calc_GetToken( cal, d1, 512, __FUNCTION__);
	Calc_GetToken( cal, d2, 512, __FUNCTION__);

	i1 = atoi( d1);
	i2 = atoi( d2);

	i = i2 * i1;

	sz = sprintf( d, "%d", i);

	rtn = Calc_PutToken( cal, d, sz, __FUNCTION__);

	return rtn;
}

int Calc_ActDevi( CALC *cal, char *oper)
{
	int		rtn, sz;
	char	d[ 512], d1[ 512], d2[ 512];
	int		i, i1, i2;

	Calc_GetToken( cal, d1, 512, __FUNCTION__);
	Calc_GetToken( cal, d2, 512, __FUNCTION__);

	i1 = atoi( d1);
	i2 = atoi( d2);

	i = i2 / i1;

	sz = sprintf( d, "%d", i);

	rtn = Calc_PutToken( cal, d, sz, __FUNCTION__);

	return rtn;
}

int Calc_ActOpen( CALC *cal, char *oper)
{
	int		rtn, sz;
	char	d[ 512], d1[ 512], d2[ 512];
	int		i, i1, i2;

	return rtn;
}

int Calc_ActClos( CALC *cal, char *oper)
{
}

int Calc_ActEqul( CALC *cal, char *oper)
{
}

int Calc_Start( CALC *cal)
{
	int		rtn, c, sz;
	char	rec[ 512];
	int		stat = 0;

	while( 1)
	{
		c = Calc_GetChar( cal, __FUNCTION__);
		switch( c)
		{
			case -1:
				return 1;
				return 0;
			case '\n' : /* line feed */
			case '\r' : /* carriage return */
			case '\f' : /* form feed */
			case '\t' : /* horizontal tab */
			case '\v' : /* veritical tab */
			case ' '  : /* space */
				sz = Calc_GetWord( cal, rec, 512, __FUNCTION__);
				Calc_PutToken( cal, rec, sz, __FUNCTION__);
				stat = 0;
				break;
			case '('  :
			case ')'  :
			case '+'  :
			case '-'  :
			case '*'  :
			case '/'  :
			case '='  :
				sz = Calc_GetWord( cal, rec, 512, __FUNCTION__);
				Calc_PutToken( cal, rec, sz, __FUNCTION__);
				Calc_PutWord( cal, c, __FUNCTION__);
				Calc_ProcOper( cal);
				if( c == '=')	return 0;
				break;
			default:
				Calc_PutWord( cal, c, __FUNCTION__);
				stat++;
				break;
		}
	}
}

int Calc_ProcOper( CALC *cal)
{
	int		rtn, sz;
	char	stack[ 512] = "";
	int		s_sz;
	char	oper[ 512] = "";
	int		o_sz;

	LogDel( "Calc_ProcOper ... ");

	o_sz = Calc_GetWord( cal, oper, 512, __FUNCTION__);

	while( 1)
	{
		s_sz = Calc_GetOper( cal, stack, 512, __FUNCTION__);
		if( s_sz == 0)
		{
			Calc_PutOper( cal, oper, o_sz, __FUNCTION__);
			return 1;
		}

		rtn = Calc_CheckOper( cal, oper, stack);
		LogDel( "Calc_CheckOper oper = [%s] stack = [%s] rtn=[%d]", oper, stack, rtn);
		if( rtn > 0)
		{
			Calc_PutOper( cal, stack, s_sz, __FUNCTION__);
			Calc_PutOper( cal, oper, o_sz, __FUNCTION__);
			return 1;
		}

		rtn = Calc_CalcOper( cal, stack);
	}

	return 1;
}

int Calc_Print( CALC *cal)
{
	int		pos;

	printf( "oper  stack count = [%d]\n", cal->os_cnt);
	pos = cal->os_cnt -1;
	while( pos >= 0)
	{
		printf( "oper  stack %d = [%s]\n", pos, cal->os[ pos]);
		pos--;
	}

	printf( "token stack count = [%d]\n", cal->ts_cnt);
	pos = cal->ts_cnt -1;
	while( pos >= 0)
	{
		printf( "token stack %d = [%s]\n", pos, cal->ts[ pos]);
		pos--;
	}
	printf( "--------------------------------------\n");

	return 1;
}











