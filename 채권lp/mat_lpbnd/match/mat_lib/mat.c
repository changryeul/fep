/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
**	mat.c			- 매칭 시스템 주요 모듈 생성/삭제/오픈/클로즈
**	mat_sise.c		- 매칭시스템 시세 처리
**	mat_stat.c		- 통계처리 상태출력
**	mat_order.c		- 주문 처리
**	mat_match.c		- 매칭 처리
**	mat_execute.c	- 체결 처리
**	mat_group.c		- 그룹주문 처리
**	convert.c		- 주문 구조체 변환
**	print.c			- 매칭관련 구조체 출력
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
	{ 82000,	"주문오류." },
	{ 82001,	"주문타입은 신규/정정/취소만 가능합니다." },
	{ 82003,	"원주문이 없습니다." },
	{ 82004,	"매매구분은 BUY/SELL만 가능합니다." },
	{ 82005,	"주문수량을 확인해주세요." },
	{ 82006,	"주문유형은 시장가/지정가/예약주문만 가능합니다." },
	{ 82007,	"주문 구분 미지원" },
	{ 82008,	"그룹주문 에러" },
	{ 82009,	"그룹주문번호 없음" },
	{ 82010,	"그룹주문번호 중복" },
	{ 82011,	"시세제공이 중단 되어 체결이 거부 됩니다. 관리자에게 연락 하세요!" },
	{ 82012,	"시장가격이 급변하여 체결이 거부됩니다." },
	{ 82020,	"상품구분 에러 [주문 구분 미지원]" },
	{ 82030,	"고객정보가 없습니다." },
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
	else
	{
		mat->map = Mem_GetPtr( mat->mem);
	}

	mat->sem = Sem_Create( mat->key);
	if( mat->sem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error;
	}

	/* create order pipe */
	sprintf( mat->map->stat.ord_pipe, "%s/%s", getenv( "MAT_DAT"), MAT_ORD_PIPE);
	LogDel( "ord_pipe=[%s]", mat->map->stat.ord_pipe);
	rtn = mkfifo( mat->map->stat.ord_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.ord_pipe);
	}

	mat->ord_fd = open( mat->map->stat.ord_pipe, O_RDWR);
	if( mat->ord_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.ord_pipe);
	}

	/* create execute pipe */
	sprintf( mat->map->stat.exe_pipe, "%s/%s", getenv( "MAT_DAT"), MAT_EXE_PIPE);
	LogDel( "exe_pipe=[%s]", mat->map->stat.exe_pipe);
	rtn = mkfifo( mat->map->stat.exe_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.exe_pipe);
	}

	mat->exe_fd = open( mat->map->stat.exe_pipe, O_RDWR);
	if( mat->exe_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.exe_pipe);
	}

	/* create matching pipe */
	sprintf( mat->map->stat.mat_pipe, "%s/%s", getenv( "MAT_DAT"), MAT_MAT_PIPE);
	LogDel( "mat_pipe=[%s]", mat->map->stat.mat_pipe);
	rtn = mkfifo( mat->map->stat.mat_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.mat_pipe);
	}

	mat->exe_fd = open( mat->map->stat.mat_pipe, O_RDWR);
	if( mat->exe_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.mat_pipe);
	}

	Mat_Init( mat);

	return mat;

	error:
		return NULL;
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

	/* order pipe create */
	sprintf( mat->map->stat.ord_pipe, "%s", MAT_ORD_PIPE);
	LogDel( "ord__pipe=[%s]", mat->map->stat.ord_pipe);
	rtn = mkfifo( mat->map->stat.ord_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s] ... 계속 진행 합니다.", mat->map->stat.ord_pipe);
		/* goto error_3; */
	}

	mat->ord_fd = open( mat->map->stat.ord_pipe, O_RDWR);
	if( mat->ord_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.ord_pipe);
		goto error_3;
	}

	/* execute pipe create */
	sprintf( mat->map->stat.exe_pipe, "%s", MAT_EXE_PIPE);
	LogDel( "exe_pipe=[%s]", mat->map->stat.exe_pipe);
	rtn = mkfifo( mat->map->stat.exe_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s] ... 계속 진행합니다.", mat->map->stat.exe_pipe);
		/* goto error_3; */
	}

	mat->exe_fd = open( mat->map->stat.exe_pipe, O_RDWR);
	if( mat->exe_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.exe_pipe);
		goto error_4;
	}

	/* create matching pipe */
	sprintf( mat->map->stat.mat_pipe, "%s", MAT_MAT_PIPE);
	LogDel( "mat_pipe=[%s]", mat->map->stat.mat_pipe);
	rtn = mkfifo( mat->map->stat.mat_pipe, 0644);
	if( rtn < 0)
	{
		LogErr( "mkfifo error. name=[%s]", mat->map->stat.mat_pipe);
	}

	mat->exe_fd = open( mat->map->stat.mat_pipe, O_RDWR);
	if( mat->exe_fd < 0)
	{
		LogErr( "pipe open error. name=[%s]", mat->map->stat.mat_pipe);
		goto error_5;
	}

	Mat_Init( mat);

	return mat;

	error_5:
		unlink( mat->map->stat.mat_pipe);
	error_4:
		unlink( mat->map->stat.exe_pipe);
	error_3:
		unlink( mat->map->stat.ord_pipe);
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

	close( mat->ord_fd);
	rtn = unlink( mat->map->stat.ord_pipe);
	if( rtn < 0)
	{
		LogErr( "pipe unlink error. name=[%s]", mat->map->stat.ord_pipe);
	}

	close( mat->exe_fd);
	rtn = unlink( mat->map->stat.exe_pipe);
	if( rtn < 0)
	{
		LogErr( "pipe unlink error. name=[%s]", mat->map->stat.exe_pipe);
	}

	close( mat->mat_fd);
	rtn = unlink( mat->map->stat.mat_pipe);
	if( rtn < 0)
	{
		LogErr( "pipe unlink error. name=[%s]", mat->map->stat.mat_pipe);
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
MAT* Mat_Open( int service)
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

	while( Continue && service == 0)
	{
		if( mat->map->stat.service) break;

		LogWar( "매칭 서비스 준비중 ... mat->map->stat.service=[%d]", mat->map->stat.service);
		sleep( 1);
	}

	mat->sem = Sem_Open( mat->key);
	if( mat->sem == NULL)
	{
		LogLib( "Sem_Open error.");
		goto error_2;
	}

	LogDel( "order pipe open ... name=[%s]", mat->map->stat.ord_pipe);
	mat->ord_fd = open( mat->map->stat.ord_pipe, O_RDWR);
	if( mat->ord_fd < 0)
	{
		LogDel( "ptr=[%p]", mat->map);
		LogErr( "pipe open error. name=[%s]", mat->map->stat.ord_pipe);
		goto error_3;
	}

	LogDel( "execute pipe open ... name=[%s]", mat->map->stat.exe_pipe);
	mat->exe_fd = open( mat->map->stat.exe_pipe, O_RDWR);
	if( mat->exe_fd < 0)
	{
		LogDel( "ptr=[%p]", mat->map);
		LogErr( "pipe open error. name=[%s]", mat->map->stat.exe_pipe);
		goto error_4;
	}

	LogDel( "matching pipe open ... name=[%s]", mat->map->stat.mat_pipe);
	mat->mat_fd = open( mat->map->stat.mat_pipe, O_RDWR);
	if( mat->mat_fd < 0)
	{
		LogDel( "ptr=[%p]", mat->map);
		LogErr( "pipe open error. name=[%s]", mat->map->stat.mat_pipe);
		goto error_5;
	}

	return mat;

	error_5:
		close( mat->exe_fd);
	error_4:
		close( mat->ord_fd);
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

	if( mat->mat_fd > 0)	close( mat->mat_fd);
	if( mat->exe_fd > 0)	close( mat->exe_fd);
	if( mat->ord_fd > 0)	close( mat->ord_fd);

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
**  공유메모리 변수들 초기화
***************************************************************************** */
int Mat_Init( MAT *mat)
{
	int			i;
	MAT_INDEX	*index;

	time( &mat->map->stat.ctime);
	mat->map->stat.max_rec = MAT_MAX_RECORD;
	mat->map->stat.wpos = 1;
	mat->map->grp_pos = 1;

	/* index initial */
	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		index = &mat->map->index[ i];
		index->no = i;					/* index number */
		index->base_cur = 0;			/* 기준통화 */
		index->cont_cur = 0;			/* 상대통화 */
		index->point = 0;				/* 소숫점 이하 자리 */
		index->unit = 0;				/* 통화단위 */
		index->start[ 0].start = -1;	/* 매수 start position */
		index->start[ 0].start_cnt = 0;	/* 매수 start position */
		index->start[ 0].wait = -1;		/* 매수 start position */
		index->start[ 0].wait_cnt = 0;	/* 매수 start position */
		index->start[ 1].start = -1;	/* 매도 start position */
		index->start[ 1].start_cnt = 0;	/* 매도 start position */
		index->start[ 1].wait = -1;		/* 매도 start position */
		index->start[ 1].wait_cnt = 0;	/* 매도 start position */
	}

#if 0
	Mat_AddIndex( mat, "USD/KRW", 2, 1);
	Mat_AddIndex( mat, "JPY/KRW", 2, 100);
	Mat_AddIndex( mat, "EUR/KRW", 2, 1);
	Mat_AddIndex( mat, "GBP/KRW", 2, 1);
	Mat_AddIndex( mat, "AUD/KRW", 2, 1);
	Mat_AddIndex( mat, "NZD/KRW", 2, 1);
	Mat_AddIndex( mat, "CAD/KRW", 2, 1);
	Mat_AddIndex( mat, "CHF/KRW", 2, 1);
	Mat_AddIndex( mat, "CNH/KRW", 2, 1);
	Mat_AddIndex( mat, "HKD/KRW", 2, 1);
	Mat_AddIndex( mat, "SGD/KRW", 2, 1);
	Mat_AddIndex( mat, "SEK/KRW", 2, 1);
	Mat_AddIndex( mat, "THB/KRW", 2, 1);
	Mat_AddIndex( mat, "CNY/KRW", 2, 1);

	Mat_AddIndex( mat, "EUR/USD", 5, 1);
	Mat_AddIndex( mat, "GBP/USD", 5, 1);
	Mat_AddIndex( mat, "AUD/USD", 5, 1);
	Mat_AddIndex( mat, "NZD/USD", 5, 1);
	Mat_AddIndex( mat, "USD/CAD", 5, 1);
	Mat_AddIndex( mat, "USD/CHF", 5, 1);
	Mat_AddIndex( mat, "USD/JPY", 3, 1);
	Mat_AddIndex( mat, "USD/CNH", 5, 1);
	Mat_AddIndex( mat, "USD/HKD", 5, 1);
	Mat_AddIndex( mat, "USD/SGD", 5, 1);
	Mat_AddIndex( mat, "USD/SEK", 5, 1);
	Mat_AddIndex( mat, "USD/THB", 3, 1);
	Mat_AddIndex( mat, "USD/CNY", 5, 1);
#endif


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
**  통화정보 추가 - MAT_INDEX add
***************************************************************************** */
int Mat_AddIndex( MAT *mat, char *current, int point, int unit)
{
	int			i;
	int			base, cont;
	MAT_INDEX	*index;
#if 0
	20231120 시세 table 삭제
	MAT_SISE	*sise;
#endif

	base = Mat_GetCurrentInt( mat, &current[ 0]);
	cont = Mat_GetCurrentInt( mat, &current[ 4]);

	/*************************************/
	/* MAT_INDEX initial set             */
	/*************************************/
	index = Mat_GetIndex( mat, base, cont);
	if( index != NULL)
	{
		LogDel( "이미 등록된 통화입니다. Update 합니다. current=[%s]", current);
	}
	else
	{
		for( i = 0; i < MAT_MAX_CURR; i++)
		{
			index = &mat->map->index[ i];
			if( index->base_cur == 0 && index->cont_cur == 0) break;
		}
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
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  수수료 가져오기
**  group_check - 여러번 수행하는것 방지 ... count에 문제가 생김
***************************************************************************** */
int Mat_GetFeeFunc( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side, int group_check)
{
	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  MAT_START index에 pos 위치 추가
**  처음 위치에 pos를 추가하고 등록된 rec를 다음위치로 set
***************************************************************************** */
int Mat_Insert( MAT *mat, MAT_START *start, int pos)
{
	MAT_RECORD	*rec, *next_rec;
	MAT_HEAD	*head, *next_head;
	int			next_pos = -1;

	rec      = &mat->map->rec[ pos];
	head     = &rec->head;

	/* 최초 record */
	if( start->start <= 0)
	{
		LogDel( "first record. pos=[%d]", pos);
		start->start = pos;
		head->prev = -1;
		head->next = -1;
	}
	else
	{
		/* get next record */
		next_pos  = start->start;
		next_rec  = &mat->map->rec[ next_pos];
		next_head = &next_rec->head;

		/* insert start pos */
		start->start = pos;
		head->prev = -1;
		head->next = next_pos;

		/* set next record */
		next_head->prev = pos;
	}

	/* count set */
	start->start_cnt++;
	mat->map->stat.rec_cnt++;
	LogDbg( "INSERT START pos=[%d] prev=[%d] next=[%d] cnt=[%d]", pos, -1, next_pos, start->start_cnt);
	gettimeofday( &head->ord_time, NULL);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  pos 위치 삭제
***************************************************************************** */
int Mat_Delete( MAT *mat, MAT_START *start, int pos)
{
	int			prev_pos, next_pos;
	MAT_RECORD	*rec, *prev_rec, *next_rec;
	MAT_HEAD	*head;

	/* record delete */
	rec      = &mat->map->rec[ pos];
	head     = &rec->head;
	prev_pos = head->prev;
	next_pos = head->next;

	LogDel( "delete pos   = [%d]", pos);
	LogDel( "prev pos     = [%d]", prev_pos);
	LogDel( "next pos     = [%d]", next_pos);

	if( start->start <= 0)
	{
		LogCri( "record not found. pos=[%d]", pos);
		return -1;
	}

	if( prev_pos <=  0)		/* 처음이면 */
	{
		LogDel( "first        = [%d]", pos);
		start->start = next_pos;
		LogDel( "start->start = [%d]", start->start);
		next_rec = &mat->map->rec[ head->next];
		next_rec->head.prev = -1;
	}
	else
	{
		prev_rec = &mat->map->rec[ head->prev];
		prev_rec->head.next = next_pos;
	}

	if( next_pos <= 0)		/* 마지막이면 */
	{
		LogDel( "last record. pos=[%d]", pos);
	}
	else
	{
		next_rec = &mat->map->rec[ head->next];
		next_rec->head.prev = prev_pos;
	}

	/* index에 record 갯수를 1 감소 */
	start->start_cnt--;
	mat->map->stat.rec_cnt--;
	LogDel( "DELETE START pos=[%d] prev=[%d] next=[%d] cnt=[%d]", pos, prev_pos, next_pos, start->start_cnt);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  wait insert 
**  wait dll 첫번째에 등록
***************************************************************************** */
int Mat_InsertWait( MAT *mat, MAT_START *start, int pos)
{
	int			next_pos = -1;
	MAT_RECORD	*rec, *next_rec;
	MAT_HEAD	*head, *next_head;

	rec       = &mat->map->rec[ pos];
	head      = &rec->head;

	if( start->wait <= 0)
	{
		start->wait = pos;

		start->wait     = pos;
		head->wait_prev = -1;
		head->wait_next = -1;
	}
	else
	{
		next_pos  = start->wait;
		next_rec  = &mat->map->rec[ next_pos];
		next_head = &next_rec->head;

		start->wait     = pos;
		head->wait_prev = -1;
		head->wait_next = next_pos;

		next_head->wait_prev = pos;
	}

	start->wait_cnt++;
	mat->map->stat.rec_cnt++;
	LogDel( "INSERT WAIT pos=[%d] prev=[%d] next=[%d] cnt=[%d]", pos, -1, next_pos, start->wait_cnt);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  wait delete 
**  일치하는 pos를 찾아 dll에서 삭제
***************************************************************************** */
int Mat_DeleteWait( MAT *mat, MAT_START *start, int pos)
{
	int			prev_pos, next_pos;
	MAT_RECORD	*rec, *prev_rec, *next_rec;
	MAT_HEAD	*head, *prev_head, *next_head;

	rec  = &mat->map->rec[ pos];
	head = &rec->head;
	prev_pos = head->wait_prev;
	next_pos = head->wait_next;

	if( prev_pos <= 0) /* first record */
	{
		LogDel( "first record. [ps=[%d]", pos);
		start->wait = next_pos;

		if( next_pos > 0)
		{
			next_rec  = &mat->map->rec[ next_pos];
			next_head = &next_rec->head;
			next_head->wait_prev = -1;
		}
	}
	else	
	{
		prev_rec  = &mat->map->rec[ prev_pos];
		prev_head = &prev_rec->head;

		prev_head->wait_next = next_pos;
	}

	if( next_pos > 0)
	{
		next_rec  = &mat->map->rec[ next_pos];
		next_head = &next_rec->head;

		next_head->wait_prev = prev_pos;
	}
	else
	{
		LogDel( "last record. [ps=[%d]", pos);
	}

	start->wait_cnt--;
	mat->map->stat.rec_cnt--;
	LogDel( "DELETE WAIT pos=[%d] prev=[%d] next=[%d] cnt=[%d]", pos, -1, next_pos, start->wait_cnt);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 그룹주문 찾기
**  opt=0: 빈 record 찾기
**  opt=1: 일치 record 찾기
***************************************************************************** */
int Mat_FindGroup( MAT *mat, MAT_GROUP *grp, int opt)
{
	int			i, pos;
	MAT_GROUP	*gp;

	if( mat->map->grp_pos <= 0) mat->map->grp_pos = 1;	/* 개발시에만 적용 - shm create시 1로 초기화 안했을시 적용 */
	pos = mat->map->grp_pos;

	switch( opt)
	{
		case 0:		/* 빈 record 찾기 */
			gp = &mat->map->grp[ mat->map->grp_pos];
			while( gp->start > 0)
			{
				mat->map->grp_pos++;
				if( mat->map->grp_pos == pos) break;
				if( mat->map->grp_pos >= MAT_MAX_GROUP)	mat->map->grp_pos = 1;
				gp = &mat->map->grp[ mat->map->grp_pos];
			}
			if( gp->start > 0) return -1;
			pos = mat->map->grp_pos;
			mat->map->grp_pos++;
			if( mat->map->grp_pos >= MAT_MAX_GROUP)	mat->map->grp_pos = 1;
			return pos;
		case 1:		/* 일치 record 찾기 */
			for( i = 1; i < MAT_MAX_GROUP; i++)
			{
				gp = &mat->map->grp[ i];
				if( gp->start <= 0) continue;
				if( !memcmp( gp->id, grp->id, sizeof( grp->id)))
				{
					return i;
				}
			}
			return -1;
	}
	return 0;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 그룹주문 찾기 - OLD 20240117
***************************************************************************** */
int Mat_WaitFind( MAT *mat, MAT_START *start, ORDER *obook)
{
	int			pos;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER		*book;

	pos = start->wait;
	while( pos > 0)
	{
		rec  = &mat->map->rec[ pos];
		LogDel( "rec ptr=[%p]", rec);
		head = &rec->head;
		book = ( ORDER *)&rec->ord;
		LogDel( "compare pos=[%d] book->GrpOrdnNo=[%.*s] obook->GrpOrdnNo=[%.*s]",
				pos, sizeof( book->GrpOrdnNo), book->GrpOrdnNo, sizeof( obook->GrpOrdnNo), obook->GrpOrdnNo);
		if( !memcmp( book->GrpOrdnNo, obook->GrpOrdnNo, sizeof( book->GrpOrdnNo)))
		{
			return pos;
		}
		pos = head->wait_next;
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
			if( mat->map->stat.wpos >= MAT_MAX_RECORD)	mat->map->stat.wpos = 1;
			return pos;
		}
		pos++;
		if( pos >= MAT_MAX_RECORD)	pos = 1;
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
int Mat_GetCurrentInt( MAT *mat, char *curr)
{
	int			i;
	MAT_CURRENT	*current;

	for( i = 0; MAT_MAX_CURRENT; i++)
	{
		current = &mat->map->current[ i];
		if( !memcmp( curr, current->str, 3))
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
**  통화 int를 string으로 환산
***************************************************************************** */
char *Mat_GetCurrentString( MAT *mat, int curr)
{
	MAT_CURRENT	*current;

	current = &mat->map->current[ curr];
	return current->str;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - position
**  @retval     실패    - -1
**  @brief
**  index에 해당 position의 주문이 있는지를 check
***************************************************************************** */
int Mat_FindPos( MAT *mat, MAT_INDEX *index, int side, int pos)
{
	int			rec_pos;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;

	rec_pos = index->start[ side].start;
	while( rec_pos > 0)
	{
		LogDel( "rec_pos=[%d] pos=[%d]", rec_pos, pos);
		if( rec_pos == pos) return pos;
		rec  = &mat->map->rec[ rec_pos];
		head = &rec->head;
		rec_pos = head->next;
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
	ORDER		*book;

	head = &rec->head;
	book = ( ORDER *)&rec->ord;

	switch( opt)
	{
		case MAT_FIRST:
			base = Mat_GetCurrentInt( mat, &book->Symbol[ 0]);
			cont = Mat_GetCurrentInt( mat, &book->Symbol[ 4]);
			index = Mat_GetIndex( mat, base, cont);
			if( index == NULL)
			{
				LogMsg( "해당 통화 거래가 존재하지 않습니다. current=[%.7s]", &book->Symbol);
				return NULL;
			}
			side = book->Side[ 0] - '1';
			LogDel( "side=[%d]", side);
			pos = index->start[ side].start;
			break;
		case MAT_LAST:
			break;
			
		case MAT_PREV:
			pos = head->prev;
			if( pos <= 0)
			{
				LogMsg( "처음 레코드 입니다.");
				return NULL;
			}
			break;
		case MAT_NEXT:
			pos = head->next;
			if( pos <= 0)
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
	if( pos <= 0) return NULL;
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

	for( i = 0; i < MAT_MAX_CURR; i++)
	{
		if( base == mat->map->index[ i].base_cur && cont == mat->map->index[ i].cont_cur)
			return &mat->map->index[ i];
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
int Mat_SetHead( MAT *mat, MAT_RECORD *rec, int task)
{
	int			pos;
	MAT_HEAD	*head;

	head = &rec->head;

	switch( task)
	{
		case MAT_HEAD_DELETE:
			pos = rec->pos;
			memset( rec, 0x00, sizeof( MAT_RECORD));
			rec->pos = pos;
			head->prev = -1;
			head->next = -1;
			head->wait_prev = -1;
			head->wait_next = -1;
			head->grp_prev = -1;
			head->grp_next = -1;
			break;
		case MAT_HEAD_ORDER:
			head->gubun = 1;
			gettimeofday( &head->ord_time, NULL);
			break;
		case MAT_HEAD_EXECUTE:
			head->gubun = 2;
			gettimeofday( &head->mat_time, NULL);
			break;
		case MAT_HEAD_CONFORM:
			gettimeofday( &head->con_time, NULL);
			break;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  code로 message 조회
***************************************************************************** */
char *Mat_MessageGet( MAT *mat, int code)
{
	int			i;
	MAT_MESSAGE	*msg;

	for( i = 0; i < MAT_MAX_MSG; i++)
	{
		msg = &mat->map->msg[ i];
		if( msg->code == 0) continue;
		if( code == msg->code) return msg->msg;
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

	retry:
	rtn = Sem_LockT( mat->sem, 1000000);
	if( rtn < 0)
	{
		LogMsg( "Mat_Lock timeout. id=[%d] ", mat->sem->id);
		if( Continue == 0) return -1;
		goto retry;
	}

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
**  @return     prev record position
**  @brief
**  record를 list에서 삭제
***************************************************************************** */
int Mat_RecordUnlink( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side)
{
	int			rtn;
	int			ppos;
	MAT_HEAD	*head;

	/* record delete */
	head = &rec->head;
	ppos = head->prev;

	rtn = Mat_Delete( mat, &index->start[ side], pos);
	if( rtn < 0)
	{
		LogCri( "Mat_Delete error. rtn=[%d] cur=[%d%d] side=[%d] pos=[%d]", 
				rtn, index->base_cur, index->cont_cur, side, pos);
		return rtn;
	}

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

	pos = index->start[ side].start;
	while( pos > 0 && Continue)
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
**  주문번호와 일치하는 주문 찾기
***************************************************************************** */
int Mat_FindRecordByClOrdID( MAT *mat, MAT_INDEX *index, ORDER *obook)
{
	int			side;
	int			pos;
	char		ClOrdID[ 24 +1];
	MAT_RECORD	*rec;
	ORDER		*order;
	MAT_HEAD	*head;

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	LogDel( "find order ----------------------");
	LogDel( "           side           =[%d]", side);
	LogDel( "           ClOrdID        =[%.*s]", sizeof( obook->ClOrdID), obook->ClOrdID);

	/* 임시로 update 필드로 주문을 찾는다 ... update할 position으로 테스트 */
	memcpy( ClOrdID, obook->ClOrdID, sizeof( obook->ClOrdID));
	ClOrdID[ sizeof( ClOrdID)] = 0;

	pos = index->start[ side].start;
	while( pos > 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		order = ( ORDER *)&rec->ord;
		LogDel( "           ClOrdID        =[%.*s]", sizeof( order->ClOrdID), order->ClOrdID);
		if( !memcmp( order->ClOrdID, ClOrdID, sizeof( order->ClOrdID)))
		{
			LogDel( "find ClOrdID ... pos=[%d]", pos);
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
**  원주문번호와 일치하는 주문 찾기
***************************************************************************** */
int Mat_FindRecordByOrigClOrdID( MAT *mat, MAT_INDEX *index, ORDER *obook)
{
	int			side;
	int			pos;
	char		OrigClOrdID[ 24 +1];
	MAT_RECORD	*rec;
	ORDER		*order;
	MAT_HEAD	*head;

	/* 정정/취소 주문은 매수/매도, 통화코드가 같아야 하므로 같은 index 내에서 원 주문을 찾는다. */
	side = ( int)(obook->Side[0] - '1');

	LogDel( "find order ----------------------");
	LogDel( "           side           =[%d]", side);
	LogDel( "           OrigClOrdID    =[%.*s]", sizeof( obook->OrigClOrdID), obook->OrigClOrdID);

	/* 임시로 update 필드로 주문을 찾는다 ... update할 position으로 테스트 */
	memcpy( OrigClOrdID, obook->OrigClOrdID, sizeof( obook->OrigClOrdID));
	OrigClOrdID[ sizeof( OrigClOrdID)] = 0;

	pos = index->start[ side].start;
	while( pos > 0 && Continue)
	{
		rec = &mat->map->rec[ pos];
		order = ( ORDER *)&rec->ord;
		LogDel( "           ClOrdID        =[%.*s]", sizeof( order->ClOrdID), order->ClOrdID);
		if( !memcmp( order->ClOrdID, OrigClOrdID, sizeof( order->ClOrdID)))
		{
			LogDel( "find ClOrdID ... pos=[%d]", pos);
			return pos;
		}
		head = &rec->head;
		pos = head->next;
	}

	return -1;
}

#if JANG_OLD
/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  원주문번호와 일치하는 주문 찾기
***************************************************************************** */
int Mat_SetJang( MAT *mat, char *pair, MAT_JANG_REC *db_jang)
{
	int				rtn;
	int				base_int;
	int				cont_int;
	MAT_JANG_REC	*jang;
	MAT_INDEX		*index;

	base_int = Mat_GetCurrentInt( mat, &pair[ 0]);
	cont_int = Mat_GetCurrentInt( mat, &pair[ 4]);
	LogDel( "base_int=[%d] cont_int=[%d]", base_int, cont_int);

#if 0
	MAT_JANG_REC_Print( db_jang);
#endif

	index = Mat_GetIndex( mat, base_int, cont_int);
	if( index == NULL)
	{
		LogCri( "Mat_GetIndex error. base_int=[%d] cont_int=[%d]", base_int, cont_int);
		return -1;
	}
	jang = &index->jang[ db_jang->id];

	memcpy( jang, db_jang, sizeof( MAT_JANG_REC));

	return 1;
}
#endif

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 주문 찾기
***************************************************************************** */
int Mat_MessageReset( MAT *mat)
{
	mat->map->msg_cnt = 0;

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  일치하는 주문 찾기
***************************************************************************** */
int Mat_MessageAdd( MAT *mat, int err, char *msg)
{
	MAT_MESSAGE	*mp;

	mp = &mat->map->msg[ mat->map->msg_cnt];

	mp->code = err;
	memcpy( mp->msg, msg, strlen( msg) +1);
	mat->map->msg_cnt++;

	return mat->map->msg_cnt;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  code로 message 조회
***************************************************************************** */
int Mat_MessageList( MAT *mat)
{
	int			i;
	MAT_MESSAGE	*msg;

	for( i = 0; i < mat->map->msg_cnt; i++)
	{
		msg = &mat->map->msg[ i];
		if( msg->code == 0) continue;
		printf( "[%5d][%s]\n", msg->code, msg->msg);
	}

	return i;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  code로 message 조회
***************************************************************************** */
int Mat_SetMessage( MAT *mat, ORDER *book, int code)
{
	int			i;
	MAT_MESSAGE	*msg;

	for( i = 0; i < MAT_MAX_MSG; i++)
	{
		msg = &mat->map->msg[ i];
		if( msg->code != code) continue;
		ItoA( book->RefuslCd, msg->code, sizeof( book->RefuslCd));
		AtoA( book->Text,     msg->msg,  sizeof( book->Text));
		LogDbg( "[%.*s][%.*s]\n", sizeof( book->RefuslCd), book->RefuslCd, sizeof( book->Text), book->Text);
	}

	return i;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  code로 message 조회
***************************************************************************** */
int Mat_SetEmsg( MAT *mat, ORDER *book, MAT_E_MSG *msg)
{
	ItoA( book->RefuslCd, atoi( msg->code), sizeof( book->RefuslCd));
	AtoA( book->Text,     msg->mesg,  sizeof( book->Text));

	return 1;
}


