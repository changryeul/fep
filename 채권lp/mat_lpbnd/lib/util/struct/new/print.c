#include "log.h"
#include "bok_fx.h"

int FX0067_DB_FORM_Print( FX0067_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0067_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek                8   13 = [%f]", 	ptr->GeumAek);
    LogNoh( "선물 환율                     SeonMulHwanYul         8   21 = [%f]", 	ptr->SeonMulHwanYul);
    LogNoh( "현물 환율                     HyeonMulHwanYul        8   29 = [%f]", 	ptr->HyeonMulHwanYul);
    LogNoh( "Fixing Date                   FixingDate             9   37 = [%9.9s]", 	ptr->FixingDate);
    LogNoh( "Value Date                    ValueDate              9   46 = [%9.9s]", 	ptr->ValueDate);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0067_DB_FORM ]----");

    return sizeof( FX0067_DB_FORM);
}

int FX0068_DB_FORM_Print( FX0068_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0068_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek                8   13 = [%f]", 	ptr->GeumAek);
    LogNoh( "Near value date               NearValueDate          9   21 = [%9.9s]", 	ptr->NearValueDate);
    LogNoh( "Far value date                FarValueDate           9   30 = [%9.9s]", 	ptr->FarValueDate);
    LogNoh( "기간(일수)                    GiGan                  4   39 = [%d]", 	ptr->GiGan);
    LogNoh( "마진(전)                      MaJin                  8   43 = [%f]", 	ptr->MaJin);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0068_DB_FORM ]----");

    return sizeof( FX0068_DB_FORM);
}

int FX0069_DB_FORM_Print( FX0069_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0069_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "상품구분                      Good_Fg                2    0 = [%2.2s]", 	ptr->Good_Fg);
    LogNoh( "체결번호                      CheGyeolNo             4    2 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    6 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek                8   15 = [%f]", 	ptr->GeumAek);
    LogNoh( "시작일                        SiJakDate              9   23 = [%9.9s]", 	ptr->SiJakDate);
    LogNoh( "종료일                        JongRyoDate            9   32 = [%9.9s]", 	ptr->JongRyoDate);
    LogNoh( "기간(일수)                    GiGan                  4   41 = [%d]", 	ptr->GiGan);
    LogNoh( "고정금리                      GoJeongGeumLi          8   45 = [%f]", 	ptr->GoJeongGeumLi);
    LogNoh( "변동금리                      ByeondongGeumLi        8   53 = [%f]", 	ptr->ByeondongGeumLi);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        5   61 = [%5.5s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        5   66 = [%5.5s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0069_DB_FORM ]----");

    return sizeof( FX0069_DB_FORM);
}

int FX0070_DB_FORM_Print( FX0070_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0070_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "Type                          Type                   2   13 = [%2.2s]", 	ptr->Type);
    LogNoh( "Put/Call                      PutCall                2   15 = [%2.2s]", 	ptr->PutCall);
    LogNoh( "금액                          GeumAek                8   17 = [%f]", 	ptr->GeumAek);
    LogNoh( "행사가격                      HaengSaGaGyeok         8   25 = [%f]", 	ptr->HaengSaGaGyeok);
    LogNoh( "옵션가격(원화)                OptionGaGyeokWon       8   33 = [%f]", 	ptr->OptionGaGyeokWon);
    LogNoh( "옵션가격(달러화)              OptionGaGyeokUS        8   41 = [%f]", 	ptr->OptionGaGyeokUS);
    LogNoh( "행사만기일                    HaengSaManGiDate       9   49 = [%9.9s]", 	ptr->HaengSaManGiDate);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        5   58 = [%5.5s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        5   63 = [%5.5s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0070_DB_FORM ]----");

    return sizeof( FX0070_DB_FORM);
}

int FX0071_DB_FORM_Print( FX0071_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0071_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek                8   13 = [%f]", 	ptr->GeumAek);
    LogNoh( "환율                          HwanYul                8   21 = [%f]", 	ptr->HwanYul);
    LogNoh( "Fixing Date                   FixingDate             9   29 = [%9.9s]", 	ptr->FixingDate);
    LogNoh( "Value Date                    ValueDate              9   38 = [%9.9s]", 	ptr->ValueDate);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        5   47 = [%5.5s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        5   52 = [%5.5s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0071_DB_FORM ]----");

    return sizeof( FX0071_DB_FORM);
}

int FX0072_DB_FORM_Print( FX0072_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0072_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "Term                          Term                   3   13 = [%3.3s]", 	ptr->Term);
    LogNoh( "Put/Call                      PutCall                5   16 = [%5.5s]", 	ptr->PutCall);
    LogNoh( "금액                          GeumAek                8   21 = [%f]", 	ptr->GeumAek);
    LogNoh( "옵션가격(원화)                OptionGaGyeokWon       8   29 = [%f]", 	ptr->OptionGaGyeokWon);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0072_DB_FORM ]----");

    return sizeof( FX0072_DB_FORM);
}

int FX0073_DB_FORM_Print( FX0073_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0073_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             4    0 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           9    4 = [%9.9s]", 	ptr->CheGyeolDate);
    LogNoh( "기준통화 코드                 GiJunTongHwaCode       4   13 = [%4.4s]", 	ptr->GiJunTongHwaCode);
    LogNoh( "상대통화 코드                 SangDaeTongHwaCode     4   17 = [%4.4s]", 	ptr->SangDaeTongHwaCode);
    LogNoh( "상품 코드                     SangPumCode            2   21 = [%2.2s]", 	ptr->SangPumCode);
    LogNoh( "기일 코드                     GiilCode               3   23 = [%3.3s]", 	ptr->GiilCode);
    LogNoh( "체결 시각                     CheGyeolTime           7   26 = [%7.7s]", 	ptr->CheGyeolTime);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        5   33 = [%5.5s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        5   38 = [%5.5s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "매매율                        MaeMaeYul              8   43 = [%f]", 	ptr->MaeMaeYul);
    LogNoh( "체결 금액                     CheGyeolGeumAek        8   51 = [%f]", 	ptr->CheGyeolGeumAek);
    LogNoh( "현물 환율                     HyeonMulHwanYul        8   59 = [%f]", 	ptr->HyeonMulHwanYul);
    LogNoh( "결제 일자                     GyeolJeDate            9   67 = [%9.9s]", 	ptr->GyeolJeDate);
    LogNoh( "만기 일자                     ManGiDate              9   76 = [%9.9s]", 	ptr->ManGiDate);
    LogNoh( "장내/장외 구분                JangNaeOeFlag          2   85 = [%2.2s]", 	ptr->JangNaeOeFlag);
    LogNoh( "Near value date               NearValueDate          9   87 = [%9.9s]", 	ptr->NearValueDate);
    LogNoh( "Far value date                FarValueDate           9   96 = [%9.9s]", 	ptr->FarValueDate);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0073_DB_FORM ]----");

    return sizeof( FX0073_DB_FORM);
}

