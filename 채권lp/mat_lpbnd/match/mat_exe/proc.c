/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "mat.h"
#include "order.h"
#include "sise.h"
#include "main.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;
#if 0
extern MAT_ORDER	*MatOrder;
#endif
extern MAT			*Mat;
extern SMQ			*Smq;
extern PARAM		*Param;

/** ***************************************************************************
**	
***************************************************************************** */
/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 정합 check
***************************************************************************** */
int Proc_Execute()
{
	int			rtn;
	int			pos = -1;
	char		*ptr;
	ORDER		*order;
	MAT_RECORD	*rec;
	MAT_HEAD	*head;
	ORDER_SEND	_send, *send = &_send;
	int			timeout = Param->timeout;

	/* 체결 ID */
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;
	char		exe_id[ 128];
	char		time_buf[ 32];

	pos = Mat_GetConform( Mat);
	if( pos > 0)
	{
		LogMsg( "주문확인 대기 체결 처리. pos=[%d]", pos);
	}
	/* 주문확인 대기 체결이 있을경우 pipo timeout을 0.1초로 처리 */
	if( Mat->map->conform_cnt > 0) timeout = 10000;

	/* 주문확인 대기가 없을때는 정상 Process */
	if( pos <= 0)
	{
		pos = Mat_Execute( Mat, timeout);
		if( pos <= 0)
		{
			if( pos != MAT_TIMEOUT) 
			{
				return pos;
			}
			LogDel( "Mat_Execute timeout ... timeout = [%d.%d]", timeout / 1000000, timeout % 1000000);
			return MAT_TIMEOUT;
		}
	}

	Mat_StatisticsSet( Mat, MAT_STAT_MAT, MAT_STAT_START);

	rec  = &Mat->map->rec[ pos];
	head = &rec->head;

	if( Param->log_level < 5)	MAT_HEAD_Print( head);

	if( head->con_time.tv_sec == 0)
	{
		LogMsg( "주문확인전송까지 대기합니다. pos=[%d] conform_cnt=[%d]", pos, Mat->map->conform_cnt);
		rtn = Mat_PutConform( Mat, pos);
		if( rtn < 0)
		{
			LogCri( "체결 저장 오류 pos=[%d]", pos);
		}
		return 0;
	}

	LogDbg( "체결처리 pos=[%d] error=[%d]", pos, head->error);
	order = ( ORDER *)&Mat->map->rec[ pos].ord;

	/* 체결 ID DB Write */
	rtn = Db_TRG005LInsert( rec);
	if( rtn <= 0)
	{
		LogCri( "Db_TRG005LInsert error. rtn=[%d]", rtn);
		head->error = 82103;
	}

	if( head->error != 0)
	{
		head->ord_stat = 4;

		/* 체결 에러의 경우는 강제취소로 setting하여 처리 */
		LogMsg( "체결에러 error=[%d]", head->error);

		AtoA( order->OrigClOrdID,   order->ClOrdID,   sizeof( order->OrigClOrdID));
		AtoA( order->ClOrdID,       " ",              sizeof( order->ClOrdID));

		ItoA( order->RefuslCd, head->error, sizeof( order->RefuslCd));
		ptr = Mat_MessageGet( Mat, head->error);
		if( ptr == NULL)
		{
			/* 에러 메시지와 코드는 에러 발생 단계에서 처리 - 엔진에러가 아닌경우(수수료 처리) */
			AtoA( order->Text, "알수없는 에러", sizeof( order->Text));
		}
		else
		{
			AtoA( order->Text, ptr, sizeof( order->Text));
		}
		switch( head->error)
		{
			case 82030:	/* 고객정보가 없습니다. */
				AtoA( order->MsgType,   "8", sizeof( order->OrdStatus));
				AtoA( order->OrdStatus, "4", sizeof( order->OrdStatus));
				AtoA( order->ExecType,  "4", sizeof( order->OrdStatus));
				break;
			default:
				AtoA( order->MsgType,   "8", sizeof( order->OrdStatus));
				AtoA( order->OrdStatus, "4", sizeof( order->OrdStatus));
				AtoA( order->ExecType,  "4", sizeof( order->OrdStatus));
				break;
		}
	}
	else
	{
		head->ord_stat = 2;

		/* 메세지유형 */
		AtoA( order->MsgType,  "8", sizeof( order->MsgType));
		/* 주문상태 */
		AtoA( order->OrdStatus, "2", sizeof( order->OrdStatus));
		/* 거래유형 */
		AtoA( order->ExecType, "2", sizeof( order->ExecType));

		/* 체결 ID */
		time( &cur_time);
		localtime_r( &cur_time, tp);
		sprintf( exe_id, "2%04d%02d%02d%02d%02d%02d%06d",
				tp->tm_year +1900,
				tp->tm_mon +1,
				tp->tm_mday,
				tp->tm_hour,
				tp->tm_min,
				tp->tm_sec,
				pos);
		AtoA( order->ExecID, exe_id, sizeof( order->ExecID));
		LogDbg( "exe_id       =[%s]", exe_id);

		/* 체결가 */
		order->LastPx = head->exe_price;
		LogDbg( "exe_price    =[%15f]", order->LastPx);

		/* 체결 수량 */
		order->CumQty = order->OrderQty;
		order->LastQty = order->OrderQty;

		/* 고객 스프레드 */
		if( order->TrdTypeDcd[ 0] == '3') /* 대행 및 RFQ일 경우 고객스프레드를 고객 체결 마진으로 */
		{
			order->LastSprPx = order->PriceSpr;
		}
		else
		{
			order->LastSprPx = head->fee_out.rec[ 0].d_cus_spr;
		}

		/* 체결 시간을 주문시간 TransactTime에 update - gmt time */
		gmtime_r( &cur_time, tp);
		memset( time_buf, 0x00, sizeof( time_buf));
		sprintf( time_buf, "%04d%02d%02d-%02d:%02d:%02d:%02d",
				tp->tm_year + 1900, tp->tm_mon +1, tp->tm_mday,
				tp->tm_hour, tp->tm_min, tp->tm_sec, 0);
		memcpy( order->TransactTime, time_buf, strlen( time_buf));


		LogDel( "price=[%f] exe_price=[%f]", order->Price, order->LastPx);
		/*
		DtoA0ND( &order->LastPx2, head->exe_price, sizeof( order->LastPx2), 4);
		LogDbg( "price=[%f]", head->exe_price);
		LogDbg( "order->LastPx2=[%*.s]", sizeof( order->LastPx2), &order->LastPx2);
		*/

		/*
		LogDbg( "ORDER_Print order=[%p]", order);
		ORDER_Print( order);
		*/
	}

	rtn = Proc_SendConvert( send, order);

	/*
	LogDbg( "ORDER_Print order=[%p]", order);
	ORDER_Print( order);
	LogDbg( "ORDER_SEND__Print send=[%p]", send);
	*/
	ORDER_SEND_Print( send);

	retry:
	rtn = Smq_Send( Smq, ( char *)send, sizeof( ORDER_SEND));
	if( rtn < 0)
	{
		if( rtn == SMQ_TIMEOUT) /* 빈공간이 없을때 */
		{
			sleep( 1);
			goto retry;
		}
		LogCri( "Smq_Send error. smq=[%p] size=[%d]", Smq, sizeof( ORDER_SEND)); 
		return -1;
	}

	gettimeofday( &head->snd_time, NULL);
	head->gubun = 0;
	Mat_StatisticsSet( Mat, MAT_STAT_MAT, MAT_STAT_END);
	/*
	memset( rec, 0x00, sizeof( MAT_RECORD));
	*/

	return 1;
}


