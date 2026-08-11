/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리 
**
**  mat.c		- 매칭엔진 기본 라이브러리
**  mat_group.c	- 그룹주문 처리
**  mat_stat.c	- 공유메모리 상태등를 출력
**  mat_jang.c	- 주문/매칭 장운영 check
**  convert.c	- 수신주문/처리주문/송신주문 간의 convert
**  print.c		- 출력을 위한 formatting ... struct 명령을 기본
**  allog.c		- 수수료 엔진에서 쓰는 APLog를 위한 모듈
**  smq.c		- 주문수신 체결송신용 queue를 위한 모듈
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "sem.h"
#include "order.h"
#include "sise.h"
#include "comm.h"
#include "smq.h"

#ifndef MAT_H
#define	MAT_H		1

#define	JANG_OLD	0

#define		MAT_MAX_RECORD			100000			/* 최대 주문 저장 건수 */
#define		MAT_MAX_CURRENT			32				/* max 통화 종류 - krw,usd,eur,jpy ... */
#define		MAT_MAX_CURR			64				/* max 거래 종류 - usd:krw, jpy:krw, eur:krw ... */ 
#define		MAT_MAX_JANG			1000			/* 장운영 check - 999 이상은 검토필요 */
#define		MAT_MAX_STATIS			10				/* max 통계 record */
#define		MAT_MAX_MSG				1000			/* error message mat record */
#define		MAT_MAX_GROUP			10				/* 동시에 처리 가능한 그룹 갯수 */
#define		MAT_IPC_KEY				0xfa001001		/* IPC 접근 key */
#define		MAT_ORD_PIPE			"mat_ord.fifo"	/* named pipe path */
#define		MAT_EXE_PIPE			"mat_exe.fifo"	/* named pipe path */
#define		MAT_MAT_PIPE			"mat_mat.fifo"	/* named pipe path */
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
#define		MAT_INSERT_GROUP		4

#define		MAT_FIRST				1
#define		MAT_PREV				2
#define		MAT_NEXT				3
#define		MAT_LAST				4

#define		MAT_MAX_GAP				1000000			/* 통계제외 micro seonds */
#define		MAT_STAT_RCV			0				/* 수신 통계 */	
#define		MAT_STAT_REJ			1				/* 거부 통계 */
#define		MAT_STAT_CNF			2				/* 접수 통계 */
#define		MAT_STAT_ORD			3				/* 주문 통계 */
#define		MAT_STAT_CAN			4				/* 취소 통계 */
#define		MAT_STAT_MAT			5				/* 체결 통계 */
#define		MAT_STAT_PEE			6				/* 체결 전송 통계 */
#define		MAT_STAT_DIR			7				/* 즉시체결  통계 */    
#define		MAT_STAT_SIS			8				/* 시세 통계 */    
#if 0
#define		MAT_STAT_RCV			10				/* 수신 */
#define		MAT_STAT_ORD			20				/* 주문 */
#define		MAT_STAT_MAT			30				/* 체결 */
#define		MAT_STAT_SI1			40				/* matching start */
#define		MAT_STAT_SI2			41				/* matching end   */
#endif
#define		MAT_STAT_START			1				/* 측정 시작 */
#define		MAT_STAT_END			0				/* 측정 종료 */
#define		MAT_STAT_COUNT			2				/* count만 측정 */

/* header set function argument */
#define		MAT_HEAD_DELETE			0
#define		MAT_HEAD_ORDER			1				
#define		MAT_HEAD_EXECUTE		2
#define		MAT_HEAD_CONFORM		3

#ifndef MAX
#define		MAX(x,y)				((x>y)?x:y)
#define		MIN(x,y)				((x<y)?x:y)
#endif

/* 통화 표시 */
extern char MatCurrent[ 32][ 4];
/* 주문 상태 표시 */
extern char *StrCodeGubun[];
extern char *StrCodeStat[];
extern char *StrCodeSide[];
extern char *StrCodePrice[];
extern char *StrCodeOrig[];
extern char *StrCodeType[];
extern char *StrCodePrty[];
extern char *StrCodeTran[];

