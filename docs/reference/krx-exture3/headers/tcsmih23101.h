#ifndef _KRX_TCSMIH23101_H
#define _KRX_TCSMIH23101_H

/* TCSMIH23101 외화증권대용가(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Reuter_Issue_Code[14];                 /*    7 로이터종목코드 (String 14) */
    char Foreign_Currency_Securities_Issue_Name[25];/*    8 외화증권종목명 (String 25) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   10 결제일자 (String 8) */
    char Foreign_Currency_Securities_Forncurr_Secu_Coupn_Rt[10];/*   11 외화증권표면이자율 (Float 10) */
    char Par_Amount[22];                        /*   12 액면금액 (Float 22) */
    char Issue_Date[8];                         /*   13 발행일자 (String 8) */
    char Expiration_Date[8];                    /*   14 만기일자 (String 8) */
    char Coupon_Type_Name[6];                   /*   15 표면이자유형명 (String 6) */
    char Issue_Country_Name[30];                /*   16 발행국가명 (String 30) */
    char Currency_Internationalstandardizationorganization_Doce[3];/*   17 통화ISO코드 (String 3) */
    char Next_Period_Interest_Payment_Date[8];  /*   18 차기이자지급일자 (String 8) */
    char Valuation_Price[15];                   /*   19 기준시세 (Float 15) */
    char Haircut_Ratio[4];                      /*   20 사정비율 (Long 4) */
    char Filler_Value[983];                     /*   21 필러값 (String 983) */
} TCSMIH23101_DATA;

#endif  /* _KRX_TCSMIH23101_H */
