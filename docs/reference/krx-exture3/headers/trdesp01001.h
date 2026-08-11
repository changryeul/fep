#ifndef _KRX_TRDESP01001_H
#define _KRX_TRDESP01001_H

/* TRDESP01001 파생종목정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1293 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Underlying_Asset_Identification[3];    /*    7 기초자산ID (String 3) */
    char Underlying_Code[2];                    /*    8 기초자산코드 (String 2) */
    char Class_Id[11];                          /*    9 상품ID (String 11) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Market_Korean_Name[80];                /*   11 시장한글명 (String 80) */
    char Market_English_Name[80];               /*   12 시장영문명 (String 80) */
    char Securities_Group_Korean_Name[80];      /*   13 증권그룹한글명 (String 80) */
    char Securities_Group_English_Name[80];     /*   14 증권그룹영문명 (String 80) */
    char Underlying_Asset_Korean_Name[80];      /*   15 기초자산한글명 (String 80) */
    char Underlying_Asset_English_Name[80];     /*   16 기초자산영문명 (String 80) */
    char Issue_Shorten_Code[9];                 /*   17 종목단축코드 (String 9) */
    char Issue_Korean_Name[80];                 /*   18 종목한글명 (String 80) */
    char Issue_English_Name[80];                /*   19 종목영문명 (String 80) */
    char Issue_English_Abbreviation[40];        /*   20 종목영문약명 (String 40) */
    char Listing_Date[8];                       /*   21 상장일자 (String 8) */
    char Listing_Type_Code[1];                  /*   22 상장유형코드 (String 1) */
    char Me_Grp_No[2];                          /*   23 ME그룹번호 ★신규 (string 2) */
    char Expiration_Month[6];                   /*   24 결제월 (String 6) */
    char Last_Trading_Date[8];                  /*   25 최종거래일자 (String 8) */
    char Last_Settlement_Date[8];               /*   26 최종결제일자 (String 8) */
    char Expiration_Date[8];                    /*   27 만기일자 (String 8) */
    char Contract_Size[22];                     /*   28 거래단위 ★변경 (Float 22) */
    char Settlement_Multiplier[22];             /*   29 거래승수 (Float 22) */
    char Exercise_Price[18];                    /*   30 행사가격 (Float 18) */
    char At_The_Money_Type_Code[1];             /*   31 ATM구분코드 (String 1) */
    char For_Inquiry_Exercise_Price[18];        /*   32 조회용행사가격 (Float 18) */
    char Arrangement_Trade_Yes_Or_No[1];        /*   33 정리매매여부 (String 1) */
    char Trading_Halt_Yes_Or_No[1];             /*   34 거래정지여부 (String 1) */
    char Halt_Rsn_Cd[3];                        /*   35 거래정지사유코드 (String 3) */
    char Halt_Strt_Dd[8];                       /*   36 거래정지개시일자 (String 8) */
    char Halt_End_Dd[8];                        /*   37 거래정지종료일자 (String 8) */
    char Last_Settlement_Method_Code[1];        /*   38 최종결제방법코드 (String 1) */
    char Right_Type_Code[1];                    /*   39 권리유형코드 (String 1) */
    char Exercise_Type_Code[1];                 /*   40 권리행사유형코드 (String 1) */
    char Adjustment_Type_Code[1];               /*   41 조정구분코드 (String 1) */
    char Month_Goods_Type_Code[1];              /*   42 월물구분코드 (String 1) */
    char Spread_Base_Issue_Type_Code[1];        /*   43 스프레드기준종목구분코드 (String 1) */
    char Spd_Compst_Isu_Cd1[12];                /*   44 스프레드구성종목코드1 (String 12) */
    char Spd_Compst_Isu_Cd2[12];                /*   45 스프레드구성종목코드2 (String 12) */
    char Underlying_Asset_Issue_Code[12];       /*   46 기초자산종목코드 (String 12) */
    char Market_Operation_Group_Code[3];        /*   47 장운영상품그룹ID (String 3) */
    char Remaining_Days[8];                     /*   48 잔존일수 (Long 8) */
    char Market_Making_Type_Code[1];            /*   49 시장조성구분코드 (String 1) */
    char Last_Trading_Time[9];                  /*   50 최종체결시각 (String 9) */
    char First_Trading_Date[8];                 /*   51 최초체결일자 (String 8) */
    char Limit_Price_Order_Condition_Type_Code[5];/*   52 지정가호가취소조건코드 (Long 5) */
    char Market_Price_Order_Condition_Type_Code[5];/*   53 시장가호가취소조건코드 (Long 5) */
    char Conditional_Limit_Price_Order_Condition_Type_Code[5];/*   54 조건부지정가취소호가조건코드 (Long 5) */
    char Best_Limit_Price_Order_Condition_Type_Code[5];/*   55 최유리지정가취소호가조건코드 (Long 5) */
    char Limit_Price_Of_First_Best_Order_Order_Condition_Type_Code[5];/*   56 최우선지정가취소호가조건코드 (Long 5) */
    char Upper_Limit_Price[11];                 /*   57 상한가 (String 11) */
    char Lower_Limit_Price[11];                 /*   58 하한가 (String 11) */
    char Base_Price[11];                        /*   59 기준가격 (String 11) */
    char Base_Price_Type_Code[2];               /*   60 기준가격구분코드 (String 2) */
    char Base_Price_Application_Maximum_Theory_Price[16];/*   61 기준가격적용최대이론가격 (Float 16) */
    char Settlement_Price[18];                  /*   62 정산가격 (Float 18) */
    char Settlement_Price_Type_Code[2];         /*   63 정산가격구분코드 ★변경 (String 2) */
    char Margin_Base_Price[18];                 /*   64 증거금기준가격 (Float 18) */
    char Margin_Base_Price_Type_Code[2];        /*   65 증거금기준가격구분코드 (String 2) */
    char Closing_Price[11];                     /*   66 종가 (String 11) */
    char Closing_Price_Type_Code[1];            /*   67 종가구분코드 (String 1) */
    char Future_Circuit_Breakers_Upper_Limit_Price[11];/*   68 선물CIRCUIT_BREAKERS상한가 (String 11) */
    char Future_Circuit_Breakers_Lower_Limit_Price[11];/*   69 선물CIRCUIT_BREAKERS하한가 (String 11) */
    char I_F_Price_Sign_Use_Yes_Or_No[1];       /*   70 I/F가격부호사용여부 (String 1) */
    char I_F_Integer_Valid_Digit[3];            /*   71 I/F가격정수유효자리수 (Long 3) */
    char I_F_Fraction_Valid_Digit[3];           /*   72 I/F가격소수유효자리수 (Long 3) */
    char Liqu_Adm_Yn[1];                        /*   73 유동성관리여부 (String 1) */
    char Nontr_Yn[1];                           /*   74 휴장여부 (String 1) */
    char Nontr_Rsn_Cd[2];                       /*   75 휴장사유코드 (String 2) */
    char Dynmc_Prc_Lmt_Yn[1];                   /*   76 실시간가격제한여부 (String 1) */
    char If_Dynmc_Uplmtprc_Intval[11];          /*   77 I/F실시간상한가간격 (String 11) */
    char If_Dynmc_Lwlmtprc_Intval[11];          /*   78 I/F실시간하한가간격 (String 11) */
    char Nego_Blktrd_Bas_Isu_Cd[12];            /*   79 협의대량매매기준종목코드 (String 12) */
    char Ple_Appl_Dirct_Cd[1];                  /*   80 가격제한확대적용방향코드 (string 1) */
    char Prc_Lmt_Lst_Step[3];                   /*   81 가격제한최종단계 (long 3) */
    char If_Uplmt_Qty[16];                      /*   82 I/F상한수량 (long 16) */
    char If_Lwlmt_Qty[16];                      /*   83 I/F하한수량 (long 16) */
    char If_Nego_Blktrd_Uplmt_Qty[16];          /*   84 I/F협의대량매매상한수량 (long 16) */
    char If_Nego_Blktrd_Lwlmt_Qty[16];          /*   85 I/F협의대량매매하한수량 (long 16) */
    char Expww[2];                              /*   86 결제주 (String 2) */
    char Bas_Prod_Id[11];                       /*   87 기준상품ID (string 11) */
    char Sub_Prod_Id[11];                       /*   88 부대상품ID (string 11) */
    char Bas_Prod_Isu_Cnt[6];                   /*   89 기준상품 종목수 (long 6) */
    char Sub_Prod_Isu_Cnt[6];                   /*   90 부대상품 종목수 (long 6) */
    char Dorm_Yn[1];                            /*   91 휴면여부 (String 1) */
    char Dorm_Design_Dd[8];                     /*   92 휴면지정일자 (String 8) */
    char Prc_Unit_Rule_Id[10];                  /*   93 가격단위규칙ID (String 10) */
} TRDESP01001_DATA;

#endif  /* _KRX_TRDESP01001_H */
