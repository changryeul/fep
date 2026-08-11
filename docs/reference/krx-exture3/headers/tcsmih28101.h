#ifndef _KRX_TCSMIH28101_H
#define _KRX_TCSMIH28101_H

/* TCSMIH28101 국채전문유통시장차감결제내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Settlement_Date[8];                    /*    5 결제일자 (String 8) */
    char Settlement_Number[10];                 /*    6 결제번호 (String 10) */
    char Dvp_Step[2];                           /*    7 DVP단계 (String 2) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Deduction_Payment_Quantity[22];        /*   10 차감납부수량 (Float 22) */
    char Deduction_Receive_Quantity[22];        /*   11 차감수령수량 (Float 22) */
    char Deduction_Payment_Money[22];           /*   12 차감납부대금 (Float 22) */
    char Deduction_Receive_Money[22];           /*   13 차감수령대금 (Float 22) */
    char Dvp_Settlement_Exclude_Yes_Or_No[2];   /*   14 DVP결제제외여부 (String 2) */
    char Trst_Princ_Integ_Tp_Cd[2];             /*   15 위탁자기통합구분코드 (String 2) */
    char Filler_Value[1040];                    /*   16 필러값 (String 1040) */
} TCSMIH28101_DATA;

#endif  /* _KRX_TCSMIH28101_H */
