#ifndef _KRX_TCSMIH43301_H
#define _KRX_TCSMIH43301_H

/* TCSMIH43301 장중추가증거금_부과내역통보(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Rnd_No[4];                             /*    5 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    6 회차시각 (String 9) */
    char Clearing_Settlement_Market_Identification[3];/*    7 청산결제시장ID (String 3) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Trst_Net_Risk_Mrgn[23];                /*    9 위탁순위험증거금 (Float 23) */
    char Trst_Vari_Mrgn[23];                    /*   10 위탁변동증거금 (Float 23) */
    char Trst_Reqval[28];                       /*   11 위탁필요액 (Float 28) */
    char Princ_Net_Risk_Mrgn[23];               /*   12 자기순위험증거금 (Float 23) */
    char Princ_Vari_Mrgn[23];                   /*   13 자기변동증거금 (Float 23) */
    char Princ_Reqval[28];                      /*   14 자기필요액 (Float 28) */
    char Reqval_Agg[28];                        /*   15 필요액합계 (Float 28) */
    char Impost_Bas_Tm[9];                      /*   16 부과산출기준시각(T시) (String 9) */
    char Impost_Bas_Tm_Tot_Depo_Amt[22];        /*   17 부과산출기준시각_총예탁금액 (Float 22) */
    char Depo_Val_Cmp_Im_Reqval_Rto[10];        /*   18 예탁액대비장중추가증거금필요액비율 (Float 10) */
    char Impost_Bas_Tm_2[9];                    /*   19 부과기준시각(T+1시) (String 9) */
    char Impost_Bas_Tm_Tot_Depo_Amt_2[22];      /*   20 부과기준시각_총예탁금액 (Float 22) */
    char Im[22];                                /*   21 장중추가증거금 (Float 22) */
    char Final_Tm[9];                           /*   22 확정시각 (String 9) */
    char Pay_Deadline[9];                       /*   23 납부시한 (String 9) */
    char Filler_Value[860];                     /*   24 필러값 (String 860) */
} TCSMIH43301_DATA;

#endif  /* _KRX_TCSMIH43301_H */
