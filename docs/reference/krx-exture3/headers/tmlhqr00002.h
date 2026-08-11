#ifndef _KRX_TMLHQR00002_H
#define _KRX_TMLHQR00002_H

/* TMLHQR00002 ETN LP 취득/처분수량 정정 신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 586 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 ★신규 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 ★신규 (string 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 ★신규 (string 8) */
    char Mbr_No[5];                             /*    4 회원번호 ★신규 (string 5) */
    char Mkt_Id[3];                             /*    5 시장ID ★신규 (string 3) */
    char Secugrp_Id[2];                         /*    6 증권그룹ID ★신규 (string 2) */
    char Isu_Cd[12];                            /*    7 종목코드 ★신규 (string 12) */
    char Bfmod_Tdd_Lp_Hd_Bas_Qty[15];           /*    8 정정전당일LP보유기준수량 ★신규 (long 15) */
    char Bfmod_Lp_Hd_Rto[7];                    /*    9 정정전LP보유비율 ★신규 (float 7) */
    char Bfmod_Prevdd_Lp_Hd_Qty[15];            /*   10 정정전전일LP보유수량 ★신규 (long 15) */
    char Bfmod_Tdd_Lp_Hd_Inc_Qty[15];           /*   11 정정전당일LP보유증가수량 ★신규 (long 15) */
    char Bfmod_Tdd_Lp_Hd_Dec_Qty[15];           /*   12 정정전당일LP보유감소수량 ★신규 (long 15) */
    char Afmod_Tdd_Lp_Hd_Bas_Qty[15];           /*   13 정정후당일LP보유기준수량 ★신규 (long 15) */
    char Afmod_Lp_Hd_Rto[7];                    /*   14 정정후LP보유비율 ★신규 (float 7) */
    char Afmod_Prevdd_Lp_Hd_Qty[15];            /*   15 정정후전일LP보유수량 ★신규 (long 15) */
    char Afmod_Tdd_Lp_Hd_Inc_Qty[15];           /*   16 정정후당일LP보유증가수량 ★신규 (long 15) */
    char Afmod_Tdd_Lp_Hd_Dec_Qty[15];           /*   17 정정후당일LP보유감소수량 ★신규 (long 15) */
    char Mod_Rsn[400];                          /*   18 정정사유 ★신규 (string 400) */
} TMLHQR00002_DATA;

#endif  /* _KRX_TMLHQR00002_H */
