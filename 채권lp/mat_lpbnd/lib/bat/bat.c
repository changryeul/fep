/** ***************************************************************************
**	@file		bat.c
**	@date		2022/08/25
**	@author		최동춘
**	@version	V1.0.20220825
**	@brif		
**	배치 관리 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>

#include "mem.h"
#include "cfg.h"
#include "etc.h"
#include "bat.h"


/** ***************************************************************************
**	@fn			size_t Bat_GetSize( BAT *bat)
**	@param		none
**	@return		batch table shared memory size
**	@retval		-1   실패
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리 size를 반환 - 내부함수
***************************************************************************** */
size_t Bat_GetSize( int opt)
{
	size_t	sz;

	switch( opt)
	{
		case BAT_MEM_SZ:
			sz = ( sizeof( BAT_TBL) * BAT_MAX_TBL) + sizeof( BAT_STAT);
			break;
		case BAT_TBL_SZ:
			sz = ( sizeof( BAT_TBL) * BAT_MAX_TBL);
			break;
		default:
			sz = 0;
			break;
	}
	return sz;
}

/** ***************************************************************************
**	@fn			BAT *Bat_Malloc()
**	@param		none
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 구조체 allocation - 내부 함수
***************************************************************************** */
BAT *Bat_Malloc()
{
	BAT	*bat;

	bat = malloc( sizeof( BAT));
	if( bat == NULL)
	{
		LogErr( "malloc error.");
		return NULL;
	}
	memset( bat, 0x00, sizeof( BAT));

	return bat;
}

/** ***************************************************************************
**	@fn			int Bat_Free()
**	@param		BAT 구조체 pointer
**	@return		항상 1
**	@exception
**	@remark
**	@brief		
**	배치 구조체 free - 내부 함수
***************************************************************************** */
int Bat_Free( BAT *bat)
{
	free( bat);
	return 1;
}

/** ***************************************************************************
**	@fn			BAT *Bat_Open( char *f_name)
**	@param		batch 작업 명 ... NULL=admin
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 작업을 위한 초기화 작업
**	배치 공유 메모리 attach
***************************************************************************** */
BAT *Bat_Create( char *id)
{
	int		rtn, sz;
	BAT		*bat;

	bat = Bat_Malloc();
	if( bat == NULL)
	{
		LogErr( "Bat_Malloc error.");
		goto error;
	}

	sz = Bat_GetSize( BAT_MEM_SZ);

	bat->mem = Mem_Create( BAT_MEM_KEY, sz);
	if( bat->mem == NULL)
	{
		LogWar( "Mem_Create error. Try Mem_Open. key=[0x%08x] sz=[%d]", BAT_MEM_KEY, sz);
		bat->mem = Mem_Open( BAT_MEM_KEY);
		if( bat->mem == NULL)
		{
			LogCri( "Mem_Create and Mem_Open error. key=[0x%08x]", BAT_MEM_KEY);
			goto error_1;
		}
	}
	LogLib( "Mem_Create or Mem_Open success. ptr=[%p] key=[0x%08x] sz=[%d]", bat->mem, BAT_MEM_KEY, sz);

	bat->base = Mem_GetPtr( bat->mem);
	bat->curr = bat->base;
	bat->stat = ( BAT_STAT *)( ( char *)bat->curr + Bat_GetSize( BAT_TBL_SZ)); 
	time( &bat->stat->create);

	return bat;

	error_1:
		Bat_Free( bat);
	error:
		LogCri( "error. id=[%s]", id);
		return NULL;

}

/** ***************************************************************************
**	@fn			BAT *Bat_Open( char *f_name)
**	@param		batch 작업 명 ... NULL=admin
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 작업을 위한 초기화 작업
**	배치 공유 메모리 attach
***************************************************************************** */
BAT *Bat_Open( char *id)
{
	BAT		*bat;

	bat = Bat_Malloc();
	if( bat == NULL)
	{
		LogErr( "Bat_Malloc error.");
		goto error;
	}

	bat->mem = Mem_Open( BAT_MEM_KEY);
	if( bat->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", BAT_MEM_KEY);
		goto error_1;
	}

	bat->base = Mem_GetPtr( bat->mem);
	bat->curr = bat->base;
	bat->stat = ( BAT_STAT *)( ( char *)bat->curr + Bat_GetSize( BAT_TBL_SZ)); 

	LogLib( "bat->base=[%p]", bat->base);

	return bat;

	error_1:
		Bat_Free( bat);
	error:
		LogCri( "error. id=[%s]", id);
		return NULL;
}

