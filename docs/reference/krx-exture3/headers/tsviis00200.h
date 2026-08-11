#ifndef _KRX_TSVIIS00200_H
#define _KRX_TSVIIS00200_H

/* TSVIIS00200 세부심리자료 요청(2차) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 224 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Request_Date[8];                       /*    5 요청일자 (String 8) */
    char Member_Number[5];                      /*    6 회원번호 (String 5) */
    char Branch_Number[5];                      /*    7 지점번호 (String 5) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Account_Name[80];                      /*    9 계좌명 (String 80) */
    char Resident_Registration_Number[20];      /*   10 주민등록번호 (String 20) */
    char Start_Date[8];                         /*   11 시작일자 (String 8) */
    char End_Date[8];                           /*   12 종료일자 (String 8) */
    char Account_Type_Code[2];                  /*   13 계좌구분코드 (String 2) */
    char Request_Official_Document_Number[30];  /*   14 요청공문번호 (String 30) */
    char Second_Data_Request_Contents[13];      /*   15 2차자료요청내용 (String 13) */
} TSVIIS00200_DATA;

#endif  /* _KRX_TSVIIS00200_H */
