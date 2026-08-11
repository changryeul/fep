#ifndef _KRX_TSVIIS00400_H
#define _KRX_TSVIIS00400_H

/* TSVIIS00400 실제투자자주문내역요청 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 110 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Request_Investor_Type_Code[1];         /*    9 요청투자자구분코드 (String 1) */
    char Start_Date[8];                         /*   10 시작일자 (String 8) */
    char End_Date[8];                           /*   11 종료일자 (String 8) */
    char Request_Official_Document_Number[30];  /*   12 요청공문번호 (String 30) */
} TSVIIS00400_DATA;

#endif  /* _KRX_TSVIIS00400_H */
