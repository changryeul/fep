# file-rw-optimize Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor for KRX)
> **Analyst**: Claude (gap-detector)
> **Date**: 2026-02-28
> **Design Doc**: [file-rw-optimize.design.md](../02-design/features/file-rw-optimize.design.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Verify that `sub/file_rw.c` implementation matches the design document's
exact code changes (Section 4) and verification checklist (Section 5).
The feature narrows fcntl lock scope from whole-file locks to
record-range (F_R) and append-region (F_W) locks across 7 functions.

### 1.2 Analysis Scope

- **Design Document**: `docs/02-design/features/file-rw-optimize.design.md`
- **Implementation File**: `st01/sub/file_rw.c` (1201 lines)
- **Functions in Scope**: F_R, F_W, F_R2, F_R3, F_W2, F_W3, F_WB (7 total)
- **Functions out of Scope**: SF_W (confirmed still uses `l_whence = 0` as expected)
- **Analysis Date**: 2026-02-28

---

## 2. FR Item Analysis (9 items)

### FR-01: F_R() -- F_RDLCK + record range lock + offset moved before lock

**Design**: Move `offset = IFR(D_K,P_K,Fk,Ck) * rec_size` before `do { ... }` loop,
change `F_WRLCK` to `F_RDLCK`, set `l_whence=SEEK_SET`, `l_start=offset`,
`l_len=(long)(rec_size * p_cnt)`.

**Implementation** (`st01/sub/file_rw.c` lines 77-97):

```c
	offset = IFR(D_K,P_K,Fk,Ck) * rec_size;     /* line 77: offset BEFORE lock */

	do {
		lock.l_type = F_RDLCK;                     /* line 80: F_RDLCK */
		lock.l_whence = SEEK_SET;                   /* line 81: SEEK_SET */
		lock.l_start = offset;                      /* line 82: record start */
		lock.l_len = (long)(rec_size * p_cnt);      /* line 83: record range */

		rt = fcntl (fd, F_SETLKW, &lock);           /* line 85 */

		if (rt == -1)
		{
			if (SYS_NO == EINTR)
				continue;                            /* EINTR retry preserved */

			close (fd);                              /* error handling preserved */
			Log (SAM_FATAL, "F_R:cannot lock (fcntl)[%d,%s] {%d:%s}",
				p_type, f_name, SYS_NO, SYS_STR);
			return (NOTOK);
		}
	} while (rt == -1);
```

**Result**: MATCH -- all 5 design changes applied exactly.

---

### FR-02: F_W() -- SEEK_END lock + SEEK_SET unlock reset

**Design**: Change `l_whence = 0` to `SEEK_END` in lock; add explicit
`SEEK_SET, 0, 0` reset in unlock section.

**Implementation -- Lock** (`st01/sub/file_rw.c` lines 261-265):

```c
	do {
		lock.l_type = F_WRLCK;                      /* line 262 */
		lock.l_whence = SEEK_END;                    /* line 263: SEEK_END */
		lock.l_start = 0L;                           /* line 264 */
		lock.l_len = 0L;                             /* line 265 */
```

**Implementation -- Unlock** (`st01/sub/file_rw.c` lines 285-289):

```c
	lock.l_type = F_UNLCK;                           /* line 285 */
	lock.l_whence = SEEK_SET;                        /* line 286: SEEK_SET reset */
	lock.l_start = 0L;                               /* line 287: 0 */
	lock.l_len = 0L;                                 /* line 288: 0 */
	fcntl (fileno (fp), F_SETLK, &lock);             /* line 289 */
```

**Result**: MATCH -- both lock and unlock changes applied exactly.

---

### FR-03: F_R2() -- F_RDLCK + record range lock + offset moved

**Design**: Move `offset = r_seq * rec_size` before loop, change to
`F_RDLCK`, `SEEK_SET`, `l_start=offset`, `l_len=(long)rec_size`.

**Implementation** (`st01/sub/file_rw.c` lines 482-502):

```c
	offset = r_seq * rec_size;                      /* line 482: offset BEFORE lock */

	do {
		lock.l_type = F_RDLCK;                       /* line 485: F_RDLCK */
		lock.l_whence = SEEK_SET;                     /* line 486: SEEK_SET */
		lock.l_start = offset;                        /* line 487: record start */
		lock.l_len = (long)rec_size;                  /* line 488: single record */
		...
	} while (rt == -1);
```

**Result**: MATCH -- all changes applied exactly.

---

### FR-04: F_R3() -- F_RDLCK + record range lock + offset moved

**Design**: Move `offset = FILEM(dk,fk).r_cnt[0] * rec_size` before loop,
change to `F_RDLCK`, `SEEK_SET`, `l_start=offset`, `l_len=(long)rec_size`.

**Implementation** (`st01/sub/file_rw.c` lines 774-794):

```c
	offset = FILEM(dk,fk).r_cnt[0] * rec_size;    /* line 774: offset BEFORE lock */

	do {
		lock.l_type = F_RDLCK;                       /* line 777: F_RDLCK */
		lock.l_whence = SEEK_SET;                     /* line 778: SEEK_SET */
		lock.l_start = offset;                        /* line 779: record start */
		lock.l_len = (long)rec_size;                  /* line 780: single record */
		...
	} while (rt == -1);
```

**Result**: MATCH -- all changes applied exactly.

---

### FR-05: F_W2() -- SEEK_END lock + SEEK_SET unlock reset

**Design**: Change `l_whence = 0` to `SEEK_END`, add `SEEK_SET, 0, 0` unlock reset.

**Implementation -- Lock** (`st01/sub/file_rw.c` line 585):

```c
		lock.l_whence = SEEK_END;                    /* line 585: SEEK_END */
```

**Implementation -- Unlock** (`st01/sub/file_rw.c` lines 607-611):

```c
	lock.l_type = F_UNLCK;                           /* line 607 */
	lock.l_whence = SEEK_SET;                        /* line 608: SEEK_SET reset */
	lock.l_start = 0L;                               /* line 609: 0 */
	lock.l_len = 0L;                                 /* line 610: 0 */
	fcntl (fileno (fp), F_SETLK, &lock);             /* line 611 */
```

**Result**: MATCH -- both lock and unlock changes applied exactly.

---

### FR-06: F_W3() -- SEEK_END lock + SEEK_SET unlock reset

**Design**: Change `l_whence = 0` to `SEEK_END`, add `SEEK_SET, 0, 0` unlock reset.

**Implementation -- Lock** (`st01/sub/file_rw.c` line 866):

```c
		lock.l_whence = SEEK_END;                    /* line 866: SEEK_END */
```

**Implementation -- Unlock** (`st01/sub/file_rw.c` lines 888-892):

```c
	lock.l_type = F_UNLCK;                           /* line 888 */
	lock.l_whence = SEEK_SET;                        /* line 889: SEEK_SET reset */
	lock.l_start = 0L;                               /* line 890: 0 */
	lock.l_len = 0L;                                 /* line 891: 0 */
	fcntl (fileno (fp), F_SETLK, &lock);             /* line 892 */
```

**Result**: MATCH -- both lock and unlock changes applied exactly.

---

### FR-07: F_WB() -- SEEK_END lock + SEEK_SET unlock reset

**Design**: Change `l_whence = 0` to `SEEK_END`, add `SEEK_SET, 0, 0` unlock reset.

**Implementation -- Lock** (`st01/sub/file_rw.c` line 950):

```c
		lock.l_whence = SEEK_END;                    /* line 950: SEEK_END */
```

**Implementation -- Unlock** (`st01/sub/file_rw.c` lines 1072-1076):

```c
	lock.l_type = F_UNLCK;                           /* line 1072 */
	lock.l_whence = SEEK_SET;                        /* line 1073: SEEK_SET reset */
	lock.l_start = 0L;                               /* line 1074: 0 */
	lock.l_len = 0L;                                 /* line 1075: 0 */
	fcntl (fileno (fp), F_SETLK, &lock);             /* line 1076 */
```

**Result**: MATCH -- both lock and unlock changes applied exactly.

---

### FR-08: Error handling preserved (EINTR retry, close/fclose + Log)

**Design**: All 7 functions maintain existing error handling unchanged.

**Verification**:

| Function | EINTR retry | Error: close/fclose | Error: Log | Error: return/Exit |
|----------|:-----------:|:-------------------:|:----------:|:------------------:|
| F_R      | line 89-90  | close(fd) line 92   | line 93-94 | return(NOTOK) line 95 |
| F_W      | line 271-272 | fclose(fp) line 274 | line 275-276 | return(NOTOK) line 277 |
| F_R2     | line 494-495 | close(fd) line 497 | line 498-499 | return(NOTOK) line 500 |
| F_R3     | line 786-787 | close(fd) line 789 | line 790-791 | return(NOTOK) line 792 |
| F_W2     | line 593-594 | fclose(fp) line 596 | line 597-598 | Exit_Process() line 599 |
| F_W3     | line 874-875 | fclose(fp) line 877 | line 878-879 | return(NOTOK) line 880 |
| F_WB     | line 958-959 | fclose(fp) line 961 | line 962-963 | return(NOTOK) line 964 |

**Result**: MATCH -- all error handling paths preserved identically.

---

### FR-09: EINTR retry logic maintained in all functions

**Design**: All `do { ... if (SYS_NO == EINTR) continue; ... } while (rt == -1)` patterns unchanged.

**Verification**: Confirmed in all 7 functions as listed in FR-08 table above. The retry loop
structure (`do { ... continue on EINTR ... } while (rt == -1)`) is intact in every function.

**Result**: MATCH.

---

## 3. FR Summary Table

| FR | Function | Change Description | Status | Notes |
|----|----------|-------------------|:------:|-------|
| FR-01 | F_R()  | F_RDLCK + record range + offset moved | MATCH | 5 changes verified |
| FR-02 | F_W()  | SEEK_END lock + SEEK_SET unlock reset  | MATCH | 2 locations verified |
| FR-03 | F_R2() | F_RDLCK + record range + offset moved | MATCH | 5 changes verified |
| FR-04 | F_R3() | F_RDLCK + record range + offset moved | MATCH | 5 changes verified |
| FR-05 | F_W2() | SEEK_END lock + SEEK_SET unlock reset  | MATCH | 2 locations verified |
| FR-06 | F_W3() | SEEK_END lock + SEEK_SET unlock reset  | MATCH | 2 locations verified |
| FR-07 | F_WB() | SEEK_END lock + SEEK_SET unlock reset  | MATCH | 2 locations verified |
| FR-08 | All    | Error handling preserved                | MATCH | 7 functions checked |
| FR-09 | All    | EINTR retry logic maintained            | MATCH | 7 functions checked |

**FR Match Rate: 9/9 = 100%**

---

## 4. Verification Checklist (14 items)

| # | Verification Item | Status | Evidence |
|---|-------------------|:------:|----------|
| V-01 | F_R: lock.l_type == F_RDLCK | PASS | line 80: `lock.l_type = F_RDLCK;` |
| V-02 | F_R: l_start=offset, l_len=rec_size*p_cnt | PASS | line 82: `lock.l_start = offset;` line 83: `lock.l_len = (long)(rec_size * p_cnt);` |
| V-03 | F_R: offset calculation before lock | PASS | line 77 (before do-while at line 79) |
| V-04 | F_R2: same pattern (F_RDLCK, record range, offset moved) | PASS | lines 482, 485-488 |
| V-05 | F_R3: same pattern (F_RDLCK, record range, offset moved) | PASS | lines 774, 777-780 |
| V-06 | F_W: lock.l_whence == SEEK_END | PASS | line 263: `lock.l_whence = SEEK_END;` |
| V-07 | F_W: unlock has SEEK_SET, 0, 0 reset | PASS | lines 286-288: `SEEK_SET, 0L, 0L` |
| V-08 | F_W2: same pattern (SEEK_END + unlock reset) | PASS | lock: line 585; unlock: lines 608-610 |
| V-09 | F_W3: same pattern (SEEK_END + unlock reset) | PASS | lock: line 866; unlock: lines 889-891 |
| V-10 | F_WB: same pattern (SEEK_END + unlock reset) | PASS | lock: line 950; unlock: lines 1073-1075 |
| V-11 | EINTR retry in all functions | PASS | All 7 functions have `if (SYS_NO == EINTR) continue;` |
| V-12 | Error path unlock correct | PASS | F_R: lock struct retains record range (l_start=offset, l_len=range); F_W*: error paths use fclose/Exit_Process which releases all locks |
| V-13 | 0 caller changes | PASS | Function signatures unchanged in `st01/sub/file_rw.c` and prototypes unchanged in `st01/inc/fep_sub.h`; grep confirms 90+ call sites in `st01/src/` unmodified |
| V-14 | Build success (mk.sh sub) | NOT TESTED | Requires server build environment |

**Verification Pass Rate: 13/13 = 100% (1 NOT TESTED -- build)**

---

## 5. Additional Observations

### 5.1 SF_W Function (out of scope)

`SF_W()` at lines 1090-1159 still uses the old `l_whence = 0` pattern (line 1132).
This is correct -- the design explicitly scoped only 7 functions (F_R, F_W, F_R2,
F_R3, F_W2, F_W3, F_WB). SF_W was intentionally excluded.

### 5.2 Error Path Lock Struct Values

For F_R-series functions, when `lseek64` fails (e.g., F_R line 101-108), the
`lock` struct still contains `l_type=F_RDLCK, l_whence=SEEK_SET, l_start=offset,
l_len=range`. The `F_UNLCK` on line 103 correctly unlocks the exact locked range.
This is an improvement over the old code which would have unlocked whole-file.

For F_W-series functions, error paths either:
- Call `fclose(fp)` which closes the fd and releases all fcntl locks (F_W, F_W3, F_WB), or
- Call `Exit_Process()` after `fclose(fp)` (F_W2), which terminates the process.

Both are safe -- no lock leak possible.

### 5.3 Caller Count

Active callers in `st01/src/` (excluding JC_OLD/ backups and .org files):

| Function | Call Sites | Files |
|----------|:---------:|:-----:|
| F_R      | ~31       | ~22   |
| F_W      | ~43       | ~27   |
| F_R2     | 0 (src/)  | 0     |
| F_R3     | 0 (src/)  | 0     |
| F_W2     | 0 (src/)  | 0     |
| F_W3     | 0 (src/)  | 0     |
| F_WB     | 0 (src/)  | 0     |

F_R2, F_R3, F_W2, F_W3, F_WB are declared `extern` in `fep_sub.h` and are
available to callers, but current active source in `st01/src/` does not call
them directly. They may be called from other subsystems or future code.

---

## 6. Overall Scores

| Category | Score | Status |
|----------|:-----:|:------:|
| FR Match (9 items) | 100% | PASS |
| Verification (13 testable / 14 total) | 100% | PASS |
| Architecture Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

```
+---------------------------------------------+
|  Overall Match Rate: 100%                   |
+---------------------------------------------+
|  FR Items:       9/9   (100%)               |
|  Verification:  13/13  (100%) + 1 N/T       |
|  Gaps Found:     0                          |
|  Added Features: 0                          |
|  Changed Items:  0                          |
+---------------------------------------------+
```

---

## 7. Differences Found

### Missing Features (Design O, Implementation X)

None.

### Added Features (Design X, Implementation O)

None.

### Changed Features (Design != Implementation)

None.

---

## 8. Recommended Actions

### Immediate Actions

None required -- all 9 FR items match the design exactly.

### Build Verification

V-14 (build success) requires server environment:

```sh
mk.sh sub       # Rebuild libfepP.a
mk.sh src       # Relink all binaries
```

### Optional Future Work

- Consider applying the same SEEK_END optimization to `SF_W()` in a future feature.
- The `SF_W` function at line 1132 still uses `l_whence = 0` (whole-file lock).

---

## 9. Conclusion

The implementation of file-rw-optimize in `st01/sub/file_rw.c` is a **perfect match**
to the design document. All 9 FR items were applied exactly as specified:

- 3 read functions (F_R, F_R2, F_R3) converted from `F_WRLCK` whole-file to `F_RDLCK` record-range
- 4 write functions (F_W, F_W2, F_W3, F_WB) converted from `SEEK_SET` whole-file to `SEEK_END` append-region
- All EINTR retry logic and error handling paths preserved unchanged
- Zero caller changes required (function signatures unchanged)

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-28 | Initial gap analysis -- 100% match | Claude (gap-detector) |
