#ifndef _KRX_TRDESP90801_H
#define _KRX_TRDESP90801_H

/* TRDESP90801 장운영정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TRDESP00801) */
/* DATA 길이 합계 = 58 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Board_Id[2];                           /*    6 보드ID (String 2) */
    char Tsc_Prod_Grp_Id[3];                    /*    7 장운영상품그룹ID (String 3) */
    char Brd_Evt_Id[3];                         /*    8 보드이벤트ID (String 3) */
    char Brd_Evt_Strt_Tm[9];                    /*    9 보드이벤트시작시각 (String 9) */
} TRDESP90801_DATA;

#endif  /* _KRX_TRDESP90801_H */
