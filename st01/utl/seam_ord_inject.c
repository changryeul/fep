/*------------------------------------------------------------------------
#   Module  : SEAM 주문큐 raw 주입기 (forwarder P2 검증 스텁)
#   File    : utl/seam_ord_inject.c
#
#   전략(po_) emit 대역: SEAM_ORDQ_BOND에 채권 주문(TCHODR40001) 1건을
#   raw shmget/shmat + seam_ring_write(순수)로 주입. FEP 프로세스 등록 불요
#   (seam_peek와 동형). forwarder(pb_1109_mp)가 SEAM_ORD_R로 소비하는지 검증.
#
#   ⚠ 세그먼트는 먼저 기동한 SEAM_ORD_Init 소비자(forwarder)가 생성. 이 도구는
#   기존 세그먼트에 attach만(없으면 에러). 단일 주입이라 sem 없이 raw write
#   (w_seq는 데이터 기록 후 발행이라 리더가 미완성 레코드 보지 않음).
#
#   빌드: cc -I../inc -o ../bin/seam_ord_inject seam_ord_inject.c ../sub/seam_ring.c
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include "seam_queue.h"

#define FBF_DATA_OFF    70      /* FILE_BUFF_FORMAT 내 Data 오프셋 */

int main(int argc, char *argv[])
{
    key_t   key = SEAM_ORD_Q_SHM_KEY;
    char   *div = getenv("_FEP_DIV");
    int     id, qidx = SEAM_ORDQ_BOND;
    SEAM_ORD_SHM *s;
    char    rec[SEAM_Q_RECSZ];
    char   *d = rec + FBF_DATA_OFF;

    (void)argc; (void)argv;

    if (div != NULL && strncmp(div, "TEST", 4) == 0)
        key += 0x01000000L;

    id = shmget(key, 0, 0);
    if (id < 0) {
        fprintf(stderr, "seam_ord_inject: SEAM_ORD SHM[%#lx] not found (forwarder 먼저 기동?)\n",
                (long)key);
        return 2;
    }
    s = (SEAM_ORD_SHM *)shmat(id, NULL, 0);
    if (s == (void *)-1) { perror("shmat"); return 2; }

    /* 채권 주문 wire 레코드: header 공백 + Data[11]=TrCode */
    memset(rec, ' ', sizeof(rec));
    memcpy(d + 0,  "00000000001", 11);      /* DataSeq            */
    memcpy(d + 11, "TCHODR40001", 11);      /* TrCode(채권 일반호가) */
    memcpy(d + 22, "00", 2);                /* Megrp              */
    memcpy(d + 39, "0000000001", 10);       /* OrderNo(관측용)    */

    if (seam_ring_write(&s->q[qidx], rec, SEAM_ORD_WIRE_RECSZ) != 1) {
        fprintf(stderr, "seam_ord_inject: seam_ring_write fail\n");
        return 1;
    }
    printf("seam_ord_inject: wrote 1 TCHODR40001 to SEAM_ORDQ_BOND (w_seq=%ld)\n",
            s->q[qidx].w_seq);
    shmdt(s);
    return 0;
}

/*************************************************************************
    End of Program (seam_ord_inject.c)
*************************************************************************/
