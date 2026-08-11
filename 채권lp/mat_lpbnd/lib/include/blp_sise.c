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

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 체결 시세 처리
***************************************************************************** */
int Blp_ProcessSiseExe( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise, CO_G701K *g701k)
{
	int		rtn;

	rtn = Blp_ConvertExe( blp, sise, g701k);		
	if( rtn < 0)
	{
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 호가 시세 처리
***************************************************************************** */
int Blp_ProcessSiseHoga( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise, CO_G701K *g701k)
{
	int			rtn;
	BLP_ARG		*arg = &tbl->arg;
	BLP_TIME	*mk_time = &arg->mk_time[ arg->mk_stat -1];

	/***********************************/
	/* 시세 convert                    */
	/***********************************/
	rtn = Blp_ConvertHoga( blp, sise, ( CO_B601K *)g701k);		
	if( rtn < 0)
	{
		return 0;
	}

	/***********************************/
	/* pre set                         */
	/***********************************/
	memcpy( tbl->board_id, g701k->board_id, sizeof( tbl->board_id));

	/***********************************/
	/* 시장 조성 호가 조건 검사        */
	/***********************************/
	if( ( sise->ask[ 0][ 0] - sise->bid[ 0][ 0]) > mk_time->sped_prc[ 0])
	{
		LogMsg( "시장조성 조건이 맞지 않습니다.");
		LogMsg( "    ask1[%10.2f] - bid1[%10.2f] = [%10.2f] > sped1_prc[%10.2f]",
				sise->ask[ 0][ 0], sise->bid[ 0][ 0],  sise->ask[ 0][ 0] - sise->bid[ 0][ 0], mk_time->sped_prc[ 0]);
		/* 참고 - LP가 들어가 있는경우는 시장조건은 항상 참 이기 때문에 주문 check 불필요 (cancel process) */
		return 0;
	}

	tbl->ask_base = ( BLP_MAX_HOGA -1) / 2 -1;		/* 매도 중간 position을 구함 - 매도/매수 중간값 자리 비워 놓음 */
	tbl->bid_base = ( BLP_MAX_HOGA +1) / 2;			/* 매수 중간 position을 구함 */
	/***********************************/
	/* HOGA TABLE 구성                 */
	/***********************************/
	LogDbg( "호가 테이블 구성");
	memcpy( &tbl->old, &tbl->rec, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	// memset( &tbl->rec[ 0], 0x00, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	rtn = Blp_MakeHogaTbl( blp, tbl, sise);
	if( rtn < 0)
	{
		LogCri( "Blp_MakeHogaTbl error.");
		return -1;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 시세 처리 - 전체 종목 처리 (현재 안씀)
***************************************************************************** */
int Blp_ProcessSise( BLP *blp, char *krx_sise)
{
	int				rtn;
	int				pos;
	CO_G701K		*g701k = ( CO_G701K *)krx_sise;
	BLP_TBL	*tbl;
	BLP_SISE		*sise;
	BLP_ARG			*arg;
	BLP_TIME		*mk_time;

	/***********************************/
	/* LP 종목인지 check               */
	/***********************************/
	pos = Blp_GetItem( blp, g701k->item_code);
	if( pos <= 0)
	{
		LogMsg( "종목이 없습니다. item_code=[%.12s]", g701k->item_code);
		return 0;
	}
	LogDbg( "pos=[%d]", pos);
	tbl     = &blp->map->tbl[ pos];
	sise    = &tbl->sise;
	arg     = &tbl->arg;
	mk_time = &arg->mk_time[ arg->mk_stat -1];

	/***********************************/
	/* 시세 convert                    */
	/***********************************/
	if( !memcmp( krx_sise, "G701K", 5)) 
	{ 
		LogDbg( "시세 체결 처리");
		CO_G701K_Print( ( CO_G701K *)krx_sise);
		rtn = Blp_ConvertExe( blp, sise, ( CO_G701K *)krx_sise);		
	}
	else
	if( !memcmp( krx_sise, "B601K", 5)) 
	{ 
		LogDbg( "시세 호가 처리");
		CO_B601K_Print( ( CO_B601K *)krx_sise);
		rtn = Blp_ConvertHoga( blp, sise, ( CO_B601K *)krx_sise);
	}
	else
	{
		LogCri( "시세 데이타 오류. sise=[%.5s]", krx_sise);
		rtn = -1;
	}
	// BLP_SISE_Print( sise);

	/***********************************/
	/* 시장 조성 호가 조건 검사        */
	/***********************************/
	if( ( sise->ask[ 0][ 0] - sise->bid[ 0][ 0]) > mk_time->sped_prc[ 0])
	{
		LogMsg( "시장조성 조건이 맞지 않습니다.");
		LogMsg( "    ask1[%10.2f] - bid1[%10.2f] = [%10.2f] > sped1_prc[%10.2f]",
				sise->ask[ 0][ 0], sise->bid[ 0][ 0],  sise->ask[ 0][ 0] - sise->bid[ 0][ 0], mk_time->sped_prc[ 0]);
		/* 참고 - LP가 들어가 있는경우는 시장조건은 항상 참 이기 때문에 주문 check 불필요 (cancel process) */
		return 0;
	}

	/***********************************/
	/* pre set                         */
	/***********************************/
	tbl->seq_no++;		/* 시세 순번 = space  단순 1 증가 */
	sise->exe_cnt++;
	memcpy( tbl->board_id, g701k->board_id, sizeof( tbl->board_id));

	tbl->ask_base = ( BLP_MAX_HOGA -1) / 2 -1;		/* 매도 중간 position을 구함 - 매도/매수 중간값 자리 비워 놓음 */
	tbl->bid_base = ( BLP_MAX_HOGA +1) / 2;			/* 매수 중간 position을 구함 */

	/***********************************/
	/* HOGA TABLE 구성                 */
	/***********************************/
	LogDbg( "호가 테이블 구성");
	memcpy( &tbl->old, &tbl->rec, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	// memset( &tbl->rec[ 0], 0x00, sizeof( BLP_HOGA_REC) * BLP_MAX_HOGA);
	rtn = Blp_MakeHogaTbl( blp, tbl, sise);
	if( rtn < 0)
	{
		LogCri( "Blp_MakeHogaTbl error.");
		return -1;
	}

	/***********************************/
	/* 주문                            */
	/***********************************/
	/* check order price */
	LogDbg( "주문 check");
	rtn = Blp_CheckOrder( blp, tbl, sise);
	if( rtn < 0)
	{
		LogCri( "Blp_CheckOrder error.");
		return -1;
	}


	return rtn;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  sise receive from file - for test
***************************************************************************** */
int Blp_GetSiseFromFile( BLP *blp, CO_B601K *b601k, char *f_name)
{
	static FILE	*fp = NULL;
	static int	line = 0;
	char		*ptr;
	char		rec[ 8192];
	int			st = 0;

	if( fp == NULL)
	{
		fp = fopen( f_name, "r");
		if( fp == NULL)
		{
			LogErr( "fopen error. name=[%s]", f_name);
			return -1;
		}
	}

	ptr = fgets( rec, 8192, fp);
	if( ptr == NULL)
	{
		fclose( fp);
		fp = NULL;
		line = 0;
		return 0;
	}
	line++;
	LogDbg( "rec=[%d:%s]", line, rec);
	if( rec[ 0] == '#') 
	{
		if( !memcmp( &rec[1], "sleep", 5))
		{
			st = atoi( &rec[ 6]);
		}
		LogDbg( "sleep [%d]", st);
		if( st) sleep( st);
		return 0;
	}
	memcpy( b601k, rec, sizeof( CO_B601K));

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  호가 테이블 구성
***************************************************************************** */
int Blp_MakeHogaTbl( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise)
{
	int				pos;		/* rec 호가 위치 */
	int				no;			/* sise 호가 위치 */
	int				ho;			/* 호가순번 0:매도1호가 1:매도2호가 -1:매수1호가 */
	int				lp_no = 0;	/* LP 순번 */
	double			price;
	BLP_HOGA_REC	*rec, *old;;
	BLP_ORD			*ord;

	LogDbg( "start ....... ");

	/* 매도/매수 중간값 계산 */
	rec = &tbl->rec[ tbl->ask_base +1];
	rec->price = ( sise->ask[ 0][ 0] + sise->bid[ 0][ 0]) / 2;


	ho = 1;
	no = 0;
	lp_no = 0;
	price = sise->ask[ 0][ 0];	/* 매도 1호가 */
	for( pos = tbl->ask_base; pos >= BLP_SISE_HOGA; pos--)					/* 매도 set */
	{
		rec = &tbl->rec[ pos];
		old = &tbl->old[ pos];
		rec->ab = 1;
		rec->no = pos;
		rec->ho = ho++;
		rec->price = price;
		if( price == sise->ask[ 0][ no] && no < BLP_SISE_HOGA)				/* 시세 호가에 수량 set  */
		{
			rec->s_ho = no +1;
			rec->volume = sise->ask[ 1][ no];
			no++;
		}
		else
		{
			rec->s_ho = 0;
			rec->volume = 0;
		}

		/* 호가갭 계산 - 이전과 변화 gap */
		if( rec->price != old->price)				 						
		{
			rec->gap = -( int)(( old->price - rec->price) / tbl->arg.tick_unit);
			LogDbg( "ask old=[%10.2f] new=[%10.2f] rec->gap=[%d] vol=[%10.0f]", old->price, rec->price, rec->gap, rec->gap_vol);
		}
		else
		{
			rec->gap = 0;
		}

		/* 잔량 계산 */
		if( rec->volume != old->volume) 		rec->gap_vol = rec->volume - old->volume;
		else									rec->gap_vol = 0.0;

		/* LP 주문 위치 */
		ord = &tbl->order[ lp_no];
		// 20251001 if( price == ord->ask_prc)
		if( rec->price == ord->ask_prc)
		{
			rec->lp_no = lp_no +1;
			lp_no++;
		}
		else
		{
			rec->lp_no = 0;
		}



		price += tbl->arg.tick_unit;
	}
	if( no < BLP_SISE_HOGA)	/* 호가 TABLE 범위안에 시세호가가 없을시 */
	{
		for( pos = BLP_SISE_HOGA -1; pos >= 0; pos--)
		{
			rec = &tbl->rec[ pos];
			rec->ab = 1;
			rec->ho = 0;
			rec->s_ho = no +1;
			rec->no = 0;
			rec->price = sise->ask[ 0][ no];
			rec->volume = sise->ask[ 1][ no];
			no++;
			if( no >= BLP_SISE_HOGA) break;
		}
		for( ; pos >= 0; pos--)
		{
			rec = &tbl->rec[ pos];
			memset( rec, 0x00, sizeof( BLP_HOGA_REC));
		}
	}

	ho = -1;
	no = 0;
	lp_no = 0;
	price = sise->ask[ 0][ 0];	/* 매도 1호가 - 매수/매도 사이에 갭이 있을 수 있으므로 매도1호가 기준 */
	for( pos = tbl->bid_base; pos < BLP_MAX_HOGA - BLP_SISE_HOGA; pos++)		/* 매수 set */
	{
		price -= tbl->arg.tick_unit;

		rec = &tbl->rec[ pos];
		old = &tbl->old[ pos];
		rec->ab = 2;
		rec->no = pos;
		rec->ho = ho--;
		rec->price = price;
		if( price == sise->bid[ 0][ no] && no < BLP_SISE_HOGA)
		{
			rec->s_ho = no +1;
			rec->volume = sise->bid[ 1][ no];
			no++;
		}
		else
		{
			rec->s_ho = 0;
			rec->volume = 0;
		}

		if( rec->price != old->price)				 						/* 호가갭 계산 */
		{
			rec->gap = -( int)(( old->price - rec->price) / tbl->arg.tick_unit);
			LogDbg( "bid old=[%10.2f] new=[%10.2f] rec->gap=[%d]", old->price, rec->price, rec->gap);
		}
		else
		{
			rec->gap = 0;
		}

		/* 잔량 계산 */
		if( rec->volume != old->volume) 			rec->gap_vol = rec->volume - old->volume;
		else										rec->gap_vol = 0.0;

		/* LP 주문 위치 */
		ord = &tbl->order[ lp_no];
		// 20251001 if( price == ord->bid_prc)
		if( rec->price == ord->bid_prc)
		{
			rec->lp_no = lp_no +1;
			lp_no++;
		}
		else
		{
			rec->lp_no = 0;
		}
	}
	LogDbg( "bid no=[%d]", no);
	if( no < BLP_SISE_HOGA)	/* 호가 TABLE 범위안에 시세호가가 없을시 */
	{
		for( pos = BLP_MAX_HOGA - BLP_SISE_HOGA; pos < BLP_MAX_HOGA; pos++)
		{
			rec = &tbl->rec[ pos];
			rec->ab = 2;
			rec->ho = 0;
			rec->s_ho = no +1;
			rec->no = 0;
			rec->price = sise->bid[ 0][ no];
			rec->volume = sise->bid[ 1][ no];
			no++;
			if( no >= BLP_SISE_HOGA) break;
		}
		for( ; pos < BLP_MAX_HOGA; pos++)
		{
			rec = &tbl->rec[ pos];
			memset( rec, 0x00, sizeof( BLP_HOGA_REC));
		}
	}

	return pos;
}

