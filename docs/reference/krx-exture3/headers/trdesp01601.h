#ifndef _KRX_TRDESP01601_H
#define _KRX_TRDESP01601_H

/* TRDESP01601 고속 알고리즘 거래자 정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 251 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 ★신규 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 ★신규 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 ★신규 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 ★신규 (String 8) */
    char Trdr_Id[6];                            /*    5 거래자ID ★신규 (String 6) */
    char Acnt_No[12];                           /*    6 계좌번호 ★신규 (String 12) */
    char Mktpartc_No[5];                        /*    7 시장참가자번호 ★신규 (String 5) */
    char Trdr_Main_Id[4];                       /*    8 거래자메인ID ★신규 (String 4) */
    char Cntr_Cd[3];                            /*    9 국가코드 ★신규 (String 3) */
    char Invst_Tp_Cd[4];                        /*   10 투자자구분코드 ★신규 (String 4) */
    char Forninvst_Tp_Cd[2];                    /*   11 외국인투자자구분코드 (String 2) */
    char Prn_Corp_Tp_Cd[1];                     /*   12 개인법인구분코드 ★신규 (String 1) */
    char Maintr_Mkt_Tp_Cd[1];                   /*   13 주거래시장구분코드 ★신규 (String 1) */
    char Trd_Mkt_Tp_Cd[5];                      /*   14 거래시장구분코드 ★신규 (Long 5) */
    char Corp_Grp_Tp_Cd[1];                     /*   15 법인단체구분코드 ★신규 (String 1) */
    char Corp_Nm[80];                           /*   16 법인명 ★신규 (String 80) */
    char Mbr_Yn[1];                             /*   17 회원사여부 ★신규 (String 1) */
    char Prngrp_Nm[80];                         /*   18 개인및단체명 ★신규 (String 80) */
    char Grp_Tp_Cd[1];                          /*   19 단체구분코드 ★신규 (String 1) */
    char Trdr_Sub_Id[2];                        /*   20 거래자서브ID ★신규 (String 2) */
    char Trdr_Clss_Cd[1];                       /*   21 거래자분류코드 ★신규 (String 1) */
    char Trst_Princ_Tp_Cd[2];                   /*   22 위탁자기구분코드 ★신규 (String 2) */
    char Acnt_Tp_Cd[2];                         /*   23 계좌구분코드 ★신규 (String 2) */
} TRDESP01601_DATA;

#endif  /* _KRX_TRDESP01601_H */
