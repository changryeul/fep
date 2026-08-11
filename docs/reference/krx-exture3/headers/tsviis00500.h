#ifndef _KRX_TSVIIS00500_H
#define _KRX_TSVIIS00500_H

/* TSVIIS00500 예방조치 장중건전주문 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 181 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Member_Number[5];                      /*    4 회원번호 (String 5) */
    char Request_Date[8];                       /*    5 적출일자 (String 8) */
    char Exch_Id[2];                            /*    6 거래소ID (String 2) */
    char Market_Identification[3];              /*    7 시장ID (String 3) */
    char Detection_Tmzn_Type_Code[2];           /*    8 적출시간대구분코드 (String 2) */
    char Detection_Type_Code[2];                /*    9 적출유형코드 (String 2) */
    char Issue_Code[12];                        /*   10 종목코드 (String 12) */
    char Branch_Number[5];                      /*   11 지점번호 (String 5) */
    char Account_Number[12];                    /*   12 계좌번호 (String 12) */
    char Action_Contents[100];                  /*   13 조치내용 (String 100) */
} TSVIIS00500_DATA;

#endif  /* _KRX_TSVIIS00500_H */
