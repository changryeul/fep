#include "/fsfxwin/fep/arb/include/market.h"
#include "/fsfxwin/fep/arb/include/log.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

static int CmpExpcode(const void *a, const void *b);
static int BsearchMode(char *base, char *sd, int recnum, int recsize, int (*compar)(), int mode);
static int fut_key_search (int tot_cnt, char *keybuf);
double prev_px_fut_bid = 0, prev_px_fut_ask = 0, prev_px_spot_bid = 0, prev_px_spot_ask = 0; 

SHM_FIN_FUT *g_md_shm_fut;
SHM_FX      *g_md_shm_spot;

static inline void _log_timestamp(char *buf, size_t bufsz) {
    struct timeval tv; gettimeofday(&tv, NULL);
    struct tm tm; localtime_r(&tv.tv_sec, &tm);
    long us = tv.tv_usec;
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &tm);
    size_t n = strlen(buf);
    snprintf(buf + n, bufsz - n, ".%06ld", us);
}
// ------------------------------------------------------------
// market_snapshot
//  기능: 실제 시세 데이터 시세 주입 (공유 메모리 G_SNAP에 직접)
// ------------------------------------------------------------
int market_snapshot(int fut_idx, double sp, double bp)
{
	char tmp[30];
	double px_fut_bid = 0, px_fut_ask = 0, px_spot_bid = 0, px_spot_ask = 0; 
	int fut_qty_ask = 0, fut_qty_bid = 0;

	/* 선물가격 */
	memset(tmp, 0x00, sizeof(tmp));
	memcpy(tmp, g_md_shm_fut[fut_idx].B6.bid1_price, sizeof(g_md_shm_fut[fut_idx].B6.bid1_price));  // 선물매수1단계우선호가가격
    px_fut_bid   = strtod(tmp, NULL);

	memset(tmp, 0x00, sizeof(tmp));
	memcpy(tmp, g_md_shm_fut[fut_idx].B6.ask1_price , sizeof(g_md_shm_fut[fut_idx].B6.ask1_price)); // 선물매도1단계우선호가가격 
    px_fut_ask   = strtod(tmp, NULL);

	/* 현물가격 */
	px_spot_bid  = g_md_shm_spot->B6.bidprc; 
	px_spot_ask  = g_md_shm_spot->B6.offerprc; 

	memset(tmp, 0x00, sizeof(tmp));
	memcpy(tmp, g_md_shm_fut[fut_idx].B6.bid1_qty , sizeof(g_md_shm_fut[fut_idx].B6.bid1_qty)); 
	fut_qty_bid  = atoi(tmp);

	memset(tmp, 0x00, sizeof(tmp));
	memcpy(tmp, g_md_shm_fut[fut_idx].B6.ask1_qty , sizeof(g_md_shm_fut[fut_idx].B6.ask1_qty)); 
	fut_qty_ask  = atoi(tmp);

	char ts[32]; 
	_log_timestamp(ts, sizeof(ts));

	if (((px_spot_bid + sp) - px_fut_ask >= bp) && fut_qty_ask > 99) {
		printf("[%s]선물매수ask Sp[%.2f]Bp[%.2f]SELL[%.2f] S[bid:%.2f ask:%.2f] F[bid:%.2f ask:%.2f qty_bid:%d qty_ask:%d]\n",
			ts, sp, bp, (px_spot_bid + sp) - px_fut_ask, px_spot_bid, px_spot_ask, px_fut_bid, px_fut_ask, fut_qty_bid, fut_qty_ask);

	}

	if ((px_fut_bid - (px_spot_ask + sp) >= bp) && fut_qty_bid > 99) {
		printf("[%s]선물매도bid Sp[%.2f]Bp[%.2f]BUY[%.2f] S[bid:%.2f ask:%.2f] F[bid:%.2f ask:%.2f qty_bid:%d qty_ask:%d]\n",
			ts, sp, bp, px_fut_bid - (px_spot_ask + sp), px_spot_bid, px_spot_ask, px_fut_bid, px_fut_ask, fut_qty_bid, fut_qty_ask);

	}

	return 0;
}

/******************************************************************************
 * * FUNCTION:    CmpIDXMasterShcode
 * * DESCRIPTION: Qrery key 비교함수.
 * * PARAMETERS:
 * *              const void *a - Compare memory A
 * *              const void *b - Compare memory B
 * * RETURNED:    compare value
 * ******************************************************************************/
