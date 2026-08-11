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
#include "smq.h"
#include "order.h"
#include "sise.h"
#include "main.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;
extern MAT			*Mat;
extern SMQ			*SmqSend;
extern SMQ			*SmqRecv;

extern MAT_REJECT	MatReject[];

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
int Proc_Order( char *data, int sz)
{
	int			rtn, rej_code;
	int			pos, grp_seq;
	int			order_opt = 0;
	ORDER		_order, *order = &_order;
	ORDER_SEND	_send, *send = &_send;
	ORDER_RECV	*recv = ( ORDER_RECV *)data;

	MAT_RECORD	*rec;

	/* for OrdID */
	time_t		cur_time;
	struct tm	_tp, *tp = &_tp;
	int			id_sz;
	char		ord_id[ 128];

	Mat_StatisticsSet( Mat, MAT_STAT_RCV, MAT_STAT_COUNT);
	Mat_StatisticsSet( Mat, MAT_STAT_ORD, MAT_STAT_START);
	Mat_StatisticsSet( Mat, MAT_STAT_CNF, MAT_STAT_START);

	/* 주문 convert and write */
	rtn = Proc_OrderConvert( order, recv);
	ORDER_Print( order);

	/* 주문 사전 check - field 값 check */
	rej_code = Proc_OrderPreCheck( order);
	if( rej_code < 0)
	{
		LogCri( "주문 정합 error.");
		LogCri( "주문 거부 처리.");
		rtn = Proc_OrderReject( order, rej_code, NULL);
		if( rtn < 0)
		{
			LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
			return -1;
		}
		return rej_code;
	}

	/* 주문타입 check */
	switch( order->TrdTypeDcd[ 0])
	{
		case '1':	/* 일반주문 */
			break;
		case '2':	/* SPOT 고정 */
		case '3':	/* 대행 및 RFQ */
			/*
			if( !memcmp( order->SettType, "SWP", 3))
			{
				rej_code = -82007;
				rtn = Proc_OrderReject( order, rej_code, "주문 구분 미지원. SWAP 주문은 SPOT 고정 및 대행주문이 없습니다.");
				if( rtn < 0)
				{
					LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
					return -1;
				}
				return rej_code;
			}
			*/
			break;
		default:
			rej_code = -82007;
			rtn = Proc_OrderReject( order, rej_code, "주문 구분 미지원. 주문타입 오류입니다.");
			if( rtn < 0)
			{
				LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
				return -1;
			}
			return -rej_code;

	}

	/* 그룹주문 check */
	grp_seq = AtoI( order->GrpOrdnSeq, sizeof( order->GrpOrdnSeq));
	LogDbg( "Group order ... grp_seq=[%d]", grp_seq);


	switch( order->MsgType[ 0])
	{
		case 'D':	/* 신규 */
			if( grp_seq <= 0)		order_opt = MAT_INSERT;
			else					order_opt = MAT_INSERT_GROUP;
			pos = Mat_Order( Mat, order, order_opt);
			if( pos < 0)
			{
				LogCri( "주문 거부 처리.");
				rej_code = pos;
				rtn = Proc_OrderReject( order, rej_code, NULL);
				if( rtn < 0)
				{
					LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
					return -1;
				}
				return rej_code;
			}
			rec = Mat_GetRecordByPos( Mat, pos);
			break;
		case 'G':	/* 정정 */
			pos = Mat_Order( Mat, order, MAT_UPDATE);
			if( pos < 0)
			{
				LogCri( "주문 거부 처리.");
				rej_code = -82003;
				rtn = Proc_OrderReject( order, rej_code, NULL);
				if( rtn < 0)
				{
					LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
					return -1;
				}
				return rej_code;
			}
			rec = Mat_GetRecordByPos( Mat, pos);
			break;
		case 'F':	/* 취소 */
			pos = Mat_Order( Mat, order, MAT_DELETE);
			if( pos < 0)
			{
				LogCri( "주문 거부 처리.");
				rej_code = -82003;
				rtn = Proc_OrderReject( order, rej_code, NULL);
				if( rtn < 0)
				{
					LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
					return -1;
				}
				return rej_code;
			}
			/* 취소주문은 record가 이미 삭제가 되어 있으므로 수신한 record 사용 */
			rec = NULL;
			break;
		default:
			break;
	}
	Mat_StatisticsSet( Mat, MAT_STAT_ORD, MAT_STAT_END);

	if( pos < 0)
	{
		LogCri( "주문 거부 처리.");
		rtn = Proc_OrderReject( order, pos, NULL);
		if( rtn < 0)
		{
			LogCri( "주문 거부 처리 Error. rtn=[%d]", rtn);
			return -1;
		}
		return rej_code;
	}

	if( rec != NULL) order = ( ORDER *)&rec->ord;
	LogDbg( "주문번호=[%.*s]", sizeof( order->OrdID), order->OrdID);

	/******************/
	/* 주문 접수 전송 */
	/******************/
	/* convert */
	rtn = Proc_SendConvert( send, order);
	if( rtn < 0)
	{
		LogCri( "Proc_SendConvert fail.");
		return -1;
	}

	/* 주문확인 - 메세지유형=[8] */
	memcpy( send->MsgType, "8", sizeof( send->MsgType));
	switch( order->MsgType[ 0])
	{
		default:
		case 'D':		/* new order */
			memcpy( send->OrdStatus, "0", sizeof( send->OrdStatus));
			memcpy( send->ExecType,  "0", sizeof( send->OrdStatus));
			break;
		case 'G':		/* 정정 */
			memcpy( send->OrdStatus, "5", sizeof( send->OrdStatus));
			memcpy( send->ExecType,  "5", sizeof( send->OrdStatus));
			break;
	#if 0
			memcpy( send->ExecType, "E", sizeof( send->OrdStatus));
			break;
	#endif
		case 'F':		/* 취소 */
			memcpy( send->OrdStatus, "4", sizeof( send->OrdStatus));
			memcpy( send->ExecType,  "4", sizeof( send->OrdStatus));
			/* 신규/정정은 insert시 주문번호가 생성되지만 삭제는 생성되지 않으므로 여기서 부여 */
			/* record가 생성 되지 않았으르로 position(PPPPPP)은 0(pos=0) 으로 */
			/* 접수번호 OrdID 생성 YYYYMMDDHHMMSSPPPPPP */
			time( &cur_time);
			localtime_r( &cur_time, tp);
			id_sz = sprintf( ord_id, "1%04d%02d%02d%02d%02d%02d%06d",
					tp->tm_year +1900,
					tp->tm_mon +1,
					tp->tm_mday,
					tp->tm_hour,
					tp->tm_min,
					tp->tm_sec,
					0 /* pos */);
			memcpy( send->OrdID, ord_id, id_sz);
			LogDbg( "ord_id       =[%s]", ord_id);
			LogDbg( "order->OrdID =[%.*s]", sizeof( send->OrdID), send->OrdID);
			break;
			
	}
#if 0
	ORDER_SEND_Print( send);
#endif

	rtn = Proc_ExecuteSend( send);
	if( rtn < 0)
	{
		LogCri( "주문확인전송 실패.");
		ORDER_SEND_Print( send);
		return -1;
	}
	Mat_StatisticsSet( Mat, MAT_STAT_CNF, MAT_STAT_END);

	if( rec != NULL) Mat_SetHead( Mat, rec, MAT_HEAD_CONFORM);


	return 1;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  주문 사전 check
***************************************************************************** */
int Proc_OrderPreCheck( ORDER *order)
{
	int		msg_len;
	int		i = 0;

	/* 주문 타입 - D-신규,G-정정,F-취소 */
	switch( order->MsgType[ 0])
	{
		case 'D':	/* 신규 */
			break;
		case 'G':	/* 정정 */
		case 'F':	/* 취소 */
			if( order->OrigClOrdID[ 0] == ' ' || order->OrigClOrdID[ 0] == 0)
			{
				LogCri( "정정/취소 주문에 원주문 ID가 없습니다. order->OrigClOrdID=[%.*s]", 
						sizeof( order->OrigClOrdID), order->OrigClOrdID);
				ItoA( order->RefuslCd, 82003, sizeof( order->RefuslCd));
				msg_len = sprintf( order->Text, "%s", "정정/취소 주문에 원주문 ID가 없습니다.");
				order->Text[ msg_len] = ' ';
				return -82003;
			}
			break;
		default:
			LogCri( "주문타입 에러 ... order->MsgType=[%c]", order->MsgType[ 0]);
			ItoA( order->RefuslCd, 82001, sizeof( order->RefuslCd));
			msg_len = sprintf( order->Text, "%s", "주문타입은 신규/정정/취소만 가능합니다.");
			order->Text[ msg_len] = ' ';
			return -82001;
	}

	/* 매매구분 - (1-BUY(Sell &   Buy), 2-SELL(Buy & Sell)) */
	switch( order->Side[ 0])
	{
		case '1':	/* BUY  */
		case '2':	/* SELL */
			break;
		default:
			LogCri( "매매구분 에러 1=BUY,2=SELL ... order->Side=[%c]", order->Side[ 0]);
			ItoA( order->RefuslCd, 82004, sizeof( order->RefuslCd));
			msg_len = sprintf( order->Text, "%s", "매매구분은 BUY/SELL만 가능합니다.");
			order->Text[ msg_len] = ' ';
			return -82004;
	}

	/* 주문수량 */
	if( order->OrderQty <= 0.00D)
	{
		LogCri( "주문수량 에러 [주문수량을 확인해주세요] ... order->qty=[%15.4f]", order->OrderQty);
		ItoA( order->RefuslCd, 82005, sizeof( order->RefuslCd));
		msg_len = sprintf( order->Text, "%s", "주문수량을 확인해주세요.");
		order->Text[ msg_len] = ' ';
		return -82005;
	}
	LogDbg( "order->OrderQty=[%15.4f]", order->OrderQty);

	/* 주문유형 - (1-시장가,2-지정가,3-예약주문 */
	switch( order->OrdType[ 0])
	{
		case '1':	/* 시장가   */
			break;
		case '2':	/* 지정가   */
		case '3':	/* 예약주문 */
			if( !memcmp( order->SettType, "SWP", 3))
			{
				ItoA( order->RefuslCd, 82006, sizeof( order->RefuslCd));
				LogErr( "SWAP주문은 시장가만 가능합니다.");
				msg_len = sprintf( order->Text, "%s", "SWAP주문은 시장가만 가능합니다.");
				order->Text[ msg_len] = ' ';
				return -82006;
			}
			break;
		default:
			LogCri( "주문유형 에러 ... order->OrdType=[%c]", order->OrdType[ 0]);
			ItoA( order->RefuslCd, 82006, sizeof( order->RefuslCd));
			msg_len = sprintf( order->Text, "%s", "주문유형은 시장가/지정가/예약주문만 가능합니다.");
			order->Text[ msg_len] = ' ';
			return -82006;

	}

	/* 상품구분 - 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SWAP,8-MAR */
	i = 0;
	if( !memcmp( order->SettType, "SPT", 3))		i = 1;
	else if( !memcmp( order->SettType, "FWD", 3))	i = 2;
	else if( !memcmp( order->SettType, "SWP", 3))	i = 3;
	else if( !memcmp( order->SettType, "BAR", 3))	i = 1;
	if( i == 0)
	{
		LogCri( "상품구분 에러 [주문 구분 미정의] ... order->SettType=[%.3s]", order->SettType);
		ItoA( order->RefuslCd, 82020, sizeof( order->RefuslCd));
		msg_len = sprintf( order->Text, "%s", "상품 구분 미지원");
		order->Text[ msg_len] = ' ';
		return -82020;
	}

	return 1;
}
	
/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  거부 setting
***************************************************************************** */
int Proc_OrderReject( ORDER *order, int rej_code, char *msg)
{
	int			rtn;
	ORDER_SEND	_send, *send = &_send;
	int			tot = 0, seq = 0;
	int			msg_len = 0;

	int			base_cur, cont_cur;
	MAT_INDEX	*index;

	Mat_StatisticsSet( Mat, MAT_STAT_REJ, MAT_STAT_START);

	if( msg == NULL)
	{
		Mat_SetMessage( Mat, order, -rej_code);
	}
	else
	{
		ItoA( order->RefuslCd, -rej_code, sizeof( order->RefuslCd));
		msg_len = sprintf( order->Text, "%s", msg);
		order->Text[ msg_len] = ' ';
	}

	rtn = Proc_SendConvert( send, order);

	/* 주문상태 set */
	send->MsgType[ 0] = '8';
	send->OrdStatus[ 0] = '8';
	send->ExecType[ 0] = '8';

	ORDER_SEND_Print( send);

	rtn = Proc_ExecuteSend( send);
	if( rtn < 0)
	{
		LogCri( "Proc_ExecuteSend error. rtn=[%d]", rtn);
		ORDER_SEND_Print( send);
		return -1;
	}
	Mat_StatisticsSet( Mat, MAT_STAT_REJ, MAT_STAT_END);

	/* 그룹주문 마지막 checck */
	tot = AtoI( order->GrpOrdnCnt, sizeof( order->GrpOrdnCnt));
	seq = AtoI( order->GrpOrdnSeq, sizeof( order->GrpOrdnSeq));

	if( tot > 0)
	{
		if( tot == seq)
		{
			LogMsg( "그룹주문 마지막 주문 거부 처리 ... 나머지 INSERT");
			LogMsg( "    GrpOrdnNo = [%.*s]", sizeof( order->GrpOrdnNo), order->GrpOrdnNo);
			LogMsg( "    tot       = [%d]", tot);
			LogMsg( "    seq       = [%d]", seq);
			base_cur = Mat_GetCurrentInt( Mat, &order->Symbol[ 0]);
			cont_cur = Mat_GetCurrentInt( Mat, &order->Symbol[ 4]);
			index = Mat_GetIndex( Mat, base_cur, cont_cur);
			rtn = Mat_RecordInsertGroup( Mat, index, order, 0, 1);
			if( rtn < 0)
			{
				LogCri( "그룹주문 Process error. 주문 LOSS !!!, rtn=[%d]", rtn);
				return -1;
			}
		}
	}


	return 1;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  거부 SMQ send
***************************************************************************** */
int Proc_ExecuteSend( ORDER_SEND *send)
{
	int			rtn;
	int			send_sz;

	send_sz = sizeof( ORDER_SEND);
	rtn = Smq_Send( SmqSend, ( char *)send, send_sz);
	if( rtn < 0)
	{
		LogCri( "Smq_Send error. rtn=[%d]", rtn);
		return -1;
	}

	return 1;
}



