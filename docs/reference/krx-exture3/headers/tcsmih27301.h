#ifndef _KRX_TCSMIH27301_H
#define _KRX_TCSMIH27301_H

/* TCSMIH27301 Buy-in회원별결제수량 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Buyin_Type_Complt_Yn[1];               /*    5 Buy-in구분완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Clearing_Member_Type[1];               /*    8 결제회원구분 (String 1) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Trading_Date[8];                       /*   10 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   11 결제일자 (String 8) */
    char Buyin_Type_Code[1];                    /*   12 Buy-in 구분코드 (String 1) */
    char Issue_Code[12];                        /*   13 종목코드 (String 12) */
    char Trust_Decuction_Ask_Quantity[15];      /*   14 위탁차감매도수량 (Long 15) */
    char Trust_Decuction_Bid_Quantity[15];      /*   15 위탁차감매수수량 (Long 15) */
    char Principal_Decuction_Ask_Quantity[15];  /*   16 상품차감매도수량 (Long 15) */
    char Principal_Decuction_Bid_Quantity[15];  /*   17 상품차감매수수량 (Long 15) */
    char Total_Deduction_Ask_Quantity[15];      /*   18 총차감매도수량 (Long 15) */
    char Total_Deduction_Bid_Quantity[15];      /*   19 총차감매수수량 (Long 15) */
    char Filler_Value[1038];                    /*   20 FILLER (String 1038) */
} TCSMIH27301_DATA;

#endif  /* _KRX_TCSMIH27301_H */
