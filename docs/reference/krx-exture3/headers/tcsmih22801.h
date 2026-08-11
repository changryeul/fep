#ifndef _KRX_TCSMIH22801_H
#define _KRX_TCSMIH22801_H

/* TCSMIH22801 공동기금과부족금액통보(주식·파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Joint_Fund_Market_Identification[3];   /*    4 공동기금시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Calc_Date[8];                          /*    6 산출일 (String 8) */
    char Pay_Date[8];                           /*    7 납부일 (String 8) */
    char Regul_Tmly_Type_Code[1];               /*    8 정기수시구분코드 (String 1) */
    char Joint_Fund_Required_Value[22];         /*    9 공동기금필요액 (Float 22) */
    char Joint_Fund_Deposit_Amount[22];         /*   10 공동기금예탁금액 (Float 22) */
    char Ovres_Shotrs_Type_Code[1];             /*   11 과부족구분코드 (String 1) */
    char Joint_Fund_Overs_Shorts_Amount[22];    /*   12 공동기금과부족금액 (Float 22) */
    char Filler_Value[1078];                    /*   13 필러값 (String 1078) */
} TCSMIH22801_DATA;

#endif  /* _KRX_TCSMIH22801_H */
