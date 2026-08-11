/*------------------------------------------------------------------------
#   Module  : 주문 레이턴시 리포트 도구 (F1 order-latency-metrics)
#   File    : lat_report.c
#
#   lat_trace가 기록한 .lat 파일(들)을 읽어 (proc, key)별 IN→OUT
#   구간(µs)을 짝짓고, proc별 count/min/p50/p90/p99/max를 출력한다.
#
#   입력 라인 포맷: <epoch_usec>|<proc>|<point>|<key>
#     point는 IN / OUT 표준화 (order_inject 포함 전 지점 공통)
#
#   Usage:
#     lat_report <file.lat> [file2.lat ...]
#
#   Note: 단독 빌드 (libfepP 비의존). 테스트를 위해
#         -DLAT_REPORT_NO_MAIN 으로 main 제외 컴파일 가능.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAT_PROC_LEN    32
#define LAT_POINT_LEN   8
#define LAT_KEY_LEN     32
#define LAT_MAX_EV      4000000     /* 이벤트 상한 (약 400만 = 200만 구간) */

typedef struct {
    long long   usec;
    char        proc[LAT_PROC_LEN];
    char        point[LAT_POINT_LEN];
    char        key[LAT_KEY_LEN];
} LAT_EV;

/*------------------------------------------------------------------------
    lat_parse_line: 1라인 파싱. 0=성공, -1=형식 불량(스킵)
------------------------------------------------------------------------*/
static int lat_parse_line(const char *line, LAT_EV *ev)
{
    const char  *p1, *p2, *p3;
    char        *endp;
    size_t      len;

    if (line == NULL || ev == NULL)
        return -1;

    p1 = strchr(line, '|');
    if (p1 == NULL || p1 == line)
        return -1;
    p2 = strchr(p1 + 1, '|');
    if (p2 == NULL || p2 == p1 + 1)
        return -1;
    p3 = strchr(p2 + 1, '|');
    if (p3 == NULL || p3 == p2 + 1)
        return -1;
    if (p3[1] == '\0' || p3[1] == '\n')
        return -1;

    ev->usec = strtoll(line, &endp, 10);
    if (endp != p1)                     /* 1필드 전체가 숫자가 아니면 불량 */
        return -1;

    len = (size_t)(p2 - (p1 + 1));
    if (len >= LAT_PROC_LEN)
        len = LAT_PROC_LEN - 1;
    memcpy(ev->proc, p1 + 1, len);
    ev->proc[len] = '\0';

    len = (size_t)(p3 - (p2 + 1));
    if (len >= LAT_POINT_LEN)
        len = LAT_POINT_LEN - 1;
    memcpy(ev->point, p2 + 1, len);
    ev->point[len] = '\0';

    len = strlen(p3 + 1);
    if (len > 0 && (p3 + 1)[len - 1] == '\n')
        len--;
    if (len == 0)
        return -1;
    if (len >= LAT_KEY_LEN)
        len = LAT_KEY_LEN - 1;
    memcpy(ev->key, p3 + 1, len);
    ev->key[len] = '\0';

    return 0;
}

/*------------------------------------------------------------------------
    이벤트 정렬: (proc, key, usec) 오름차순
------------------------------------------------------------------------*/
static int lat_ev_cmp(const void *a, const void *b)
{
    const LAT_EV *ea = (const LAT_EV *)a;
    const LAT_EV *eb = (const LAT_EV *)b;
    int rt;

    rt = strcmp(ea->proc, eb->proc);
    if (rt != 0)
        return rt;
    rt = strcmp(ea->key, eb->key);
    if (rt != 0)
        return rt;
    if (ea->usec < eb->usec)
        return -1;
    if (ea->usec > eb->usec)
        return 1;
    return 0;
}

/*------------------------------------------------------------------------
    lat_pair: (proc,key)별 시각순으로 IN 다음의 첫 OUT을 짝지어
              구간(µs)을 out_us에 채운다. 리턴: 구간 수.
              입력 배열은 내부에서 정렬된다 (이벤트 순서는 파괴됨).
------------------------------------------------------------------------*/
static int lat_pair(LAT_EV *evs, int n, long *out_us)
{
    int         i, cnt;
    long long   in_usec;
    int         have_in;

    if (evs == NULL || n <= 0 || out_us == NULL)
        return 0;

    qsort(evs, (size_t)n, sizeof(LAT_EV), lat_ev_cmp);

    cnt = 0;
    have_in = 0;
    in_usec = 0;

    for (i = 0; i < n; i++) {
        /* (proc,key) 그룹 경계에서 미결 IN 폐기 */
        if (i > 0 && (strcmp(evs[i].proc, evs[i-1].proc) != 0 ||
                      strcmp(evs[i].key,  evs[i-1].key)  != 0))
            have_in = 0;

        if (strcmp(evs[i].point, "IN") == 0) {
            in_usec = evs[i].usec;      /* 연속 IN이면 마지막 IN 기준 */
            have_in = 1;
        }
        else if (strcmp(evs[i].point, "OUT") == 0 && have_in) {
            out_us[cnt] = (long)(evs[i].usec - in_usec);
            cnt++;
            have_in = 0;
        }
    }

    return cnt;
}

