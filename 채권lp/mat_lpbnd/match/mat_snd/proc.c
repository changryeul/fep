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
#include "proc.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;
extern MAT_ORDER	*MatOrder;

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
int Proc_Match( MAT *mat, MATSISE *sise)
{
	int		rtn;
	ORDER	_order, *order = &_order;

	LogDel( "Match Symb=[%.6s] bid=[%9f] ask=[%9f]", 
		sise->symb, sise->bidprc, sise->offerprc);

	rtn = Mat_Match( mat, sise);
	if( rtn < 0)
	{
		LogCri( "Mat_Match error. rtn=[%d]", rtn);
		MATSISE_Print( sise);
		return rtn;
	}
	LogDel( "Mat_Match ... rtn = [%d]", rtn);

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
**  주문 정합 check
***************************************************************************** */
int Proc_OrderConvert( ORDER *order, ORDER_RECV *recv)
{
	memcpy( order->MsgType         , recv->MsgType         		, sizeof( order->MsgType         ));
	memcpy( order->OrdStatus       , "0"                   		, sizeof( order->OrdStatus       ));
	memcpy( order->ExecType        , recv->tran_ptrncd     		, sizeof( order->ExecType        ));
	memcpy( order->LgenNo          , recv->CustID          		, sizeof( order->LgenNo          ));
	memcpy( order->OrgnGb          , recv->OrgnGb          		, sizeof( order->OrgnGb          ));
	memcpy( order->BkNo            , recv->BkNo            		, sizeof( order->BkNo            ));
	memcpy( order->ClOrdID         , recv->ClOrdID         		, sizeof( order->ClOrdID         ));
	memcpy( order->OrigClOrdID     , recv->OrigClOrdID     		, sizeof( order->OrigClOrdID     ));
	memcpy( order->Currency        , recv->Currency        		, sizeof( order->Currency        ));
	        order->OrderQty        = AtoD( recv->OrderQty  		, sizeof( recv->OrderQty         ));
	memcpy( order->OrdType         , recv->OrdType         		, sizeof( order->OrdType         ));
	memcpy( order->SlipCmpPrice    , recv->SlipCmpPrice    		, sizeof( order->SlipCmpPrice    ));
	memcpy( order->SlipPip         , recv->SlipPip         		, sizeof( order->SlipPip         ));
	        order->CumQty          = 0.0D;
	        order->LastQty         = 0.0D;
	        order->LeavesQty       = 0.0D;
	memset( order->OrdID           , 0x20         		        , sizeof( order->OrdID           ));
	memset( order->ExecID          , 0x20         		        , sizeof( order->ExecID          ));
	        order->Price           = AtoD( recv->Price     		, sizeof( recv->Price            ));
	        order->LastPx1         = 0.0D;
	        order->LastPx2         = 0.0D;
	        order->LastPx3         = 0.0D;
	memcpy( order->Side            , recv->Side            		, sizeof( order->Side            ));
	memcpy( order->TimeInForce     , recv->TimeInForce     		, sizeof( order->TimeInForce     ));
	memcpy( order->RefuslCd        , "0000000000"          		, sizeof( order->RefuslCd        ));
	memset( order->Text            , ' '                   		, sizeof( order->Text            ));
	memcpy( order->TransactTime    , recv->TransactTime    		, sizeof( order->TransactTime    ));
	memcpy( order->SettType        , recv->SettType        		, sizeof( order->SettType        ));
	
	memcpy( order->Symbol          , recv->Symbol          		, sizeof( order->Symbol          ));
	memcpy( order->ValueDate1      , recv->ValueDate1      		, sizeof( order->ValueDate1      ));
	memcpy( order->ValueDate2      , recv->ValueDate2      		, sizeof( order->ValueDate2      ));
	memset( order->OfprKeyVal      , '0'                   		, sizeof( order->OfprKeyVal      ));
	
	memcpy( order->NearSettType    , recv->NearSettType    		, sizeof( order->NearSettType    ));
	
	memcpy( order->NearLegSide     , recv->NearLegSide     		, sizeof( order->NearLegSide     ));
	memcpy( order->NearLegSettlDate, recv->NearLegSettlDate		, sizeof( order->NearLegSettlDate));
	        order->NearLegPrice1   = AtoD( recv->NearLegPrice   , sizeof( recv->NearLegPrice     ));
	        order->NearLegPrice2   = AtoD( recv->NearLegPrice   , sizeof( recv->NearLegPrice     ));
	        order->NearLegPrice3   = AtoD( recv->NearLegPrice   , sizeof( recv->NearLegPrice     ));
	        order->NearLegPriceSprd= AtoD( recv->NearLegPriceSprd, sizeof( recv->NearLegPriceSprd ));
	memcpy( order->FarSettType     , recv->FarSettType     		, sizeof( order->FarSettType     ));
	
	memcpy( order->FarLegSide      , recv->FarLegSide      		, sizeof( order->FarLegSide      ));
	memcpy( order->FarLegSettlDate , recv->FarLegSettlDate 		, sizeof( order->FarLegSettlDate ));
	        order->FarLegPrice1    = AtoD( recv->FarLegPrice    , sizeof( recv->FarLegPrice      ));
	        order->FarLegPrice2    = AtoD( recv->FarLegPrice    , sizeof( recv->FarLegPrice      ));
	        order->FarLegPrice3    = AtoD( recv->FarLegPrice    , sizeof( recv->FarLegPrice      ));
	        order->FarLegPriceSprd = AtoD( recv->FarLegPriceSprd, sizeof( recv->FarLegPriceSprd  ));
	memcpy( order->filler          , recv->Filler          		, sizeof( order->filler          ));
	memcpy( order->Eof             , recv->Eof             		, sizeof( order->Eof             ));

	/*
	ORDER_Print( order);
	*/

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
**  주문 정합 check
***************************************************************************** */
int Proc_SendConvert( ORDER_SEND *send, ORDER *order)
{
	char	conv_buf[ 1024];
	int		dp = 4;

	memcpy( send->MsgType         , order->MsgType         , sizeof(send->MsgType         ));    
	memcpy( send->OrdStatus       , order->OrdStatus       , sizeof(send->OrdStatus       ));    
	memcpy( send->ExecType        , order->ExecType        , sizeof(send->ExecType        ));    
	/*
	memcpy( send->                , order->OrgnGb          , sizeof(send->                ));    
	*/
	memcpy( send->LgenNo          , order->LgenNo          , sizeof(send->LgenNo          ));    
	memcpy( send->BkNo            , order->BkNo            , sizeof(send->BkNo            ));    
	memcpy( send->ClOrdID         , order->ClOrdID         , sizeof(send->ClOrdID         ));    
	memcpy( send->OrigClOrdID     , order->OrigClOrdID     , sizeof(send->OrigClOrdID     ));    
	memcpy( send->Currency        , order->Currency        , sizeof(send->Currency        ));    
	DtoA0ND( send->OrderQty       , order->OrderQty        , sizeof(send->OrderQty        ), dp);    
	/*
	memcpy( send->                , order->SlipCmpPrice    , sizeof(send->                ));    
	memcpy( send->                , order->SlipPip         , sizeof(send->                ));    
	DtoA  ( send->                , order->OrderQty        , sizeof(send->                ));    
	*/
	DtoA0ND( send->CumQty         , order->CumQty          , sizeof(send->CumQty          ), dp);    
	DtoA0ND( send->LastQty        , order->LastQty         , sizeof(send->LastQty         ), dp);    
	DtoA0ND( send->LeavesQty      , order->LeavesQty       , sizeof(send->LeavesQty       ), dp);    
	memcpy( send->OrdID           , order->OrdID           , sizeof(send->OrdID           ));
	memcpy( send->ExecID          , order->ExecID          , sizeof(send->ExecID          ));
	DtoA0ND( send->Price          , order->Price           , sizeof(send->Price           ), dp);    
	DtoA0ND( send->LastPx1        , order->LastPx1         , sizeof(send->LastPx1         ), dp);    
	DtoA0ND( send->LastPx2        , order->LastPx2         , sizeof(send->LastPx2         ), dp);    
	DtoA0ND( send->LastPx3        , order->LastPx3         , sizeof(send->LastPx3         ), dp);    
	memcpy( send->Side            , order->Side            , sizeof(send->Side            ));    
	memcpy( send->TimeInForce     , order->TimeInForce     , sizeof(send->TimeInForce     ));    
	memcpy( send->RefuslCd        , order->RefuslCd        , sizeof(send->RefuslCd        ));    
	memcpy( send->Text            , order->Text            , sizeof(send->Text            ));    
	memcpy( send->TransactTime    , order->TransactTime    , sizeof(send->TransactTime    ));    
	memcpy( send->SettType        , order->SettType        , sizeof(send->SettType        ));    
	memcpy( send->Symbol          , order->Symbol          , sizeof(send->Symbol          ));    
	memcpy( send->ValueDate1      , order->ValueDate1      , sizeof(send->ValueDate1      ));    
	memcpy( send->ValueDate2      , order->ValueDate2      , sizeof(send->ValueDate2      ));    
	memcpy( send->OfprKeyVal      , order->OfprKeyVal      , sizeof(send->OfprKeyVal      ));    
	memcpy( send->NearSettType    , order->NearSettType    , sizeof(send->NearSettType    ));    
	memcpy( send->NearLegSide     , order->NearLegSide     , sizeof(send->NearLegSide     ));    
	memcpy( send->NearLegSettlDate, order->NearLegSettlDate, sizeof(send->NearLegSettlDate));    
	DtoA0ND( send->NearLegPrice1  , order->NearLegPrice1   , sizeof(send->NearLegPrice1   ), dp);    
	DtoA0ND( send->NearLegPrice2  , order->NearLegPrice2   , sizeof(send->NearLegPrice2   ), dp);    
	DtoA0ND( send->NearLegPrice3  , order->NearLegPrice3   , sizeof(send->NearLegPrice3   ), dp);    
	DtoA0ND( send->NearLegPriceSprd, order->NearLegPriceSprd, sizeof(send->NearLegPriceSprd), dp);    
	memcpy( send->FarSettType     , order->FarSettType     , sizeof(send->FarSettType     ));    
	memcpy( send->FarLegSide      , order->FarLegSide      , sizeof(send->FarLegSide      ));    
	memcpy( send->FarLegSettlDate , order->FarLegSettlDate , sizeof(send->FarLegSettlDate ));    
	DtoA0ND( send->FarLegPrice1   , order->FarLegPrice1    , sizeof(send->FarLegPrice1    ), dp);    
	DtoA0ND( send->FarLegPrice2   , order->FarLegPrice2    , sizeof(send->FarLegPrice2    ), dp);    
	DtoA0ND( send->FarLegPrice3   , order->FarLegPrice3    , sizeof(send->FarLegPrice3    ), dp);    
	DtoA0ND( send->FarLegPriceSprd, order->FarLegPriceSprd , sizeof(send->FarLegPriceSprd ), dp);    
	memcpy( send->filler          , order->filler          , sizeof(send->filler          ));    
	memcpy( send->Eof             , order->Eof             , sizeof(send->Eof             ));    

	return 1;
}

