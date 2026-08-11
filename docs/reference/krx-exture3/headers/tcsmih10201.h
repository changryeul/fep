#ifndef _KRX_TCSMIH10201_H
#define _KRX_TCSMIH10201_H

/* TCSMIH10201 옵션증거금기준가격(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 272 (interface-list.csv '길이(헤더제외)'와 대조) */
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
    char Isu_Cd[12];                            /*   10 종목코드 (String 12) */
    char Mrgn_Bas_Prc[18];                      /*   11 증거금기준가격 (Float 18) */
    char Mrgn_Bas_Prc_Tp_Cd[2];                 /*   12 증거금기준가격구분코드 (String 2) */
    char Filr_Val[196];                         /*   13 필러값 (String 196) */
} TCSMIH10201_DATA;

#endif  /* _KRX_TCSMIH10201_H */
