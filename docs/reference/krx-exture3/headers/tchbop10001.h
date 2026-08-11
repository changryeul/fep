#ifndef _KRX_TCHBOP10001_H
#define _KRX_TCHBOP10001_H

/* TCHBOP10001 대량매매주문서거절 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 186 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Ask_And_Bid_Type_Code[1];              /*    6 매도매수구분코드 (String 1) */
    char Treasury_Stock_Statement_Identification[5];/*    7 자사주신고서ID (String 5) */
    char Treasury_Stock_Trading_Method_Code[1]; /*    8 자사주매매방법코드 (String 1) */
    char Negotiation_Invalid_Type_Code[1];      /*    9 협상무효구분코드 (String 1) */
    char Member_Number[5];                      /*   10 회원번호 (String 5) */
    char Account_Number[12];                    /*   11 계좌번호 (String 12) */
    char Order_Quantity[10];                    /*   12 주문수량 (Long 10) */
    char Order_Price[11];                       /*   13 주문가격 (Float 11) */
    char Negotiation_Number[6];                 /*   14 협상번호 (Long 6) */
    char Block_Trade_Negotiation_Detail_Number[6];/*   15 협상상세번호 (Long 6) */
    char Negotiator_Identification[10];         /*   16 협상자ID (String 10) */
    char Negotiation_Result_Rejected_Reason_Code[2];/*   17 협상결과거부사유코드 (String 2) */
    char Order_Date[8];                         /*   18 호가일자 (String 8) */
    char Member_Firm_Rejected_Time[9];          /*   19 회원사거부시각 (String 9) */
    char Member_Use_Area[60];                   /*   20 회원사용영역 (String 60) */
} TCHBOP10001_DATA;

#endif  /* _KRX_TCHBOP10001_H */
