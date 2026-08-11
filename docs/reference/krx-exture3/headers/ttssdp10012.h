#ifndef _KRX_TTSSDP10012_H
#define _KRX_TTSSDP10012_H

/* TTSSDP10012 ELW Knockout 평가기간 중 기초자산 고저가 정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 114 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Underlying_Asset_Trading_Date[8];      /*    4 기초자산매매일자 (String 8) */
    char Knockout_Occurrence_Date[8];           /*    5 KO발생일자 (String 8) */
    char Ante_Meridiem_Post_Meridiem_Type_Code[1];/*    6 오전오후구분코드 (String 1) */
    char Knockout_Issue_Code[12];               /*    7 KO종목코드 (String 12) */
    char Composition_Issue_Code[12];            /*    8 구성종목코드 (String 12) */
    char Idx_Id[12];                            /*    9 지수ID ★신규 (string 12) */
    char Highest_Price[11];                     /*   10 고가 (Float 11) */
    char Lowest_Price[11];                      /*   11 저가 (Float 11) */
    char Filler[9];                             /*   12 필러값 ★신규 (String 9) */
} TTSSDP10012_DATA;

#endif  /* _KRX_TTSSDP10012_H */
