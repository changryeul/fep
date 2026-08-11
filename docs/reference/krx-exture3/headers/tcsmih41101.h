#ifndef _KRX_TCSMIH41101_H
#define _KRX_TCSMIH41101_H

/* TCSMIH41101 장중추가증거금_해제내역통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Rnd_No[4];                             /*    5 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    6 회차시각 (String 9) */
    char Margin_Market_Identification[3];       /*    7 청산결제시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Trust_Principal_Integration_Type_Code[2];/*    9 위탁자기통합구분코드 (String 2) */
    char Net_Risk_Mrgn[23];                     /*   10 순위험증거금 (Float 23) */
    char Credit_Risk_Mrgn[23];                  /*   11 신용위험증거금 (Float 23) */
    char Regulss_Setl_Amt[22];                  /*   12 장중결제금액 (Float 22) */
    char Reqval_Agg[28];                        /*   13 필요액합계(T시) (Float 28) */
    char Releas_Bas_Tm[9];                      /*   14 해제기준시각 (String 9) */
    char Releas_Bas_Tm_Tot_Depo_Amt[22];        /*   15 해제기준시각_총예탁금액(T시+0.5시) (Float 22) */
    char Releas_Bas_Tm_Fcnfrm_Depo_Amt[22];     /*   16 해제기준시각_확인용예탁금액 (Float 22) */
    char Depo_Val_Cmp_Im_Reqval_Rto[10];        /*   17 예탁액대비장중추가증거금필요액비율 (Float 10) */
    char Final_Tm[9];                           /*   18 확정시각 (String 9) */
    char Filler_Value[978];                     /*   19 필러값 (String 978) */
} TCSMIH41101_DATA;

#endif  /* _KRX_TCSMIH41101_H */
