#ifndef _KRX_TCSMIH26101_H
#define _KRX_TCSMIH26101_H

/* TCSMIH26101 수수료(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Non_Clrearing_Member_Number[5];        /*    8 거래전문회원번호 (String 5) */
    char Trading_Date[8];                       /*    9 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   10 결제일자 (String 8) */
    char Pay_Date[8];                           /*   11 납부일자 (String 8) */
    char Fee[22];                               /*   12 수수료 (Float 22) */
    char Trading_Krx_Trading_Fee[22];           /*   13 거래체결KRX거래수수료 (Float 22) */
    char Trading_Krx_Clearing_Settlement_Fee[22];/*   14 거래체결KRX청산결제수수료 (Float 22) */
    char Filler_Value[1064];                    /*   13 필러값 (String 1064) */
} TCSMIH26101_DATA;

#endif  /* _KRX_TCSMIH26101_H */
