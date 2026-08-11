# select-recv-bounds-check 분석 Report

> **분석 Type**: Gap 분석 (설계 vs Implementation)
>
> **프로젝트**: FEP (Front-End Processor)
> **Analyst**: Claude
> **날짜**: 2026-02-25
> **설계 Doc**: [select-recv-bounds-check.design.md](../02-design/features/select-recv-bounds-check.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

Verify that all 11 functional requirements (FR-01 through FR-11) specified in the design document are correctly implemented in `select_recv.c`. Focus areas: exact condition expressions, `return (NOTOK)` presence, `{ }` block formatting, Log message function names, Sise_Select_Receive untouched, and function signatures unchanged.

### 1.2 분석 범위

- **설계 Document**: `docs/02-design/features/select-recv-bounds-check.design.md`
- **Implementation 파일**: `st01/sub/select_recv.c`
- **분석 날짜**: 2026-02-25

---

## 2. FR-by-FR Comparison

### 2.1 Select_Receive (FR-01, FR-02)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| Lower bound condition | `pkt_len < TCP_HEAD_LEN - 5` | `pkt_len < TCP_HEAD_LEN - 5` (L69) | 일치 |
| Upper bound condition | `pkt_len > TCP_BUFF_MAX_LEN - 5` | `pkt_len > TCP_BUFF_MAX_LEN - 5` (L69) | 일치 |
| Combined with `\|\|` | `if (lower \|\| upper)` | `if (pkt_len < TCP_HEAD_LEN - 5 \|\| pkt_len > TCP_BUFF_MAX_LEN - 5)` (L69) | 일치 |
| `{ }` block | Required | Present (L70, L73) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L72) | 일치 |
| Log message | `"Select_Receive:invalid Length[%d]"` | `"Select_Receive:invalid Length[%d]"` (L71) | 일치 |
| Function signature | `int Select_Receive (int p_sfd, char *p_recv)` | `int Select_Receive (int p_sfd, char *p_recv)` (L20) | 일치 |

**FR-01 (upper bound added)**: Match
**FR-02 (return NOTOK + brace block)**: Match

---

### 2.2 Select_Receive2 (FR-03, FR-04)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| Lower bound condition | `pkt_len <= p_len` | `pkt_len <= p_len` (L145) | 일치 |
| Upper bound condition | `pkt_len > TCP_BUFF_MAX_LEN` | `pkt_len > TCP_BUFF_MAX_LEN` (L145) | 일치 |
| Combined with `\|\|` | `if (lower \|\| upper)` | `if (pkt_len <= p_len \|\| pkt_len > TCP_BUFF_MAX_LEN)` (L145) | 일치 |
| `{ }` block | Required | Present (L146, L149) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L148) | 일치 |
| Log message | `"Select_Receive2:invalid Length[%d]"` | `"Select_Receive2:invalid Length[%d]"` (L147) | 일치 |
| Function signature | `int Select_Receive2 (int p_sfd, char *p_recv, int p_len)` | `int Select_Receive2 (int p_sfd, char *p_recv, int p_len)` (L96) | 일치 |

**FR-03 (new guard block inserted)**: Match
**FR-04 (return NOTOK + brace block)**: Match

---

### 2.3 Select_Receive_Krx (FR-05)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| Existing `pkt_len > 0` guard | Kept as-is | `if (pkt_len > 0)` (L221) | 일치 |
| Upper bound condition (nested) | `pkt_len > KRX_DATA_BUFF_SIZE - p_len` | `pkt_len > KRX_DATA_BUFF_SIZE - p_len` (L223) | 일치 |
| `{ }` block | Required | Present (L224, L227) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L226) | 일치 |
| Log message | `"Select_Receive_Krx:invalid Length[%d]"` | `"Select_Receive_Krx:invalid Length[%d]"` (L225) | 일치 |
| Function signature | `int Select_Receive_Krx (int p_sfd, char *p_recv, int p_len)` | `int Select_Receive_Krx (int p_sfd, char *p_recv, int p_len)` (L172) | 일치 |

**FR-05 (upper bound added inside existing pkt_len > 0 block)**: Match

---

