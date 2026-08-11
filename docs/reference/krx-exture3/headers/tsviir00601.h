#ifndef _KRX_TSVIIR00601_H
#define _KRX_TSVIIR00601_H

/* TSVIIR00601 투자자식별정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 184 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Account_Number[12];                    /*    5 계좌번호 (String 12) */
    char Customer_Crypto_Identification_Information[128];/*    6 투자자식별정보 (String 128) */
    char Account_Opening_Date[8];               /*    7 계좌개설일자 (String 8) */
    char New_Or_Modify_Type_Code[1];            /*    8 신규정정구분코드 (String 1) */
} TSVIIR00601_DATA;

#endif  /* _KRX_TSVIIR00601_H */
