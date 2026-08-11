#ifndef _KRX_TCSMIH52301_H
#define _KRX_TCSMIH52301_H

/* TCSMIH52301 [야간파생] 위탁자기별증거금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    1 트랜잭션코드 (string 11) */
    char Trnsm_Dd[8];                           /*    1 전송일자 (string 8) */
    char Emsg_Complt_Yn[1];                     /*    1 전문완료여부 (string 1) */
    char Prod_Complt_Yn[1];                     /*    1 상품완료여부 (string 1) */
    char Mbr_No[5];                             /*    1 회원번호 (string 5) */
    char Nclr_Mbr_No[5];                        /*    1 거래전문회원번호 (string 5) */
    char Trst_Princ_Integ_Tp_Cd[2];             /*    1 위탁자기통합구분코드 (string 2) */
    char Cr_Mrgn_Reqval[22];                    /*    1 신용위험증거금소요액 (float 22) */
    char Trd_Mrgn_Reqval[22];                   /*    1 거래증거금소요액 (float 22) */
    char Cashabl_Asst_Amt[22];                  /*    1 현금성자산금액 (float 22) */
    char Filr_Val[1090];                        /*    1 필러값 (string 1090) */
} TCSMIH52301_DATA;

#endif  /* _KRX_TCSMIH52301_H */
