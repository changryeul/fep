#ifndef _KRX_TCSMIH22601_H
#define _KRX_TCSMIH22601_H

/* TCSMIH22601 시장조성자헤지한도계산용거래량가중평균델타 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Futures_And_Option_Type_Code[1];       /*    6 선물옵션구분코드 (String 1) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Bid_Dleta[20];                         /*    8 매수델타 (Float 20) */
    char Ask_Dleta[20];                         /*    9 매도델타 (Float 20) */
    char Filler_Value[1108];                    /*   10 필러값 (String 1108) */
} TCSMIH22601_DATA;

#endif  /* _KRX_TCSMIH22601_H */
