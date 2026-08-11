#ifndef _KRX_TCSMIR30601_H
#define _KRX_TCSMIR30601_H

/* TCSMIR30601 사후증거금계좌신고및접수통보(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 185 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Mkt_Id[3];                             /*    4 시장ID ★변경 (String 3) */
    char Member_Number[5];                      /*    5 회원번호 (String 5) */
    char Declaration_Time[8];                   /*    6 신고시각 (String 8) */
    char Rejected_Code[2];                      /*    7 거부코드 (String 2) */
    char Declaration_Account_Number[12];        /*    8 신고계좌번호 (String 12) */
    char Declaration_Type_Detail_Type_Code[1];  /*    9 신고유형상세구분코드 (String 1) */
    char Country_Code[3];                       /*   10 국가코드 (String 3) */
    char Investor_Type_Code[4];                 /*   11 투자자구분코드 (String 4) */
    char Foreign_Investor_Type_Code[2];         /*   12 외국인투자자구분코드 (String 2) */
    char Trust_Principal_Type_Code[2];          /*   13 위탁자기구분코드 (String 2) */
    char Link_Spot_Account_Number[12];          /*   14 연계현물계좌번호 (String 12) */
    char Link_Spot_Account_Member_Number[5];    /*   15 연계현물계좌회원번호 (String 5) */
    char Link_After_The_Fact_General_Account_Number_1[12];/*   16 연계사후일반계좌1 (String 12) */
    char Link_After_The_Fact_General_Account_Number_2[12];/*   17 연계사후일반계좌2 (String 12) */
    char Link_After_The_Fact_General_Account_Number_3[12];/*   18 연계사후일반계좌3 (String 12) */
    char Member_User_Area[60];                  /*   19 회원사용영역 (String 60) */
} TCSMIR30601_DATA;

#endif  /* _KRX_TCSMIR30601_H */
