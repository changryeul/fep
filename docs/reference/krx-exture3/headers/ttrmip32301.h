#ifndef _KRX_TTRMIP32301_H
#define _KRX_TTRMIP32301_H

/* TTRMIP32301 주식종목정보 공개 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 146 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Opn_Info_Tp_Cd[3];                     /*    4 공개정보구분코드 (String 3) */
    char Tsc_Prod_Grp_Id[3];                    /*    5 장운영상품그룹ID (String 3) */
    char Isu_Cd[12];                            /*    6 종목코드 (String 12) */
    char Opn_Tm[9];                             /*    7 공개시각 (String 9) */
    char Bas_Prc[11];                           /*    8 기준가격 (Float 11) */
    char Uplmtprc[11];                          /*    9 상한가 (Float 11) */
    char Lwlmtprc[11];                          /*   10 하한가 (Float 11) */
    char Valu_Prc[11];                          /*   11 평가가격 (Float 11) */
    char Hgst_Ord_Prc[11];                      /*   12 최고호가가격 (Float 11) */
    char Lwst_Ord_Prc[11];                      /*   13 최저호가가격 (Float 11) */
    char Opnprc_Bas_Prc_Yn[1];                  /*   14 시가기준가격여부 (String 1) */
    char Off_Tp_Cd[2];                          /*   15 락구분코드 (String 2) */
    char Parval_Chg_Tp_Cd[2];                   /*   16 액면가변경구분코드 (String 2) */
    char Qtyunit[6];                            /*   17 매매수량단위 (Long 6) */
    char List_Shrs[16];                         /*   18 상장주식수 (Long 16) */
    char Design_Yn[1];                          /*   19 지정여부 (String 1) */
    char Preoffhr_Clsprc_Posbl_Yn[1];           /*   20 장개시전시간외종가가능여부 (String 1) */
} TTRMIP32301_DATA;

#endif  /* _KRX_TTRMIP32301_H */
