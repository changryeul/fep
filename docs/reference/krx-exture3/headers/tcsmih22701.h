#ifndef _KRX_TCSMIH22701_H
#define _KRX_TCSMIH22701_H

/* TCSMIH22701 시장조성자헤지한도계산용지수반영시가총액비중 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Index_Related_Underlying_Asset_Id[3];  /*    6 지수관련기초자산ID (String 3) */
    char Composition_Issue_Code[12];            /*    7 구성종목코드 (String 12) */
    char Market_Capitalization_Weight[13];      /*    8 시가총액비중 (Float 13) */
    char Filler_Value[1133];                    /*    9 필러값 (String 1133) */
} TCSMIH22701_DATA;

#endif  /* _KRX_TCSMIH22701_H */