#define	MAX_HOLIDAY_REC		256
typedef struct _mat_holiday_rec_
{
	int		day;
	char	str[ 16];
	char	comment[ 128];
}	MAT_HOLIDAY_REC;

typedef struct _mat_holiday_
{
	MAT_HOLIDAY_REC	rec[ MAX_HOLIDAY_REC];
	int				cnt;
}	MAT_HOLIDAY;


/* 통화 - USD KRW .... */
/* 호가 - M:시장가 F:지정가 */
/* 상품 - BAR SPT FWD MAR SWP ... */
/* 테너 - ALL TOD TOM SPT W01 ... */
typedef struct _mat_jang_rec_
{
	char	key[ 16];		/* 10자리 통화(3)호가(1)상품(3)테너(3) -> ex:USDMSPTTOM */
	int		used;			/* 1 - 사용 */
	time_t	start;
	time_t	end;
}	MAT_JANG_REC;

typedef struct _mat_jang_time_
{
	int		used;
	time_t	start;
	time_t	end;
}	MAT_JANG_TIME;

typedef struct _mat_jang_
{
	MAT_JANG_REC	rec[ MAT_MAX_JANG];
	int				cnt;
}	MAT_JANG;

typedef struct _mat_current_
{
	int		num;
	char	str[ 8];
	char	comment[ 32];
	char	filler[ 20];
}	MAT_CURRENT;

typedef struct _mat_reject_
{
	int		code;
	char	msg[ 512];
}	MAT_REJECT;

typedef struct _mat_message_
{
	int		code;
	char	msg[ 100];
	char	filler[ 24];
}	MAT_MESSAGE;

typedef struct _mat_group_
{
	int		start;						/* 그룹주문 시작 position - single linked list */
	char	id[ 11];					/* 그룹 주문 번호 - GrpOrdnNo */
	int		seq;						/* 그룹 주문 건수 - GrpOrdnCnt */
	int		tot;						/* 그룹 주문 건수 - GrpOrdnCnt */
	int		cnt;						/* 저장된 그룹주문 건수 - GrpOrdnSeq */
}	MAT_GROUP;

typedef struct _mat_head_
{
	int				gubun;				/* 0-빈 record, 1-주문 record, 2-체결 record */
	int				ord_stat;			/* 주문 상태 0-none, 1-주문, 2-체결, 3-취소, 4-강제취소, 5-정정, 6-그룹 */
	int				error;				/* 주문 error number 0:no error */
	struct timeval	rcv_time;			/* 주문 접수 시간  */
	struct timeval	con_time;			/* 주문 확인 송신 시간 - 체결 역전 방지용 */
	struct timeval	dly_time;			/* 주문 확인 대기 시간 - 체결 역전 방지용 */
	struct timeval	ord_time;			/* 주문 저장 시간  */
	struct timeval	mat_time;			/* 주문 체결 시간  */
	struct timeval	snd_time;			/* 체결 송신 시간 */
	MATSISE			sise_curr;			/* 계산된 시세 기본통화/상대통화 */
	MATSISE			sise_base;			/* 기본통화 시세 USD/기본통화 */
	MATSISE			sise_cont;			/* 상대통화 시세 USD/상대통화 */
	SPLIT_IN_ST		fee_in;				/* 수수료 계산을 위한 in 파라메터 - SWAP인 경우 near */
	SPLIT_OUT_ST	fee_out;			/* 수수료 결과 */
	SPLIT_IN_ST		fee_in_far;			/* 수수료 계산을 위한 in 파라메터 - SWAP인 경우 far */
	SPLIT_OUT_ST	fee_out_far;		/* 수수료 결과 */
	double			price;				/* 주문 가격 - 시세 data와 비교하기위한 가격 - mat_ord 에서 set */
	double			exe_price;			/* 체결가격 */
	int				mat_type;			/* matching type 일반 = 0 */
										/* 트레일링 스탑 주문 = 1 */
										/* 그룹주문 = 2 */
										/* stop market - 3 */
										/* stop limit - 4 */
	int				ts_gap;				/* trailling stop 주문 pips gap 범위 */
	int				idx_no;				/* record가 속한 index number */
	int				jang_id;			/* 적용할 장운영 nnnfffjjj - swap_near/1000000 swap_far%1000000/1000 order%1000 */
	int				prev;				/* 이전 record 위치, 시작 record = -1 */
	int				next;				/* 다음 record 위치, 끝 record = -1 */
	int				wait_prev;			/* 주문 대기열 이전 record 위치 */
	int				wait_next;			/* 주문 대기열 다음 record 위치 */
	int				grp_prev;			/* group 주문 이전 */
	int				grp_next;			/* group 주문 다음 */
}	MAT_HEAD;

