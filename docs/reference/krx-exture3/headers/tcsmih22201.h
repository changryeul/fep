#ifndef _KRX_TCSMIH22201_H
#define _KRX_TCSMIH22201_H

/* TCSMIH22201 구분착오발생내역통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Acnt_No[12];                           /*    6 계좌번호 (String 12) */
    char Isu_Cd[12];                            /*    7 종목코드 (String 12) */
    char Brd_Id[2];                             /*    8 보드ID (String 2) */
    char Trd_No[11];                            /*    9 체결번호 (Long 11) */
    char Askbid_Tp_Cd[1];                       /*   10 매도매수구분코드 (String 1) */
    char Trdbok_Trst_Princ_Tp_Cd[2];            /*   11 거래장위탁자기구분코드 (String 2) */
    char Trdbok_Cntr_Cd[3];                     /*   12 거래장국가코드 (String 3) */
    char Trdbok_Invst_Tp_Cd[4];                 /*   13 거래장투자자구분코드 (String 4) */
    char Trdbok_Forninvst_Tp_Cd[2];             /*   14 거래장외국인투자자
구분코드 (String 2) */
    char Acnt_Trst_Princ_Tp_Cd[2];              /*   15 계좌상위탁자기구분코드 (String 2) */
    char Acnt_Cntr_Cd[3];                       /*   16 계좌상국가코드 (String 3) */
    char Acnt_Invst_Tp_Cd[4];                   /*   17 계좌상투자자구분코드 (String 4) */
    char Acnt_Forninvst_Tp_Cd[2];               /*   18 계좌상외국인투자자
구분코드 (String 2) */
    char Filr_Val[1102];                        /*   19 필러값 (String 1102) */
} TCSMIH22201_DATA;

#endif  /* _KRX_TCSMIH22201_H */
