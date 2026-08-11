#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "log.h"
#include "mem.h"
#include "cfg.h"
#include "proc.h"
#include "etc.h"

extern int errno;


/** ***************************************************************************
**	@fn			MEM *ProcTbl_Create( char *cfg_name)
**	@param		char *cfg_name - config file name
**	@return		+    성공( MEM *)
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	프로세스 테이블 공유메모리 생성
***************************************************************************** */
MEM *ProcTbl_Create( char *cfg_name)
{
	MEM			*mem;
	int			sz;
	CFG			*cfg;
	key_t		key;
	char		version[ 64];
	PROC_TBL	*base, *daemon;
	PROC_STAT	*stat;

	LogDel( "Config file open ... name=[%s]", cfg_name);
	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open( cfg_name=[%s]) error.", cfg_name);
		goto error;
	}
	key = Cfg_GetInt( cfg, "key");
	Cfg_Get( cfg, "version", version, 64);

	sz = ( sizeof( PROC_TBL) * ( MAX_PROC_TBL +1)) + sizeof( PROC_STAT);

	mem = Mem_Create( key, sz);
	if( mem == NULL)
	{
		LogErr( "Mem_Create error. key=[0x%08x] size=[%d]", key, sz);
		goto error_1;
	}

	base = Mem_GetPtr( mem);
	daemon = &base[ MAX_PROC_TBL];
	if( daemon == NULL) goto error_1;
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	stat->key = key;
	stat->cnt = 0;
	memcpy( stat->ver_string, version, strlen( version) +1);
	time( &stat->create);

	LogApp( "Process table create. key=[0x%08x] sz=[%d] id=[%d] ptr=[%p]", 
			key, sz, stat->ver_string, mem->ptr);

	Cfg_Close( cfg);

	return mem;

	error_1:
		Cfg_Close( cfg);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			MEM *ProcTbl_Open( char *cfg_name)
**	@param		char *cfg_name - config file name
**	@return		+    성공(MEM *)
**	@retval		NULL 실패
**	@exception
**	@remark
**	@brief		
**	프로세스 공유메모리 연결
***************************************************************************** */
MEM *ProcTbl_Open( char *cfg_name)
{
	key_t	key;
	CFG		*cfg;
	MEM		*mem;

	LogDel( "Config file open ... name=[%s]", cfg_name);
	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open( cfg_name=[%s]) error.", cfg_name);
		goto error;
	}
	key = Cfg_GetInt( cfg, "key");

	mem = Mem_Open( key);
	if( mem == NULL)
	{
		LogErr( "Mem_Open error. key=[0x%08x]", key);
		goto error;
	}

	Cfg_Close( cfg);

	return mem;

	error:
		Cfg_Close( cfg);
		return NULL;
}

/** ***************************************************************************
**	@fn			int ProcTbl_Close( MEM *mem)
**	@param		MEM *mem - 공유메모리 포인터
**	@return		+   항상
**	@exception
**	@remark
**	@brief		
**	프로세스 공유메모리연결을 끊는다.
***************************************************************************** */
int ProcTbl_Close( MEM *mem)
{
	Mem_Close( mem);
	return 1;
}

/** ***************************************************************************
**	@fn			int ProcTbl_Info( MEM *mem)
**	@param		MEM *mem
**	@return		+   항상
**	@exception
**	@remark
**	@brief		
**	프로세스 공유메모리 테이블을 출력(stdout)
***************************************************************************** */
int ProcTbl_Info( MEM *mem)
{
	int				i = 0;
	PROC_TBL		*base, *daemon;
	PROC_STAT		*stat;

	base = Mem_GetPtr( mem);
	daemon = &base[ MAX_PROC_TBL];
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	printf( "key           = [0x%08x]\n", stat->key);
	printf( "max record    = [%d]\n", MAX_PROC_TBL);
	printf( "record count  = [%d]\n", stat->cnt);
	printf( "create date   = [%s]\n", TtoS( stat->create));
	printf( "version       = [%s]\n", stat->ver_string);
	printf( "week          = [%d]\n", stat->week);
	while( stat->holiday[ i] > 0)
	{
		printf( "Holiday[%3d]           = [%d]\n", i, stat->holiday[ i]);
		i++;
	}

	printf( "---[ PROC TABLE }-----------------------------------------------------------------------------------------\n");
	printf( " pos id              c u s    pid    at     at     smtwtfsh o run max start    end       name\n");
	printf( "                     m s t           start  end    uouehrao p cnt run time     time \n");
	printf( "                     d e             HHMMSS HHMMSS nneduitl t                       \n");
	printf( "----------------------------------------------------------------------------------------------------------\n");
	for( i = 0; i < stat->cnt; i++)
	{
		ProcTbl_InfoSub( i, &base[ i]);
	}
	ProcTbl_InfoSub( MAX_PROC_TBL, daemon);
	printf( "----------------------------------------------------------------------------------------------------------\n");
	printf( " cmd=9:stop,1:run st=0:none,1:run,9:stop opt=0:batch,1:exec\n");

	return 1;
}

/** ***************************************************************************
**	@fn			int ProcTbl_InfoSub( int pos, PROC_TBL *rec)
**	@param		int      pos  - 테이블 위치(출력용)
**	@param		PROC_TBL *rec - 출력할 테이블 포인터
**	@return		+   항상
**	@exception
**	@remark
**	@brief		
**	프로세스 테이블 항목을 출력
***************************************************************************** */
int ProcTbl_InfoSub( int pos, PROC_TBL *rec)
{
	char *ptr;
	printf( "%4d ", pos);
	printf( "%-15.15s ", rec->id);
	printf( "%1d ", rec->cmd);
	printf( "%1d ", rec->used);
	printf( "%1d ", rec->stat);
	printf( "%9d ", rec->pid);
	printf( "%6d ", rec->s_time);
	printf( "%6d ", rec->e_time);
	printf( "%-8s ", rec->week);
	printf( "%1d ", rec->option);
	printf( "%3d ", rec->r_cnt);
	printf( "%3d ", rec->max_run);
	ptr = TtoS( rec->sr_time);
	printf( "%s ", &ptr[ 11]);
	ptr = TtoS( rec->er_time);
	printf( "%s ", &ptr[ 11]);
	printf( "%s ", rec->name);
	printf( "\n");

	return 1;
}

