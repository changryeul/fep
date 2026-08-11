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
#include "struct.h"
#include "main.h"
#include "task.h"

/*************************************************************************************************
 *
*************************************************************************************************/
typedef struct	_stloader_keyword_
{
	int		no;												/* key number						*/
	char	word[ 32];										/* key word							*/
	char	comment[ 512];
}	STLOADER_KEYWORD;

STLOADER_KEYWORD	StLoaderKeyTable[ 32] = 
{
	{	1,		"typedef",	""},
	{	2,		"struct",	""},
	{	3,		"double",	""},
	{	-1,		"",			""}
};

/************************************************************************************************
 *
************************************************************************************************/
ST_LOADER   *StLoader_Open( char *file_name);

int StLoader()
{
	int			rtn, pos;
	ST_LOADER	*st;
	char		*f_name, *o_name, buf[ 512];
	FILE		*ofp;
	// FILE		*ifp;
	struct stat	stat_buf;

	f_name = Param->file_list[ 0];
	o_name = Param->out_file;

	if( o_name[ 0] == 0)
	{
		ofp = stdout;
	}
	else
	{
		sprintf( buf, "a+");

		rtn = stat( o_name, &stat_buf);
		if( rtn == 0)
		{
			printf( "Output file already exist. name=[%s] \n", f_name);
			printf( "input action (c)ancel/(a)ppend/(o)verwrite (default=cancel): ");
			fgets( buf, 512, stdin);
			switch( buf[ 0])
			{
				case 'a':
					printf( "append file. name=[%s]\n", f_name);
					sprintf( buf, "a+");
					break;
				case 'o':
					printf( "overwrite file. name=[%s]\n", f_name);
					sprintf( buf, "w+");
					break;
				case 'c':
				default :
					return -1;
			}
		}

		LogMsg( "Output file open. name=[%s] option=[%s]", o_name, buf);
		ofp = fopen( o_name, buf);
		if( ofp == NULL)
		{
			LogErr( "fopen error. name=[%s]", o_name);
			return -1;
		}
	}

	pos = 1;
	f_name = Param->file_list[ pos];
	while( f_name != NULL)
	{
		printf( "loading file. name=[%s] \n", f_name);
		st = StLoader_Open( f_name);
		if( st == NULL)
		{
			LogCri( "StLoader_Open error. name=[%s]", f_name);
			pos++;
			f_name = Param->file_list[ pos];
		}

		rtn = StLoader_PrintFile( st, ofp);

		StLoader_Close( st);

		pos++;
		f_name = Param->file_list[ pos];
	}

	/*
	st = StLoader_Open( "test.h");
	if( st == NULL)
	{
		return -1;
	}

	StLoader_Close( st);
	*/

	
	/*
	fp = fopen( "test.c", "a+");
	StLoader_PrintFile( st, stdout);
	StLoader_PrintFile( st, fp);
	fclose( fp);
	*/

	return 1;

}

ST_LOADER	*StLoader_Open( char *file_name)
{
	ST_LOADER	*st;

	st = malloc( sizeof( ST_LOADER));
	if( st == NULL)
	{
		LogErr( "malloc error. ST_LOADER sz=[%d]", sizeof( ST_LOADER));
		goto error_1;
	}
	memset( st, 0, sizeof( ST_LOADER));

	memcpy( st->f_name, file_name, strlen( file_name) +1);

	st->st_form = Dll_Open( 0);
	if( st->st_form == NULL)
	{
		LogCri( "Dll_Open error. st->st_form");
		goto error_2;
	}

	st->define = Dll_Open( 0);
	if( st->define == NULL)
	{
		LogCri( "Dll_Open error. st->define");
		goto error_3;
	}

	st->type = Dll_Open( 0);
	if( st->type == NULL)
	{
		LogCri( "Dll_Open error. st->type");
		goto error_4;
	}

	memset( st->rec, 0x00, 8192);
	StLoader_LoadFile( st);


	return st;

	error_4:
		Dll_Close( st->define);
	error_3:
		Dll_Close( st->st_form);
	error_2:
		free( st);
	error_1:
		return NULL;
}

int StLoader_Close( ST_LOADER *st)
{
	STRUCT_FORM		*form;
	STRUCT_MEMBER	*member;
	DEFINE_FORM		*def;
	TYPEDEF_FORM	*type;
	int				pos;

	LogDel( "StLoader_Close. ptr=%p]", st);

	LogDel( "st->cmt ptr=[%p]", st->cmt);
	if( st->cmt != NULL) 
	{
		free( st->cmt);
		st->cmt = NULL;
		st->c_sz = 0;
	}

	if( st->st_form != NULL)
	{
		LogDel( "st->st_form ptr=[%p]", st->st_form);
		form = Dll_GetFirstPtr( st->st_form);
		while( form != NULL)
		{
			member = Dll_GetFirstPtr( form->member);
			while( member != NULL)
			{
				if( member->name != NULL)		free( member->name);
				if( member->type != NULL)		free( member->type);
				if( member->comment != NULL)	free( member->comment);

				if( member->size != NULL)
				{
					pos = 0;
					while( pos < member->s_cnt)
					{
						if( member->size[ pos] != NULL)	free( member->size[ pos]);
						pos++;
					}
					free( member->size);
				}
				free( member);
				member = Dll_GetNextPtr( form->member);
			}
			form = Dll_GetNextPtr( st->st_form);
		}
	}
	LogDel( "Dll_Close st->st_form ptr=[%p]", st->st_form);
	Dll_Close( st->st_form);

	if( st->define != NULL)
	{
		LogDel( "st->define ptr=[%p]", st->define);
		def = Dll_GetFirstPtr( st->define);
		while( def != NULL)
		{
			if( def->name != NULL)		free( def->name);
			if( def->value != NULL)		free( def->name);
			def = Dll_GetNextPtr( st->define);
		}
		Dll_Close( st->define);
	}

	if( st->type != NULL)
	{
		LogDel( "st->type ptr=[%p]", st->type);
		type = Dll_GetFirstPtr( st->type);
		while( type != NULL)
		{
			if( type->name != NULL)			free( type->name);
			if( type->type != NULL)			free( type->type);
			type = Dll_GetNextPtr( st->type);
		}
		Dll_Close( st->type);
	}

	if( st->fp != NULL)	fclose( st->fp);
	free( st);

	return 1;
}

