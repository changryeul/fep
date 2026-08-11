#ifndef _KRX_TCSMIH43601_H
#define _KRX_TCSMIH43601_H

/* TCSMIH43601 장중추가증거금_증거금기준가격(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 185 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Round_Number[4];                       /*    5 회차번호 (Long 4) */
    char Round_Time[9];                         /*    6 회차시각 (String 9) */
    char Calculate_Base_Time[9];                /*    7 데이터산출기준시각 (String 9) */
    char Market_Identification[3];              /*   11 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*   12 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*   13 종목코드 (String 12) */
    char Margin_Base_Price[18];                 /*   14 증거금기준가격 (Float 18) */
    char Filler_Value[97];                      /*   15 필러값 (String 97) */
} TCSMIH43601_DATA;

#endif  /* _KRX_TCSMIH43601_H */
