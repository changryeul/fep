/** ***************************************************************************
**  @file       slp.h
**  @date       2024/02/01
**  @author     cdc
**  @version    V0.0.1
**  @brif
**  double linked list position manager library
**  double linked list를 포지션으로 관리 - 공유 메모리용
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "sem.h"

#ifndef SLP_H
#define	SLP_H

#define	SLP_MAX_CURR			32				/* MAX 통화정보 */
#define	SLP_MAX_PAIR			64				/* MAX 통화거래정보 */
#define	SLP_MAX_STATIS			16				/* MAX 통계 record */

/* 통계 */
#define	SLP_STAT_MAX_GAP		1000000			/* 통계 무시 시간 - micro seconds */
#define	SLP_STAT_COUNT			0
#define	SLP_STAT_START			1
#define	SLP_STAT_END			2
#define	SLP_STAT_SISE			0				/* 시세 통계 */
#define	SLP_STAT_MATCH			1				/* 매칭 통계 */

#define	SLP_STATUS_PTR( x)		( x->status)
#define SLP_MAX_IDX( x)			( x->status->param.max_idx)
#define SLP_SZ_IDX( x)			( x->status->param.idx_size)
#define SLP_GET_IDX( x, y)		( ( SLP_INDEX *)( ( size_t)x->index + ( ( size_t)SLP_SZ_IDX( x) * ( size_t)y)))
#define SLP_MAX_REC( x)			( x->status->param.max_rec)
#define SLP_SZ_REC( x)			( x->status->param.rec_size)
#define SLP_GET_REC( x, y)		( ( SLP_RECORD *)( ( size_t)x->rec + ( ( size_t)SLP_SZ_REC( x) * ( size_t)y)))

typedef struct {
    char    type        [ 2];   /* FA,FB SWAP rate 구분 위해                                            */
    char    excode      [ 1];   /* SMB/KMB/EBS/CMB/BEST/ZCUST/MATCHING                                  */
    char    bidex       [ 1];   /* BID원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
    char    askex       [ 1];   /* ASK원천 : S:SMB, K:KMB, E:EBS, C:CMB                                 */
    char    symb        [ 7];   /* root symbol                                                          */
    char    id          [32];   /* 호가 id                                                              */
    char    date        [ 8];   /* 수신일자 YYYYMMDD (서버시간)                                         */
    char    time        [ 9];   /* 수신시간 HHMMSSSSS                                                   */
    double  usdbid          ;   /* Current USDKRW BID                                                   */
    double  usdask          ;   /* Current USDKRW OFFER                                                 */
    double  bidprc          ;   /* Price of the MarketData Entry                                        */
    double  askprc          ;   /* Price of the MarketData Entry                                        */
    double  bidqty          ;   /* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
    double  askqty          ;   /* Quantity of the MarketData Entry. Always “0” For USD/KRW, CNH/KRW  */
    double  midprc          ;   /* 중간가                                                               */
    double  fillprc         ;   /* 체결가                                                               */
    time_t  ctime           ;   /* time_t convert                                                       */
    time_t  price_time      ;   /* 시세 유효 시간                                                       */
    char    filler     [112];   /* 256 byte 맞춤                                                        */
} SLP_SISE;

typedef struct _slp_current_
{
	int			num;							/* 내부 사용 번호 */
	char		str[ 8];						/* 화폐 표현 string */
	int			point;							/* 소숫점 */
	char		comment[ 32];					/* 설명 */
}	SLP_CURR;

typedef struct _slp_pair_
{
	int			num;
	int			base;
	int			cont;
	int			point;
	int			unit;
	char		symbol[ 8];
	char		comment[ 64];
}	SLP_PAIR;

typedef struct _slp_param_
{
	key_t		key;							/* shared memory, semaphore key */
	int			max_rec;						/* max data count */
	int			rec_size;						/* data record size */
	int			max_idx;						/* max index count */
	int			idx_size;						/* index size */
	int			stat_size;						/* status size */
	size_t		size;							/* shared memory total size */
	char		cfg_name[ 512];					/* config file path */
	char		mat_path[ 512];					/* matching fifo path */
}	SLP_PARAM;

typedef struct _slp_statistics_
{
	int				id;             			/* 통계 id ... ex) pid_t */
	int				cur;            			/* current */
	int				cnt;            			/* count */
	int				max;            			/* 최대값 */
	int				min;            			/* 최소값 */
	int				avr;            			/* average */
	int				tot;            			/* total */
	struct timeval	srt;            			/* start time */
	struct timeval	end;            			/* end time */
}   SLP_STATIS;

typedef struct _slp_status_
{
	SLP_PARAM	param;							/* slp parameter - from config file */
	int			service;						/* service on/off - 0=off,1=on */
	int			idx_pos;						/* index add position */
	int			idx_cnt;						/* index count */
	int			dat_pos;						/* data add position */
	int			dat_cnt;						/* data count */
	SLP_CURR	curr[ SLP_MAX_CURR];			/* 통화 정보 */
	SLP_STATIS	statis[ SLP_MAX_STATIS];		/* 통계 data */
}	SLP_STATUS;

