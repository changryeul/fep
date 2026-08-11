#ifndef _KRX_TRDESP50401_H
#define _KRX_TRDESP50401_H

/* TRDESP50401 채권 기관코드 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 282 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Msg_Seq[11];                           /*    1 메세지일련번호 (long 11) */
    char Tr_Cd[11];                             /*    2 트랜잭션코드 (String 11) */
    char Trnsm_Dd[8];                           /*    3 전송일자 (String 8) */
    char Bz_Dd[8];                              /*    4 영업일자 (String 8) */
    char Inst_Cd[4];                            /*    5 기관코드 (String 4) */
    char Inst_Nm[80];                           /*    6 기관명 (String 80) */
    char Inst_Srt_Nm[40];                       /*    7 기관단축명 (String 40) */
    char Inst_Eng_Nm[80];                       /*    8 기관영문명 (String 80) */
    char Inst_Eng_Srt_Nm[40];                   /*    9 기관영문단축명 (String 40) */
} TRDESP50401_DATA;

#endif  /* _KRX_TRDESP50401_H */
