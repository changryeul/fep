/*************************************************************************
    Module      : . MICHE order-number lookup cache (F6 miche-index)
    File        : . miche_idx.c
    Comment     : . FNV-1a 해시 + linear probing, eviction 허용 캐시
                  . 삭제 연산 없음 - 낡은 항목은 호출측 검증에서 걸러짐
                  . 단일 스레드 이벤트 루프 전용
*************************************************************************/

#include <string.h>

#include "miche_idx.h"

/*----------------------------------------------------------------------*/
static unsigned int miche_hash(const char *key)
/*----------------------------------------------------------------------*/
{
    unsigned int    h;
    int             i;

    h = 2166136261u;                        /* FNV offset basis */

    for (i = 0; i < MICHE_IDX_KEYLEN; i ++) {
        h ^= (unsigned char)key[i];
        h *= 16777619u;                     /* FNV prime */
    }

    return (h & (MICHE_IDX_HASH - 1));
}

/*************************************************************************
    Function        : . Miche_Idx_Reset
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Miche_Idx_Reset(MICHE_IDX *ix)
/*----------------------------------------------------------------------*/
{
    int     i;

    for (i = 0; i < MICHE_IDX_HASH; i ++)
        ix->slot[i] = -1;

    return;
}   /* End of Miche_Idx_Reset ()    */

/*************************************************************************
    Function        : . Miche_Idx_Put
    Parameters IN   : . key10 : OrderNo 10바이트 (null 종료 불필요)
                      . slot  : F_MiChe 슬롯 인덱스
    Comment         : . 동일 키는 갱신, PROBE 상한 초과 시 마지막 위치
                        덮어씀 (eviction - 캐시이므로 허용)
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Miche_Idx_Put(MICHE_IDX *ix, const char *key10, int slot)
/*----------------------------------------------------------------------*/
{
    unsigned int    h;
    int             p;

    h = miche_hash(key10);

    for (p = 0; p < MICHE_IDX_PROBE; p ++) {
        if (ix->slot[h] == -1 ||
                memcmp(ix->key[h], key10, MICHE_IDX_KEYLEN) == 0)
            break;

        h = (h + 1) & (MICHE_IDX_HASH - 1);
    }

    /* PROBE 초과 시 h는 마지막 프로브 위치 - 덮어씀 */
    memcpy(ix->key[h], key10, MICHE_IDX_KEYLEN);
    ix->slot[h] = slot;

    return;
}   /* End of Miche_Idx_Put ()  */

/*************************************************************************
    Function        : . Miche_Idx_Get
    Return Code     : . int : slot 또는 -1 (미등록/추방됨)
    Comment         : . 리턴된 slot은 호출측이 F_MiChe[..][slot].OrderNo
                        memcmp 검증 후 사용해야 함
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Miche_Idx_Get(MICHE_IDX *ix, const char *key10)
/*----------------------------------------------------------------------*/
{
    unsigned int    h;
    int             p;

    h = miche_hash(key10);

    for (p = 0; p < MICHE_IDX_PROBE; p ++) {
        if (ix->slot[h] == -1)
            return (-1);

        if (memcmp(ix->key[h], key10, MICHE_IDX_KEYLEN) == 0)
            return (ix->slot[h]);

        h = (h + 1) & (MICHE_IDX_HASH - 1);
    }

    return (-1);
}   /* End of Miche_Idx_Get ()  */

/*************************************************************************
    End of Program (miche_idx.c)
*************************************************************************/
