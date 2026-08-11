#ifndef _KRX_TCSMIH28201_H
#define _KRX_TCSMIH28201_H

/* TCSMIH28201 일반-소액채권시장차감거래량 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 매매일 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Trust_Ask_Quantity[15];                /*    8 위탁매도수량 (Long 15) */
    char Trust_Bid_Quantity[15];                /*    9 위탁매수수량 (Long 15) */
    char Principal_Ask_Quantity[15];            /*   10 자기매도수량 (Long 15) */
    char Principal_Bid_Quantity[15];            /*   11 자기매수수량 (Long 15) */
    char Trustbusiness_Ask_Quantity[15];        /*   12 신탁매도수량 (Long 15) */
    char Trustbusiness_Bid_Quantity[15];        /*   13 신탁매수수량 (Long 15) */
    char Total_Deduction_Ask_Quantity[15];      /*   14 총차감매도수량 (Long 15) */
    char Total_Deduction_Bid_Quantity[15];      /*   15 총차감매수수량 (Long 15) */
    char Filler_Value[1024];                    /*   16 필러값 (String 1024) */
} TCSMIH28201_DATA;

#endif  /* _KRX_TCSMIH28201_H */
