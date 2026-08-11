#ifndef _KRX_TTRMIP31303_H
#define _KRX_TTRMIP31303_H

/* TTRMIP31303 임의종료 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 131 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Issue_Code[12];                        /*    5 종목코드 (String 12) */
    char Random_End_Application_Type_Code[1];   /*    6 임의종료적용구분코드 (String 1) */
    char Random_End_Deemed_Trading_Information[11];/*    7 임의종료잠정체결가 (Float 11) */
    char Random_End_Deemed_Highest_Price[11];   /*    8 임의종료예상최고가 (Float 11) */
    char Random_End_Deemed_Highest_Price_Divergency_Rate[13];/*    9 임의종료예상최고가괴리율 (Float 13) */
    char Random_End_Deemed_Lowest_Price[11];    /*   10 임의종료예상최저가 (Float 11) */
    char Random_End_Deemed_Lowest_Price_Divergency_Rate[13];/*   11 임의종료예상최저가괴리율 (Float 13) */
    char Last_Price[11];                        /*   12 직전가격 (Float 11) */
    char Last_Price_Divergency_Rate[13];        /*   13 직전가격괴리율 (Float 13) */
    char Random_End_Release_Time[9];            /*   14 임의종료해제시각 (String 9) */
} TTRMIP31303_DATA;

#endif  /* _KRX_TTRMIP31303_H */