int FX0074_DB_FORM_Print( FX0074_DB_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0074_DB_FORM ]------------------------------------------------------------------");
    LogNoh( "상품 코드                     SangPumCode            3    0 = [%3.3s]", 	ptr->SangPumCode);
    LogNoh( "체결번호                      CheGyeolNo             4    3 = [%d]", 	ptr->CheGyeolNo);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        5    7 = [%5.5s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        5   12 = [%5.5s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "금액(단위:USD)                GeumAek                8   17 = [%f]", 	ptr->GeumAek);
    LogNoh( "기준통화 코드                 GiJunTongHwaCode       4   25 = [%4.4s]", 	ptr->GiJunTongHwaCode);
    LogNoh( "상대통화 코드                 SangDaeTongHwaCode     4   29 = [%4.4s]", 	ptr->SangDaeTongHwaCode);
    LogNoh( "체결 시각                     CheGyeolTime           7   33 = [%7.7s]", 	ptr->CheGyeolTime);
    LogNoh( "장내/장외 구분                JangNaeOeFlag          2   40 = [%2.2s]", 	ptr->JangNaeOeFlag);
    LogNoh( "결제일                        GyeolJeDate            9   42 = [%9.9s]", 	ptr->GyeolJeDate);
    LogNoh( "시작일                        SiJakDate              9   51 = [%9.9s]", 	ptr->SiJakDate);
    LogNoh( "종료일                        JongRyoDate            9   60 = [%9.9s]", 	ptr->JongRyoDate);
    LogNoh( "행사만기일                    HaengSaManGiDate       9   69 = [%9.9s]", 	ptr->HaengSaManGiDate);
    LogNoh( "기일 코드                     GiilCode               3   78 = [%3.3s]", 	ptr->GiilCode);
    LogNoh( "거래 환율                     GeoRaeHwanYul          8   81 = [%f]", 	ptr->GeoRaeHwanYul);
    LogNoh( "근일물 거래환율               NearRate               8   89 = [%f]", 	ptr->NearRate);
    LogNoh( "원일물 거래환율               FarRate                8   97 = [%f]", 	ptr->FarRate);
    LogNoh( "시장평균환율 대비 가감율(단�  MarginWon              8  105 = [%f]", 	ptr->MarginWon);
    LogNoh( "고정금리                      GoJeongGeumLi          8  113 = [%f]", 	ptr->GoJeongGeumLi);
    LogNoh( "중개사 거래 변동금리 코드     ByeondongGeumLiCode    3  121 = [%3.3s]", 	ptr->ByeondongGeumLiCode);
    LogNoh( "변동금리                      ByeondongGeumLi        8  124 = [%f]", 	ptr->ByeondongGeumLi);
    LogNoh( "Put/Call                      PutCall                2  132 = [%2.2s]", 	ptr->PutCall);
    LogNoh( "Type                          Type                   2  134 = [%2.2s]", 	ptr->Type);
    LogNoh( "옵션가격(달러화)              OptionGaGyeokUS        8  136 = [%f]", 	ptr->OptionGaGyeokUS);
    LogNoh( "%s", "------------------------------------------------------------------[ FX0074_DB_FORM ]----");

    return sizeof( FX0074_DB_FORM);
}

int FX0067_DATA_FORM_Print( FX0067_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0067_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek               16   13 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "선물 환율                     SeonMulHwanYul        12   29 = [%12.12s]", 	ptr->SeonMulHwanYul);
    LogNoh( "현물 환율                     HyeonMulHwanYul       12   41 = [%12.12s]", 	ptr->HyeonMulHwanYul);
    LogNoh( "Fixing Date                   FixingDate             8   53 = [%8.8s]", 	ptr->FixingDate);
    LogNoh( "Value Date                    ValueDate              8   61 = [%8.8s]", 	ptr->ValueDate);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0067_DATA_FORM ]----");

    return sizeof( FX0067_DATA_FORM);
}

int FX0068_DATA_FORM_Print( FX0068_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0068_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek               16   13 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "Near value date               NearValueDate          8   29 = [%8.8s]", 	ptr->NearValueDate);
    LogNoh( "Far value date                FarValueDate           8   37 = [%8.8s]", 	ptr->FarValueDate);
    LogNoh( "기간(일수)                    GiGan                  4   45 = [%4.4s]", 	ptr->GiGan);
    LogNoh( "마진(전)                      MaJin                 16   49 = [%16.16s]", 	ptr->MaJin);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0068_DATA_FORM ]----");

    return sizeof( FX0068_DATA_FORM);
}