typedef struct _mat_start_
{
	int			start;					/* start position, 없으면 -1 */
	int			start_cnt;				/* index 내의 주문 수량 */
	int			wait;					/* 주문 wait start position, ex) 그룹주문 전체 수신까지 대기 */
	int			wait_cnt;				/* index 내의 wait 주문 수량 */
}	MAT_START;

typedef struct _order_start_
{
	int			no;					/* index number */
	int			base_cur;			/* 기준통화 A */
	int			cont_cur;			/* 상대통화 B */
	int			price_time;			/* 시세 유효시간 */
	MATSISE		sise_curr;			/* 현재 시세 - base, cont로 계산 ... 없어도 될지 검토 */
	MATSISE		sise_base;			/* 기준통화 최종시세 - usd:A 시세, usd:krw이면 없음 */
	MATSISE		sise_cont;			/* 상대통화 최종시세 - usd:B */
	int			point;				/* 소숫점 이하 처리 */
	int			unit;				/* 거래단위 */
	MAT_START	start[ 2];			/* 매수(0)/매도(1) start position - 없으면 -1 */
	int			mat_cnt;			/* matching count */
	time_t		mat_time;			/* matching last update time */
	int			sis_cnt;			/* sise count */
	time_t		sis_time;			/* sise last update time */
	int			count[ 8];			/* filler */
}	MAT_INDEX;

typedef struct _mat_statistics_
{
	int				id;				/* 통계 id ... ex) pid_t */
	int				max;			/* 최대값 */
	int				min;			/* 최소값 */
	int				tot;			/* total */
	int				cnt;			/* count */
	int				avr;			/* average */
	int				cur;			/* current */
	struct timeval	srt;			/* start time */
	struct timeval	end;			/* end time */
}	MAT_STATIS;

typedef struct _data_area_
{
	int			pos;				/* shared memory position */
	MAT_HEAD	head;				/* matching head */
	char		ord[ 2048];			/* 주문 record */
	char		filler[ 80];		/* 3600 byte에 맞춤 */
}	MAT_RECORD;

typedef struct _mat_status_
{
	int			service;			/* service 상태 0-not yet, 1-service - 로드 후 1로 세팅 */
	int			rec_cnt;			/* 공유메모리에 저장된 주문 건수 */
	int			exe_cnt;			/* 처리안된 체결 건수 */
	int			max_rec;			/* record 갯수 */
	int			max_curr;			/* 거래쌍 갯수 - index 갯수 */
	int			wpos;				/* 주문 write position - 1부터 시작 0번 record는 안씀 */
	time_t		ctime;				/* shared memory create time */
	int			holiday;			/* 휴일 여부 0-휴일아님 1-휴일 */
	int			last_day;			/* 매월 마지막 영업일 여부 */
	time_t		last_time;			/* 마지막 영업일 장마감 시간 */
	time_t		cancel_start;		/* 강제 취소 시작  - 유효주문을 강제 취소하고 신규 주문을 거부 */
	time_t		cancel_end;			/* 강제 취소 종료 */
	time_t		start;				/* 장시작 시간 */
	time_t		end;				/* 장 마감 */
	int			sise;				/* 시세 멈춤 1=시세멈춤 - 강제취소 시간에 시세에 의한 체결 방지 */
	time_t		bs_time;			/* start batch run time */
	time_t		be_time;			/* end batch run time */
	int			bs_int;				/* start batch int time - set */
	int			be_int;				/* end batch int time - set */
	key_t		key;				/* semaphore, shared_memory 접근 key */
	char		ord_pipe[ 512];		/* order named pipe path + name */
	char		exe_pipe[ 512];		/* execute named pipe path + name */
	char		mat_pipe[ 512];		/* matching named pipe path + name */
	MAT_STATIS	statis[ MAT_MAX_STATIS];	/* 통계 */
}	MAT_STATUS;

