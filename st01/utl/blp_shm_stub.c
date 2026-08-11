/*------------------------------------------------------------------------
#   Module  : BLP 매칭엔진 SHM 스텁 (전략 부팅검증 P3a)
#   File    : utl/blp_shm_stub.c
#
#   채권 LP 전략(pa_5050_mp)은 매칭엔진의 얇은 어댑터로, Blp_Open→Mem_Open(BLP_KEY)
#   로 매칭엔진이 만든 BLP SHM에 attach만 한다(생성 X). 실제 매칭엔진
#   (mat_lpbnd/match 등)은 대규모 별도 시스템이므로, P3a(전략 부팅검증)에서는
#   이 스텁이 매칭엔진 대역으로 BLP SHM/세마포어만 생성해 전략이 부팅되게 한다.
#
#   미러 대상: blp_all.c Blp_Create — Mem_Create(BLP_KEY,sizeof(BLP_MAP)) +
#   Sem_Create(BLP_KEY) + map->stat.key/service. (service=1이라 전략은 대기 스킵)
#
#   빌드: cc -m64 -Iinc -I<mat include/blp/sem/mcp> blp_shm_stub.c libblp.a -o bin/blp_shm_stub
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "fep_fepp.h"    /* FEP 프렐류드(shm_memory.h 의존 타입 선행 — 전략과 동일 순서) */
#include "pa_struct.h"
#include "mem.h"
#include "sem.h"
#include "mcp.h"
#include "blp.h"         /* BLP_KEY(0xbb001001), BLP_MAP */

int main(void)
{
    MEM     *mem;
    SEM     *sem;
    BLP_MAP *map;

    mem = Mem_Create(BLP_KEY, sizeof(BLP_MAP));
    if (mem == NULL) {
        fprintf(stderr, "blp_shm_stub: Mem_Create fail key=0x%08x size=%zu\n",
                (unsigned)BLP_KEY, sizeof(BLP_MAP));
        return 1;
    }
    map = (BLP_MAP *)Mem_GetPtr(mem);
    memset(map, 0, sizeof(BLP_MAP));
    map->stat.key     = BLP_KEY;
    map->stat.service = 1;      /* 매칭엔진 서비스중 표시 */

    sem = Sem_Create(BLP_KEY);
    if (sem == NULL) {
        fprintf(stderr, "blp_shm_stub: Sem_Create fail key=0x%08x\n", (unsigned)BLP_KEY);
        return 1;
    }

    printf("blp_shm_stub: BLP SHM+SEM created key=0x%08x size=%zu (매칭엔진 대역)\n",
            (unsigned)BLP_KEY, sizeof(BLP_MAP));
    return 0;
}

/*************************************************************************
    End of Program (blp_shm_stub.c)
*************************************************************************/
