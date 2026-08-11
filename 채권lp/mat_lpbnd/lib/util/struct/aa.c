#include "bok_fx.h"

int _FX0067_DB_FORM_Print( _FX0067_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0067_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek                8    1 = [%f]        \n", ptr->GeumAek);
    printf( "선물 환율                     SeonMulHwanYul         8    9 = [%f]        \n", ptr->SeonMulHwanYul);
    printf( "현물 환율                     HyeonMulHwanYul        8   17 = [%f]        \n", ptr->HyeonMulHwanYul);
    printf( "Fixing Date                   FixingDate             1   25 = [%1.1s]     \n", ptr->FixingDate);
    printf( "Value Date                    ValueDate              1   26 = [%1.1s]     \n", ptr->ValueDate);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0067_DB_FORM ]----\n");

    return sizeof( _FX0067_DB_FORM);
}

int FX0067_DB_FORM_Print( FX0067_DB_FORM* ptr)
{
    return _FX0067_DB_FORM_Print( FX0067_DB_FORM);
}

int _FX0068_DB_FORM_Print( _FX0068_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0068_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek                8    1 = [%f]        \n", ptr->GeumAek);
    printf( "Near value date               NearValueDate          1    9 = [%1.1s]     \n", ptr->NearValueDate);
    printf( "Far value date                FarValueDate           1   10 = [%1.1s]     \n", ptr->FarValueDate);
    printf( "마진(전)                      MaJin                  8   11 = [%f]        \n", ptr->MaJin);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0068_DB_FORM ]----\n");

    return sizeof( _FX0068_DB_FORM);
}

int FX0068_DB_FORM_Print( FX0068_DB_FORM* ptr)
{
    return _FX0068_DB_FORM_Print( FX0068_DB_FORM);
}

int _FX0069_DB_FORM_Print( _FX0069_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0069_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "상품구분                      Good_Fg                2    0 = [%2.2s]     \n", ptr->Good_Fg);
    printf( "체결일자                      CheGyeolDate           1    2 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek                8    3 = [%f]        \n", ptr->GeumAek);
    printf( "시작일                        SiJakDate              1   11 = [%1.1s]     \n", ptr->SiJakDate);
    printf( "종료일                        JongRyoDate            1   12 = [%1.1s]     \n", ptr->JongRyoDate);
    printf( "고정금리                      GoJeongGeumLi          8   13 = [%f]        \n", ptr->GoJeongGeumLi);
    printf( "변동금리                      ByeondongGeumLi        8   21 = [%f]        \n", ptr->ByeondongGeumLi);
    printf( "매입 업체 코드                MaeIpEopCheCode        5   29 = [%5.5s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        5   34 = [%5.5s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0069_DB_FORM ]----\n");

    return sizeof( _FX0069_DB_FORM);
}

int FX0069_DB_FORM_Print( FX0069_DB_FORM* ptr)
{
    return _FX0069_DB_FORM_Print( FX0069_DB_FORM);
}

int _FX0070_DB_FORM_Print( _FX0070_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0070_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "Type                          Type                   2    1 = [%2.2s]     \n", ptr->Type);
    printf( "Put/Call                      PutCall                2    3 = [%2.2s]     \n", ptr->PutCall);
    printf( "금액                          GeumAek                8    5 = [%f]        \n", ptr->GeumAek);
    printf( "행사가격                      HaengSaGaGyeok         8   13 = [%f]        \n", ptr->HaengSaGaGyeok);
    printf( "옵션가격(원화)                OptionGaGyeokWon       8   21 = [%f]        \n", ptr->OptionGaGyeokWon);
    printf( "옵션가격(달러화)              OptionGaGyeokUS        8   29 = [%f]        \n", ptr->OptionGaGyeokUS);
    printf( "행사만기일                    HaengSaManGiDate       1   37 = [%1.1s]     \n", ptr->HaengSaManGiDate);
    printf( "매입 업체 코드                MaeIpEopCheCode        5   38 = [%5.5s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        5   43 = [%5.5s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0070_DB_FORM ]----\n");

    return sizeof( _FX0070_DB_FORM);
}

int FX0070_DB_FORM_Print( FX0070_DB_FORM* ptr)
{
    return _FX0070_DB_FORM_Print( FX0070_DB_FORM);
}

int _FX0071_DB_FORM_Print( _FX0071_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0071_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek                8    1 = [%f]        \n", ptr->GeumAek);
    printf( "환율                          HwanYul                8    9 = [%f]        \n", ptr->HwanYul);
    printf( "Fixing Date                   FixingDate             1   17 = [%1.1s]     \n", ptr->FixingDate);
    printf( "Value Date                    ValueDate              1   18 = [%1.1s]     \n", ptr->ValueDate);
    printf( "매입 업체 코드                MaeIpEopCheCode        5   19 = [%5.5s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        5   24 = [%5.5s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0071_DB_FORM ]----\n");

    return sizeof( _FX0071_DB_FORM);
}

int FX0071_DB_FORM_Print( FX0071_DB_FORM* ptr)
{
    return _FX0071_DB_FORM_Print( FX0071_DB_FORM);
}

int _FX0072_DB_FORM_Print( _FX0072_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0072_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "Term                          Term                   3    1 = [%3.3s]     \n", ptr->Term);
    printf( "Put/Call                      PutCall                5    4 = [%5.5s]     \n", ptr->PutCall);
    printf( "금액                          GeumAek                8    9 = [%f]        \n", ptr->GeumAek);
    printf( "옵션가격(원화)                OptionGaGyeokWon       8   17 = [%f]        \n", ptr->OptionGaGyeokWon);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0072_DB_FORM ]----\n");

    return sizeof( _FX0072_DB_FORM);
}

int FX0072_DB_FORM_Print( FX0072_DB_FORM* ptr)
{
    return _FX0072_DB_FORM_Print( FX0072_DB_FORM);
}

