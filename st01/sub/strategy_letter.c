/*------------------------------------------------------------------------
 *  strategy_letter.c — 전략번호(ApType) → 시장 letter 매핑 (순수 함수)
 *
 *  시장 재편 A안 §5.2. po_9000_mp Start_Client의 `pa_` 하드코딩(걸림돌 ①)을
 *  대체 — 전략 기동 시 cp할 TYPE 바이너리(`p{letter}_{ApType}_mp`)의 letter 결정.
 *
 *  밴드(prefix) 테이블 + longest-match + 기본값('o'):
 *    - 같은 시장에 LP/전략 추가 → 그 시장 밴드에 흡수(무변경 수용).
 *    - 신규 시장 밴드만 STRATEGY_MAP에 1행 추가.
 *    - 후일 config-backed(proc.ini/config-DB)로 확장 시 이 함수만 교체.
 *------------------------------------------------------------------------*/
#include    <string.h>
#include    "strategy_letter.h"

/* prefix 매칭 테이블(longest-match). 신규 시장/전략은 여기 추가. */
static const struct {
    const char *prefix;     /* 전략번호 앞자리 밴드 */
    char        letter;     /* 시장 letter */
}   STRATEGY_MAP[] = {
    { "50", 'b' },          /* 채권 전략 밴드(LP 등) — 단일시장=채권       */
    { "52", 'o' },          /* 통화선물×FX 차익 — 크로스마켓=OMS 코어      */
    /* 예: { "51", 'c' } 파생 단일시장 전략, { "53", 'o' } FX차익 … 추가   */
};

#define STRATEGY_MAP_CNT  ((int)(sizeof(STRATEGY_MAP) / sizeof(STRATEGY_MAP[0])))
#define STRATEGY_DEFAULT  'o'    /* 미지정 → OMS 코어(크로스마켓 기본)       */

/*----------------------------------------------------------------------*/
char    Strategy_Letter(const char *aptype)
/*----------------------------------------------------------------------*/
{
    int     i, best_len = 0;
    char    best = STRATEGY_DEFAULT;

    if (aptype == NULL)
        return STRATEGY_DEFAULT;

    for (i = 0; i < STRATEGY_MAP_CNT; i++) {
        int plen = (int)strlen(STRATEGY_MAP[i].prefix);
        if (strncmp(aptype, STRATEGY_MAP[i].prefix, plen) == 0 && plen > best_len) {
            best_len = plen;                    /* longest-prefix match */
            best     = STRATEGY_MAP[i].letter;
        }
    }
    return best;
}

/*************************************************************************
    End of Program (strategy_letter.c)
*************************************************************************/
