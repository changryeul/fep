#ifndef _KRX_TCSMIH22302_H
#define _KRX_TCSMIH22302_H

/* TCSMIH22302 신용위험거래증거금_경고및부과통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Noti_Tp_Cd[1];                         /*    6 통보구분코드 (String 1) */
    char Cr_Lmt_Amt[22];                        /*    7 신용위험한도금액 (Float 22) */
    char Adj_Net_Risk_Mrgn[23];                 /*    8 조정순위험증거금 (Float 23) */
    char Grt_Amt[22];                           /*    9 지급보증금액 (Float 22) */
    char Credit_Risk_Mrgn[23];                  /*   10 신용위험증거금 (Float 23) */
    char Filler_Value[1073];                    /*   11 필러값 (String 1073) */
} TCSMIH22302_DATA;

#endif  /* _KRX_TCSMIH22302_H */
