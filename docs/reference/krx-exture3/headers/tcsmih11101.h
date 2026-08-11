#ifndef _KRX_TCSMIH11101_H
#define _KRX_TCSMIH11101_H

/* TCSMIH11101 신고업무운영시간(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Mkt_Id[3];                             /*    5 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    6 회원번호 ★변경 (String 5) */
    char Report_Business_Type_Code[2];          /*    7 신고업무구분코드 ★변경 (String 2) */
    char Business_Start_Time[4];                /*    8 업무개시시각 (String 4) */
    char Business_End_Time[4];                  /*    9 업무종료시각 (String 4) */
    char Filler_Value[1151];                    /*   10 필러값 (String 1151) */
} TCSMIH11101_DATA;

#endif  /* _KRX_TCSMIH11101_H */
