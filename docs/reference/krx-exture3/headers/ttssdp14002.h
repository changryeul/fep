#ifndef _KRX_TTSSDP14002_H
#define _KRX_TTSSDP14002_H

/* TTSSDP14002 시장조성자 매도 체결 증권거래세 면세 및 수수료 면제 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TTSSDP14001) */
/* DATA 길이 합계 = 139 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Mkt_Id[3];                             /*    3 시장ID (String 3) */
    char Trnsm_Dd[8];                           /*    4 전송일자 (String 8) */
    char Trd_Dd[8];                             /*    5 거래일자 (String 8) */
    char Isu_Cd[12];                            /*    6 종목코드 (String 12) */
    char Secugrp_Id[2];                         /*    7 증권그룹ID (String 2) */
    char Brd_Id[2];                             /*    8 보드ID (String 2) */
    char Sess_Id[2];                            /*    9 세션ID (String 2) */
    char Trd_No[11];                            /*   10 체결번호 (Long 11) */
    char Trd_Tm[9];                             /*   11 체결시각 (String 9) */
    char Mbr_No[5];                             /*   12 회원번호 (String 5) */
    char Acnt_No[12];                           /*   13 계좌번호 (String 12) */
    char Trd_Prc[11];                           /*   14 체결가격 (Float 11) */
    char Trdvol[10];                            /*   15 체결수량 (Long 10) */
    char Trdval[22];                            /*   16 거래대금 (Float 22) */
} TTSSDP14002_DATA;

#endif  /* _KRX_TTSSDP14002_H */
