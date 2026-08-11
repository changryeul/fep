/*------------------------------------------------------------------------
#   Module  : FX 유틸 (유안타 arb 편입 P4-1) — Key_Search_FX / alrt_msg
#   File    : fx_util.c
#
#   유안타 pa_7070(차익)·pa_7400_ur(FX 시세수신)가 쓰는 지원 함수인데 유안타
#   private libfepP에만 있어 미제공 → FEP libfepP에 구현 편입.
#
#   Key_Search_FX(excode, symb): 전역 RISK.FX_Sise[0][]에서 (m_exch,m_item_cd)로
#     검색해 index 반환. 없으면 첫 빈 슬롯에 등록(m_item_cd[0]==0). 만석 NOTOK.
#     - 시세수신(pa_7400)이 등록, 전략(pa_7070)이 조회 — 동일 함수로 일관.
#   ※ alrt_msg 는 alarm.c(OMS 알림 실 구현)로 이관됨 — 여기 스텁 제거(2026-08-10).
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*----------------------------------------------------------------------*/
int     Key_Search_FX(char *excode, char *symb)
/*----------------------------------------------------------------------*/
{
    int     i, empty = -1;

    if (Shm_Risk == NULL || excode == NULL || symb == NULL)
        return (NOTOK);

    for (i = 0; i < SHM_MAX_FX; i++) {
        FX_SISE_FORMAT *e = &Shm_Risk[0].FX_Sise[0][i];

        if (memcmp(e->m_exch, excode, sizeof (e->m_exch)) == 0 &&
                memcmp(e->m_item_cd, symb, sizeof (e->m_item_cd)) == 0)
            return (i);                         /* 기존 종목 */

        if (empty < 0 && e->m_item_cd[0] == 0x00)
            empty = i;                          /* 첫 빈 슬롯 기억 */
    }

    if (empty >= 0) {                           /* 신규 등록 */
        memcpy(Shm_Risk[0].FX_Sise[0][empty].m_exch,    excode, sizeof (Shm_Risk[0].FX_Sise[0][empty].m_exch));
        memcpy(Shm_Risk[0].FX_Sise[0][empty].m_item_cd, symb,   sizeof (Shm_Risk[0].FX_Sise[0][empty].m_item_cd));
        return (empty);
    }
    return (NOTOK);                             /* 만석 */
}

/*************************************************************************
    End of Program (fx_util.c)
*************************************************************************/