int FX0069_DATA_FORM_Print( FX0069_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0069_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "상품구분                      Good_Fg                1    0 = [%1.1s]", 	ptr->Good_Fg);
    LogNoh( "체결번호                      CheGyeolNo             5    1 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    6 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek               16   14 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "시작일                        SiJakDate              8   30 = [%8.8s]", 	ptr->SiJakDate);
    LogNoh( "종료일                        JongRyoDate            8   38 = [%8.8s]", 	ptr->JongRyoDate);
    LogNoh( "기간(일수)                    GiGan                  4   46 = [%4.4s]", 	ptr->GiGan);
    LogNoh( "고정금리                      GoJeongGeumLi         12   50 = [%12.12s]", 	ptr->GoJeongGeumLi);
    LogNoh( "변동금리                      ByeondongGeumLi       12   62 = [%12.12s]", 	ptr->ByeondongGeumLi);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        4   74 = [%4.4s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        4   78 = [%4.4s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0069_DATA_FORM ]----");

    return sizeof( FX0069_DATA_FORM);
}

int FX0070_DATA_FORM_Print( FX0070_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0070_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "Type                          Type                   1   13 = [%1.1s]", 	ptr->Type);
    LogNoh( "Put/Call                      PutCall                1   14 = [%1.1s]", 	ptr->PutCall);
    LogNoh( "금액                          GeumAek               16   15 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "행사가격                      HaengSaGaGyeok        16   31 = [%16.16s]", 	ptr->HaengSaGaGyeok);
    LogNoh( "옵션가격(원화)                OptionGaGyeokWon      16   47 = [%16.16s]", 	ptr->OptionGaGyeokWon);
    LogNoh( "옵션가격(달러화)              OptionGaGyeokUS       16   63 = [%16.16s]", 	ptr->OptionGaGyeokUS);
    LogNoh( "행사만기일                    HaengSaManGiDate       8   79 = [%8.8s]", 	ptr->HaengSaManGiDate);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        4   87 = [%4.4s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        4   91 = [%4.4s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0070_DATA_FORM ]----");

    return sizeof( FX0070_DATA_FORM);
}

int FX0071_DATA_FORM_Print( FX0071_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0071_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "금액                          GeumAek               16   13 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "환율                          HwanYul               12   29 = [%12.12s]", 	ptr->HwanYul);
    LogNoh( "Fixing Date                   FixingDate             8   41 = [%8.8s]", 	ptr->FixingDate);
    LogNoh( "Value Date                    ValueDate              8   49 = [%8.8s]", 	ptr->ValueDate);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        4   57 = [%4.4s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        4   61 = [%4.4s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0071_DATA_FORM ]----");

    return sizeof( FX0071_DATA_FORM);
}

int FX0072_DATA_FORM_Print( FX0072_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0072_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "Term                          Term                   2   13 = [%2.2s]", 	ptr->Term);
    LogNoh( "Put/Call                      PutCall                1   15 = [%1.1s]", 	ptr->PutCall);
    LogNoh( "금액                          GeumAek               16   16 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "옵션가격(원화)                OptionGaGyeokWon      16   32 = [%16.16s]", 	ptr->OptionGaGyeokWon);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0072_DATA_FORM ]----");

    return sizeof( FX0072_DATA_FORM);
}

int FX0073_DATA_FORM_Print( FX0073_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0073_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "체결번호                      CheGyeolNo             5    0 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "체결일자                      CheGyeolDate           8    5 = [%8.8s]", 	ptr->CheGyeolDate);
    LogNoh( "기준통화 코드                 GiJunTongHwaCode       3   13 = [%3.3s]", 	ptr->GiJunTongHwaCode);
    LogNoh( "상대통화 코드                 SangDaeTongHwaCode     3   16 = [%3.3s]", 	ptr->SangDaeTongHwaCode);
    LogNoh( "상품 코드                     SangPumCode            1   19 = [%1.1s]", 	ptr->SangPumCode);
    LogNoh( "기일 코드                     GiilCode               2   20 = [%2.2s]", 	ptr->GiilCode);
    LogNoh( "체결 시각                     CheGyeolTime           6   22 = [%6.6s]", 	ptr->CheGyeolTime);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        4   28 = [%4.4s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        4   32 = [%4.4s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "매매율                        MaeMaeYul             12   36 = [%12.12s]", 	ptr->MaeMaeYul);
    LogNoh( "체결 금액                     CheGyeolGeumAek       16   48 = [%16.16s]", 	ptr->CheGyeolGeumAek);
    LogNoh( "현물 환율                     HyeonMulHwanYul       12   64 = [%12.12s]", 	ptr->HyeonMulHwanYul);
    LogNoh( "결제 일자                     GyeolJeDate            8   76 = [%8.8s]", 	ptr->GyeolJeDate);
    LogNoh( "만기 일자                     ManGiDate              8   84 = [%8.8s]", 	ptr->ManGiDate);
    LogNoh( "장내/장외 구분                JangNaeOeFlag          1   92 = [%1.1s]", 	ptr->JangNaeOeFlag);
    LogNoh( "Near value date               NearValueDate          8   93 = [%8.8s]", 	ptr->NearValueDate);
    LogNoh( "Far value date                FarValueDate           8  101 = [%8.8s]", 	ptr->FarValueDate);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0073_DATA_FORM ]----");

    return sizeof( FX0073_DATA_FORM);
}

int FX0074_DATA_FORM_Print( FX0074_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0074_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "상품 코드                     SangPumCode            2    0 = [%2.2s]", 	ptr->SangPumCode);
    LogNoh( "체결번호                      CheGyeolNo             5    2 = [%5.5s]", 	ptr->CheGyeolNo);
    LogNoh( "매입 업체 코드                MaeIpEopCheCode        4    7 = [%4.4s]", 	ptr->MaeIpEopCheCode);
    LogNoh( "매도 업체 코드                MaeDoEopCheCode        4   11 = [%4.4s]", 	ptr->MaeDoEopCheCode);
    LogNoh( "금액                          GeumAek               16   15 = [%16.16s]", 	ptr->GeumAek);
    LogNoh( "기준통화 코드                 GiJunTongHwaCode       3   31 = [%3.3s]", 	ptr->GiJunTongHwaCode);
    LogNoh( "상대통화 코드                 SangDaeTongHwaCode     3   34 = [%3.3s]", 	ptr->SangDaeTongHwaCode);
    LogNoh( "체결 시각                     CheGyeolTime           6   37 = [%6.6s]", 	ptr->CheGyeolTime);
    LogNoh( "장내/장외 구분                JangNaeOeFlag          1   43 = [%1.1s]", 	ptr->JangNaeOeFlag);
    LogNoh( "결제일                        GyeolJeDate            8   44 = [%8.8s]", 	ptr->GyeolJeDate);
    LogNoh( "시작일                        SiJakDate              8   52 = [%8.8s]", 	ptr->SiJakDate);
    LogNoh( "종료일                        JongRyoDate            8   60 = [%8.8s]", 	ptr->JongRyoDate);
    LogNoh( "행사만기일                    HaengSaManGiDate       8   68 = [%8.8s]", 	ptr->HaengSaManGiDate);
    LogNoh( "기일 코드                     GiilCode               2   76 = [%2.2s]", 	ptr->GiilCode);
    LogNoh( "거래 환율                     GeoRaeHwanYul         12   78 = [%12.12s]", 	ptr->GeoRaeHwanYul);
    LogNoh( "근일물 거래환율               NearRate              12   90 = [%12.12s]", 	ptr->NearRate);
    LogNoh( "원일물 거래환율               FarRate               12  102 = [%12.12s]", 	ptr->FarRate);
    LogNoh( "시장평균환율 대비 가감율(단�  MarginWon             12  114 = [%12.12s]", 	ptr->MarginWon);
    LogNoh( "고정금리                      GoJeongGeumLi         12  126 = [%12.12s]", 	ptr->GoJeongGeumLi);
    LogNoh( "중개사 거래 변동금리 코드     ByeondongGeumLiCode    2  138 = [%2.2s]", 	ptr->ByeondongGeumLiCode);
    LogNoh( "변동금리                      ByeondongGeumLi       12  140 = [%12.12s]", 	ptr->ByeondongGeumLi);
    LogNoh( "Put/Call                      PutCall                1  152 = [%1.1s]", 	ptr->PutCall);
    LogNoh( "Type                          Type                   1  153 = [%1.1s]", 	ptr->Type);
    LogNoh( "옵션가격(달러화)              OptionGaGyeokUS       16  154 = [%16.16s]", 	ptr->OptionGaGyeokUS);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0074_DATA_FORM ]----");

    return sizeof( FX0074_DATA_FORM);
}

