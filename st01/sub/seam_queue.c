/*------------------------------------------------------------------------
 *  seam_queue.c — SEAM 스테이징 큐 SHM/락 래퍼 (프로덕션 API)
 *
 *  부문 간(pc_/pb_ 수신 → po_ OMS코어) 메시지 전달 링버퍼의 SHM 생명주기 +
 *  다중 writer 동시성(세마포어) 처리. 순수 링 산술은 seam_ring.c.
 *  시장 재편 A안 §5.1.1 (2026-08-07 확정, 전역 스테이징 SHM 링버퍼).
 *
 *  전역 고정키(SEAM_Q_SHM_KEY, +TEST 0x01000000)라 부문 무관하게 attach.
 *  세마포어는 SHM과 동일 키로 최초 1회 excl 생성(val=1), 이후 open.
 *------------------------------------------------------------------------*/
#include    "fep_sub.h"
#include    "seam_queue.h"

/*------------------------------------------------------------------------
    컴파일타임 불변식 (계약 정본):
    SEAM 슬롯(SEAM_Q_RECSZ)은 부문간 전달 레코드 FILE_BUFF_FORMAT(DATA_SIZE=400,
    =471B)을 담을 수 있어야 한다. 위반 시(RECSZ 축소 or DATA_SIZE 확대) 배열 크기가
    음수가 되어 컴파일 실패 → 480 vs 471 부류 버그를 빌드 단계에서 차단.
------------------------------------------------------------------------*/
#ifndef DATA_SIZE
#define DATA_SIZE 400
#endif
#include    "buf_struct.h"
typedef char seam_slot_fits_fbf_check[
    (SEAM_Q_RECSZ >= (int)sizeof(FILE_BUFF_FORMAT)) ? 1 : -1];
/* 주문 wire 레코드(471, DATA_SIZE=400 기준)도 슬롯에 담겨야 함(§5.3 P2). */
typedef char seam_ord_wire_fits_slot_check[
    (SEAM_Q_RECSZ >= SEAM_ORD_WIRE_RECSZ) ? 1 : -1];

/*------------------------------------------------------------------------
    Module State
------------------------------------------------------------------------*/
static SEAM_SHM *Seam       = NULL;    /* attach된 전역 세그먼트         */
static int       Seam_Shmid = -1;
static int       Seam_Semid = -1;

static SEAM_ORD_SHM *SeamOrd     = NULL;   /* 주문큐 세그먼트(0x41000016)   */
static int           SeamOrd_Shmid = -1;
static int           SeamOrd_Semid = -1;

/*------------------------------------------------------------------------
 *  SEAM_Init — SHM create/attach + 세마포어 확보 + 최초 1회 zero-init.
 *    반환: 0 성공, -1 실패.
 *------------------------------------------------------------------------*/
int     SEAM_Init(void)
{
    key_t   off = 0;
    key_t   shmkey;

    if (Seam != NULL)
        return 0;                       /* 이미 초기화됨 (프로세스당 1회) */

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        off = 0x01000000L;

    shmkey = SEAM_Q_SHM_KEY + off;

    /* 1. SHM create/attach */
    Seam_Shmid = SHM_Creat(shmkey, sizeof(SEAM_SHM));
    if (Seam_Shmid == -1) {
        Log(SYS_FATAL, "SEAM_Init: SHM_Creat[%#x] size=%d {%d:%s}",
                shmkey, (int)sizeof(SEAM_SHM), SYS_NO, SYS_STR);
        return -1;
    }
    Seam = (SEAM_SHM *)SHM_Attach(Seam_Shmid);
    if (Seam == (SEAM_SHM *)-1) {
        Log(SYS_FATAL, "SEAM_Init: SHM_Attach {%d:%s}", SYS_NO, SYS_STR);
        Seam = NULL;
        return -1;
    }

    /* 2. 세마포어: 최초 1회 excl 생성(val=1), 이미 있으면 기존 open.
          (프로덕션은 pz_memory_mp가 사전생성 권장 — 여기선 자체확보로 자립) */
    Seam_Semid = SEM_Creat_Excl(shmkey);
    if (Seam_Semid == -1)
        Seam_Semid = SEM_Creat(shmkey);
    if (Seam_Semid == -1) {
        Log(SYS_FATAL, "SEAM_Init: SEM_Creat[%#x] {%d:%s}", shmkey, SYS_NO, SYS_STR);
        return -1;
    }

    /* 3. 최초 1회 zero-init (magic 판별, 락으로 보호) */
    SEM_Lock(Seam_Semid);
    if (Seam->magic != SEAM_MAGIC) {
        memset(Seam, 0, sizeof(SEAM_SHM));
        Seam->magic = SEAM_MAGIC;
        Log(USR_OK, "SEAM_Init: zero-init SHM[%#x] size=%d",
                shmkey, (int)sizeof(SEAM_SHM));
    }
    SEM_UnLock(Seam_Semid);

    Log(USR_OK, "SEAM_Init OK shmid=%d semid=%d key=%#x",
            Seam_Shmid, Seam_Semid, shmkey);
    return 0;
}

