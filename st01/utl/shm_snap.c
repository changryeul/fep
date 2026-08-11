/*------------------------------------------------------------------------
#   Module  : 미체결(MK_PM) SHM 상태 스냅샷/비교 도구 (replay·멱등 검증용)
#   File    : utl/shm_snap.c
#
#   목적: po_ 미체결/체결 이관·STATE-driven seam 착수 전 안전판.
#     미체결 상태(MK_PREMATCH.F_MiChe[시장][계좌][idx])를 텍스트로 덤프하고
#     두 스냅샷을 비교한다. 멱등성 검증 절차:
#        shm_snap dump A            # 초기(또는 1차 적용 후)
#        <체결/응답 재적용(replay) 또는 중복수신>
#        shm_snap dump B
#        shm_snap cmp A B           # 동일해야(멱등) exit 0, 다르면 exit 1 + diff
#
#   MK_PM_SHM_KEY=0x41000013 (TEST 환경은 +0x01000000). 읽기전용 attach.
#   빌드: cc -I../inc -o ../bin/shm_snap shm_snap.c   (libfepP 불요, raw shmget/shmat)
------------------------------------------------------------------------*/
#include "fep_fepp.h"       /* 프로세스와 동일 프렐류드(→fep_sub.h→shm_memory.h 순서 보장) */
#include "pa_struct.h"
#include <sys/shm.h>

/*----------------------------------------------------------------------*/
static MK_PREMATCH *attach_mk(void)
/*----------------------------------------------------------------------*/
{
    key_t   key = MK_PM_SHM_KEY;
    char    *div = getenv("_FEP_DIV");
    int     id;
    void    *p;

    if (div != NULL && strncmp(div, "TEST", 4) == 0)
        key += 0x01000000L;                     /* TEST 환경 오프셋 */

    id = shmget(key, 0, 0);
    if (id < 0) {
        fprintf(stderr, "shm_snap: MK_PM SHM[%#lx] not found (pz_memory_mp 먼저 기동?)\n",
                (long)key);
        exit(2);
    }
    p = shmat(id, NULL, SHM_RDONLY);
    if (p == (void *)-1) {
        perror("shm_snap: shmat");
        exit(2);
    }
    return (MK_PREMATCH *)p;
}

/*----------------------------------------------------------------------*/
static void dump(FILE *out)
/*----------------------------------------------------------------------*/
{
    MK_PREMATCH *mk = attach_mk();
    int m, a, i, total = 0;

    for (m = 0; m < RISK_MK_CNT; m++) {
        for (a = 0; a < ACC_NO_CNT; a++) {
            int c = mk->MeChe_Cnt[m][a];
            if (c != 0)
                fprintf(out, "CNT mk=%d acc=%d miche=%d\n", m, a, c);
            for (i = 0; i < MAX_MICHE; i++) {
                MICHE *e = &mk->F_MiChe[m][a][i];
                /* 점유 판정: 잔량!=0 또는 주문번호 존재 */
                if (e->Jan_Cnt == 0 &&
                        (e->OrderNo[0] == 0 || e->OrderNo[0] == ' '))
                    continue;
                fprintf(out,
                        "MICHE mk=%d acc=%d idx=%d ord=%.10s orig=%.10s "
                        "item=%.12s tf=%.1s jan=%d ocnt=%.8s mkgbn=%d\n",
                        m, a, i, e->OrderNo, e->OriginalOrderNo, e->Item_Cd,
                        e->TradeFlag, e->Jan_Cnt, e->Order_Cnt, e->Mk_gbn);
                total++;
            }
        }
    }
    fprintf(stderr, "shm_snap: dumped %d occupied MICHE\n", total);
    shmdt(mk);
}

