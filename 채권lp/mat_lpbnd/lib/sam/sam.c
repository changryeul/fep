#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <errno.h>

#include "cfg.h"
#include "mem.h"
#include "sem.h"
#include "sam.h"
#include "etc.h"

extern int errno;
/*
char	SwapFileName[ 512];
*/

SAM	*Sam_Create( char *cfg_name)
{
	CFG		*cfg;
	SAM		*sam;
	int		sz;
	char	*ptr;
	int		create_flag = 0;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}
	LogDbg( "Cfg_Open success. cfg=[%p] name=[%s]", cfg, cfg_name);

	sam = malloc( sizeof( SAM));
	if( sam == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( SAM));
		goto error_1;
	}
	memset( sam, 0x00, sizeof( SAM));
	sam->fd = -1;
	sam->fifo = -1;
	LogDbg( "sam malloc. sam=[%p] sz=[%d]", sam, sizeof( SAM));

	/* only read sam->key */
	Sam_LoadCfg( sam, cfg_name);

	sam->mem = Mem_Open( sam->key);
	if( sam->mem == NULL)
	{
		LogWar( "Mem_Open error. key=[0x%08x]", sam->key);
		sz = ( sizeof( SAM_TBL) * MAX_SAM_TBL) + sizeof( SAM_STAT);
		sam->mem = Mem_Create( sam->key, sz);
		if( sam->mem == NULL)
		{
			LogCri( "Mem_Create and Mem_Open error. key=[0x%08x] sz=[%d]", sam->key, sz);
			goto error_2;
		}
		create_flag = 1;
	}
	LogMsg( "Mem_Create or Mem_Open success. mem=[%p] key=[0x%08x] sz=[%d]", sam->mem, sam->key, sz);

	ptr = Mem_GetPtr( sam->mem);
	sam->base = ( SAM_TBL *)ptr;
	sam->stat = ( SAM_STAT *)&ptr[ sizeof( SAM_TBL) * MAX_SAM_TBL];
	sprintf( sam->stat->cfg_name, "%s", cfg_name);


	if( create_flag)
	{
		time( &sam->stat->create);
		sam->stat->key = sam->key;

		Sam_LoadCfg( sam, cfg_name);
		Sam_LoadSwap( sam);
	}

	return sam;

	error_2:
		free( sam);
	error_1:
		Cfg_Close( cfg);
	error:
		return NULL;
}