/*------------------------------------------------------------------------
 *  SEAM_W — qidx 큐에 1건 기록 (세마포어 보호: 다중 writer 안전).
 *    반환: 1 성공, -1 오류.
 *------------------------------------------------------------------------*/
int     SEAM_W(int qidx, const void *rec, int recsz)
{
    int     rt;

    if (Seam == NULL && SEAM_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_Q_NQUEUE)
        return -1;

    SEM_Lock(Seam_Semid);
    rt = seam_ring_write(&Seam->q[qidx], rec, recsz);
    SEM_UnLock(Seam_Semid);

    return rt;
}

/*------------------------------------------------------------------------
 *  SEAM_R — qidx/ridx 리더가 미소비분을 out에 최대 maxrec건 소비.
 *    오버플로 유실이 새로 발생하면 로그(무음유실 금지).
 *    반환: 읽은 건수(0 이상), -1 오류.
 *------------------------------------------------------------------------*/
int     SEAM_R(int qidx, int ridx, void *out, int maxrec)
{
    int     rt;
    long    dropped_before;

    if (Seam == NULL && SEAM_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_Q_NQUEUE)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;

    dropped_before = Seam->q[qidx].dropped[ridx];

    SEM_Lock(Seam_Semid);
    rt = seam_ring_read(&Seam->q[qidx], ridx, out, maxrec);
    SEM_UnLock(Seam_Semid);

    if (Seam->q[qidx].dropped[ridx] != dropped_before)
        Log(USR_ERROR, "SEAM_R overflow q=%d r=%d dropped_total=%ld (링버퍼 유실!)",
                qidx, ridx, Seam->q[qidx].dropped[ridx]);

    return rt;
}

/*------------------------------------------------------------------------
 *  SEAM_Peek — qidx/ridx 리더가 r_seq 전진 없이 미소비분 조회(락 보호).
 *    처리 완료 후 SEAM_Commit로 전진. 반환: 조회 건수(0 이상), -1 오류.
 *------------------------------------------------------------------------*/
int     SEAM_Peek(int qidx, int ridx, void *out, int maxrec)
{
    int     rt;
    long    dropped_before;

    if (Seam == NULL && SEAM_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_Q_NQUEUE)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;

    dropped_before = Seam->q[qidx].dropped[ridx];

    SEM_Lock(Seam_Semid);
    rt = seam_ring_peek(&Seam->q[qidx], ridx, out, maxrec);
    SEM_UnLock(Seam_Semid);

    if (Seam->q[qidx].dropped[ridx] != dropped_before)
        Log(USR_ERROR, "SEAM_Peek overflow q=%d r=%d dropped_total=%ld (링버퍼 유실!)",
                qidx, ridx, Seam->q[qidx].dropped[ridx]);

    return rt;
}

/*------------------------------------------------------------------------
 *  SEAM_Commit — 처리 완료분 n건만큼 리더 커서 전진(락 보호). 반환: 0.
 *------------------------------------------------------------------------*/
int     SEAM_Commit(int qidx, int ridx, int n)
{
    if (Seam == NULL && SEAM_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_Q_NQUEUE)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;

    SEM_Lock(Seam_Semid);
    seam_ring_commit(&Seam->q[qidx], ridx, n);
    SEM_UnLock(Seam_Semid);
    return 0;
}

/*========================================================================
 *  주문큐(0x41000016) — 전략 → 시장 송신부 (§5.3, Model B)
 *  이벤트 세그먼트와 별도 SHM/세마포어. 링 코어(seam_ring_*) 재사용.
 *========================================================================*/

/*------------------------------------------------------------------------
 *  SEAM_ORD_Init — 주문큐 SHM/세마포어 확보 + 최초 1회 zero-init.
 *------------------------------------------------------------------------*/
