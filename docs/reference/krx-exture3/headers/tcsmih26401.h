#ifndef _KRX_TCSMIH26401_H
#define _KRX_TCSMIH26401_H

/* TCSMIH26401 거래전문회원의종목거래내역(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Mkt_Id[3];                             /*    5 시장ID ★변경 (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    8 거래전문회원번호 (String 5) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Ask_Trading_Volumn[10];                /*   10 매도거래량 (Long 10) */
    char Bid_Trading_Volumn[10];                /*   11 매수거래량 (Long 10) */
    char Ask_Trading_Value[22];                 /*   12 매도거래대금 (Float 22) */
    char Bid_Trading_Value[22];                 /*   13 매수거래대금 (Float 22) */
    char Filler_Value[1078];                    /*   14 필러값 (String 1078) */
} TCSMIH26401_DATA;

#endif  /* _KRX_TCSMIH26401_H */
