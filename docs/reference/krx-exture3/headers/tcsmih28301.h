#ifndef _KRX_TCSMIH28301_H
#define _KRX_TCSMIH28301_H

/* TCSMIH28301 일반-소액채권시장차감거래대금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 매매일 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Curr_Tp_Cd[1];                         /*    7 통화구분코드 (string 1) */
    char Trust_Ask_Amount[22];                  /*    8 위탁매도대금 (Float 22) */
    char Trust_Bid_Amount[22];                  /*    9 위탁매수대금 (Float 22) */
    char Principal_Ask_Amount[22];              /*   10 자기매도대금 (Float 22) */
    char Principal_Bid_Amount[22];              /*   11 자기매수대금 (Float 22) */
    char Trustbusiness_Ask_Amount[22];          /*   12 신탁매도대금 (Float 22) */
    char Trustbusiness_Bid_Amount[22];          /*   13 신탁매수대금 (Float 22) */
    char Total_Deduction_Ask_Amount[22];        /*   14 총차감매도대금 (Float 22) */
    char Total_Deduction_Bid_Amount[22];        /*   15 총차감매수대금 (Float 22) */
    char Filler_Value[979];                     /*   16 필러값 (String 979) */
} TCSMIH28301_DATA;

#endif  /* _KRX_TCSMIH28301_H */
