#ifndef _KRX_TCSMIH28601_H
#define _KRX_TCSMIH28601_H

/* TCSMIH28601 일반-소액채권시장거래대금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
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
    char Curr_Tp_Cd[1];                         /*    9 통화구분코드 (string 1) */
    char Trust_Ask_Amount[22];                  /*   10 위탁매도대금 (Float 22) */
    char Trust_Bid_Amount[22];                  /*   11 위탁매수대금 (Float 22) */
    char Principal_Ask_Amount[22];              /*   12 자기매도대금 (Float 22) */
    char Principal_Bid_Amount[22];              /*   13 자기매수대금 (Float 22) */
    char Trustbusiness_Ask_Amount[22];          /*   14 신탁매도대금 (Float 22) */
    char Trustbusiness_Bid_Amount[22];          /*   15 신탁매수대금 (Float 22) */
    char Total_Deduction_Ask_Amount[22];        /*   16 총차감매도대금 (Float 22) */
    char Total_Deduction_Bid_Amount[22];        /*   17 총차감매수대금 (Float 22) */
    char Filr_Val[973];                         /*   18 필러값 (String 973) */
} TCSMIH28601_DATA;

#endif  /* _KRX_TCSMIH28601_H */