int _FX0073_DB_FORM_Print( _FX0073_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0073_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "체결일자                      CheGyeolDate           1    0 = [%1.1s]     \n", ptr->CheGyeolDate);
    printf( "기준통화 코드                 GiJunTongHwaCode       4    1 = [%4.4s]     \n", ptr->GiJunTongHwaCode);
    printf( "상대통화 코드                 SangDaeTongHwaCode     4    5 = [%4.4s]     \n", ptr->SangDaeTongHwaCode);
    printf( "상품 코드                     SangPumCode            2    9 = [%2.2s]     \n", ptr->SangPumCode);
    printf( "기일 코드                     GiilCode               3   11 = [%3.3s]     \n", ptr->GiilCode);
    printf( "체결 시각                     CheGyeolTime           7   14 = [%7.7s]     \n", ptr->CheGyeolTime);
    printf( "매입 업체 코드                MaeIpEopCheCode        5   21 = [%5.5s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        5   26 = [%5.5s]     \n", ptr->MaeDoEopCheCode);
    printf( "매매율                        MaeMaeYul              8   31 = [%f]        \n", ptr->MaeMaeYul);
    printf( "체결 금액                     CheGyeolGeumAek        8   39 = [%f]        \n", ptr->CheGyeolGeumAek);
    printf( "현물 환율                     HyeonMulHwanYul        8   47 = [%f]        \n", ptr->HyeonMulHwanYul);
    printf( "결제 일자                     GyeolJeDate            1   55 = [%1.1s]     \n", ptr->GyeolJeDate);
    printf( "만기 일자                     ManGiDate              1   56 = [%1.1s]     \n", ptr->ManGiDate);
    printf( "장내/장외 구분                JangNaeOeFlag          2   57 = [%2.2s]     \n", ptr->JangNaeOeFlag);
    printf( "Near value date               NearValueDate          1   59 = [%1.1s]     \n", ptr->NearValueDate);
    printf( "Far value date                FarValueDate           1   60 = [%1.1s]     \n", ptr->FarValueDate);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0073_DB_FORM ]----\n");

    return sizeof( _FX0073_DB_FORM);
}

int FX0073_DB_FORM_Print( FX0073_DB_FORM* ptr)
{
    return _FX0073_DB_FORM_Print( FX0073_DB_FORM);
}

int _FX0074_DB_FORM_Print( _FX0074_DB_FORM* ptr)
{
    printf( "%s", "----[ _FX0074_DB_FORM ]-----------------------------------------------------------------\n");
    printf( "상품 코드                     SangPumCode            3    0 = [%3.3s]     \n", ptr->SangPumCode);
    printf( "매입 업체 코드                MaeIpEopCheCode        5    3 = [%5.5s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        5    8 = [%5.5s]     \n", ptr->MaeDoEopCheCode);
    printf( "금액(단위:USD)                GeumAek                8   13 = [%f]        \n", ptr->GeumAek);
    printf( "기준통화 코드                 GiJunTongHwaCode       4   21 = [%4.4s]     \n", ptr->GiJunTongHwaCode);
    printf( "상대통화 코드                 SangDaeTongHwaCode     4   25 = [%4.4s]     \n", ptr->SangDaeTongHwaCode);
    printf( "체결 시각                     CheGyeolTime           7   29 = [%7.7s]     \n", ptr->CheGyeolTime);
    printf( "장내/장외 구분                JangNaeOeFlag          2   36 = [%2.2s]     \n", ptr->JangNaeOeFlag);
    printf( "결제일                        GyeolJeDate            1   38 = [%1.1s]     \n", ptr->GyeolJeDate);
    printf( "시작일                        SiJakDate              1   39 = [%1.1s]     \n", ptr->SiJakDate);
    printf( "종료일                        JongRyoDate            1   40 = [%1.1s]     \n", ptr->JongRyoDate);
    printf( "행사만기일                    HaengSaManGiDate       1   41 = [%1.1s]     \n", ptr->HaengSaManGiDate);
    printf( "기일 코드                     GiilCode               3   42 = [%3.3s]     \n", ptr->GiilCode);
    printf( "거래 환율                     GeoRaeHwanYul          8   45 = [%f]        \n", ptr->GeoRaeHwanYul);
    printf( "근일물 거래환율               NearRate               8   53 = [%f]        \n", ptr->NearRate);
    printf( "원일물 거래환율               FarRate                8   61 = [%f]        \n", ptr->FarRate);
    printf( "시장평균환율 대비 가감율(단�  MarginWon              8   69 = [%f]        \n", ptr->MarginWon);
    printf( "고정금리                      GoJeongGeumLi          8   77 = [%f]        \n", ptr->GoJeongGeumLi);
    printf( "중개사 거래 변동금리 코드     ByeondongGeumLiCode    3   85 = [%3.3s]     \n", ptr->ByeondongGeumLiCode);
    printf( "변동금리                      ByeondongGeumLi        8   88 = [%f]        \n", ptr->ByeondongGeumLi);
    printf( "Put/Call                      PutCall                2   96 = [%2.2s]     \n", ptr->PutCall);
    printf( "Type                          Type                   2   98 = [%2.2s]     \n", ptr->Type);
    printf( "옵션가격(달러화)              OptionGaGyeokUS        8  100 = [%f]        \n", ptr->OptionGaGyeokUS);
    printf( "%s", "--------------------------------------------------------------------------------[ _FX0074_DB_FORM ]----\n");

    return sizeof( _FX0074_DB_FORM);
}

int FX0074_DB_FORM_Print( FX0074_DB_FORM* ptr)
{
    return _FX0074_DB_FORM_Print( FX0074_DB_FORM);
}

int FX0067_DATA_FORM_Print( FX0067_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0067_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek               16    5 = [%16.16s]   \n", ptr->GeumAek);
    printf( "선물 환율                     SeonMulHwanYul        12   21 = [%12.12s]   \n", ptr->SeonMulHwanYul);
    printf( "현물 환율                     HyeonMulHwanYul       12   33 = [%12.12s]   \n", ptr->HyeonMulHwanYul);
    printf( "Fixing Date                   FixingDate             0   45 = [%0.0s]     \n", ptr->FixingDate);
    printf( "Value Date                    ValueDate              0   45 = [%0.0s]     \n", ptr->ValueDate);
    printf( "%s", "----------------------------------------------------------------[ FX0067_DATA_FORM ]----\n");

    return sizeof( FX0067_DATA_FORM);
}

