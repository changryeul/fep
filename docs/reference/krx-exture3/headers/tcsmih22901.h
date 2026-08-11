#ifndef _KRX_TCSMIH22901_H
#define _KRX_TCSMIH22901_H

/* TCSMIH22901 포지션이관신청처리결과통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (string 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (string 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (string 1) */
    char Mkt_Id[3];                             /*    5 시장ID (string 3) */
    char Mbr_No[5];                             /*    6 회원번호 (string 5) */
    char Appl_Mbr_No[5];                        /*    7 신청회원번호 ★신규 (string 5) */
    char Pos_Transctrl_Appl_Id[10];             /*    8 포지션이관신청ID ★신규 (string 10) */
    char Pos_Transctrl_Appl_Tp_Cd[2];           /*    9 포지션이관신청구분코드 ★신규 (string 2) */
    char Transctrl_Mbr_No[5];                   /*   10 이관회원번호 ★신규 (string 5) */
    char Transctrl_Acnt_No[12];                 /*   11 이관계좌번호 ★신규 (string 12) */
    char Isu_Cd[12];                            /*   12 종목코드 ★신규 (string 12) */
    char Askbid_Tp_Cd[1];                       /*   13 매도매수구분코드 ★신규 (string 1) */
    char Opnint_Qty[10];                        /*   14 미결제약정수량 ★신규 (Long 10) */
    char Tkctrl_Mbr_No[5];                      /*   15 수관회원번호 ★신규 (string 5) */
    char Tkctrl_Acnt_No[12];                    /*   16 수관계좌번호 ★신규 (string 12) */
    char Filr_Val[1087];                        /*   19 필러값 (string 1087) */
} TCSMIH22901_DATA;

#endif  /* _KRX_TCSMIH22901_H */
