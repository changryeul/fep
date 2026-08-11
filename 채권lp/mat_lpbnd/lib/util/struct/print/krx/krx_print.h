/***********************************************************************************************
 *  System          : XMINS(eXtensible Multiple Interface and Solution)
 *  Program ID      : krx_struct.h
 *  Description     : define structure
 *  Author          : (주)IMECO
 *  Notice          :
 *  Revision        :
 ************************************************************************************************/
#ifndef     __KRX_STRUCT_H
#define     __KRX_STRUCT_H

#define     API_VERSION         "KMAPv2.0"          /* API 버전                               */
#define     DELIVERTOCOMPID     "          "        /* 연계시 도착 회원사 번호(향후추가 예정) */
#define     ONBEHALFOFCOMPID    "          "        /* 회신시 송신 회원사 번호(향후추가 예정) */

#define     OPEN_TIME           3  * 1000
#define     DATA_TIME           5  * 1000
#define     LIVE_TIME           5  * 1000
#define     LIVE_WAIT_TIME      6
#define     B_LIVE_TIME         10 * 1000
#define     LIRE_TIME           15 * 1000
#define     JEND_TIME           60 * 1000
#define     FOCL_TIME           65 * 1000
#define     RPLY_TIME           30 * 1000

#define     SIG_DUM_FIFO        0
#define     SIG_TCP_RECV        1
#define     SIG_DAT_FIFO        2

#define     MAX_TARGET_IP       2

#define     SZ_RJCODE           4
#define     SZ_TRCODE           11
#define     SZ_LASTSEQ          11

#define     POLL_SEND_CNT       2

/*------------------------------------------------------------------------------------------------
 * KRX head structure  ( 82 byte)
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char            sBeginString     [ 8];    /*  전문유형                                        */
    char            sBodyLength      [ 6];    /*  메시지길이                                      */
    char            sMsgType         [11];    /*  메시지 타입                                     */
    char            sMsgSeqNum       [11];    /*  일련번호                                        */
    char            sSenderCompID    [ 5];    /*  회원번호                                        */
    char            sDeliverToCompID [10];    /*  연계시도착 회원사 번호                          */
    char            sOnBehalfOfCompID[10];    /*  회신시송신 회원사 번호                          */
    char            sSendingDate     [ 8];    /*  전송일자(YYYYMMDD)                              */
    char            sSendingTime     [ 9];    /*  전송일시(HHMMDDmmm)                             */
    char            sDataCnt         [ 3];    /*  데이터 건수                                     */
    char            sEncrypt         [ 1];    /*  암호화 유무                                     */
} T_KRX_HEAD;
#define SZ_KRX_HEAD     (sizeof(T_KRX_HEAD))

/*------------------------------------------------------------------------------------------------
 * KRX HEART
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [4];    /* 거부 사유 코드                                    */
} T_KRX_HEARTBEAT;
#define SZ_KRX_HEARTBEAT     (sizeof(T_KRX_HEARTBEAT))

/*------------------------------------------------------------------------------------------------
 * KRX LOGON 요청/응답 :
 *------------------------------------------------------------------------------------------------*/
/* 로그온 요청 : 회원사 -> 거래소  */
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sUserId         [10];    /* 거래소에서 부여한 ID                             */
    char            sPassword       [30];
    char            sEncrypt        [ 1];    /* 암복호화 적용여부                                */
} T_KRX_Q_LOGON;
#define  SZ_KRX_Q_LOGON      (sizeof(T_KRX_Q_LOGON))

/* 로그온 응답 : 회원사 <- 거래소   */
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [ 4];    /* 거부 사유 코드                                   */
    char            sEncrypt        [ 1];    /* 암복호화 적용여부                                */
} T_KRX_R_LOGON;
#define SZ_KRX_R_LOGON       (sizeof(T_KRX_R_LOGON))

/*------------------------------------------------------------------------------------------------
 * KRX 업무개시 요청/응답 : 주문, 대량매매주문, 신고, 조회, 일괄송수신, 종합감리송수신
 *------------------------------------------------------------------------------------------------*/
/* 업무개시 요청/응답 : 회원사 <-> 거래소   */
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [ 4];    /* 거부사유코드                                     */
    char            sTrCd           [11];    /* 초기는 space, 재업무개시는 이전 송/수신 Tr-Code  */
    char            sLastSeq        [11];    /* 최종 처리 일련번호                               */
} T_KRX_QR_OPEN;
#define SZ_KRX_QR_OPEN      (sizeof(T_KRX_QR_OPEN))

/*------------------------------------------------------------------------------------------------
 * KRX 업무개시 요청/응답 : 체결, 장운영, DropCopy(채권)
 *------------------------------------------------------------------------------------------------*/
/* 업무개시 요청/응답 : 회원사 <-> 거래소   */
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [ 4];    /* 거부사유코드                                    */
    char            sMeGrpCnt       [ 2];    /* 매칭그룹 개수, 최초 LOGON시 0으로 SET           */
                                             /* 거래소 응답수신후 해당하는 매칭그룹수 SEt       */
    char            sMeGrpSeq   [10][11];    /* 매칭그룹#01~10 최종처리 일련번호                */
} T_KRX_QR_OPEN_1;
#define SZ_KRX_QR_OPEN_1      (sizeof(T_KRX_QR_OPEN_1))

/*------------------------------------------------------------------------------------------------
 * KRX 재송신 요청/응답 DATA.
 *------------------------------------------------------------------------------------------------*/
