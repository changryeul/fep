#ifndef _KRX_TCSMIP46601_H
#define _KRX_TCSMIP46601_H

/* TCSMIP46601 회원사고객계좌부전질권내역정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 90 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sesuence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Margin_Market_Identification[3];       /*    5 증거금시장ID ★변경 (String 3) */
    char Trust_Principal_Integration_Type_Code[2];/*    6 위탁자기통합구분코드 (String 2) */
    char Account_Number[12];                    /*    7 계좌번호 (String 12) */
    char Issue_Code[12];                        /*    8 종목코드 (String 12) */
    char Quantity[16];                          /*    9 수량 (Float 16) */
    char Pledge_Member_Number[5];               /*   10 질권자 회원 번호 (String 5) */
    char Pre_Pledge_Member_Number[5];           /*   11 전질권자 회원 번호 (String 5) */
} TCSMIP46601_DATA;

#endif  /* _KRX_TCSMIP46601_H */
