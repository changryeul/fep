/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "log.h"
#include "etc.h"
#include "mat.h"

#include "order.h"
#include "sise.h"

extern int		Continue;

char	MatCurrent[32][4] =
{
	"KRW",
	"USD",
	"EUR",
	"JPY",
	"GBP",
	"AUD",
	"NZD",
	"CAD",
	"CHF",
	"CNH",
	"CNY",
	"SGD",
	"THB",
	"DKK",
	"NOK",
	"SEK",
	"HKD",
	"\0\0\0"
};

MAT_REJECT	MatReject[ 100] =
{
	{ MAT_ERR_MSGTYPE,	"주문타입은 신규/정정/취소만 가능합니다." },
	{ MAT_ERR_ORIGID,	"원주문이 없습니다." },
	{ MAT_ERR_SIDE,		"매매구분은 BUY/SELL만 가능합니다." },
	{ MAT_ERR_QTY,		"주문수량을 확인해주세요." },
	{ MAT_ERR_ORDTYPE,	"주문유형은 시장가/지정가/예약주문만 가능합니다." },
	{ MAT_ERR_SETTTYPE,	"주문 구분 미지원" },
	{ -1,				"\0" },
};

/** ***************************************************************************
**  @fu         int Mat_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
MAT* Mat_CreateForce()
{
	int		rtn;
	MAT		*mat;

	mat = malloc( sizeof( MAT));
	if( mat == NULL)
	{
		LogErr( "malloc error.");
	}
	memset( mat, 0x00, sizeof( MAT));
	mat->key = MAT_IPC_KEY;

	mat->mem = Mem_Create( mat->key, sizeof( MAT_MAP));
	if( mat->mem == NULL)
	{
		LogLib( "Mem_Create error.");
	}
	mat->map = Mem_GetPtr( mat->mem);

	mat->sem = Sem_Create( mat->key);
	if( mat->sem == NULL)
	{
		LogLib( "Sem_Create error.");
	}

	sprintf( mat->map->stat.pipe_name, "%s", MAT_PIPE_NAME);
	LogDel( "pipe_name=[%s]", mat->map->stat.pipe_name);
	rtn = mkfifo( mat->map->stat.pipe_name, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.pipe_name);
	}

	mat->fd = open( mat->map->stat.pipe_name, O_RDWR);
	if( mat->fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.pipe_name);
	}

	Mat_Init( mat);

	return mat;
}

/** ***************************************************************************
**  @fu         int Mat_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
MAT* Mat_Create()
{
	int		rtn;
	MAT		*mat;

	mat = malloc( sizeof( MAT));
	if( mat == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( mat, 0x00, sizeof( MAT));
	mat->key = MAT_IPC_KEY;

	mat->mem = Mem_Create( mat->key, sizeof( MAT_MAP));
	if( mat->mem == NULL)
	{
		LogLib( "Mem_Create error.");
		goto error_1;
	}
	mat->map = Mem_GetPtr( mat->mem);
	mat->map->stat.key = MAT_IPC_KEY;

	mat->sem = Sem_Create( mat->key);
	if( mat->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error_2;
	}

	sprintf( mat->map->stat.pipe_name, "%s", MAT_PIPE_NAME);
	LogDel( "pipe_name=[%s]", mat->map->stat.pipe_name);
	rtn = mkfifo( mat->map->stat.pipe_name, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.pipe_name);
		goto error_3;
	}

	mat->fd = open( mat->map->stat.pipe_name, O_RDWR);
	if( mat->fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.pipe_name);
		goto error_4;
	}

	Mat_Init( mat);

	return mat;

	error_4:
		unlink( mat->map->stat.pipe_name);
	error_3:
		Sem_Remove( mat->sem);
	error_2:
		Mem_Remove( mat->mem);
	error_1:
		free( mat);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Mat_Remove( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭엔진에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Mat_Remove( MAT *mat)
{
	int		rtn;

	close( mat->fd);
	rtn = unlink( mat->map->stat.pipe_name);
	if( rtn < 0)
	{
		LogErr( "pipe unlink error. name=[%s]", mat->map->stat.pipe_name);
	}

	rtn = Sem_Remove( mat->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Remove error. sem=[%p] key=[0x%08x]", mat->sem, mat->key);
		return -1;
	}

	rtn = Mem_Remove( mat->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Remove error. mem=[%p] key=[0x%08x]", mat->mem, mat->key);
		return -1;
	}

	free( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_Open()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mat pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진 Open
***************************************************************************** */
MAT* Mat_Open()
{
	MAT		*mat;

	mat = malloc( sizeof( MAT));
	if( mat == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( mat, 0x00, sizeof( MAT));
	mat->key = MAT_IPC_KEY;

	mat->mem = Mem_Open( mat->key);
	if( mat->mem == NULL)
	{
		LogLib( "Mem_Open error.");
		goto error_1;
	}
	mat->map = Mem_GetPtr( mat->mem);
	LogDel( "attach shared memory ... ptr=[%p]", mat->map);

	mat->sem = Sem_Open( mat->key);
	if( mat->sem == NULL)
	{
		LogLib( "Sem_Open error.");
		goto error_2;
	}

	LogDel( "pipe open ... name=[%s]", mat->map->stat.pipe_name);
	mat->fd = open( mat->map->stat.pipe_name, O_RDWR);
	if( mat->fd < 0)
	{
		LogDel( "ptr=[%p]", mat->map);
		LogErr( "pipe open error. name=[%s]", mat->map->stat.pipe_name);
		goto error_3;
	}

	return mat;

	error_4:
		close( mat->fd);
	error_3:
		Sem_Close( mat->sem);
	error_2:
		Mem_Close( mat->mem);
	error_1:
		free( mat);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Mat_Close( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭엔진 Close
***************************************************************************** */
int Mat_Close( MAT *mat)
{
	int		rtn;

	rtn = Sem_Close( mat->sem);
	if( rtn < 0)
	{
		LogLib( "Sem_Close error. sem=[%p] key=[0x%08x]", mat->sem, mat->key);
		return -1;
	}

	rtn = Mem_Close( mat->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Close error. mem=[%p] key=[0x%08x]", mat->mem, mat->key);
		return -1;
	}

	free( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index initial
***************************************************************************** */
int Mat_Init( MAT *mat)
{
	int			i;
	int			rtn;
	MAT_INDEX	*index;

	time( &mat->map->stat.ctime);
	mat->map->stat.max_rec = MAT_MAX_RECORD;

	/* index initial */
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->stat.index[ i];
		index->base_cur = 0;			/* 기준통화 */
		index->cont_cur = 0;			/* 상대통화 */
		index->point = 0;				/* 소숫점 이하 자리 */
		index->unit = 0;				/* 통화단위 */
		index->start[ 0] = -1;			/* 매수 start position */
		index->start[ 1] = -1;			/* 매도 start position */
		index->cnt[ 0] = 0;				/* 매수 주문 갯수 */
		index->cnt[ 1] = 0;				/* 매도 주문 갯수 */
	}

	Mat_AddIndex( mat, "USD/KRW", 2, 1);
	Mat_AddIndex( mat, "USD/JPY", 3, 1);
	Mat_AddIndex( mat, "HKD/KRW", 2, 1);
	Mat_AddIndex( mat, "EUR/USD", 5, 1);
	Mat_AddIndex( mat, "GBP/USD", 5, 1);
	Mat_AddIndex( mat, "AUD/USD", 5, 1);
	Mat_AddIndex( mat, "NZD/USD", 5, 1);
	Mat_AddIndex( mat, "USD/CAD", 5, 1);
	Mat_AddIndex( mat, "USD/CHF", 5, 1);
	Mat_AddIndex( mat, "JPY/KRW", 2, 100);
	Mat_AddIndex( mat, "EUR/KRW", 2, 1);
	Mat_AddIndex( mat, "GBP/KRW", 2, 1);
	Mat_AddIndex( mat, "AUD/KRW", 2, 1);
	Mat_AddIndex( mat, "NZD/KRW", 2, 1);
	Mat_AddIndex( mat, "CAD/KRW", 2, 1);
	Mat_AddIndex( mat, "CHF/KRW", 2, 1);
	Mat_AddIndex( mat, "USD/CNH", 5, 1);
	Mat_AddIndex( mat, "CNH/KRW", 2, 1);
	Mat_AddIndex( mat, "SEK/KRW", 2, 1);
	Mat_AddIndex( mat, "SGD/KRW", 2, 1);
	Mat_AddIndex( mat, "THB/KRW", 2, 1);
	Mat_AddIndex( mat, "USD/HKD", 5, 1);
	Mat_AddIndex( mat, "USD/SEK", 5, 1);
	Mat_AddIndex( mat, "USD/SGD", 5, 1);
	Mat_AddIndex( mat, "USD/THB", 3, 1);

	for( i = 0; i < MAT_MAX_CONFORM; i++)
	{
		mat->map->conform[ i] = -1;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭
***************************************************************************** */
int Mat_AddIndex( MAT *mat, char *current, int point, int unit)
{
	int			i;
	int			base, cont;
	MAT_INDEX	*index;

	base = Mat_GetCurrentInt( mat, &current[ 0]);
	cont = Mat_GetCurrentInt( mat, &current[ 4]);

	index = Mat_GetIndex( mat, base, cont);
	if( index != NULL)
	{
		LogMsg( "이미 등록된 통화입니다. current=[%s]", current);
		return -1;
	}

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->stat.index[ i];
		if( index->base_cur == 0 && index->cont_cur == 0) break;
	}

	index->base_cur = base;
	index->cont_cur = cont;
	index->point = point;
	index->unit = unit;

	return i;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭
***************************************************************************** */
int Mat_Match( MAT *mat, MATSISE *sise)
{
	int			i, rtn;
	int			cnt;
	int			base_cur;
	int			cont_cur;
	MAT_INDEX	*index;

	LogDel( "Mat_Match ... start");

	Mat_StatisticsSet( mat, &mat->map->stat.stat.sis, MAT_START);

	/* 기준통화 */
	base_cur = Mat_GetCurrentInt( mat, &sise->symb[ 0]);
	if( base_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. base_cur=[%.3s]", &sise->symb[ 0]);
		return -1;
	}

	/* 상대통화 */
	cont_cur = Mat_GetCurrentInt( mat, &sise->symb[ 3]);
	if( cont_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. cont_cur=[%.3s]", &sise->symb[ 3]);
		return -1;
	}
	LogDbg( "메칭 시작 ... 기준통화/상대통화=[%s(%2d)/%s(%2d)]", 
		MatCurrent[ base_cur], base_cur,
		MatCurrent[ cont_cur], cont_cur);

	LogDbg( "              bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);

	Mat_Lock( mat);
	rtn = Mat_SiseMatch( mat, base_cur, cont_cur, sise);
	Mat_Unlock( mat);

	Mat_StatisticsSet( mat, &mat->map->stat.stat.sis, MAT_END);
	LogDel( "Mat_Match ... end");
	return 0;

	error:
		Mat_Unlock( mat);
		Mat_StatisticsSet( mat, &mat->map->stat.stat.sis, MAT_END);
		LogDel( "Mat_Match error ... end");
		return -1;

}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  new sise update - 가공된 MATSISE 처리
***************************************************************************** */
int Mat_SiseMatch( MAT *mat, int base, int cont, MATSISE *sise)
{
	int			i, rtn;
	MAT_INDEX	*index;

	LogDel( "시세 PROCESS ... sise=[%s(%2d)/%s(%2d)]", MatCurrent[ base], base, MatCurrent[ cont], cont);

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->stat.index[ i];
		LogDel( "index ... sise=[%s(%2d)/%s(%2d)]", 
				MatCurrent[ index->base_cur], index->base_cur, 
				MatCurrent[ index->cont_cur], index->cont_cur);

		/* 빈 index */
		if( index->base_cur == 0 && index->cont_cur == 0) continue;

		/* 시세 일치 통화 */
		if( base == index->base_cur && cont == index->cont_cur)
		{
			LogDel( "index->sise_curr update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
			Mat_SiseUpdate( mat, index, sise, 0);
		}

		/* USD/KRW는 시세 일치만 update */
		if( index->base_cur == 1 && index->cont_cur == 0)	continue;

		if( base == 1 && cont == 0)								/* USD/KRW */
		{
			if( index->cont_cur == 0)											/* 재정통화 cont */
			{
				LogDel( "index->sise_cont update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 1);
			}
		}
		else
		if( base == 1 && cont == index->base_cur)				/* USD/@@@ */
		{
			if( index->cont_cur == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 2);
			}
		}
		else
		if( cont == 1 && base == index->base_cur)				/* @@@/USD */
		{
			if( index->cont_cur == 0)											/* 재정통화 base */
			{
				LogDel( "index->sise_base update ... bid=[%f] ask=[%f]", sise->bidprc, sise->askprc);
				Mat_SiseUpdate( mat, index, sise, 3);
			}
		}
	} /* for */

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  sise update - FX_QUOTE_T 처리
***************************************************************************** */
int Mat_SiseUpdate( MAT *mat, MAT_INDEX *index, MATSISE *sise, int type)
{
	int				rtn;
	struct timeval	tv;
	struct tm		*tp;
	char			time_str[ 32];
	double			unit;
	MATSISE			*curr, *base, *cont;

	curr = &index->sise_curr;
	base = &index->sise_base;
	cont = &index->sise_cont;

	gettimeofday( &tv, NULL);
	tp = localtime( &tv.tv_sec);

	if( type)			/* index->sise_curr update가 아닌경우 MATSISE의 field update */
	{
		sprintf( time_str, "%04d%02d%02d%02d%02d%02d%03d", 
				tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
				tp->tm_hour, tp->tm_min, tp->tm_sec,
				tv.tv_usec / 1000);
		curr->excode[ 0] = 'M';
		curr->bidex[ 0]  = ' ';
		curr->askex[ 0]  = ' ';
		memcpy( &curr->symb[ 0], &base->symb[ 3], 3);
		memcpy( &curr->symb[ 3], &cont->symb[ 3], 3);
		memcpy( &curr->date, time_str, sizeof( curr->date) + sizeof( curr->time));
		unit = ( double)index->unit;
	}

	switch( type)
	{
		case 0:			/* current update */
			memcpy( &index->sise_curr, sise, sizeof( MATSISE));
			break;
		case 1:			/* 재정통화 USD/KRW update */
			memcpy( &index->sise_cont, sise, sizeof( MATSISE));
			if( !memcmp( &base->symb[ 0], "USD", 3))
			{
				if( cont->bidprc != 0.0 && base->askprc != 0.0)	curr->bidprc = ( cont->bidprc / base->askprc) * unit;
				else											curr->bidprc = 0.0;
				if( cont->askprc != 0.0 && base->bidprc != 0.0)	curr->askprc = ( cont->askprc / base->bidprc) * unit;
				else											curr->askprc = 0.0;
			}
			else								
			{
				if( cont->bidprc != 0.0 && base->bidprc != 0.0)	curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
				else											curr->bidprc = 0.0;
				if( cont->askprc != 0.0 && base->askprc != 0.0)	curr->askprc = ( cont->askprc * base->askprc) * unit;
				else											curr->askprc = 0.0;
			}
			break;
		case 2:			/* base update USD/@@@ */
			memcpy( &index->sise_base, sise, sizeof( MATSISE));
			if( cont->bidprc != 0.0 && base->askprc != 0.0)	curr->bidprc = ( cont->bidprc / base->askprc) * unit;
			else											curr->bidprc = 0.0;
			if( cont->askprc != 0.0 && base->bidprc != 0.0)	curr->askprc = ( cont->askprc / base->bidprc) * unit;
			else											curr->askprc = 0.0;
			break;
		case 3:			/* base update @@@/USD */
			memcpy( &index->sise_base, sise, sizeof( MATSISE));
			if( cont->bidprc != 0.0 && base->bidprc != 0.0)	curr->bidprc = ( cont->bidprc * base->bidprc) * unit;
			else											curr->bidprc = 0.0;
			if( cont->askprc != 0.0 && base->askprc != 0.0)	curr->askprc = ( cont->askprc * base->askprc) * unit;
			else											curr->askprc = 0.0;
			break;
	}

	rtn = Mat_Matching( mat, index);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @param      MAT_INDEX *index - index pointer
**  @param      int side - 0=매수,1=매도
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  매수/매도체결
***************************************************************************** */
int Mat_Matching( MAT *mat, MAT_INDEX *index)
{
	int			rtn;
	int			pos, ppos, npos;
	MAT_HEAD	*head;
	ORDER		*obook;
	MAT_RECORD	*rec;		/* curr record */
	MAT_RECORD	*prec;		/* prev record */
	MAT_RECORD	*nrec;		/* next record */
	MATSISE	*sise = &index->sise_curr;

	int			side;
	double		price;

	LogDel( "Matching base=[%s] cont=[%s]", MatCurrent[ index->base_cur], MatCurrent[ index->cont_cur]);

	LogDbg( "Matching ... [%s/%s] count[0]=[%d] count[1]=[%d]", 
			MatCurrent[ index->base_cur], MatCurrent[ index->cont_cur], index->cnt[ 0], index->cnt[ 1]);
	for( side = 0; side < 2; side++)
	{
		for( pos = index->start[ side]; pos >= 0; pos = head->next)
		{
			rec = &mat->map->rec[ pos];
			head = &rec->head;
			obook = &rec->book;

			if( side == 0)	price = sise->bidprc;
			else			price = sise->askprc;
			if( price <= 0) continue;

	
			LogDbg( "compare[%d] ... pos=[%d] price=[%12f] my=[%12f]", side, pos, price, obook->Price);
	
			/* 매칭 비교 */
			if( side == 0 && obook->Price < price) continue;
			if( side == 1 && obook->Price > price) continue;

			LogDbg( "execute ... pos=[%d] price=[%12f] my=[%12f]", pos, price, obook->Price);

			/* duble linked list 에서 삭제 */
			Mat_RecordUnlink( mat, index, rec, pos, side);

			/* 체결가 */
			head->exe_price = price;

			/* record update - gubun을 2로 setting하여 체결 레코드로 표시 */
			rec->head.gubun = 2;

			mat->map->stat.exe_cnt++;
			LogDel( "exe_cnt=[%d]", mat->map->stat.exe_cnt);
			/**** 체결 record position을 pipe에 전송 *****/
			retry:
			LogDel( "write to pipe... fd=[%d] pos=[%d]", mat->fd, pos);
			rtn = write( mat->fd, ( char *)&pos, sizeof( int));
			if( rtn < sizeof( int))
			{
				switch( errno)
				{
					case EINTR:
					case EPIPE:
						LogErr( "pipe write error. retry ... rtn=[%d]", rtn);
						goto retry;
						
				}
				/* pipe full일경우 해소 될때 까지 대기 */
				if( rtn >= 0)
				{
					LogMsg( "pipe가 full입니다. rtn=[%d]", rtn);
					sleep( 1);
					goto retry;
				}
				LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->fd, rtn);
				return -1;
			}
			LogDel( "pipe write ... rtn=[%d]", rtn);
		}	/* for pos */
	}	/* for side */

	return npos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  매수체결
***************************************************************************** */
int Mat_MatchBid( MAT *mat, MAT_INDEX *index, MATSISE *sise)
{
	int			rtn;
	int			pos, ppos, npos;
	int			side = 0;	/* 매도 */
	MAT_HEAD	*head;
	ORDER		*obook;
	MAT_RECORD	*rec;		/* curr record */
	MAT_RECORD	*prec;		/* prev record */
	MAT_RECORD	*nrec;		/* next record */

	double		price;

	pos = index->start[ side];
	while( pos >= 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		head = &rec->head;
		obook = &rec->book;

		/* bid price 이므로 ask 시세랑 비교 */
		price = sise->askprc;
		LogDel( "match bid ... price=[%12f] >= sise=[%12f]", obook->Price, price);
		/* 매칭 비교 */
		if( obook->Price >= price)
		{
			LogDel( "execute ... pos=[%d]", pos);

			/* head sise update */
			memcpy( &head->sise_curr, &index->sise_curr, sizeof( MATSISE) *3);

			/* duble linked list 에서 삭제 */
			Mat_RecordUnlink( mat, index, rec, pos, side);

			/* 체결가 */
			head->exe_price = price;

			/* record update - gubun을 2로 setting하여 체결 레코드로 표시 */
			rec->head.gubun = 2;

			mat->map->stat.exe_cnt++;
			LogDel( "exe_cnt=[%d]", mat->map->stat.exe_cnt);
			/**** 체결 record position을 pipe에 전송 *****/
			retry:
			LogDel( "write to pipe... fd=[%d] pos=[%d]", mat->fd, pos);
			rtn = write( mat->fd, ( char *)&pos, sizeof( int));
			if( rtn < sizeof( int))
			{
				switch( errno)
				{
					case EINTR:
					case EPIPE:
						LogErr( "pipe write error. retry ... rtn=[%d]", rtn);
						goto retry;
						
				}
				/* pipe full일경우 해소 될때 까지 대기 */
				if( rtn >= 0)
				{
					LogMsg( "pipe가 full입니다. rtn=[%d]", rtn);
					sleep( 1);
					goto retry;
				}
				LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->fd, rtn);
				return -1;
			}
			LogDel( "pipe write ... rtn=[%d]", rtn);
		}
		pos = head->next;
	}

	return npos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  매도체결
***************************************************************************** */
int Mat_MatchAsk( MAT *mat, MAT_INDEX *index, MATSISE *sise)
{
	int			rtn;
	int			pos, ppos, npos;
	int			side = 1;	/* 매도 */
	MAT_HEAD	*head;
	ORDER		*obook;
	MAT_RECORD	*rec;		/* curr record */
	MAT_RECORD	*prec;		/* prev record */
	MAT_RECORD	*nrec;		/* next record */

	double		price;

	pos = index->start[ side];
	while( pos >= 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		head = &rec->head;
		obook = &rec->book;

		/* ask price 이므로 bid 시세랑 비교 */
		price = sise->bidprc;
		LogDel( "match ask[%.7s] price=[%12f] >= sise=[%12f]", obook->Symbol, obook->Price, price);
		/* 매칭 비교 */
		if( obook->Price <= price)
		{
			LogDel( "execute ... pos=[%d]", pos);

			/* head sise update */
			memcpy( &head->sise_curr, &index->sise_curr, sizeof( MATSISE) *3);

			/* duble linked list 에서 삭제 */
			Mat_RecordUnlink( mat, index, rec, pos, side);

			/* 체결가 */
			head->exe_price = price;

			/* record update - gubun을 2로 setting하여 체결 레코드로 표시 */
			rec->head.gubun = 2;

			mat->map->stat.exe_cnt++;
			LogDel( "exe_cnt=[%d]", mat->map->stat.exe_cnt);
			/**** 체결 record position을 pipe에 전송 *****/
			retry:
			LogDel( "write to pipe... fd=[%d] pos=[%d]", mat->fd, pos);
			rtn = write( mat->fd, ( char *)&pos, sizeof( int));
			if( rtn < sizeof( int))
			{
				/* pipe full일경우 해소 될때 까지 대기 */
				if( rtn >= 0)
				{
					LogMsg( "pipo가 full입니다. rtn=[%d]", rtn);
					sleep( 1);
					goto retry;
				}
				LogErr( "pipe write error. fd=[%d] rtn=[%d]", mat->fd, rtn);
				return -1;
			}
			LogDel( "pipe write ... rtn=[%d]", rtn);
		}
		pos = head->next;
	}

	return npos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  체결처리
***************************************************************************** */
int Mat_Execute( MAT *mat, int timeout)
{
	int			pos;
	MAT_HEAD	*head;
	ORDER		*obook;
	MAT_RECORD	*rec;		/* curr record */

	pos = Mat_ReadPipe( mat, timeout);
	if( pos < 0)
	{
		if( pos == MAT_TIMEOUT)
		{
			LogDel( "Mat_ReadPipe timeout... exe_cnt=[%d]", mat->map->stat.exe_cnt);
			return pos;
		}
		return -1;
	}

	Mat_StatisticsSet( mat, &mat->map->stat.stat.mat, MAT_START);

	rec = &mat->map->rec[ pos];
	head = &rec->head;
	obook = &rec->book;

	LogDel( "execute send ... pos[%d]", pos);

	/* 체결 처리후 빈 record로 set */
	/*
	memset( rec, 0x00, sizeof( MAT_RECORD));
	*/

	/* lock을 안거니 matching쪽에 exe_cnt가 무결성 보장이 안됨 ... 왜인지는 모름 */
	Mat_Lock( mat);
	mat->map->stat.exe_cnt--;
	Mat_Unlock( mat);

	Mat_StatisticsSet( mat, &mat->map->stat.stat.mat, MAT_END);
	Mat_StatisticsSet( mat, &mat->map->stat.stat.exe, MAT_START);
	LogDel( "exe_cnt = [%d]", mat->map->stat.exe_cnt);
	Mat_StatisticsSet( mat, &mat->map->stat.stat.exe, MAT_END);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - MAT_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Mat_ReadPipe( MAT *mat, int timeout)
{
	int				rtn, pos;
	fd_set			rfds;
	char			rec[ 512];
	struct timeval	tv, *tp = &tv;

	tp->tv_sec  = timeout / 1000000;
	tp->tv_usec = timeout % 1000000;

	while( Continue)
	{
		FD_ZERO( &rfds);
		FD_SET( mat->fd, &rfds);

		rtn = select( mat->fd +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "pipe select error. fd=[%d]", mat->fd);
			switch( errno)
			{
				case 4:	/* Interrupted system call */
					continue;
				default:
					break;
			}
			return -1;
		}
		else if( rtn == 0)
		{
			LogDel( "recv timeout. timeout=[%d.%d]", timeout / 1000000, timeout % 1000000);
			return MAT_TIMEOUT;
		}

		if( FD_ISSET( mat->fd, &rfds))
		{
			rtn = read( mat->fd, ( char *)&pos, sizeof( int));
			if( rtn < sizeof( int))
			{
				LogErr( "pipe read error. fd=[%d] rtn=[%d]", mat->fd, rtn);
				return -1;
			}

			LogDel( "pipe read ... pos=[%d]", pos);
			return pos;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  주문 insert,update,delete
***************************************************************************** */
int Mat_Order( MAT *mat, ORDER *obook, int opt)
{
	int			rtn;
	int			i;
	int			pos;
	int 		base_cur;
	int			cont_cur;
	MAT_INDEX	*index;

	Mat_StatisticsSet( mat, &mat->map->stat.stat.rcv, MAT_COUNT);
	Mat_StatisticsSet( mat, &mat->map->stat.stat.ord, MAT_START);
	Mat_StatisticsSet( mat, &mat->map->stat.stat.cnf, MAT_START);

	/* 기준통화 */
	base_cur = Mat_GetCurrentInt( mat, &obook->Symbol[ 0]);
	if( base_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. base_cur=[%.3s]", &obook->Symbol);
		return -1;
	}

	/* 상대통화 */
	cont_cur = Mat_GetCurrentInt( mat, &obook->Symbol[ 4]);
	if( cont_cur < 0)
	{
		LogMsg( "통화정보가 올바르지 않습니다. cont_cur=[%.3s]", &obook->Symbol[ 4]);
		return -1;
	}

	/* search index */
	index = Mat_GetIndex( mat, base_cur, cont_cur);
	if( index == NULL)
	{
		LogMsg( "거래 가능한 통화가 아닙니다. 기준통화=[%d] 상대통화=[%d]", base_cur, cont_cur);
		return -1;
	}

	Mat_StatisticsSet( mat, &mat->map->stat.stat.cnf, MAT_END);
	LogDel( "opt=[%d] base=[%s:%d] cont=[%s:%d] price=[%12f]", 
			opt, MatCurrent[ base_cur], base_cur, MatCurrent[ cont_cur], cont_cur, obook->Price);

	switch( opt)
	{
		case MAT_INSERT:
			pos = Mat_RecordInsert( mat, index, obook);
			break;
		case MAT_UPDATE:
			pos = Mat_RecordDelete( mat, index, obook);
			if( pos < 0)
			{
				LogMsg( "Mat_RecordDelete error. pos=[%d]", pos);
				return -1;
			}
			pos = Mat_RecordInsert( mat, index, obook);
			break;
		case MAT_DELETE:
			pos = Mat_RecordDelete( mat, index, obook);
			break;
		default:
			LogMsg( "unknown option. opt=[%d]", opt);
			break;
	}

	Mat_StatisticsSet( mat, &mat->map->stat.stat.ord, MAT_END);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 insert 
**	무조건 첫번째에 주문 레코드 삽입
**	추후 index가 확정 되면 key에 따라 해당 위치에 삽입
***************************************************************************** */
int Mat_RecordInsert( MAT *mat, MAT_INDEX *index, ORDER	 *obook)
{
	int			pos, npos;
	int			side;
	MAT_HEAD	*head, *nhead;
	MAT_RECORD	*rec, *nrec;

	/* 빈레코드를 찾아 가져올때부터 lock을 해야 두개의 process가 떴을때도 중복이 안생김 */
	Mat_Lock( mat);

	pos = Mat_GetEmptyRecordPos( mat);
	if( pos < 0)
	{
		LogMsg( "Mat_GetEmptyRecordPos error.");
		goto error;
	}

	/* 공유메모리 index 테이블 매수(0) 매도(1)로 주문 side 매수('1') 매도('2')를 변환 */
	side = ( int)(obook->Side[0] - '1');
	if( side < 0 || side > 1)
	{
		LogCri( "매수/매도 구분 오류 ... side=[%d]", side);
		return -1;
	}

	rec = &mat->map->rec[ pos];
	rec->pos = pos;
	head = &rec->head;
	memcpy( ( char *)&rec->book, obook, sizeof( ORDER	));

	/* 최초 record */
	if( index->start[ side] == -1)
	{
		index->start[ side] = pos;
		head->prev = -1;
		head->next = -1;
		index->cnt[ side]++;
		head->gubun = 1;
		mat->map->stat.rec_cnt++;

		Mat_MakeHead( mat, head, obook);

		goto success;
	}

	/* start record get */
	npos = index->start[ side];
	nrec = &mat->map->rec[ npos];

	/* insert current record at first */
	nhead = &nrec->head;
	nhead->prev = pos;
	index->start[ side] = pos;
	head->prev = -1;
	head->next = npos;
	head->gubun = 1;
	index->cnt[ side]++;
	mat->map->stat.rec_cnt++;

	Mat_MakeHead( mat, head, obook);

	success:
		Mat_Unlock( mat);
		return pos;

	error:
		Mat_Unlock( mat);
		return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 delete 
***************************************************************************** */
int Mat_RecordDelete( MAT *mat, MAT_INDEX *index, ORDER	 *obook)
{
	int			pos, ppos, npos;
	int			side;
	MAT_HEAD	*head, *nhead;
	MAT_RECORD	*rec;
	MAT_RECORD	*prec;	/* prev record */
	MAT_RECORD	*nrec;	/* next record */

	Mat_Lock( mat);

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	/* 삭제할 주문 찾기 */
	pos = Mat_FindRecordByOrigClOrdID( mat, index, obook);
	if( pos < 0)
	{
		LogMsg( "원주문이 없습니다. OrigClOrdID=[%.*s]", sizeof( obook->OrigClOrdID), obook->OrigClOrdID);
		goto error;
	}

	/* record delete */
	rec  = &mat->map->rec[ pos];
	head = &rec->head;
	ppos = head->prev;
	npos = head->next;
	if( ppos >= 0)		/* 처음이 아니면 */
	{
		prec = &mat->map->rec[ head->prev];
		prec->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		index->start[ side] = npos;
	}
	if( npos >= 0)		/* 마지막이 아니면 */
	{
		nrec = &mat->map->rec[ head->next];
		nrec->head.prev = ppos;
	}

	/* record update - gubun을 0으로 setting하여 빈레코드로 표시 */
	rec->head.gubun = 0;
	/* index에 record 갯수를 1 감소 */
	index->cnt[ side]--;
	mat->map->stat.rec_cnt--;
	memset( rec, 0x00, sizeof( MAT_RECORD));

	success:
		Mat_Unlock( mat);
		return pos;

	error:
		Mat_Unlock( mat);
		return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     prev record position
**  @brief
**  record를 list에서 삭제
***************************************************************************** */
int Mat_RecordUnlink( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side)
{
	int			ppos, npos;
	MAT_HEAD	*head;
	MAT_RECORD	*prec;	/* prev record */
	MAT_RECORD	*nrec;	/* next record */

	/* record delete */
	head = &rec->head;
	ppos = head->prev;
	npos = head->next;

	if( ppos >= 0)		/* 처음이 아니면 */
	{
		prec = &mat->map->rec[ head->prev];
		prec->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		index->start[ side] = npos;
	}

	if( npos >= 0)		/* 마지막이 아니면 */
	{
		nrec = &mat->map->rec[ head->next];
		nrec->head.prev = ppos;
	}

	/* index에 record 갯수를 1 감소 */
	index->cnt[ side]--;
	mat->map->stat.rec_cnt--;

	return ppos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 주문 찾기
***************************************************************************** */
int Mat_FindRecordByPos( MAT *mat, MAT_INDEX *index, ORDER *obook, int find_pos)
{
	int			side;
	int			pos;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	/* 임시로 update 필드로 주문을 찾는다 ... update할 position으로 테스트 */

	pos = index->start[ side];
	while( pos >= 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		if( find_pos == pos) 
		{
			return pos;
		}
		head = &rec->head;
		pos = head->next;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 주문 찾기
***************************************************************************** */
int Mat_FindRecordByOrigClOrdID( MAT *mat, MAT_INDEX *index, ORDER *obook)
{
	int			side;
	int			pos, find_pos;
	char		OrigClOrdID[ 24 +1];
	MAT_RECORD	*rec;
	ORDER		*order;
	MAT_HEAD	*head;

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	LogDbg( "find order ----------------------");
	LogDbg( "           side           =[%d]", side);
	LogDbg( "           OrigClOrdID    =[%.*s]", sizeof( obook->OrigClOrdID), obook->OrigClOrdID);

	/* 임시로 update 필드로 주문을 찾는다 ... update할 position으로 테스트 */
	memcpy( OrigClOrdID, obook->OrigClOrdID, sizeof( obook->OrigClOrdID));
	OrigClOrdID[ sizeof( OrigClOrdID)] = 0;

	pos = index->start[ side];
	while( pos >= 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		order = &rec->book;
		LogDbg( "           ClOrdID        =[%.*s]", sizeof( order->ClOrdID), order->ClOrdID);
		if( !memcmp( order->ClOrdID, OrigClOrdID, sizeof( order->ClOrdID)))
		{
			LogDbg( "find ClOrdID ... pos=[%d]", pos);
			return pos;
		}
		head = &rec->head;
		pos = head->next;
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  주문 record에서 필요한 data를 head에 set
***************************************************************************** */
int Mat_MakeHead( MAT *mat, MAT_HEAD *head, ORDER	 *obook)
{
	head->price = obook->Price;

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  conform record에 save
***************************************************************************** */
int Mat_PutConform( MAT *mat, int pos)
{
	int				i;
	MAT_HEAD		*head;

	Mat_Lock( mat);
	for( i = 0; i < MAT_MAX_CONFORM; i++)
	{
		if( mat->map->conform[ i] < 0)
		{
			mat->map->conform[ i] = pos;
			head = &mat->map->rec[ pos].head;
			gettimeofday( &head->dly_time, NULL);
			mat->map->conform_cnt++;
			LogMsg( "주문확인 대기 ... ");
			LogMsg( "    pos=[%d] cnt=[%d]", pos, mat->map->conform_cnt);
			LogMsg( "    dly_time=[%s]", TtoS( head->dly_time.tv_sec));
			Mat_Unlock( mat);
			return i;
		}
	}
	Mat_Unlock( mat);

	LogCri( "주문확인 대기열에 빈공간이 없습니다. conform_cnt=[%d]", mat->map->conform_cnt);
	sleep( 1);
	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  conform record에서 get
***************************************************************************** */
int Mat_GetConform( MAT *mat)
{
	int				i, pos;
	struct timeval	tv;
	MAT_HEAD		*head;

	if( mat->map->conform_cnt <= 0) return -1;

	Mat_Lock( mat);
	gettimeofday( &tv, NULL);

	for( i = 0; i < MAT_MAX_CONFORM; i++)
	{
		if( mat->map->conform[ i] >= 0)
		{
			pos = mat->map->conform[ i];
			head = &mat->map->rec[ pos].head;
			if( tv.tv_sec - head->dly_time.tv_sec >= MAT_CONFORM_TIME) 
			{
				if( head->con_time.tv_sec == 0) 
				{
					LogMsg( "주문확인 체크 ... 대기");
					LogMsg( "    pos=[%d] cnt=[%d]", pos, mat->map->conform_cnt);
					LogMsg( "    con_time=[%s]", TtoS(head->con_time.tv_sec));
					LogMsg( "    dly_time=[%s]", TtoS(head->dly_time.tv_sec));
					if( tv.tv_sec - head->dly_time.tv_sec > MAT_CONFORM_TIMEOUT)
					{
						head->con_time.tv_sec = tv.tv_sec + 600; /* 주문확인이 안들어 왔어도 강제 체결 처리 */
						mat->map->conform[ i] = -1;
						mat->map->conform_cnt--;
						head->con_time.tv_sec = tv.tv_sec;
						LogMsg( "강제 체결처리  ... pos=[%d] cnt=[%d]", pos, mat->map->conform_cnt);
						Mat_Unlock( mat);
						return pos;
					}
					continue;
				}
				else
				{
					LogMsg( "주문확인 대기 해소. 체결처리..."); 
					LogMsg( "    pos=[%d] cnt=[%d] time=[%s]", pos, mat->map->conform_cnt, TtoS( head->con_time.tv_sec));
					LogMsg( "    con_time=[%s]", TtoS(head->con_time.tv_sec));
					LogMsg( "    dly_time=[%s]", TtoS(head->dly_time.tv_sec));
					mat->map->conform[ i] = -1;
					mat->map->conform_cnt--;
					Mat_Unlock( mat);
					return pos;
				}
			}
		}
	}
	Mat_Unlock( mat);
	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  빈 record 찾기
***************************************************************************** */
int Mat_GetEmptyRecordPos( MAT *mat)
{
	int		pos;
	int		cnt = 0;

	pos = mat->map->stat.wpos;

	while( Continue)
	{
		if( mat->map->rec[ pos].head.gubun == 0) 
		{
			mat->map->stat.wpos = pos +1;
			if( mat->map->stat.wpos >= MAT_MAX_RECORD)	mat->map->stat.wpos = 0;
			return pos;
		}
		pos++;
		if( pos >= MAT_MAX_RECORD)	pos = 0;
		cnt++;
		if( cnt >= MAT_MAX_RECORD)
		{
			LogMsg( "빈 record가 없습니다. Unlock/Lock을 수행합니다.");
			/* 빈 record가 생길려면 다른 프로세스가 처리 해야 하므로 unlock */
			Mat_Unlock( mat);
			sleep( 1);
			Mat_Lock( mat);
			continue;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  position을 입력하여 record 찾기
***************************************************************************** */
MAT_RECORD *Mat_GetRecordByPos( MAT *mat, int pos)
{
	return &mat->map->rec[ pos];
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  통화 string을 int로 환산
***************************************************************************** */
int Mat_GetCurrentInt( MAT *mat, char *current)
{
	int	i;

	for( i = 0; MatCurrent[ i][0] != 0; i++)
	{
		if( !memcmp( current, MatCurrent[ i], 3))
		{
			return i;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  get first/last/prev/next record pointer
**  first,last = from index
**	prev,next  = from record
***************************************************************************** */
MAT_RECORD* Mat_GetRecord( MAT *mat, MAT_RECORD *rec, int opt)
{
	int			pos;
	int			base, cont;
	int			side;
	MAT_HEAD	*head;
	MAT_INDEX	*index;

	head = &rec->head;

	switch( opt)
	{
		case MAT_FIRST:
			base = Mat_GetCurrentInt( mat, &rec->book.Symbol[ 0]);
			cont = Mat_GetCurrentInt( mat, &rec->book.Symbol[ 4]);
			index = Mat_GetIndex( mat, base, cont);
			if( index == NULL)
			{
				LogMsg( "해당 통화 거래가 존재하지 않습니다. current=[%.7s]", &rec->book.Symbol);
				return NULL;
			}
			side = rec->book.Side[ 0] - '1';
			LogDel( "side=[%d]", side);
			pos = index->start[ side];
			break;
		case MAT_LAST:
			break;
			
		case MAT_PREV:
			pos = head->prev;
			if( pos < 0)
			{
				LogMsg( "처음 레코드 입니다.");
				return NULL;
			}
			break;
		case MAT_NEXT:
			pos = head->next;
			if( pos < 0)
			{
				LogMsg( "마지막 레코드 입니다.");
				return NULL;
			}
			break;
		default:
			LogMsg( "unknown option ... opt=[%d]", opt);
			return NULL;
	}

	LogDel( "pos=[%d]", pos);
	if( pos < 0) return NULL;
	return &mat->map->rec[ pos];
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  기준/상대 통화 index 찾기
***************************************************************************** */
MAT_INDEX* Mat_GetIndex( MAT *mat, int base, int cont)
{
	int			i;
	MAT_INDEX	*index;

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		if( base == mat->map->stat.index[ i].base_cur && cont == mat->map->stat.index[ i].cont_cur)
			return &mat->map->stat.index[ i];
	}

	return NULL;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 수행
***************************************************************************** */
int Mat_Lock( MAT *mat)
{
	int		rtn;

	rtn = Sem_Lock( mat->sem);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 해제
***************************************************************************** */
int Mat_Unlock( MAT *mat)
{
	int		rtn;

	rtn = Sem_Unlock( mat->sem);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 setting
**	MAT_MAX_GAP 이상의 통계는 제외
***************************************************************************** */
int Mat_StatisticsSet( MAT *mat, STEL *stel, int opt)
{
	int				rtn, gap;

	switch( opt)
	{
		case MAT_COUNT:
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			return 1;

		case MAT_START:		/* 측정 시작 */
			gettimeofday( &stel->srt, NULL);
			return 1;

		case MAT_END:		/* 측정 종료 */
			if( stel->min <= 0 && stel->cnt == 0) stel->min = 999999999;
			gettimeofday( &stel->end, NULL);
			stel->cnt++;
			break;
	}

	/* 통계 계산 */
	gap = Mat_TimeGap( mat, &stel->srt, &stel->end);
	if( gap < 0)			return 0;
	if( gap > MAT_MAX_GAP)	return 0;
	stel->cur  = gap;
	stel->max  = MAX( stel->max, gap);
	stel->min  = MIN( stel->min, gap);
	stel->tot += gap;
	stel->avr  = stel->tot / stel->cnt;


	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  시간 차이 구하기
***************************************************************************** */
int Mat_TimeGap( MAT *mat, struct timeval *tv_1, struct timeval *tv_2)
{
	int		int_gap;
	time_t	sec_gap;
	time_t	usec_gap;

	sec_gap = tv_2->tv_sec - tv_1->tv_sec;
	usec_gap = tv_2->tv_usec - tv_1->tv_usec;

	if( usec_gap < 0)
	{
		sec_gap--;
		usec_gap = 1000000 + usec_gap;
	}

	int_gap = sec_gap * 1000000 + usec_gap;

	return int_gap;
}



/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 초기화
***************************************************************************** */
int Mat_StatisReset( MAT *mat)
{
	MAT_STATIS	*stat;

	Mat_Lock( mat);
	stat = &mat->map->stat.stat;
	memset( stat, 0x00, sizeof( MAT_STATIS));
	Mat_Unlock( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  stat 출력
***************************************************************************** */
int Mat_Stat( MAT *mat)
{
	int			i;
	int			rtn;
	int			cols = 80;

	MAT_INDEX	*index;

	printf( "mat                  = [%p]\n", mat);
	printf( "key                  = [0x%08x]\n", mat->key);
	printf( "mem                  = [%p]\n", mat->mem);
	printf( "mem->id              = [%d]\n", mat->mem->id);
	printf( "sem                  = [%p]\n", mat->sem);
	printf( "sem->id              = [%d]\n", mat->sem->id);
	printf( "fd(fifo)             = [%d]\n", mat->fd);
	printf( "base(mat->map)       = [%p]\n", ( char *)mat->map);
	printf( "---------------------------------------------\n");
	printf( "ctime                = [%s]\n", TtoS( mat->map->stat.ctime));
	printf( "rec_cnt              = [%d]\n", mat->map->stat.rec_cnt);
	printf( "exe_cnt              = [%d]\n", mat->map->stat.exe_cnt);
	printf( "wpos                 = [%d]\n", mat->map->stat.wpos);
	printf( "key                  = [%d]\n", mat->map->stat.key);
	printf( "pipe_name            = [%s]\n", mat->map->stat.pipe_name);
	printf( "---------------------------------------------\n");

	printf( "기준통화 ");
	printf( "상대통화 ");
	printf( "pips단위 ");
	printf( "매수시작 ");
	printf( "갯수 ");
	printf( "매도시작 ");
	printf( "갯수 ");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->stat.index[i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		printf( "%-8s ", MatCurrent[ index->base_cur]);
		printf( "%-8s ", MatCurrent[ index->cont_cur]);
		printf( "%d ",	index->point);
		printf( "%8d ", index->start[ 0]);
		printf( "%4d ", index->cnt[ 0]);
		printf( "%8d ", index->start[ 1]);
		printf( "%4d ", index->cnt[ 1]);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	Mat_StatIndex( mat);

	Mat_StatOrder( mat);
	Mat_StatExecute( mat);
	Mat_PrintStic( mat);
	Mat_PrintConform( mat);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index stat 출력
***************************************************************************** */
int Mat_StatIndex( MAT *mat)
{
	int			i, j;
	int			rtn;
	int			cols = 80;

	MAT_INDEX	*index;
	MAT_RECORD	*rec;
	int			pos;
	int			side;

	printf( "기준통화 ");
	printf( "상대통화 ");
	printf( "  pos ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	/* 통화 index */
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->stat.index[i];
		if( index->base_cur == 0 && index->cont_cur == 0) continue;
		printf( "%-3s(%d)   ", MatCurrent[ index->base_cur], index->base_cur);
		printf( "%-3s(%d)   ", MatCurrent[ index->cont_cur], index->cont_cur);
		printf( "\n");

		/* 매수/매도 */
		for( j = 0; j < 2; j++)
		{
			printf( "                  ");
			if( j == 0)	printf( "[매수]\n");
			else		printf( "[매도]\n");

			pos = index->start[ j];
			while( pos >= 0)	/* 끝 == -1 */
			{
				rec = &mat->map->rec[ pos];
				printf( "                  ");
				printf( "%5d ", pos);
				printf( "%15f ", rec->head.price);
				printf( "\n");

				pos = rec->head.next;
			}
		}
	}
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");



	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  공유메모리에 남아 있는 주문 출력
***************************************************************************** */
int Mat_StatOrder( MAT *mat)
{
	int			i;
	int			rtn;
	int			cols = 80;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[주문] rec_cnt    = [%d]\n", mat->map->stat.rec_cnt);
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	printf( " pos  ");
	printf( "bas ");
	printf( "con ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	for( i = 0; i < MAT_MAX_RECORD; i++)
	{
		rec   = &mat->map->rec[ i];
		head  = &rec->head;
		obook = &rec->book;
		if( head->gubun != 1) continue;
		printf( "%5d ", i);
		printf( "%.3s ", &obook->Symbol[ 0]);
		printf( "%.3s ", &obook->Symbol[ 4]);
		printf( "%15f ", rec->head.price);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  공유메모리에 남아 있는 체결 출력
***************************************************************************** */
int Mat_StatExecute( MAT *mat)
{
	int			i;
	int			rtn;
	int			cols = 80;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*obook;

	printf( "[체결] exe_cnt    = [%d]\n", mat->map->stat.exe_cnt);
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	printf( " pos  ");
	printf( "bas ");
	printf( "con ");
	printf( "%15s ", "price");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	for( i = 0; i < MAT_MAX_RECORD; i++)
	{
		rec   = &mat->map->rec[ i];
		head  = &rec->head;
		obook = &rec->book;
		if( head->gubun != 2) continue;
		printf( "%5d ", i);
		printf( "%.3s ", &obook->Symbol[ 0]);
		printf( "%.3s ", &obook->Symbol[ 4]);
		printf( "%15f ", rec->head.price);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  한개의 index table 출력
***************************************************************************** */
int Mat_PrintIndex( MAT *mat, MAT_INDEX *index)
{
	printf( "기준통화             = [%s][%d]\n", MatCurrent[ index->base_cur], index->base_cur);
	printf( "상대통화             = [%s][%d]\n", MatCurrent[ index->cont_cur], index->cont_cur);
	printf( "단위                 = [%d]\n",     index->point);
	printf( "매수시작위치         = [%d]\n",     index->start[ 0]);
	printf( "매수주문수량         = [%d]\n",     index->cnt[ 0]);
	printf( "매도시작위치         = [%d]\n",     index->start[ 1]);
	printf( "매도주문수량         = [%d]\n",     index->cnt[ 1]);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 출력
***************************************************************************** */
int Mat_PrintStic( MAT *mat)
{
	int			i;
	int			cols = 80;
	char		*gubun[ 20] = { "수신", "거부", "접수", "주문", "체결", "전송", "시세", NULL, NULL};

	MAT_STATIS	*stat;
	STEL		*stel;

	for( i = 0; i < cols; i++) printf( "-"); printf( "\n");
	printf( "%6s ", "[구분]");
	printf( "%7s ", "count");
	printf( "%7s ", "  max");
	printf( "%7s ", "  min");
	printf( "%7s ", "  tot");
	printf( "%7s ", "  avr");
	printf( "\n");

	stat = &mat->map->stat.stat;
	stel = &stat->rcv;

	for( i = 0; gubun[ i] != NULL; i++)
	{
		printf( " %.4s  ", gubun[ i]);
		printf( "%7d ", stel->cnt);
		printf( "%7d ", stel->max);
		printf( "%7d ", stel->min);
		printf( "%7d ", stel->tot);
		printf( "%7d ", stel->avr);
		printf( "\n");
		stel++;
	}

#if 0
	stel = &stat->ord;
	printf( " 주문  ");
	printf( "%5d ", stel->cnt);
	printf( "%5d ", stel->max);
	printf( "%5d ", stel->min);
	printf( "%5d ", stel->tot);
	printf( "%5d ", stel->avr);
	printf( "\n");

	stel = &stat->sis;
	printf( " 매칭  ");
	printf( "%5d ", stel->cnt);
	printf( "%5d ", stel->max);
	printf( "%5d ", stel->min);
	printf( "%5d ", stel->tot);
	printf( "%5d ", stel->avr);
	printf( "\n");
#endif

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 출력
***************************************************************************** */
int Mat_PrintConform( MAT *mat)
{
	int			i;

	printf( "conform cnt=[%d]\n", mat->map->conform_cnt);
	for( i = 0; i < MAT_MAX_CONFORM; i++)
	{
		if( mat->map->conform[ i] < 0) continue;
		printf( "i=[%2d] pos=[%7d]\n", i, mat->map->conform[ i]);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  통계 출력
***************************************************************************** */
int Mat_PrintRecord( MAT *mat, int pos)
{
	int			i;

	ORDER		*order;

	order = &mat->map->rec[ pos].book;
	MAT_ORDER_Print( order);

	return 1;
}

int MAT_ORDER_RECV_Print( ORDER_RECV* ptr)
{
    LogRaw( "%s", "----[ ORDER_RECV ]----------------------------------------------------------------------\n");
    LogRaw( "주문타입                      MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "고객번호 (내부사용자ID)       CustID                30    1 = [%.30s]\n", 	ptr->CustID);
    LogRaw( "원천구분                      OrgnGb                 1   31 = [%.1s]\n", 	ptr->OrgnGb);
    LogRaw( "북번호                        BkNo                  10   32 = [%.10s]\n", 	ptr->BkNo);
    LogRaw( "주문번호                      ClOrdID               24   42 = [%.24s]\n", 	ptr->ClOrdID);
    LogRaw( "원주문번호                    OrigClOrdID           24   66 = [%.24s]\n", 	ptr->OrigClOrdID);
    LogRaw( "기준통화코드(정보성)          Currency               3   90 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문수량                      OrderQty              20   93 = [%.20s]\n", 	ptr->OrderQty);
    LogRaw( "주문유형                      OrdType                1  113 = [%.1s]\n", 	ptr->OrdType);
    LogRaw( "주문가격                      Price                 20  114 = [%.20s]\n", 	ptr->Price);
    LogRaw( "주문시점가격(시장가의 경우)   SlipCmpPrice          20  134 = [%.20s]\n", 	ptr->SlipCmpPrice);
    LogRaw( "Slipage Pip(가격이격:체결범�  SlipPip               20  154 = [%.20s]\n", 	ptr->SlipPip);
    LogRaw( "매매구분                      Side                   1  174 = [%.1s]\n", 	ptr->Side);
    LogRaw( "주문유효시간                  TimeInForce            1  175 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "처리시각                      TransactTime          20  176 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               1  196 = [%.1s]\n", 	ptr->SettType);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  197 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  204 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  212 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "근일물상품구분코드            NearSettType           1  220 = [%.1s]\n", 	ptr->NearSettType);
    LogRaw( "NEAR매매구분(1-Buy,2-Sell)    NearLegSide            1  221 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-결제일자                 NearLegSettlDate       8  222 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       NearLegPrice          20  230 = [%.20s]\n", 	ptr->NearLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        NearLegPriceSprd      20  250 = [%.20s]\n", 	ptr->NearLegPriceSprd);
    LogRaw( "원일물상품구분코드            FarSettType            1  270 = [%.1s]\n", 	ptr->FarSettType);
    LogRaw( "FAR매매구분(1-Buy, 2-Sell)    FarLegSide             1  271 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-결제일자                  FarLegSettlDate        8  272 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       FarLegPrice           20  280 = [%.20s]\n", 	ptr->FarLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        FarLegPriceSprd       20  300 = [%.20s]\n", 	ptr->FarLegPriceSprd);
    LogRaw( "거래유형코드                  tran_ptrncd            1  320 = [%.1s]\n", 	ptr->tran_ptrncd);
    LogRaw( "Filler                        Filler               647  321 = [%.647s]\n", ptr->Filler);
    LogRaw( "EOF                           Eof                    1  968 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "----------------------------------------------------------------------[ ORDER_RECV ]----\n");

    return sizeof( ORDER_RECV);
}

int MAT_ORDER_SEND_Print( ORDER_SEND* ptr)
{
    LogRaw( "%s", "----[ ORDER_SEND ]----------------------------------------------------------------------");
    LogRaw( "메세지유형                    MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "주문상태                      OrdStatus              1    1 = [%.1s]\n", 	ptr->OrdStatus);
    LogRaw( "거래유형                      ExecType               1    2 = [%.1s]\n", 	ptr->ExecType);
    LogRaw( "시장참여자ID및트레이더번호    LgenNo                30    3 = [%.30s]\n", 	ptr->LgenNo);
    LogRaw( "북번호                        BkNo                  10   33 = [%.10s]\n", 	ptr->BkNo);
    LogRaw( "회원처리항목1                 ClOrdID               24   43 = [%.24s]\n", 	ptr->ClOrdID);
    LogRaw( "회원처리항목2                 OrigClOrdID           24   67 = [%.24s]\n", 	ptr->OrigClOrdID);
    LogRaw( "통화코드                      Currency               3   91 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문수량                      OrderQty              20   94 = [%.20s]\n", 	ptr->OrderQty);
    LogRaw( "누적체결수량                  CumQty                20  114 = [%.20s]\n", 	ptr->CumQty);
    LogRaw( "체결수량                      LastQty               20  134 = [%.20s]\n", 	ptr->LastQty);
    LogRaw( "주문잔여수량                  LeavesQty             20  154 = [%.20s]\n", 	ptr->LeavesQty);
    LogRaw( "주문번호ID                    OrdID                 30  174 = [%.30s]\n", 	ptr->OrdID);
    LogRaw( "체결ID                        ExecID                30  204 = [%.30s]\n", 	ptr->ExecID);
    LogRaw( "주문가격                      Price                 20  234 = [%.20s]\n", 	ptr->Price);
    LogRaw( "주문고객마진                  LastPx1               20  254 = [%.20s]\n", 	ptr->LastPx1);
    LogRaw( "체결가격                      LastPx2               20  274 = [%.20s]\n", 	ptr->LastPx2);
    LogRaw( "체결고객마진                  LastPx3               20  294 = [%.20s]\n", 	ptr->LastPx3);
    LogRaw( "매매구분                      Side                   1  314 = [%.1s]\n", 	ptr->Side);
    LogRaw( "체결조건                      TimeInForce            1  315 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "거부코드                      RefuslCd              10  316 = [%.10s]\n", 	ptr->RefuslCd);
    LogRaw( "내용                          Text                 200  326 = [%.200s]\n", ptr->Text);
    LogRaw( "주문일시                      TransactTime          20  526 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               1  546 = [%.1s]\n", 	ptr->SettType);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  547 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  554 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  562 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "거래호가번호                  OfprKeyVal            30  570 = [%.30s]\n", 	ptr->OfprKeyVal);
    LogRaw( "NEAR-레크상품구분코드         NearSettType           1  600 = [%.1s]\n", 	ptr->NearSettType);
    LogRaw( "NEAR-레그매수매도구분코드     NearLegSide            1  601 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-레그결제년월일           NearLegSettlDate       8  602 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "NEAR-레그체결가격             NearLegPrice1         20  610 = [%.20s]\n", 	ptr->NearLegPrice1);
    LogRaw( "NEAR-CV스프레드               NearLegPrice2         20  630 = [%.20s]\n", 	ptr->NearLegPrice2);
    LogRaw( "NEAR-CO스프레드               NearLegPrice3         20  650 = [%.20s]\n", 	ptr->NearLegPrice3);
    LogRaw( "NEAR-레그체결가격스프레드     NearLegPriceSprd      20  670 = [%.20s]\n", 	ptr->NearLegPriceSprd);
    LogRaw( "FAR-레크상품구분코드          FarSettType            1  690 = [%.1s]\n", 	ptr->FarSettType);
    LogRaw( "FAR-레그매수매도구분코드      FarLegSide             1  691 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-레그결제년월일            FarLegSettlDate        8  692 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FAR-레그체결가격              FarLegPrice1          20  700 = [%.20s]\n", 	ptr->FarLegPrice1);
    LogRaw( "FAR-CV스프레드                FarLegPrice2          20  720 = [%.20s]\n", 	ptr->FarLegPrice2);
    LogRaw( "FAR-CO스프레드                FarLegPrice3          20  740 = [%.20s]\n", 	ptr->FarLegPrice3);
    LogRaw( "FAR-레그체결가격스프레드      FarLegPriceSprd       20  760 = [%.20s]\n", 	ptr->FarLegPriceSprd);
    LogRaw( "Filler                        filler                 1  780 = [%.1s]\n", 	ptr->filler);
    LogRaw( "0x00                          Eof                    1  781 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "----------------------------------------------------------------------[ ORDER_SEND ]----");

    return sizeof( ORDER_SEND);
}

int MAT_ORDER_Print( ORDER* ptr)
{
    LogRaw( "%s", "----[ ORDER ]---------------------------------------------------------------------------\n");
    LogRaw( "메세지유형                    MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "주문상태 @                    OrdStatus              1    1 = [%.1s]\n", 	ptr->OrdStatus);
    LogRaw( "거래유형 @                    ExecType               1    2 = [%.1s]\n", 	ptr->ExecType);
    LogRaw( "시장참여자ID및트레이더번호    LgenNo                30    3 = [%.30s]\n", 	ptr->LgenNo);
    LogRaw( "북번호                        BkNo                  10   33 = [%.10s]\n", 	ptr->BkNo);
    LogRaw( "회원처리항목1                 ClOrdID               24   43 = [%.24s]\n", 	ptr->ClOrdID);
    LogRaw( "회원처리항목2                 OrigClOrdID           24   67 = [%.24s]\n", 	ptr->OrigClOrdID);
    LogRaw( "통화코드                      Currency               3   91 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문수량                      OrderQty               8   94 = [%f]\n", 	ptr->OrderQty);
    LogRaw( "누적체결수량                  CumQty                 8  102 = [%f]\n", 	ptr->CumQty);
    LogRaw( "체결수량                      LastQty                8  110 = [%f]\n", 	ptr->LastQty);
    LogRaw( "주문잔여수량                  LeavesQty              8  118 = [%f]\n", 	ptr->LeavesQty);
    LogRaw( "주문번호ID                    OrdID                 30  126 = [%.30s]\n", 	ptr->OrdID);
    LogRaw( "체결ID                        ExecID                30  156 = [%.30s]\n", 	ptr->ExecID);
    LogRaw( "주문가격                      Price                  8  186 = [%f]\n", 	ptr->Price);
    LogRaw( "주문고객마진                  LastPx1                8  194 = [%f]\n", 	ptr->LastPx1);
    LogRaw( "체결가격                      LastPx2                8  202 = [%f]\n", 	ptr->LastPx2);
    LogRaw( "체결고객마진                  LastPx3                8  210 = [%f]\n", 	ptr->LastPx3);
    LogRaw( "매매구분                      Side                   1  218 = [%.1s]\n", 	ptr->Side);
    LogRaw( "체결조건                      TimeInForce            1  219 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "거부코드                      RefuslCd              10  220 = [%.10s]\n", 	ptr->RefuslCd);
    LogRaw( "내용                          Text                 200  230 = [%.200s]\n", ptr->Text);
    LogRaw( "주문일시                      TransactTime          20  430 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               1  450 = [%.1s]\n", 	ptr->SettType);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  451 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  458 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  466 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "거래호가번호                  OfprKeyVal            30  474 = [%.30s]\n", 	ptr->OfprKeyVal);
    LogRaw( "NEAR-레크상품구분코드         NearSettType           1  504 = [%.1s]\n", 	ptr->NearSettType);
    LogRaw( "NEAR-레그매수매도구분코드     NearLegSide            1  505 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-레그결제년월일           NearLegSettlDate       8  506 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "NEAR-레그체결가격             NearLegPrice1          8  514 = [%f]\n", 	ptr->NearLegPrice1);
    LogRaw( "NEAR-CV스프레드               NearLegPrice2          8  522 = [%f]\n", 	ptr->NearLegPrice2);
    LogRaw( "NEAR-CO스프레드               NearLegPrice3          8  530 = [%f]\n", 	ptr->NearLegPrice3);
    LogRaw( "NEAR-레그체결가격스프레드     NearLegPriceSprd       8  538 = [%f]\n", 	ptr->NearLegPriceSprd);
    LogRaw( "FAR-레크상품구분코드          FarSettType            1  546 = [%.1s]\n", 	ptr->FarSettType);
    LogRaw( "FAR-레그매수매도구분코드      FarLegSide             1  547 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-레그결제년월일            FarLegSettlDate        8  548 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FAR-레그체결가격              FarLegPrice1           8  556 = [%f]\n", 	ptr->FarLegPrice1);
    LogRaw( "FAR-CV스프레드                FarLegPrice2           8  564 = [%f]\n", 	ptr->FarLegPrice2);
    LogRaw( "FAR-CO스프레드                FarLegPrice3           8  572 = [%f]\n", 	ptr->FarLegPrice3);
    LogRaw( "FAR-레그체결가격스프레드      FarLegPriceSprd        8  580 = [%f]\n", 	ptr->FarLegPriceSprd);
    LogRaw( "Filler                        filler                 1  588 = [%.1s]\n", 	ptr->filler);
    LogRaw( "0x00                          Eof                    1  589 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "---------------------------------------------------------------------------[ ORDER ]----\n");

    return sizeof( ORDER);
}

int MATSISE_Print( MATSISE* ptr)
{
    LogRaw( "%s", "----[ MATSISE ]-------------------------------------------------------------------------\n");
    LogRaw( "'S'MB/'K'MB/E'BS/'C'MB/'B'ES  excode                 1    0 = [%.1s]\n", 	ptr->excode);
    LogRaw( "BID원천 : 'S':SMB, 'K':KMB,   bidex                  1    1 = [%.1s]\n", 	ptr->bidex);
    LogRaw( "ASK원천 : 'S':SMB, 'K':KMB,   askex                  1    2 = [%.1s]\n", 	ptr->askex);
    LogRaw( "root symbol                   symb                   7    3 = [%.7s]\n", 	ptr->symb);
    LogRaw( "수신일자 YYYYMMDD (서버시간)  date                   8   10 = [%.8s]\n", 	ptr->date);
    LogRaw( "수신시간 HHMMSSSSS            time                   9   18 = [%.9s]\n", 	ptr->time);
    LogRaw( "Current USDKRW BID            usdbid                 8   27 = [%f]\n", 	ptr->usdbid);
    LogRaw( "Current USDKRW OFFER          usdask                 8   35 = [%f]\n", 	ptr->usdask);
    LogRaw( "Price of the MarketData Entr  bidprc                 8   43 = [%f]\n", 	ptr->bidprc);
    LogRaw( "Price of the MarketData Entr  askprc                 8   51 = [%f]\n", 	ptr->askprc);
    LogRaw( "Quantity of the MarketData E  bidqty                 8   59 = [%f]\n", 	ptr->bidqty);
    LogRaw( "Quantity of the MarketData E  askqty                 8   67 = [%f]\n", 	ptr->askqty);
    LogRaw( "%s", "-------------------------------------------------------------------------[ MATSISE ]----\n");

    return sizeof( MATSISE);
}