/* KRX HEAD (82 byte) + OPENING (11 byte) : 회원사 -> 거래소                                */
typedef struct
{
    T_KRX_HEAD     tHead;
    char           sLastSeq  [11];       /* 최종 처리 일련번호                              */
} T_KRX_Q_RESEND;
#define SZ_KRX_Q_RESEND      (sizeof(T_KRX_Q_RESEND))

/* KRX HEAD (82 byte) + OPENING (14 byte) : 회원사 <- 거래소                                */
typedef struct
{
    T_KRX_HEAD               tHead;
    char    sRjtCd           [ 4];       /* 거부사유코드                                    */
    char    sLastSeq         [11];       /* 최종 처리 일련번호                              */
} T_KRX_R_RESEND;
#define SZ_KRX_R_RESEND      (sizeof(T_KRX_R_RESEND))

/*------------------------------------------------------------------------------------------------
 * KRX 일련번호 요청/응답 DATA.  (11 byte)
 *------------------------------------------------------------------------------------------------*/
/* KRX HEAD (82 byte)                                      */
typedef struct
{
    T_KRX_HEAD               tHead;
} T_KRX_Q_SEQUENCE;
#define SZ_KRX_Q_SEQUENCE          (sizeof(T_KRX_Q_SEQUENCE))

/* KRX HEAD (82 byte) + 일련번호 응답 (11 byte) : 회원사 <- 거래소                              */
typedef struct
{
    T_KRX_HEAD        tHead;
    char              sLastSeq  [11];       /* 최종 처리 일련번호                               */
} T_KRX_R_SEQUENCE;
#define SZ_KRX_R_SEQUENCE          (sizeof(T_KRX_R_SEQUENCE))

/*----------------------------------------------------------------------------------------------
 * KRX LOGOUT 요청/응답
 *----------------------------------------------------------------------------------------------*/
/* LOGOUT 요청 : 회원사 -> 거래소 */
typedef struct
{
    T_KRX_HEAD      tHead;
} T_KRX_Q_LOGOUT;
#define SZ_KRX_Q_LOGOUT     (sizeof(T_KRX_Q_LOGOUT))

/*LOGOUT 응답  : 회원사 <- 거래소 */
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [ 4];    /* 거부 사유 코드                                 */
} T_KRX_R_LOGOUT;
#define SZ_KRX_R_LOGOUT     (sizeof(T_KRX_R_LOGOUT))

/*------------------------------------------------------------------------------------------------
 * 주문 (261 byte)
 *------------------------------------------------------------------------------------------------
 * 신규호가 :             TCHODR10001
 * 정정호가 :             TCHODR10002
 * 취소호가 :             TCHODR10003
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                    */
    char    sTrCd                   [11];    /* 트랜잭션코드                                      */
    char    sBoardId                [ 2];    /* 보드ID                                            */
    char    sCompNo                 [ 5];    /* 회원번호                                          */
    char    sBrnNo                  [ 5];    /* 지점번호                                          */
    char    sOrderNo                [10];    /* 주문ID                                            */
    char    sOrgOrderNo             [10];    /* 원주문ID                                          */
    char    sJCode                  [12];    /* 종목코드                                          */
    char    sMdsGb                  [ 1];    /* 매도매수구분코드                                  */
    char    sOrderGb                [ 1];    /* 정정취소구분코드                                  */
    char    sAcctNo                 [12];    /* 계좌번호                                          */
    char    sOrderQty               [10];    /* 호가수량                                          */
    char    sOrderPrice             [11];    /* 호가가격                                          */
    char    sOrderType              [ 1];    /* 호가유형코드                                      */
    char    sOrderCond              [ 1];    /* 호가조건코드                                      */
    char    sMktOrderGb             [11];    /* 시장조성자호가구분번호                            */
    char    sJasaId                 [ 5];    /* 자사주신고서ID                                    */
    char    sJasaTradeGb            [ 1];    /* 자사주매매방법코드                                */
    char    sMdType                 [ 2];    /* 매도유형코드                                      */
    char    sSyGb                   [ 2];    /* 신용구분코드                                      */
    char    sWitakGb                [ 2];    /* 위탁자기구분코드                                  */
    char    sWitakCompNo            [ 5];    /* 위탁사번호                                        */
    char    sPtGb                   [ 2];    /* PT구분코드                                        */
    char    sDaeAcctNo              [12];    /* 대용주권계좌번호                                  */
    char    sAcctGb                 [ 2];    /* 계좌구분코드                                      */
    char    sAcctMargType           [ 2];    /* 계좌증거금유형코드                                */
    char    sNationCd               [ 3];    /* 국가코드                                          */
    char    sTujaCd                 [ 4];    /* 투자자구분코드                                    */
    char    sFrgTujaGb              [ 2];    /* 외국인투자자구분코드                              */
    char    sMediaGb                [ 1];    /* 주문매체구분코드                                  */
    char    sOrderInfo              [12];    /* 주문자식별정보                                    */
    char    sMacAddr                [12];    /* MAC주소                                           */
    char    sDate                   [ 8];    /* 호가일자                                          */
    char    sOrderTime              [ 9];    /* 회원사주문시각                                    */
    char    sCustomerInfo           [60];    /* 회원사용영역                                      */
    char    sPgmDeclareCd           [ 1];    /* 프로그램호가신고구분코드                          */
} T_KRX_ORDER;
#define SZ_KRX_ORDER    (sizeof(T_KRX_ORDER))

typedef struct
{
    T_KRX_HEAD              tHead;
    T_KRX_ORDER             tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_ORDER];    /* SPACE                   */
} T_KRX_ORDER_FMT;
#define SZ_KRX_ORDER_FMT    (sizeof(T_KRX_ORDER_FMT))

