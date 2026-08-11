#ifndef _KRX_TTRKOP11301_H
#define _KRX_TTRKOP11301_H

/* TTRKOP11301 Kill Switch 처리 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 161 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 (String 2) */
    char Market_Identification[3];              /*    4 시장ID (String 3) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Account_Number[12];                    /*    6 계좌번호 (String 12) */
    char Sanctn_Releas_Tp_Cd[1];                /*    7 제재해제구분코드 (String 1) */
    char Ord_Media_Tp_Cd[1];                    /*    8 주문매체구분코드 (String 1) */
    char Order_Identification_Information[12];  /*    9 주문자식별정보 (String 12) */
    char Mac_Addr[12];                          /*   10 MAC주소 (String 12) */
    char Order_Date[8];                         /*   11 신고일자 (String 8) */
    char Mbr_Ord_Tm[9];                         /*   12 신고시각 (String 9) */
    char Member_Use_Area[60];                   /*   13 회원사용영역 (String 60) */
    char Ord_Acpt_Tm[12];                       /*   14 발동시각 ★변경 (String 12) */
    char Rej_Cd[2];                             /*   15 거부코드 (String 2) */
} TTRKOP11301_DATA;

#endif  /* _KRX_TTRKOP11301_H */
