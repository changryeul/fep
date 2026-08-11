#ifndef _KRX_TCSMIH10701_H
#define _KRX_TCSMIH10701_H

/* TCSMIH10701 당일거래종목의종목별내재변동성(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 763 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    9 회원번호 ★변경 (String 5) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Implied_Volitility[11];                /*   11 내재변동성 (Float 11) */
    char Filler_Value[696];                     /*   12 필러값 (String 696) */
} TCSMIH10701_DATA;

#endif  /* _KRX_TCSMIH10701_H */
