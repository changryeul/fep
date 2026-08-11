#ifndef _KRX_TTSSDP10001_H
#define _KRX_TTSSDP10001_H

/* TTSSDP10001 불공정거래 모니터링 지원정보 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 632 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Trading_Date[8];                       /*    5 거래일자 (String 8) */
    char Issue_Code[12];                        /*    6 종목코드 (String 12) */
    char Pre_Off_Hours_Session_Closing_Price_Bid_Normal_Qorder_Quantity[12];/*    7 장개시전시간외종가매수정상호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Ask_Normal_Qorder_Quantity[12];/*    8 장개시전시간외종가매도정상호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Bid_Modified_Order_Quantity[12];/*    9 장개시전시간외종가매수정정호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Ask_Modified_Order_Quantity[12];/*   10 장개시전시간외종가매도정정호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Bid_Canceled_Order_Quantity[12];/*   11 장개시전시간외종가매수취소호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Ask_Canceled_Order_Quantity[12];/*   12 장개시전시간외종가매도취소호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Bid_Normal_Qorder_Quantity[12];/*   13 시가단일가매수정상호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Normal_Qorder_Quantity[12];/*   14 시가단일가매도정상호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Bid_Modified_Order_Quantity[12];/*   15 시가단일가매수정정호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Modified_Order_Quantity[12];/*   16 시가단일가매도정정호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Bid_Canceled_Order_Quantity[12];/*   17 시가단일가매수취소호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Canceled_Order_Quantity[12];/*   18 시가단일가매도취소호가수량 (Long 12) */
    char Continous_Auction_Bid_Normal_Qorder_Quantity[12];/*   19 접속매매매수정상호가수량 (Long 12) */
    char Continous_Auction_Ask_Normal_Qorder_Quantity[12];/*   20 접속매매매도정상호가수량 (Long 12) */
    char Continous_Auction_Bid_Modified_Order_Quantity[12];/*   21 접속매매매수정정호가수량 (Long 12) */
    char Continous_Auction_Ask_Modified_Order_Quantity[12];/*   22 접속매매매도정정호가수량 (Long 12) */
    char Continous_Auction_Bid_Canceled_Order_Quantity[12];/*   23 접속매매매수취소호가수량 (Long 12) */
    char Continous_Auction_Ask_Canceled_Order_Quantity[12];/*   24 접속매매매도취소호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Normal_Qorder_Quantity[12];/*   25 종가단일가매수정상호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Normal_Qorder_Quantity[12];/*   26 종가단일가매도정상호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Modified_Order_Quantity[12];/*   27 종가단일가매수정정호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Modified_Order_Quantity[12];/*   28 종가단일가매도정정호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Canceled_Order_Quantity[12];/*   29 종가단일가매수취소호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Canceled_Order_Quantity[12];/*   30 종가단일가매도취소호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Bid_Normal_Qorder_Quantity[12];/*   31 장종료후시간외종가매수정상호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Ask_Normal_Qorder_Quantity[12];/*   32 장종료후시간외종가매도정상호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Bid_Modified_Order_Quantity[12];/*   33 장종료후시간외종가매수정정호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Ask_Modified_Order_Quantity[12];/*   34 장종료후시간외종가매도정정호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Bid_Canceled_Order_Quantity[12];/*   35 장종료후시간외종가매수취소호가수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Ask_Canceled_Order_Quantity[12];/*   36 장종료후시간외종가매도취소호가수량 (Long 12) */
    char After_Market_Bid_Normal_Qorder_Quantity[12];/*   37 시간외단일가애프터마켓매수정상호가수량 (Long 12) */
    char After_Market_Ask_Normal_Qorder_Quantity[12];/*   38 시간외단일가애프터마켓매도정상호가수량 (Long 12) */
    char After_Market_Bid_Modified_Order_Quantity[12];/*   39 시간외단일가애프터마켓매수정정호가수량 (Long 12) */
    char After_Market_Ask_Modified_Order_Quantity[12];/*   40 시간외단일가애프터마켓매도정정호가수량 (Long 12) */
    char After_Market_Bid_Canceled_Order_Quantity[12];/*   41 시간외단일가애프터마켓매수취소호가수량 (Long 12) */
    char After_Market_Ask_Canceled_Order_Quantity[12];/*   42 시간외단일가애프터마켓매도취소호가수량 (Long 12) */
    char Pre_Off_Hours_Session_Closing_Price_Trading_Volumn[12];/*   43 장개시전시간외종가체결수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Trading_Volumn[12];/*   44 시가단일가체결수량 (Long 12) */
    char Continous_Auction_Trading_Volumn[12];  /*   45 접속매매체결수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Trading_Volumn[12];/*   46 종가단일가체결수량 (Long 12) */
    char Post_Off_Hours_Session_Closing_Price_Trading_Volumn[12];/*   47 장종료후시간외종가체결수량 (Long 12) */
    char After_Market_Trading_Volumn[12];       /*   48 시간외단일가애프터마켓체결수량 (Long 12) */
    char Closing_Price_Last_Trading_Price[11];  /*   49 종가직전체결가격 (Float 11) */
    char Regulss_After_Market_Bid_Modord_Cnt[16];/*   50 정규장애프터마켓매수정정호가건수 (Long 16) */
    char Regulss_After_Market_Ask_Modord_Cnt[16];/*   51 정규장애프터마켓매도정정호가건수 (Long 16) */
    char Regulss_After_Market_Bid_Canclord_Cnt[16];/*   52 정규장애프터마켓매수취소호가건수 (Long 16) */
    char Regulss_After_Market_Ask_Canclord_Cnt[16];/*   53 정규장애프터마켓매도취소호가건수 (Long 16) */
} TTSSDP10001_DATA;

#endif  /* _KRX_TTSSDP10001_H */
