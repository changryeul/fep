# file-rw-optimize Planning Document

> **Summary**: F_R/F_W 파일 I/O 최적화 — 레코드 단위 잠금으로 전체 파일 잠금 대체
>
> **Project**: FEP (Front-End Processor)
> **Date**: 2026-02-28
> **Status**: Draft
> **Reference**: FEP_Architecture_Analysis.md §4.3 (성능 개선 우선순위 3위)

---

## 1. Overview

### 1.1 Purpose

`sub/file_rw.c`의 `F_R()`, `F_W()` 함수가 매 호출마다 **전체 파일 잠금**(`l_start=0, l_len=0`)을 사용. 여러 프로세스가 같은 데이터 파일에 동시 접근 시 직렬화되어 대기 시간 발생. 레코드 단위 잠금으로 변경하여 동시성 향상.

### 1.2 Background

**현재 흐름** (주문 1건 처리 시):
```
F_R():  open → 전체파일잠금(F_WRLCK, l_start=0, l_len=0) → seek → read → unlock → close
F_W():  fopen → 전체파일잠금(F_WRLCK, l_start=0, l_len=0) → write → fflush → unlock → fclose
```

**문제점**:
- `l_start=0, l_len=0`은 **파일 전체**를 잠금 (POSIX 전체 파일 잠금)
- F_R은 특정 offset의 레코드만 읽는데 전체 파일을 잠금 → 다른 프로세스가 다른 레코드에 접근할 수 없음
- F_W는 append 모드로 파일 끝에만 쓰는데 전체 파일을 잠금
- 프로세스 수가 많을수록 (PA 15개 + PB 9개) 경합 심화

**호출 현황**:
- `F_R()`: 23개 파일에서 31회 호출
- `F_W()`: 26개 파일에서 43회 호출
- `F_R2, F_W2, F_R3, F_W3, F_WB, SF_W`: src/에서 0회 호출 (선언은 있지만 미사용)

**현재 잠금 패턴** (F_R, lines 77-95):
```c
lock.l_type = F_WRLCK;
lock.l_whence = 0;
lock.l_start = 0L;     /* 파일 시작부터 */
lock.l_len = 0L;        /* 파일 끝까지 = 전체 잠금 */
rt = fcntl (fd, F_SETLKW, &lock);
```

### 1.3 Related Documents

- FEP_Architecture_Analysis.md §4.3 "F_R/F_W — 매 건마다 open/lock/close 반복"
- FEP_Architecture_Analysis.md §4.9 성능 개선 우선순위 3위

---

## 2. Scope

### 2.1 In Scope

- [ ] `F_R()` 전체 파일 잠금 → 레코드 단위 잠금 변경
- [ ] `F_W()` 전체 파일 잠금 → append 영역 잠금 변경
- [ ] `F_R2()` 동일 패턴 적용
- [ ] `F_WB()` 동일 패턴 적용 (F_WB는 fopen 후 lock → write → fflush → unlock → fclose, F_W와 동일 패턴)
- [ ] `F_R3()` 동일 패턴 적용
- [ ] `F_W3()`, `F_W2()` 동일 패턴 적용

### 2.2 Out of Scope

- **fd 풀링** (프로세스 시작 시 fd 열어두고 재사용) — 별도 피처로 분리. 파일명이 동적 계산되어 복잡도 높음
- **배치 쓰기** (매 건 fflush 대신 N건 단위) — 데이터 손실 위험. 별도 검토 필요
- **호출자 코드 변경** — file_rw.c 내부만 수정
- **SF_W()** — 시세 쓰기는 레코드 기반이 아닌 연속 append이므로 별도 검토
- **미사용 함수 삭제** — 정합성을 위해 유지, 동일 패턴 적용

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | `F_R()` 전체 파일 잠금을 레코드 범위 잠금으로 변경 (`l_start=offset, l_len=read_size`) | High | Pending |
| FR-02 | `F_W()` 전체 파일 잠금을 append 영역 잠금으로 변경 | High | Pending |
| FR-03 | `F_R2()` 레코드 단위 잠금 적용 | Medium | Pending |
| FR-04 | `F_R3()` 레코드 단위 잠금 적용 | Medium | Pending |
| FR-05 | `F_W2()` append 영역 잠금 적용 | Medium | Pending |
| FR-06 | `F_W3()` append 영역 잠금 적용 | Medium | Pending |
| FR-07 | `F_WB()` append 영역 잠금 적용 | Medium | Pending |
| FR-08 | 잠금 실패 시 기존 에러 처리 동작 유지 | High | Pending |
| FR-09 | EINTR 재시도 로직 유지 | High | Pending |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| Performance | 동시 접근 시 대기 시간 감소 | 다중 프로세스 동시 실행 시 lock 대기 측정 |
| Compatibility | 기존 호출자 74회(F_R 31 + F_W 43) 코드 변경 없음 | 코드 diff 확인 |
| Safety | 잠금 범위 축소 후에도 데이터 무결성 유지 | 동시 읽기/쓰기 테스트 |
| Portability | HP-UX, SunOS, AIX, Linux 모두 지원 | POSIX fcntl 레코드 잠금 표준 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [ ] F_R 계열(F_R, F_R2, F_R3) 전체 파일 잠금 → 레코드 범위 잠금 변경
- [ ] F_W 계열(F_W, F_W2, F_W3, F_WB) 전체 파일 잠금 → append 영역 잠금 변경
- [ ] 잠금 실패 시 에러 처리 동작 유지 (EINTR 재시도 포함)
- [ ] 변경 파일: 1개 (`sub/file_rw.c`)
- [ ] 기존 호출자 코드 변경 0건

