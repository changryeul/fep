#ifndef _KRX_TRDESP00201_H
#define _KRX_TRDESP00201_H

/* TRDESP00201 기초자산상품군 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 1200 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence[11];                  /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Transmit_Date[8];                      /*    3 전송일자 (String 8) */
    char Emsg_Complt_Yn[1];                     /*    4 전문완료여부 ★신규 (String 1) */
    char Business_Date[8];                      /*    5 영업일자 (String 8) */
    char Ulygrp_Id[4];                          /*    6 기초자산상품군ID (String 4) */
    char Uly_Id[3];                             /*    7 기초자산ID (String 3) */
    char Uly_Cd[2];                             /*    8 기초자산코드 (String 2) */
    char Mrgn_Offset_Rto[11];                   /*    9 증거금OFFSET비율 (Float 11) */
    char Rel_Scale_Rto[11];                     /*   10 상대적규모비율 (Float 11) */
    char Real_Uly_Id[3];                        /*   11 실제기초자산ID (String 3) */
    char Filler_Value[1127];                    /*   12 필러값 (String 1127) */
} TRDESP00201_DATA;

#endif  /* _KRX_TRDESP00201_H */
