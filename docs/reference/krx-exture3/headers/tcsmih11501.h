#ifndef _KRX_TCSMIH11501_H
#define _KRX_TCSMIH11501_H

/* TCSMIH11501 주식선물미결제한도및예정내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Start_Date[8];                         /*    9 적용일 (String 8) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Calculate_Type_Code[1];                /*   11 산출유형코드 (String 1) */
    char Open_Interest_Limit_Change_Reason[2];  /*   12 미결제한도변경사유 (String 2) */
    char Base_Listing_Number_Of_Shares[15];     /*   13 기준상장주식수 (Long 15) */
    char Limit_Quantity[15];                    /*   14 미결제한도계약수 (Long 15) */
    char Filler_Value[1108];                    /*   15 필러값 (String 1108) */
} TCSMIH11501_DATA;

#endif  /* _KRX_TCSMIH11501_H */