### 2.4 Select_Receive_Cli (FR-06, FR-07)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| Lower bound condition | `pkt_len < CLI_HEAD_LEN - 4` | `pkt_len < CLI_HEAD_LEN - 4` (L297) | 일치 |
| Upper bound condition | `pkt_len > CLI_BUFF_MAX_LEN - 4` | `pkt_len > CLI_BUFF_MAX_LEN - 4` (L297) | 일치 |
| Combined with `\|\|` | `if (lower \|\| upper)` | `if (pkt_len < CLI_HEAD_LEN - 4 \|\| pkt_len > CLI_BUFF_MAX_LEN - 4)` (L297) | 일치 |
| `{ }` block | Required | Present (L298, L301) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L300) | 일치 |
| Log message | `"Select_Receive_Cli:invalid Length[%d]"` | `"Select_Receive_Cli:invalid Length[%d]"` (L299) | 일치 |
| Function signature | `int Select_Receive_Cli (int p_sfd, char *p_recv)` | `int Select_Receive_Cli (int p_sfd, char *p_recv)` (L250) | 일치 |

**FR-06 (upper bound added)**: Match
**FR-07 (return NOTOK + brace block + Log function name corrected)**: Match

---

### 2.5 Select_Receive_Imeco (FR-08, FR-09)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| pkt_len computation | `AtoIf (p_recv, 4) + 16` | `AtoIf (p_recv, 4) + 16` (L369) | 일치 |
| Lower bound condition | `pkt_len < 20 - 4` | `pkt_len < 20 - 4` (L370) | 일치 |
| Upper bound condition | `pkt_len > TCP_BUFF_MAX_LEN - 4` | `pkt_len > TCP_BUFF_MAX_LEN - 4` (L370) | 일치 |
| Combined with `\|\|` | `if (lower \|\| upper)` | `if (pkt_len < 20 - 4 \|\| pkt_len > TCP_BUFF_MAX_LEN - 4)` (L370) | 일치 |
| `{ }` block | Required | Present (L371, L374) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L373) | 일치 |
| Log message | `"Select_Receive_Imeco:invalid Length[%d]"` | `"Select_Receive_Imeco:invalid Length[%d]"` (L372) | 일치 |
| Function signature | `int Select_Receive_Imeco (int p_sfd, char *p_recv)` | `int Select_Receive_Imeco (int p_sfd, char *p_recv)` (L323) | 일치 |

**FR-08 (upper bound added)**: Match
**FR-09 (return NOTOK + brace block + Log function name corrected)**: Match

---

### 2.6 Select_Receive_Imeco_Sise (FR-10, FR-11)

| Check| 항목 | Design Spec | Implementation (line) | 상태 |
|------------|-------------|----------------------|--------|
| pkt_len computation | `AtoIf (p_recv, 10) + 40` (no inline comment) | `AtoIf (p_recv, 10) + 40;` (L442, no comment) | 일치 |
| Lower bound condition | `pkt_len < 50 - 10` | `pkt_len < 50 - 10` (L443) | 일치 |
| Upper bound condition | `pkt_len > TCP_BUFF_MAX_LEN - 10` | `pkt_len > TCP_BUFF_MAX_LEN - 10` (L443) | 일치 |
| Combined with `\|\|` | `if (lower \|\| upper)` | `if (pkt_len < 50 - 10 \|\| pkt_len > TCP_BUFF_MAX_LEN - 10)` (L443) | 일치 |
| `{ }` block | Required | Present (L444, L447) | 일치 |
| `return (NOTOK)` | Required after Log | Present (L446) | 일치 |
| Log message | `"Select_Receive_Imeco_Sise:invalid Length[%d]"` | `"Select_Receive_Imeco_Sise:invalid Length[%d]"` (L445) | 일치 |
| C89 comment removal | `// header 50` inline comment removed | No inline comment on L442 | 일치 |
| Function signature | `int Select_Receive_Imeco_Sise (int p_sfd, char *p_recv)` | `int Select_Receive_Imeco_Sise (int p_sfd, char *p_recv)` (L396) | 일치 |

**FR-10 (upper bound added + C89 comment cleanup)**: Match
**FR-11 (return NOTOK + brace block + Log function name corrected)**: Match

---

## 3. Cross-Cutting Checks

### 3.1 Sise_Select_Receive NOT Modified

| Check| 항목 | Design Spec | Implementation | 상태 |
|------------|-------------|----------------|--------|
| No bounds-check changes | Out of scope (already safe via `SZ_FEEDDATA_MAX - sockpos` cap) | Lines 469-532: uses `recv()` with `SZ_FEEDDATA_MAX - sockpos` cap. No pkt_len guard added. | 일치 |

### 3.2 Function Signatures Unchanged