typedef struct _slp_index_
{
	int			pos;							/* current index position */
	int			start[ 2];						/* index start position 0=bid, 1=ask */
	int			cnt[ 2];						/* index member count 0=bid, 1=ask */
	SLP_PAIR	pair;							/* 통화 거래정보 */
	SLP_SISE	sise[ 3];						/* 시세정보 0=curr,1=base,2=cont */
}	SLP_INDEX;

typedef struct _slp_head_
{
	int			pos;							/* current record position */
	int			stat;							/* 0-empty, 1-order, 2-execute, 3-wait ... */
	int			prev;							/* prev data position start=0 */
	int			next;							/* next data position end=0 */
}	SLP_HEAD;

typedef struct _slp_rec_
{
	char		data[ 512];
}	SLP_DATA;

typedef struct _dpl_data_
{
	int			pos;
	SLP_HEAD	head;
	SLP_DATA	data;
}	SLP_RECORD;

typedef struct _dpl_
{
	MEM				*mem;						/* shared memory pointer */
	SEM				*sem;						/* semaphore pointer */
	int				mat_fd;						/* matching pipe fd */
	void			*base;						/* shared memory attrch pointer */
	SLP_STATUS		*status;					/* status - pos, count ... etc */
	SLP_INDEX		*index;						/* index - srt,end ... etc */
	SLP_RECORD		*rec;						/* data - head + record */
}	SLP;

#endif

/***** Module : slp.c *****/
SLP*        Slp_GetSlp();                                                   /* slp 구조체 malloc 및 초기화 */
int         Slp_SetPtr( SLP *slp);                                          /* slp 구조체 malloc 및 초기화 */
SLP_PARAM*  Slp_GetConfig( char *cfg_name);                                 /* config file의 변수를 slp 구조체로 load */
SLP*        Slp_Create( char *cfg_name);                                    /* slp 사용을 위한 초기 작업 */
int         Slp_Remove( SLP *slp);                                          /* slp 사용을 위한 초기 작업 */
SLP*        Slp_Open( key_t key, int opt);                                  /* slp 사용을 위한 초기 작업 */
int         Slp_Close( SLP *slp);                                           /* slp  */
int         Slp_Init( SLP *slp, char *cfg_name);                            /* slp initial - 공유 메모리 생성후 초기치 저장 */
int         Slp_Lock( SLP *slp);                                            /* slp  */
int         Slp_Unlock( SLP *slp);                                          /* slp  */
int         Slp_Stat( SLP *slp);                                            /* slp  */

/***** Module : slp_sub.c *****/
int         Slp_LoadCurr( SLP *slp, char *cfg_name);                        /* slp_sub library */
SLP_CURR*   Slp_LoadCurrSub( SLP *slp, char *line);                         /* slp_sub library */
int         Slp_GetCurrInt( SLP *slp, char *str);                           /* 통화 string(3 byte)을 통화 int로 변환 */
int         Slp_LoadPair( SLP *slp, char *cfg_name);                        /* slp_sub library */
SLP_PAIR*   Slp_LoadPairSub( SLP *slp, char *line);                         /* slp_sub library */
int         Slp_SiseConvert( SLP_SISE *dest, SLP_SISE *orig);               /* convert ORDER to ORDER_SEND */

/***** Module : slp_stat.c *****/
int         Slp_StatisticsSet( SLP *slp, int pos, int opt);                 /* 통계 setting */
int         Slp_TimeGap( SLP *slp, struct timeval *tv_1, struct timeval *tv_2);/* 시간 차이 구하기 */
int         Slp_StatisReset( SLP *slp);                                     /* 통계 초기화 */
int         Slp_PrintStic( SLP *slp);                                       /* 통계 출력 */
int         Slp_StatCurr( SLP *slp);                                        /* slp_sub library */
int         Slp_StatIndex( SLP *slp);                                       /* slp_sub library */

/***** Module : slp_sise.c *****/
int         Slp_Sise( SLP *slp, int base, int cont, SLP_SISE *sise);        /* 시세처리 - 재정,기준/상대 통화 관련된 시세 UPDATE */
int         Slp_SiseUpdate( SLP *slp, SLP_INDEX *index, SLP_SISE *sise, int type);/* 이종통화 시세 계산  */
double      Slp_Round( SLP *slp, double value, double point);               /* 신규 주문 매칭 */

/***** Module : print.c *****/
int         SLP_PARAM_Print( SLP_PARAM* ptr);
int         SLP_STATUS_Print( SLP_STATUS* ptr);
int         SLP_PAIR_Print( SLP_PAIR* ptr);
int         SLP_INDEX_Print( SLP_INDEX* ptr);
int         SLP_HEAD_Print( SLP_HEAD* ptr);
int         SLP_DATA_Print( SLP_DATA* ptr);
int         SLP_RECORD_Print( SLP_RECORD* ptr);
int         SLP_Print( SLP* ptr);