/** ***************************************************************************
**	@fn			int ProcTbl_RegiMe( MEM *mem, int cmd, char *id)
**	@param		MEM  *mem - 공유메모리 포인터
**	@param		int  cmd  - 0:stop 1:run 2:pid 3:r_cnt=0
**	@param		char *id  - proc_daemon or *
**	@return		+   성공
**	@retval		0   프로세스 테이블 없음
**	@exception
**	@remark
**	@brief		
**	프로세스 테이블에 상태 등록
**  proc_daemon 자신을 등록하기 위해 만듬
**  proc_daemon -j (데일리 작업)을 위해 cmd=3 만듬
***************************************************************************** */
int ProcTbl_RegiMe( MEM *mem, int cmd, char *id)
{
	int			i;
	PROC_TBL 	*base, *curr;
	PROC_STAT	*stat;

	LogMsg( "ProcTbl_RegiMe ... cmd=[%d] id=[%s]", cmd, id);
	base = Mem_GetPtr( mem);
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	if( !memcmp( id, "proc_d", 7))
	{
		curr = &base[ MAX_PROC_TBL];
	}
	else
	{
		for( i = 0; i < stat->cnt; i++)
		{
			curr = &base[ i];
			if( !memcmp( curr->id, id, strlen( id) +1))
			{
				break;
			}
		}
		if( i >= stat->cnt)
		{
			LogMsg( "process tabl id not found. id=[%s]", id);
			return 0;
		}
	}

	switch( cmd)
	{
		case 0:		/* stop */
			curr->cmd = 0;
			curr->stat = 0;
			curr->pid = 0;
			time( &curr->er_time);
			break;
		case 1:		/* run */
			sprintf( curr->id, "%s", id);
			curr->cmd = 0;
			curr->used = 1;
			curr->stat = 1;
			curr->pid = getpid();
			curr->option = 1;
			curr->r_cnt++;
			time( &curr->sr_time);
			sprintf( curr->name, "%s", "proc_d");
			break;
		case 2:		/* pid 등록 */
			curr->pid = getpid();
			break;
		case 3:		/* r_cnt clear - for daily cleadr */
			curr->r_cnt = 0;
			break;
		default:
			break;
	}

	return 1;
}