int FX0068_DATA_FORM_Print( FX0068_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0068_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek               16    5 = [%16.16s]   \n", ptr->GeumAek);
    printf( "Near value date               NearValueDate          0   21 = [%0.0s]     \n", ptr->NearValueDate);
    printf( "Far value date                FarValueDate           0   21 = [%0.0s]     \n", ptr->FarValueDate);
    printf( "기간(일수)                    GiGan                  4   21 = [%4.4s]     \n", ptr->GiGan);
    printf( "마진(전)                      MaJin                 16   25 = [%16.16s]   \n", ptr->MaJin);
    printf( "%s", "----------------------------------------------------------------[ FX0068_DATA_FORM ]----\n");

    return sizeof( FX0068_DATA_FORM);
}

int FX0069_DATA_FORM_Print( FX0069_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0069_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "상품구분                      Good_Fg                1    0 = [%1.1s]     \n", ptr->Good_Fg);
    printf( "체결번호                      CheGyeolNo             5    1 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    6 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek               16    6 = [%16.16s]   \n", ptr->GeumAek);
    printf( "시작일                        SiJakDate              0   22 = [%0.0s]     \n", ptr->SiJakDate);
    printf( "종료일                        JongRyoDate            0   22 = [%0.0s]     \n", ptr->JongRyoDate);
    printf( "기간(일수)                    GiGan                  4   22 = [%4.4s]     \n", ptr->GiGan);
    printf( "고정금리                      GoJeongGeumLi         12   26 = [%12.12s]   \n", ptr->GoJeongGeumLi);
    printf( "변동금리                      ByeondongGeumLi       12   38 = [%12.12s]   \n", ptr->ByeondongGeumLi);
    printf( "매입 업체 코드                MaeIpEopCheCode        4   50 = [%4.4s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        4   54 = [%4.4s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "----------------------------------------------------------------[ FX0069_DATA_FORM ]----\n");

    return sizeof( FX0069_DATA_FORM);
}

int FX0070_DATA_FORM_Print( FX0070_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0070_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "Type                          Type                   1    5 = [%1.1s]     \n", ptr->Type);
    printf( "Put/Call                      PutCall                1    6 = [%1.1s]     \n", ptr->PutCall);
    printf( "금액                          GeumAek               16    7 = [%16.16s]   \n", ptr->GeumAek);
    printf( "행사가격                      HaengSaGaGyeok        16   23 = [%16.16s]   \n", ptr->HaengSaGaGyeok);
    printf( "옵션가격(원화)                OptionGaGyeokWon      16   39 = [%16.16s]   \n", ptr->OptionGaGyeokWon);
    printf( "옵션가격(달러화)              OptionGaGyeokUS       16   55 = [%16.16s]   \n", ptr->OptionGaGyeokUS);
    printf( "행사만기일                    HaengSaManGiDate       0   71 = [%0.0s]     \n", ptr->HaengSaManGiDate);
    printf( "매입 업체 코드                MaeIpEopCheCode        4   71 = [%4.4s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        4   75 = [%4.4s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "----------------------------------------------------------------[ FX0070_DATA_FORM ]----\n");

    return sizeof( FX0070_DATA_FORM);
}

int FX0071_DATA_FORM_Print( FX0071_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0071_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "금액                          GeumAek               16    5 = [%16.16s]   \n", ptr->GeumAek);
    printf( "환율                          HwanYul               12   21 = [%12.12s]   \n", ptr->HwanYul);
    printf( "Fixing Date                   FixingDate             0   33 = [%0.0s]     \n", ptr->FixingDate);
    printf( "Value Date                    ValueDate              0   33 = [%0.0s]     \n", ptr->ValueDate);
    printf( "매입 업체 코드                MaeIpEopCheCode        4   33 = [%4.4s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        4   37 = [%4.4s]     \n", ptr->MaeDoEopCheCode);
    printf( "%s", "----------------------------------------------------------------[ FX0071_DATA_FORM ]----\n");

    return sizeof( FX0071_DATA_FORM);
}

int FX0072_DATA_FORM_Print( FX0072_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0072_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "Term                          Term                   2    5 = [%2.2s]     \n", ptr->Term);
    printf( "Put/Call                      PutCall                1    7 = [%1.1s]     \n", ptr->PutCall);
    printf( "금액                          GeumAek               16    8 = [%16.16s]   \n", ptr->GeumAek);
    printf( "옵션가격(원화)                OptionGaGyeokWon      16   24 = [%16.16s]   \n", ptr->OptionGaGyeokWon);
    printf( "%s", "----------------------------------------------------------------[ FX0072_DATA_FORM ]----\n");

    return sizeof( FX0072_DATA_FORM);
}

int FX0073_DATA_FORM_Print( FX0073_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0073_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "체결번호                      CheGyeolNo             5    0 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "체결일자                      CheGyeolDate           0    5 = [%0.0s]     \n", ptr->CheGyeolDate);
    printf( "기준통화 코드                 GiJunTongHwaCode       3    5 = [%3.3s]     \n", ptr->GiJunTongHwaCode);
    printf( "상대통화 코드                 SangDaeTongHwaCode     3    8 = [%3.3s]     \n", ptr->SangDaeTongHwaCode);
    printf( "상품 코드                     SangPumCode            1   11 = [%1.1s]     \n", ptr->SangPumCode);
    printf( "기일 코드                     GiilCode               2   12 = [%2.2s]     \n", ptr->GiilCode);
    printf( "체결 시각                     CheGyeolTime           6   14 = [%6.6s]     \n", ptr->CheGyeolTime);
    printf( "매입 업체 코드                MaeIpEopCheCode        4   20 = [%4.4s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        4   24 = [%4.4s]     \n", ptr->MaeDoEopCheCode);
    printf( "매매율                        MaeMaeYul             12   28 = [%12.12s]   \n", ptr->MaeMaeYul);
    printf( "체결 금액                     CheGyeolGeumAek       16   40 = [%16.16s]   \n", ptr->CheGyeolGeumAek);
    printf( "현물 환율                     HyeonMulHwanYul       12   56 = [%12.12s]   \n", ptr->HyeonMulHwanYul);
    printf( "결제 일자                     GyeolJeDate            0   68 = [%0.0s]     \n", ptr->GyeolJeDate);
    printf( "만기 일자                     ManGiDate              0   68 = [%0.0s]     \n", ptr->ManGiDate);
    printf( "장내/장외 구분                JangNaeOeFlag          1   68 = [%1.1s]     \n", ptr->JangNaeOeFlag);
    printf( "Near value date               NearValueDate          0   69 = [%0.0s]     \n", ptr->NearValueDate);
    printf( "Far value date                FarValueDate           0   69 = [%0.0s]     \n", ptr->FarValueDate);
    printf( "%s", "----------------------------------------------------------------[ FX0073_DATA_FORM ]----\n");

    return sizeof( FX0073_DATA_FORM);
}

