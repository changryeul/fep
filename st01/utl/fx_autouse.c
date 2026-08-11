/*------------------------------------------------------------------------
#   fx_autouse.c — Shm_FX[idx].auto_use[slot] 세팅 (pf_ 도어벨 fanout 구독 시뮬)
#
#   pf_7400_ur 은 Set_Sise 후 auto_use[i]!=0 인 슬롯 i 의 도어벨(pf_dbell_i)만 링한다.
#   실전략(arb)은 l_arb_proc 에서 auto_use[OD_SEQ]=1 을 자기 구독으로 세팅하지만,
#   도어벨 배선(P4-full d) 검증에서는 이 도구로 auto_use 를 수동 세팅해 pf_ fanout 을
#   유발한다. (raw shmget/shmat, libfepP 불요)
#
#   usage: fx_autouse <idx> <slot> [val=1]
------------------------------------------------------------------------*/
#include "fep_fepp.h"       /* 프로세스와 동일 프렐류드(→shm_memory.h: SHM_FX/FX_SHM_KEY/MAX_AUTO_PROC) */
#include "pa_struct.h"
#include <sys/shm.h>

int     main(int argc, char *argv[])
{
    int         idx  = (argc > 1) ? atoi(argv[1]) : 0;
    int         slot = (argc > 2) ? atoi(argv[2]) : 0;
    int         val  = (argc > 3) ? atoi(argv[3]) : 1;
    int         shmid;
    SHM_FX     *fx;

    shmid = shmget((key_t)FX_SHM_KEY, 0, 0666);
    if (shmid < 0) {
        printf("fx_autouse: FX SHM not found key=0x%x\n", (unsigned)FX_SHM_KEY);
        return (1);
    }
    fx = (SHM_FX *)shmat(shmid, NULL, 0);
    if (fx == (SHM_FX *)-1) {
        perror("fx_autouse: shmat");
        return (1);
    }
    if (slot < 0 || slot >= MAX_AUTO_PROC) {
        printf("fx_autouse: slot %d out of range [0,%d)\n", slot, MAX_AUTO_PROC);
        return (1);
    }
    fx[idx].auto_use[slot] = (char)val;
    printf("fx_autouse: Shm_FX[%d].auto_use[%d]=%d set\n", idx, slot, val);
    return (0);
}
