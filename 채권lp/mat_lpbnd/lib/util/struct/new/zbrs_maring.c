14:24:47-458861 [37mD[0m main.c:ParamPrint(136)          output file name          = [margin.h]
14:24:47-458882 [37mM[0m main.c:ParamPrint(144)          file_cnt                  = [0]
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
    printf( "                              InvstrCode             2   85 = [%2.2s]     \n", ptr->InvstrCode);
    printf( "                              FrgInvstrCode          2   87 = [%2.2s]     \n", ptr->FrgInvstrCode);
    printf( "                              Filler2               23   89 = [%23.23s]   \n", ptr->Filler2);
    printf( "                              DpsamtTotamt          16  112 = [%16.16s]   \n", ptr->DpsamtTotamt);
    printf( "                              Dps                   16  128 = [%16.16s]   \n", ptr->Dps);
    printf( "                              SubstAmt              16  144 = [%16.16s]   \n", ptr->SubstAmt);
    printf( "                              Mgn                   16  160 = [%16.16s]   \n", ptr->Mgn);
    printf( "                              MnyMgn                16  176 = [%16.16s]   \n", ptr->MnyMgn);
    printf( "                              AddMgnTp               1  192 = [%1.1s]     \n", ptr->AddMgnTp);
    printf( "                              AddMgn                16  193 = [%16.16s]   \n", ptr->AddMgn);
    printf( "                              MnyOrdAbleAmt         16  209 = [%16.16s]   \n", ptr->MnyOrdAbleAmt);
    printf( "                              OrdAbleAmt            16  225 = [%16.16s]   \n", ptr->OrdAbleAmt);
    printf( "                              MnyOrdAbleAmt         16  241 = [%16.16s]   \n", ptr->MnyOrdAbleAmt);
    printf( "                              MgnWrkOrd             16  257 = [%16.16s]   \n", ptr->MgnWrkOrd);
    printf( "                              Filler3               26  273 = [%26.26s]   \n", ptr->Filler3);
    printf( "                              AfmgnLimit            16  299 = [%16.16s]   \n", ptr->AfmgnLimit);
    printf( "                              AfmgnOverYn            1  315 = [%1.1s]     \n", ptr->AfmgnOverYn);
    printf( "                              FutsNetUnsttLmtTp      1  316 = [%1.1s]     \n", ptr->FutsNetUnsttLmtTp);
    printf( "                              FutOptDeltaPosRestrc   7  317 = [%7.7s]     \n", ptr->FutOptDeltaPosRestrc);
    printf( "                              IdFutsOpttDeltaPosRestrc   7  324 = [%7.7s]     \n", ptr->IdFutsOpttDeltaPosRestrc);
    printf( "                              NowFutsOptDelta        7  331 = [%7.7s]     \n", ptr->NowFutsOptDelta);
    printf( "                              NowLdFutOptDelta       7  338 = [%7.7s]     \n", ptr->NowLdFutOptDelta);
    printf( "                              FutOptDeltaLmtOverYn   1  345 = [%1.1s]     \n", ptr->FutOptDeltaLmtOverYn);
    printf( "                              LongOrdDelta           8  346 = [%8.8s]     \n", ptr->LongOrdDelta);
    printf( "                              ShortOrdDelta          8  354 = [%8.8s]     \n", ptr->ShortOrdDelta);
    printf( "                              OptValue              12  362 = [%12.12s]   \n", ptr->OptValue);
    printf( "                              FutValue              12  374 = [%12.12s]   \n", ptr->FutValue);
    printf( "                              Filler4                2  386 = [%2.2s]     \n", ptr->Filler4);
    printf( "                              NxtDpsamtTotamt       16  388 = [%16.16s]   \n", ptr->NxtDpsamtTotamt);
    printf( "                              NxtDps                16  404 = [%16.16s]   \n", ptr->NxtDps);
    printf( "                              NxtSubstAmt           16  420 = [%16.16s]   \n", ptr->NxtSubstAmt);
    printf( "                              NxtMgn                16  436 = [%16.16s]   \n", ptr->NxtMgn);
    printf( "                              NxtMnyMgn             16  452 = [%16.16s]   \n", ptr->NxtMnyMgn);
    printf( "                              MLongOrdDelta          8  468 = [%8.8s]     \n", ptr->MLongOrdDelta);
    printf( "                              MShortOrdDelta         8  476 = [%8.8s]     \n", ptr->MShortOrdDelta);
    printf( "                              Filler5               14  484 = [%14.14s]   \n", ptr->Filler5);
    printf( "                              AcntLmtOverYn          1  498 = [%1.1s]     \n", ptr->AcntLmtOverYn);
    printf( "                              DInqOrdAbleYn          1  499 = [%1.1s]     \n", ptr->DInqOrdAbleYn);
    printf( "                              AcntFnoTotDelPos      16  500 = [%16.16s]   \n", ptr->AcntFnoTotDelPos);
    printf( "                              AcntFnoLastDelPos     16  516 = [%16.16s]   \n", ptr->AcntFnoLastDelPos);
    printf( "                              AcntFnoNxtDtLastDelPos  16  532 = [%16.16s]   \n", ptr->AcntFnoNxtDtLastDelPos);
    printf( "                              AcntFnoTotDelPos2     16  548 = [%16.16s]   \n", ptr->AcntFnoTotDelPos2);
    printf( "                              AcntTotDelPos         16  564 = [%16.16s]   \n", ptr->AcntTotDelPos);
    printf( "                              AcntFnoNxtDtLastDelPos2  16  580 = [%16.16s]   \n", ptr->AcntFnoNxtDtLastDelPos2);
    printf( "                              IdFnoLastDelPos       16  596 = [%16.16s]   \n", ptr->IdFnoLastDelPos);
    printf( "                              IdFnoLastDelPos       16  612 = [%16.16s]   \n", ptr->IdFnoLastDelPos);
    printf( "                              IdFnoNxtDtLastDelPos  16  628 = [%16.16s]   \n", ptr->IdFnoNxtDtLastDelPos);
    printf( "                              IdFnoTotDelPos2       16  644 = [%16.16s]   \n", ptr->IdFnoTotDelPos2);
    printf( "                              IdFnoLastDelPos2      16  660 = [%16.16s]   \n", ptr->IdFnoLastDelPos2);
    printf( "                              IdFnoNxtDtLastDelPos2  16  676 = [%16.16s]   \n", ptr->IdFnoNxtDtLastDelPos2);
    printf( "                              RiskExpAmt            16  692 = [%16.16s]   \n", ptr->RiskExpAmt);
    printf( "                              RiskExpLmtAmt         16  708 = [%16.16s]   \n", ptr->RiskExpLmtAmt);
    printf( "                              RiskExpLmtOverYn       1  724 = [%1.1s]     \n", ptr->RiskExpLmtOverYn);
    printf( "                              DeltaLmtOrdAbleYn      1  725 = [%1.1s]     \n", ptr->DeltaLmtOrdAbleYn);
    printf( "                              Filler6               25  726 = [%25.25s]   \n", ptr->Filler6);
    printf( "                              IdmAddMgnTp            1  751 = [%1.1s]     \n", ptr->IdmAddMgnTp);
    printf( "                              IdmOrdAbleYn           1  752 = [%1.1s]     \n", ptr->IdmOrdAbleYn);
    printf( "                              Filler7                3  753 = [%3.3s]     \n", ptr->Filler7);
    printf( "---------------------------------------------------------------------[ ZBRS_MARGIN ]----\n");

    return sizeof( ZBRS_MARGIN);
}

