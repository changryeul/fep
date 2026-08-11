#ifndef _KRX_TCSMIH27401_H
#define _KRX_TCSMIH27401_H

/* TCSMIH27401 Buy-in회원별결제대금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Buyin_Type_Complt_Yn[1];               /*    5 Buy-in구분완료여부 (String 1) */
    char Clearing_Member_Type[1];               /*    6 결제회원구분 (String 1) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Trading_Date[8];                       /*    8 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    9 결제일자 (String 8) */
    char Buyin_Type_Code[1];                    /*   10 Buy-in 구분코드 (String 1) */
    char Deduction_Ask_Amount[22];              /*   11 차감매도대금 (Long 22) */
    char Deduction_Bid_Amount[22];              /*   12 차감매수대금 (Long 22) */
    char Field[1101];                           /*   13 FILLER (String 1101) */
} TCSMIH27401_DATA;

#endif  /* _KRX_TCSMIH27401_H */