int FX1000_DATA_FORM_Print( FX1000_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ FX1000_DATA_FORM ]----------------------------------------------------------------");
    LogNoh( "보고기관코드                  BoGoGiGwanCode         4    0 = [%4.4s]", 	ptr->BoGoGiGwanCode);
    LogNoh( "취급점포코드                  ChwiGeupJeomPoCode     4    4 = [%4.4s]", 	ptr->ChwiGeupJeomPoCode);
    LogNoh( "전송일자(오늘자)              JeonSongDate           8    8 = [%8.8s]", 	ptr->JeonSongDate);
    LogNoh( "전송횟수                      JeonSongCount          2   16 = [%2.2s]", 	ptr->JeonSongCount);
    LogNoh( "전송구분                      JeonSongType           1   18 = [%c]", 	ptr->JeonSongType);
    LogNoh( "처리번호                      CheoRiNo               6   19 = [%6.6s]", 	ptr->CheoRiNo);
    LogNoh( "                              FileCode               6   25 = [%6.6s]", 	ptr->FileCode);
    LogNoh( "보고점포그룹구분              BoGoJeomPoGroup        1   31 = [%c]", 	ptr->BoGoJeomPoGroup);
    LogNoh( "작성기준일자                  JakSeongGiJunDate      8   32 = [%8.8s]", 	ptr->JakSeongGiJunDate);
    LogNoh( "                              Report                 1   40 = [%c]", 	ptr->Report);
    LogNoh( "%s", "----------------------------------------------------------------[ FX1000_DATA_FORM ]----");

    return sizeof( FX1000_DATA_FORM);
}

int FILE_HEADER_FORM_Print( FILE_HEADER_FORM* ptr)
{
    LogNoh( "%s", "----[ FILE_HEADER_FORM ]----------------------------------------------------------------");
    LogNoh( "보고기관코드                  BoGoGiGwanCode         4    0 = [%4.4s]", 	ptr->BoGoGiGwanCode);
    LogNoh( "취급점포코드                  ChwiGeupJeomPoCode     4    4 = [%4.4s]", 	ptr->ChwiGeupJeomPoCode);
    LogNoh( "취급점포소재지                ChwiGeupJeomPoPost     6    8 = [%6.6s]", 	ptr->ChwiGeupJeomPoPost);
    LogNoh( "보고점포그룹구분              BoGoJeomPoGroup        1   14 = [%c]", 	ptr->BoGoJeomPoGroup);
    LogNoh( "전송일자(오늘자)              JeonSongDate           8   15 = [%8.8s]", 	ptr->JeonSongDate);
    LogNoh( "작성기준일자                  JakSeongGiJunDate      8   23 = [%8.8s]", 	ptr->JakSeongGiJunDate);
    LogNoh( "전송횟수                      JeonSongCount          2   31 = [%2.2s]", 	ptr->JeonSongCount);
    LogNoh( "전송구분                      JeonSongType           1   33 = [%c]", 	ptr->JeonSongType);
    LogNoh( "처리번호                      CheoRiNo               6   34 = [%6.6s]", 	ptr->CheoRiNo);
    LogNoh( "%s", "----------------------------------------------------------------[ FILE_HEADER_FORM ]----");

    return sizeof( FILE_HEADER_FORM);
}

int FX0067_FILE_FORM_Print( FX0067_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0067_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0067_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0067_FILE_FORM ]----");

    return sizeof( FX0067_FILE_FORM);
}

int FX0068_FILE_FORM_Print( FX0068_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0068_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0068_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0068_FILE_FORM ]----");

    return sizeof( FX0068_FILE_FORM);
}

int FX0069_FILE_FORM_Print( FX0069_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0069_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0069_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0069_FILE_FORM ]----");

    return sizeof( FX0069_FILE_FORM);
}

int FX0070_FILE_FORM_Print( FX0070_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0070_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0070_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0070_FILE_FORM ]----");

    return sizeof( FX0070_FILE_FORM);
}

int FX0071_FILE_FORM_Print( FX0071_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0071_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0071_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0071_FILE_FORM ]----");

    return sizeof( FX0071_FILE_FORM);
}