static int CmpExpcode(const void *a, const void *b)
{
    KS_EXPCODE *aa, *bb;
    aa = (KS_EXPCODE *)a;
    bb = (KS_EXPCODE *)b;
    return memcmp(aa->expcode, bb->expcode, sizeof(aa->expcode));
}


/*******************************************************************************
 * 설명      : Mode(LE,LT,EQ,GE,GT)를 가지고 해당 Position을 찾아냄.
 * Prototype : int BsearchMode(char *base, char *sd, int recnum, int recsize,
 *                 int (*compar)(), int mode);
 * Arguments :  char    *base       - 전체 Space
 *              char    *sd         - Key record
 *              int     recnum      - Record count
 *              int     recsize     - Record size
 *              int     (*compar)() - Compare Function
 *              int     mode        - LE,LT,EQ,GT,GE
 * Return    : Success - Search index, Fail - -1
 * ****************************************************************************/
static int BsearchMode(char *base, char *sd, int recnum, int recsize, int (*compar)(), int mode)
{
    int fpos, lpos, cpos, cmp;

    if (recnum <= 0) return -1;

    if (recnum == 1) {
        cmp = compar(base,sd);
        if (cmp==0) {
            if ((mode == EQ)||(mode == LE)||(mode == GE)) return 0;
        }
        else if (cmp < 0) {
            if ((mode == LT)||(mode == LE)) return 0;
        }
        else {
            if ((mode == GT)||(mode == GE)) return 0;
        }

        return -1;
    }

    fpos = 0;
    lpos = recnum - 1;
    cpos = (recnum - 1) / 2;

    while(1) {
        cmp = compar(base+(recsize*cpos),sd);
        if (cmp==0) {
            if ((mode==EQ)||(mode==LE)||(mode==GE)) {
                if ((cpos>=0)&&(cpos<recnum)) return cpos;
                else                         return -1;
            }
            else if (mode==GT) {
                if (((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                else                                 return -1;
            }
            else {
                if (((cpos-1)>=0)&&((cpos-1)<recnum)) return cpos-1;
                else                                 return -1;
            }
        }
        else if (cmp > 0) {
            if (cpos == lpos) {
                if ((mode==LE)||(mode==LT)||(mode==EQ)) {
                    return -1;
                }
                else {
                    if ((cpos>=0)&&(cpos<recnum)) return cpos;
                    else                         return -1;
                }
            }
            lpos = cpos;
            cpos = (fpos+lpos)/2;
        }
      else if (cmp < 0) {
            if (cpos == fpos) {
                cmp = compar(base+(recsize*lpos),sd);
                if (cmp == 0) {
                    if ((mode==LE)||(mode==GE)||(mode==EQ)) {
                        if ((lpos>=0)&&(lpos<recnum)) return lpos;
                        else                         return -1;
                    }
                    else if (mode==LT) {
                        if (((lpos-1)>=0)&&((lpos-1)<recnum)) return lpos-1;
                        else                                 return -1;
                    }
                    else {
                        return -1;
                    }
                }
                else if (cmp > 0) {
                    if ((mode==LE)||(mode==LT)) {
                        if (((lpos-1)>=0)&&((lpos-1)<recnum)) return lpos-1;
                        else                                 return -1;
                    }
                    else if ((mode==GE)||(mode==GT)) {
                        if (((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                        else                                 return -1;
                    }
                    else {
                        return -1;
                    }
                }
                else {
                    if ((mode==LE)||(mode==LT)) {
                        if (((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                        else                                 return -1;
                    }
                    else if ((mode==GE)||(mode==GT)) {
                        return -1;
                    }
                    else {
                        return -1;
                    }
                }
            }
            fpos = cpos;
            cpos = (fpos+lpos)/2;
        }
    }
}

// ------------------------------------------------------------
// fut_key_search 
//  기능: 선물종목 인덱스 키 Search 
// ------------------------------------------------------------
static int fut_key_search (int tot_cnt, char *keybuf)
{
	key_t   k;
	KS_EXPCODE      *key1;      /* KEY 1 - 금융파생(선물) */
	SHM_KEY_ARRY    *shm_item;	

	int  pos;
    int  shmid;
	int  shmsize;

    if (memcmp ((char *)getenv ("_FEP_DIV"), "TEST", 4) == 0)
        k = 0x01000000L;
    else
        k = 0x00000000L;

	/* 종목코드 sort 용  Shm Attach  --------------------------------------------------------------*/
	shmsize = sizeof(SHM_KEY_ARRY) * 1;  
    
    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(ITEM_SHM_KEY + k, shmsize, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("선물종목메모리 생성 . errrno[%d:%s]\n",errno,strerror(errno));
        return -1;
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    shm_item = (SHM_KEY_ARRY *)shmat(shmid, NULL, 0);
    if (shm_item == (void *)-1) {
        printf("선물종목키메모리 attach failed . errrno[%d:%s]\n",errno,strerror(errno));
        return -2;
    }
	/*----------------------------------------------------------------------------------------------*/
	key1 = (KS_EXPCODE *)(&shm_item[0].D_Key);

    pos = BsearchMode((char *)key1, keybuf, tot_cnt, sizeof(KS_EXPCODE), CmpExpcode, EQ);

    if (pos < 0 || key1[pos].idx < 0)
        return -1; 
    else
    {
        return  key1[pos].idx;
    }
}

int main(int argc, char *argv[])
{
    int  shmid;
	int  shmsize;
 	key_t   k;

	if (argc != 4)
	{
		fprintf(stderr, "Usage : %s KR4A75610001(1월물) 수기sprd 진입bp\n", argv[0]);
		return 1;
	}

	const char *symbol = argv[1];
	char *s_sprd, *s_bp; 
	double sprd = strtod(argv[2], &s_sprd);
	double bp   = strtod(argv[3], &s_bp);
	double d_sprd = 0, d_bp = 0;

	//if (sprd != 0)
	//  d_sprd = (int)(sprd * 100) / 100.0;
	d_sprd = sprd;

	//if (bp != 0)
	//  d_bp = (int)(bp * 100) / 100.0;
	d_bp = bp;

    if (memcmp ((char *)getenv("_FEP_DIV"), "TEST", 4) == 0)
        k = 0x01000000L;
    else
        k = 0x00000000L;

	/* 선물시세 Shm Attach  --------------------------------------------------------------*/
	shmsize = sizeof(SHM_FIN_FUT) * SHM_MAX_DEV_FIF;   // 1,000
    
    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FF_SHM_KEY+k, shmsize, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("선물시세 메모리 생성 . errrno[%d:%s]\n",errno,strerror(errno));
        return -1;
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_fut = (SHM_FIN_FUT *)shmat(shmid, NULL, 0);
    if (g_md_shm_fut == (void *)-1) {
        printf("현물시세 메모리 attach failed . errrno[%d:%s]\n",errno,strerror(errno));
        return -1;
    }

	/* 현물시세 Shm Attach ----------------------------------------------------------------*/
	shmsize = sizeof(SHM_FX) * SHM_MAX_FX;   // 1

    // 1. 공유 메모리 세그먼트 생성 (이미 있으면 기존 것 사용)
    shmid = shmget(FX_SHM_KEY+k, shmsize, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("현물시세 메모리 생성 . errrno[%d:%s]\n",errno,strerror(errno));
        return -1;
    }
    // 2. 공유 메모리를 프로세스 주소 공간에 attach
    g_md_shm_spot = (SHM_FX *)shmat(shmid, NULL, 0);
    if (g_md_shm_spot == (void *)-1) {
        printf("현물시세 메모리 attach failed . errrno[%d:%s]\n",errno,strerror(errno));
        return -1;
    }

    // 전략에 해당하는 선물, 현물 종목 시세 수신처리 등록 및 접근
    KS_EXPCODE key; // 선물시세 key
    memset(&key, 0, sizeof(key));
    memcpy(key.expcode, symbol, sizeof(key.expcode)); // key.expcode - 종목표준코드(12byte)
  
	/* 선물종목 Search */
    int tot_cnt = g_md_shm_fut[0].total_item_cnt; // total 종목 count
    int fut_idx = fut_key_search(tot_cnt, (char*)&key); // DEV_MK_FIF 금융상품선물(국채,금리,통화)
    if (fut_idx == -1)
    {
        printf("선물시세 fut_key_search fail!! (종목마스터 미수신 or 이상상태) [%.*s]\n", 12 , symbol); 
        return -1;
    }
    printf("선물시세 종목 idx [%d]\n", fut_idx); 

	while(1)
	{
		market_snapshot(fut_idx, d_sprd, d_bp);
		usleep(10000); //10ms 
	}

    return 0;
}