typedef struct
{
    T_KRX_ORDER             tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_ORDER];    /* SPACE                   */
} T_KRX_ORDER_400;
#define SZ_KRX_ORDER_400    (sizeof(T_KRX_ORDER_400))

/*------------------------------------------------------------------------------------------------
 * 주문응답 : 거래소 -> 회원사
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    T_KRX_HEAD      tHead;
    char            sRjtCd          [ 4];  /* 거부사유코드                                       */
    char            sLastSeq        [11];  /* 처리 일련번호                                      */
} T_KRX_ORDER_RESP;
#define SZ_KRX_ORDER_RESP    (sizeof(T_KRX_ORDER_RESP))

/*------------------------------------------------------------------------------------------------
 * Combination 주문 (362 byte)
 *------------------------------------------------------------------------------------------------
 * 신규호가 :  TCHCOR10001 - 선물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                    */
    char    sTrCd                   [11];    /* 트랜잭션코드                                      */
    char    sBoardId                [ 2];    /* 보드ID                                            */
    char    sCompNo                 [ 5];    /* 회원번호                                          */
    char    sBrnNo                  [ 5];    /* 지점번호                                          */
    char    sAcctNo                 [12];    /* 계좌번호                                          */
    char    sOrderType              [ 1];    /* 호가유형코드                                      */
    char    sOrderCond              [ 1];    /* 호가조건코드                                      */
    char    sOrderCnt               [ 2];    /* 주문개수                                          */
    char    sOrderNo1               [10];    /* 주문ID1                                           */
    char    sJCode1                 [12];    /* 종목코드1                                         */
    char    sMdsGb1                 [ 1];    /* 매도매수구분코드1                                 */
    char    sOrderQty1              [10];    /* 호가수량1                                         */
    char    sOrderPrice1            [11];    /* 호가가격1                                         */
    char    sOrderNo2               [10];    /* 주문ID2                                           */
    char    sJCode2                 [12];    /* 종목코드2                                         */
    char    sMdsGb2                 [ 1];    /* 매도매수구분코드2                                 */
    char    sOrderQty2              [10];    /* 호가수량2                                         */
    char    sOrderPrice2            [11];    /* 호가가격2                                         */
    char    sOrderNo3               [10];    /* 주문ID3                                           */
    char    sJCode3                 [12];    /* 종목코드3                                         */
    char    sMdsGb3                 [ 1];    /* 매도매수구분코드3                                 */
    char    sOrderQty3              [10];    /* 호가수량3                                         */
    char    sOrderPrice3            [11];    /* 호가가격3                                         */
    char    sOrderNo4               [10];    /* 주문ID4                                           */
    char    sJCode4                 [12];    /* 종목코드4                                         */
    char    sMdsGb4                 [ 1];    /* 매도매수구분코드4                                 */
    char    sOrderQty4              [10];    /* 호가수량4                                         */
    char    sOrderPrice4            [11];    /* 호가가격4                                         */
    char    sWitakGb                [ 2];    /* 위탁자기구분코드                                  */
    char    sWitakCompNo            [ 5];    /* 위탁사번호                                        */
    char    sPtGb                   [ 2];    /* PT구분코드                                        */
    char    sDaeAcctNo              [12];    /* 대용주권계좌번호                                  */
    char    sAcctGb                 [ 2];    /* 계좌구분코드                                      */
    char    sAcctMargType           [ 2];    /* 계좌증거금유형코드                                */
    char    sNationCd               [ 3];    /* 국가코드                                          */
    char    sTujaCd                 [ 4];    /* 투자자구분코드                                    */
    char    sFrgTujaGb              [ 2];    /* 외국인투자자구분코드                              */
    char    sMediaGb                [ 1];    /* 주문매체구분코드                                  */
    char    sOrderInfo              [12];    /* 주문자식별정보                                    */
    char    sMacAddr                [12];    /* MAC주소                                           */
    char    sDate                   [ 8];    /* 호가일자                                          */
    char    sOrderTime              [ 9];    /* 회원사주문시각                                    */
    char    sCustomerInfo           [60];    /* 회원사용영역                                      */
} T_COMBINATION_ORDER;
#define SZ_COMBINATION_ORDER    (sizeof(T_COMBINATION_ORDER))

typedef struct
{
    T_COMBINATION_ORDER     tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_COMBINATION_ORDER];    /* SPACE              */
} T_COMBINATION_ORDER_400;
#define SZ_COMBINATION_ORDER_400    (sizeof(T_COMBINATION_ORDER_400))

/*------------------------------------------------------------------------------------------------
 * KILL SWITCH (147 byte)
 *------------------------------------------------------------------------------------------------
 * Kill Switch 입력 :  TCHKOR10001
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                    */
    char    sTrCd                   [11];    /* 트랜잭션코드                                      */
    char    sMarketId               [ 3];    /* 시장ID                                            */
    char    sBoardId                [ 2];    /* 보드ID                                            */
    char    sCompNo                 [ 5];    /* 회원번호                                          */
    char    sAcctNo                 [12];    /* 계좌번호                                          */
    char    sSanctnReleasTPCd       [ 1];    /* 제재해제구분코드                                  */
    char    sMediaGb                [ 1];    /* 주문매체구분코드                                  */
    char    sOrderInfo              [12];    /* 주문자식별정보                                    */
    char    sMacAddr                [12];    /* MAC주소                                           */
    char    sDate                   [ 8];    /* 신고일자                                          */
    char    sOrderTime              [ 9];    /* 신고시각                                          */
    char    sCustomerInfo           [60];    /* 회원사용영역                                      */
} T_KILL_SWITCH;
#define SZ_KILL_SWITCH    (sizeof(T_KILL_SWITCH))

