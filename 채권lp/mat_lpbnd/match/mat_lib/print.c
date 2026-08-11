#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "order.h"
#include "sise.h"
#include "comm.h"
#include "mat.h"
#include "etc.h"

#include "/fxwin/sw/mat/lib/include/order.h"

int ORDER_HEAD_Print( ORDER_HEAD* ptr)
{
    LogRaw( "%s", "----[ ORDER_HEAD ]----------------------------------------------------------------------\n");
    LogRaw( "구분코드                      Code                   4    0 = [%.4s]\n", 	ptr->Code);
    LogRaw( "서브코드                      SubCode                4    4 = [%.4s]\n", 	ptr->SubCode);
    LogRaw( "연속데이타 번호 1 ~ n - 단일  DataCount              2    8 = [%.2s]\n", 	ptr->DataCount);
    LogRaw( "전체 데이타 갯수 n - 단일데�  DataTotal              2   10 = [%.2s]\n", 	ptr->DataTotal);
    LogRaw( "데이타길이 - 헤더제외         DataSize               4   12 = [%.4s]\n", 	ptr->DataSize);
    LogRaw( "filler - total size = 32      Filler                16   16 = [%.16s]\n", 	ptr->Filler);
    LogRaw( "%s", "----------------------------------------------------------------------[ ORDER_HEAD ]----\n");

    return sizeof( ORDER_HEAD);
}

