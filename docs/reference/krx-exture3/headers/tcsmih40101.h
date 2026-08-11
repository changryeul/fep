#ifndef _KRX_TCSMIH40101_H
#define _KRX_TCSMIH40101_H

/* TCSMIH40101 장중추가증거금_선물정산가격(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Tracsaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Rnd_No[4];                             /*    6 회차번호 (Long 4) */
    char Rnd_Tm[9];                             /*    7 회차시각 (String 9) */
    char Im_Calc_Bas_Tm[9];                     /*    8 데이터산출기준시각 (String 9) */
    char Market_Identification[3];              /*    9 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   10 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*   11 기초자산코드 (String 2) */
    char Member_Number[5];                      /*   12 회원번호 ★변경 (String 5) */
    char Issue_Code[12];                        /*   13 종목코드 (String 12) */
    char Settlement_Multiplier[22];             /*   14 거래승수 (Float 22) */
    char Today_Settlement_Price[18];            /*   15 금일정산가격 (Float 18) */
    char Previous_Day_Settlement_Price[18];     /*   16 전일정산가격 (Float 18) */
    char Settlement_Price_Type_Code[2];         /*   17 정산가격구분코드 ★변경 (String 2) */
    char Filler_Value[1062];                    /*   18 필러값 (String 1062) */
} TCSMIH40101_DATA;

#endif  /* _KRX_TCSMIH40101_H */