typedef struct
{
    T_KILL_SWITCH           tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KILL_SWITCH];    /* SPACE                    */
} T_KILL_SWITCH_400;
#define SZ_KILL_SWITCH_400    (sizeof(T_KILL_SWITCH_400))

/*------------------------------------------------------------------------------------------------
 * 체결 (242 byte)
 *------------------------------------------------------------------------------------------------
 * 기준체결         : TTRTDP21301 - 현물/선물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                    */
    char    sTrCd                   [11];    /* 트랜잭션코드                                      */
    char    sMeGrpNo                [ 2];    /* ME그룹번호                                        */
    char    sBoardId                [ 2];    /* 보드ID                                            */
    char    sCompNo                 [ 5];    /* 회원번호                                          */
    char    sBrnNo                  [ 5];    /* 지점번호                                          */
    char    sOrderNo                [10];    /* 주문ID                                            */
    char    sOrgOrderNo             [10];    /* 원주문ID                                          */
    char    sJCode                  [12];    /* 종목코드                                          */
    char    sChNo                   [11];    /* 체결번호                                          */
    char    sChPrice                [11];    /* 체결가격                                          */
    char    sChAmt                  [10];    /* 체결수량                                          */
    char    sChType                 [ 2];    /* 세션ID                                            */
    char    sChDate                 [ 8];    /* 체결일자                                          */
    char    sChTime                 [ 9];    /* 체결시각                                          */
    char    sNearChPrice            [11];    /* 근월물체결가격                                    */
    char    sFarChPrice             [11];    /* 원월물체결가격                                    */
    char    sMdsGb                  [ 1];    /* 매도매수구분코드                                  */
    char    sAcctNo                 [12];    /* 계좌번호                                          */
    char    sMktOrderGb             [11];    /* 시장조성자호가구분번호                            */
    char    sWitakCompNo            [ 5];    /* 위탁사번호                                        */
    char    sDaeAcctNo              [12];    /* 대용주권계좌번호                                  */
    char    sCustomerInfo           [60];    /* 회원사용영역                                      */
} T_KRX_SETTLE;
#define SZ_KRX_SETTLE       (sizeof(T_KRX_SETTLE))

typedef struct
{
    T_KRX_HEAD              tHead;
    T_KRX_SETTLE            tData;
} T_KRX_SETTLE_FMT;
#define SZ_KRX_SETTLE_FMT   (sizeof(T_KRX_SETTLE_FMT))

typedef struct
{
    T_KRX_SETTLE            tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_SETTLE];    /* SPACE                     */
} T_KRX_SETTLE_400;
#define SZ_KRX_SETTLE_400   (sizeof(T_KRX_SETTLE_400))

/*------------------------------------------------------------------------------------------------
 * 회원처리호가 (287 byte)
 *------------------------------------------------------------------------------------------------
 * 정상                     : TTRODP11301 - 현물/선물
 * 거부                     : TTRODP11321 - 현물/선물
 * 자동취소                 : TTRODP11303 - 현물/선물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                   */
    char    sTrCd                   [11];    /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];    /* ME그룹번호                                       */
    char    sBoardId                [ 2];    /* 보드ID                                           */
    char    sCompNo                 [ 5];    /* 회원번호                                         */
    char    sBrnNo                  [ 5];    /* 지점번호                                         */
    char    sOrderNo                [10];    /* 주문ID                                           */
    char    sOrgOrderNo             [10];    /* 원주문ID                                         */
    char    sJCode                  [12];    /* 종목코드                                         */
    char    sMdsGb                  [ 1];    /* 매도매수구분코드                                 */
    char    sOrderGb                [ 1];    /* 정정취소구분코드                                 */
    char    sAcctNo                 [12];    /* 계좌번호                                         */
    char    sOrderQty               [10];    /* 호가수량                                         */
    char    sOrderPrice             [11];    /* 호가가격                                         */
    char    sOrderType              [ 1];    /* 호가유형코드                                     */
    char    sOrderCond              [ 1];    /* 호가조건코드                                     */
    char    sMktOrderGb             [11];    /* 시장조성자호가구분번호                           */
    char    sJasaId                 [ 5];    /* 자사주신고서ID                                   */
    char    sJasaTradeGb            [ 1];    /* 자사주매매방법코드                               */
    char    sMdType                 [ 2];    /* 매도유형코드                                     */
    char    sSyGb                   [ 2];    /* 신용구분코드                                     */
    char    sWitakGb                [ 2];    /* 위탁자기구분코드                                 */
    char    sWitakCompNo            [ 5];    /* 위탁사번호                                       */
    char    sPtGb                   [ 2];    /* PT구분코드                                       */
    char    sDaeAcctNo              [12];    /* 대용주권계좌번호                                 */
    char    sAcctGb                 [ 2];    /* 계좌구분코드                                     */
    char    sAcctMargType           [ 2];    /* 계좌증거금유형코드                               */
    char    sNationCd               [ 3];    /* 국가코드                                         */
    char    sTujaCd                 [ 4];    /* 투자자구분코드                                   */
    char    sFrgTujaGb              [ 2];    /* 외국인투자자구분코드                             */
    char    sMediaGb                [ 1];    /* 주문매체구분코드                                 */
    char    sOrderInfo              [12];    /* 주문자식별정보                                   */
    char    sMacAddr                [12];    /* MAC주소                                          */
    char    sDate                   [ 8];    /* 호가일자                                         */
    char    sOrderTime              [ 9];    /* 회원사주문시각                                   */
    char    sCustomerInfo           [60];    /* 회원사용영역                                     */
    char    sOrderRecvTime          [ 9];    /* 호가접수시각                                     */
    char    sRealOrderQty           [10];    /* 실정정취소호가수량                               */
    char    sAutoCancelGb           [ 1];    /* 자동취소처리구분코드                             */
    char    sRejectCd               [ 4];    /* 호가거부사유코드                                 */
    char    sPgmDeclareCd           [ 1];    /* 프로그램호가신고구분코드                         */
} T_KRX_CONFIRM;
#define SZ_KRX_CONFIRM   (sizeof(T_KRX_CONFIRM))

