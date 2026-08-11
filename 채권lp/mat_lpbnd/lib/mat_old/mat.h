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

#include "mem.h"
#include "sem.h"
#include "order.h"
#include "sise.h"

#ifndef MAT_H
#define	MAT_H		1

#define		MAT_MAX_RECORD			10000			/* 최대 주문 저장 건수 */
#define		MAT_MAX_CURR			30				/* max 거래 종류 - usd:krw, jpy:krw, eur:krw ... */ 
#define		MAT_IPC_KEY				0xfa001001		/* IPC 접근 key */
#define		MAT_PIPE_NAME			"/app/fxwin/mat/dat/mat.fifo"	/* named pipe path */
#define		MAT_TIMEOUT				-9999
#define		MAT_MAX_CONFORM			100				/* 체결 역전 방지용 주문확인 전송 안된 주문 list */
#define		MAT_CONFORM_TIME		1				/* 주문확인이 안들어 왔을때 대기하는 시간 */
#define		MAT_CONFORM_TIMEOUT		30				/* 주문확인 없이 체결 전송 TIMEOUT  */

#define		MAT_KRW					0
#define		MAT_USD					1
#define		MAT_EUR					2
#define		MAT_JPY					3
#define		MAT_GBP					4
#define		MAT_AUD					5
#define		MAT_NZD					6
#define		MAT_CAD					7
#define		MAT_CHF					8
#define		MAT_CNH					9
#define		MAT_CNY					10
#define		MAT_SGD					11
#define		MAT_THB					12
#define		MAT_DKK					13
#define		MAT_NOK					14
#define		MAT_SEK					15
#define		MAT_HKD					16

#define		MAT_INSERT				1
#define		MAT_UPDATE				2
#define		MAT_DELETE				3

#define		MAT_FIRST				1
#define		MAT_PREV				2
#define		MAT_NEXT				3
#define		MAT_LAST				4

#define		MAT_MAX_GAP				5000000			/* 통계제외 micro seonds */
#define		MAT_STAT_RCV			10				/* 수신 */
#define		MAT_STAT_ORD			20				/* 주문 */
#define		MAT_STAT_MAT			30				/* 체결 */
#define		MAT_STAT_SI1			40				/* matching start */
#define		MAT_STAT_SI2			41				/* matching end   */
#define		MAT_START				1				/* 측정 시작 */
#define		MAT_END					0				/* 측정 종료 */
#define		MAT_COUNT				2				/* count만 측정 */

#define		MAT_ERR_MSGTYPE			10001			/* 주문타입은 신규/정정/취소만 가능합니다. */
#define		MAT_ERR_ORIGID			10002			/* 원주문이 없습니다. */
#define		MAT_ERR_SIDE			10003			/* 매매구분은 BUY/SELL만 가능합니다. */
#define		MAT_ERR_QTY				10004			/* 주문수량을 확인해주세요. */
#define		MAT_ERR_ORDTYPE			10005			/* 주문유형은 시장가/지정가/예약주문만 가능합니다. */
#define		MAT_ERR_SETTTYPE		10006			/* 주문 구분 미지원 */

#ifndef MAX
#define		MAX(x,y)				((x>y)?x:y)
#define		MIN(x,y)				((x<y)?x:y)
#endif

typedef struct _mat_reject_
{
	int		code;
	char	msg[ 512];
}	MAT_REJECT;

typedef struct _oms_obook_t_
{
	char	side[ 1];
	char	base_cur[ 3];
	char	cont_cur[ 3];
	double	price;
	int		order;
	int		update;
}	ORDER_NOT;

typedef struct _mat_head_
{
	int				gubun;				/* 0-빈 record, 1-주문 record, 2-체결 record */
	struct timeval	rcv_time;			/* 주문 접수 시간  */
	struct timeval	con_time;			/* 주문 확인 송신 시간 - 체결 역전 방지용 */
	struct timeval	dly_time;			/* 주문 확인 대기 시간 - 체결 역전 방지용 */
	struct timeval	ord_time;			/* 주문 저장 시간  */
	struct timeval	mat_time;			/* 주문 체결 시간  */
	struct timeval	snd_time;			/* 체결 송신 시간 */
	MATSISE			sise_curr;			/* 계산된 시세 기본통화/상대통화 */
	MATSISE			sise_base;			/* 기본통화 시세 USD/기본통화 */
	MATSISE			sise_cont;			/* 상대통화 시세 USD/상대통화 */
	double			price;				/* 주문 가격 - 마크업을 뺀 가격, 시세 data와 비교하기위한 가격, 0=시장가 */
	double			exe_price;			/* 체결가격 */
	int				mat_type;			/* matching type - 트레일링 스탑 주문 or 기타 주문을 위한 flag 0=일반, 1=tralling stop */
	int				ts_gap;				/* trailling stop 주문 pips gap 범위 */
	int				prev;				/* 이전 record 위치, 시작 record = -1 */
	int				next;				/* 다음 record 위치, 끝 record = -1 */
}	MAT_HEAD;


