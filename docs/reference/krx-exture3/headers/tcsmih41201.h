#ifndef _KRX_TCSMIH41201_H
#define _KRX_TCSMIH41201_H

/* TCSMIH41201 장중추가증거금_위탁자기별거래증거금소요액(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Rnd_No[4];                             /*    6 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    7 회차시각 (String 9) */
    char Im_Calc_Bas_Tm[9];                     /*    8 데이터산출기준시각 (String 9) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*   10 거래전문회원번호 (String 5) */
    char Trust_Principal_Intergration_Type_Code[2];/*   11 위탁자기통합구분코드 (String 2) */
    char Trade_Margin_Required_Value[22];       /*   12 거래증거금소요액 (Float 22) */
    char Filler_Value[1112];                    /*   13 필러값 (String 1112) */
} TCSMIH41201_DATA;

#endif  /* _KRX_TCSMIH41201_H */
