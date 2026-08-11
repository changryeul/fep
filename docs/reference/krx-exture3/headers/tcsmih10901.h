#ifndef _KRX_TCSMIH10901_H
#define _KRX_TCSMIH10901_H

/* TCSMIH10901 외화평가가격(파생시장) - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (Long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 (String 1) */
    char Mbr_No[5];                             /*    5 회원번호 ★변경 (String 5) */
    char Curr_Std_Cd[12];                       /*    6 통화표준코드 (String 12) */
    char Curr_Iso_Cd[3];                        /*    7 통화ISO코드 (String 3) */
    char Bas_Exchrt_Prc[11];                    /*    8 기준환율시세 (Float 11) */
    char Hairct_Rto[4];                         /*    9 사정비율 (Long 4) */
    char Valu_Prc[11];                          /*   10 평가가격 (Float 11) */
    char Dp_Exchrt_Unit[4];                     /*   11 표시환율단위 (Long 4) */
    char Bas_Prc_Noti_Dd[8];                    /*   12 기준시세고시일자 (String 8) */
    char Filr_Val[1111];                        /*   13 필러값 (String 1111) */
} TCSMIH10901_DATA;

#endif  /* _KRX_TCSMIH10901_H */
