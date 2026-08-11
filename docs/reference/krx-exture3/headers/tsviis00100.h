#ifndef _KRX_TSVIIS00100_H
#define _KRX_TSVIIS00100_H

/* TSVIIS00100 특이 위탁자 계좌 기본정보 요청(1차) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 107 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Member_Number[5];                      /*    7 회원번호 (String 5) */
    char Branch_Number[5];                      /*    8 지점번호 (String 5) */
    char Account_Number[12];                    /*    9 계좌번호 (String 12) */
    char Account_Type_Code[2];                  /*   10 계좌구분코드 (String 2) */
    char Request_Official_Document_Number[30];  /*   11 요청공문번호 (String 30) */
} TSVIIS00100_DATA;

#endif  /* _KRX_TSVIIS00100_H */
