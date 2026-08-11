# stat-save-optimize Planning Document

> **Summary**: Stat_Save() 파일 I/O 빈도를 시간 기반 스로틀링으로 95% 이상 감소
>
> **Project**: FEP (Front-End Processor)
> **Date**: 2026-02-27
> **Status**: Draft
> **Reference**: FEP_Architecture_Analysis.md §4.2

---

## 1. Overview

### 1.1 Purpose

`Stat_Save()` 함수가 모든 프로세스의 메인 루프 반복마다 호출되어, 매 호출 시 `fopen → fcntl lock → fwrite → fflush → fcntl unlock → fclose` 전체 파일 I/O 사이클을 수행한다. 이를 시간 기반 스로틀링으로 불필요한 디스크 I/O를 제거한다.

### 1.2 Background

**현재 동작 (`sub/stat_save.c`)**:

```
Stat_Save() 호출 시마다:
1. gettimeofday() + localtime_r() — 시간 계산
2. AtoIf() × 4 — 기동/종료 시간 파싱
3. 영업시간 외 판정 → 해당 시 Exit_Process()
4. sprintf() — 파일 경로 구성
5. fopen("r+") — 파일 열기
6. fcntl(F_SETLKW) — 전체 파일 잠금
7. sprintf() — 상태 데이터 포맷팅
8. fseek(0) + fwrite() + fflush() — 디스크 기록
9. fcntl(F_UNLCK) + fclose() — 잠금 해제 + 닫기
```

**호출 빈도**:
- 58건의 호출 위치, 45개 소스 파일 (PA 31, PB 9, PW 4, SUB 2)
- poll timeout이 5ms(장중 ts 프로세스)인 경우 → **초당 최대 200회** fopen/fwrite/fclose
- 실제 필요 빈도: 프로세스 상태는 1~5초 간격 갱신이면 충분

**기록하는 데이터**: `IF_SEQ`, `START_STAT`, `TCP_NSTAT` 등 — 모두 SHM 매크로에서 읽은 값. SHM에 이미 존재하는 데이터를 파일로 복사하는 역할.

**파일 경로**: `{_FEP_DAT}/{SubSystem}/00000000/{process_id}_stat`

### 1.3 Related Documents

- FEP_Architecture_Analysis.md §4.2 "Stat_Save() 매 루프 호출 — 불필요한 파일 I/O"
- FEP_Architecture_Analysis.md §4.9 성능 개선 우선순위 2위

---

## 2. Scope

### 2.1 In Scope

- [ ] `sub/stat_save.c`의 `Stat_Save()` 함수 내부에 시간 기반 스로틀링 추가
- [ ] 스로틀링 간격: 기본 3초 (매크로 상수로 정의)
- [ ] 영업시간 외 종료 로직은 매 호출 유지 (안전성)
- [ ] 파일 I/O 부분만 스로틀 대상

### 2.2 Out of Scope

- 호출자 코드 변경 (58개 call site는 그대로 유지)
- Stat_Save() 함수 시그니처 변경
- SHM 기반 상태 저장으로의 전환 (향후 과제)
- Seq_Save() / Dshm_Seq_Save() 최적화 (별도 기능)
- stat 파일 소비자(모니터링) 측 변경

---

## 3. Requirements

### 3.1 Functional Requirements

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-01 | Stat_Save() 내부에 시간 기반 스로틀링 추가: 마지막 파일 기록 이후 `STAT_SAVE_INTERVAL_SEC`초 미만이면 파일 I/O 건너뜀 | High | Pending |
| FR-02 | 영업시간 외 종료 판정(gettimeofday + 시간 비교 + Exit_Process)은 매 호출마다 실행 유지 | High | Pending |
| FR-03 | `STAT_SAVE_INTERVAL_SEC` 매크로를 `inc/fep_sub.h` 또는 `sub/stat_save.c` 내에 정의 (기본값 3) | Medium | Pending |
| FR-04 | 프로세스 종료 시(setsigfatal.c의 시그널 핸들러) Stat_Save 호출은 스로틀 무시하고 즉시 기록 | High | Pending |
| FR-05 | static 변수로 마지막 기록 시각 보관 (`static time_t last_save_time`) | Medium | Pending |

### 3.2 Non-Functional Requirements

| Category | Criteria | Measurement Method |
|----------|----------|-------------------|
| Performance | 파일 I/O 95% 이상 감소 (3초 간격 기준) | 호출 횟수 대비 실제 fopen 횟수 비율 |
| Compatibility | 기존 호출자 58건 코드 변경 없음 | 코드 diff 확인 |
| Safety | 영업시간 종료 판정 지연 없음 | 시간 비교 로직이 스로틀 전에 실행됨 확인 |
| Reliability | 프로세스 비정상 종료 시 최대 3초 이내의 상태 정보 손실만 허용 | 시그널 핸들러 즉시 기록 확인 |

---

## 4. Success Criteria

### 4.1 Definition of Done

- [ ] Stat_Save() 내부 스로틀링 구현 완료
- [ ] 시그널 핸들러(setsigfatal.c) 경로에서 강제 기록 확인
- [ ] 영업시간 종료 로직 매 호출 실행 확인
- [ ] STAT_SAVE_INTERVAL_SEC 매크로 정의
- [ ] 변경 파일: sub/stat_save.c (1개) + 선택적으로 inc/fep_sub.h (1개)
- [ ] 기존 호출자 코드 변경 0건

### 4.2 Quality Criteria

