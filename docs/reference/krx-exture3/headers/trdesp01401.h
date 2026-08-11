#ifndef _KRX_TRDESP01401_H
#define _KRX_TRDESP01401_H

/* TRDESP01401 ELW기초자산구성 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 112 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Elw_Underlying_Asset_Sequence_Number[11];/*    8 ELW기초자산일련번호 (Long 11) */
    char Composition_Issue_Code[12];            /*    9 구성종목코드 (String 12) */
    char Compsition_Ratio[13];                  /*   10 구성비율 (Float 13) */
    char Idx_Id[12];                            /*   11 ELW지수ID (string 12) */
    char Filler[9];                             /*   12 필러값 (String 9) */
} TRDESP01401_DATA;

#endif  /* _KRX_TRDESP01401_H */