int FX0074_DATA_FORM_Print( FX0074_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX0074_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "상품 코드                     SangPumCode            2    0 = [%2.2s]     \n", ptr->SangPumCode);
    printf( "체결번호                      CheGyeolNo             5    2 = [%5.5s]     \n", ptr->CheGyeolNo);
    printf( "매입 업체 코드                MaeIpEopCheCode        4    7 = [%4.4s]     \n", ptr->MaeIpEopCheCode);
    printf( "매도 업체 코드                MaeDoEopCheCode        4   11 = [%4.4s]     \n", ptr->MaeDoEopCheCode);
    printf( "금액                          GeumAek               16   15 = [%16.16s]   \n", ptr->GeumAek);
    printf( "기준통화 코드                 GiJunTongHwaCode       3   31 = [%3.3s]     \n", ptr->GiJunTongHwaCode);
    printf( "상대통화 코드                 SangDaeTongHwaCode     3   34 = [%3.3s]     \n", ptr->SangDaeTongHwaCode);
    printf( "체결 시각                     CheGyeolTime           6   37 = [%6.6s]     \n", ptr->CheGyeolTime);
    printf( "장내/장외 구분                JangNaeOeFlag          1   43 = [%1.1s]     \n", ptr->JangNaeOeFlag);
    printf( "결제일                        GyeolJeDate            0   44 = [%0.0s]     \n", ptr->GyeolJeDate);
    printf( "시작일                        SiJakDate              0   44 = [%0.0s]     \n", ptr->SiJakDate);
    printf( "종료일                        JongRyoDate            0   44 = [%0.0s]     \n", ptr->JongRyoDate);
    printf( "행사만기일                    HaengSaManGiDate       0   44 = [%0.0s]     \n", ptr->HaengSaManGiDate);
    printf( "기일 코드                     GiilCode               2   44 = [%2.2s]     \n", ptr->GiilCode);
    printf( "거래 환율                     GeoRaeHwanYul         12   46 = [%12.12s]   \n", ptr->GeoRaeHwanYul);
    printf( "근일물 거래환율               NearRate              12   58 = [%12.12s]   \n", ptr->NearRate);
    printf( "원일물 거래환율               FarRate               12   70 = [%12.12s]   \n", ptr->FarRate);
    printf( "시장평균환율 대비 가감율(단�  MarginWon             12   82 = [%12.12s]   \n", ptr->MarginWon);
    printf( "고정금리                      GoJeongGeumLi         12   94 = [%12.12s]   \n", ptr->GoJeongGeumLi);
    printf( "중개사 거래 변동금리 코드     ByeondongGeumLiCode    2  106 = [%2.2s]     \n", ptr->ByeondongGeumLiCode);
    printf( "변동금리                      ByeondongGeumLi       12  108 = [%12.12s]   \n", ptr->ByeondongGeumLi);
    printf( "Put/Call                      PutCall                1  120 = [%1.1s]     \n", ptr->PutCall);
    printf( "Type                          Type                   1  121 = [%1.1s]     \n", ptr->Type);
    printf( "옵션가격(달러화)              OptionGaGyeokUS       16  122 = [%16.16s]   \n", ptr->OptionGaGyeokUS);
    printf( "%s", "----------------------------------------------------------------[ FX0074_DATA_FORM ]----\n");

    return sizeof( FX0074_DATA_FORM);
}

int FX1000_DATA_FORM_Print( FX1000_DATA_FORM* ptr)
{
    printf( "%s", "----[ FX1000_DATA_FORM ]----------------------------------------------------------------\n");
    printf( "보고기관코드                  BoGoGiGwanCode         4    0 = [%4.4s]     \n", ptr->BoGoGiGwanCode);
    printf( "취급점포코드                  ChwiGeupJeomPoCode     4    4 = [%4.4s]     \n", ptr->ChwiGeupJeomPoCode);
    printf( "전송일자(오늘자)              JeonSongDate           8    8 = [%8.8s]     \n", ptr->JeonSongDate);
    printf( "전송횟수                      JeonSongCount          2   16 = [%2.2s]     \n", ptr->JeonSongCount);
    printf( "전송구분                      JeonSongType           1   18 = [%1.1s]     \n", ptr->JeonSongType);
    printf( "처리번호                      CheoRiNo               6   19 = [%6.6s]     \n", ptr->CheoRiNo);
    printf( "                              FileCode               6   25 = [%6.6s]     \n", ptr->FileCode);
    printf( "보고점포그룹구분              BoGoJeomPoGroup        1   31 = [%1.1s]     \n", ptr->BoGoJeomPoGroup);
    printf( "작성기준일자                  JakSeongGiJunDate      8   32 = [%8.8s]     \n", ptr->JakSeongGiJunDate);
    printf( "                              Report                 1   40 = [%1.1s]     \n", ptr->Report);
    printf( "%s", "----------------------------------------------------------------[ FX1000_DATA_FORM ]----\n");

    return sizeof( FX1000_DATA_FORM);
}