int ORDER_RECV_Print( ORDER_RECV* ptr)
{
    LogRaw( "%s", "----[ ORDER_RECV ]----------------------------------------------------------------------\n");
    LogRaw( "주문타입                      MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "고객번호 (내부사용자ID)       CustID                30    1 = [%.30s]\n", 	ptr->CustID);
    LogRaw( "펀드번호                      FundNO                12    1 = [%.12s]\n", 	ptr->FundNO);
    LogRaw( "원천구분                      OrgnGb                 1   31 = [%.1s]\n", 	ptr->OrgnGb);
    LogRaw( "PU 코드                       PuCd                   5   32 = [%.5s]\n", 	ptr->PuCd);
    LogRaw( "북번호                        BkNo                  20   37 = [%.20s]\n", 	ptr->BkNo);
    LogRaw( "주문번호                      ClOrdID               11   57 = [%.11s]\n", 	ptr->ClOrdID);
    LogRaw( "원주문번호                    OrigClOrdID           11   68 = [%.11s]\n", 	ptr->OrigClOrdID);
    LogRaw( "기준통화코드(정보성)          Currency               3   79 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문수량                      OrderQty              23   82 = [%.23s]\n", 	ptr->OrderQty);
    LogRaw( "주문유형 1:시장,2:지정,3:예? OrdType                1  105 = [%.1s]\n", 	ptr->OrdType);
    LogRaw( "주문가격                      Price                 20  106 = [%.20s]\n", 	ptr->Price);
    LogRaw( "고객스프레드                  PriceSpr              20  126 = [%.20s]\n", 	ptr->PriceSpr);
    LogRaw( "주문시점가격(시장가의 경우)   SlipCmpPrice          20  146 = [%.20s]\n", 	ptr->SlipCmpPrice);
    LogRaw( "슬립피지허용가격(시장가만 의  SlipPip               20  166 = [%.20s]\n", 	ptr->SlipPip);
    LogRaw( "매매구분                      Side                   1  186 = [%.1s]\n", 	ptr->Side);
    LogRaw( "주문유효시간                  TimeInForce            1  187 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "처리시각                      TransactTime          20  188 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               3  208 = [%.3s]\n", 	ptr->SettType);
    LogRaw( "테너 ID                       TnrId                  3  211 = [%.3s]\n", 	ptr->TnrId);
    LogRaw( "테너유형구분코드              TnrPtrnDcd             1  214 = [%.1s]\n", 	ptr->TnrPtrnDcd);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  215 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  222 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  230 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "근일물상품구분코드            NearSettType           3  238 = [%.3s]\n", 	ptr->NearSettType);
    LogRaw( "테너ID                        NearTnrId              3  241 = [%.3s]\n", 	ptr->NearTnrId);
    LogRaw( "테너유형구분코드              NearTnrPtrnDcd         1  244 = [%.1s]\n", 	ptr->NearTnrPtrnDcd);
    LogRaw( "NEAR매매구분(1-Buy,2-Sell)    NearLegSide            1  245 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-결제일자                 NearLegSettlDate       8  246 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       NearLegPrice          20  254 = [%.20s]\n", 	ptr->NearLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        NearLegPriceSprd      20  274 = [%.20s]\n", 	ptr->NearLegPriceSprd);
    LogRaw( "원일물상품구분코드            FarSettType            3  294 = [%.3s]\n", 	ptr->FarSettType);
    LogRaw( "테너ID                        FarTnrId               3  297 = [%.3s]\n", 	ptr->FarTnrId);
    LogRaw( "테너유형구분코드              FarTnrPtrnDcd          1  300 = [%.1s]\n", 	ptr->FarTnrPtrnDcd);
    LogRaw( "FAR매매구분(1-Buy, 2-Sell)    FarLegSide             1  301 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-결제일자                  FarLegSettlDate        8  302 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       FarLegPrice           20  310 = [%.20s]\n", 	ptr->FarLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        FarLegPriceSprd       20  330 = [%.20s]\n", 	ptr->FarLegPriceSprd);
    LogRaw( "거래유형코드                  TranPtrnCd             1  350 = [%.1s]\n", 	ptr->TranPtrnCd);
    LogRaw( "그룹주문번호                  GrpOrdnNo             11  351 = [%.11s]\n", 	ptr->GrpOrdnNo);
    LogRaw( "그룹주문건수                  GrpOrdnCnt            10  362 = [%.10s]\n", 	ptr->GrpOrdnCnt);
    LogRaw( "그룹주문순번                  GrpOrdnSeq            10  372 = [%.10s]\n", 	ptr->GrpOrdnSeq);
    LogRaw( "그룹주문 상품코드 SPT,FWD     GrpFxPdcd              3  382 = [%.3s]\n", 	ptr->GrpFxPdcd);
    LogRaw( "그룹주문 타입                 TrdTypeDcd             1  385 = [%.1s]\n", 	ptr->TrdTypeDcd);
    LogRaw( "TrdTypeDcd=2일경우 스팟가격   SpotPrc               20  386 = [%.20s]\n", 	ptr->SpotPrc);
    LogRaw( "TrdTypeDcd=2일경우 스팟가격   SpotPrcSpr            20  406 = [%.20s]\n", 	ptr->SpotPrcSpr);
    LogRaw( "주문수량을 달러로 환산한 금액 UsdQty                23  406 = [%.23s]\n", 	ptr->UsdQty);
    LogRaw( "Filler                        Filler               597  426 = [%.597s]\n", ptr->Filler);
    LogRaw( "EOF 0x00                      Eof                    1 1023 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "----------------------------------------------------------------------[ ORDER_RECV ]----\n");

    return sizeof( ORDER_RECV);
}

int ORDER_SEND_Print( ORDER_SEND* ptr)
{
    LogRaw( "%s", "----[ ORDER_SEND ]----------------------------------------------------------------------\n");
    LogRaw( "메세지유형      3-거부        MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "주문상태                      OrdStatus              1    1 = [%.1s]\n", 	ptr->OrdStatus);
    LogRaw( "거래유형                      ExecType               1    2 = [%.1s]\n", 	ptr->ExecType);
    LogRaw( "시장참여자ID및트레이더번호    LgenNo                16    3 = [%.16s]\n", 	ptr->LgenNo);
    LogRaw( "북번호                        BkNo                   5   19 = [%.5s]\n", 	ptr->BkNo);
    LogRaw( "북번호                        FxBkId                20   24 = [%.20s]\n", 	ptr->FxBkId);
    LogRaw( "회원처리항목1                 ClOrdID               11   44 = [%.11s]\n", 	ptr->ClOrdID);
    LogRaw( "회원처리항목2                 OrigClOrdID           11   55 = [%.11s]\n", 	ptr->OrigClOrdID);
    LogRaw( "통화코드                      Currency               3   66 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문수량                      OrderQty              23   69 = [%.23s]\n", 	ptr->OrderQty);
    LogRaw( "누적체결수량                  CumQty                23   92 = [%.23s]\n", 	ptr->CumQty);
    LogRaw( "체결수량                      LastQty               23  115 = [%.23s]\n", 	ptr->LastQty);
    LogRaw( "주문잔여수량                  LeavesQty             23  138 = [%.23s]\n", 	ptr->LeavesQty);
    LogRaw( "주문번호ID                    OrdID                 30  161 = [%.30s]\n", 	ptr->OrdID);
    LogRaw( "체결ID                        ExecID                30  191 = [%.30s]\n", 	ptr->ExecID);
    LogRaw( "주문가격                      Price                 20  221 = [%.20s]\n", 	ptr->Price);
    LogRaw( "주문가격고객스프레드          PriceSpr              20  241 = [%.20s]\n", 	ptr->PriceSpr);
    LogRaw( "체결가격                      LastPx                20  261 = [%.20s]\n", 	ptr->LastPx);
    LogRaw( "체결가격고객스프레드          LastSprPx             20  281 = [%.20s]\n", 	ptr->LastSprPx);
    LogRaw( "매매구분                      Side                   1  301 = [%.1s]\n", 	ptr->Side);
    LogRaw( "체결조건                      TimeInForce            1  302 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "거부코드                      RefuslCd              10  303 = [%.10s]\n", 	ptr->RefuslCd);
    LogRaw( "내용                          Text                 200  313 = [%.200s]\n", 	ptr->Text);
    LogRaw( "주문일시                      TransactTime          20  513 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               3  533 = [%.3s]\n", 	ptr->SettType);
    LogRaw( "테너 ID                       TnrId                  3  536 = [%.3s]\n", 	ptr->TnrId);
    LogRaw( "테너유형구분                  TnrPtrnDcd             1  539 = [%.1s]\n", 	ptr->TnrPtrnDcd);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  540 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  547 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  555 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "거래호가번호                  OfprKeyVal            60  563 = [%.60s]\n", 	ptr->OfprKeyVal);
    LogRaw( "NEAR-레크상품구분코드         NearSettType           3  623 = [%.3s]\n", 	ptr->NearSettType);
    LogRaw( "테너 ID                       NearTnrId              3  626 = [%.3s]\n", 	ptr->NearTnrId);
    LogRaw( "테너 유형구분                 NearTnrPtrnDcd         1  629 = [%.1s]\n", 	ptr->NearTnrPtrnDcd);
    LogRaw( "NEAR-레그매수매도구분코드     NearLegSide            1  630 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-레그결제년월일           NearLegSettlDate       8  631 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "NEAR-레그체결가격 SPOT+SWAP   NearLegMktPrc         20  639 = [%.20s]\n", 	ptr->NearLegMktPrc);
    LogRaw( "NEAR-CV 가격                  NearLegCvPrc          20  659 = [%.20s]\n", 	ptr->NearLegCvPrc);
    LogRaw( "NEAR-CO 가격                  NearLegCoPrc          20  679 = [%.20s]\n", 	ptr->NearLegCoPrc);
    LogRaw( "NEAR-CU 가격                  NearLegCusPrc         20  699 = [%.20s]\n", 	ptr->NearLegCusPrc);
    LogRaw( "NEAR-CU스프레드               NearLegCusSpr         20  719 = [%.20s]\n", 	ptr->NearLegCusSpr);
    LogRaw( "FAR-레크상품구분코드          FarSettType            3  739 = [%.3s]\n", 	ptr->FarSettType);
    LogRaw( "테너 ID                       FarTnrId               3  742 = [%.3s]\n", 	ptr->FarTnrId);
    LogRaw( "테너 유형구분                 FarTnrPtrnDcd          1  745 = [%.1s]\n", 	ptr->FarTnrPtrnDcd);
    LogRaw( "FAR-레그매수매도구분코드      FarLegSide             1  746 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-레그결제년월일            FarLegSettlDate        8  747 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FAR-레그체결가격 SPOT+SWAP    FarLegMktPrc          20  755 = [%.20s]\n", 	ptr->FarLegMktPrc);
    LogRaw( "FAR-CV 가격                   FarLegCvPrc           20  775 = [%.20s]\n", 	ptr->FarLegCvPrc);
    LogRaw( "FAR-CO 가격                   FarLegCoPrc           20  795 = [%.20s]\n", 	ptr->FarLegCoPrc);
    LogRaw( "FAR-CU 가격                   FarLegCusPrc          20  815 = [%.20s]\n", 	ptr->FarLegCusPrc);
    LogRaw( "FAR-CU스프레드                FarLegCusSpr          20  835 = [%.20s]\n", 	ptr->FarLegCusSpr);
    LogRaw( "Filler                        filler               168  855 = [%.168s]\n", 	ptr->filler);
    LogRaw( "0x00                          Eof                    1 1023 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "----------------------------------------------------------------------[ ORDER_SEND ]----\n");

    return sizeof( ORDER_SEND);
}

int ORDER_Print( ORDER* ptr)
{
    LogRaw( "%s", "----[ ORDER ]---------------------------------------------------------------------------\n");
    LogRaw( "메세지유형                    MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    LogRaw( "주문상태                      OrdStatus              1    1 = [%.1s]\n", 	ptr->OrdStatus);
    LogRaw( "고객번호 (내부사용자ID)       CustID                30    2 = [%.30s]\n", 	ptr->CustID);
    LogRaw( "펀드번호                      FundNO                12    1 = [%.12s]\n", 	ptr->FundNO);
    LogRaw( "거래유형 @                    ExecType               1   32 = [%.1s]\n", 	ptr->ExecType);
    LogRaw( "원천구분                      OrgnGb                 1   33 = [%.1s]\n", 	ptr->OrgnGb);
    LogRaw( "시장참여자ID및트레이더번호    LgenNo                30   34 = [%.30s]\n", 	ptr->LgenNo);
    LogRaw( "북번호                        BkNo                  20   64 = [%.20s]\n", 	ptr->BkNo);
    LogRaw( "북번호                        FxBkId                20   84 = [%.20s]\n", 	ptr->FxBkId);
    LogRaw( "회원처리항목1                 ClOrdID               11  104 = [%.11s]\n", 	ptr->ClOrdID);
    LogRaw( "회원처리항목2                 OrigClOrdID           11  115 = [%.11s]\n", 	ptr->OrigClOrdID);
    LogRaw( "통화코드                      Currency               3  126 = [%.3s]\n", 	ptr->Currency);
    LogRaw( "주문유형                      OrdType                1  129 = [%.1s]\n", 	ptr->OrdType);
    LogRaw( "주문시점가격(시장가)          SlipCmpPrice           8  130 = [%f]\n", 	ptr->SlipCmpPrice);
    LogRaw( "Slipage Pip                   SlipPip                8  138 = [%f]\n", 	ptr->SlipPip);
    LogRaw( "주문수량                      OrderQty               8  146 = [%f]\n", 	ptr->OrderQty);
    LogRaw( "누적체결수량                  CumQty                 8  154 = [%f]\n", 	ptr->CumQty);
    LogRaw( "체결수량                      LastQty                8  162 = [%f]\n", 	ptr->LastQty);
    LogRaw( "주문잔여수량                  LeavesQty              8  170 = [%f]\n", 	ptr->LeavesQty);
    LogRaw( "주문번호ID                    OrdID                 30  178 = [%.30s]\n", 	ptr->OrdID);
    LogRaw( "체결ID                        ExecID                30  208 = [%.30s]\n", 	ptr->ExecID);
    LogRaw( "주문가격                      Price                  8  238 = [%f]\n", 	ptr->Price);
    LogRaw( "고객스프레드                  PriceSpr               8  246 = [%f]\n", 	ptr->PriceSpr);
    LogRaw( "주문고객마진                  OrdSprPrc              8  254 = [%f]\n", 	ptr->OrdSprPrc);
    LogRaw( "체결가격                      LastPx                 8  262 = [%f]\n", 	ptr->LastPx);
    LogRaw( "체결고객마진                  LastSprPx              8  270 = [%f]\n", 	ptr->LastSprPx);
    LogRaw( "매매구분                      Side                   1  278 = [%.1s]\n", 	ptr->Side);
    LogRaw( "체결조건                      TimeInForce            1  279 = [%.1s]\n", 	ptr->TimeInForce);
    LogRaw( "거부코드                      RefuslCd              10  280 = [%.10s]\n", 	ptr->RefuslCd);
    LogRaw( "내용                          Text                 200  290 = [%.200s]\n", 	ptr->Text);
    LogRaw( "주문일시                      TransactTime          20  490 = [%.20s]\n", 	ptr->TransactTime);
    LogRaw( "FX상품구분코드                SettType               3  510 = [%.3s]\n", 	ptr->SettType);
    LogRaw( "테너 ID                       TnrId                  3  513 = [%.3s]\n", 	ptr->TnrId);
    LogRaw( "테너유형구분코드              TnrPtrnDcd             1  516 = [%.1s]\n", 	ptr->TnrPtrnDcd);
    LogRaw( "FX상품코드     : USD/KRW      Symbol                 7  517 = [%.7s]\n", 	ptr->Symbol);
    LogRaw( "결제시작일자                  ValueDate1             8  524 = [%.8s]\n", 	ptr->ValueDate1);
    LogRaw( "결제종료일자                  ValueDate2             8  532 = [%.8s]\n", 	ptr->ValueDate2);
    LogRaw( "거래호가번호                  OfprKeyVal            30  540 = [%.30s]\n", 	ptr->OfprKeyVal);
    LogRaw( "NEAR-레크상품구분코드         NearSettType           3  570 = [%.3s]\n", 	ptr->NearSettType);
    LogRaw( "테너 ID                       NearTnrId              3  573 = [%.3s]\n", 	ptr->NearTnrId);
    LogRaw( "테너유형구분코드              NearTnrPtrnDcd         1  576 = [%.1s]\n", 	ptr->NearTnrPtrnDcd);
    LogRaw( "NEAR-레그매수매도구분코드     NearLegSide            1  577 = [%.1s]\n", 	ptr->NearLegSide);
    LogRaw( "NEAR-레그결제년월일           NearLegSettlDate       8  578 = [%.8s]\n", 	ptr->NearLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       NearLegPrice           8  586 = [%f]\n", 	ptr->NearLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        NearLegPriceSprd       8  586 = [%f]\n", 	ptr->NearLegPriceSprd);
    LogRaw( "NEAR-레그체결가격             NearLegMktPrc          8  586 = [%f]\n", 	ptr->NearLegMktPrc);
    LogRaw( "NEAR-CV스프레드               NearLegCvPrc           8  594 = [%f]\n", 	ptr->NearLegCvPrc);
    LogRaw( "NEAR-CO스프레드               NearLegCoPrc           8  602 = [%f]\n", 	ptr->NearLegCoPrc);
    LogRaw( "NEAR-레그체결가격스프레드     NearLegCusPrc          8  610 = [%f]\n", 	ptr->NearLegCusPrc);
    LogRaw( "NEAR-Cu prc (spread)          NearLegCusSpr          8  618 = [%f]\n", 	ptr->NearLegCusSpr);
    LogRaw( "FAR-레크상품구분코드          FarSettType            3  626 = [%.3s]\n", 	ptr->FarSettType);
    LogRaw( "테너 ID                       FarTnrId               3  629 = [%.3s]\n", 	ptr->FarTnrId);
    LogRaw( "테너유형구분코드              FarTnrPtrnDcd          1  632 = [%.1s]\n", 	ptr->FarTnrPtrnDcd);
    LogRaw( "FAR-레그매수매도구분코드      FarLegSide             1  633 = [%.1s]\n", 	ptr->FarLegSide);
    LogRaw( "FAR-레그결제년월일            FarLegSettlDate        8  634 = [%.8s]\n", 	ptr->FarLegSettlDate);
    LogRaw( "FWD는 FWD환율(고객가격)       FarLegPrice            8  586 = [%f]\n", 	ptr->FarLegPrice);
    LogRaw( "FWD는 FWD환율 스프레드        FarLegPriceSprd        8  586 = [%f]\n", 	ptr->FarLegPriceSprd);
    LogRaw( "FAR-레그체결가격              FarLegMktPrc           8  642 = [%f]\n", 	ptr->FarLegMktPrc);
    LogRaw( "FAR-CV스프레드                FarLegCvPrc            8  650 = [%f]\n", 	ptr->FarLegCvPrc);
    LogRaw( "FAR-CO스프레드                FarLegCoPrc            8  658 = [%f]\n", 	ptr->FarLegCoPrc);
    LogRaw( "FAR-레그체결가격스프레드      FarLegCusPrc           8  666 = [%f]\n", 	ptr->FarLegCusPrc);
    LogRaw( "FAR-Cu prc (spread)           FarLegCusSpr           8  674 = [%f]\n", 	ptr->FarLegCusSpr);
    LogRaw( "거래유형코드                  TranPtrnCd             1  682 = [%.1s]\n", 	ptr->TranPtrnCd);
    LogRaw( "그룹주문번호                  GrpOrdnNo             11  683 = [%.11s]\n", 	ptr->GrpOrdnNo);
    LogRaw( "그룹주문건수                  GrpOrdnCnt            10  694 = [%.10s]\n", 	ptr->GrpOrdnCnt);
    LogRaw( "그룹주문순번                  GrpOrdnSeq            10  704 = [%.10s]\n", 	ptr->GrpOrdnSeq);
    LogRaw( "그룹주문 상품코드 SPT,FWD     GrpFxPdcd              3  714 = [%.3s]\n", 	ptr->GrpFxPdcd);
    LogRaw( "주문 타입                     TrdTypeDcd             1  717 = [%.1s]\n", 	ptr->TrdTypeDcd);
    LogRaw( "TrdTypeDcd=2일경우 스팟가격   SpotPrc                8  718 = [%f]\n", 	ptr->SpotPrc);
    LogRaw( "TrdTypeDcd=2일경우 스팟가격   SpotPrcSpr             8  726 = [%f]\n", 	ptr->SpotPrcSpr);
    LogRaw( "주문수량을 달러로 환산한 금액 UsdQty                 8  726 = [%f]\n", 	ptr->UsdQty);
    LogRaw( "Filler                        filler                 1  746 = [%.1s]\n", 	ptr->filler);
    LogRaw( "0x00                          Eof                    1  747 = [%.1s]\n", 	ptr->Eof);
    LogRaw( "%s", "---------------------------------------------------------------------------[ ORDER ]----\n");

    return sizeof( ORDER);
}

int ORDER_PrintFile( ORDER* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ ORDER ]---------------------------------------------------------------------------\n");
    fprintf( fp, "메세지유형                    MsgType                1    0 = [%.1s]\n", 	ptr->MsgType);
    fprintf( fp, "주문상태                      OrdStatus              1    1 = [%.1s]\n", 	ptr->OrdStatus);
    fprintf( fp, "고객번호 (내부사용자ID)       CustID                30    2 = [%.30s]\n", 	ptr->CustID);
    fprintf( fp, "펀드번호                      FundNO                12    1 = [%.12s]\n", 	ptr->FundNO);
    fprintf( fp, "거래유형 @                    ExecType               1   32 = [%.1s]\n", 	ptr->ExecType);
    fprintf( fp, "원천구분                      OrgnGb                 1   33 = [%.1s]\n", 	ptr->OrgnGb);
    fprintf( fp, "시장참여자ID및트레이더번호    LgenNo                30   34 = [%.30s]\n", 	ptr->LgenNo);
    fprintf( fp, "북번호                        BkNo                  20   64 = [%.20s]\n", 	ptr->BkNo);
    fprintf( fp, "북번호                        FxBkId                20   84 = [%.20s]\n", 	ptr->FxBkId);
    fprintf( fp, "회원처리항목1                 ClOrdID               11  104 = [%.11s]\n", 	ptr->ClOrdID);
    fprintf( fp, "회원처리항목2                 OrigClOrdID           11  115 = [%.11s]\n", 	ptr->OrigClOrdID);
    fprintf( fp, "통화코드                      Currency               3  126 = [%.3s]\n", 	ptr->Currency);
    fprintf( fp, "주문유형                      OrdType                1  129 = [%.1s]\n", 	ptr->OrdType);
    fprintf( fp, "주문시점가격(시장가)          SlipCmpPrice           8  130 = [%f]\n", 	ptr->SlipCmpPrice);
    fprintf( fp, "Slipage Pip                   SlipPip                8  138 = [%f]\n", 	ptr->SlipPip);
    fprintf( fp, "주문수량                      OrderQty               8  146 = [%f]\n", 	ptr->OrderQty);
    fprintf( fp, "누적체결수량                  CumQty                 8  154 = [%f]\n", 	ptr->CumQty);
    fprintf( fp, "체결수량                      LastQty                8  162 = [%f]\n", 	ptr->LastQty);
    fprintf( fp, "주문잔여수량                  LeavesQty              8  170 = [%f]\n", 	ptr->LeavesQty);
    fprintf( fp, "주문번호ID                    OrdID                 30  178 = [%.30s]\n", 	ptr->OrdID);
    fprintf( fp, "체결ID                        ExecID                30  208 = [%.30s]\n", 	ptr->ExecID);
    fprintf( fp, "주문가격                      Price                  8  238 = [%f]\n", 	ptr->Price);
    fprintf( fp, "고객스프레드                  PriceSpr               8  246 = [%f]\n", 	ptr->PriceSpr);
    fprintf( fp, "주문고객마진                  OrdSprPrc              8  254 = [%f]\n", 	ptr->OrdSprPrc);
    fprintf( fp, "체결가격                      LastPx                 8  262 = [%f]\n", 	ptr->LastPx);
    fprintf( fp, "체결고객마진                  LastSprPx              8  270 = [%f]\n", 	ptr->LastSprPx);
    fprintf( fp, "매매구분                      Side                   1  278 = [%.1s]\n", 	ptr->Side);
    fprintf( fp, "체결조건                      TimeInForce            1  279 = [%.1s]\n", 	ptr->TimeInForce);
    fprintf( fp, "거부코드                      RefuslCd              10  280 = [%.10s]\n", 	ptr->RefuslCd);
    fprintf( fp, "내용                          Text                 200  290 = [%.200s]\n", 	ptr->Text);
    fprintf( fp, "주문일시                      TransactTime          20  490 = [%.20s]\n", 	ptr->TransactTime);
    fprintf( fp, "FX상품구분코드                SettType               3  510 = [%.3s]\n", 	ptr->SettType);
    fprintf( fp, "테너 ID                       TnrId                  3  513 = [%.3s]\n", 	ptr->TnrId);
    fprintf( fp, "테너유형구분코드              TnrPtrnDcd             1  516 = [%.1s]\n", 	ptr->TnrPtrnDcd);
    fprintf( fp, "FX상품코드     : USD/KRW      Symbol                 7  517 = [%.7s]\n", 	ptr->Symbol);
    fprintf( fp, "결제시작일자                  ValueDate1             8  524 = [%.8s]\n", 	ptr->ValueDate1);
    fprintf( fp, "결제종료일자                  ValueDate2             8  532 = [%.8s]\n", 	ptr->ValueDate2);
    fprintf( fp, "거래호가번호                  OfprKeyVal            30  540 = [%.30s]\n", 	ptr->OfprKeyVal);
    fprintf( fp, "NEAR-레크상품구분코드         NearSettType           3  570 = [%.3s]\n", 	ptr->NearSettType);
    fprintf( fp, "테너 ID                       NearTnrId              3  573 = [%.3s]\n", 	ptr->NearTnrId);
    fprintf( fp, "테너유형구분코드              NearTnrPtrnDcd         1  576 = [%.1s]\n", 	ptr->NearTnrPtrnDcd);
    fprintf( fp, "NEAR-레그매수매도구분코드     NearLegSide            1  577 = [%.1s]\n", 	ptr->NearLegSide);
    fprintf( fp, "NEAR-레그결제년월일           NearLegSettlDate       8  578 = [%.8s]\n", 	ptr->NearLegSettlDate);
    fprintf( fp, "FWD는 FWD환율(고객가격)       NearLegPrice           8  586 = [%f]\n", 	ptr->NearLegPrice);
    fprintf( fp, "FWD는 FWD환율 스프레드        NearLegPriceSprd       8  586 = [%f]\n", 	ptr->NearLegPriceSprd);
    fprintf( fp, "NEAR-레그체결가격             NearLegMktPrc          8  586 = [%f]\n", 	ptr->NearLegMktPrc);
    fprintf( fp, "NEAR-CV스프레드               NearLegCvPrc           8  594 = [%f]\n", 	ptr->NearLegCvPrc);
    fprintf( fp, "NEAR-CO스프레드               NearLegCoPrc           8  602 = [%f]\n", 	ptr->NearLegCoPrc);
    fprintf( fp, "NEAR-레그체결가격스프레드     NearLegCusPrc          8  610 = [%f]\n", 	ptr->NearLegCusPrc);
    fprintf( fp, "NEAR-Cu prc (spread)          NearLegCusSpr          8  618 = [%f]\n", 	ptr->NearLegCusSpr);
    fprintf( fp, "FAR-레크상품구분코드          FarSettType            3  626 = [%.3s]\n", 	ptr->FarSettType);
    fprintf( fp, "테너 ID                       FarTnrId               3  629 = [%.3s]\n", 	ptr->FarTnrId);
    fprintf( fp, "테너유형구분코드              FarTnrPtrnDcd          1  632 = [%.1s]\n", 	ptr->FarTnrPtrnDcd);
    fprintf( fp, "FAR-레그매수매도구분코드      FarLegSide             1  633 = [%.1s]\n", 	ptr->FarLegSide);
    fprintf( fp, "FAR-레그결제년월일            FarLegSettlDate        8  634 = [%.8s]\n", 	ptr->FarLegSettlDate);
    fprintf( fp, "FWD는 FWD환율(고객가격)       FarLegPrice            8  586 = [%f]\n", 	ptr->FarLegPrice);
    fprintf( fp, "FWD는 FWD환율 스프레드        FarLegPriceSprd        8  586 = [%f]\n", 	ptr->FarLegPriceSprd);
    fprintf( fp, "FAR-레그체결가격              FarLegMktPrc           8  642 = [%f]\n", 	ptr->FarLegMktPrc);
    fprintf( fp, "FAR-CV스프레드                FarLegCvPrc            8  650 = [%f]\n", 	ptr->FarLegCvPrc);
    fprintf( fp, "FAR-CO스프레드                FarLegCoPrc            8  658 = [%f]\n", 	ptr->FarLegCoPrc);
    fprintf( fp, "FAR-레그체결가격스프레드      FarLegCusPrc           8  666 = [%f]\n", 	ptr->FarLegCusPrc);
    fprintf( fp, "FAR-Cu prc (spread)           FarLegCusSpr           8  674 = [%f]\n", 	ptr->FarLegCusSpr);
    fprintf( fp, "거래유형코드                  TranPtrnCd             1  682 = [%.1s]\n", 	ptr->TranPtrnCd);
    fprintf( fp, "그룹주문번호                  GrpOrdnNo             11  683 = [%.11s]\n", 	ptr->GrpOrdnNo);
    fprintf( fp, "그룹주문건수                  GrpOrdnCnt            10  694 = [%.10s]\n", 	ptr->GrpOrdnCnt);
    fprintf( fp, "그룹주문순번                  GrpOrdnSeq            10  704 = [%.10s]\n", 	ptr->GrpOrdnSeq);
    fprintf( fp, "그룹주문 상품코드 SPT,FWD     GrpFxPdcd              3  714 = [%.3s]\n", 	ptr->GrpFxPdcd);
    fprintf( fp, "주문 타입                     TrdTypeDcd             1  717 = [%.1s]\n", 	ptr->TrdTypeDcd);
    fprintf( fp, "TrdTypeDcd=2일경우 스팟가격   SpotPrc                8  718 = [%f]\n", 	ptr->SpotPrc);
    fprintf( fp, "TrdTypeDcd=2일경우 스팟가격   SpotPrcSpr             8  726 = [%f]\n", 	ptr->SpotPrcSpr);
    fprintf( fp, "주문수량을 달러로 환산한 금액 UsdQty                 8  726 = [%f]\n", 	ptr->UsdQty);
    fprintf( fp, "Filler                        filler                 1  746 = [%.1s]\n", 	ptr->filler);
    fprintf( fp, "0x00                          Eof                    1  747 = [%.1s]\n", 	ptr->Eof);
    fprintf( fp, "%s", "---------------------------------------------------------------------------[ ORDER ]----\n");

    return sizeof( ORDER);
}

int MATSISE_Print( MATSISE* ptr)
{
    LogRaw( "%s", "----[ MATSISE ]-------------------------------------------------------------------------\n");
    LogRaw( "FA SWAP rate 구분 위해        type                   2    0 = [%.2s]\n", 	ptr->type);
    LogRaw( "SMB/KMB/EBS/CMB/BEST/ZCUST/M  excode                 1    2 = [%.1s]\n", 	ptr->excode);
    LogRaw( "BID원천 : S:SMB, K:KMB, E:EB  bidex                  1    3 = [%.1s]\n", 	ptr->bidex);
    LogRaw( "ASK원천 : S:SMB, K:KMB, E:EB  askex                  1    4 = [%.1s]\n", 	ptr->askex);
    LogRaw( "root symbol                   symb                   7    5 = [%.7s]\n", 	ptr->symb);
    LogRaw( "호가 id                       id                    32   12 = [%.32s]\n", 	ptr->id);
    LogRaw( "수신일자 YYYYMMDD (서버시간)  date                   8   44 = [%.8s]\n", 	ptr->date);
    LogRaw( "수신시간 HHMMSSSSS            time                   9   52 = [%.9s]\n", 	ptr->time);
    LogRaw( "Current USDKRW BID            usdbid                 8   61 = [%f]\n", 	ptr->usdbid);
    LogRaw( "Current USDKRW OFFER          usdask                 8   69 = [%f]\n", 	ptr->usdask);
    LogRaw( "Price of the MarketData Entr  bidprc                 8   77 = [%f]\n", 	ptr->bidprc);
    LogRaw( "Price of the MarketData Entr  askprc                 8   85 = [%f]\n", 	ptr->askprc);
    LogRaw( "Quantity of the MarketData E  bidqty                 8   93 = [%f]\n", 	ptr->bidqty);
    LogRaw( "Quantity of the MarketData E  askqty                 8  101 = [%f]\n", 	ptr->askqty);
    LogRaw( "중간가                        midprc                 8  109 = [%f]\n", 	ptr->midprc);
    LogRaw( "체결가                        fillprc                8  117 = [%f]\n", 	ptr->fillprc);
    LogRaw( "time_t convert                ctime                  8  125 = [%s]\n",  TtoS( ptr->ctime));
    LogRaw( "시세 유효 시간                price_time             8  133 = [%s]\n",  TtoS( ptr->price_time));
    LogRaw( "256 byte 맞춤                 filler               112  141 = [%.112s]\n", 	ptr->filler);
    LogRaw( "%s", "-------------------------------------------------------------------------[ MATSISE ]----\n");

    return sizeof( MATSISE);
}

int MATSISE_PrintFile( MATSISE* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ MATSISE ]-------------------------------------------------------------------------\n");
    fprintf( fp, "FA SWAP rate 구분 위해        type                   2    0 = [%.2s]\n", 	ptr->type);
    fprintf( fp, "SMB/KMB/EBS/CMB/BEST/ZCUST/M  excode                 1    2 = [%.1s]\n", 	ptr->excode);
    fprintf( fp, "BID원천 : S:SMB, K:KMB, E:EB  bidex                  1    3 = [%.1s]\n", 	ptr->bidex);
    fprintf( fp, "ASK원천 : S:SMB, K:KMB, E:EB  askex                  1    4 = [%.1s]\n", 	ptr->askex);
    fprintf( fp, "root symbol                   symb                   7    5 = [%.7s]\n", 	ptr->symb);
    fprintf( fp, "호가 id                       id                    32   12 = [%.32s]\n", 	ptr->id);
    fprintf( fp, "수신일자 YYYYMMDD (서버시간)  date                   8   44 = [%.8s]\n", 	ptr->date);
    fprintf( fp, "수신시간 HHMMSSSSS            time                   9   52 = [%.9s]\n", 	ptr->time);
    fprintf( fp, "Current USDKRW BID            usdbid                 8   61 = [%f]\n", 	ptr->usdbid);
    fprintf( fp, "Current USDKRW OFFER          usdask                 8   69 = [%f]\n", 	ptr->usdask);
    fprintf( fp, "Price of the MarketData Entr  bidprc                 8   77 = [%f]\n", 	ptr->bidprc);
    fprintf( fp, "Price of the MarketData Entr  askprc                 8   85 = [%f]\n", 	ptr->askprc);
    fprintf( fp, "Quantity of the MarketData E  bidqty                 8   93 = [%f]\n", 	ptr->bidqty);
    fprintf( fp, "Quantity of the MarketData E  askqty                 8  101 = [%f]\n", 	ptr->askqty);
    fprintf( fp, "중간가                        midprc                 8  109 = [%f]\n", 	ptr->midprc);
    fprintf( fp, "체결가                        fillprc                8  117 = [%f]\n", 	ptr->fillprc);
    fprintf( fp, "time_t convert                ctime                  8  125 = [%s]\n",  TtoS( ptr->ctime));
    fprintf( fp, "시세 유효 시간                price_time             8  133 = [%s]\n",  TtoS( ptr->price_time));
    fprintf( fp, "256 byte 맞춤                 filler               112  141 = [%.112s]\n", 	ptr->filler);
    fprintf( fp, "%s", "-------------------------------------------------------------------------[ MATSISE ]----\n");

    return sizeof( MATSISE);
}

int APSISE_Print( APSISE* ptr)
{
    LogRaw( "%s", "----[ APSISE ]--------------------------------------------------------------------------\n");
    LogRaw( "FA SWAP rate 구분 위해        type                   2    0 = [%.2s]\n", 	ptr->type);
    LogRaw( "SMB/KMB/EBS/CMB/BEST/ZCUST    excode                 1    2 = [%.1s]\n", 	ptr->excode);
    LogRaw( "BID원천 : S:SMB, K:KMB, E:EB  bidex                  1    3 = [%.1s]\n", 	ptr->bidex);
    LogRaw( "ASK원천 : S:SMB, K:KMB, E:EB  offerex                1    4 = [%.1s]\n", 	ptr->offerex);
    LogRaw( "root symbol                   symb                   7    5 = [%.7s]\n", 	ptr->symb);
    LogRaw( "호가 id                       id                    32   12 = [%.32s]\n", 	ptr->id);
    LogRaw( "수신일자 YYYYMMDD (서버시간)  date                   8   44 = [%.8s]\n", 	ptr->date);
    LogRaw( "수신시간 HHMMSSSSS            time                   9   52 = [%.9s]\n", 	ptr->time);
    LogRaw( "Current USDKRW BID            usdbid                 8   61 = [%f]\n", 	ptr->usdbid);
    LogRaw( "Current USDKRW OFFER          usdoffer               8   69 = [%f]\n", 	ptr->usdoffer);
    LogRaw( "Price of the MarketData Entr  bidprc                 8   77 = [%f]\n", 	ptr->bidprc);
    LogRaw( "Price of the MarketData Entr  offerprc               8   85 = [%f]\n", 	ptr->offerprc);
    LogRaw( "Quantity of the MarketData E  bidqty                 8   93 = [%f]\n", 	ptr->bidqty);
    LogRaw( "Quantity of the MarketData E  offerqty               8  101 = [%f]\n", 	ptr->offerqty);
    LogRaw( "중간가                        midprc                 8  109 = [%f]\n", 	ptr->midprc);
    LogRaw( "체결가                        fillprc                8  117 = [%f]\n", 	ptr->fillprc);
    LogRaw( "%s", "--------------------------------------------------------------------------[ APSISE ]----\n");

    return sizeof( APSISE);
}

int APSISE_PrintFile( APSISE* ptr, FILE *fp)
{
    fprintf( fp,"%s", "----[ APSISE ]--------------------------------------------------------------------------\n");
    fprintf( fp,"FA SWAP rate 구분 위해        type                   2    0 = [%.2s]\n", 	ptr->type);
    fprintf( fp,"SMB/KMB/EBS/CMB/BEST/ZCUST    excode                 1    2 = [%.1s]\n", 	ptr->excode);
    fprintf( fp,"BID원천 : S:SMB, K:KMB, E:EB  bidex                  1    3 = [%.1s]\n", 	ptr->bidex);
    fprintf( fp,"ASK원천 : S:SMB, K:KMB, E:EB  offerex                1    4 = [%.1s]\n", 	ptr->offerex);
    fprintf( fp,"root symbol                   symb                   7    5 = [%.7s]\n", 	ptr->symb);
    fprintf( fp,"호가 id                       id                    32   12 = [%.32s]\n", 	ptr->id);
    fprintf( fp,"수신일자 YYYYMMDD (서버시간)  date                   8   44 = [%.8s]\n", 	ptr->date);
    fprintf( fp,"수신시간 HHMMSSSSS            time                   9   52 = [%.9s]\n", 	ptr->time);
    fprintf( fp,"Current USDKRW BID            usdbid                 8   61 = [%f]\n", 	ptr->usdbid);
    fprintf( fp,"Current USDKRW OFFER          usdoffer               8   69 = [%f]\n", 	ptr->usdoffer);
    fprintf( fp,"Price of the MarketData Entr  bidprc                 8   77 = [%f]\n", 	ptr->bidprc);
    fprintf( fp,"Price of the MarketData Entr  offerprc               8   85 = [%f]\n", 	ptr->offerprc);
    fprintf( fp,"Quantity of the MarketData E  bidqty                 8   93 = [%f]\n", 	ptr->bidqty);
    fprintf( fp,"Quantity of the MarketData E  offerqty               8  101 = [%f]\n", 	ptr->offerqty);
    fprintf( fp,"중간가                        midprc                 8  109 = [%f]\n", 	ptr->midprc);
    fprintf( fp,"체결가                        fillprc                8  117 = [%f]\n", 	ptr->fillprc);
    fprintf( fp,"%s", "--------------------------------------------------------------------------[ APSISE ]----\n");

    return sizeof( APSISE);
}

int SISE_ENTRY_Print( SISE_ENTRY* ptr)
{
    LogRaw( "%s", "----[ SISE_ENTRY ]----------------------------------------------------------------------\n");
    LogRaw( "0=bid,1=offer                 MDEntry_Type           2    0 = [%.2s]\n", 	ptr->MDEntry_Type);
    LogRaw( "Price                         MDEntry_Px            16    2 = [%.16s]\n", 	ptr->MDEntry_Px);
    LogRaw( "Quantity                      MDEntry_Size          16   18 = [%.16s]\n", 	ptr->MDEntry_Size);
    LogRaw( "date                          MDEntry_Date           8   34 = [%.8s]\n", 	ptr->MDEntry_Date);
    LogRaw( "Unique identifier             Quote_EntryID         32   42 = [%.32s]\n", 	ptr->Quote_EntryID);
    LogRaw( "Always SP (SPOT) Tenor        Sett_Type              2   74 = [%.2s]\n", 	ptr->Sett_Type);
    LogRaw( "Best Price                    MDBest_Px             16   76 = [%.16s]\n", 	ptr->MDBest_Px);
    LogRaw( "Best Quantity                 MDBest_Size           16   92 = [%.16s]\n", 	ptr->MDBest_Size);
    LogRaw( "%s", "----------------------------------------------------------------------[ SISE_ENTRY ]----\n");

    return sizeof( SISE_ENTRY);
}

int FX_QUOTE_T_Print( FX_QUOTE_T* ptr)
{
    LogRaw( "%s", "----[ FX_QUOTE_T ]----------------------------------------------------------------------\n");
    LogRaw( "Message type.                 Msg_Type               4    0 = [%.4s]\n", 	ptr->Msg_Type);
    LogRaw( "Message sender identifie      Sender_CompID         15    4 = [%.15s]\n", 	ptr->Sender_CompID);
    LogRaw( "Sending time (GMT). Time      Sending_Time          24   19 = [%.24s]\n", 	ptr->Sending_Time);
    LogRaw( "Primary Currency/Counter      Symb                   8   43 = [%.8s]\n", 	ptr->Symb);
    LogRaw( "Number of entries in th       No_MDEntries           4   51 = [%.4s]\n", 	ptr->No_MDEntries);
    SISE_ENTRY_Print( &ptr->entry[0]);
    SISE_ENTRY_Print( &ptr->entry[1]);
    LogRaw( "%s", "----------------------------------------------------------------------[ FX_QUOTE_T ]----\n");

    return sizeof( FX_QUOTE_T);
}

int SPLIT_IN_ST_Print( SPLIT_IN_ST* ptr)
{
    LogRaw( "%s", "----[ SPLIT_IN_ST ]---------------------------------------------------------------------\n");
    LogRaw( "고객번호                      s_csac_idnt_no        31    0 = [%.31s]\n", 	ptr->s_csac_idnt_no);
    LogRaw( "원천구분                      s_csac_orgn_gb         2   31 = [%.2s]\n", 	ptr->s_csac_orgn_gb);
    LogRaw( "통화페어                      s_pair_id              8   33 = [%.8s]\n", 	ptr->s_pair_id);
    LogRaw( "FX상품구분코드                s_sett_type            4   41 = [%.4s]\n", 	ptr->s_sett_type);
    LogRaw( "테너유형구분코드(S:표준,U:비  s_tnr_ptrn_dcd         2   45 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
    LogRaw( "테너ID                        s_tnr_id               4   47 = [%.4s]\n", 	ptr->s_tnr_id);
    LogRaw( "매입매도구분코드              s_bysel_dcd            2   51 = [%.2s]\n", 	ptr->s_bysel_dcd);
    LogRaw( "만기종료년월일                s_expi_fnsh_ymd        9   53 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    LogRaw( "만기시작년월일                s_expi_sttg_ymd        9   62 = [%.9s]\n", 	ptr->s_expi_sttg_ymd);
    LogRaw( "주문가격조건코드              s_ordn_prc_cncd        2   71 = [%.2s]\n", 	ptr->s_ordn_prc_cncd);
    LogRaw( "주문가격                      d_fx_ordn_prc          8   73 = [%f]\n", 	ptr->d_fx_ordn_prc);
    LogRaw( "BID USDKRW 가격               d_bid_usd_prc          8   81 = [%f]\n", 	ptr->d_bid_usd_prc);
    LogRaw( "ASK USDKRW 가격               d_ask_usd_prc          8   89 = [%f]\n", 	ptr->d_ask_usd_prc);
    LogRaw( "BID 비재정 가격               d_bid_std_prc          8   97 = [%f]\n", 	ptr->d_bid_std_prc);
    LogRaw( "ASK 비재정 가격               d_ask_std_prc          8  105 = [%f]\n", 	ptr->d_ask_std_prc);
    LogRaw( "BID 재정 가격                 d_bid_fnl_prc          8  113 = [%f]\n", 	ptr->d_bid_fnl_prc);
    LogRaw( "ASK 재정 가격                 d_ask_fnl_prc          8  121 = [%f]\n", 	ptr->d_ask_fnl_prc);
    LogRaw( "주문수량                      d_fx_ordn_amt          8  129 = [%f]\n", 	ptr->d_fx_ordn_amt);
    LogRaw( "주문USD환산수량               d_usd_amt              8  129 = [%f]\n", 	ptr->d_usd_amt);
    LogRaw( "%s", "---------------------------------------------------------------------[ SPLIT_IN_ST ]----\n");

    return sizeof( SPLIT_IN_ST);
}

int SPLIT_IN_ST_PrintFile( SPLIT_IN_ST* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ SPLIT_IN_ST ]---------------------------------------------------------------------\n");
    fprintf( fp, "고객번호                      s_csac_idnt_no        31    0 = [%.31s]\n", 	ptr->s_csac_idnt_no);
    fprintf( fp, "원천구분                      s_csac_orgn_gb         2   31 = [%.2s]\n", 	ptr->s_csac_orgn_gb);
    fprintf( fp, "통화페어                      s_pair_id              8   33 = [%.8s]\n", 	ptr->s_pair_id);
    fprintf( fp, "FX상품구분코드                s_sett_type            4   41 = [%.4s]\n", 	ptr->s_sett_type);
    fprintf( fp, "테너유형구분코드(S:표준,U:비  s_tnr_ptrn_dcd         2   45 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
    fprintf( fp, "테너ID                        s_tnr_id               4   47 = [%.4s]\n", 	ptr->s_tnr_id);
    fprintf( fp, "매입매도구분코드              s_bysel_dcd            2   51 = [%.2s]\n", 	ptr->s_bysel_dcd);
    fprintf( fp, "만기종료년월일                s_expi_fnsh_ymd        9   53 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    fprintf( fp, "만기시작년월일                s_expi_sttg_ymd        9   62 = [%.9s]\n", 	ptr->s_expi_sttg_ymd);
    fprintf( fp, "주문가격조건코드              s_ordn_prc_cncd        2   71 = [%.2s]\n", 	ptr->s_ordn_prc_cncd);
    fprintf( fp, "주문가격                      d_fx_ordn_prc          8   73 = [%f]\n", 	ptr->d_fx_ordn_prc);
    fprintf( fp, "BID USDKRW 가격               d_bid_usd_prc          8   81 = [%f]\n", 	ptr->d_bid_usd_prc);
    fprintf( fp, "ASK USDKRW 가격               d_ask_usd_prc          8   89 = [%f]\n", 	ptr->d_ask_usd_prc);
    fprintf( fp, "BID 비재정 가격               d_bid_std_prc          8   97 = [%f]\n", 	ptr->d_bid_std_prc);
    fprintf( fp, "ASK 비재정 가격               d_ask_std_prc          8  105 = [%f]\n", 	ptr->d_ask_std_prc);
    fprintf( fp, "BID 재정 가격                 d_bid_fnl_prc          8  113 = [%f]\n", 	ptr->d_bid_fnl_prc);
    fprintf( fp, "ASK 재정 가격                 d_ask_fnl_prc          8  121 = [%f]\n", 	ptr->d_ask_fnl_prc);
    fprintf( fp, "주문수량                      d_fx_ordn_amt          8  129 = [%f]\n", 	ptr->d_fx_ordn_amt);
    fprintf( fp, "주문USD환산수량               d_usd_amt              8  129 = [%f]\n", 	ptr->d_usd_amt);
    fprintf( fp, "%s", "---------------------------------------------------------------------[ SPLIT_IN_ST ]----\n");

    return sizeof( SPLIT_IN_ST);
}

int SPLIT_PRD_LIST_Print( SPLIT_PRD_LIST* ptr)
{
    LogRaw( "%s", "----[ SPLIT_PRD_LIST ]------------------------------------------------------------------\n");
    LogRaw( "거래내역일련번호              n_trhs_srn             4    0 = [%d]\n", 	ptr->n_trhs_srn);
    LogRaw( "통화페어ID                    s_cncr_pair_id         8    4 = [%.8s]\n", 	ptr->s_cncr_pair_id);
    LogRaw( "거래유형                      s_sett_type            4   12 = [%.4s]\n", 	ptr->s_sett_type);
    LogRaw( "테너유형구분코드(S:표준,U:비  s_tnr_ptrn_dcd         2   16 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
    LogRaw( "테너ID                        s_tnr_id               4   18 = [%.4s]\n", 	ptr->s_tnr_id);
    LogRaw( "만기종료년월일                s_expi_fnsh_ymd        9   22 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    LogRaw( "매입매도구분코드              s_bysel_dcd            2   31 = [%.2s]\n", 	ptr->s_bysel_dcd);
    LogRaw( "시장SPOT가격                  d_mrkt_spt_prc         8   33 = [%f]\n", 	ptr->d_mrkt_spt_prc);
    LogRaw( "시장SWAP가격         (SWAP포  d_mrkt_swap_prc        8   41 = [%f]\n", 	ptr->d_mrkt_swap_prc);
    LogRaw( "FX시장가격                    d_fx_mrkt_prc          8   49 = [%f]\n", 	ptr->d_fx_mrkt_prc);
    LogRaw( "cover dealer 스프레드(cover   d_cvr_spr              8   57 = [%f]\n", 	ptr->d_cvr_spr);
    LogRaw( "cover dealer 가격    (cover   d_fx_cvr_prc           8   65 = [%f]\n", 	ptr->d_fx_cvr_prc);
    LogRaw( "corp dealer 스프레드 (corp    d_sls_spr              8   73 = [%f]\n", 	ptr->d_sls_spr);
    LogRaw( "당사스프레드 (본점마진)       d_orcy_spr             8   81 = [%f]\n", 	ptr->d_orcy_spr);
    LogRaw( "FX당사가격   (본점가격)       d_fx_orcy_prc          8   89 = [%f]\n", 	ptr->d_fx_orcy_prc);
    LogRaw( "고객스프레드 (영업점마진)     d_cus_spr              8   97 = [%f]\n", 	ptr->d_cus_spr);
    LogRaw( "FX고객가격                    d_fx_csac_prc          8  105 = [%f]\n", 	ptr->d_fx_csac_prc);
    LogRaw( "%s", "------------------------------------------------------------------[ SPLIT_PRD_LIST ]----\n");

    return sizeof( SPLIT_PRD_LIST);
}

int SPLIT_PRD_LIST_PrintFile( SPLIT_PRD_LIST* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ SPLIT_PRD_LIST ]------------------------------------------------------------------\n");
    fprintf( fp, "거래내역일련번호              n_trhs_srn             4    0 = [%d]\n", 	ptr->n_trhs_srn);
    fprintf( fp, "통화페어ID                    s_cncr_pair_id         8    4 = [%.8s]\n", 	ptr->s_cncr_pair_id);
    fprintf( fp, "거래유형                      s_sett_type            4   12 = [%.4s]\n", 	ptr->s_sett_type);
    fprintf( fp, "테너유형구분코드(S:표준,U:비  s_tnr_ptrn_dcd         2   16 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
    fprintf( fp, "테너ID                        s_tnr_id               4   18 = [%.4s]\n", 	ptr->s_tnr_id);
    fprintf( fp, "만기종료년월일                s_expi_fnsh_ymd        9   22 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    fprintf( fp, "매입매도구분코드              s_bysel_dcd            2   31 = [%.2s]\n", 	ptr->s_bysel_dcd);
    fprintf( fp, "시장SPOT가격                  d_mrkt_spt_prc         8   33 = [%f]\n", 	ptr->d_mrkt_spt_prc);
    fprintf( fp, "시장SWAP가격         (SWAP포  d_mrkt_swap_prc        8   41 = [%f]\n", 	ptr->d_mrkt_swap_prc);
    fprintf( fp, "FX시장가격                    d_fx_mrkt_prc          8   49 = [%f]\n", 	ptr->d_fx_mrkt_prc);
    fprintf( fp, "cover dealer 스프레드(cover   d_cvr_spr              8   57 = [%f]\n", 	ptr->d_cvr_spr);
    fprintf( fp, "cover dealer 가격    (cover   d_fx_cvr_prc           8   65 = [%f]\n", 	ptr->d_fx_cvr_prc);
    fprintf( fp, "corp dealer 스프레드 (corp    d_sls_spr              8   73 = [%f]\n", 	ptr->d_sls_spr);
    fprintf( fp, "당사스프레드 (본점마진)       d_orcy_spr             8   81 = [%f]\n", 	ptr->d_orcy_spr);
    fprintf( fp, "FX당사가격   (본점가격)       d_fx_orcy_prc          8   89 = [%f]\n", 	ptr->d_fx_orcy_prc);
    fprintf( fp, "고객스프레드 (영업점마진)     d_cus_spr              8   97 = [%f]\n", 	ptr->d_cus_spr);
    fprintf( fp, "FX고객가격                    d_fx_csac_prc          8  105 = [%f]\n", 	ptr->d_fx_csac_prc);
    fprintf( fp, "%s", "------------------------------------------------------------------[ SPLIT_PRD_LIST ]----\n");

    return sizeof( SPLIT_PRD_LIST);
}

int SPLIT_OUT_ST_Print( SPLIT_OUT_ST* ptr)
{
	int		i;

    LogRaw( "%s", "----[ SPLIT_OUT_ST ]--------------------------------------------------------------------\n");
    LogRaw( "통화페어                      s_pair_id              8    0 = [%.8s]\n", 	ptr->s_pair_id);
    LogRaw( "레코드 건수 1, 3              n_rec_cnt              4    8 = [%d]\n", 	ptr->n_rec_cnt);
	for( i = 0; i < ptr->n_rec_cnt; i++)
    	SPLIT_PRD_LIST_Print( &ptr->rec[ i]);
    LogRaw( "%s", "--------------------------------------------------------------------[ SPLIT_OUT_ST ]----\n");

    return sizeof( SPLIT_OUT_ST);
}

int SPLIT_OUT_ST_PrintFile( SPLIT_OUT_ST* ptr, FILE *fp)
{
	int		i;

    fprintf( fp, "%s", "----[ SPLIT_OUT_ST ]--------------------------------------------------------------------\n");
    fprintf( fp, "통화페어                      s_pair_id              8    0 = [%.8s]\n", 	ptr->s_pair_id);
    fprintf( fp, "레코드 건수 1, 3              n_rec_cnt              4    8 = [%d]\n", 	ptr->n_rec_cnt);
	for( i = 0; i < ptr->n_rec_cnt; i++)
    	SPLIT_PRD_LIST_PrintFile( &ptr->rec[ i], fp);
    fprintf( fp, "%s", "--------------------------------------------------------------------[ SPLIT_OUT_ST ]----\n");

    return sizeof( SPLIT_OUT_ST);
}

int MAT_E_MSG_Print( MAT_E_MSG* ptr)
{
    LogRaw( "%s", "----[ MAT_E_MSG ]-----------------------------------------------------------------------\n");
    LogRaw( "프로세스 name 16->30          pname                 30    0 = [%.30s]\n", 	ptr->pname);
    LogRaw( "Language type                 ltyp                   2   30 = [%.2s]\n", 	ptr->ltyp);
    LogRaw( "Message type  S:FXON H:HOST   mtyp                   1   32 = [%.1s]\n", 	ptr->mtyp);
    LogRaw( "Error Code type  M:메시지 그  rtyp                   1   33 = [%.1s]\n", 	ptr->rtyp);
    LogRaw( "Error Code 10-->12            code                  12   34 = [%.12s]\n", 	ptr->code);
    LogRaw( "Error Message                 mesg                 256   46 = [%.256s]\n", 	ptr->mesg);
    LogRaw( "%s", "-----------------------------------------------------------------------[ MAT_E_MSG ]----\n");

    return sizeof( MAT_E_MSG);
}

int SPLIT_IN_ST_File( SPLIT_IN_ST* ptr, FILE *fp)
{
    fprintf( fp, "----[ SPLIT_IN_ST ]---------------------------------------------------------------------\n");
    fprintf( fp, "고객번호                      s_csac_idnt_no        31    0 = [%.31s]\n", 	ptr->s_csac_idnt_no);
    fprintf( fp, "통화페어                      s_pair_id              8   31 = [%.8s]\n", 	ptr->s_pair_id);
    fprintf( fp, "FX상품구분코드                s_sett_type            4   39 = [%.4s]\n", 	ptr->s_sett_type);
    fprintf( fp, "테너유형구분코드(S:표준,U:비  s_tnr_ptrn_dcd         2   43 = [%.2s]\n", 	ptr->s_tnr_ptrn_dcd);
    fprintf( fp, "테너ID                        s_tnr_id               4   45 = [%.4s]\n", 	ptr->s_tnr_id);
    fprintf( fp, "매입매도구분코드              s_bysel_dcd            2   49 = [%.2s]\n", 	ptr->s_bysel_dcd);
    fprintf( fp, "만기종료년월일                s_expi_fnsh_ymd        9   51 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    fprintf( fp, "만기시작년월일                s_expi_sttg_ymd        9   60 = [%.9s]\n", 	ptr->s_expi_sttg_ymd);
    fprintf( fp, "주문가격조건코드              s_ordn_prc_cncd        2   69 = [%.2s]\n", 	ptr->s_ordn_prc_cncd);
    fprintf( fp, "주문가격                      d_fx_ordn_prc          8   71 = [%f]\n", 	ptr->d_fx_ordn_prc);
    fprintf( fp, "BID USDKRW 가격               d_bid_usd_prc          8   79 = [%f]\n", 	ptr->d_bid_usd_prc);
    fprintf( fp, "ASK USDKRW 가격               d_ask_usd_prc          8   87 = [%f]\n", 	ptr->d_ask_usd_prc);
    fprintf( fp, "BID 비재정 가격               d_bid_std_prc          8   95 = [%f]\n", 	ptr->d_bid_std_prc);
    fprintf( fp, "ASK 비재정 가격               d_ask_std_prc          8  103 = [%f]\n", 	ptr->d_ask_std_prc);
    fprintf( fp, "BID 재정 가격                 d_bid_fnl_prc          8  111 = [%f]\n", 	ptr->d_bid_fnl_prc);
    fprintf( fp, "ASK 재정 가격                 d_ask_fnl_prc          8  119 = [%f]\n", 	ptr->d_ask_fnl_prc);
    fprintf( fp, "---------------------------------------------------------------------[ SPLIT_IN_ST ]----\n");

    return sizeof( SPLIT_IN_ST);
}

int SPLIT_PRD_LIST_File( SPLIT_PRD_LIST* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ SPLIT_PRD_LIST ]------------------------------------------------------------------\n");
    fprintf( fp, "거래내역일련번호              n_trhs_srn             4    0 = [%d]\n", 	ptr->n_trhs_srn);
    fprintf( fp, "통화페어ID                    s_cncr_pair_id         7   65 = [%.7s]\n", 	ptr->s_cncr_pair_id);
    fprintf( fp, "테너ID                        s_tnr_id               4   72 = [%.4s]\n", 	ptr->s_tnr_id);
    fprintf( fp, "만기종료년월일                s_expi_fnsh_ymd        9   76 = [%.9s]\n", 	ptr->s_expi_fnsh_ymd);
    fprintf( fp, "매입매도구분코드              s_bysel_dcd            2   85 = [%.2s]\n", 	ptr->s_bysel_dcd);
    fprintf( fp, "시장SPOT가격                  d_mrkt_spt_prc         8   87 = [%f]\n", 	ptr->d_mrkt_spt_prc);
    fprintf( fp, "시장SWAP가격                  d_mrkt_swap_prc        8   95 = [%f]\n", 	ptr->d_mrkt_swap_prc);
    fprintf( fp, "FX시장가격                    d_fx_mrkt_prc          8  103 = [%f]\n", 	ptr->d_fx_mrkt_prc);
    fprintf( fp, "cover dealer 스프레드         d_cvr_spr              8  111 = [%f]\n", 	ptr->d_cvr_spr);
    fprintf( fp, "cover dealer 가격             d_fx_cvr_prc           8  119 = [%f]\n", 	ptr->d_fx_cvr_prc);
    fprintf( fp, "corp dealer 스프레드          d_sls_spr              8  127 = [%f]\n", 	ptr->d_sls_spr);
    fprintf( fp, "당사스프레드 (본점마진)       d_orcy_spr             8  135 = [%f]\n", 	ptr->d_orcy_spr);
    fprintf( fp, "FX당사가격   (본점가격)       d_fx_orcy_prc          8  143 = [%f]\n", 	ptr->d_fx_orcy_prc);
    fprintf( fp, "고객스프레드 (영업점마진)     d_cus_spr              8  151 = [%f]\n", 	ptr->d_cus_spr);
    fprintf( fp, "FX고객가격                    d_fx_csac_prc          8  159 = [%f]\n", 	ptr->d_fx_csac_prc);
    fprintf( fp, "%s", "------------------------------------------------------------------[ SPLIT_PRD_LIST ]----\n");

    return sizeof( SPLIT_PRD_LIST);
}

int SPLIT_OUT_ST_File( SPLIT_OUT_ST* ptr, FILE *fp)
{
	int		i;

    fprintf( fp, "%s", "----[ SPLIT_OUT_ST ]--------------------------------------------------------------------\n");
    fprintf( fp, "통화페어                      s_pair_id              7    0 = [%.7s]\n", 	ptr->s_pair_id);
    fprintf( fp, "레코드 건수                   n_rec_cnt              4    7 = [ %1d]\n", 	ptr->n_rec_cnt);
    for( i = 0; i < 3; i++) 
	{
		fprintf( fp, ">>> SPLIT_PRD_LIST [%d]\n", i);
		SPLIT_PRD_LIST_File( &ptr->rec[ i], fp);
	}
    fprintf( fp, "%s", "--------------------------------------------------------------------[ SPLIT_OUT_ST ]----\n");

    return sizeof( SPLIT_OUT_ST);
}

int MAT_COMM_HEAD_Print( MAT_COMM_HEAD* ptr)
{
    LogRaw( "%s", "----[ MAT_COMM_HEAD ]-------------------------------------------------------------------\n");
    LogRaw( "자신 제외 전체 size           len                    4    0 = [%.4s]\n", 	ptr->len);
    LogRaw( "LINK|LIOK|ERCD|HTBT|HTOK|DAT  type                   4    4 = [%.4s]\n", 	ptr->type);
    LogRaw( "순번                          seq                    8    8 = [%.8s]\n", 	ptr->seq);
    LogRaw( "block count                   block                  2   16 = [%.2s]\n", 	ptr->block);
    LogRaw( "error code                    code                   2   18 = [%.2s]\n", 	ptr->code);
    LogRaw( "YYYYMMDDssssss                date                  20   20 = [%.20s]\n", 	ptr->date);
    LogRaw( "filler                        filer                 10   40 = [%.10s]\n", 	ptr->filler);
    LogRaw( "%s", "-------------------------------------------------------------------[ MAT_COMM_HEAD ]----\n");

    return sizeof( MAT_COMM_HEAD);
}

int MAT_DATA_HEAD_Print( MAT_DATA_HEAD* ptr)
{
    LogRaw( "%s", "----[ MAT_DATA_HEAD ]-------------------------------------------------------------------\n");
    LogRaw( "data length                   len                   10    0 = [%.10s]\n", 	ptr->len);
    LogRaw( "LOGIN|REJE|IF01 ...           type                   5   10 = [%.5s]\n", 	ptr->type);
    LogRaw( "sequence                      seq                   10   15 = [%.10s]\n", 	ptr->seq);
    LogRaw( "응답코드                      code                   5   25 = [%.5s]\n", 	ptr->code);
    LogRaw( "                              date                  20   30 = [%.20s]\n", 	ptr->date);
    LogRaw( "%s", "-------------------------------------------------------------------[ MAT_DATA_HEAD ]----\n");

    return sizeof( MAT_DATA_HEAD);
}

int MAT_PACKET_Print( MAT_PACKET* ptr)
{
    LogRaw( "%s", "----[ MAT_PACKET ]----------------------------------------------------------------------\n");
    MAT_COMM_HEAD_Print( &ptr->comm_head);
    MAT_DATA_HEAD_Print( &ptr->data_head);
    LogRaw( "                              data                 8192    0 = [%.8192s]\n", 	ptr->data);
    LogRaw( "%s", "----------------------------------------------------------------------[ MAT_PACKET ]----\n");

    return sizeof( MAT_PACKET);
}

int MAT_HEAD_Print( MAT_HEAD* ptr)
{
	LogRaw( "###################################################\n");
	LogRaw( "##### 헤더 정보                               #####\n");
	LogRaw( "###################################################\n");
    LogRaw( "%s", "----[ MAT_HEAD ]------------------------------------------------------------------------\n");
    LogRaw( "0-빈 record, 1-주문 record,   gubun                  4    0 = [%d]\n", 	ptr->gubun);
    LogRaw( "상태                          ord_stat               4    0 = [%d]\n", 	ptr->ord_stat);
    LogRaw( "주문 error number 0:no error  error                  4    4 = [%d]\n", 	ptr->error);
	LogRaw( "###################################################\n");
	LogRaw( "##### 통계 정보                               #####\n");
	LogRaw( "###################################################\n");
    LogRaw( "접수시간                                             4    4 = [%s:%06d]\n", 	
			TtoS( ptr->rcv_time.tv_sec), ptr->rcv_time.tv_usec);
    LogRaw( "주문확인송신시간                                     4    4 = [%s:%06d]\n", 	
			TtoS( ptr->con_time.tv_sec), ptr->con_time.tv_usec);
    LogRaw( "주문확인대기시간                                     4    4 = [%s:%06d]\n", 	
			TtoS( ptr->dly_time.tv_sec), ptr->dly_time.tv_usec);
    LogRaw( "주문저장시간                                         4    4 = [%s:%06d]\n", 	
			TtoS( ptr->ord_time.tv_sec), ptr->ord_time.tv_usec);
    LogRaw( "주문체결시간                                         4    4 = [%s:%06d]\n", 	
			TtoS( ptr->mat_time.tv_sec), ptr->mat_time.tv_usec);
    LogRaw( "체결송신시간                                         4    4 = [%s:%06d]\n", 	
			TtoS( ptr->snd_time.tv_sec), ptr->snd_time.tv_usec);
	LogRaw( "###################################################\n");
	LogRaw( "##### 시세 정보                               #####\n");
	LogRaw( "###################################################\n");
	LogRaw( "##### 기준통화/상대통화   #####\n");
    MATSISE_Print( &ptr->sise_curr);
	LogRaw( "##### USD/기준통화        #####\n");
    MATSISE_Print( &ptr->sise_base);
	LogRaw( "##### USD/상대통화        #####\n");
    MATSISE_Print( &ptr->sise_cont);
	LogRaw( "###################################################\n");
	LogRaw( "##### 수수료  정보                            #####\n");
	LogRaw( "###################################################\n");
	LogRaw( "##### 수수료 INPUT        #####\n");
    SPLIT_IN_ST_Print( &ptr->fee_in);
	LogRaw( "##### 수수료 OUTPUT       #####\n");
    SPLIT_OUT_ST_Print( &ptr->fee_out);
    LogRaw( "주문 가격                     price                  8    8 = [%f]\n", 	ptr->price);
    LogRaw( "체결가격                      exe_price              8   16 = [%f]\n", 	ptr->exe_price);
    LogRaw( "matching type 일반 = 0        mat_type               4   24 = [%d]\n", 	ptr->mat_type);
    LogRaw( "trailling stop 주문 pips gap  ts_gap                 4   28 = [%d]\n", 	ptr->ts_gap);
    LogRaw( "record가 속한 index number    idx_no                 4   32 = [%d]\n", 	ptr->idx_no);
    LogRaw( "장ID MA1 TD2 TM3 SP4 FW5      jang_id                4   32 = [%d]\n", 	ptr->jang_id);
    LogRaw( "이전 record 위치, 시작 recor  prev                   4   36 = [%d]\n", 	ptr->prev);
    LogRaw( "다음 record 위치, 끝 record   next                   4   40 = [%d]\n", 	ptr->next);
    LogRaw( "주문 대기열 이전 record 위치  wait_prev              4   44 = [%d]\n", 	ptr->wait_prev);
    LogRaw( "주문 대기열 다음 record 위치  wait_next              4   48 = [%d]\n", 	ptr->wait_next);
    LogRaw( "group 주문 이전               grp_prev               4   52 = [%d]\n", 	ptr->grp_prev);
    LogRaw( "group 주문 다음               grp_next               4   56 = [%d]\n", 	ptr->grp_next);
    LogRaw( "%s", "------------------------------------------------------------------------[ MAT_HEAD ]----\n");
	LogRaw( "###################################################\n");
	LogRaw( "##### 주문/체결                               #####\n");
	LogRaw( "###################################################\n");

    return sizeof( MAT_HEAD);
}

int MAT_HEAD_PrintFile( MAT_HEAD* ptr, FILE *fp)
{
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 헤더 정보                               #####\n");
	fprintf( fp, "###################################################\n");
    fprintf( fp, "%s", "----[ MAT_HEAD ]------------------------------------------------------------------------\n");
    fprintf( fp, "0-빈 record, 1-주문 record,   gubun                  4    0 = [%d]\n", 	ptr->gubun);
    fprintf( fp, "상태                          ord_stat               4    0 = [%d]\n", 	ptr->ord_stat);
    fprintf( fp, "주문 error number 0:no error  error                  4    4 = [%d]\n", 	ptr->error);
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 통계 정보                               #####\n");
	fprintf( fp, "###################################################\n");
    fprintf( fp, "접수시간                                             4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->rcv_time.tv_sec), ptr->rcv_time.tv_usec);
    fprintf( fp, "주문확인송신시간                                     4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->con_time.tv_sec), ptr->con_time.tv_usec);
    fprintf( fp, "주문확인대기시간                                     4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->dly_time.tv_sec), ptr->dly_time.tv_usec);
    fprintf( fp, "주문저장시간                                         4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->ord_time.tv_sec), ptr->ord_time.tv_usec);
    fprintf( fp, "주문체결시간                                         4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->mat_time.tv_sec), ptr->mat_time.tv_usec);
    fprintf( fp, "체결송신시간                                         4    4 = [%s:%06ld]\n", 	
			TtoS( ptr->snd_time.tv_sec), ptr->snd_time.tv_usec);
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 시세 정보                               #####\n");
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 기준통화/상대통화   #####\n");
    MATSISE_PrintFile( &ptr->sise_curr, fp);
	fprintf( fp, "##### USD/기준통화        #####\n");
    MATSISE_PrintFile( &ptr->sise_base, fp);
	fprintf( fp, "##### USD/상대통화        #####\n");
    MATSISE_PrintFile( &ptr->sise_cont, fp);
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 수수료  정보                            #####\n");
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 수수료 INPUT        #####\n");
   	SPLIT_IN_ST_PrintFile( &ptr->fee_in, fp);
	fprintf( fp, "##### 수수료 OUTPUT       #####\n");
   	SPLIT_OUT_ST_PrintFile( &ptr->fee_out, fp);
	if( ptr->fee_in_far.s_csac_idnt_no[ 0] != 0)
	{
		fprintf( fp, "##### 수수료 INPUT SWAP FAR  #####\n");
    	SPLIT_IN_ST_PrintFile( &ptr->fee_in_far, fp);
		fprintf( fp, "##### 수수료 OUTPUT SWAP FAR #####\n");
    	SPLIT_OUT_ST_PrintFile( &ptr->fee_out_far, fp);
	}
    fprintf( fp, "주문 가격                     price                  8    8 = [%f]\n", 	ptr->price);
    fprintf( fp, "체결가격                      exe_price              8   16 = [%f]\n", 	ptr->exe_price);
    fprintf( fp, "matching type 일반 = 0        mat_type               4   24 = [%d]\n", 	ptr->mat_type);
    fprintf( fp, "trailling stop 주문 pips gap  ts_gap                 4   28 = [%d]\n", 	ptr->ts_gap);
    fprintf( fp, "record가 속한 index number    idx_no                 4   32 = [%d]\n", 	ptr->idx_no);
    fprintf( fp, "장ID MA1 TD2 TM3 SP4 FW5      jang_id                4   32 = [%d]\n", 	ptr->jang_id);
    fprintf( fp, "이전 record 위치, 시작 recor  prev                   4   36 = [%d]\n", 	ptr->prev);
    fprintf( fp, "다음 record 위치, 끝 record   next                   4   40 = [%d]\n", 	ptr->next);
    fprintf( fp, "주문 대기열 이전 record 위치  wait_prev              4   44 = [%d]\n", 	ptr->wait_prev);
    fprintf( fp, "주문 대기열 다음 record 위치  wait_next              4   48 = [%d]\n", 	ptr->wait_next);
    fprintf( fp, "group 주문 이전               grp_prev               4   52 = [%d]\n", 	ptr->grp_prev);
    fprintf( fp, "group 주문 다음               grp_next               4   56 = [%d]\n", 	ptr->grp_next);
    fprintf( fp, "%s", "------------------------------------------------------------------------[ MAT_HEAD ]----\n");
	fprintf( fp, "###################################################\n");
	fprintf( fp, "##### 주문/체결                               #####\n");
	fprintf( fp, "###################################################\n");

    return sizeof( MAT_HEAD);
}

int MAT_START_Print( MAT_START* ptr)
{
    LogRaw( "%s", "----[ MAT_START ]-----------------------------------------------------------------------\n");
    LogRaw( "start position, 없으면 -1     start                  4    0 = [%d]\n", 	ptr->start);
    LogRaw( "index 내의 주문 수량          start_cnt              4    4 = [%d]\n", 	ptr->start_cnt);
    LogRaw( "주문 wait start position, ex  wait                   4    8 = [%d]\n", 	ptr->wait);
    LogRaw( "index 내의 wait 주문 수량     wait_cnt               4   12 = [%d]\n", 	ptr->wait_cnt);
    LogRaw( "%s", "-----------------------------------------------------------------------[ MAT_START ]----\n");

    return sizeof( MAT_START);
}

int MAT_START_PrintFile( MAT_START* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ MAT_START ]-----------------------------------------------------------------------\n");
    fprintf( fp, "start position, 없으면 -1     start                  4    0 = [%d]\n", 	ptr->start);
    fprintf( fp, "index 내의 주문 수량          start_cnt              4    4 = [%d]\n", 	ptr->start_cnt);
    fprintf( fp, "주문 wait start position, ex  wait                   4    8 = [%d]\n", 	ptr->wait);
    fprintf( fp, "index 내의 wait 주문 수량     wait_cnt               4   12 = [%d]\n", 	ptr->wait_cnt);
    fprintf( fp, "%s", "-----------------------------------------------------------------------[ MAT_START ]----\n");

    return sizeof( MAT_START);
}

int MAT_INDEX_Print( MAT_INDEX* ptr)
{
    LogRaw( "%s", "----[ MAT_INDEX ]-----------------------------------------------------------------------\n");
    LogRaw( "index number                  no                     4    0 = [%d]\n", 	ptr->no);
    LogRaw( "기준통화 A                    base_cur               4    4 = [%d]\n", 	ptr->base_cur);
    LogRaw( "상대통화 B                    cont_cur               4    8 = [%d]\n", 	ptr->cont_cur);
    LogRaw( "시세 유효시간                 price_time             4   12 = [%d]\n", 	ptr->price_time);
    MATSISE_Print( &ptr->sise_curr);
    MATSISE_Print( &ptr->sise_base);
    MATSISE_Print( &ptr->sise_cont);
    LogRaw( "소숫점 이하 처리              point                  4   16 = [%d]\n", 	ptr->point);
    LogRaw( "거래단위                      unit                   4   20 = [%d]\n", 	ptr->unit);
    MAT_START_Print( &ptr->start[ 0]);
    MAT_START_Print( &ptr->start[ 1]);
    LogRaw( "%s", "-----------------------------------------------------------------------[ MAT_INDEX ]----\n");

    return sizeof( MAT_INDEX);
}

int MAT_INDEX_PrintFile( MAT_INDEX* ptr, FILE *fp)
{
    fprintf( fp, "%s", "----[ MAT_INDEX ]-----------------------------------------------------------------------\n");
    fprintf( fp, "index number                  no                     4    0 = [%d]\n", 	ptr->no);
    fprintf( fp, "기준통화 A                    base_cur               4    4 = [%d]\n", 	ptr->base_cur);
    fprintf( fp, "상대통화 B                    cont_cur               4    8 = [%d]\n", 	ptr->cont_cur);
    fprintf( fp, "시세 유효시간                 price_time             4   12 = [%d]\n", 	ptr->price_time);
	fprintf( fp, "[ 현재통화 시세 ]\n");
    MATSISE_PrintFile( &ptr->sise_curr, fp);
	fprintf( fp, "[ 기본통화 시세 ]\n");
    MATSISE_PrintFile( &ptr->sise_base, fp);
	fprintf( fp, "[ 상대통화 시세 ]\n");
    MATSISE_PrintFile( &ptr->sise_cont, fp);
    fprintf( fp, "소숫점 이하 처리              point                  4   16 = [%d]\n", 	ptr->point);
    fprintf( fp, "거래단위                      unit                   4   20 = [%d]\n", 	ptr->unit);
	fprintf( fp, "[ 매수 INDEX ]\n");
    MAT_START_PrintFile( &ptr->start[ 0], fp);
	fprintf( fp, "[ 매도 INDEX ]\n");
    MAT_START_PrintFile( &ptr->start[ 1], fp);
    fprintf( fp, "%s", "-----------------------------------------------------------------------[ MAT_INDEX ]----\n");

    return sizeof( MAT_INDEX);
}

int MAT_GROUP_Print( MAT_GROUP* ptr)
{
    LogRaw( "%s", "----[ MAT_GROUP ]-----------------------------------------------------------------------\n");
    LogRaw( "그룹주문 시작 position - sin  start                  4    0 = [%d]\n", 	ptr->start);
    LogRaw( "그룹 주문 번호 - GrpOrdnNo    id                    11    4 = [%.11s]\n", 	ptr->id);
    LogRaw( "그룹 주문 건수 - GrpOrdnCnt   seq                    4   15 = [%d]\n", 	ptr->seq);
    LogRaw( "그룹 주문 건수 - GrpOrdnCnt   tot                    4   19 = [%d]\n", 	ptr->tot);
    LogRaw( "저장된 그룹주문 건수 - GrpOr  cnt                    4   23 = [%d]\n", 	ptr->cnt);
    LogRaw( "%s", "-----------------------------------------------------------------------[ MAT_GROUP ]----\n");

    return sizeof( MAT_GROUP);
}


