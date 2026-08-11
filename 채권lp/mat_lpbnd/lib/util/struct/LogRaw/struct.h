#ifndef	_STRUCT_H_
#define	_STRUCT_H_	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dll.h"

#ifndef		MAX
#define		MAX( x, y)		(( x > y ) ? x : y)
#define		MIN( x, y)		(( x < y ) ? x : y)
#endif

#define		ST_BUF_SZ		8192

typedef struct _define_form_
{
	char	*name;
	char	*value;
}	DEFINE_FORM;

typedef struct	_typedef_form_
{
	char	*type;
	char	*name;
}	TYPEDEF_FORM;

typedef	struct _struct_member_
{
	char	*name;
	char	*type;
	char	**size;
	int		s_cnt;
	char	*comment;
}	STRUCT_MEMBER;

typedef struct _struct_form_
{
	char	*name;
	char	*type_name;						/* typedef name */
	DLL		*member;						/* STRUCT_MEMBER */
	int		m_cnt;
}	STRUCT_FORM;

typedef struct _struct_loader_
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

	DLL				*st_form;						/* STRUCT_FORM */
	int				st_cnt;

	DLL				*define;						/* DEFINE_FORM */
	int				d_cnt;

	DLL				*type;							/* DEFINE_FORM */
	int				t_cnt;

	char			*cmt;							/* comment temp */
	int				c_sz;

	STRUCT_FORM		*temp_form;
	STRUCT_MEMBER	*temp_member;

}	ST_LOADER;

/********************************************************************************************************
* function define
********************************************************************************************************/
TYPEDEF_FORM*	StLoader_Typedef( ST_LOADER *st);
STRUCT_FORM*	StLoader_Struct( ST_LOADER *st);
DEFINE_FORM*	StLoader_Define( ST_LOADER *st);
char*			StLoader_GetSize( ST_LOADER *st);
char*			StLoader_CommentTrim( ST_LOADER *st, char *comment);

#endif	/* _STRUCT_H_ */

/***** Module : struct.c *****/
int         StLoader();
ST_LOADER*  StLoader_Open( char *file_name);
int         StLoader_Close( ST_LOADER *st);
int         StLoader_LoadFile( ST_LOADER *st);
int         StLoader_GetLine( ST_LOADER *st);
int         StLoader_CheckKeyword( ST_LOADER *st, char *word, int sz);
int         StLoader_Start( ST_LOADER *st);
TYPEDEF_FORM* StLoader_Typedef( ST_LOADER *st);
STRUCT_FORM* StLoader_Struct( ST_LOADER *st);
int         StLoader_Member( ST_LOADER *st, STRUCT_FORM *form);
char*       StLoader_GetSize( ST_LOADER *st);
int         StLoader_Comment( ST_LOADER *st);
int         StLoader_CommentPlus( ST_LOADER *st);
int         StLoader_EndofLine( ST_LOADER *st);
int         StLoader_Push( ST_LOADER *st);
int         StLoader_Pop( ST_LOADER *st, char *data, int sz);
int         StLoader_StackClear( ST_LOADER *st);
int         StLoader_CheckType( ST_LOADER *st, char *type);
int         StLoader_PrintFile( ST_LOADER *st, FILE *fp);
char*       StLoader_CommentTrim( ST_LOADER *st, char *comment);
int         StLoader_Print( ST_LOADER *st);
int         StLoader_PrintMember( ST_LOADER *st, STRUCT_FORM *form);

