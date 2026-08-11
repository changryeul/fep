#ifndef _KRX_TCSMIH10801_H
#define _KRX_TCSMIH10801_H

/* TCSMIH10801 최종결제가격(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Mkt_Id[3];                             /*    6 시장ID (String 3) */
    char Secugrp_Id[2];                         /*    7 증권그룹ID (String 2) */
    char Uly_Cd[2];                             /*    8 기초자산코드 (String 2) */
    char Mbr_No[5];                             /*    9 회원번호 ★변경 (String 5) */
    char Lst_Setl_Prc[18];                      /*   10 최종결제가격 (Float 18) */
    char Lst_Setl_Prc_Tp_Cd[1];                 /*   11 최종결제가격구분코드 (String 1) */
    char Filr_Val[1137];                        /*   12 필러값 (String 1137) */
} TCSMIH10801_DATA;

#endif  /* _KRX_TCSMIH10801_H */
