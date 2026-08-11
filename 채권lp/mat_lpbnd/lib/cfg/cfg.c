#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "cfg.h"

#if 0
void *Malloc( size_t sz, int line);
void *Realloc( void *ptr, size_t sz, int line);
#endif

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
char*		Cfg_GetEnvPtr( CFG *cfg, char *name);
void*		Cfg_GetPtr( CFG *cfg, char *name);
void* 		Cfg_GetListPtr( CFG *cfg);

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
CFG *Cfg_Open( char *f_name)
{
	int		rtn;
	int		sz;
	CFG		*cfg;

	LogDel( "Cfg_Open start. f_name=[%s]", f_name);

	cfg = ( CFG *)malloc( sizeof( CFG));
	if( cfg == NULL)
	{
		LogErr( "Malloc error. size=[%d]", sizeof( CFG));
		goto error_1;
	}
	memset( cfg, 0, sizeof( CFG));

	
	sz = strlen( f_name) +1;
	cfg->f_name = ( char *)malloc( sz);
	if( cfg->f_name == NULL)
	{
		LogErr( "Malloc error. cfg->f_name size=[%d]", sz);
		goto error_2;
	}
	memcpy( cfg->f_name, f_name, sz);

	cfg->fp = fopen( cfg->f_name, "r");
	if( cfg->fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", f_name);
		goto error_3;
	}

	rtn = Cfg_Load( cfg);
	if( rtn < 0)
	{
		LogCri( "Cfg_Load error.");
		goto error_4;
	}

	fclose( cfg->fp);
	cfg->fp = NULL;

	return cfg;

	error_4:
		fclose( cfg->fp);
	error_3:
		free( cfg->f_name);
	error_2:
		free( cfg);
	error_1:
		return NULL;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Close( CFG *cfg)
{
	int				cnt = 0;

	if( cfg == NULL) return -1;

	Cfg_MemberClose( cfg, cfg->member);;

	LogDel( "Cfg_MemberClose success");

	if( cfg->stack != NULL)
	{
		while( cnt < cfg->s_cnt)
		{
			if( cfg->stack[ cnt] != NULL) free( cfg->stack[ cnt]);
			cnt++;
		}
		free( cfg->stack);
	}

	if( cfg->fp != NULL) fclose( cfg->fp);
	if( cfg->f_name != NULL) free( cfg->f_name);
	free( cfg);

	LogDel( "Cfg_Close return 1");
	return 1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_MemberClose( CFG *cfg, CFG_MEMBER *ptr)
{
	CFG_MEMBER		*member;

	switch( ptr->type)
	{
		case CFG_TYPE_VALUE:
			free( ptr->value);
			ptr->value = NULL;
			break;
		case CFG_TYPE_DLL:
			member = ( CFG_MEMBER *)Dll_GetFirstPtr( ptr->value);
			while( member != NULL)
			{
				LogDel( "call Cfg_MemberClose ... cfg=[%p] ptr=[%p] member=[%p]", cfg, ptr, member);
				Cfg_MemberClose( cfg, member);
				member = ( CFG_MEMBER *)Dll_GetNextPtr( ptr->value);
			}
			Dll_Close( ( DLL *)ptr->value);
			break;
		default:
			break;

	}
	LogDel( "ptr      =[%p]", ptr);
	LogDel( "ptr->name=[%s][%p]", ptr->name, ptr->name);
	if( ptr->name != NULL) free( ptr->name);
	ptr->name = NULL;
	free( ptr);
	ptr = NULL;
	return 1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetLine( CFG *cfg, const char *call)
{
	char	*ptr;

	ptr = fgets( cfg->rec, CFG_FILE_BUF_SZ, cfg->fp);
	if( ptr == NULL) return -1;

	cfg->lin++;
	LogDel( "rec(%s)=[%s]", call, cfg->rec);

	cfg->col = 0;
	return 1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetFile( CFG *cfg, const char *call)
{
	int		rtn;
	int		c;

	if( cfg->rec[ cfg->col] == 0)
	{
		rtn = Cfg_GetLine( cfg, call);
		if( rtn < 0)
		{
			LogDel( "EOF. fp=[%p] name=[%s]", cfg->fp, cfg->f_name);
			return 0;
		}
	}

	c = cfg->rec[ cfg->col];
	LogDel( "%-10s getc r_pos[%3d] w_pos[%4d] char[%c][%03d][%02x]", call, cfg->col, cfg->w_pos, c, c, c);
	cfg->col++;

	return c;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_PutFile( CFG *cfg, int c)
{
	cfg->col--;
	cfg->rec[ cfg->col] = c;

	return cfg->col;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_PutChar( CFG *cfg, int c, const char *call)
{
	LogDel( "%-10s putc r_pos[%3d] w_pos[%4d] char[%c][%03d][%02x]", 
			call, cfg->col, cfg->w_pos, c, c, c);
	cfg->word[ cfg->w_pos] = c;
	cfg->w_pos++;
	if( cfg->w_pos >= CFG_WORD_BUF_SZ) return -1;

	return cfg->w_pos;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Push( CFG *cfg, const char *call)
{
	if( cfg->w_pos <= 0) return 0;

	cfg->word[ cfg->w_pos] = 0;
	LogDel( "%-10s push pos[%3d] word[%s] sz[%3d]", call, cfg->s_cnt, cfg->word, cfg->w_pos);

	cfg->stack = ( char **)realloc( cfg->stack, (sizeof( char *) * (cfg->s_cnt +2)));
	if( cfg->stack == NULL)
	{
		LogErr( "Realloc error. cfg->stack size=[%d]", sizeof( char *) * (cfg->s_cnt +2));
		return -1;
	}

	cfg->stack[ cfg->s_cnt] = ( char *)malloc( cfg->w_pos +1);
	if( cfg->stack[ cfg->s_cnt] == NULL)
	{
		LogErr( "Malloc error. cfg->stack[%d] size=[%d]", cfg->s_cnt, sizeof( char *) * (cfg->s_cnt +2));
		return -1;
	}
	LogDel( "stack alloc. cnt=[%d] ptr=[%p] data=[%s]", cfg->s_cnt, cfg->stack[ cfg->s_cnt], cfg->word);

	memcpy( cfg->stack[ cfg->s_cnt], cfg->word, cfg->w_pos +1);
	cfg->s_cnt++;

	cfg->stack[ cfg->s_cnt] = NULL;

	cfg->w_pos = 0;
	return 1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
char *Cfg_Pop( CFG *cfg, const char *call)
{
	static char	*ptr;

	if( cfg->s_cnt < 1) 
	{
		LogDel( "count error. call=[%s] cfg->s_cnt=[%d]", call, cfg->s_cnt);
		return 0;
	}

	cfg->s_cnt--;
	ptr = cfg->stack[ cfg->s_cnt];

	cfg->stack = ( char **)realloc( cfg->stack, sizeof( char *) * (cfg->s_cnt +2));
	if( cfg->stack == NULL)
	{
		LogErr( "Realloc error. cfg->stack size=[%d]", sizeof( char *) * (cfg->s_cnt +2));
		return NULL;
	}
	cfg->stack[ cfg->s_cnt] = NULL;

	LogDel( "%-10s pop  pos[%3d] word[%s]", call, cfg->s_cnt, ptr);

	return ptr;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Load( CFG *cfg)
{
	int		rtn;

	rtn = Cfg_Start( cfg);

	if( LogDelFlag) 	Cfg_Print( cfg);

	return rtn;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Start( CFG *cfg)
{
	int			rtn;
	int			c;
	int			stat = 0;
	CFG_MEMBER	*member;

	member = ( CFG_MEMBER *)malloc( sizeof( CFG_MEMBER));
	if( member == NULL)
	{
		LogErr( "Malloc error. member size=[%d]", sizeof( CFG_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( CFG_MEMBER));

	member->name  = NULL;
	/*
	member->name  = cfg->f_name;
	*/
	member->type  = CFG_TYPE_DLL;
	member->value = Dll_Open( 0);
	if( member->value == NULL)
	{
		LogCri( "Dll_Open error.");
		goto error_2;
		
	}
	cfg->member = member;

	LogDel( "start member      = [%p]", member);
	LogDel( "start member name = [%s]", member->name);
	LogDel( "start member typr = [%d]", member->type);
	LogDel( "start member dll  = [%p]", member->value);

	while( 1)
	{
		c = Cfg_GetFile( cfg, "start");
		switch( c)
		{
			case 0x00:	/* null */
				return 1;
			case '#' :
				rtn = Cfg_GetComment( cfg);
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
			case 0x09:  /* horizontal tab */
			case 0x20:  /* space */
				rtn = Cfg_Push( cfg, "start");
				if( rtn > 0) stat++;
				break;
			case '=' :
				if( stat <= 0)
				{
					LogCri( "no operland found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetName( cfg, member);
				if( rtn < 0)
				{
					LogCri( "Cfg_GetValue error.");
					return -1;
				}
				stat = 0;
				break;
			case '{' :
				if( stat <= 0)
				{
					LogCri( "no operland. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetSection( cfg, member);
				stat = 0;
				break;
			case '[' :
				/*
				if( stat > 0)
				{
					CDC
					LogCri( "syntax error. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				*/
				rtn = Cfg_GetWinSection( cfg, member);
				stat = 1;
				break;
			default:
				if( stat > 0)
				{
					LogCri( "too many operland found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_PutChar( cfg, c, "start");
				if( rtn < 0) return -1;
				break;
		}
	}
	return 1;

#if 0
	error_3:
		Dll_Close( ( DLL *)member->value);
#endif
	error_2:
		free( member);
	error_1:
		return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetComment( CFG *cfg)
{
	int		c;

	while( 1)
	{
		c = Cfg_GetFile( cfg, "comment");
		switch( c)
		{
			case 0x00:	/* null */
				return 1;
			case 0x0D:	/* carriage return */
			case 0x0A:	/* line feed */
				Cfg_PutFile( cfg, c);
				return 1;
			default:
				break;
		}
	}
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetName( CFG *cfg, CFG_MEMBER *arg_ptr)
{
	int			rtn;
	CFG_MEMBER	*member;

	member = ( CFG_MEMBER *)malloc( sizeof( CFG_MEMBER));
	if( member == NULL)
	{
		LogErr( "Malloc error. member size=[%d]", sizeof( CFG_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( CFG_MEMBER));

	member->name = Cfg_Pop( cfg, "name");
	if( member->name == NULL)
	{
		LogCri( "Cfg_Pop error. member->name cfg=[%p]", cfg);
		goto error_2;
	}
	LogDel( "member->name          = [%s][%p]", member->name, member->name);

	member->type = CFG_TYPE_VALUE;
	LogDel( "member->type          = [%d]", member->type);

	rtn = Cfg_GetValue( cfg, member);
	if( rtn < 0)
	{
		LogCri( "Cfg_GetValue error.");
		LogCri( "global member        = [%p]", member);
		LogCri( "global member name   = [%s]", member->name);
		LogCri( "global member typr   = [%d]", member->type);
		LogCri( "global member value  = [%s]", member->value);
		goto error_2;
	}

	Dll_Add( ( DLL *)arg_ptr->value, member, sizeof( CFG_MEMBER));

	LogDel( "global member        = [%p]", member);
	LogDel( "global member name   = [%s]", member->name);
	LogDel( "global member typr   = [%d]", member->type);
	LogDel( "global member value  = [%s]", member->value);

	if( LogDelFlag)	Cfg_Print( cfg);

	return 1;

	error_2:
		free( member);
	error_1:
		return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetListName( CFG *cfg, CFG_MEMBER *arg_ptr)
{
	CFG_MEMBER	*member;

	member = ( CFG_MEMBER *)malloc( sizeof( CFG_MEMBER));
	if( member == NULL)
	{
		LogErr( "Malloc error. member size=[%d]", sizeof( CFG_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( CFG_MEMBER));

	member->name = Cfg_Pop( cfg, "name");
	if( member->name == NULL)
	{
		LogCri( "Cfg_Pop error. member->name cfg=[%p]", cfg);
		goto error_2;
	}
	LogDel( "member->name          = [%s][%p]", member->name, member->name);

	member->type = CFG_TYPE_LIST;
	LogDel( "member->type          = [%d]", member->type);

	member->value = NULL;

	Dll_Add( ( DLL *)arg_ptr->value, member, sizeof( CFG_MEMBER));

	LogDel( "global list member        = [%p]", member);
	LogDel( "global list member name   = [%s]", member->name);
	LogDel( "global list member typr   = [%d]", member->type);
	LogDel( "global list member value  = [%s]", member->value);

	if( LogDelFlag)	Cfg_Print( cfg);

	return 1;

	error_2:
		free( member);
	error_1:
		return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetSection( CFG *cfg, CFG_MEMBER *arg_ptr)
{
	int			rtn, c, stat = 0;
	CFG_MEMBER	*member;

	member = ( CFG_MEMBER *)malloc( sizeof( CFG_MEMBER));
	if( member == NULL)
	{
		LogErr( "Malloc error. member size=[%d]", sizeof( CFG_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( CFG_MEMBER));

	member->name = Cfg_Pop( cfg, "name");
	if( member->name == NULL)
	{
		LogCri( "Cfg_Pop error. member->name cfg=[%p]", cfg);
		goto error_2;
	}
	LogDel( "member->name          = [%s][%p]", member->name, member->name);

	member->type = CFG_TYPE_DLL;
	LogDel( "member->type          = [%d]", member->type);

	member->value = Dll_Open( 0);
	if( member->value == NULL)
	{
		LogCri( "GetSection Dll_Open error.");
		return -1;
	}

	Dll_Add( ( DLL *)arg_ptr->value, member, sizeof( CFG_MEMBER));

	while( 1)
	{
		c = Cfg_GetFile( cfg, "section");
		switch( c)
		{
			case 0x00:	/* null */
				return 1;
			case '#' :
				rtn = Cfg_GetComment( cfg);
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
			case 0x09:  /* horizontal tab */
			case 0x20:  /* space */
				rtn = Cfg_Push( cfg, "section");
				if( rtn > 0) stat = 2;
				break;
			case '=' :
				rtn = Cfg_Push( cfg, "section");
				if( rtn > 0) stat = 2;
				if( stat <= 0)
				{
					LogCri( "no operland found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetName( cfg, member);
				if( rtn < 0)
				{
					LogCri( "Cfg_GetValue error.");
					return -1;
				}
				stat = 0;
				break;
			case '\"':
			case '\'':
				rtn = Cfg_GetString( cfg, c);
				if( rtn < 0) return rtn;
				Cfg_Push( cfg, "section");
				LogDel( "call Cfg_GetListName at Cfg_GetSection");
				rtn = Cfg_GetListName( cfg, member);
				break;
			case '{' :
				rtn = Cfg_Push( cfg, "section");
				if( rtn > 0) stat = 2;
				if( stat <= 0)
				{
					LogCri( "no operland. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetSection( cfg, member);
				stat = 0;
				break;
			case '}' :
				if( stat >= 1)
				{
					rtn = Cfg_GetListName( cfg, member);
				}
				return 1;
			default:
				if( stat > 1)
				{
					rtn = Cfg_GetListName( cfg, member);
				}
				rtn = Cfg_PutChar( cfg, c, "section");
				if( rtn < 0) return -1;
				stat = 1;
				break;
		}
	}
	return 1;

	error_2:
		free( member);
	error_1:
		return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetWinSection( CFG *cfg, CFG_MEMBER *arg_ptr)
{
	int			rtn, c, stat = 0;
	CFG_MEMBER	*member;

	member = ( CFG_MEMBER *)malloc( sizeof( CFG_MEMBER));
	if( member == NULL)
	{
		LogErr( "Malloc error. member size=[%d]", sizeof( CFG_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( CFG_MEMBER));

	member->type = CFG_TYPE_DLL;
	LogDel( "member->type          = [%d]", member->type);

	member->value = Dll_Open( 0);
	if( member->value == NULL)
	{
		LogCri( "GetSection Dll_Open error.");
		return -1;
	}

	Dll_Add( ( DLL *)arg_ptr->value, member, sizeof( CFG_MEMBER));

	rtn = Cfg_GetWinName( cfg, member);
	member->name = Cfg_Pop( cfg, "name");
	if( member->name == NULL)
	{
		LogCri( "Cfg_Pop error. member->name cfg=[%p]", cfg);
		goto error_2;
	}
	LogDel( "member->name          = [%s][%p]", member->name, member->name);

	while( 1)
	{
		c = Cfg_GetFile( cfg, "win_sec");
		switch( c)
		{
			case 0x00:	/* null */
				return 1;
			case '#' :
				rtn = Cfg_GetComment( cfg);
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
			case 0x09:  /* horizontal tab */
			case 0x20:  /* space */
				rtn = Cfg_Push( cfg, "win_sec");
				if( rtn > 0) stat++;
				break;
			case '=' :
				rtn = Cfg_Push( cfg, "win_sec");
				if( rtn > 0) stat++;
				if( stat <= 0)
				{
					LogCri( "no operland found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetName( cfg, member);
				if( rtn < 0)
				{
					LogCri( "Cfg_GetValue error.");
					return -1;
				}
				stat = 0;
				break;
			case '[' :
			case '{' :
				rtn = Cfg_Push( cfg, "win_sec");
				if( rtn > 0) stat++;
				Cfg_PutFile( cfg, c);
				return 1;
			case '}' :
				return 1;
			default:
				if( stat > 1)
				{
					LogCri( "too many operland found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_PutChar( cfg, c, "win_sec");
				if( rtn < 0) return -1;
				stat = 1;
				break;
		}
	}
	return 1;

	error_2:
		free( member);
	error_1:
		return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetWinName( CFG *cfg, CFG_MEMBER *member)
{
	int		rtn, c;

	LogDel( "GetString start");

	while( 1)
	{
		c = Cfg_GetFile( cfg, "win_name");
		switch( c)
		{
			case 0   :
				return 0;
			case ']' :
				Cfg_Push( cfg, "win_name");
				return 1;
			default:
				rtn = Cfg_PutChar( cfg, c, "win_name");
				if( rtn < 0) return -1;
				break;
		}
	}
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetValue( CFG *cfg, CFG_MEMBER *member)
{
	int			rtn, c, stat = 0;

	LogDel( "value memver                = [%p]", member);
	LogDel( "value member->type          = [%d]", member->type);

	while( 1)
	{
		c = Cfg_GetFile( cfg, "value");
		switch( c)
		{
			case 0x00:	/* null */
				return 1;
			case '#' :
				rtn = Cfg_GetComment( cfg);
			case '}' :
				Cfg_PutFile( cfg, c);
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
				rtn = Cfg_Push( cfg, "value");
				if( rtn > 0) stat = 2;
				member->value = Cfg_Pop( cfg, "value");
				if( member->value == NULL)
				{
					LogCri( "no value found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					LogCri( "Cfg_Pop error. member->value cfg=[%p]", cfg);
					return -1;
				}
				LogDel( "member->value         = [%s]", member->value);
				return 1;
			case 0x09:  /* horizontal tab */
			case '\v':  /* veritical tab */
			case 0x20:  /* space */
				if( stat == 0) continue;
#if 0
				/* 20191106 여러 value 입력 가능 
				else if( stat == 1)
				{
					rtn = Cfg_Push( cfg, "value");
					if( rtn > 0) stat = 2;
				}
				else if( stat > 2)
				{
					LogCri( "too many value. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				*/
#else
				rtn = Cfg_PutChar( cfg, c, "value");
				if( rtn < 0) return -1;
				break;
#endif

				member->value = Cfg_Pop( cfg, "value");
				if( member->value == NULL)
				{
					LogCri( "no value found. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					LogCri( "Cfg_Pop error. member->value cfg=[%p]", cfg);
					return -1;
				}
				LogDel( "member->value         = [%s]", member->value);
				return 1;
			case '=' :
				LogCri( "syntax error. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
				return -1;
			case '\"':
			case '\'':
				if( stat >= 1)
				{
					LogCri( "syntax error. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				rtn = Cfg_GetString( cfg, c);
				if( rtn < 0) return rtn;
				break;
			case '\\':
				Cfg_GetChar( cfg);
				stat = 1;
				break;
			case '$' :
				rtn =  Cfg_GetEnv( cfg);
				if( rtn <= 0) return rtn;
				break;
			default:
				/* 20191106 여러개 변수 입력 가능 
				if( stat > 1)
				{
					LogCri( "too many value. stat=[%d] at=[%d:%d]", stat, cfg->lin, cfg->col);
					return -1;
				}
				*/
				rtn = Cfg_PutChar( cfg, c, "value");
				if( rtn < 0) return -1;
				stat = 1;
				break;
		}
	}

	return 1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetString( CFG *cfg, int ch)
{
	int		rtn, c;

	LogDel( "GetString start");

	while( 1)
	{
		c = Cfg_GetFile( cfg, "string");
		switch( c)
		{
			case 0   :
				return 0;
			case '$' :
				rtn =  Cfg_GetEnv( cfg);
				if( rtn <= 0) return rtn;
				break;
			case '\\':
				Cfg_GetChar( cfg);
				break;
			case '\"':
			case '\'':
				if( c == ch) return 1;
				LogCri( "syntax error. at=[%d:%d]", cfg->lin, cfg->col);
				return -1;
			default:
				rtn = Cfg_PutChar( cfg, c, "string");
				if( rtn < 0) return -1;
				break;
		}
	}
}

/******************************************************************************
 *
******************************************************************************/
int Cfg_GetChar( CFG *cfg)
{
	int					c;
	int					o = 0, x = 0;

	LogDel( "GetChar start");


	c = Cfg_GetFile( cfg, "get char");
	switch( c)
	{
		case 0x00:  /* null */
			return 1;
		case 'a' : Cfg_PutChar( cfg, '\a', "getch"); break; /* bell	*/
		case 'b' : Cfg_PutChar( cfg, '\b', "getch"); break; /* back space */
		case 'f' : Cfg_PutChar( cfg, '\f', "getch"); break; /* form feed */
		case 'n' : Cfg_PutChar( cfg, '\n', "getch"); break; /* line feed */
		case 'r' : Cfg_PutChar( cfg, '\r', "getch"); break; /* carriage return */
		case 't' : Cfg_PutChar( cfg, '\t', "getch"); break; /* horizontal tab */
		case 'v' : Cfg_PutChar( cfg, '\v', "getch"); break; /* veritical tab */
		case '\\': Cfg_PutChar( cfg, '\\', "getch"); break; /* back slash */
		case '\'': Cfg_PutChar( cfg, '\'', "getch"); break; /* single quote */
		case '\"': Cfg_PutChar( cfg, '\"', "getch"); break; /* double quote */
		case '?' : Cfg_PutChar( cfg, '\?', "getch"); break; /* question mark */
		case '$' : Cfg_PutChar( cfg, '$',  "getch"); break; /* question mark */
		case 'x' :
			x = Cfg_GetHexChar( cfg);
			Cfg_PutChar( cfg, x, "getch");
			break;
		case 'B' :
			x = Cfg_GetBitChar( cfg);
			Cfg_PutChar( cfg, x, "getch");
			break;
		default:
			if( c == '0')			
			{
				o = Cfg_GetOctChar( cfg);
			}
			else if( isdigit( c))	
			{
				Cfg_PutFile( cfg, c);
				o = Cfg_GetDecChar( cfg);
			}
			Cfg_PutChar( cfg, o, "getch");
			break;
	}

	return 1;
}

/******************************************************************************
 *
******************************************************************************/
int Cfg_GetHexChar( CFG *cfg)
{
	char	rec[ 512];
	int		pos = 0, c, rtn;

	while( 1)
	{
		c = Cfg_GetFile( cfg, "hex");
		switch( c)
		{
			case 0x00:  /* null */
				return 1;
			default  :
				if( isxdigit( c))
				{
					rec[ pos++] = c;
					break;
				}
				Cfg_PutFile( cfg, c);
				rec[ pos] = 0;
				rtn = strtol( rec, NULL, 16);
				LogDel( "hex=[%s] conv=[%d]", rec, rtn);
				return rtn;
		}
	}
}

/******************************************************************************
 *
******************************************************************************/
int Cfg_GetBitChar( CFG *cfg)
{
	char	rec[ 512];
	int		pos = 0, c, rtn;

	while( 1)
	{
		c = Cfg_GetFile( cfg, "bit");
		switch( c)
		{
			case 0x00:  /* null */
				return 1;
			case '0' :
			case '1' :
				rec[ pos++] = c;
				break;
			default  :
				Cfg_PutFile( cfg, c);
				rec[ pos] = 0;
				rtn = strtol( rec, NULL, 2);
				LogDel( "hex=[%s] conv=[%d]", rec, rtn);
				return rtn;
		}
	}
}

/******************************************************************************
 *
******************************************************************************/
int Cfg_GetOctChar( CFG *cfg)
{
	char	rec[ 512];
	int		pos = 0, c, rtn;

	while( 1)
	{
		c = Cfg_GetFile( cfg, "oct");
		switch( c)
		{
			case 0x00:  /* null */
				return 1;
			default  :
				if( c > 0x30 && c < 0x39)
				{
					rec[ pos++] = c;
					break;
				}
				Cfg_PutFile( cfg, c);
				rec[ pos] = 0;
				rtn = strtol( rec, NULL, 8);
				LogDel( "oct=[%s] conv=[%d]", rec, rtn);
				return rtn;
		}
	}
}

/******************************************************************************
 *
******************************************************************************/
int Cfg_GetDecChar( CFG *cfg)
{
	char	rec[ 512];
	int		pos = 0, c, rtn;

	while( 1)
	{
		c = Cfg_GetFile( cfg, "dec");
		switch( c)
		{
			case 0x00:  /* null */
				return 1;
			default  :
				if( isdigit( c))
				{
					rec[ pos++] = c;
					break;
				}
				Cfg_PutFile( cfg, c);
				rec[ pos] = 0;
				rtn = strtol( rec, NULL, 10);
				LogDel( "dec=[%s] conv=[%d]", rec, rtn);
				return rtn;
		}
	}
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetEnv( CFG *cfg)
{
	int		rtn, c, pos = 0;
	char	rec[ 512], *ptr, *base;

	LogDel( "GetEnv ... ");
	if( LogDelFlag)	Cfg_Print( cfg);

	rtn = Cfg_Push( cfg, "GetEnv");

	while( 1)
	{
		c = Cfg_GetFile( cfg, "get_env");
		switch( c)
		{
			case 0x00:	/* null */
				return 0;
			case 0x0D:  /* carriage return */
			case 0x0A:  /* line feed */
			case 0x09:  /* horizontal tab */
			case '\v':  /* veritical tab */
			case '\\':  /* back slash */
			case 0x20:  /* space */
			case '\"':
			case '\'':
			case '$' :
			case '/' :
				base = ptr = Cfg_Pop( cfg, "GetEnv");
				if( ptr != NULL)
				{
					while( *ptr != 0) 
					{
						rtn = Cfg_PutChar( cfg, *ptr, "string");
						if( rtn < 0) return -1;
						ptr++;
					}
				}
				free( base);
				if( pos <= 0) return -1;
				rec[ pos] = 0;
				ptr = getenv( rec);
				if( ptr == NULL)
				{
					ptr = Cfg_GetEnvPtr( cfg, rec);
					if( ptr == NULL)
					{
						LogCri( "environment variable \"$%s\" not found. at=[%d:%d]", rec, cfg->lin, cfg->col);
						return -1;
					}
				}
				while( *ptr != 0) 
				{
					rtn = Cfg_PutChar( cfg, *ptr, "string");
					if( rtn < 0) return -1;
					ptr++;
				}
				Cfg_PutFile( cfg, c);
				return 1;
			default:
				rec[ pos++] = c;
				if( pos >= 512) return -1;
				break;
		}
	}
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
void* Cfg_GetFirstPtr( CFG *cfg)
{
	CFG_MEMBER	*mp;

	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	mp = ( CFG_MEMBER *)Dll_GetFirstPtr( cfg->cur_mp->value);
	if( mp == NULL) return NULL;

	switch( mp->type)
	{
		case CFG_TYPE_VALUE :
			return mp->value;
		case CFG_TYPE_LIST :
		case CFG_TYPE_DLL :
			return mp->name;
	}

	return NULL;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
void* Cfg_GetNextPtr( CFG *cfg)
{
	CFG_MEMBER	*mp;

	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	mp = ( CFG_MEMBER *)Dll_GetNextPtr( cfg->cur_mp->value);
	if( mp == NULL) return NULL;

	switch( mp->type)
	{
		case CFG_TYPE_VALUE :
			return mp->value;
		case CFG_TYPE_LIST :
		case CFG_TYPE_DLL :
			return mp->name;
	}

	return NULL;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
void* Cfg_GetPtr( CFG *cfg, char *name)
{
	int			sz;
	CFG_MEMBER	*mp;

	if( name == NULL) return NULL;
	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	sz = strlen( name);

	mp = ( CFG_MEMBER *)Dll_GetFirstPtr( cfg->cur_mp->value);
	while( mp != NULL)
	{
		if( mp->type == CFG_TYPE_VALUE)
		{
			LogDel( "compare dll=[%s] name=[%s], sz=[%d]", mp->name, name, sz);
			if( !memcmp( name, mp->name, sz +1)) return mp->value;
		}
		mp = ( CFG_MEMBER *)Dll_GetNextPtr( cfg->cur_mp->value);
	}

	LogMsg( "name not found. name=[%s]", name);
	return NULL;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
void* Cfg_GetListPtr( CFG *cfg)
{
	CFG_MEMBER	*mp;
	static DLL	*ptr = NULL;

	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	if( ptr != cfg->cur_mp->value)
	{
		ptr = ( DLL *)cfg->cur_mp->value;
		mp = ( CFG_MEMBER *)Dll_GetFirstPtr( ptr);
	}
	else
	{
		mp = ( CFG_MEMBER *)Dll_GetNextPtr( ptr);
	}

	if( mp == NULL)	return NULL;

	return mp->name;

	return NULL;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_GetInt( CFG *cfg, char *name)
{
	char		*ptr;

	if( name == NULL) return -1;
	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	ptr = ( char *)Cfg_GetPtr( cfg, name);
	if( ptr == NULL)
	{
		LogDel( "name not found. name=[%s]", name);
		return -1;
	}

	LogDel( "ptr=[%s]", ptr);
	switch( *ptr)
	{
		case 'H':
			return ( int)strtol( ptr+1, 0, 16);
		case 'B':
			return ( int)strtol( ptr+1, 0, 2);
		case 'O':
			return ( int)strtol( ptr+1, 0, 8);
		default:
			return ( int)strtol( ptr, 0, 10);
	}

	return atoi( ptr);
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_PutEnv( CFG *cfg)
{
	int			rtn;
	char		rec[ 512];
	CFG_MEMBER	*mp;

	rtn = Cfg_SetClear( cfg);
	if( rtn < 0)
	{
		LogCri( "Cfg_SetClear error.");
		return -1;
	}

	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;
	mp = cfg->cur_mp;

	mp = ( CFG_MEMBER *)Dll_GetFirstPtr( cfg->cur_mp->value);
	while( mp != NULL)
	{
		LogDel( "member ptr=[%p] name=[%s] type=[%d]", mp, mp->name, mp->type);
		if( mp->type == CFG_TYPE_VALUE) 
		{
			sprintf( rec, "%s=%s", mp->name, ( char *)mp->value);
			rtn = putenv( rec);
			if( rtn != 0)
			{
				LogErr( "putenv error. value=[%s]", rec);
			}
		}
		mp = ( CFG_MEMBER *)Dll_GetNextPtr( cfg->cur_mp->value);
	}

	return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Get( CFG *cfg, char *name, char *rec, int r_sz)
{
	int			sz;
	char		*ptr;

	if( name == NULL) return -1;
	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;

	ptr = ( char *)Cfg_GetPtr( cfg, name);
	if( ptr == NULL) 
	{
		LogDel( "name not found. name=[%s]", name);
		return -1;
	}

	sz = strlen( ptr);
	sz = ( sz < r_sz) ? sz : r_sz;
	memcpy( rec, ptr, sz +1);

	return sz;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Set( CFG *cfg, char *name)
{
	int			rtn, sz;
	CFG_MEMBER	*mp;

	LogDel( "Cfg_Set name=[%s]", name);

	rtn = Cfg_SetClear( cfg);
	if( rtn < 0)
	{
		LogCri( "Cfg_SetClear error. name=[%s]", name);
		return -1;
	}
	if( name == NULL) return 0;

	if( cfg->cur_mp == NULL)	cfg->cur_mp = cfg->member;
	mp = cfg->cur_mp;
	sz = strlen( name);

	mp = ( CFG_MEMBER *)Dll_GetFirstPtr( cfg->cur_mp->value);
	while( mp != NULL)
	{
		LogDel( "member ptr=[%p] name=[%s] type=[%d]", mp, mp->name, mp->type);
		if( mp->type == CFG_TYPE_DLL) 
		{
			LogDel( "compare dll=[%s] name=[%s], sz=[%d]", mp->name, name, sz);
			if( !memcmp( mp->name, name, sz +1))
			{
				rtn = Cfg_SetMakeKeyword( cfg, name);
				if( rtn < 0)
				{
					LogCri( "Cfg_MakeKeyword error. name=[%s]", name);
					return -1;
				}
				cfg->cur_mp = mp;
				return cfg->key_cnt;
			}
		}
		mp = ( CFG_MEMBER *)Dll_GetNextPtr( cfg->cur_mp->value);
	}

	return -1;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_SetClear( CFG *cfg)
{
	while( cfg->key_cnt)
	{
		if( cfg->key[ cfg->key_cnt] != NULL) free( cfg->key[ cfg->key_cnt]);
		cfg->key_cnt--;
	}
	cfg->key = ( char **)realloc( cfg->key, sizeof( char *) * ( cfg->key_cnt +2));
	if( cfg->key == NULL)
	{
		LogErr( "Realloc error. cfg->key sz=[%d]", sizeof( char *) * ( cfg->key_cnt +2));
		return -1;
	}
	cfg->key[ cfg->key_cnt] = NULL;
	cfg->cur_mp = NULL;

	return cfg->key_cnt;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_SetMakeKeyword( CFG *cfg, char *name)
{
	int		sz;

	cfg->key = ( char **)realloc( cfg->key, sizeof( char *) * ( cfg->key_cnt +2));
	if( cfg->key == NULL)
	{
		LogErr( "Realloc error. cfg->key sz=[%d]", sizeof( char *) * ( cfg->key_cnt +2));
		return -1;
	}

	sz = strlen( name);
	cfg->key[ cfg->key_cnt] = ( char *)malloc( sz +1);
	if( cfg->key[ cfg->key_cnt] == NULL)
	{
		LogErr( "Malloc error. cfg->key name sz=[%d]", sz);
		return -1;
	}
	memcpy( cfg->key[ cfg->key_cnt], name, sz +1);
	cfg->key_cnt++;
	cfg->key[ cfg->key_cnt] = NULL;

	return cfg->key_cnt;
}

/*****************************************************************************************************************
 *
*****************************************************************************************************************/
char *Cfg_GetEnvPtr( CFG *cfg, char *name)
{
	int			sz;
	CFG_MEMBER	*start, *ptr;

	sz = strlen( name);

	start = cfg->member;

	ptr = ( CFG_MEMBER *)Dll_GetFirstPtr( start->value);
	while( ptr != NULL)
	{
		switch( ptr->type)
		{
			case CFG_TYPE_VALUE:
				if( !memcmp( ptr->name, name, sz+1))
				{
					return ( char *)ptr->value;
				}
				break;
			case CFG_TYPE_DLL:
			default:
				break;
		}
		ptr = ( CFG_MEMBER *)Dll_GetNextPtr( start->value);
	}

	return NULL;
}


/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetFirstNamePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	LogDel( "cfg->mamber=[%p]", cfg->member);
	if( cfg->member == NULL) return 0;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Cfg_FindName name=[%s] not found.", name);
		return NULL;
	}

	LogDel( "Cfg_FindName mp=[%p] name=[%s] found.", mp, name);

	vp = ( CFG_MEMBER *)Dll_GetFirstPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	LogDel( "return vp->name=[%p:%s]", vp->name, vp->name);
	return vp->name;
}

/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetNextNamePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	vp = ( CFG_MEMBER *)Dll_GetNextPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	return vp->name;
}

/******************************************************************************
 *
******************************************************************************/
CFG_MEMBER *Cfg_GetFirstMemberPtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	LogDel( "cfg->mamber=[%p]", cfg->member);
	if( cfg->member == NULL) return 0;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Cfg_FindName name=[%s] not found.", name);
		return NULL;
	}

	LogDel( "Cfg_FindName mp=[%p] name=[%s] found.", mp, name);

	vp = ( CFG_MEMBER *)Dll_GetFirstPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	LogDel( "return vp->name=[%p:%s]", vp->name, vp->name);
	return vp;
}

/******************************************************************************
 *
******************************************************************************/
CFG_MEMBER *Cfg_GetNextMemberPtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	vp = ( CFG_MEMBER *)Dll_GetNextPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	return vp;
}

/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetCurrNamePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	vp = ( CFG_MEMBER *)Dll_GetCurrPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	return vp->name;
}

/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetFirstValuePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	LogDel( "cfg->mamber=[%p]", cfg->member);
	if( cfg->member == NULL) return 0;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	LogDel( "Dll_GetFindFuncPtr mp=[%p] name=[%s] found.", mp, name);

	vp = ( CFG_MEMBER *)Dll_GetFirstPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	LogDel( "return vp->name=[%p:%s]", vp->name, vp->name);
	return vp->value;
}

/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetNextValuePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	vp = ( CFG_MEMBER *)Dll_GetNextPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	return vp->value;
}

/******************************************************************************
 *
******************************************************************************/
void *Cfg_GetCurrValuePtr( CFG *cfg, char *name)
{
	CFG_MEMBER	*mp, *vp;

	mp = Cfg_FindName( cfg, cfg->member, name);
	if( mp == NULL)
	{
		LogDel( "Dll_GetFindFuncPtr name=[%s] not found.", name);
		return NULL;
	}

	vp = ( CFG_MEMBER *)Dll_GetCurrPtr( mp->value);
	if( vp == NULL) 
	{
		LogDel( "[%s] not found.", mp->name);
		return NULL;
	}

	return vp->value;
}

/******************************************************************************
 *
******************************************************************************/
CFG_MEMBER *Cfg_FindName( CFG *cfg, CFG_MEMBER *dp, char *name)
{
	int			sz;
	CFG_MEMBER	*mp;

	switch( dp->type)
	{
		case CFG_TYPE_VALUE:
		case CFG_TYPE_LIST:
			sz = CFG_MAX( strlen( dp->name), strlen( name));
			if( memcmp( dp->name, name, sz) == 0) return dp;
			break;
		case CFG_TYPE_DLL:
			mp = ( CFG_MEMBER *)Dll_GetFirstPtr( dp->value);
			while( mp != NULL)
			{
				LogDel( "find name. dp->name=[%s] name[%s]", mp->name, name);
				sz = CFG_MAX( strlen( mp->name), strlen( name));
				if( memcmp( mp->name, name, sz) == 0) return mp;
				if( mp->type == CFG_TYPE_DLL) 
				{
					mp = Cfg_FindName( cfg, mp, name);
					if( mp != NULL) return mp;
				}
				mp = ( CFG_MEMBER *)Dll_GetNextPtr( dp->value);
			}
			break;
		default:
			break;
	}
	return NULL;
}


/*****************************************************************************************************************
 *
*****************************************************************************************************************/
int Cfg_Print( CFG *cfg)
{
	int			i;

	printf( "=================================================================================================================\n");
	printf( "CFG ptr                      = [%p]\n", cfg);
	printf( "f_name                       = [%s]\n", cfg->f_name);
	printf( "fp                           = [%p]\n", cfg->fp);
	printf( "lin                          = [%d]\n", cfg->lin);
	printf( "col                          = [%d]\n", cfg->col);
	printf( "word                         = [%*.*s]\n", cfg->w_pos, cfg->w_pos, cfg->word);
	printf( "w_pos                        = [%d]\n", cfg->w_pos);
	printf( "stack                        = [%p]\n", cfg->stack);
	printf( "s_cnt                        = [%d]\n", cfg->s_cnt);
	for( i = 0; i < cfg->s_cnt; i++)
		printf( "stack[%3d]                   = [%s]\n", i, cfg->stack[ i]);

	printf( "-----------------------------------------------------------------------------------------------------------------\n");
	printf( "%-10s ", "ptr");
	printf( "%-30s", "name");
	printf( "%s ", "type");
	printf( "%s ", "value");
	printf( "\n");
	printf( "-----------------------------------------------------------------------------------------------------------------\n");

	Cfg_PrintMember( cfg, cfg->member, "");
	printf( "=================================================================================================================\n");

	return 1;
}

int Cfg_PrintMember( CFG *cfg, CFG_MEMBER *member, const char *name)
{
	char 		rec[ 512];
	CFG_MEMBER	*ptr;

	if( member->name == NULL)	sprintf( rec, "%s", name);
	else 						sprintf( rec, "%s %s", name, member->name);

	printf( "%p ", member);
	printf( "%-30s", rec);
	printf( "%4d ", member->type);

	switch( member->type)
	{
		case CFG_TYPE_VALUE:
			printf( "[%s]", ( char *)member->value);
			printf( "\n");
			break;
		case CFG_TYPE_LIST:
			printf( "\n");
			break;
		case CFG_TYPE_DLL:
			printf( "\n");
			ptr = ( CFG_MEMBER *)Dll_GetFirstPtr( member->value);
			while( ptr != NULL)
			{
				Cfg_PrintMember( cfg, ptr, rec);
				ptr = ( CFG_MEMBER *)Dll_GetNextPtr( member->value);
			}
			break;
	}

	return 1;
}

