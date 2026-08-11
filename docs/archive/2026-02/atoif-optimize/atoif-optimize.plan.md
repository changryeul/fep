# Plan: atoif-optimize

## Overview

FEP_Architecture_Analysis.md §4.7 성능 개선 항목. 문자열→숫자 변환 함수 3개(`AtoIf`, `AtoLf`, `AtoDf`)의 내부 루프를 O(n*10)에서 O(n)으로 최적화.

## Background

현재 코드는 각 문자를 숫자로 변환할 때 `'0'`부터 `'9'`까지 최대 10회 비교하는 내부 루프를 사용:

```c
for (j = 0; j < 10; j++) {
    if (*(p_ascii+i) == ('0' + j))
        break;
}
if (j < 10) jj = jj * 10 + j;
```

직접 산술(`c - '0'`)로 대체하면 문자당 10회 비교가 1회 뺄셈+범위 검사로 줄어든다.

## Scope

### Target Files (3)

| File | Function | Return Type | Active Call Sites |
|------|----------|-------------|-------------------|
| `sub/atoif.c` | `AtoIf(char*, int)` | `int` | 621 (src 558 + sub 63) |
| `sub/atolf.c` | `AtoLf(char*, int)` | `long` | 25 (src 22 + sub 3) |
| `sub/atodf.c` | `AtoDf(char*, int)` | `double` | 72 (src 69 + sub 3) |

**Total**: 718 call sites. **Caller changes: 0** (함수 시그니처 불변).

### Out of Scope

- `ItoAf` (itoaf.c) — 이미 효율적 (modular arithmetic, O(n))
- 호출자 코드 변경 — 시그니처 동일, 동작 동일
- `dtoaf.c` — dtoaf-dead-code-delete 피처에서 이미 삭제됨

## Functional Requirements

### FR-01: AtoIf — 내부 루프 제거

**현재**: `for (j=0; j<10; j++) if (*(p_ascii+i) == ('0'+j)) break;`
**변경**: `if (c >= '0' && c <= '9') jj = jj * 10 + (c - '0');`

- O(n*10) → O(n) per character
- 음수 부호(`'-'`) 처리 유지
- 비숫자 문자 건너뛰기 동작 유지 (기존과 동일: j>=10이면 무시)
- 반환 타입 `int` 유지

### FR-02: AtoLf — 동일 패턴 적용

- AtoIf와 동일한 최적화
- 반환 타입 `long` 유지
- 음수 처리 유지

### FR-03: AtoDf — 소수점 처리 포함 최적화

- 내부 `for (j=0; j<10; j++)` 루프를 직접 산술로 대체
- 소수점(`.`) 처리 로직 유지
- 음수 처리 유지
- 반환 타입 `double` 유지

### FR-04: 동작 호환성 보장

- 선행 공백/0 처리: 기존과 동일 (공백은 무시, 선행 0은 숫자로 처리)
- 빈 문자열 (p_len == 0): 기존과 동일 (0 반환)
- 음수 부호 위치: 기존과 동일 (문자열 어디에 있든 감지)
- 비숫자 문자: 기존과 동일 (무시)
- `+` 부호: 기존과 동일 (무시, 양수 취급)

### FR-05: 코딩 스타일 유지

- C89/ANSI C 준수
- 기존 주석 헤더 형식 유지
- 함수 프로토타입 위치 유지 (파일 상단)
- 탭 들여쓰기 유지

## Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| 동작 변경 | Very Low | 수학적으로 동일한 변환 — `('0'+j) == c` ↔ `c-'0' == j` |
| 오버플로 | None | 기존과 동일한 누적 로직 (`jj * 10 + digit`) |
| 호출자 영향 | None | 시그니처 불변, 718곳 변경 0건 |

## Expected Impact

- **성능**: AtoIf/AtoLf/AtoDf 함수 5~10배 속도 향상 (문자당 비교 10회 → 1회)
- **코드 라인**: 각 파일 순 감소 (내부 for 루프 4줄 → 범위 검사 2줄)
- **가독성**: 표준적인 `c - '0'` 관용구로 의도가 명확해짐
- **영향 범위**: pa_5010_mp.c(171회), pz_memory_conf.c(33회), config_db.c(35회) 등 고빈도 호출자에서 누적 효과