/*----------------------------------------------------------------------*/
static RISK *attach_risk(void)
/*----------------------------------------------------------------------*/
{
    key_t   key = RISK_SHM_KEY;
    char    *div = getenv("_FEP_DIV");
    int     id;
    void    *p;

    if (div != NULL && strncmp(div, "TEST", 4) == 0)
        key += 0x01000000L;                     /* TEST 환경 오프셋 */

    id = shmget(key, 0, 0);
    if (id < 0) {
        fprintf(stderr, "shm_snap: RISK SHM[%#lx] not found (pz_memory_mp 먼저 기동?)\n",
                (long)key);
        exit(2);
    }
    p = shmat(id, NULL, SHM_RDONLY);
    if (p == (void *)-1) {
        perror("shm_snap: shmat(risk)");
        exit(2);
    }
    return (RISK *)p;
}

/*----------------------------------------------------------------------*/
static void dump_risk(FILE *out)
/*----------------------------------------------------------------------*/
{
    RISK *r = attach_risk();
    int  m, it, a, total = 0;

    for (m = 0; m < RISK_MK_CNT; m++) {
        for (it = 0; it < SHM_MAX_NOTE; it++) {
            for (a = 0; a < ACC_NO_CNT; a++) {
                PROFIT *pf = &r->ProFit[m][it][a];
                /* 점유 판정: 보유수량 또는 미체결수량이 있으면 */
                if (pf->item_getcnt == 0 &&
                        pf->item_su_michecnt == 0 && pf->item_do_michecnt == 0)
                    continue;
                fprintf(out,
                        "PROFIT mk=%d item=%d acc=%d getcnt=%ld avg=%.2f "
                        "su_miche=%d do_miche=%d\n",
                        m, it, a, pf->item_getcnt, pf->item_get_avg_price,
                        pf->item_su_michecnt, pf->item_do_michecnt);
                total++;
            }
        }
    }
    fprintf(stderr, "shm_snap: dumped %d occupied PROFIT\n", total);
    shmdt(r);
}

/*----------------------------------------------------------------------*/
static int cmp(const char *fa, const char *fb)
/*----------------------------------------------------------------------*/
{
    FILE    *a = fopen(fa, "r"), *b = fopen(fb, "r");
    char    la[512], lb[512];
    int     line = 0, diff = 0;

    if (a == NULL || b == NULL) {
        fprintf(stderr, "shm_snap: cannot open [%s]/[%s]\n", fa, fb);
        return 2;
    }
    while (1) {
        char *ra = fgets(la, sizeof(la), a);
        char *rb = fgets(lb, sizeof(lb), b);
        line++;
        if (ra == NULL && rb == NULL) break;
        if (ra == NULL || rb == NULL || strcmp(la, lb) != 0) {
            if (diff < 10)
                fprintf(stderr, "  DIFF line %d:\n    A: %s    B: %s",
                        line, ra ? la : "(EOF)\n", rb ? lb : "(EOF)\n");
            diff++;
        }
    }
    fclose(a); fclose(b);
    if (diff == 0)
        printf("shm_snap cmp: IDENTICAL (멱등 OK)\n");
    else
        printf("shm_snap cmp: %d line(s) DIFFER (멱등 위반)\n", diff);
    return diff == 0 ? 0 : 1;
}

/*----------------------------------------------------------------------*/
int main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    if (argc >= 4 && strcmp(argv[1], "cmp") == 0)
        return cmp(argv[2], argv[3]);

    if (argc >= 3 && strcmp(argv[1], "dump") == 0) {
        FILE *out = fopen(argv[2], "w");
        if (out == NULL) { perror("fopen"); return 2; }
        dump(out);
        fclose(out);
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "dump") == 0) {
        dump(stdout);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "risk") == 0) {
        FILE *out = fopen(argv[2], "w");
        if (out == NULL) { perror("fopen"); return 2; }
        dump_risk(out);
        fclose(out);
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "risk") == 0) {
        dump_risk(stdout);
        return 0;
    }

    fprintf(stderr,
            "Usage: shm_snap dump [outfile]     # 미체결(MK_PM) SHM 상태 덤프\n"
            "       shm_snap risk [outfile]     # 한도/손익(RISK.ProFit) 덤프\n"
            "       shm_snap cmp <A> <B>        # 두 스냅샷 비교(멱등 검증)\n");
    return 2;
}

/*************************************************************************
    End of Program (shm_snap.c)
*************************************************************************/
