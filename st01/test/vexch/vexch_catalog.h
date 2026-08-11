/*------------------------------------------------------------------------
#   vexch_catalog.h — 가상거래소 상품 카탈로그 (공유)
#
#   cfg/vexch.ini 를 파싱해 상품 배열(vx_cat)로 로드. 거래소 엔진
#   (mock_krx_server=주문/체결)과 시세 발행기(vx_sise_pub)가 공유한다.
#   상품 추가 = cfg/vexch.ini 블록 1개 (코드 무변경).
------------------------------------------------------------------------*/
#ifndef VEXCH_CATALOG_H
#define VEXCH_CATALOG_H

#define VX_MAX_PRODUCT   32

typedef struct {
    char name[32];              /* [PRODUCT_*]                        */
    char market[2];             /* b채권 c파생 a현물 f FX             */
    /* 주문/체결 (TCP KRX) */
    char order_tr[16];          /* 주문 TrCode (거래소 수신)          */
    int  order_size;
    char resp_tr[16];           /* 회원처리호가(응답) TrCode          */
    int  resp_size;
    char exec_tr[16];           /* 회원체결결과(체결) TrCode          */
    int  exec_size;
    char wrapper[20];           /* push 래퍼 MsgType (TCHTDP00000)    */
    char fill_rule[12];         /* ack | full | partial | reject      */
    int  enabled;
    /* 주문 프로토콜 (거래원) */
    char order_proto[12];       /* krx(기본) | fx_smb (SMB_ST/TCP)     */
    char fx_excode[2];          /* FX 거래원: J JPM/N NH/E EBS/C CMB.. */
    int  fx_port;               /* FX 거래원 엔진 TCP 포트            */
    /* 시세 (UDP) */
    char sise_kind[8];          /* none | fx | krx                    */
    char sise_ip[20];
    int  sise_port;
    char sise_symbol[12];       /* fx root symbol                     */
    char sise_excode[2];        /* fx 거래소코드 1자                  */
    char sise_tr[16];           /* krx 시세 TrCode prefix (A301F 등)  */
    int  sise_size;             /* krx 시세 전문 바이트               */
} VX_PRODUCT;

extern VX_PRODUCT vx_cat[VX_MAX_PRODUCT];
extern int        vx_cat_cnt;

/* cfg/vexch.ini 로드 → vx_cat[] 채움. 반환=상품 수(>=0), -1=파일열기실패.
   log_fn: 로그 콜백(NULL이면 stdout). */
int vx_load_catalog(const char *path, void (*log_fn)(const char *));

#endif  /* VEXCH_CATALOG_H */
