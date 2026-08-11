/*
	####UNKNOWN   
	---------------------------------------
	int Func( 
		char A, // Commant 
		char B)
	---------------------------------------
	int Func( A, B)
	int A, B;
	---------------------------------------
	like printf format
	---------------------------------------
*/
	
	





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define	 ON  1
#define  OFF 0

FILE	*fp, *fs;
char	buff[512];
char	out_file[ 512];

struct ffblk
{
	char	Path[ 512];
	char 	ff_name[ 512];
};

char	Usage[1024] =  "Find function in C-source file(s)\n"
		"\tUsage : cfunc -o <save file> <source file name> ...\n"
		"\t\t<source file name>  ; C-source file name, Ex. main.c *.c ...\n"
		"\t\t<save file> ; Saving file to the result\n";
char	*nocode = "\"+-=$%;!<>&?|";

main(int argc,char *argv[])
{
	struct	ffblk ff;
	int		i,j,k;
	int		brace,bropen,brclose,extra,quotes;
	int		comment;
	int		ffind = 0, argcnt = 0;
	long	lines;
	char	c,d;
	char	*p;

	if( argc < 2)
	{
		printf( "%s", Usage);
		return 0;
	}
	fs = stdout;

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
				printf( "%s", Usage);
				break;
		}
	}

	while( optind < argc)
	{
		printf( "===== file=[%s]\n", argv[ optind]);
	do {
		fp=fopen(argv[optind],"r");
		if( fp == NULL) continue;

		fprintf( fs, "\n/*** Module : %s ***/\n",argv[optind]);
		brace = 0;
		lines = 0L;
		comment = OFF;
		while(1) 
		{
			p = fgets(buff,512,fp);
			if( p == NULL) break;
			/*
			printf( "comment(%d) brace(%d) quotes(%d)-%s", comment, brace, quotes, p);
			*/

			k = strlen(buff);
			lines++;
			if(brace == 0) 
			{ /* function not started */
				bropen = brclose = 0;
				extra = 0;
				for( j=0; j<k; j++) 
				{
					c = buff[j];
					if( c=='/' ) 
					{
						if(buff[j+1] == '*') 
						{
							comment = ON;
							j++;
						} 
						else if( buff[j-1] == '*') 
						{
							comment = OFF;
						}
					}
					if( comment == ON) continue;
					else if(c == '(') bropen++;
					else if(c == ')') brclose++;
					else if(c == '{') brace++;
					else if(c == '}') brace--;
					else 
					{
						i = 0;
						while(1) 
						{
							if( nocode[i] == 0) break;
							else if( c == nocode[i]) 
							{
								extra++;
							}
							i++;
						}
					}
				}
				if( brace == 0 && ffind == 1) /* CDC */
				{
					PrintArg( buff);
				}
				if( brace == 1 && ffind == 1) /* CDC */
				{
					PrintEnd( fs);
				}
				
				if(extra != 0) continue;
				if( bropen == 1 && brclose == 1 && brace == 0) {
					/* CDC find function */
					/* 
					fprintf(fs,"%5ld:%s",lines,buff);
					fprintf(fs,"%s",buff);
					*/

					PrintFn( buff);
					printf("%5ld:%s",lines,buff);
					ffind = 1;
					argcnt = 0;
				}
			}	/* if(brace == 0) */
			else 
			{
				ffind = 0;
				for(j=0;j<k;j++) 
				{
					if(buff[j] == '\'' || buff[j] == '\"') 
					{
						if( quotes == 1)	quotes=0;
						else				quotes=1;
					}
					if( quotes) continue;
					if( buff[j]=='/' ) 
					{
						if(buff[j+1] == '*') 
						{
							comment = ON;
							j++;
						} 
						else if(buff[j-1] == '*') 
						{
							comment = OFF;
						}
					}
					if(comment == ON) continue;
					if(buff[j] == '{') 
					{
						brace++;
					} 
					else if(buff[j] == '}') 
					{
						brace--;
					}
				}
			}
		}
		fclose(fp);
	} while( 0);
	optind++;
	}

	fprintf( fs, "\n");
	if( fs != stdout) fclose(fs);
}

DIR				*Dir;
struct dirent	*DirEnt;
struct stat		Stat;

findfirst( char *Path, struct ffblk *ff, int Option)
{
	char	Name[ 512];

	Dir = ( DIR *)opendir( Path);
	if( Dir == NULL) return -1;

	memmove( ff->Path, Path, strlen( Path) +1);

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
		printf( Name, "%s/%s", Path, DirEnt->d_name);
		sprintf( Name, "%s/%s", Path, DirEnt->d_name);
		lstat( Name, &Stat);
		if( S_ISDIR( Stat.st_mode)) continue;
		memmove( ff->ff_name, DirEnt->d_name, strlen( DirEnt->d_name) +1);
		return 0;
	}
}

findnext( struct ffblk *ff)
{
	char Name[ 512];

	while( 1)
	{
		DirEnt = readdir( Dir);
		if( DirEnt == NULL)
		{
			closedir( Dir);
			return -1;
		}
		if( DirEnt->d_name[ 0] == '.') continue;
		sprintf( Name, "%s/%s", ff->Path, DirEnt->d_name);
		lstat( Name, &Stat);
		if( S_ISDIR( Stat.st_mode)) continue;
		memmove( ff->ff_name, DirEnt->d_name, strlen( DirEnt->d_name) +1);
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
		fprintf( Fp, "int\t\t");
	}

	/*
	printf( "Func.fname                     = [%s]\n", Func.fname);
	printf( "Func.argcnt                    = [%d]\n", Func.argcnt);
	for( i = 0; i < Func.argcnt; i++)
	{
		printf( "Func.arg[%d]                 = [%s]\n", i, Func.arg[i]);
	}
	*/

	if( Func.argcnt == 0)
	{
		Len = strlen( Func.fname);
		for( i = 0; i < Len; i++)
			if( Func.fname[ i] == '\n') Func.fname[ i] = '\0';
#if 1
		fprintf( Fp, "%s", Func.fname);
#else
		PrintFunc( Fp, Func.fname);
#endif
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

PrintFunc( FILE *Fp, char *FuncName)
{
	int		stat = 0;
	char 	*ptr;
	char	buf[ 512];

	ptr = strchr( FuncName, '(');
	if( ptr == NULL)
	{
		fprintf( Fp, "%s", FuncName);
		return 1;
	}

	while( 1)
	{
		ptr--;
		switch( *ptr)
		{
			case ' ':
			case '*':
				if( stat >= 1) 
				{
					ptr++;
					memcpy( buf, ptr, strlen( ptr) +1);
					*ptr = 0;
					fprintf( Fp, "%s\t\t", FuncName);
					fprintf( Fp, "%s", buf);
					return 1;
				}
				break;
			default:
				stat++;
				break;
		}
	}

	return 0;
}





