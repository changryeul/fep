#ifndef _KRX_TCSMIH27101_H
#define _KRX_TCSMIH27101_H

/* TCSMIH27101 회원별이연결제대금정보(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Settlement_Date[8];                    /*    4 결제일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    5 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Mkt_Id[3];                             /*    7 시장ID (String 3) */
    char Secugrp_Id[2];                         /*    8 증권그룹ID (String 2) */
    char Isu_Cd[12];                            /*    9 종목코드 (String 12) */
    char Trust_Principal_Type_Code[2];          /*   10 위탁자기구분코드 (String 2) */
    char Setl_Fail_Qty[15];                     /*   11 결제페일수량 (Long 15) */
    char Clsprc[12];                            /*   12 종가 (Long 12) */
    char Contionuous_Net_Settlement_Amount[22]; /*   13 이연결제대금 (Long 22) */
    char Contionuous_Net_Settlement_Amount_Settlement_Position[1];/*   14 이연결제대금결제포지션 (String 1) */
    char Filler_Value[1087];                    /*   15 필러값 (String 1087) */
} TCSMIH27101_DATA;

#endif  /* _KRX_TCSMIH27101_H */
