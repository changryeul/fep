#ifndef _KRX_TCSMIH27001_H
#define _KRX_TCSMIH27001_H

/* TCSMIH27001 회원별페일확정정보(주식시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Trading_Date[8];                       /*    4 거래일자 (String 8) */
    char Settlement_Date[8];                    /*    5 결제일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    6 전문완료여부 (String 1) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Market_Identification[3];              /*    8 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    9 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Trust_Principal_Type_Code[2];          /*   11 위탁자기구분코드 (String 2) */
    char Settlement_Position_Code[1];           /*   12 결제포지션코드 (String 1) */
    char Settlement_Fail_Quantity[15];          /*   13 결제페일수량 (Long 15) */
    char Fail_Accumulation_Days[3];             /*   14 페일누적일수 (Long 3) */
    char Filler_Value[1110];                    /*   15 필러값 (String 1110) */
} TCSMIH27001_DATA;

#endif  /* _KRX_TCSMIH27001_H */