int StLoader_LoadFile( ST_LOADER *st)
{
	// char 	*p;

	LogDel( "header file open. name=[%s]", st->f_name);
	StLoader_Start( st);

	/*
	while( 1)
	{
		rtn = StLoader_GetLine( st);
		if( rtn < 0) break;
		printf( "%s", st->rec);
	}
	*/

	if( st->fp != NULL) fclose( st->fp);
	st->fp = NULL;

	return 1;
}

int StLoader_GetLine( ST_LOADER *st)
{
	char	*p;

	LogDel( "StLoader_GetLine ...");

	if( st->fp == NULL)
	{
		st->fp = fopen( st->f_name, "r");
		if( st->fp == NULL)
		{
			LogErr( "file open error. name=[%s]", st->f_name);
			return -1;
		}
		st->lin = 0;
		st->col = 0;
	}

	p = fgets( st->rec, ST_BUF_SZ, st->fp);
	if( p == NULL)
	{
		LogDel( "End of file. name=[%s] fp=[%p]", st->f_name, st->fp);
		return -1;
	}

	st->lin++;
	st->col = 0;

	LogDel( "StLoader_GetLine ... line=[%d]", st->lin);
	LogDel( "[%s]", st->rec);
	return st->lin;
}

/************************************************************************************************
 *
************************************************************************************************/
int StLoader_CheckKeyword( ST_LOADER *st, char *word, int sz)
{
	// int					rtn;
	// int					pos = 0;
	int					cmp_sz;
	char				*ptr;
	STLOADER_KEYWORD	*kp = &StLoaderKeyTable[ 0];
	STRUCT_FORM			*form;
	TYPEDEF_FORM		*type;

	LogDel( "Check keyword ... word=[%2d:%.*s] ", sz, sz, word);

	while( kp->no >= 0)
	{
		cmp_sz = MAX( sz, strlen( kp->word));
		LogDel( "Compare ...    keyword=[%2d:%s]", cmp_sz, kp->word);
		if( !memcmp( word, kp->word, cmp_sz))
		{
			switch( kp->no)
			{
				case 1 :
					type = StLoader_Typedef( st);
					if( type == NULL)
					{
						LogCri( "StLoader_Typedef error.");
						return -1;
					}
					ptr = Dll_Add( st->type, type, sizeof( TYPEDEF_FORM));
					if( ptr == NULL)
					{
						LogCri( "Dll_Add error. st->type.");
						return -1;
					}
					st->t_cnt++;
					LogDel( "add typedef_form. cnt=[%d] ptr=[%p]", st->t_cnt, type);
					return st->t_cnt;
					break;
				case 2 :
					st->w_cnt = 0;
					form = StLoader_Struct( st);
					if( form == NULL)
					{
						LogCri( "StLoader_Struct error.");
						return -1;
					}
					ptr = Dll_Add( st->st_form, form, sizeof( STRUCT_FORM));
					if( ptr == NULL)
					{
						LogCri( "Dll_Add error. st->st_form.");
						return -1;
					}
					st->st_cnt++;
					LogDel( "add struct_form. cnt=[%d] ptr=[%p]", st->st_cnt, form);
					return st->st_cnt;
				default:
					return 0;
			}
			return kp->no;
		}
		kp++;
	}

	LogDel( "No action keyword ... word=[%d:%s] ", sz, word);
	return 0;
}

