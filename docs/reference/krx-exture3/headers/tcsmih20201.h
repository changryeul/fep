#ifndef _KRX_TCSMIH20201_H
#define _KRX_TCSMIH20201_H

/* TCSMIH20201 정정후거래전문회원거래내역(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 297 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Mkt_Id[3];                             /*    6 시장ID (String 3) */
    char Secugrp_Id[2];                         /*    7 증권그룹ID (String 2) */
    char Uly_Cd[2];                             /*    8 기초자산코드 (String 2) */
    char Mbr_No[5];                             /*    9 회원번호 (String 5) */
    char Nclr_Mbr_No[5];                        /*   10 거래전문회원번호 (String 5) */
    char Isu_Cd[12];                            /*   11 종목코드 (String 12) */
    char Brn_No[5];                             /*   12 지점번호 (String 5) */
    char Ord_Id[10];                            /*   13 주문ID (String 10) */
    char Trd_No[11];                            /*   14 체결번호 (Long 11) */
    char Askbid_Tp_Cd[1];                       /*   15 매도매수구분코드 (String 1) */
    char If_Trd_Prc[14];                        /*   16 체결가격 (Float 14) */
    char Trdvol[10];                            /*   17 체결수량 (Long 10) */
    char Trd_Tm[9];                             /*   18 체결시각 (String 9) */
    char Spd_Nbmm_Trd_Prc[11];                  /*   19 스프레드근월물체결가격 (Float 11) */
    char Spd_Futrmm_Trd_Prc[11];                /*   20 스프레드원월물체결가격 (Float 11) */
    char Acnt_No[12];                           /*   21 계좌번호 (String 12) */
    char Trst_Princ_Tp_Cd[2];                   /*   22 위탁자기구분코드 (String 2) */
    char Cntr_Cd[3];                            /*   23 국가코드 (String 3) */
    char Invst_Tp_Cd[4];                        /*   24 투자자구분코드 (String 4) */
    char Forninvst_Tp_Cd[2];                    /*   25 외국인투자자구분코드 (String 2) */
    char Trdbok_Tp_Cd[1];                       /*   26 거래장구분코드 ★변경 (String 1) */
    char Err_Trdcorrct_Tp_Cd[1];                /*   27 착오거래정정구분코드 (String 1) */
    char Filr_Val[129];                         /*   28 필러값 (String 129) */
} TCSMIH20201_DATA;

#endif  /* _KRX_TCSMIH20201_H */
