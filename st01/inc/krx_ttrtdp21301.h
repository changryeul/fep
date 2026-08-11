#ifndef _KRX_TTRTDP21301_H
#define _KRX_TTRTDP21301_H
/*------------------------------------------------------------------------
#   전문   : TTRTDP21301 회원체결결과 - 송신(응답), 현/파 공용, DATA 233 bytes
#   규격   : KRX EXTURE 3.0 v3.24 (2026-07-30 주식선물옵션정기변경)
#   근거   : docs/reference/krx-exture3/ (interface-list.csv '길이(헤재외)'=233, '현/파')
#
#   회원사 호가(TCHODR1xxxx)의 체결 결과를 거래소가 송신. 파생 미체결 잔량
#   감소(po_1490_mp Make/Analyze_Che_Deriv) + 손익/잔고(Che_Rtn_Deriv)에 사용.
#   경쟁대량은 TTRTDP21302/21303 로 별도 전문.
#
#   [전문 관리 규약]
#     - 전문별 1개 헤더파일 / 필드명 = 항목영문명 Title_Case_언더스코어
#     - 고정폭 ASCII 전문이라 전 필드 char[len] (오프셋은 struct 순서로 보장)
#     - 공통 헤더(KRX_HEADER 82B)는 pa_struct.h 공용 → 여기엔 DATA부만
#     - ★변경 = 차세대(v3.24) 변경 항목
#     - sizeof(TTRTDP21301_DATA)==233 회귀가드: test/unit/test_krx_struct.c
------------------------------------------------------------------------*/
typedef struct {
    char Message_Sequence_Number[11];                    /*  1 메세지일련번호 (Long 11)   */
    char Transaction_Code[11];                           /*  2 트랜잭션코드 (String 11)   */
    char Me_Grp_No[2];                                   /*  3 ME그룹번호 (String 2)      */
    char Board_Id[2];                                    /*  4 보드ID (String 2)          */
    char Member_Number[5];                               /*  5 회원번호 (String 5)        */
    char Branch_Number[5];                               /*  6 지점번호 (String 5)        */
    char Order_Identification[10];                       /*  7 주문ID (String 10)         */
    char Original_Order_Identification[10];              /*  8 원주문ID (String 10)       */
    char Issue_Code[12];                                 /*  9 종목코드 (String 12)       */
    char Trading_Number[11];                             /* 10 체결번호 (Long 11)         */
    char Trading_Price[11];                              /* 11 체결가격 (Float 11)        */
    char Trading_Volumn[10];                             /* 12 체결수량 (Long 10)         */
    char Sess_Id[2];                                     /* 13 세션ID (String 2)          */
    char Trading_Date[8];                                /* 14 체결일자 (String 8)        */
    char Trading_Time[9];                                /* 15 체결시각 (String 9)        */
    char The_Nearby_Month_Trading_Price[11];             /* 16 근월물체결가격 (Float 11)  */
    char The_Future_Month_Trading_Price[11];             /* 17 원월물체결가격 (Float 11)  */
    char Ask_Bid_Type_Code[1];                           /* 18 매도매수구분코드 (String 1)*/
    char Account_Number[12];                             /* 19 계좌번호 (String 12)       */
    char Mm_Ord_Tp_Cd[1];                                /* 20 시장조성자호가구분코드 ★변경 (String 1) */
    char Trust_Company_Number[5];                        /* 21 위탁사번호 (String 5)      */
    char Substitute_Stock_Certificate_Account_Number[12];/* 22 대용주권계좌번호 (String 12) */
    char Member_Use_Area[60];                            /* 23 회원사용영역 (String 60)   */
    char Last_Ask_Bid_Type_Code[1];                      /* 24 최종매도매수구분코드 (String 1) */
}   TTRTDP21301_DATA;   /* DATA 길이 합계 = 233 */
#endif  /* _KRX_TTRTDP21301_H */
