#ifndef _KRX_TCSMIH23602_H
#define _KRX_TCSMIH23602_H

/* TCSMIH23602 착오거래옵션결제차금확정내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    7 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    9 거래전문회원번호 (String 5) */
    char Settlement_Date[8];                    /*   10 결제일자 (String 8) */
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Trust_Ask_Trading_Value[22];           /*   12 위탁매도거래대금 (Float 22) */
    char Trust_Bid_Trading_Value[22];           /*   13 위탁매수거래대금 (Float 22) */
    char Principal_Ask_Trading_Value[22];       /*   14 자기매도거래대금 (Float 22) */
    char Principal_Bid_Trading_Value[22];       /*   15 자기매수거래대금 (Float 22) */
    char Trust_Exercise_Payoff[23];             /*   16 위탁권리행사차금 (Float 23) */
    char Trust_Exercise_Assignment_Payoff[23];  /*   17 위탁권리배정차금 (Float 23) */
    char Principal_Exercise_Payoff[23];         /*   18 자기권리행사차금 (Float 23) */
    char Principal_Exercise_Assignment_Payoff_Amount[23];/*   19 자기권리배정차금금액 (Float 23) */
    char Principal_Exercise_Assignment_Payoff_Amount_2[952];/*   20 필러값 (String 952) */
} TCSMIH23602_DATA;

#endif  /* _KRX_TCSMIH23602_H */