/************************************************************************************************
 *
************************************************************************************************/
int StLoader_Start( ST_LOADER *st)
{
	int				rtn;
	// void			*ptr;
	// STRUCT_FORM		*st_form;

	while( 1)
	{
		LogDel( "start pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '#' :
				st->w_cnt = 0;
				/* define 구현이 필요 없을듯 하여 나중에 테스트 
				StLoader_NumberSign( st);
				*/
				StLoader_EndofLine( st);
				break;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( st->w_cnt <= 0) break;
				st->word[ st->w_cnt] = 0;
				rtn = StLoader_CheckKeyword( st, st->word, st->w_cnt);
				if( rtn < 0)	goto error_1;
				st->w_cnt = 0;
				break;
			case '{' :
				StLoader_CheckKeyword( st, st->word, st->w_cnt);
				st->word[ st->w_cnt++] = st->rec[ st->col];
				st->col++;
				StLoader_Push( st);
				break;
			case '/' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_CommentPlus( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case '*' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_Comment( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			default  :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}

	error_1:
		LogDel( "Syntax error. line=[%d] column=[%d]", st->lin, st->col);
		return -1;
	
}

/************************************************************************************************
 *
************************************************************************************************/
TYPEDEF_FORM *StLoader_Typedef( ST_LOADER *st)
{
	int				rtn;
	// int				stat = 0;
	char			rec[ 8192];
	TYPEDEF_FORM	*type;

	st->w_cnt = 0;

	type = malloc( sizeof( TYPEDEF_FORM));
	if( type == NULL)
	{
		LogErr( "malloc error. type size=[%d]", sizeof( TYPEDEF_FORM));
		return NULL;
	}
	memset( type, 0, sizeof( TYPEDEF_FORM));

	while( 1)
	{
		LogDel( "typedef pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
			case '{' :
				if( st->w_cnt <= 0) break;
				st->word[ st->w_cnt] = 0;
				type->type = malloc( st->w_cnt +1);
				if( type->type == NULL)
				{
					LogErr( "malloc error. type size=[%d]", st->w_cnt +1);
					goto error_2;
				}
				memcpy( type->type, st->word, st->w_cnt +1);
				rtn = StLoader_CheckKeyword( st, st->word, st->w_cnt);
				if( rtn <= 0)
				{
					free( type->type);
					st->w_cnt = 0;
					goto error_2;
				}
				st->w_cnt = 0;
				continue;
			case '/' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_CommentPlus( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case '*' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_Comment( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case ';' :
				LogDel( "get struct name ...");
				rtn = StLoader_Pop( st, rec, 8192);
				if( rtn <= 0) goto error_2;
				type->name = malloc( rtn +1);
				if( type->name == NULL)
				{
					LogErr( "malloc error. type->name sz=[%d]", rtn);
					goto error_2;
				}
				memcpy( type->name, rec, rtn +1);
				LogDel( "typedef ... END. type=[%s] name=[%s]", type->type, type->name);
				return type;
			default  :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}
	return type;

	error_2:
		free( type);
	// error_1:
		LogDel( "Error. line=[%d] column=[%d]", st->lin, st->col);
		return NULL;
}

/************************************************************************************************
 *
 ***********************************************************************************************/
STRUCT_FORM *StLoader_Struct( ST_LOADER *st)
{
	int				rtn;
	int				stat = 0;
	// char			*ptr;
	STRUCT_FORM		*st_form;
	// STRUCT_MEMBER	*member;
	STRUCT_MEMBER	*m_ptr = NULL;
	char			comment[ 8192];
	int				cmt_sz;

	st_form = malloc( sizeof( STRUCT_FORM));
	if( st_form == NULL)
	{
		LogErr( "malloc error. STRUCT_FORM sz=[%d]", sizeof ( STRUCT_FORM));
		goto error_1;
	}
	memset( st_form, 0, sizeof( STRUCT_FORM));
	LogDel( "st_form=[%p]", st_form);

	st_form->member = Dll_Open( 0);
	if( st_form->member == NULL)
	{
		LogCri( "Dll_Open error.");
		goto error_2;
	}
	LogDel( "member Dll_Open=[%p]", st_form->member);

	while( 1)
	{
		LogDel( "struct pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
				break;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( st->w_cnt <= 0) break;
				st->word[ st->w_cnt] = 0;
				st_form->name = malloc( st->w_cnt +1);
				if( st_form->name == NULL)
				{
					LogErr( "malloc error. struct name sz=[%d]", st->w_cnt +1);
				}
				memcpy( st_form->name, st->word, st->w_cnt +1);
				LogDel( "struct name=[%s]", st_form->name);
				st->w_cnt = 0;
				stat++;
				break;
			case '{' :
				stat++;
				rtn = StLoader_Member( st, st_form);
				if( rtn < 0)
				{
					LogCri( "StLoader_Member error.");
					st->rec[ st->col] = 0;
					continue;
					goto error_3;
				}
				break;
			case '}' :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				StLoader_Push( st);
				break;
			case ']' :
			case '[' :
				StLoader_Push( st);
				st->word[ st->w_cnt++] = st->rec[ st->col];
				StLoader_Push( st);
				break;
			case '*' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					cmt_sz = StLoader_Comment( st);
					LogDel( "StLoader_Comment return cmt_sz=[%d]", cmt_sz);
					if( cmt_sz > 0)
					{
						LogDel( "comment sz=[%d]", cmt_sz);
						comment[ cmt_sz] = 0;
						if( m_ptr != NULL)
						{
							m_ptr->comment = malloc( cmt_sz +1);
							if( m_ptr->comment == NULL)
							{
								LogErr( "malloc error. comment sz=[%d]", cmt_sz +1);
								goto error_3;
							}
							memcpy( m_ptr->comment, comment, cmt_sz +1);
							LogDel( "add comment. m_ptr=[%p] comment=[%s]", m_ptr, comment);
						}
					}
				}
				else
				{
					StLoader_Push( st);
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case ';' :
				st->word[ st->w_cnt] = 0;
				StLoader_Push( st);
				st_form->type_name = malloc( st->w_cnt +1);
				if( st_form->type_name == NULL)
				{
					LogErr( "malloc error. struct type_form sz=[%d]", st->w_cnt +1);
					goto error_3;
				}
				memcpy( st_form->type_name, st->word, st->w_cnt +1);
				st->w_cnt = 0;
				LogDel( "add type_name=[%s]", st_form->type_name);
				LogDel( "StLoader_Struct ... END. return st_form=[%p]", st_form);
				/*
				StLoader_PrintMember( st, st_form);
				*/
				return st_form;
			default  :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}

	error_3:
		Dll_Close( st_form->member);
	error_2:
		free( st_form);
	error_1:
	// error:
		return NULL;
}

/************************************************************************************************
 *
************************************************************************************************/
int StLoader_Member( ST_LOADER *st, STRUCT_FORM *form)
{
	int				rtn;
	int				stat = 0, sz;
	void			*ptr;
	STRUCT_MEMBER	*member, *m_ptr;

	LogDel( "StLoader_Member ...");
	st->col++;

	member = malloc( sizeof( STRUCT_MEMBER));
	if( member == NULL)
	{
		LogErr( "malloc error. member sz=[%d]", sizeof( STRUCT_MEMBER));
		goto error_1;
	}
	memset( member, 0, sizeof( STRUCT_MEMBER));

	while( 1)
	{
		LogDel( "member pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				if( st->w_cnt <= 0) break;
				st->word[ st->w_cnt] = 0;
				switch( stat)
				{
					case 0:		/* type */
						member->type = realloc( member->type, st->w_cnt +0);
						if( member->type == NULL)
						{
							LogErr( "malloc error. member->type sz=[%d]", st->w_cnt +1);
							goto error_2;
						}
						memcpy( member->type, st->word, st->w_cnt +1);
						LogDel( "add type. name=[%s]", member->type);
						st->w_cnt = 0;
						stat++;
						break;
					case 1:		/* name */
						if( st->word[ st->w_cnt -1] == '*')
						{
							sz = strlen( member->type);
							member->type = realloc( member->type, sz + st->w_cnt +1);
							if( member->type == NULL)
							{
								LogErr( "malloc error. member->type sz=[%d]", sz + st->w_cnt +1);
								goto error_2;
							}
							memcpy( &member->type[ sz], st->word, st->w_cnt +1);
							LogDel( "add type. name=[%s]", member->type);
							st->w_cnt = 0;
							break;
						}
						member->name = malloc( st->w_cnt +1);
						if( member->name == NULL)
						{
							LogErr( "malloc error. member->name sz=[%d]", st->w_cnt +1);
							goto error_3;
						}
						memcpy( member->name, st->word, st->w_cnt +1);
						LogDel( "add name. name=[%s]", member->name);
						st->w_cnt = 0;
						stat++;
						break;
					case 2:		/* size */
					default:
						break;
				}
				break;
			case '{' :
				StLoader_Push( st);
				st->word[ st->w_cnt++] = st->rec[ st->col];
				StLoader_Struct( st);
				break;
			case '}' :
				return form->m_cnt;
				break;
			case '[' :
				st->word[ st->w_cnt] = 0;
				switch( stat)
				{
					case 0:		/* type */
						LogMsg( "Syntax error. at=[%d:%d]", st->lin, st->col);
						goto error_3;
					case 1:		/* name */
						if( st->word[ st->w_cnt -1] == '*')
						{
							LogMsg( "Syntax error. at=[%d:%d]", st->lin, st->col);
							goto error_3;
						}
						member->name = malloc( st->w_cnt +1);
						if( member->name == NULL)
						{
							LogErr( "malloc error. member->name sz=[%d]", st->w_cnt +1);
							goto error_3;
						}
						memcpy( member->name, st->word, st->w_cnt +1);
						LogDel( "add name. name=[%s]", member->name);
						st->w_cnt = 0;
						stat++;
						break;
					case 2:		/* size */
					default:
						break;
				}
				if( stat < 2)
				{
					LogMsg( "Syntax error. at=[%d:%d]", st->lin, st->col);
					goto error_3;
				}
				ptr = StLoader_GetSize( st);
				if( ptr == 0)
				{
					LogMsg( "StLoader_GetSize error.");
					goto error_3;
				}
				sz = strlen( ptr);
				member->size = realloc( member->size, sizeof( char *) * ( member->s_cnt +2));
				if( member->size == NULL)
				{
					LogErr( "malloc error. member->size sz=[%d]", sizeof( char *) * ( member->s_cnt +2));
					goto error_3;
				}
				member->size[ member->s_cnt] = ptr;
				member->s_cnt++;
				break;
			case ']' :
				LogMsg( "Syntax error. at=[%d:%d]", st->lin, st->col);
				goto error_3;
			case '/' :		/* 20250219 */
			case '*' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					if( st->rec[ st->col] == '*')
					{
						rtn = StLoader_Comment( st);
					}
					else if(  st->rec[ st->col] == '/')	/* 20250219 */
					{
						rtn = StLoader_CommentPlus( st);
					}
					/* 첫번째 comment만 가져오기 위함 20230907 */
					if( m_ptr != NULL && m_ptr->comment == NULL)
					{
						m_ptr->comment = malloc( st->c_sz +1);
						if( m_ptr->comment == NULL)
						{
							LogErr( "malloc error. member comment sz=[%d]", st->c_sz +1);
							return -1;
						}
						memcpy( m_ptr->comment, st->cmt, st->c_sz +1);
						free( st->cmt);
						st->cmt = NULL;
						st->c_sz = 0; /* 20230907 */
					}
				}
				else
				{
					if( stat == 1)
					{
						sz = strlen( member->type);
						member->type = realloc( member->type, sz + 1);
						if( member->type == NULL)
						{
							LogErr( "malloc error. member->type sz=[%d]", sz + st->w_cnt +1);
							goto error_3;
						}
						member->type[ sz] = '*';
						member->type[ sz +1] = 0;
						LogDel( "add type. name=[%s]", member->type);
						st->w_cnt = 0;
						break;
					}
					else
					{
						LogDel( "stat=[%d]", stat);
						LogDel( "member type=[%p] name=[%p] member=[%p]", member->type, member->name, member);
						LogMsg( "Syntax error. at=[%d:%d]. Ommit member name=[%s]", st->lin, st->col, member->name);
						goto error_2;
					}
				}
				break;
			case ';' :
				st->word[ st->w_cnt] = 0;
				if( st->w_cnt > 0)
				{
					switch( stat)
					{
						case 0:		/* type */
							goto error_3;
						case 1:		/* name */
							if( st->word[ st->w_cnt -1] == '*')
							{
								sz = strlen( member->type);
								member->type = realloc( member->type, sz + st->w_cnt +1);
								if( member->type == NULL)
								{
									LogErr( "malloc error. member->type sz=[%d]", sz + st->w_cnt +1);
									goto error_2;
								}
								memcpy( &member->type[ sz], st->word, st->w_cnt +1);
								LogDel( "add type. name=[%s]", member->type);
								st->w_cnt = 0;
								break;
							}
							member->name = malloc( st->w_cnt +1);
							if( member->name == NULL)
							{
								LogErr( "malloc error. member->name sz=[%d]", st->w_cnt +1);
								goto error_3;
							}
							memcpy( member->name, st->word, st->w_cnt +1);
							LogDel( "add name. name=[%s]", member->name);
							st->w_cnt = 0;
							stat++;
							break;
						case 2:		/* size */
						default:
							break;
					}
				}
				m_ptr = Dll_Add( form->member, member, sizeof( STRUCT_MEMBER));
				if( m_ptr == NULL)
				{
					LogCri( "Dll_Add error. member=[%p]", member);
					goto error_3;
				}
				form->m_cnt++;
				LogDel( "add member. m_cnt=[%d] type=[%s] name=[%s] s_cnt=[%d]", 
						form->m_cnt, member->type, member->name, member->s_cnt);
				member = malloc( sizeof( STRUCT_MEMBER));
				if( member == NULL)
				{
					LogErr( "malloc error. member sz=[%d]", sizeof( STRUCT_MEMBER));
					goto error_3;
				}
				memset( member, 0, sizeof( STRUCT_MEMBER));
				st->w_cnt = 0;
				stat = 0;
				break;
			default  :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}

	error_3:
	error_2:
		if( member != NULL)
		{
			if( member->name != NULL) free( member->name);
			if( member->type != NULL) free( member->type);
			free( member);
		}
	error_1:
		return -1;
}

/************************************************************************************************
 *
************************************************************************************************/
char *StLoader_GetSize( ST_LOADER *st)
{
	int		rtn;
	char	*ptr;

	st->col++;

	while( 1)
	{
		LogDel( "get_size pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '\t':
			case '\r':
			case '\n':
			case ' ' :
				break;
			case '/' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_CommentPlus( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case '*' :
				if( st->word[ st->w_cnt -1] == '/') 
				{
					st->w_cnt--;
					StLoader_Comment( st);
				}
				else
				{
					st->word[ st->w_cnt++] = st->rec[ st->col];
				}
				break;
			case ']' :
				st->word[ st->w_cnt] = 0;
				ptr = malloc( st->w_cnt +1);
				if( ptr == NULL)
				{
					LogErr( "malloc error. sz=[%d]", st->w_cnt +1);
					return NULL;
				}
				memcpy( ptr, st->word, st->w_cnt +1);
				st->w_cnt = 0;
				return ptr;
			default  :
				st->word[ st->w_cnt++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}
	return NULL;
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int StLoader_Comment( ST_LOADER *st)
{
	int		rtn;

	LogDel( "StLoader_Comment ... ");

	st->col++;

	LogDel( "st->cmt=[%p]", st->cmt);
	if( st->cmt == NULL)
	{
		st->cmt = malloc( 8192);
		if( st->cmt == NULL)
		{
			LogErr( "malloc error. comment st->cmt sz=[%d]", 8192);
			return -1;
		}
	}
	st->c_sz = 0;

	while( 1)
	{
		LogDel( "comment pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '/' :
				if( st->c_sz <= 0) 
				{
					st->cmt[ st->c_sz++] = st->rec[ st->col];
					break;
				}
				if( st->cmt[ st->c_sz -1] == '*') 
				{
					LogDel( "StLoader_Comment ... END");
					st->c_sz--;
					st->cmt[ st->c_sz] = 0;
					return st->c_sz;
				}
				st->cmt[ st->c_sz++] = st->rec[ st->col];
				break;
			case '*' :
				if( st->c_sz >= 8192) break;
				st->cmt[ st->c_sz++] = st->rec[ st->col];
				break;
			default  :
				if( st->c_sz >= 8192) break;
				st->cmt[ st->c_sz++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}
}

/************************************************************************************************
 *
 ***********************************************************************************************/
int StLoader_CommentPlus( ST_LOADER *st)
{
	int		rtn;

	LogDel( "StLoader_CommentPlus ... ");

	st->col++;

	if( st->cmt == NULL)
	{
		st->cmt = malloc( 8192);
		if( st->cmt == NULL)
		{
			LogErr( "malloc error. comment st->cmt sz=[%d]", 8192);
			return -1;
		}
	}
	st->c_sz = 0;

	while( 1)
	{
		LogDel( "comment_plus pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '\n':
				LogDel( "StLoader_Comment ... END");
				st->cmt[ st->c_sz] = 0;
				return st->c_sz;
				break;
			default  :
				if( st->c_sz >= 8192) break;
				st->cmt[ st->c_sz++] = st->rec[ st->col];
				break;
		}
		st->col++;
	}
}

int StLoader_EndofLine( ST_LOADER *st)
{
	int		rtn;
	// char 	word[ 512];
	// int		cnt = 0;

	LogDel( "StLoader_EndofLine ... ");

	st->col++;

	while( 1)
	{
		LogDel( "EOL pos=[%3d:%3d] char=[%c][0x%02x][%3d]", 
				st->lin, st->col, st->rec[ st->col], st->rec[ st->col], st->rec[ st->col]);
		switch( st->rec[ st->col])
		{
			case '\\':
			case 0   :
				rtn = StLoader_GetLine( st);
				if( rtn < 0) return 0;
				continue;
			case '\n' :
				StLoader_StackClear( st);
				return 1;
			default  :
				break;
		}
		st->col++;
	}
}

int StLoader_Push( ST_LOADER *st)
{
	int		pos = 0;

	if( st->stack == NULL)
	{
		st->stack = malloc( sizeof( char *));
		if( st->stack == NULL)
		{
			LogErr( "malloc error. stack size=[%d]", sizeof( char *));
			return -1;
		}
		st->stack[ st->s_cnt] = NULL;
	}

	if( st->w_cnt <= 0) return 0;

	st->stack = realloc( st->stack, sizeof( char *) * ( st->s_cnt +2));
	if( st->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}

	st->stack[ st->s_cnt] = malloc( st->w_cnt +1);
	if( st->stack[ st->s_cnt] == NULL)
	{
		LogErr( "malloc error. stack data sz=[%d]", st->w_cnt);
		return -1;
	}

	st->word[ st->w_cnt] = 0;
	memcpy( st->stack[ st->s_cnt], st->word, st->w_cnt +1);
	st->stack[ st->s_cnt][ st->w_cnt] = 0;
	LogDel( "add stack. stack=[%p] stack[%d]=[%p] data=[%s] sz=[%d]", 
			st->stack, st->s_cnt, st->stack[ st->s_cnt], st->stack[ st->s_cnt], st->w_cnt);

	st->s_cnt++;
	st->stack[ st->s_cnt] = NULL;

	pos = st->s_cnt;
	while( pos >= 0) 
	{
		LogDel( "stack[%2d]=[%s]", pos, st->stack[ pos]);
		pos--;
	}

	return st->w_cnt;
}

int StLoader_Pop( ST_LOADER *st, char *data, int sz)
{
	// int		pos = 0;
	int		s_sz;

	if( st->stack == NULL)
	{
		return 0;
	}

	st->s_cnt--;
	s_sz = strlen( st->stack[ st->s_cnt]);
	if( s_sz >sz) s_sz = sz -1;
	memcpy( data, st->stack[ st->s_cnt], s_sz);
	data[ s_sz] = 0;

	free( st->stack[ st->s_cnt]);
	st->stack[ st->s_cnt] = NULL;

	st->stack = realloc( st->stack, sizeof( char *) * ( st->s_cnt +2));
	if( st->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}

	LogDel( "StLoader_Pop ... data=[%d:%s]", s_sz, data);

	return s_sz;
}

int StLoader_StackClear( ST_LOADER *st)
{
	int pos;

	LogDel( "stack clear. cnt=[%d]", st->s_cnt);
	if( st->s_cnt <= 0) return 1;
	pos = st->s_cnt;
	while( pos >= 0)
	{
		LogDel( "stack clear pos=[%d] stack=[%p]", pos, st->stack[ pos]);
		if( st->stack[ pos] != NULL) free( st->stack[ pos]);
		pos--;
	}

	st->s_cnt = 0;
	st->stack = realloc( st->stack, sizeof( char *));
	if( st->stack == NULL)
	{
		LogErr( "realloc error. stack size=[%d]", sizeof( char *));
		return -1;
	}
	st->stack[ st->s_cnt] = NULL;

	return 1;
}

int StLoader_CheckType( ST_LOADER *st, char *type)
{
	int		sz;
	int		pos = 0;
	char	t_tbl[ 512][ 32] =
	{
		"    ",
		"char",
		"int",
		"FILE",
		"DLL",
		""
	};

	sz = strlen( type);

	while( t_tbl[ pos][ 0] != 0)
	{
		if( !memcmp( type, &t_tbl[ pos], sz)) return pos;
		pos++;
	}

	return 0;
}

int StLoader_MemberSize( ST_LOADER *st, char **sp)
{
	int		sz = 0;
	char	rec[ 512];
	char	token[ 8] = " +\t\n";
	char	*ptr;

	if( sp == NULL) return 0;
	memcpy( rec, sp[ 0], strlen( sp[ 0]) +1);

	ptr = strtok( rec, token);
	while( ptr != NULL)
	{
		sz += atoi( ptr);
		ptr = strtok( NULL, token);
	}

	return sz;
}

int StLoader_PrintFile( ST_LOADER *st, FILE *fp)
{
	// int				rtn;
	int				sz, i, sum = 0;
	// int				type;
	int				m_sz;
	char			*ptr;
	char			prt_buf[ 512];
	char			cmt_buf[ 512];
	// char			char_sz[8] = "1";
	STRUCT_FORM		*mp[ 512];
	int				mp_cnt = 0;
	STRUCT_FORM		*form;
	STRUCT_MEMBER	*member;

	fprintf( fp, "#include \"%s\"\n", st->f_name);
	fprintf( fp, "\n");

	form = Dll_GetFirstPtr( st->st_form);
	while( form != NULL)
	{
		if( strlen( form->type_name) <= 0)
		{
			mp[ mp_cnt++] = form;
			form = Dll_GetNextPtr( st->st_form);
			continue;
		}
		fprintf( fp, "int %s_Print( %s* ptr)\n", form->type_name, form->type_name);
		fprintf( fp, "{\n");

		if( form->m_cnt == 0)
		{
			for( i = 0; i < mp_cnt; i++)
			{
				LogDel( "mp[ i]->name=[%s] form->type_name=[%s]", mp[ i]->name, form->name);
				LogDel( "--- m_cnt=[%d]", mp[i]->m_cnt);

				if( !strncmp( mp[ i]->name, form->name, strlen( form->type_name)))
				{
					form->member = mp[ i]->member;
					form->m_cnt = mp[ i]->m_cnt;
					LogDel( "form->m_cnt=[%d]", form->m_cnt);
					break;
				}
			}

			/*
			fprintf( fp, "    return %s_Print( %s);\n", form->name, form->type_name);
			fprintf( fp, "}\n\n");
			form = Dll_GetNextPtr( st->st_form);
			continue;
			*/
		}
		LogDel( "m_cnt=[%d]", form->m_cnt);

		/* guide line */
		fprintf( fp, "    printf( \"%%s\", \"----");
		sz = strlen( form->type_name);
		fprintf( fp, "[ %s ]", form->type_name);
		for( i = 0; i < 80 - sz; i++)	fputc( '-', fp);
		fprintf( fp, "\\n\");\n");

		sum = 0;
		member = Dll_GetFirstPtr( form->member);
		while( member != NULL)
		{
			if( !memcmp( member->type, "    ", strlen( member->type)))
			{
				ptr = strchr( member->type, '*');
				if( ptr == NULL)
				{
					fprintf( fp, "    %s_Print( &ptr->%s);\n", member->type, member->name);
				}
				else
				{
					*ptr = 0;
					fprintf( fp, "    %s_Print( ptr->%s);\n", member->type, member->name);
				}
			}
			else
			if( !memcmp( member->type, "char", strlen( member->type)))
			{
				m_sz = StLoader_MemberSize( st, member->size);
				if( m_sz)
				{
					LogDel( "member->size=[%d]", m_sz);
					fprintf( fp, "    ");
					sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
					sprintf( prt_buf, "[%%.%ds]", m_sz);
					fprintf( fp, "printf( \"%s%-20s %3d %4d = %s\\n\", \tptr->%s);", 
							cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
					sum += m_sz;
					fprintf( fp, "\n");
				}
				else
				{
					LogDel( "member->size=[%d]", m_sz);
					m_sz = 1;
					fprintf( fp, "    ");
					sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
					sprintf( prt_buf, "[%%c]");
					fprintf( fp, "printf( \"%s%-20s %3d %4d = %s\\n\", \tptr->%s);", 
							cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
					sum += m_sz;
					fprintf( fp, "\n");
				}
			}
			else
			if( !memcmp( member->type, "int", strlen( member->type)))
			{
				m_sz = sizeof( int);
				LogDel( "member->size=[%d]", m_sz);
				fprintf( fp, "    ");
				sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
				sprintf( prt_buf, "[%%d]");
				fprintf( fp, "printf( \"%s%-20s %3d %4d = %s\\n\", \tptr->%s);", 
						cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
				sum += m_sz;
				fprintf( fp, "\n");
			}
			else
			if( !memcmp( member->type, "long", strlen( member->type)))
			{
				m_sz = sizeof( long);
				LogDel( "member->size=[%d]", m_sz);
				fprintf( fp, "    ");
				sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
				sprintf( prt_buf, "[%%ld]");
				fprintf( fp, "printf( \"%s%-20s %3d %4d = %s\\n\", \tptr->%s);", 
						cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
				sum += m_sz;
				fprintf( fp, "\n");
			}
			else
			if( !memcmp( member->type, "double", strlen( member->type)))
			{
				m_sz = sizeof( double);
				LogDel( "member->size=[%d]", m_sz);
				fprintf( fp, "    ");
				sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
				sprintf( prt_buf, "[%%f]");
				fprintf( fp, "printf( \"%s%-20s %3d %4d = %s\\n\", \tptr->%s);", 
						cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
				sum += m_sz;
				fprintf( fp, "\n");
			}
			else
			{
				ptr = strchr( member->type, '*');
				if( ptr == NULL)
				{
					fprintf( fp, "    %s_Print( &ptr->%s);\n", member->type, member->name);
				}
				else
				{
					*ptr = 0;
					fprintf( fp, "    %s_Print( ptr->%s);\n", member->type, member->name);
				}
			}







#if 0
			type = StLoader_CheckType( st, member->type);
			switch( type)
			{
				case 8 :
				case 7 :
				case 6 :
				case 5 :
				case 4 :
				case 3 :		/* char* */
				case 2 :		/* int */
					break;
				case 1 :		/* char */
					m_sz = StLoader_MemberSize( st, member->size);
					LogDel( "member->size=[%d]", m_sz);
					fprintf( fp, "    ");
					sprintf( cmt_buf, "%-30.28s", StLoader_CommentTrim( st, member->comment));
					sprintf( prt_buf, "[%%%d.%ds]", m_sz, m_sz);
					fprintf( fp, "printf( \"%s%-20s %3d %4d = %-12s\\n\", ptr->%s);", 
							cmt_buf, member->name, m_sz, sum, prt_buf, member->name);
					sum += m_sz;
					fprintf( fp, "\n");
					break;
				case 0 :
					ptr = strchr( member->type, '*');
					if( ptr == NULL)
					{
						fprintf( fp, "    %s_Print( &ptr->%s);\n", member->type, member->name);
					}
					else
					{
						*ptr = 0;
						fprintf( fp, "    %s_Print( ptr->%s);\n", member->type, member->name);
					}
					break;
				default:
					break;
			}
#endif
			member = Dll_GetNextPtr( form->member);
		}

		/* guide line */
		sz = strlen( form->type_name);
		fprintf( fp, "    printf( \"%%s\", \"");
		for( i = 0; i < 80 - sz; i++)	fputc( '-', fp);
		fprintf( fp, "[ %s ]", form->type_name);
		fprintf( fp, "----");
		fprintf( fp, "\\n\");\n");

		fprintf( fp, "\n");
		fprintf( fp, "    return sizeof( %s);\n", form->type_name);
		fprintf( fp, "}\n");
		fprintf( fp, "\n");
		form = Dll_GetNextPtr( st->st_form);
	}

	return 1;
}

char *StLoader_CommentTrim( ST_LOADER *st, char *comment)
{
	static char		rec[ 512];
	char			*ptr;
	int				cnt = 0, stat = 0;

	ptr = comment;

	if( comment == NULL)
	{
		rec[ 0] = 0;
		return rec;
	}

	while( *ptr != 0)
	{
		switch( *ptr)
		{
			case '\t' :
			case ' ' :
				if( stat/* == 1*/)
				{
					rec[ cnt++] = *ptr;
					stat++;
				}
				break;
			default  :
				rec[ cnt++] = *ptr;
				stat++;
				break;
		}
		ptr++;
	}

	rec[ cnt] = 0;

	return rec;
}

int StLoader_Print( ST_LOADER *st)
{
	// char			*ptr;
	STRUCT_FORM		*form;
	TYPEDEF_FORM	*type;

	type = Dll_GetFirstPtr( st->type);
	while( type != NULL)
	{
		printf( "%-20s", type->type);
		printf( "%-20s", type->name);
		printf( "\n");
		type = Dll_GetNextPtr( st->type);
	}

	form = Dll_GetFirstPtr( st->st_form);
	while( form != NULL)
	{
		printf( "%-20s", form->name);
		printf( "%-30s", form->type_name);
		printf( "%3d", form->m_cnt);
		printf( "\n");
		form = Dll_GetNextPtr( st->st_form);
	}

	return 1;
}

int StLoader_PrintMember( ST_LOADER *st, STRUCT_FORM *form)
{
	int				cnt = 0;
	STRUCT_MEMBER	*member;

	printf( "name        = [%s]\n", form->name);
	printf( "type_name   = [%s]\n", form->type_name);
	printf( "m_cnt       = [%d]\n", form->m_cnt);
	printf( "----------------------------------------------------------------------------------------------------------------\n");
	printf( "type                ");
	printf( "name                ");
	printf( "comment                                 ");
	printf( "cnt");
	printf( "\n");
	printf( "----------------------------------------------------------------------------------------------------------------\n");

	member = Dll_GetFirstPtr( form->member);
	while( member != NULL)
	{
		printf( "%-20s", member->type);
		printf( "%-20s", member->name);
		printf( "%-40.38s", member->comment);
		printf( "%3d ", member->s_cnt);
		cnt = 0;
		while( cnt < member->s_cnt)
		{
			printf( "%-5s ", member->size[ cnt]);
			cnt++;
		}
		printf( "\n");

		member = Dll_GetNextPtr( form->member);
	}
	printf( "----------------------------------------------------------------------------------------------------------------\n");

	return 1;
}





