#ifndef _KRX_TTSSDP20003_H
#define _KRX_TTSSDP20003_H

/* TTSSDP20003 일별 종목별 주문체결 수량송신 - KRX EXTURE 3.0 v3.24 전문 (자동생성, DATA부) */
/* DATA 길이 합계 = 402 (interface-list.csv '길이(헤더제외)'와 대조) */
typedef struct {
    char Message_Sequence_Number[11];           /*    1 메세지일련번호 (Long 11) */
    char Transaction_Code[11];                  /*    2 트랜잭션코드 (String 11) */
    char Market_Identification[3];              /*    3 시장ID (String 3) */
    char Transmit_Date[8];                      /*    4 전송일자 (String 8) */
    char Trading_Date[8];                       /*    5 매매일자 (String 8) */
    char Class_Id[11];                          /*    6 상품ID (String 11) */
    char Issue_Code[12];                        /*    7 종목코드 (String 12) */
    char Expmm_No[4];                           /*    8 결제월번호 (Long 4) */
    char Ask_Nmlord_Qty[12];                    /*    9 매도정상호가수량 (Long 12) */
    char Bid_Nmlord_Qty[12];                    /*   10 매수정상호가수량 (Long 12) */
    char Ask_Modord_Qty[12];                    /*   11 매도정정호가수량 (Long 12) */
    char Bid_Modord_Qty[12];                    /*   12 매수정정호가수량 (Long 12) */
    char Ask_Canclord_Qty[12];                  /*   13 매도취소호가수량 (Long 12) */
    char Bid_Canclord_Qty[12];                  /*   14 매수취소호가수량 (Long 12) */
    char Trdvol[10];                            /*   15 체결수량 (Long 10) */
    char Open_Price_Single_Price_Auction_Bid_Normal_Qorder_Quantity[12];/*   16 시가단일가매수정상호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Normal_Qorder_Quantity[12];/*   17 시가단일가매도정상호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Bid_Modified_Order_Quantity[12];/*   18 시가단일가매수정정호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Modified_Order_Quantity[12];/*   19 시가단일가매도정정호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Bid_Canceled_Order_Quantity[12];/*   20 시가단일가매수취소호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Ask_Canceled_Order_Quantity[12];/*   21 시가단일가매도취소호가수량 (Long 12) */
    char Continous_Auction_Bid_Normal_Qorder_Quantity[12];/*   22 접속매매매수정상호가수량 (Long 12) */
    char Continous_Auction_Ask_Normal_Qorder_Quantity[12];/*   23 접속매매매도정상호가수량 (Long 12) */
    char Continous_Auction_Bid_Modified_Order_Quantity[12];/*   24 접속매매매수정정호가수량 (Long 12) */
    char Continous_Auction_Ask_Modified_Order_Quantity[12];/*   25 접속매매매도정정호가수량 (Long 12) */
    char Continous_Auction_Bid_Canceled_Order_Quantity[12];/*   26 접속매매매수취소호가수량 (Long 12) */
    char Continous_Auction_Ask_Canceled_Order_Quantity[12];/*   27 접속매매매도취소호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Normal_Qorder_Quantity[12];/*   28 종가단일가매수정상호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Normal_Qorder_Quantity[12];/*   29 종가단일가매도정상호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Modified_Order_Quantity[12];/*   30 종가단일가매수정정호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Modified_Order_Quantity[12];/*   31 종가단일가매도정정호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Bid_Canceled_Order_Quantity[12];/*   32 종가단일가매수취소호가수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Ask_Canceled_Order_Quantity[12];/*   33 종가단일가매도취소호가수량 (Long 12) */
    char Open_Price_Single_Price_Auction_Trading_Volumn[12];/*   34 시가단일가체결수량 (Long 12) */
    char Continous_Auction_Trading_Volumn[12];  /*   35 접속매매체결수량 (Long 12) */
    char Closing_Price_Single_Price_Call_Auction_Trading_Volumn[12];/*   36 종가단일가체결수량 (Long 12) */
} TTSSDP20003_DATA;

#endif  /* _KRX_TTSSDP20003_H */
