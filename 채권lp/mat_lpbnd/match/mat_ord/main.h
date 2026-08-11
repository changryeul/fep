/** ***************************************************************************
**  @file       main.h
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  메인 모듈 관련 헤더
**  프로그램 사용 파라메터 정의
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "map.h"
#include "mat.h"

#include "order.h"

#ifndef MAIN_H
#define	MAIN_H	1

extern CFG		*Cfg;
extern MAP		*Map;
extern int		Continue;

typedef struct _param_
{
	int		argc;						/** @var command line argument count */
	char	**argv;						/** @var command line argument */
	char	argo[ 32];					/** @var argument option buffer */
	int		args;						/** @var argument option size */
	char 	cfg_name[ 512];				/* config file name */
	char	log_name[ 512];				/* log file name */
	int		log_flag;					/* 0-console 1-file 2-mem */
	int		log_level;					/* log_level */
	char	sq_name[ 32];				/* send execute queue name */
	char	rq_name[ 32];				/* recv order queue name */
	int		interval;					/* loop interval */
	int		timeout;					/* recv timeout */
}	PARAM;
extern PARAM	*Param;

#endif	/* MAIN_H */

/***** Module : lib *****/
int f_get_cust_prc( SPLIT_IN_ST *p_in, SPLIT_OUT_ST *p_out, MAT_E_MSG *msg);


/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

/***** Module : ../convert.c *****/
int         Proc_OrderConvert( ORDER *order, ORDER_RECV *recv);             /* convert ORDER_RECV to ORDER */
int         Proc_SendConvert( ORDER_SEND *send, ORDER *order);              /* convert ORDER to ORDER_SEND */

/***** Module : proc.c *****/
int         Proc_Order( char *data, int sz);                                /* 주문 정합 check */
int         Proc_OrderPreCheck( ORDER *order);                              /* 주문 사전 check */
int         Proc_OrderCheck( ORDER *order);                                 /* 주문 check */
int         Proc_OrderReject( ORDER *order, int rej_code, char *msg);       /* 거부 setting */
int         Proc_ExecuteSend( ORDER_SEND *send);                            /* 거부 SMQ send */

/***** Module : mat_order.c *****/
int         Mat_Order( MAT *mat, ORDER *obook, int opt);                    /* 주문 insert,update,delete - mat_rcv main */
int         Mat_RecordInsert( MAT *mat, MAT_INDEX *index, ORDER	 *obook, int jang_id);/* 주문 insert  */
int         Mat_RecordDelete( MAT *mat, MAT_INDEX *index, ORDER	 *obook);   /* 주문 delete  */
int         Mat_RecordInsertGroup( MAT *mat, MAT_INDEX *index, ORDER *obook, int jang_id, int opt);/* 그룹주문 insert  */
int         Mat_JangCheckOrder( MAT *mat, MAT_INDEX *index, ORDER *order);  /* 원주문번호와 일치하는 주문 찾기 */
int         Mat_CheckFee( MAT *mat, MAT_INDEX *index, ORDER	 *obook);       /* 주문 delete  */
int         Mat_CheckFeeFar( MAT *mat, MAT_INDEX *index, ORDER	 *obook);    /* 주문 delete  */
int         Mat_CustCp( MAT *mat, SPLIT_IN_ST *pee_in, ORDER	 *obook);      /* 고객번호 CustID(16) + FundNO(12)를 수수료 pee_in->s_csac_idnt_no로 copy */

/***** Module : mat_fee.c *****/
double      Mat_FeeProcess( MAT *mat, MAT_INDEX *index, int side, int pos); /* 일반주문 수수료 계산 lib call */
int         Mat_CheckFee( MAT *mat, MAT_INDEX *index, ORDER *obook);        /* 일반주문 수수료 계산 lib call */
double      Mat_GetFee( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* 일반주문 수수료 계산 lib call */
double      Mat_GetFeeSwap( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP 주문 수수료 계산 lib call */
double      Mat_GetFeeNear( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */
double      Mat_GetFeeFar( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */
int         Mat_SetFeeSise( MAT *mat, MAT_INDEX *index, SPLIT_IN_ST *fee_in);/* SWAP NEAR 주문 수수료 계산 lib call */
int         Mat_CustCp( MAT *mat, SPLIT_IN_ST *fee_in, ORDER	 *obook);      /* 고객번호 CustID(16) + FundNO(12)를 수수료 fee_in->s_csac_idnt_no로 copy */
int         Mat_SetFeeOrder( MAT *mat, ORDER *obook, SPLIT_IN_ST *fee_in, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */

