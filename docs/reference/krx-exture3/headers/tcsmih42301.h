#ifndef _KRX_TCSMIH42301_H
#define _KRX_TCSMIH42301_H

/* TCSMIH42301 거래증거금과부족내역통보(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Before_Previous_Day_Trading_Margin_Required_Value[22];/*    6 전전일거래증거금소요액 (Float 22) */
    char Prevdd_Trading_Margin_Required_Value[22];/*    7 전일거래증거금소요액 (Float 22) */
    char Trading_Margin_Required_Value[22];     /*    8 거래증거금소요액 (Float 22) */
    char Withdrawal_Limit_Amount[22];           /*    9 인출제한금액 (Float 22) */
    char Trading_Margin_Valuation[22];          /*   10 거래증거금평가금액 (Float 22) */
    char Ovres_Shorts_Type_Code[1];             /*   11 과부족구분코드 (String 1) */
    char Trading_Margin_Valuation_Amount[22];   /*   12 거래증거금과부족금액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Required_Value[22];/*   13 현금성자산거래증거금소요액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Valuation[22];/*   14 현금성자산거래증거금평가금액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Ovres_Shorts_Type_Code[1];/*   15 현금성자산과부족구분코드 (String 1) */
    char Cashable_Asset_Trading_Margin_Valuation_Amount[22];/*   16 현금성자산거래증거금과부족금액 (Float 22) */
    char Filler_Value[964];                     /*   17 필러값 (String 964) */
} TCSMIH42301_DATA;

#endif  /* _KRX_TCSMIH42301_H */
