/*------------------------------------------------------------------------
#   Module  : SEAM 스테이징 큐 상태 peek 도구 (부문간 seam 검증용)
#   File    : utl/seam_peek.c
#
#   목적: pc_/pb_ 수신 → 전역 SEAM 링버퍼 착지(생산자측) 검증.
#     각 큐(응답/체결)의 w_seq(발행)·r_seq(리더별 소비)·dropped(오버플로)와
#     마지막 레코드의 Data 프리뷰(TR코드 확인용)를 덤프한다.
#
#   SEAM_Q_SHM_KEY=0x41000015 (TEST 환경 +0x01000000). 읽기전용 attach.
#   빌드: cc -I../inc -o ../bin/seam_peek seam_peek.c   (libfepP 불요, raw shmget/shmat)
#
#   레코드 = FILE_BUFF_FORMAT. Data 필드 오프셋 = 70
#     (Seq8+If_Seq8+ApType8+ResponseCode4+RecvTime1 10+RecvTime2 12+DataHeader20).
------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include "seam_queue.h"

#define FBF_DATA_OFF    70      /* FILE_BUFF_FORMAT 내 Data 시작 오프셋 */

/*----------------------------------------------------------------------*/
static SEAM_SHM *attach_seam(void)
/*----------------------------------------------------------------------*/
{
    key_t   key = SEAM_Q_SHM_KEY;
    char    *div = getenv("_FEP_DIV");
    int     id;
    void    *p;

    if (div != NULL && strncmp(div, "TEST", 4) == 0)
        key += 0x01000000L;                     /* TEST 환경 오프셋 */

    id = shmget(key, 0, 0);
    if (id < 0) {
        fprintf(stderr, "seam_peek: SEAM SHM[%#lx] not found (pc_/po_ 먼저 기동?)\n",
                (long)key);
        exit(2);
    }
    p = shmat(id, NULL, SHM_RDONLY);
    if (p == (void *)-1) {
        perror("seam_peek: shmat");
        exit(2);
    }
    return (SEAM_SHM *)p;
}

/*----------------------------------------------------------------------*/
int main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    SEAM_SHM    *s = attach_seam();
    int         q;

    (void)argc; (void)argv;

    if (s->magic != SEAM_MAGIC) {
        printf("SEAM not initialized (magic=%#lx, expected %#lx)\n",
                (long)s->magic, (long)SEAM_MAGIC);
        return 2;
    }

    for (q = 0; q < SEAM_Q_NQUEUE; q++) {
        SEAM_QUEUE  *Q = &s->q[q];
        const char  *name = (q == SEAM_Q_RESP) ? "RESP" :
                            (q == SEAM_Q_EXEC) ? "EXEC" : "?";

        printf("SEAM q=%d(%s) w_seq=%ld r_seq=[%ld,%ld,%ld,%ld] dropped=[%ld,%ld,%ld,%ld]\n",
                q, name, Q->w_seq,
                Q->r_seq[0], Q->r_seq[1], Q->r_seq[2], Q->r_seq[3],
                Q->dropped[0], Q->dropped[1], Q->dropped[2], Q->dropped[3]);

        if (Q->w_seq > 0) {
            long    last = (Q->w_seq - 1) % SEAM_Q_SLOTS;
            char    *data = Q->slot[last] + FBF_DATA_OFF;
            /* 응답: Data[15..]=회원처리호가(TrCode@Data[26]), 체결: Data[11]=TrCode */
            printf("   last rec Data[0:60]=[%.60s]\n", data);
        }
    }
    return 0;
}

/*************************************************************************
    End of Program (seam_peek.c)
*************************************************************************/
