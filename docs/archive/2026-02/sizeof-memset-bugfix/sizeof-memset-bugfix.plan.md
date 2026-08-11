# sizeof-memset-bugfix Planning Document

> **Summary**: Fix critical sizeof() pointer arithmetic bug in pb_1200_tr.c DecryptBody()
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **저자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

Fix a critical memset bug in `pb_1200_tr.c:520` where `sizeof(DataBuff-82)` evaluates to `sizeof(char*)` (8 bytes on 64-bit) instead of the intended buffer size of 4014 bytes. This means only 8 bytes are cleared before decrypted KRX data is copied in, leaving stale data in the remaining 4006 bytes.

### 1.2 배경

The `DecryptBody()` function handles decryption of KRX bond response messages. After decrypting, it clears the body portion of `DataBuff[4096]` starting at offset 82 (KRX header length), then copies the decrypted data. The `sizeof(DataBuff-82)` expression performs pointer arithmetic inside `sizeof()`, which yields the size of a pointer (8) rather than the intended `4096 - 82 = 4014`.

Additionally, line 522 uses a hardcoded `82` instead of the defined constant `KRX_HEAD_LEN`, which is inconsistent with line 520 that correctly uses `KRX_HEAD_LEN` for the offset.

### 1.3 관련 문서

- Header: `inc/pa_struct.h:11` — `#define KRX_HEAD_LEN (sizeof(KRX_HEADER))` (82 bytes)
- CLAUDE.md: KRX Message Structure section

---

## 2. 범위

### 2.1 범위 내

- [x] Fix `sizeof(DataBuff-82)` → `sizeof(DataBuff) - KRX_HEAD_LEN` on line 520
- [x] Replace hardcoded `82` with `KRX_HEAD_LEN` on line 522

### 2.2 범위 외

- Other files — codebase scan confirmed this is the only sizeof misuse
- Functional behavior changes beyond correcting the memset size
- Refactoring DecryptBody() or surrounding code

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | Fix `sizeof(DataBuff-82)` to `sizeof(DataBuff) - KRX_HEAD_LEN` in `pb_1200_tr.c:520` | Critical | Pending |
| FR-02 | Replace hardcoded `82` with `KRX_HEAD_LEN` in `pb_1200_tr.c:522` | Medium | Pending |

### 3.2 Detailed 분석

**FR-01 — sizeof bug (Critical)**

```
File: st01/src/PB/pb_1200_tr.c:520
Function: DecryptBody()

Before:
  memset (&DataBuff[KRX_HEAD_LEN], 0, sizeof(DataBuff-82));
  // sizeof(DataBuff-82) = sizeof(char*) = 8 bytes (64-bit)
  // Only clears 8 of 4014 bytes — stale data remains

After:
  memset (&DataBuff[KRX_HEAD_LEN], 0, sizeof(DataBuff) - KRX_HEAD_LEN);
  // sizeof(DataBuff) - KRX_HEAD_LEN = 4096 - 82 = 4014 bytes
  // Correctly clears entire body area
```

**FR-02 — Hardcoded constant (Medium)**

```
File: st01/src/PB/pb_1200_tr.c:522

Before:
  RecvLen = 82 + dec_len;

After:
  RecvLen = KRX_HEAD_LEN + dec_len;
```

### 3.3 비기능 요구사항

| 범주 | 기준 | 검증 |
|----------|----------|-------------|
| Correctness | memset clears exactly 4014 bytes (4096-82) | Code review + sizeof arithmetic verification |
| Consistency | No hardcoded `82` where `KRX_HEAD_LEN` is available | Grep verification |
| Safety | Zero change to program flow or logic | Only memset size and constant substitution |

---

## 4. 성공 기준

### 4.1 완료 정의

- [x] FR-01: sizeof expression fixed
- [x] FR-02: Hardcoded 82 replaced with KRX_HEAD_LEN
- [x] Gap analysis confirms 100% match rate
- [x] No other sizeof misuse in codebase (pre-verified by scan)

### 4.2 영향 평가

- **Before fix**: memset clears 8 bytes → 4006 bytes of stale data may persist in DataBuff body area after decryption
- **After fix**: memset clears 4014 bytes → entire body area properly zeroed before decrypted data copy
- **위험**: The subsequent `memcpy` on line 521 overwrites `dec_len` bytes, so the stale data only matters if `dec_len < 4014` (which is common for short messages). This fix prevents information leakage from previous messages.

---

## 5. 위험 및 완화

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| Larger memset affects performance | Low | Low | 4014 bytes vs 8 bytes — negligible for per-message operation |
| Fix changes behavior for short messages | 중간 || 중간 | Intentional — stale data in buffer body was the bug |
| Wrong constant used | High | Low | KRX_HEAD_LEN = sizeof(KRX_HEADER) = 82, matches original intent |

---

## 6. 파일 요약

| 파일 | 변경된 라인 | FR 항목 |
|------|:------------:|:--------:|
| `st01/src/PB/pb_1200_tr.c` | 2 | FR-01, FR-02 |
| **Total** | **2** | **2** |

---

## 7. 다음 단계

1. [x] Write design document (`sizeof-memset-bugfix.design.md`)
2. [x] Implement fix (2 line changes)
3. [x] Gap analysis

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft | Claude Code |
