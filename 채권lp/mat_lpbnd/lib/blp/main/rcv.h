char     sb31_exe_ymd                 [  8+1];  // 1  실행일자          
int      sb31_exe_no                         ;  // 2  실행번호
char     sb31_proc_stus_dstcd         [  1+1];  // 3  처리상태구분코드
char     sb31_start_yms               [ 14+1];  // 4  시작일시              
char     sb31_end_yms                 [ 14+1];  // 5  종료일시              
char     sb31_lp_start_yms            [ 14+1];  // 6  유동성공급시작일시    
char     sb31_lp_end_yms              [ 14+1];  // 7  유동성공급종료일시    
char     sb31_item_cd                 [ 12+1];  // 8  종목코드              
char     sb31_mm_item_dstcd           [  2+1];  // 9  유동성공급종목구분코드
double   sb31_sped1_prc                      ;  // 10 스프래드1가격         
double   sb31_sped2_prc                      ;  // 11 스프래드2가격         
double   sb31_sped3_prc                      ;  // 12 스프래드3가격         
double   sb31_ord1_qanty                     ;  // 13 주문1수량             
double   sb31_ord2_qanty                     ;  // 14 주문2수량             
double   sb31_ord3_qanty                     ;  // 15 주문3수량             
int      sb31_ord_qanty_unit                 ;  // 16 주문수량단위          
char     sb31_dspratio_taget_dstcd    [  2+1];  // 17 괴리율대상구분코드    
char     sb31_dspratio_dstcd          [  1+1];  // 18 KRX괴리율구분코드         
double   sb31_dspratio                       ;  // 19 괴리율                
double   sb31_tick_unit                      ;  // 20 틱단위                
char     sb31_trdr_uno                [  5+1];  // 21 거래원번호            
char     sb31_trdr_no                 [  4+1];  // 22 CMBS트레이더번호      
double   sb31_prv_yild                       ;  // 23 민간평가수익률        
double   sb31_prv_prc                        ;  // 24 민간평가가격          
double   sb31_clsng_prc                      ;  // 25 종가                  
double   sb31_clsng_yild                     ;  // 26 종가수익률            

