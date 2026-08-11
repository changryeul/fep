#ifndef _KRX_TMLHQR00001_H
#define _KRX_TMLHQR00001_H

/* TMLHQR00001 ETN LP 정규시장 외 취득/처분 신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 113 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 ★신규 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 ★신규 (string 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 ★신규 (string 8) */
    char Mbr_No[5];                             /*    4 회원번호 ★신규 (string 5) */
    char Mkt_Id[3];                             /*    5 시장ID ★신규 (string 3) */
    char Secugrp_Id[2];                         /*    6 증권그룹ID ★신규 (string 2) */
    char Isu_Cd[12];                            /*    7 종목코드 ★신규 (string 12) */
    char List_Shrs[16];                         /*    8 상장주식수 ★신규 (long 16) */
    char Tdd_Bid_Qty[15];                       /*    9 당일매수수량 ★신규 (long 15) */
    char Tdd_Ask_Qty[15];                       /*   10 당일매도수량 ★신규 (long 15) */
    char Tdd_Incdec_Qty[15];                    /*   11 당일증감수량 ★신규 (long 15) */
} TMLHQR00001_DATA;

#endif  /* _KRX_TMLHQR00001_H */