/*------------------------------------------------------------------------
    lat_pctl: nearest-rank 백분위. 입력은 오름차순 정렬 전제.
              p=0.0 → 최솟값, p=1.0 → 최댓값
------------------------------------------------------------------------*/
static long lat_pctl(long *sorted, int n, double p)
{
    int rank;

    if (sorted == NULL || n <= 0)
        return 0;
    if (p <= 0.0)
        return sorted[0];
    if (p >= 1.0)
        return sorted[n - 1];

    rank = (int)(p * n + 0.999999);     /* ceil(p*n) */
    if (rank < 1)
        rank = 1;
    if (rank > n)
        rank = n;

    return sorted[rank - 1];
}

#ifndef LAT_REPORT_NO_MAIN

static int lat_long_cmp(const void *a, const void *b)
{
    long la = *(const long *)a;
    long lb = *(const long *)b;

    if (la < lb) return -1;
    if (la > lb) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    LAT_EV  *evs;
    long    *spans;
    FILE    *fp;
    char    line[512];
    int     i, n_ev, n_bad, n_span, grp_start, grp_n;

    if (argc < 2) {
        printf("Usage: lat_report <file.lat> [file2.lat ...]\n");
        printf("  line format: <epoch_usec>|<proc>|<point>|<key>\n");
        return 1;
    }

    evs = (LAT_EV *)malloc(sizeof(LAT_EV) * LAT_MAX_EV);
    if (evs == NULL) {
        fprintf(stderr, "[ERROR] malloc failed\n");
        return 1;
    }

    n_ev = 0;
    n_bad = 0;

    for (i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "[WARN] cannot open %s\n", argv[i]);
            continue;
        }
        while (fgets(line, sizeof(line), fp) != NULL && n_ev < LAT_MAX_EV) {
            if (lat_parse_line(line, &evs[n_ev]) == 0)
                n_ev++;
            else
                n_bad++;
        }
        fclose(fp);
    }

    if (n_bad > 0)
        fprintf(stderr, "[WARN] skipped %d malformed line(s)\n", n_bad);

    if (n_ev == 0) {
        fprintf(stderr, "[ERROR] no events\n");
        free(evs);
        return 1;
    }

    spans = (long *)malloc(sizeof(long) * (n_ev / 2 + 1));
    if (spans == NULL) {
        fprintf(stderr, "[ERROR] malloc failed\n");
        free(evs);
        return 1;
    }

    /* 전체를 (proc,key,usec) 정렬 후 proc 그룹 단위로 짝짓기/통계 */
    qsort(evs, (size_t)n_ev, sizeof(LAT_EV), lat_ev_cmp);

    printf("%-16s %8s %10s %10s %10s %10s %10s\n",
            "proc", "count", "min(us)", "p50(us)", "p90(us)", "p99(us)", "max(us)");
    printf("---------------- -------- ---------- ---------- ----------"
            " ---------- ----------\n");

    grp_start = 0;
    for (i = 1; i <= n_ev; i++) {
        if (i == n_ev || strcmp(evs[i].proc, evs[grp_start].proc) != 0) {
            grp_n = i - grp_start;
            n_span = lat_pair(&evs[grp_start], grp_n, spans);
            if (n_span > 0) {
                qsort(spans, (size_t)n_span, sizeof(long), lat_long_cmp);
                printf("%-16s %8d %10ld %10ld %10ld %10ld %10ld\n",
                        evs[grp_start].proc, n_span,
                        lat_pctl(spans, n_span, 0.0),
                        lat_pctl(spans, n_span, 0.50),
                        lat_pctl(spans, n_span, 0.90),
                        lat_pctl(spans, n_span, 0.99),
                        lat_pctl(spans, n_span, 1.0));
            }
            else {
                printf("%-16s %8d %10s %10s %10s %10s %10s\n",
                        evs[grp_start].proc, 0, "-", "-", "-", "-", "-");
            }
            grp_start = i;
        }
    }

    fprintf(stderr, "[INFO] events=%d\n", n_ev);

    free(spans);
    free(evs);
    return 0;
}

#endif  /* LAT_REPORT_NO_MAIN */

/*************************************************************************
    End of Program (lat_report.c)
*************************************************************************/
