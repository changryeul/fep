/** ***************************************************************************
**  @file       blp.c
**  @date       2025/09/01
**  @author     cdc
**  @version    V0.0.20250901
**  @brif
**  채권 시장조성 라이브러리
**	blp.c			- 
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define		DATA_SIZE	2048
#include "shm_memory.h"
#include "pa_struct.h"
#include "buf_struct.h"

#include "mem.h"
#include "sem.h"
#include "etc.h"
#ifndef _OMS_SOURCE_
#include "log.h"
#else
#include "log_conv.h"
#endif
#include "blp.h"

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
extern int		Continue;
extern SHM_NOTE	*Shm_Note;
char			*LpOrdBuffer = "00000000001TCHMOR4000100KTSG100501031950100000944          KR103503GF95311000000000010000100000000009935.00000100000000009933.0020400001010002019015005056BF939D20251021                                                                     900140000000000216                                                                                                                                                 \n";
char			*OrdBuffer   = "00000000001TCHODR1000100KTSG10050103195                    KR700593000331100000001824000000012300000070500200000     00410  00      000110010020030040005056BF939D0117201721212768                                                            90014000000000000          0";

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  주문가격 계산 및 호가 제출
***************************************************************************** */
int Blp_ProcessOrder( BLP *blp, BLP_TBL *tbl)
{
	int			rtn;
	double		std_prc;
	double		ask_prc;
	double		bid_prc;
	double		diff_prc;
	int			i;
	// int			ask_pos, bid_pos;
	// int			ask_old, bid_old;

	BLP_ARG		*arg  = &tbl->arg;
	BLP_TIME	*mk_time = &arg->mk_time[ arg->mk_stat -1];
	BLP_SISE	*sise = &tbl->sise;
	// BLP_ORD		*order;

	for( i = 0; i < BLP_MAX_LP; i++)
	{
		
		/****************************/
		/* 중간값 및 주문 가격 계산 */
		/****************************/
		std_prc = ( sise->ask[ 0][ 0] + sise->bid[ 0][ 0]) / 2.0;
		ask_prc = std_prc + ( mk_time->sped_prc[ i] / 2.0);
		bid_prc = std_prc - ( mk_time->sped_prc[ i] / 2.0);

		LogDbg( "std_prc[ %d]=[%f]", i, std_prc);
		LogDbg( "ask_prc[ %d]=[%f]", i, ask_prc);
		LogDbg( "bid_prc[ %d]=[%f]", i, bid_prc);

		/* 주문 가격 보정 */
		if( tbl->arg.tick_unit == 0.5)
		{
			diff_prc = ask_prc - floor( ask_prc);
			if( diff_prc >= 0.75)	ask_prc = round( ask_prc);
			else
			if( diff_prc >= 0.5 )	ask_prc = round( ask_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	ask_prc = round( ask_prc) + 0.5;
			else
									ask_prc = round( ask_prc);

			diff_prc = bid_prc - floor( bid_prc);
			if( diff_prc >= 0.75)	bid_prc = round( bid_prc);
			else
			if( diff_prc >= 0.5 )	bid_prc = round( bid_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	bid_prc = round( bid_prc) + 0.5;
			else
									bid_prc = round( bid_prc);
		}
		else
		if( tbl->arg.tick_unit == 1.0)
		{
			ask_prc = round( ask_prc);
			bid_prc = round( bid_prc);
		}
		else
		{
			ask_prc = round( ask_prc * 10.0) / 10.0;
			bid_prc = round( bid_prc * 10.0) / 10.0;
		}

		/*************/
		/* 주문 전송 */
		/*************/
		rtn = Blp_Order( blp, tbl, i, ask_prc, bid_prc);
		if( rtn < 0)
		{
			LogCri( "Blp_Order error.");
			return -1;
		}

		LogDbg( "-------------------------------------------------");
	}


	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  거래소 주문응답/체결 처리
***************************************************************************** */
int Blp_ProcessExecute( BLP *blp, FILE_BUFF_FORMAT *file_buffer)
{
	int			rtn;
	BLP_TBL		*tbl = &blp->map->tbl[ blp->tbl_pos];

	KRX_NOTE_SETTLE_RESP_DATA		*resp;			/* TTRODP4130 */
	KRX_LP_NOTE_SETTLE_RESP_DATA	*resp_lp;		/* TTRMOP4130 */
	KRX_NOTE_SETTLE_DATA			*settle;		/* TTRTDP4230 */

	if( blp->tbl_pos < 0)
	{
		LogCri( "전략이 초기화 되지 않았습니다.");
		return -1;
	}

	FILE_BUFF_FORMAT_Print( file_buffer);
	LogDbg( "file_buffer=[%d:%s]", strlen( ( char *)file_buffer), file_buffer);


	resp      = ( KRX_NOTE_SETTLE_RESP_DATA *)&file_buffer->Data[ 35];
	resp_lp   = ( KRX_LP_NOTE_SETTLE_RESP_DATA *)&file_buffer->Data[ 35];
	settle    = ( KRX_NOTE_SETTLE_DATA         *)&file_buffer->Data[ 20];

	if( !memcmp( resp->TrCode, "TTRODP4130", 10))
	{
		/* 주문 확인 */
		LogMsg( "주문확인 수신");
		Blp_ProcOrdChk( blp, tbl, ( KRX_NOTE_SETTLE_RESP_DATA *)&file_buffer->Data[ 35]);
	}
	else
	if( !memcmp( resp_lp->Transaction_Code, "TTRMOP4130", 10))
	{
		/* LP 주문 확인 */
		LogMsg( "LP 주문확인 수신");
		Blp_ProcLpOrdChk( blp, tbl, ( KRX_LP_NOTE_SETTLE_RESP_DATA *)&file_buffer->Data[ 35]);
	}
	else
	if( !memcmp( settle->TrCode, "TTRTDP4230", 10))
	{
		/* 체결 */
		LogMsg( "체결 수신");
		Blp_ProcExeChk( blp, tbl, settle);
	}
	else
	{
		LogCri( "unknown data type.");
		KRX_NOTE_SETTLE_RESP_DATA_Print( resp);
		KRX_LP_NOTE_SETTLE_RESP_DATA_Print( resp_lp);
		KRX_NOTE_SETTLE_DATA_Print( settle);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  주문응답처리 - TTRMOP41301
***************************************************************************** */
int Blp_ProcOrdChk( BLP *blp, BLP_TBL *tbl, KRX_NOTE_SETTLE_RESP_DATA *ttrodp41301)
{
	int			i;
	BLP_HEDGE	*hg;
	char		test_ord[ 32];

	LogMsg( "햇지 주문확인 수신 ");
	KRX_NOTE_SETTLE_RESP_DATA_Print( ttrodp41301);

	if( memcmp( ttrodp41301->TrCode, "TTRODP41301", 11))	/* 정상이 아니면 - 거부 or 자동취소 */
	{
		/* 주문 거부 process */
		LogMsg( "주문 거부 ");
		/* 해당 주문을 찾아 stat 변경 */
		for( i = 0; i < tbl->hedge_cnt; i++)
		{
			hg = &tbl->hedge[ i];

			if( !memcmp( hg->ord_no, ttrodp41301->OrderNo, sizeof( ttrodp41301->OrderNo)))
			{
				LogMsg( "주문거부 수신 ... ord->ord_no=[%.10s] ttrodp41301->OrdNo=[%.10s]", hg->ord_no, ttrodp41301->OrderNo);
				if( hg->ord_stat > 0)
				{
					hg->res_stat = 2;
					break;
				}
				else
				{
					LogCri( "주문 상태 오류. hg->ord_stat=[%d]", hg->ord_stat);
					hg->res_stat = 5;
					return -1;
				}

				if( !memcmp( ttrodp41301->Order_Rejected_Reason, "0804", 4))		/* 잔량 없음 */
				{
					LogMsg( "정정 혹은 취소의 잔량이 없습니다.");
					return 1;
				}
				else
				{
					LogMsg( "호가거부사유=[%.*s]", sizeof( ttrodp41301->Order_Rejected_Reason), ttrodp41301->Order_Rejected_Reason);
					hg->res_stat = 2;
				}
			}
			return -1;
		}
	}

	/* 해당 주문을 찾아 stat 변경 */
	for( i = 0; i < tbl->hedge_cnt; i++)
	{
		hg = &tbl->hedge[ i];

		if( !memcmp( hg->ord_no, ttrodp41301->OrderNo, sizeof( ttrodp41301->OrderNo)))
		{
			LogMsg( "주문확인 수신 ... ord->ord_no=[%.10s] ttrodp41301->OrdNo=[%.10s]", hg->ord_no, ttrodp41301->OrderNo);
			if( hg->ord_stat > 0)
			{
				hg->res_stat = 1;
				if( hg->ord_stat == 2)	/* 취소주문 응답이면 햇지 완료 */
				{
					hg->done = 2;
				}
				break;
			}
			else
			{
				LogCri( "주문 상태 오류. hg->ord_stat=[%d]", hg->ord_stat);
				hg->res_stat = 5;
				return -1;
			}
		}

	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  LP 주문응답처리
***************************************************************************** */
int Blp_ProcLpOrdChk( BLP *blp, BLP_TBL *tbl, KRX_LP_NOTE_SETTLE_RESP_DATA *ttrodp41301)
{
	int			i;
	BLP_ORD		*ord;
	char		test_ord[ 32];
	int			rej_stat = 0;
	int			rej_code;

	LogMsg( "주문 응답 처리 ");
	KRX_LP_NOTE_SETTLE_RESP_DATA_Print( ttrodp41301);

	/* 해당 주문을 찾아 stat 변경 */
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		ord = &tbl->order[ i];

		if( !memcmp( ord->ord_no, ttrodp41301->OrderNo, sizeof( ttrodp41301->OrderNo)))
		{
			if( memcmp( ttrodp41301->Transaction_Code, "TTRMOP41301", 11)) /* 주문 거부 */
			{
				LogMsg( "주문거부 수신 ... ord->ord_no=[%.10s] ttrodp41301->OrdNo=[%.10s]", ord->ord_no, ttrodp41301->OrderNo);
				rej_code = AtoI( ttrodp41301->Order_Rejected_Reason, sizeof( ttrodp41301->Order_Rejected_Reason));
				LogMsg( "    거부사유 = [%d]", rej_code);
				if( rej_code == 804)		/* 정정취소 잔량 없음 */
				{
					LogMsg( "정정취소 잔량 없음");
					ord->res_stat = 3;
					continue;
				}
				ord->stat = 9;
				ord->res_stat = 3;
				rej_stat++;
			}
			else
			if( ord->ord_stat > 0)
			{
				LogMsg( "주문확인 수신 ... ord->ord_no=[%.10s] ttrodp41301->OrdNo=[%.10s]", ord->ord_no, ttrodp41301->OrderNo);
				ord->res_stat = 1;
				break;
			}
			else
			{
				LogCri( "주문 상태 오류. ord->stat=[%d]", ord->stat);
				ord->stat = 9;
				return -1;
			}
		}

	}

	/* 주문중 하나라도 거부가 있으면 */
	if( rej_stat > 0)
	{
		tbl->mode = 2;
		tbl->mode_next = 4;
		LogMsg( "모든 주문을 취소하고 전략을 종료합니다.");
		return -1;
	}

	/* 모든 LP 주문에 응답을 받았는지 check */
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		ord = &tbl->order[ i];

		if( ord->stat % 2) return 1;
	}

	LogMsg( "LP 주문 확인 완료 tbl->stat= [%d] -> [2] 변경", tbl->stat);
	tbl->stat = 2;	/* 주문 확인 완료 */

	/* 취소모드에서는 취소완료 여부 체크 */
	if( tbl->mode == 2)	
	{
		Blp_ProcessCancel( blp, tbl);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  체결 처리
***************************************************************************** */
int Blp_ProcExeChk( BLP *blp, BLP_TBL *tbl, KRX_NOTE_SETTLE_DATA *settle)
{
	int			i;
	int			comp_cnt = 0;		/* 전체 주문 완료 여부 */
	BLP_ORD		*ord;
	BLP_HEDGE	*hdg;
	double		exe_prc;
	double		exe_vol;
	int			trd_code;

	/* 체결 전문 */
	KRX_NOTE_SETTLE_DATA_Print( settle);


	if( tbl->mode == 1 || tbl->mode == 2) /* 전략모드 취소모드(체결 추가 반영) */
	{
		/* 해당 주문을 찾아 체결 반영 */
		for( i = 0; i < BLP_MAX_LP; i++)
		{
			ord = &tbl->order[ i];

			if( !memcmp( ord->ord_no, settle->OrderNo, sizeof( settle->OrderNo)))
			{
				LogMsg( "체결 수신 ... ord->ord_no=[%.10s] settle->OrdNo=[%.10s]", ord->ord_no, settle->OrderNo);
				exe_prc  = AtoD( settle->Trading_price, sizeof( settle->Trading_price));
				exe_vol  = AtoD( settle->Trading_Volumn, sizeof( settle->Trading_Volumn));
				trd_code = AtoI( settle->TradeFlag, sizeof( settle->TradeFlag));
				LogMsg( "    trd_code = [%d]", trd_code);
				LogMsg( "    exe_prc  = [%.2f]", exe_prc);
				LogMsg( "    exe_vol  = [%.0f]", exe_vol);
				if( trd_code == 1) 	ord->ask_exe_vol += exe_vol;		/* 매도 체결 */
				else				ord->bid_exe_vol += exe_vol;		/* 매수 체결 */
				ord->res_stat = 2;
				tbl->mode = 2;
				tbl->mode_next = 3;
				LogMsg( "모든 주문을 취소하고 햇지 모드로 전환합니다.");
				break;
			}
		}
#if 0
		tbl->mode = 2;		/* 취소 처리 모드로 변경 */
		time( &tbl->mode_time);

		/* TODO 체결 수량 반영 */
		/* for test LP1에 매도에 10000주 체결 */
		ord = &tbl->order[ 0];
		ord->ask_exe_vol += 10000.0;
#endif

		Blp_ProcessCancel( blp, tbl);
	}
	else
	if( tbl->mode == 3) /* 햇지모드 */
	{
		/* 해당 주문을 찾아 체결 반영 */
		for( i = 0; i < tbl->hedge_cnt; i++)
		{
			hdg = &tbl->hedge[ i];

			if( hdg->done > 0)	/* 햇지 완료된 주문은 skip */
			{
				comp_cnt++;
				continue;
			}

			if( !memcmp( hdg->ord_no, settle->OrderNo, sizeof( settle->OrderNo)))
			{
				LogMsg( "체결 수신 ... ord->ord_no=[%.10s] settle->OrdNo=[%.10s]", ord->ord_no, settle->OrderNo);
				exe_prc  = AtoD( settle->Trading_price, sizeof( settle->Trading_price));
				exe_vol  = AtoD( settle->Trading_Volumn, sizeof( settle->Trading_Volumn));
				trd_code = AtoI( settle->TradeFlag, sizeof( settle->TradeFlag));
				LogMsg( "    trd_code = [%d]", trd_code);
				LogMsg( "    exe_prc  = [%.2f]", exe_prc);
				LogMsg( "    exe_vol  = [%.0f]", exe_vol);
				hdg->exe_vol += exe_vol;
				hdg->res_stat = 3;
				LogMsg( "햇지모드 - 체결 반영");
				LogMsg( "    주문 수량 = [%.0f]", hdg->vol);
				LogMsg( "    체결 수량 = [%.0f]", exe_vol);
				LogMsg( "    누적 수량 = [%.0f]", hdg->exe_vol);
				if( hdg->exe_vol >= hdg->vol)		/* 햇지 완료 */
				{
					LogMsg( "햇지모드 - 햇지 완료");
					hdg->res_stat = 4;
					hdg->done = 1;
					comp_cnt++;
				}
			}
		}
		if( comp_cnt >= tbl->hedge_cnt)				/* 햇지 완료 여부 */
		{
			LogMsg( "햇지모드 - 전체 수량 햇지 완료");
			tbl->hedge_cnt = 0;
			tbl->mode = 0;
			tbl->mode_next = 1;
			tbl->stat = 0;							/* 모든 주문 해소 */
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  모든 LP 주문을 취소
***************************************************************************** */
int Blp_ProcessCancel( BLP *blp, BLP_TBL *tbl)
{
	int			rtn;
	int			i;
	// int			wait = 1;
	int			cancel_stat = 0;	/* 취소 완료 = 0 */
	BLP_ORD		*ord, *old;

	/* 해당 주문을 찾아 취소 처리 */
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		ord = &tbl->order[ i];
		old = &tbl->old_order[ i];

		if( ord->ord_stat != 0 && ord->res_stat == 0)	/* 주문응답을 못받은 상태 */
		{
			LogDbg( "취소처리를 위한 주문 응답 대기중 ... ");
			cancel_stat = 1;
			continue;
		}

		/* 취소주문 전송 여부 */
		if( ord->ord_stat == 3) continue;

		if( ord->res_stat == 3)	continue;						/* 주문 거부를 받은 상태 */

		/* 주문 취소 처리 */
		memcpy( old, ord, sizeof( BLP_ORD));						/* 취소를 위해 원주문을 old로 copy */
		memcpy( ord->org_no, ord->ord_no, sizeof( ord->org_no));	/* 주문번호를 원주문 번호로 copy */
		rtn = Blp_GetOrderNoOMS( blp, ord->ord_no, sizeof( ord->ord_no));
		if( rtn < 0)
		{
			LogCri( "주문 채번 오류. rtn=[%d]", rtn);
			return -1;
		}
		LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
		ord->ord_stat = 3;
		ord->res_stat = 0;
		cancel_stat = 1;
		Blp_MakeLpOrder( blp, tbl, i, 2);
	}

	if( cancel_stat == 0)
	{
		LogMsg( "주문 취소 완료");
		if( tbl->mode_next != 0)
		{
			tbl->mode = tbl->mode_next;
			LogMsg( "    mode=[%d]로 전환  [0:대기모드 1:전략수행 2:취소모드 3:햇지모드 4:종료]", tbl->mode);
			tbl->mode_next = 0;
		}
		else
		{
			tbl->mode = 3;
		}
		if( tbl->mode == 3)
		{
			LogMsg( "    햇지 모드로 전환");
			tbl->mode = 3;
			time( &tbl->mode_time);
			Blp_ProcHdgOrd( blp, tbl);	/* 햇지 주문은 한번 - 주문후는 체결 모니터링(ProcessHedge) */
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  모든 LP 주문을 취소
***************************************************************************** */
int Blp_ProcHdgOrd( BLP *blp, BLP_TBL *tbl)
{
	int				rtn;
	int				i;
	BLP_ORD			*ord;
	BLP_HEDGE		*hedge;
	int				mode = 0;						/* 자전방지를 위한 check	0:매도단순햇지 
																				1:매수단순햇지
																				2:매도단방향 
																				3:매수단방향 
																				4:총합햇지 
													*/
	int				ask_cnt = 0, bid_cnt = 0, tot_cnt = 0;		/* 3개 LP 주문의 체결 count */
	double			ask_tot_exe;
	double			bid_tot_exe;

	/* 체결된 총 수량을 구한다 - 자전 방지 */
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		ord = &tbl->order[ i];
		ask_tot_exe += ord->ask_exe_vol;
		bid_tot_exe += ord->bid_exe_vol;
		if( ord->ask_exe_vol > 0.0) ask_cnt++;
		if( ord->bid_exe_vol > 0.0) bid_cnt++;
	}
	tot_cnt = ask_cnt + bid_cnt;
	if( tot_cnt == 1)	
	{
		if( ask_cnt != 0)	mode = 0;
		else				mode = 1;
	}
	else
	if( tot_cnt > 1)
	{
		if( bid_cnt == 0)	mode = 2;				/* 매도 체결이 여러개 있는경우 */
		else
		if( ask_cnt == 0)	mode = 3;				/* 매수 체결이 여러개 있는경우 */
		else
							mode = 4;				/* 매수/매도 체결이 여러개 있는경우 */
	}

	switch( mode)
	{
		case 0: /* 매도 단순 햇지 - 체결 주문을 찾아 방향만 반대로 */
		case 2: /* 매도 단방향 햇지 */
			for( i = 0; i < BLP_MAX_LP; i++)
			{
				ord = &tbl->order[ i];
				if( ord->ask_exe_vol > 0.0)
				{
					hedge = &tbl->hedge[ tbl->hedge_cnt];
					memset( hedge, 0x00, sizeof( BLP_HEDGE));
					hedge->ord_stat = 1;
					hedge->res_stat = 0;
					rtn = Blp_GetOrderNoOMS( blp, hedge->ord_no, sizeof( ord->ord_no));
					if( rtn < 0)
					{
						LogCri( "주문 채번 오류. rtn=[%d]", rtn);
						return -1;
					}
					LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
					hedge->side = 2;	/* 매도 체결 -> 매수로 */
					hedge->prc  = ord->ask_prc;
					hedge->vol  = ord->ask_exe_vol;
					time( &hedge->ord_time);
					Blp_MakeOrder( blp, tbl, tbl->hedge_cnt, 0);
					tbl->hedge_cnt++;
				}
			}
			break;
		case 1: /* 매수 단순 햇지 */
		case 3:	/* 매수 단방향 */
			for( i = 0; i < BLP_MAX_LP; i++)
			{
				ord = &tbl->order[ i];
				if( ord->bid_exe_vol > 0.0)
				{
					hedge = &tbl->hedge[ tbl->hedge_cnt];
					memset( hedge, 0x00, sizeof( BLP_HEDGE));
					hedge->ord_stat = 1;
					hedge->res_stat = 0;
					rtn = Blp_GetOrderNoOMS( blp, hedge->ord_no, sizeof( ord->ord_no));
					if( rtn < 0)
					{
						LogCri( "주문 채번 오류. rtn=[%d]", rtn);
						return -1;
					}
					LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
					hedge->side = 1;	/* 매수 체결 -> 매도로 */
					hedge->prc  = ord->bid_prc;
					hedge->vol  = ord->bid_exe_vol;
					time( &hedge->ord_time);
					Blp_MakeOrder( blp, tbl, tbl->hedge_cnt, 0);
					tbl->hedge_cnt++;
				}
			}
			break;
		case 4:	/* 총합 햇지 - 총합을 구해 1호가로 주문 */
			if( ask_tot_exe - bid_tot_exe > 0.0)
			{
				ord = &tbl->order[ 0];
				hedge = &tbl->hedge[ tbl->hedge_cnt];
				memset( hedge, 0x00, sizeof( BLP_HEDGE));
				hedge->ord_stat = 1;
				hedge->res_stat = 0;
				rtn = Blp_GetOrderNoOMS( blp, hedge->ord_no, sizeof( ord->ord_no));
				if( rtn < 0)
				{
					LogCri( "주문 채번 오류. rtn=[%d]", rtn);
					return -1;
				}
				LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
				hedge->side = 2;	/* 매도 체결 -> 매수로 */
				hedge->prc  = ord->ask_prc;
				hedge->vol  = ask_tot_exe - bid_tot_exe;
				time( &hedge->ord_time);
				Blp_MakeOrder( blp, tbl, tbl->hedge_cnt, 0);
				tbl->hedge_cnt++;
			}
			else
			{
				ord = &tbl->order[ 0];
				hedge = &tbl->hedge[ tbl->hedge_cnt];
				memset( hedge, 0x00, sizeof( BLP_HEDGE));
				hedge->ord_stat = 1;
				hedge->res_stat = 0;
				rtn = Blp_GetOrderNoOMS( blp, hedge->ord_no, sizeof( ord->ord_no));
				if( rtn < 0)
				{
					LogCri( "주문 채번 오류. rtn=[%d]", rtn);
					return -1;
				}
				LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
				hedge->side = 2;	/* 매수 체결 -> 매도로 */
				hedge->prc  = ord->bid_prc;
				hedge->vol  = bid_tot_exe - ask_tot_exe;
				time( &hedge->ord_time);
				Blp_MakeOrder( blp, tbl, tbl->hedge_cnt, 0);
				tbl->hedge_cnt++;
			}
			break;
		default:
			LogCri( "햇지 주문 오류. mode=[%d]", mode);
			break;
	}
			
	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  모든 LP 주문을 취소
***************************************************************************** */
int Blp_ProcessHedge( BLP *blp, BLP_TBL *tbl)
{
	int			i;
	int			rtn;
	// int			sleep_time;
	time_t		cur_time;
	int			done = 1;
	BLP_HEDGE	*hg;
	BLP_ARG		*arg = &tbl->arg;
	BLP_TIME	*arg_time = &arg->mk_time[ arg->mk_stat -1];

	LogDbg( "햇지 주문 감시");

	time( &cur_time);
	LogDbg( "HERE ... 1");
	for( i = 0; i < tbl->hedge_cnt; i++)
	{
		hg = &tbl->hedge[ i];
		if( hg->done > 1)	done = hg->done;		/* 완료 여부 1:체결 완료 2:취소완료 */
		if( hg->done) continue;

		done = 0;
		hg->gap_time = cur_time - hg->ord_time;
		LogDbg( "hg->gap_time > tbl->hedge_time = [%d] > [%d]", hg->gap_time, arg_time->rev_wait);
		if( hg->gap_time > arg_time->rev_wait)
		{
			/* 시간 지남 - 취소 주문 */
			if( hg->ord_stat != 1) continue;
			/* 주문 응답을 못받은 상태 */
			if( hg->ord_stat == 1 && hg->res_stat == 0 ) continue;
			memcpy( hg->org_no, hg->ord_no, sizeof( hg->org_no));	/* 취소주문을 위해 원주문번호 copy */
			LogDbg( "    원주문번호 = [%.12s]", hg->org_no);
			rtn = Blp_GetOrderNoOMS( blp, hg->ord_no, sizeof( hg->ord_no));
			if( rtn < 0)
			{
				LogCri( "주문 채번 오류. rtn=[%d]", rtn);
				return -1;
			}
			LogDbg( "    주문번호 채번 = [%.12s]", hg->ord_no);
			LogDbg( "    원주문번호    = [%.12s]", hg->org_no);
			hg->ord_stat = 2;
			hg->res_stat = 0;
			Blp_MakeOrder( blp, tbl, i, 2);
		}
	}
	LogDbg( "HERE ... 2 done=[%d]", done);

	if( done == 2)	/* 주문 취소됨 */
	{
		LogMsg( "햇지 실패 완료 ");
		tbl->stat = 0;		/* 전략 종료 준비 */
		tbl->mode = 0;		/* 취소 처리 되었으므로 전략 종료 */
		tbl->mode_next = 0;
	}
	else
	if( done == 1)	/* 햇지 성공으로 다시 전략 모드 */
	{
		LogDbg( "HERE ... 21");
		/* 체결이후 전략모드로 돌아갈 시간 check */
		if( cur_time - hg->ord_time > arg_time->exe_delay)
		{
			LogMsg( "종료 [%3d/%3d]",  arg_time->exe_delay - ( int)( cur_time - hg->ord_time), arg_time->exe_delay);
			LogMsg( "햇지 성공 완료 - 전략수행 모드로 변경");
			tbl->mode = 1;
			Blp_ClearTbl( blp, tbl);
			time( &tbl->mode_time);
		}
		else
		{
			LogMsg( "햇지 성공 완료 - 전략수행 모드 변경 준비 ord_time=[%d] > delay=[%d]",  
					( int)( cur_time - hg->ord_time), arg_time->exe_delay);
			while( Continue)
			{
				time( &cur_time);
				if( cur_time - hg->ord_time > arg_time->exe_delay) break;
				LogMsg( "대기 [%3d/%3d]",  arg_time->exe_delay - ( int)( cur_time - hg->ord_time), arg_time->exe_delay);
			}
		}
	}

	LogDbg( "HERE ... 3");
	return 1;
}



/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      BLP *blp - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  기준 가격에 대한 유효 호가 설정 
***************************************************************************** */
int Blp_CheckOrder( BLP *blp, BLP_TBL *tbl, BLP_SISE *sise)
{
	int		rtn;
	double	std_prc;
	double	ask_prc;
	double	bid_prc;
	double	diff_prc;
	int		i;
	// int		ask_pos, bid_pos;
	// int		ask_old, bid_old;

	BLP_ARG		*arg = &tbl->arg;
	BLP_TIME	*mk_time = &arg->mk_time[ arg->mk_stat -1];


	for( i = 0; i < BLP_MAX_LP; i++)
	{
		/****************************/
		/* 중간값 및 주문 가격 계산 */
		/****************************/
		std_prc = ( sise->ask[ 0][ 0] + sise->bid[ 0][ 0]) / 2.0;
		ask_prc = std_prc + ( mk_time->sped_prc[ i] / 2.0);
		bid_prc = std_prc - ( mk_time->sped_prc[ i] / 2.0);

		LogDbg( "std_prc[ %d]=[%f]", i, std_prc);
		LogDbg( "ask_prc[ %d]=[%f]", i, ask_prc);
		LogDbg( "bid_prc[ %d]=[%f]", i, bid_prc);

		/* 주문 가격 보정 */
		if( tbl->arg.tick_unit == 0.5)
		{
			diff_prc = ask_prc - floor( ask_prc);
			if( diff_prc >= 0.75)	ask_prc = round( ask_prc);
			else
			if( diff_prc >= 0.5 )	ask_prc = round( ask_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	ask_prc = round( ask_prc) + 0.5;
			else
									ask_prc = round( ask_prc);

			diff_prc = bid_prc - floor( bid_prc);
			if( diff_prc >= 0.75)	bid_prc = round( bid_prc);
			else
			if( diff_prc >= 0.5 )	bid_prc = round( bid_prc) - 0.5;
			else
			if( diff_prc >= 0.25 )	bid_prc = round( bid_prc) + 0.5;
			else
									bid_prc = round( bid_prc);
		}
		else
		if( tbl->arg.tick_unit == 1.0)
		{
			ask_prc = round( ask_prc);
			bid_prc = round( bid_prc);
		}
		else
		{
			ask_prc = round( ask_prc * 10.0) / 10.0;
			bid_prc = round( bid_prc * 10.0) / 10.0;
		}

		/*************/
		/* 주문 전송 */
		/*************/
		rtn = Blp_Order( blp, tbl, i, ask_prc, bid_prc);
		if( rtn < 0)
		{
			LogCri( "Blp_Order error.");
			return -1;
		}

		LogDbg( "-------------------------------------------------");
	}


	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      lp_no - 주문 호가 테이블 번호
**  @param      ask_prc - 주문 매도 가격
**  @param      ask_prc - 주문 매수 가격
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  LP 주문 
***************************************************************************** */
int Blp_Order( BLP *blp, BLP_TBL *tbl, int lp_no, double ask_prc, double bid_prc)
{
	int			rtn;
	int			ask_pos, bid_pos;			/* 호가 테이블의 주문 위치 */
	int			ask_old, bid_old;			/* 이전 호가 테이블의 주문 위치 */
	BLP_ORD		*ord = &tbl->order[ lp_no];
	BLP_ORD		*old = &tbl->old_order[ lp_no];
	BLP_ARG		*arg = &tbl->arg;
	BLP_TIME	*mk_time = &arg->mk_time[ arg->mk_stat -1];

	arg = &tbl->arg;

	/******************/
	/* 주문 상태 체크 */
	/******************/
	if( ord->ord_stat != 0 && ord->res_stat == 0)		/* 주문 응답을 못 받은 상태 */
	{
		/* 주문/정정/취소 확인 대기 */
		LogMsg( "주문 확인 대기 상태입니다.");
		return 0;
	}

	/* 주문 가능 상태 */
	LogMsg( "주문 가능 상태");

	LogDbg( "Blp_Order ... [%d]", lp_no);
	LogDbg( "    매도     =[%10.2f]", ask_prc);
	LogDbg( "    매수     =[%10.2f]", bid_prc);

	/* 호가 테이블의 주문 위치 */
	ask_pos = Blp_GetPrice( blp, tbl, ask_prc);
	bid_pos = Blp_GetPrice( blp, tbl, bid_prc);
	ask_old = Blp_GetPrice( blp, tbl, ord->ask_prc);
	bid_old = Blp_GetPrice( blp, tbl, ord->bid_prc);
	LogDbg( "ask[ %d]=new:old[%2d:%2d]", lp_no, ask_pos, ask_old);
	LogDbg( "bid[ %d]=new:old[%2d:%2d]", lp_no, bid_pos, bid_old);

	/* 호가 테이블 수정 */
	tbl->rec[ ask_old].lp_no = 0;
	tbl->rec[ bid_old].lp_no = 0;
	tbl->rec[ ask_pos].lp_no = lp_no +1; /* 1, 2, 3 */
	tbl->rec[ bid_pos].lp_no = lp_no +1;

	/* 주문 가격 */

	/* 주문 상태 */
	LogDbg( "ord->stat=[%d]", ord->stat);
	if( ord->ord_stat == 0 || ord->ord_stat == 3)	/* 주문이 없는 상태 이거나 취소 주문이 나간 상태는 주문이 없으므로 */
	{
		LogDbg( "    신규주문");

		// ord->stat = 2;		/* 테스트를 위해 주문 확인이 들어온걸로 set */
		ord->ord_stat = 1;
		ord->res_stat = 0;
		tbl->stat = 1;

		ord->ask_prc = ask_prc;
		ord->bid_prc = bid_prc;
		rtn = Blp_GetOrderNoOMS( blp, ord->ord_no, sizeof( ord->ord_no));
		ord->ask_vol = mk_time->ord_qanty[ lp_no] * arg->ord_qanty_unit;
		ord->bid_vol = mk_time->ord_qanty[ lp_no] * arg->ord_qanty_unit;
		LogDbg( "    신규호가");
		LogDbg( "        ask_prc=[%10.2f] -> [%10.2f] vloume=[%10.0f]", old->ask_prc, ord->ask_prc, ord->ask_vol);
		LogDbg( "        bid_prc=[%10.2f] -> [%10.2f] vloume=[%10.0f]", old->bid_prc, ord->bid_prc, ord->bid_vol);
		rtn = Blp_MakeLpOrder( blp, tbl, lp_no, 0);
		if( rtn < 0)
		{
			LogCri( "Blp_MakeLpOrder error.");
			return -1;
		}
	}
	else								/* 정정 주문 */
	{
		LogDbg( "    정정주문");

		if( ask_prc == ord->ask_prc)
		{
			if( bid_prc == ord->bid_prc)
			{
				LogDbg( "    주문유지");
				LogDbg( "        ask_prc=[%10.2f]", ask_prc);
				LogDbg( "        bid_prc=[%10.2f]", bid_prc);
				return 1;
			}
		}
		else
		if( ask_prc < 9000.00) return 1;
		memcpy( old, ord, sizeof( BLP_ORD));

		// ord->stat = 4;		/* 테스트를 위해 정정 확인이 들어온걸로 set */
		ord->ord_stat = 2;
		ord->res_stat = 0;
		tbl->stat = 1;

		ord->ask_prc = ask_prc;
		ord->bid_prc = bid_prc;

		memcpy( ord->org_no, ord->ord_no, sizeof( ord->org_no));
		rtn = Blp_GetOrderNoOMS( blp, ord->ord_no, sizeof( ord->ord_no));
		LogDbg( "    주문번호 채번 = [%.12s]", ord->ord_no);
		ord->ask_vol = mk_time->ord_qanty[ lp_no] * arg->ord_qanty_unit;
		ord->bid_vol = mk_time->ord_qanty[ lp_no] * arg->ord_qanty_unit;
		LogDbg( "    호가변경");
		LogDbg( "        ask_prc=[%10.2f] -> [%10.2f] vloume=[%10.0f]", old->ask_prc, ord->ask_prc, ord->ask_vol);
		LogDbg( "        bid_prc=[%10.2f] -> [%10.2f] vloume=[%10.0f]", old->bid_prc, ord->bid_prc, ord->bid_vol);
		rtn = Blp_MakeLpOrder( blp, tbl, lp_no, 1);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 호가 테이블 리스트
***************************************************************************** */
int Blp_MakeOrder( BLP *blp, BLP_TBL *tbl, int no, int option)
{
	// char		*hdg_stat[ 8] = { "신규", "정정", "취소", NULL, NULL};
	BLP_HEDGE		*hdg = &tbl->hedge[ no];
	char			time_buf[ 32];
	struct timeval	tv;
	struct tm		tp;
	char			buf[ 8192];
	KRX_NOTE_JUMUN_DATA		*order = ( KRX_NOTE_JUMUN_DATA *)buf;
	BLP_MEMBER_AREA			*ma    = ( BLP_MEMBER_AREA *)&order->MembershipItem;

	memset( buf, 0x20, sizeof( buf));
	gettimeofday( &tv, NULL);
	localtime_r( &tv.tv_sec, &tp);

	sprintf( time_buf, "%02d%02d%02d%03d", tp.tm_hour, tp.tm_min, tp.tm_sec, (int)tv.tv_usec / 1000);
	LogDbg( "time_buf=[%s]", time_buf);


	memcpy( order, OrdBuffer, sizeof( KRX_LP_NOTE_JUMUN_DATA));
	/* TEST */
	order->DataSeq[ 0] = '0';
	STRtoSTR( &order->DataSeq[ 1], 		hdg->ord_no, 			sizeof( order->DataSeq));
	STRtoSTR( order->ItemCode, 			tbl->item_code,			sizeof( order->ItemCode));
	STRtoSTR( order->AccountNo, 		tbl->arg.account_no,	sizeof( order->AccountNo));
	// STRtoSTR( order->Order_Date, 		tbl->arg.exe_ymd,		sizeof( order->Order_Date));
	sprintf( time_buf, "%04d%02d%02d", tp.tm_year + 1900, tp.tm_mon +1, tp.tm_mday);
	STRtoSTR( order->Order_Date, 		time_buf,				sizeof( order->Order_Date));
	sprintf( time_buf, "%02d%02d%02d%03d", tp.tm_hour, tp.tm_min, tp.tm_sec, (int)tv.tv_usec / 1000);
	STRtoSTR( order->Member_Send_Time, 	time_buf,				sizeof( order->Member_Send_Time));
	
	switch( option)
	{
		case 0:
			LogDbg( "신규 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( hdg->ord_no), hdg->ord_no);
			LogDbg( "    side     =[%d]", hdg->side);
			LogDbg( "    가격     =[%10.2f]", hdg->prc);
			LogDbg( "    수량     =[%10.0f]", hdg->vol);
			STRtoSTR( order->Transaction_Code,		"TCHODR40001",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo, 				hdg->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo, 		"          ", 	sizeof( order->OriginalOrderNo));
			INTtoSTR( order->TradeFlag, 			hdg->side, 		sizeof( order->TradeFlag));
			STRtoSTR( order->New_Modify_Cancel_gbn, "1", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->OrderQuantity,			hdg->vol,		sizeof( order->OrderQuantity));
			DBLtoS02( order->Price,					hdg->prc,		sizeof( order->Price));
			STRtoSTR( order->Order_Type,			"2",			sizeof( order->Order_Type));
			STRtoSTR( order->Order_Condition,		"0",			sizeof( order->Order_Condition));
			if( hdg->side == 1)
			STRtoSTR( order->Ask_Type,				"01",			sizeof( order->Ask_Type));
			else
			STRtoSTR( order->Ask_Type,				"00",			sizeof( order->Ask_Type));
			STRtoSTR( order->Trust_Principal_Type,	"30",			sizeof( order->Trust_Principal_Type));
			STRtoSTR( order->Investor_Type,			"4000",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"16",			sizeof( order->Account_Type));
			STRtoSTR( order->Country_Code,			"410",			sizeof( order->Country_Code));
			STRtoSTR( order->Foreign_Investor_Type,	"00",			sizeof( order->Foreign_Investor_Type));
			break;
		case 1:
			LogDbg( "정정 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( hdg->ord_no), hdg->ord_no);
			LogDbg( "    side     =[%d]", hdg->side);
			LogDbg( "    가격     =[%10.2f]", hdg->prc);
			LogDbg( "    수량     =[%10.0f]", hdg->vol);
			STRtoSTR( order->Transaction_Code,		"TCHODR40002",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo, 				hdg->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo, 		hdg->org_no, 	sizeof( order->OriginalOrderNo));
			INTtoSTR( order->TradeFlag, 			hdg->side, 		sizeof( order->TradeFlag));
			STRtoSTR( order->New_Modify_Cancel_gbn, "2", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->OrderQuantity,			hdg->vol,		sizeof( order->OrderQuantity));
			DBLtoS02( order->Price,					hdg->prc,		sizeof( order->Price));
			STRtoSTR( order->Order_Type,			"1",			sizeof( order->Order_Type));
			STRtoSTR( order->Order_Condition,		"0",			sizeof( order->Order_Condition));
			STRtoSTR( order->Ask_Type,				"  ",			sizeof( order->Ask_Type));
			STRtoSTR( order->Trust_Principal_Type,	"30",			sizeof( order->Trust_Principal_Type));
			STRtoSTR( order->Investor_Type,			"    ",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"  ",			sizeof( order->Account_Type));
			STRtoSTR( order->Country_Code,			"   ",			sizeof( order->Country_Code));
			STRtoSTR( order->Foreign_Investor_Type,	"  ",			sizeof( order->Foreign_Investor_Type));
			break;
		case 2:
			LogDbg( "취소 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( hdg->ord_no), hdg->ord_no);
			LogDbg( "    side     =[%d]", hdg->side);
			LogDbg( "    가격     =[%10.2f]", hdg->prc);
			LogDbg( "    수량     =[%10.0f]", hdg->vol);
			STRtoSTR( order->Transaction_Code,		"TCHODR40003",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo, 				hdg->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo, 		hdg->org_no, 	sizeof( order->OriginalOrderNo));
			INTtoSTR( order->TradeFlag, 			hdg->side, 		sizeof( order->TradeFlag));
			STRtoSTR( order->New_Modify_Cancel_gbn, "3", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->OrderQuantity,			hdg->vol,		sizeof( order->OrderQuantity));
			DBLtoS02( order->Price,					0.0,			sizeof( order->Price));
			STRtoSTR( order->Order_Type,			" ",			sizeof( order->Order_Type));
			STRtoSTR( order->Order_Condition,		" ",			sizeof( order->Order_Condition));
			STRtoSTR( order->Ask_Type,				"  ",			sizeof( order->Ask_Type));
			STRtoSTR( order->Trust_Principal_Type,	"  ",			sizeof( order->Trust_Principal_Type));
			STRtoSTR( order->Investor_Type,			"    ",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"  ",			sizeof( order->Account_Type));
			STRtoSTR( order->Country_Code,			"   ",			sizeof( order->Country_Code));
			STRtoSTR( order->Foreign_Investor_Type,	"  ",			sizeof( order->Foreign_Investor_Type));
			break;
		default:
			LogDbg( "주문오류");
			LogDbg( "    주문번호 =[%.*s]", sizeof( hdg->ord_no), hdg->ord_no);
			LogDbg( "    side     =[%d]", hdg->side);
			LogDbg( "    가격     =[%10.2f]", hdg->prc);
			LogDbg( "    수량     =[%10.0f]", hdg->vol);
			break;
	}

	STRtoSTR( ma->auto_yn,					"A",			sizeof( ma->auto_yn));
	STRtoSTR( ma->strategy,					"ST",			sizeof( ma->strategy));
	STRtoSTR( ma->stg_no,					"01",			sizeof( ma->stg_no));
	STRtoSTR( ma->market,					"02",			sizeof( ma->market));
	INTtoSTR( ma->index,					tbl->item_no,	sizeof( ma->index));
	INTtoSTR( ma->acc_seq,					1,				sizeof( ma->acc_seq));
	STRtoSTR( ma->stock,					"0",			sizeof( ma->stock));
	STRtoSTR( ma->spread,					"0",			sizeof( ma->spread));
	STRtoSTR( ma->proc_nm,					&tbl->stg_id[3],sizeof( ma->proc_nm));
	KRX_NOTE_JUMUN_DATA_Print( ( KRX_NOTE_JUMUN_DATA *)order);
	Blp_UpdateHoga( blp, tbl, no +1);
	// Od_Write_Data( 1 /* 채권 */, option /* 신규/정정/취소 */, 2 /* 채권 LP */, 3 /* 매수/매도 */, tbl->item_no);
	Od_Write_DataBond( 1, ( char *)order, ( int)sizeof( KRX_NOTE_JUMUN_DATA));

	return 1;
}

/**************************************************************************************/
/*    261Byte의 KRX주문 포맷을 만들고 마지막으로 회원사처리항목 값 설정               */
/*    회원사처리항목 60바이트중 앞 30바이트는 원장에서 요청한 값을 넣어야 하고,       */
/*          뒤 30바이트는 아래의 조건에 맞게 Setting해야함 (Client도 동일)            */
/*    30           : A(서버자동주문), C(Client에서 낸 주문)                           */
/*    31, 32       : ST(Strategy)                                                     */
/*    33, 34       : 전략번호 01 ~ 99                                                 */
/*    35, 36       : 시장구분                                                         */
/*                   1(지수선물) 2(지수옵션) 3(주식선물) 4(주식옵션)                  */
/*                   5(유가증권/ELW/ETF/ETN) 6(코스닥)                                */
/*                   8(KRX300) 9(Kosdaq150 Futures) 10(Kosdaq150 Options)             */
/*    37,38,39,40,41 : A0의 Seq번호 Ex) 236 => (00236)                                */
/*    42, 43       : 계좌번호 Seq                                                     */
/*    44           : 0 (Default, 유가증권과 주식선물만 선택 나머진 0)                 */
/*                 : 유가증권(5)일 경우 => 0(Normal), 1(ELW), 2(ETN), 3(ETF)          */
/*                 : 주식선물(3)일 경우 => 해당종목이 코스피면 0, 코스닥이면 1        */
/*    45           : 스프레드여부, 0:normal, 1:스프레드                               */
/*    46,47,48,49  : Process Nick Name                                                */
/*                   pa_50101mp => 0101, pa_50203mp => 0203                           */
/**************************************************************************************/
/**************************************************************************************/
/*    MembershipItem => 30 byte부터 사용가능                                          */
/*    파생(IMECO는 20바이트만 준다.)                                                  */
/*        따라서 스프레드는 사용못한다.                                               */
/**************************************************************************************/
/*     4, 5, 6, 7, 8 : 스프레드 근월물 종목Seq(미사용)                                */
/*    16,17,18,19,20 : 스프레드 원월물 종목Seq(미사용)                                */
/*    30 : A(서버자동주문), C(메리츠Client주문) T(윈웨이매체)                         */
/*       : 매체구분 (A:Auto or All, C:메리츠매체, T:윈웨이매체                        */
/*         A는 자동주문/주문응답,체결등 주문관련된건(TR100XXX) 양쪽매체에 보낸다.     */
/*         Data Header 50 Byte중 20번째 1자리와 같이 사용한다(조회등)                 */
/*    31,32,33,34 : 전략에서 사용                                                     */
/*    35, 36 : 시장구분은 35 1자리만으로 체크                                         */
/*     (운용상품분류코드)  (시장index) (운영상품index) (시장구분명)                   */
/*             01                   0           1      채권일반                       */
/*             02                   0           2      채권LP                         */
/*             11                   1           1      금융파생_국채선물              */
/*             12                   1           2      금융파생_통화선물              */
/*             13                   1           3      금융파생_금리선물              */
/*                                                                                    */
/*    37,38,39,40,41 : A0 seq번호 ex) 236 => (00236)                                  */
/*    42, 43 : 계좌번호 seq                                                           */
/*    44     : 미사용                                                                 */
/*    45     : 스프레드종목이면 1, 아니면 0                                           */
/*    46     : 주식선물 && 주식옵션에서 유가증권종목이면 '1', 코스닥종목이면 '2'      */
/*             나머지시장이면 '0'                                                     */
/*    47,48,49,50 : ApType 4자리 (50101중 0101만) Set                                 */
/**************************************************************************************/
/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  채권시장조성 호가 테이블 리스트
***************************************************************************** */
int Blp_MakeLpOrder( BLP *blp, BLP_TBL *tbl, int no, int option)
{
	BLP_ORD		*ord = &tbl->order[ no];
	BLP_ORD		*old = &tbl->old_order[ no];
	char			time_buf[ 32];
	struct timeval	tv;
	struct tm		tp;
	char			buf[ 8192];
	KRX_LP_NOTE_JUMUN_DATA	*order = ( KRX_LP_NOTE_JUMUN_DATA *)buf;
	BLP_MEMBER_AREA			*ma    = ( BLP_MEMBER_AREA *)&order->MembershipItem;

	memset( buf, 0x20, sizeof( buf));
	gettimeofday( &tv, NULL);
	localtime_r( &tv.tv_sec, &tp);

	sprintf( time_buf, "%02d%02d%02d%03d", tp.tm_hour, tp.tm_min, tp.tm_sec, (int)tv.tv_usec / 1000);
	LogDbg( "time_buf=[%s]", time_buf);


	memcpy( order, LpOrdBuffer, sizeof( KRX_LP_NOTE_JUMUN_DATA));
	/* TEST */
	order->DataSeq[ 0] = '0';
	STRtoSTR( &order->DataSeq[ 1], 		ord->ord_no, 			sizeof( order->DataSeq));
	STRtoSTR( order->ItemCode, 			tbl->item_code,			sizeof( order->ItemCode));
	STRtoSTR( order->AccountNo, 		tbl->arg.account_no,	sizeof( order->AccountNo));
	// STRtoSTR( order->Order_Date, 		tbl->arg.exe_ymd,		sizeof( order->Order_Date));
	sprintf( time_buf, "%04d%02d%02d", tp.tm_year + 1900, tp.tm_mon +1, tp.tm_mday);
	STRtoSTR( order->Order_Date, 		time_buf,				sizeof( order->Order_Date));
	sprintf( time_buf, "%02d%02d%02d%03d", tp.tm_hour, tp.tm_min, tp.tm_sec, (int)tv.tv_usec / 1000);
	STRtoSTR( order->Member_Send_Time, 	time_buf,				sizeof( order->Member_Send_Time));
	
	switch( option)
	{
		case 0:
			LogDbg( "신규 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( ord->ord_no), ord->ord_no);
			LogDbg( "    매도     =[%10.2f][%10.0f]", ord->ask_prc, ord->ask_vol);
			LogDbg( "    매수     =[%10.2f][%10.0f]", ord->bid_prc, ord->bid_vol);
			STRtoSTR( order->Transaction_Code,		"TCHMOR40001",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo, 				ord->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo, 		"          ", 	sizeof( order->OriginalOrderNo));
			STRtoSTR( order->New_Modify_Cancel_gbn, "1", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->Ask_Offer_Qty,			ord->ask_vol,	sizeof( order->Ask_Offer_Qty));
			DBLtoS02( order->Ask_Offer_Prc,			ord->ask_prc,	sizeof( order->Ask_Offer_Prc));
			DBLtoS00( order->Bid_Offer_Qty,			ord->bid_vol,	sizeof( order->Bid_Offer_Qty));
			DBLtoS02( order->Bid_Offer_Prc,			ord->bid_prc,	sizeof( order->Bid_Offer_Prc));
			STRtoSTR( order->Order_Condition,		"0",			sizeof( order->Order_Condition));
			STRtoSTR( order->Investor_Type,			"4000",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"16",			sizeof( order->Account_Type));
			break;
		case 1:
			LogDbg( "정정 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( old->ord_no), old->ord_no);
			LogDbg( "    매도     =[%10.2f][%10.0f]", old->ask_prc, old->ask_vol);
			LogDbg( "    매수     =[%10.2f][%10.0f] ->", old->bid_prc, old->bid_vol);
			LogDbg( "    주문번호 =[%.*s]", sizeof( ord->ord_no), ord->ord_no);
			LogDbg( "    매도     =[%10.2f][%10.0f]", ord->ask_prc, ord->ask_vol);
			LogDbg( "    매수     =[%10.2f][%10.0f]", ord->bid_prc, ord->bid_vol);
			memcpy( ord->org_no, old->ord_no, sizeof( ord->org_no));
			STRtoSTR( order->Transaction_Code,		"TCHMOR40002",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo, 				ord->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo, 		old->ord_no, 	sizeof( order->OriginalOrderNo));
			STRtoSTR( order->New_Modify_Cancel_gbn, "2", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->Ask_Offer_Qty,			ord->ask_vol,	sizeof( order->Ask_Offer_Qty));
			DBLtoS02( order->Ask_Offer_Prc,			ord->ask_prc,	sizeof( order->Ask_Offer_Prc));
			DBLtoS00( order->Bid_Offer_Qty,			ord->bid_vol,	sizeof( order->Bid_Offer_Qty));
			DBLtoS02( order->Bid_Offer_Prc,			ord->bid_prc,	sizeof( order->Bid_Offer_Prc));
			STRtoSTR( order->Order_Condition,		"0",			sizeof( order->Order_Condition));
			STRtoSTR( order->Investor_Type,			"    ",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"  ",			sizeof( order->Account_Type));
			break;
		case 2:
			LogDbg( "취소 주문");
			LogDbg( "    주문번호 =[%.*s]", sizeof( ord->ord_no), ord->ord_no);
			LogDbg( "    매도     =[%10.2f][%10.0f]", ord->ask_prc, ord->ask_vol);
			LogDbg( "    매수     =[%10.2f][%10.0f]", ord->bid_prc, ord->bid_vol);
			memcpy( ord->org_no, old->ord_no, sizeof( ord->org_no));
			STRtoSTR( order->Transaction_Code,		"TCHMOR40003",	sizeof( order->Transaction_Code));
			STRtoSTR( order->OrderNo,				ord->ord_no, 	sizeof( order->OrderNo));
			STRtoSTR( order->OriginalOrderNo,		old->ord_no, 	sizeof( order->OriginalOrderNo));
			STRtoSTR( order->New_Modify_Cancel_gbn, "3", 			sizeof( order->New_Modify_Cancel_gbn));
			DBLtoS00( order->Ask_Offer_Qty,			0.0,			sizeof( order->Ask_Offer_Qty));
			DBLtoS02( order->Ask_Offer_Prc,			0.0,			sizeof( order->Ask_Offer_Prc));
			DBLtoS00( order->Bid_Offer_Qty,			0.0,			sizeof( order->Bid_Offer_Qty));
			DBLtoS02( order->Bid_Offer_Prc,			0.0,			sizeof( order->Bid_Offer_Prc));
			STRtoSTR( order->Order_Type,			" ",			sizeof( order->Order_Type));
			STRtoSTR( order->Order_Condition,		" ",			sizeof( order->Order_Condition));
			STRtoSTR( order->Investor_Type,			"    ",			sizeof( order->Investor_Type));
			STRtoSTR( order->Account_Type,			"  ",			sizeof( order->Account_Type));
			DBLtoS00( order->Mm_Order_Type_No,		0.0,			sizeof( order->Mm_Order_Type_No));
			break;
		default:
			LogDbg( "주문오류");
			break;
	}

	STRtoSTR( ma->auto_yn,					"A",			sizeof( ma->auto_yn));
	STRtoSTR( ma->strategy,					"ST",			sizeof( ma->strategy));
	STRtoSTR( ma->stg_no,					"01",			sizeof( ma->stg_no));
	STRtoSTR( ma->market,					"02",			sizeof( ma->market));
	INTtoSTR( ma->index,					tbl->item_no,	sizeof( ma->index));
	INTtoSTR( ma->acc_seq,					1,				sizeof( ma->acc_seq));
	STRtoSTR( ma->stock,					"0",			sizeof( ma->stock));
	STRtoSTR( ma->spread,					"0",			sizeof( ma->spread));
	STRtoSTR( ma->proc_nm,					&tbl->stg_id[3],sizeof( ma->proc_nm));
	KRX_LP_NOTE_JUMUN_DATA_Print( ( KRX_LP_NOTE_JUMUN_DATA *)order);
	Blp_UpdateHoga( blp, tbl, no +1);
	// Od_Write_Data( 1 /* 채권 */, option /* 신규/정정/취소 */, 2 /* 채권 LP */, 3 /* 매수/매도 */, tbl->item_no);
	Od_Write_DataBond( 1, ( char *)order, ( int)sizeof( KRX_LP_NOTE_JUMUN_DATA));

	return 1;
}

/** ***************************************************************************
**  @fu         int Blp_Close( BLP *blp)
**  @param      blp - BLP struct pointer
**  @param      tbl - 종목별 tablp
**  @param      order  - 호가별 주문 상태 
**  @param      option - 0:신규 1:정정 3:취소
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  주문 번호 채번
***************************************************************************** */
#ifndef _OMS_SOURCE_
int Blp_GetOrderNoOMS( BLP *blp, char *rec, int sz)
{
	static long int		seq = 0;

	seq++;
	sprintf( rec, "%0*ld", 10, seq);
	rec[ 10] = 0;
	LogDbg( "Local OrderNo ordno=[%s]", rec);

	return 1;
}
#else
int Blp_GetOrderNoOMS( BLP *blp, char *rec, int sz)
{
    /* ApType별 주문번호 채번(10만번대만 사용, 6자리만 쓴다) */
	GetOrderNo( rec, 10);
	rec[ 10] = 0;
	LogDbg( "Blp_GetOrderNoOMS ordno=[%s]", rec);

	return 1;
}
#endif