typedef struct _shm_map_
{
	MAT_STATUS	stat;						/* 관리 struct */
	MAT_CURRENT	current[ MAT_MAX_CURRENT];	/* 통화 표시 int */
	MAT_INDEX	index[ MAT_MAX_CURR];		/* 통화 start */
	MAT_RECORD	rec[ MAT_MAX_RECORD];		/* 주문 record */
	MAT_GROUP	grp[ MAT_MAX_GROUP];		/* group 주문 */
	int			grp_pos;					/* group 주문 저장 위치 */
	int			conform[ MAT_MAX_CONFORM];	/* 역전방지 체결 table (빈 position=-1) */
	int			conform_cnt;				/* 주문 position 저장 갯수 */
	MAT_MESSAGE	msg[ MAT_MAX_MSG];			/* 에러 코드 */
	int			msg_cnt;					/* error code count */
	MAT_JANG	jang;						/* 장운영 */
	char		filler[ 1024];
}	MAT_MAP;

typedef struct _mat_
{
	key_t		key;				/* ipc 접근 key */
	MEM			*mem;				/* shared memory struct */
	SEM			*sem;				/* semaphore struct */
	int			ord_fd;				/* order fifo file desc */
	int			exe_fd;				/* execute fifo file desc */
	int			mat_fd;				/* matching fifo file desc */
	MAT_MAP		*map;				/* shared memory base pointer */
}	MAT;

#endif	/* MAT_H */


/***** Module : mat.c *****/
MAT*        Mat_CreateForce();                                              /* 매칭엔진에 필요한 ipc를 생성 */
MAT*        Mat_Create();                                                   /* 매칭엔진에 필요한 ipc를 생성 */
int         Mat_Remove( MAT *mat);                                          /* 매칭엔진에서 생성한 ipc를 삭제 */
MAT*        Mat_Open( int service);                                         /* 매칭엔진 Open */
int         Mat_Close( MAT *mat);                                           /* 매칭엔진 Close */
int         Mat_Init( MAT *mat);                                            /* 공유메모리 변수들 초기화 */
int         Mat_AddIndex( MAT *mat, char *current, int point, int unit);    /* 통화정보 추가 - MAT_INDEX add */
int         Mat_GetFeeFunc( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side, int group_check);/* 수수료 가져오기 */
int         Mat_Insert( MAT *mat, MAT_START *start, int pos);               /* MAT_START index에 pos 위치 추가 */
int         Mat_Delete( MAT *mat, MAT_START *start, int pos);               /* pos 위치 삭제 */
int         Mat_InsertWait( MAT *mat, MAT_START *start, int pos);           /* wait insert  */
int         Mat_DeleteWait( MAT *mat, MAT_START *start, int pos);           /* wait delete  */
int         Mat_FindGroup( MAT *mat, MAT_GROUP *grp, int opt);              /* 일치하는 그룹주문 찾기 */
int         Mat_WaitFind( MAT *mat, MAT_START *start, ORDER *obook);        /* 일치하는 그룹주문 찾기 - OLD 20240117 */
int         Mat_MakeHead( MAT *mat, MAT_HEAD *head, ORDER	 *obook);         /* 주문 record에서 필요한 data를 head에 set */
int         Mat_PutConform( MAT *mat, int pos);                             /* conform record에 save */
int         Mat_GetConform( MAT *mat);                                      /* conform record에서 get */
int         Mat_GetEmptyRecordPos( MAT *mat);                               /* 빈 record 찾기 */
MAT_RECORD* Mat_GetRecordByPos( MAT *mat, int pos);                         /* position을 입력하여 record 찾기 */
int         Mat_GetCurrentInt( MAT *mat, char *curr);                       /* 통화 string을 int로 환산 */
char*       Mat_GetCurrentString( MAT *mat, int curr);                      /* 통화 int를 string으로 환산 */
int         Mat_FindPos( MAT *mat, MAT_INDEX *index, int side, int pos);    /* index에 해당 position의 주문이 있는지를 check */
MAT_RECORD* Mat_GetRecord( MAT *mat, MAT_RECORD *rec, int opt);             /* get first/last/prev/next record pointer */
MAT_INDEX*  Mat_GetIndex( MAT *mat, int base, int cont);                    /* 기준/상대 통화 index 찾기 */
int         Mat_SetHead( MAT *mat, MAT_RECORD *rec, int task);              /* semaphore lock 수행 */
char*       Mat_MessageGet( MAT *mat, int code);                            /* code로 message 조회 */
int         Mat_Lock( MAT *mat);                                            /* semaphore lock 수행 */
int         Mat_Unlock( MAT *mat);                                          /* semaphore lock 해제 */
int         Mat_RecordUnlink( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side);/* record를 list에서 삭제 */
int         Mat_FindRecordByPos( MAT *mat, MAT_INDEX *index, ORDER *obook, int find_pos);/* 일치하는 주문 찾기 */
int         Mat_FindRecordByOrigClOrdID( MAT *mat, MAT_INDEX *index, ORDER *obook);/* 일치하는 주문 찾기 */
int         Mat_MessageReset( MAT *mat);                                    /* 일치하는 주문 찾기 */
int         Mat_MessageAdd( MAT *mat, int err, char *msg);                  /* 일치하는 주문 찾기 */
int         Mat_MessageList( MAT *mat);                                     /* code로 message 조회 */
int         Mat_SetMessage( MAT *mat, ORDER *book, int code);               /* code로 message 조회 */
int         Mat_SetEmsg( MAT *mat, ORDER *book, MAT_E_MSG *msg);            /* code로 message 조회 */

