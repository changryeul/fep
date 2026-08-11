# sizeof-memset-bugfix Design Document

> **Summary**: Fix critical sizeof() pointer arithmetic bug and hardcoded constant in pb_1200_tr.c DecryptBody()
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **저자**: Claude Code
> **날짜**: 2026-02-22
> **상태**: 초안
> **Plan Reference**: `docs/01-plan/features/sizeof-memset-bugfix.plan.md`

---

## 1. 설계 개요

### 1.1 범위

2 line changes in 1 file (`st01/src/PB/pb_1200_tr.c`), function `DecryptBody()`.

### 1.2 Context

```c
// File: st01/src/PB/pb_1200_tr.c
// Line 100: char DataBuff[4096], IpAddr[20], ApType[10];
// inc/pa_struct.h:11: #define KRX_HEAD_LEN (sizeof(KRX_HEADER))  /* 82 */

int DecryptBody(char *databuff, int body_len)  // line 493
{
    // ... decryption via INL_Decrypt ...

    // lines 520-522 (bug area):
    memset (&DataBuff[KRX_HEAD_LEN], 0, sizeof(DataBuff-82));
    memcpy (&DataBuff[KRX_HEAD_LEN], dec_data, dec_len);
    RecvLen = 82 + dec_len;
}
```

---

## 2. Detailed 변경사항

### FR-01: Fix sizeof() pointer arithmetic bug (Critical)

**파일**: `st01/src/PB/pb_1200_tr.c:520`

**Before**:
```c
	memset (&DataBuff[KRX_HEAD_LEN],	0,			sizeof(DataBuff-82));
```

**Problem**: `DataBuff` is `char[4096]`. In the expression `sizeof(DataBuff-82)`:
1. `DataBuff` decays to `char*`
2. `DataBuff - 82` is pointer arithmetic, result type is `char*`
3. `sizeof(char*)` = 8 on 64-bit systems
4. Only 8 bytes are cleared instead of the intended 4014 bytes (4096 - 82)

**After**:
```c
	memset (&DataBuff[KRX_HEAD_LEN],	0,			sizeof(DataBuff) - KRX_HEAD_LEN);
```

**Arithmetic verification**:
- `sizeof(DataBuff)` = 4096 (array size, no decay inside sizeof)
- `KRX_HEAD_LEN` = `sizeof(KRX_HEADER)` = 82
- 결과: 4096 - 82 = 4014 bytes cleared

### FR-02: Replace hardcoded constant (Medium)

**파일**: `st01/src/PB/pb_1200_tr.c:522`

**Before**:
```c
	RecvLen = 82 + dec_len;							// (Korean comment)
```

**After**:
```c
	RecvLen = KRX_HEAD_LEN + dec_len;							// (Korean comment)
```

**Rationale**: `KRX_HEAD_LEN` is already used on lines 520-521 for the same offset. Using the named constant ensures consistency and protects against future header size changes.

---

## 3. Implementation Order

Single batch — both changes are in adjacent lines (520, 522) of the same function.

| Order | FR | Line | Change |
|:-----:|:--:|:----:|--------|
| 1 | FR-01 | 520 | `sizeof(DataBuff-82)` → `sizeof(DataBuff) - KRX_HEAD_LEN` |
| 2 | FR-02 | 522 | `82` → `KRX_HEAD_LEN` |

---

## 4. 검증 기준

| ID | Check | Method |
|----|-------|--------|
| V-01 | `sizeof(DataBuff-82)` no longer appears in file | Grep |
| V-02 | `sizeof(DataBuff) - KRX_HEAD_LEN` appears on line 520 | Read |
| V-03 | `KRX_HEAD_LEN + dec_len` appears on line 522 | Read |
| V-04 | No hardcoded `82` in DecryptBody() except comments | Read |
| V-05 | Surrounding code (lines 519, 521, 523) unchanged | Read |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft | Claude Code |
