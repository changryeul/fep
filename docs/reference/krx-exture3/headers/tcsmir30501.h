#ifndef _KRX_TCSMIR30501_H
#define _KRX_TCSMIR30501_H

/* TCSMIR30501 착오거래정정신고및접수/승인통보(주식·파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1326 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char If_Decl_Tm[8];                         /*    6 신고시각 (String 8) */
    char Err_Trdcorrct_Rej_Rsn_Cd[2];           /*    7 착오거래정정거부사유코드 ★변경 (String 2) */
    char Trd_Dd[8];                             /*    8 거래일자 (String 8) */
    char Decl_Input_Tp_Cd[1];                   /*    9 신고입력구분코드 (String 1) */
    char Isu_Cd[12];                            /*   10 종목코드 (String 12) */
    char Brd_Id[2];                             /*   11 보드ID (String 2) */
    char Str_Trdcorrct_Trd_No[11];              /*   12 체결번호 (Long 11) */
    char Str_Trdcorrct_Grp_No[5];               /*   13 착오거래정정그룹번호 (Long 5) */
    char Str_Bygrp_Trdcorrct_Appl_Cnt[4];       /*   14 그룹별착오거래정정신청건수 (Long 4) */
    char Askbid_Tp_Cd[1];                       /*   15 매도매수구분코드 (String 1) */
    char Aftrdcorrct_Trst_Acnt_No[12];          /*   16 거래정정후위탁계좌번호 (String 12) */
    char Aftrdcorrct_Trst_Tp_Cd[2];             /*   17 거래정정후위탁구분코드 (String 2) */
    char Aftrdcorrct_Trst_Cntr_Cd[3];           /*   18 거래정정후위탁국가코드 (String 3) */
    char Aftrdcorrct_Trst_Invst_Tp_Cd[4];       /*   19 거래정정후위탁투자자구분코드 (String 4) */
    char Aftrdcorrct_Trst_Forninvst_Cd[2];      /*   20 거래정정후위탁외국인투자자구분코드 (String 2) */
    char Str_Aftrdcorrct_Trst_Trdvol[10];       /*   21 거래정정후위탁체결수량 (Long 10) */
    char Aftrdcorrct_Trstcom_No[5];             /*   22 거래정정후위탁사번호 (String 5) */
    char Aftrdcorrct_Princ_Acnt_No[12];         /*   23 거래정정후자기계좌번호 (String 12) */
    char Aftrdcorrct_Princ_Tp_Cd[2];            /*   24 거래정정후자기구분코드 (String 2) */
    char Aftrdcorrct_Princ_Cntr_Cd[3];          /*   25 거래정정후자기국가코드 (String 3) */
    char Aftrdcorrct_Princ_Invst_Tp_Cd[4];      /*   26 거래정정후자기투자자구분코드 (String 4) */
    char Aftrdcorrct_Princ_Forninvst_Cd[2];     /*   27 거래정정후자기외국인투자자구분코드 (String 2) */
    char Str_Aftrdcorrct_Princ_Trdvol[10];      /*   28 거래정정후자기체결수량 (Long 10) */
    char Err_Trdcorrct_Inside_Ctrl_Yn[1];       /*   29 착오거래정정내부통제여부 (String 1) */
    char Opertr_Nm[80];                         /*   30 입력자성명 (String 80) */
    char Opertr_Tel_No[20];                     /*   31 입력자전화번호 (String 20) */
    char Err_Trdcorrct_Rsn_Cd[2];               /*   32 착오거래정정사유코드 (String 2) */
    char Err_Trdcorrct_Dtl_Rsn[1000];           /*   33 착오거래정정상세사유 (String 1000) */
    char Mbr_Use_Area2[60];                     /*   34 회원사용영역 (String 60) */
} TCSMIR30501_DATA;

#endif  /* _KRX_TCSMIR30501_H */
