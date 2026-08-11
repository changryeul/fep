#ifndef _KRX_TCSMIR30401_H
#define _KRX_TCSMIR30401_H

/* TCSMIR30401 구분착오정정신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 132 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Declaration_Time[8];                   /*    6 신고시각 (String 8) */
    char Type_Error_Modification_Rejected_Reason_Code[2];/*    7 구분착오정정거부사유코드 (String 2) */
    char Account_Number[12];                    /*    8 계좌번호 (String 12) */
    char Declaration_Input_Type_Code[1];        /*    9 신고입력구분코드 (String 1) */
    char Trust_Principal_Type_Code[2];          /*   10 위탁자기구분코드 (String 2) */
    char Country_Code[3];                       /*   11 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   12 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   13 외국인투자자구분코드 (String 2) */
    char Member_User_Area[60];                  /*   14 회원사용영역 (String 60) */
} TCSMIR30401_DATA;

#endif  /* _KRX_TCSMIR30401_H */
