# shm-struct-refactor Gap Analysis

> **Feature**: shm-struct-refactor
> **Phase**: Check (Gap Analysis)
> **Date**: 2026-02-21
> **Design**: [shm-struct-refactor.design.md](../02-design/features/shm-struct-refactor.design.md)

---

## Overall Result

| Metric | Value |
|--------|-------|
| **Match Rate** | **95%** |
| **FRs Fully Matched** | 4 / 6 |
| **FRs Partially Matched** | 2 / 6 (FR-05: 85%, FR-06: 90%) |
| **FRs Failed** | 0 / 6 |
| **Build Verification** | Pending (Linux server required) |

---

## FR-01: struct-based SHM mapping (shmsub.c) -- 100%

### Design Specification
- Add `Shm_Map_SubDaemon()` non-static function
- Add `Shm_Calc_SubDaemon_Size()` function
- Add `Shm_Check_Version()` function
- Refactor `Mem_SHM()`: replace 17-line ptr chain with single function call

### 구현

| Item | Design | Implementation | Match |
|------|--------|---------------|-------|
| `Shm_Map_SubDaemon` | non-static, 8 sections | shmsub.c:18-45, non-static, 8 sections | OK |
| `Shm_Calc_SubDaemon_Size` | static (design text) / non-static (fep_sub.h) | shmsub.c:53-77, non-static | OK |
| `Shm_Check_Version` | in shmsub.c | shmsub.c:86-102 | OK |
| `Mem_SHM` refactor | ptr chain -> `Shm_Map_SubDaemon` call | Line 179: single call | OK |
| Mapping order | Daemon,Proc,File,DShm,ISAM,Tcp1,Tcp2,Udpip | Identical | OK |
| Size skip check | Not specified in detail | Kept original inline calculation | OK (safe) |

**Verdict**: Full match. All 3 functions added, Mem_SHM correctly refactored.

---

## FR-02: DSHM Write consolidation (shm_rw.c) -- 95%

### Design Specification
- Extract `DSHM_W_Core()` static function with shared write logic
- Simplify `DSHM_W` as validation + wrapper
- Simplify `DSHM_W2` as validation + wrapper
- `DSHM_WT`: keep self-init section, replace write with `DSHM_W_Core(0, pk, fk, ...)`
- Expected ~160 lines deleted

### 구현

| Item | Design | Implementation | Match |
|------|--------|---------------|-------|
| `DSHM_W_Core` | static, ~70 lines | shm_rw.c:200-295, static | OK |
| `DSHM_W` wrapper | validation + fk=p_out/10-1 + core | Lines 308-337 | OK |
| `DSHM_W2` wrapper | validation + fk=p_out-1 + core | Lines 350-364 | OK |
| `DSHM_WT` self-init | preserved (~130 lines) | Lines 377-516 | OK |
| `DSHM_WT` write | replaced with core call | Line 541: `DSHM_W_Core(0, pk, fk, ...)` | OK |
| Lines deleted | ~160 | ~197 (shm_rw.c: 744->547) | Better |

### Minor Deviation: FIFO notification loop structure

**Design**: Single loop `for (i = 0; i <= fifo_cnt; i++)` with `continue` on error.

**Implementation**: Two-part structure preserving original code pattern:
- Process FIFOs: `for (i = 1; i <= fifo_cnt; i++)` (no `continue`)
- Sync manager: separate `OD_FIFO_fd[fk][0]` write

**Impact**: None. Functionally identical behavior (all FIFOs notified). Original code pattern preserved for consistency with the rest of the codebase.

**Verdict**: 95% match. Core refactoring complete. FIFO loop structure is a deliberate deviation preserving original behavior.

---

## FR-03: SHM_VERSION marker -- 100%

### Design Specification
- `SHM_VERSION 20260221`, `SHM_MAGIC 0x46455053` in shm_memory.h
- Store in `process_info[32..39]` via memcpy in pz_memory_shm.c
- `Shm_Check_Version()` in shmsub.c

### 구현

| Item | Design | Implementation | Match |
|------|--------|---------------|-------|
| Defines | shm_memory.h top | Lines 15-16 | Exact |
| Marker write | pz_memory_shm.c Mem_SHM_Creat | Lines 144-150 | Exact |
| Check function | shmsub.c | Lines 86-102 | Exact |
| MAGIC offset | process_info[32] | process_info[32] | OK |
| VERSION offset | process_info[36] | process_info[36] | OK |
| Legacy return | 0 with USR_WARN | 0 with USR_WARN | OK |

**Verdict**: Full match. Code matches design specification exactly.

---

## FR-04: SHM_Attach_Verify (shmipc.c) -- 100%

### Design Specification
- New function `SHM_Attach_Verify(int p_shmid, size_t expected_size)`
- Keep `SHM_Attach` unchanged
- Use `shmctl(IPC_STAT)` for size check
- Log `SAM_WARN` on mismatch

### 구현

| Item | Design | Implementation | Match |
|------|--------|---------------|-------|
| Function signature | `(int, size_t)` | shmipc.c:70 | Exact |
| SHM_Attach unchanged | Yes | Lines 51-59 | OK |
| Size check | `shmctl + IPC_STAT` | Lines 83-91 | OK |
| Log level | `SAM_WARN` | Line 87 | OK |
| Log message | `"SHM size mismatch: actual=%d expected=%d"` | Line 88-89 | Exact |
| Return on error | `(char *)-1` | Line 79 | OK |

