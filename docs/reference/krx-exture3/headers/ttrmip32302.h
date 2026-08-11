#ifndef _KRX_TTRMIP32302_H
#define _KRX_TTRMIP32302_H

/* TTRMIP32302 채권종목정보 공개 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 182 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Opn_Info_Tp_Cd[3];                     /*    4 공개정보구분코드 (String 3) */
    char Tsc_Prod_Grp_Id[3];                    /*    5 장운영상품그룹ID (String 3) */
    char Isu_Cd[12];                            /*    6 종목코드 (String 12) */
    char Opn_Tm[9];                             /*    7 공개시각 (String 9) */
    char Int_Pay_Methd_Cd[2];                   /*    8 이자지급방법코드 (string 2) */
    char Isu_Dd[8];                             /*    9 발행일자 (string 8) */
    char Redmpt_Dd[8];                          /*   10 상환일자 (string 8) */
    char Sale_Dd[8];                            /*   11 매출일자 (string 8) */
    char Bnd_Isu_Rt[13];                        /*   12 채권발행율 (float 13) */
    char Coupn_Rt[14];                          /*   13 표면이자율 (float 14) */
    char Int_Pay_Calc_Mms[4];                   /*   14 이자지급계산월수 (long 4) */
    char Int_Dd_Pay_Bas_Tp_Cd[1];               /*   15 이자일지급기준구분코드 (string 1) */
    char Int_Mmend_Tp_Cd[1];                    /*   16 이자월말구분코드 (string 1) */
    char Int_Unwon_Ud_Procs_Cd[1];              /*   17 이자원단위미만처리코드 (string 1) */
    char Bnd_Pre_Sale_Int_Pay_Methd_Cd[1];      /*   18 채권선매출이자지급방법코드 (string 1) */
    char Curr_Tp_Cd[1];                         /*   19 통화구분코드 (string 1) */
    char Exp_Redmpt_Rto[13];                    /*   20 만기상환비율 (float 13) */
    char Grt_Yd[13];                            /*   21 보장수익률 (float 13) */
    char Defr_Mms[4];                           /*   22 거치개월수 (long 4) */
    char Split_Redmpt_Cnt[5];                   /*   23 분할상환횟수 (long 5) */
    char Spd[10];                               /*   24 가산금리 (float 10) */
    char Tdd_Bas_Prc[11];                       /*   25 당일기준가격 (float 11) */
    char Bk_Holdy_Dd_Int_Pay_Decsn_Cd[1];       /*   26 은행휴무일이자지급결정코드 (string 1) */
    char Hbrdbnd_Yn[1];                         /*   27 신종자본증권여부 (string 1) */
    char Coupn_Rt_Decsn_Bas_Int_Cd[2];          /*   28 표면이자율결정기준금리코드 (string 2) */
    char Int_Rt_Decsn_Bas_Dd[8];                /*   29 이자율결정기준일자 (string 8) */
    char Arrantrd_Yn[1];                        /*   30 정리매매여부 (string 1) */
} TTRMIP32302_DATA;

#endif  /* _KRX_TTRMIP32302_H */
