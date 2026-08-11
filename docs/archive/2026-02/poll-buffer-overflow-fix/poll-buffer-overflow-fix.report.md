# poll-buffer-overflow-fix 완료 보고서

> **기능**: poll-buffer-overflow-fix
> **유형**: BUG FIX (Critical)
> **프로젝트**: FEP (Front-End Processor)
> **날짜**: 2026-02-22
> **일치율**: 100% (4/4)
> **반복**: 0

---

## 1. 실행 요약

`pb_7200_tr.c`의 `Init_Parameters()` 함수가 `Poll[1]` (인덱스 1)에 쓰는 중요 버퍼 오버플로우 버그 수정. 배열은 `struct pollfd Poll[1]` (1개 요소만, 유효한 인덱스: 0)로 선언. 이로 인해 프로세스 시작마다 BSS 세그먼트가 조용히 손상됨.

**근본 원인**: `pb_7100_ts.c`에서 복사 (올바르게 `Poll[2]` 선언). pb_7200_tr에서 배열 크기와 PollCnt가 1로 축소되었지만 dead `Poll[1]` 할당은 남겨짐.

**수정**: 2개의 dead 줄 제거. 동작 변화 없음 — 쓰여진 값이 poll되지 않음 (`PollCnt = 1`).

---

## 2. PDCA 사이클 개요

| Phase | Status | 세부사항 |
|-------|:------:|---------|
| Plan | Done | 버그 확인, 범위 검증 (1개 파일, 4개 유사 파일 확인됨) |
| Design | Done | 정확한 BEFORE/AFTER with 줄 번호 |
| Do | Done | Init_Parameters()에서 2줄 제거 |
| Check | Done | 100% 일치율 (4/4 항목) |
| Act | Skipped | 첫 번째 체크에서 100% |
| Report | Done | 이 문서 |

---

## 3. 구현 요약

| 파일 | 변경 | Delta |
|------|--------|:-----:|
| `src/PB/pb_7200_tr.c` | `Poll[1].fd = INPUT_FD;` 및 `Poll[1].events = POLLIN;` 제거 | **-2** |

### 범위 검증

`struct pollfd Poll[1]`을 사용하는 동일한 버그에 대해 검사한 파일:

| 파일 | 버그? | 이유 |
|------|:----:|--------|
| `pb_7200_tr.c` | **YES (fixed)** | Poll 크기 [1]로 Poll[1] 접근 |
| `pa_7000_tr.c` | No | Poll[0]만 사용 |
| `pa_8200_tr.c` | No | Poll[0]만 사용 |
| `pb_8200_tr.c` | No | Poll[0]만 사용 |
| `pz_fepp.c` | No | Poll[0]만 사용 |

---

## 4. 품질 지표

| 지표 | 값 |
|--------|-------|
| 일치율 | 100% |
| 반복 | 0 |
| 파일 변경 | 1 |
| 줄 제거 | 2 |
| 줄 추가 | 0 |

### 누적 프로젝트 지표 (10개 기능)

| 기능 | 유형 | 일치율 | Delta |
|---------|------|:----------:|:-----:|
| config-db-migration | New feature | 90% | +4500 |
| tr-struct-refactor | Refactor | 92% | ~0 |
| ini-config-analysis | Documentation | 97.5% | 0 |
| shm-struct-refactor | Refactor | 95% | -19 |
| fifo-struct-refactor | Refactor | 97% | ~0 |
| pa-pb-dedup (Phase 1) | Dedup | 90% | -1133 |
| pa-pb-dedup-phase2 | Dedup | 97% | -605 |
| pa-pb-dedup-phase3 | Dedup | 100% | -183 |
| fifo-event-rtn-adoption | Dedup | 100% | -195 |
| **poll-buffer-overflow-fix** | **Bug fix** | **100%** | **-2** |

---

## 5. 교훈

1. **복사-붙여넣기는 버그 벡터**: 파일이 다른 곳에서 forked되고 배열 크기가 변경될 때, 이전 인덱스를 참조하는 dead code가 탐지되지 않은 채 남을 수 있음
2. **BSS 오버플로우는 조용함**: 스택 오버플로우와 달리 글로벌 배열 뒤에 쓰기는 인접한 글로벌 변수를 손상시키지만 충돌을 일으키지 않음 — 버그를 거의 보이지 않게 만듦
3. **체계적 범위 검사가 중요**: 5개의 `Poll[1]` 선언 파일을 모두 검증하여 1개만 버그를 가지고 있음을 확인하여 false negatives 방지

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | Initial report | Claude Code |
