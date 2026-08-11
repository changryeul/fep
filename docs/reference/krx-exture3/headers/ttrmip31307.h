#ifndef _KRX_TTRMIP31307_H
#define _KRX_TTRMIP31307_H

/* TTRMIP31307 VI(변동성완화장치) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 117 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Me_Procs_Tm[9];                        /*    6 ME처리시각 (String 9) */
    char Vi_Releas_Tm[9];                       /*    7 VI해제시각 (String 9) */
    char Vi_Appl_Tp_Cd[1];                      /*    8 VI적용구분코드 (String 1) */
    char Vi_Kind_Cd[1];                         /*    9 VI종류코드 (String 1) */
    char Static_Vi_Tg_Bas_Prc[11];              /*   10 정적VI발동기준가격 (Float 11) */
    char Dynmc_Vi_Tg_Bas_Prc[11];               /*   11 동적VI발동기준가격 (Float 11) */
    char Vi_Tg_Prc[11];                         /*   12 VI발동가격 (Float 11) */
    char Static_Vi_Tg_Divrg_Rt[13];             /*   13 정적VI발동가격괴리율 (Float 13) */
    char Dynmc_Vi_Tg_Divrg_Rt[13];              /*   14 동적VI발동가격괴리율 (Float 13) */
} TTRMIP31307_DATA;

#endif  /* _KRX_TTRMIP31307_H */
