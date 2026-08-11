#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define COMM_HDR 50
#define DATA_HDR 50
#define TOTAL_HDR (COMM_HDR + DATA_HDR)
#define MAGIC "MAGC"
#define MAX_BODY 8192

typedef struct {
        char    Length [4];         /* Length Field             */
        char    MsgType[4];         /* Message Type             */
                                    // LINK/LIOK, LIVE/LIVE, DATA/DAOK, EROR
        char    ResponsCode[4];     /* OK:0000, NOTOK : !0000   */
        char    TradeDate[8];       /* Trade Data   '20200101'  */
        char    SeqNo[8];           /* Sequence number          */
        char    Filler[22];         /* Space                    */
}CommHdr;

typedef struct {
   char    TrCode [6];              /* TrCode               */     
   char    Scr_key[4];              /* 화면키   cid로 사용   */
   char    ErrCode[4];              /* Error Code           */     
   char    ApType_Cd[5];            /* Aptype Code          */     
                                    /* Process 이름 (50101) */     
                                    /* 자동기동/자동종료/강제종료시 */
   char    Media_gbn[1];            /* 매체구분(A/C/T)      */     
   char    Cid   [10];              /* cid값                */     
   char    Filler[20];              /* 예비                 */     
}DataHdr;

typedef struct {
    char     sb31_exe_ymd                 [  8];  // 1  실행일자          
    char     sb31_exe_no                  [5]         ;  // 2  실행번호
    char     sb31_proc_stus_dstcd         [  1];  // 3  처리상태구분코드
    char     sb31_start_yms               [ 14];  // 4  시작일시              
    char     sb31_end_yms                 [ 14];  // 5  종료일시              
    char     sb31_lp_start_yms            [ 14];  // 6  유동성공급시작일시    
    char     sb31_lp_end_yms              [ 14];  // 7  유동성공급종료일시    
    char     sb31_item_cd                 [ 12];  // 8  종목코드              
    char     sb31_mm_item_dstcd           [  2];  // 9  유동성공급종목구분코드
    char     sb31_sped1_prc               [11] ;  // 10 스프래드1가격         
    char     sb31_sped2_prc               [11] ;  // 11 스프래드2가격         
    char     sb31_sped3_prc               [11]   ;  // 12 스프래드3가격         
    char     sb31_ord1_qanty              [10]  ;  // 13 주문1수량             
    char     sb31_ord2_qanty              [10]    ;  // 14 주문2수량             
    char     sb31_ord3_qanty              [10]    ;  // 15 주문3수량             
    char     sb31_ord_qanty_unit          [5]      ;  // 16 주문수량단위          
    char     sb31_dspratio_taget_dstcd    [  2];  // 17 괴리율대상구분코드    
    char     sb31_dspratio_dstcd          [  1];  // 18 KRX괴리율구분코드         
    char     sb31_dspratio                [11] ;  // 19 괴리율                
    char     sb31_tick_unit               [11] ;  // 20 틱단위                
    char     sb31_trdr_uno                [  5];  // 21 거래원번호            
    char     sb31_trdr_no                 [  4];  // 22 CMBS트레이더번호      
    char     sb31_prv_yild                [11]  ;  // 23 민간평가수익률        
    char     sb31_prv_prc                 [11]  ;  // 24 민간평가가격          
    char     sb31_clsng_prc               [11]  ;  // 25 종가                  
    char     sb31_clsng_yild              [11]  ;  // 26 종가수익률            
    char     sb31_acct_no                 [11]; // 계좌번호
} sb31;

int parse_headers(const char in[TOTAL_HDR],  uint32_t *cid_out, int *body_len);
void build_headers(  int body_len, char out[TOTAL_HDR]);
 
int send_packet_fd_timed(int fd, CommHdr *ch, DataHdr *dh, void * body, int body_len, int timeout_ms);
int recv_packet_fd_timed(int fd, CommHdr *ch, DataHdr *dh, void * body, int *body_len,  int max_body, int timeout_ms);

#endif
