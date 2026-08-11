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
#include "smq.h"

#include "order.h"

#ifndef PROCESS_H
#define	PROCESS_H	1

extern CFG		*Cfg;
extern MAP		*Map;
extern int		Continue;

typedef struct _mat_order_
{
	char	recv[ 8192];			/* receive buffer */
	int		rsz;					/* receive size */
	char	send[ 8192];			/* send buffer */
	int		ssz;					/* send size */
	MAT		*mat;					/* 매칭엔진 pointer */
	SMQ		*smq_send;				/* smq pointer for send */
	SMQ		*smq_recv;				/* smq pointer for recv */
}	MAT_ORDER;

#endif	/* PROCESS_H */

/***** Module : print.c *****/
int         ORDER_RECV_Print( ORDER_RECV* ptr);
int         ORDER_SEND_Print( ORDER_SEND* ptr);
int         ORDER_Print( ORDER* ptr);

/***** Module : process.c *****/
int         Proc_Execute( MAT_ORDER *mo, int timeout);                      /* 주문 정합 check */
int         Proc_Match( MAT *mat, FX_QUOTE_T *sise);                        /* 주문 정합 check */
int         Proc_Order( MAT_ORDER *mo);                                     /* 주문 정합 check */
int         Proc_OrderPreCheck( ORDER *order);                              /* 주문 사전 check */
int         Proc_OrderCheck( ORDER *order);                                 /* 주문 check */
int         Proc_OrderReject( MAT_ORDER *mo, ORDER *order, int rej_code);   /* 주문 check */
int         Proc_ExecuteSend( MAT_ORDER *mo, ORDER_SEND *send);             /* 주문 check */
int         Proc_OrderConvert( ORDER *order, ORDER_RECV *recv);             /* 주문 정합 check */
int         Proc_SendConvert( ORDER_SEND *send, ORDER *order);              /* 주문 정합 check */