typedef struct _order_start_
{
	int			base_cur;			/* 기준통화 A */
	int			cont_cur;			/* 상대통화 B */
	MATSISE		sise_curr;			/* 현재 시세 - base, cont로 계산 ... 없어도 될지 검토 */
	MATSISE		sise_base;			/* 기준통화 최종시세 - usd:A 시세, usd:krw이면 없음 */
	MATSISE		sise_cont;			/* 상대통화 최종시세 - usd:B */
	int			point;				/* 소숫점 이하 처리 */
	int			unit;				/* 거래단위 */
	int			start[ 2];			/* 매수(0)/매도(1) start position - 없으면 -1 */
	int			cnt[ 2];			/* 매수(0)/매도(1) 등록된 record 건수 */
}	MAT_INDEX;

typedef struct _mat_statistics_element_
{
	int				max;			/* 최대값 */
	int				min;			/* 최소값 */
	int				tot;			/* total */
	int				cnt;			/* count */
	int				avr;			/* average */
	int				cur;			/* current */
	struct timeval	srt;			/* start time */
	struct timeval	end;			/* end time */
}	STEL;

typedef struct _mat_statistics_
{
	STEL		rcv;				/* 수신 통계 */
	STEL		rej;				/* 거부 통계 */
	STEL		cnf;				/* 접수 통계 */
	STEL		ord;				/* 주문 통계 */
	STEL		mat;				/* 체결 통계 */
	STEL		exe;				/* 체결 전송 통계 */
	STEL		sis;				/* 시세 통계 */
}	MAT_STATIS;

typedef struct _data_area_
{
	int			pos;				/* shared memory position */
	MAT_HEAD	head;				/* matching head */
	ORDER		book;				/* order record */
	char		filler[ 100];		/* filler - 512 byte 맞추기 */
}	MAT_RECORD;

typedef struct _mat_status_
{
	int			rec_cnt;			/* 공유메모리에 저장된 주문 건수 */
	int			exe_cnt;			/* 처리안된 체결 건수 */
	int			max_rec;			/* record 갯수 */
	int			max_curr;			/* 거래쌍 갯수 - index 갯수 */
	int			wpos;				/* 주문 write position */
	time_t		ctime;				/* shared memory create time */
	key_t		key;				/* semaphore, shared_memory 접근 key */
	char		pipe_name[ 512];	/* named pipe path + name */
	MAT_INDEX	index[ MAT_MAX_CURR];	/* 통화 start */
	MAT_STATIS	stat;				/* 통계 */
}	MAT_STATUS;

typedef struct _shm_map_
{
	MAT_RECORD	rec[ MAT_MAX_RECORD];	/* 주문 record */
	MAT_STATUS	stat;					/* 관리 struct */
	int			conform[ MAT_MAX_CONFORM];	/* 역전방지 체결 table (빈 position=-1) */
	int			conform_cnt;				/* 주문 position 저장 갯수 */
}	MAT_MAP;

typedef struct _mat_
{
	key_t		key;				/* ipc 접근 key */
	MEM			*mem;				/* shared memory struct */
	SEM			*sem;				/* semaphore struct */
	int			fd;					/* fifo file desc */
	MAT_MAP		*map;				/* shared memory base pointer */
}	MAT;

#endif	/* MAT_H */

