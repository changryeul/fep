/** ***************************************************************************
**  @file       blp.c
**  @date       2025/09/01
**  @author     cdc
**  @version    V0.0.20250901
**  @brif
**  채권 시장조성 라이브러리
**	blp.c			- 
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define		DATA_SIZE	2048
#include "shm_memory.h"
#include "pa_struct.h"
#include "buf_struct.h"

#include "mem.h"
#include "sem.h"
#include "etc.h"
#ifndef _OMS_SOURCE_
#include "log.h"
#else
#include "log_conv.h"
#endif
#include "blp.h"

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
extern int		Continue;
extern SHM_NOTE	*Shm_Note;

/*****************************************************************************/
/* CREATE/REMOVE/OPEN/CLOSE FUNCTIOM                                         */
/*****************************************************************************/
/** ***************************************************************************
**  @fu         int Blp_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권 시장조성에 필요한 ipc를 생성 ##########
**	shared memory, semaphore 생성
***************************************************************************** */
BLP* Blp_CreateForce( key_t blp_key)
{
	BLP		*blp;

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Create( blp->key, sizeof( BLP_MAP));
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Create error.");
	}
	else
	{
		blp->map = Mem_GetPtr( blp->mem);
	}

	blp->sem = Sem_Create( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error;
	}

	Blp_ShmInit( blp);	/* 공유 메모리 초기화 */
	Blp_Init( blp);		/* struct 초기화 */

	return blp;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권시장조성에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