/** ***************************************************************************
**	@fn			BAT *Bat_Open( char *f_name)
**	@param		batch 작업 명 ... NULL=admin
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 작업을 위한 초기화 작업
**	배치 공유 메모리 attach
***************************************************************************** */
BAT *Bat_OpenByCreat( char *id)
{
	int			rtn;
	BAT			*bat;
	BAT_TBL		*bp;

	bat = Bat_Open( id);
	if( bat == NULL)
	{
		LogErr( "Bat_Open( id=[%s]) error.", id);
		goto error;
	}

	bat->curr = Bat_FindId( bat, id);
	if( bat->curr == NULL)
	{
		LogCri( "Bat_OpenByCreat error. id not found. id=[%s]", id);
		goto error_1;
	}
	bp = bat->curr;

	bat->opt = 1;

	bat->curr->st_file = 1;
	bat->curr->cnt_file = 0;
	bat->curr->err_file = 0;
	time( &bat->curr->tm_file);
	sprintf( bp->msg_file, "%s", "파일 생성       ");

	rtn = Bat_OpenDataFile( bat, bp);
	if( rtn < 0)
	{
		Bat_SetStat( bat, 0, errno, "%s", "파일 생성 오류");
		LogCri( "Bat_OpenDataFile( bat=[%p], bat->curr=[%p]) error.", bat, bp);
		goto error_1;
	}

	return bat;

	error_1:
		Bat_Close( bat);
	error:
		LogCri( "error. id=[%s]", id);
		return NULL;
}

/** ***************************************************************************
**	@fn			BAT *Bat_Open( char *f_name)
**	@param		batch 작업 명 ... NULL=admin
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 작업을 위한 초기화 작업
**	배치 공유 메모리 attach
***************************************************************************** */
BAT *Bat_OpenBySend( char *id)
{
	int			rtn;
	BAT			*bat;
	BAT_TBL		*bp;

	bat = Bat_Open( id);
	if( bat == NULL)
	{
		LogErr( "Bat_Open( id=[%s]) error.", id);
		goto error;
	}

	bat->curr = Bat_FindId( bat, id);
	if( bat->curr == NULL)
	{
		LogCri( "Bat_OpenByCreat error. id not found. id=[%s]", id);
		goto error_1;
	}
	bp = bat->curr;

	bat->opt = 2;

	bat->curr->st_send = 1;
	bat->curr->cnt_send = 0;
	bat->curr->err_send = 0;
	time( &bat->curr->tm_send);
	sprintf( bp->msg_send, "%s", "파일 오픈       ");

	rtn = Bat_OpenDataFile( bat, bp);
	if( rtn < 0)
	{
		Bat_SetStat( bat, 0, errno, "%s", "파일 오픈 오류");
		LogCri( "Bat_OpenDataFile( bat=[%p], bat->curr=[%p]) error.", bat, bp);
		goto error_1;
	}

	return bat;

	error_1:
		Bat_Close( bat);
	error:
		LogCri( "error. id=[%s]", id);
		return NULL;
}

/** ***************************************************************************
**	@fn			BAT *Bat_Open( char *f_name)
**	@param		batch 작업 명 ... NULL=admin
**	@return		BAT 구조체 pointer
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	배치 작업을 위한 초기화 작업
**	배치 공유 메모리 attach
***************************************************************************** */
BAT *Bat_OpenByRecv( char *id)
{
	int			rtn;
	BAT			*bat;
	BAT_TBL		*bp;
	char		code[ 32];

	sprintf( code, "FX%s", &id[2]);

	bat = Bat_Open( code);
	if( bat == NULL)
	{
		LogErr( "Bat_Open( id=[%s]) error.", id);
		goto error;
	}

	bat->curr = Bat_FindId( bat, code);
	if( bat->curr == NULL)
	{
		LogCri( "Bat_OpenByRecv error. id not found. id=[%s]", code);
		goto error_1;
	}
	bp = bat->curr;

	bat->opt = 3;

	bat->curr->st_recv = 1;
	bat->curr->cnt_recv = 0;
	bat->curr->err_recv = 0;
	time( &bat->curr->tm_recv);
	sprintf( bp->msg_recv, "%s", "파일 오픈       ");

	rtn = Bat_OpenDataFile( bat, bp);
	if( rtn < 0)
	{
		Bat_SetStat( bat, 0, errno, "%s", "파일 오픈 오류");
		LogCri( "Bat_OpenDataFile( bat=[%p], bat->curr=[%p]) error.", bat, bp);
		goto error_1;
	}

	return bat;

	error_1:
		Bat_Close( bat);
	error:
		LogCri( "error. id=[%s]", id);
		return NULL;
}

