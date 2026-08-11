#ifndef _KRX_TSVIIS00700_H
#define _KRX_TSVIIS00700_H

/* TSVIIS00700 회원사계좌번호요청 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 103 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Request_Date[8];                       /*    4 요청일자 (String 8) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Person_Information_Sequence_Number[10];/*    6 개인정보일련번호 (String 10) */
    char Member_Account_Number[20];             /*    7 회원사계좌번호 (String 20) */
    char Request_Official_Document_Number[30];  /*    8 요청공문번호 (String 30) */
} TSVIIS00700_DATA;

#endif  /* _KRX_TSVIIS00700_H */
