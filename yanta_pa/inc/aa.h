typedef struct {
    char    BeginString[8];         /* 전문유형(API프로토콜의버젼)      */
    char    BodyLength[6];          /* 메시지길이(Body전체길이)         */
    char    MsgType[11];            /* 메시지타입                       */
                                    /* 세션메시지타입 및 TR Code        */
                                    /* "S"+"CH"+Type(3)+일련번호(5)     */
    char    MsgSeqNum[11];          /* 일련번호                         */
                                    /* 주문요청 Body첫번째항목 일련번호 */
                                    /* 주문응답 채널 최종 처리 일련번호 */
    char    SenderCompID[5];        /* 회원번호                         */
    char    DeliverToCompID[10];    /* 연계시도착회원사번호             */
    char    OnBehalfOfCompID[10];   /* 회신시송신회원사번호             */
    char    SendingTime[17];        /* 전송일시(YYYYMMDDHHMMSSMS)       */
    char    DataCnt[3];             /* 데이터건수                       */
    char    Encrypt[1];             /* 암호화유무(Y/N)                  */
}   KRX_HEADER;

