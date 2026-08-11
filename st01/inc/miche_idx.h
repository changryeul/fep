/*************************************************************************
    File        : . miche_idx.h
    Comment     : . 미체결(MICHE) 주문번호 조회 캐시 (F6 miche-index)
                  . 프로세스-로컬 - SHM 아님. 권위가 아닌 캐시:
                    Get 결과는 반드시 슬롯 OrderNo memcmp 검증 후 사용,
                    미스/불일치 시 기존 선형 스캔 폴백 + Put 보정
*************************************************************************/
#ifndef _MICHE_IDX_H
#define _MICHE_IDX_H

#define MICHE_IDX_HASH      16384   /* 2^14 (로드팩터 <0.61 @10000)    */
#define MICHE_IDX_PROBE     32      /* linear probing 상한              */
#define MICHE_IDX_KEYLEN    10      /* OrderNo 10바이트                 */

typedef struct {
    char    key[MICHE_IDX_HASH][MICHE_IDX_KEYLEN];
    int     slot[MICHE_IDX_HASH];   /* -1 = empty                       */
}   MICHE_IDX;

extern  void    Miche_Idx_Reset(MICHE_IDX *ix);
extern  void    Miche_Idx_Put(MICHE_IDX *ix, const char *key10, int slot);
extern  int     Miche_Idx_Get(MICHE_IDX *ix, const char *key10);

#endif  /* _MICHE_IDX_H */
