#ifndef _KRX_TCSMIH11001_H
#define _KRX_TCSMIH11001_H

/* TCSMIH11001 금리,국채수익율(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Underlying_Asset_Code[2];              /*    5 기초자산코드 (String 2) */
    char Member_Number[5];                      /*    6 회원번호 ★변경 (String 5) */
    char Announcement_Institution_Type_Code[2]; /*    7 발표기관구분코드 (String 2) */
    char Bond_Issue_Code[12];                   /*    8 채권종목코드 (String 12) */
    char Disclosure_Time_Type_Code[2];          /*    9 공시시각구분코드 (String 2) */
    char Yield[13];                             /*   10 수익률 (Float 13) */
    char Bond_Price[16];                        /*   11 채권가격 (Float 16) */
    char Filler_Value[1117];                    /*   12 필러값 (String 1117) */
} TCSMIH11001_DATA;

#endif  /* _KRX_TCSMIH11001_H */