| Function | Design Signature | Implementation Signature | 상태 |
|----------|-----------------|-------------------------|--------|
| `Select_Receive` | `(int p_sfd, char *p_recv)` | `(int p_sfd, char *p_recv)` L20 | 일치 |
| `Select_Receive2` | `(int p_sfd, char *p_recv, int p_len)` | `(int p_sfd, char *p_recv, int p_len)` L96 | 일치 |
| `Select_Receive_Krx` | `(int p_sfd, char *p_recv, int p_len)` | `(int p_sfd, char *p_recv, int p_len)` L172 | 일치 |
| `Select_Receive_Cli` | `(int p_sfd, char *p_recv)` | `(int p_sfd, char *p_recv)` L250 | 일치 |
| `Select_Receive_Imeco` | `(int p_sfd, char *p_recv)` | `(int p_sfd, char *p_recv)` L323 | 일치 |
| `Select_Receive_Imeco_Sise` | `(int p_sfd, char *p_recv)` | `(int p_sfd, char *p_recv)` L396 | 일치 |
| `Sise_Select_Receive` | Not modified | `(int p_sfd, char *p_recv)` L469 (unchanged) | 일치 |

### 3.3 All Guards Have `return (NOTOK)` (Not Log-Only)

| Function | Log present | return (NOTOK) present | 상태 |
|----------|:-----------:|:---------------------:|--------|
| Select_Receive | L71 | L72 | 일치 |
| Select_Receive2 | L147 | L148 | 일치 |
| Select_Receive_Krx | L225 | L226 | 일치 |
| Select_Receive_Cli | L299 | L300 | 일치 |
| Select_Receive_Imeco | L372 | L373 | 일치 |
| Select_Receive_Imeco_Sise | L445 | L446 | 일치 |

### 3.4 All Guards Use `{ }` Block Formatting

| Function | Opening `{` | Closing `}` | 상태 |
|----------|:-----------:|:-----------:|--------|
| Select_Receive | L70 | L73 | 일치 |
| Select_Receive2 | L146 | L149 | 일치 |
| Select_Receive_Krx | L224 | L227 | 일치 |
| Select_Receive_Cli | L298 | L301 | 일치 |
| Select_Receive_Imeco | L371 | L374 | 일치 |
| Select_Receive_Imeco_Sise | L444 | L447 | 일치 |

---

## 4. 일치 Rate 요약

```
+-----------------------------------------------+
|  Overall Match Rate: 100%                      |
+-----------------------------------------------+
|  Total FR items:       11                      |
|  Matched:              11  (100%)              |
|  Gaps:                  0  (0%)                |
|  Deviations:            0  (0%)                |
+-----------------------------------------------+
```

| 범주 | Score | 상태 |
|----------|:-----:|:------:|
| Design Match (FR compliance) | 100% | PASS |
| Guard Completeness (return NOTOK) | 100% | PASS |
| Block Formatting (`{ }`) | 100% | PASS |
| Log Message Accuracy | 100% | PASS |
| Signature Preservation | 100% | PASS |
| Out-of-Scope Untouched | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 5. 검증 Checklist (from 설계 Section 5)

- [x] 6 functions all have upper+lower bound guard before Recvn call
- [x] All guards perform `return (NOTOK)` (not log-only)
- [x] No function signature changes
- [x] `Sise_Select_Receive` is NOT modified
- [x] Log messages use the correct function name matching the actual function
- [ ] `mk.sh sub` build success (not verified in this analysis -- requires build environment)

---

## 6. Differences Found

### Missing Features (설계 O, Implementation X)

None.

### Added Features (설계 X, Implementation O)

None.

### Changed Features (설계 != Implementation)

None.

---

## 7. Recommended Actions

No corrective actions needed. The implementation matches the design document with 100% fidelity across all 11 FR items.

### Remaining 검증

1. **Build verification**: Run `mk.sh sub` to confirm compilation success with the bounds-check changes.
2. **Integration test**: Verify that callers (pa_1600_tr, pw_3010_tr, pa_3100_ts, pa_8100_ts, pa_2100_ts, pa_2700_tr, etc.) handle the new `NOTOK` returns correctly.

---

## 8. 다음 단계

- [x] Gap analysis complete
- [ ] Build verification (`mk.sh sub`)
- [ ] Write completion report (`select-recv-bounds-check.report.md`)

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-25 | Initial analysis -- 100% match | Claude |
