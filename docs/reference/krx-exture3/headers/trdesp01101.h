#ifndef _KRX_TRDESP01101_H
#define _KRX_TRDESP01101_H

/* TRDESP01101 파생종목 조정 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 183 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Adj_Factr_1[22];                       /*    7 조정계수1 (Float 22) */
    char Adj_Factr_2[22];                       /*    8 조정계수2 (Float 22) */
    char Bfadj_Setlmult[22];                    /*    9 조정전거래승수 (Float 22) */
    char Afadj_Setlmult[22];                    /*   10 조정후거래승수 (Float 22) */
    char Bfadj_Prc[18];                         /*   11 조정전가격 (Float 18) */
    char Afadj_Prc[18];                         /*   12 조정후가격 (Float 18) */
    char Opnint_Qty_Adj_Factr[6];               /*   13 미결제약정수량조정계수 (Long 6) */
} TRDESP01101_DATA;

#endif  /* _KRX_TRDESP01101_H */
