#ifndef _KRX_TCSMIH43401_H
#define _KRX_TCSMIH43401_H

/* TCSMIH43401 장중추가증거금_해제내역통보(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
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
    char Trst_Net_Risk_Mrgn[23];                /*    9 위탁순위험증거금(T+1.5시) (Float 23) */
    char Trst_Vari_Mrgn[23];                    /*   10 위탁변동증거금(T+1.5시) (Float 23) */
    char Trst_Reqval[28];                       /*   11 위탁필요액(T+1.5시) (Float 28) */
    char Princ_Net_Risk_Mrgn[23];               /*   12 자기순위험증거금(T+1.5시) (Float 23) */
    char Princ_Vari_Mrgn[23];                   /*   13 자기변동증거금(T+1.5시) (Float 23) */
    char Princ_Reqval[28];                      /*   14 자기필요액(T+1.5시) (Float 28) */
    char Reqval_Agg[28];                        /*   15 필요액합계(T+1.5시) (Float 28) */
    char Releas_Bas_Tm[9];                      /*   16 해제기준시각(T+2시) (String 9) */
    char Releas_Bas_Tm_Tot_Depo_Amt[22];        /*   17 해제기준시각_총예탁금액(T시+2시) (Float 22) */
    char Depo_Val_Cmp_Im_Reqval_Rto[10];        /*   18 예탁액대비장중추가증거금필요액비율(T시+2시) (Float 10) */
    char Final_Tm[9];                           /*   19 확정시각 (String 9) */
    char Filler_Value[922];                     /*   20 필러값 (String 922) */
} TCSMIH43401_DATA;

#endif  /* _KRX_TCSMIH43401_H */
