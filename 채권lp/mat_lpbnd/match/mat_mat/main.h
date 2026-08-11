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
#include "sise.h"

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
	char	cfg_name[ 512];				/** cfg file name */
	char	log_name[ 512];				/** log file name */
	int		log_flag;					/** log flag */
	int		log_level;					/** log level */
	char	addr[ 32];					/** sise server addr */
	int		port;						/** sise server port */
	int		interval;					/** main loop interval */
	int		timeout;					/** wait timeout */
}	PARAM;
extern PARAM	*Param;

typedef struct _my_data_
{
	time_t		cur_time;
	int			alarm;
}	MY_DATA;
extern MY_DATA	*MyData;

#endif	/* MAIN_H */

/***** Module : ----- *****/
double l_nfloor (double d_val , int n_digit);

/***** Module : print.c *****/
int         ORDER_RECV_Print( ORDER_RECV* ptr);
int         ORDER_SEND_Print( ORDER_SEND* ptr);
int         ORDER_Print( ORDER* ptr);

/***** Module : main.c *****/
int         main( int argc, char *argv[]);                                  /* GetOption   - 인수 분석 및 변수 초기화 */
int         GetOption( int argc, char *argv[]);                             /* 프로그램 시작시 받은 인수를 분석, 프로그램에서 사용할 변수를 초기화 한다. */
void        SignalProcess( int sig_id);                                     /* 프로그램에서 사용할 시그널 */
int         InitProcess( int argc, char *argv[]);                           /* 프로그램 초기화 */
int         MainProcess( int argc, char *argv[]);                           /* 프로그램 수행 */
int         TermProcess( int argc, char *argv[]);                           /* 프로세스 종료 루틴 수행 */
int         ParamPrint( PARAM *param);                                      /* 프로그램에서 사용할 전역변수들의 구조체 값 표시 */

/***** Module : proc.c *****/
int         Proc_Timeout( MAT *mat);                                        /* 신규주문 매칭 */
int         Proc_Match( MAT *mat, APSISE *sise);                            /* 매칭 */
int         Proc_MatchPipe( MAT *mat, int pos);                             /* 매칭 */
int         Proc_OrderConvert( ORDER *order, ORDER_RECV *recv);             /* 주문 정합 check */
int         Proc_SendConvert( ORDER_SEND *send, ORDER *order);              /* 주문 정합 check */

/***** Module : mat_match.c *****/
int         Mat_Matching( MAT *mat, int idx_pos);                           /* index에 등록된 주문을 검색하여 매칭 */
int         Mat_MatchRecord( MAT *mat, MAT_INDEX *index, int side, int pos);/* 가격비교하여 매수/매도체결 */
int         Mat_MatchExecute( MAT *mat, MAT_INDEX *index, MAT_RECORD *rec, int pos, int side, int base_pos, int group_check);/* 매칭 체결 처리 */
int         Mat_MatchTimeout( MAT *mat);                                    /* 신규 주문 매칭 */
int         Mat_JangMatch( MAT *mat, MAT_INDEX *index, int pos);            /* 신규 주문 매칭 */
double      Mat_Round( MAT *mat, double value, double point);               /* 신규 주문 매칭 */
double      Mat_GetFee( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* 일반주문 수수료 계산 lib call */
double      Mat_GetFeeSwap( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP 주문 수수료 계산 lib call */
double      Mat_GetFeeNear( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */
double      Mat_GetFeeFar( MAT *mat, MAT_INDEX *index, int side, int pos, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */
int         Mat_SetFeeSise( MAT *mat, MAT_INDEX *index, SPLIT_IN_ST *fee_in);/* SWAP NEAR 주문 수수료 계산 lib call */
int         Mat_CustCp( MAT *mat, SPLIT_IN_ST *fee_in, ORDER	 *obook);      /* 고객번호 CustID(16) + FundNO(12)를 수수료 fee_in->s_csac_idnt_no로 copy */
int         Mat_SetFeeOrder( MAT *mat, ORDER *obook, SPLIT_IN_ST *fee_in, int opt);/* SWAP NEAR 주문 수수료 계산 lib call */