typedef struct
{
    T_KRX_HEAD              tHead;
    T_KRX_CONFIRM           tData;
} T_KRX_CONFIRM_FMT;
#define SZ_KRX_CONFIRM_FMT  (sizeof(T_KRX_CONFIRM_FMT))

typedef struct
{
    T_KRX_CONFIRM           tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_CONFIRM];    /* SPACE                   */
} T_KRX_CONFIRM_400;
#define SZ_KRX_CONFIRM_400   (sizeof(T_KRX_CONFIRM_400))

/*------------------------------------------------------------------------------------------------
 * Kill Switch 처리 (162 byte)
 *------------------------------------------------------------------------------------------------
 * 정상 : TTRKOP11301
 * 거부 : TTRKOP11302
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 메세지일련번호                                    */
    char    sTrCd                   [11];    /* 트랜잭션코드                                      */
    char    sMeGrpNo                [ 2];    /* ME그룹번호                                        */
    char    sMarketId               [ 3];    /* 시장ID                                            */
    char    sBoardId                [ 2];    /* 보드ID                                            */
    char    sCompNo                 [ 5];    /* 회원번호                                          */
    char    sAcctNo                 [12];    /* 계좌번호                                          */
    char    sSanctnReleasTPCd       [ 1];    /* 제재해제구분코드                                  */
    char    sMediaGb                [ 1];    /* 주문매체구분코드                                  */
    char    sOrderInfo              [12];    /* 주문자식별정보                                    */
    char    sMacAddr                [12];    /* MAC주소                                           */
    char    sDate                   [ 8];    /* 신고일자                                          */
    char    sOrderTime              [ 9];    /* 신고시각                                          */
    char    sCustomerInfo           [60];    /* 회원사용영역                                      */
    char    sOrderRecvTime          [ 9];    /* 접수시각                                          */
    char    sRejectCd               [ 4];    /* 거부코드                                          */
} T_KILL_SWITCH_CONFIRM;
#define SZ_KILL_SWITCH_CONFIRM    (sizeof(T_KILL_SWITCH_CONFIRM))

typedef struct
{
    T_KILL_SWITCH_CONFIRM   tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KILL_SWITCH_CONFIRM];    /* SPACE            */
} T_KILL_SWITCH_CONFIRM_400;
#define SZ_KILL_SWITCH_CONFIRM_400    (sizeof(T_KILL_SWITCH_CONFIRM_400))

/*------------------------------------------------------------------------------------------------
 * 체결 인터페이스 종료 (153 byte)
 *------------------------------------------------------------------------------------------------
 * 체결 인터페이스 종료 : TCHEDP99000 - 현물/선물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 데이타일련번호                                   */
    char    sTrCd                   [11];    /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];    /* 매칭그룹 번호                                    */
    char    sMeGrpCnt               [ 2];    /* 매칭그룹 개수                                    */
    char    sMeGrpSeq           [10][11];    /* 매칭그룹#01~10 최종처리 일련번호                 */
    char    sDate                   [ 8];    /* 전송일자                                         */
    char    sTime                   [ 9];    /* 전송시각                                         */
} T_SETTLE_IF_END;
#define SZ_SETTLE_IF_END    (sizeof(T_SETTLE_IF_END))

typedef struct
{
    T_SETTLE_IF_END         tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_SETTLE_IF_END];    /* SPACE                 */
} T_SETTLE_IF_END_400;
#define SZ_SETTLE_IF_END_400    (sizeof(T_SETTLE_IF_END_400))

/*------------------------------------------------------
 * 회원처리항목(60 Byte)
 *------------------------------------------------------*/
typedef struct
{
    char    sSrvGb          [ 2];
    char    sBizGb          [ 1];
    char    sAcctGb         [ 1];
    char    sOrderGb        [ 8];
    char    sFiller1        [ 7];
    char    sSubId          [ 7];
    char    sFiller2        [ 2];
    char    sEtc            [17];           /* 2016.04.14 -by blue, 실시간 건수조회 관련 22 -> 17로 변경 */
    char    sSessGb         [ 5];           /* 2016.04.14 -by blue, 실시간 건수조회 관련                 */
    char    sTrPk           [ 3];
    char    sExGb           [ 1];
    char    sBrsFiller      [ 6];
} T_CUST_INFO;
#define SZ_CUST_INFO        (sizeof(T_CUST_INFO))

