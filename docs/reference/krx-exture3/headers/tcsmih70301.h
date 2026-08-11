#ifndef _KRX_TCSMIH70301_H
#define _KRX_TCSMIH70301_H

/* TCSMIH70301 [야간파생]계좌종목별미결제약정수량(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 248 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    1 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    1 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    1 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    1 상품완료여부 (String 1) */
    char Mkt_Id[3];                             /*    1 시장ID (String 3) */
    char Secugrp_Id[2];                         /*    1 증권그룹ID (String 2) */
    char Uly_Cd[2];                             /*    1 기초자산코드 (String 2) */
    char Mbr_No[5];                             /*    1 회원번호 (String 5) */
    char Nclr_Mbr_No[5];                        /*    1 거래전문회원번호 (String 5) */
    char Acnt_No[12];                           /*   15 계좌번호 (String 12) */
    char Isu_Cd[12];                            /*   15 종목코드 (String 12) */
    char Prevdd_Ask_Opnint_Qty[12];             /*   15 전일매도미결제약정수량 (Long 12) */
    char Prevdd_Bid_Opnint_Qty[12];             /*   15 전일매수미결제약정수량 (Long 12) */
    char Prsnt_Ask_Opnint_Qty[10];              /*   15 현재매도미결제약정수량 (Long 10) */
    char Prsnt_Bid_Opnint_Qty[10];              /*   15 현재매수미결제약정수량 (Long 10) */
    char Filr_Val[131];                         /*  131 필러값 (String 131) */
} TCSMIH70301_DATA;

#endif  /* _KRX_TCSMIH70301_H */
