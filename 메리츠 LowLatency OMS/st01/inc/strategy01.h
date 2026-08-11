#ifndef     __STRATEGY01_H
#define     __STRATEGY01_H
/*------------------------------------------------------------------------
#   Module  : common header files
#   File    : strategy.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include <stdbool.h>
//#include  "fep_sub.h"
//#include  "fep_interface.h"

#ifndef BASKET_MAX
#define BASKET_MAX 300
#endif

// 다음회차 진행 조건
typedef struct {
    double      FillRate;
    double      Basis;  
    int         CmpType;
} NextCond_t;

// 자동정정조건
typedef struct AroCond {
    int         Enable                      ;       // 자동정정여부 (0:자동정정X 1:자동정정O)
    double      Time                        ;       // 조건체크 시간 간격(설정된 시간 간격마다 조건확인)
    int         MonitorCnt                  ;       // 호가수량 sum할 갯수(1~5)
    double      Ratio                       ;       // 자동정정기준값(%) = sum(상대호가수량)/sum(자기호가수량)*100
    int         Tick                        ;       // 정정시호가간격(-5~+5)  +:상대호가쪽으로 -:자기호가쪽으로
    int         MaxCnt                      ;       // 최대 자동정정을 수행할 횟수
    int         ClearMode                   ;       // 최대 자동정정후 미체결수량 처리방법 1:상대1호가로정정주문 0:취소주문
    double      QuantityRatio               ;       // 정정수량 %(남은 수량은 자동정정X)
} AroCond_t;

// 주문진행상황 정보
typedef struct {
    double      OrderedQty                  ;       // 주문수량
    double      OrderedAmt                  ;       // 주문금액
    double      FilledQty                   ;       // 체결수량
    double      FilledAmt                   ;       // 체결금액 =sum(체결가*체결수량)
    double      FilledAmtOP                 ;       // 체결금액 =sum(주문가*체결수량)
    double      FillRate                    ;       // 체결율   =FilledAmtOP/OrderedAmt*100
} ProgressInfo_t;

// 바스켓 주문 정보
typedef struct {
    int             ShmIdx                  ;       // 공유메모리인덱스
    int             IssIdx                  ;       // 종목인덱스
    int             IssueType               ;       // 0:default 0:유가증권 1:ELW 2:ETN 3:ETF, 0:코스피주식선물  1:코드닥주식선물
    char            Code                [12];       // 종목코드
    int             Side                    ;       // 매도/매수구분 1:매도, 2:매수
    int             OrderPrcType            ;       // 호가타입 (1:상대2호가 2:상대1호가 3:현재가 4:자기1호가 5:자기2호가)
    double          OrderQty                ;       // 1회주문수량
    int             Order_Condition         ;       // 호가조건 0:일반(FAS) 3:IOC(FAK) 4:FOK
    char            Account_Type        [ 4];       // 계좌구분코드
    char            Account_Margin_Type [ 4];       // 계좌증거금 유형코드
    char            PositionType        [ 1];       // 1:long매도 2:short매도
    double          Multiplier              ;       // 승수
    double          CurrentRatio            ;       // 유동비율
    double          ListedNumber            ;       // 상장주식수
    double          Price               [ 5];       // 종목가격현재가(상대2호가,상대1호가,현재가,자기1호가,자기2호가) 
    AroCond_t       AroCond                 ;       // 자동정정조건
    ProgressInfo_t  InfoSum                 ;       // 전체진행정보(회차별 합계)
    ProgressInfo_t  InfoCurr                ;       // 마지막회차정보
} BskOrder_t;

// 바스켓 정보
typedef struct Basket {
    int             StrategyNo              ;       // 전략번호(Shm 인덱스)
    char            Ip                [16+1];       // IP주소
    char            MacAddr           [16+1];       // MAC
    char            Name              [16+1];       // 이름
    char            FundCodeBsk       [16+1];       // 바스켓펀드코드
    char            FundCodeHdg       [16+1];       // 헷지펀드코드
    int             AccountIdxBsk           ;       // 계좌인덱스(바스켓)
    int             AccountIdxHdg           ;       // 계좌인덱스(헷지)
    char            ProcessNick       [ 8+1];       // 프로세스별명
    char            ChannelCode         [ 8];       // 082:파생본부
    char            OperatorId          [12];       // 사번
    double          BaseIndex               ;       // 지수전일종가
    int             RepeatCnt               ;       // 반복횟수
    int             RepeatNo                ;       // 현재반복차수
    int             BasketCnt               ;       // 바스켓종목수
    double          Basis                   ;       // 베이시스(헤지-바스켓)
    int             Pause                   ;       // 일시중지(0:동작, 1:일시중지)
    int             ResultCnt               ;       // 신규주문시 응답 받은 갯수
    int             Date                    ;       // 날짜
    ProgressInfo_t  HdgInfoSum              ;       // 헷지진행정보(전체:회차별합계)
    ProgressInfo_t  BskInfoSum              ;       // 바스켓진행정보(전체:회차별합계)
    ProgressInfo_t  HdgInfoCurr             ;       // 헷지진행정보(현재:마지막회차)
    ProgressInfo_t  BskInfoCurr             ;       // 바스켓진행정보(현재:마지막회차)
    NextCond_t      NextCond                ;       // 다음반복실행조건
    BskOrder_t      Hdg                     ;       // 헤지주문
    BskOrder_t      Bsk         [BASKET_MAX];       // 바스켓주문
} Basket_t;


#define ARBITRAGE_MAX               100

typedef struct {
    int         GroupNo                 ;           // 그룹번호
    char        TypeABC            [1+1];           // 타입 'A', 'B', 'C'
    bool        OnFlag                  ;           // On/Off
    char        FundCode            [16];           // 펀드코드
    int         AccountIdx              ;           // 계좌인덱스
    int         ShmIdx                  ;           // 시세공유메모리인덱스
    int         IssIdx                  ;           // 종목인덱스
    double      OrderQty                ;           // 주문수량
    int         RepeatMax               ;           // 해당그룹전략을 수행할 총횟수(화면:Time)
    int         OrderPrcType            ;           // 호가타입 (1:상대2호가 2:상대1호가 3:현재가 4:자기1호가 5:자기2호가)
    int         Side                    ;           // 매도/매수구분 1:매도 2:매수
    int         Order_Condition         ;           // 호가조건 0:일반(FAS) 3:IOC(FAK) 4:FOK
    int         ArbtType                ;           // 차익 타입(1, 2, 3)
    double      BuyBasis                ;           // 매수베이시스
    double      SellBasis               ;           // 매도베이시스
    double      BuyMrktBasis            ;           // 매수시장베이시스
    double      SellMrktBasis           ;           // 매도시장베이시스
    double      Multiplier              ;           // 승수
    double      Interval                ;           // 1회 체결후 다음조건을 감시할 인터벌(초)
    char        PriorityFlag       [1+1];           // 우선체결 ('Y':우선체결 ' ':우선체결아님)
    int         RepeatNo                ;           // 발주횟수
    double      OrderedQty              ;           // 발주수량
    double      OrderedAmt              ;           // 발주금액
    double      FillQty                 ;           // 체결수량
    double      FillAmt                 ;           // 체결금액
    double      LastOrderTime           ;           // 마지막발주시간
    double      MrktBuyBasis            ;           // 시장매수베이시스
    double      MrktSellBasis           ;           // 시장매도베이시스
} ArbtOrder_t;


typedef struct ArbtSet {
    int         GroupCount              ;           // 차익익거래갯수(그룹갯수)
    char        Name              [16+1];           // 차익거래이름
    int         Date                    ;           // 날짜
    char        Ip                [16+1];           // IP 주소
    char        MacAddr           [16+1];           // MAC
    char        OperatorId        [16+1];           // 사원번호
    ArbtOrder_t ArbtOrder[ARBITRAGE_MAX][3];        // 착익거래주문
} ArbtSet_t;


/*************************************************************************
    End of Program (strategy.h)
*************************************************************************/
#endif
