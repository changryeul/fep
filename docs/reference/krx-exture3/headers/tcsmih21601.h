#ifndef _KRX_TCSMIH21601_H
#define _KRX_TCSMIH21601_H

/* TCSMIH21601 거래증거금질권설정말소(입출고)통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Account_Number[12];                    /*    6 계좌번호 (String 12) */
    char Trust_Principal_Integration_Type_Code[2];/*    7 위탁자기통합구분코드 (String 2) */
    char Trading_Margin_Required_Value[22];     /*    8 거래증거금소요액 (Float 22) */
    char Ksd_Right_Of_Pledge_Establish_And_Release_Type_Code[1];/*    9 예탁원질권설정해제구분코드 (String 1) */
    char Margin_Asset_Type_Code[1];             /*   10 증거금자산구분코드
(증권구분코드) (String 1) */
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Issue_Name[80];                        /*   12 종목명 (String 80) */
    char Quantity[15];                          /*   13 수량 (Long 15) */
    char Substitute_Price[11];                  /*   14 대용가격 (Float 11) */
    char Valuation_Amount[22];                  /*   15 평가금액 (Float 22) */
    char Ovres_Shorts_Type_Code[1];             /*   16 과부족구분코드 (String 1) */
    char Trading_Margin_Valuation_Amount[22];   /*   17 거래증거금과부족금액 (Float 22) */
    char Filler_Value[961];                     /*   18 필러값 (String 961) */
} TCSMIH21601_DATA;

#endif  /* _KRX_TCSMIH21601_H */
