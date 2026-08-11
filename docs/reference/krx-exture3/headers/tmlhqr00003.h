#ifndef _KRX_TMLHQR00003_H
#define _KRX_TMLHQR00003_H

/* TMLHQR00003 주식 시장조성 일별 보유량 신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 140 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 ★신규 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 ★신규 (string 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 ★신규 (string 8) */
    char Mbr_No[5];                             /*    4 회원번호 ★신규 (string 5) */
    char Mkt_Id[3];                             /*    5 시장ID ★신규 (string 3) */
    char Isu_Cd[12];                            /*    6 종목코드 ★신규 (string 12) */
    char Lglmkt_Chg_Qty[15];                    /*    7 장내변동수량 ★신규 (long 15) */
    char Otc_Loan_Chg_Qty[15];                  /*    8 장외차입변동수량 ★신규 (long 15) */
    char Otc_Loan_Excld_Chg_Qty[15];            /*    9 장외차입제외변동수량 ★신규 (long 15) */
    char Bas_Gen_Bal_Qty[15];                   /*   10 일반잔고수량 ★신규 (long 15) */
    char Bas_Loan_Bal_Qty[15];                  /*   11 차입잔고수량 ★신규 (long 15) */
    char Bas_Bal_Agg_Qty[15];                   /*   12 잔고합계수량 ★신규 (long 15) */
} TMLHQR00003_DATA;

#endif  /* _KRX_TMLHQR00003_H */
