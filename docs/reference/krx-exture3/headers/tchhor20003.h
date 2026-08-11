#ifndef _KRX_TCHHOR20003_H
#define _KRX_TCHHOR20003_H

/* TCHHOR20003 장개시전 협의거래수탁거부신고 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* (변형: 레이아웃 공유 그룹 TCHHOR20001) */
/* DATA 길이 합계 = 71 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Me_Grp_No[2];                          /*    3 ME그룹번호 ★신규 (String 2) */
    char Board_Id[2];                           /*    4 보드ID (String 2) */
    char Mbr_No[5];                             /*    5 회원번호 (String 5) */
    char Isu_Cd[12];                            /*    6 종목코드 (String 12) */
    char Askbid_Tp_Cd[1];                       /*    7 매도매수구분코드 (String 1) */
    char Modcancl_Tp_Cd[1];                     /*    8 정정취소구분코드 (String 1) */
    char Ord_Dd[8];                             /*    9 호가일자 (String 8) */
    char Trd_Rpt_Id[9];                         /*   10 거래리포트ID (String 9) */
    char Acnt_Pw[9];                            /*   11 계좌비밀번호 (String 9) */
} TCHHOR20003_DATA;

#endif  /* _KRX_TCHHOR20003_H */