int FILE_HEADER_FORM_Print( FILE_HEADER_FORM* ptr)
{
    printf( "%s", "----[ FILE_HEADER_FORM ]----------------------------------------------------------------\n");
    printf( "보고기관코드                  BoGoGiGwanCode         4    0 = [%4.4s]     \n", ptr->BoGoGiGwanCode);
    printf( "취급점포코드                  ChwiGeupJeomPoCode     4    4 = [%4.4s]     \n", ptr->ChwiGeupJeomPoCode);
    printf( "취급점포소재지                ChwiGeupJeomPoPost     6    8 = [%6.6s]     \n", ptr->ChwiGeupJeomPoPost);
    printf( "보고점포그룹구분              BoGoJeomPoGroup        1   14 = [%1.1s]     \n", ptr->BoGoJeomPoGroup);
    printf( "전송일자(오늘자)              JeonSongDate           8   15 = [%8.8s]     \n", ptr->JeonSongDate);
    printf( "작성기준일자                  JakSeongGiJunDate      8   23 = [%8.8s]     \n", ptr->JakSeongGiJunDate);
    printf( "전송횟수                      JeonSongCount          2   31 = [%2.2s]     \n", ptr->JeonSongCount);
    printf( "전송구분                      JeonSongType           1   33 = [%1.1s]     \n", ptr->JeonSongType);
    printf( "처리번호                      CheoRiNo               6   34 = [%6.6s]     \n", ptr->CheoRiNo);
    printf( "%s", "----------------------------------------------------------------[ FILE_HEADER_FORM ]----\n");

    return sizeof( FILE_HEADER_FORM);
}

int FX0067_FILE_FORM_Print( FX0067_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0067_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0067_FILE_FORM ]----\n");

    return sizeof( FX0067_FILE_FORM);
}

int FX0068_FILE_FORM_Print( FX0068_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0068_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0068_FILE_FORM ]----\n");

    return sizeof( FX0068_FILE_FORM);
}

int FX0069_FILE_FORM_Print( FX0069_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0069_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0069_FILE_FORM ]----\n");

    return sizeof( FX0069_FILE_FORM);
}

int FX0070_FILE_FORM_Print( FX0070_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0070_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0070_FILE_FORM ]----\n");

    return sizeof( FX0070_FILE_FORM);
}

int FX0071_FILE_FORM_Print( FX0071_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0071_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0071_FILE_FORM ]----\n");

    return sizeof( FX0071_FILE_FORM);
}

int FX0072_FILE_FORM_Print( FX0072_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0072_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0072_FILE_FORM ]----\n");

    return sizeof( FX0072_FILE_FORM);
}

int FX0073_FILE_FORM_Print( FX0073_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0073_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0073_FILE_FORM ]----\n");

    return sizeof( FX0073_FILE_FORM);
}

int FX0074_FILE_FORM_Print( FX0074_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX0074_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX0074_FILE_FORM ]----\n");

    return sizeof( FX0074_FILE_FORM);
}

int FX1000_FILE_FORM_Print( FX1000_FILE_FORM* ptr)
{
    printf( "%s", "----[ FX1000_FILE_FORM ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ FX1000_FILE_FORM ]----\n");

    return sizeof( FX1000_FILE_FORM);
}

int NET_COMMON_FORM_Print( NET_COMMON_FORM* ptr)
{
    printf( "%s", "----[ NET_COMMON_FORM ]-----------------------------------------------------------------\n");
    printf( "전문길이정보                  TelgLength             4    0 = [%4.4s]     \n", ptr->TelgLength);
    printf( "'FOREXFTP2'                   Transaction            9    4 = [%9.9s]     \n", ptr->Transaction);
    printf( "'FXB'                         SystemName             3   13 = [%3.3s]     \n", ptr->SystemName);
    printf( "'7103'                        CompanyCode            4   16 = [%4.4s]     \n", ptr->CompanyCode);
    printf( "                              MessageCode            8   20 = [%8.8s]     \n", ptr->MessageCode);
    printf( "'  '(space)                   Filler                 2   28 = [%2.2s]     \n", ptr->Filler);
    printf( "                              ContinueYN             1   30 = [%1.1s]     \n", ptr->ContinueYN);
    printf( "'000'                         ResponseCode           3   31 = [%3.3s]     \n", ptr->ResponseCode);
    printf( "%s", "-----------------------------------------------------------------[ NET_COMMON_FORM ]----\n");

    return sizeof( NET_COMMON_FORM);
}

int NET_WORK_OPEN_FORM_Print( NET_WORK_OPEN_FORM* ptr)
{
    printf( "%s", "----[ NET_WORK_OPEN_FORM ]--------------------------------------------------------------\n");
    printf( "                              CurrentDateTime       14    0 = [%14.14s]   \n", ptr->CurrentDateTime);
    printf( "'7103FTP0000000000001'        SendID                20   14 = [%20.20s]   \n", ptr->SendID);
    printf( "'7103FTP@'                    SendPW                16   34 = [%16.16s]   \n", ptr->SendPW);
    printf( "                              ConfirmGankyuk         4   50 = [%4.4s]     \n", ptr->ConfirmGankyuk);
    printf( "'B'                           SendMode               1   54 = [%1.1s]     \n", ptr->SendMode);
    printf( "'0'(사용안함)                 CompressFg             1   55 = [%1.1s]     \n", ptr->CompressFg);
    printf( "이어받기 사용여부             InheritYN              1   56 = [%1.1s]     \n", ptr->InheritYN);
    printf( "%s", "--------------------------------------------------------------[ NET_WORK_OPEN_FORM ]----\n");

    return sizeof( NET_WORK_OPEN_FORM);
}

int NET_WORK_CLOSE_FORM_Print( NET_WORK_CLOSE_FORM* ptr)
{
    printf( "%s", "----[ NET_WORK_CLOSE_FORM ]-------------------------------------------------------------\n");
    printf( "                              CloseDateTime         14    0 = [%14.14s]   \n", ptr->CloseDateTime);
    printf( "%s", "-------------------------------------------------------------[ NET_WORK_CLOSE_FORM ]----\n");

    return sizeof( NET_WORK_CLOSE_FORM);
}

int NET_FILE_SEND_REQ_FORM_Print( NET_FILE_SEND_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_SEND_REQ_FORM ]----------------------------------------------------------\n");
    printf( "                              SendReqDate            8    0 = [%8.8s]     \n", ptr->SendReqDate);
    printf( "                              FileCode               6    8 = [%6.6s]     \n", ptr->FileCode);
    printf( "                              DataType               2   14 = [%2.2s]     \n", ptr->DataType);
    printf( "%s", "----------------------------------------------------------[ NET_FILE_SEND_REQ_FORM ]----\n");

    return sizeof( NET_FILE_SEND_REQ_FORM);
}