/***** Module : mat_group.c *****/
int         Mat_InsertGroup( MAT *mat, ORDER *obook, int pos);              /* 일치하는 그룹주문을 wait에서 삭제 */
int         Mat_GroupDelete( MAT *mat, MAT_START *start, int del_pos);      /* 일치하는 그룹주문을 wait에서 삭제 */
int         Mat_GroupNew( MAT *mat, MAT_GROUP *grp, int pos);               /* 그룹주문을 wait index 추가  */
int         Mat_GroupAdd( MAT *mat, MAT_START *start, ORDER *obook, int pos);/* 그룹주문 double linked list 추가  */
int         Mat_GroupEnd( MAT *mat, MAT_START *start, ORDER *obook, int pos);/* 그룹주문 double linked list 추가  */

/***** Module : smq.c *****/
SMQ*        Smq_CreateForce( char *cfg_name);                               /* SMQ에 필요한 ipc를 생성 */
SMQ*        Smq_Create( char *cfg_name);                                    /* SMQ에 필요한 ipc를 생성 */
int         Smq_Remove( SMQ *smq);                                          /* SMQ에서 생성한 ipc를 삭제 */
int         Smq_RemoveForce( char *cfg_name);                               /* SMQ에서 생성한 ipc를 삭제 */
SMQ*        Smq_Open( char *name);                                          /* SMQ Open */
int         Smq_Close( SMQ *smq);                                           /* SMQ Close */
int         Smq_Init( SMQ *smq);                                            /* index initial */
int         Smq_Load( SMQ *smq, char *cfg_name);                            /* index initial */
int         Smq_GetIndex( SMQ *smq, char *line, int pos);                   /* index initial */
int         Smq_FindIndex( SMQ *smq, char *name);                           /* find index - name=NULL 이면 빈 index return */
int         Smq_WritePipe( SMQ *smq);                                       /* pipe로 부터 체결 record position을 수신 */
int         Smq_ReadPipe( SMQ *smq, int timeout);                           /* pipe로 부터 체결 record position을 수신 */
int         Smq_Write( SMQ *smq, char *rec, int sz);                        /* data write - lock 없음 */
int         Smq_Read( SMQ *smq, char *rec, int sz);                         /* queue read - lock 없음 */
int         Smq_Send( SMQ *smq, char *rec, int sz);                         /* data send - lock/unlock */
int         Smq_Recv( SMQ *smq, char *rec, int sz, int timeout);            /* data receive - lock/unlock */
int         Smq_Check( SMQ *smq, int timeout);                              /* pipe event check */
int         Smq_Commit( SMQ *smq);                                          /* Lock 해제 및 insert/delete */
int         Smq_Rollback( SMQ *smq);                                        /* Lock 해제 */
int         Smq_SetRecord( SMQ *smq, char *rec, int sz);                    /* 빈 record get - Lock 유지, Smq_PutRecord/Smq_Commit - Lock 해제 */
int         Smq_PutRecord( SMQ *smq);                                       /* Lock 해제 및 insert */
char*       Smq_SetPtr( SMQ *smq);                                          /* record pointer get - Lock 유지, Smq_PutPtr에서 Lock 해제 */
int         Smq_PutPtr( SMQ *smq, char *rec, int sz);                       /* Lock 해제 및 insert */
int         Smq_InsertRecord( SMQ *smq);                                    /* internal - smq->pos 내용을 index에 insert */
int         Smq_GetRecord( SMQ *smq, char *rec, int sz, int timeout);       /* current queue get - Lock 유지, Smq_DeleteRecord에서 Lock 해제 */
int         Smq_DelRecord( SMQ *smq);                                       /* index에서 record delete, Lock 해제 */
int         Smq_DeleteRecord( SMQ *smq);                                    /* internal - index에서 record delete */
int         Smq_GetEmptyRecordPos( SMQ *smq);                               /* 빈 record 찾기 */
SMQ_RECORD* Smq_GetRecordByPos( SMQ *smq, int pos);                         /* position을 입력하여 record 찾기 */
int         Smq_MemLock( SMQ *smq);                                         /* semaphore lock 수행 - 공유메모리 */
int         Smq_MemUnlock( SMQ *smq);                                       /* semaphore lock 해제 - 공유메모리 */
int         Smq_Lock( SMQ *smq);                                            /* semaphore lock 수행 - queue */
int         Smq_Unlock( SMQ *smq);                                          /* semaphore lock 해제 - queue */
int         Smq_StatisReset( SMQ *smq);                                     /* stat 출력 */
int         Smq_Stat( SMQ *smq);                                            /* stat 출력 */

