#ifndef _KRX_TCSMIH24101_H
#define _KRX_TCSMIH24101_H

/* TCSMIH24101 과다호가부담금 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Underlying_Asset_Code[2];              /*    7 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    8 회원번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Trdr_Id[6];                            /*   10 거래자ID (String 6) */
    char Trading_Date[8];                       /*   11 거래일자 (String 8) */
    char Settlement_Date[8];                    /*   12 결제일자 (String 8) */
    char Monthly_Accumulation_Detection_Count[3];/*   13 월간누적적출건수 (Long 3) */
    char Excs_Ord_Brdnchrg_Impost_Yn[1];        /*   14 과다호가부담금부과여부 (String 1) */
    char Excs_Ord_Brdnchrg[22];                 /*   15 과다호가부담금액 (Float 22) */
    char Filler_Value[1097];                    /*   16 필러값 (String 1097) */
} TCSMIH24101_DATA;

#endif  /* _KRX_TCSMIH24101_H */
