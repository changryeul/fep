#ifndef _KRX_TCSMIH41301_H
#define _KRX_TCSMIH41301_H

/* TCSMIH41301 장중추가증거금_기초자산변동률부과요건(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Rnd_No[4];                             /*    5 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    6 회차시각 (String 9) */
    char Im_Calc_Bas_Tm[9];                     /*    7 데이터산출기준시각 (String 9) */
    char Mrgn_Kind_Tp_Cd[1];                    /*    8 증거금종류구분코드 (String 1) */
    char Underlying_Asset_Code[2];              /*    9 기초자산코드 (String 2) */
    char Clsprc[11];                            /*   10 종가 (Float 11) */
    char Prevdd_Adj_Clsprc[18];                 /*   11 전일조정종가 (Float 18) */
    char Prc_Chg_Rt[13];                        /*   12 가격변동률 (Float 13) */
    char Prc_Chg_Mrgn_Rt[13];                   /*   13 가격변동증거금률 (String 13) */
    char Prc_Chg_Rt_Cmp_Mrgn_Rt_Rto[13];        /*   14 가격변동률대비증거금률비율 (String 13) */
    char Prc_Chg_Rt_Cmp_Mrgn_Rt_Bas_Rto[13];    /*   15 가격변동률대비증거금률기준비율 (String 13) */
    char Im_Prc_Chg_Bas_Satisfact_Yn[1];        /*   16 장중추가증거금가격변동기준충족여부 (String 1) */
    char Filler_Value[1062];                    /*   17 필러값 (String 1062) */
} TCSMIH41301_DATA;

#endif  /* _KRX_TCSMIH41301_H */