/** ***************************************************************************
**	@fn			int BatTbl_OpenDataFile( BAT_TBL *tp)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		fd   file descripter
**	@retval		-1   오류
**	@exception
**	@remark
**	@brief		
**	배치 data 파일을 OPEN
**	tp->stat에 따라서 파일 open 모드가 달라집니다.
**		tp->stat == 0 -> O_CREAT + O_RDWR
**		tp->stat == 1 -> O_RDONLY
**	따라서 배치파일을 생성 하려면 먼저 Bat_SetStat() 함수를 불러 tp->stat를
**	변경한 후 Bat_OpenDataFile을 호출 하십시요
***************************************************************************** */
int Bat_OpenDataFile( BAT *bat, BAT_TBL *tp)
{
	time_t	cur_time;
	char	f_name[ 512];
	int		opt;

	switch( bat->opt)
	{
		case 1:
			opt = O_RDWR + O_CREAT + O_TRUNC;
			time( &cur_time);
			memcpy( tp->nm_file, tp->name, strlen( tp->name) +1);
			TtoA( tp->nm_file, cur_time);
			tp->stat = 1;
			tp->st_file = 1;
			tp->cnt_file = 0;
			time( &tp->tm_file);
			sprintf( f_name, "%s/%s", bat->stat->path, tp->nm_file);
			break;
		case 2:
			opt = O_RDONLY;
			tp->stat = 3;
			tp->st_send = 1;
			tp->cnt_send = 0;
			memcpy( tp->nm_send, tp->nm_file, strlen( tp->nm_file) +1);
			time( &tp->tm_send);
			sprintf( f_name, "%s/%s", bat->stat->path, tp->nm_send);
			break;
		case 3:
			opt = O_RDWR + O_CREAT + O_TRUNC;
			memcpy( tp->nm_recv, tp->nm_send, strlen( tp->nm_send) +1);
			memcpy( tp->nm_recv, "MS", 2);
			tp->stat = 5;
			tp->st_recv = 1;
			tp->cnt_recv = 0;
			time( &tp->tm_recv);
			sprintf( f_name, "%s/%s", bat->stat->path, tp->nm_recv);
			break;
	}

	bat->fd = open( f_name, opt, 0664);
	if( bat->fd < 0)
	{
		LogErr( "batch file open error. name=[%s]", f_name);
		goto error;
	}
	LogLib( "Bat_OpenDataFile name=[%s] fd=[%d]", f_name, bat->fd);

	return bat->fd;
	error:
		return -1;
}

/** ***************************************************************************
**	@fn			int Bat_Close( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리 detach
**	구조체 free
***************************************************************************** */
int Bat_Close( BAT *bat)
{
	if( bat->fd > 0) close( bat->fd);

	Mem_Close( bat->mem);
	Bat_Free( bat);

	return 1;
}

/** ***************************************************************************
**	@fn			int Bat_Close( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	bok_df에서 생성된 파일 이름을 return
***************************************************************************** */
char *Bat_GetFileName( BAT *bat, char *id)
{
	BAT_TBL		*bt;

	bt = Bat_FindId( bat, id);
	if( bt == NULL)
	{
		LogCri( "Bat_FindId error. id not found. id=[%s]", id);
		return NULL;
	}

	return bt->nm_file;
}

/** ***************************************************************************
**	@fn			int Bat_LoadSwapFile( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    성공
**	@retval		-1   실패
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리로 swap 파일을 로드합니다. 
**	BAT_STAT->swap_path를 참조
***************************************************************************** */
int Bat_LoadSwapFile( BAT *bat, char *f_name)
{
	int		rtn;
	int		fd, sz;
	BAT_TBL	*ptr;

	sz = Bat_GetSize( BAT_MEM_SZ);
	ptr = malloc( sz);
	if( ptr == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz);
		goto error;
	}

	fd = open( f_name, O_RDWR);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", f_name);
		goto error_1;
	}

	rtn = read( fd, ptr, sz);
	if( rtn < sz)
	{
		LogErr( "swap file read error. SWAP 파일의 공유 메모리 크기가 다릅니다. sz=[%d] rtn=[%d]", sz, rtn);
		goto error_2;
	}

	memcpy( bat->base, ptr, sz);
	LogMsg( "SWAP 파일을 배치 공유메모리로 로드하였습니다. name=[%s]", f_name);
	free( ptr);
	close( fd);

	return 1;

	error_2:
		close( fd);
	error_1:
		free( ptr);
	error:
		LogCri( "Swap load error. bat=[%p] name=[%s]", bat, f_name);
		return -1;
}

/** ***************************************************************************
**	@fn			int Bat_SaveSwapFile( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    성공
**	@retval		-1   실패
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리로 swap 파일을 로드합니다. 
**	BAT_STAT->swap_path를 참조
***************************************************************************** */
int Bat_SaveSwapFile( BAT *bat)
{
	int		rtn;
	int		fd, sz;
	BAT_TBL	*ptr;

	fd = open( bat->stat->swp_path, O_RDWR | O_CREAT, 0664);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", bat->stat->swp_path);
		goto error;
	}

	rtn = write( fd, bat->base, BAT_MEM_SZ);
	if( rtn < BAT_MEM_SZ)
	{
		LogErr( "swap file read error. SWAP 파일의 공유 메모리 크기가 다릅니다. sz=[%d] rtn=[%d]", BAT_MEM_SZ, rtn);
		goto error_1;
	}

	LogMsg( "공유메모리 저장. name=[%s] sz=[%d]", bat->stat->swp_path, BAT_MEM_SZ);

	close( fd);
	return 1;

	error_1:
		close( fd);
	error:
		LogCri( "Swap save error. bat=[%p] name=[%s]", bat, bat->stat->swp_path);
		return -1;
}

