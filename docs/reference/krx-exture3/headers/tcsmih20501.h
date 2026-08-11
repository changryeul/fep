#ifndef _KRX_TCSMIH20501_H
#define _KRX_TCSMIH20501_H

/* TCSMIH20501 선물일일정산(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
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
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Profit_Loss_Type_Code[1];              /*   12 이익손실구분코드 (String 1) */
    char Trust_That_Day_Payoff[23];             /*   13 위탁당일차금 (Float 23) */
    char Profit_Loss_Type_Code_2[1];            /*   14 이익손실구분코드 (String 1) */
    char Trust_Update_Payoff[23];               /*   15 위탁갱신차금 (Float 23) */
    char Profit_Loss_Type_Code_3[1];            /*   16 이익손실구분코드 (String 1) */
    char Trust_Last_Settlement_Payoff[23];      /*   17 위탁최종결제차금 (Float 23) */
    char Profit_Loss_Type_Code_4[1];            /*   18 이익손실구분코드 (String 1) */
    char Principal_That_Day_Payoff[23];         /*   19 자기당일차금 (Float 23) */
    char Profit_Loss_Type_Code_5[1];            /*   20 이익손실구분코드 (String 1) */
    char Principal_Update_Payoff[23];           /*   21 자기갱신차금 (Float 23) */
    char Profit_Loss_Type_Code_6[1];            /*   22 이익손실구분코드 (String 1) */
    char Principal_Last_Settlement_Payoff[23];  /*   23 자기최종결제차금 (Float 23) */
    char Filler_Value[995];                     /*   24 필러값 (String 995) */
} TCSMIH20501_DATA;

#endif  /* _KRX_TCSMIH20501_H */