int NET_FILE_SEND_RES_FORM_Print( NET_FILE_SEND_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_SEND_RES_FORM ]----------------------------------------------------------\n");
    printf( "                              SendReqDate            8    0 = [%8.8s]     \n", ptr->SendReqDate);
    printf( "                              Result                 1    8 = [%1.1s]     \n", ptr->Result);
    printf( "                              DataCnt                3    9 = [%3.3s]     \n", ptr->DataCnt);
    printf( "%s", "----------------------------------------------------------[ NET_FILE_SEND_RES_FORM ]----\n");

    return sizeof( NET_FILE_SEND_RES_FORM);
}

int NET_FILE_BEGIN_REQ_FORM_Print( NET_FILE_BEGIN_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_BEGIN_REQ_FORM ]---------------------------------------------------------\n");
    printf( "                              FileCode               6    0 = [%6.6s]     \n", ptr->FileCode);
    printf( "                              DataType               2    6 = [%2.2s]     \n", ptr->DataType);
    printf( "                              RecordLength           4    8 = [%4.4s]     \n", ptr->RecordLength);
    printf( "                              CompressFg             1   12 = [%1.1s]     \n", ptr->CompressFg);
    printf( "                              TotalLength           10   13 = [%10.10s]   \n", ptr->TotalLength);
    printf( "%s", "---------------------------------------------------------[ NET_FILE_BEGIN_REQ_FORM ]----\n");

    return sizeof( NET_FILE_BEGIN_REQ_FORM);
}

int NET_FILE_BEGIN_RES_FORM_Print( NET_FILE_BEGIN_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_BEGIN_RES_FORM ]---------------------------------------------------------\n");
    printf( "                              FileCode               6    0 = [%6.6s]     \n", ptr->FileCode);
    printf( "                              DataType               2    6 = [%2.2s]     \n", ptr->DataType);
    printf( "                              RecordLength           4    8 = [%4.4s]     \n", ptr->RecordLength);
    printf( "                              CompressFg             1   12 = [%1.1s]     \n", ptr->CompressFg);
    printf( "                              TotalLength           10   13 = [%10.10s]   \n", ptr->TotalLength);
    printf( "                              InheritYN              1   23 = [%1.1s]     \n", ptr->InheritYN);
    printf( "                              RecvLength            10   24 = [%10.10s]   \n", ptr->RecvLength);
    printf( "%s", "---------------------------------------------------------[ NET_FILE_BEGIN_RES_FORM ]----\n");

    return sizeof( NET_FILE_BEGIN_RES_FORM);
}

int NET_FILE_DATA_SEND_FORM_Print( NET_FILE_DATA_SEND_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_DATA_SEND_FORM ]---------------------------------------------------------\n");
    printf( "                              Sequence               7    0 = [%7.7s]     \n", ptr->Sequence);
    printf( "                              SentLength            10    7 = [%10.10s]   \n", ptr->SentLength);
    printf( "                              DataLength             4   17 = [%4.4s]     \n", ptr->DataLength);
    printf( "%s", "---------------------------------------------------------[ NET_FILE_DATA_SEND_FORM ]----\n");

    return sizeof( NET_FILE_DATA_SEND_FORM);
}

int NET_FILE_DATA_FORM_Print( NET_FILE_DATA_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_DATA_FORM ]--------------------------------------------------------------\n");
    printf( "                              FileCode               6    0 = [%6.6s]     \n", ptr->FileCode);
    printf( "                              DataType               2    6 = [%2.2s]     \n", ptr->DataType);
    printf( "                              SequenceNo             7    8 = [%7.7s]     \n", ptr->SequenceNo);
    printf( "%s", "--------------------------------------------------------------[ NET_FILE_DATA_FORM ]----\n");

    return sizeof( NET_FILE_DATA_FORM);
}

int NET_LOST_REQ_FORM_Print( NET_LOST_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_LOST_REQ_FORM ]---------------------------------------------------------------\n");
    printf( "                              LastSequence           7    0 = [%7.7s]     \n", ptr->LastSequence);
    printf( "                              TotSentLength         10    7 = [%10.10s]   \n", ptr->TotSentLength);
    printf( "%s", "---------------------------------------------------------------[ NET_LOST_REQ_FORM ]----\n");

    return sizeof( NET_LOST_REQ_FORM);
}

int NET_LOST_RES_FORM_Print( NET_LOST_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_LOST_RES_FORM ]---------------------------------------------------------------\n");
    printf( "                              Result                 2    0 = [%2.2s]     \n", ptr->Result);
    printf( "                              LastSequenceNo         7    2 = [%7.7s]     \n", ptr->LastSequenceNo);
    printf( "                              LastRecvLength        10    9 = [%10.10s]   \n", ptr->LastRecvLength);
    printf( "%s", "---------------------------------------------------------------[ NET_LOST_RES_FORM ]----\n");

    return sizeof( NET_LOST_RES_FORM);
}

int NET_FILE_CLOSE_REQ_FORM_Print( NET_FILE_CLOSE_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_CLOSE_REQ_FORM ]---------------------------------------------------------\n");
    printf( "                              LastSequence           7    0 = [%7.7s]     \n", ptr->LastSequence);
    printf( "                              TotSentLength         10    7 = [%10.10s]   \n", ptr->TotSentLength);
    printf( "                              CloseDateTime         14   17 = [%14.14s]   \n", ptr->CloseDateTime);
    printf( "%s", "---------------------------------------------------------[ NET_FILE_CLOSE_REQ_FORM ]----\n");

    return sizeof( NET_FILE_CLOSE_REQ_FORM);
}

int NET_FILE_CLOSE_RES_FORM_Print( NET_FILE_CLOSE_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_FILE_CLOSE_RES_FORM ]---------------------------------------------------------\n");
    printf( "                              Result                 2    0 = [%2.2s]     \n", ptr->Result);
    printf( "                              LastSequenceNo         7    2 = [%7.7s]     \n", ptr->LastSequenceNo);
    printf( "                              LastRecvLength        10    9 = [%10.10s]   \n", ptr->LastRecvLength);
    printf( "                              CloseDateTime         14   19 = [%14.14s]   \n", ptr->CloseDateTime);
    printf( "                              ServerControlNo       33   33 = [%33.33s]   \n", ptr->ServerControlNo);
    printf( "%s", "---------------------------------------------------------[ NET_FILE_CLOSE_RES_FORM ]----\n");

    return sizeof( NET_FILE_CLOSE_RES_FORM);
}