/***** Module : convert.c *****/
int         Proc_OrderConvert( ORDER *order, ORDER_RECV *recv);             /* convert ORDER_RECV to ORDER */
int         Proc_SendConvert( ORDER_SEND *send, ORDER *order);              /* convert ORDER to ORDER_SEND */
int         Proc_SiseConvert( MATSISE *dest, MATSISE *orig);                /* convert ORDER to ORDER_SEND */

/***** Module : print.c *****/
int         ORDER_HEAD_Print( ORDER_HEAD* ptr);
int         ORDER_RECV_Print( ORDER_RECV* ptr);
int         ORDER_SEND_Print( ORDER_SEND* ptr);
int         ORDER_Print( ORDER* ptr);
int         ORDER_PrintFile( ORDER* ptr, FILE *fp);
int         MATSISE_Print( MATSISE* ptr);
int         MATSISE_PrintFile( MATSISE* ptr, FILE *fp);
int         APSISE_Print( APSISE* ptr);
int         APSISE_PrintFile( APSISE* ptr, FILE *fp);
int         SISE_ENTRY_Print( SISE_ENTRY* ptr);
int         FX_QUOTE_T_Print( FX_QUOTE_T* ptr);
int         SPLIT_IN_ST_Print( SPLIT_IN_ST* ptr);
int         SPLIT_IN_ST_PrintFile( SPLIT_IN_ST* ptr, FILE *fp);
int         SPLIT_PRD_LIST_Print( SPLIT_PRD_LIST* ptr);
int         SPLIT_PRD_LIST_PrintFile( SPLIT_PRD_LIST* ptr, FILE *fp);
int         SPLIT_OUT_ST_Print( SPLIT_OUT_ST* ptr);
int         SPLIT_OUT_ST_PrintFile( SPLIT_OUT_ST* ptr, FILE *fp);
int         MAT_E_MSG_Print( MAT_E_MSG* ptr);
int         SPLIT_IN_ST_File( SPLIT_IN_ST* ptr, FILE *fp);
int         SPLIT_PRD_LIST_File( SPLIT_PRD_LIST* ptr, FILE *fp);
int         SPLIT_OUT_ST_File( SPLIT_OUT_ST* ptr, FILE *fp);
int         MAT_COMM_HEAD_Print( MAT_COMM_HEAD* ptr);
int         MAT_DATA_HEAD_Print( MAT_DATA_HEAD* ptr);
int         MAT_PACKET_Print( MAT_PACKET* ptr);
int         MAT_HEAD_Print( MAT_HEAD* ptr);
int         MAT_HEAD_PrintFile( MAT_HEAD* ptr, FILE *fp);
int         MAT_START_Print( MAT_START* ptr);
int         MAT_START_PrintFile( MAT_START* ptr, FILE *fp);
int         MAT_INDEX_Print( MAT_INDEX* ptr);
int         MAT_INDEX_PrintFile( MAT_INDEX* ptr, FILE *fp);
int         MAT_GROUP_Print( MAT_GROUP* ptr);
int         MAT_STATIS_Print( MAT_STATIS* ptr);
int         MAT_RECORD_Print( MAT_RECORD* ptr);
int         MAT_STATUS_Print( MAT_STATUS* ptr);
int         MAT_MAP_Print( MAT_MAP* ptr);
int         MAT_Print( MAT* ptr);