/** ***************************************************************************
**	@fn			int Bat_LoadConfig( BAT *bat, char *f_name, int flag)
**	@param		BAT 구조체 pointer
**	@param		config file name
**	@param		load flag 1:항상로드 0:config파일의 load_config값 참조해서 load
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	지정된 config파일을 읽어 공유 메모리에 적용 합니다.
**	flag 값이 1이면 config값을 항상 로드합니다.
**	flag 값이 0이면 config 파일 내의 "load_config" 값을 참조하여 
**	load_config 값이 1이면 공유메모리로 로드합니다.
***************************************************************************** */
int Bat_LoadConfig( BAT *bat, char *f_name, int flag)
{
	int			rtn;
	CFG			*cfg;
	void		*ptr;
	BAT_TBL		*tbl;
	int			load_config;

	cfg = Cfg_Open( f_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", f_name);
		goto error;
	}

	if( flag == 0)
	{
		load_config = Cfg_GetInt( cfg, "load_config");
		if( load_config <= 0)
		{
			LogLib( "Not check config. load_config=[%d]", load_config);
			Cfg_Close( cfg);
			return 1;
		}
	}
	LogLib( "Check config file. name=[%s]", f_name);


	Cfg_Get( cfg, "path",     bat->stat->path,     sizeof( bat->stat->path));
	Cfg_Get( cfg, "swp_name", bat->stat->swp_path, sizeof( bat->stat->swp_path));
	Cfg_Get( cfg, "cfg_name", bat->stat->cfg_path, sizeof( bat->stat->cfg_path));
	Cfg_Get( cfg, "version",  bat->stat->version,  sizeof( bat->stat->version));
	bat->stat->daily = Cfg_GetInt( cfg, "daily_time");

	ptr = Cfg_GetFirstNamePtr( cfg, "BATCH_TABLE");
	while( ptr != NULL)
	{
		tbl = Bat_MakeTbl( bat, ptr);
		rtn = Bat_WriteTbl( bat, tbl);

		ptr = Cfg_GetNextNamePtr( cfg, "BATCH_TABLE");
	}
	Cfg_Close( cfg);

	return 1;

	error_1:
		Cfg_Close( cfg);
	error:
		LogCri( "환경파일 로드에 실패하였습니다. name=[%s]", f_name);
		return -1;
}

/** ***************************************************************************
**	@fn			int Bat_LoadConfig( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리 detach
**	구조체 free
***************************************************************************** */
BAT_TBL *Bat_MakeTbl( BAT *bat, char *str)
{
	int				stat = 0;
	static BAT_TBL	b_tbl;
	BAT_TBL			*tbl = &b_tbl;
	char			rec[ 512];
	char			*ptr, *token = " :\n";

	memset( tbl, 0x00, sizeof( BAT_TBL));
	memcpy( rec, str, strlen( str) +1);

	ptr = strtok( rec, token);
	while( ptr != NULL)
	{
		switch( stat)
		{
			case 0:		/* batch job number */
				tbl->no = atoi( ptr);
				break;
			case 1:		/* job process flag */
				tbl->job = atoi( ptr);
				break;
			case 2:		/* batch id */
				Trim( ptr);
				memcpy( tbl->id, ptr, strlen( ptr) +1);
				break;
			case 3:		/* batch file name */
				Trim( ptr);
				memcpy( tbl->name, ptr, strlen( ptr) +1);
				break;
			case 4:		/* start time */
				tbl->str_time = atoi( ptr);
				break;
			case 5:		/* end time */
				tbl->end_time = atoi( ptr);
				break;
			case 6:		/* error time */
				tbl->err_time = atoi( ptr);
				break;
			default:
				break;
		}
		ptr = strtok( NULL, token);
		stat++;
	}

	return tbl;
}