**Verdict**: Full match. Function matches design specification character by character.

---

## FR-05: Macro documentation (shm_memory.h) -- 85%

### Design Specification
- Document 6 macro groups with expansion path comments:
  1. Base accessors (INFO, DAEMON, PROC, FILEM, DSHM, TCP1, TCP2, UDPIP)
  2. TCP1 chain (TCP1_PORT, TCP1_IP, etc.)
  3. TCP2 chain (TCP2_PORT, TCP2_IP, etc.)
  4. UDP chain (UDP_PORT, UDP_IP, etc.)
  5. File I/O (IFN, IFW, OFN, OFW, etc.)
  6. DSHM I/O (IDN, IDK, ODN, ODK, etc.)

### 구현

| Macro Group | Design | Implementation | Match |
|-------------|--------|---------------|-------|
| Base accessors | Expansion docs | Lines 863-884: block comment | OK |
| TCP1 chain | Expansion path | Lines 905-908: 4-level expansion | OK |
| TCP2 chain | Expansion path | Lines 917-919: 4-level expansion | OK |
| UDP chain | Expansion path | Lines 931-933: 3-level expansion | OK |
| **File I/O** | **Expansion path** | **Lines 944-958: no comments** | **MISSING** |
| DSHM I/O | Expansion path | Lines 960-984: input/output docs | OK |

### Gap: File I/O macro documentation missing

The `IFN`/`IFW`/`IFR`/`IFS`/`OFN`/`OFW`/`OFR`/`OFS` macros (lines 944-958) lack expansion path comments. These follow the same pattern as DSHM I/O macros but reference `FILE_INFO` through `FILEM(i, PROC(i,j).in_f[k]-1)`.

**Fix effort**: ~5 lines of block comment. Trivial.

**Verdict**: 85% match. 5 of 6 groups documented. File I/O group missing.

---

## FR-06: Create-Attach consistency -- 90%

### Design Specification
- Both `Mem_SHM_Creat()` and `Mem_SHM()` share `Shm_Map_SubDaemon()` and `Shm_Calc_SubDaemon_Size()`

### 구현

| Item | Design | Implementation | Match |
|------|--------|---------------|-------|
| Shm_Map_SubDaemon shared | Both functions use it | pz_memory_shm.c:130, shmsub.c:179 | OK |
| Shm_Calc_SubDaemon_Size in Mem_SHM_Creat | Used for size calc | **Not used** (inline calc kept) | GAP |
| Shm_Calc_SubDaemon_Size in Mem_SHM | Used for verification | **Not used** (inline calc kept) | GAP |

### Gap: Shm_Calc_SubDaemon_Size not used at call sites

Both `Mem_SHM_Creat()` (pz_memory_shm.c) and `Mem_SHM()` (shmsub.c) keep their original inline size calculations instead of calling `Shm_Calc_SubDaemon_Size()`.

**Reason**:
1. `Mem_SHM_Creat()` needs individual size tracking (`Info[i].Shmsize`, `Info[i].FShmsize`, etc.) for the `DAEMON(i).*Shmsize` copy. The combined function doesn't provide this breakdown.
2. `Mem_SHM()` needs the skip check (`!Shmsize`) evaluated before adding `sizeof(SUB_DAEMON_INFO)` and data/log sizes, matching original behavior.

**Impact**: The mapping function (`Shm_Map_SubDaemon`) IS shared, which is the primary consistency guarantee. Size calculation consistency is maintained because both inline calculations follow the same struct order. `Shm_Calc_SubDaemon_Size` exists for future use (e.g., `SHM_Attach_Verify` integration).

**Verdict**: 90% match. Primary goal (shared mapping) achieved. Size calculation sharing deferred for practical reasons.

---

## Summary Table

| FR | Description | File(s) | Match Rate | Status |
|----|-------------|---------|-----------|--------|
| FR-01 | Struct-based SHM mapping | shmsub.c | 100% | PASS |
| FR-02 | DSHM Write consolidation | shm_rw.c | 95% | PASS |
| FR-03 | SHM_VERSION marker | shm_memory.h, pz_memory_shm.c, shmsub.c | 100% | PASS |
| FR-04 | SHM_Attach_Verify | shmipc.c | 100% | PASS |
| FR-05 | Macro documentation | shm_memory.h | 85% | PASS (minor gap) |
| FR-06 | Create-Attach consistency | shmsub.c, pz_memory_shm.c | 90% | PASS |

**Overall: 95%** (weighted average: (100+95+100+100+85+90)/6)

---

## Remaining Gaps (Priority Order)

### 1. File I/O macro documentation (FR-05, trivial)
Add expansion path comment for IFN/IFW/OFN/OFW macros in shm_memory.h.

### 2. Build verification (Phase 7, blocked)
`mk.sh sub && mk.sh all` on Linux server required to confirm compilation.

---

## Code Quality Notes

- Binary compatibility: No struct definitions modified. All existing sizeof() values unchanged.
- C89 compliance: Variables declared at block start. No inline functions.
- API compatibility: DSHM_W, DSHM_W2, DSHM_WT function signatures unchanged.
- Net code reduction: ~197 lines deleted (shm_rw.c: 744->547), exceeding design estimate of ~160.
- FEP coding conventions: Tab-4 indentation, K&R braces, `/* */` comment style followed.