- [ ] Gap Analysis 일치율 90% 이상
- [ ] 빌드 성공 (mk.sh sub)
- [ ] 코드 리뷰 완료

---

## 5. Technical Design Preview

### 5.1 접근 방식: 함수 내부 스로틀링

**핵심 아이디어**: `Stat_Save()` 함수 시그니처를 변경하지 않고, 내부에 `static` 타임스탬프를 두어 간격 미달 시 파일 I/O를 건너뛴다. 호출자 58곳은 수정 불필요.

```c
void Stat_Save (void)
{
    static time_t last_save_time = 0;
    time_t now;
    /* ... gettimeofday, 영업시간 판정 (매 호출 실행) ... */

    /* 스로틀: STAT_SAVE_INTERVAL_SEC 미만이면 파일 I/O 건너뜀 */
    now = tv.tv_sec;
    if (now - last_save_time < STAT_SAVE_INTERVAL_SEC)
        return;
    last_save_time = now;

    /* ... 이하 기존 파일 I/O 코드 그대로 ... */
}
```

### 5.2 시그널 핸들러 강제 기록

`setsigfatal.c`에서 호출되는 경우, 프로세스 종료 직전이므로 스로틀을 무시해야 한다.

**방안 A (선호)**: `Stat_Save()` 내부의 `last_save_time`을 외부에서 리셋하는 방법
```c
/* stat_save.c에 추가 */
void Stat_Save_Force (void) {
    /* last_save_time을 0으로 리셋 후 Stat_Save 호출 */
}
```

**방안 B**: setsigfatal.c는 이미 `Stat_Save()` 호출 전에 프로세스 상태를 변경하므로, 시그널 핸들러에서는 `Stat_Save_Force()`를 호출.

### 5.3 영업시간 판정 분리

현재 `Stat_Save()` 안에 영업시간 판정 + `Exit_Process()` 호출이 포함되어 있다. 이 로직은 스로틀 대상이 아니므로 파일 I/O 전에 위치해야 한다.

```
Stat_Save() 실행 흐름:
1. gettimeofday() — 항상 실행
2. 영업시간 비교 — 항상 실행 (영업시간 외이면 Exit_Process)
3. 스로틀 검사 — 간격 미달이면 return
4. 파일 I/O — 간격 도달 시에만 실행
```

현재 코드 순서가 이미 이 흐름과 일치한다 (lines 31-61: 시간 계산/영업시간, lines 63-137: 파일 I/O). 따라서 스로틀 검사를 line 62 직후에 삽입하면 된다.

---

## 6. Risks and Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| 모니터링 도구가 stat 파일의 실시간 갱신에 의존 | 중간 | 낮음 | 3초 간격이면 운영 모니터링에 충분. 필요시 간격 조정 가능 |
| 프로세스 비정상 종료 시 마지막 상태가 최대 3초 지연 | 낮음 | 낮음 | FR-04: 시그널 핸들러에서 강제 기록으로 커버 |
| static 변수가 fork() 이후 자식에서 초기화되지 않음 | 낮음 | 낮음 | FEP 프로세스는 daemon화 이후 fork하지 않음 |
| time() 호출 오버헤드 | 없음 | 해당없음 | 이미 gettimeofday()를 호출 중이므로 추가 syscall 없음 (tv.tv_sec 재사용) |

---

## 7. Impact Analysis

### 7.1 변경 파일

| File | Change | Risk |
|------|--------|------|
| `sub/stat_save.c` | 스로틀 로직 추가 (~10줄), `Stat_Save_Force()` 추가 (~5줄) | 낮음 |
| `inc/fep_sub.h` | `STAT_SAVE_INTERVAL_SEC` 매크로, `Stat_Save_Force()` extern | 낮음 |
| `sub/setsigfatal.c` | `Stat_Save()` → `Stat_Save_Force()` 변경 (1줄) | 낮음 |

### 7.2 영향 범위

- **호출자 변경 없음**: 58개 call site, 45개 파일 모두 수정 불필요
- **빌드 영향**: `libfepP.a`만 재빌드 → 전체 바이너리 재링크 필요 (`mk.sh all`)
- **런타임 영향**: 파일 I/O 빈도 약 95%+ 감소

### 7.3 I/O 감소 예상치

| 프로세스 유형 | poll timeout | 현재 빈도 | 개선 후 빈도 | 감소율 |
|-------------|-------------|----------|------------|--------|
| ts (장중) | 3~5초 | ~15회/분 | ~20회/분 | 0% (이미 간격 이상) |
| tr (데이터) | 5~15초 | ~6회/분 | ~20회/분 | 0% (이미 간격 이상) |
| mp (FIFO 내부루프) | 즉시 반환 | 수백회/분 | ~20회/분 | **95%+** |
| dd (배분 내부루프) | 즉시 반환 | 수백회/분 | ~20회/분 | **95%+** |
| ur (UDP) | 패킷당 | 수백회/분 | ~20회/분 | **95%+** |

**핵심**: poll timeout이 충분한 프로세스(ts/tr)는 이미 3초 이상 간격이므로 영향 없음. **실제 I/O 감소는 내부 루프(mp/dd)와 고빈도 프로세스(ur)에서 발생**.

---

## 8. Next Steps

1. [ ] Design 문서 작성 (`stat-save-optimize.design.md`)
2. [ ] 구현 (sub/stat_save.c, inc/fep_sub.h, sub/setsigfatal.c)
3. [ ] Gap Analysis
4. [ ] Completion Report

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | Initial draft | Claude |
