/** ***************************************************************************
**  @file       slp.h
**  @date       2024/02/02
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  double linked list position manager library
**  double linked list를 포지션으로 관리 - 공유 메모리용
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "log.h"
#include "cfg.h"
#include "slp.h"

extern int	Continue;

/** ***************************************************************************
**  @fn         SLP *Slp_GetSlp()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 구조체 malloc 및 초기화
***************************************************************************** */
SLP	*Slp_GetSlp()
{
	SLP	*slp;

	slp = ( SLP *)malloc( sizeof( SLP));
	if( slp == NULL)
	{
		LogErr( "malloc error. sz=[%d]", sizeof( SLP));
		return NULL;
	}
	memset( slp, 0, sizeof( SLP));

	return slp;
}

/** ***************************************************************************
**  @fn         SLP *Slp_GetSlp()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 구조체 malloc 및 초기화
***************************************************************************** */
int Slp_SetPtr( SLP *slp)
{
	SLP_PARAM	*param;

	slp->base   = Mem_GetPtr( slp->mem);
	param       = ( SLP_PARAM *)  slp->base;
	slp->status = ( SLP_STATUS *) slp->base;
	slp->index  = ( SLP_INDEX *)  ( ( size_t)slp->status + ( size_t)param->stat_size);
	slp->rec    = ( SLP_RECORD *) ( ( size_t)slp->index + ( size_t)( param->max_idx * param->idx_size));

	LogDbg( "param->key       = [0x%08x]", param->key);
	LogDbg( "param->max_rec   = [%d]", param->max_rec);
	LogDbg( "param->rec_size  = [%d]", param->rec_size);
	LogDbg( "param->max_idx   = [%d]", param->max_idx);
	LogDbg( "param->idx_size  = [%d]", param->idx_size);
	LogDbg( "param->stat_size = [%d]", param->stat_size);
	LogDbg( "param->size      = [%d]", param->size);

	LogDbg( "index position   = [%ld]", ( size_t)slp->index - ( size_t)slp->base);
	LogDbg( "record position  = [%ld]", ( size_t)slp->rec - ( size_t)slp->base);
	return 1;
}

/** ***************************************************************************
**  @fn         SLP_PARAM *SLP *Slp_GetConfig( char *cfg_name)
**  @param      char *cfg_name - 파일이름
**  @return     +    성공
**  @retval     -    실패
**  @exception
**  @remark
**  @brief
**  config file의 변수를 slp 구조체로 load
***************************************************************************** */
SLP_PARAM *Slp_GetConfig( char *cfg_name)
{
	CFG					*cfg;
	static SLP_PARAM	_slp_param, *slp_param = &_slp_param;

	memset( slp_param, 0, sizeof( SLP_PARAM));
	sprintf( slp_param->cfg_name, "%s", cfg_name);

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s] cfg=[%p]", cfg_name, cfg);
		goto error;
	}

	/* shared memory initial parameter */
	slp_param->key       = Cfg_GetInt( cfg, "key");
	slp_param->max_rec   = Cfg_GetInt( cfg, "max_rec");
	slp_param->rec_size  = Cfg_GetInt( cfg, "rec_size");
	slp_param->max_idx   = Cfg_GetInt( cfg, "max_idx");
	slp_param->idx_size  = Cfg_GetInt( cfg, "idx_size");
	slp_param->stat_size = Cfg_GetInt( cfg, "stat_size");

	/* event pipe path */
	Cfg_Get( cfg, "mat_path", slp_param->mat_path, 512);

	LogDel( "slp_param->key       = [0x%08x]", slp_param->key);
	LogDel( "slp_param->max_rec   = [%d]", slp_param->max_rec);
	LogDel( "slp_param->rec_size  = [%d]", slp_param->rec_size);
	LogDel( "slp_param->max_idx   = [%d]", slp_param->max_idx);
	LogDel( "slp_param->idx_size  = [%d]", slp_param->idx_size);
	LogDel( "slp_param->stat_size = [%d]", slp_param->stat_size);
	LogDel( "slp_param->mat_path  = [%s]", slp_param->mat_path);

	if( slp_param->max_rec <= 0) goto error_1;

	Cfg_Close( cfg);

	return slp_param;

	error_1:
		LogMsg( "close cfg. cfg_name=[%s] cfg=[%p]", cfg_name, cfg);
		Cfg_Close( cfg);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 사용을 위한 초기 작업
