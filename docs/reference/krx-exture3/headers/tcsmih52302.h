#ifndef _KRX_TCSMIH52302_H
#define _KRX_TCSMIH52302_H

/* TCSMIH52302 [야간파생] 신용위험거래증거금_경고및부과통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    1 트랜잭션코드 (string 11) */
    char Trnsm_Dd[8];                           /*    1 전송일자 (string 8) */
    char Emsg_Complt_Yn[1];                     /*    1 전문완료여부 (string 1) */
    char Mbr_No[5];                             /*    1 회원번호 (string 5) */
    char Noti_Tp_Cd[1];                         /*    1 통보구분코드 (string 1) */
    char Cr_Lmt_Amt[22];                        /*    1 신용위험한도금액 (float 22) */
    char Adj_Net_Risk_Mrgn[23];                 /*    1 조정순위험증거금 (float 23) */
    char Grt_Amt[22];                           /*    1 지급보증금액 (float 22) */
    char Credit_Risk_Mrgn[23];                  /*    1 신용위험증거금 (float 23) */
    char Filr_Val[1073];                        /*    1 필러값 (string 1073) */
} TCSMIH52302_DATA;

#endif  /* _KRX_TCSMIH52302_H */