/*------------------------------------------------------------------------------------------------
 * 공개장운영 (89 byte) (77 -> 89)
 *------------------------------------------------------------------------------------------------
 * 공개장운영 : TTRMIP31301 - 현물/선물/채권
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 메세지일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sProdGrpId              [ 3];   /* 장운영상품그룹ID                                 */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sBoardEvtId             [ 3];   /* 보드이벤트ID                                     */
    char    sBoardEvtSrtTime        [ 9];   /* 보드이벤트시작시각                               */
    char    sBoardEvtAppGrpCd       [ 5];   /* 보드이벤트적용군코드                             */
    char    sSessSrtEndCd           [ 2];   /* 세션개시종료코드                                 */
    char    sSessId                 [ 2];   /* 세션ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sClassID                [11];   /* 상품ID                                           */
    char    sHaltRsnCode            [ 3];   /* 거래정지사유코드                                  */
    char    sHaltTypeCode           [ 1];   /* 거래정지발생유형코드                              */
    char    sApplStep               [ 2];   /* 적용단계                                         */ /* Add 20150615 */
    char    sBasIsuPleOccrCd        [ 1];   /* 기준종목가격확대발생코드                         */ /* Add 20150615 */
    char    sPleSchdlTm             [ 9];   /* 가격확대예정시각                                 */ /* Add 20150615 */
} T_KRX_JANG;
#define SZ_KRX_JANG    (sizeof(T_KRX_JANG))

typedef struct
{
    T_KRX_JANG              tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG];    /* SPACE                   */
} T_KRX_JANG_400;
#define SZ_KRX_JANG_400    (sizeof(T_KRX_JANG_400))

typedef struct
{
    T_KRX_HEAD              tHead;
    T_KRX_JANG              tData;
} T_KRX_JANG_FMT;
#define SZ_KRX_JANG_FMT    (sizeof(T_KRX_JANG_FMT))

/*------------------------------------------------------------------------------------------------
 * 주식종목정보 (146 byte)
 *------------------------------------------------------------------------------------------------
 * 주식종목정보 : TTRMIP32301 - 현물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];    /* 데이타일련번호                                   */
    char    sTrCd                   [11];    /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];    /* ME그룹번호                                       */
    char    sOpenInfoCode           [ 3];    /* 공개정보구분코드                                 */
    char    sProdGrpId              [ 3];    /* 상품그룹시장ID                                   */
    char    sJCode                  [12];    /* 종목코드                                         */
    char    sOpenTime               [ 9];    /* 공개시각                                         */
    char    sBasePrice              [11];    /* 기준가격                                         */
    char    sHighLmtPrc             [11];    /* 상한가                                           */
    char    sLowLmtPrc              [11];    /* 하한가                                           */
    char    sValuePrc               [11];    /* 평가가격                                         */
    char    sHighOrdPrc             [11];    /* 최고호가가격                                     */
    char    sLowOrdPrc              [11];    /* 최저호가가격                                     */
    char    sOpenPrcGb              [ 1];    /* 시가기준가여부                                   */
    char    sOffTypeCode            [ 2];    /* 락구분코드                                       */
    char    sParTypeCode            [ 2];    /* 액면가변경구분코드                               */
    char    sOrdQtyUnit             [ 6];    /* 매매수량단위                                     */
    char    sSharesNo               [16];    /* 상장주식수                                       */
    char    sArrantYN               [ 1];    /* 정리매매/관리종목여부                            */
    char    sPreHrYN                [ 1];    /* 장개시전시간외종가가능여부                       */
} T_KRX_JANG_INFO;
#define SZ_KRX_JANG_INFO    (sizeof(T_KRX_JANG_INFO))

typedef struct
{
    T_KRX_JANG_INFO         tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_INFO];    /* SPACE                   */
} T_KRX_JANG_INFO_400;
#define SZ_KRX_JANG_INFO_400    (sizeof(T_KRX_JANG_INFO_400))

/*------------------------------------------------------------------------------------------------
 * 기준가결정 (71 byte)
 *------------------------------------------------------------------------------------------------
 * 기준가결정 : TTRMIP31302 - 현물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sBasePrice              [11];   /* 기준가격                                         */
    char    sHighLmtPrc             [11];   /* 상한가                                           */
    char    sLowLmtPrc              [11];   /* 하한가                                           */
} T_KRX_JANG_BASE;
#define SZ_KRX_JANG_BASE    (sizeof(T_KRX_JANG_BASE))

typedef struct
{
    T_KRX_JANG_BASE         tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_BASE];    /* SPACE                   */
} T_KRX_JANG_BASE_400;
#define SZ_KRX_JANG_BASE_400    (sizeof(T_KRX_JANG_BASE_400))

/*------------------------------------------------------------------------------------------------
 * 임의종료 (131 byte)
 *------------------------------------------------------------------------------------------------
 * 임의종료 : TTRMIP31303 - 현물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sRandomType             [ 1];   /* 임의종료적용구분코드                             */
    char    sDeemedPrice            [11];   /* 임의종료잠정체결가                               */
    char    sDeemedHighPrc          [11];   /* 임의종료예상최고가                               */
    char    sDeemedHighRate         [13];   /* 임의종료예상최고가괴리율                         */
    char    sDeemedLowPrc           [11];   /* 임의종료예상최저가                               */
    char    sDeemedLowRate          [13];   /* 임의종료예상최저가괴리율                         */
    char    sLastPrice              [11];   /* 직전가격                                         */
    char    sLastRate               [13];   /* 직전가격괴리율                                   */
    char    sRandomEndTime          [ 9];   /* 임의종료해제시각                                 */
} T_KRX_JANG_STOP;
#define SZ_KRX_JANG_STOP    (sizeof(T_KRX_JANG_STOP))

typedef struct
{
    T_KRX_JANG_STOP         tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_STOP];    /* SPACE                   */
} T_KRX_JANG_STOP_400;
#define SZ_KRX_JANG_STOP_400    (sizeof(T_KRX_JANG_STOP_400))