int NET_PW_CHG_REQ_FORM_Print( NET_PW_CHG_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_PW_CHG_REQ_FORM ]-------------------------------------------------------------\n");
    printf( "'7103FTP0000000000001'        SendID                20    0 = [%20.20s]   \n", ptr->SendID);
    printf( "'7103FTP@'                    SendPW_OLD            16   20 = [%16.16s]   \n", ptr->SendPW_OLD);
    printf( "'7103FTP@'                    SendPW_NEW            16   36 = [%16.16s]   \n", ptr->SendPW_NEW);
    printf( "%s", "-------------------------------------------------------------[ NET_PW_CHG_REQ_FORM ]----\n");

    return sizeof( NET_PW_CHG_REQ_FORM);
}

int NET_PW_CHG_RES_FORM_Print( NET_PW_CHG_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_PW_CHG_RES_FORM ]-------------------------------------------------------------\n");
    printf( "                              Result                 2    0 = [%2.2s]     \n", ptr->Result);
    printf( "%s", "-------------------------------------------------------------[ NET_PW_CHG_RES_FORM ]----\n");

    return sizeof( NET_PW_CHG_RES_FORM);
}

int NET_STATUS_REQ_FORM_Print( NET_STATUS_REQ_FORM* ptr)
{
    printf( "%s", "----[ NET_STATUS_REQ_FORM ]-------------------------------------------------------------\n");
    printf( "                              CurrentDateTime       14    0 = [%14.14s]   \n", ptr->CurrentDateTime);
    printf( "%s", "-------------------------------------------------------------[ NET_STATUS_REQ_FORM ]----\n");

    return sizeof( NET_STATUS_REQ_FORM);
}

int NET_STATUS_RES_FORM_Print( NET_STATUS_RES_FORM* ptr)
{
    printf( "%s", "----[ NET_STATUS_RES_FORM ]-------------------------------------------------------------\n");
    printf( "                              CurrentDateTime       14    0 = [%14.14s]   \n", ptr->CurrentDateTime);
    printf( "%s", "-------------------------------------------------------------[ NET_STATUS_RES_FORM ]----\n");

    return sizeof( NET_STATUS_RES_FORM);
}

int NET_DATA_FX0067_Print( NET_DATA_FX0067* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0067 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0067 ]----\n");

    return sizeof( NET_DATA_FX0067);
}

int NET_DATA_FX0068_Print( NET_DATA_FX0068* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0068 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0068 ]----\n");

    return sizeof( NET_DATA_FX0068);
}

int NET_DATA_FX0069_Print( NET_DATA_FX0069* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0069 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0069 ]----\n");

    return sizeof( NET_DATA_FX0069);
}

int NET_DATA_FX0070_Print( NET_DATA_FX0070* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0070 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0070 ]----\n");

    return sizeof( NET_DATA_FX0070);
}

int NET_DATA_FX0071_Print( NET_DATA_FX0071* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0071 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0071 ]----\n");

    return sizeof( NET_DATA_FX0071);
}

int NET_DATA_FX0072_Print( NET_DATA_FX0072* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0072 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0072 ]----\n");

    return sizeof( NET_DATA_FX0072);
}

int NET_DATA_FX0073_Print( NET_DATA_FX0073* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0073 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0073 ]----\n");

    return sizeof( NET_DATA_FX0073);
}

int NET_DATA_FX0074_Print( NET_DATA_FX0074* ptr)
{
    printf( "%s", "----[ NET_DATA_FX0074 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX0074 ]----\n");

    return sizeof( NET_DATA_FX0074);
}

int NET_DATA_FX1000_Print( NET_DATA_FX1000* ptr)
{
    printf( "%s", "----[ NET_DATA_FX1000 ]-----------------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------------[ NET_DATA_FX1000 ]----\n");

    return sizeof( NET_DATA_FX1000);
}

int PACKET_WORK_OPEN_Print( PACKET_WORK_OPEN* ptr)
{
    printf( "%s", "----[ PACKET_WORK_OPEN ]----------------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------------[ PACKET_WORK_OPEN ]----\n");

    return sizeof( PACKET_WORK_OPEN);
}

int PACKET_WORK_CLOSE_Print( PACKET_WORK_CLOSE* ptr)
{
    printf( "%s", "----[ PACKET_WORK_CLOSE ]---------------------------------------------------------------\n");
    printf( "%s", "---------------------------------------------------------------[ PACKET_WORK_CLOSE ]----\n");

    return sizeof( PACKET_WORK_CLOSE);
}

int PACKET_FILE_SEND_REQUEST_Print( PACKET_FILE_SEND_REQUEST* ptr)
{
    printf( "%s", "----[ PACKET_FILE_SEND_REQUEST ]--------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------[ PACKET_FILE_SEND_REQUEST ]----\n");

    return sizeof( PACKET_FILE_SEND_REQUEST);
}

int PACKET_FILE_SEND_RESPONSE_Print( PACKET_FILE_SEND_RESPONSE* ptr)
{
    printf( "%s", "----[ PACKET_FILE_SEND_RESPONSE ]-------------------------------------------------------\n");
    printf( "%s", "-------------------------------------------------------[ PACKET_FILE_SEND_RESPONSE ]----\n");

    return sizeof( PACKET_FILE_SEND_RESPONSE);
}

int PACKET_FILE_BEGIN_REQUEST_Print( PACKET_FILE_BEGIN_REQUEST* ptr)
{
    printf( "%s", "----[ PACKET_FILE_BEGIN_REQUEST ]-------------------------------------------------------\n");
    printf( "%s", "-------------------------------------------------------[ PACKET_FILE_BEGIN_REQUEST ]----\n");

    return sizeof( PACKET_FILE_BEGIN_REQUEST);
}

int PACKET_FILE_BEGIN_RESPONSE_Print( PACKET_FILE_BEGIN_RESPONSE* ptr)
{
    printf( "%s", "----[ PACKET_FILE_BEGIN_RESPONSE ]------------------------------------------------------\n");
    printf( "%s", "------------------------------------------------------[ PACKET_FILE_BEGIN_RESPONSE ]----\n");

    return sizeof( PACKET_FILE_BEGIN_RESPONSE);
}

