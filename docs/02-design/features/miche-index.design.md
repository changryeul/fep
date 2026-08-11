# miche-index Design Document

> **Summary**: 미체결(MICHE) 주문번호 조회 O(10,000) 선형 스캔을 프로세스-로컬 "검증+폴백" 캐시로 O(1)화. SHM 레이아웃 무변경(장마감 배포 제약 없음), 캐시 미스/불일치 시 기존 스캔 폴백으로 정확성 보장. 가치: 저지연이 아니라 **체결 폭주 시 처리량/데이터 신선도** (2026-08-06 조회수준 판단 반영).
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-06
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F6

---

## 1. 조사 결과 (설계를 결정지은 사실들)

1. **MICHE writer는 둘**: `pa_1291_mp`(응답: 삽입/정정/취소, `pa_1290_mp.c` 스캔 루프 6곳 — 삽입 :305/:677, 조회 :400/:620/:762/:887)와 **`pa_1491/1492_mp`(체결: 잔량 감소, `pa_1490_mp.c:231` 스캔 — 체결 핫패스의 본체)**. 잠금 없이 같은 (mk,acc) 파티션을 공유 → SHM에 공유 인덱스를 두면 다중 writer 정합성 문제가 생김.
2. 조회 루프는 모두 **첫 매치에서 break** — "첫 매치" 시맨틱은 OrderNo의 당일 유일성(KRX 부여) 하에서 캐시 조회와 등가.
3. 잔량 0 슬롯은 지워지지 않고 `Jan_Cnt<=0`으로 남으며 OrderNo 필드 보존 — 기존 스캔은 이런 슬롯도 매치함. 슬롯 재사용(삽입이 덮어씀) 시점에 사실상 삭제.
4. `pa_9000_mp`는 기동 시 전체 리셋만 수행 (운영 중 아님).

## 2. 설계 — 프로세스-로컬 검증+폴백 캐시

### 2.1 원칙

- **SHM 무변경**: 인덱스는 각 프로세스의 정적 메모리. 세그먼트 크기 불변 → `shmsub.c` 자동 재생성 함정 없음, 장마감 배포 제약 없음, pz 수정 없음.
- **캐시이지 권위가 아님**: `Get` 결과는 반드시 슬롯의 OrderNo `memcmp` 검증을 거치고, 미스/불일치 시 **기존 선형 스캔이 그대로 실행**(폴백) 후 결과로 캐시 보정. 다른 프로세스의 변이로 캐시가 낡아도 정확성 불변 — F4 fd_cache와 동일 철학.
- **rebuild 불필요**: 빈 캐시는 폴백 스캔이 자연 웜업. 기동 절차 무변경.

### 2.2 모듈 (`sub/miche_idx.c` + `inc/miche_idx.h` 신규 — 순수 로직, TDD)

```c
#define MICHE_IDX_HASH   16384          /* 2^14, 로드팩터 <0.61 @10000 */
#define MICHE_IDX_PROBE  32             /* linear probing 상한 */
#define MICHE_IDX_KEYLEN 10             /* OrderNo 10바이트 */

typedef struct {
    char    key[MICHE_IDX_HASH][MICHE_IDX_KEYLEN];
    int     slot[MICHE_IDX_HASH];       /* -1 = empty */
} MICHE_IDX;

void Miche_Idx_Reset(MICHE_IDX *ix);
void Miche_Idx_Put(MICHE_IDX *ix, const char *key10, int slot);
int  Miche_Idx_Get(MICHE_IDX *ix, const char *key10);   /* slot or -1 */
```

- 해시: FNV-1a 10바이트 (OrderNo는 우측정렬 공백 패딩 가능 — 바이트 해시가 안전)
- linear probing, 동일 키 발견 시 갱신, PROBE 상한 초과 시 마지막 프로브 위치 덮어씀(eviction 허용 — 캐시 시맨틱)
- 삭제 연산 없음 (낡은 항목은 Get 후 호출측 검증에서 걸러지고 Put이 덮어씀)
- 메모리: 인덱스 1개 224KB × 계좌 20 ≈ 4.5MB/프로세스 (정적)

### 2.3 호출측 전환 패턴

```c
/* before: for (i = 0; i < MAX_MICHE; i++) { if (memcmp(...OrderNo, key, 10)==0) { 본문; break; } } */

/* after */
i = Miche_Idx_Get(&Midx[acc_seq], key);
if (i >= 0 && memcmp(F_MiChe[mk][acc][i].OrderNo, key, 10) != 0)
    i = -1;                                     /* 낡은 캐시 - 폴백 */
if (i < 0) {
    for (i = 0; i < MAX_MICHE; i++)             /* 기존 스캔 그대로 (폴백) */
        if (memcmp(F_MiChe[mk][acc][i].OrderNo, key, 10) == 0)
            break;
    if (i < MAX_MICHE)
        Miche_Idx_Put(&Midx[acc_seq], key, i);  /* 보정 */
}
if (i < MAX_MICHE) { 기존 본문 }               /* 못찾음 처리도 기존 그대로 */
```

### 2.4 전환 범위 (1차: 효과 큰 3곳, 나머지는 동일 패턴 후속)

| 지점 | 내용 | 1차 |
|------|------|-----|
| `pa_1490_mp.c:231` | 체결 잔량 감소 조회 — **체결마다** | O |
| `pa_1290_mp.c:400` | 정정/취소 원주문 조회 | O |
| `pa_1290_mp.c:305` 루프 내부 | 신규 등록 완료 시 `Put(OrderNo, i)` (캐시 적재) | O |
| `pa_1290_mp.c:620/:762/:887`, `:677` Put | 자동취소/LP 경로 | 후속 (동일 패턴) |

- 인덱스 배열: 각 파일에 `static MICHE_IDX Midx[ACC_NO_CNT];` + `Init_Parameters`/`main`에서 Reset. mk는 컴파일 상수(MK_GBN)라 acc 차원만.

### 2.5 시맨틱 주의점 (문서화)

- 동일 OrderNo가 두 슬롯에 존재하는 비정상 데이터에서는 기존 스캔(최저 인덱스 첫 매치)과 캐시(마지막 Put 슬롯)가 다른 슬롯을 반환할 수 있음. KRX 주문번호는 당일 유일 — 위반은 상류 버그이며 캐시 유무와 무관하게 이미 비결정적.
- 삽입 빈슬롯 탐색 O(10,000)(:305/:677)은 1차 범위 외 (free-list는 다중 writer 보정 설계가 필요 — 후속).

## 3. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | Put/Get/Reset, 동일 키 갱신, 미등록 -1 | unit |
| V-02 | 충돌 시 probing 정확성 (같은 버킷 다수 키) | unit |
| V-03 | PROBE 초과 eviction 후에도 Get 정확(-1 또는 올바른 슬롯) | unit |
| V-04 | 10,000키 적재 후 전수 Get 일치 | unit |
| V-05 | 호출측: 검증 실패 → 폴백 → Put 보정 경로 | 코드 검토 |
| V-06 | 서버 빌드 + 전체 테스트 | 서버 |
