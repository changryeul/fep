# file-rw-fd-pool Design Document

> **Summary**: 파일 큐 핫패스(F_R/F_W)와 FIFO 통지의 매건 open/close를 범용 fd 캐시(`sub/fd_cache.c`)로 제거. 실측(lat_bench) 기준 F_W 8.77µs/F_R 4.74µs 중 open/close 비중(≈3µs/쌍)을 절감. MAX_CNT 배칭은 정합성 문제로 제외.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-05
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F4

---

## 1. 조사 결과

### 1.1 file_rw.c 구조

- 모든 데이터 파일 경로는 `%s/<부문>/00000000/<파일명>` — **고정 디렉토리, 일자 롤오버 없음** (F3의 `_seq`와 동일). `compact.sh`도 삭제하지 않음 → 프로세스 수명 동안 fd 캐시 유효.
- 매건 open/close 지점:
  - `F_R` (`file_rw.c:66,115`) — 주문 인바운드 핫패스 (pb_1101_ts)
  - `F_W` (`:236,272`) — 체결 아웃바운드 핫패스 (pb_1201_tr)
  - **FIFO 통지**: `F_W_Proc:367-383`, `F_W2_Proc:677-689`, `F_WB:995-1007` — 레코드당 × 소비자 수만큼 open/write("1")/close 반복
  - 비핫패스: `F_R2/F_R3/F_W2/F_W3/F_WB/SF_W`의 파일 open/close — 1차 범위 제외 (아래 2.3)

### 1.2 MAX_CNT 배칭 제외 결정 (마스터 계획 F4 항목 축소)

`MAX_CNT` 1→8 상향은 기계적 변경이 아님:
- `Make_Data_Block`(pb_1100_ts.c)이 레코드 루프에서 **모든 레코드에 동일한 `INT_SEQ+1`을 DataSeq로 부여** — 다건 시 중복 일련번호
- `Data_Event_Rtn`의 `Add_Count(PS_R_1, 1)`이 상수 1 — 다건 소비 시 커서 미달 전진(주문 재전송 위험)
- 다건을 한 KRX 패킷으로 보낼지/건별로 보낼지, 암호화(`EncryptAndMakeSendPacket`) 단위 재설계 필요

→ 주문 정확성에 닿는 변경이므로 **F4에서 제외**하고, 파일 큐 자체를 DSHM으로 대체하는 F5에서 배칭을 함께 설계한다. 마스터 계획 F4 항목에서 "MAX_CNT 1→8" 삭제.

## 2. 설계

### 2.1 범용 fd 캐시 (`sub/fd_cache.c` 신규 — 순수 로직, 단위 테스트 대상)

```c
/* inc/fep_sub.h */
extern int      Fd_Cache_Get (const char *, int, int);      /* 경로별 raw fd */
extern FILE     *Fp_Cache_Get (const char *, const char *); /* 경로별 FILE*  */
extern void     Fd_Cache_Close_All (void);
```

- 정적 테이블 32슬롯, 키 = 경로 문자열. 최초 호출 시 open/fopen64, 이후 동일 fd/FILE* 반환.
- **오버플로/오픈 실패 시 -1/NULL 반환** — 호출측이 기존 방식(직접 open + 사용 후 close)으로 폴백. 캐시는 순수 최적화이며 실패해도 동작 불변.
- `Fd_Cache_Close_All`: 테스트/종료용 전체 close.
- 스레드 안전 불필요 (단일 스레드 이벤트 루프).

### 2.2 호출측 전환 패턴 (동작 보존 폴백)

```c
/* before */                          /* after */
fd = open(f_name, flags, 0664);       fd = Fd_Cache_Get(f_name, flags, 0664);
...                                   cached = (fd != -1);
close(fd);                            if (!cached) fd = open(f_name, flags, 0664);
                                      ...
                                      if (!cached) close(fd);
```

### 2.3 전환 범위 (핫패스 한정)

| 지점 | 전환 | 근거 |
|------|------|------|
| `F_R` 파일 fd (`:66,115`) | O | 주문 인바운드 매건 |
| `F_W` 파일 FILE* (`:236,272`) | O | 체결 아웃바운드 매건 |
| `F_W_Proc` FIFO 통지 (`:367-383`) | O | 매건 × 소비자 수 |
| `F_W2_Proc` FIFO 통지 (`:677-689`) | O | F_W2/F_W3 공용 — 동일 헬퍼 기계 적용 |
| `F_WB` FIFO 통지 (`:995-1007`) | O | 동일 |
| `F_R2/F_R3/F_W2/F_W3/F_WB/SF_W` 파일 open/close | X | 비핫패스(일괄/장운영) — 후속. 범위 최소화로 리뷰 표면 축소 |

**FIFO 캐시 주의**: 캐시된 FIFO fd는 FIFO 재생성(pz 재기동) 시 구 inode를 가리킴. 현행 기동 순서상 프로세스 운행 중 pz 재기동은 지원 상태가 아니므로 허용 — 설계 문서에 명시.

**fcntl 잠금과 fd 캐시**: fcntl 레코드 잠금은 (프로세스, 파일) 단위라 같은 프로세스가 같은 fd를 재사용해도 잠금/해제 시맨틱 불변. 단, **fclose가 아닌 명시적 F_UNLCK 유지 필수** (기존 코드도 unlock 후 close — unlock 로직 그대로 유지).

## 3. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | 동일 경로 → 동일 fd/FILE*, 다른 경로 → 다른 슬롯 | unit |
| V-02 | 오버플로(32 초과)/오픈 실패 시 -1/NULL 폴백 | unit |
| V-03 | Close_All 후 재획득 | unit |
| V-04 | 호출측 폴백 경로 동작 (캐시 실패 시 기존 방식) | 코드 검토 |
| V-05 | 잠금/해제 시맨틱 불변 (unlock 코드 유지) | 코드 검토 |
| V-06 | 서버 빌드 + 전체 단위 테스트 + lat_bench 대응 패턴 재측정 | 서버 |
