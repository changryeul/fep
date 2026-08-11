#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define		MAX_SR_LEN_400			400

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

T_KRX_HEAD_Print( T_KRX_HEAD* ptr)
{
    printf( "----[ T_KRX_HEAD ]----------------------------------------------------------------------\n");
    printf( "전문유형                      sBeginString           8    0 = [%8.8s]     \n", ptr->sBeginString);
    printf( "메시지길이                    sBodyLength            6    8 = [%6.6s]     \n", ptr->sBodyLength);
    printf( "메시지타입                    sMsgType              11   14 = [%11.11s]   \n", ptr->sMsgType);
    printf( "일련번호                      sMsgSeqNum            11   25 = [%11.11s]   \n", ptr->sMsgSeqNum);
    printf( "회원번호                      sSenderCompID          5   36 = [%5.5s]     \n", ptr->sSenderCompID);
    printf( "연계시도착회원사번호          sDeliverToCompID      10   41 = [%10.10s]   \n", ptr->sDeliverToCompID);
    printf( "회신시송신회원사번호          sOnBehalfOfCompID     10   51 = [%10.10s]   \n", ptr->sOnBehalfOfCompID);
    printf( "전송일자(YYYYMMDD)            sSendingDate           8   61 = [%8.8s]     \n", ptr->sSendingDate);
    printf( "전송일시(HHMMDDmmm)           sSendingTime           9   69 = [%9.9s]     \n", ptr->sSendingTime);
    printf( "데이터건수                    sDataCnt               3   78 = [%3.3s]     \n", ptr->sDataCnt);
    printf( "암호화유무                    sEncrypt               1   81 = [%1.1s]     \n", ptr->sEncrypt);
    printf( "----------------------------------------------------------------------[ T_KRX_HEAD ]----\n");

    return sizeof( T_KRX_HEAD);
}

