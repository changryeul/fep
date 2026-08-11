#ifndef _KRX_TRDESP03301_H
#define _KRX_TRDESP03301_H

/* TRDESP03301 경매매신청정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 96 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Mkt_Id[3];                             /*    3 시장ID (String 3) */
    char Trnsm_Dd[8];                           /*    4 전송일자 (String 8) */
    char Onewayauct_Appl_Dd[8];                 /*    5 경매매신청일자 (String 8) */
    char Secugrp_Id[2];                         /*    6 증권그룹ID (String 2) */
    char Isu_Cd[12];                            /*    7 종목코드 (string 12) */
    char Askbid_Tp_Cd[1];                       /*    8 매도매수구분코드 (string 1) */
    char Onewayauct_Appl_Mbr_No[5];             /*    9 경매매신청회원번호 (String 5) */
    char Onewayauct_Appl_Qty[12];               /*   10 경매매신청수량 (long 12) */
    char Onewayauct_Min_Trdvol[12];             /*   11 경매매최소체결수량 (long 12) */
    char Onewayauct_Lwst_Bid_Prc[11];           /*   12 경매매최저입찰가격 (String 11) */
} TRDESP03301_DATA;

#endif  /* _KRX_TRDESP03301_H */
