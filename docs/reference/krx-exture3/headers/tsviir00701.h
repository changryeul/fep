#ifndef _KRX_TSVIIR00701_H
#define _KRX_TSVIIR00701_H

/* TSVIIR00701 회원사계좌번호내역 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 76 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Request_Date[8];                       /*    4 요청일자 (String 8) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Member_Account_Number[20];             /*    6 회원사계좌번호 (String 20) */
    char Account_Number[12];                    /*    7 계좌번호 (String 12) */
    char Request_Official_Document_Number[1];   /*    8 회원사계좌번호상태구분코드 (String 1) */
} TSVIIR00701_DATA;

#endif  /* _KRX_TSVIIR00701_H */
