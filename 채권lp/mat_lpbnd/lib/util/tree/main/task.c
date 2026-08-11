/** ***************************************************************************
**  @file       task.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  명령어 수행 프로그램
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tree.h>
#include "main.h"
#include "task.h"

/********** USER FUNCTION **********/
int	CmdTest( int argc, char *argv[]);
/***********************************/
int	CmdHelp( int argc, char *argv[]);
int	CmdQuit( int argc, char *argv[]);

CMD		*Cmd = NULL;
CMD_TBL	CmdTable[] =
{
	{	1,		"test",			CmdTest,		"none",					"command test"},
	{	100,	"help",			CmdHelp,		"none",					"help message"},
	{	101,	"quit",			CmdQuit,		"none",					"stop process"},
	{	102,	"exit",			CmdQuit,		"none",					"stop process"},
	{	-1,		"\0",			NULL,			"\0",					"\0"}
};

/********** USER DEFINE VALIABLE **********/
/******************************************/

/** ***************************************************************************
**  @function   CMD 사용을 위한 초기화  ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int TaskInit()
{
	Cmd = Cmd_Open( CmdTable);
	if( Cmd == NULL)
	{
		LogCri( "Cmd_Open error. cmd_table=[%p]", CmdTable);
		return -1;
	}

	return 1;
}

/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int CmdTest( int argc, char *argv[])
{
	if( argc < 1)
	{
		printf( "argument error.\n");
		Cmd_HelpCommand( Cmd, argv[ 0]);
		return 0;
	}

	LogDbg( "test.............");

	return 1;
}


/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int TaskCurrentTreeView( int argc, char *argv[])
{
	int		rtn;
	FILE	*fp;
	char	cwd[ 512];

	fp = fopen( Param->tmp_file, "w+");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->tmp_file);
		return -1;
	}
	getcwd( cwd, 512);
	LogDbg( "cwd=[%s]", cwd);
	fprintf( fp, "%03d %s\n", 0, cwd);
	Tree_ScanDir( cwd, fp);

	fclose( fp);

	rtn = Tree_View( Param->tmp_file);
	if( rtn < 0)
	{
		LogErr( "Tree_View error.");
		return -1;
	}
	fprintf( stdout, ".");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int TaskFindDir( int argc, char *argv[])
{
	int		i, no = 0;
	int		nfind = 0;
	char	**ptr;
	char	rec[ 512];

	ptr = Tree_Search( Param->dat_file, argc, argv, &nfind);
	if( ptr == NULL)
	{
		fprintf( stderr, "directory not found. name=[%s]\n", argv[ 1]);
		return -1;
	}

	if( nfind <= 0)
	{
		fprintf( stdout, ".");
		return 1;
	}
	else
	if( nfind > 1)
	{
		for( i = 0; ptr[ i] != NULL; i++)
		{
			fprintf( stderr, "%3d %s\n", i +1, ptr[ i]);
		}
		fprintf( stderr, "select change directory no:");
		fgets( rec, 512, stdin);
		no = atoi( rec) -1;
		if( no < 0)		sprintf( rec, ".");
		else 			memcpy( rec, ptr[ no], strlen( ptr[ no]) +1);
	}
	else
	{
		memcpy( rec, ptr[ 0], strlen( ptr[ 0]) +1);
	}

	fprintf( stdout, "%s", rec);

	for( i = 0; ptr[ i] != NULL; i++) free( ptr[ i]);
	free( ptr);

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int TaskTreeView( int argc, char *argv[])
{
	int		rtn;

	rtn = Tree_View( Param->dat_file);
	if( rtn < 0)
	{
		LogErr( "Tree_View error.");
		return -1;
	}
	fprintf( stdout, ".");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int TaskScanDir( int argc, char *argv[])
{
	int		i;
	FILE	*fp;

	fp = fopen( Param->dat_file, "w+");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", Param->dat_file);
		return -1;
	}

	for( i = 0; i < DirListCnt; i++)
	{
		fprintf( fp, "%03d %s\n", 0, DirList[ i]);
		Tree_ScanDir( DirList[ i], fp);
	}

	fclose( fp);

	fprintf( stdout, ".");

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */
int NotMainProcess( int argc, char *argv[])
{
	int		rtn = 0;
	int		i;
	FILE	*fp;
	char	**ptr;
	char	cwd[ 512];
	char	rec[ 512];
	int		no, nfind = 0;

	switch( Task)
	{
		case 0:		/* search directory */
			if( argc < 2 || argv[ 1][ 0] == '.')	/* tree view current directory */
			{
				fp = fopen( Param->tmp_file, "w+");
				if( fp == NULL)
				{
					LogErr( "fopen error. name=[%s]", Param->tmp_file);
					return -1;
				}
				getcwd( cwd, 512);
				LogDbg( "cwd=[%s]", cwd);
				fprintf( fp, "%03d %s\n", 0, cwd);
				Tree_ScanDir( cwd, fp);

				fclose( fp);

				rtn = Tree_View( Param->tmp_file);
				if( rtn < 0)
				{
					LogErr( "Tree_View error.");
					return -1;
				}
				fprintf( stdout, ".");
			}
			else						/* search directory */
			{
				ptr = Tree_Search( Param->dat_file, argc, argv, &nfind);
				if( ptr == NULL)
				{
					fprintf( stderr, "directory not found. name=[%s]\n", argv[ 1]);
					return -1;
				}

				if( nfind <= 0)
				{
					fprintf( stdout, ".");
					return 1;
				}
				else
				if( nfind > 1)
				{
					for( i = 0; ptr[ i] != NULL; i++)
					{
						fprintf( stderr, "%3d %s\n", i +1, ptr[ i]);
					}
					fprintf( stderr, "select change directory no:");
					fgets( rec, 512, stdin);
					no = atoi( rec) -1;
					if( no < 0)		sprintf( rec, ".");
					else 			memcpy( rec, ptr[ no], strlen( ptr[ no]) +1);
				}
				else
				{
					memcpy( rec, ptr[ 0], strlen( ptr[ 0]) +1);
				}

				fprintf( stdout, "%s", rec);

				for( i = 0; ptr[ i] != NULL; i++) free( ptr[ i]);
				free( ptr);
			}
			break;
		case 1:		/* tree view */
			rtn = Tree_View( Param->dat_file);
			if( rtn < 0)
			{
				LogErr( "Tree_View error.");
				return -1;
			}
			fprintf( stdout, ".");
			break;
		case 2:		/* scan directory */
			fp = fopen( Param->dat_file, "w+");
			if( fp == NULL)
			{
				LogErr( "fopen error. name=[%s]", Param->dat_file);
				return -1;
			}

			for( i = 0; i < DirListCnt; i++)
			{
				fprintf( fp, "%03d %s\n", 0, DirList[ i]);
				Tree_ScanDir( DirList[ i], fp);
			}

			fclose( fp);

			fprintf( stdout, ".");
			break;
		default:	/* unknown */
			break;
	}


	return rtn;
}


/** ***************************************************************************
**  @function   정의된 함수 ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */
int CmdHelp( int argc, char *argv[])
{
	int		rtn;

	if( argc >= 2)
	{
		rtn = Cmd_HelpCommand( Cmd, argv[ 1]);
		return rtn;
	}

	rtn = Cmd_IntHelp( Cmd, argc, argv);
	return rtn;
}

int CmdQuit( int argc, char *argv[])
{
	LogDbg( "quit.............");

	exit( 1);
	return 1;
}

