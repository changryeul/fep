#ifndef SIMPLE_MSGQ_H
#define SIMPLE_MSGQ_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "adapter.h"

typedef struct {
    int     strat_idx;
    int     set_idx;
    int     set_id;
    long    ord_id;
    int     leg;     // 현물/선물
    int     side;
    long    qty;
    double  px;
    int     ts_hhmmss;
} OrderLogMsg;

//------------------------------------------------------------------------------- 
//  현물후처리내부 (주문내용외 부가정보) 
//------------------------------------------------------------------------------- 
typedef struct { 
    char        lgen_no                 [ 10]   ;  // 거래참여자번호 
    char        bk_no                   [ 11]   ;  // 북번호 
    char        rulez_no                [ 15]   ;  // Rule 번호 (with 전략번호) 
    char        strg_grp_no             [ 10]   ;  // 전략그룹주문번호 
    char        trdr_no                 [  4]   ;  // 트레이더번호
    char        auto_tran_dstcd         [  1]   ;  // 자동거래구분코드
												   // 'A'-USD 차익 'B'-JPY 차익 'C'-CNH 차익 'N'-NDF 차익
    SMB_ORD     smb_rcv                         ;  // SMB전문 
} SMB_STRGMSG;

//------------------------------------------------------------------------------- 
//  선물후처리내부 (주문내용외 부가정보) 
//------------------------------------------------------------------------------- 
typedef struct { 
    char        body_len                [  3];   // Body Length
    char        channel                 [  1];   // E:원장 G:Algo
    char        product_dstcd           [  1];   // A.통화선물 B.채권선물 C.채권 D.FX
    char        fcm_id                  [ 10];   // 증권/선물 회원사 번호
    char        rulez_no                [ 15];   // Rule 번호 (with 전략번호)
    char        ref_no                  [ 15];   // 참조번호: 자동거래구분코드(1자리) + 실행일자(8자리) + 일련번호(6자리)
    char        strg_grp_no             [ 10];   // 전략그룹번호. OMS에서는 기본 셋팅. 화면에서는 0 셋팅(신규/정정/취소 모두).
    char        bk_no                   [ 11];   // 북번호
    char        trdr_no                 [  4];   // 트레이더번호 
    char        day_ngt_dstcd           [  1];   // 주야간구분코드 : 주간-1 야간-2 
	YSKMSG_ORD  ysk_rcv                      ;   // KRX전문 
} YSK_STRGMSG;

typedef struct {
   long mtype;
   char mtext[2048];
} MsgBuf;

// ===== 함수 프로토타입
int msg_queue_init(int msg_type);
int msg_queue_send(int msgid,  MsgBuf *msg, int mtext_data_len);

// SPOT 주문 후처리 msg 조립
int orderlogq_msg_build (OrderLogMsg *msg, int packet_type, SMB_STRGMSG *spot_out, YSK_STRGMSG *fut_out);

// 차익자동정지알림메세지
int arb_stop_msg_send(char *msg);

//------------------------------------------------------------------------------- 
// 차익거래 전송 메세지 
//------------------------------------------------------------------------------- 
typedef struct { 
    char        exe_ymd                 [  8];  // 1 실행일자
	char        exe_no                  [  5];  // 2 실행번호
    char        bk_no                   [ 11];  // 3 북번호 
	char        trdr_no                 [  4];  // 4 트레이더번호
	char        rule_id                 [  5];  // 5 룰ID
	char        stop_msg                [128];  // 6 정지메세지 
} ALRTMSG;

// ALRT_TYPE /fsfxwin/win/src/mon/keioms/BW100010 데몬 식별자 
#define ALRT_TYPE   551001
#define E5001_MSG  "비정상 호가 수신 Error. 전산담당자에게 문의하세요.|"
#define E5002_MSG  "현선물차익거래 전략 자동 종료되었습니다.|"
#define E5003_MSG  "선물 종목마스터 미수신 상태. 전산담당자에게 문의하세요.|"
#define E5004_MSG  "전략별 할당 주문번호 대역이 초과되었습니다. 전산담당자에게 문의하세요.|"

#define E5101_MSG  "청산 대상 잔여 포지션 없음. 포지션 청산 완료|"
#define E5102_MSG  "포지션 불균형 상태로 전산담당자에게 문의하세요.|"
#define E5103_MSG  "현물 주문재시도 횟수 초과|"

#define E5201_MSG  "선물 주문 거부(거부사유: [5531]선물거래내역)|"
#define E5202_MSG  "현물 주문 거부|"
#define E5203_MSG  "현물 주문 거부, 선물 주문 거부(거부사유: [5531]선물거래내역)|"

#define E9999_MSG  "현선물차익거래 System Error. 전산담당자에게 문의하세요.|"

#endif