/*------------------------------------------------------------------------------------------------
 * 종목마감 (105 byte)
 *------------------------------------------------------------------------------------------------
 * 종목마감 : TTRMIP31304 - 현물/선물/채권
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sClosePrice             [11];   /* 종목마감종가                                     */
    char    sClosePriceGb           [ 1];   /* 종목마감가격구분코드                             */
    char    sOTHighLmtPrc           [11];   /* 종목마감시간외단일가상한가                       */
    char    sOTLowLmtPrc            [11];   /* 종목마감시간외단일가상한가                       */
    char    sBIBasePrc              [11];   /* 종목마감매입인도기준가격                         */
    char    sBIHighLmtPrc           [11];   /* 종목마감매입인도단일가상한가                     */
    char    sBILowLmtPrc            [11];   /* 종목마감매입인도단일가상한가                     */
} T_KRX_JANG_END;
#define SZ_KRX_JANG_END    (sizeof(T_KRX_JANG_END))

typedef struct
{
    T_KRX_JANG_END          tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_END];    /* SPACE                   */
} T_KRX_JANG_END_400;
#define SZ_KRX_JANG_END_400    (sizeof(T_KRX_JANG_END_400))

/*------------------------------------------------------------------------------------------------
 * 배분정보 (49 byte)
 *------------------------------------------------------------------------------------------------
 * 배분정보 : TTRMIP31306 - 현물/선물/채권
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sAllocApplTpCd          [ 1];   /* 배분적용구분코드                                 */
    char    sAllocProcsTpCd         [ 1];   /* 배분처리구분코드                                 */
    char    sAllocReleasTime        [ 9];   /* 배분해제시각                                     */
} T_KRX_JANG_DIV;
#define SZ_KRX_JANG_DIV    (sizeof(T_KRX_JANG_DIV))

typedef struct
{
    T_KRX_JANG_DIV          tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_DIV];    /* SPACE                   */
} T_KRX_JANG_DIV_400;
#define SZ_KRX_JANG_DIV_400    (sizeof(T_KRX_JANG_DIV_400))

/*------------------------------------------------------------------------------------------------
 * VI (117 byte) (93 -> 117)
 *------------------------------------------------------------------------------------------------
 * VI : TTRMIP31307 - 현물
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sMeProcsTime            [ 9];   /* ME처리시각                                       */
    char    sViReleasTime           [ 9];   /* VI해제시각                                       */
    char    sViApplTpCd             [ 1];   /* VI적용구분코드                                   */
    char    sViKindCd               [ 1];   /* VI종류코드                                       */
    char    sStaticViTgBasPrc       [11];   /* 정적VI발동기준가격                               */ /* Add 20150615 */
    char    sDynmcViTgBasPrc        [11];   /* 동적VI발동기준가격                               */ /* Add 20150615 */
    char    sViTgPrc                [11];   /* VI발동가격                                       */ /* Add 20150615 */
    char    sStaticViTgDivrgRt      [13];   /* 정적VI발동가격괴리율                             */ /* Add 20150615 */
    char    sDynmcViTgDivrgRt       [13];   /* 동적VI발동가격괴리율                             */ /* Add 20150615 */
} T_KRX_JANG_VI;
#define SZ_KRX_JANG_VI    (sizeof(T_KRX_JANG_VI))

typedef struct
{
    T_KRX_JANG_VI           tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_VI];    /* SPACE                   */
} T_KRX_JANG_VI_400;
#define SZ_KRX_JANG_VI_400    (sizeof(T_KRX_JANG_VI_400))

/*------------------------------------------------------------------------------------------------
 * 동적가격제한 (70 byte)
 *------------------------------------------------------------------------------------------------
 * VI : TTRMIP31308 - 파생
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sMeProcsTime            [ 9];   /* ME처리시각                                       */
    char    sDynmcPrcLmtYn          [ 1];   /* 동적가격제한여부                                 */
    char    sDynmcUpLmtPrice        [11];   /* 동적상한가                                       */
    char    sDynmcLwLmtPrice        [11];   /* 동적하한가                                       */

} T_KRX_JANG_LMT;
#define SZ_KRX_JANG_LMT    (sizeof(T_KRX_JANG_LMT))

typedef struct
{
    T_KRX_JANG_LMT          tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_LMT];    /* SPACE                   */
} T_KRX_JANG_LMT_400;
#define SZ_KRX_JANG_LMT_400    (sizeof(T_KRX_JANG_LMT_400))

/*------------------------------------------------------------------------------------------------
 * 가격제한폭확대발동 (84 byte) (Add 20150615)
 *------------------------------------------------------------------------------------------------
 * 가격제한폭확대발동 : TTRMIP31309 - 파생 
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char    sSeq                    [11];   /* 데이타일련번호                                   */
    char    sTrCd                   [11];   /* 트랜잭션코드                                     */
    char    sMeGrpNo                [ 2];   /* ME그룹번호                                       */
    char    sBoardId                [ 2];   /* 보드ID                                           */
    char    sJCode                  [12];   /* 종목코드                                         */
    char    sMeProcsTm              [ 9];   /* ME처리시각                                       */
    char    sPrcExpnTm              [ 9];   /* 가격확대시각                                     */
    char    sPleUpLmtStep           [ 3];   /* 가격제한확대상한단계                             */
    char    sPleLwLmtStep           [ 3];   /* 가격제한확대하한단계                             */
    char    sUpLmtPrc               [11];   /* 상한가                                           */
    char    sLwLmtPrc               [11];   /* 하한가                                           */
} T_KRX_JANG_EXPN;
#define SZ_KRX_JANG_EXPN    (sizeof(T_KRX_JANG_EXPN))