/** ***************************************************************************
**	@fn			int Bat_WriteTbl( BAT *bat)
**	@param		BAT 구조체 pointer
**	@return		1    레코드 추가
**	@retval		0    레코드 수정
**	@retval		-1   에러
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리 detach
**	구조체 free
***************************************************************************** */
int Bat_WriteTbl( BAT *bat, BAT_TBL *tbl)
{
	int			add_flag = 0;
	BAT_TBL		*tp;

	tp = Bat_FindId( bat, tbl->id);
	if( tp == NULL)
	{
		LogMsg( "BAT_TBL not found. 배치 테이블을 추가합니다. id=[%s]", tbl->id);
		add_flag = 1;
	}

	if( add_flag)
	{
		tp = &bat->base[ bat->stat->cnt];
		memcpy( tp, tbl, sizeof( BAT_TBL));
		bat->stat->cnt++;
		return 1;
	}

	if( tp->no != tbl->no)
	{
		LogMsg( "배치 테이블 no를 수정합니다. id=[%s] old=[%d] new=[%d]", tp->id, tp->no, tbl->no);
		tp->no = tbl->no;
	}
	if( tp->job != tbl->job)
	{
		LogMsg( "배치 테이블 job을 수정합니다. id=[%s] old=[%d] new=[%d]", tp->id, tp->job, tbl->job);
		tp->job = tbl->job;
	}
	if( memcmp( tp->name, tbl->name, Max( strlen( tbl->name), strlen( tp->name)) +1))
	{
		LogMsg( "배치 테이블 name을 수정합니다. id=[%s] old=[%s] new=[%s]", tp->id, tp->name, tbl->name);
		memcpy( tp->name, tbl->name, strlen( tbl->name) +1);
	}
	if( tp->err_time != tbl->err_time)
	{
		LogMsg( "배치 테이블 err_time을 수정합니다. id=[%s] old=[%d] new=[%d]", tp->id, tp->err_time, tbl->err_time);
		tp->err_time = tbl->err_time;
	}
	if( tp->str_time != tbl->str_time)
	{
		LogMsg( "배치 테이블 str_time(시작시간)을 수정합니다. id=[%s] old=[%d] new=[%d]", tp->id, tp->str_time, tbl->str_time);
		tp->str_time = tbl->str_time;
	}
	if( tp->end_time != tbl->end_time)
	{
		LogMsg( "배치 테이블 end_time(종료시간)을 수정합니다. id=[%s] old=[%d] new=[%d]", tp->id, tp->end_time, tbl->end_time);
		tp->end_time = tbl->end_time;
	}

	return 0;

}