#if 0
/***** Module : aplog.c *****/
int         APLog( char *pname, int level, char *format, ...);
#endif

/***** Module : mat_jang.c *****/
int         Mat_GetJangPdcdNo( MAT *mat, char *Pdcd);                       /* 장운영 - 상품코드 번호 return */
int         Mat_GetJangTnrNo( MAT *mat, char *TnrId);                       /* 장운영 - 테너 번호 return */
int         Mat_GetJangId( MAT *mat, ORDER *order, int flag);               /* 장운영 */
int         Mat_JangCheck( MAT *mat, ORDER *order, int jang_id);            /* 장운영 */
int         Mat_JangCheckSwap( MAT *mat, int curr, int hoga, ORDER *order, int flag);/* 장운영 */
time_t      Mat_JangHtoT( MAT *mat, char *str_time);                        /* 장운영 - string(HHMMSS)을 time_t 값으로 convert */

/***** Module : mat_stat.c *****/
int         Mat_StatisticsSet( MAT *mat, int pos, int opt);                 /* 통계 setting */
int         Mat_TimeGap( MAT *mat, struct timeval *tv_1, struct timeval *tv_2);/* 시간 차이 구하기 */
int         Mat_StatisReset( MAT *mat);                                     /* 통계 초기화 */
int         Mat_Stat( MAT *mat);                                            /* stat 출력 */
int         Mat_StatIndex( MAT *mat);                                       /* index stat 출력 */
int         Mat_StatIndexPos( MAT *mat, int idx_pos);                       /* index stat 출력 */
int         Mat_StatOrder( MAT *mat);                                       /* 공유메모리에 남아 있는 주문 출력 */
int         Mat_StatRecord( MAT *mat, int filter);                          /* 공유메모리에 남아 있는 주문 출력 */
int         Mat_StatExecute( MAT *mat);                                     /* 공유메모리에 남아 있는 체결 출력 */
int         Mat_StatJangCurr( MAT *mat, char *key[]);                       /* 공유메모리에 남아 있는 체결 출력 */
int         Mat_StatJangPair( MAT *mat);                                    /* 공유메모리에 남아 있는 체결 출력 */
int         Mat_PrintIndex( MAT *mat, MAT_INDEX *index);                    /* 한개의 index table 출력 */
int         Mat_PrintStic( MAT *mat);                                       /* 통계 출력 */
int         Mat_PrintSticRaw( MAT *mat);                                    /* 통계 출력 to log file */
int         Mat_PrintConform( MAT *mat);                                    /* 통계 출력 */
int         Mat_StatGroup( MAT *mat);                                       /* 통계 출력 */
int         Mat_PrintRecord( MAT *mat, int pos);                            /* 통계 출력 */
int         Mat_RecordToFile( MAT *mat, int pos, FILE *fp);                 /* 통계 출력 */
int         Mat_OrderList( MAT *mat);                                       /* 통화 조회 */
int         Mat_StrCode( int code);
int         MAT_HEAD_File( MAT_HEAD* ptr, FILE *fp);                        /* 통계 출력 */

