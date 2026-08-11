#ifndef _KRX_TCSMIH21401_H
#define _KRX_TCSMIH21401_H

/* TCSMIH21401 거래증거금과부족내역통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Mkt_Id[3];                             /*    5 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Trust_Principal_Integration_Type_Code[2];/*    7 위탁자기통합구분코드 (String 2) */
    char Credit_Risk_Margin_Required_Value[22]; /*    8 신용위험증거금소요액 (Float 22) */
    char Trading_Margin_Required_Value[22];     /*    9 거래증거금소요액 (Float 22) */
    char Trading_Margin_Valuation[22];          /*   10 거래증거금평가금액 (Float 22) */
    char Ovres_Shorts_Type_Code[1];             /*   11 과부족구분코드 (String 1) */
    char Trading_Margin_Valuation_Amount[22];   /*   12 거래증거금과부족금액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Required_Value[22];/*   13 현금성자산거래증거금소요액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Valuation[22];/*   14 현금성자산거래증거금평가금액 (Float 22) */
    char Cashable_Asset_Trading_Margin_Ovres_Shorts_Type_Code[1];/*   15 현금성자산과부족구분코드 (String 1) */
    char Cashable_Asset_Trading_Margin_Valuation_Amount[22];/*   16 현금성자산거래증거금과부족금액 (Float 22) */
    char Filler_Value[1003];                    /*   17 필러값 (String 1003) */
} TCSMIH21401_DATA;

#endif  /* _KRX_TCSMIH21401_H */
