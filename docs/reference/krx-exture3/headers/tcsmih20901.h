#ifndef _KRX_TCSMIH20901_H
#define _KRX_TCSMIH20901_H

/* TCSMIH20901 인수도결제옵션계좌별권리행사및인수도내역(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Settlement_Date[8];                    /*   11 결제일자 (String 8) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Issue_Code[12];                        /*   13 종목코드 (String 12) */
    char Exercise_Price[18];                    /*   14 행사가격 (Float 18) */
    char Contract_Size[22];                     /*   15 거래단위 (Float 22) */
    char Settlement_Multiplier[22];             /*   16 거래승수 (Float 22) */
    char Exercise_Assignment_Type_Code[1];      /*   17 행사배정구분코드 (String 1) */
    char Exercise_Assignment_Quantity[10];      /*   18 행사배정수량 (Long 10) */
    char Underwriting_And_Delivering_Quantity[17];/*   19 인수도수량 (Long 17) */
    char Underwriting_And_Delivering_Amount[23];/*   20 인수도금액 (Float 23) */
    char Filler_Value[1006];                    /*   21 필러값 (String 1006) */
} TCSMIH20901_DATA;

#endif  /* _KRX_TCSMIH20901_H */
