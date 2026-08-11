#ifndef _KRX_TCSMIH26201_H
#define _KRX_TCSMIH26201_H

/* TCSMIH26201 종목별결제내역(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Ty[1];                     /*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    8 거래전문회원번호 (String 5) */
    char Settlement_Type[1];                    /*    9 결제구분 (String 1) */
    char Trading_Date[8];                       /*   10 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   11 결제일자 (String 8) */
    char Issue_Code[12];                        /*   12 종목코드 (String 12) */
    char Liquid_Provider_Yes_Or_No[1];          /*   13 LP여부 (String 1) */
    char Trust_Ask_Trading_Volumn[12];          /*   14 위탁매도체결수량 (Long 12) */
    char Trust_Bid_Trading_Volumn[12];          /*   15 위탁매수체결수량 (Long 12) */
    char Securities_In_Out_Type_Code[1];        /*   16 입출고구분코드 (String 1) */
    char Trust_Settlement_Quantity[15];         /*   17 위탁결제수량 (Long 15) */
    char Principal_Ask_Trading_Volumn[12];      /*   18 자기매도체결수량 (Long 12) */
    char Principal_Bid_Trading_Volumn[12];      /*   19 자기매수체결수량 (Long 12) */
    char Securities_In_Out_Type_Code_2[1];      /*   20 입출고구분코드 (String 1) */
    char Principal_Settlement_Quantity[15];     /*   21 자기결제수량 (Long 15) */
    char Trust_Ask_Trading_Value[22];           /*   22 위탁매도거래대금 (Float 22) */
    char Trust_Bid_Trading_Value[22];           /*   23 위탁매수거래대금 (Float 22) */
    char Securities_In_Out_Type_Code_3[1];      /*   24 입출금구분코드 (String 1) */
    char Trust_Settlement_Value[22];            /*   25 위탁결제금액 (Float 22) */
    char Principal_Ask_Trading_Value[22];       /*   26 자기매도거래대금 (Float 22) */
    char Principal_Bid_Trading_Value[22];       /*   27 자기매수거래대금 (Float 22) */
    char Securities_In_Out_Type_Code_4[1];      /*   28 입출금구분코드 (String 1) */
    char Principal_Settlement_Value[22];        /*   29 자기결제금액 (Float 22) */
    char Filler_Value[910];                     /*   30 필러값 (String 910) */
} TCSMIH26201_DATA;

#endif  /* _KRX_TCSMIH26201_H */
