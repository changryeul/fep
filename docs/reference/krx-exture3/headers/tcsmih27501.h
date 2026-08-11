#ifndef _KRX_TCSMIH27501_H
#define _KRX_TCSMIH27501_H

/* TCSMIH27501 차익거래면세한도(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Tracsaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Base_Future_Code[2];                   /*    6 기본선물코드 (String 2) */
    char Account_Number[12];                    /*    7 계좌번호 (String 12) */
    char Trading_Date[8];                       /*    8 거래일자 (String 8) */
    char Market_Identification[3];              /*    9 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   10 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*   11 종목코드 (String 12) */
    char Ask_Trading_Volumn[10];                /*   12 매도거래량 (Long 10) */
    char Ask_Trading_Value[22];                 /*   13 매도거래대금 (Float 22) */
    char Filler_Value[1093];                    /*   14 필러값 (String 1093) */
} TCSMIH27501_DATA;

#endif  /* _KRX_TCSMIH27501_H */
