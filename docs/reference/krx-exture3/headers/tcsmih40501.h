#ifndef _KRX_TCSMIH40501_H
#define _KRX_TCSMIH40501_H

/* TCSMIH40501 장중추가증거금_종목별내재변동성(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 763 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
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
    char Implied_Volitility[11];                /*   14 내재변동성 (Float 11) */
    char Filler_Value[674];                     /*   15 필러값 (String 674) */
} TCSMIH40501_DATA;

#endif  /* _KRX_TCSMIH40501_H */