BLP* Blp_Create( key_t blp_key)
{
	BLP		*blp;

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Create( blp->key, sizeof( BLP_MAP));
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Create error.");
		goto error_1;
	}
	blp->map = Mem_GetPtr( blp->mem);
	blp->map->stat.key = blp->key;

	blp->sem = Sem_Create( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error_2;
	}

	Blp_ShmInit( blp);	/* 공유 메모리 초기화 */
	Blp_Init( blp);		/* struct 초기화 */

	return blp;

	error_2:
		if( blp->sem != NULL) Sem_Remove( blp->sem);
		Mem_Remove( blp->mem);
	error_1:
		free( blp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Remove( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Blp_Remove( BLP *blp)
{
	int		rtn;

	rtn = Sem_Remove( blp->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Remove error. sem=[%p] key=[0x%08x]", blp->sem, blp->key);
		return -1;
	}

	rtn = Mem_Remove( blp->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Remove error. mem=[%p] key=[0x%08x]", blp->mem, blp->key);
		return -1;
	}

	free( blp);

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Open()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - blp pointer
**  @retval     실패    - NULL
**  @brief
**  채권시장조성 Open
***************************************************************************** */
BLP* Blp_Open( key_t blp_key, int service)
{
	BLP		*blp;

	LogDbg( "Blp_Open start ... key=[0x%08x] service=[%d]", blp_key, service);

	blp = malloc( sizeof( BLP));
	if( blp == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( blp, 0x00, sizeof( BLP));
	blp->key = blp_key;

	blp->mem = Mem_Open( blp->key);
	if( blp->mem == NULL)
	{
		LogLib( "Mem_Open error.");
		goto error_1;
	}
	blp->map = Mem_GetPtr( blp->mem);
	LogDel( "attach shared memory ... ptr=[%p]", blp->map);

	while( Continue && service == 0)
	{
		if( blp->map->stat.service) break;

		LogWar( "채권 시장조성 서비스 준비중 ... blp->map->stat.service=[%d]", blp->map->stat.service);
	}

	blp->sem = Sem_Open( blp->key);
	if( blp->sem == NULL)
	{
		LogLib( "Sem_Open error.");
		goto error_2;
	}

	Blp_Init( blp);		/* struct 초기화 */

	LogDbg( "Blp_Open success ... blp=[%p]", blp);
	return blp;

	error_2:
		if( blp->sem != NULL) Sem_Close( blp->sem);
		Mem_Close( blp->mem);
	error_1:
		free( blp);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 Close
***************************************************************************** */
int Blp_Close( BLP *blp)
{
	int		rtn;

	rtn = Sem_Close( blp->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Close error. sem=[%p] key=[0x%08x]", blp->sem, blp->key);
		return -1;
	}

	rtn = Mem_Close( blp->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Close error. mem=[%p] key=[0x%08x]", blp->mem, blp->key);
		return -1;
	}

	free( blp);

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  공유메모리 값을 초기화 - create 시만 
***************************************************************************** */
int Blp_ShmInit( BLP *blp)
{
	int				i;
	BLP_MAP			*map = blp->map;
	BLP_TBL	*tbl;

	map->stat.key = blp->key;

	for( i = 1; i < BLP_MAX_TBL; i++)
	{
		tbl = &map->tbl[ i];
		tbl->id = i;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  BLP struct 값 initial - open 시
***************************************************************************** */
int Blp_Init( BLP *blp)
{
	// int				i;
	// BLP_MAP			*map = blp->map;
	// BLP_TBL	*tbl;

	blp->tbl_pos = -1; 	/* Blp_SetArg 에서 set */

	return 1;
}

/*****************************************************************************/
/* Process FUNCTIOM                                                          */
/*****************************************************************************/
/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  전략 실행
***************************************************************************** */
int Blp_Process( BLP *blp, void *data)
{
	int				rtn;
	int				task = 0;	/* 1:시세처리 2:주문응답처리 3:체결처리 */
	time_t			cur_time;
	// char			time_buf[ 32];
	BLP_TBL			*tbl;
	BLP_SISE		*sise;
	char			*ptr;
	int				sz;

	CO_G701K		*g701k = ( CO_G701K *)data;		/* KRX 체결 시세 */
	CO_B601K		*b601k = ( CO_B601K *)data;		/* KRX 호가 시세 */
	KRX_LP_NOTE_SETTLE_RESP_DATA	*ttrmop41301 = ( KRX_LP_NOTE_SETTLE_RESP_DATA *)data;	/* 정상 41302:거부 41303:자동취소 */

	if( blp->tbl_pos < 0)
	{
		LogCri( "전략이 초기화 되지 않았습니다. blp->tbl_pos=[%d]", blp->tbl_pos);
		return -1;
	}

	time( &cur_time);

	tbl  = &blp->map->tbl[ blp->tbl_pos];
	sise = &tbl->sise;

	/***************/
	/* mode check  */
	/***************/
	switch( tbl->mode)
	{
		case 0:		/* 대기 모드 */
			if( tbl->stat != 0) 		/* 모든 주문 취소 */
			{
				/* TODO order cancel process */
				LogDbg( "대기모드 ");
			}
			else
			{
				Blp_WaitProcess( blp);	/* 주문 및 호가 clear */
			}
			return 0;
		case 1:		/* 전략 수행 */
			LogDbg( "전략수행 ");
			break;
		case 2:		/* 취소 모드 */
			if( tbl->stat != 0) 		/* 모든 주문 취소 */
			{
				/* TODO order cancel process */
				Blp_ProcessCancel( blp, tbl);
			}
			break;
		case 3:		/* 햇지 모드 */
			if( tbl->stat != 0) 		/* 햇지 주문 */
			{
				/* TODO order hedge process */
				Blp_ProcessHedge( blp, tbl);
			}
			break;
		default:
			break;
	}

	/**************/
	/* 시간 check */
	/**************/
	rtn = Blp_TimeCheck( blp, tbl);
	if( rtn <= 0)
	{
		return 0;
	}

	tbl->proc_cnt++;
	gettimeofday( &tbl->proc_time, NULL);

	if( data == NULL) return 0;

	/**************/
	/* 시세 처리  */
	/**************/
	/* 아래 Blp_ProcessSise는 전체 종목 처리 - 여기는 한개 종목 처리를 전제로 */
	if( !memcmp( ( char *)data, "G701K", 5))		/* KRX 체결 시세 */
	{
		LogDbg( "KRX 체결 시세 process");
		if( tbl->mode >= 2)	/* 취소/햇지 모드는 시세 처리 않함 */
		{
			LogMsg( "취소/햇지 모드 tbl->mode=[%d]", tbl->mode);
			if( tbl->mode == 2)			rtn = Blp_ProcessCancel( blp, tbl);
			else if( tbl->mode == 3)	rtn = Blp_ProcessHedge( blp, tbl);
			return 0;
		}
		ptr = g701k->item_code;
		sz  = sizeof( g701k->item_code);
		if( memcmp( ptr, tbl->item_code, sz))	goto error_code;
		rtn = Blp_ProcessSiseExe( blp, tbl, sise, g701k);
		if( rtn <= 0)
		{
			LogDbg( "처리되지 않았습니다. rtn=[%d]", rtn);
			return rtn;
		}
		task = 1;
	}
	else
	if( !memcmp( ( char *)data, "B601K", 5))		/* KRX 호가 시세 */
	{
		LogDbg( "KRX 호가 시세 process");
		if( tbl->mode >= 2)	/* 취소/햇지 모드는 시세 처리 안함 */
		{
			LogMsg( "취소/햇지 모드 tbl->mode=[%d]", tbl->mode);
			if( tbl->mode == 2)			rtn = Blp_ProcessCancel( blp, tbl);
			else if( tbl->mode == 3)	rtn = Blp_ProcessHedge( blp, tbl);
			return 0;
		}
		ptr = b601k->item_code;
		sz  = sizeof( b601k->item_code);
		if( memcmp( ptr, tbl->item_code, sz))	goto error_code;

		rtn = Blp_ProcessSiseHoga( blp, tbl, sise, g701k);
		if( rtn <= 0)
		{
			LogDbg( "처리되지 않았습니다. rtn=[%d]", rtn);
			return rtn;
		}
		task = 1;
	}
	else
	/*******************/
	/* 주문응답 처리   */
	/*******************/
	if( !memcmp( ttrmop41301->Transaction_Code, "TTRMOP4130", 10))		/* 주문 정상 응답 */
	{
		task = 2;
	}
	else
	/*******************/
	/* 체결 처리       */
	/*******************/
	if( !memcmp( ttrmop41301->Transaction_Code, "TTRTDP4230", 10))		/* 채결 결과 통보 */
	{
		task = 3;
	}

	switch( task)
	{
		case 1:		/* 시세 처리 */
			Blp_ProcessOrder( blp, tbl);
			rtn = 1;
			break;
		case 2:		/* 주문응답 처리 */
			if( tbl->mode == 1) 		rtn = Blp_ProcLpOrdChk( blp, tbl, ttrmop41301);
			else if( tbl->mode == 2)	rtn = Blp_ProcLpOrdChk( blp, tbl, ttrmop41301);
			else if( tbl->mode == 3)	rtn = Blp_ProcOrdChk( blp, tbl, ( void *)ttrmop41301);
			break;
		case 3:		/* LP 중단 처리 */
			Blp_ProcExeChk( blp, tbl, ( void *)ttrmop41301);
			rtn = 1;
			break;
	}

	if( rtn < 0)
	{
		LogDbg( "전략 수행에 문제가 생겨 중단합니다.");
		tbl->mode = 2;
		tbl->mode_next = 4;
		return -1;
	}
	
	
	return 1;

	error_code:
		LogMsg( "처리 종목이 아닙니다. tbl=[%.12s] arg=[%.12s]", tbl->item_code, ptr);
		return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  전략 종료
***************************************************************************** */
int Blp_StopProcess( BLP *blp, void *data)
{
	static int	cnt = 0;
	BLP_TBL		*tbl  = &blp->map->tbl[ blp->tbl_pos];

	LogDbg( "전략을 종료하고 구동을 종료 합니다.");
	tbl->mode = 2;
	tbl->mode_next = 4;

	while( Continue)
	{
		LogDbg( "전략 종료 대기중 입니다. cnt=[%d/300]", ++cnt);
		if( tbl->mode != 4)
		{
			Blp_Process( blp, NULL);
			sleep( 1);
		}
		else
		{
			break;
		}
		if( cnt >= 300) break;
	}
	tbl->mode = 4;
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  tiemout시 전략 check
***************************************************************************** */
int Blp_WaitProcess( BLP *blp)
{
	static int	cnt = 0;
	BLP_TBL		*tbl  = &blp->map->tbl[ blp->tbl_pos];

	LogDbg( "전략 대기.");

	if( tbl->mode_next == 1)
	{
		LogMsg( "전략 수행 준비 ... ");
		memset( &tbl->order[ 0],		0x00, sizeof( BLP_ORD) * BLP_MAX_LP);
		memset( &tbl->old_order[ 0],	0x00, sizeof( BLP_ORD) * BLP_MAX_LP);
		memset( &tbl->hedge[ 0],		0x00, sizeof( BLP_HEDGE) * BLP_MAX_HEDGE);
		memset( &tbl->sise,				0x00, sizeof( BLP_SISE));
		memset( &tbl->rec[ 0],			0x00, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
		memset( &tbl->old[ 0],			0x00, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);

		tbl->hedge_cnt = 0;
		tbl->mode = tbl->mode_next;
		tbl->mode_next = 0;
	}
	else
	{
		LogMsg( "전략 수행 종료 ... ");
		memset( &tbl->stg_id, 0x00, sizeof( tbl->stg_id));	/* 다음 전략을 위해 전략번호 clear */
		sleep( 1);
#ifndef _OMS_SOURCE_
		return -1;
#else
		Exit_Process();
#endif
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1	 전략
**  @retval     실패    - 0
**  @brief
**  시장조성 시간 check 대기/오전/오후/마감
***************************************************************************** */
int Blp_TimeCheck( BLP *blp, BLP_TBL *tbl)
{
	int 			i;
	int				mk_stat = 0;

	time_t			cur_time;
	BLP_TIME		*mk_time;

	time( &cur_time);
	for( i = 0; i < BLP_MAX_MK_STAT; i++)
	{
		mk_time = &tbl->arg.mk_time[ i];

		if( cur_time >= mk_time->start && cur_time <= mk_time->end) 
		{
			mk_stat = i +1;
			break;
		}
	}

	if( mk_stat != tbl->arg.mk_stat)		/* 장 변경 */
	{
		LogMsg( "장상태 변경 [%d] -> [%d]", tbl->arg.mk_stat, mk_stat);
		if( mk_stat == 0)					/* 대기모드로 - 주문 정리 */
		{
			if( tbl->mode == 1)				/* 전략수행모드 이면 취소모드로 */
			{
				tbl->mode = 2;
			}
		}
	}

	if( mk_stat == 0) 
	{
		tbl->arg.mk_stat = mk_stat;
		LogDbg( "cur_time   =[%s]", TtoS( cur_time));
		LogDbg( "start      =[%s]", TtoS( mk_time->start));
		LogDbg( "end        =[%s]", TtoS( mk_time->end));
		LogMsg( "전략 실행 시간이 아닙니다.");
		return 0;
	}

	tbl->arg.mk_stat = mk_stat;


	return 1;
}

/*****************************************************************************/
/* FUNCTIOM                                                                  */
/*****************************************************************************/
/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 테이블 번호
**  @retval     실패    - -1
**  @brief
**  새로운 전략을 실행하기 위한 clear
***************************************************************************** */
int Blp_ClearTbl( BLP *blp, BLP_TBL *tbl)
{
	int				i;
	// BLP_ORD			*ord;
	BLP_HOGA_REC	*hoga;

	/* 호가 테이블에 있는 LP 주문 번호 지우기 */
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		hoga = &tbl->rec[ i];
		hoga->lp_no = 0;
	}

	/* 주문 테이블 clear */
	memset( tbl->order, 	0x00, sizeof( BLP_ORD) * BLP_MAX_LP);
	memset( tbl->old_order,	0x00, sizeof( BLP_ORD) * BLP_MAX_LP);

	/* 햇지 테이블 clear */
	tbl->hedge_cnt = 0;
	memset( tbl->hedge,		0x00, sizeof( BLP_HEDGE) * BLP_MAX_HEDGE);

	return 1;
}
/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 테이블 번호
**  @retval     실패    - -1
**  @brief
**  종목코드로 BLP_TBL 찾기
***************************************************************************** */
int Blp_GetItem( BLP *blp, char *item_code)
{
	int				i;
	int				cnt;
	BLP_TBL	*tbl = &blp->map->tbl[ 0];

	cnt = tbl->id;	/* 등록된 종목 갯수 */
	LogDbg( "cnt=[%d]", cnt);

	for( i = 1; i <= cnt; i++)
	{
		tbl = &blp->map->tbl[ i];
		LogDbg( "mem[%.12s] arg[%.12s]", tbl->item_code, item_code);
		if( !memcmp( tbl->item_code, item_code, 12)) return i;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 호가 record 번호
**  @retval     실패    - -1
**  @brief
**  가격으로 BLP_HOGA_REC 찾기
***************************************************************************** */
int Blp_GetPrice( BLP *blp, BLP_TBL *tbl, double price)
{
	int				i;
	// int				cnt;
	BLP_HOGA_REC	*rec;

	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		rec = &tbl->rec[ i];
		if( price == rec->price) return i;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  종목코드 등록
***************************************************************************** */
int Blp_AddItem( BLP *blp, char *item_code)
{
	int				pos;
	BLP_TBL	*tbl = &blp->map->tbl[ 0];

	pos = Blp_GetItem( blp, item_code);
	if( pos > 0)
	{
		LogMsg( "이미 등록된 종목 코드 입니다. item_code=[%.12s]", item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);

	Blp_Lock( blp);
	tbl->id++;
	pos = tbl->id;
	if( pos >= BLP_MAX_TBL)
	{
		LogCri( "더이상 등록할 TABLE 영역이 없습니다.");
		goto error;
	}
	Blp_Unlock( blp);

	tbl = &blp->map->tbl[ pos];
	memcpy( tbl->item_code, item_code, sizeof( tbl->item_code));
	tbl->id = pos;
	// tbl->tick = 1.00;

	return pos;

	error:
		Blp_Unlock( blp);
		return -1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  호가 테이블 LP 주문 update
***************************************************************************** */
int Blp_UpdateHoga( BLP *blp, BLP_TBL *tbl, int lp_no)
{
	int				i;
	BLP_HOGA_REC	*rec;
	BLP_ORD			*ord = &tbl->order[ lp_no -1];

	LogDbg( "lp_no=[%d]", lp_no);
	/* update lp order */
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		rec = &tbl->rec[ i];


		if( rec->price == 0.0) continue;

		// LogDbg( "    i=[%2d] price=[%7.2f] lp_no=[%d]", i, rec->price, rec->lp_no);
		if( rec->lp_no == lp_no)		rec->lp_no = 0;
		if( rec->price == ord->ask_prc)	rec->lp_no = lp_no;
		if( rec->price == ord->bid_prc)	rec->lp_no = lp_no;
		// LogDbg( "    i=[%2d] price=[%7.2f] lp_no=[%d]\n", i, rec->price, rec->lp_no);
	}

#if 0
	for( pos = tbl->ask_base; pos >= BLP_SISE_HOGA; pos--)					/* 매도 set */
	{
		rec = &tbl->rec[ pos];
		if( rec->price == ord->ask_prc)
		{
			rec->lp_no = lp_no;
		}
		else
		{
			if( rec->lp_no == lp_no)	rec->lp_no = 0;
		}
	}

	for( pos = tbl->bid_base; pos < BLP_MAX_HOGA - BLP_SISE_HOGA; pos++)		/* 매수 set */
	{
		rec = &tbl->rec[ pos];

		if( rec->price == ord->bid_prc)
		{
			rec->lp_no = lp_no;
		}
		else
		{
			if( rec->lp_no == lp_no)	rec->lp_no = 0;
		}
	}
#endif

	return i;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 호가 테이블 리스트
***************************************************************************** */
int Blp_List( BLP *blp)
{
	int				i, col = 120;
	BLP_MAP			*map = blp->map;
	BLP_TBL	*tbl = &blp->map->tbl[ 0];

	LogRaw( "key           = [0x%08x]\n", map->stat.key);
	LogRaw( "oms_key       = [0x%08x]\n", map->stat.oms_key);
	LogRaw( "count         = [%d]\n", tbl->id);

	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-2.2s ", "id");
	LogRaw( "%-6.6s ", "   cnt");
	LogRaw( "%-6.6s ", "   seq");
	LogRaw( "%-2.2s ", "bi");
	LogRaw( "%-12.12s ", "item_code");
	LogRaw( "%-2.2s ", "ab ");
	LogRaw( "%-2.2s ", "bb ");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 1; i < BLP_MAX_TBL; i++)
	{
		tbl = &map->tbl[ i];
		LogRaw( "%2d ", tbl->id);
		LogRaw( "%6d ", tbl->proc_cnt);
		LogRaw( "%6d ", tbl->seq_no);
		LogRaw( "%2.2s ", tbl->board_id);
		LogRaw( "%12.12s ", tbl->item_code);
		LogRaw( "%2d ", tbl->ask_base);
		LogRaw( "%2d ", tbl->bid_base);
		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");




	return 1;
}

/*****************************************************************************/
/* SUB FUNCTIOM                                                              */
/*****************************************************************************/
/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  semaphore lock
***************************************************************************** */
int Blp_Lock( BLP *blp)
{
	Sem_Lock( blp->sem);
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  semaphore unlock
***************************************************************************** */
int Blp_Unlock( BLP *blp)
{
	Sem_Unlock( blp->sem);

	return 1;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* UTILITY or DEVELOPE  FUNCTIOM                                              */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 공유메모리 상태
***************************************************************************** */
int Blp_Stat( BLP *blp, char *item_code)
{
	int				i, col = 120;
	int				pos;
	BLP_TBL	*tbl;
	BLP_HOGA_REC	*rec;

	if( item_code == NULL)	return -1;

	pos = Blp_GetItem( blp, item_code);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);

	tbl = &blp->map->tbl[ pos];

	BLP_ARG_Print( &tbl->arg);

	LogMsg( "id         = [%d]", tbl->id);
	LogMsg( "proc_cnt   = [%d]", tbl->proc_cnt);
	LogMsg( "seq_no     = [%d]", tbl->seq_no);
	LogMsg( "board_id   = [%.2s]", tbl->board_id);
	LogMsg( "item_code  = [%.12s]", tbl->item_code);
	LogMsg( "ask_base   = [%2d]", tbl->ask_base);
	LogMsg( "bid_base   = [%2d]", tbl->bid_base);

	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");
	LogRaw( "%-2.2s ", "ab");
	LogRaw( "%-2.2s ", "no");
	LogRaw( "%-3.3s ", "seq");
	LogRaw( "%-2.2s ", " s");
	LogRaw( "%-10.10s ", "  price");
	LogRaw( "%-10.10s ", "  volume");
	LogRaw( "                    ");
	LogRaw( "%-2.2s ", "ab");
	LogRaw( "%-2.2s ", "no");
	LogRaw( "%-3.3s ", "seq");
	LogRaw( "%-2.2s ", " s");
	LogRaw( "%-10.10s ", "  price");
	LogRaw( "%-10.10s ", "  volume");
	LogRaw( "\n");
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		rec = &tbl->rec[ i];
		LogRaw( "%2d ", rec->ab);
		LogRaw( "%2d ", rec->no);
		LogRaw( "%3d ", rec->ho);
		if( rec->s_ho == 0)		LogRaw( "   ",  rec->s_ho);
		else					LogRaw( "%2d ", rec->s_ho);
		LogRaw( "%10.2f ", rec->price);
		LogRaw( "%10.0f ", rec->volume);
		LogRaw( "                    ");

		rec = &tbl->old[ i];
		LogRaw( "%2d ", rec->ab);
		LogRaw( "%2d ", rec->no);
		LogRaw( "%3d ", rec->ho);
		if( rec->s_ho == 0)		LogRaw( "   ",  rec->s_ho);
		else					LogRaw( "%2d ", rec->s_ho);
		LogRaw( "%10.2f ", rec->price);
		LogRaw( "%10.0f ", rec->volume);


		LogRaw( "\n");
	}
	for( i = 0; i < col; i++) LogRaw( "-"); LogRaw( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  sise receive from file - for test
***************************************************************************** */
int Blp_SetArg( BLP *blp, char *rec, char *ap_type)
{
	int				pos;
	BLP_TBL			*tbl;
	BLP_ARG			_arg, *arg = &_arg;

	memset( arg, 0x00, sizeof( BLP_ARG));
	Blp_ConvertArg( arg, rec);

	pos = Blp_GetItem( blp, arg->item_cd);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", arg->item_cd);
		pos = Blp_AddItem( blp, arg->item_cd);
		if( pos <= 0)
		{
			LogMsg( "종목을 등록 할 수없습니다. item_code=[%.12s]", arg->item_cd);
			return 0;
		}
		LogMsg( "종목을 등록 하였습니다. item_code=[%.12s]", arg->item_cd);
	}

	LogDbg( "pos=[%d]", pos);
	blp->tbl_pos = pos;
	tbl = &blp->map->tbl[ pos];

	/* 테스트를 위한 강제 세팅 */
	/* 나중에 시간 설정을 넣어야 한다 - 테스트로 걍제 세팅 */
	BLP_ARG_Print( arg);

	memset( tbl, 0x00, sizeof( BLP_TBL));		/* 전략 초기화 */
	tbl->mode = 1;			/* 전략수행 모드 */
	time( &tbl->mode_time);
	gettimeofday( &tbl->proc_time, NULL);
	memcpy( tbl->stg_id, ap_type, sizeof( tbl->stg_id) -1);

	memcpy( &tbl->arg, arg, sizeof( BLP_ARG));
	memcpy( &tbl->item_code, arg->item_cd, 12);
	tbl->item_pos = pos;

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  util - struct timeval to string
***************************************************************************** */
char* Blp_TvToS( struct timeval *tv)
{
	struct tm	*tp;
	static char str[ 64];

	if( tv == NULL)
	{
		sprintf( str, "%s", "0000/00/00-00:00:00.000000");
		return str;
	}

	tp = localtime( &tv->tv_sec);
	if( tp == NULL)
	{
		sprintf( str, "%s", "0000/00/00-00:00:00.000000");
		return str;
	}

	sprintf( str, "%04d/%02d/%02d-%02d:%02d:%02d.%06d",
			tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
			tp->tm_hour, tp->tm_min, tp->tm_sec,
			( int)tv->tv_usec);

	return str;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  for sise receive - test
***************************************************************************** */
int Blp_SiseOpen( BLP *blp)
{
	MCP	*mcp;

	mcp = Mcp_OpenServer( "224.0.0.1", 30022);
	if( mcp == NULL)
	{
		LogErr( "multicast server open error. addr=[%s] port=[%d]", "224.0.0.1", 30022);
		return -1;
	}
	blp->mcp = mcp;

	return mcp->sock;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  for sise receive - test
***************************************************************************** */
int Blp_SiseRecv( BLP *blp, char *buf, int sz)
{
	int		rtn;

	rtn = Mcp_RecvT( blp->mcp, buf, sz, 1000000);
	if( rtn < 0)
	{
		LogErr( "Mcp_Recv error. buf=[%p] sz=[%d]", buf, sz);
		return -1;
	}

	return rtn;
}


#ifndef _OMS_SOURCE_

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  for sise receive - test
***************************************************************************** */
int Od_Write_DataBond( int option, char *buf, int sz)
{
	KRX_LP_NOTE_JUMUN_DATA	*order = ( KRX_LP_NOTE_JUMUN_DATA *)buf;

	KRX_LP_NOTE_JUMUN_DATA_Print( order);
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  for sise receive - test
***************************************************************************** */
int GetOrderNo( char *buf, int sz)
{
	return 1;
}

#endif


