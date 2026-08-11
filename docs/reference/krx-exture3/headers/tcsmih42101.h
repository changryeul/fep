#ifndef _KRX_TCSMIH42101_H
#define _KRX_TCSMIH42101_H

/* TCSMIH42101 거래증거금_종목별증거금률(증권시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 150 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Tracsaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Market_Identification[3];              /*    5 시장ID (String 3) */
    char Securities_Group_Identification[2];    /*    6 증권그룹ID (String 2) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Isu_Kor_Abbrv[40];                     /*    8 종목한글약명 (String 40) */
    char Trd_Mrgn_Rt[13];                       /*    9 거래증거금률 (String 13) */
    char Filler_Value[49];                      /*   10 필러값 (String 49) */
} TCSMIH42101_DATA;

#endif  /* _KRX_TCSMIH42101_H */
