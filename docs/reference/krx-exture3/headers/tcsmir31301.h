#ifndef _KRX_TCSMIR31301_H
#define _KRX_TCSMIR31301_H

/* TCSMIR31301 착오거래포지션이관신청및접수통보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 151 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mbr_No[5];                             /*    4 회원번호 (String 5) */
    char Pos_Transctrl_Appl_Id[10];             /*    5 포지션이관신청ID ★신규 (String 10) */
    char Pos_Transctrl_Orgn_Appl_Id[10];        /*    6 포지션이관원신청ID ★신규 (String 10) */
    char Decl_Input_Tp_Cd[1];                   /*    7 신고입력구분코드 (String 1) */
    char Errtrd_Dd[8];                          /*    8 착오거래일자 (String 8) */
    char Transctrl_Acnt_No[12];                 /*    9 이관계좌번호 (String 12) */
    char Isu_Cd[12];                            /*   10 종목코드 (String 12) */
    char Askbid_Tp_Cd[1];                       /*   11 매도매수구분코드 ★신규 (String 1) */
    char Opnint_Qty[10];                        /*   12 미결제약정수량 (Long 10) */
    char Tkctrl_Acnt_No[12];                    /*   13 수관계좌번호 (String 12) */
    char Tkctrl_Trst_Princ_Tp_Cd[2];            /*   14 수관위탁자기구분코드 (String 2) */
    char Tkctrl_Sb_Stkcert_Acnt_No[12];         /*   15 수관대용주권계좌번호 (String 12) */
    char Tkctrl_Acnt_Tp_Cd[2];                  /*   16 수관계좌구분코드 (String 2) */
    char Tkctrl_Acnt_Mrgn_Tp_Cd[2];             /*   17 수관계좌증거금유형코드 (String 2) */
    char Tkctrl_Cntr_Cd[3];                     /*   18 수관국가코드 (String 3) */
    char Tkctrl_Invst_Tp_Cd[4];                 /*   19 수관투자자구분코드 (String 4) */
    char Tkctrl_Forninvst_Tp_Cd[2];             /*   20 수관외국인투자자구분코드 (String 2) */
    char Appl_Tm[9];                            /*   21 신청시각 (String 9) */
    char Errtrd_Pos_Transctrl_Rej_Cd[4];        /*   22 착오거래포지션이관신청거부사유코드 (String 4) */
} TCSMIR31301_DATA;

#endif  /* _KRX_TCSMIR31301_H */
