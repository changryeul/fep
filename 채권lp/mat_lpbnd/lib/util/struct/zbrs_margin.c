08:22:00-507239 [37mD[0m main.c:ParamPrint(136)          output file name          = [margin.h]
08:22:00-507260 [37mM[0m main.c:ParamPrint(144)          file_cnt                  = [0]
loading file. name=[margin.h] 
#include "margin.h"

ZBRS_MARGIN_Print( ZBRS_MARGIN* ptr)
{
    printf( "----[ ZBRS_MARGIN ]---------------------------------------------------------------------\n");
    printf( "                              DataSize               4    0 = [%4.4s]     \n", ptr->DataSize);
    printf( "                              DataCode               4    4 = [%4.4s]     \n", ptr->DataCode);
    printf( "                              FrimNo                 3    8 = [%3.3s]     \n", ptr->FrimNo);
    printf( "                              TrxDate                8   11 = [%8.8s]     \n", ptr->TrxDate);
    printf( "                              TrxTime                8   19 = [%8.8s]     \n", ptr->TrxTime);
    printf( "                              DataTp                 1   27 = [%1.1s]     \n", ptr->DataTp);
    printf( "                              RealTp                 1   28 = [%1.1s]     \n", ptr->RealTp);
    printf( "                              Filler1               29   29 = [%29.29s]   \n", ptr->Filler1);
    printf( "                              AcntNo                20   58 = [%20.20s]   \n", ptr->AcntNo);
    printf( "                              AcntTpCode             2   78 = [%2.2s]     \n", ptr->AcntTpCode);
    printf( "                              AcntMgnTpCode          2   80 = [%2.2s]     \n", ptr->AcntMgnTpCode);
    printf( "                              CntryCode              3   82 = [%3.3s]     \n", ptr->CntryCode);
    printf( "                              InvstrCode             4   85 = [%4.4s]     \n", ptr->InvstrCode);
    printf( "                              FrgInvstrCode          2   89 = [%2.2s]     \n", ptr->FrgInvstrCode);
    printf( "                              Filler2               23   91 = [%23.23s]   \n", ptr->Filler2);
    printf( "                              DpsamtTotamt          16  114 = [%16.16s]   \n", ptr->DpsamtTotamt);
    printf( "                              Dps                   16  130 = [%16.16s]   \n", ptr->Dps);
    printf( "                              SubstAmt              16  146 = [%16.16s]   \n", ptr->SubstAmt);
    printf( "                              Mgn                   16  162 = [%16.16s]   \n", ptr->Mgn);
    printf( "                              MnyMgn                16  178 = [%16.16s]   \n", ptr->MnyMgn);
    printf( "                              AddMgnTp               1  194 = [%1.1s]     \n", ptr->AddMgnTp);
    printf( "                              AddMgn                16  195 = [%16.16s]   \n", ptr->AddMgn);
    printf( "                              MnyOrdAbleAmt         16  211 = [%16.16s]   \n", ptr->MnyOrdAbleAmt);
    printf( "                              OrdAbleAmt            16  227 = [%16.16s]   \n", ptr->OrdAbleAmt);
    printf( "                              MnyAddMgn             16  243 = [%16.16s]   \n", ptr->MnyAddMgn);
    printf( "                              MgnWrkOrd             16  259 = [%16.16s]   \n", ptr->MgnWrkOrd);
    printf( "                              Filler3               26  275 = [%26.26s]   \n", ptr->Filler3);
    printf( "                              AfmgnLimit            16  301 = [%16.16s]   \n", ptr->AfmgnLimit);
    printf( "                              AfmgnOverYn            1  317 = [%1.1s]     \n", ptr->AfmgnOverYn);
    printf( "                              FutsNetUnsttLmtTp      1  318 = [%1.1s]     \n", ptr->FutsNetUnsttLmtTp);
    printf( "                              FutOptDeltaPosRestrc   7  319 = [%7.7s]     \n", ptr->FutOptDeltaPosRestrc);
    printf( "                              IdFutsOpttDeltaPosRestrc   7  326 = [%7.7s]     \n", ptr->IdFutsOpttDeltaPosRestrc);
    printf( "                              NowFutsOptDelta        7  333 = [%7.7s]     \n", ptr->NowFutsOptDelta);
    printf( "                              NowLdFutOptDelta       7  340 = [%7.7s]     \n", ptr->NowLdFutOptDelta);
    printf( "                              FutOptDeltaLmtOverYn   1  347 = [%1.1s]     \n", ptr->FutOptDeltaLmtOverYn);
    printf( "                              LongOrdDelta           8  348 = [%8.8s]     \n", ptr->LongOrdDelta);
    printf( "                              ShortOrdDelta          8  356 = [%8.8s]     \n", ptr->ShortOrdDelta);
    printf( "                              OptValue              12  364 = [%12.12s]   \n", ptr->OptValue);
    printf( "                              FutValue              12  376 = [%12.12s]   \n", ptr->FutValue);
    printf( "                              Filler4                2  388 = [%2.2s]     \n", ptr->Filler4);
    printf( "                              NxtDpsamtTotamt       16  390 = [%16.16s]   \n", ptr->NxtDpsamtTotamt);
    printf( "                              NxtDps                16  406 = [%16.16s]   \n", ptr->NxtDps);
    printf( "                              NxtSubstAmt           16  422 = [%16.16s]   \n", ptr->NxtSubstAmt);
    printf( "                              NxtMgn                16  438 = [%16.16s]   \n", ptr->NxtMgn);
    printf( "                              NxtMnyMgn             16  454 = [%16.16s]   \n", ptr->NxtMnyMgn);
    printf( "                              MLongOrdDelta          8  470 = [%8.8s]     \n", ptr->MLongOrdDelta);
    printf( "                              MShortOrdDelta         8  478 = [%8.8s]     \n", ptr->MShortOrdDelta);
    printf( "                              Filler5               14  486 = [%14.14s]   \n", ptr->Filler5);
    printf( "                              AcntLmtOverYn          1  500 = [%1.1s]     \n", ptr->AcntLmtOverYn);
    printf( "                              DInqOrdAbleYn          1  501 = [%1.1s]     \n", ptr->DInqOrdAbleYn);
    printf( "                              AcntFnoTotDelPos      16  502 = [%16.16s]   \n", ptr->AcntFnoTotDelPos);
    printf( "                              AcntFnoLastDelPos     16  518 = [%16.16s]   \n", ptr->AcntFnoLastDelPos);
    printf( "                              AcntFnoNxtDtLastDelPos  16  534 = [%16.16s]   \n", ptr->AcntFnoNxtDtLastDelPos);
    printf( "                              AcntFnoTotDelPos2     16  550 = [%16.16s]   \n", ptr->AcntFnoTotDelPos2);
    printf( "                              AcntTotDelPos         16  566 = [%16.16s]   \n", ptr->AcntTotDelPos);
    printf( "                              AcntFnoNxtDtLastDelPos2  16  582 = [%16.16s]   \n", ptr->AcntFnoNxtDtLastDelPos2);
    printf( "                              IdFnoTotDelPos        16  598 = [%16.16s]   \n", ptr->IdFnoTotDelPos);
    printf( "                              IdFnoLastDelPos       16  614 = [%16.16s]   \n", ptr->IdFnoLastDelPos);
    printf( "                              IdFnoNxtDtLastDelPos  16  630 = [%16.16s]   \n", ptr->IdFnoNxtDtLastDelPos);
    printf( "                              IdFnoTotDelPos2       16  646 = [%16.16s]   \n", ptr->IdFnoTotDelPos2);
    printf( "                              IdFnoLastDelPos2      16  662 = [%16.16s]   \n", ptr->IdFnoLastDelPos2);
    printf( "                              IdFnoNxtDtLastDelPos2  16  678 = [%16.16s]   \n", ptr->IdFnoNxtDtLastDelPos2);
    printf( "                              RiskExpAmt            16  694 = [%16.16s]   \n", ptr->RiskExpAmt);
    printf( "                              RiskExpLmtAmt         16  710 = [%16.16s]   \n", ptr->RiskExpLmtAmt);
    printf( "                              RiskExpLmtOverYn       1  726 = [%1.1s]     \n", ptr->RiskExpLmtOverYn);
    printf( "                              DeltaLmtOrdAbleYn      1  727 = [%1.1s]     \n", ptr->DeltaLmtOrdAbleYn);
    printf( "                              Filler6               25  728 = [%25.25s]   \n", ptr->Filler6);
    printf( "                              IdmAddMgnTp            1  753 = [%1.1s]     \n", ptr->IdmAddMgnTp);
    printf( "                              IdmOrdAbleYn           1  754 = [%1.1s]     \n", ptr->IdmOrdAbleYn);
    printf( "                              Filler7                3  755 = [%3.3s]     \n", ptr->Filler7);
    printf( "---------------------------------------------------------------------[ ZBRS_MARGIN ]----\n");

    return sizeof( ZBRS_MARGIN);
}

