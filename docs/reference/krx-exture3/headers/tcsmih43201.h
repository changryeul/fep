#ifndef _KRX_TCSMIH43201_H
#define _KRX_TCSMIH43201_H

/* TCSMIH43201 장중추가증거금_위탁자기별거래증거금소요액(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Rnd_No[4];                             /*    5 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    6 회차시각 (String 9) */
    char Im_Calc_Bas_Tm[9];                     /*    7 데이터산출기준시각 (String 9) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    9 거래전문회원번호 (String 5) */
    char Trust_Principal_Intergration_Type_Code[2];/*   10 위탁자기통합구분코드 (String 2) */
    char Trading_Margin_Required_Value[22];     /*   11 거래증거금소요액 (Float 22) */
    char Filler_Value[1113];                    /*   12 필러값 (String 1113) */
} TCSMIH43201_DATA;

#endif  /* _KRX_TCSMIH43201_H */
