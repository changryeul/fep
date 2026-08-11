#ifndef _KRX_TCSMIP46201_H
#define _KRX_TCSMIP46201_H

/* TCSMIP46201 비과세거래내역(코스닥포함) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 107 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    6 결제일자 (String 8) */
    char Securities_Company_Number[5];          /*    7 증권회사번호 (String 5) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Unit_Price[10];                        /*    9 단가 (Long 10) */
    char Quantity[15];                          /*   10 수량 (Long 15) */
    char Zero_Tax_Rate_Type_Code[1];            /*   11 세율구분코드 (String 1) */
    char Member_Number[5];                      /*   12 회원번호 (String 5) */
    char Stock_Issue_Price[10];                 /*   13 주식발행가격 (Long 10) */
} TCSMIP46201_DATA;

#endif  /* _KRX_TCSMIP46201_H */
