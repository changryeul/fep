#ifndef _KRX_TRDESP00601_H
#define _KRX_TRDESP00601_H

/* TRDESP00601 유동성공급자 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 201 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Liquid_Provider_Member_Float[5];       /*    8 LP회원번호 (String 5) */
    char Board_Id[2];                           /*    9 보드ID (String 2) */
    char Market_Making_Liquid_Provider_Type_Code[1];/*   10 시장조성LP구분코드 (String 1) */
    char Liquid_Provider_Start_Date[8];         /*   11 LP개시일자 (String 8) */
    char Liquid_Provider_End_Date[8];           /*   12 LP종료일자 (String 8) */
    char Min_Ord_Amt[22];                       /*   13 최소호가금액 (float 22) */
    char Lp_Minimum_Order_Quantity_Multiple[11];/*   14 LP최소호가수량배수 (Long 11) */
    char First_Best_Order_Spread_Unit_Code[1];  /*   15 최우선호가스프레드단위코드 (String 1) */
    char Order_Spread_Value[22];                /*   16 호가스프레드값 (Float 22) */
    char Non_Trading_Order_Spread_Value[22];    /*   17 휴장호가스프레드값 (Float 22) */
    char Ask_Order_Minimum_Amount[22];          /*   18 매도최소호가금액 (Float 22) */
    char Bid_Order_Minimum_Amount[22];          /*   19 매수최소호가금액 (Float 22) */
} TRDESP00601_DATA;

#endif  /* _KRX_TRDESP00601_H */
