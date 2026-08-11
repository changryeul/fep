#ifndef _KRX_TCSMIH50401_H
#define _KRX_TCSMIH50401_H

/* TCSMIH50401 [야간파생] 계좌별증거금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 633 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    1 트랜잭션코드 (string 11) */
    char Trnsm_Dd[8];                           /*    1 전송일자 (string 8) */
    char Emsg_Complt_Yn[1];                     /*    1 전문완료여부 (string 1) */
    char Mbr_No[5];                             /*    1 회원번호 (string 5) */
    char Nclr_Mbr_No[5];                        /*    1 거래전문회원번호 (string 5) */
    char Mrgn_Tp_Cd[2];                         /*   10 증거금유형코드 (string 2) */
    char Acnt_No[12];                           /*   10 계좌번호 (string 12) */
    char Acnt_Tp_Cd[2];                         /*   10 계좌구분코드 (string 2) */
    char Mrgn_Dc_Appl_Yn[1];                    /*   10 증거금할인적용여부 (string 1) */
    char Byacnt_Ndc_Mrgn_Reqval[23];            /*   10 계좌별비할인증거금소요액 (float 23) */
    char Byacnt_Dc_Mrgn_Reqval[23];             /*   10 계좌별할인증거금소요액 (float 23) */
    char Filr_Val[529];                         /*    1 필러값 (string 529) */
} TCSMIH50401_DATA;

#endif  /* _KRX_TCSMIH50401_H */
