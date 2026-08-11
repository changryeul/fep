#ifndef _KRX_TCSMIH28701_H
#define _KRX_TCSMIH28701_H

/* TCSMIH28701 REPO차감결제수량 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    6 결제일자 (String 8) */
    char Dealer_Number[5];                      /*    7 딜러번호 (String 5) */
    char Trading_Member_Number[5];              /*    8 거래회원번호 (String 5) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Repobond_Principal_Deduction_Pay_Quantity[15];/*   10 REPO 자기차감납부수량 (Long 15) */
    char Repobond_Principal_Deduction_Receive_Quantity[15];/*   11 REPO 자기차감수령수량 (Long 15) */
    char Repobond_Trustbusiness_Deduction_Pay_Quantity[15];/*   12 REPO 신탁차감납부수량 (Long 15) */
    char Repobond_Trustbusiness_Deduction_Receive_Quantity[15];/*   13 REPO 신탁차감수령수량 (Long 15) */
    char Repobond_Trust_Deduction_Pay_Quantity[15];/*   14 REPO 위탁차감납부수량 (Long 15) */
    char Repobond_Trust_Deduction_Receive_Quantity[15];/*   15 REPO 위탁차감수령수량 (Long 15) */
    char Repo_Bond_Deduction_Pay_Quantity[15];  /*   15 REPO 차감납부수량 (Long 15) */
    char Repo_Bond_Deduction_Receive_Quantity[15];/*   16 REPO 차감수령수량 (Long 15) */
    char Filr_Val[1011];                        /*   17 필러값 (String 1011) */
} TCSMIH28701_DATA;

#endif  /* _KRX_TCSMIH28701_H */
