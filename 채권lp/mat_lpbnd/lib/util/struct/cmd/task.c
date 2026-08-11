#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"
#include "cmd.h"
#include "task.h"

int	TaskOpen( int argc, char *argv[]);
int	TaskClose( int argc, char *argv[]);
int	TaskPrint( int argc, char *argv[]);
int	TaskTest( int argc, char *argv[]);
int	TaskTemp( int argc, char *argv[]);

int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

extern int 	LogPrint;

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"open",			TaskOpen,		"file_name", 	"header file open - struct load"},
	{	1,		"close",		TaskClose,		"none", 		"close struct loader"},
	{	1,		"print",		TaskPrint,		"none", 		"Format Print"		},
	{	1,		"test",			TaskTest,		"none", 		"Format Print"		},
	{	1,		"temp",			TaskTemp,		"none", 		"Format Print"		},
	{	5,		"help",			CmdHelp,		"none", 		"command display"		},
	{	99,		"quit",			CmdQuit,		"none", 		"stop process"		},
	{	-1,		"\0",			NULL,			"\0",			"\0"				}
};

ST_LOADER	*StLoaderPtr;


TaskOpen( int argc, char *argv[])
{
	char	*f_name;

	if( argc < 2)
	{
		LogCri( "Argument error. argc=[%d]", argc);
		LogApp( "Usage: %s file_name");
		return 0;
	}
	f_name = argv[ 1];

	StLoaderPtr = StLoader_Open( f_name);
	if( StLoaderPtr == NULL)
	{
		LogCri( "StLoader_Open error. name=[%s]", f_name);
		return 0;
	}
	LogDbg( "StLoader_Open success. name=[%s] ptr=[%p]", f_name, StLoaderPtr);

	return 1;
}


TaskClose( int argc, char *argv[])
{
	int			rtn;
	char		*f_name;

	LogDbg( "StLoader_Close. ptr=[%p]", StLoaderPtr);
	rtn = StLoader_Close( StLoaderPtr);
	StLoaderPtr = NULL;

	return 1;
}

CompFuncName( void *org, void *cmp, int sz)
{
	int				rtn, len;
	STRUCT_FORM		*form = ( STRUCT_FORM *)cmp;
	char			*str = ( char *)org;

	LogDbg( "HERE... name=[%s] form->name=[%s] form->type_name=[%s]", str, form->name, form->type_name);
	if( form->name != NULL)
	{
		LogDel( "HERE...1 name=[%s]", form->name);
		len = Max( strlen( form->name), sz);
		LogDel( "HERE...1-1");
		rtn =  memcmp( form->name, str, len);
		LogDel( "HERE...1-2");
	}
	else if( form->type_name != NULL)
	{
		LogDel( "HERE...2 type_name=[%p]", form->type_name);
		len = Max( strlen( form->type_name), sz);
		LogDel( "HERE...2-1 type_name=[%p]", form->type_name);
		rtn = memcmp( form->type_name, str, len);
		LogDel( "END...4 rtn=[%d]", rtn);
		return rtn;
	}
	else return 0;

	LogDel( "HERE...3");
	if( rtn != 0)
	{
		LogDel( "HERE...4");
		len = Max( strlen( form->type_name), sz);
		rtn =  memcmp( form->type_name, str, len);
	}
	LogDbg( "END...5 rtn=[%d]", rtn);
	return rtn;
}

TaskPrint( int argc, char *argv[])
{
	int				rtn, i;
	int				pos, sz;
	int				col_cnt = 120;
	int				name_sz = 50;
	int				type_sz = 4;
	int				size_sz = 4;
	int				pos_sz = 4;

	STRUCT_FORM		*form, *stsz;
	STRUCT_MEMBER	*member;

	form = Dll_GetFirstPtr( StLoaderPtr->st_form);
	while( form != NULL)
	{
		for( i = 0; i < col_cnt; i++) printf( "-"); printf( "\n");
		if( form->name != NULL) printf( "struct %s |", form->name);
		if( form->type_name) printf( "typedef %s |", form->type_name);
		printf( " %d member(s)", form->m_cnt);
		printf( "\n");
		for( i = 0; i < col_cnt; i++) printf( "-"); printf( "\n");
		printf( "    %-*s %-*s %-*s %-*s comment\n", 
				name_sz, "name",
				type_sz, "type",
				size_sz, "size",
				pos_sz, "pos"
				);
		for( i = 0; i < col_cnt; i++) printf( "-"); printf( "\n");
		pos = 0;
		sz = 0;
		member = Dll_GetFirstPtr( form->member);
		while( member != NULL)
		{
			printf( "    ");
			printf( "%-*s ", name_sz, member->name);
			printf( "%-*s ", type_sz, member->type);
			if( member->s_cnt > 0)
			{
				for( i = 0; i < member->s_cnt; i++)
				{
					printf( "%*s ", size_sz, member->size[i]);
					sz += atoi( member->size[ i]);
				}
			}
			else
			{
				printf( "%*s ", size_sz, "");
				/*
				LogDbg( "call.. Dll_GetFindFuncPtrLocal ptr=[%p]", StLoaderPtr->st_form);
				stsz = Dll_GetFindFuncPtrLocal( StLoaderPtr->st_form, member->type, strlen( member->name), CompFuncName);
				LogDbg( "find func stsz=[%p] name=[%s]", stsz, member->name);
				*/
			}
			printf( "%*d", pos_sz, pos);
			printf( "%s", member->comment);
			printf( "\n");
			pos += sz;
			member = Dll_GetNextPtr( form->member);
		}
		form = Dll_GetNextPtr( StLoaderPtr->st_form);
	}

	return 1;
}

TaskTest( int argc, char *argv[])
{
	StLoader_PrintFile( StLoaderPtr, stdout);
	return 0;
}

TaskTemp( int argc, char *argv[])
{
	int				rtn, i;
	int				pos, sz;
	int				col_cnt = 120;
	int				name_sz = 50;
	int				type_sz = 4;
	int				size_sz = 4;
	int				pos_sz = 4;

	STRUCT_FORM		*form, *stsz;
	STRUCT_MEMBER	*member;

	form = Dll_GetFirstPtr( StLoaderPtr->st_form);
	while( form != NULL)
	{
		if( form->name != NULL) printf( "name=[%s]\n", form->name);
		if( form->type_name) printf( "type_name=[%s]\n", form->type_name);

		member = Dll_GetFirstPtr( form->member);
		while( member != NULL)
		{
			printf( "\tname     =[%s]\n", member->name);
			printf( "\ttype     =[%s]\n", member->type);
			printf( "\tcomment  =[%s]\n", member->comment);
			if( member->s_cnt > 0)
			{
				for( i = 0; i < member->s_cnt; i++)
				{
					printf( "\tsize[%2d] =[%s]\n", i, member->size[i]);
				}
			}
			else
			{
				printf( "\tsize     =[0]\n");
			}
			member = Dll_GetNextPtr( form->member);
		}
		form = Dll_GetNextPtr( StLoaderPtr->st_form);
	}

	return 1;
}

CmdHelp( int argc, char *argv[])
{
	Cmd_IntHelp( Cmd, argc, argv);

	return 1;
}

CmdQuit( int argc, char *argv[])
{
	LogDel( "quit.............");

	exit( 1);
	return 1;
}

