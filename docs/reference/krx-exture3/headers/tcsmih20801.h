#ifndef _KRX_TCSMIH20801_H
#define _KRX_TCSMIH20801_H

/* TCSMIH20801 현금결제옵션계좌별권리행사결제내역(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 678 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*   10 거래전문회원번호 (String 5) */
    char Account_Number[12];                    /*   11 계좌번호 (String 12) */
    char Issue_Code[12];                        /*   12 종목코드 (String 12) */
    char Exercise_Assignment_Type_Code[1];      /*   13 행사배정구분코드 (String 1) */
    char Exercise_Assignment_Quantity[10];      /*   14 행사배정수량 (Long 10) */
    char Exercise_Payoff[23];                   /*   15 권리행사차금 (Float 23) */
    char Filler_Value[571];                     /*   16 필러값 (String 571) */
} TCSMIH20801_DATA;

#endif  /* _KRX_TCSMIH20801_H */
