/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
**	mat.c			- 매칭 시스템 주요 모듈 생성/삭제/오픈/클로즈
**	mat_sise.c		- 매칭시스템 시세 처리
**	mat_stat.c		- 통계처리 상태출력
**	mat_order.c		- 주문 처리
**	mat_match.c		- 매칭 처리
**	mat_execute.c	- 체결 처리
**	mat_group.c		- 그룹주문 처리
**	convert.c		- 주문 구조체 변환
**	print.c			- 매칭관련 구조체 출력
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "log.h"
#include "etc.h"
#include "mat.h"

#include "order.h"
#include "sise.h"

extern int		Continue;

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  장운영 - 상품코드 번호 return
***************************************************************************** */
int Mat_GetJangPdcdNo( MAT *mat, char *Pdcd)
{
	int		rtn = -1;

	if(      !strncmp( Pdcd, "BAR", 3))			rtn = 0;
	else if( !strncmp( Pdcd, "SPT", 3))			rtn = 1;
	else if( !strncmp( Pdcd, "FWD", 3))			rtn = 2;
	else if( !strncmp( Pdcd, "SWP", 3))			rtn = 3;
	else if( !strncmp( Pdcd, "MAR", 3))			rtn = 4;

	return rtn;
}

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  장운영 - 테너 번호 return
***************************************************************************** */
int Mat_GetJangTnrNo( MAT *mat, char *TnrId)
{
	if(      !strncmp( TnrId, "TOD", 3))			return 1;
	else if( !strncmp( TnrId, "TOM", 3))			return 2;
	else if( !strncmp( TnrId, "SPT", 3))			return 3;
	else if( !strncmp( TnrId, "W01", 3))			return 0;
	else if( !strncmp( TnrId, "M01", 3))			return 0;
	else if( !strncmp( TnrId, "M02", 3))			return 0;
	else if( !strncmp( TnrId, "M03", 3))			return 0;
	else if( !strncmp( TnrId, "M06", 3))			return 0;
	else if( !strncmp( TnrId, "Y01", 3))			return 0;
	else if( !strncmp( TnrId, "ALL", 3))			return 0;
	else if( !strncmp( TnrId, "   ", 3))			return 0;
	else											return -1;
}

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @param      order*	- 주문 record
**  @param      flag	- 0-장 check, 1-장운영은 check하지 않고 id만 get
**  @return     성공    - 양수 jang_id
**  @retval     실패    - 음수 error code
**  @brief
**  장운영
**  장운영 recort 위치 get
***************************************************************************** */
int Mat_GetJangId( MAT *mat, ORDER *order, int flag)
{
	int		i;
	int		code;
	int		base_curr, cont_curr;
	int		check = 1;		/* 장운영 체크 여부 */
	int		curr = -1;		/* 통화코드 번호 */
	int		hoga = -1;		/* 0:시장가 1:지정가 */
	int		prod = -1;		/* 상품번호 */
	int		tnr  = -1;		/* 테너번호 */
	char	key_str[ 32];
	time_t			cur_time;
	int				jang_id = -1;
	MAT_JANG		*jang;
	MAT_JANG_REC	*jp;

	char	*hoga_str[ 8]  = { "M", "F", NULL};
	char	*prod_str[ 16] = { "BAR", "SPT", "FWD", "SWP", "MAR", NULL};
	char	*tnr_str[ 16]  = { "ALL", "TOD", "TOM", "SPT", NULL};

	if( flag == 1)	check = 0;	/* flag가 1일경우 - jang_id만 return */

	time( &cur_time);

	/************************/
	/* 주문 stop check      */
	/* 모든 주문 불가       */
	/************************/
	/* 엔진 장운영 check */
   	if( ( cur_time >= mat->map->stat.cancel_start) && ( cur_time < mat->map->stat.cancel_end))
   	{
   		LogMsg( "주문 불가 시간 check");
   		LogMsg( "        start=[%s]", TtoS( mat->map->stat.cancel_start));
   		LogMsg( "        curr =[%s]", TtoS( cur_time));
   		LogMsg( "        end  =[%s]", TtoS( mat->map->stat.cancel_end));
       	code = 82102;	/* 매매 가능 시간이 아닙니다. */
       	LogCri( "주문 불가 시간 error. code=[%d]", code);
       	return -code;
	}

	/************************/
	/* 전체 장운영 check    */
	/************************/
	/* 장운영 check 하지 않는 예외 */
	/* 주문타입 = 대행, RFQ */
	if( order->TrdTypeDcd[ 0] == '3')		
	{
		LogMsg( "대행주문 - 장 체크 없음 order->TrdTypeDcd[ 0]=[%c]", order->TrdTypeDcd[ 0]);
		check = 0;
		// return 1;           
	}
	/* 주문유형 = 예약주문 */
	if( order->OrdType[ 0] == '3')			
	{
		LogMsg( "예약주문 - 장 체크 없음 order->OrdType[ 0]=[%c]", order->OrdType[ 0]);
		check = 0;
		// return 1;           
	}

    /* 장 마감 이후만 에러 - 장 전 주문은 OK */
	if( check)
	{
    	if( /* ( cur_time < mat->map->stat.start) || */( cur_time > mat->map->stat.end))
    	{
    		LogMsg( "전체 장시간 check");
    		LogMsg( "        start=[%s] - 장전 주문 체크안함", TtoS( mat->map->stat.start));
    		LogMsg( "        curr =[%s]", TtoS( cur_time));
    		LogMsg( "        end  =[%s]", TtoS( mat->map->stat.end));
        	code = 82102;	/* 매매 가능 시간이 아닙니다. */
        	LogCri( "전체 장시간 error. code=[%d]", code);
        	return -code;
    	}
    	/* 월말 매매 가능 시간 check */
		/* 사용안함 - 배치에서 체크 하지만 월말 매매시간을 전체 장운영 시간과 같이 만듬 mat_bat.cfg */
    	if( mat->map->stat.last_day)	
    	{
        	time( &cur_time);
        	if( mat->map->stat.last_time > 0)
        	{
            	if( cur_time >= mat->map->stat.last_time)
            	{
                	LogCri( "월말 매매가능 시간 아님. 거부처리");
                	code = 82201;	/* "82201  :월말 매매가능 시간이 아닙니다." */
                	return -code;
            	}
        	}
    	}
		LogDbg( "전체 장시간 OK");
	}
	else
	{
   		LogMsg( "전체 장시간 check 예외 적용");
		return 1;
	}

	/************************/
	/* 통화코드             */
	/************************/
	base_curr = Mat_GetCurrentInt( mat, &order->Symbol[ 0]);		/* 기준통화 */
	cont_curr = Mat_GetCurrentInt( mat, &order->Symbol[ 4]);		/* 상대통화 */
	LogDbg( "Symbol=[%.7s] base=[%d] cont=[%d]", &order->Symbol, base_curr, cont_curr);
	if( base_curr == 1)			/* USD */
	{
		if( cont_curr == 0)		/* USD/KRW */
		{
			curr = base_curr;	/* USD/KRW는 USD 장운영 적용 */
		}
		else
		{
			curr = cont_curr;	/* USD/XXX는 USD외의 장운영 적용 */
		}
	}
	else						/* 재정 이종 */
	{
		if( cont_curr == 0)		/* 재정 */
		{
			curr = base_curr;	/* 재정은 기본통화 장운영으로 */
		}
		else					/* 이종 - 영연방:외국통화 표시법 cont_curr는 무조건 USE(1) 이다 */
		{
			curr = base_curr;	/* USD가 아닌 통화 */
		}
	}
	if( curr < 0)
	{
		LogCri( "통화코드가 없습니다. order->Symbol=[%.7s] base=[%d] cont=[%d]", order->Symbol, base_curr, cont_curr);
		code = 82202;			/* "82202  :통화 코드를 확인 하세요." */
		return -code;
	}
	memcpy( &key_str[ 0], Mat_GetCurrentString( mat, curr), 3);
	LogDbg( "통화코드 =[%.3s:%d] [%.7s]", &key_str[ 0], curr, &order->Symbol[ 0]);

	/**************************/
	/* 호가코드 시장가/지정가 */
	/**************************/
	hoga = order->OrdType[ 0] - '1';	/* 0:시장가, 1:지정가 예약주문 없음 */
	if( hoga < 0 || hoga > 1)
	{
		LogCri( "주문유형이 없습니다. order->OrdType=[%.1s]", order->OrdType);
		code = 82203;			/* "82203  :호가 유형을 확인 하세요." */
		return -code;

	}
	memcpy( &key_str[ 3], hoga_str[ hoga], 1);
	LogDbg( "호가구분 =[%.1s  :%d] [%.1s]", &key_str[ 3], hoga, order->OrdType);

	/************************/
	/* 상품구분             */
	/************************/
	if( order->TranPtrnCd[ 0] == '2')	prod = 4;						/* MAR */
	else
	if( order->TranPtrnCd[ 0] == '1' || order->TranPtrnCd[ 0] == '3')	/* 일반, RFQ */
	{
		prod = Mat_GetJangPdcdNo( mat, order->SettType);					/* 상품구분 코드 get */
		if( prod < 0)
		{
			LogCri( "상품 장운영이 없습니다. order->SettType=[%.3s]", order->SettType);
			code = 82204;		/* "82204  :상품구분을 확인 하세요." */
			return -code;
		}
	}
	else
	{
		LogCri( "거레유형코드 error. order->TranPtrnCd[ 0] = [%c]", order->TranPtrnCd[ 0]);
		code = 82205;			/* "82205  :거래유형 코드를 확인 하세요." */
		return -code;
	}
	if( prod < 0)
	{
		LogCri( "상품구분코드 error. order->TranPtrnCd[ 0]=[%c] order->SettType=[%.3s]", order->TranPtrnCd[ 0], order->SettType);
		code = 82204;			/* "82204  :상품구분을 확인 하세요." */
		return -code;
	}
	memcpy( &key_str[ 4], prod_str[ prod], 3);
	LogDbg( "상품구분 =[%.3s:%d] [%.1s][%.3s]", &key_str[ 4], prod, order->TranPtrnCd, order->SettType);

	/************************/
	/* 테너                 */
	/************************/
	if( prod == 4)	/* MAR는 전체 장운영 적용 - 무조건 ALL */
	{
		tnr = 0;
	}
	else
	if( prod == 0)	/* 바로환전은 전체 장운영 적용 - 무조건 ALL */
	{
		tnr = 0;
	}
	else
	if( prod == 3)	/* SWAP일때는 NEAR의 tenor를 적용하여 check - swap은 near만 체크하면 된다 */
	{
		tnr = Mat_GetJangTnrNo( mat, order->NearTnrId);
	}
	else			/* 일반 상품 */
	{
		tnr = Mat_GetJangTnrNo( mat, order->TnrId);
		if( tnr < 0)
		{
			LogCri( "테너정보 오류. order->TndId=[%.3s]", order->TnrId);
			code = 82206;			/*  "82206  :테너 정보 오류입니다." */
			return -code;
		}
	}
	memcpy( &key_str[ 7], tnr_str[ tnr], 3);
	LogDbg( "테너구분 =[%.3s:%d] [%.3s]", tnr_str[ tnr], tnr, order->TnrId);

	key_str[ 10] = 0;
	LogDbg( "Key      =[%s]", key_str);

	jang = &mat->map->jang;

	LogDbg( "jang->cnt=[%d]", jang->cnt);
	for( i = 1; i < jang->cnt; i++)
	{
		jp = &jang->rec[ i];
		if( jp->used != 1) continue;
		if( !memcmp( jp->key, key_str, 10)) break;
	}

	if( i >= jang->cnt)
	{
		code = 82102;	/* "82102  :매매 가능 시간이 아닙니다." */
		LogCri( "jang not found. 매매 가능 시간이 아닙니다.");
		return -code;
	}

	jang_id = i;

	LogDbg( "curr     =[%s]", TtoS( cur_time));
	LogDbg( "start    =[%s]", TtoS( jp->start));
	LogDbg( "end      =[%s]", TtoS( jp->end));
	LogDbg( "jang_id  =[%d]", jang_id);

	if( check == 0) return jang_id;

	/************************/
	/* check                */
	/************************/
	/* memory */
	if( jp->used != 1)
	{
		LogCri( "장운영에 오류 ... jp->used=[%d]", jp->used);
		code = 82207;			/* "82207  :거래 가능시간 체크중 오류가 빌생 했습니다.." */
		return -code;
	}
	/* day */
	if( jp->start == 0)
	{
		LogCri( "거래가능일 아님 ... jang_id=[%d]", jang_id);
		code = 82211;			/* "82211  :해당 통화의 거래가능 날짜가 아닙니다." */
		return -code;
	}
	/* start */
	if( cur_time < jp->start)
	{
		LogCri( "시작시간 아님 ... jang_id=[%d]", jang_id);
		code = 82208;			/* "82208  :거래 가능시간 전입니다.." */
		return -code;
	}
	/* end */
	if( cur_time > jp->end)
	{
		LogCri( "장 끝남 ... jang_id=[%d]", jang_id);
		code = 82209;			/* "82209  :거래 가능시간이 지났습니다.." */
		return -code;
	}
	LogDbg( "장운영 check OK.");

	return jang_id;
}

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @param      order*	- 주문 record
**  @param      jang_id	- 주문에서 찾은 장 ID : head->jang_id
**  @return     성공    - 양수 jang_id
**  @retval     실패    - 음수 error code
**  @brief
**  장운영
**  flag 0-주문 장운영, 1-체결 장운영
**  체결 장 check
***************************************************************************** */
int Mat_JangCheck( MAT *mat, ORDER *order, int jang_id)
{
	int				code;
	time_t			cur_time;
	MAT_JANG_REC	*jp;

	time( &cur_time);
	jp = &mat->map->jang.rec[ jang_id];

	/************************/
	/* 전체 장운영 check    */
	/************************/
	/* 장운영 check 하지 않는 예외 */
	if( order->TrdTypeDcd[ 0] == '3')		return 1;           /* 주문타입 = 대행, RFQ */
	if( order->OrdType[ 0] == '3')			return 1;           /* 주문유형 = 예약주문 */

    /* 장 마감 이후만 에러 - 장 전 주문은 OK */
    if( /* ( cur_time < mat->map->stat.start) || */( cur_time > mat->map->stat.end))
    {
    	LogMsg( "전체 장시간 check");
    	LogMsg( "        start=[%s] - 장전 주문 체크안함", TtoS( mat->map->stat.start));
    	LogMsg( "        curr =[%s]", TtoS( cur_time));
    	LogMsg( "        end  =[%s]", TtoS( mat->map->stat.end));
        code = 82102;	/* 매매 가능 시간이 아닙니다. */
        LogCri( "전체 장시간 error. code=[%d]", code);
        return -code;
    }
    /* 월말 매매 가능 시간 check */
	/* 사용안함 - 배치에서 체크 하지만 월말 매매시간을 전체 장운영 시간과 같이 만듬 mat_bat.cfg */
    if( mat->map->stat.last_day)	
    {
        time( &cur_time);
        if( mat->map->stat.last_time > 0)
        {
            if( cur_time >= mat->map->stat.last_time)
            {
                LogCri( "월말 매매가능 시간 아님. 거부처리");
                code = 82201;	/* "82201  :월말 매매가능 시간이 아닙니다." */
                return -code;
            }
        }
    }
	LogDbg( "전체 장시간 OK");

	/************************/
	/* 개별통화 장운영      */
	/************************/
	/* memory */
	if( jp->used != 1)
	{
		LogDbg( "장운영에 오류 ... jang_id=[%d]", jang_id);
		code = 82207;			/* "82207  :거래 가능시간 체크중 오류가 빌생 했습니다.." */
		return -code;
	}
	/* day */
	if( jp->start == 0)
	{
		LogDbg( "거래가능일 아님 ... jang_id=[%d]", jang_id);
		code = 82211;			/* "82211  :해당 통화의 거래가능 날짜가 아닙니다." */
		return -code;
	}
	/* start */
	if( cur_time < jp->start)
	{
		LogDbg( "시작시간 아님 ... jang_id=[%d]", jang_id);
		code = 82208;			/* "82208  :거래 가능시간 전입니다.." */
		return -code;
	}
	/* end */
	if( cur_time > jp->end)
	{
		LogDbg( "장 끝남 ... jang_id=[%d]", jang_id);
		code = 82209;			/* "82209  :거래 가능시간이 지났습니다.." */
		return -code;
	}
	LogDbg( "장운영 check OK. jang_id=[%d]", jang_id);

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @param      order*	- 주문 record
**  @param      flag	- 0-주문, 1-체결
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  장운영
**  flag 0-주문 장운영, 1-체결 장운영
***************************************************************************** */
int Mat_JangCheckSwap( MAT *mat, int curr, int hoga, ORDER *order, int flag)
{
	int		code;
	int		base_curr, cont_curr;
	int		near_prod = -1, far_prod = -1;		/* 상품번호 */
	int		near_tnr  = -1, far_tnr = -1;		/* 테너번호 */
	time_t			cur_time;
	MAT_JANG_TIME	*jang_time;

	char	*hoga_str[ 3] = { "시장", "지정", NULL};
	char	*prod_str[ 6] = { "BAR", "SPT", "FWD", "SWP", "MAR", NULL};
	char	*tnr_str[ 5]  = { "ALL", "TOD", "TOM", "SPT", NULL};


	return 1;

	time( &cur_time);

	/*********************************************************************************/
	/* NEAR                                                                          */
	/*********************************************************************************/
	/************************/
	/* 상품구분             */
	/************************/
	if( order->TranPtrnCd[ 0] == '2')	near_prod = 4;						/* MAR */
	else
	if( order->TranPtrnCd[ 0] == '1' || order->TranPtrnCd[ 0] == '3')	/* 일반, RFQ */
	{
		near_prod = Mat_GetJangPdcdNo( mat, order->SettType);					/* 상품구분 코드 get */
		if( near_prod < 0)
		{
			LogCri( "상품 장운영이 없습니다. order->SettType=[%.3s]", order->SettType);
			code = 82204;		/* "82204  :상품구분을 확인 하세요." */
			return -code;
		}
	}
	else
	{
		LogCri( "거레유형코드 error. order->TranPtrnCd[ 0] = [%c]", order->TranPtrnCd[ 0]);
		code = 82205;			/* "82205  :거래유형 코드를 확인 하세요." */
		return -code;
	}
	if( near_prod < 0)
	{
		LogCri( "상품구분코드 error. order->TranPtrnCd[ 0]=[%c] order->SettType=[%.3s]", order->TranPtrnCd[ 0], order->SettType);
		code = 82204;			/* "82204  :상품구분을 확인 하세요." */
		return -code;
	}
	LogDbg( "상품구분 =[%.3s :%d] [%.1s]", prod_str[ near_prod], near_prod, order->TranPtrnCd);

	/************************/
	/* 테너                 */
	/************************/
	near_tnr = Mat_GetJangTnrNo( mat, order->TnrId);
	if( near_tnr < 0)
	{
		LogCri( "테너정보 오류. order->TndId=[%.3s]", order->TnrId);
		code = 82206;			/*  "82206  :테너 정보 오류입니다." */
		return -code;
	}
	LogDbg( "테너구분 =[%.3s :%d] [%.3s]", tnr_str[ near_tnr], near_tnr, order->TnrId);

	/************************/
	/* check                */
	/************************/
	// jang_time = &mat->map->jang.rec[ curr][ hoga][ near_prod][ near_tnr];
	LogDbg( "장운영 사용 = [%d]", jang_time->used);
	LogDbg( "       통화 = [%d]", curr);
	LogDbg( "       호가 = [%d]", hoga);
	LogDbg( "       상품 = [%d]", near_prod);
	LogDbg( "       테너 = [%d]", near_tnr);
	LogDbg( "       ptr  = [%p]", jang_time);

	/* memory */
	if( jang_time->used != 1)
	{
		LogCri( "장운영에 오류 ... jang_time->used=[%d]", jang_time->used);
		code = 82207;			/* "82207  :거래 가능시간 체크중 오류가 빌생 했습니다.." */
		return -code;
	}
	/* start */
	if( cur_time < jang_time->start)
	{
		LogCri( "시작시간 아님 ... jang_time->used=[%d]", jang_time->used);
		code = 82208;			/* "82208  :거래 가능시간 전입니다.." */
		return -code;
	}
	/* end */
	if( cur_time > jang_time->end)
	{
		LogCri( "장 끝남 ... jang_time->used=[%d]", jang_time->used);
		code = 82209;			/* "82209  :거래 가능시간이 지났습니다.." */
		return -code;
	}
	LogDbg( "NEAR 장운영 check OK.");

	/*********************************************************************************/
	/* FAR                                                                           */
	/*********************************************************************************/
	/************************/
	/* 상품구분             */
	/************************/
	if( order->TranPtrnCd[ 0] == '2')	far_prod = 4;						/* MAR */
	else
	if( order->TranPtrnCd[ 0] == '1' || order->TranPtrnCd[ 0] == '3')	/* 일반, RFQ */
	{
		far_prod = Mat_GetJangPdcdNo( mat, order->SettType);					/* 상품구분 코드 get */
		if( far_prod < 0)
		{
			LogCri( "상품 장운영이 없습니다. order->SettType=[%.3s]", order->SettType);
			code = 82204;		/* "82204  :상품구분을 확인 하세요." */
			return -code;
		}
	}
	else
	{
		LogCri( "거레유형코드 error. order->TranPtrnCd[ 0] = [%c]", order->TranPtrnCd[ 0]);
		code = 82205;			/* "82205  :거래유형 코드를 확인 하세요." */
		return -code;
	}
	if( far_prod < 0)
	{
		LogCri( "상품구분코드 error. order->TranPtrnCd[ 0]=[%c] order->SettType=[%.3s]", order->TranPtrnCd[ 0], order->SettType);
		code = 82204;			/* "82204  :상품구분을 확인 하세요." */
		return -code;
	}
	LogDbg( "상품구분 =[%.3s :%d] [%.1s]", prod_str[ far_prod], far_prod, order->TranPtrnCd);

	/************************/
	/* 테너                 */
	/************************/
	far_tnr = Mat_GetJangTnrNo( mat, order->TnrId);
	if( far_tnr < 0)
	{
		LogCri( "테너정보 오류. order->TndId=[%.3s]", order->TnrId);
		code = 82206;			/*  "82206  :테너 정보 오류입니다." */
		return -code;
	}
	LogDbg( "테너구분 =[%.3s :%d] [%.3s]", tnr_str[ far_tnr], far_tnr, order->TnrId);

	/************************/
	/* check                */
	/************************/
	// jang_time = &mat->map->jang.rec[ curr][ hoga][ far_prod][ far_tnr];
	LogDbg( "장운영 사용 = [%d]", jang_time->used);
	LogDbg( "       통화 = [%d]", curr);
	LogDbg( "       호가 = [%d]", hoga);
	LogDbg( "       상품 = [%d]", far_prod);
	LogDbg( "       테너 = [%d]", far_tnr);
	LogDbg( "       ptr  = [%p]", jang_time);

	/* memory */
	if( jang_time->used != 1)
	{
		LogCri( "장운영에 오류 ... jang_time->used=[%d]", jang_time->used);
		code = 82207;			/* "82207  :거래 가능시간 체크중 오류가 빌생 했습니다.." */
		return -code;
	}
	/* start */
	if( cur_time < jang_time->start)
	{
		LogCri( "시작시간 아님 ... jang_time->used=[%d]", jang_time->used);
		code = 82208;			/* "82208  :거래 가능시간 전입니다.." */
		return -code;
	}
	/* end */
	if( cur_time > jang_time->end)
	{
		LogCri( "장 끝남 ... jang_time->used=[%d]", jang_time->used);
		code = 82209;			/* "82209  :거래 가능시간이 지났습니다.." */
		return -code;
	}
	LogDbg( "FAR 장운영 check OK.");

	return 1;
}

/** ***************************************************************************
**  @fu         int Mat_Jang()
**  @param      MAT*	- 엔진 pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  장운영 - string(HHMMSS)을 time_t 값으로 convert
**  오늘날짜 기준 - 단 시간값이 24보다 크면 익일로 환산됨
***************************************************************************** */
time_t Mat_JangHtoT( MAT *mat, char *str_time)
{
	time_t		cur_time;
	time_t		get_time;
	int			int_time;
	struct tm	_tm, *tp = &_tm;
	char		buf[ 32];

	LogDel( "str_time=[%.6s]", str_time);
	memcpy( buf, str_time, 6);
	buf[ 6] = 0;
	/* 999999 이면 거래불가 - time_t 값을 0으로 세팅하여 거래 불가 */
	if( !memcmp( str_time, "999999", 6))
	{
		return 0;
	}

	int_time = atoi( buf);
	LogDel( "int_time=[%06d]", int_time);

	time( &cur_time);
	localtime_r( &cur_time, tp);
	tp->tm_hour = int_time / 10000;
	tp->tm_min  = ( int_time % 10000) / 100;
	tp->tm_sec  = int_time % 100;

	get_time = mktime( tp);

	return get_time;
}