/** ***************************************************************************
**	@fn			BAT_TBL *Bat_FindId( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		BAT_TBL 구조체 pointer
**	@exception
**	@remark
**	@brief		
**	배치 공유 메모리 detach
**	구조체 free
***************************************************************************** */
BAT_TBL *Bat_FindId( BAT *bat, char *id)
{
	int		i;
	int		sz, cmp_sz;
	BAT_TBL	*tp;

	sz = strlen( id);
	for( i = 0; i < bat->stat->cnt; i++)
	{
		tp = &bat->base[ i];
		cmp_sz = Max( sz, strlen( tp->id));
		if( !memcmp( tp->id, id, cmp_sz +1)) return tp;
	}

	return NULL;
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_STAT 구조체 내용 조회
***************************************************************************** */
int Bat_PrintStat( BAT *bat)
{
	printf( "create               = [%s]\n", TtoS( bat->stat->create));
	printf( "daily time           = [%d]\n", bat->stat->daily);
	printf( "daily process time   = [%s]\n", TtoS( bat->stat->daily_at));
	printf( "file path            = [%s]\n", bat->stat->path);
	printf( "swap path            = [%s]\n", bat->stat->swp_path);
	printf( "config path          = [%s]\n", bat->stat->cfg_path);
	printf( "version              = [%s]\n", bat->stat->version);
	printf( "cnt                  = [%d]\n", bat->stat->cnt);
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_PrintTable( BAT *bat)
{
	int		i;
	int		column = 128;
	BAT_TBL	*tp;

	for( i = 0; i < column; i++) putchar( '-'); putchar( '\n');
	printf( "no  id           j name                      file                      stat stat_time           err_tm errno r_sz   cnt\n");
	for( i = 0; i < column; i++) putchar( '-'); putchar( '\n');

	for( i = 0; i < bat->stat->cnt; i++)
	{
		tp = &bat->base[ i];
		printf( "%3d ", tp->no);
		printf( "%-12s ", tp->id);
		printf( "%1d ", tp->job);
		printf( "%-25s ", tp->name);
		printf( "%-25s ", tp->file);
		printf( "%4d ", tp->stat);
		printf( "%s ", TtoS( tp->stat_time));
		printf( "%06d ", tp->err_time);
		printf( "%5d ", tp->bat_errno);
		printf( "%4d ", tp->rec_sz);
		printf( "\n");
	}
	for( i = 0; i < column; i++) putchar( '-'); putchar( '\n');

	return 1;
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_PrintBatTbl( BAT *bat, BAT_TBL *bp, int flag)
{
	if( flag == 0) flag = 0xffffffff;

	if( flag & 0x80000000)	printf( "----------------------------------------------------------------\n");
	if( flag & 0x00000001)	printf( "   table pointer        = [%p]\n", bp);
	if( flag & 0x00000002)	printf( "cm cmd                  = [%d]\n", bp->job);
	if( flag & 0x00000004)	printf( "id id                   = [%s]\n", bp->id);
	if( flag & 0x00000008)	printf( "na name                 = [%s]\n", bp->name);
	if( flag & 0x00000010)	printf( "fi file                 = [%s]\n", bp->file);
	if( flag & 0x00000020)	printf( "   record size          = [%d]\n", bp->rec_sz);
	if( flag & 0x00000100)	printf( "bs batch stat           = [%d]\n", bp->stat);
	if( flag & 0x00000200)	printf( "   batch stat time      = [%s]\n", TtoS( bp->stat_time));
	if( flag & 0x00000400)	printf( "st batch start time     = [%d]\n", bp->str_time);
	if( flag & 0x00000800)	printf( "et batch end time       = [%d]\n", bp->end_time);
	if( flag & 0x00001000)	printf( "er batch error time     = [%d]\n", bp->err_time);
	if( flag & 0x00002000)	printf( "en batch error number   = [%d]\n", bp->bat_errno);
	if( flag & 0x00004000)	printf( "em batch error message  = [%s]\n", bp->bat_errmsg);
	if( flag & 0x80000000)	printf( "----------------------------------------------------------------\n");
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_GetCnt( BAT *bat)
{

	switch( bat->opt)
	{
		case 1:	/* file create */
			return bat->curr->cnt_file;
		case 2:	/* file send */
			return bat->curr->cnt_send;
		case 3:	/* file recv */
			return bat->curr->cnt_recv;
		default:
			break;
	}
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_GetFileCnt( BAT *bat, BAT_TBL *bp)
{
	return bp->cnt_file;
}

int Bat_GetSendCnt( BAT *bat, BAT_TBL *bp)
{
	return bp->cnt_send;
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_GetCntFile( BAT *bat)
{
	return bat->curr->cnt_file;
}

/** ***************************************************************************
**	@fn			int Bat_PrintStat( BAT *bat, char *id)
**	@param		BAT 구조체 pointer
**	@return		1    항상
**	@exception
**	@remark
**	@brief		
**	BAT_TBL 구조체 내용 조회
***************************************************************************** */
int Bat_SetBatTbl( BAT *bat, BAT_TBL *bp, char *value, int flag)
{
	int		sz;

	sz = strlen( value) +1;

	switch( flag)
	{
		case 0x00000002 :	bp->job = atoi( value);	break;
		case 0x00000004 :	memcpy( bp->id, value, sz);	break;
		case 0x00000008 :	memcpy( bp->name, value, sz);	break;
		case 0x00000010 :	memcpy( bp->file, value, sz);	break;
		case 0x00000100 :	bp->stat = atoi( value);	break;
		case 0x00000400 :	bp->str_time = atoi( value);	break;
		case 0x00000800 :	bp->end_time = atoi( value);	break;
		case 0x00001000 :	bp->err_time = atoi( value);	break;
		case 0x00002000 :	bp->bat_errno = atoi( value);	break;
		case 0x00004000 :	memcpy( bp->bat_errmsg, value, sz);	break;
		default:
			break;
	}
	return 1;
}

/** ***************************************************************************
**	@fn			int BatTbl_SetStat( BAT_TBL *tp, int stat)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@param		배치 처리상태
**	@return		1    항상
**	@retval		-1   오류
**	@exception
**	@remark
**	@brief		
**	배치 처리상태를 저장
**	BAT_D2FS  - db to file start
**	BAT_D2FE  - db to file end
**	BAT_S2BS  - send to bok start
**	BAT_S2BE  - send to bok end
**	BAT_R2FS  - receive to file start
**	BAT_R2FE  - receive to file end
**	BAT_L2DS  - load to db start
**	BAT_L2DE  - load to db end
***************************************************************************** */
int Bat_SetStat( BAT *bat, int stat, int err, char *format, ...)
{
	va_list		args;
	int			sz, rtn;
	char		buf[ 128];
	BAT_TBL		*bp;

	va_start( args, format);
	sz = vsnprintf( buf, 128, format, args);
	va_end( args);

	bp = bat->curr;

	switch( bat->opt)
	{
		case 1:	/* file create */
			bp->st_file = stat;
			bp->err_file = err;
			time( &bp->tm_file);
			memcpy( bp->msg_file, buf, sz +1);
			break;
		case 2:	/* file send */
			bp->st_send = stat;
			bp->err_send = err;
			time( &bp->tm_send);
			memcpy( bp->msg_send, buf, sz +1);
			break;
		case 3:	/* file recv */
			bp->st_recv = stat;
			bp->err_recv = err;
			time( &bp->tm_recv);
			memcpy( bp->msg_recv, buf, sz +1);
			break;
		default:
			break;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			int Bat_CloseDataFile( BAT *bat, BAT_TBL *tp)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   항상
**	@exception
**	@remark
**	@brief		
**	배치 data를 저장할 파일을 OPEN
***************************************************************************** */
int Bat_CloseDataFile( BAT *bat, BAT_TBL *tp, int err_no, char *msg)
{
	BAT_TBL		*bp;

	close( bat->fd);
	bat->fd = -1;

	if( tp == NULL) bp = bat->curr;
	else			bp = tp;

	Bat_ErrMessage( bat, bp, err_no, msg);
	if( err_no != 0) return 1;

	if( bp->stat == 1) bp->stat = 2;
	else
	if( bp->stat == 3) bp->stat = 4;
	time( &bp->stat_time);

	return 1;
}

/** ***************************************************************************
**	@fn			int Bat_CloseDataFile( BAT *bat, BAT_TBL *tp)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   항상
**	@exception
**	@remark
**	@brief		
**	배치 data를 저장할 파일을 OPEN
***************************************************************************** */
int Bat_Write( BAT *bat, char *data, int sz)
{
	int		rtn;
	BAT_TBL	*bp;

	bp = bat->curr;

	rtn = write( bat->fd, data, sz);
	if( rtn < sz)
	{
		LogErr( "batch error.  write( bat->fd=[%d], data=[%p], sz=[%d]) error. rtn=[%d]", bat->fd, data, sz, rtn);
		return -1;
	}
	rtn = write( bat->fd, "\n", 1);
	switch( bat->opt)
	{
		case 1:	bp->cnt_file++;	break;
		case 2: bp->cnt_send++;	break;
		case 3: bp->cnt_recv++;	break;
		default:				break;
	}
	LogDump( data, sz, "Batch write. id=[%s] opt=[%d] fd=[%d] cnt=[%d]", bp->id, bat->opt, bat->fd, bp->cnt_file);

	return rtn;
}

/** ***************************************************************************
**	@fn			int Bat_CloseDataFile( BAT *bat, BAT_TBL *tp)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   항상
**	@exception
**	@remark
**	@brief		
**	배치 data를 저장할 파일을 OPEN
***************************************************************************** */
int Bat_Read( BAT *bat, char *data, int sz)
{
	int		rtn;
	char	rec[ 512];
	BAT_TBL	*bp;

	bp = bat->curr;

	LogApp( "Bat_ReadDataFile read data=[%p] sz=[%d] fd=[%d]", data, sz, bat->fd);
	rtn = read( bat->fd, data, sz);
	if( rtn < sz)
	{
		if( rtn < 0)	LogErr( "batch file read error. fd=[%d]", bat->fd);
		else			LogApp( "batch file read [EOF]. fd=[%d] rtn=[%d]", bat->fd, rtn);
		return -1;
	}
	LogApp( "Bat_ReadDataFile read rtn=[%d]", rtn);
	rtn = read( bat->fd, rec, 1);

	switch( bat->opt)
	{
		case 1:	bp->cnt_file++;	break;
		case 2: bp->cnt_send++;	break;
		case 3: bp->cnt_recv++;	break;
		default:				break;
	}

	return sz;
}

/** ***************************************************************************
**	@fn			int Bat_CloseDataFile( BAT *bat, BAT_TBL *tp)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   항상
**	@exception
**	@remark
**	@brief		
**	배치 data를 저장할 파일을 OPEN
***************************************************************************** */
int Bat_ReadDataFile( BAT *bat, BAT_TBL *tp, char *data, int sz)
{
	int		rtn;
	char	rec[ 512];

	LogApp( "Bat_ReadDataFile read data=[%p] sz=[%d] fd=[%d]", data, sz, bat->fd);
	rtn = read( bat->fd, data, sz);
	if( rtn < sz)
	{
		if( rtn < 0)	LogErr( "batch file read error. fd=[%d]", bat->fd);
		else			LogMsg( "batch file read [EOF]. fd=[%d] rtn=[%d]", bat->fd, rtn);
		return -1;
	}
	LogApp( "Bat_ReadDataFile read rtn=[%d]", rtn);
	rtn = read( bat->fd, rec, 1);
	tp->cnt_send++;

	return sz;
}

/** ***************************************************************************
**	@fn			int Bat_ErrMessage( BAT *bat, BAT_TBL *bt, int err_no, char *msg)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   SUCCESS
**	@retval		-1  ERROR
**	@exception
**	@remark
**	@brief		
**	공유메모리 테이블에 에러 메세지 등록
***************************************************************************** */
int Bat_ErrMessage( BAT *bat, BAT_TBL *bt, int err_no, char *msg)
{
	bt->bat_errno = err_no;
	memcpy( bt->bat_errmsg, msg, strlen( msg) +1);

	return 1;
}

/** ***************************************************************************
**	@fn			BAT_TBL *Bat_CheckSendStat( BAT *bat)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   SUCCESS
**	@retval		-1  ERROR
**	@exception
**	@remark
**	@brief		
**	모든 배치 작업에 대하여 전송 가능한 생태인지 체크한다
**  배치 테이블 처음부터 검생하여 전송가능한 상태인 테이블을 발견하면
**	해당 배치 테이블포인터를 리턴 합니다.
***************************************************************************** */
BAT_TBL *Bat_CheckSendStat( BAT *bat)
{
	int			i;
	time_t		cur_time;
	int			int_time;
	struct tm	*tp;
	BAT_TBL		*bp;

	time( &cur_time);
	tp = localtime( &cur_time);
	int_time = tp->tm_hour * 10000 + tp->tm_min * 100 + tp->tm_sec;

	for( i = 0; i < bat->stat->cnt; i++)
	{
		bp = &bat->base[ i];
		LogLib( "Batch check. id=[%s]", bp->id);
		if( int_time < bp->str_time || int_time > bp->end_time)
		{
			LogLib( "배치 실행시간이 아님. bp->str_time=[%d] bp->end_time=[%d] int_time=[%d]", 
					bp->str_time, bp->end_time, int_time);
			continue;
		}
		if( bp->stat >= 4)
		{
			LogLib( "배치 전송 완료. bp->stat=[%d]", bp->stat);
			continue;
		}
		if( bp->stat != 2)
		{
			LogLib( "배치 전송 상태가 아님. bp->stat=[%d]", bp->stat);
			continue;
		}
		return bp;
	}

	return NULL;
}

/** ***************************************************************************
**	@fn			int Bat_Daily( BAT *bat)
**	@param		BAT 구조체 pointer
**	@param		BAT_TBL 구조체 pointer
**	@return		1   SUCCESS
**	@retval		-1  ERROR
**	@exception
**	@remark
**	@brief		
**	함수가 수행되면 항상 배치 작업을 수행
**	코멘트를 풀면 아래 작업 수행합니다.
**	배치 일일 clear 작업
**	당일 배치 작업이 이미 돌았다면 작업을 하지 않습니다.
**	단 데일리 배치 작업시간 이후에는 당일에 작업이 돌았더라도 일일작업을 
**	수행합니다. (당일 강제로 작업을 진행한 경우 클리어 되지 않았을 수 있음) 
***************************************************************************** */
int Bat_Daily( BAT *bat)
{
	int			rtn;
	int			i;
	int			today, jobday;
	int			int_time;
	time_t		cur_time;
	struct tm	*tp;
	BAT_TBL		*bp;

	time( &cur_time);
	tp = localtime( &cur_time);
	today = ( tp->tm_year + 1900) * 10000 + ( tp->tm_mon +1) * 100 + tp->tm_mday;
	int_time = tp->tm_hour * 10000 + tp->tm_min * 100 + tp->tm_sec;
	tp = localtime( &bat->stat->daily_at);
	jobday = ( tp->tm_year + 1900) * 10000 + ( tp->tm_mon +1) * 100 + tp->tm_mday;

	/*
	if( today == jobday && int_time < bat->stat->daily)
	{
		LogMsg( "이미 일일 배치 작업을 완료 하였습니다. today=[%d] jobday=[%d]", today, jobday);
		return 1;
	}
	*/

	LogMsg( "일일 BATCH CLEAR 작업을 시작합니다. at=[%s]", TtoS( cur_time));
	for( i = 0; i < bat->stat->cnt; i++)
	{
		bp = &bat->base[ i];
		LogMsg( "clear record id=[%s]", bp->id);
		bp->stat = 0;
		memset( bp->file, 0x00, sizeof( bp->file));
		bp->stat_time = 0;
		bp->rec_sz = 0;
		bp->bat_errno = 0;

		bp->st_file = 0;
		bp->tm_file = 0;
		bp->cnt_file = 0;
		bp->err_file = 0;
		memset( bp->nm_file, 0x00, sizeof( bp->nm_file));
		memset( bp->msg_file, 0x00, sizeof( bp->msg_file));

		bp->st_send = 0;
		bp->tm_send = 0;
		bp->cnt_send = 0;
		bp->err_send = 0;
		memset( bp->nm_send, 0x00, sizeof( bp->nm_send));
		memset( bp->msg_send, 0x00, sizeof( bp->msg_file));

		bp->st_recv = 0;
		bp->tm_recv = 0;
		bp->cnt_recv = 0;
		bp->err_recv = 0;
		memset( bp->nm_recv, 0x00, sizeof( bp->nm_recv));
		memset( bp->msg_recv, 0x00, sizeof( bp->msg_file));
		bp->msg_file[ sizeof( bp->msg_recv) -1] = 0;

		memset( bp->bat_errmsg, 0x00, sizeof( bp->bat_errmsg));
	}
	time( &cur_time);
	LogMsg( "일일 BATCH CLEAR 작업을 완료했습니다.. at=[%s]", TtoS( cur_time));
	bat->stat->daily_at = cur_time;

	return 1;
}








