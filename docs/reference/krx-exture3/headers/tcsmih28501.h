#ifndef _KRX_TCSMIH28501_H
#define _KRX_TCSMIH28501_H

/* TCSMIH28501 일반-소액채권시장거래량 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 매매일 (String 8) */
    char Settlement_Type[1];                    /*    6 결제구분 (String 1) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Non_Member_Number[5];                  /*    8 비회원번호 (String 5) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Trust_Ask_Quantity[15];                /*   10 위탁매도수량 (Long 15) */
    char Trust_Ask_Amount[22];                  /*   11 위탁매도대금 (Float 22) */
    char Trust_Bid_Quantity[15];                /*   12 위탁매수수량 (Long 15) */
    char Trust_Bid_Amount[22];                  /*   13 위탁매수대금 (Float 22) */
    char Principal_Ask_Quantity[15];            /*   14 자기매도수량 (Long 15) */
    char Principal_Ask_Amount[22];              /*   15 자기매도대금 (Float 22) */
    char Principal_Bid_Quantity[15];            /*   16 자기매수수량 (Long 15) */
    char Principal_Bid_Amount[22];              /*   17 자기매수대금 (Float 22) */
    char Trustbusiness_Ask_Quantity[15];        /*   18 신탁매도수량 (Long 15) */
    char Trustbusiness_Ask_Amount[22];          /*   19 신탁매도대금 (Float 22) */
    char Trustbusiness_Bid_Quantity[15];        /*   20 신탁매수수량 (Long 15) */
    char Trustbusiness_Bid_Amount[22];          /*   21 신탁매수대금 (Float 22) */
    char Filr_Val[916];                         /*   22 필러값 (String 916) */
} TCSMIH28501_DATA;

#endif  /* _KRX_TCSMIH28501_H */
