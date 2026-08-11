#ifndef _KRX_TRDESP03701_H
#define _KRX_TRDESP03701_H

/* TRDESP03701 ETN 조기상환 조건 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 115 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (string 11) */
    char Mkt_Id[3];                             /*    3 시장ID (string 3) */
    char Trnsm_Dd[8];                           /*    4 전송일자 (string 8) */
    char Bz_Dd[8];                              /*    5 영업일자 (string 8) */
    char Isu_Cd[12];                            /*    6 종목코드 (string 12) */
    char Etn_Eary_Redmpt_Cycle_Cd[2];           /*    7 ETN조기상환주기코드 (string 2) */
    char Eary_Redmpt_Valu_Dd[8];                /*    8 조기상환평가일자 (string 8) */
    char Eary_Redmpt_Cond_Rel_Cd[1];            /*    9 조기상환조건관계코드 (string 1) */
    char Eary_Redmpt_Bas1_Idx[10];              /*   10 조기상환기준1지수 (float 10) */
    char Eary_Redmpt_Bas2_Idx[10];              /*   11 조기상환기준2지수 (float 10) */
    char Eary_Redmpt_Prc[23];                   /*   12 제비용차감전조기상환가격 (float 23) */
    char Eary_Redmpt_Pay_Dd[8];                 /*   13 조기상환지급일자 (string 8) */
} TRDESP03701_DATA;

#endif  /* _KRX_TRDESP03701_H */
