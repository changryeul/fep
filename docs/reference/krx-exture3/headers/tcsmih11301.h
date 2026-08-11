#ifndef _KRX_TCSMIH11301_H
#define _KRX_TCSMIH11301_H

/* TCSMIH11301 기준채권정보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Underlying_Asset_Code[2];              /*    6 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    7 회원번호 ★변경 (String 5) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Bond_Code[12];                         /*    9 채권종목코드 (String 12) */
    char Bond_Korean_Name[80];                  /*   10 채권한글명 (String 80) */
    char Bond_Standard_Goods_Face_Interest_Rate[13];/*   11 채권표준물표면금리 (Float 13) */
    char Bond_Issue_Date[8];                    /*   12 채권발행일자 (String 8) */
    char Bond_Expiration_Date[8];               /*   13 채권만기일자 (String 8) */
    char Annual_Bond_Coupon_Payment_Count[4];   /*   14 연간채권이표지급횟수 (Long 4) */
    char Conversion_Factor[22];                 /*   15 채권전환계수 (Float 22) */
    char Filler_Value[1002];                    /*   16 필러값 (String 1002) */
} TCSMIH11301_DATA;

#endif  /* _KRX_TCSMIH11301_H */