### 4.2 Quality Criteria

- [ ] Gap Analysis 일치율 90% 이상
- [ ] 빌드 성공 (`mk.sh sub`)

---

## 5. Technical Design Preview

### 5.1 접근 방식: 레코드 단위 잠금

**F_R — 읽기 잠금 (레코드 범위)**:

현재 F_R은 `offset = IFR(...) * rec_size` 위치부터 `rec_size * p_cnt` 바이트를 읽음. 이 범위만 잠그면 됨.

```c
/* Before: 전체 파일 잠금 */
lock.l_start = 0L;
lock.l_len = 0L;

/* After: 읽을 레코드 범위만 잠금 */
offset = IFR(D_K,P_K,Fk,Ck) * rec_size;
lock.l_start = offset;
lock.l_len = (long)(rec_size * p_cnt);
```

추가로, 읽기 전용이므로 `F_RDLCK`(읽기 잠금)으로 변경 가능. 읽기 잠금은 다른 읽기와 공존 가능 → 동시 읽기 성능 향상.

```c
/* Before: 쓰기 잠금 (배타적) */
lock.l_type = F_WRLCK;

/* After: 읽기 잠금 (공유) */
lock.l_type = F_RDLCK;
```

**F_W — append 영역 잠금**:

F_W는 `fopen64("a+w")`로 파일 끝에 추가. 잠금은 현재 파일 크기부터 쓸 크기만큼.

```c
/* Before: 전체 파일 잠금 */
lock.l_start = 0L;
lock.l_len = 0L;

/* After: append 영역 잠금 — 파일 끝부터 */
lock.l_whence = SEEK_END;
lock.l_start = 0L;
lock.l_len = 0L;  /* 파일 끝 이후 전체 (append 영역) */
```

`SEEK_END` + `l_start=0` + `l_len=0`은 "파일 끝부터 끝까지" = append 영역만 잠금. 기존 데이터 읽기와 공존 가능.

### 5.2 왜 전체 파일 잠금 대신 레코드 잠금인가

- **F_R**은 특정 offset의 레코드만 읽음 → 해당 범위만 잠그면 다른 프로세스가 다른 레코드 접근 가능
- **F_W**는 파일 끝에 append → append 영역만 잠그면 기존 데이터 읽기와 공존
- POSIX `fcntl` 레코드 잠금은 모든 타겟 플랫폼(HP-UX, SunOS, AIX, Linux)에서 지원
- 함수 시그니처/반환값 변경 없음 → 호출자 영향 0

### 5.3 주의사항

- F_R은 현재 `F_WRLCK`(쓰기 잠금)을 사용. 읽기 후 IFR 값을 갱신하는 경우가 있으므로 (lines 112-118), 해당 경로에서는 쓰기 잠금이 필요. 단, IFR 갱신은 SHM에서 수행되므로 파일 잠금과 무관. → `F_RDLCK`로 변경 안전
- F_W의 `fflush()` 시점은 변경하지 않음 (이번 스코프 외)
- offset 계산은 lock 설정 전에 수행해야 함 (기존 코드에서 offset 계산은 lock 후에 수행 → lock 전으로 이동 필요)

---

## 6. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| 레코드 잠금 범위 오류로 데이터 손상 | 높음 | 낮음 | offset 계산을 기존 로직 그대로 재사용. rec_size * p_cnt 정확히 계산 |
| F_R에서 F_RDLCK 사용 시 동시 쓰기와 경합 | 중간 | 낮음 | F_W는 append 영역만 잠그므로 F_R의 읽기 범위와 겹치지 않음 |
| SEEK_END 기반 잠금의 플랫폼 호환성 | 낮음 | 매우 낮음 | POSIX 표준. HP-UX/AIX/SunOS/Linux 모두 지원 |
| offset 계산 순서 변경으로 인한 side effect | 중간 | 낮음 | F_R에서만 해당. offset 계산은 순수 산술이므로 lock 전후 동일 결과 |

---

## 7. Impact Analysis

### 7.1 변경 파일

| File | Change | Risk |
|------|--------|------|
| `sub/file_rw.c` | F_R/F_W 계열 7개 함수의 lock 범위 축소 | 중간 |

### 7.2 영향 범위

- **호출자 변경 없음**: F_R 31회 + F_W 43회 = 74회 호출 모두 수정 불필요
- **빌드 영향**: `libfepP.a` 재빌드 → 전체 바이너리 재링크
- **런타임 영향**: 동시 파일 접근 시 잠금 대기 시간 감소

### 7.3 개선 효과

| 시나리오 | 현재 | 개선 후 |
|----------|------|---------|
| 2개 프로세스가 같은 파일의 다른 레코드 읽기 | 직렬화 (전체 잠금) | 동시 실행 (레코드 잠금) |
| 읽기와 append 동시 수행 | 직렬화 (전체 잠금) | 동시 실행 (범위 분리) |
| 단일 프로세스 접근 | 변화 없음 | 변화 없음 |

---

## 8. Next Steps

1. [ ] Design 문서 작성 (`file-rw-optimize.design.md`)
2. [ ] 구현 (`sub/file_rw.c` 수정)
3. [ ] Gap Analysis
4. [ ] Completion Report

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-28 | Initial draft | Claude |
