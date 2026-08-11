#ifndef _KRX_TCSMIH20401_H
#define _KRX_TCSMIH20401_H

/* TCSMIH20401 계좌별거래증거금소요액(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 633 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    6 거래전문회원번호 (String 5) */
    char Account_Margin_Type_Code[2];           /*    7 계좌증거금유형코드 (String 2) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Account_Type_Code[2];                  /*    9 계좌구분코드 (String 2) */
    char Discount_Margin_Application_Yes_Or_No[1];/*   10 거래증거금할인적용여부 (String 1) */
    char By_Account_No_Discount_Margin_Required_Value[23];/*   11 계좌별비할인증거금소요액 (Float 23) */
    char By_Account_Discount_Margin_Required_Value[23];/*   12 계좌별할인증거금소요액 (Float 23) */
    char Filler_Value[529];                     /*   13 필러값 (String 529) */
} TCSMIH20401_DATA;

#endif  /* _KRX_TCSMIH20401_H */
