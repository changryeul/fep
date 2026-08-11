#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#define	 ON  1
#define  OFF 0

FILE	*fp, *fs;
char	buff[512];
char	out_file[ 512];

DIR				*Dir;
struct dirent	*DirEnt;
struct stat		Stat;


typedef struct _find_file_st_
{
	char	path[ 512];
	char 	name[ 512];
}	FIND_FILE;

FIND_FILE	FindFile;

char	*err =  "Find function in C-source file(s)\n"
		"\tUsage : cfunc -o <save file> <source file name> ...\n"
		"\t\t<source file name>  ; C-source file name, Ex. main.c *.c ...\n"
		"\t\t<save file> ; Saving file to the result\n";
char	*nocode = "\"+-=$%;!<>&?|";

main(int argc,char *argv[])
{
	int		rtn;

	if( argc < 2)
	{
		printf( "%s", err);
		return 0;
	}

	GetOption( argc, argv);

	rtn = InitProcess( argc, argv);
	if( rtn > 0)
	{
		rtn = MainProcess( argc, argv);
	}
	rtn = TermProcess( argc, argv);


}

GetOption( int argc, char *argv[])
{
	char	c, d;

	while( 1)
	{
		c = getopt( argc, argv, "o:");
		if( c <= 0) break;
		switch( c)
		{
			case 'o' :
				sprintf( out_file, "%s",  optarg);
				printf( "output file name=[%s]\n", out_file);
				fs = fopen( out_file, "a+");
				if( fs == NULL)
				{
					printf( "file open error. name=[%s]", out_file);
				}
				break;
			case '?' :
				if( optopt == 'o')	printf( "option -f requires save file_name\n");
				break;
			default  :
				printf( "%s", err);
				break;
		}
	}
	return 1;
}

InitProcess( int argc, char *argv[])
{
	return 1;
}

MainProcess( int argc, char *argv[])
{
	int			rtn;
	FIND_FILE	*ff = &FindFile;

	rtn = FindFirst( ".", ff);
	printf( "path=[%s]\n", ff->path);
	printf( "name=[%s]\n", ff->name);
}

TermProcess( int argc, char *argv[])
{
}


FindFirst( char *path, FIND_FILE *ff)
{
	int		rtn;
	char	name[ 512];

	Dir = ( DIR *)opendir( path);
	if( Dir == NULL) return -1;

	memmove( ff->path, path, strlen( path) +1);

	while( 1)
	{
		DirEnt = readdir( Dir);
		if( DirEnt == NULL)
		{
			closedir( Dir);
			return -1;
		}
		/*
		if( DirEnt->d_name[ 0] == '.') continue;
		*/
		printf( "%s/%s\n", path, DirEnt->d_name);
		sprintf( name, "%s/%s", path, DirEnt->d_name);
		rtn = lstat( name, &Stat);
		if( rtn < 0)
		{
			printf( "lstat error. name=[%s] err=[%d:%s]\n", name, errno, strerror( errno));
		}
		if( S_ISDIR( Stat.st_mode)) continue;
		memmove( ff->name, DirEnt->d_name, strlen( DirEnt->d_name) +1);
		return 0;
	}
}

FindNext( FIND_FILE *ff)
{
	char name[ 512];

	while( 1)
	{
		DirEnt = readdir( Dir);
		if( DirEnt == NULL)
		{
			closedir( Dir);
			return -1;
		}
		if( DirEnt->d_name[ 0] == '.') continue;
		sprintf( name, "%s/%s", ff->path, DirEnt->d_name);
		lstat( name, &Stat);
		if( S_ISDIR( Stat.st_mode)) continue;
		memmove( ff->name, DirEnt->d_name, strlen( DirEnt->d_name) +1);
		return 0;
	}
}

struct _Fn
{
	char fname[ 512];
	char arg[ 64][ 128];
	int  argcnt;
}	Func;


PrintFn( char *Buff)
{
	memmove( Func.fname, Buff, strlen( Buff) +1);
	Func.argcnt = 0;
	return 0;
}

PrintArg( char *Buff)
{
	int		i = 0;

	while( 1)
	{
		switch( Buff[ i])
		{
			case '\n':
			case '\0':
				return 0;
			case ' ' :
			case '\t':
				i++;
				continue;
			default  :
				break;
		}
		break;
	}

	memmove( Func.arg[ Func.argcnt], Buff, strlen( Buff) +1);
	Func.argcnt++;
	return 0;
}

PrintEnd( FILE *Fp)
{
	int		rtn;
	int 	i, j;
	int		Len = 0;
	int		SpcFlag = 0;

	rtn = WordCnt( Func.fname);
	if( rtn < 1)
	{
		fprintf( Fp, "int\t");
	}

	if( Func.argcnt == 0)
	{
		Len = strlen( Func.fname);
		for( i = 0; i < Len; i++)
			if( Func.fname[ i] == '\n') Func.fname[ i] = '\0';
		fprintf( Fp, "%s", Func.fname);
	}
	else
	{
		Len = strlen( Func.fname);
		for( i = 0; i < Len; i++)
		{
			fputc( Func.fname[ i], Fp);
			if( Func.fname[ i] == '(') break;
		}
		for( i = 0; i < Func.argcnt; i++)
		{
			Len = strlen( Func.arg[ i]);
			for( j = 0; j < Len; j++)
			{
				switch( Func.arg[ i][ j])
				{
					case ';'  : break;
					case '\n' :
					case ','  :
						fprintf( Fp, ",");
						WordPrint( Fp, Func.arg[ i]);
					case '\t' : 
					case ' '  : 
						SpcFlag = 1;
						continue;
					default   : 
						if( SpcFlag) 
						{
							fputc( ' ', Fp);
							SpcFlag = 0;
						}
						fputc( Func.arg[ i][ j], Fp); 
						continue;
				}
				break;
			}
			if( ( i +1) >= Func.argcnt) 
			{
				fprintf( Fp, ")");
				continue;
			}
			fprintf( Fp, ", ");
		}
	}
	fprintf( Fp, ";\n");
	return 0;
}

WordPrint( FILE *Fd, char *Str)
{
	int		i;
	char 	Buf[ 64];

	for( i = 0; i < 32; i++)
	{
		if( !isalnum( Str[ i])) break;
		Buf[ i] = Str[ i];
	}
	Buf[ i] = 0;

	fprintf( Fd, " %s", Buf);

	return 0;
}

WordCnt( char *p)
{
	int	cnt = 0;
	int	sf = 0;

	while( *p != 0)
	{
		switch( *p)
		{
			case '\t':
			case ' ' :
				sf = 1;
				break;
			case '\r':
			case '\n':
			case '\0':
			case '(' :
				if( sf) cnt++;
				return cnt;
			case '*' :
				cnt++;
				sf = 1;
				break;
			default  :
				if( sf) cnt++;
				sf = 0;
				break;
		}
		p++;
	}

	return cnt;
}





