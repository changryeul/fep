/** ***************************************************************************
**  @file       smq.c
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

#ifndef SMQ_H
#define	SMQ_H		1

#define		SMQ_MAX_REC				1000			/* 최대 주문 저장 건수 */
#define		SMQ_REC_SZ				1024			/* 최대 record size */
#define		SMQ_NAME_SZ				32				/* max queue name size */
#define		SMQ_MAX_IDX				30				/* max index 종류  ... */ 
#define		SMQ_IPC_KEY				0xfa010001		/* IPC 접근 key */
#define		SMQ_PIPE_DIR			"/app/fxwin/mat/dat"	/* named pipe path - 파이프 이름은 SMQ_PIPE_DIR/[queue_name].fifo */
#define		SMQ_SEM_TIMEOUT			100000
#define		SMQ_TIMEOUT				-9999
#define		SMQ_NODATA				-9998
#define		SMQ_CONFORM_CNT			5				/* 초단위 - 이시간이후에도 주문확인 없을시 강제 체결 처리 */

#ifndef MAX
#define		MAX(x,y)				((x>y)?x:y)
#define		MIN(x,y)				((x<y)?x:y)
#endif

typedef struct _smq_head_
{
	int				gubun;				/* 0-빈 record, 1-data record */
	int				sz;					/* data record size */
	struct timeval	w_time;				/* write 시간  */
	struct timeval	r_time;				/* read 시간 */
	int				prev;				/* 이전 record 위치, 시작 record = -1 */
	int				next;				/* 다음 record 위치, 끝 record = -1 */
}	SMQ_HEAD;

typedef struct _smq_index_
{
	char			name[ SMQ_NAME_SZ];	/* queue index name - 파이프 이름도 queue name 으로 정의 name.fifo */
										/* index 0번은 NULL 이름일때 사용 --- smq.fifo */
	key_t			key;				/* semaphore key */
	int				start;				/* dll index start position */
	int				end;				/* dll index end position */
	int				cnt;				/* data record count */
	int				dcnt;				/* data count from start */
	pid_t			spid;				/* last send pid */
	time_t			stime;				/* last send time */
	pid_t			rpid;				/* last recv pid */
	time_t			rtime;				/* last recv time */
	char			comment[ 128];		/* 설명 */
}	SMQ_INDEX;


typedef struct _smq_statistics_element_
{
	int				max;			/* 최대값 */
	int				min;			/* 최소값 */
	int				tot;			/* total */
	int				cnt;			/* count */
	int				avr;			/* average */
	struct timeval	srt;			/* start time */
	struct timeval	end;			/* end time */
}	SMQ_STEL;

typedef struct _smq_statistics_
{
	SMQ_STEL		rcv;			/* 수신 통계 */
	SMQ_STEL		rej;			/* 거부 통계 */
	SMQ_STEL		cnf;			/* 접수 통계 */
	SMQ_STEL		ord;			/* 주문 통계 */
	SMQ_STEL		smq;			/* 체결 통계 */
	SMQ_STEL		exe;			/* 체결 전송 통계 */
	SMQ_STEL		sis;			/* 시세 통계 */
}	SMQ_STATIS;

typedef struct _smq_data_area_
{
	int			pos;				/* shared memory position */
	SMQ_HEAD	head;				/* smqching head */
	char		rec[ SMQ_REC_SZ];	/* data record */
	char		filler[ 100];		/* filler - 512 byte 맞추기 */
}	SMQ_RECORD;

typedef struct _smq_status_
{
	int			max_rec;			/* 최대 record 갯수 */
	int			rec_cnt;			/* 공유메모리에 저장된 data 건수 */
	int			wpos;				/* write position - 빈 record 찾기 시작 위치 */
	time_t		ctime;				/* shared memory create time */
	key_t		key;				/* semaphore, shared_memory 접근 key */
	char		pipe_dir[ 512];		/* named pipe path */
	SMQ_STATIS	stat;				/* 통계 */
}	SMQ_STATUS;

typedef struct _smq_shm_map_
{
	SMQ_RECORD	rec[ SMQ_MAX_REC];	/* 주문 record */
	SMQ_INDEX	index[ SMQ_MAX_IDX];/* index start */
	SMQ_STATUS	stat;				/* 관리 struct */
}	SMQ_MAP;

typedef struct _smq_
{
	char		name[ SMQ_NAME_SZ];	/* queue name */
	int			idx;				/* index position */
	MEM			*mem;				/* shared memory struct */
	SEM			*sem_mem;			/* semaphore struct - for smq shared memory */
	SEM			*sem;				/* semaphore struct */
	int			fd;					/* fifo file desc */
	SMQ_MAP		*map;				/* shared memory base pointer */

	int			pos;				/* empty record position - set Smq_SetRecord/Smq_GetRecord */
	void		*ptr;				/* user record pointer */
	int			sz;					/* user record size */
	int			flag;				/* 0-commit, 1-write, 2-read */
}	SMQ;

#endif	/* SMQ_H */

/*
 * 각 function은 lock사용을 위해 다음 짝으로 사용
 *
 * lock						lock 해제									설명
 * Smq_SetRecord			Smq_PutRecord/Smq_Commit/Smq_Rollback		data 자리를 예약후, 해제function에서 copy 및 insert
 * Smq_GetRecord			Smq_DeleteRecord/Smq_Commit/Sqm_Rollback	data를 get후 처리완료 이후 queue delete
 * Smq_SetPtr				Smq_PutPtr
 *
 */

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

