#ifndef _KRX_TCSMIH22301_H
#define _KRX_TCSMIH22301_H

/* TCSMIH22301 위탁자기별거래증거금소요액(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    7 거래전문회원번호 (String 5) */
    char Trust_Principal_Intergration_Type_Code[2];/*    8 위탁자기통합구분코드 (String 2) */
    char Credit_Risk_Margin_Required_Value[22]; /*    9 신용위험증거금소요액 (Float 22) */
    char Trading_Margin_Required_Value[22];     /*   10 거래증거금소요액 (Float 22) */
    char Cashable_Asset_Pay_Required_Value[22]; /*   11 현금성자산납부소요액 (Float 22) */
    char Filler_Value[1090];                    /*   12 필러값 (String 1090) */
} TCSMIH22301_DATA;

#endif  /* _KRX_TCSMIH22301_H */
