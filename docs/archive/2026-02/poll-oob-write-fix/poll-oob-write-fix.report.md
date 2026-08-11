# poll-oob-write-fix 완료 보고서

> **기능**: poll-oob-write-fix
> **유형**: BUG FIX (Critical) + Dead Code Cleanup
> **프로젝트**: FEP (Front-End Processor)
> **날짜**: 2026-02-22
> **일치율**: 100% (7/7)
> **반복**: 0

---

## 1. 실행 요약

2개 파일의 `Init_Parameters()`가 `Poll[2]` (인덱스 2)에 쓰는 버퍼 오버플로우 버그 수정. 배열은 `struct pollfd Poll[2]` (2개 요소만, 유효한 인덱스: 0, 1)로 선언됨. 또한 2개 추가 파일에서 동일한 주석 처리된 dead code 정리.

**근본 원인**: ancestor 파일 (예: `pa_1100_ts.c`)에서 복사. 이들은 올바르게 `Poll[3]` 선언하고 3개 인덱스 모두 사용. TR variants가 2개 event sources만으로 생성될 때 배열은 `Poll[2]`로 축소되었지만 INPUT_FD를 위한 dead `Poll[2]` 초기화는 남겨짐.

**수정**: 4개의 dead 줄 제거 (2 active OOB writes + 2 주석 블록 = 12개 total 줄 제거). 동작 변화 없음 — 쓰여진 값이 poll되지 않음 (`PollCnt` max 2, switch는 인덱스 0-1만 처리).

---

## 2. PDCA 사이클 개요

| Phase | Status | 세부사항 |
|-------|:------:|---------:|
| Plan | Done | 버그 확인, 모든 8개 `Poll[2]` 선언 파일 검증 |
| Design | Done | 4개 파일을 위한 exact BEFORE/AFTER with 줄 번호 |
| Do | Done | 4개 Init_Parameters() 함수에서 dead code 제거 |
| Check | Done | 100% 일치율 (7/7 항목) |
| Act | Skipped | 첫 번째 체크에서 100% |
| Report | Done | 이 문서 |

---

## 3. 구현 요약

| 파일 | 변경 | Delta |
|------|--------|:-----:|
| `src/PB/pb_1200_tr.c` | active `Poll[2].fd = INPUT_FD`와 `Poll[2].events = POLLIN` 제거 | **-2** |
| `src/PA/pa_1200_tr.c` | active `Poll[2].fd = INPUT_FD`와 `Poll[2].events = POLLIN` 제거 | **-2** |
| `src/PB/pb_7800_tr.c` | `/* Poll[2].fd ... */` comment 블록 제거 | **-4** |
| `src/PA/pa_7800_tr.c` | `/* Poll[2].fd ... */` comment 블록 제거 | **-4** |
| **합계** | | **-12** |

### 범위 검증

`struct pollfd Poll[2]`를 사용하는 모든 파일이 동일한 버그에 대해 검사됨:

| 파일 | 버그? | 이유 |
|------|:----:|--------|
| `pb_1200_tr.c` | **YES (fixed)** | 배열 크기 [2]로 active Poll[2] write |
| `pa_1200_tr.c` | **YES (fixed)** | 배열 크기 [2]로 active Poll[2] write |
| `pb_7800_tr.c` | Commented (cleaned) | Poll[2] write가 이미 주석 처리됨 |
| `pa_7800_tr.c` | Commented (cleaned) | Poll[2] write가 이미 주석 처리됨 |
| `pa_8100_ts.c` | No | Poll[0]과 Poll[1]만 사용 |
| `pb_8100_ts.c` | No | Poll[0]과 Poll[1]만 사용 |
| `pa_7100_ts.c` | No | Poll[0]과 Poll[1]만 사용 |
| `pb_7100_ts.c` | No | Poll[0]과 Poll[1]만 사용 |

---

## 4. 품질 지표

| 지표 | 값 |
|--------|-------|
| 일치율 | 100% |
| 반복 | 0 |
| 파일 변경 | 4 |
| 줄 제거 | 12 |
| 줄 추가 | 0 |

### 누적 프로젝트 지표 (11개 기능)

| 기능 | 유형 | 일치율 | Delta |
|---------|------|:----------:|------:|
| config-db-migration | New feature | 90% | +4500 |
| tr-struct-refactor | Refactor | 92% | ~0 |
| ini-config-analysis | Documentation | 97.5% | 0 |
| shm-struct-refactor | Refactor | 95% | -19 |
| fifo-struct-refactor | Refactor | 97% | ~0 |
| pa-pb-dedup (Phase 1) | Dedup | 90% | -1133 |
| pa-pb-dedup-phase2 | Dedup | 97% | -605 |
| pa-pb-dedup-phase3 | Dedup | 100% | -183 |
| fifo-event-rtn-adoption | Dedup | 100% | -195 |
| poll-buffer-overflow-fix | Bug fix | 100% | -2 |
| **poll-oob-write-fix** | **Bug fix** | **100%** | **-12** |

---

## 5. 교훈

1. **같은 버그 클래스, 다른 배열 크기**: Poll[1] OOB 버그 (기능 #10)과 이 Poll[2] OOB 버그는 같은 근본 원인 — 배열 크기 축소를 동반한 복사-붙여넣기. 한 인스턴스를 수정한 후 체계적인 스캔이 두 번째를 찾음.
2. **주석 처리된 코드는 악취**: pb_7800_tr.c와 pa_7800_tr.c는 버그가 이미 줄들을 주석 처리하여 "수정"되었지만 dead code는 남겨짐. 적절한 정리는 주석 블록을 완전히 제거.
3. **Poll 인덱스 vs event constant 정렬**: 올바른 패턴은 FIFO를 위한 Poll[FIFO_EVENT=0]과 socket을 위한 Poll[SOCKET_EVENT=1]. `Poll[2]` (INPUT_FD)는 해당하는 event constant나 switch case가 없어 dead code를 확인.

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | Initial report | Claude Code |
