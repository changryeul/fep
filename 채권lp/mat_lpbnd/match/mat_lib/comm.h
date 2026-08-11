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
#include "smq.h"

#ifndef COMM_H
#define	COMM_H		1

typedef struct _mat_comm_head_
{
	char	len		[ 4];			/* 자신 제외 전체 size */
	char	type	[ 4];			/* LINK|LIOK|ERCD|HTBT|HTOK|DATA|DAOK|STOP */
	char	seq		[ 8];			/* 순번 */
	char	block	[ 2];			/* block count */
	char	code	[ 2];			/* error code */
	char	date	[20];			/* YYYYMMDDssssss */
	char	filler	[10];			/* filler */
}	MAT_COMM_HEAD;

typedef struct _mat_data_head_
{
	char	len		[10];			/* data length */
	char	type	[ 5];			/* LOGIN|REJE|IF01 ... */
	char	seq		[10];			/* sequence */
	char	code	[ 5];			/* 응답코드 */
	char	date	[20];			/* */
}	MAT_DATA_HEAD;

typedef struct _mat_packet_
{
	MAT_COMM_HEAD	comm_head;
	MAT_DATA_HEAD	data_head;
	char			data[ 8192];
}	MAT_PACKET;

#endif	/* COMM_H */


