#ifndef _KRX_TSVIIS00300_H
#define _KRX_TSVIIS00300_H

/* TSVIIS00300 파생심리계좌기본정보,매매입출금내역에대한요청 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 140 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Request_Date[8];                       /*    6 요청일자 (String 8) */
    char Time[8];                               /*    7 시각 (String 8) */
    char Business_Type[10];                     /*    8 요청내용 (String 10) */
    char Investigation_Object_Start_Date[8];    /*    9 심리대상개시일자 (String 8) */
    char Investigation_Object_End_Date[8];      /*   10 심리대상종료일자 (String 8) */
    char Member_Number_2[5];                    /*   11 회원번호 (String 5) */
    char Branch_Number[5];                      /*   12 지점번호 (String 5) */
    char Account_Number[12];                    /*   13 계좌번호 (String 12) */
    char Submit_Deadline_Date[8];               /*   14 제출기한일자 (String 8) */
    char Data_Request_Official_Document_Number[30];/*   15 자료요청공문번호 (String 30) */
} TSVIIS00300_DATA;

#endif  /* _KRX_TSVIIS00300_H */
