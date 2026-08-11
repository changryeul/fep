#ifndef _KRX_TRDESP00701_H
#define _KRX_TRDESP00701_H

/* TRDESP00701 자사주 신청정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 103 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Treasury_Stock_Application_Date[8];    /*    5 자사주신청일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Treasury_Stock_Statement_Identification[5];/*    8 자사주신고서ID (String 5) */
    char Ask_And_Bid_Type_Code[1];              /*    9 매도매수구분코드 (String 1) */
    char Treasury_Stock_Trading_Method_Code[1]; /*   10 자사주매매방법코드 (String 1) */
    char Treasury_Stock_Application_Maket_Participant_Float[5];/*   11 자사주신청시장참가자번호 (String 5) */
    char Opening_Price_Single_Price_Call_Auction_Treasury_Stock_Application_Quantity[12];/*   12 시가단일가자사주신청수량 (Long 12) */
    char Continous_Auction_Treasury_Stock_Application_Quantity[12];/*   13 접속매매자사주신청수량 (Long 12) */
    char Off_Hours_Session_Block_Trade_Treasury_Stock_Application_Quantity[12];/*   14 시간외대량매매자사주신청수량 (Long 12) */
} TRDESP00701_DATA;

#endif  /* _KRX_TRDESP00701_H */