***************************************************************************** */
SLP	*Slp_Create( char *cfg_name)
{
	int			rtn;
	SLP			*slp;
	SLP_PARAM	*slp_param;

	slp = Slp_GetSlp();
	if( slp == NULL)
	{
		LogCri( "Slp_GetSlp error. ptr=[%p]", slp);
		goto error;
	}

	slp_param = Slp_GetConfig( cfg_name);
	if( slp_param == NULL)
	{
		LogCri( "Slp_GetConfig error. name=[%s] rtn=[%p]", cfg_name, slp_param);
		goto error_1;
	}

	slp_param->size  = slp_param->max_rec * slp_param->rec_size;	/* size of data */
	slp_param->size += slp_param->max_idx * slp_param->idx_size;	/* size of index */
	slp_param->size += slp_param->stat_size;						/* size of status */
	LogDbg( "slp_param->size      = [%d]", slp_param->size);

	/* shared memory create */
	slp->mem = Mem_Create( slp_param->key, slp_param->size);
	if( slp->mem == NULL)
	{
		LogCri( "Mem_Create error. key=[0x%08x] size=[%d]", slp_param->key, slp_param->size);
		goto error_1;
	}
	LogMsg( "shared memory create. key=[0x%08x] size=[%d] id=[%d]", slp_param->key, slp_param->size, slp->mem->id);
	slp->base   = Mem_GetPtr( slp->mem);

	memcpy( slp->base, slp_param, sizeof( SLP_PARAM));
	Slp_SetPtr( slp);


	/* semaphore create */
	slp->sem = Sem_Create( slp_param->key);
	if( slp->sem == NULL)
	{
		LogCri( "Sem_Create error. key=[0x%08x]", slp_param->key);
		goto error_2;
	}
	LogMsg( "semaphore create. key=[0x%08x] id=[%d]", slp_param->key, slp->sem->id);

	/* fifo create */
	rtn = mkfifo( slp_param->mat_path, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", slp_param->mat_path);
	}

	slp->mat_fd = open( slp_param->mat_path, O_RDWR, 0644);
	if( slp->mat_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", slp_param->mat_path);
		goto error_3;
	}
	LogMsg( "matching pipe create. name=[%s] fd=[%d]", slp_param->mat_path, slp->mat_fd);

	Slp_Init( slp, cfg_name);

	return slp;

	error_3:
		LogMsg( "remove semaphore. key=[0x%08x] sem=[%p]", slp_param->key, slp->sem);
		/* Sem_Remove( slp->sem); */
	error_2:
		LogMsg( "remove shared memory. key=[0x%08x] mem=[%p]", slp_param->key, slp->mem);
		/* Mem_Remove( slp->mem); */
	error_1:
		LogMsg( "free slp pointer. ptr=[%d]", slp);
		free( slp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 사용을 위한 초기 작업
***************************************************************************** */
int Slp_Remove( SLP *slp)
{
	LogMsg( "remove matching pipe. name=[%s] fd=[%d]", slp->status->param.mat_path, slp->mat_fd);
	close( slp->mat_fd);
	unlink( slp->status->param.mat_path);
	LogMsg( "remove shared memory. ptr=[%p] key=[0x%08x] id=[%d]", slp->mem, slp->mem->key, slp->mem->id);
	Sem_Remove( slp->sem);
	LogMsg( "remove semaphore. ptr=[%p] key=[0x%08x] id=[%d]", slp->sem, slp->sem->key, slp->sem->id);
	Mem_Remove( slp->mem);

	LogMsg( "free slp memory. ptr=[%p]", slp);
	free( slp);

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      key_t	key	- ipc 접근 key
**  @param      int		opt	- 0:wait while service=on, 1:nowait open
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 사용을 위한 초기 작업
***************************************************************************** */
SLP	*Slp_Open( key_t key, int opt)
{
	SLP			*slp;

	slp = Slp_GetSlp();
	if( slp == NULL)
	{
		LogCri( "Slp_GetSlp error. ptr=[%p]", slp);
		goto error;
	}

	slp->mem = Mem_Open( key);
	if( slp->mem == NULL)
	{
		LogCri( "Mem_Open error. key=[0x%08x]", key);
		goto error_1;
	}
	LogMsg( "shared memory open. key=[0x%08x] id=[%d]", key, slp->mem->id);

	slp->base   = Mem_GetPtr( slp->mem);
	Slp_SetPtr( slp);

	if( opt == 0)
	{
		while( slp->status->service == 0)
		{
			LogMsg( "서비스가 ON(1) 상태 일때까지 기다립니다. slp->status->service=[%d]", slp->status->service);
			if( Continue == 0)
			{
				goto error_2;
			}
			sleep( 1);
		}
	}

	slp->sem = Sem_Open( key);
	if( slp->sem == NULL)
	{
		LogCri( "Sem_Open error. key=[0x%08x]", key);
		goto error_2;
	}
	LogMsg( "semaphore open. key=[0x%08x] id=[%d]", key, slp->sem->id);
	LogMsg( "slp->sem->lock_cnt=[%d]", slp->sem->lock_cnt);

	slp->mat_fd = open( slp->status->param.mat_path, O_RDWR);
	if( slp->mat_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", slp->status->param.mat_path);
		goto error_3;
	}

	return slp;

	error_3:
		LogMsg( "close semaphore. key=[0x%08x] sem=[%p]", key, slp->sem);
		Sem_Close( slp->sem);
	error_2:
		LogMsg( "close shared memory. key=[0x%08x] mem=[%p]", key, slp->mem);
		Mem_Close( slp->mem);
	error_1:
		LogMsg( "free slp pointer. ptr=[%d]", slp);
		free( slp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 
***************************************************************************** */
int Slp_Close( SLP *slp)
{
	LogMsg( "close shared memory. ptr=[%p] key=[0x%08x] id=[%d]", slp->mem, slp->mem->key, slp->mem->id);
	Sem_Close( slp->sem);
	LogMsg( "close semaphore. ptr=[%p] key=[0x%08x] id=[%d]", slp->sem, slp->sem->key, slp->sem->id);
	Mem_Close( slp->mem);

	LogMsg( "free slp memory. ptr=[%p]", slp);
	free( slp);

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp initial - 공유 메모리 생성후 초기치 저장
***************************************************************************** */
int Slp_Init( SLP *slp, char *cfg_name)
{
	int			i;
	SLP_STATUS	*stat;
	SLP_INDEX	*index;
	SLP_RECORD	*rec;

	stat = slp->status;

	stat->dat_pos = 1;	/* SLP_RECORD의 첫번째 record는 쓰지 않는다 */

	for( i = 0; i < SLP_MAX_IDX( slp); i++)
	{
		index = SLP_GET_IDX( slp, i);
		index->pos = i;
	}

	LogDbg( "sz=[%ld]", ( size_t)slp->rec - ( size_t)slp->base);
	for( i = 0; i < SLP_MAX_REC( slp); i++)
	{
		rec = SLP_GET_REC( slp, i);
		rec->pos = i;
	}

	Slp_LoadCurr( slp, cfg_name);
	Slp_LoadPair( slp, cfg_name);

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 
***************************************************************************** */
int Slp_Insert( SLP *slp)
{
	int			rtn;
	SLP_INDEX	*index;
	SLP_RECORD	*rec;

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 
***************************************************************************** */
int Slp_Lock( SLP *slp)
{
	int		rtn = 1;

	retry:
	rtn = Sem_LockT( slp->sem, 1000000);
	if( rtn < 0) 
	{
		LogCri( "Sem_Lock unable. lock_cnt=[%d]", slp->sem->lock_cnt);
		goto retry;
	}

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 
***************************************************************************** */
int Slp_Unlock( SLP *slp)
{
	int		rtn;

	rtn = Sem_Unlock( slp->sem);
	if( rtn < 0)
	{
		LogCri( "Sem_Unlock unable. lock_cnt=[%d]", slp->sem->lock_cnt);
	}

	return 1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_( int opt)
**  @param      char *f_name - 파일이름 NULL:stdout
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp 
***************************************************************************** */
int Slp_Stat( SLP *slp)
{

	SLP_STATUS_Print( slp->status);

	return 1;
}

