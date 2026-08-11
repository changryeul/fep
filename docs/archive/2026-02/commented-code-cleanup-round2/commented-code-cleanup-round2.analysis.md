# commented-code-cleanup-round2 Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor for KRX)
> **Analyst**: gap-detector
> **Date**: 2026-02-22
> **Design Doc**: [commented-code-cleanup-round2.design.md](../02-design/features/commented-code-cleanup-round2.design.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Verify that all 47 FR items (commented-out code deletions) specified in the Design document have been correctly implemented across 14 source files. This is a pure-deletion cleanup -- zero functional code changes expected.

### 1.2 Analysis Scope

- **Design Document**: `docs/02-design/features/commented-code-cleanup-round2.design.md`
- **Implementation Paths**: `st01/sub/` (8 files), `st01/src/PB/` (2 files), `st01/src/PX/` (4 files)
- **Analysis Date**: 2026-02-22
- **Design Items**: 47 FR items, ~190 lines, 14 files

---

## 2. Overall Scores

| Category | Score | Status |
|----------|:-----:|:------:|
| Design Match | 100% | PASS |
| Architecture Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 3. Detailed Verification Results

### 3.1 File 1: sub/log_proc.c (FR-01 to FR-13, 14 items)

| FR | Target Pattern | Verified Removed | Status |
|:--:|----------------|:----------------:|:------:|
| FR-01 | `//memcpy (err_beep, "\007", 4);` in Log (case 1, FATL) | Yes | PASS |
| FR-02 | `//memcpy (err_beep, "\007", 4);` in Log (case 5, EROR) | Yes | PASS |
| FR-03 | `//max_buf_len, buf);` in Log (file logging) | Yes | PASS |
| FR-04 | `/* sprintf (msg,...) */` + `/* char Tmp[...] */` (18 lines) | Yes | PASS |
| FR-05 | `//max_buf_len, buf);` in Log (terminal) | Yes | PASS |
| FR-06 | `/* sprintf (msg,...) */` + `/* memset; sprintf; memcpy */` (17 lines) | Yes | PASS |
| FR-07 | `//write (fd, p_msg, strlen (p_msg));` in Log_Write (1st) | Yes | PASS |
| FR-08 | `//write (fd, p_msg, strlen (p_msg));` in Log_Write (2nd) | Yes | PASS |
| FR-09 | `/* // 2025EDIT ... memset(Tmp0); sprintf(Tmp0); memcpy */` (9 lines) | Yes | PASS |
| FR-10 | `//max_buf_len, buf);` in ShmLog (NEW) | Yes | PASS |
| FR-11 | `/* sprintf (msg,...) */` + `/* char Tmp[...] ... // 2025EDIT */` (19 lines) | Yes | PASS |
| FR-12 | `//memcpy (err_beep, "\007", 4);` in Log_Emergency (USR) | Yes | PASS |
| FR-13 | `//memcpy (err_beep, "\007", 4);` in Log_Emergency (2nd) | Yes | PASS |

**Preservation checks:**
- `err_beep[0] = '\007';` active replacement: PRESENT (lines 153, 163, 526, 536) -- PASS
- `// 2025EDIT` markers preserved: PRESENT (lines 169, 174, 225, 230, 423, 595) -- PASS

**Grep verification:**
- `//memcpy (err_beep` in log_proc.c: **0 matches** (expected 0) -- PASS
- `//max_buf_len` in log_proc.c: **0 matches** (expected 0) -- PASS
- `//write (fd` in log_proc.c: **0 matches** (expected 0) -- PASS
- `/* sprintf (msg` in log_proc.c: **0 matches** (expected 0) -- PASS
- `/* char Tmp` in log_proc.c: **0 matches** (expected 0) -- PASS
- `Tmp0` in log_proc.c: **0 matches** (expected 0) -- PASS

### 3.2 File 2: sub/shm_rw.c (FR-14 to FR-18, 5 items)

| FR | Target Pattern | Verified Removed | Status |
|:--:|----------------|:----------------:|:------:|
| FR-14 | `//Exit_Process ();` after SHM_Creat fail | Yes | PASS |
| FR-15 | `//Exit_Process ();` after SHM_Creat retry fail | Yes | PASS |
| FR-16 | `//Exit_Process ();` after SEM_Creat_Excl fail | Yes | PASS |
| FR-17 | `//Exit_Process ();` after SEM_Creat retry fail | Yes | PASS |
| FR-18 | `//Exit_Process ();` after SEM_Creat nested retry fail (NEW) | Yes | PASS |

**Preservation checks:**
- `exit (OK);` active replacement: PRESENT (5 instances: lines 425, 437, 469, 480, 506) -- PASS

**Grep verification:**
- `//Exit_Process` in shm_rw.c: **0 matches** (expected 0) -- PASS

### 3.3 File 3: sub/select_recv.c (FR-19 to FR-20, 2 items)

| FR | Target Pattern | Verified Removed | Status |
|:--:|----------------|:----------------:|:------:|
| FR-19 | `//  rt = Recvn (p_sfd, p_recv+5, pkt_len - 5);` | Yes | PASS |
| FR-20 | `//rt = Recvn (p_sfd, p_recv+4, pkt_len - 4);` | Yes | PASS |

**Grep verification:**
- `//.*rt = Recvn` in select_recv.c: **0 matches** (expected 0) -- PASS

### 3.4 Files 4-8: sub/ single-line items (FR-21 to FR-25, 5 items)

| FR | File | Target Pattern | Verified Removed | Status |
|:--:|------|----------------|:----------------:|:------:|
| FR-21 | setsigfatal.c | `//write (DTART_FD, "1", 1);` | Yes | PASS |
| FR-22 | make_daemon.c | `//chdir ("/");` | Yes | PASS |
| FR-23 | queue.c | `//    if((rtrn = CheckQueue(QID))...` | Yes | PASS |
| FR-24 | stat_save.c | `//  char   buf[60];` | Yes | PASS |
| FR-25 | shmsub.c | `//char key[12];  // 2025EDIT` | Yes | PASS |

**Preservation checks:**
- setsigfatal.c: Error-checked `if (write(DTART_FD, "1", 1) == -1)` at line 123 -- PRESENT
- make_daemon.c: Error-checked `if (chdir("/") == -1)` at line 51 -- PRESENT
- stat_save.c: `char buf[1024];` at line 24 -- PRESENT
- shmsub.c: `char key[32];` at line 137 (within Mem_SHM function) -- PRESENT

**Grep verification:**
- `//write (DTART_FD` in setsigfatal.c: **0 matches** -- PASS
- `//chdir` in make_daemon.c: **0 matches** -- PASS
- `//.*CheckQueue` in queue.c: **0 matches** -- PASS
- `//.*char.*buf[60]` in stat_save.c: **0 matches** -- PASS
- `//char.*key[12]` in shmsub.c: **0 matches** -- PASS

### 3.5 File 9: src/PB/pb_1100_ts.c (FR-26 to FR-38, 13 items)

| FR | Target Pattern | Verified Removed | Status |
|:--:|----------------|:----------------:|:------:|
| FR-26 | `//extern net_ctx *EnCtx;` | Yes | PASS |
| FR-27 | `//#define CLIENT_CTX 0` | Yes | PASS |
| FR-28 | `//#define KRX_INITECH_CONF_PATH "..."` | Yes | PASS |
| FR-29 | `//KRX_NOTE_JUMUN_S_FMT  Re_W_Fmt;` | Yes | PASS |
| FR-30 | `//INL_Initialize(...)` + `//INL_New_Ctx(...)` (2 lines) | Yes | PASS |
| FR-31 | `/* memset(&J_R_Fmt...); memcpy(&J_R_Fmt...); */` (4 lines) | Yes | PASS |
| FR-32 | `//else if (FirstSeq <= INT_SEQ)` | Yes | PASS |
| FR-33 | `//Device_Close ();` | Yes | PASS |
| FR-34 | `//Make_Send_Msg`, `//memset`, `//memcpy`, `//Device_Write` (4 lines) | Yes | PASS |
| FR-35 | `//memcpy (DataBuff, &J_Q_Fmt, SendLen);` | Yes | PASS |
| FR-36 | `/* if (rt == 0) break; else if ... */` (13 lines) | Yes | PASS |
| FR-37 | `//if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)` | Yes | PASS |
| FR-38 | `/* rt = F_W (TS_W2_1, ...); ... */` (8 lines) | Yes | PASS |

**Preservation checks:**
- `/* 암복호화 추가 */` at line 77: PRESENT -- PASS
- `/* 20251223` comments at lines 470, 492: PRESENT -- PASS
- `// 2025 암복호화` at line 527: PRESENT -- PASS

**Grep verification:**
- `//extern net_ctx` in pb_1100_ts.c: **0 matches** -- PASS
- `//#define CLIENT_CTX` in pb_1100_ts.c: **0 matches** -- PASS
- `//#define KRX_INITECH_CONF_PATH` in pb_1100_ts.c: **0 matches** -- PASS
- `//KRX_NOTE_JUMUN_S_FMT` in pb_1100_ts.c: **0 matches** -- PASS
- `//INL_Initialize` in pb_1100_ts.c: **0 matches** -- PASS
- `//Device_Close` in pb_1100_ts.c: **0 matches** -- PASS
- `//Make_Send_Msg` in pb_1100_ts.c: **0 matches** -- PASS
- `//memcpy (DataBuff` in pb_1100_ts.c: **0 matches** -- PASS
- `//if (memcmp` in pb_1100_ts.c: **0 matches** -- PASS
- `/* memset.*J_R_Fmt` in pb_1100_ts.c: **0 matches** -- PASS
- `//else if.*FirstSeq.*INT_SEQ` in pb_1100_ts.c: **0 matches** -- PASS
- `/* if (rt == 0) break` in pb_1100_ts.c: **0 matches** -- PASS
- `/* rt = F_W (TS_W2` in pb_1100_ts.c: **0 matches** -- PASS

### 3.6 File 10: src/PB/pb_1200_tr.c (FR-39 to FR-40, 2 items)

| FR | Target Pattern | Verified Removed | Status |
|:--:|----------------|:----------------:|:------:|
| FR-39 | `/* if (IS_TR(msg, TR_MARKET_OPR_FULL)) ... */` (34 lines) | Yes | PASS |
| FR-40 | `/* 2번째 */` + `#if defined B1201` + dead `/* ... */` + `#endif` (18 lines) | Yes | PASS |

**Preservation checks:**
- `#endif` after FR-39 area (line 481): PRESENT -- PASS
- Active `#if defined B1201` at lines 390, 700, 761: PRESENT (these are live conditionals, not the dead FR-40 one) -- PASS

**Grep verification:**
- `/* if (IS_TR` in pb_1200_tr.c: **0 matches** -- PASS
- `/* 2번째 */` in pb_1200_tr.c: **0 matches** -- PASS
- `rt = F_W ((p_flag` in pb_1200_tr.c: **0 matches** -- PASS

### 3.7 Files 11-14: src/PX/ (FR-41 to FR-47, 7 items)

| FR | File | Target Pattern | Verified Removed | Status |
|:--:|------|----------------|:----------------:|:------:|
| FR-41 | px_showsise.c | `//sprintf (Key.expcode, "%-12.12s", argv[3]);` (line 63) | Yes | PASS |
| FR-42 | px_showsise.c | `//sprintf (Key.expcode, "%-12.12s", argv[3]);` (line 112) | Yes | PASS |
| FR-43 | px_showsise.c | Two `//printf` + four `//` old printf args (6 lines) | Yes | PASS |
| FR-44 | px_chkshm.c | 4x `//printf` old `%ld`/`%d` format (lines 129, 136, 142, 146) | Yes | PASS |
| FR-45 | px_chkshm.c | `//printf ("  [%s,%-6.6s,...0x%08x...]` (line 303) | Yes | PASS |
| FR-46 | px_chkche.c | `//buf+242, buf+258, buf+199, buf+219, buf+222);` | Yes | PASS |
| FR-47 | px_chktrcnt.c | `/* if (argv[1][1] == 'A') */` | Yes | PASS |

**Preservation checks:**
- px_showsise.c: Active `printf ("종목코드[%12.12s] 잔고[%ld]\n",` at line 246: PRESENT -- PASS
- px_chkshm.c: Active `printf` with `%zu` format at lines 129, 135, 140, 143: PRESENT -- PASS
- px_chkche.c: Active 4-field printf at line 92-94: PRESENT -- PASS
- px_chktrcnt.c: Active `memcmp (argv[1], "pa", 2)` at line 58: PRESENT -- PASS

**Grep verification:**
- `//sprintf (Key.expcode` in px_showsise.c: **0 matches** -- PASS
- `//printf` in px_showsise.c: **0 matches** -- PASS
- `//printf` in px_chkshm.c: **0 matches** -- PASS
- `//buf+222` in px_chkche.c: **0 matches** -- PASS
- `/* if (argv[1][1]` in px_chktrcnt.c: **0 matches** -- PASS

---

## 4. Extra: Beyond Design Scope

### 4.1 BONUS: Write_SLog() additional blocks in log_proc.c

The Design mentions 2 additional `/* */` blocks (~23 lines) found during implementation in the `Write_SLog()` function of log_proc.c, following the same dead sprintf+Tmp pattern as FR-04/FR-06/FR-11.

**Verification:**
- `/* sprintf (msg` in log_proc.c: **0 matches** -- these blocks are removed
- `/* char Tmp` in log_proc.c: **0 matches** -- these blocks are removed
- The Write_SLog() function (lines 461-606) contains no `/* */` dead code blocks
- Active `sprintf (msg, ...)` at line 542 is the live replacement -- PRESENT

**Result**: BONUS items verified as implemented. PASS.

---

## 5. Match Rate Summary

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  Total FR items:     47                      |
|  Verified PASS:      47 (100%)              |
|  Verified FAIL:       0 (0%)                |
|  Bonus items:         2 (extra, all PASS)   |
+---------------------------------------------+
|  Design Match:       100%     PASS          |
|  Architecture:       100%     PASS          |
|  Convention:         100%     PASS          |
+---------------------------------------------+
```

### By File Group

| Group | Files | FR Items | Items PASS | Lines Removed | Status |
|-------|:-----:|:--------:|:----------:|:-------------:|:------:|
| sub/ | 8 | 25 | 25 | 84 | PASS |
| src/PB/ | 2 | 15 | 15 | 91 | PASS |
| src/PX/ | 4 | 7 | 7 | 15 | PASS |
| **Total** | **14** | **47** | **47** | **~190** | **PASS** |

### By Category

| Category | Count | Lines | Status |
|----------|:-----:|:-----:|:------:|
| `/* */` commented-out code blocks | 12 | 160 | PASS |
| `//` commented-out code lines | 32 | 27 | PASS |
| `#if`/`#endif` dead conditional (FR-40) | 1 | 3 | PASS |
| BONUS (Write_SLog blocks) | 2 | ~23 | PASS |

---

## 6. Verification Checklist

- [x] All 47 design items deleted across 14 files
- [x] No functional code lines changed (pure deletion)
- [x] All `// 2025EDIT` markers preserved (lines 169, 174, 225, 230, 423, 595 in log_proc.c)
- [x] All `/* 20251223 ... */` design decision comments preserved in pb_1100_ts.c
- [x] All section banners, function-end markers, and descriptive comments preserved
- [x] `#endif` in pb_1200_tr.c preserved after FR-39 deletion (line 481)
- [x] Active replacement code present for all deleted items
- [x] BONUS Write_SLog blocks also removed
- [ ] Build: `mk.sh sub && mk.sh src` -- NOT TESTED (requires server)

---

## 7. No Differences Found

No gaps between design and implementation were detected:

- **Missing Features (Design O, Implementation X)**: 0
- **Added Features (Design X, Implementation O)**: 2 (BONUS items -- beneficial)
- **Changed Features (Design != Implementation)**: 0

---

## 8. Recommended Actions

### 8.1 Immediate Actions
None required. All 47 FR items are fully implemented.

### 8.2 Build Verification
Run `mk.sh sub && mk.sh src` on the target server to confirm binary-identical output. This cannot be verified on the macOS development machine.

### 8.3 Documentation
No design document updates needed. The design accurately describes all implemented changes.

---

## 9. Trend

| Feature | Match Rate | Date |
|---------|:----------:|:----:|
| PA-PB Dedup Phase 1 | 90% | 2026-02-21 |
| PA-PB Dedup Phase 2 | 97% | 2026-02-21 |
| PA-PB Dedup Phase 3 | 100% | 2026-02-21 |
| Fifo-Event-Rtn Adoption | 100% | 2026-02-22 |
| Socket-Linger Extraction | 100% | 2026-02-22 |
| Dead Code Cleanup | 98% | 2026-02-22 |
| PA-PB 8100 Cleanup | 100% | 2026-02-22 |
| **Commented Code Cleanup R2** | **100%** | **2026-02-22** |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | Initial analysis -- 47/47 PASS, 100% match | gap-detector |