int PACKET_FILE_CLOSE_REQ_Print( PACKET_FILE_CLOSE_REQ* ptr)
{
    printf( "%s", "----[ PACKET_FILE_CLOSE_REQ ]-----------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------[ PACKET_FILE_CLOSE_REQ ]----\n");

    return sizeof( PACKET_FILE_CLOSE_REQ);
}

int PACKET_FILE_CLOSE_RES_Print( PACKET_FILE_CLOSE_RES* ptr)
{
    printf( "%s", "----[ PACKET_FILE_CLOSE_RES ]-----------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------[ PACKET_FILE_CLOSE_RES ]----\n");

    return sizeof( PACKET_FILE_CLOSE_RES);
}

int PACKET_LOST_REQUEST_Print( PACKET_LOST_REQUEST* ptr)
{
    printf( "%s", "----[ PACKET_LOST_REQUEST ]-------------------------------------------------------------\n");
    printf( "%s", "-------------------------------------------------------------[ PACKET_LOST_REQUEST ]----\n");

    return sizeof( PACKET_LOST_REQUEST);
}

int PACKET_LOST_RESPONSE_Print( PACKET_LOST_RESPONSE* ptr)
{
    printf( "%s", "----[ PACKET_LOST_RESPONSE ]------------------------------------------------------------\n");
    printf( "%s", "------------------------------------------------------------[ PACKET_LOST_RESPONSE ]----\n");

    return sizeof( PACKET_LOST_RESPONSE);
}

int PACKET_PW_CHG_REQUEST_Print( PACKET_PW_CHG_REQUEST* ptr)
{
    printf( "%s", "----[ PACKET_PW_CHG_REQUEST ]-----------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------[ PACKET_PW_CHG_REQUEST ]----\n");

    return sizeof( PACKET_PW_CHG_REQUEST);
}

int PACKET_PW_CHG_RESPONSE_Print( PACKET_PW_CHG_RESPONSE* ptr)
{
    printf( "%s", "----[ PACKET_PW_CHG_RESPONSE ]----------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------[ PACKET_PW_CHG_RESPONSE ]----\n");

    return sizeof( PACKET_PW_CHG_RESPONSE);
}

int PACKET_STATUS_REQUEST_Print( PACKET_STATUS_REQUEST* ptr)
{
    printf( "%s", "----[ PACKET_STATUS_REQUEST ]-----------------------------------------------------------\n");
    printf( "%s", "-----------------------------------------------------------[ PACKET_STATUS_REQUEST ]----\n");

    return sizeof( PACKET_STATUS_REQUEST);
}

int PACKET_STATUS_RESPONSE_Print( PACKET_STATUS_RESPONSE* ptr)
{
    printf( "%s", "----[ PACKET_STATUS_RESPONSE ]----------------------------------------------------------\n");
    printf( "%s", "----------------------------------------------------------[ PACKET_STATUS_RESPONSE ]----\n");

    return sizeof( PACKET_STATUS_RESPONSE);
}

int PACKET_FILE_FX0067_Print( PACKET_FILE_FX0067* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0067 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0067 ]----\n");

    return sizeof( PACKET_FILE_FX0067);
}

int PACKET_FILE_FX0068_Print( PACKET_FILE_FX0068* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0068 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0068 ]----\n");

    return sizeof( PACKET_FILE_FX0068);
}

int PACKET_FILE_FX0069_Print( PACKET_FILE_FX0069* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0069 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0069 ]----\n");

    return sizeof( PACKET_FILE_FX0069);
}

int PACKET_FILE_FX0070_Print( PACKET_FILE_FX0070* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0070 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0070 ]----\n");

    return sizeof( PACKET_FILE_FX0070);
}

int PACKET_FILE_FX0071_Print( PACKET_FILE_FX0071* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0071 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0071 ]----\n");

    return sizeof( PACKET_FILE_FX0071);
}

int PACKET_FILE_FX0072_Print( PACKET_FILE_FX0072* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0072 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0072 ]----\n");

    return sizeof( PACKET_FILE_FX0072);
}

int PACKET_FILE_FX0073_Print( PACKET_FILE_FX0073* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0073 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0073 ]----\n");

    return sizeof( PACKET_FILE_FX0073);
}

int PACKET_FILE_FX0074_Print( PACKET_FILE_FX0074* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX0074 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX0074 ]----\n");

    return sizeof( PACKET_FILE_FX0074);
}

int PACKET_FILE_FX1000_Print( PACKET_FILE_FX1000* ptr)
{
    printf( "%s", "----[ PACKET_FILE_FX1000 ]--------------------------------------------------------------\n");
    printf( "%s", "--------------------------------------------------------------[ PACKET_FILE_FX1000 ]----\n");

    return sizeof( PACKET_FILE_FX1000);
}

int PACKET_MESSAGE_FORM_Print( PACKET_MESSAGE_FORM* ptr)
{
    printf( "%s", "----[ PACKET_MESSAGE_FORM ]-------------------------------------------------------------\n");
    printf( "                              Data                 4045    0 = [%4045.4045s]\n", ptr->Data);
    printf( "%s", "-------------------------------------------------------------[ PACKET_MESSAGE_FORM ]----\n");

    return sizeof( PACKET_MESSAGE_FORM);
}

int RESULT_FORM_Print( RESULT_FORM* ptr)
{
    printf( "%s", "----[ RESULT_FORM ]---------------------------------------------------------------------\n");
    printf( "                              Result                 8    0 = [%8.8s]     \n", ptr->Result);
    printf( "                              FileCode               9    8 = [%9.9s]     \n", ptr->FileCode);
    printf( "                              Message              100   17 = [%100.100s] \n", ptr->Message);
    printf( "%s", "---------------------------------------------------------------------[ RESULT_FORM ]----\n");

    return sizeof( RESULT_FORM);
}

int RECORD_COUNT_FORM_Print( RECORD_COUNT_FORM* ptr)
{
    printf( "%s", "----[ RECORD_COUNT_FORM ]---------------------------------------------------------------\n");
    printf( "%s", "---------------------------------------------------------------[ RECORD_COUNT_FORM ]----\n");

    return sizeof( RECORD_COUNT_FORM);
}