int     SEAM_ORD_Init(void)
{
    key_t   off = 0;
    key_t   shmkey;

    if (SeamOrd != NULL)
        return 0;

    if (memcmp(_FEP_DIV, "TEST", 4) == 0)
        off = 0x01000000L;

    shmkey = SEAM_ORD_Q_SHM_KEY + off;

    SeamOrd_Shmid = SHM_Creat(shmkey, sizeof(SEAM_ORD_SHM));
    if (SeamOrd_Shmid == -1) {
        Log(SYS_FATAL, "SEAM_ORD_Init: SHM_Creat[%#x] size=%d {%d:%s}",
                shmkey, (int)sizeof(SEAM_ORD_SHM), SYS_NO, SYS_STR);
        return -1;
    }
    SeamOrd = (SEAM_ORD_SHM *)SHM_Attach(SeamOrd_Shmid);
    if (SeamOrd == (SEAM_ORD_SHM *)-1) {
        Log(SYS_FATAL, "SEAM_ORD_Init: SHM_Attach {%d:%s}", SYS_NO, SYS_STR);
        SeamOrd = NULL;
        return -1;
    }

    SeamOrd_Semid = SEM_Creat_Excl(shmkey);
    if (SeamOrd_Semid == -1)
        SeamOrd_Semid = SEM_Creat(shmkey);
    if (SeamOrd_Semid == -1) {
        Log(SYS_FATAL, "SEAM_ORD_Init: SEM_Creat[%#x] {%d:%s}", shmkey, SYS_NO, SYS_STR);
        return -1;
    }

    SEM_Lock(SeamOrd_Semid);
    if (SeamOrd->magic != SEAM_ORD_MAGIC) {
        memset(SeamOrd, 0, sizeof(SEAM_ORD_SHM));
        SeamOrd->magic = SEAM_ORD_MAGIC;
        Log(USR_OK, "SEAM_ORD_Init: zero-init SHM[%#x] size=%d",
                shmkey, (int)sizeof(SEAM_ORD_SHM));
    }
    SEM_UnLock(SeamOrd_Semid);

    Log(USR_OK, "SEAM_ORD_Init OK shmid=%d semid=%d key=%#x",
            SeamOrd_Shmid, SeamOrd_Semid, shmkey);
    return 0;
}

/*------------------------------------------------------------------------
 *  SEAM_ORD_W — 전략이 qidx(시장) 주문큐에 1건 기록(세마포어 보호). 반환 1.
 *------------------------------------------------------------------------*/
int     SEAM_ORD_W(int qidx, const void *rec, int recsz)
{
    int     rt;

    if (SeamOrd == NULL && SEAM_ORD_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_ORD_NQUEUE)
        return -1;

    SEM_Lock(SeamOrd_Semid);
    rt = seam_ring_write(&SeamOrd->q[qidx], rec, recsz);
    SEM_UnLock(SeamOrd_Semid);

    return rt;
}

/*------------------------------------------------------------------------
 *  SEAM_ORD_R — 송신부가 qidx/ridx 주문큐 소비(at-most-once). 반환 건수(-1 오류).
 *    오버플로 유실 발생 시 로그(주문 유실은 치명 — 알람).
 *------------------------------------------------------------------------*/
int     SEAM_ORD_R(int qidx, int ridx, void *out, int maxrec)
{
    int     rt;
    long    dropped_before;

    if (SeamOrd == NULL && SEAM_ORD_Init() != 0)
        return -1;
    if (qidx < 0 || qidx >= SEAM_ORD_NQUEUE)
        return -1;
    if (ridx < 0 || ridx >= SEAM_Q_NREADER)
        return -1;

    dropped_before = SeamOrd->q[qidx].dropped[ridx];

    SEM_Lock(SeamOrd_Semid);
    rt = seam_ring_read(&SeamOrd->q[qidx], ridx, out, maxrec);
    SEM_UnLock(SeamOrd_Semid);

    if (SeamOrd->q[qidx].dropped[ridx] != dropped_before)
        Log(USR_ERROR, "SEAM_ORD_R overflow q=%d r=%d dropped_total=%ld (주문 유실! 알람)",
                qidx, ridx, SeamOrd->q[qidx].dropped[ridx]);

    return rt;
}

/*************************************************************************
    End of Program (seam_queue.c)
*************************************************************************/