/***** Module : mat.c *****/
MAT*        Mat_CreateForce();                                              /* 매칭엔진에 필요한 ipc를 생성 */
MAT*        Mat_Create();                                                   /* 매칭엔진에 필요한 ipc를 생성 */
int         Mat_Remove( MAT *mat);                                          /* 매칭엔진에서 생성한 ipc를 삭제 */
MAT*        Mat_Open();                                                     /* 매칭엔진 Open */
int         Mat_Close( MAT *mat);                                           /* 매칭엔진 Close */
int         Mat_Init( MAT *mat);                                            /* index initial */
int         Mat_AddIndex( MAT *mat, char *current, int point, int unit);    /* 매칭 */
int         Mat_SiseMatch( MAT *mat, int base, int cont, MATSISE *sise);    /* new sise update - 가공된 MATSISE 처리 */
int         Mat_SiseUpdate( MAT *mat, MAT_INDEX *index, MATSISE *sise, int type);/* sise update - FX_QUOTE_T 처리 */
int         NotMat_SiseUpdate( MAT *mat, int base, int cont, MATSISE *sise);/* sise update - FX_QUOTE_T 처리 */
int         Mat_SiseCalc( MAT *mat, MAT_INDEX *index, int base, int cont, int flag);/* sise 계산 - 재정통화 일때만 */
int         Mat_Match( MAT *mat, MATSISE *sise);                            /* 매칭 */
int         Mat_MatchBySise( MAT *mat, MAT_INDEX *index, MATSISE *sise);    /* 매칭 */
int         Mat_Matching( MAT *mat, MAT_INDEX *index);                      /* 매수/매도체결 */
int         Mat_MatchBid( MAT *mat, MAT_INDEX *index, MATSISE *sise);       /* 매수체결 */
int         Mat_MatchAsk( MAT *mat, MAT_INDEX *index, MATSISE *sise);       /* 매도체결 */
int         Mat_Execute( MAT *mat, int timeout);                            /* 체결처리 */
int         Mat_ReadPipe( MAT *mat, int timeout);                           /* pipe로 부터 체결 record position을 수신 */
int         Mat_Order( MAT *mat, ORDER *obook, int opt);                    /* 주문 insert,update,delete */
int         Mat_RecordInsert( MAT *mat, MAT_INDEX *index, ORDER	 *obook);   /* 주문 insert  */
int         Mat_RecordDelete( MAT *mat, MAT_INDEX *index, ORDER	 *obook);   /* 주문 delete  */
int         Mat_RecordUnlink( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side);/* record를 list에서 삭제 */
int         Mat_FindRecordByPos( MAT *mat, MAT_INDEX *index, ORDER *obook, int find_pos);/* 일치하는 주문 찾기 */
int         Mat_FindRecordByOrigClOrdID( MAT *mat, MAT_INDEX *index, ORDER *obook);/* 일치하는 주문 찾기 */
int         Mat_MakeHead( MAT *mat, MAT_HEAD *head, ORDER	 *obook);         /* 주문 record에서 필요한 data를 head에 set */
int         Mat_PutConform( MAT *mat, int pos);                             /* conform record에 save */
int         Mat_GetConform( MAT *mat);                                      /* conform record에서 get */
int         Mat_GetEmptyRecordPos( MAT *mat);                               /* 빈 record 찾기 */
MAT_RECORD* Mat_GetRecordByPos( MAT *mat, int pos);                         /* position을 입력하여 record 찾기 */
int         Mat_GetCurrentInt( MAT *mat, char *current);                    /* 통화 string을 int로 환산 */
MAT_RECORD* Mat_GetRecord( MAT *mat, MAT_RECORD *rec, int opt);             /* get first/last/prev/next record pointer */
MAT_INDEX*  Mat_GetIndex( MAT *mat, int base, int cont);                    /* 기준/상대 통화 index 찾기 */
int         Mat_Lock( MAT *mat);                                            /* semaphore lock 수행 */
int         Mat_Unlock( MAT *mat);                                          /* semaphore lock 해제 */
int         Mat_StatisticsSet( MAT *mat, STEL *stel, int opt);              /* 통계 setting */
int         Mat_TimeGap( MAT *mat, struct timeval *tv_1, struct timeval *tv_2);/* 시간 차이 구하기 */
int         Mat_StatisReset( MAT *mat);                                     /* 통계 초기화 */
int         Mat_Stat( MAT *mat);                                            /* stat 출력 */
int         Mat_StatIndex( MAT *mat);                                       /* index stat 출력 */
int         Mat_StatOrder( MAT *mat);                                       /* 공유메모리에 남아 있는 주문 출력 */
int         Mat_StatExecute( MAT *mat);                                     /* 공유메모리에 남아 있는 체결 출력 */
int         Mat_PrintIndex( MAT *mat, MAT_INDEX *index);                    /* 한개의 index table 출력 */
int         Mat_PrintStic( MAT *mat);                                       /* 통계 출력 */
int         Mat_PrintConform( MAT *mat);                                    /* 통계 출력 */
int         Mat_PrintRecord( MAT *mat, int pos);                            /* 통계 출력 */
int         MAT_ORDER_RECV_Print( ORDER_RECV* ptr);
int         MAT_ORDER_SEND_Print( ORDER_SEND* ptr);
int         MAT_ORDER_Print( ORDER* ptr);
int         MATSISE_Print( MATSISE* ptr);

