#ifndef _KRX_TCSMIH40601_H
#define _KRX_TCSMIH40601_H

/* TCSMIH40601 장중추가증거금_계좌종목별미결제약정수량(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 248 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Rnd_No[4];                             /*    6 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    7 회차시각 (String 9) */
    char Im_Calc_Bas_Tm[9];                     /*    8 데이터산출기준시각 (String 9) */
    char Market_Identification[3];              /*    9 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   10 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*   11 기초자산코드 (String 2) */
    char Member_Number[5];                      /*   12 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*   13 거래전문회원번호 (String 5) */
    char Account_Number[12];                    /*   14 계좌번호 (String 12) */
    char Issue_Code[12];                        /*   15 종목코드 (String 12) */
    char Prevdd_Ask_Opnint_Qty[12];             /*   16 전일매도미결제약정수량 ★신규 (Long 12) */
    char Prevdd_Bid_Opnint_Qty[12];             /*   17 전일매수미결제약정수량 ★신규 (Long 12) */
    char Prsnt_Ask_Opnint_Qty[10];              /*   18 현재매도미결제약정수량 ★신규 (Long 10) */
    char Prsnt_Bid_Opnint_Qty[10];              /*   19 현재매수미결제약정수량 ★신규 (Long 10) */
    char Filler_Value[109];                     /*   20 필러값 (String 109) */
} TCSMIH40601_DATA;

#endif  /* _KRX_TCSMIH40601_H */