int FX0072_FILE_FORM_Print( FX0072_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0072_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0072_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0072_FILE_FORM ]----");

    return sizeof( FX0072_FILE_FORM);
}

int FX0073_FILE_FORM_Print( FX0073_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0073_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0073_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0073_FILE_FORM ]----");

    return sizeof( FX0073_FILE_FORM);
}

int FX0074_FILE_FORM_Print( FX0074_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX0074_FILE_FORM ]----------------------------------------------------------------");
    FILE_HEADER_FORM_Print( &ptr->Head);
    FX0074_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX0074_FILE_FORM ]----");

    return sizeof( FX0074_FILE_FORM);
}

int FX1000_FILE_FORM_Print( FX1000_FILE_FORM* ptr)
{
    LogNoh( "%s", "----[ FX1000_FILE_FORM ]----------------------------------------------------------------");
    FX1000_DATA_FORM_Print( &ptr->Data);
    LogNoh( "%s", "----------------------------------------------------------------[ FX1000_FILE_FORM ]----");

    return sizeof( FX1000_FILE_FORM);
}

int NET_COMMON_FORM_Print( NET_COMMON_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_COMMON_FORM ]-----------------------------------------------------------------");
    LogNoh( "전문길이정보                  TelgLength             4    0 = [%4.4s]", 	ptr->TelgLength);
    LogNoh( "'FOREXFTP2'                   Transaction            9    4 = [%9.9s]", 	ptr->Transaction);
    LogNoh( "'FXB'                         SystemName             3   13 = [%3.3s]", 	ptr->SystemName);
    LogNoh( "'7103'                        CompanyCode            4   16 = [%4.4s]", 	ptr->CompanyCode);
    LogNoh( "                              MessageCode            8   20 = [%8.8s]", 	ptr->MessageCode);
    LogNoh( "'  '(space)                   Filler                 2   28 = [%2.2s]", 	ptr->Filler);
    LogNoh( "                              ContinueYN             1   30 = [%1.1s]", 	ptr->ContinueYN);
    LogNoh( "'000'                         ResponseCode           3   31 = [%3.3s]", 	ptr->ResponseCode);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_COMMON_FORM ]----");

    return sizeof( NET_COMMON_FORM);
}

int NET_WORK_OPEN_FORM_Print( NET_WORK_OPEN_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_WORK_OPEN_FORM ]--------------------------------------------------------------");
    LogNoh( "                              CurrentDateTime       14    0 = [%14.14s]", 	ptr->CurrentDateTime);
    LogNoh( "'7103FTP0000000000001'        SendID                20   14 = [%20.20s]", 	ptr->SendID);
    LogNoh( "'7103FTP@'                    SendPW                16   34 = [%16.16s]", 	ptr->SendPW);
    LogNoh( "                              ConfirmGankyuk         4   50 = [%4.4s]", 	ptr->ConfirmGankyuk);
    LogNoh( "'B'                           SendMode               1   54 = [%1.1s]", 	ptr->SendMode);
    LogNoh( "'0'(사용안함)                 CompressFg             1   55 = [%1.1s]", 	ptr->CompressFg);
    LogNoh( "이어받기 사용여부             InheritYN              1   56 = [%1.1s]", 	ptr->InheritYN);
    LogNoh( "%s", "--------------------------------------------------------------[ NET_WORK_OPEN_FORM ]----");

    return sizeof( NET_WORK_OPEN_FORM);
}

int NET_WORK_CLOSE_FORM_Print( NET_WORK_CLOSE_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_WORK_CLOSE_FORM ]-------------------------------------------------------------");
    LogNoh( "                              CloseDateTime         14    0 = [%14.14s]", 	ptr->CloseDateTime);
    LogNoh( "%s", "-------------------------------------------------------------[ NET_WORK_CLOSE_FORM ]----");

    return sizeof( NET_WORK_CLOSE_FORM);
}

int NET_FILE_SEND_REQ_FORM_Print( NET_FILE_SEND_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_SEND_REQ_FORM ]----------------------------------------------------------");
    LogNoh( "                              SendReqDate            8    0 = [%8.8s]", 	ptr->SendReqDate);
    LogNoh( "                              FileCode               6    8 = [%6.6s]", 	ptr->FileCode);
    LogNoh( "                              DataType               2   14 = [%2.2s]", 	ptr->DataType);
    LogNoh( "%s", "----------------------------------------------------------[ NET_FILE_SEND_REQ_FORM ]----");

    return sizeof( NET_FILE_SEND_REQ_FORM);
}

int NET_FILE_SEND_RES_FORM_Print( NET_FILE_SEND_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_SEND_RES_FORM ]----------------------------------------------------------");
    LogNoh( "                              SendReqDate            8    0 = [%8.8s]", 	ptr->SendReqDate);
    LogNoh( "                              Result                 1    8 = [%1.1s]", 	ptr->Result);
    LogNoh( "                              DataCnt                3    9 = [%3.3s]", 	ptr->DataCnt);
    LogNoh( "%s", "----------------------------------------------------------[ NET_FILE_SEND_RES_FORM ]----");

    return sizeof( NET_FILE_SEND_RES_FORM);
}

int NET_FILE_BEGIN_REQ_FORM_Print( NET_FILE_BEGIN_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_BEGIN_REQ_FORM ]---------------------------------------------------------");
    LogNoh( "                              FileCode               6    0 = [%6.6s]", 	ptr->FileCode);
    LogNoh( "                              DataType               2    6 = [%2.2s]", 	ptr->DataType);
    LogNoh( "                              RecordLength           4    8 = [%4.4s]", 	ptr->RecordLength);
    LogNoh( "                              CompressFg             1   12 = [%1.1s]", 	ptr->CompressFg);
    LogNoh( "                              TotalLength           10   13 = [%10.10s]", 	ptr->TotalLength);
    LogNoh( "%s", "---------------------------------------------------------[ NET_FILE_BEGIN_REQ_FORM ]----");

    return sizeof( NET_FILE_BEGIN_REQ_FORM);
}

