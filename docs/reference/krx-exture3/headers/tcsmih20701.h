#ifndef _KRX_TCSMIH20701_H
#define _KRX_TCSMIH20701_H

/* TCSMIH20701 수수료및협회비(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Prod_Complt_Yn[1];                     /*    5 상품완료여부 (String 1) */
    char Market_Identification[3];              /*    6 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    7 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    9 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*   10 거래전문회원번호 (String 5) */
    char Settlement_Date[8];                    /*   11 결제일자 (String 8) */
    char Fee[22];                               /*   12 수수료 (Float 22) */
    char Trading_Krx_Trading_Fee[22];           /*   13 거래체결KRX거래수수료 (Float 22) */
    char Trading_Krx_Clearing_Settlement_Fee[22];/*   14 거래체결KRX청산결제수수료 (Float 22) */
    char Trading_Ndv_Cme_Trading_Fee[22];       /*   15 거래체결야간CME거래수수료 (Float 22) */
    char Trading_Ndv_Cme_Clearing_Settlement_Fee[22];/*   16 거래체결야간CME청산결제수수료 (Float 22) */
    char Last_Settlement_Trading_Fee[22];       /*   17 최종결제거래수수료 (Float 22) */
    char Last_Settlement_Clearing_Settlement_Fee[22];/*   18 최종결제청산결제수수료 (Float 22) */
    char Association_Membership_Fee[22];        /*   19 협회비 (Float 22) */
    char Underwriting_And_Delivering_Association_Membership_Fee[22];/*   20 최종결제협회비 (Float 22) */
    char Filler_Value[945];                     /*   21 필러값 (String 945) */
} TCSMIH20701_DATA;

#endif  /* _KRX_TCSMIH20701_H */
