#ifndef _KRX_TRDESP01201_H
#define _KRX_TRDESP01201_H

/* TRDESP01201 현물종목이벤트 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 77 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Event_Kind_Code[2];                    /*    8 이벤트종류코드 (String 2) */
    char Event_Occurrence_Reason_Code[4];       /*    9 이벤트발생사유코드 (String 4) */
    char Event_Start_Date[8];                   /*   10 이벤트개시일자 (String 8) */
    char Event_End_Date[8];                     /*   11 이벤트종료일자 (String 8) */
} TRDESP01201_DATA;

#endif  /* _KRX_TRDESP01201_H */