T_KRX_HEARTBEAT_Print( T_KRX_HEARTBEAT* ptr)
{
    printf( "----[ T_KRX_HEARTBEAT ]-----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "-----------------------------------------------------------------[ T_KRX_HEARTBEAT ]----\n");

    return sizeof( T_KRX_HEARTBEAT);
}

T_KRX_Q_LOGON_Print( T_KRX_Q_LOGON* ptr)
{
    printf( "----[ T_KRX_Q_LOGON ]-------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거래소에서부여한ID            sUserId               10    0 = [%10.10s]   \n", ptr->sUserId);
    printf( "                              sPassword             30   10 = [%30.30s]   \n", ptr->sPassword);
    printf( "암복호화적용여부              sEncrypt               1   40 = [%1.1s]     \n", ptr->sEncrypt);
    printf( "-------------------------------------------------------------------[ T_KRX_Q_LOGON ]----\n");

    return sizeof( T_KRX_Q_LOGON);
}

T_KRX_R_LOGON_Print( T_KRX_R_LOGON* ptr)
{
    printf( "----[ T_KRX_R_LOGON ]-------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "암복호화적용여부              sEncrypt               1    4 = [%1.1s]     \n", ptr->sEncrypt);
    printf( "-------------------------------------------------------------------[ T_KRX_R_LOGON ]----\n");

    return sizeof( T_KRX_R_LOGON);
}

T_KRX_QR_OPEN_Print( T_KRX_QR_OPEN* ptr)
{
    printf( "----[ T_KRX_QR_OPEN ]-------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "초기는space,재업무개시는이전  sTrCd                 11    4 = [%11.11s]   \n", ptr->sTrCd);
    printf( "최종처리일련번호              sLastSeq              11   15 = [%11.11s]   \n", ptr->sLastSeq);
    printf( "-------------------------------------------------------------------[ T_KRX_QR_OPEN ]----\n");

    return sizeof( T_KRX_QR_OPEN);
}

T_KRX_QR_OPEN_1_Print( T_KRX_QR_OPEN_1* ptr)
{
    printf( "----[ T_KRX_QR_OPEN_1 ]-----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "거래소응답수신후해당하는매칭  sMeGrpCnt              2    4 = [%2.2s]     \n", ptr->sMeGrpCnt);
    printf( "매칭그룹#01~10최종처리일련번  sMeGrpSeq             10    6 = [%10.10s]   \n", ptr->sMeGrpSeq);
    printf( "-----------------------------------------------------------------[ T_KRX_QR_OPEN_1 ]----\n");

    return sizeof( T_KRX_QR_OPEN_1);
}

T_KRX_Q_RESEND_Print( T_KRX_Q_RESEND* ptr)
{
    printf( "----[ T_KRX_Q_RESEND ]------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "최종처리일련번호              sLastSeq              11    0 = [%11.11s]   \n", ptr->sLastSeq);
    printf( "------------------------------------------------------------------[ T_KRX_Q_RESEND ]----\n");

    return sizeof( T_KRX_Q_RESEND);
}

T_KRX_R_RESEND_Print( T_KRX_R_RESEND* ptr)
{
    printf( "----[ T_KRX_R_RESEND ]------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "최종처리일련번호              sLastSeq              11    4 = [%11.11s]   \n", ptr->sLastSeq);
    printf( "------------------------------------------------------------------[ T_KRX_R_RESEND ]----\n");

    return sizeof( T_KRX_R_RESEND);
}

T_KRX_Q_SEQUENCE_Print( T_KRX_Q_SEQUENCE* ptr)
{
    printf( "----[ T_KRX_Q_SEQUENCE ]----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "----------------------------------------------------------------[ T_KRX_Q_SEQUENCE ]----\n");

    return sizeof( T_KRX_Q_SEQUENCE);
}

T_KRX_R_SEQUENCE_Print( T_KRX_R_SEQUENCE* ptr)
{
    printf( "----[ T_KRX_R_SEQUENCE ]----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "최종처리일련번호              sLastSeq              11    0 = [%11.11s]   \n", ptr->sLastSeq);
    printf( "----------------------------------------------------------------[ T_KRX_R_SEQUENCE ]----\n");

    return sizeof( T_KRX_R_SEQUENCE);
}

T_KRX_Q_LOGOUT_Print( T_KRX_Q_LOGOUT* ptr)
{
    printf( "----[ T_KRX_Q_LOGOUT ]------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "------------------------------------------------------------------[ T_KRX_Q_LOGOUT ]----\n");

    return sizeof( T_KRX_Q_LOGOUT);
}

T_KRX_R_LOGOUT_Print( T_KRX_R_LOGOUT* ptr)
{
    printf( "----[ T_KRX_R_LOGOUT ]------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "------------------------------------------------------------------[ T_KRX_R_LOGOUT ]----\n");

    return sizeof( T_KRX_R_LOGOUT);
}

T_KRX_ORDER_Print( T_KRX_ORDER* ptr)
{
    printf( "----[ T_KRX_ORDER ]---------------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "보드ID                        sBoardId               2   22 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   24 = [%5.5s]     \n", ptr->sCompNo);
    printf( "지점번호                      sBrnNo                 5   29 = [%5.5s]     \n", ptr->sBrnNo);
    printf( "주문ID                        sOrderNo              10   34 = [%10.10s]   \n", ptr->sOrderNo);
    printf( "원주문ID                      sOrgOrderNo           10   44 = [%10.10s]   \n", ptr->sOrgOrderNo);
    printf( "종목코드                      sJCode                12   54 = [%12.12s]   \n", ptr->sJCode);
    printf( "매도매수구분코드              sMdsGb                 1   66 = [%1.1s]     \n", ptr->sMdsGb);
    printf( "정정취소구분코드              sOrderGb               1   67 = [%1.1s]     \n", ptr->sOrderGb);
    printf( "계좌번호                      sAcctNo               12   68 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "호가수량                      sOrderQty             10   80 = [%10.10s]   \n", ptr->sOrderQty);
    printf( "호가가격                      sOrderPrice           11   90 = [%11.11s]   \n", ptr->sOrderPrice);
    printf( "호가유형코드                  sOrderType             1  101 = [%1.1s]     \n", ptr->sOrderType);
    printf( "호가조건코드                  sOrderCond             1  102 = [%1.1s]     \n", ptr->sOrderCond);
    printf( "시장조성자호가구분번호        sMktOrderGb           11  103 = [%11.11s]   \n", ptr->sMktOrderGb);
    printf( "자사주신고서ID                sJasaId                5  114 = [%5.5s]     \n", ptr->sJasaId);
    printf( "자사주매매방법코드            sJasaTradeGb           1  119 = [%1.1s]     \n", ptr->sJasaTradeGb);
    printf( "매도유형코드                  sMdType                2  120 = [%2.2s]     \n", ptr->sMdType);
    printf( "신용구분코드                  sSyGb                  2  122 = [%2.2s]     \n", ptr->sSyGb);
    printf( "위탁자기구분코드              sWitakGb               2  124 = [%2.2s]     \n", ptr->sWitakGb);
    printf( "위탁사번호                    sWitakCompNo           5  126 = [%5.5s]     \n", ptr->sWitakCompNo);
    printf( "PT구분코드                    sPtGb                  2  131 = [%2.2s]     \n", ptr->sPtGb);
    printf( "대용주권계좌번호              sDaeAcctNo            12  133 = [%12.12s]   \n", ptr->sDaeAcctNo);
    printf( "계좌구분코드                  sAcctGb                2  145 = [%2.2s]     \n", ptr->sAcctGb);
    printf( "계좌증거금유형코드            sAcctMargType          2  147 = [%2.2s]     \n", ptr->sAcctMargType);
    printf( "국가코드                      sNationCd              3  149 = [%3.3s]     \n", ptr->sNationCd);
    printf( "투자자구분코드                sTujaCd                4  152 = [%4.4s]     \n", ptr->sTujaCd);
    printf( "외국인투자자구분코드          sFrgTujaGb             2  156 = [%2.2s]     \n", ptr->sFrgTujaGb);
    printf( "주문매체구분코드              sMediaGb               1  158 = [%1.1s]     \n", ptr->sMediaGb);
    printf( "주문자식별정보                sOrderInfo            12  159 = [%12.12s]   \n", ptr->sOrderInfo);
    printf( "MAC주소                       sMacAddr              12  171 = [%12.12s]   \n", ptr->sMacAddr);
    printf( "호가일자                      sDate                  8  183 = [%8.8s]     \n", ptr->sDate);
    printf( "회원사주문시각                sOrderTime             9  191 = [%9.9s]     \n", ptr->sOrderTime);
    printf( "회원사용영역                  sCustomerInfo         60  200 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "프로그램호가신고구분코드      sPgmDeclareCd          1  260 = [%1.1s]     \n", ptr->sPgmDeclareCd);
    printf( "---------------------------------------------------------------------[ T_KRX_ORDER ]----\n");

    return sizeof( T_KRX_ORDER);
}

T_KRX_ORDER_FMT_Print( T_KRX_ORDER_FMT* ptr)
{
    printf( "----[ T_KRX_ORDER_FMT ]-----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    T_KRX_ORDER_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_ORDER, MAX_SR_LEN_400-SZ_KRX_ORDER, ptr->sSpace);
    printf( "-----------------------------------------------------------------[ T_KRX_ORDER_FMT ]----\n");

    return sizeof( T_KRX_ORDER_FMT);
}

T_KRX_ORDER_400_Print( T_KRX_ORDER_400* ptr)
{
    printf( "----[ T_KRX_ORDER_400 ]-----------------------------------------------------------------\n");
    T_KRX_ORDER_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_ORDER, MAX_SR_LEN_400-SZ_KRX_ORDER, ptr->sSpace);
    printf( "-----------------------------------------------------------------[ T_KRX_ORDER_400 ]----\n");

    return sizeof( T_KRX_ORDER_400);
}

T_KRX_ORDER_RESP_Print( T_KRX_ORDER_RESP* ptr)
{
    printf( "----[ T_KRX_ORDER_RESP ]----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    printf( "거부사유코드                  sRjtCd                 4    0 = [%4.4s]     \n", ptr->sRjtCd);
    printf( "처리일련번호                  sLastSeq              11    4 = [%11.11s]   \n", ptr->sLastSeq);
    printf( "----------------------------------------------------------------[ T_KRX_ORDER_RESP ]----\n");

    return sizeof( T_KRX_ORDER_RESP);
}

T_COMBINATION_ORDER_Print( T_COMBINATION_ORDER* ptr)
{
    printf( "----[ T_COMBINATION_ORDER ]-------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "보드ID                        sBoardId               2   22 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   24 = [%5.5s]     \n", ptr->sCompNo);
    printf( "지점번호                      sBrnNo                 5   29 = [%5.5s]     \n", ptr->sBrnNo);
    printf( "계좌번호                      sAcctNo               12   34 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "호가유형코드                  sOrderType             1   46 = [%1.1s]     \n", ptr->sOrderType);
    printf( "호가조건코드                  sOrderCond             1   47 = [%1.1s]     \n", ptr->sOrderCond);
    printf( "주문개수                      sOrderCnt              2   48 = [%2.2s]     \n", ptr->sOrderCnt);
    printf( "주문ID1                       sOrderNo1             10   50 = [%10.10s]   \n", ptr->sOrderNo1);
    printf( "종목코드1                     sJCode1               12   60 = [%12.12s]   \n", ptr->sJCode1);
    printf( "매도매수구분코드1             sMdsGb1                1   72 = [%1.1s]     \n", ptr->sMdsGb1);
    printf( "호가수량1                     sOrderQty1            10   73 = [%10.10s]   \n", ptr->sOrderQty1);
    printf( "호가가격1                     sOrderPrice1          11   83 = [%11.11s]   \n", ptr->sOrderPrice1);
    printf( "주문ID2                       sOrderNo2             10   94 = [%10.10s]   \n", ptr->sOrderNo2);
    printf( "종목코드2                     sJCode2               12  104 = [%12.12s]   \n", ptr->sJCode2);
    printf( "매도매수구분코드2             sMdsGb2                1  116 = [%1.1s]     \n", ptr->sMdsGb2);
    printf( "호가수량2                     sOrderQty2            10  117 = [%10.10s]   \n", ptr->sOrderQty2);
    printf( "호가가격2                     sOrderPrice2          11  127 = [%11.11s]   \n", ptr->sOrderPrice2);
    printf( "주문ID3                       sOrderNo3             10  138 = [%10.10s]   \n", ptr->sOrderNo3);
    printf( "종목코드3                     sJCode3               12  148 = [%12.12s]   \n", ptr->sJCode3);
    printf( "매도매수구분코드3             sMdsGb3                1  160 = [%1.1s]     \n", ptr->sMdsGb3);
    printf( "호가수량3                     sOrderQty3            10  161 = [%10.10s]   \n", ptr->sOrderQty3);
    printf( "호가가격3                     sOrderPrice3          11  171 = [%11.11s]   \n", ptr->sOrderPrice3);
    printf( "주문ID4                       sOrderNo4             10  182 = [%10.10s]   \n", ptr->sOrderNo4);
    printf( "종목코드4                     sJCode4               12  192 = [%12.12s]   \n", ptr->sJCode4);
    printf( "매도매수구분코드4             sMdsGb4                1  204 = [%1.1s]     \n", ptr->sMdsGb4);
    printf( "호가수량4                     sOrderQty4            10  205 = [%10.10s]   \n", ptr->sOrderQty4);
    printf( "호가가격4                     sOrderPrice4          11  215 = [%11.11s]   \n", ptr->sOrderPrice4);
    printf( "위탁자기구분코드              sWitakGb               2  226 = [%2.2s]     \n", ptr->sWitakGb);
    printf( "위탁사번호                    sWitakCompNo           5  228 = [%5.5s]     \n", ptr->sWitakCompNo);
    printf( "PT구분코드                    sPtGb                  2  233 = [%2.2s]     \n", ptr->sPtGb);
    printf( "대용주권계좌번호              sDaeAcctNo            12  235 = [%12.12s]   \n", ptr->sDaeAcctNo);
    printf( "계좌구분코드                  sAcctGb                2  247 = [%2.2s]     \n", ptr->sAcctGb);
    printf( "계좌증거금유형코드            sAcctMargType          2  249 = [%2.2s]     \n", ptr->sAcctMargType);
    printf( "국가코드                      sNationCd              3  251 = [%3.3s]     \n", ptr->sNationCd);
    printf( "투자자구분코드                sTujaCd                4  254 = [%4.4s]     \n", ptr->sTujaCd);
    printf( "외국인투자자구분코드          sFrgTujaGb             2  258 = [%2.2s]     \n", ptr->sFrgTujaGb);
    printf( "주문매체구분코드              sMediaGb               1  260 = [%1.1s]     \n", ptr->sMediaGb);
    printf( "주문자식별정보                sOrderInfo            12  261 = [%12.12s]   \n", ptr->sOrderInfo);
    printf( "MAC주소                       sMacAddr              12  273 = [%12.12s]   \n", ptr->sMacAddr);
    printf( "호가일자                      sDate                  8  285 = [%8.8s]     \n", ptr->sDate);
    printf( "회원사주문시각                sOrderTime             9  293 = [%9.9s]     \n", ptr->sOrderTime);
    printf( "회원사용영역                  sCustomerInfo         60  302 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "-------------------------------------------------------------[ T_COMBINATION_ORDER ]----\n");

    return sizeof( T_COMBINATION_ORDER);
}

T_COMBINATION_ORDER_400_Print( T_COMBINATION_ORDER_400* ptr)
{
    printf( "----[ T_COMBINATION_ORDER_400 ]---------------------------------------------------------\n");
    T_COMBINATION_ORDER_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_COMBINATION_ORDER, MAX_SR_LEN_400-SZ_COMBINATION_ORDER, ptr->sSpace);
    printf( "---------------------------------------------------------[ T_COMBINATION_ORDER_400 ]----\n");

    return sizeof( T_COMBINATION_ORDER_400);
}

T_KILL_SWITCH_Print( T_KILL_SWITCH* ptr)
{
    printf( "----[ T_KILL_SWITCH ]-------------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "시장ID                        sMarketId              3   22 = [%3.3s]     \n", ptr->sMarketId);
    printf( "보드ID                        sBoardId               2   25 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   27 = [%5.5s]     \n", ptr->sCompNo);
    printf( "계좌번호                      sAcctNo               12   32 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "제재해제구분코드              sSanctnReleasTPCd      1   44 = [%1.1s]     \n", ptr->sSanctnReleasTPCd);
    printf( "주문매체구분코드              sMediaGb               1   45 = [%1.1s]     \n", ptr->sMediaGb);
    printf( "주문자식별정보                sOrderInfo            12   46 = [%12.12s]   \n", ptr->sOrderInfo);
    printf( "MAC주소                       sMacAddr              12   58 = [%12.12s]   \n", ptr->sMacAddr);
    printf( "신고일자                      sDate                  8   70 = [%8.8s]     \n", ptr->sDate);
    printf( "신고시각                      sOrderTime             9   78 = [%9.9s]     \n", ptr->sOrderTime);
    printf( "회원사용영역                  sCustomerInfo         60   87 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "-------------------------------------------------------------------[ T_KILL_SWITCH ]----\n");

    return sizeof( T_KILL_SWITCH);
}

T_KILL_SWITCH_400_Print( T_KILL_SWITCH_400* ptr)
{
    printf( "----[ T_KILL_SWITCH_400 ]---------------------------------------------------------------\n");
    T_KILL_SWITCH_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KILL_SWITCH, MAX_SR_LEN_400-SZ_KILL_SWITCH, ptr->sSpace);
    printf( "---------------------------------------------------------------[ T_KILL_SWITCH_400 ]----\n");

    return sizeof( T_KILL_SWITCH_400);
}

T_KRX_SETTLE_Print( T_KRX_SETTLE* ptr)
{
    printf( "----[ T_KRX_SETTLE ]--------------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   26 = [%5.5s]     \n", ptr->sCompNo);
    printf( "지점번호                      sBrnNo                 5   31 = [%5.5s]     \n", ptr->sBrnNo);
    printf( "주문ID                        sOrderNo              10   36 = [%10.10s]   \n", ptr->sOrderNo);
    printf( "원주문ID                      sOrgOrderNo           10   46 = [%10.10s]   \n", ptr->sOrgOrderNo);
    printf( "종목코드                      sJCode                12   56 = [%12.12s]   \n", ptr->sJCode);
    printf( "체결번호                      sChNo                 11   68 = [%11.11s]   \n", ptr->sChNo);
    printf( "체결가격                      sChPrice              11   79 = [%11.11s]   \n", ptr->sChPrice);
    printf( "체결수량                      sChAmt                10   90 = [%10.10s]   \n", ptr->sChAmt);
    printf( "세션ID                        sChType                2  100 = [%2.2s]     \n", ptr->sChType);
    printf( "체결일자                      sChDate                8  102 = [%8.8s]     \n", ptr->sChDate);
    printf( "체결시각                      sChTime                9  110 = [%9.9s]     \n", ptr->sChTime);
    printf( "근월물체결가격                sNearChPrice          11  119 = [%11.11s]   \n", ptr->sNearChPrice);
    printf( "원월물체결가격                sFarChPrice           11  130 = [%11.11s]   \n", ptr->sFarChPrice);
    printf( "매도매수구분코드              sMdsGb                 1  141 = [%1.1s]     \n", ptr->sMdsGb);
    printf( "계좌번호                      sAcctNo               12  142 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "시장조성자호가구분번호        sMktOrderGb           11  154 = [%11.11s]   \n", ptr->sMktOrderGb);
    printf( "위탁사번호                    sWitakCompNo           5  165 = [%5.5s]     \n", ptr->sWitakCompNo);
    printf( "대용주권계좌번호              sDaeAcctNo            12  170 = [%12.12s]   \n", ptr->sDaeAcctNo);
    printf( "회원사용영역                  sCustomerInfo         60  182 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "--------------------------------------------------------------------[ T_KRX_SETTLE ]----\n");

    return sizeof( T_KRX_SETTLE);
}

T_KRX_SETTLE_FMT_Print( T_KRX_SETTLE_FMT* ptr)
{
    printf( "----[ T_KRX_SETTLE_FMT ]----------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    T_KRX_SETTLE_Print( &ptr->tData);
    printf( "----------------------------------------------------------------[ T_KRX_SETTLE_FMT ]----\n");

    return sizeof( T_KRX_SETTLE_FMT);
}

T_KRX_SETTLE_400_Print( T_KRX_SETTLE_400* ptr)
{
    printf( "----[ T_KRX_SETTLE_400 ]----------------------------------------------------------------\n");
    T_KRX_SETTLE_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_SETTLE, MAX_SR_LEN_400-SZ_KRX_SETTLE, ptr->sSpace);
    printf( "----------------------------------------------------------------[ T_KRX_SETTLE_400 ]----\n");

    return sizeof( T_KRX_SETTLE_400);
}

T_KRX_CONFIRM_Print( T_KRX_CONFIRM* ptr)
{
    printf( "----[ T_KRX_CONFIRM ]-------------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   26 = [%5.5s]     \n", ptr->sCompNo);
    printf( "지점번호                      sBrnNo                 5   31 = [%5.5s]     \n", ptr->sBrnNo);
    printf( "주문ID                        sOrderNo              10   36 = [%10.10s]   \n", ptr->sOrderNo);
    printf( "원주문ID                      sOrgOrderNo           10   46 = [%10.10s]   \n", ptr->sOrgOrderNo);
    printf( "종목코드                      sJCode                12   56 = [%12.12s]   \n", ptr->sJCode);
    printf( "매도매수구분코드              sMdsGb                 1   68 = [%1.1s]     \n", ptr->sMdsGb);
    printf( "정정취소구분코드              sOrderGb               1   69 = [%1.1s]     \n", ptr->sOrderGb);
    printf( "계좌번호                      sAcctNo               12   70 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "호가수량                      sOrderQty             10   82 = [%10.10s]   \n", ptr->sOrderQty);
    printf( "호가가격                      sOrderPrice           11   92 = [%11.11s]   \n", ptr->sOrderPrice);
    printf( "호가유형코드                  sOrderType             1  103 = [%1.1s]     \n", ptr->sOrderType);
    printf( "호가조건코드                  sOrderCond             1  104 = [%1.1s]     \n", ptr->sOrderCond);
    printf( "시장조성자호가구분번호        sMktOrderGb           11  105 = [%11.11s]   \n", ptr->sMktOrderGb);
    printf( "자사주신고서ID                sJasaId                5  116 = [%5.5s]     \n", ptr->sJasaId);
    printf( "자사주매매방법코드            sJasaTradeGb           1  121 = [%1.1s]     \n", ptr->sJasaTradeGb);
    printf( "매도유형코드                  sMdType                2  122 = [%2.2s]     \n", ptr->sMdType);
    printf( "신용구분코드                  sSyGb                  2  124 = [%2.2s]     \n", ptr->sSyGb);
    printf( "위탁자기구분코드              sWitakGb               2  126 = [%2.2s]     \n", ptr->sWitakGb);
    printf( "위탁사번호                    sWitakCompNo           5  128 = [%5.5s]     \n", ptr->sWitakCompNo);
    printf( "PT구분코드                    sPtGb                  2  133 = [%2.2s]     \n", ptr->sPtGb);
    printf( "대용주권계좌번호              sDaeAcctNo            12  135 = [%12.12s]   \n", ptr->sDaeAcctNo);
    printf( "계좌구분코드                  sAcctGb                2  147 = [%2.2s]     \n", ptr->sAcctGb);
    printf( "계좌증거금유형코드            sAcctMargType          2  149 = [%2.2s]     \n", ptr->sAcctMargType);
    printf( "국가코드                      sNationCd              3  151 = [%3.3s]     \n", ptr->sNationCd);
    printf( "투자자구분코드                sTujaCd                4  154 = [%4.4s]     \n", ptr->sTujaCd);
    printf( "외국인투자자구분코드          sFrgTujaGb             2  158 = [%2.2s]     \n", ptr->sFrgTujaGb);
    printf( "주문매체구분코드              sMediaGb               1  160 = [%1.1s]     \n", ptr->sMediaGb);
    printf( "주문자식별정보                sOrderInfo            12  161 = [%12.12s]   \n", ptr->sOrderInfo);
    printf( "MAC주소                       sMacAddr              12  173 = [%12.12s]   \n", ptr->sMacAddr);
    printf( "호가일자                      sDate                  8  185 = [%8.8s]     \n", ptr->sDate);
    printf( "회원사주문시각                sOrderTime             9  193 = [%9.9s]     \n", ptr->sOrderTime);
    printf( "회원사용영역                  sCustomerInfo         60  202 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "호가접수시각                  sOrderRecvTime         9  262 = [%9.9s]     \n", ptr->sOrderRecvTime);
    printf( "실정정취소호가수량            sRealOrderQty         10  271 = [%10.10s]   \n", ptr->sRealOrderQty);
    printf( "자동취소처리구분코드          sAutoCancelGb          1  281 = [%1.1s]     \n", ptr->sAutoCancelGb);
    printf( "호가거부사유코드              sRejectCd              4  282 = [%4.4s]     \n", ptr->sRejectCd);
    printf( "프로그램호가신고구분코드      sPgmDeclareCd          1  286 = [%1.1s]     \n", ptr->sPgmDeclareCd);
    printf( "-------------------------------------------------------------------[ T_KRX_CONFIRM ]----\n");

    return sizeof( T_KRX_CONFIRM);
}

T_KRX_CONFIRM_FMT_Print( T_KRX_CONFIRM_FMT* ptr)
{
    printf( "----[ T_KRX_CONFIRM_FMT ]---------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    T_KRX_CONFIRM_Print( &ptr->tData);
    printf( "---------------------------------------------------------------[ T_KRX_CONFIRM_FMT ]----\n");

    return sizeof( T_KRX_CONFIRM_FMT);
}

T_KRX_CONFIRM_400_Print( T_KRX_CONFIRM_400* ptr)
{
    printf( "----[ T_KRX_CONFIRM_400 ]---------------------------------------------------------------\n");
    T_KRX_CONFIRM_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_CONFIRM, MAX_SR_LEN_400-SZ_KRX_CONFIRM, ptr->sSpace);
    printf( "---------------------------------------------------------------[ T_KRX_CONFIRM_400 ]----\n");

    return sizeof( T_KRX_CONFIRM_400);
}

T_KILL_SWITCH_CONFIRM_Print( T_KILL_SWITCH_CONFIRM* ptr)
{
    printf( "----[ T_KILL_SWITCH_CONFIRM ]-----------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "시장ID                        sMarketId              3   24 = [%3.3s]     \n", ptr->sMarketId);
    printf( "보드ID                        sBoardId               2   27 = [%2.2s]     \n", ptr->sBoardId);
    printf( "회원번호                      sCompNo                5   29 = [%5.5s]     \n", ptr->sCompNo);
    printf( "계좌번호                      sAcctNo               12   34 = [%12.12s]   \n", ptr->sAcctNo);
    printf( "제재해제구분코드              sSanctnReleasTPCd      1   46 = [%1.1s]     \n", ptr->sSanctnReleasTPCd);
    printf( "주문매체구분코드              sMediaGb               1   47 = [%1.1s]     \n", ptr->sMediaGb);
    printf( "주문자식별정보                sOrderInfo            12   48 = [%12.12s]   \n", ptr->sOrderInfo);
    printf( "MAC주소                       sMacAddr              12   60 = [%12.12s]   \n", ptr->sMacAddr);
    printf( "신고일자                      sDate                  8   72 = [%8.8s]     \n", ptr->sDate);
    printf( "신고시각                      sOrderTime             9   80 = [%9.9s]     \n", ptr->sOrderTime);
    printf( "회원사용영역                  sCustomerInfo         60   89 = [%60.60s]   \n", ptr->sCustomerInfo);
    printf( "접수시각                      sOrderRecvTime         9  149 = [%9.9s]     \n", ptr->sOrderRecvTime);
    printf( "거부코드                      sRejectCd              4  158 = [%4.4s]     \n", ptr->sRejectCd);
    printf( "-----------------------------------------------------------[ T_KILL_SWITCH_CONFIRM ]----\n");

    return sizeof( T_KILL_SWITCH_CONFIRM);
}

T_KILL_SWITCH_CONFIRM_400_Print( T_KILL_SWITCH_CONFIRM_400* ptr)
{
    printf( "----[ T_KILL_SWITCH_CONFIRM_400 ]-------------------------------------------------------\n");
    T_KILL_SWITCH_CONFIRM_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KILL_SWITCH_CONFIRM, MAX_SR_LEN_400-SZ_KILL_SWITCH_CONFIRM, ptr->sSpace);
    printf( "-------------------------------------------------------[ T_KILL_SWITCH_CONFIRM_400 ]----\n");

    return sizeof( T_KILL_SWITCH_CONFIRM_400);
}

T_SETTLE_IF_END_Print( T_SETTLE_IF_END* ptr)
{
    printf( "----[ T_SETTLE_IF_END ]-----------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "매칭그룹번호                  sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "매칭그룹개수                  sMeGrpCnt              2   24 = [%2.2s]     \n", ptr->sMeGrpCnt);
    printf( "매칭그룹#01~10최종처리일련번  sMeGrpSeq             10   26 = [%10.10s]   \n", ptr->sMeGrpSeq);
    printf( "전송일자                      sDate                  8   36 = [%8.8s]     \n", ptr->sDate);
    printf( "전송시각                      sTime                  9   44 = [%9.9s]     \n", ptr->sTime);
    printf( "-----------------------------------------------------------------[ T_SETTLE_IF_END ]----\n");

    return sizeof( T_SETTLE_IF_END);
}

T_SETTLE_IF_END_400_Print( T_SETTLE_IF_END_400* ptr)
{
    printf( "----[ T_SETTLE_IF_END_400 ]-------------------------------------------------------------\n");
    T_SETTLE_IF_END_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_SETTLE_IF_END, MAX_SR_LEN_400-SZ_SETTLE_IF_END, ptr->sSpace);
    printf( "-------------------------------------------------------------[ T_SETTLE_IF_END_400 ]----\n");

    return sizeof( T_SETTLE_IF_END_400);
}

T_CUST_INFO_Print( T_CUST_INFO* ptr)
{
    printf( "----[ T_CUST_INFO ]---------------------------------------------------------------------\n");
    printf( "                              sSrvGb                 2    0 = [%2.2s]     \n", ptr->sSrvGb);
    printf( "                              sBizGb                 1    2 = [%1.1s]     \n", ptr->sBizGb);
    printf( "                              sAcctGb                1    3 = [%1.1s]     \n", ptr->sAcctGb);
    printf( "                              sOrderGb               8    4 = [%8.8s]     \n", ptr->sOrderGb);
    printf( "                              sFiller1               7   12 = [%7.7s]     \n", ptr->sFiller1);
    printf( "                              sSubId                 7   19 = [%7.7s]     \n", ptr->sSubId);
    printf( "                              sFiller2               2   26 = [%2.2s]     \n", ptr->sFiller2);
    printf( "2016.04.14-byblue,실시간건수  sEtc                  17   28 = [%17.17s]   \n", ptr->sEtc);
    printf( "2016.04.14-byblue,실시간건수  sSessGb                5   45 = [%5.5s]     \n", ptr->sSessGb);
    printf( "                              sTrPk                  3   50 = [%3.3s]     \n", ptr->sTrPk);
    printf( "                              sExGb                  1   53 = [%1.1s]     \n", ptr->sExGb);
    printf( "                              sBrsFiller             6   54 = [%6.6s]     \n", ptr->sBrsFiller);
    printf( "---------------------------------------------------------------------[ T_CUST_INFO ]----\n");

    return sizeof( T_CUST_INFO);
}

T_KRX_JANG_Print( T_KRX_JANG* ptr)
{
    printf( "----[ T_KRX_JANG ]----------------------------------------------------------------------\n");
    printf( "메세지일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "장운영상품그룹ID              sProdGrpId             3   24 = [%3.3s]     \n", ptr->sProdGrpId);
    printf( "보드ID                        sBoardId               2   27 = [%2.2s]     \n", ptr->sBoardId);
    printf( "보드이벤트ID                  sBoardEvtId            3   29 = [%3.3s]     \n", ptr->sBoardEvtId);
    printf( "보드이벤트시작시각            sBoardEvtSrtTime       9   32 = [%9.9s]     \n", ptr->sBoardEvtSrtTime);
    printf( "보드이벤트적용군코드          sBoardEvtAppGrpCd      5   41 = [%5.5s]     \n", ptr->sBoardEvtAppGrpCd);
    printf( "세션개시종료코드              sSessSrtEndCd          2   46 = [%2.2s]     \n", ptr->sSessSrtEndCd);
    printf( "세션ID                        sSessId                2   48 = [%2.2s]     \n", ptr->sSessId);
    printf( "종목코드                      sJCode                12   50 = [%12.12s]   \n", ptr->sJCode);
    printf( "상품ID                        sClassID              11   62 = [%11.11s]   \n", ptr->sClassID);
    printf( "거래정지사유코드              sHaltRsnCode           3   73 = [%3.3s]     \n", ptr->sHaltRsnCode);
    printf( "거래정지발생유형코드          sHaltTypeCode          1   76 = [%1.1s]     \n", ptr->sHaltTypeCode);
    printf( "Add20150615                   sApplStep              2   77 = [%2.2s]     \n", ptr->sApplStep);
    printf( "Add20150615                   sBasIsuPleOccrCd       1   79 = [%1.1s]     \n", ptr->sBasIsuPleOccrCd);
    printf( "Add20150615                   sPleSchdlTm            9   80 = [%9.9s]     \n", ptr->sPleSchdlTm);
    printf( "----------------------------------------------------------------------[ T_KRX_JANG ]----\n");

    return sizeof( T_KRX_JANG);
}

T_KRX_JANG_400_Print( T_KRX_JANG_400* ptr)
{
    printf( "----[ T_KRX_JANG_400 ]------------------------------------------------------------------\n");
    T_KRX_JANG_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG, MAX_SR_LEN_400-SZ_KRX_JANG, ptr->sSpace);
    printf( "------------------------------------------------------------------[ T_KRX_JANG_400 ]----\n");

    return sizeof( T_KRX_JANG_400);
}

T_KRX_JANG_FMT_Print( T_KRX_JANG_FMT* ptr)
{
    printf( "----[ T_KRX_JANG_FMT ]------------------------------------------------------------------\n");
    T_KRX_HEAD_Print( &ptr->tHead);
    T_KRX_JANG_Print( &ptr->tData);
    printf( "------------------------------------------------------------------[ T_KRX_JANG_FMT ]----\n");

    return sizeof( T_KRX_JANG_FMT);
}

T_KRX_JANG_INFO_Print( T_KRX_JANG_INFO* ptr)
{
    printf( "----[ T_KRX_JANG_INFO ]-----------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "공개정보구분코드              sOpenInfoCode          3   24 = [%3.3s]     \n", ptr->sOpenInfoCode);
    printf( "상품그룹시장ID                sProdGrpId             3   27 = [%3.3s]     \n", ptr->sProdGrpId);
    printf( "종목코드                      sJCode                12   30 = [%12.12s]   \n", ptr->sJCode);
    printf( "공개시각                      sOpenTime              9   42 = [%9.9s]     \n", ptr->sOpenTime);
    printf( "기준가격                      sBasePrice            11   51 = [%11.11s]   \n", ptr->sBasePrice);
    printf( "상한가                        sHighLmtPrc           11   62 = [%11.11s]   \n", ptr->sHighLmtPrc);
    printf( "하한가                        sLowLmtPrc            11   73 = [%11.11s]   \n", ptr->sLowLmtPrc);
    printf( "평가가격                      sValuePrc             11   84 = [%11.11s]   \n", ptr->sValuePrc);
    printf( "최고호가가격                  sHighOrdPrc           11   95 = [%11.11s]   \n", ptr->sHighOrdPrc);
    printf( "최저호가가격                  sLowOrdPrc            11  106 = [%11.11s]   \n", ptr->sLowOrdPrc);
    printf( "시가기준가여부                sOpenPrcGb             1  117 = [%1.1s]     \n", ptr->sOpenPrcGb);
    printf( "락구분코드                    sOffTypeCode           2  118 = [%2.2s]     \n", ptr->sOffTypeCode);
    printf( "액면가변경구분코드            sParTypeCode           2  120 = [%2.2s]     \n", ptr->sParTypeCode);
    printf( "매매수량단위                  sOrdQtyUnit            6  122 = [%6.6s]     \n", ptr->sOrdQtyUnit);
    printf( "상장주식수                    sSharesNo             16  128 = [%16.16s]   \n", ptr->sSharesNo);
    printf( "정리매매관리종목여부          sArrantYN              1  144 = [%1.1s]     \n", ptr->sArrantYN);
    printf( "장개시전시간외종가가능여부    sPreHrYN               1  145 = [%1.1s]     \n", ptr->sPreHrYN);
    printf( "-----------------------------------------------------------------[ T_KRX_JANG_INFO ]----\n");

    return sizeof( T_KRX_JANG_INFO);
}

T_KRX_JANG_INFO_400_Print( T_KRX_JANG_INFO_400* ptr)
{
    printf( "----[ T_KRX_JANG_INFO_400 ]-------------------------------------------------------------\n");
    T_KRX_JANG_INFO_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_INFO, MAX_SR_LEN_400-SZ_KRX_JANG_INFO, ptr->sSpace);
    printf( "-------------------------------------------------------------[ T_KRX_JANG_INFO_400 ]----\n");

    return sizeof( T_KRX_JANG_INFO_400);
}

T_KRX_JANG_BASE_Print( T_KRX_JANG_BASE* ptr)
{
    printf( "----[ T_KRX_JANG_BASE ]-----------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "기준가격                      sBasePrice            11   38 = [%11.11s]   \n", ptr->sBasePrice);
    printf( "상한가                        sHighLmtPrc           11   49 = [%11.11s]   \n", ptr->sHighLmtPrc);
    printf( "하한가                        sLowLmtPrc            11   60 = [%11.11s]   \n", ptr->sLowLmtPrc);
    printf( "-----------------------------------------------------------------[ T_KRX_JANG_BASE ]----\n");

    return sizeof( T_KRX_JANG_BASE);
}

T_KRX_JANG_BASE_400_Print( T_KRX_JANG_BASE_400* ptr)
{
    printf( "----[ T_KRX_JANG_BASE_400 ]-------------------------------------------------------------\n");
    T_KRX_JANG_BASE_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_BASE, MAX_SR_LEN_400-SZ_KRX_JANG_BASE, ptr->sSpace);
    printf( "-------------------------------------------------------------[ T_KRX_JANG_BASE_400 ]----\n");

    return sizeof( T_KRX_JANG_BASE_400);
}

T_KRX_JANG_STOP_Print( T_KRX_JANG_STOP* ptr)
{
    printf( "----[ T_KRX_JANG_STOP ]-----------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "임의종료적용구분코드          sRandomType            1   38 = [%1.1s]     \n", ptr->sRandomType);
    printf( "임의종료잠정체결가            sDeemedPrice          11   39 = [%11.11s]   \n", ptr->sDeemedPrice);
    printf( "임의종료예상최고가            sDeemedHighPrc        11   50 = [%11.11s]   \n", ptr->sDeemedHighPrc);
    printf( "임의종료예상최고가괴리율      sDeemedHighRate       13   61 = [%13.13s]   \n", ptr->sDeemedHighRate);
    printf( "임의종료예상최저가            sDeemedLowPrc         11   74 = [%11.11s]   \n", ptr->sDeemedLowPrc);
    printf( "임의종료예상최저가괴리율      sDeemedLowRate        13   85 = [%13.13s]   \n", ptr->sDeemedLowRate);
    printf( "직전가격                      sLastPrice            11   98 = [%11.11s]   \n", ptr->sLastPrice);
    printf( "직전가격괴리율                sLastRate             13  109 = [%13.13s]   \n", ptr->sLastRate);
    printf( "임의종료해제시각              sRandomEndTime         9  122 = [%9.9s]     \n", ptr->sRandomEndTime);
    printf( "-----------------------------------------------------------------[ T_KRX_JANG_STOP ]----\n");

    return sizeof( T_KRX_JANG_STOP);
}

T_KRX_JANG_STOP_400_Print( T_KRX_JANG_STOP_400* ptr)
{
    printf( "----[ T_KRX_JANG_STOP_400 ]-------------------------------------------------------------\n");
    T_KRX_JANG_STOP_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_STOP, MAX_SR_LEN_400-SZ_KRX_JANG_STOP, ptr->sSpace);
    printf( "-------------------------------------------------------------[ T_KRX_JANG_STOP_400 ]----\n");

    return sizeof( T_KRX_JANG_STOP_400);
}

T_KRX_JANG_END_Print( T_KRX_JANG_END* ptr)
{
    printf( "----[ T_KRX_JANG_END ]------------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "종목마감종가                  sClosePrice           11   38 = [%11.11s]   \n", ptr->sClosePrice);
    printf( "종목마감가격구분코드          sClosePriceGb          1   49 = [%1.1s]     \n", ptr->sClosePriceGb);
    printf( "종목마감시간외단일가상한가    sOTHighLmtPrc         11   50 = [%11.11s]   \n", ptr->sOTHighLmtPrc);
    printf( "종목마감시간외단일가상한가    sOTLowLmtPrc          11   61 = [%11.11s]   \n", ptr->sOTLowLmtPrc);
    printf( "종목마감매입인도기준가격      sBIBasePrc            11   72 = [%11.11s]   \n", ptr->sBIBasePrc);
    printf( "종목마감매입인도단일가상한가  sBIHighLmtPrc         11   83 = [%11.11s]   \n", ptr->sBIHighLmtPrc);
    printf( "종목마감매입인도단일가상한가  sBILowLmtPrc          11   94 = [%11.11s]   \n", ptr->sBILowLmtPrc);
    printf( "------------------------------------------------------------------[ T_KRX_JANG_END ]----\n");

    return sizeof( T_KRX_JANG_END);
}

T_KRX_JANG_END_400_Print( T_KRX_JANG_END_400* ptr)
{
    printf( "----[ T_KRX_JANG_END_400 ]--------------------------------------------------------------\n");
    T_KRX_JANG_END_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_END, MAX_SR_LEN_400-SZ_KRX_JANG_END, ptr->sSpace);
    printf( "--------------------------------------------------------------[ T_KRX_JANG_END_400 ]----\n");

    return sizeof( T_KRX_JANG_END_400);
}

T_KRX_JANG_DIV_Print( T_KRX_JANG_DIV* ptr)
{
    printf( "----[ T_KRX_JANG_DIV ]------------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "배분적용구분코드              sAllocApplTpCd         1   38 = [%1.1s]     \n", ptr->sAllocApplTpCd);
    printf( "배분처리구분코드              sAllocProcsTpCd        1   39 = [%1.1s]     \n", ptr->sAllocProcsTpCd);
    printf( "배분해제시각                  sAllocReleasTime       9   40 = [%9.9s]     \n", ptr->sAllocReleasTime);
    printf( "------------------------------------------------------------------[ T_KRX_JANG_DIV ]----\n");

    return sizeof( T_KRX_JANG_DIV);
}

T_KRX_JANG_DIV_400_Print( T_KRX_JANG_DIV_400* ptr)
{
    printf( "----[ T_KRX_JANG_DIV_400 ]--------------------------------------------------------------\n");
    T_KRX_JANG_DIV_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_DIV, MAX_SR_LEN_400-SZ_KRX_JANG_DIV, ptr->sSpace);
    printf( "--------------------------------------------------------------[ T_KRX_JANG_DIV_400 ]----\n");

    return sizeof( T_KRX_JANG_DIV_400);
}

T_KRX_JANG_VI_Print( T_KRX_JANG_VI* ptr)
{
    printf( "----[ T_KRX_JANG_VI ]-------------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "ME처리시각                    sMeProcsTime           9   38 = [%9.9s]     \n", ptr->sMeProcsTime);
    printf( "VI해제시각                    sViReleasTime          9   47 = [%9.9s]     \n", ptr->sViReleasTime);
    printf( "VI적용구분코드                sViApplTpCd            1   56 = [%1.1s]     \n", ptr->sViApplTpCd);
    printf( "VI종류코드                    sViKindCd              1   57 = [%1.1s]     \n", ptr->sViKindCd);
    printf( "Add20150615                   sStaticViTgBasPrc     11   58 = [%11.11s]   \n", ptr->sStaticViTgBasPrc);
    printf( "Add20150615                   sDynmcViTgBasPrc      11   69 = [%11.11s]   \n", ptr->sDynmcViTgBasPrc);
    printf( "Add20150615                   sViTgPrc              11   80 = [%11.11s]   \n", ptr->sViTgPrc);
    printf( "Add20150615                   sStaticViTgDivrgRt    13   91 = [%13.13s]   \n", ptr->sStaticViTgDivrgRt);
    printf( "Add20150615                   sDynmcViTgDivrgRt     13  104 = [%13.13s]   \n", ptr->sDynmcViTgDivrgRt);
    printf( "-------------------------------------------------------------------[ T_KRX_JANG_VI ]----\n");

    return sizeof( T_KRX_JANG_VI);
}

T_KRX_JANG_VI_400_Print( T_KRX_JANG_VI_400* ptr)
{
    printf( "----[ T_KRX_JANG_VI_400 ]---------------------------------------------------------------\n");
    T_KRX_JANG_VI_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_VI, MAX_SR_LEN_400-SZ_KRX_JANG_VI, ptr->sSpace);
    printf( "---------------------------------------------------------------[ T_KRX_JANG_VI_400 ]----\n");

    return sizeof( T_KRX_JANG_VI_400);
}

T_KRX_JANG_LMT_Print( T_KRX_JANG_LMT* ptr)
{
    printf( "----[ T_KRX_JANG_LMT ]------------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "ME처리시각                    sMeProcsTime           9   38 = [%9.9s]     \n", ptr->sMeProcsTime);
    printf( "동적가격제한여부              sDynmcPrcLmtYn         1   47 = [%1.1s]     \n", ptr->sDynmcPrcLmtYn);
    printf( "동적상한가                    sDynmcUpLmtPrice      11   48 = [%11.11s]   \n", ptr->sDynmcUpLmtPrice);
    printf( "동적하한가                    sDynmcLwLmtPrice      11   59 = [%11.11s]   \n", ptr->sDynmcLwLmtPrice);
    printf( "------------------------------------------------------------------[ T_KRX_JANG_LMT ]----\n");

    return sizeof( T_KRX_JANG_LMT);
}

T_KRX_JANG_LMT_400_Print( T_KRX_JANG_LMT_400* ptr)
{
    printf( "----[ T_KRX_JANG_LMT_400 ]--------------------------------------------------------------\n");
    T_KRX_JANG_LMT_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_LMT, MAX_SR_LEN_400-SZ_KRX_JANG_LMT, ptr->sSpace);
    printf( "--------------------------------------------------------------[ T_KRX_JANG_LMT_400 ]----\n");

    return sizeof( T_KRX_JANG_LMT_400);
}

T_KRX_JANG_EXPN_Print( T_KRX_JANG_EXPN* ptr)
{
    printf( "----[ T_KRX_JANG_EXPN ]-----------------------------------------------------------------\n");
    printf( "데이타일련번호                sSeq                  11    0 = [%11.11s]   \n", ptr->sSeq);
    printf( "트랜잭션코드                  sTrCd                 11   11 = [%11.11s]   \n", ptr->sTrCd);
    printf( "ME그룹번호                    sMeGrpNo               2   22 = [%2.2s]     \n", ptr->sMeGrpNo);
    printf( "보드ID                        sBoardId               2   24 = [%2.2s]     \n", ptr->sBoardId);
    printf( "종목코드                      sJCode                12   26 = [%12.12s]   \n", ptr->sJCode);
    printf( "ME처리시각                    sMeProcsTm             9   38 = [%9.9s]     \n", ptr->sMeProcsTm);
    printf( "가격확대시각                  sPrcExpnTm             9   47 = [%9.9s]     \n", ptr->sPrcExpnTm);
    printf( "가격제한확대상한단계          sPleUpLmtStep          3   56 = [%3.3s]     \n", ptr->sPleUpLmtStep);
    printf( "가격제한확대하한단계          sPleLwLmtStep          3   59 = [%3.3s]     \n", ptr->sPleLwLmtStep);
    printf( "상한가                        sUpLmtPrc             11   62 = [%11.11s]   \n", ptr->sUpLmtPrc);
    printf( "하한가                        sLwLmtPrc             11   73 = [%11.11s]   \n", ptr->sLwLmtPrc);
    printf( "-----------------------------------------------------------------[ T_KRX_JANG_EXPN ]----\n");

    return sizeof( T_KRX_JANG_EXPN);
}

T_KRX_JANG_EXPN_400_Print( T_KRX_JANG_EXPN_400* ptr)
{
    printf( "----[ T_KRX_JANG_EXPN_400 ]-------------------------------------------------------------\n");
    T_KRX_JANG_EXPN_Print( &ptr->tData);
    printf( "SPACE                         sSpace               ***    0 = [%*.*s]     \n", MAX_SR_LEN_400-SZ_KRX_JANG_EXPN, MAX_SR_LEN_400-SZ_KRX_JANG_EXPN, ptr->sSpace);
    printf( "-------------------------------------------------------------[ T_KRX_JANG_EXPN_400 ]----\n");

    return sizeof( T_KRX_JANG_EXPN_400);
}

T_KRX_ORDER_FRGN_ADD_Print( T_KRX_ORDER_FRGN_ADD* ptr)
{
    printf( "----[ T_KRX_ORDER_FRGN_ADD ]------------------------------------------------------------\n");
    printf( "외국인고유번호                sFrgnNo                6    0 = [%6.6s]     \n", ptr->sFrgnNo);
    printf( "예비주문번호                  sExtraOrderNo         10    6 = [%10.10s]   \n", ptr->sExtraOrderNo);
    printf( "FILLER                        sFiller               24   16 = [%24.24s]   \n", ptr->sFiller);
    printf( "------------------------------------------------------------[ T_KRX_ORDER_FRGN_ADD ]----\n");

    return sizeof( T_KRX_ORDER_FRGN_ADD);
}

T_KRX_ORDER_FRGN_Print( T_KRX_ORDER_FRGN* ptr)
{
    printf( "----[ T_KRX_ORDER_FRGN ]----------------------------------------------------------------\n");
    T_KRX_ORDER_Print( &ptr->tKrxOrder);
    T_KRX_ORDER_FRGN_ADD_Print( &ptr->tKrxOrderFrgnAdd);
    printf( "----------------------------------------------------------------[ T_KRX_ORDER_FRGN ]----\n");

    return sizeof( T_KRX_ORDER_FRGN);
}

T_KRX_CONFIRM_FRGN_ADD_Print( T_KRX_CONFIRM_FRGN_ADD* ptr)
{
    printf( "----[ T_KRX_CONFIRM_FRGN_ADD ]----------------------------------------------------------\n");
    printf( "외국인고유번호                sFrgnNo                6    0 = [%6.6s]     \n", ptr->sFrgnNo);
    printf( "처리내용                      sRejectCd              4    6 = [%4.4s]     \n", ptr->sRejectCd);
    printf( "FILLER                        sFiller               30   10 = [%30.30s]   \n", ptr->sFiller);
    printf( "----------------------------------------------------------[ T_KRX_CONFIRM_FRGN_ADD ]----\n");

    return sizeof( T_KRX_CONFIRM_FRGN_ADD);
}

/*** Module : print.c ***/
int	T_KRX_HEAD_Print( T_KRX_HEAD* ptr);
int	T_KRX_HEARTBEAT_Print( T_KRX_HEARTBEAT* ptr);
int	T_KRX_Q_LOGON_Print( T_KRX_Q_LOGON* ptr);
int	T_KRX_R_LOGON_Print( T_KRX_R_LOGON* ptr);
int	T_KRX_QR_OPEN_Print( T_KRX_QR_OPEN* ptr);
int	T_KRX_QR_OPEN_1_Print( T_KRX_QR_OPEN_1* ptr);
int	T_KRX_Q_RESEND_Print( T_KRX_Q_RESEND* ptr);
int	T_KRX_R_RESEND_Print( T_KRX_R_RESEND* ptr);
int	T_KRX_Q_SEQUENCE_Print( T_KRX_Q_SEQUENCE* ptr);
int	T_KRX_R_SEQUENCE_Print( T_KRX_R_SEQUENCE* ptr);
int	T_KRX_Q_LOGOUT_Print( T_KRX_Q_LOGOUT* ptr);
int	T_KRX_R_LOGOUT_Print( T_KRX_R_LOGOUT* ptr);
int	T_KRX_ORDER_Print( T_KRX_ORDER* ptr);
int	T_KRX_ORDER_FMT_Print( T_KRX_ORDER_FMT* ptr);
int	T_KRX_ORDER_400_Print( T_KRX_ORDER_400* ptr);
int	T_KRX_ORDER_RESP_Print( T_KRX_ORDER_RESP* ptr);
int	T_COMBINATION_ORDER_Print( T_COMBINATION_ORDER* ptr);
int	T_COMBINATION_ORDER_400_Print( T_COMBINATION_ORDER_400* ptr);
int	T_KILL_SWITCH_Print( T_KILL_SWITCH* ptr);
int	T_KILL_SWITCH_400_Print( T_KILL_SWITCH_400* ptr);
int	T_KRX_SETTLE_Print( T_KRX_SETTLE* ptr);
int	T_KRX_SETTLE_FMT_Print( T_KRX_SETTLE_FMT* ptr);
int	T_KRX_SETTLE_400_Print( T_KRX_SETTLE_400* ptr);
int	T_KRX_CONFIRM_Print( T_KRX_CONFIRM* ptr);
int	T_KRX_CONFIRM_FMT_Print( T_KRX_CONFIRM_FMT* ptr);
int	T_KRX_CONFIRM_400_Print( T_KRX_CONFIRM_400* ptr);
int	T_KILL_SWITCH_CONFIRM_Print( T_KILL_SWITCH_CONFIRM* ptr);
int	T_KILL_SWITCH_CONFIRM_400_Print( T_KILL_SWITCH_CONFIRM_400* ptr);
int	T_SETTLE_IF_END_Print( T_SETTLE_IF_END* ptr);
int	T_SETTLE_IF_END_400_Print( T_SETTLE_IF_END_400* ptr);
int	T_CUST_INFO_Print( T_CUST_INFO* ptr);
int	T_KRX_JANG_Print( T_KRX_JANG* ptr);
int	T_KRX_JANG_400_Print( T_KRX_JANG_400* ptr);
int	T_KRX_JANG_FMT_Print( T_KRX_JANG_FMT* ptr);
int	T_KRX_JANG_INFO_Print( T_KRX_JANG_INFO* ptr);
int	T_KRX_JANG_INFO_400_Print( T_KRX_JANG_INFO_400* ptr);
int	T_KRX_JANG_BASE_Print( T_KRX_JANG_BASE* ptr);
int	T_KRX_JANG_BASE_400_Print( T_KRX_JANG_BASE_400* ptr);
int	T_KRX_JANG_STOP_Print( T_KRX_JANG_STOP* ptr);
int	T_KRX_JANG_STOP_400_Print( T_KRX_JANG_STOP_400* ptr);
int	T_KRX_JANG_END_Print( T_KRX_JANG_END* ptr);
int	T_KRX_JANG_END_400_Print( T_KRX_JANG_END_400* ptr);
int	T_KRX_JANG_DIV_Print( T_KRX_JANG_DIV* ptr);
int	T_KRX_JANG_DIV_400_Print( T_KRX_JANG_DIV_400* ptr);
int	T_KRX_JANG_VI_Print( T_KRX_JANG_VI* ptr);
int	T_KRX_JANG_VI_400_Print( T_KRX_JANG_VI_400* ptr);
int	T_KRX_JANG_LMT_Print( T_KRX_JANG_LMT* ptr);
int	T_KRX_JANG_LMT_400_Print( T_KRX_JANG_LMT_400* ptr);
int	T_KRX_JANG_EXPN_Print( T_KRX_JANG_EXPN* ptr);
int	T_KRX_JANG_EXPN_400_Print( T_KRX_JANG_EXPN_400* ptr);
int	T_KRX_ORDER_FRGN_ADD_Print( T_KRX_ORDER_FRGN_ADD* ptr);
int	T_KRX_ORDER_FRGN_Print( T_KRX_ORDER_FRGN* ptr);
int	T_KRX_CONFIRM_FRGN_ADD_Print( T_KRX_CONFIRM_FRGN_ADD* ptr);

typedef struct _packet_print_table_
{
	int			no;
	int			pos;
	char		data[ 512];
	int			(* act)();
	char		comment[ 512];
	
}	PACKET_PRINT_TABLE;

PACKET_PRINT_TABLE	PacketPrintTable[] =
{
	{	1,	11,		"TTRMIP31309",			T_KRX_JANG_EXPN_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31308",			T_KRX_JANG_LMT_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31307",			T_KRX_JANG_VI_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31306",			T_KRX_JANG_DIV_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31304",			T_KRX_JANG_END_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31303",			T_KRX_JANG_STOP_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31302",			T_KRX_JANG_BASE_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP32301",			T_KRX_JANG_INFO_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31301",			T_KRX_JANG_Print,				"장운영"}, 
	{	1,	11,		"TCHEDP",				T_SETTLE_IF_END_Print,			"SETTLE END"}, 
	{	1,	11,		"TTRKOR",				T_KILL_SWITCH_CONFIRM_Print,	"SETTLE Kill Switch"}, 
	{	1,	11,		"TTRODP",				T_KRX_CONFIRM_Print,			"CONFIRM"}, 
	{	1,	11,		"TTRTDP",				T_KRX_SETTLE_Print,				"SETTLE"}, 
	{	1,	11,		"TCHKDR1",				T_KILL_SWITCH_Print,			"ORDER Kill Switch"}, 
	{	1,	11,		"TCHCDR1",				T_COMBINATION_ORDER_Print,		"ORDER Combination"}, 
	{	1,	11,		"TCHODR1",				T_KRX_ORDER_Print,				"ORDER"}, 
	{	1,	14,		"TCHODR00000",			T_KRX_ORDER_FMT_Print,			"HEAD + ORDER"}, 
	{	1,	14,		"SCHLIQ00000",			T_KRX_Q_LOGON_Print,			"HEAD + LOGON"}, 
	{	1,	14,		"SCHLIR00000",			T_KRX_R_LOGON_Print,			"HEAD + LOGON 응답"}, 
	{	1,	14,		"SCHOPQ00000",			T_KRX_QR_OPEN_Print,			"HEAD + 업무개시"}, 
	{	1,	14,		"SCHOPQ10000",			T_KRX_QR_OPEN_1_Print,			"HEAD + 업무개시 1"}, 
	{	1,	14,		"SCHOPR00000",			T_KRX_QR_OPEN_Print,			"HEAD + 업무개시"}, 
	{	1,	14,		"SCHOPR10000",			T_KRX_QR_OPEN_1_Print,			"HEAD + 업무개시 1"}, 
	{	1,	14,		"SCHHEQ00000",			T_KRX_HEARTBEAT_Print,			"HEAD + 회선시험"}, 
	{	1,	14,		"SCHHER00000",			T_KRX_HEARTBEAT_Print,			"HEAD + 회선시험 응답"}, 
	{	1,	14,		"SCHLOQ00000",			T_KRX_Q_LOGOUT_Print,			"HEAD + LOGOUT"}, 
	{	1,	14,		"SCHLOR00000",			T_KRX_R_LOGOUT_Print,			"HEAD + LOGOUT 응답"}, 
	{	1,	0,		"KMAPv2.0",				T_KRX_HEAD_Print,				"HEAD"}, 
	{ 	-1,	-1,		"",						NULL,							""}
};


/*********************************************************************************************************
 *
*********************************************************************************************************/
PacketPrint( char *data)
{
	int					rtn;
	char				*ptr, buf[ 512];
	PACKET_PRINT_TABLE	*pp_tbl = &PacketPrintTable[ 0];
	PACKET_PRINT_TABLE	*pp_tbl_idx[ 512];
	int					idx = 0, pos;


	while( 1)
	{
		if( pp_tbl->no < 0) break;
		ptr = strstr( data, pp_tbl->data);
		if( ptr != NULL)
		{
			if( ptr - pp_tbl->pos < data)
			{
				pp_tbl++;
				continue;
			}
			pp_tbl_idx[ idx++] = pp_tbl;
		}
		pp_tbl++;
	}

	if( idx > 1)
	{
		pos = 0;
		while( pos < idx)
		{
			printf( "%3d ",		pos +1);
			printf( "%-20s ",	pp_tbl_idx[ pos]->data);
			printf( "%-30s ",	pp_tbl_idx[ pos]->comment);
			printf( "\n");
			pos++;
		}

		printf( "select view no: ");
		fgets( buf, 512, stdin);
		pos = atoi( buf);
	}
	else if( idx == 1)
	{
		pos = 1;
	}
	else
	{
		printf( "packet not found ... \n");
		return 1;
	}

	if( pos <= 0 || pos > idx) 
	{
		printf( "select error. input=[%d]\n", pos);
		return 1;
	}

	pp_tbl = pp_tbl_idx[ pos -1];
	ptr = strstr( data, pp_tbl->data);
	ptr -= pp_tbl->pos;
	pp_tbl->act( ptr);

	return 1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
TaskData( int argc, char *argv[])
{
	int		pos = 0, sz;
	char	*ptr;
	char	rec[ 8192];
	char	data[ 65535];

	printf( "input data ... ");
	printf( "[\"Enter\" key = 입력종료]\n");

	while( 1)
	{
		ptr = fgets( rec, 8192, stdin);
		if( rec[ 0] == '\n') break;
		ptr = strchr( rec, '\n');
		*ptr = 0;
		sz = strlen( rec);
		memcpy( &data[ pos], rec, sz +1);
		pos += sz;
	}

	PacketPrint( data);

	return 1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
main( int argc, char *argv[])
{
	TaskData( argc, argv);
	return 1;
}


