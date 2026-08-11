#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <errno.h>

#include "cfg.h"
#include "stp.h"
#include "etc.h"

extern int errno;
char	SwapFileName[ 512];


/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP	*Stp_Create( char *cfg_name)
{
	int		rtn, i;
	CFG		*cfg;
	STP		*stp;
	int		sz;
	char	*ptr;
	int		create_flag = 0;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}
	LogDel( "Cfg_Open success. cfg=[%p] name=[%s]", cfg, cfg_name);

	stp = malloc( sizeof( STP));
	if( stp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( STP));
		goto error_1;
	}
	memset( stp, 0x00, sizeof( STP));
	LogDel( "stp malloc. stp=[%p] sz=[%d]", stp, sizeof( STP));

	stp->mem = Mem_Open( STP_MEM_KEY);
	if( stp->mem == NULL)
	{
		LogWar( "Mem_Open error. key=[0x%08x]", STP_MEM_KEY);
		sz = ( sizeof( STP_TBL) * MAX_STP_TBL) + sizeof( STP_STAT);
		stp->mem = Mem_Create( STP_MEM_KEY, sz);
		if( stp->mem == NULL)
		{
			LogCri( "Mem_Create and Mem_Open error. key=[0x%08x] sz=[%d]", STP_MEM_KEY, sz);
			goto error_2;
		}
		create_flag = 1;
	}
	LogDel( "Mem_Create or Mem_Open success. mem=[%p] key=[0x%08x] sz=[%d]", stp->mem, STP_MEM_KEY, sz);

	ptr = Mem_GetPtr( stp->mem);
	stp->base = ( STP_TBL *)ptr;
	stp->stat = ( STP_STAT *)&ptr[ sizeof( STP_TBL) * MAX_STP_TBL];
	time( &stp->stat->create);

	Cfg_Get( cfg, "swap_file_name", stp->stat->swap_name, 512);
	Cfg_Get( cfg, "sam_file_path", stp->stat->path, 512);

	if( create_flag)
	{
		Stp_LoadSwap( stp, cfg_name);
		Stp_LoadCfg( stp, cfg_name);
	}

	Cfg_Close( cfg);

	for( i = 0; i < stp->stat->cnt; i++)
	{
		stp->curr = &stp->base[ i];
		rtn = Stp_MakeIpc( stp);
		if( rtn < 0)
		{
			LogCRI( "Stp_MakeIpc error. stp=[%p]", stp);
			goto error_2;
		}
	}

	return stp;

	error_2:
		free( stp);
	error_1:
		Cfg_Close( cfg);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Remove( STP *stp)
{
	int		rtn;
	char	fifo_name[ 512], stp_name[ 512];

	if( stp == NULL)
	{
		LogCri( "Stp이 없습니다. stp=(null)"); 
		return -1;
	}
	if( stp->curr == NULL)
	{
		LogCri( "Stp file table이 없습니다. stp->curr=(null)"); 
		return -1;
	}

	/* stp file remove */
	sprintf( stp_name, "%s/%s.stp", stp->stat->path, stp->curr->name);
	LogDel( "remove stp file... name=[%s]", stp_name);
	if( stp->fd >= 0)
	{
		LogDel( "close stp file... name=[%s] fd=[%d]", stp_name, stp->fd);
		close( stp->fd);
		stp->fd = -1;
	}
	rtn = unlink( stp_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", stp_name);
	else
		LogDel( "unlink success. name=[%s]", stp_name);

	/* fifo remove */
	sprintf( fifo_name, "%s/%s.fifo", stp->stat->path, stp->curr->name);
	LogDel( "remove fifo... name=[%s]", fifo_name);
	if( stp->fifo >= 0)
	{
		LogDel( "close fifo... name=[%s] fd=[%d]", fifo_name, stp->fifo);
		close( stp->fifo);
		stp->fifo = -1;
	}
	rtn = unlink( fifo_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", fifo_name);
	else
		LogDel( "unlink success. name=[%s]", fifo_name);

	/* semaphore remove */
	if( stp->sem == NULL)
	{
		stp->sem = Sem_Open( stp->curr->key);
	}

	if( stp->sem != NULL)
	{
		LogDel( "remove semaphore... key=[0x%08x], ptr=[%p]", stp->curr->key, stp->sem);
		rtn = Sem_Remove( stp->sem);
		if( rtn < 0)
			LogCri( "Sem_Remove error.");
		else
			LogDel( "Sem_Remove success.");

		stp->sem = NULL;
	}

	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_RemoveAll( STP *stp)
{
	int		i;
	int		rtn;
	char	fifo_name[ 512], stp_name[ 512];

	if( stp == NULL)
	{
		LogCri( "Stp이 없습니다. stp=(null)"); 
		return -1;
	}
	if( stp->curr == NULL)
	{
		LogCri( "Stp file table이 없습니다. stp->curr=(null)"); 
		return -1;
	}

	for( i = 0; i < stp->stat->cnt; i++)
	{
		stp->curr = &stp->base[ i];
		rtn = Stp_Remove( stp);
		if( rtn < 0)
		{
			LogMsg( "remove fail!!!!! id=[%s]", stp->curr->id);
			sleep( 1);
		}
		else
		{
			LogMsg( "remove success.  id=[%s]", stp->curr->id);
		}
	}

	/* shared memort detach */
	if( stp->mem != NULL)
	{
		LogDel( "shared memory detached. ptr=[%p]", stp->mem);
		Mem_Remove( stp->mem);
	}

	free( stp);


	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Clear( STP *stp)
{
	int		rtn;
	time_t	cur_time;
	char	fifo_name[ 512], stp_name[ 512], bak_name[512], bak_path[ 512];

	if( stp == NULL)
	{
		LogCri( "Stp이 없습니다. stp=(null)"); 
		return -1;
	}
	if( stp->curr == NULL)
	{
		LogCri( "Stp file table이 없습니다. stp->curr=(null)"); 
		return -1;
	}

	/* stp file rename */
	sprintf( stp_name, "%s/%s.stp", stp->stat->path, stp->curr->name);

	memcpy( bak_path, stp->stat->backup_path, strlen( stp->stat->backup_path) +1);
	time( &cur_time);
	TtoA( bak_path, cur_time);
	rtn = mkdir( bak_path, 0777);
	LogApp( "백업 디랙토리 생성 ... name=[%s] rtn=[%d]", bak_path, rtn);
	sprintf( bak_name, "%s/%s.stp", bak_path, stp->curr->name);
	LogDel( "rename stp file... name=[%s->%s]", stp_name, bak_name);
	if( stp->fd > 0)
	{
		LogDel( "close stp file... name=[%s] fd=[%d]", stp_name, stp->fd);
		close( stp->fd);
		stp->fd = -1;
	}
	rtn = rename( stp_name, bak_name);
	if( rtn < 0)
		LogErr( "rename error. name=[%s->%s]", stp_name, bak_name);
	else
		LogDel( "rename success. name=[%s->%s]", stp_name, bak_name);

	/* new stp file create */
	stp->fd = open( stp_name, O_CREAT + O_RDWR + O_EXCL, 0666);
	if( stp->fd < 0)
	{
		LogErr( "stp file create error. name=[%s]", stp_name);
		return -1;
	}
	LogDel( "stp file create success. name=[%s] stp->fd=[%d]", fifo_name, stp->fd);
	close( stp->fd);

	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_Open( char *id)
**	@param		id - stp id
**	@return		정상	stp pointer
**	@retval		오류	NULL
**	@exception
**	@remark
**	@brief		
**	malloc stp struct, attach shared memory, open ipc 매체(semaphore/fifo/file)
***************************************************************************** */
STP *Stp_Open( char *id)
{
	int		rtn;
	char	*ptr;
	STP		*stp;
	char	fifo_name[ 512], stp_name[ 512];

	stp = malloc( sizeof( STP));
	if( stp == NULL)
	{
		LogErr( "malloc error. stp size=[%d]", sizeof( STP));
		goto error;
	}
	memset( stp, 0x00, sizeof( STP));
	LogDel( "stp malloc. stp=[%p]", stp);

	stp->mem = Mem_Open( STP_MEM_KEY);
	if( stp->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", STP_MEM_KEY);
		goto error_1;
	}
	ptr = Mem_GetPtr( stp->mem);
	stp->base = ( STP_TBL *)ptr;
	stp->curr = stp->base;
	stp->stat = ( STP_STAT *)&ptr[ sizeof( STP_TBL) * MAX_STP_TBL];

	if( id == NULL)
	{
		LogMsg( "관리자 mode open. id=[null] stp=[%p]", stp);
		return stp;
	}

	rtn = Stp_OpenIpc( stp, id);
	if( rtn < 0)
	{
		LogCri( "Stp_OpenIpc error. rtn=[%d]", rtn);
		goto error_2;
	}

	return stp;

	error_2:
		Mem_Close( stp->mem);
	error_1:
		free( stp);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_OpenBySubid( char *subid)
**	@param		id - stp id
**	@return		정상	stp pointer
**	@retval		오류	NULL
**	@exception
**	@remark
**	@brief		
**  Stp_Opne과 같은 역할 수행 ... 단 파라메터를 subid(STP_TBL->sub_id)로 받는다.
**	malloc stp struct, attach shared memory, open ipc 매체(semaphore/fifo/file)
***************************************************************************** */
STP *Stp_OpenBySubId( char *subid)
{
	int		rtn;
	char	*ptr;
	char	id[ 32];
	STP		*stp;
	STP_TBL	*tp;
	char	fifo_name[ 512], stp_name[ 512];

	if( subid == NULL)
	{
		LogMsg( "Stp_OpenBySubid는 관리자 mode를 지원하지 않습니다. subid=[%s]", subid);
		return NULL;
	}

	stp = malloc( sizeof( STP));
	if( stp == NULL)
	{
		LogErr( "malloc error. stp size=[%d]", sizeof( STP));
		goto error;
	}
	memset( stp, 0x00, sizeof( STP));
	LogDel( "stp malloc. stp=[%p]", stp);

	stp->mem = Mem_Open( STP_MEM_KEY);
	if( stp->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", STP_MEM_KEY);
		goto error_1;
	}
	ptr = Mem_GetPtr( stp->mem);
	stp->base = ( STP_TBL *)ptr;
	stp->curr = stp->base;
	stp->stat = ( STP_STAT *)&ptr[ sizeof( STP_TBL) * MAX_STP_TBL];

	tp = Stp_FindSubId( stp, subid);
	if( tp == NULL)
	{
		LogCri( "subid not found. subid=[%s]", subid);
		goto error_2;
	}
	memcpy( id, tp->id, strlen( tp->id) +1);

	rtn = Stp_OpenIpc( stp, id);
	if( rtn < 0)
	{
		LogCri( "Stp_OpenIpc error. rtn=[%d]", rtn);
		goto error_2;
	}

	return stp;

	error_2:
		Mem_Close( stp->mem);
	error_1:
		free( stp);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_OpenIpc( STP *stp, char *id)
**	@param		STP		*stp - stp 구조체 pointer
**	@param		char	*id - stp id
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송매체 open sem/fifo/file - create는 daemon에서 한다
**  side effect로 stp->curr를 전달된 id의 테이블 포인터 값으로 세팅
***************************************************************************** */
int Stp_OpenIpc( STP *stp, char *id)
{
	int		rtn;
	char	*ptr;
	char	fifo_name[ 512], stp_name[ 512];

	stp->curr = Stp_FindId( stp, id);
	if( stp->curr == NULL)
	{
		LogCri( "Stp_FindId error. id=[%s]", id);
		goto error;
	}

	stp->sem = Sem_Open( stp->curr->key);
	if( stp->sem == NULL)
	{
		LogCri( "Sem_Open error. key=[0x%08x]", stp->curr->key);
		goto error;
	}
	LogDel( "Sem_Open success. key=[0x%08x]", stp->curr->key);

	sprintf( fifo_name, "%s/%s.fifo", stp->stat->path, stp->curr->name);
	stp->fifo = open( fifo_name, O_RDWR);
	if( stp->fifo < 0)
	{
		LogErr( "fifo open error. name=[%s]", fifo_name);
		goto error_1;
	}
	LogDel( "fifo open success. name=[%s] fifo=[%d]", fifo_name, stp->fifo);

	sprintf( stp_name, "%s/%s.stp", stp->stat->path, stp->curr->name);
	stp->fd = open( stp_name, O_RDWR);
	if( stp->fd < 0)
	{
		LogErr( "stp_file open error. name=[%s]", stp_name);
		goto error_2;
	}
	LogDel( "stp_file open success. name=[%s] fifo=[%d]", stp_name, stp->fd);

	return 1;

	error_3:
		close( stp->fifo);
	error_2:
		stp->fifo = -1;
	error_1:
		Sem_Close( stp->sem);
	error:
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP *Stp_MakeSub( STP *stpp, char *id)
{
	int		rtn;
	STP		*stp;

	stp = malloc( sizeof( STP));
	if( stp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( STP));
		goto error;
	}
	memset( stp, 0x00, sizeof( STP));

	stp->mem = stpp->mem;
	stp->base = stpp->base;
	stp->stat = stpp->stat;
	stp->curr = stpp->curr;
	/*
	memcpy( stp->stat->path, stpp->stat->path, strlen( stpp->stat->path) +1);
	memcpy( stp->stat->bak_path, stpp->stat->bak_path, strlen( stpp-stat->>bak_path) +1);
	*/

	rtn = Stp_OpenIpc( stp, id);
	if( rtn < 0)
	{
		LogCri( "Stp_OpenIpc error. rtn=[%d]", rtn);
		goto error_1;
	}

	return stp;

	error_1:
		free( stp);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP_WRITE *Stp_OpenWrite( STP *stp)
{
	int				rtn, i, cnt;
	STP				*sp;
	STP_WRITE		*swp;
	char			fifo_name[ 512], stp_name[ 512];

	swp = malloc( sizeof( STP_WRITE));
	if( swp == NULL)
	{
		LogErr( "malloc error. swp size=[%d]", sizeof( STP_WRITE));
		goto error;
	}
	memset( swp, 0x00, sizeof( STP_WRITE));
	LogDel( "stp malloc. swp=[%p]", swp);

	swp->mem  = stp->mem;
	swp->base = stp->base;
	swp->stat = stp->stat;
	swp->curr = stp->curr;
	cnt = swp->cnt = swp->stat->cnt;
	LogDel( "cnt=[%d]", cnt);

	for( i = 0; i < cnt; i++)
	{
		LogMsg( "stp process. pos=[%d]", i);
		swp->stp[ i] = malloc( sizeof( STP));
		if( swp->stp[ i] == NULL)
		{
			LogErr( "malloc error.");
			goto error_2;
		}
		memset( swp->stp[ i], 0x00, sizeof( STP));
		sp = swp->stp[ i];
		memcpy( sp, stp, sizeof( STP));
		sp->curr = &swp->base[ i];
		sp->fifo = -1;
		sp->fd = -1;
		LogMsg( "open process. pos=[%d] id=[%s]", i, sp->curr->id);

		sp->sem = Sem_Open( sp->curr->key);
		if( sp->sem == NULL)
		{
			LogCri( "Sem_Open error. key=[0x%08x]", sp->curr->key);
			goto error_2;
		}
		LogDel( "semaphore open success. sem=[%p]", sp->sem);

		sprintf( fifo_name, "%s/%s.fifo", sp->stat->path, sp->curr->name);
		sp->fifo = open( fifo_name, O_RDWR);
		if( sp->fifo < 0)
		{
			LogErr( "fifo open error. name=[%s]", fifo_name);
			goto error_2;
		}
		LogDel( "fifo open success. fifo=[%d]", sp->fifo);

		sprintf( stp_name, "%s/%s.stp", sp->stat->path, sp->curr->name);
		sp->fd = open( stp_name, O_RDWR);
		if( sp->fd < 0)
		{
			LogErr( "stp file open error. name=[%s]", stp_name);
			goto error_2;
		}
		LogDel( "stp file open success. fd=[%d]", sp->fd);
	}

	return swp;

	error_3:
		close( stp->fifo);
		stp->fifo = -1;
	error_2:
		for( ; i >= 0; i--)
		{
			sp = swp->stp[ i];
			if( sp == NULL) continue;

			if( sp->sem != NULL) 			{ Sem_Close( sp->sem);	sp->sem = NULL; }
			if( sp->fifo >= 0)				{ close( sp->fifo);		sp->fifo = -1; }
			if( sp->fd >= 0)				{ close( sp->fd);		sp->fd = -1; }
			if( sp != NULL) 				{ free( sp);			swp->stp[ i] = NULL; }
		}
	error_1:
		free( swp);
	error:
		return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_CloseWrite( STP_WRITE *swp)
{
	int		i, cnt;
	STP		*stp;

	cnt = swp->stat->cnt;

	for( i = 0; i < cnt; i++)
	{
		stp = swp->stp[ i];
		if( stp == NULL) continue;
		if( stp->sem != NULL)
		{
			Sem_Close( stp->sem);
			stp->sem = NULL;
		}
		if( stp->fifo >= 0)
		{
			close( stp->fifo);
			stp->fifo = -1;
		}
		if( stp->fd >= 0)
		{
			close( stp->fd);
			stp->fd = -1;
		}
		free( stp);
		stp = NULL;
	}
	free( swp);
	swp = NULL;
	/* shared memory는 close 하지않음 - stp_write 오픈할때 인수인 stp에서 close */

	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Close( STP *stp)
{
	int		rtn;

	if( stp->fd >= 0)
	{
		LogDel( "stp_file close. fd=[%d]", stp->fd);
		close( stp->fd);
		stp->fd = -1;
	}
	if( stp->fifo >= 0)
	{
		LogDel( "fifo close. fifo=[%d]", stp->fifo);
		close( stp->fifo);
		stp->fifo = -1;
	}
	if( stp->sem != NULL)
	{
		LogDel( "semaphore close. sem=[%p]", stp->sem);
		Sem_Close( stp->sem);
		stp->sem = NULL;
	}
	if( stp->mem != NULL)
	{
		LogDel( "deteched shared memory. mem=[%p]", stp->mem);
		Mem_Close( stp->mem);
		stp->mem = NULL;
		stp->curr = NULL;
	}

	free( stp);
	stp = NULL;

	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_LoadSwap( STP *stp, char *cfg_name)
{
	int			rtn;
	int			fd;
	int			sz, cmp_sz;
	struct stat	st;
	char		swap_file_name[ 512];
	char		ver_string[ 64];
	char		*ptr;
	STP_STAT	*stp_st;
	CFG			*cfg;


	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}

	rtn = Cfg_Get( cfg, "swap_file_name", swap_file_name, 512);
	if( rtn <= 0)
	{
		LogWar( "config load error. name=[swap_file_name] cfg=[%p]", cfg);
	}
	LogDel( "swap_file_name=[%s]", swap_file_name);

	rtn = Cfg_Get( cfg, "VERSION", ver_string, 64);
	if( rtn <= 0)
	{
		LogWar( "config load error. name=[VERSION] cfg=[%p]", cfg);
	}
	LogDel( "VERSION=[%s]", ver_string);

	sz = ( sizeof( STP_TBL) * MAX_STP_TBL) + sizeof( STP_STAT);
	rtn = stat( swap_file_name, &st);
	if( rtn < 0)
	{
		LogErr( "stat error. name=[%s]",  swap_file_name);
		return 0;
	}
	if( st.st_size != sz)
	{
		LogCri( "스왑파일 크기가 일치하지 않습니다. sz=[%d] st_size=[%d]", sz, st.st_size);
		return 0;
	}

	ptr = malloc( sz);
	if( ptr == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sz);
		goto error;
	}
	LogDel( "malloc. ptr=[%p] sz=[%d]", ptr, sz);

	fd = open( swap_file_name, O_RDONLY);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", swap_file_name);
		goto error_1;
	}
	LogDel( "open file. name=[%s] fd=[%d]", swap_file_name, fd);

	rtn = read( fd, ptr, sz);
	if( rtn != sz)
	{
		LogCri( "스왑파일을 읽지 못했습니다. name=[%s] fd=[%d] sz=[%d] rtn=[%d]", swap_file_name, fd, sz, rtn);
		goto error_2;
	}
	stp_st = (STP_STAT *)&ptr[ sizeof( STP_TBL) * MAX_STP_TBL];

	cmp_sz = Max( strlen( ver_string), strlen( stp_st->ver_string));
	if( memcmp( stp_st->ver_string, ver_string, cmp_sz))
	{
		LogCri( "스왑파일 버전이 다릅니다. file->ver_string=[%s] cfg->ver_string=[%s]", stp_st->ver_string, ver_string);
		goto error_2;
	}

	memcpy( stp->base, ptr, sz);
	LogDel( "스왑파일을 메모리로 로드 하였습니다. stp->base=[%p] sz=[%d]", stp->base, sz);

	close( fd);
	free( ptr);
	Cfg_Close( cfg);

	return sz;

	error_2:
		close( fd);
	error_1:
		free( ptr);
		Cfg_Close( cfg);
	error:
		LogCri( "Stp_LoadSwap error.");
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP *Stp_GetStpPtr( STP_WRITE *swp, char *id)
{
	int		i, cnt;
	int		sz, csz;
	STP		*sp;

	cnt = swp->cnt;
	sz = strlen( id);

	for( i = 0; i < cnt; i++)
	{
		sp = swp->stp[ i];
		if( sp == NULL) continue;

		LogDel( "compare arg=[%s] stp=[%s]", id, sp->curr->id);

		csz = strlen( sp->curr->id);
		if( sz != csz) continue;
		if( !memcmp( sp->curr->id, id, sz)) return sp;
	}
	return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_LoadCfg( STP *stp, char *cfg_name)
{
	int			rtn;
	int			update_flag = 0;
	char		*rec;
	STP_TBL	*tp;
	CFG			*cfg;

	LogDel( "Stp_LoadCfg config file open. name=[%s]", cfg_name);
	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}

	Cfg_Get( cfg, "VERSION", stp->stat->ver_string, sizeof( stp->stat->ver_string));
	Cfg_Get( cfg, "sam_file_path", stp->stat->path, sizeof( stp->stat->path));
	Cfg_Get( cfg, "bak_file_path", stp->stat->backup_path, sizeof( stp->stat->backup_path));

	rec = Cfg_GetFirstNamePtr( cfg, "stp_table");
	while( rec != NULL)
	{
		LogDel( "Cfg_GetFirstNamePtr rec=[%s]", rec);
		tp = Stp_MakeTbl( stp, rec);
		if( tp != NULL)
		{
			rtn = Stp_UpdateTbl( stp, tp);
			if( rtn < 0) goto error;
			if( rtn > 0)	/* update table */
			{
				LogMsg( "테이블이 변경되었으므로 IPC를 재로드합니다.");
				Stp_MakeIpc( stp);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "stp_table");
	}

	Cfg_Close( cfg);

	return 1;

	error:
		Cfg_Close( cfg);
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_LoadCfgForce( STP *stp, char *cfg_name)
{
	int			rtn;
	int			update_flag = 0;
	char		*rec;
	STP_TBL	*tp;
	CFG			*cfg;

	LogDel( "Stp_LoadCfg config file open. name=[%s]", cfg_name);
	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}

	Cfg_Get( cfg, "VERSION", stp->stat->ver_string, sizeof( stp->stat->ver_string));
	Cfg_Get( cfg, "sam_file_path", stp->stat->path, sizeof( stp->stat->path));
	Cfg_Get( cfg, "bak_file_path", stp->stat->backup_path, sizeof( stp->stat->backup_path));

	rec = Cfg_GetFirstNamePtr( cfg, "stp_table");
	while( rec != NULL)
	{
		LogDel( "Cfg_GetFirstNamePtr rec=[%s]", rec);
		tp = Stp_MakeTbl( stp, rec);
		if( tp != NULL)
		{
			/* Stp_UpdateTbl - side effect ... stp->curr 재설정 */
			rtn = Stp_UpdateTbl( stp, tp);
			if( rtn < 0) goto error;
			LogMsg( "IPC를 재로드합니다. id=[%s]", stp->curr->id);
			Stp_MakeIpc( stp);
		}
		rec = Cfg_GetNextNamePtr( cfg, "stp_table");
	}

	Cfg_Close( cfg);

	return 1;

	error:
		Cfg_Close( cfg);
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	config 파일 로드 - config 파일의 load_config 플레그를 체크
**  Stp_CfgLoad call
***************************************************************************** */
int Stp_CfgCheck( STP *stp, char *cfg_name)
{
	int			rtn, load_config = 0;
	int			update_flag = 0;
	char		*rec;
	CFG			*cfg;
	STP_TBL		*tp;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	load_config = Cfg_GetInt( cfg, "load_config");
	Cfg_Close( cfg);

	LogDel( "Check config... name=[%s] load_config=[%d]", cfg_name, load_config);
	if( load_config == 0) 
	{
		return 0;
	}

	Stp_CfgLoad( stp, cfg_name);

	return 1;

	error:
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_CfgLoad( STP *stp, char *cfg_name)
{
	int			rtn, load_config = 0;
	int			update_flag = 0;
	char		*rec;
	CFG			*cfg;
	STP_TBL		*tp;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	rec = Cfg_GetFirstNamePtr( cfg, "stp_table");
	while( rec != NULL)
	{
		LogDel( "config read. rec=[%s]", rec);
		tp = Stp_MakeTbl( stp, rec);
		if( tp != NULL)
		{
			rtn = Stp_UpdateTbl( stp, tp);
			if( rtn < 0) goto error;
			else if( rtn > 0)
			{
				LogMsg( "테이블이 변경되었으므로 IPC를 재로드합니다.");
				Stp_MakeIpc( stp);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "stp_table");
	}

	Cfg_Close( cfg);

	return 1;

	error:
		Cfg_Close( cfg);
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	load_config를 체크하지 않고 config 파일을 로드
**  변경부분 체크없이 무조건 로드한다
***************************************************************************** */
int Stp_CheckCfg( STP *stp, CFG *cfg)
{
	int			rtn, load_config = 0;
	int			update_flag = 0;
	char		*rec;
	STP_TBL		*tp;

	load_config = Cfg_GetInt( cfg, "load_config");
	LogDel( "Check config... load_config=[%d]", load_config);
	if( load_config == 0) return 0;


	rec = Cfg_GetFirstNamePtr( cfg, "stp_table");
	while( rec != NULL)
	{
		tp = Stp_MakeTbl( stp, rec);
		if( tp != NULL)
		{
			rtn = Stp_UpdateTbl( stp, tp);
			if( rtn < 0) goto error;
			else if( rtn > 0)
			{
				LogMsg( "테이블이 변경되었으므로 IPC를 재로드합니다.");
				Stp_MakeIpc( stp);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "stp_table");
	}

	return 1;

	error:
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_SwapFile( STP *stp, char *swap_file_name)
{
	int			rtn;
	int			update_flag = 0;
	int			fd;
	size_t		sz;
	char		*rec;

	LogDel( "swap process. name=[%s]", swap_file_name);

	fd = open( swap_file_name, O_CREAT | O_RDWR, 0666);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", swap_file_name);
		return -1;
	}

	sz = ( sizeof( STP_TBL) * MAX_STP_TBL) + sizeof( STP_STAT);

	rtn = write( fd, stp->base, sz);
	if( rtn != sz)
	{
		LogCri( "Swap file write error. sz=[%d] write_sz=[%d]", sz, rtn);
		goto error;
	}
	LogDel( "swap to file. fd=[%d] sz=[%d]", fd, sz);

	close( fd);
	return 1;

	error:
		close( fd);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_LoadFile( STP *stp, char *swap_file_name)
{
	int			rtn;
	int			update_flag = 0;
	int			fd;
	size_t		sz;
	char		*rec;

	LogDel( "load from file process. name=[%s]", swap_file_name);

	fd = open( swap_file_name, O_RDWR);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", swap_file_name);
		return -1;
	}

	sz = ( sizeof( STP_TBL) * MAX_STP_TBL) + sizeof( STP_STAT);

	rtn = read( fd, stp->base, sz);
	if( rtn != sz)
	{
		LogErr( "Swap file read error. fd=[%d] sz=[%d] read_sz=[%d]", fd, sz, rtn);
		goto error;
	}
	LogMsg( "load from file. fd=[%d] sz=[%d]", fd, sz);

	close( fd);
	return 1;

	error:
		close( fd);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP_TBL *Stp_MakeTbl( STP *stp, char *rec)
{
	int				stat = 0;
	static STP_TBL	tblbuf;
	STP_TBL			*tbl = &tblbuf;
	char			*token = ":\n";
	char			buf[ 8192], *ptr;
	int				*iptr;

	memcpy( buf, rec, strlen( rec) +1);
	memset( tbl, 0x00, sizeof( STP_TBL));

	ptr = strtok( buf, token);
	while( ptr != NULL)
	{
		LogDel( "stat=[%d] ptr=[%s]", stat, ptr);
		switch( stat)
		{
			case 0:		/* id */
				Trim( ptr);
				memcpy( tbl->id, ptr, strlen( ptr) +1);
				break;
			case 1:		/* used */
				tbl->used = StoI( ptr);
				break;
			case 2:		/* type */
				tbl->type = StoI( ptr);
				break;
			case 3:		/* sem key */
				tbl->key = StoI( ptr);
				break;
			case 4:		/* name */
				Trim( ptr);
				memcpy( tbl->name, ptr, strlen( ptr) +1);
				break;
			case 5:		/* record length */
				tbl->rlen = atoi( ptr);
				break;
			case 6:		/* delimiter */
				iptr = ( int *)tbl->dm;
				*iptr = StoI( ptr);
				break;
			case 7:		/* bind address */
				Trim( ptr);
				memcpy( tbl->baddr, ptr, strlen( ptr) +1);
				memcpy( tbl->caddr, ptr, strlen( ptr) +1);
				break;
			case 8:		/* port */
				tbl->sport = atoi( ptr);
				tbl->cport = atoi( ptr);
				break;
			case 9:		/* fm */
				memcpy( tbl->fm, ptr, sizeof( tbl->fm));
				break;
			case 10:		/* sub_id */
				Trim( ptr);
				memcpy( tbl->sub_id, ptr, strlen( ptr) +1);
				break;
			case 11:	/* 회사명 */
				Trim( ptr);
				memcpy( tbl->com_name, ptr, strlen( ptr) +1);
				break;
			default:
				break;
	
		}
		ptr = strtok( NULL, token);
		stat++;
	}

	if( stat < 9)
	{
		LogCri( "Stp_MakeTbl error. stat=[%d] rec=[%s]", stat, rec);
		return NULL;
	}

	/* Stp_PrintTbl( stp, tbl); */
	LogDel( "make tbl. tbl=[%p] id=[%-15s] key=[0x%08x] name=[%15s] rlne=[%d] dm=[0x%08x]", 
			tbl, tbl->id, tbl->key, tbl->name, tbl->rlen, tbl->dm);
	return tbl;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_MakeIpc( STP *stp)
{
	int			rtn;
	char		f_name[ 512];

	LogMsg( "MakeIpc start..................................");
	LogMsg( "Stp ipc make. stp=[%p] id=[%s]", stp, stp->curr->id);

	if( stp->sem != NULL)
	{
		LogMsg( "Sem close. sem=[%p]", stp->sem);
		Sem_Close( stp->sem);
		stp->sem = NULL;
	}
	if( stp->fifo > 0)
	{
		LogMsg( "fifo close. fifo=[%d]", stp->fifo);
		close( stp->fifo);
		stp->fifo = -1;
	}
	if( stp->fd > 0)
	{
		LogMsg( "stp file close. fd=[%d]", stp->fd);
		close( stp->fd);
		stp->fd = -1;
	}

	stp->sem = Sem_Open( stp->curr->key);
	if( stp->sem == NULL)
	{
		LogMsg( "Sem_Open error. key=[0x%08x]", stp->curr->key);
		stp->sem = Sem_Create( stp->curr->key);
		if( stp->sem == NULL)
		{
			LogCri( "Sem_Open and Sem_Create error. key=[0x%08x]", stp->curr->key);
			goto error;
		}
	}
	LogMsg( "semaphore open or create. key=[0x%08x] id=[%d]", stp->curr->key, Sem_GetId( stp->sem));

	sprintf( f_name, "%s/%s.fifo", stp->stat->path, stp->curr->name);
	stp->fifo = open( f_name, O_RDWR);
	if( stp->fifo < 0)
	{
		LogMsg( "fifo open error. rtn=[%d] name=[%s] err=[%d:%s]", rtn, f_name, errno, strerror( errno));
		rtn = mkfifo( f_name, 0666);
		if( rtn < 0)
		{
			LogErr( "open and mkfifo error. name=[%s]", f_name);
			goto error_1;
		}
		stp->fifo = open( f_name, O_RDWR);
	}
	LogMsg( "fifo oepn or create. name=[%s] fd=[%d]", f_name, stp->fifo);

	sprintf( f_name, "%s/%s.stp", stp->stat->path, stp->curr->name);
	stp->fd = open( f_name, O_RDWR);
	if( stp->fd < 0)
	{
		LogMsg( "stp file open error. rtn=[%d] name=[%s] err=[%d:%s]", rtn, f_name, errno, strerror( errno));
		stp->fd = open( f_name, O_CREAT | O_RDWR, 0666);
		if( stp->fd < 0)
		{
			LogErr( "open and create error. name=[%s]", f_name);
			goto error_2;
		}
	}
	LogMsg( "stp file oepn or create. name=[%s] fd=[%d]", f_name, stp->fd);


	if( stp->fd > 0)		close( stp->fd);
	if( stp->fifo > 0)		close( stp->fifo);
	if( stp->sem != NULL)	Sem_Close( stp->sem);
	return 1;

	error_2:
		close( stp->fifo);
	error_1:
		Sem_Close( stp->sem);
	error:
		LogMsg( "Stp_MakeIpc error.");
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_UpdateTbl( STP *stp, STP_TBL *tbl)
{
	int			update_flag = 0;
	int			*optr, *nptr;

	stp->curr = Stp_FindId( stp, tbl->id);
	if( stp->curr == NULL)
	{
		LogDel( "Stp table not found. add table. id=[%s]", tbl->id);

		/* insert stp table - 같은 id가 없으면 새로운 record */
		stp->curr = &stp->base[ stp->stat->cnt];
		memcpy( stp->curr, tbl, sizeof( STP_TBL));
		LogMsg( "Add stp table. id=[%s] pos=[%d]", stp->curr->id, stp->stat->cnt);
		stp->curr->pos = stp->stat->cnt;
		stp->stat->cnt++;
		Stp_PrintTbl( stp, stp->curr);
		return 1;
	}

	/* update stp table */
	if( memcmp( stp->curr->id, tbl->id, Max( strlen( stp->curr->id), strlen( tbl->id) +1))) return 0;

	/* protocol type */
	if( stp->curr->type != tbl->type)
	{
		LogMsg( "Stp table update protocol type. stp->curr->type old=[%d] new=[%d]", stp->curr->type, tbl->type);
		update_flag = 1;
		stp->curr->type = tbl->type;
	}

	/* used */
	if( stp->curr->used != tbl->used)
	{
		LogMsg( "Stp table update used. stp->curr->used old=[%d] new=[%d]", stp->curr->used, tbl->used);
		update_flag = 1;
		stp->curr->used = tbl->used;
	}

	/* 세마포어 key */
	if( stp->curr->key != tbl->key)
	{
		LogMsg( "Stp table update semaphore key. stp->curr->key old=[0x%08x] new=[0x%08x]", stp->curr->key, tbl->key);
		update_flag = 1;
		stp->curr->key = tbl->key;
	}
	/* 파일 name */
	if( memcmp( stp->curr->name, tbl->name, Max( strlen( stp->curr->name), strlen( tbl->name) +1)))
	{
		LogMsg( "Stp table update ipc file name. stp->curr->name old=[%s] new=[%s]", stp->curr->name, tbl->name);
		update_flag = 1;
		memcpy( stp->curr->name, tbl->name, strlen( tbl->name) +1);
	}
	/* record 길이 */
	if( stp->curr->rlen != tbl->rlen)
	{
		LogMsg( "Stp table update record length key. stp->curr->rlen old=[%d] new=[%d]", stp->curr->rlen, tbl->rlen);
		update_flag = 1;
		stp->curr->rlen = tbl->rlen;
	}
	/* 딜리미터 */
	optr = ( int *)&stp->curr->dm;
	nptr = ( int *)&tbl->dm;
	if( *optr != *nptr)
	{
		LogMsg( "Stp table update delimiter. stp->curr->dm old=[0x%08x] new=[0x%08x]", *optr, *nptr);
		*optr = *nptr;
	}
	/* bind address */
	if( memcmp( stp->curr->baddr, tbl->baddr, Max( strlen( stp->curr->baddr), strlen( tbl->baddr) +1)))
	{
		LogMsg( "Stp table update record bind address. stp->curr->baddr old=[%s] new=[%s]", stp->curr->baddr, tbl->baddr);
		update_flag = 1;
		memcpy( stp->curr->baddr, tbl->baddr, strlen( tbl->baddr) +1);
		memcpy( stp->curr->caddr, tbl->baddr, strlen( tbl->baddr) +1);
	}
	/* server port */
	if( stp->curr->sport != tbl->sport)
	{
		LogMsg( "Stp table update record server port. stp->curr->sport old=[%d] new=[%d]", stp->curr->sport, tbl->sport);
		update_flag = 1;
		stp->curr->sport = tbl->sport;
		stp->curr->cport = tbl->sport;
	}
	/* fm */
	if( memcmp( stp->curr->fm, tbl->fm, Max( strlen( stp->curr->fm), strlen( tbl->fm) +1)))
	{
		LogMsg( "Stp table update record fm. stp->curr->fm old=[%s] new=[%s]", stp->curr->fm, tbl->fm);
		update_flag = 1;
		memcpy( stp->curr->fm, tbl->fm, sizeof( stp->curr->fm));
	}
	/* sub_id */
	if( memcmp( stp->curr->sub_id, tbl->sub_id, Max( strlen( stp->curr->sub_id), strlen( tbl->sub_id) +1)))
	{
		LogMsg( "Stp table update record sub_id. stp->curr->sub_id old=[%s] new=[%s]", stp->curr->sub_id, tbl->sub_id);
		update_flag = 1;
		memcpy( stp->curr->sub_id, tbl->sub_id, strlen( tbl->sub_id) +1);
	}
	/* com_name */
	if( memcmp( stp->curr->com_name, tbl->com_name, Max( strlen( stp->curr->com_name), strlen( tbl->com_name) +1)))
	{
		LogMsg( "Stp table update record com_name. stp->curr->com_name old=[%s] new=[%s]", stp->curr->com_name, tbl->com_name);
		update_flag = 1;
		memcpy( stp->curr->com_name, tbl->com_name, strlen( tbl->com_name) +1);
	}

	if( update_flag == 1)	return 1;
	else					return 0;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP_TBL *Stp_FindId( STP *stp, char *id)
{
	int		sz, c_sz, i;
	int		find_flag = 0;
	STP_TBL	*curr;

	sz = strlen( id);
	for( i = 0; i < stp->stat->cnt; i++)
	{
		curr = &stp->base[ i];
		c_sz = strlen( curr->id);
		if( sz != c_sz) continue;
		if( !memcmp( curr->id, id, sz))
		{
			find_flag = 1;
			break;
		}
	}
	if( find_flag)	return curr;
	else			return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
STP_TBL *Stp_FindSubId( STP *stp, char *subid)
{
	int		sz, c_sz, i;
	int		find_flag = 0;
	STP_TBL	*curr;

	sz = strlen( subid);
	for( i = 0; i < stp->stat->cnt; i++)
	{
		curr = &stp->base[ i];
		c_sz = strlen( curr->sub_id);
		if( sz != c_sz) continue;
		if( !memcmp( curr->sub_id, subid, sz))
		{
			find_flag = 1;
			break;
		}
	}
	if( find_flag)	return curr;
	else			return NULL;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Write( STP *stp, char *rec, int sz)
{
	int			rtn;
	int			rec_len, space_len;
	int			seq, pos;
	off_t		seek_pos, seek_rtn;
	STP_HEAD	head, *hp = &head;;
	char		buf[ 65535];
	int			buf_pos;
	ssize_t		wcnt = 0;
	time_t		cur_time;

	LogDel( "Stp_Write len=[%d]", sz);
	space_len = stp->curr->rlen - sz;
	if( space_len < 0)
	{
		LogCri( "record length too long. rec_len=[%d] len=[%d]", stp->curr->rlen, sz);
		return -1;
	}

	time( &cur_time);

	rtn = Sem_Lock( stp->sem);
	if( rtn < 0)
	{
		LogErr( "Sem_Lock error. rtn=[%d]", rtn);
		return -1;
	}

	pos = stp->curr->wpos;
	rec_len = Stp_GetRecLen( stp);
	seek_pos = rec_len * pos;

	seek_rtn = lseek( stp->fd, seek_pos, SEEK_SET);
	if( seek_pos != seek_rtn)
	{
		LogErr( "lseek error. seek_pos[%lld]!=seek_rtn[%lld]", seek_pos, seek_rtn);
		goto error;
	}

	/* 순번은 위치 +1로 --- 위치정보는 0부터 시작, 순번은 1부터 시작 */
	ItoA( ( char *)hp->seq,  stp->curr->wpos +1, sizeof( hp->seq));
	ItoA( ( char *)hp->len,  sz, sizeof( hp->len));
	memcpy( hp->time, TtoS( cur_time), sizeof( hp->time));
	hp->spc[0] = ' ';

	buf_pos = 0;
	memcpy( &buf[ buf_pos], hp, sizeof( STP_HEAD));
	buf_pos += sizeof( STP_HEAD);
	memcpy( &buf[ buf_pos], rec, sz);
	buf_pos += sz;
	memset( &buf[ buf_pos], 0x20, space_len);
	buf_pos += space_len;
	memcpy( &buf[ buf_pos], stp->curr->dm, strlen( stp->curr->dm));
	buf_pos += strlen( stp->curr->dm);
	buf[ buf_pos] = 0;

	if( buf_pos != rec_len)
	{
		LogCri( "Size mismatch. buf_pos=[%d] rec_len=[%d]", buf_pos, rec_len);
		goto error;
	}

	wcnt = write( stp->fd, buf, buf_pos);
	if( wcnt != rec_len)
	{
		LogCri( "Stp write error. fd=[%d] rec_len[%d] != wcnt[%d]", stp->fd, rec_len, wcnt);
		goto error;
	}

	rtn = write( stp->fifo, " ", 1);
	if( rtn < 1)
	{
		LogCri( "Fifo write error. fifo=[%d]", stp->fifo);
		goto error;
	}

	stp->curr->wpid = getpid();
	stp->curr->wtime = cur_time;

	stp->curr->wpos++;
	switch( stp->curr->type)
	{
		default:
			LogCri( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
			stp->curr->wseq = ( SEQ_T)stp->curr->wpos;
			break;
		case 0:		/* old */
		case 1:		/* old */
			stp->curr->wseq = (SEQ_T)stp->curr->wpos + stp->curr->bseq;
			break;
		case 2:		/* new2 */
		case 3:		/* new3 */
		case 4:		/* push */
		case 5:		/* fix */
			stp->curr->wseq = (SEQ_T)stp->curr->wpos - stp->curr->bseq;
			break;
	}

	rtn = Sem_Unlock( stp->sem);

	//LogDump( buf, sz, "Stp write. stp->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%.*s]", stp->curr->wpos, sz, sz, rec);
	LogMsg("Stp write. stp->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%.300s]", stp->curr->wpos, sz, rec);
	return 1;

	error:
		LogDump( rec, sz, "Stp file write error. stp=[%p] id=[%s] rec=[%p] sz=[%d]", stp, stp->curr->id, rec, sz);
		Sem_Unlock( stp->sem);
		return -1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_WriteId( STP_WRITE *stp_write, char *id, char *rec, int sz)
{
	int		i, rtn;
	int		id_sz, cmp_sz;
	STP	*cstp = NULL;

	LogDel( "Stp_WriteId start... stp_write=[%p] id=[%s] rec=[%p] sz=[%d]", stp_write, id, rec, sz);
	id_sz = strlen( id);

	for( i = 0; i < stp_write->cnt; i++)
	{
		LogDel( "cstp=[%p]", stp_write->stp[i]);
		LogDel( "compare ... i=[%3d] id=[%s]", i, stp_write->stp[ i]->curr->id);
		cmp_sz = Max( strlen( stp_write->stp[ i]->curr->id), id_sz);
		if( memcmp( stp_write->stp[ i]->curr->id, id, cmp_sz)) continue;
		cstp = stp_write->stp[ i];
		break;
	}

	if( cstp == NULL)
	{
		LogCri( "Id not found. id=[%s]", id);
		return -1;
	}

	rtn = Stp_Write( cstp, rec, sz);
	if( rtn < 0)
	{
		LogCri( "Stp_Write error. rec=[%p] sz=[%d] rtn=[%d]", rec, sz, rtn);
		return rtn;
	}
	LogDel( "Sam_Write success. sz=[%d] rtn=[%d] seq=[%lld]", sz, rtn, cstp->curr->wseq);

	return rtn;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Read( STP *stp, char *rec, int sz)
{
	int			rtn, rec_len, cp_sz;
	off_t		pos, rtn_pos;
	char		buf[ 65535];
	STP_HEAD	*hd = ( STP_HEAD *)&buf;
	int			rec_sz;
	SEQ_T	seq;

	if( stp->curr->rpos >= stp->curr->wpos)
	{
		return 0;
	}

	rec_len = Stp_GetRecLen( stp);
	pos = Stp_GetPos( stp, stp->curr->rpos);
	if( pos < 0)
	{
		LogCri( "Stp file pos error. rpos=[%d]", stp->curr->rpos);
		return -1;
	}
	
	rtn_pos = lseek( stp->fd, pos, SEEK_SET);
	if( rtn_pos != pos)
	{
		LogCri( "lseek error. pos=[%lld] rtn_pos=[%lld]", pos, rtn_pos);
		return -1;
	}

	rtn = read( stp->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		LogErr( "Stp file read error. request=[%d] rtn=[%d]", rec_len, rtn);
		return -1;
	}

	seq = AtoLL( hd->seq, sizeof( hd->seq));
	rec_sz = AtoI( hd->len, sizeof( hd->len));
	if( seq != stp->curr->rpos)
	{
		LogCri( "Stp file read error. seq=[%d] tbl->seq=[%d]", seq, stp->curr->rpos);
		return -1;
	}

	if( sz <= rec_sz)
	{
		LogCri( "Record size is too short. sz=[%d] read_size=[%d]", sz, rec_sz);
		return -1;
	}
	memcpy( rec, &buf[ sizeof( STP_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	stp->curr->rpos++;
	stp->curr->rseq++;;

	return rec_sz;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_ReadT( STP *stp, char *rec, int sz, int timeout)
{
	int				rtn, rec_len, cp_sz;
	off_t			spos, rtn_pos;
	char			buf[ 65535];
	STP_HEAD		*hp = ( STP_HEAD *)&buf;
	STP_TBL			*tp;
	int				rec_sz;
	SEQ_T		seq;
	time_t			cur_time, rec_time;

	tp = stp->curr;
	LogDel( "------------------------------------------------------------------");
	LogDel( "check rpos=[%d] >= wpos=[%d]", tp->rpos, tp->wpos);

	if( tp->rpos < tp->wpos) timeout = 0;
	rtn = Stp_WaitFifo( stp, timeout);
	if( rtn < 0)
	{
		LogCri( "Stp_WaitFifo error. stp=[%p] timeout=[%d]", stp, timeout);
		return -1;
	}
	else
	if( rtn == 0 && timeout != 0)
	{
		LogMsg( "Stp_WaitFifo timeout. timeout=[%d]", timeout);
		return 0;
	}

	rec_len = Stp_GetRecLen( stp);
	spos = Stp_GetPos( stp, tp->rpos +1);
	if( spos < 0)
	{
		LogCri( "Stp file pos error. rpos=[%d]", tp->rpos);
		return -1;
	}
	
	rtn_pos = lseek( stp->fd, spos, SEEK_SET);
	if( rtn_pos != spos)
	{
		LogCri( "lseek error. spos=[%lld] rtn_pos=[%lld]", spos, rtn_pos);
		return -1;
	}

	rtn = read( stp->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		LogErr( "Stp file read error. request=[%d] rtn=[%d]", rec_len, rtn);
		return -errno;
	}

	seq = AtoLL( hp->seq, sizeof( hp->seq));
	rec_sz = AtoI( hp->len, sizeof( hp->len));
	if( sz <= rec_sz)
	{
		LogCri( "Buffer size is too short. sz=[%d] read_size=[%d]", sz, rec_sz);
		return -1;
	}

	time( &cur_time);
	rec_time = StoT( hp->time);
	if( cur_time - rec_time > 1L)
	{
		LogWar( "수신 전문에 시간차가 %d초 발생했습니다.", cur_time - rec_time);
	}

	memcpy( rec, &buf[ sizeof( STP_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	tp->rpid = getpid();
	time( &tp->rtime);

	tp->rpos++;
	tp->rseq++;;

	LogDump( buf, rec_len, "Stp_Read success. tp->rpos=[%d] rec_sz(sz)=[%d] rec=[%s]", tp->rpos, rec_sz, rec);

	return rec_sz;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_ReadST( STP *stp, SEQ_T seq, char *rec, int sz, int timeout)
{
	int				rtn, rec_len, cp_sz;
	off_t			spos, rtn_pos;
	char			buf[ 65535];
	STP_TBL			*tp;
	STP_HEAD		*hd = ( STP_HEAD *)&buf;
	int				pos, rec_sz;
	time_t			cur_time, rec_time;

	LogDbg( "Stp_ReadPT start. stp=[%p] pos=[%d] rec=[%p] sz=[%d] to=[%d]", stp, pos, rec, sz, timeout);
	LogDbg( "seq      = [%lld]", seq);

	tp = stp->curr;
	switch( stp->curr->type)
	{
		default:
			LogCri( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
			pos = ( int)seq;
			break;
		case 0:		/* old */
		case 1:		/* old */
			pos = ( int)( seq - stp->curr->bseq);
			break;
		case 2:		/* new2 */
		case 3:		/* new3 */
		case 4:		/* push */
		case 5:		/* fix */
			pos = ( int)( seq + stp->curr->bseq);
			break;
	}
	rec_len = Stp_GetRecLen( stp);

	LogDbg( "pos      = [%d]", pos);
	LogDbg( "wpos     = [%d]", tp->wpos);
	
	if( pos <= tp->wpos) timeout = 0;
	LogDbg( "timeout  = [%d.%d]", timeout / 1000000, timeout % 1000000);
	rtn = Stp_WaitFifo( stp, timeout);
	if( rtn < 0)
	{
		LogCri( "Stp_WaitFifo error. stp=[%p] timeout=[%d] rtn=[%d]", stp, timeout, rtn);
		return -1;
	}
	else
	if( rtn == 0 && timeout != 0)
	{
		LogDbg( "Stp_WaitFifo timeout. timeout=[%d]", timeout);
		return 0;
	}

	spos = Stp_GetPos( stp, pos);
	if( spos < 0)
	{
		LogCri( "no data. rpos=[%d]", tp->rpos);
		return 0;
	}

	rtn_pos = lseek( stp->fd, spos, SEEK_SET);
	if( rtn_pos != spos)
	{
		LogCri( "lseek error. spos=[%lld] rtn_pos=[%lld]", spos, rtn_pos);
		return 0;
	}

	rtn = read( stp->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		LogErr( "Stp file read error. fd=[%d] buf=[%p] sz=[%d] rtn=[%d]", stp->fd, buf, rec_len, rtn);
		return 0;
	}
	LogDbg( "HEAD seq         = [%*.*s]", sizeof( hd->seq),  sizeof( hd->seq),  hd->seq);
	LogDbg( "HEAD len         = [%*.*s]", sizeof( hd->len),  sizeof( hd->len),  hd->len);
	LogDbg( "HEAD time        = [%*.*s]", sizeof( hd->time), sizeof( hd->time), hd->time);

	pos = AtoI( hd->seq, sizeof( hd->seq));
	rec_sz = AtoI( hd->len, sizeof( hd->len));
	if( sz <= rec_sz)
	{
		LogCri( "Buffer size is too short. sz=[%d] read_size=[%d]", sz, rec_sz);
		return -1;
	}

	time( &cur_time);
	rec_time = StoT( hd->time);
	if( cur_time - rec_time > 1L)
	{
		LogWar( "수신 전문에 시간차가 %d초 발생했습니다.", cur_time - rec_time);
	}

	memcpy( rec, &buf[ sizeof( STP_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	tp->rpid = getpid();
	time( &tp->rtime);

	tp->rpos = pos;
	switch( stp->curr->type)
	{
		default:
			LogCri( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
			tp->rseq = ( SEQ_T)tp->rpos;
			break;
		case 0:		/* old */
		case 1:		/* old */
			tp->rseq = (SEQ_T)seq;
			break;
		case 2:		/* new2 */
		case 3:		/* new3 */
		case 4:		/* push */
		case 5:		/* fix */
			tp->rseq = (SEQ_T)seq;
			break;
	}

	LogDbg( "rpos     = [%d]", tp->rpos);
	LogDbg( "rseq     = [%lld]", tp->rseq);
	LogDump( buf, rec_sz, "Stp_Read success. tp->rpos=[%d] rec_sz(sz)=[%d] rec=[%.*s]", tp->rpos, rec_sz, rec_sz, rec);

	return rec_sz;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_WaitFifo( STP *stp, int timeout)
{
	int				rtn;
	fd_set			rfds;
	char			buf[ 8192];
	struct timeval	tv;

	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;

	FD_ZERO( &rfds);
	FD_SET( stp->fifo, &rfds);

	LogDel( "select wait... stp->fifo=[%d] timeout=[%d:%d]", stp->fifo, tv.tv_sec, tv.tv_usec);
	rtn = select( stp->fifo +1, &rfds, NULL, NULL, &tv);
	if( rtn < 0)
	{
		LogErr( "select error.");
		return -1;
	}
	else
	if( rtn == 0)
	{
		LogDel( "select timeout... stp->fifo=[%d] timeout=[%d:%d]", stp->fifo, tv.tv_sec, tv.tv_usec);
		return 0;
	}
	LogDel( "select... rtn=[%d]", rtn);


	if( FD_ISSET( stp->fifo, &rfds))
	{
		rtn = read( stp->fifo, buf, 8192);
		if( rtn <= 0)
		{
			LogCri( "Fifo read error. fifo=[%d] rtn=[%d]", stp->fifo, rtn);
			return -1;
		}
		LogDel( "Fifo read %d byte(s).", rtn);
		return 1;
	}

	return 0;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_GetReadPos( STP *stp)
{
	return stp->curr->rpos;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_GetRecLen( STP *stp)
{
	return sizeof( STP_HEAD) + stp->curr->rlen + strlen( stp->curr->dm);
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
off_t Stp_GetPos( STP *stp, int seq)
{
	off_t	pos;

	pos = (off_t)Stp_GetRecLen( stp) * ( off_t)( seq -1);

	LogDel( "stp pos=[%d] seq=[%d]", pos, seq);

	return pos;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
#if 1
int Stp_DailyProcess( STP *stp)
{
	int			i, cnt;
	time_t		cur_time;
	int			int_day, job_day;
	STP_TBL		*stpp;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = stp->stat->cnt;
	for( i = 0; i < cnt; i++)
	{
		stpp = &stp->base[ i];
		job_day = TtoD( stpp->daily);
		if( job_day == int_day)
		{
			LogCri( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			return 0;
		}
		LogMsg( "Daily clear sequence. stpp=[%p] id=[%s] type=[%d]", stpp, stpp->id, stpp->type);
		switch( stpp->type)
		{
			case 0:		/* type 0 - bseq(0) wseq(none) rseq(none) */
			case 1: 	/* type 1 - bseq(none) wseq(none) rseq(none) */
				LogMsg( "일일 작업이 없습니다.");
				break;
			case 2: 	/* type 2 - bseq(bseq+wseq) wseq(0) rseq(0) */
			case 3:
			case 4:		/* push */
			case 5:		/* fix  */
				LogMsg( "일일 작업이 시작.");
				LogMsg( "작업전 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업전 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업전 stpp->rseq   = [%lld]", stpp->rseq);
				if( stpp->wseq != stpp->rseq)
				{
					LogWar( "Sequence mismatch. id=[%s] wseq=[%lld] rseq=[%lld]", stpp->id, stpp->wseq, stpp->rseq);
				}
				stpp->bseq += stpp->wseq;
				stpp->wseq = 0;
				stpp->rseq = 0;
				LogMsg( "작업후 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업후 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업후 stpp->rseq   = [%lld]", stpp->rseq);
				break;
			default:	/* unknown  */
				LogCri( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
				LogMsg( "unknown type=[%d]", stpp->type);
				break;
		}
		time( &stpp->daily);
	}
}
#else
int Stp_DailyProcess( STP *stp)
{
	int			i, cnt;
	time_t		cur_time;
	int			int_day, job_day;
	STP_TBL		*tp;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = stp->stat->cnt;
	LogDel( "here... 1-1");
	for( i = 0; i < cnt; i++)
	{
		tp = &stp->base[ i];
		job_day = TtoD( tp->daily);
		if( job_day == int_day)
		{
			LogCri( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			return 0;
		}
		LogMsg( "Daily clear sequence. id =[%s]", tp->id);
		time( &tp->daily);
	}
}
#endif

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
#if 1
int Stp_WeeklyProcess( STP *stp)
{
	int			i, cnt;
	STP_TBL		*stpp;
	int			int_day, job_day;
	time_t		cur_time;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = stp->stat->cnt;
	for( i = 0; i < cnt; i++)
	{
		stpp = &stp->base[ i];
		job_day = TtoD( stpp->daily);
		if( job_day == int_day)
		{
			LogCri( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			return 0;
		}
		LogMsg( "Weekly clear sequence. id =[%s] type=[%d]", stpp->id, stpp->type);
		switch( stpp->type)
		{
			case 0:		/* type 0 (old) - bseq(0) wseq(0) rseq(0) */
			case 1: 	/* type 1 (new1) - bseq(none) wseq(none) rseq(none) */
				LogMsg( "주말 작업이 시작... type=[%d] - bseq(0) wseq(0) rseq(0)", stpp->type);
				LogMsg( "작업전 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업전 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업전 stpp->rseq   = [%lld]", stpp->rseq);
				stpp->wpos = 0;
				stpp->rpos = 0;
				stpp->bseq += stpp->wseq;
				stpp->wseq = stpp->bseq;
				stpp->rseq = stpp->bseq;
				LogMsg( "작업후 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업후 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업후 stpp->rseq   = [%lld]", stpp->rseq);
				break;
			case 2: 	/* type 2 (new2) - bseq(bseq+wseq) wseq(0) rseq(0) */
			case 3: 	/* type 3 (new3) - bseq(bseq+wseq) wseq(0) rseq(0) */
			case 4:		/* type 4 (push) */
			case 5:		/* type 5 (fix)  - bseq(0) wseq(0) rseq(0) */
				LogMsg( "주말 작업이 시작... type=[%d] - bseq(0) wseq(0) rseq(0)", stpp->type);
				LogMsg( "작업전 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업전 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업전 stpp->rseq   = [%lld]", stpp->rseq);
				if( stpp->wseq != stpp->rseq)
				{
					LogWar( "Sequence mismatch. id=[%s] wseq=[%lld] rseq=[%lld]", stpp->id, stpp->wseq, stpp->rseq);
				}
				stpp->wpos = 0;
				stpp->rpos = 0;
				stpp->bseq = 0;
				stpp->wseq = 0;
				stpp->rseq = 0;
				LogMsg( "작업후 stpp->base   = [%lld]", stpp->bseq);
				LogMsg( "작업후 stpp->wseq   = [%lld]", stpp->wseq);
				LogMsg( "작업후 stpp->rseq   = [%lld]", stpp->rseq);
				break;
			default:	/* unknown  */
				LogCri( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
				LogMsg( "unknown type=[%d]", stpp->type);
				break;
		}

		stp->curr = stpp;
		time( &stpp->daily);
		LogMsg( "IPC Clear 작업을 시작합니다. id=[%s] sub_id=[%s]", stpp->id, stpp->sub_id);
		Stp_Clear( stp);
	}

	return 1;
}

#else
int Stp_WeeklyProcess( STP *stp)
{
	int			i, cnt;
	STP_TBL		*tp;
	int			int_day, job_day;
	time_t		cur_time;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = stp->stat->cnt;
	for( i = 0; i < cnt; i++)
	{
		tp = &stp->base[ i];
		job_day = TtoD( tp->daily);
		if( job_day == int_day)
		{
			LogCri( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			return 0;
		}
		LogMsg( "Weekly clear sequence. id =[%s]", tp->id);
		tp->wpos = 0;
		tp->rpos = 0;
		stp->curr = tp;
		Stp_Clear( stp);
		time( &tp->daily);
	}
}
#endif

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Print( STP *stp)
{
	printf( "-----[ STP STAT ]----------------------------------------------------\n");
	printf( "stp             = [%p]\n", stp);
	printf( "mem             = [%p]\n", stp->mem);
	printf( "fifo            = [%d]\n", stp->fifo);
	printf( "fd              = [%d]\n", stp->fd);
	printf( "ver_string      = [%s]\n", stp->stat->ver_string);
	printf( "create          = [%s]\n", TtoS( stp->stat->create));
	printf( "swap_name       = [%s]\n", stp->stat->swap_name);
	printf( "path            = [%s]\n", stp->stat->path);
	printf( "backup_path     = [%s]\n", stp->stat->backup_path);
	printf( "----------------------------------------------------[ STP STAT ]-----\n");

	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_PrintList( STP *stp)
{
	int			i;
	int			col = 130;
	char		*ptr;
	STP_TBL		*st;

	for( i = 0; i < col; i++) printf( "-"); printf( "\n");
	printf( "                    u t                                                                                                  \n");
	printf( "                    s y                                                                                                  \n");
	printf( "                    e p                                                                                                  \n");
	printf( "pos id              d s  wpos   rpos   bseq   wseq   rseq  wtime    rtime    sem_key    name    sub_id          comp_name\n");
	for( i = 0; i < col; i++) printf( "-"); printf( "\n");

	for( i = 0; i < stp->stat->cnt; i++)
	{
		st = &stp->base[ i];
		printf( "%3d ", i);
		printf( "%-15s ", st->id);
		printf( "%1d ", st->used);
		printf( "%1d ", st->type);
		printf( "%6d ", st->wpos);
		printf( "%6d ", st->rpos);
		printf( "%6lld ", st->bseq);
		printf( "%6lld ", st->wseq);
		printf( "%6lld ", st->rseq);
		ptr = TtoS( st->wtime);
		printf( "%s ", &ptr[ 11]);
		ptr = TtoS( st->rtime);
		printf( "%s ", &ptr[ 11]);
		printf( "0x%08x ", st->key);
		printf( "%-7s ", st->name);
		printf( "%-15s ", st->sub_id);
		printf( "%-15s ", st->com_name);
		printf("\n");
	}
	for( i = 0; i < col; i++) printf( "-"); printf( "\n");
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_PrintTblRaw( STP *stp, STP_TBL *tp)
{
	LogRaw( "po ( int       pos            ) record pos                             = [%d]\n",     tp->pos            );
	LogRaw( "id ( char      id     [ 32]   ) stp file id                            = [%s]\n",     tp->id             );
	LogRaw( "ty ( int       type           ) stp protocol type                      = [%d]\n",     tp->type           );
	LogRaw( "up ( time_t    uptime         ) last update time                       = [%s]\n",     TtoS( tp->uptime)  );
	LogRaw( "da ( time_t    uptime         ) last daily job day                     = [%s]\n",     TtoS( tp->daily)   );
	LogRaw( "k  ( key_t     key            ) sem 접근 키                            = [0x%08x]\n", tp->key            );
	LogRaw( "wp ( int       wpos           ) write pos                              = [%d]\n",     tp->wpos           );
	LogRaw( "rp ( int       rpos           ) read pos                               = [%d]\n",     tp->rpos           );
	LogRaw( "bs ( long long bseq           ) base sequence                          = [%lld]\n",   tp->bseq           );
	LogRaw( "ws ( long long wseq           ) write sequence                         = [%lld]\n",   tp->wseq           );
	LogRaw( "rs ( long long rseq           ) read sequence                          = [%lld]\n",   tp->rseq           );
	LogRaw( "ss ( long long sseq           ) send sequence                          = [%lld]\n",   tp->sseq           );
	LogRaw( "   ( pid_t     wpid           ) last write process id                  = [%d]\n",     tp->wpid           );
	LogRaw( "   ( pid_t     rpid           ) last read process id                   = [%d]\n",     tp->rpid           );
	LogRaw( "   ( time_t    wtime          ) last write time                        = [%s]\n",     TtoS( tp->wtime)   );
	LogRaw( "   ( time_t    rtime          ) last read time                         = [%s]\n",     TtoS( tp->rtime)   );
	LogRaw( "   ( time_t    stime          ) last send time                         = [%s]\n",     TtoS( tp->stime)   );
	LogRaw( "   ( time_t    ctime          ) last connect time                      = [%s]\n",     TtoS( tp->ctime)   );
	LogRaw( "ba ( char      baddr  [ 64]   ) bind address                           = [%s]\n",     tp->baddr          );
	LogRaw( "ca ( char      caddr  [ 64]   ) server address                         = [%s]\n",     tp->caddr          );
	LogRaw( "sp ( int       sport          ) server wait port                       = [%d]\n",     tp->sport          );
	LogRaw( "cp ( int       cport          ) connect port                           = [%d]\n",     tp->cport          );
	LogRaw( "st ( int       stat           ) session stat                           = [%d]\n",     tp->stat           );
	LogRaw( "na ( char      name   [ 64]   ) stp file name ... *.fifo *.sam         = [%s]\n",     tp->name           );
	LogRaw( "rl ( int       rlen           ) stp file record length                 = [%d]\n",     tp->rlen           );
	LogRaw( "dm ( char      dm     [  4]   ) delimiter - \\r\\n or \\n or \\0           = [0x%08x]\n", *( int *)&tp->dm[0] );
	LogRaw( "si ( char      sub_id [ 32]   ) sub_id                                 = [%s]\n",     tp->sub_id         );
	LogRaw( "co ( char      com_name[ 32]  ) com_name                               = [%s]\n",     tp->com_name        );
	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_PrintTbl( STP *stp, STP_TBL *tp)
{
	printf( "po ( int       pos            ) record pos                             = [%d]\n",     tp->pos            );
	printf( "id ( char      id     [ 32]   ) stp file id                            = [%s]\n",     tp->id             );
	printf( "ty ( int       type           ) stp protocol type                      = [%d]\n",     tp->type           );
	printf( "up ( time_t    uptime         ) last update time                       = [%s]\n",     TtoS( tp->uptime)  );
	printf( "da ( time_t    uptime         ) last daily job day                     = [%s]\n",     TtoS( tp->daily)   );
	printf( "k  ( key_t     key            ) sem 접근 키                            = [0x%08x]\n", tp->key            );
	printf( "wp ( int       wpos           ) write pos                              = [%d]\n",     tp->wpos           );
	printf( "rp ( int       rpos           ) read pos                               = [%d]\n",     tp->rpos           );
	printf( "bs ( long long bseq           ) base sequence                          = [%lld]\n",   tp->bseq           );
	printf( "ws ( long long wseq           ) write sequence                         = [%lld]\n",   tp->wseq           );
	printf( "rs ( long long rseq           ) read sequence                          = [%lld]\n",   tp->rseq           );
	printf( "ss ( long long sseq           ) send sequence                          = [%lld]\n",   tp->sseq           );
	printf( "   ( pid_t     wpid           ) last write process id                  = [%d]\n",     tp->wpid           );
	printf( "   ( pid_t     rpid           ) last read process id                   = [%d]\n",     tp->rpid           );
	printf( "   ( time_t    wtime          ) last write time                        = [%s]\n",     TtoS( tp->wtime)   );
	printf( "   ( time_t    rtime          ) last read time                         = [%s]\n",     TtoS( tp->rtime)   );
	printf( "   ( time_t    stime          ) last send time                         = [%s]\n",     TtoS( tp->stime)   );
	printf( "   ( time_t    ctime          ) last connect time                      = [%s]\n",     TtoS( tp->ctime)   );
	printf( "ba ( char      baddr  [ 64]   ) bind address                           = [%s]\n",     tp->baddr          );
	printf( "ca ( char      caddr  [ 64]   ) server address                         = [%s]\n",     tp->caddr          );
	printf( "sp ( int       sport          ) server wait port                       = [%d]\n",     tp->sport          );
	printf( "cp ( int       cport          ) connect port                           = [%d]\n",     tp->cport          );
	printf( "st ( int       stat           ) session stat                           = [%d]\n",     tp->stat           );
	printf( "na ( char      name   [ 64]   ) stp file name ... *.fifo *.sam         = [%s]\n",     tp->name           );
	printf( "rl ( int       rlen           ) stp file record length                 = [%d]\n",     tp->rlen           );
	printf( "dm ( char      dm     [  4]   ) delimiter - \\r\\n or \\n or \\0           = [0x%08x]\n", *( int *)&tp->dm[0] );
	printf( "si ( char      sub_id [ 32]   ) sub_id                                 = [%s]\n",     tp->sub_id         );
	printf( "co ( char      com_name[ 32]  ) com_name                               = [%s]\n",     tp->com_name        );
	return 1;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_InfoField( STP *stp, STP_TBL *tp, char *name)
{
	char		*ptr, cmd[ 512];

	if( !memcmp( name, "po", 2))		
		printf( "po ( int       pos            ) record pos                             = [%d]\n",     tp->pos            );
	else if( !memcmp( name, "id", 2))	
		printf( "id ( char      id[ 32]        ) stp file id                            = [%s]\n",     tp->id             );
	else if( !memcmp( name, "up", 2))	
		printf( "up ( time_t    uptime         ) last update time                       = [%s]\n",     TtoS( tp->uptime)  );
	else if( !memcmp( name, "da", 2))	
		printf( "da ( time_t    uptime         ) last daily job day                     = [%s]\n",     TtoS( tp->daily)   );
	else if( !memcmp( name, "k" , 1))	
		printf( "k  ( key_t     key            ) sem 접근 키                            = [0x%08x]\n", tp->key            );
	else if( !memcmp( name, "wp", 2))	
		printf( "wp ( int       wpos           ) write pos                              = [%d]\n",     tp->wpos           );
	else if( !memcmp( name, "rp", 2))	
		printf( "rp ( int       rpos           ) read pos                               = [%d]\n",     tp->rpos           );
	else if( !memcmp( name, "na", 2))	
		printf( "na ( char      name[ 64]      ) stp file name ... *.fifo *.sam         = [%s]\n", tp->name               );
	else if( !memcmp( name, "rl", 2))	
		printf( "rl ( int       rlen           ) stp file record length                 = [%d]\n", tp->rlen               );
	else if( !memcmp( name, "bs", 2))	
		printf( "bs ( int       rlen           ) stp file record length                 = [%lld]\n", tp->bseq               );
	else if( !memcmp( name, "ws", 2))	
		printf( "ws ( int       rlen           ) stp file record length                 = [%lld]\n", tp->wseq               );
	else if( !memcmp( name, "rs", 2))	
		printf( "ss ( int       rlen           ) stp file record length                 = [%lld]\n", tp->rseq               );
	else if( !memcmp( name, "ss", 2))	
		printf( "ss ( int       rlen           ) stp file record length                 = [%lld]\n", tp->sseq               );

	else if( !memcmp( name, "ba", 2))	
		printf( "ba ( char      baddr  [ 64]   ) bind address                           = [%s]\n",     tp->baddr          );
	else if( !memcmp( name, "ca", 2))	
		printf( "ca ( char      caddr  [ 64]   ) server address                         = [%s]\n",     tp->caddr          );
	else if( !memcmp( name, "sp", 2))	
		printf( "sp ( int       sport          ) server wait port                       = [%d]\n",     tp->sport          );
	else if( !memcmp( name, "cp", 2))	
		printf( "cp ( int       cport          ) connect port                           = [%d]\n",     tp->cport          );
	else if( !memcmp( name, "st", 2))	
		printf( "st ( int       stat           ) session stat                           = [%d]\n",     tp->stat           );
	else if( !memcmp( name, "na", 2))	
		printf( "na ( char      name   [ 64]   ) stp file name ... *.fifo *.sam         = [%s]\n",     tp->name           );
	else if( !memcmp( name, "rl", 2))	
		printf( "rl ( int       rlen           ) stp file record length                 = [%d]\n",     tp->rlen           );
	else if( !memcmp( name, "si", 2))	
		printf( "si ( char      sub_id [ 32]   ) sub_id                                 = [%s]\n",     tp->sub_id         );
	else if( !memcmp( name, "co", 2))	
		printf( "co ( char      com_name[ 32]  ) com_name                               = [%s]\n",     tp->com_name        );

	else if( !memcmp( name, "dm", 2))	
		printf( "dm ( char      dm[ 4]         ) delimiter - \\r\\n or \\n or \\0           = [0x%08x]\n", *( int *)&tp->dm[0] );
	else	return 0;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_EditField( STP *stp, STP_TBL *tp, char *name, char *val)
{
	char		*ptr, cmd[ 512];

	if( !memcmp( name, "po", 2))		tp->pos = atoi( val);
	else if( !memcmp( name, "id", 2))	memcpy( tp->id, val, strlen( val) +1);
	else if( !memcmp( name, "up", 2))	tp->uptime = StoT( val);
	else if( !memcmp( name, "da", 2))	tp->daily  = StoT( val);
	else if( !memcmp( name, "k" , 1))	tp->key = StoI( val);
	else if( !memcmp( name, "wp", 2))	tp->wpos = AtoI( val, strlen( val));
	else if( !memcmp( name, "rp", 2))	tp->rpos = AtoI( val, strlen( val));
	else if( !memcmp( name, "na", 2))	memcpy( tp->name, val, strlen( val) +1);
	else if( !memcmp( name, "rl", 2))	tp->rlen = atoi( val);
	else if( !memcmp( name, "bs", 2))	tp->bseq = AtoLL( val, strlen( val));
	else if( !memcmp( name, "ws", 2))	tp->wseq = AtoLL( val, strlen( val));
	else if( !memcmp( name, "rs", 2))	tp->rseq = AtoLL( val, strlen( val));
	else if( !memcmp( name, "ss", 2))	tp->sseq = AtoLL( val, strlen( val));

	else if( !memcmp( name, "ba", 2))	memcpy( tp->baddr, val, strlen( val) +1);
	else if( !memcmp( name, "ca", 2))	memcpy( tp->caddr, val, strlen( val) +1);
	else if( !memcmp( name, "sp", 2))	tp->sport = AtoI( val, strlen( val));
	else if( !memcmp( name, "cp", 2))	tp->cport = AtoI( val, strlen( val));
	else if( !memcmp( name, "st", 2))	tp->stat = AtoI( val, strlen( val));
	else if( !memcmp( name, "na", 2))	memcpy( tp->caddr, val, strlen( val) +1);
	else if( !memcmp( name, "rl", 2))	tp->rlen = AtoI( val, strlen( val));
	else if( !memcmp( name, "si", 2))	memcpy( tp->sub_id, val, strlen( val) +1);
	else if( !memcmp( name, "co", 2))	memcpy( tp->com_name, val, strlen( val) +1);

	else if( !memcmp( name, "dm", 2))	*( int *)&tp->dm[0] = StoI( val);
	else	return 0;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_Stat( STP *stp, int stat)
{
	char	*stat_name[ 16] = { "NONE", "WAIT", "CONN", "END", NULL, 
			NULL, NULL, NULL, NULL, "ERR", NULL };

	LogDbg( "Set stat. id=[%s] stat=[%d:%s]", stp->curr->id, stat, stat_name[ stat]);

	switch( stat)
	{
		case 0:		/* STP_STAT_NONE */
			stp->curr->stat = 0;
			break;
		case 1:		/* STP_STAT_WAIT */
			stp->curr->stat = 1;
			break;
		case 2:		/* STP_STAT_CONN */
			stp->curr->stat = 2;
			time( &stp->curr->ctime);
			break;
		case 3:		/* STP_STAT_END */
			stp->curr->stat = 3;
			time( &stp->curr->dtime);
			break;
		case 9:		/* STP_STAT_ERR */
			stp->curr->stat = 9;
			break;
	}

	return stat;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
void *Stp_GetPtr( STP *stp, int opt)
{
	void	*ptr = NULL;

	switch( opt)
	{
		case STP_BIND_ADDR:
			return stp->curr->baddr;
		case STP_CONN_ADDR:
			return stp->curr->baddr;
		case STP_FORM_TYPE:
			return stp->curr->fm;
		default:
			break;
	}

	return ptr;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_GetInt( STP *stp, int opt)
{
	int		i = -1;

	switch( opt)
	{
		case STP_SVR_PORT:
			return stp->curr->sport;
		case STP_CLI_PORT:
			return stp->curr->cport;
		case STP_READ_POS:
			return stp->curr->rpos;
		case STP_WRITE_POS:
			return stp->curr->wpos;
		default:
			break;
	}

	return i;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
SEQ_T Stp_GetSeq( STP *stp, int opt)
{
	SEQ_T		ll = -1LL;

	switch( opt)
	{
		case STP_READ_SEQ:
			return stp->curr->rseq;
		case STP_WRITE_SEQ:
			return stp->curr->wseq;
		case STP_SEND_SEQ:
			return stp->curr->sseq;
		default:
			break;
	}

	return ll;
}

/** ***************************************************************************
**	@fn			int Stp_( )
**	@param		STP *stp - stp 구조체 pointer
**	@return		+   정상
**	@retval		-   오류
**	@exception
**	@remark
**	@brief		
**	stp data 전송
***************************************************************************** */
int Stp_SetSeq( STP *stp, int opt, SEQ_T seq)
{
	switch( opt)
	{
		case STP_READ_SEQ:
			switch( stp->curr->type)
			{
				default:
					LogWar( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
					stp->curr->rseq = seq; 
					stp->curr->rpos = ( int)seq; 
					break;
				case 0: 
				case 1:
					stp->curr->rseq = seq; 
					stp->curr->rpos = ( int )(seq - stp->curr->bseq); 
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					stp->curr->rseq = seq; 
					stp->curr->rpos = ( int)(seq + stp->curr->bseq);
					break;
			}
			break;
		case STP_WRITE_SEQ:
			switch( stp->curr->type)
			{
				default:
					LogWar( "정의되지 않은 TYPE입니다. type=[%d]", stp->curr->type);
					stp->curr->wseq = seq;
					stp->curr->wpos = ( int)seq;
					break;
				case 0:
				case 1:
					stp->curr->wseq = seq;
					stp->curr->wpos = ( int)( seq - stp->curr->bseq);
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					stp->curr->wseq = seq;
					stp->curr->wpos = ( int)( seq + stp->curr->bseq);
					break;
			}
			break;
		case STP_SEND_SEQ:
			stp->curr->sseq = seq;
			break;
		default:
			break;
	}

	return 1;
}