typedef struct
{
    T_KRX_JANG_EXPN         tData;
    char                    sSpace[MAX_SR_LEN_400-SZ_KRX_JANG_EXPN];    /* SPACE                */
} T_KRX_JANG_EXPN_400;
#define SZ_KRX_JANG_EXPN_400    (sizeof(T_KRX_JANG_EXPN_400))

/*------------------------------------------------------------------------------------------------
 * KRX FOREIGN ORDER(SZ_KRX_ORDER(261) + SZ_KRX_ORDER_FRGN_ADD(40) = 301), 2018.11.19
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char                  sFrgnNo      [ 6];       /* 외국인 고유번호                */
    char                  sExtraOrderNo[10];       /* 예비주문번호                   */
    char                  sFiller      [24];       /* FILLER                         */
} T_KRX_ORDER_FRGN_ADD;
#define SZ_KRX_ORDER_FRGN_ADD    (sizeof(T_KRX_ORDER_FRGN_ADD))

typedef struct
{
    T_KRX_ORDER           tKrxOrder;
    T_KRX_ORDER_FRGN_ADD  tKrxOrderFrgnAdd;
} T_KRX_ORDER_FRGN;
#define SZ_KRX_ORDER_FRGN    (sizeof(T_KRX_ORDER_FRGN))

/*------------------------------------------------------------------------------------------------
 * KRX FOREIGN CONFIRM/SETTLE ADD(SZ_KRX_CONFIRM_FRGN_ADD(40)), 2018.11.19
 *------------------------------------------------------------------------------------------------*/
typedef struct
{
    char                  sFrgnNo      [ 6];       /* 외국인 고유번호                */
    char                  sRejectCd    [ 4];       /* 처리내용                       */
    char                  sFiller      [30];       /* FILLER                         */
} T_KRX_CONFIRM_FRGN_ADD;
#define SZ_KRX_CONFIRM_FRGN_ADD    (sizeof(T_KRX_CONFIRM_FRGN_ADD))

/*------------------------------------------------------------------------------------------------
 * KRX TR-CODE
 *------------------------------------------------------------------------------------------------*/
#define     Q_LOGON             1
#define     R_LOGON             2
#define     Q_HEARTBEAT         3
#define     R_HEARTBEAT         4
#define     Q_OPENING           5
#define     R_OPENING           6
#define     Q_LOGOUT            7
#define     R_LOGOUT            8
#define     Q_ORDER             9
#define     R_ORDER             10

#define     Q_SEQUENCE          11
#define     R_SEQUENCE          12
#define     Q_RESEND            13
#define     R_RESEND            14

#define     Q_INIT_SESSION      "SCHLIQ00101"    /* InitHandShake Q                             */
#define     R_INIT_SESSION      "SCHLIQ00102"    /* InitHandShake R                             */
#define     Q_UPDATE_SESSION    "SCHLIQ00103"    /* UpdateHandShake Q                           */
#define     R_UPDATE_SESSION    "SCHLIQ00104"    /* UpdateHandShake R                           */
#define     Q_FINAL_SESSION     "SCHLIQ00105"    /* FinalHandShake Q                            */

#define     Q_LOGON_S           "SCHLIQ00000"    /* 로그온 요청                                 */
#define     R_LOGON_S           "SCHLIR00000"    /* 로그온 응답                                 */
#define     Q_OPENING_S         "SCHOPQ00000"    /* 업무개시 요청                               */
#define     R_OPENING_S         "SCHOPR00000"    /* 업무개시 응답                               */
#define     Q_OPENING1_S        "SCHOPQ10000"    /* 업무개시 요청(체결,장운영 전용)             */
#define     R_OPENING1_S        "SCHOPR10000"    /* 업무개시 응답(체결,장운영 전용)             */
#define     Q_HEARTBEAT_S       "SCHHEQ00000"    /* 회선시험 요청                               */
#define     R_HEARTBEAT_S       "SCHHER00000"    /* 회선시험 응답                               */
#define     Q_LOGOUT_S          "SCHLOQ00000"    /* 로그아웃 요청                               */
#define     R_LOGOUT_S          "SCHLOR00000"    /* 로그아웃 응답                               */

#define     Q_ORDER_S           "TCHODR00000"    /* 호가입력                                    */
#define     R_SETTLE_S          "TCHTDP00000"    /* 체결                                        */
#define     R_BOND_S            "TCHDRP00000"    /* 체권 체결                                   */
#define     R_JANG_S            "TCHMIP00000"    /* 장운영                                      */
#define     R_QUERY_S           "TCHQEY00000"    /* 조회 요청                                   */
#define     R_STOP_S            "99999999999"    /* TR송신종료                                  */

#define     Q_TCHSTOP_S         "TCHEDP99000"    /* 체결 인터페이스 종료                        */
#define     Q_TJASTOP_S         "TCHEDP99001"    /* 장운영 인터페이스 종료                      */

#define     Q_TCSSTOP1_S        "TCSMIH29901"    /* 실시간송신 - 업무 마감                      */
#define     Q_TCSSTOP2_S        "TCSMIH29902"    /* 실시간수신 - 최종 마감                      */

#define     Q_SEQUENCE_S        "SCHSQQ00000"    /* 일련번호 요청                               */
#define     R_SEQUENCE_S        "SCHSQR00000"    /* 일련번호 응답                               */
#define     Q_RESEND_S          "SCHRSQ00000"    /* 재송신 요청                                 */
#define     R_RESEND_S          "SCHRSR00000"    /* 재송신 응답                                 */

#define     S_TFSBYS            "TFSBYR00000"    /* 외국인일일변동송신                          */

#endif
