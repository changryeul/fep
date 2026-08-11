/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "tree.h"

/** ***************************************************************************
**  GLOBAL
***************************************************************************** */

/** ***************************************************************************
**  @fn         int Tree_( TREE *Tree)
**  @param      TREE	*tree - tree pointer
**  @param      char	*path - 스캔할 기본 디렉토리
**  @param      FILE	*fp   - 저장할 파일 pointer
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  인수로 받은 디렉토리의 하위 디렉토리를 스캔하여 파일에 저장
***************************************************************************** */
int Tree_ScanDir( char *path, FILE *fp)
{
	int				rtn = 0;
	static int		depth = 0;
	char			name[ 512];
	DIR				*dirp;
	struct dirent	*dir_ent;
	struct stat	stat_buf;

	dirp = opendir( path);
	if( dirp == NULL)
	{
		return 0;
	}

	while( 1)
	{
		dir_ent = readdir( dirp);
		if( dir_ent == NULL)
		{
			break;
		}

		if( dir_ent->d_name[ 0] == '.') continue;

		sprintf( name, "%s/%s", path, dir_ent->d_name);
		rtn = stat( name, &stat_buf);
		if( rtn < 0)
		{
			return 0;
		}

		if( S_ISDIR( stat_buf.st_mode)) 
		{
			depth++;
			fprintf( fp, "%03d %s\n", depth,name);
			rtn = Tree_ScanDir( name, fp);
			depth--;
			if( rtn) break;
		}
		/*
		else if( stat_buf.st_mode & ( S_IFLNK & (~S_IFREG)))
		{
			fprintf( fp, "%02d ->%s\n", depth, name);
		}
		*/
	}
	closedir( dirp);

	return rtn;
}

/** ***************************************************************************
**  @fn         int Tree_( TREE *Tree)
**  @param      char *f_name   - tree view를 보여줄 디렉토리 list 파일
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  directory 파일 list data 를 tree view로 출력
***************************************************************************** */
int Tree_View( char *f_name)
{
	int				i;
	static int		depth = -1;
	static int		old_depth = 0;
	char			*ptr, *path, *name;
	FILE			*fp;
	char			line[ 512];

	fp = fopen( f_name, "r");
	if( fp == NULL)
	{
		return -1;
	}

	ptr = fgets( line, 512, fp);
	while( ptr != NULL)
	{
		line[ strlen( line) -1] = 0;
		old_depth = depth;
		depth = AtoI( line, 3);
		path = &line[ 4];

		if( depth == 0) 
		{
			if( old_depth != -1) fprintf( stderr, "\n");
			fprintf( stderr, "[%s]\n", path);
		}
		else
		{
			if( old_depth == 0)
			{
				for( i = 0; i < depth; i++) fprintf( stderr, "%11s", " ");
			}
			else
			if( depth <= old_depth) 
			{
				fprintf( stderr, "\n");
				for( i = 0; i < depth; i++) fprintf( stderr, "%11s", " ");
			}
			name = strrchr( path, '/');
			name++;
			fprintf( stderr, "-[%-8s]", name);
		}


		ptr = fgets( line, 512, fp);
	}

	fprintf( stderr, "\n");

	fclose( fp);

	return 1;
}

/** ***************************************************************************
**  @fn         int Tree_( TREE *Tree)
**  @param      char *f_name   - tree view를 보여줄 디렉토리 list 파일
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  directory 파일 list data 를 tree view로 출력
***************************************************************************** */
char **Tree_Search( char *f_name, int argc, char *argv[], int *nfind)
{
	char			*ptr, *path, *path_curr, *name;
	FILE			*fp;
	char			line[ 512];

	int				i, sz;
	char			**stack;
	int				stack_cnt = 0;

	fp = fopen( f_name, "r");
	if( fp == NULL)
	{
		LogErr( "fopen error. name=[%s]", f_name);
		return NULL;
	}

	stack = malloc( sizeof( char *) * ( stack_cnt +2));

	ptr = fgets( line, 512, fp);
	while( ptr != NULL)
	{
		line[ strlen( line) -1] = 0;
		path = &line[ 4];

		path_curr = path;
		for( i = 1; i < argc -1; i++)
		{
			LogDel( "%d %s", i, argv[ i]);
#if 1
			path_curr = strstr( path_curr, argv[ i]);
			if( path_curr == NULL) goto next_line;
#else
			/* 20250219 -- 시작하는 string 일치로 바꿈 */
			if( strncmp( path_curr, argv[ i], strlen( argv[ i]))) goto next_line;
#endif
		}

		name = strrchr( path, '/');
		name++;
		ptr = strstr( name, argv[ i]);
		if( ptr != NULL)
		{
			stack = realloc( stack, sizeof( char *) * ( stack_cnt +2));
			sz = strlen( path);
			stack[ stack_cnt] = malloc( sz +1);
			memcpy( stack[ stack_cnt], path, sz +1);
			stack_cnt++;
			stack[ stack_cnt] = NULL;
		}

		next_line:
		ptr = fgets( line, 512, fp);
	}

	if( stack_cnt > 0)
	{
		*nfind = stack_cnt;
	}
	else
	{
		*nfind = 0;
		free( stack);
		stack = NULL;
	}

	fclose( fp);

	return stack;
}


