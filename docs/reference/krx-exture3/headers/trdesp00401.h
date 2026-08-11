#ifndef _KRX_TRDESP00401_H
#define _KRX_TRDESP00401_H

/* TRDESP00401 선물옵션 상하한가 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 86 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Uly_Id[3];                             /*    7 기초자산ID (String 3) */
    char Underlying_Asset_Code[2];              /*    8 기초자산코드 (String 2) */
    char Issue_Code[12];                        /*    9 종목코드 (String 12) */
    char Prc_Lmt_Step[3];                       /*   10 가격제한단계 (long 3) */
    char Uplmtprc[11];                          /*   11 상한가(호가최고한도가격) (String 11) */
    char Lwlmtprc[11];                          /*   12 하한가(호가최저한도가격) (String 11) */
    char Ple_Yn[1];                             /*   13 가격제한확대여부 (string 1) */
} TRDESP00401_DATA;

#endif  /* _KRX_TRDESP00401_H */
