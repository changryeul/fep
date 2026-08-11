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

#include "log.h"
#include "etc.h"

#include "mat.h"
#include "order.h"
#include "sise.h"

/** ***************************************************************************
**	GLOBAL
***************************************************************************** */
extern int			Continue;

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
**  convert ORDER_RECV to ORDER
***************************************************************************** */
int Proc_OrderConvert( ORDER *order, ORDER_RECV *recv)
{
	memcpy( order->MsgType         , recv->MsgType         		, sizeof( order->MsgType         ));
	memcpy( order->OrdStatus       , "0"                   		, sizeof( order->OrdStatus       ));
	memcpy( order->CustID          , recv->CustID     		    , sizeof( order->CustID          ));
	memcpy( order->FundNO          , recv->FundNO     		    , sizeof( order->FundNO          ));
	memcpy( order->ExecType        , recv->TranPtrnCd     		, sizeof( order->ExecType        ));
	memcpy( order->LgenNo          , recv->CustID          		, sizeof( order->LgenNo          ));
	memcpy( order->OrgnGb          , recv->OrgnGb          		, sizeof( order->OrgnGb          ));
	memcpy( order->BkNo            , recv->BkNo            		, sizeof( order->BkNo            ));
	memset( order->FxBkId          , 0x20         		        , sizeof( order->FxBkId          ));
	memcpy( order->ClOrdID         , recv->ClOrdID         		, sizeof( order->ClOrdID         ));
	memcpy( order->OrigClOrdID     , recv->OrigClOrdID     		, sizeof( order->OrigClOrdID     ));
	memcpy( order->Currency        , recv->Currency        		, sizeof( order->Currency        ));
	        order->OrderQty        = AtoD( recv->OrderQty  		, sizeof( recv->OrderQty         ));
	memcpy( order->OrdType         , recv->OrdType         		, sizeof( order->OrdType         ));
	        order->SlipCmpPrice    = AtoD( recv->SlipCmpPrice   , sizeof( recv->SlipCmpPrice     ));
	        order->SlipPip         = AtoD( recv->SlipPip        , sizeof( recv->SlipPip          ));
	        order->CumQty          = 0.0D;
	        order->LastQty         = 0.0D;
	        order->LeavesQty       = 0.0D;
	memset( order->OrdID           , 0x20         		        , sizeof( order->OrdID           ));
	memset( order->ExecID          , 0x20         		        , sizeof( order->ExecID          ));
	        order->Price           = AtoD( recv->Price     		, sizeof( recv->Price            ));
	        order->PriceSpr        = AtoD( recv->PriceSpr     	, sizeof( recv->PriceSpr         ));
			order->OrdSprPrc       = 0.0D;
	        order->LastPx          = 0.0D;
	        order->LastSprPx       = 0.0D;
	memcpy( order->Side            , recv->Side            		, sizeof( order->Side            ));
	memcpy( order->TimeInForce     , recv->TimeInForce     		, sizeof( order->TimeInForce     ));
	memcpy( order->RefuslCd        , "0000000000"          		, sizeof( order->RefuslCd        ));
	memset( order->Text            , ' '                   		, sizeof( order->Text            ));
	memcpy( order->TransactTime    , recv->TransactTime    		, sizeof( order->TransactTime    ));
	memcpy( order->SettType        , recv->SettType        		, sizeof( order->SettType        ));

	memcpy( order->TnrId           , recv->TnrId        		, sizeof( order->TnrId           ));
	memcpy( order->TnrPtrnDcd      , recv->TnrPtrnDcd     		, sizeof( order->TnrPtrnDcd      ));
	
	memcpy( order->Symbol          , recv->Symbol          		, sizeof( order->Symbol          ));
	memcpy( order->ValueDate1      , recv->ValueDate1      		, sizeof( order->ValueDate1      ));
	memcpy( order->ValueDate2      , recv->ValueDate2      		, sizeof( order->ValueDate2      ));
	memset( order->OfprKeyVal      , ' '                   		, sizeof( order->OfprKeyVal      ));
	
	memcpy( order->NearSettType    , recv->NearSettType    		, sizeof( order->NearSettType    ));
	memcpy( order->NearTnrId       , recv->NearTnrId    		, sizeof( order->NearTnrId       ));
	memcpy( order->NearTnrPtrnDcd  , recv->NearTnrPtrnDcd   	, sizeof( order->NearTnrPtrnDcd  ));
	
	memcpy( order->NearLegSide     , recv->NearLegSide     		, sizeof( order->NearLegSide     ));
	memcpy( order->NearLegSettlDate, recv->NearLegSettlDate		, sizeof( order->NearLegSettlDate));

#if 0
	        order->NearLegMktPrc   = AtoD( recv->NearLegMktPrc  , sizeof( recv->NearLegMktPrc    ));
	        order->NearLegCvPrc    = AtoD( recv->NearLegCvPrc   , sizeof( recv->NearLegCvPrc     ));
	        order->NearLegCoPrc    = AtoD( recv->NearLegCoPrc   , sizeof( recv->NearLegCoPrc     ));
#endif

	        order->NearLegPrice    = AtoD( recv->NearLegPrice   , sizeof( recv->NearLegPrice     ));
	        order->NearLegPriceSprd= AtoD( recv->NearLegPriceSprd, sizeof( recv->NearLegPriceSprd));
			order->NearLegMktPrc   = 0.0D;
        	order->NearLegCvPrc    = 0.0D;
        	order->NearLegCoPrc    = 0.0D;
	        order->NearLegCusPrc   = 0.0D;
	        order->NearLegCusSpr   = 0.0D;

	memcpy( order->FarSettType     , recv->FarSettType     		, sizeof( order->FarSettType     ));
	memcpy( order->FarTnrId        , recv->FarTnrId     		, sizeof( order->FarTnrId     ));
	memcpy( order->FarTnrPtrnDcd   , recv->FarTnrPtrnDcd     	, sizeof( order->FarTnrPtrnDcd     ));
	
	memcpy( order->FarLegSide      , recv->FarLegSide      		, sizeof( order->FarLegSide      ));
	memcpy( order->FarLegSettlDate , recv->FarLegSettlDate 		, sizeof( order->FarLegSettlDate ));

#if 0
	        order->FarLegMktPrc    = AtoD( recv->FarLegMktPrc   , sizeof( recv->FarLegMktPrc     ));
	        order->FarLegCvPrc     = AtoD( recv->FarLegCvPrc    , sizeof( recv->FarLegCvPrc      ));
	        order->FarLegCoPrc     = AtoD( recv->FarLegCoPrc    , sizeof( recv->FarLegCoPrc      ));
#endif

	        order->FarLegPrice     = AtoD( recv->FarLegPrice   , sizeof( recv->FarLegPrice       ));
	        order->FarLegPriceSprd = AtoD( recv->FarLegPriceSprd, sizeof( recv->FarLegPriceSprd  ));
			order->FarLegMktPrc    = 0.0D;
			order->FarLegCvPrc     = 0.0D;
			order->FarLegCoPrc     = 0.0D;
	        order->FarLegCusPrc    = 0.0D;
	        order->FarLegCusSpr    = 0.0D;
			
	memcpy( order->TranPtrnCd      , recv->TranPtrnCd           , sizeof( order->TranPtrnCd      ));
	memcpy( order->GrpOrdnNo       , recv->GrpOrdnNo            , sizeof( order->GrpOrdnNo       ));
	memcpy( order->GrpOrdnCnt      , recv->GrpOrdnCnt           , sizeof( order->GrpOrdnCnt      ));
	memcpy( order->GrpOrdnSeq      , recv->GrpOrdnSeq           , sizeof( order->GrpOrdnSeq      ));

	memcpy( order->GrpFxPdcd       , recv->GrpFxPdcd            , sizeof( order->GrpFxPdcd      ));
	memcpy( order->TrdTypeDcd      , recv->TrdTypeDcd           , sizeof( order->TrdTypeDcd      ));
	        order->SpotPrc         = AtoD( recv->SpotPrc        , sizeof( recv->SpotPrc     ));
	        order->SpotPrcSpr      = AtoD( recv->SpotPrcSpr     , sizeof( recv->SpotPrcSpr  ));
	        order->UsdQty          = AtoD( recv->UsdQty         , sizeof( recv->UsdQty      ));

	memset( order->filler          , ' '          				, sizeof( order->filler          ));
	memcpy( order->filler          , recv->Filler          		, sizeof( order->filler          ));
			order->Eof[ 0]         = 0x00;

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
**  convert ORDER to ORDER_SEND
***************************************************************************** */
int Proc_SendConvert( ORDER_SEND *send, ORDER *order)
{
	memcpy( send->MsgType         , order->MsgType         , sizeof(send->MsgType         ));    
	memcpy( send->OrdStatus       , order->OrdStatus       , sizeof(send->OrdStatus       ));    
	memcpy( send->ExecType        , order->ExecType        , sizeof(send->ExecType        ));    
	/*
	memcpy( send->                , order->OrgnGb          , sizeof(send->                ));    
	*/
	memcpy( send->LgenNo          , order->LgenNo          , sizeof(send->LgenNo          ));    
	memcpy( send->BkNo            , order->BkNo            , sizeof(send->BkNo            ));    
	memcpy( send->FxBkId          , order->FxBkId          , sizeof(send->FxBkId          ));    
	memcpy( send->ClOrdID         , order->ClOrdID         , sizeof(send->ClOrdID         ));    
	memcpy( send->OrigClOrdID     , order->OrigClOrdID     , sizeof(send->OrigClOrdID     ));    
	memcpy( send->Currency        , order->Currency        , sizeof(send->Currency        ));    
	DtoA0N( send->OrderQty       , order->OrderQty        , sizeof(send->OrderQty        ));    
	/*
	memcpy( send->                , order->SlipCmpPrice    , sizeof(send->                ));    
	memcpy( send->                , order->SlipPip         , sizeof(send->                ));    
	DtoA  ( send->                , order->OrderQty        , sizeof(send->                ));    
	*/
	DtoA0N( send->CumQty         , order->CumQty          , sizeof(send->CumQty          ));    
	DtoA0N( send->LastQty        , order->LastQty         , sizeof(send->LastQty         ));    
	DtoA0N( send->LeavesQty      , order->LeavesQty       , sizeof(send->LeavesQty       ));    
	memcpy( send->OrdID           , order->OrdID           , sizeof(send->OrdID           ));
	memcpy( send->ExecID          , order->ExecID          , sizeof(send->ExecID          ));
	DtoA0N( send->Price          , order->Price           , sizeof(send->Price           ));    
	DtoA0N( send->PriceSpr       , order->PriceSpr        , sizeof(send->PriceSpr        ));    
	DtoA0N( send->LastPx         , order->LastPx          , sizeof(send->LastPx          ));    
	DtoA0N( send->LastSprPx      , order->LastSprPx       , sizeof(send->LastSprPx       ));    
	memcpy( send->Side            , order->Side            , sizeof(send->Side            ));    
	memcpy( send->TimeInForce     , order->TimeInForce     , sizeof(send->TimeInForce     ));    
	memcpy( send->RefuslCd        , order->RefuslCd        , sizeof(send->RefuslCd        ));    
	memcpy( send->Text            , order->Text            , sizeof(send->Text            ));    
	memcpy( send->TransactTime    , order->TransactTime    , sizeof(send->TransactTime    ));    
	memcpy( send->SettType        , order->SettType        , sizeof(send->SettType        ));    
	memcpy( send->TnrId           , order->TnrId           , sizeof(send->TnrId       ));
	memcpy( send->TnrPtrnDcd      , order->TnrPtrnDcd      , sizeof(send->TnrPtrnDcd  ));
	memcpy( send->Symbol          , order->Symbol          , sizeof(send->Symbol          ));    
	memcpy( send->ValueDate1      , order->ValueDate1      , sizeof(send->ValueDate1      ));    
	memcpy( send->ValueDate2      , order->ValueDate2      , sizeof(send->ValueDate2      ));    
	memcpy( send->OfprKeyVal      , order->OfprKeyVal      , sizeof(send->OfprKeyVal      ));    

	memcpy( send->NearSettType    , order->NearSettType    , sizeof(send->NearSettType    ));    
	memcpy( send->NearTnrId       , order->NearTnrId       , sizeof(send->NearTnrId       ));
	memcpy( send->NearTnrPtrnDcd  , order->NearTnrPtrnDcd  , sizeof(send->NearTnrPtrnDcd  ));

	memcpy( send->NearLegSide     , order->NearLegSide     , sizeof(send->NearLegSide     ));    
	memcpy( send->NearLegSettlDate, order->NearLegSettlDate, sizeof(send->NearLegSettlDate));    

	DtoA0N( send->NearLegMktPrc  , order->NearLegMktPrc   , sizeof(send->NearLegMktPrc   ));    
	DtoA0N( send->NearLegCvPrc   , order->NearLegCvPrc    , sizeof(send->NearLegCvPrc    ));    
	DtoA0N( send->NearLegCoPrc   , order->NearLegCoPrc    , sizeof(send->NearLegCoPrc    ));    
	DtoA0N( send->NearLegCusPrc  , order->NearLegCusPrc   , sizeof(send->NearLegCusPrc   ));    
	DtoA0N( send->NearLegCusSpr  , order->NearLegCusSpr   , sizeof(send->NearLegCusSpr   ));    

	memcpy( send->FarSettType     , order->FarSettType     , sizeof(send->FarSettType     ));    
	memcpy( send->FarTnrId        , order->FarTnrId        , sizeof(send->FarTnrId        ));
	memcpy( send->FarTnrPtrnDcd   , order->FarTnrPtrnDcd   , sizeof(send->FarTnrPtrnDcd   ));

	memcpy( send->FarLegSide      , order->FarLegSide      , sizeof(send->FarLegSide      ));    
	memcpy( send->FarLegSettlDate , order->FarLegSettlDate , sizeof(send->FarLegSettlDate ));    
	DtoA0N( send->FarLegMktPrc   , order->FarLegMktPrc    , sizeof(send->FarLegMktPrc    ));    
	DtoA0N( send->FarLegCvPrc    , order->FarLegCvPrc     , sizeof(send->FarLegCvPrc     ));    
	DtoA0N( send->FarLegCoPrc    , order->FarLegCoPrc     , sizeof(send->FarLegCoPrc     ));    
	DtoA0N( send->FarLegCusPrc   , order->FarLegCusPrc    , sizeof(send->FarLegCusPrc    ));    
	DtoA0N( send->FarLegCusSpr   , order->FarLegCusSpr    , sizeof(send->FarLegCusSpr    ));    

#if 0
	memcpy( send->GrpOrdnNo       , order->GrpOrdnNo       , sizeof(send->GrpOrdnNo       ));
	memcpy( send->GrpOrdnCnt      , order->GrpOrdnCnt      , sizeof(send->GrpOrdnCnt      ));
	memcpy( send->GrpOrdnSeq      , order->GrpOrdnSeq      , sizeof(send->GrpOrdnSeq      ));
#endif
	memcpy( send->filler          , order->filler          , sizeof(send->filler          ));    
	memcpy( send->Eof             , order->Eof             , sizeof(send->Eof             ));    

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
**  convert ORDER to ORDER_SEND
***************************************************************************** */
int Proc_SiseConvert( MATSISE *dest, MATSISE *orig)
{
	memcpy( dest->type,			orig->type,			sizeof( dest->type));
	memcpy( dest->excode,		orig->excode,		sizeof( dest->excode));
	memcpy( dest->bidex,		orig->bidex,		sizeof( dest->bidex));
	memcpy( dest->askex,		orig->askex,		sizeof( dest->askex));
	memcpy( dest->symb,			orig->symb,			sizeof( dest->symb));
	memcpy( dest->id,			orig->id,			sizeof( dest->id));
	memcpy( dest->date,			orig->date,			sizeof( dest->date));
	memcpy( dest->time,			orig->time,			sizeof( dest->time));
	dest->usdbid		= orig->usdbid;
	dest->usdask		= orig->usdask;
	if( orig->bidprc != 0.0)	dest->bidprc		= orig->bidprc;
	if( orig->askprc != 0.0)	dest->askprc		= orig->askprc;
	dest->bidqty		= orig->bidqty;
	dest->askqty		= orig->askqty;
	dest->midprc		= orig->midprc;
	dest->fillprc		= orig->fillprc;
	dest->ctime			= orig->ctime;
	dest->price_time	= orig->price_time;

	return 1;
}

