ㅌ₩ㅌ dtoaf-dead-code-delete Planning Document

> **요약**: Delete unused `dtoaf.c` from shared library — zero callers, no extern, contains buffer overflow bugs
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

Remove the dead code file `sub/dtoaf.c` (function `DtoAf`) from the codebase. This function:
- Has **zero callers** across the entire codebase
- Has **no extern declaration** in any header file (not in `fep_sub.h`)
- Is not referenced in any Makefile
- Contains real buffer overflow vulnerabilities (`buf[20]` overflow, dynamic format string)

The sibling conversion functions (`AtoDf`, `AtoLf`, `AtoIf`, `ItoAf`) are all actively used. Only `DtoAf` is dead code.

### 1.2 배경

Discovered during a systematic codebase scan for security vulnerabilities. Initial classification was "CRITICAL: dynamic format string vulnerability" but investigation revealed the function is never called. Deletion is the cleanest fix — consistent with our dead-code-cleanup history (Feature #5: -1,945 lines across 36 files).

### 1.3 Evidence of Dead Code

| Check | 결과 |
|-------|--------|
| `grep -r "DtoAf" st01/src/` | 0 matches |
| `grep -r "DtoAf" st01/sub/` | Only definition in dtoaf.c itself |
| `grep -r "DtoAf" st01/inc/` | 0 matches (no extern in fep_sub.h) |
| `grep -r "DtoAf" st01/make/` | 0 matches |
| Sibling `ItoAf` callers | 80+ active call sites |
| Sibling `AtoDf` callers | 5 active source files |
| Sibling `AtoLf` callers | 2 active source files |

---

## 2. 범위

### 2.1 범위 내

- [x] Delete `sub/dtoaf.c` (dead code, 31 lines)

### 2.2 범위 외

- Sibling files `atodf.c`, `atolf.c`, `atoif.c`, `itoaf.c` (all actively used)
- Any Makefile changes (`Make_Lib_P_c.sh` auto-discovers `sub/*.c`, so deletion is auto-detected)
- Any header changes (no extern exists for `DtoAf`)

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | Delete `sub/dtoaf.c` | High | Pending |

### 3.2 비기능 요구사항

| 범주 | 기준 |
|----------|----------|
| Build | `mk.sh sub` succeeds without `dtoaf.o` in `libfepP.a` |
| Correctness | Zero functional change — no binary references this code |

---

## 4. 성공 기준

### 4.1 완료 정의

- [x] `sub/dtoaf.c` deleted
- [x] `grep -r "DtoAf"` returns zero matches in active code
- [x] `dtoaf.o` no longer produced or archived into `libfepP.a`

---

## 5. 위험 및 완화

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| Unknown caller in unscanned code | 낮음 | Very| 낮음 | Full grep of entire repo confirms zero callers |
| Future need for double-to-string | 낮음 || 낮음 | `sprintf` with `"%.0f"` is a one-liner if ever needed |

---

## 8. 다음 단계

1. [x] Write design document
2. [x] Delete the file
3. [x] Verify build and grep

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