int NET_FILE_BEGIN_RES_FORM_Print( NET_FILE_BEGIN_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_BEGIN_RES_FORM ]---------------------------------------------------------");
    LogNoh( "                              FileCode               6    0 = [%6.6s]", 	ptr->FileCode);
    LogNoh( "                              DataType               2    6 = [%2.2s]", 	ptr->DataType);
    LogNoh( "                              RecordLength           4    8 = [%4.4s]", 	ptr->RecordLength);
    LogNoh( "                              CompressFg             1   12 = [%1.1s]", 	ptr->CompressFg);
    LogNoh( "                              TotalLength           10   13 = [%10.10s]", 	ptr->TotalLength);
    LogNoh( "                              InheritYN              1   23 = [%1.1s]", 	ptr->InheritYN);
    LogNoh( "                              RecvLength            10   24 = [%10.10s]", 	ptr->RecvLength);
    LogNoh( "%s", "---------------------------------------------------------[ NET_FILE_BEGIN_RES_FORM ]----");

    return sizeof( NET_FILE_BEGIN_RES_FORM);
}

int NET_FILE_DATA_SEND_FORM_Print( NET_FILE_DATA_SEND_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_DATA_SEND_FORM ]---------------------------------------------------------");
    LogNoh( "                              Sequence               7    0 = [%7.7s]", 	ptr->Sequence);
    LogNoh( "                              SentLength            10    7 = [%10.10s]", 	ptr->SentLength);
    LogNoh( "                              DataLength             4   17 = [%4.4s]", 	ptr->DataLength);
    LogNoh( "%s", "---------------------------------------------------------[ NET_FILE_DATA_SEND_FORM ]----");

    return sizeof( NET_FILE_DATA_SEND_FORM);
}

int NET_FILE_DATA_FORM_Print( NET_FILE_DATA_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_DATA_FORM ]--------------------------------------------------------------");
    LogNoh( "                              FileCode               6    0 = [%6.6s]", 	ptr->FileCode);
    LogNoh( "                              DataType               2    6 = [%2.2s]", 	ptr->DataType);
    LogNoh( "                              SequenceNo             7    8 = [%7.7s]", 	ptr->SequenceNo);
    LogNoh( "%s", "--------------------------------------------------------------[ NET_FILE_DATA_FORM ]----");

    return sizeof( NET_FILE_DATA_FORM);
}

int NET_LOST_REQ_FORM_Print( NET_LOST_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_LOST_REQ_FORM ]---------------------------------------------------------------");
    LogNoh( "                              LastSequence           7    0 = [%7.7s]", 	ptr->LastSequence);
    LogNoh( "                              TotSentLength         10    7 = [%10.10s]", 	ptr->TotSentLength);
    LogNoh( "%s", "---------------------------------------------------------------[ NET_LOST_REQ_FORM ]----");

    return sizeof( NET_LOST_REQ_FORM);
}

int NET_LOST_RES_FORM_Print( NET_LOST_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_LOST_RES_FORM ]---------------------------------------------------------------");
    LogNoh( "                              Result                 2    0 = [%2.2s]", 	ptr->Result);
    LogNoh( "                              LastSequenceNo         7    2 = [%7.7s]", 	ptr->LastSequenceNo);
    LogNoh( "                              LastRecvLength        10    9 = [%10.10s]", 	ptr->LastRecvLength);
    LogNoh( "%s", "---------------------------------------------------------------[ NET_LOST_RES_FORM ]----");

    return sizeof( NET_LOST_RES_FORM);
}

int NET_FILE_CLOSE_REQ_FORM_Print( NET_FILE_CLOSE_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_CLOSE_REQ_FORM ]---------------------------------------------------------");
    LogNoh( "                              LastSequence           7    0 = [%7.7s]", 	ptr->LastSequence);
    LogNoh( "                              TotSentLength         10    7 = [%10.10s]", 	ptr->TotSentLength);
    LogNoh( "                              CloseDateTime         14   17 = [%14.14s]", 	ptr->CloseDateTime);
    LogNoh( "%s", "---------------------------------------------------------[ NET_FILE_CLOSE_REQ_FORM ]----");

    return sizeof( NET_FILE_CLOSE_REQ_FORM);
}

int NET_FILE_CLOSE_RES_FORM_Print( NET_FILE_CLOSE_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_FILE_CLOSE_RES_FORM ]---------------------------------------------------------");
    LogNoh( "                              Result                 2    0 = [%2.2s]", 	ptr->Result);
    LogNoh( "                              LastSequenceNo         7    2 = [%7.7s]", 	ptr->LastSequenceNo);
    LogNoh( "                              LastRecvLength        10    9 = [%10.10s]", 	ptr->LastRecvLength);
    LogNoh( "                              CloseDateTime         14   19 = [%14.14s]", 	ptr->CloseDateTime);
    LogNoh( "                              ServerControlNo       33   33 = [%33.33s]", 	ptr->ServerControlNo);
    LogNoh( "%s", "---------------------------------------------------------[ NET_FILE_CLOSE_RES_FORM ]----");

    return sizeof( NET_FILE_CLOSE_RES_FORM);
}

int NET_PW_CHG_REQ_FORM_Print( NET_PW_CHG_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_PW_CHG_REQ_FORM ]-------------------------------------------------------------");
    LogNoh( "'7103FTP0000000000001'        SendID                20    0 = [%20.20s]", 	ptr->SendID);
    LogNoh( "'7103FTP@'                    SendPW_OLD            16   20 = [%16.16s]", 	ptr->SendPW_OLD);
    LogNoh( "'7103FTP@'                    SendPW_NEW            16   36 = [%16.16s]", 	ptr->SendPW_NEW);
    LogNoh( "%s", "-------------------------------------------------------------[ NET_PW_CHG_REQ_FORM ]----");

    return sizeof( NET_PW_CHG_REQ_FORM);
}

int NET_PW_CHG_RES_FORM_Print( NET_PW_CHG_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_PW_CHG_RES_FORM ]-------------------------------------------------------------");
    LogNoh( "                              Result                 2    0 = [%2.2s]", 	ptr->Result);
    LogNoh( "%s", "-------------------------------------------------------------[ NET_PW_CHG_RES_FORM ]----");

    return sizeof( NET_PW_CHG_RES_FORM);
}

