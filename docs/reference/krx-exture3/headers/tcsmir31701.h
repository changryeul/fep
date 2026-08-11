#ifndef _KRX_TCSMIR31701_H
#define _KRX_TCSMIR31701_H

/* TCSMIR31701 포지션이관신청및접수통보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 203 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Mbr_No[5];                             /*    4 회원번호 (String 5) */
    char Pos_Transctrl_Appl_Id[10];             /*    5 포지션이관신청ID (String 10) */
    char Pos_Transctrl_Orgn_Appl_Id[10];        /*    6 포지션이관원신청ID (String 10) */
    char Decl_Input_Tp_Cd[1];                   /*    7 신고입력구분코드 ★변경 (String 1) */
    char Pos_Transctrl_Appl_Tp_Cd[2];           /*    8 포지션이관신청구분코드 ★변경 (String 2) */
    char Transctrl_Acnt_No[12];                 /*    9 이관계좌번호 (String 12) */
    char Isu_Cd[12];                            /*   10 종목코드 ★변경 (String 12) */
    char Askbid_Tp_Cd[1];                       /*   11 매도매수구분코드 ★변경 (String 1) */
    char Opnint_Qty[10];                        /*   12 미결제약정수량 ★변경 (Long 10) */
    char Tkctrl_Mbr_No[5];                      /*   13 수관회원번호 (String 5) */
    char Tkctrl_Acnt_No[12];                    /*   14 수관계좌번호 (String 12) */
    char Real_Nm_Confrm_No[20];                 /*   15 실명확인번호 ★신규 (String 20) */
    char Mbr_Use_Area[60];                      /*   16 회원사용영역 (String 60) */
    char Appl_Tm[9];                            /*   17 신청시각 (String 9) */
    char Pos_Transctrl_Appl_Rej_Rsn_Cd[4];      /*   18 포지션이관신청거부사유코드 ★변경 (String 4) */
} TCSMIR31701_DATA;

#endif  /* _KRX_TCSMIR31701_H */