int Sam_Remove( SAM *sam)
{
	int		rtn;
	char	fifo_name[ 512], sam_name[ 512];

	if( sam == NULL)
	{
		LogCri( "Sam이 없습니다. sam=(null)"); 
		return -1;
	}
	if( sam->curr == NULL)
	{
		LogCri( "Sam file table이 없습니다. sam->curr=(null)"); 
		return -1;
	}

	LogMsg( "Sam ipc remove ......................... start");

	/* sam file remove */
	sprintf( sam_name, "%s/%s.sam", sam->stat->sam_path, sam->curr->name);
	if( sam->fd >= 0)
	{
		LogMsg( "sam file       close    = [%d]", sam->fd);
		close( sam->fd);
		sam->fd = -1;
	}
	LogMsg( "sam file       remove   = [%s]", sam_name);
	rtn = unlink( sam_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", sam_name);

	/* fifo remove */
	sprintf( fifo_name, "%s/%s.fifo", sam->stat->sam_path, sam->curr->name);
	LogDbg( "remove fifo... name=[%s]", fifo_name);
	if( sam->fifo >= 0)
	{
		LogMsg( "fifo           close    = [%d]", sam->fd);
		close( sam->fifo);
		sam->fifo = -1;
	}
	LogMsg( "fifo           remove   = [%s]", fifo_name);
	rtn = unlink( fifo_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", fifo_name);

	/* semaphore remove */
	LogDbg( "remove semaphore... key=[0x%08x], ptr=[%p]", sam->curr->key, sam->sem);
	if( sam->sem != NULL)
	{
		LogMsg( "semaphore      remove   = [%p]", sam->sem);
		rtn = Sem_Remove( sam->sem);
		if( rtn < 0)
			LogCri( "Sem_Remove error.");
		sam->sem = NULL;
	}

	/* shared memort detach */
	if( sam->mem != NULL)
	{
		LogMsg( "shared memory  detached = [%p]", sam->mem);
		Mem_Close( sam->mem);
	}
	LogMsg( "Sam ipc remove ......................... end");

	free( sam);


	return 1;
}

int Sam_RemoveSub( SAM *sam, SAM_TBL *tp)
{
	int		rtn;
	char	fifo_name[ 512], sam_name[ 512];

	/* sam file remove */
	sprintf( sam_name, "%s/%s.sam", sam->stat->sam_path, tp->name);
	LogMsg( "    file      = [%s]", sam_name);
	rtn = unlink( sam_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", sam_name);

	/* fifo remove */
	sprintf( fifo_name, "%s/%s.fifo", sam->stat->sam_path, tp->name);
	LogMsg( "    fifo      = [%s]", fifo_name);
	rtn = unlink( fifo_name);
	if( rtn < 0)
		LogErr( "unlink error. name=[%s]", fifo_name);

	/* semaphore remove */
	LogMsg( "    semaphore = [0x%08x]", tp->key);
	if( sam->sem == NULL)
	{
		sam->sem = Sem_Open( tp->key);
		if( sam->sem == NULL)
		{
			LogCri( "Sem_Open( tp->key=[0x%08x]) error.", tp->key);
		}
	}
	if( sam->sem != NULL)
	{
		rtn = Sem_Remove( sam->sem);
		if( rtn < 0)
			LogCri( "Sem_Remove error.");
		sam->sem = NULL;
	}

	return 1;
}

int Sam_RemoveAll( SAM *sam)
{
	int			rtn, i;
	char		fifo_name[ 512], sam_name[ 512];
	SAM_TBL		*tp;

	if( sam == NULL)
	{
		LogCri( "Sam이 없습니다. sam=(null)"); 
		return -1;
	}

	LogMsg( "Sam ipc remove ......................... start");

	/* swap file remove */
	LogMsg( "swap file       = [%s]", sam->stat->swp_name);
	unlink( sam->stat->swp_name);

	for( i = 0; i < sam->stat->cnt; i++)
	{
		tp = &sam->base[ i];
		LogMsg( "[%s]", tp->id);
		rtn = Sam_RemoveSub( sam, tp);
	}

	/* shared memort remove */
	if( sam->mem != NULL)
	{
		LogMsg( "shared memory   = [%p]", sam->mem);
		Mem_Remove( sam->mem);
	}
	LogMsg( "Sam ipc remove ......................... end");


	free( sam);


	return 1;
}

int Sam_ClearSub( SAM *sam, SAM_TBL *tp)
{
	int			rtn;
	time_t		cur_time;
	char		fifo_name[ 512], sam_name[ 512], bak_name[512], bak_path[ 512];
	char		*swp_name_ptr;
	struct stat st_buf;

	if( sam == NULL)
	{
		LogCri( "Sam이 없습니다. sam=(null)"); 
		return -1;
	}

	/* create backup directory */
	memcpy( bak_path, sam->stat->bak_path   , strlen( sam->stat->bak_path   ) +1);
	time( &cur_time);
	TtoA( bak_path, cur_time);
	rtn = stat( bak_path, &st_buf);
	if( rtn < 0)
	{
		rtn = mkdir( bak_path, 0777);
		LogApp( "백업 디랙토리 생성 ... name=[%s] rtn=[%d]", bak_path, rtn);
	}

	if( tp == NULL)
	{
		LogCri( "Sam file table이 없습니다. tp=(null)"); 
		return -1;
	}

	/* sam file rename */
	sprintf( sam_name, "%s/%s.sam", sam->stat->sam_path, tp->name);
	sprintf( bak_name, "%s/%s.sam", bak_path, tp->name);
	LogDbg( "rename sam file... name=[%s->%s]", sam_name, bak_name);
	if( sam->fd > 0)
	{
		LogDbg( "close sam file... name=[%s] fd=[%d]", sam_name, sam->fd);
		close( sam->fd);
		sam->fd = -1;
	}
	rtn = rename( sam_name, bak_name);
	if( rtn < 0)
		LogErr( "rename error. name=[%s->%s]", sam_name, bak_name);
	else
		LogDbg( "rename success. name=[%s->%s]", sam_name, bak_name);

	/* new sam file create */
	sam->fd = open( sam_name, O_CREAT + O_RDWR + O_EXCL, 0666);
	if( sam->fd < 0)
	{
		LogErr( "sam file create error. name=[%s]", sam_name);
		return -1;
	}
	LogDbg( "sam file create success. name=[%s] sam->fd=[%d]", sam_name, sam->fd);
	close( sam->fd);

	/* fifo check */
	sprintf( fifo_name, "%s/%s.fifo", sam->stat->sam_path, sam->curr->name);
	LogDbg( "check fifo... name=[%s]", fifo_name);
	rtn = stat( fifo_name, &st_buf);
	if( rtn < 0)
	{
		LogMsg( "stat( fifo_name=[%s], &st_buf) error. try mkfifo ...", fifo_name);
		sam->fifo = mkfifo( fifo_name, 0666);
		if( sam->fifo < 0)
		{
			LogErr( "open and mkfifo error. name=[%s]", fifo_name);
			return 1;
		}
		LogMsg( "sam->fifo       create  = [%d]", sam->fifo);
		close( sam->fifo);
		sam->fifo = -1;
	}

	/* semaphore check */
	sam->sem = Sem_Open( tp->key);
	if( sam->sem == NULL)
	{
		LogMsg( "Sem_Open( tp->key=[0x%08x]) error. try create ...", tp->key);
		sam->sem = Sem_Create( tp->key);
		LogMsg( "sam->sem        create  = [%p]", sam->sem);
		Sem_Close( sam->sem);
		sam->sem = NULL;
	}

	return 1;
}

int Sam_Clear( SAM *sam, SAM_TBL *tp)
{
	int			rtn, i;
	time_t		cur_time;
	SAM_TBL		*tbl_ptr;
	char		fifo_name[ 512], sam_name[ 512], bak_name[512], bak_path[ 512];
	char		*swp_name_ptr;
	struct stat st_buf;

	if( sam == NULL)
	{
		LogCri( "Sam이 없습니다. sam=(null)"); 
		return -1;
	}

	/* create backup directory */
	memcpy( bak_path, sam->stat->bak_path   , strlen( sam->stat->bak_path   ) +1);
	time( &cur_time);
	TtoA( bak_path, cur_time);
	rtn = stat( bak_path, &st_buf);
	if( rtn < 0)
	{
		rtn = mkdir( bak_path, 0777);
		LogApp( "백업 디랙토리 생성 ... name=[%s] rtn=[%d]", bak_path, rtn);
	}

	if( tp == NULL)
	{
		LogMsg( "모든 파일을 백업으로 옮깁니다. tp=(null)"); 
		/* swap file rename */
		rtn = stat( sam->stat->swp_name, &st_buf);
		if( rtn >= 0)
		{
			swp_name_ptr = strrchr( sam->stat->swp_name, '/');
			swp_name_ptr++;
			sprintf( bak_name, "%s/%s.sam", bak_path, swp_name_ptr);
			rtn = rename( sam->stat->swp_name, bak_name);
			if( rtn < 0)
				LogErr( "rename error. name=[%s->%s]", sam->stat->swp_name, bak_name);
			else
				LogDbg( "rename success. name=[%s->%s]", sam->stat->swp_name, bak_name);
		}
	}
	else
	{
		Sam_ClearSub( sam, tp);
		return 1;
	}

	for( i = 0; i < sam->stat->cnt; i++)
	{
		tbl_ptr = &sam->base[ i];
		rtn = Sam_ClearSub( sam, tbl_ptr);
		if( rtn < 0)
			LogWar( "Sam_ClearSub( sam=[%p], tbl_ptr=[%p]) error. id=[%s]", sam, tbl_ptr, tbl_ptr->id);
	}

	return 1;
}

SAM *Sam_Open( key_t key, char *id)
{
	int		rtn;
	char	*ptr;
	SAM		*sam;
	char	fifo_name[ 512], sam_name[ 512];

	sam = malloc( sizeof( SAM));
	if( sam == NULL)
	{
		LogErr( "malloc error. sam size=[%d]", sizeof( SAM));
		goto error;
	}
	memset( sam, 0x00, sizeof( SAM));
	LogDbg( "sam malloc. sam=[%p]", sam);
	sam->fd = -1;
	sam->fifo = -1;

	sam->mem = Mem_Open( key);
	if( sam->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", key);
		goto error_1;
	}
	ptr = Mem_GetPtr( sam->mem);
	sam->base = ( SAM_TBL *)ptr;
	sam->curr = sam->base;
	sam->stat = ( SAM_STAT *)&ptr[ sizeof( SAM_TBL) * MAX_SAM_TBL];

	if( id == NULL)
	{
		LogMsg( "관리자 mode open. id=[null] sam=[%p]", sam);
		return sam;
	}

	rtn = Sam_OpenIpc( sam, id);
	if( rtn < 0)
	{
		LogCri( "Sam_OpenIpc error. rtn=[%d]", rtn);
		goto error_2;
	}

	return sam;

	error_2:
		Mem_Close( sam->mem);
	error_1:
		free( sam);
	error:
		return NULL;
}

int Sam_OpenIpc( SAM *sam, char *id)
{
	int		rtn;
	char	*ptr;
	char	fifo_name[ 512], sam_name[ 512];

	sam->curr = Sam_FindId( sam, id);
	if( sam->curr == NULL)
	{
		LogCri( "Sam_FindId error. id=[%s]", id);
		goto error;
	}

	sam->sem = Sem_Open( sam->curr->key);
	if( sam->sem == NULL)
	{
		LogCri( "Sem_Open error. key=[0x%08x]", sam->curr->key);
		goto error;
	}
	LogDbg( "Sem_Open success. key=[0x%08x]", sam->curr->key);

	sprintf( fifo_name, "%s/%s.fifo", sam->stat->sam_path, sam->curr->name);
	sam->fifo = open( fifo_name, O_RDWR);
	if( sam->fifo < 0)
	{
		LogErr( "fifo open error. name=[%s]", fifo_name);
		goto error_1;
	}
	LogDbg( "fifo open success. name=[%s] fifo=[%d]", fifo_name, sam->fifo);

	sprintf( sam_name, "%s/%s.sam", sam->stat->sam_path, sam->curr->name);
	sam->fd = open( sam_name, O_RDWR);
	if( sam->fd < 0)
	{
		LogErr( "sam_file open error. name=[%s]", sam_name);
		goto error_2;
	}
	LogDbg( "sam_file open success. name=[%s] fd=[%d]", sam_name, sam->fd);

	return 1;

	error_3:
		close( sam->fifo);
	error_2:
		sam->fifo = -1;
	error_1:
		Sem_Close( sam->sem);
	error:
		return -1;
}

SAM *Sam_MakeSub( SAM *samp, char *id)
{
	int		rtn;
	SAM		*sam;

	sam = malloc( sizeof( SAM));
	if( sam == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( SAM));
		goto error;
	}
	memset( sam, 0x00, sizeof( SAM));

	sam->mem = samp->mem;
	sam->base = samp->base;
	sam->stat = samp->stat;
	sam->curr = samp->curr;
	sam->fd = -1;
	sam->fifo = -1;
	/*
	memcpy( sam->stat->sam_path, samp->stat->sam_path, strlen( samp->stat->sam_path) +1);
	memcpy( sam->stat->bak_path, samp->stat->bak_path, strlen( samp-stat->>bak_path) +1);
	*/

	rtn = Sam_OpenIpc( sam, id);
	if( rtn < 0)
	{
		LogCri( "Sam_OpenIpc error. rtn=[%d]", rtn);
		goto error_1;
	}

	return sam;

	error_1:
		free( sam);
	error:
		return NULL;
}

SAM_WRITE *Sam_OpenWrite( SAM *sam)
{
	int				rtn, i, cnt;
	SAM				*sp;
	SAM_WRITE		*swp;
	char			fifo_name[ 512], sam_name[ 512];

	swp = malloc( sizeof( SAM_WRITE));
	if( swp == NULL)
	{
		LogErr( "malloc error. swp size=[%d]", sizeof( SAM_WRITE));
		goto error;
	}
	memset( swp, 0x00, sizeof( SAM_WRITE));
	LogDbg( "sam malloc. swp=[%p]", swp);

	swp->mem  = sam->mem;
	swp->base = sam->base;
	swp->stat = sam->stat;
	cnt = swp->cnt = swp->stat->cnt;
	LogDbg( "cnt=[%d]", cnt);

	for( i = 0; i < cnt; i++)
	{
		LogMsg( "sam process. pos=[%d]", i);
		swp->sam[ i] = malloc( sizeof( SAM));
		if( swp->sam[ i] == NULL)
		{
			LogErr( "malloc error.");
			goto error_2;
		}
		memset( swp->sam[ i], 0x00, sizeof( SAM));
		sp = swp->sam[ i];
		memcpy( sp, sam, sizeof( SAM));
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
		LogDbg( "semaphore open success. sem=[%p]", sp->sem);

		sprintf( fifo_name, "%s/%s.fifo", sp->stat->sam_path, sp->curr->name);
		sp->fifo = open( fifo_name, O_RDWR | O_NONBLOCK);
		if( sp->fifo < 0)
		{
			LogErr( "fifo open error. name=[%s]", fifo_name);
			goto error_2;
		}
		LogDbg( "fifo open success. fifo=[%d]", sp->fifo);

		sprintf( sam_name, "%s/%s.sam", sp->stat->sam_path, sp->curr->name);
		sp->fd = open( sam_name, O_RDWR);
		if( sp->fd < 0)
		{
			LogErr( "sam file open error. name=[%s]", sam_name);
			goto error_2;
		}
		LogDbg( "sam file open success. fd=[%d]", sp->fd);
	}

	return swp;

	error_3:
		close( sam->fifo);
		sam->fifo = -1;
	error_2:
		for( ; i >= 0; i--)
		{
			sp = swp->sam[ i];
			if( sp == NULL) continue;

			if( sp->sem != NULL) 			{ Sem_Close( sp->sem);	sp->sem = NULL; }
			if( sp->fifo >= 0)				{ close( sp->fifo);		sp->fifo = -1; }
			if( sp->fd >= 0)				{ close( sp->fd);		sp->fd = -1; }
			if( sp != NULL) 				{ free( sp);			swp->sam[ i] = NULL; }
		}
	error_1:
		free( swp);
	error:
		return NULL;
}

int Sam_CloseWrite( SAM_WRITE *swp)
{
	int		i, cnt;
	SAM		*sam;

	cnt = swp->stat->cnt;

	for( i = 0; i < cnt; i++)
	{
		sam = swp->sam[ i];
		if( sam == NULL) continue;
		if( sam->sem != NULL)
		{
			Sem_Close( sam->sem);
			sam->sem = NULL;
		}
		if( sam->fifo >= 0)
		{
			close( sam->fifo);
			sam->fifo = -1;
		}
		if( sam->fd >= 0)
		{
			close( sam->fd);
			sam->fd = -1;
		}
		free( sam);
		sam = NULL;
	}
	free( swp);
	swp = NULL;
	/* shared memory는 close 하지않음 - sam_write 오픈할때 인수인 sam에서 close */

	return 1;
}

int Sam_Close( SAM *sam)
{
	int		rtn;

	if( sam->fd >= 0)
	{
		LogDbg( "sam_file close. fd=[%d]", sam->fd);
		close( sam->fd);
		sam->fd = -1;
	}
	if( sam->fifo >= 0)
	{
		LogDbg( "fifo close. fifo=[%d]", sam->fifo);
		close( sam->fifo);
		sam->fifo = -1;
	}
	if( sam->sem != NULL)
	{
		LogDbg( "semaphore close. sem=[%p]", sam->sem);
		Sem_Close( sam->sem);
		sam->sem = NULL;
	}
	if( sam->mem != NULL)
	{
		LogDbg( "deteched shared memory. mem=[%p]", sam->mem);
		Mem_Close( sam->mem);
		sam->mem = NULL;
		sam->curr = NULL;
	}

	free( sam);
	sam = NULL;

	return 1;
}

int Sam_LoadSwap( SAM *sam)
{
	int			rtn;
	int			fd;
	int			sz, cmp_sz;
	struct stat	st;
	char		*ptr;
	SAM_STAT	*sam_st;

#if 0
	rtn = Cfg_Get( cfg, "swap_file_name", swap_file_name, 512);
	if( rtn <= 0)
	{
		LogWar( "config load error. name=[swap_file_name] cfg=[%p]", cfg);
	}
	LogDbg( "swap_file_name=[%s]", swap_file_name);

	rtn = Cfg_Get( cfg, "VERSION", ver_string, 64);
	if( rtn <= 0)
	{
		LogWar( "config load error. name=[VERSION] cfg=[%p]", cfg);
	}
	LogDbg( "VERSION=[%s]", ver_string);
#endif

	sz = ( sizeof( SAM_TBL) * MAX_SAM_TBL) + sizeof( SAM_STAT);
	rtn = stat( sam->stat->swp_name, &st);
	if( rtn < 0)
	{
		LogErr( "stat error. name=[%s]",  sam->stat->swp_name);
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
	LogDbg( "malloc. ptr=[%p] sz=[%d]", ptr, sz);

	fd = open( sam->stat->swp_name, O_RDONLY);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", sam->stat->swp_name);
		goto error_1;
	}
	LogDbg( "open file. name=[%s] fd=[%d]", sam->stat->swp_name, fd);

	rtn = read( fd, ptr, sz);
	if( rtn != sz)
	{
		LogCri( "스왑파일을 읽지 못했습니다. name=[%s] fd=[%d] sz=[%d] rtn=[%d]", sam->stat->swp_name, fd, sz, rtn);
		goto error_2;
	}
	sam_st = (SAM_STAT *)&ptr[ sizeof( SAM_TBL) * MAX_SAM_TBL];

	cmp_sz = Max( strlen( sam->stat->ver_string), strlen( sam_st->ver_string));
	if( memcmp( sam_st->ver_string, sam->stat->ver_string, cmp_sz))
	{
		LogCri( "스왑파일 버전이 다릅니다. file->ver_string=[%s] sam->stat->ver_string=[%s]", 
				sam_st->ver_string, sam->stat->ver_string);
		goto error_2;
	}

	memcpy( sam->base, ptr, sz);
	LogDbg( "스왑파일을 메모리로 로드 하였습니다. sam->base=[%p] sz=[%d]", sam->base, sz);

	close( fd);
	free( ptr);

	return sz;

	error_2:
		close( fd);
	error_1:
		free( ptr);
	error:
		LogCri( "Sam_LoadSwap error.");
		return -1;
}

SAM *Sam_GetSamPtr( SAM_WRITE *swp, char *id)
{
	int		i, cnt;
	int		sz, csz;
	SAM		*sp;

	cnt = swp->cnt;
	sz = strlen( id);

	for( i = 0; i < cnt; i++)
	{
		sp = swp->sam[ i];
		if( sp == NULL) continue;

		LogDbg( "compare arg=[%s] sam=[%s]", id, sp->curr->id);

		csz = strlen( sp->curr->id);
		if( sz != csz) continue;
		if( !memcmp( sp->curr->id, id, sz)) return sp;
	}
	return NULL;
}

int Sam_GetWritePos( SAM *Sam)
{
	return Sam->curr->wpos;
}

int Sam_GetReadPos( SAM *Sam)
{
	return Sam->curr->rpos;
}

int Sam_LoadCfg( SAM *sam, char *cfg_name)
{
	int			rtn;
	CFG			*cfg;
	int			update_flag = 0;
	char		*rec;
	char		ver_string[ 64];
	SAM_TBL		*tp;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open( cfg_name=[%s]) error.", cfg_name);
		goto error;
	}

	sam->key = Cfg_GetInt( cfg, "key");
	LogDel( "sam->key                  = [0x%08x]", sam->key);
	if( sam->mem == NULL) return 1;

	Cfg_Get( cfg, "VERSION", ver_string, sizeof( ver_string));

	if( strcmp( sam->stat->ver_string, ver_string))
	{
		LogWar( "version mismatch... file=[%s] mem=[%s]", ver_string, sam->stat->ver_string);
	}

	Cfg_Get( cfg, "VERSION",  sam->stat->ver_string, sizeof( ver_string));
	Cfg_Get( cfg, "swp_name", sam->stat->swp_name, sizeof( sam->stat->swp_name));
	Cfg_Get( cfg, "sam_path", sam->stat->sam_path, sizeof( sam->stat->sam_path));
	Cfg_Get( cfg, "bak_path", sam->stat->bak_path, sizeof( sam->stat->bak_path   ));
	Cfg_Get( cfg, "cfg_name", sam->stat->cfg_name, sizeof( sam->stat->cfg_name));

	LogDel( "sam->stat->ver_string     = [%s]", sam->stat->ver_string);
	LogDel( "sam->stat->sam_path       = [%s]", sam->stat->sam_path);
	LogDel( "sam->stat->bak_path       = [%s]", sam->stat->bak_path);
	LogDel( "sam->stat->cfg_name       = [%s]", sam->stat->cfg_name);

	rec = Cfg_GetFirstNamePtr( cfg, "sam_table");
	while( rec != NULL)
	{
		tp = Sam_MakeTbl( sam, rec);
		if( tp != NULL)
		{
			rtn = Sam_UpdateTbl( sam, tp);
			if( rtn < 0) goto error;
			else if( rtn > 0)
			{
				Sam_MakeIpc( sam);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "sam_table");
	}

	Cfg_Close( cfg);
	return 1;

	error_1:
		Cfg_Close( cfg);
	error:
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

int Sam_CfgCheck( SAM *sam, char *cfg_name)
{
	int			rtn, load_config = 0;
	int			update_flag = 0;
	char		*rec;
	CFG			*cfg;
	SAM_TBL		*tp;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	load_config = Cfg_GetInt( cfg, "load_config");
	LogDbg( "Check config... load_config=[%d]", load_config);
	if( load_config == 0) return 0;


	rec = Cfg_GetFirstNamePtr( cfg, "sam_table");
	while( rec != NULL)
	{
		tp = Sam_MakeTbl( sam, rec);
		if( tp != NULL)
		{
			rtn = Sam_UpdateTbl( sam, tp);
			if( rtn < 0) goto error;
			else if( rtn > 0)
			{
				Sam_MakeIpc( sam);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "sam_table");
	}

	return 1;

	error:
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

int Sam_CheckCfg( SAM *sam, CFG *cfg)
{
	int			rtn, load_config = 0;
	int			update_flag = 0;
	char		*rec;
	SAM_TBL		*tp;

	load_config = Cfg_GetInt( cfg, "load_config");
	LogDbg( "Check config... load_config=[%d]", load_config);
	if( load_config == 0) return 0;


	rec = Cfg_GetFirstNamePtr( cfg, "sam_table");
	while( rec != NULL)
	{
		tp = Sam_MakeTbl( sam, rec);
		if( tp != NULL)
		{
			rtn = Sam_UpdateTbl( sam, tp);
			if( rtn < 0) goto error;
			else if( rtn > 0)
			{
				Sam_MakeIpc( sam);
			}
		}
		rec = Cfg_GetNextNamePtr( cfg, "sam_table");
	}

	return 1;

	error:
		LogWar( "Load config error. rtn=[%d] id=[%s]", rtn, tp->id);
		return -1;
}

int Sam_Swap( SAM *sam)
{
	int			rtn;
	int			update_flag = 0;
	int			fd;
	size_t		sz;
	char		*rec;

	LogDel( "swap process. name=[%s]", sam->stat->swp_name);

	fd = open( sam->stat->swp_name, O_CREAT | O_RDWR, 0666);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", sam->stat->swp_name);
		return -1;
	}

	sz = ( sizeof( SAM_TBL) * MAX_SAM_TBL) + sizeof( SAM_STAT);

	rtn = write( fd, sam->base, sz);
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

int Sam_SwapFile( SAM *sam, char *file_name)
{
	int			rtn;
	int			update_flag = 0;
	int			fd;
	size_t		sz;
	char		*rec;

	LogDbg( "swap process. name=[%s]", sam->stat->swp_name);

	fd = open( file_name, O_CREAT | O_RDWR, 0666);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", file_name);
		return -1;
	}

	sz = ( sizeof( SAM_TBL) * MAX_SAM_TBL) + sizeof( SAM_STAT);

	rtn = write( fd, sam->base, sz);
	if( rtn != sz)
	{
		LogCri( "Swap file write error. sz=[%d] write_sz=[%d]", sz, rtn);
		goto error;
	}
	LogMsg( "swap to file. fd=[%d] sz=[%d]", fd, sz);

	close( fd);
	return 1;

	error:
		close( fd);
		return -1;
}

int Sam_LoadFile( SAM *sam, char *swap_file_name)
{
	int			rtn;
	int			update_flag = 0;
	int			fd;
	size_t		sz;
	char		*rec;

	LogDbg( "load from file process. name=[%s]", swap_file_name);

	fd = open( swap_file_name, O_RDWR);
	if( fd < 0)
	{
		LogErr( "swap file open error. name=[%s]", swap_file_name);
		return -1;
	}

	sz = ( sizeof( SAM_TBL) * MAX_SAM_TBL) + sizeof( SAM_STAT);

	rtn = read( fd, sam->base, sz);
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

SAM_TBL *Sam_MakeTbl( SAM *sam, char *rec)
{
	int				stat = 0;
	static SAM_TBL	tblbuf;
	SAM_TBL			*tbl = &tblbuf;
	char			*token = ":\n";
	char			buf[ 8192];
	char			*ptr = buf, *end = ptr;
	int				*iptr;

	memcpy( buf, rec, strlen( rec) +1);
	memset( tbl, 0x00, sizeof( SAM_TBL));

	/* strtok를 strpbrk로 바꿈
	ptr = strtok( buf, token);
	*/
	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end)	*end = 0;
		LogDel( "stat=[%d] ptr=[%s]", stat, ptr);
		switch( stat)
		{
			case 0:		/* id */
				Trim( ptr);
				memcpy( tbl->id, ptr, strlen( ptr) +1);
				break;
			case 1:		/* used */
				tbl->used = atoi( ptr);
				break;
			case 2:		/* sem key */
				tbl->key = StoI( ptr);
				break;
			case 3:		/* name */
				Trim( ptr);
				memcpy( tbl->name, ptr, strlen( ptr) +1);
				break;
			case 4:		/* record length */
				tbl->rlen = atoi( ptr);
				break;
			case 5:		/* delimiter */
				iptr = ( int *)tbl->dm;
				*iptr = StoI( ptr);
				break;
			case 6:		/* job_time */
				tbl->job_time = atoi( ptr);
				break;
			case 7:		/* user area */
				LogDel( "user area ptr=[%s]", ptr);
				tbl->user_sz = strlen( ptr);
				memcpy( tbl->user, ptr, tbl->user_sz +1);
				TrimN( tbl->user, tbl->user_sz);
				break;
			default:
				break;
	
		}
		/*
		ptr = strtok( NULL, token);
		*/
		ptr = end +1;
		stat++;
	}

	if( stat < 4)
	{
		LogCri( "Sam_MakeTbl error. stat=[%d] rec=[%s]", stat, rec);
		return NULL;
	}

	/* Sam_PrintTbl( sam, tbl); */
	LogDel( "make tbl. tbl=[%p] id=[%-15s] key=[0x%08x] name=[%15s] rlne=[%d] dm=[0x%08x]", 
			tbl, tbl->id, tbl->key, tbl->name, tbl->rlen, tbl->dm);
	return tbl;
}

int Sam_MakeIpcAll( SAM *sam)
{
	int			rtn;
	int			i;
	SAM_TBL		*stp;

	for( i = 0; i < sam->stat->cnt; i++)
	{
		sam->curr = &sam->base[ i];
		rtn = Sam_MakeIpc( sam);
		if( rtn < 0)
		{
			LogLib( "Sam_MakeIpc( sam) error. id=[%s] rtn=[%d]", sam, sam->curr->id, rtn);
		}
	}
}

int Sam_MakeIpc( SAM *sam)
{
	int			rtn;
	char		f_name[ 512];

	LogMsg( "Sam ipc make................................... ");
	LogMsg( "sam             ptr     = [%p]", sam);
	LogMsg( "id              name    = [%s]", sam->curr->id);


	if( sam->sem != NULL)
	{
		LogMsg( "sam->sem        close   = [%p]", sam->sem);
		Sem_Close( sam->sem);
		sam->sem = NULL;
	}
	if( sam->fifo > 0)
	{
		LogMsg( "sam->fifo       close   = [%d]", sam->fifo);
		close( sam->fifo);
		sam->fifo = -1;
	}
	if( sam->fd > 0)
	{
		LogMsg( "sam->fd         close   = [%d]", sam->fd);
		close( sam->fd);
		sam->fd = -1;
	}

	LogMsg( "sam->curr->key  try     = [0x%08x]", sam->curr->key);
	sam->sem = Sem_Open( sam->curr->key);
	if( sam->sem == NULL)
	{
		sam->sem = Sem_Create( sam->curr->key);
		if( sam->sem == NULL)
		{
			LogCri( "Sem_Create error. key=[0x%08x]", sam->curr->key);
			goto error;
		}
		LogMsg( "sam->sem        create  = [%p]", sam->sem);
	}
	else
		LogMsg( "sam->sem        open    = [%p]", sam->sem);

	sprintf( f_name, "%s/%s.fifo", sam->stat->sam_path, sam->curr->name);
	LogMsg( "fifo            try     = [%s]", f_name);
	sam->fifo = open( f_name, O_RDWR);
	if( sam->fifo < 0)
	{
		sam->fifo = mkfifo( f_name, 0666);
		if( sam->fifo < 0)
		{
			LogErr( "open and mkfifo error. name=[%s]", f_name);
			goto error_1;
		}
		LogMsg( "sam->fifo       create  = [%d]", sam->fifo);
	}
	else
		LogMsg( "sam->fifo       open    = [%d]", sam->fifo);

	sprintf( f_name, "%s/%s.sam", sam->stat->sam_path, sam->curr->name);
	LogMsg( "file            try     = [%s]", f_name);
	sam->fd = open( f_name, O_RDWR);
	if( sam->fd < 0)
	{
		LogMsg( "sam file open error. rtn=[%d] name=[%s] err=[%d:%s]", rtn, f_name, errno, strerror( errno));
		sam->fd = open( f_name, O_CREAT | O_RDWR, 0666);
		if( sam->fd < 0)
		{
			LogErr( "open and create error. name=[%s]", f_name);
			goto error_2;
		}
		LogMsg( "sam->fd         create  = [%d]", sam->fd);
	}
	else
		LogMsg( "sam->fd         open    = [%d]", sam->fd);


	LogMsg( "sam->fd         close   = [%d]", sam->fd);
	close( sam->fd);
	LogMsg( "sam->fifo       close   = [%d]", sam->fifo);
	close( sam->fifo);
	Sem_Close( sam->sem);
	LogMsg( "sam->sem        close   = [%p]", sam->sem);
	LogMsg( "Sam ipc make................................... end");
	return 1;

	error_2:
		close( sam->fifo);
	error_1:
		Sem_Close( sam->sem);
	error:
		LogMsg( "Sam_MakeIpc error.");
		return -1;
}

int Sam_UpdateTbl( SAM *sam, SAM_TBL *tbl)
{
	int			update_flag = 0;
	int			*optr, *nptr;

	sam->curr = Sam_FindId( sam, tbl->id);
	if( sam->curr == NULL)
	{
		LogDbg( "Sam table not found. add table. id=[%s]", tbl->id);

		/* insert sam table */
		sam->curr = &sam->base[ sam->stat->cnt];
		memcpy( sam->curr, tbl, sizeof( SAM_TBL));
		LogMsg( "Add sam table. id=[%s] pos=[%d]", sam->curr->id, sam->stat->cnt);
		sam->curr->pos = sam->stat->cnt;
		sam->stat->cnt++;
		Sam_PrintTbl( sam, sam->curr);
		return 1;
	}

	/* update sam table */
	if( memcmp( sam->curr->id, tbl->id, Max( strlen( sam->curr->id), strlen( tbl->id) +1))) return 0;

	/* semaphore key */
	if( sam->curr->key != tbl->key)
	{
		LogMsg( "Sam table update semaphore key. sam->curr->key old=[0x%08x] new=[0x%08x]", sam->curr->key, tbl->key);
		update_flag = 1;
		sam->curr->key = tbl->key;
	}

	/* ipc name */
	if( memcmp( sam->curr->name, tbl->name, Max( strlen( sam->curr->name), strlen( tbl->name) +1)))
	{
		LogMsg( "Sam table update ipc file name. sam->curr->name old=[%s] new=[%s]", sam->curr->name, tbl->name);
		update_flag = 1;
		memcpy( sam->curr->name, tbl->name, strlen( tbl->name) +1);
	}

	/* used */
	if( sam->curr->used != tbl->used)
	{
		LogMsg( "Sam table update used. sam->curr->used old=[%d] new=[%d]", sam->curr->used, tbl->used);
		update_flag = 1;
		sam->curr->used = tbl->used;
	}

	/* record len */
	if( sam->curr->rlen != tbl->rlen)
	{
		LogMsg( "Sam table update record length key. sam->curr->rlen old=[%d] new=[%d]", sam->curr->rlen, tbl->rlen);
		update_flag = 1;
		sam->curr->rlen = tbl->rlen;
	}

	/* delimiter */
	optr = ( int *)&sam->curr->dm;
	nptr = ( int *)&tbl->dm;
	if( *optr != *nptr)
	{
		LogMsg( "Sam table update delimiter. sam->curr->dm old=[0x%08x] new=[0x%08x]", *optr, *nptr);
		update_flag = 1;
		*optr = *nptr;
	}

	/* job_time */
	if( sam->curr->job_time != tbl->job_time)
	{
		LogMsg( "Sam table update job_time. sam->curr->job_time old=[%d] new=[%d]", sam->curr->job_time, tbl->job_time);
		update_flag = 1;
		sam->curr->job_time = tbl->job_time;
	}

	if( ( sam->curr->user_sz != tbl->user_sz) || memcmp( sam->curr->user, tbl->user, tbl->user_sz))
	{
		LogMsg( "Sam table update user area.");
		LogDump( sam->curr->user, sam->curr->user_sz, "OLD DATA");
		LogDump( tbl->user, tbl->user_sz, "NEW DATA");
		update_flag = 1;
		memset( sam->curr->user, 0x00, SAM_USER_SZ);
		sam->curr->user_sz = tbl->user_sz;
		memcpy( sam->curr->user, tbl->user, tbl->user_sz);
	}

	if( update_flag == 1)	return 1;
	else					return 0;
}

void *Sam_GetUserPtr( SAM *sam)
{
	return ( void *)sam->curr->user;
}

int Sam_GetUserArea( SAM *sam, SAM_TBL *stp, char *rec)
{
	int		i;

	for( i = 0; i < stp->user_sz; i++)
	{
		switch( stp->user[ i])
		{
			case 0:		rec[ i] = ' '; 					break;
			default:	rec[ i] = stp->user[ i];		break;
		}
	}
	rec[ i] = 0;
	return stp->user_sz;
}

SAM_TBL *Sam_FindId( SAM *sam, char *id)
{
	int		sz, c_sz, i;
	int		find_flag = 0;
	SAM_TBL	*curr;

	sz = strlen( id);
	for( i = 0; i < sam->stat->cnt; i++)
	{
		curr = &sam->base[ i];
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

int Sam_WriteId( SAM_WRITE *sw, char *id, char *rec, int sz)
{
	int		i, rtn;
	int		len, cmp_sz;
	SAM		*sam = NULL;

	len = strlen( id);

	for( i = 0; i < sw->cnt; i++)
	{
		cmp_sz = Max( strlen( sw->sam[ i]->curr->id), len);
		if( memcmp( sw->sam[ i]->curr->id, id, cmp_sz)) continue;
		sam = sw->sam[ i];
		break;
	}

	if( sam == NULL)
	{
		LogCri( "Sam_WriteId( SAM_WRITE *sw=[%p], char *id=[%s], char *rec=[%p], int sz=[%d]) error. id not found. id=[%s]", 
				sw, id, rec, sz, id);
		return -1;
	}

	rtn = Sam_Write( sam, rec, sz);
	if( rtn < 0)
	{
		LogCri( "Sam_Write( sam=[%p], rec=[%s], sz=[%d]) error. rtn=[%d]", sam, rec, sz, rtn);
		return rtn;
	}

	return rtn;
}

int Sam_Write( SAM *sam, char *rec, int sz)
{
	int			rtn;
	int			rec_len, space_len;
	int			seq, pos;
	off_t		seek_pos, seek_rtn;
	SAM_HEAD	head, *hp = &head;;
	char		buf[ 65535];
	int			buf_pos;
	ssize_t		wcnt = 0;
	time_t		cur_time;
	struct stat	stat_buf;

	LogDbg( "Sam_Write len=[%d]", sz);
	space_len = sam->curr->rlen - sz;
	if( space_len < 0)
	{
		LogCri( "record length too long. rec_len=[%d] len=[%d]", sam->curr->rlen, sz);
		return -1;
	}

	time( &cur_time);

	rtn = Sem_Lock( sam->sem);
	if( rtn < 0)
	{
		LogErr( "Sem_Lock error. rtn=[%d]", rtn);
		return -1;
	}

	pos = sam->curr->wpos;
	rec_len = Sam_GetRecLen( sam);
	seek_pos = rec_len * pos;

	seek_rtn = lseek( sam->fd, seek_pos, SEEK_SET);
	if( seek_pos != seek_rtn)
	{
		LogErr( "lseek error. seek_pos[%lld]!=seek_rtn[%lld]", seek_pos, seek_rtn);
		goto error;
	}

	/* 순번은 위치 +1로 --- 위치정보는 0부터 시작, 순번은 1부터 시작 */
	ItoA( ( char *)hp->seq,  sam->curr->wpos +1, sizeof( hp->seq));
	ItoA( ( char *)hp->len,  sz, sizeof( hp->len));
	memcpy( hp->time, TtoS( cur_time), sizeof( hp->time));
	hp->spc[0] = ' ';

	buf_pos = 0;
	memcpy( &buf[ buf_pos], hp, sizeof( SAM_HEAD));
	buf_pos += sizeof( SAM_HEAD);
	memcpy( &buf[ buf_pos], rec, sz);
	buf_pos += sz;
	memset( &buf[ buf_pos], 0x20, space_len);
	buf_pos += space_len;
	memcpy( &buf[ buf_pos], sam->curr->dm, strlen( sam->curr->dm));
	buf_pos += strlen( sam->curr->dm);
	buf[ buf_pos] = 0;

	if( buf_pos != rec_len)
	{
		LogCri( "Size mismatch. buf_pos=[%d] rec_len=[%d]", buf_pos, rec_len);
		goto error;
	}

	wcnt = write( sam->fd, buf, buf_pos);
	if( wcnt != rec_len)
	{
		LogCri( "Sam write error. fd=[%d] rec_len[%d] != wcnt[%d]", sam->fd, rec_len, wcnt);
		goto error;
	}

	rtn = fsync( sam->fd);
	if( rtn < 0)
	{
		LogErr( "fsync( sam->fd=[%d]) error. rtn=[%d]", sam->fd);
		goto error;
	}

	fstat( sam->fifo, &stat_buf);
	if( stat_buf.st_size < 8192)
	{
		rtn = write( sam->fifo, " ", 1);
		if( rtn < 1)
		{
			if( rtn == 0)
			{
				LogWar( "Fifo write error. maybe full. fifo=[%d]", sam->fifo);
			}
			else
			{
				switch( errno)
				{
					case 11: /* Resource temporarily unavailable - fifo full */
						LogWar( "Fifo write error. fifo=[%d] err=[%d:%s]", sam->fifo, errno, strerror( errno));
						break;
					default:
						LogErr( "Fifo write error. fifo=[%d]", sam->fifo);
						goto error;
				}
			}
		}
	}

	sam->curr->wpid = getpid();
	sam->curr->wtime = cur_time;

	sam->curr->wpos++;
	rtn = Sem_Unlock( sam->sem);

	//LogDbg( "Sam write. sam->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%32.32s]", sam->curr->wpos, wcnt, rec);
	//LogDddd( buf, sizeof( SAM_HEAD) + sz, "Sam write. sam->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%s]", sam->curr->wpos, wcnt, rec);
	LogMsg("Sam write. sam->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%.200s...]", sam->curr->wpos, wcnt, rec);
	//LogDump( buf, sizeof( SAM_HEAD) + sz, "Sam write. sam->curr->wpos=[%d] wcnt(sz)=[%d] rec=[%s]", sam->curr->wpos, wcnt, rec);
	return 1;

	error:
		LogDump( rec, sz, "Sam file write error. sam=[%p] id=[%s] rec=[%p] sz=[%d]", sam, sam->curr->id, rec, sz);
		Sem_Unlock( sam->sem);
		return -1;
}

/* sam 파일 시작 포지션은 0으로 시작 - 실제 순번은 1부터 시작 */
int Sam_Read( SAM *sam, char *rec, int sz)
{
	int			rtn, rec_len, cp_sz;
	off_t		pos, rtn_pos;
	char		buf[ 65535];
	SAM_HEAD	*hd = ( SAM_HEAD *)&buf;
	int			rec_sz;
	int			seq;

	if( sam->curr->rpos >= sam->curr->wpos)
	{
		return 0;
	}

	rec_len = Sam_GetRecLen( sam);

	pos = Sam_GetPos( sam, sam->curr->rpos);
	if( pos < 0)
	{
		LogCri( "Sam file pos error. rpos=[%d]", sam->curr->rpos +1);
		return -1;
	}
	
	rtn_pos = lseek( sam->fd, pos, SEEK_SET);
	if( rtn_pos != pos)
	{
		LogCri( "lseek error. pos=[%lld] rtn_pos=[%lld]", pos, rtn_pos);
		return -1;
	}

	rtn = read( sam->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		LogErr( "Sam file read error. request=[%d] rtn=[%d]", rec_len, rtn);
		return -1;
	}

	seq = AtoI( hd->seq, sizeof( hd->seq));
	rec_sz = AtoI( hd->len, sizeof( hd->len));
	if( seq != sam->curr->rpos +1)
	{
		LogCri( "Sam file read error. seq=[%d] tbl->seq=[%d]", seq, sam->curr->rpos +1);
		return -1;
	}

	if( sz <= rec_sz)
	{
		LogCri( "Record size is too short. sz=[%d] read_size=[%d]", sz, rec_sz);
		return -1;
	}
	memcpy( rec, &buf[ sizeof( SAM_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	time( &sam->curr->rtime);
	sam->curr->rpos++;

	return rec_sz;
}

/* sam 파일 시작 포지션은 0으로 시작 - 실제 순번은 1부터 시작 */
int Sam_ReadT( SAM *sam, char *rec, int sz, int timeout)
{
	int				rtn, rec_len, cp_sz;
	off_t			spos, rtn_pos;
	char			buf[ 65535];
	SAM_HEAD		*hp = ( SAM_HEAD *)&buf;
	SAM_TBL			*tp;
	int				rec_sz;
	int				seq;
	time_t			cur_time, rec_time;

	tp = sam->curr;
	LogDbg( "------------------------------------------------------------------");
	LogDbg( "check rpos=[%d] >= wpos=[%d]", tp->rpos, tp->wpos);

	if( tp->rpos < tp->wpos) timeout = 0;
	rtn = Sam_WaitFifo( sam, timeout);
	if( rtn < 0)
	{
		LogCri( "Sam_WaitFifo error. sam=[%p] timeout=[%d]", sam, timeout);
		return -1;
	}
	else
	if( rtn == 0 && timeout != 0)
	{
		LogDbg( "Sam_WaitFifo timeout. timeout=[%d]", timeout);
		return 0;
	}

	rec_len = Sam_GetRecLen( sam);
	spos = Sam_GetPos( sam, tp->rpos);
	if( spos < 0)
	{
		LogCri( "Sam file pos error. rpos=[%d]", tp->rpos);
		return -1;
	}
	
	rtn_pos = lseek( sam->fd, spos, SEEK_SET);
	if( rtn_pos != spos)
	{
		LogCri( "lseek error. spos=[%lld] rtn_pos=[%lld]", spos, rtn_pos);
		return -1;
	}

	rtn = read( sam->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		LogErr( "Sam file read error. request=[%d] rtn=[%d]", rec_len, rtn);
		return -errno;
	}

	seq = AtoI( hp->seq, sizeof( hp->seq));
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

	memcpy( rec, &buf[ sizeof( SAM_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	tp->rpid = getpid();
	time( &tp->rtime);

	tp->rpos++;

	LogDump( buf, sizeof( SAM_HEAD) + rec_sz, "Sam_Read success. tp->rpos=[%d] rec_sz(sz)=[%d] rec=[%s]", tp->rpos, rec_sz, rec);

	return rec_sz;
}

int Sam_ReadPT( SAM *sam, int pos, char *rec, int sz, int timeout)
{
	int				rtn, rec_len, cp_sz;
	off_t			spos, rtn_pos;
	char			buf[ 65535];
	SAM_TBL			*tp;
	SAM_HEAD		*hd = ( SAM_HEAD *)&buf;
	int				seq, rec_sz;
	time_t			cur_time, rec_time;

	LogDbg( "Sam_ReadPT( SAM *sam=[%p], int pos=[%d], char *rec=[%p], int sz=[%d], int timeout=[%d.%d]) start.", 
			sam, pos, rec, sz, timeout/1000000, timeout%1000000);
	LogDbg( "------------------------------------------------------------------");

	tp = sam->curr;
	rec_len = Sam_GetRecLen( sam);
	
	if( pos < tp->wpos) timeout = 0;
	rtn = Sam_WaitFifo( sam, timeout);
	if( rtn < 0)
	{
		LogCri( "Sam_WaitFifo error. sam=[%p] timeout=[%d] rtn=[%d]", sam, timeout, rtn);
		return -1;
	}
	else
	if( rtn == 0 && timeout != 0)
	{
		LogDbg( "Sam_WaitFifo timeout. timeout=[%d.%06d]", timeout / 1000000, timeout % 1000000);
		return 0;
	}

	spos = Sam_GetPos( sam, pos);
	if( spos < 0)
	{
		LogMsg( "no data. rpos=[%d]", tp->rpos);
		return 0;
	}

	rtn_pos = lseek( sam->fd, spos, SEEK_SET);
	if( rtn_pos != spos)
	{
		LogCri( "lseek error. spos=[%lld] rtn_pos=[%lld]", spos, rtn_pos);
		return 0;
	}

	rtn = read( sam->fd, buf, rec_len);
	if( rtn < rec_len)
	{
		if( rtn == 0) 
		{
			LogMsg( "read( sam->fd=[%d], buf=[%p], rec_len=[%d]) error. Maybi end of file. rtn=[%d]", sam->fd, buf, rec_len, rtn);
			return 0;
		}
		LogErr( "read( sam->fd=[%d], buf=[%p], rec_len=[%d]) error. rtn=[%d]", sam->fd, buf, rec_len, rtn);
		return -1;
	}
	LogDbg( "HEAD seq         = [%*.*s]", sizeof( hd->seq),  sizeof( hd->seq),  hd->seq);
	LogDbg( "HEAD len         = [%*.*s]", sizeof( hd->len),  sizeof( hd->len),  hd->len);
	LogDbg( "HEAD time        = [%*.*s]", sizeof( hd->time), sizeof( hd->time), hd->time);

	seq = AtoI( hd->seq, sizeof( hd->seq));
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

	memcpy( rec, &buf[ sizeof( SAM_HEAD)], rec_sz);
	rec[ rec_sz] = 0;

	tp->rpid = getpid();
	time( &tp->rtime);

	/* write pos와 맞추기 위하여 다음 읽을 위치로 pos를 증가시킨다. */
	tp->rpos = pos;
	tp->rpos++;

	LogApp( buf, sizeof( SAM_HEAD) + rec_sz, "Sam_Read success. tp->rpos=[%d] rec_sz(sz)=[%d] rec=[%s]", tp->rpos, rec_sz, rec);

	return rec_sz;
}

int Sam_WaitFifo( SAM *sam, int timeout)
{
	int				rtn;
	fd_set			rfds;
	char			buf[ 8192];
	struct timeval	tv;

	tv.tv_sec = timeout / 1000000;
	tv.tv_usec = timeout % 1000000;

	FD_ZERO( &rfds);
	FD_SET( sam->fifo, &rfds);

	LogDbg( "select wait... sam->fifo=[%d] timeout=[%d:%d]", sam->fifo, tv.tv_sec, tv.tv_usec);
	rtn = select( sam->fifo +1, &rfds, NULL, NULL, &tv);
	if( rtn < 0)
	{
		LogErr( "select error.");
		return -1;
	}
	else
	if( rtn == 0)
	{
		LogDbg( "select timeout... sam->fifo=[%d] timeout=[%d:%d]", sam->fifo, tv.tv_sec, tv.tv_usec);
		return 0;
	}
	LogDbg( "select... rtn=[%d]", rtn);


	if( FD_ISSET( sam->fifo, &rfds))
	{
		rtn = read( sam->fifo, buf, 8192);
		if( rtn <= 0)
		{
			LogCri( "Fifo read error. fifo=[%d] rtn=[%d]", sam->fifo, rtn);
			return -1;
		}
		LogDbg( "Fifo read %d byte(s).", rtn);
		return 1;
	}

	return 0;
}

int Sam_GetRecLen( SAM *sam)
{
	return sizeof( SAM_HEAD) + sam->curr->rlen + strlen( sam->curr->dm);
}

off_t Sam_GetPos( SAM *sam, int pos)
{
	off_t	offset;

	offset = (off_t)Sam_GetRecLen( sam) * ( off_t)( pos);

	LogDbg( "sam offset=[%d] pos=[%d]", offset, pos);

	return offset;
}

int Sam_JobProcess( SAM *sam, int opt)
{
	int			i;
	int			job_time;
	int			job_day;
	int			job_flag;
	time_t		cur_time;
	time_t		bat_time;
	time_t		day_time;
	int			int_time, curint_time;
	struct tm	*tp, job_tb, cur_tb;
	SAM_TBL		*sp;

	static int	tbl_print = 1;

	if( sam == NULL) return 0;

	time( &cur_time);
	tp = localtime_r( &cur_time, &cur_tb);
	int_time = cur_tb.tm_hour * 10000 + cur_tb.tm_min * 100 + cur_tb.tm_sec;

	for( i = 0; i < sam->stat->cnt; i++)
	{
		sp = &sam->base[ i];

		tp = localtime_r( &sp->daily, &job_tb);


		job_flag = sp->job_time / 100000000;
		job_day = ( sp->job_time % 100000000) / 1000000;
		job_time = sp->job_time % 1000000;

		LogMsg( "---[%s]----------------------------------", sp->id);
		if( tbl_print)
		{
			LogMsg( "sp->job_time           = [%09d]", sp->job_time);
			LogMsg( "job_flag               = [%01d]", job_flag);
			LogMsg( "job_day                = [%02d]", job_day);
			LogMsg( "job_time               = [%06d]", job_time);
		}

		switch( job_flag)
		{
			case 0:	/* daily job */
				LogTst( "Daily   check  [ job ]    [ cur ]");

				LogTst( "      day       %6d  != %6d", job_tb.tm_mday, cur_tb.tm_mday);
				if( job_tb.tm_mday == cur_tb.tm_mday) continue;

				LogTst( "      time      %6d  <= %6d", job_time, int_time);
				if( job_time > int_time) continue;

				LogTst( "      job       %6d  != %6d", job_tb.tm_mday, cur_tb.tm_mday);
				if( job_tb.tm_mday == cur_tb.tm_mday) continue;

				break;
			case 1:	/* weekly job */
				LogTst( "Weekly  check  [ job ]    [ cur ]");

				/*
				LogTst( "      day       %6d  != %6d", job_tb.tm_mday, cur_tb.tm_mday);
				if( job_tb.tm_mday == cur_tb.tm_mday) continue;
				*/

				LogTst( "     wday       %6d  == %6d", job_day, cur_tb.tm_wday);
				if( job_day != cur_tb.tm_wday) continue;

				LogTst( "      time      %6d  <= %6d", job_time, int_time);
				if( job_time > int_time) continue;

				/* weekly의 경우 최초 0으로 시작, 0(일요일)에 clear 하는경우 영원히 안돌아서 고침 
				LogTst( "      job       %6d  != %6d", job_tb.tm_wday, cur_tb.tm_wday);
				if( job_tb.tm_wday == cur_tb.tm_wday) continue;
				*/
				/* 하루가 지나가면 다시 job이 돌아야 한다 */
				LogTst( "      check     %6d  >= %6d", cur_time - sp->daily, 86400);
				if( ( cur_time - sp->daily) < 86400) continue;

				break;
			case 2:	/* monthly job */
				LogTst( "Monthly check  [ job ]    [ cur ]");

				LogTst( "      mon       %6d  != %6d", job_tb.tm_mon, cur_tb.tm_mon);
				if( job_tb.tm_mon == cur_tb.tm_mon) continue;

				LogTst( "      day       %6d  == %6d", job_day, cur_tb.tm_mday);
				if( job_day != cur_tb.tm_mday) continue;

				LogTst( "      time      %6d  <= %6d", job_time, int_time);
				if( job_time > int_time) continue;

				LogTst( "      job       %6d  != %6d", job_tb.tm_mon, cur_tb.tm_mon);
				if( job_tb.tm_mon == cur_tb.tm_mon) continue;

				break;
		}
		LogMsg( "JobProcess id=[%s]", sp->id);
		LogMsg( "---[%s]----------------------------------", sp->id);
		LogMsg( "sp->job_time           = [%09d]", sp->job_time);
		LogMsg( "job_flag               = [%01d]", job_flag);
		LogMsg( "job_day                = [%02d]", job_day);
		LogMsg( "job_time               = [%06d]", job_time);

		Sam_Clear( sam, sp);
		LogMsg( "clear   wpos  = [%d]", sp->wpos);
		LogMsg( "clear   rpos  = [%d]", sp->wpos);
		LogMsg( "clear   wtime = [%d]", sp->wtime);
		LogMsg( "clear   rtime = [%d]", sp->rtime);
		sp->wpos = 0;
		sp->rpos = 0;
		sp->wtime = 0;
		sp->rtime = 0;
		time( &sp->daily);
	}

	tbl_print = 0;

	return 0;
}

/* opt == 1 이면 이미 배치가 수행 되었어도 강제로 수행 */
int Sam_DailyProcess( SAM *sam, int opt)
{
	int			i, cnt;
	time_t		cur_time;
	int			int_day, job_day;
	SAM_TBL		*tp;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = sam->stat->cnt;
	LogDbg( "here... 1-1");
	for( i = 0; i < cnt; i++)
	{
		tp = &sam->base[ i];
		job_day = TtoD( tp->daily);
		if( job_day == int_day)
		{
			LogWar( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			if( opt == 0) return 0;
		}
		LogMsg( "Daily clear sequence. id =[%s]", tp->id);
		tp->wpos = 0;
		tp->rpos = 0;
		sam->curr = tp;
		Sam_Clear( sam, NULL);
		time( &tp->daily);
	}
}

/* opt == 1 이면 이미 배치가 수행 되었어도 강제로 수행 */
int Sam_WeeklyProcess( SAM *sam, int opt)
{
	int			i, cnt;
	SAM_TBL		*tp;
	int			int_day, job_day;
	time_t		cur_time;

	time( &cur_time);
	int_day = TtoD( cur_time);

	cnt = sam->stat->cnt;
	for( i = 0; i < cnt; i++)
	{
		tp = &sam->base[ i];
		job_day = TtoD( tp->daily);
		if( job_day == int_day)
		{
			LogWar( "이미 daily(weekly) 배치가 실행 되었습니다. cur_day=[%d] bat_day=[%d]", int_day, job_day);
			if( opt == 0) return 0;
		}
		LogMsg( "Weekly clear sequence. id =[%s]", tp->id);
		tp->wpos = 0;
		tp->rpos = 0;
		sam->curr = tp;
		Sam_Clear( sam, NULL);
		time( &tp->daily);
	}
}

int Sam_Print( SAM *sam)
{
	printf( "sam             = [%p]\n", sam);
	printf( "mem             = [%p]\n", sam->mem);
	printf( "fifo            = [%d]\n", sam->fifo);
	printf( "fd              = [%d]\n", sam->fd);
	printf( "-----[ SAM STAT ]----------------------------------------------------\n");
	printf( "key             = [0x%08x]\n", sam->stat->key);
	printf( "ver_string      = [%s]\n", sam->stat->ver_string);
	printf( "create          = [%s]\n", TtoS( sam->stat->create));
	printf( "swp_name        = [%s]\n", sam->stat->swp_name);
	printf( "sam_path        = [%s]\n", sam->stat->sam_path);
	printf( "bak_path        = [%s]\n", sam->stat->bak_path);
	printf( "cfg_name        = [%s]\n", sam->stat->cfg_name);
	printf( "----------------------------------------------------[ SAM STAT ]-----\n");

	return 1;
}

int Sam_PrintList( SAM *sam)
{
	int			i;
	int			col = 130;
	SAM_TBL		*stp;
	char		*ptr;
	time_t		cur_time;
	char		rec[ SAM_USER_SZ];

	time( &cur_time);
	cur_time -= 86400;

	for( i = 0; i < col; i++) printf( "-"); printf( "\n");
	printf( "pos id               wpos   rpos  wtime    rtime    sem_key    name            job_time   user_field\n");
	for( i = 0; i < col; i++) printf( "-"); printf( "\n");

	for( i = 0; i < sam->stat->cnt; i++)
	{
		stp = &sam->base[ i];
		printf( "%3d ", i);
		printf( "%-15s ", stp->id);
		printf( "%6d ", stp->wpos);
		printf( "%6d ", stp->rpos);

		ptr = TtoS( stp->wtime);
		if( stp->wtime - cur_time > 0)	printf( "%s ", &ptr[ 11]);
		else							printf( "%.8s ", &ptr[ 2]);
		ptr = TtoS( stp->rtime);
		if( stp->rtime - cur_time > 0)	printf( "%s ", &ptr[ 11]);
		else							printf( "%.8s ", &ptr[ 2]);
		printf( "0x%08x ", stp->key);
		printf( "%-15s ", stp->name);
		printf( "%09d ", stp->job_time);
		Sam_GetUserArea( sam, stp, rec);
		printf( "%s ", rec);
		printf("\n");
	}
	for( i = 0; i < col; i++) printf( "-"); printf( "\n");
}

int Sam_PrintTbl( SAM *sam, SAM_TBL *tp)
{
	printf( "[ %s ]\n", tp->id);
	printf( "po ( int       pos            ) record pos                             = [%d]\n",     tp->pos            );
	printf( "id ( char      id[ 32]        ) sam file id                            = [%s]\n",     tp->id             );
	printf( "up ( time_t    uptime         ) last update time                       = [%s]\n",     TtoS( tp->uptime)  );
	printf( "da ( time_t    uptime         ) last daily job day                     = [%s]\n",     TtoS( tp->daily)   );
	printf( "k  ( key_t     key            ) sem 접근 키                            = [0x%08x]\n", tp->key            );
	printf( "wp ( int       wpos           ) write pos                              = [%d]\n",     tp->wpos           );
	printf( "rp ( int       rpos           ) read pos                               = [%d]\n",     tp->rpos           );
	printf( "   ( pid_t     wpid           ) last write process id                  = [%d]\n",     tp->wpid           );
	printf( "   ( pid_t     rpid           ) last read process id                   = [%d]\n",     tp->rpid           );
	printf( "   ( time_t    wtime          ) last write time                        = [%s]\n",     TtoS( tp->wtime)   );
	printf( "   ( time_t    rtime          ) last read time                         = [%s]\n",     TtoS( tp->rtime)   );
	printf( "na ( char      name[ 64]      ) sam file name ... *.fifo *.sam         = [%s]\n",     tp->name           );
	printf( "rl ( int       rlen           ) sam file record length                 = [%d]\n",     tp->rlen           );
	printf( "dm ( char      dm[ 4]         ) delimiter - \\r\\n or \\n or \\0           = [0x%08x]\n", *( int *)&tp->dm[0] );
	printf( "   ( char      filler[ 64]    ) filler                                 = [%s]\n",     tp->filler         );
	return 1;
}

int Sam_InfoField( SAM *sam, SAM_TBL *tp, char *name)
{
	char		*ptr, cmd[ 512];

	if( !memcmp( name, "po", 2))		
		printf( "po ( int       pos            ) record pos                             = [%d]\n",     tp->pos            );
	else if( !memcmp( name, "id", 2))	
		printf( "id ( char      id[ 32]        ) sam file id                            = [%s]\n",     tp->id             );
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
		printf( "na ( char      name[ 64]      ) sam file name ... *.fifo *.sam         = [%s]\n", tp->name               );
	else if( !memcmp( name, "rl", 2))	
		printf( "rl ( int       rlen           ) sam file record length                 = [%d]\n", tp->rlen               );
	else if( !memcmp( name, "dm", 2))	
		printf( "dm ( char      dm[ 4]         ) delimiter - \\r\\n or \\n or \\0           = [0x%08x]\n", *( int *)&tp->dm[0] );
	else	return 0;
}

int Sam_EditField( SAM *sam, SAM_TBL *tp, char *name, char *val)
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
	else if( !memcmp( name, "dm", 2))	*( int *)&tp->dm[0] = StoI( val);
	else	return 0;
}

int Sam_PrintTblFp( SAM *sam, SAM_TBL *tp, FILE *fp)
{
	fprintf( fp, "< %s >\n", tp->id);
	fprintf( fp, "  pos      int       ( record pos                       ) = [%d]\n",     tp->pos            );
	fprintf( fp, "* id       char[ 32] ( sam file id                      ) = [%s]\n",     tp->id             );
	fprintf( fp, "* uptime   time_t    ( last update time                 ) = [%s]\n",     TtoS( tp->uptime)  );
	fprintf( fp, "* daily    time_t    ( last daily job day               ) = [%s]\n",     TtoS( tp->daily)   );
	fprintf( fp, "* job_time int       ( JDDHHMMSS J=type DD=day(wday)    ) = [%d]\n",     tp->job_time       );
	fprintf( fp, "* key      key_t     ( sem 접근 키                      ) = [0x%08x]\n", tp->key            );
	fprintf( fp, "* wpos     int       ( write pos                        ) = [%d]\n",     tp->wpos           );
	fprintf( fp, "* rpos     int       ( read pos                         ) = [%d]\n",     tp->rpos           );
	fprintf( fp, "  wpid     pid_t     ( last write process id            ) = [%d]\n",     tp->wpid           );
	fprintf( fp, "  rpid     pid_t     ( last read process id             ) = [%d]\n",     tp->rpid           );
	fprintf( fp, "  wtime    time_t    ( last write time                  ) = [%s]\n",     TtoS( tp->wtime)   );
	fprintf( fp, "  rtime    time_t    ( last read time                   ) = [%s]\n",     TtoS( tp->rtime)   );
	fprintf( fp, "* name     char[ 64] ( sam file name ... *.fifo *.sam   ) = [%s]\n",     tp->name           );
	fprintf( fp, "* rlen     int       ( sam file record length           ) = [%d]\n",     tp->rlen           );
	fprintf( fp, "* dm       char[ 4]  ( delimiter - \\r\\n or \\n or \\0     ) = [0x%08x]\n", *( int *)&tp->dm[0] );
	fprintf( fp, "* user     char[ 512]( user area                        ) = [%s]\n",     tp->user           );
	fprintf( fp, "* user_sz  int       ( config load user field size      ) = [%d]\n",     tp->user_sz        );
	fprintf( fp, "  filler   char[ 64] ( filler                           ) = [%s]\n",     tp->filler         );
	if( fp == stdout) return 1;

	fprintf( fp, "* - 수정 가능한 필드\n");
	fprintf( fp, "    주의: 수정후 원복 된다면 config file이 로드 되는지 확인 바랍니다.\n");
	return 1;

}

int Sam_PrintTblFile( SAM *sam, SAM_TBL *tp, char *fname)
{
	FILE	*fp;

	fp = fopen( fname, "w+");
	if( fp == NULL)
	{
		LogErr( "fopen( fname=[%s], \"w+\") error.", fname);
		return 0;
	}
	Sam_PrintTblFp( sam, tp, fp);

	fclose( fp);

	return 1;
}

int Sam_EditRecFile( SAM *sam, char *f_name)
{
	int		rtn;
	pid_t	pid;
	int		status;

	pid = fork();
	if( pid < 0)
	{
		LogErr( "fork error. pid=[%d]", pid);
		return -1;
	}
	else
	if( pid == 0)
	{
		rtn = execl( "/usr/bin/vi", "vi", f_name, NULL, NULL);
		if( rtn < 0)
		{
			LogErr( "execl( \"/usr/bin/vi\", \"vi\", f_name=[%s], NULL, NULL) error.", f_name);
		}
		exit( 1);
	}

	rtn = waitpid( pid, &status, 0);
	if( rtn < 0)
	{
		LogErr( "waitpid( pid=[%d], &status, 0) error. rtn=[%d]", pid, rtn);
		return -1;
	}

	return 1;
}

int Sam_LoadRecFile( SAM *sam, SAM_TBL *sam_tbl, char *f_name)
{
	char	*ptr;
	FILE	*fp;
	char	rec[ 512];
	char	*token = "[]\n";

	fp = fopen( f_name, "r");
	if( fp == NULL)
	{
		LogErr( "fopen( f_name=[%s], \"w+\") error.", f_name);
		return 0;
	}
	
	while( 1)
	{
		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) break;
		ptr = strtok( rec, token);
		while( ptr != NULL) ptr = strtok( NULL, token);
		Sam_GetEditLine( sam, sam_tbl, rec);
	}

	Sam_PrintTblFp( sam, sam_tbl, fp);

	fclose( fp);
}

int Sam_GetEditLine( SAM *sam, SAM_TBL *sp, char *rec)
{
	int		i;
	char	*ptr = &rec[ 61];
	char	*nm = &rec[ 2];

	if( !memcmp( nm, "id", 2)) 				{ memcpy( sp->id, ptr, strlen( ptr) +1); }
	else if( !memcmp( nm, "uptime", 6))		{ sp->uptime = StoT( ptr); }
	else if( !memcmp( nm, "daily", 5))		{ sp->daily = StoT( ptr); }
	else if( !memcmp( nm, "job_time", 8))	{ sp->job_time = atoi( ptr); }
	else if( !memcmp( nm, "key", 3))		{ sp->key = StoI( ptr); }
	else if( !memcmp( nm, "wpos", 4))		{ sp->wpos = atoi( ptr); }
	else if( !memcmp( nm, "rpos", 4))		{ sp->rpos = atoi( ptr); }
	else if( !memcmp( nm, "name", 4))		{ memcpy( sp->name, ptr, strlen( ptr) +1); }
	else if( !memcmp( nm, "rlen", 4))		{ sp->rlen = atoi( ptr); }
	else if( !memcmp( nm, "dm", 2))			{ i = StoI( ptr); memcpy( sp->dm, &i, sizeof( int)); }
	else if( !memcmp( nm, "user_sz", 7))	{ sp->user_sz = atoi( ptr); }
	else if( !memcmp( nm, "user", 4))		{ memcpy( sp->user, ptr, strlen( ptr) +1); }

	return 1;
}