int NET_STATUS_REQ_FORM_Print( NET_STATUS_REQ_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_STATUS_REQ_FORM ]-------------------------------------------------------------");
    LogNoh( "                              CurrentDateTime       14    0 = [%14.14s]", 	ptr->CurrentDateTime);
    LogNoh( "%s", "-------------------------------------------------------------[ NET_STATUS_REQ_FORM ]----");

    return sizeof( NET_STATUS_REQ_FORM);
}

int NET_STATUS_RES_FORM_Print( NET_STATUS_RES_FORM* ptr)
{
    LogNoh( "%s", "----[ NET_STATUS_RES_FORM ]-------------------------------------------------------------");
    LogNoh( "                              CurrentDateTime       14    0 = [%14.14s]", 	ptr->CurrentDateTime);
    LogNoh( "%s", "-------------------------------------------------------------[ NET_STATUS_RES_FORM ]----");

    return sizeof( NET_STATUS_RES_FORM);
}

int NET_DATA_FX0067_Print( NET_DATA_FX0067* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0067 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0067_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0067 ]----");

    return sizeof( NET_DATA_FX0067);
}

int NET_DATA_FX0068_Print( NET_DATA_FX0068* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0068 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0068_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0068 ]----");

    return sizeof( NET_DATA_FX0068);
}

int NET_DATA_FX0069_Print( NET_DATA_FX0069* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0069 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0069_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0069 ]----");

    return sizeof( NET_DATA_FX0069);
}

int NET_DATA_FX0070_Print( NET_DATA_FX0070* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0070 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0070_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0070 ]----");

    return sizeof( NET_DATA_FX0070);
}

int NET_DATA_FX0071_Print( NET_DATA_FX0071* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0071 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0071_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0071 ]----");

    return sizeof( NET_DATA_FX0071);
}

int NET_DATA_FX0072_Print( NET_DATA_FX0072* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0072 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0072_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0072 ]----");

    return sizeof( NET_DATA_FX0072);
}

int NET_DATA_FX0073_Print( NET_DATA_FX0073* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0073 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0073_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0073 ]----");

    return sizeof( NET_DATA_FX0073);
}

int NET_DATA_FX0074_Print( NET_DATA_FX0074* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX0074 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX0074_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0074 ]----");

    return sizeof( NET_DATA_FX0074);
}

int NET_DATA_FX1000_Print( NET_DATA_FX1000* ptr)
{
    LogNoh( "%s", "----[ NET_DATA_FX1000 ]-----------------------------------------------------------------");
    NET_FILE_DATA_FORM_Print( &ptr->Info);
    FX1000_FILE_FORM_Print( &ptr->Data);
    LogNoh( "%s", "-----------------------------------------------------------------[ NET_DATA_FX1000 ]----");

    return sizeof( NET_DATA_FX1000);
}

int PACKET_WORK_OPEN_Print( PACKET_WORK_OPEN* ptr)
{
    LogNoh( "%s", "----[ PACKET_WORK_OPEN ]----------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_WORK_OPEN_FORM_Print( &ptr->Info);
    LogNoh( "%s", "----------------------------------------------------------------[ PACKET_WORK_OPEN ]----");

    return sizeof( PACKET_WORK_OPEN);
}

int PACKET_WORK_CLOSE_Print( PACKET_WORK_CLOSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_WORK_CLOSE ]---------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_WORK_CLOSE_FORM_Print( &ptr->Info);
    LogNoh( "%s", "---------------------------------------------------------------[ PACKET_WORK_CLOSE ]----");

    return sizeof( PACKET_WORK_CLOSE);
}

int PACKET_FILE_SEND_REQUEST_Print( PACKET_FILE_SEND_REQUEST* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_SEND_REQUEST ]--------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_SEND_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "--------------------------------------------------------[ PACKET_FILE_SEND_REQUEST ]----");

    return sizeof( PACKET_FILE_SEND_REQUEST);
}

int PACKET_FILE_SEND_RESPONSE_Print( PACKET_FILE_SEND_RESPONSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_SEND_RESPONSE ]-------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_SEND_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-------------------------------------------------------[ PACKET_FILE_SEND_RESPONSE ]----");

    return sizeof( PACKET_FILE_SEND_RESPONSE);
}

int PACKET_FILE_BEGIN_REQUEST_Print( PACKET_FILE_BEGIN_REQUEST* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_BEGIN_REQUEST ]-------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_BEGIN_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-------------------------------------------------------[ PACKET_FILE_BEGIN_REQUEST ]----");

    return sizeof( PACKET_FILE_BEGIN_REQUEST);
}

int PACKET_FILE_BEGIN_RESPONSE_Print( PACKET_FILE_BEGIN_RESPONSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_BEGIN_RESPONSE ]------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_BEGIN_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "------------------------------------------------------[ PACKET_FILE_BEGIN_RESPONSE ]----");

    return sizeof( PACKET_FILE_BEGIN_RESPONSE);
}

int PACKET_FILE_CLOSE_REQ_Print( PACKET_FILE_CLOSE_REQ* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_CLOSE_REQ ]-----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_CLOSE_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-----------------------------------------------------------[ PACKET_FILE_CLOSE_REQ ]----");

    return sizeof( PACKET_FILE_CLOSE_REQ);
}

int PACKET_FILE_CLOSE_RES_Print( PACKET_FILE_CLOSE_RES* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_CLOSE_RES ]-----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_CLOSE_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-----------------------------------------------------------[ PACKET_FILE_CLOSE_RES ]----");

    return sizeof( PACKET_FILE_CLOSE_RES);
}

int PACKET_LOST_REQUEST_Print( PACKET_LOST_REQUEST* ptr)
{
    LogNoh( "%s", "----[ PACKET_LOST_REQUEST ]-------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_LOST_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-------------------------------------------------------------[ PACKET_LOST_REQUEST ]----");

    return sizeof( PACKET_LOST_REQUEST);
}