/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_SwapLoad( MEM *mem, char *swap_file_name)
{
	int				i;
	int				rtn;
	int				fd;
	struct stat		stat_buf;
	char			*mem_ptr, *swap_rec;
	PROC_STAT		*swap_stat, *mem_stat;
	PROC_TBL		*ptp, *base;
	off_t			mem_size;

	LogDel( "Swap file load. name=[%s]", swap_file_name);

	rtn = stat( swap_file_name, &stat_buf);
	if( rtn < 0)
	{
		LogErr( "stat error. name=[%s]", swap_file_name);
		return -1;
	}

	mem_size = sizeof( PROC_TBL) * ( MAX_PROC_TBL +1) + sizeof( PROC_STAT);

	LogDel( "swap file stat ... name=[%s]", swap_file_name);
	LogDel( "size         =[%lld]", stat_buf.st_size);
	LogDel( "access       =[%s]", TtoS( stat_buf.st_atime));
	LogDel( "modified     =[%s]", TtoS( stat_buf.st_mtime));
	LogDel( "change       =[%s]", TtoS( stat_buf.st_ctime));

	if( mem_size != stat_buf.st_size)
	{
		LogCri( "스왑파일 크기가 공유메모리 크기랑 다릅니다.. mem_size=[%lld] swap_size=[%lld]", mem_size, stat_buf.st_size);
		LogCri( "스왑파일을 공유메모리로 로드하지 않습니다.. name=[%s]", swap_file_name);
		return -1;
	}

	swap_rec = malloc( mem_size +1);
	if( swap_rec == NULL)
	{
		LogErr( "malloc( mem_size=[%d]) error.", mem_size);
		goto error;
	}
	swap_stat = ( PROC_STAT *)&swap_rec[ sizeof( PROC_TBL) * ( MAX_PROC_TBL +1)];

	fd = open( swap_file_name, O_RDONLY);
	if( fd < 0)
	{
		LogErr( "file open error. name=[%s]", swap_file_name);
		goto error_1;
	}

	mem_ptr = Mem_GetPtr( mem);
	mem_stat = ( PROC_STAT *)&mem_ptr[ sizeof( PROC_TBL) * ( MAX_PROC_TBL +1)];

	rtn = read( fd, swap_rec, mem_size);
	if( rtn < mem_size)
	{
		LogErr( "file read error. name=[%s] sz=[%lld] rtn=[%d]", swap_file_name, mem_size, rtn);
		goto error_2;
	}

	close( fd);

	if( strcmp( swap_stat->ver_string, mem_stat->ver_string))
	{
		LogCri( "스왑파일 version이 다릅니다.. file->version=[%s] mem->version=[%s]", swap_stat->ver_string, mem_stat->ver_string);
		LogCri( "스왑파일을 공유메모리로 로드하지 않습니다.. name=[%s]", swap_file_name);
		return 0;
	}

	memcpy( mem_ptr, swap_rec, mem_size);
	LogMsg( "스왑파일을 프로세스 테이블 공유메모리로 로드하였습니다. name=[%s] size=[%lld]", swap_file_name, mem_size);

	base = ( PROC_TBL *)mem_ptr;
	for( i = 0; i < mem_stat->cnt; i++)
	{
		ptp = &base[ i];

		ptp->pid = 0;
		ptp->stat = 0;
	}

	return rtn;

	error_2:
		close( fd);
	error_1:
		free( swap_rec);
	error:
		return -1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_SwapProc( MEM *mem, char *swap_file_name)
{
	int				rtn;
	int				fd;
	char			*mem_ptr;
	off_t			mem_size;

	LogDel( "shared memory to file. name=[%s]", swap_file_name);

	mem_size = sizeof( PROC_TBL) * ( MAX_PROC_TBL +1) + sizeof( PROC_STAT);
	mem_ptr = Mem_GetPtr( mem);

	fd = open( swap_file_name, O_RDWR | O_CREAT, 0666);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", swap_file_name);
		return -1;
	}

	rtn = write( fd, mem_ptr, mem_size);
	if( rtn < mem_size)
	{
		LogErr( "swap file write error. name=[%s] mem_size=[%lld] rtn=[%d]", swap_file_name, mem_size, rtn);
		goto error;
	}

	close( fd);
	LogDel( "프로세스 테이블을 스왑파일로 저장하였습니다. name=[%s] size=[%lld]", swap_file_name, mem_size);

	return rtn;

	error:
		close( fd);
		return -1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_Compare( const void *a1, const void *a2)
{
	int		*i1, *i2;

	i1 = ( int *)a1;
	i2 = ( int *)a2;
	LogDel( "[%d]=[%d]", *i1, *i2);

	return *i1 - *i2;
}


/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
**  opt -- 처음 proc_swap가 실행 되었을때 강제 실행하기 위한 옵션 log_config == 0 이어도 실행
***************************************************************************** */
int ProcTbl_CheckCfg( MEM *mem, char *cfg_file_name, int opt)
{
	char		*proc_rec, *holiday_ptr;
	PROC_TBL	proc_buf;
	PROC_TBL	*proc_ptr = &proc_buf;
	PROC_TBL	*base, *daemon;
	PROC_STAT	*stat;
	CFG			*cfg;
	int			load_flag;
	int			pos = 0, int_day, update_flag = 0;

	base    = Mem_GetPtr( mem);
	daemon  = &base[ MAX_PROC_TBL];
	if( daemon == NULL) goto error;
	stat    = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];


	LogDel( "Config file open. name=[%s]", cfg_file_name);
	cfg = Cfg_Open( cfg_file_name);
	if( cfg == NULL)
	{
		LogCri( "config file open error. name=[%s]", cfg_file_name);
		goto error;
	}

	load_flag = Cfg_GetInt( cfg, "load_config");
	if( load_flag || opt == 1)
	{
		LogDel( "load_config =[%d]  --- \'0\' 이외의 값이면 config 파일을 로드", load_flag);
		/* process_table load */
		LogDel( "Check process_table table");
		proc_rec = Cfg_GetFirstNamePtr( cfg, "process_table");
		while( proc_rec != NULL)
		{
			memset( proc_ptr, 0x00, sizeof( PROC_TBL));
			ProcTbl_MakeRec( mem, proc_ptr, proc_rec);
			ProcTbl_LoadRec( mem, proc_ptr);
			proc_rec = Cfg_GetNextNamePtr( cfg, "process_table");
		}

#if 0
		/* holiday table load */
		LogDel( "Check holiday table");
		holiday_ptr = Cfg_GetFirstNamePtr( cfg, "holiday");
		while( holiday_ptr != NULL)
		{
			LogDel( "holiday[%3d]=[%d] holiday_ptr=[%p][%s] update_flag=[%d]", pos, stat->holiday[ pos], holiday_ptr, holiday_ptr, update_flag);
			int_day = atoi( holiday_ptr);
			if( int_day <= 0) break;
			if( stat->holiday[ pos] != int_day)
			{
				LogMsg( "holiday table update. old[%d]!=new[%d]", stat->holiday[ pos], int_day);
				stat->holiday[ pos] = int_day;
				update_flag = 1;
			}
			pos++;
			holiday_ptr = Cfg_GetNextNamePtr( cfg, "holiday");
		}
		if( update_flag)
		{
			qsort( &stat->holiday, pos, sizeof( int), ProcTbl_Compare);
		}
#endif
	}

	Cfg_Close( cfg);
	return 1;

	error:
		return -1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_MakeRec( MEM *mem, PROC_TBL *proc_tbl, char *proc_rec)
{
	char	*p;
	char	rec[ 512];
	char	*tok_str = ":\r\n";

	memcpy( rec, proc_rec, strlen( proc_rec) +1);
	LogDel( "Make record. str=[%s]", rec);

	/* get id */
	p = strtok( rec, tok_str);
	if( p == NULL)
	{
		LogDel( "get id error.");
		return -1;
	}
	TrimR( p);
	memcpy( proc_tbl->id, p, strlen( p) +1);

	/* get used */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get used error.");
		return -2;
	}
	proc_tbl->used = atoi( p);

	/* get max_run */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get max_run error.");
		return -2;
	}
	proc_tbl->max_run = atoi( p);

	/* get start */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get start time error.");
		return -3;
	}
	proc_tbl->s_time = atoi( p);

	/* get end */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get end time error.");
		return -4;
	}
	proc_tbl->e_time = atoi( p);

	/* get week */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get week error.");
		return -5;
	}
	memcpy( proc_tbl->week, p, strlen( p) +1);

	/* get path */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get path error.");
		return -6;
	}
	TrimR( p);
	memcpy( proc_tbl->path, p, strlen( p) +1);

	/* get name */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get name error.");
		return -7;
	}
	memcpy( proc_tbl->name, p, strlen( p) +1);

	/* get option */
	p = strtok( NULL, tok_str);
	if( p == NULL)
	{
		LogDel( "get option error.");
		return -8;
	}
	proc_tbl->option = atoi( p);

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_LoadRec( MEM *mem, PROC_TBL *proc)
{
	int			i, sz, len, find_flag = 0;
	PROC_TBL	*base, *curr;
	PROC_STAT	*stat;

	base    = Mem_GetPtr( mem);
	stat    = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	sz = strlen( proc->id);
	for( i = 0; i < stat->cnt; i++)
	{
		curr = &base[ i];
		len = Max( strlen( curr->id), sz);
		if( len <= 0) continue;
		LogDel( "check process id. proc->id=[%s] curr->id=[%s]", proc->id, curr->id);
		if( !memcmp( proc->id, curr->id, len)) 
		{
			find_flag = 1;
			break;
		}
	}

	if( find_flag == 0)
	{
		LogMsg( "프로세스 테이블에 레코드를 추가 하였습니다. id=[%s] pos=[%d]", proc->id, stat->cnt);
		curr = &base[ stat->cnt];
		memcpy( curr, proc, sizeof( PROC_TBL));
		stat->cnt++;
		return 1;
	}

	if( curr->used != proc->used)
	{
		LogMsg( "id=[%s] 사용여부가 변경 되었습니다. old=[%d] new=[%d]", proc->id, curr->used, proc->used);
		curr->used = proc->used;
	}
	if( curr->max_run != proc->max_run)
	{
		LogMsg( "id=[%s] 최대 수행 횟수가 변경 되었습니다. old=[%d] new=[%d]", proc->id, curr->max_run, proc->max_run);
		curr->max_run = proc->max_run;
	}
	if( curr->s_time != proc->s_time)
	{
		LogMsg( "id=[%s] 시작시간이 변경 되었습니다. old=[%d] new=[%d]", proc->id, curr->s_time, proc->s_time);
		curr->s_time = proc->s_time;
	}
	if( curr->e_time != proc->e_time)
	{
		LogMsg( "id=[%s] 종료시간이 변경 되었습니다. old=[%d] new=[%d]", proc->id, curr->e_time, proc->e_time);
		curr->e_time = proc->e_time;
	}
	len = Max( strlen( curr->week), strlen( proc->week));
	if( memcmp( curr->week, proc->week, len))
	{
		LogMsg( "id=[%s] 실행요일이 변경 되었습니다. old=[%.8s] new=[%.8s]", proc->id, curr->week, proc->week);
		memcpy( curr->week, proc->week, strlen( proc->week) +1);
	}
	len = Max( strlen( curr->path), strlen( proc->path));
	if( memcmp( curr->path, proc->path, len))
	{
		LogMsg( "id=[%s] 디렉토리가 변경 되었습니다. old=[%s] new=[%s]", proc->id, curr->path, proc->path);
		memcpy( curr->path, proc->path, strlen( proc->path) +1);
	}
	len = Max( strlen( curr->name), strlen( proc->name));
	if( memcmp( curr->name, proc->name, len))
	{
		LogMsg( "id=[%s] 실행 변수가 변경 되었습니다. old=[%s] new=[%s]", proc->id, curr->name, proc->name);
		memcpy( curr->name, proc->name, strlen( proc->name) +1);
	}
	if( curr->option != proc->option)
	{
		LogMsg( "id=[%s] 프로세스 옵션이 변경 되었습니다. old=[%d] new=[%d]", proc->id, curr->option, proc->option);
		curr->option = proc->option;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_ClearRec( MEM *mem)
{
	int			i, today, int_day;
	time_t		cur_time;
	struct tm	*tp;
	PROC_TBL	*base, *curr;
	PROC_STAT	*stat;

	base = Mem_GetPtr( mem);
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	stat->pre_week = stat->week;

	/* holiday check */
	time( &cur_time);
	tp = localtime( &cur_time);
	today  = ( tp->tm_year +1900) * 10000;
	today += ( tp->tm_mon +1) * 100;
	today += tp->tm_mday;
	stat->week = tp->tm_wday;

#if 0
	/* prev day */
	cur_time -= ( 24 * 3600);
	tp = localtime( &cur_time);
	stat->pre_week = tp->tm_wday;

	/* next day */
	cur_time += ( 24 * 3600) * 2;
	tp = localtime( &cur_time);
	stat->nex_week = tp->tm_wday;
#endif

	/*
	 * 휴일테이블 LOAD에서 날짜를 setting 하지만 여기서 한번 더 한다고 문제 될것이 없어
	 * 지우지 않음
	 */
	for( i = 0; i < MAX_HOLI_TBL; i++)
	{
		int_day = stat->holiday[ i];
		if( int_day <= 0) break;

		if( int_day == today)
		{
			stat->week = 7;
			LogMsg( "Set holiday... stat->week=[%d]", stat->week);
			break;
		}
	}
	LogMsg( "Today is [%d]. stat->holiday=[%d]", today, stat->week);

	for( i = 0; i < stat->cnt; i++)
	{
		curr = &base[ i];
		if( strlen( curr->id) <= 0) continue;
		if( curr->pid > 0)
		{
			curr->r_cnt = 1;
		}
		else
		{
			curr->stat = 0;
			curr->r_cnt = 0;
			/*
			curr->sr_time = 0;
			curr->er_time = 0;
			*/
		}
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_GetHolidayInt( MEM *mem, char *line)
{
	int		stat = 0;
	int		day = 0;
	char	*token = ":";
	char	rec[ 512];
	char	*ptr = rec, *end = ptr;

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:		/* position */
				break;
			case 1:		/* int day */
				day = atoi( ptr);
				return day;
				break;
			default:
				break;
		}

		ptr = end +1;
		stat++;
	}

	return 0;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int ProcTbl_LoadHolidayTbl( MEM *mem, char *cfg_name)
{
	int			i, today;
	time_t		cur_time;
	struct tm	*tp;
	PROC_TBL	*base;
	PROC_STAT	*stat;

	CFG			*cfg;
	char		*ptr;		/* holiday line record */
	int			cnt = 0;	/* holiday cnt */
	int			int_day;	/* holiday int day */

	base = Mem_GetPtr( mem);
	stat = ( PROC_STAT *)&base[ MAX_PROC_TBL +1];

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. cfg=[%p]", cfg);
		return -1;
	}

	ptr = Cfg_GetFirstNamePtr( cfg, "holiday_list");
	while( ptr != NULL)
	{
		int_day = ProcTbl_GetHolidayInt( mem, ptr);
		if( int_day <= 0) break;
		stat->holiday[ cnt] = int_day;
		cnt++;
		if( cnt >= MAX_HOLI_TBL)
		{
			LogCri( "휴일 테이블 공간이 없습니다. cnt=[%d] max=[%d]", cnt, MAX_HOLI_TBL);
			break;
		}
		ptr = Cfg_GetNextNamePtr( cfg, "holiday_list");
	}

	LogMsg( "휴일 테이블 로드. cnt=[%d]", cnt);
	qsort( stat->holiday, cnt, sizeof( int), ProcTbl_Compare);

	/* clear remain table */
	for( i = cnt; i < MAX_HOLI_TBL; i++)	stat->holiday[ i] = 0;


	/* holiday check */
	time( &cur_time);
	tp = localtime( &cur_time);
	today  = ( tp->tm_year +1900) * 10000;
	today += ( tp->tm_mon +1) * 100;
	today += tp->tm_mday;
	stat->week = tp->tm_wday;

	for( i = 0; i < MAX_HOLI_TBL; i++)
	{
		int_day = stat->holiday[ i];
		if( int_day <= 0) break;

		if( int_day == today)
		{
			stat->week = 7;
			LogMsg( "Set holiday... stat->week=[%d]", stat->week);
			break;
		}
	}
	LogMsg( "Today is [%d]. stat->holiday=[%d]", today, stat->week);

	return 1;
}


/******************************************************************************************************
*
* PROCESS
*
******************************************************************************************************/
/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
PROC *Proc_Open( char *cfg_name)
{
	PROC	*proc;

	proc = malloc( sizeof( PROC));
	if( proc == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( PROC));
		goto error;
	}

	proc->mem = ProcTbl_Open( cfg_name);
	if( proc->mem == NULL)
	{
		LogCri( "ProcTbl_Open error. abort Proc_Open");
		goto error_1;
	}
	proc->base     = Mem_GetPtr( proc->mem);
	proc->curr     = proc->base;
	proc->daemon   = &proc->base[ MAX_PROC_TBL];
	proc->stat     = ( PROC_STAT *)&proc->base[ MAX_PROC_TBL +1];

	return proc;

	error_1:
		free( proc);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_Close( PROC *proc)
{
	ProcTbl_Close( proc->mem);
	free( proc);

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		0   성공
**	@retval		1   실패
**	@exception
**	@remark
**	@brief		
**  모니터링을 위한 장애여부 판단
**	프로세스 관리
***************************************************************************** */
int Proc_HealthCheck( PROC_TBL *tbl)
{
	time_t		cur_time;
	int			int_time;
	int			gap = 0;	/* 프로세스가 시작, 종료 할때까지 걸리는 시간 감안 */

	time( &cur_time);
	int_time = TtoI( cur_time);

	LogDel( "CHECK -------------");
	LogDel( "used       =[%d]", tbl->used);
	LogDel( "stat       =[%d]", tbl->stat);
	LogDel( "r_cnt      =[%d]", tbl->r_cnt);
	LogDel( "option     =[%d]", tbl->option);
	LogDel( "int_time   =[%d]", int_time);
	LogDel( "s_time     =[%d]", tbl->s_time);
	LogDel( "e_time     =[%d]", tbl->e_time);

	LogDel( "stat       =[%d]", 1);
	if( tbl->used == 0) return 0;							/* 사용하지 않는 TABLE */
	LogDel( "stat       =[%d]", 2);
	if( tbl->stat == 1)										/* 이미 RUN 상태 */
	{
		if( tbl->s_time == 0 && tbl->e_time == 0)			/* 항상 떠 있는 프로세스 */
		{
			return 0;
		}
		else
		if( tbl->s_time > tbl->e_time)						/* 다음날 까지 수행하는 프로세스 */
		{
			if( int_time < ( tbl->s_time + gap) && int_time > ( tbl->e_time + gap)) return 1;
		}
		else
		{
			if( int_time < ( tbl->s_time + gap)) return 1;	/* 시작시간 이전 */
			if( int_time > ( tbl->e_time + gap)) return 1;	/* 종료시간 이후  */
		}
		if( tbl->cmd == 1) tbl->cmd = 0;					/* run 명령 무시 */
		return 0;
	}

	/* 이하 ... 프로세스가 죽은경우 */
	LogDel( "stat       =[%d]", 3);
	if( tbl->option == 0 && tbl->r_cnt > 0) 				/* batch job이 이미 실행 */
	{
		return 0;		
	}

	LogDel( "stat       =[%d]", 4);

	if( tbl->s_time == 0 && tbl->e_time == 0)			/* 항상 떠 있는 프로세스 */
	{
		if( tbl->stat == 9) return 0;					/* 명령에 의해 중지된 경우 */
		return 1;
	}
	else
	if( tbl->s_time > tbl->e_time)							/* 다음날 까지 수행하는 프로세스 */
	{
		if( int_time <= ( tbl->s_time +gap) && int_time > ( tbl->e_time +gap)) return 0;
	}
	else
	{
		if( int_time <= ( tbl->s_time +gap)) return 0;		/* 시작시간 이전 */
		if( int_time >= ( tbl->e_time +gap)) return 0;		/* 종료시간 이후  */
	}

	LogDel( "stat       =[%d]", 5);
	if( tbl->stat == 9) return 0;							/* 명령에 의해 중지된 경우 */
#if 0
	if( tbl->r_cnt >= tbl->max_run) return 0;				/* 실행 count를 넘은경우 */
	if( tbl->week[ proc->stat->week] == '0') return 0;		/* 해당요일 수행 */
#endif

	LogDel( "rtn        =[%d]", 1);
	return 1;
}

/** ***************************************************************************
**	@fn			Proc_CheckPid( PROC *proc)
**	@param		PROC *Proc - process pointer
**	@return		+   항상
**	@exception
**	@remark
**	@brief		
**	프로세스 테이블에 등록된 pid를 check하여(kill function) 실제 떠 있는지 검사
**  프로세스가 떠있지 않으면 pid=0,stat=0으로 setting
***************************************************************************** */
int	Proc_CheckPid( PROC *proc)
{
	int			rtn;
	int			i;
	PROC_TBL	*ptp;

	for( i = 0; i < proc->stat->cnt; i++)
	{
		ptp = &proc->base[ i];
		rtn = kill( ptp->pid, 0);
		if( rtn < 0)
		{
			if( errno == ESRCH)		/* not exist */
			{
				LogLib( "process not exist. id=[%s] pid=[%d]", ptp->id, ptp->pid);
				ptp->stat = 0;
				ptp->pid = 0;
			}
		}
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_Run( PROC *proc)
{
	int			rtn, i;
	time_t		cur_time;
	int			int_time;
	struct tm	*tp;
	PROC_TBL	*curr;
	pid_t		pid;

	time( &cur_time);
	tp = localtime( &cur_time);
	int_time  = tp->tm_hour * 10000;
	int_time += tp->tm_min * 100;
	int_time += tp->tm_sec;

	LogDel( "process cnt=[%d]", proc->stat->cnt);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		LogDel( "Check run check id =[%s]", curr->id);
		rtn = Proc_RunCheck( proc, curr, int_time);
		LogDel( "Check run check id=[%s] rtn=[%d]", curr->id, rtn);
		if( rtn) 
		{
			pid = Proc_RunProc( proc, curr);
			if( pid < 0) 
			{
			}
		}
		LogDel( "Check run process rtn=[%d] pid=[%d]", rtn, pid);
	}

	return rtn;
}

/** ***************************************************************************
**	@fn			Proc_RunCheck( PROC *proc, PROC_TBL *tbl, int int_time)
**	@param		PROC *Proc - process pointer
**	@param		PROC_TBL *tbl - process table pointer
**	@param		int int_time - 기준시간
**	@return		+   실행필요
**	@retval		0   실행필요없음
**	@exception
**	@remark
**	@brief		
**	프로세스를 실행해야 하는지 검사
**  날짜는(week)는 프로세스 배치가 실행되는 시점에 다음날로 넘어간다
***************************************************************************** */
int Proc_RunCheck( PROC *proc, PROC_TBL *tbl, int int_time)
{
	LogDel( "CHECK -------------");
	LogDel( "used       =[%d]", tbl->used);
	LogDel( "stat       =[%d]", tbl->stat);
	LogDel( "r_cnt      =[%d]", tbl->r_cnt);
	LogDel( "option     =[%d]", tbl->option);
	LogDel( "int_time   =[%d]", int_time);
	LogDel( "s_time     =[%d]", tbl->s_time);
	LogDel( "e_time     =[%d]", tbl->e_time);

	if( tbl->used == 0) return 0;							/* 사용하지 않는 TABLE */
	if( tbl->stat == 1)										/* 이미 RUN 상태 */
	{
		if( tbl->cmd == 1) tbl->cmd = 0;					/* run 명령 무시 */
		return 0;
	}
	LogDel( "종료된 process");
	if( tbl->r_cnt >= tbl->max_run) return 0;				/* 실행 count를 넘은경우 */
	LogDel( "실행 count가 아직 안넘은 process cnt=[%d/%d]", tbl->r_cnt, tbl->max_run);
	if( tbl->option == 0 && tbl->r_cnt > 0) return 0;		/* batch job이 이미 실행 */

	LogDel( "배치가 아니거나 이미 실행");
	if( tbl->week[ proc->stat->week] == '0') 				/* 해당요일 수행 */
	{
		return 0;
	}

	if( tbl->s_time == 0 && tbl->e_time == 0)				/* 항상 떠 있는 프로세스 인데 죽은 경우 */
	{
		if( tbl->cmd == 1)									/* 실행 명령 */
		{
			tbl->cmd = 0; 
			return 1;
		}
		if( tbl->stat == 9) return 0;						/* 명령에 의해 중지된 경우 */
		return 1;
	}
	else
	if( tbl->s_time > tbl->e_time)							/* 다음날 까지 수행하는 프로세스 */
	{
		if( ( int_time < tbl->s_time) && ( int_time > tbl->e_time)) return 0;
	}
	else
	{
		if( int_time < tbl->s_time) return 0;
		if( int_time > tbl->e_time) return 0;
	}

	if( tbl->cmd == 1) 										/* 실행 명령 */
	{
		tbl->cmd = 0;
		tbl->stat = 0;
		return 0;											/* 다음 check에서 조건에 맞으면 실행 */
	}
	if( tbl->stat == 9) return 0;							/* 명령에 의해 중지된 경우 */

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_RunProc( PROC *proc, PROC_TBL *tbl)
{
	int		rtn;
	pid_t	pid;
	char	*ptr, *argv[ 64], cmd[ 512];
	int		pos = 0;
	char	*tokstr = " \t\n\r\v\f";

	/*
	sprintf( cmd, "%s/%s", tbl->path, tbl->name);
	*/
	sprintf( cmd, "%s", tbl->name);
	LogMsg( "프로세스 실행. id=[%s] run_cnt=[%d] cmd=[%s]", tbl->id, tbl->r_cnt +1, cmd);
	ptr = strtok( cmd, tokstr);
	while( ptr != NULL)
	{
		argv[ pos++] = ptr;
		ptr = strtok( NULL, tokstr);
	}
	argv[ pos++] = NULL;
	argv[ pos++] = NULL;

	pid = fork();
	if( pid < 0)
	{
		LogErr( "fork error.");
		return -1;
	}
	else if( pid == 0)
	{
		rtn = chdir( tbl->path);
		if( rtn < 0)
		{
			LogErr( "디렉토리 변경오류. 현디렉토리에서 실행합니다. path=[%s]", tbl->path);
		}

		rtn = execv( argv[ 0], argv);
		if( rtn < 0)
		{
			LogErr( "execv error. id=[%s] path=[%s] name=[%s]", tbl->id, tbl->path, tbl->name);
		}
		sleep( 1);
		exit( 0);
	}

	LogMsg( "프로세스를 실행하였습니다. pid=[%d]", pid);
	tbl->pid = pid;
	tbl->r_cnt++;
	tbl->stat = 1;
	time( &tbl->sr_time);

	return ( int)pid;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_Stop( PROC *proc)
{
	int			rtn, i;
	time_t		cur_time;
	int			int_time;
	struct tm	*tp;
	PROC_TBL	*curr;
	pid_t		pid = 0;

	time( &cur_time);
	tp = localtime( &cur_time);
	int_time  = tp->tm_hour * 10000;
	int_time += tp->tm_min * 100;
	int_time += tp->tm_sec;

	LogDel( "process cnt=[%d]", proc->stat->cnt);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		LogDel( "Check run process id =[%s]", curr->id);
		rtn = Proc_StopCheck( proc, curr, int_time);
		if( rtn) 
		{
			pid = Proc_StopProc( proc, curr);
			if( pid <= 0)
			{
			}
		}
		LogDel( "Check stop process rtn=[%d] pid=[%d]", rtn, pid);
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   중지
**	@retval		0   중지 안함
**	@exception
**	@remark
**	@brief		
**  프로세스 중지해야 할지 check
**	프로세스 관리
***************************************************************************** */
int Proc_StopCheck( PROC *proc, PROC_TBL *tbl, int int_time)
{
	LogDel( "CHECK -------------");
	LogDel( "used       =[%d]", tbl->used);
	LogDel( "stat       =[%d]", tbl->stat);
	LogDel( "r_cnt      =[%d]", tbl->r_cnt);
	LogDel( "option     =[%d]", tbl->option);
	LogDel( "int_time   =[%d]", int_time);
	LogDel( "s_time     =[%d]", tbl->s_time);
	LogDel( "e_time     =[%d]", tbl->e_time);

	if( tbl->used == 0) return 0;							/* 사용하지 않는 TABLE */
	if( tbl->pid <= 0)	return 0;							/* 이미 STOP 상태 */
	if( tbl->stat == 0)										/* 이미 STOP 상태 */
	{
		if( tbl->cmd == 9)
		{
			tbl->cmd = 0;
			if( tbl->option != 0)							/* 배치이면 STOP상태가 없다 */
				tbl->stat = 9;								/* 다시 살리지 않음 */
			else
				LogWar( "이미 중지된 배치 프로세스입니다. ");
		}
		return 0;
	}

	if( tbl->cmd  == 9)	return 1;							/* stop 명령이 입력된경우 */

	if( tbl->s_time == 0 && tbl->e_time == 0)				/* 항상 떠있는 프로세스 */
	{
		return 0;
	}
	if( tbl->s_time > tbl->e_time)							/* 다음날 까지 수행하는 프로세스 */
	{
		/* 수행 시간이 아닌경우 0 return */
		if( int_time >= tbl->s_time) return 0;
		if( int_time <= tbl->e_time) return 0;
	}
	else
	{
		/* 수행 시간이 아닌경우 0 return */
		if( int_time >= tbl->s_time && int_time <= tbl->e_time) return 0;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_StopProc( PROC *proc, PROC_TBL *tbl)
{
	int			rtn;

	if( tbl->pid <= 0)
	{
		LogCri( "이미 중단된 프로세스. id=[%s] stat=[%d] pid=[%d]", tbl->id, tbl->stat, tbl->pid);
		if( tbl->cmd == 9)	
		{
			tbl->cmd = 0;
			tbl->stat = 9;
		}
		return 0;
	}

	rtn = kill( tbl->pid, SIGTERM);
	if( rtn < 0)
	{
		LogErr( "kill error. id=[%s] pid=[%d]", tbl->id, tbl->pid);
		if( errno == ESRCH) /* no process */
		{
			Proc_StopMake( proc, tbl, 0);
			tbl->pid = 0;
			return rtn;
		}
	}
	LogMsg( "Send SIGTERM signal. pid=[%d] rtn=[%d] cnt=[%d]", tbl->pid, rtn, tbl->cmd_cnt);
	tbl->cmd_cnt++;
	if( tbl->cmd_cnt > 10)
	{
		rtn = kill( tbl->pid, SIGKILL);
		if( rtn < 0)
		{
			LogErr( "kill error. id=[%s] pid=[%d]", tbl->id, tbl->pid);
			if( errno == ESRCH) /* no process */
			{
				Proc_StopMake( proc, tbl, 0);
				tbl->pid = 0;
				return rtn;
			}
		}
		tbl->cmd_cnt = 0;
	}

	return rtn;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_StopAll( PROC *proc)
{
	int			rtn, i;
	PROC_TBL	*curr;

	LogDel( "process cnt=[%d]", proc->stat->cnt);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		if( curr->stat == 1 && curr->pid != 0) 
		{
			LogMsg( "Send SIGTERM signal. id=[%s] pid=[%d]", curr->id, curr->pid);
			rtn = kill( curr->pid, SIGTERM);
			if( rtn < 0)
			{
				LogErr( "kill error. id=[%s] pid=[%d]", curr->id, curr->pid);
				if( errno == ESRCH) /* no process */
				{
					Proc_StopMake( proc, curr, 0);
				}
			}
		}
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_WaitAll( PROC *proc)
{
	int			i;
	PROC_TBL	*curr;
	int			stat = 1, cnt = 30;

	while( stat)
	{
		Proc_Check( proc);
		sleep( 1);
		stat = 0;
		for( i = 0; i < proc->stat->cnt; i++)
		{
			curr = &proc->base[ i];
			if( curr->stat == 1 && curr->pid != 0) 
			{
				LogMsg( "Wait process stop. id=[%s] pid=[%d] cnt=[%d]", curr->id, curr->pid, cnt);
				if( cnt < 20) Proc_StopProc( proc, curr);
				stat++;
			}
		}
		cnt--;
		if( cnt <= 0) break;
	}

	LogMsg( "All process stoped.");

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_Check( PROC *proc)
{
	int			rtn, cnt = MAX_PROC_TBL;
	pid_t		pid;
	int			stat;
	PROC_TBL	*curr;

	while( cnt)
	{
		pid = wait3( ( int *)&stat, WNOHANG, ( struct rusage *)0);
		if( pid < 0) return -1;
		if( pid > 0)
		{
			curr = Proc_FindPid( proc, pid);
			if( curr == NULL)
			{
				LogWar( "종료된 프로세스 아이디가 없습니다. pid=[%d]", pid);
			}
			else
			{
				rtn = Proc_StopMake( proc, curr, stat);
				if( rtn < 0)
				{
				}
			}
		}
		cnt--;
	}

	return rtn;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
PROC_TBL *Proc_FindPid( PROC *proc, pid_t pid)
{
	int			i;
	PROC_TBL	*curr;

	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		if( curr->pid == pid) return curr;
	}

	return NULL;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
PROC_TBL *Proc_FindId( PROC *proc, char *id)
{
	int			i;
	int			sz, cmp_sz;
	PROC_TBL	*curr;

	sz = strlen( id);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		cmp_sz = Max( sz, strlen( curr->id));
		if( !memcmp( curr->id, id, cmp_sz)) return curr;
	}

	return NULL;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_StopMake( PROC *proc, PROC_TBL *curr, int stat)
{
	LogWar( "프로세스 종료. id=[%s] name=[%s] pid=[%d]", curr->id, curr->name, curr->pid);
	if( WCOREDUMP( stat))		LogMsg( "Term stat - WCOREDUMP(%3d)    created core image         ",  WCOREDUMP( stat));     
	if( WEXITSTATUS( stat))		LogMsg( "Term stat - WEXITSTATUS(%3d)  call exit arg              ",  WEXITSTATUS( stat));
	if( WIFCONTINUED( stat))	LogMsg( "Term stat - WIFCONTINUED(%3d) process continued          ",  WIFCONTINUED( stat));
	if( WIFEXITED( stat))		LogMsg( "Term stat - WIFEXITED(%3d)    terminated normally        ",  WIFEXITED( stat));
	if( WIFSIGNALED( stat))		LogMsg( "Term stat - WIFSIGNALED(%3d)  receipt signal             ",  WIFSIGNALED( stat));
	if( WIFSTOPPED( stat))		LogMsg( "Term stat - WIFSTOPPED(%3d)   process stopped            ",  WIFSTOPPED( stat));
	if( WSTOPSIG( stat))		LogMsg( "Term stat - WSTOPSIG(%3d)     received stop signal       ",  WSTOPSIG( stat));
	if( WTERMSIG( stat))		LogMsg( "Term stat - WTERMSIG(%3d)     received termination signal",  WTERMSIG( stat));  


	if( curr->cmd == 9) /* 명령에 의해 죽은경우 다시 살아나는것을 방지 */
	{
		curr->cmd = 0;
		curr->stat = 9;
	}
	else
	{
		curr->stat = 0;
	}
	time( &curr->er_time);
	curr->pid = 0;
	curr->cmd_cnt = 0;

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_StopCmd( PROC *proc, char *id)
{
	int			rtn;
	int			i, sz;
	int			find_flag = 0;
	PROC_TBL	*curr;

	if( !memcmp( id, "proc_d", 6) || !memcmp( id, "all", 4))
	{
		if( proc->daemon->pid <= 0)
		{
			LogMsg( "이미 프로세스가 종료 되었습니다.. id=[%s]\n", id);
			printf( "이미 프로세스가 종료 되었습니다.. id=[%s]\n", id);
			return 0;
		}
		LogMsg( "FEP ALL PROCESS STOP. id=[%s]", id);
		printf( "FEP ALL PROCESS STOP. id=[%s]\n", id);
		rtn = kill( proc->daemon->pid, SIGTERM);
		if( rtn < 0)
		{
			LogErr( "kill error.");
			if( errno == ESRCH)
			{
				proc->daemon->pid = 0;
			}
		}
		return 1;
	}

	sz = strlen( id);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		if( !memcmp( curr->id, id, sz))
		{
			curr->cmd = 9;
			find_flag = 1;
		}
	}

	if( find_flag <= 0)
	{
		LogMsg( "해당 ID가 없습니다. id=[%s]", id);
		return 0;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_RunCmd( PROC *proc, char *id)
{
	int			i, sz;
	int			find_flag = 0;
	PROC_TBL	*curr;

	sz = strlen( id);
	for( i = 0; i < proc->stat->cnt; i++)
	{
		curr = &proc->base[ i];
		if( !memcmp( curr->id, id, sz))
		{
			curr->cmd = 1;
			find_flag = 1;
		}
	}

	if( find_flag <= 0)
	{
		LogMsg( "해당 ID가 없습니다. id=[%s]", id);
		return 0;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	proc_d pid get
***************************************************************************** */
int Proc_GetDaemonPid( PROC *proc)
{
	PROC_TBL	*tbl;

	tbl = &proc->base[ MAX_PROC_TBL];

	return tbl->pid;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_InfoProc( PROC *proc, char *id)
{
	int			pos;
	PROC_TBL	*curr;

	curr = Proc_FindId( proc, id);
	if( curr == NULL)
	{
		if( !isdigit( id[ 0])) 
		{
			LogMsg( "해당 프로세스가 없습니다. id=[%s]", id);
			return 0;
		}
		pos = atoi( id);
		curr = &proc->base[ pos];
	}

	Proc_InfoProcSub( proc, curr);
	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_InfoProcSub( PROC *proc, PROC_TBL *curr)
{
	printf( "id      프로그램 ID                       = [%s]\n", curr->id);
	printf( "cmd     프로세스 Control 명령             = [%d]\n", curr->cmd);
	printf( "used    레코드 사용여부                   = [%d]\n", curr->used);
	printf( "stat    프로세스상태(0:none 1:run 9:kill) = [%d]\n", curr->stat);
	printf( "pid     priocess id                       = [%d]\n", curr->pid);
	printf( "s_time  시작시간 HHMMSS                   = [%d]\n", curr->s_time);
	printf( "e_time  종료시간 HHMMSS                   = [%d]\n", curr->e_time);
	printf( "week    실행요일 일월화수목금토공         = [%s]\n", curr->week);
	printf( "path    프로그램 위치                     = [%s]\n", curr->path);
	printf( "name    프로그램 실행 명령                = [%s]\n", curr->name);
	printf( "option  respawn(0:no 1:yes)               = [%d]\n", curr->option);
	printf( "r_cnt   실행횟수                          = [%d]\n", curr->r_cnt);
	printf( "max_run 최대 실행 횟수                    = [%d]\n", curr->max_run);
	printf( "sr_timr 프로세스 실행 시간                = [%s]\n", TtoS( curr->sr_time));
	printf( "er_time 프로세스 종료 시간                = [%s]\n", TtoS( curr->er_time));

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_SetProc( PROC *proc, char *id, char *name, char *value)
{
	PROC_TBL	*curr;

	curr = Proc_FindId( proc, id);
	if( curr == NULL)
	{
		printf( "해당 id가 없습니다. id=[%s]", id);
		return 0;
	}

	if( !memcmp( name, "id",  			2))     { sprintf( curr->id, "%s", value);    }
	else if( !memcmp( name, "cmd",  	3))     { curr->cmd = atoi( value);           }
	else if( !memcmp( name, "used",  	4))     { curr->used = atoi( value);          }
	else if( !memcmp( name, "stat",  	4))     { curr->stat = atoi( value);          }
	else if( !memcmp( name, "pid",  	3))     { curr->pid = atoi( value);           }
	else if( !memcmp( name, "s_time",	6))     { curr->s_time = atoi( value);        }
	else if( !memcmp( name, "e_time",	6))     { curr->e_time = atoi( value);        }
	else if( !memcmp( name, "week",  	4))     { sprintf( curr->week, "%s", value);  }
	else if( !memcmp( name, "path",  	4))     { sprintf( curr->path, "%s", value);  }
	else if( !memcmp( name, "name",  	4))     { sprintf( curr->name, "%s", value);  }
	else if( !memcmp( name, "option",	6))     { curr->option = atoi( value);        }
	else if( !memcmp( name, "r_cnt", 	5))     { curr->r_cnt = atoi( value);         }
	else if( !memcmp( name, "max_run",	7))     { curr->max_run = atoi( value);       }
	else										
	{
		printf( "필드가 없습니다. name=[%s]\n", name);
		return 0;
	}
	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_Infofield( PROC *proc, PROC_TBL *curr, char *name)
{
	int len;
	
	len = strlen( name);

	if( !memcmp( name, "id",            len)) printf( "id      프로그램 ID                       = [%s]\n", curr->id);
	else if( !memcmp( name, "cmdd",     len)) printf( "cmd     프로세스 Control 명령             = [%d]\n", curr->cmd);
	else if( !memcmp( name, "used",     len)) printf( "used    레코드 사용여부                   = [%d]\n", curr->used);
	else if( !memcmp( name, "stat",     len)) printf( "stat    프로세스상태(0:none 1:run 9:kill) = [%d]\n", curr->stat);
	else if( !memcmp( name, "pid",      len)) printf( "pid     priocess id                       = [%d]\n", curr->pid);
	else if( !memcmp( name, "s_time",   len)) printf( "s_time  시작시간 HHMMSS                   = [%d]\n", curr->s_time);
	else if( !memcmp( name, "e_time",   len)) printf( "e_time  종료시간 HHMMSS                   = [%d]\n", curr->e_time);
	else if( !memcmp( name, "week",     len)) printf( "week    실행요일 일월화수목금토공         = [%s]\n", curr->week);
	else if( !memcmp( name, "path",     len)) printf( "path    프로그램 위치                     = [%s]\n", curr->path);
	else if( !memcmp( name, "name",     len)) printf( "name    프로그램 실행 명령                = [%s]\n", curr->name);
	else if( !memcmp( name, "option",   len)) printf( "option  respawn(0:no 1:yes)               = [%d]\n", curr->option);
	else if( !memcmp( name, "r_cnt",    len)) printf( "r_cnt   실행횟수                          = [%d]\n", curr->r_cnt);
	else if( !memcmp( name, "max_run",  len)) printf( "max_run 최대 실행 횟수                    = [%d]\n", curr->max_run);
	else if( !memcmp( name, "sr_timr",  len)) printf( "sr_timr 프로세스 실행 시간                = [%s]\n", TtoS( curr->sr_time));
	else if( !memcmp( name, "er_time",  len)) printf( "er_time 프로세스 종료 시간                = [%s]\n", TtoS( curr->er_time));
	else return 0;

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_SetField( PROC *proc, PROC_TBL *curr, char *name, char *value)
{
	int len;

	len = strlen( name);

	if( !memcmp( name, "id",            len)) { sprintf( curr->id, "%s", value);    }
	else if( !memcmp( name, "cmdd",     len)) { curr->cmd = atoi( value);           }
	else if( !memcmp( name, "used",     len)) { curr->used = atoi( value);          }
	else if( !memcmp( name, "stat",     len)) { curr->stat = atoi( value);          }
	else if( !memcmp( name, "pid",      len)) { curr->pid = atoi( value);           }
	else if( !memcmp( name, "s_time",   len)) { curr->s_time = atoi( value);        }
	else if( !memcmp( name, "e_time",   len)) { curr->e_time = atoi( value);        }
	else if( !memcmp( name, "week",     len)) { sprintf( curr->week, "%s", value);  }
	else if( !memcmp( name, "path",     len)) { sprintf( curr->path, "%s", value);  }
	else if( !memcmp( name, "name",     len)) { sprintf( curr->name, "%s", value);  }
	else if( !memcmp( name, "option",   len)) { curr->option = atoi( value);        }
	else if( !memcmp( name, "r_cnt",    len)) { curr->r_cnt = atoi( value);         }
	else if( !memcmp( name, "max_run",  len)) { curr->max_run = atoi( value);       }
	else return 0;

	return 1;
}

/** ***************************************************************************
**	@fn			Proc_( )
**	@param		PROC *Proc - process pointer
**	@return		+   성공
**	@retval		-   실패
**	@exception
**	@remark
**	@brief		
**	프로세스 관리
***************************************************************************** */
int Proc_DailyJob( PROC *proc)
{
	time_t		cur_time;


	time( &cur_time);
	LogMsg( "DailyJob start. at=[%s]", TtoS( cur_time));
	Proc_CheckPid( proc);
	ProcTbl_ClearRec( proc->mem);

	return 1;
}