int PACKET_LOST_RESPONSE_Print( PACKET_LOST_RESPONSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_LOST_RESPONSE ]------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_LOST_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "------------------------------------------------------------[ PACKET_LOST_RESPONSE ]----");

    return sizeof( PACKET_LOST_RESPONSE);
}

int PACKET_PW_CHG_REQUEST_Print( PACKET_PW_CHG_REQUEST* ptr)
{
    LogNoh( "%s", "----[ PACKET_PW_CHG_REQUEST ]-----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_PW_CHG_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-----------------------------------------------------------[ PACKET_PW_CHG_REQUEST ]----");

    return sizeof( PACKET_PW_CHG_REQUEST);
}

int PACKET_PW_CHG_RESPONSE_Print( PACKET_PW_CHG_RESPONSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_PW_CHG_RESPONSE ]----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_PW_CHG_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "----------------------------------------------------------[ PACKET_PW_CHG_RESPONSE ]----");

    return sizeof( PACKET_PW_CHG_RESPONSE);
}

int PACKET_STATUS_REQUEST_Print( PACKET_STATUS_REQUEST* ptr)
{
    LogNoh( "%s", "----[ PACKET_STATUS_REQUEST ]-----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_STATUS_REQ_FORM_Print( &ptr->Info);
    LogNoh( "%s", "-----------------------------------------------------------[ PACKET_STATUS_REQUEST ]----");

    return sizeof( PACKET_STATUS_REQUEST);
}

int PACKET_STATUS_RESPONSE_Print( PACKET_STATUS_RESPONSE* ptr)
{
    LogNoh( "%s", "----[ PACKET_STATUS_RESPONSE ]----------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_STATUS_RES_FORM_Print( &ptr->Info);
    LogNoh( "%s", "----------------------------------------------------------[ PACKET_STATUS_RESPONSE ]----");

    return sizeof( PACKET_STATUS_RESPONSE);
}

int PACKET_FILE_FX0067_Print( PACKET_FILE_FX0067* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0067 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0067_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0067 ]----");

    return sizeof( PACKET_FILE_FX0067);
}

int PACKET_FILE_FX0068_Print( PACKET_FILE_FX0068* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0068 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0068_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0068 ]----");

    return sizeof( PACKET_FILE_FX0068);
}

int PACKET_FILE_FX0069_Print( PACKET_FILE_FX0069* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0069 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0069_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0069 ]----");

    return sizeof( PACKET_FILE_FX0069);
}

int PACKET_FILE_FX0070_Print( PACKET_FILE_FX0070* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0070 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0070_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0070 ]----");

    return sizeof( PACKET_FILE_FX0070);
}

int PACKET_FILE_FX0071_Print( PACKET_FILE_FX0071* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0071 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0071_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0071 ]----");

    return sizeof( PACKET_FILE_FX0071);
}

int PACKET_FILE_FX0072_Print( PACKET_FILE_FX0072* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0072 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0072_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0072 ]----");

    return sizeof( PACKET_FILE_FX0072);
}

int PACKET_FILE_FX0073_Print( PACKET_FILE_FX0073* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0073 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0073_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0073 ]----");

    return sizeof( PACKET_FILE_FX0073);
}

int PACKET_FILE_FX0074_Print( PACKET_FILE_FX0074* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX0074 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX0074_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0074 ]----");

    return sizeof( PACKET_FILE_FX0074);
}

int PACKET_FILE_FX1000_Print( PACKET_FILE_FX1000* ptr)
{
    LogNoh( "%s", "----[ PACKET_FILE_FX1000 ]--------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    NET_DATA_FX1000_Print( &ptr->Data);
    LogNoh( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX1000 ]----");

    return sizeof( PACKET_FILE_FX1000);
}

int PACKET_MESSAGE_FORM_Print( PACKET_MESSAGE_FORM* ptr)
{
    LogNoh( "%s", "----[ PACKET_MESSAGE_FORM ]-------------------------------------------------------------");
    NET_COMMON_FORM_Print( &ptr->Common);
    NET_FILE_DATA_SEND_FORM_Print( &ptr->DataSend);
    LogNoh( "                              Data                 4045    0 = [%4045.4045s]", 	ptr->Data);
    LogNoh( "%s", "-------------------------------------------------------------[ PACKET_MESSAGE_FORM ]----");

    return sizeof( PACKET_MESSAGE_FORM);
}

int RESULT_FORM_Print( RESULT_FORM* ptr)
{
    LogNoh( "%s", "----[ RESULT_FORM ]---------------------------------------------------------------------");
    LogNoh( "                              Result                 8    0 = [%8.8s]", 	ptr->Result);
    LogNoh( "                              FileCode               9    8 = [%9.9s]", 	ptr->FileCode);
    LogNoh( "                              Message              100   17 = [%100.100s]", 	ptr->Message);
    LogNoh( "%s", "---------------------------------------------------------------------[ RESULT_FORM ]----");

    return sizeof( RESULT_FORM);
}

int RECORD_COUNT_FORM_Print( RECORD_COUNT_FORM* ptr)
{
    LogNoh( "%s", "----[ RECORD_COUNT_FORM ]---------------------------------------------------------------");
    LogNoh( "                              fx0067                 4    0 = [%d]", 	ptr->fx0067);
    LogNoh( "                              fx0068                 4    4 = [%d]", 	ptr->fx0068);
    LogNoh( "                              fx0069                 4    8 = [%d]", 	ptr->fx0069);
    LogNoh( "                              fx0070                 4   12 = [%d]", 	ptr->fx0070);
    LogNoh( "                              fx0071                 4   16 = [%d]", 	ptr->fx0071);
    LogNoh( "                              fx0072                 4   20 = [%d]", 	ptr->fx0072);
    LogNoh( "                              fx0073                 4   24 = [%d]", 	ptr->fx0073);
    LogNoh( "                              fx0074                 4   28 = [%d]", 	ptr->fx0074);
    LogNoh( "%s", "---------------------------------------------------------------[ RECORD_COUNT_FORM ]----");

    return sizeof( RECORD_COUNT_FORM);
}

