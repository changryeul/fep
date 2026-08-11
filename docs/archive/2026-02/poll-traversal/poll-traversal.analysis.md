# poll-traversal Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor)
> **Analyst**: Claude (gap-detector)
> **Date**: 2026-02-28
> **Design Doc**: [poll-traversal.design.md](../02-design/features/poll-traversal.design.md)
> **Plan Doc**: [poll-traversal.plan.md](../01-plan/features/poll-traversal.plan.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Design document specifies merging dual poll-event traversal loops into single loops across 12 files (1 shared function + 11 inline sites). This analysis verifies every verification item (V-01 through V-16) and functional requirement (FR-01 through FR-04) against the actual implementation.

### 1.2 Analysis Scope

- **Design Document**: `docs/02-design/features/poll-traversal.design.md`
- **Plan Document**: `docs/01-plan/features/poll-traversal.plan.md`
- **Implementation Files**: 12 files across `st01/sub/`, `st01/src/PA/`, `st01/src/PB/`, `st01/src/PW/`
- **Analysis Date**: 2026-02-28

---

## 2. Verification Item Results

### V-01: detect_poll_event single loop applied

**File**: `st01/sub/poll_event.c`
**Status**: PASS

The function at lines 19-47 contains exactly one `for` loop:
```c
for (i = 0; i < poll_cnt; i++)
```
No second loop exists. The dual-loop pattern (separate POLLHUP loop + POLLIN loop) has been fully replaced.

### V-02: first_pollin variable declared and initialized to -2

**File**: `st01/sub/poll_event.c`
**Status**: PASS

Line 21 declares `int i, first_pollin;` and line 23 initializes `first_pollin = -2;`. This matches the design exactly.

### V-03: detect_poll_event return semantics preserved (-1, -2, >=0)

**File**: `st01/sub/poll_event.c`
**Status**: PASS

- `-1`: returned at line 33 when `i == socket_event_idx` and POLLHUP detected
- `-2`: default value of `first_pollin` (line 23), returned at line 46 if no POLLIN found
- `>=0`: `first_pollin` set to `i` at line 42, returned at line 46

All three return semantics are preserved identically to the design.

### V-04: Dual for loops completely removed from all 11 inline files

**Status**: PASS

Each of the 11 inline files now contains exactly one `for` loop for poll event processing (no second POLLIN-only loop):

| # | File | Single loop location | Dual loop residue |
|---|------|---------------------|-------------------|
| 1 | `st01/src/PA/pa_1200_tr.c` | Line 256 | None |
| 2 | `st01/src/PB/pb_1200_tr.c` | Line 260 | None |
| 3 | `st01/src/PA/pa_7000_tr.c` | Line 210 | None |
| 4 | `st01/src/PA/pa_8200_tr.c` | Line 224 | None |
| 5 | `st01/src/PB/pb_7200_tr.c` | Line 166 | None |
| 6 | `st01/src/PB/pb_7100_ts.c` | Line 212 | None |
| 7 | `st01/src/PB/pb_8100_ts.c` | Line 259 | None |
| 8 | `st01/src/PB/pb_1800_ts.c` | Line 260 | None |
| 9 | `st01/src/PB/pb_7800_tr.c` | Line 168 | None |
| 10 | `st01/src/PW/pw_4000_ts.c` | Line 237 | None |
| 11 | `st01/src/PA/pa_3100_ts.c` | Line 706 | None |

### V-05: switch is inside POLLIN if block in all 11 files

**Status**: PASS

Every inline file has the `switch(i)` statement nested inside the `if (Poll[i].revents & POLLIN)` block, not outside the loop. Verified by inspection of each file:

| # | File | POLLIN if line | switch line | Inside? |
|---|------|:--------------:|:-----------:|:-------:|
| 1 | pa_1200_tr.c | 270 | 274 | Yes |
| 2 | pb_1200_tr.c | 274 | 278 | Yes |
| 3 | pa_7000_tr.c | 223 | 227 | Yes |
| 4 | pa_8200_tr.c | 237 | 241 | Yes |
| 5 | pb_7200_tr.c | 179 | 183 | Yes |
| 6 | pb_7100_ts.c | 225 | 229 | Yes |
| 7 | pb_8100_ts.c | 272 | 276 | Yes |
| 8 | pb_1800_ts.c | 274 | 278 | Yes |
| 9 | pb_7800_tr.c | 182 | 186 | Yes |
| 10 | pw_4000_ts.c | 251 | 255 | Yes |
| 11 | pa_3100_ts.c | 714 | 718 | Yes |

### V-06: POLLHUP check before POLLIN in all 12 files

**Status**: PASS

All 12 files have the pattern:
```c
if (Poll[i].revents & POLLHUP) { ... }
if (Poll[i].revents & POLLIN)  { ... }
```
POLLHUP check appears first in every single-loop body. The shared function (`poll_event.c`) also checks POLLHUP before POLLIN at lines 27-37 vs 39-43.

### V-07: Poll[i].revents = 0 preserved in all 12 files

**Status**: PASS

| # | File | revents = 0 location |
|---|------|---------------------|
| 1 | poll_event.c | Line 41: `poll_arr[i].revents = 0;` |
| 2 | pa_1200_tr.c | Line 272: `Poll[i].revents = 0;` |
| 3 | pb_1200_tr.c | Line 276: `Poll[i].revents = 0;` |
| 4 | pa_7000_tr.c | Line 225: `Poll[i].revents = 0;` |
| 5 | pa_8200_tr.c | Line 239: `Poll[i].revents = 0;` |
| 6 | pb_7200_tr.c | Line 181: `Poll[i].revents = 0;` |
| 7 | pb_7100_ts.c | Line 227: `Poll[i].revents = 0;` |
| 8 | pb_8100_ts.c | Line 274: `Poll[i].revents = 0;` |
| 9 | pb_1800_ts.c | Line 276: `Poll[i].revents = 0;` |
| 10 | pb_7800_tr.c | Line 184: `Poll[i].revents = 0;` |
| 11 | pw_4000_ts.c | Line 253: `Poll[i].revents = 0;` |
| 12 | pa_3100_ts.c | Line 716: `Poll[i].revents = 0;` |

### V-08: POLLHUP continue removed (unnecessary in single loop) in 11 inline files

**Status**: PASS

In the original dual-loop pattern, the POLLHUP loop contained `continue;` to skip to the next fd. In the merged single loop, `continue` is unnecessary because the POLLIN check follows immediately. None of the 11 inline files have `continue;` inside the POLLHUP block. All POLLHUP blocks end with closing brace, then fall through to the POLLIN check.

### V-09: POLLIN break removed (switch break only) in 11 inline files

**Status**: PASS

In the original dual-loop pattern, the POLLIN loop contained `break;` to exit after finding the first POLLIN event. In the merged single loop, the only `break;` statements are inside `switch` cases (case-level breaks). No loop-level `break;` exists in any POLLIN block.

### V-10: Each file's switch cases preserved correctly

**Status**: PASS

| # | File | Design cases | Implementation cases | Match |
|---|------|-------------|---------------------|:-----:|
| 1 | pa_1200_tr.c | FIFO_EVENT, SOCKET_EVENT, default | FIFO_EVENT, SOCKET_EVENT, default | Yes |
| 2 | pb_1200_tr.c | FIFO_EVENT, SOCKET_EVENT, default | FIFO_EVENT, SOCKET_EVENT, default | Yes |
| 3 | pa_7000_tr.c | SOCKET_EVENT, default | SOCKET_EVENT, default | Yes |
| 4 | pa_8200_tr.c | SOCKET_EVENT, default | SOCKET_EVENT, default | Yes |
| 5 | pb_7200_tr.c | SOCKET_EVENT, default | SOCKET_EVENT, default | Yes |
| 6 | pb_7100_ts.c | SOCKET_EVENT, DATA_EVENT, default | SOCKET_EVENT, DATA_EVENT, default | Yes |
| 7 | pb_8100_ts.c | SOCKET_EVENT, DATA_EVENT, default | SOCKET_EVENT, DATA_EVENT, default | Yes |
| 8 | pb_1800_ts.c | FIFO_EVENT, SOCKET_EVENT, DATA_EVENT, default | FIFO_EVENT, SOCKET_EVENT, DATA_EVENT, default | Yes |
| 9 | pb_7800_tr.c | FIFO_EVENT, SOCKET_EVENT, default | FIFO_EVENT, SOCKET_EVENT, default | Yes |
| 10 | pw_4000_ts.c | FIFO_EVENT, SOCKET_EVENT(Receive_Packet), FILE_EVENT, default | FIFO_EVENT, SOCKET_EVENT(Receive_Packet), FILE_EVENT, default | Yes |
| 11 | pa_3100_ts.c | FIFO_EVENT, case 1, default | FIFO_EVENT, case 1, default | Yes |

### V-11: Log vs SLog function preserved (pw_4000_ts and pa_3100_ts use SLog)

**Status**: PASS

- `pw_4000_ts.c` lines 243-248: Uses `SLog(TCP_ERROR, ...)` and `SLog(SYS_ERROR, ...)` -- correct
- `pa_3100_ts.c` lines 710-711: Uses `SLog(SYS_ERROR, ...)` -- correct
- All other 10 files use `Log(...)` -- correct

### V-12: pw_4000_ts Receive_Packet special handling preserved

**Status**: PASS

`pw_4000_ts.c` lines 260-264:
```c
case	SOCKET_EVENT:
    rt = Receive_Packet ();
    if (rt == NOTOK)
        return;
    break;
```
This matches the design exactly. The `Receive_Packet()` call with NOTOK check and `return` is preserved.

### V-13: pa_3100_ts secondary loop ANY POLLHUP -> return preserved

**Status**: PASS

`pa_3100_ts.c` lines 708-712:
```c
if (Poll[i].revents & POLLHUP)
{
    SLog (SYS_ERROR, "poll hangup[%d,%d]", i, PollCnt);
    return;
}
```
Unlike other files that check `if (i == SOCKET_EVENT)`, this secondary loop returns on ANY POLLHUP regardless of fd index. This behavior is preserved exactly as specified in the design.

### V-14: Indentation style preserved per file (tab vs space)

**Status**: PASS

| # | File | Expected style | Actual style | Match |
|---|------|---------------|-------------|:-----:|
| 1 | poll_event.c | Tab | Tab | Yes |
| 2 | pa_1200_tr.c | Tab | Tab | Yes |
| 3 | pb_1200_tr.c | Tab | Tab | Yes |
| 4 | pa_7000_tr.c | Tab | Tab | Yes |
| 5 | pa_8200_tr.c | Tab | Tab | Yes |
| 6 | pb_7200_tr.c | Tab | Tab | Yes |
| 7 | pb_7100_ts.c | Tab | Tab | Yes |
| 8 | pb_8100_ts.c | Tab | Tab | Yes |
| 9 | pb_1800_ts.c | Space | Space | Yes |
| 10 | pb_7800_tr.c | Space | Space | Yes |
| 11 | pw_4000_ts.c | Tab | Tab | Yes |
| 12 | pa_3100_ts.c | Space | Space | Yes |

### V-15: detect_poll_event 7 callers unchanged

**Status**: PASS

grep confirms 7 callers of `detect_poll_event` in `st01/src/`, all using the same unchanged pattern `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);`:

| # | File | Line | Pattern |
|---|------|:----:|---------|
| 1 | pa_1100_ts.c | 246 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 2 | pa_3100_ts.c | 213 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 3 | pa_7100_ts.c | 182 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 4 | pa_7800_tr.c | 167 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 5 | pa_8100_ts.c | 271 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 6 | pb_1100_ts.c | 267 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |
| 7 | pb_8200_tr.c | 233 | `i = detect_poll_event(Poll, PollCnt, SOCKET_EVENT);` |

None of these callers required modification -- the function signature and return semantics are unchanged.

### V-16: Build success

**Status**: NOT TESTED

Build verification requires the server environment (`mk.sh sub && mk.sh src`). Not testable on the current analysis machine.

---

## 3. Functional Requirements Verification

### FR-01: detect_poll_event() single loop with first_pollin

**Status**: PASS (100%)

The shared function in `st01/sub/poll_event.c` (lines 19-47) implements exactly the design:
- Single `for (i = 0; i < poll_cnt; i++)` loop
- `first_pollin` variable initialized to `-2`
- POLLHUP checked first; socket POLLHUP returns `-1` immediately
- POLLIN recorded only once (`&& first_pollin == -2`)
- `revents = 0` cleared for the first POLLIN fd
- Returns `first_pollin` at end (either `-2` or `>=0`)

### FR-02: 11 inline dual loops merged to single loop with inline switch

**Status**: PASS (100%)

All 11 inline files have been converted from the dual-loop + external-switch pattern to the single-loop + internal-switch pattern. Each file's specific cases, error handling, and special behavior are preserved.

### FR-03: Behavioral equivalence (POLLHUP priority per descriptor)

**Status**: PASS (100%)

In the single-loop implementation:
- Each descriptor's POLLHUP is checked before its POLLIN in the same iteration
- Socket POLLHUP causes immediate `return` (or `return -1` for shared function)
- Multiple POLLIN events in the same poll cycle are now all processed (improvement over the old break-on-first pattern)
- pa_3100_ts.c preserves the unique "ANY POLLHUP -> return" behavior

### FR-04: C89 coding style

**Status**: PASS (100%)

- All variables declared at function scope top (no inline declarations)
- Tab indentation preserved in tab-indented files; space indentation preserved in space-indented files
- No C99/C++ features introduced
- Comments follow existing Korean/English mixed style
- `/* */` block comments used for the new "poll event single traverse" comment

---

## 4. Gap Analysis Summary

### 4.1 Missing Features (Design present, Implementation absent)

None found.

### 4.2 Added Features (Implementation present, Design absent)

None found.

### 4.3 Changed Features (Design differs from Implementation)

None found. All 12 files match their design specifications exactly.

### 4.4 Match Rate Summary

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  V-01 detect_poll_event single loop:  PASS   |
|  V-02 first_pollin declaration:       PASS   |
|  V-03 Return semantics (-1,-2,>=0):   PASS   |
|  V-04 Dual loops removed (11):        PASS   |
|  V-05 switch inside POLLIN (11):      PASS   |
|  V-06 POLLHUP before POLLIN (12):     PASS   |
|  V-07 revents = 0 preserved (12):     PASS   |
|  V-08 POLLHUP continue removed (11):  PASS   |
|  V-09 POLLIN break removed (11):      PASS   |
|  V-10 switch cases preserved (11):    PASS   |
|  V-11 Log vs SLog preserved:          PASS   |
|  V-12 pw_4000 Receive_Packet:         PASS   |
|  V-13 pa_3100 ANY POLLHUP->return:    PASS   |
|  V-14 Indent style preserved (12):    PASS   |
|  V-15 7 callers unchanged:            PASS   |
|  V-16 Build success:                  N/T    |
+---------------------------------------------+
|  PASS: 15 / 15 (testable)                   |
|  NOT TESTED: 1 (V-16, requires server)      |
+---------------------------------------------+
```

---

## 5. Scores

| Category | Score | Status |
|----------|:-----:|:------:|
| Design Match (V-01 to V-13) | 100% | PASS |
| Architecture Compliance (FR-01 to FR-04) | 100% | PASS |
| Convention Compliance (V-14, FR-04) | 100% | PASS |
| Caller Compatibility (V-15) | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 6. File-by-File Verification Detail

### 6.1 poll_event.c (Step 1 -- shared function)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/sub/poll_event.c`

Design specifies:
```c
int i, first_pollin;
first_pollin = -2;
for (i = 0; i < poll_cnt; i++) {
    if (POLLHUP) { if socket return -1; log; }
    if (POLLIN && first_pollin == -2) { revents=0; first_pollin=i; }
}
return first_pollin;
```

Implementation (lines 19-47) matches this exactly. Function signature unchanged: `int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx)`.

### 6.2 pa_1200_tr.c (Step 2-A)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PA/pa_1200_tr.c`

Lines 255-288: Single loop with comment `/* poll event single traverse -- POLLHUP check then POLLIN */`, POLLHUP before POLLIN, switch(i) inside POLLIN with cases FIFO_EVENT and SOCKET_EVENT. Matches design 2-A exactly.

### 6.3 pb_1200_tr.c (Step 2-B)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_1200_tr.c`

Lines 259-292: Identical pattern to pa_1200_tr.c. Cases: FIFO_EVENT, SOCKET_EVENT. Matches design.

### 6.4 pa_7000_tr.c (Step 2-C)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PA/pa_7000_tr.c`

Lines 209-238: SOCKET_EVENT only + default. Uses `Log` (not SLog). Matches design.

### 6.5 pa_8200_tr.c (Step 2-D)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PA/pa_8200_tr.c`

Lines 223-252: SOCKET_EVENT only + default. Uses `Log`. Matches design.

### 6.6 pb_7200_tr.c (Step 2-E)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_7200_tr.c`

Lines 165-194: SOCKET_EVENT only + default. 2-level indentation (inside `else` block of while loop body). Uses `Log`. Matches design.

### 6.7 pb_7100_ts.c (Step 2-F)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_7100_ts.c`

Lines 211-243: SOCKET_EVENT, DATA_EVENT + default. 2-level indentation (inside `else` block). Uses `Log`. Matches design.

### 6.8 pb_8100_ts.c (Step 2-G)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_8100_ts.c`

Lines 258-290: SOCKET_EVENT, DATA_EVENT + default. The old "PB-specific manual for-loop" comment block has been removed, replaced with the standard single-traverse comment. Uses `Log`. Tab indentation. Matches design.

### 6.9 pb_1800_ts.c (Step 2-H)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_1800_ts.c`

Lines 259-295: FIFO_EVENT, SOCKET_EVENT, DATA_EVENT + default. Space indentation. Uses `Log`. Matches design.

### 6.10 pb_7800_tr.c (Step 2-I)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PB/pb_7800_tr.c`

Lines 167-200: FIFO_EVENT, SOCKET_EVENT + default. Space indentation. Uses `Log`. Matches design.

### 6.11 pw_4000_ts.c (Step 2-J)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PW/pw_4000_ts.c`

Lines 236-274: FIFO_EVENT, SOCKET_EVENT (with `rt = Receive_Packet()` + NOTOK return), FILE_EVENT + default (with `return` instead of `Exit_Process`). Uses `SLog` throughout. Matches design exactly.

### 6.12 pa_3100_ts.c (Step 2-K -- secondary loop)

**Path**: `/Users/ichang-yeol/MyWork/fep/st01/src/PA/pa_3100_ts.c`

Lines 705-732: Secondary loop in `Jang_End_Time_Process()`. ANY POLLHUP causes immediate `return` (no SOCKET_EVENT check). Cases: FIFO_EVENT, case 1 (Jang_End_Time_File_Rtn), default. Uses `SLog`. Space indentation. The main loop at line 213 still uses `detect_poll_event()` -- unchanged as expected. Matches design exactly.

---

## 7. Recommended Actions

### 7.1 Immediate

None required. All verification items pass.

### 7.2 Build Verification (V-16)

When server access is available, run:
```sh
mk.sh sub && mk.sh src
```
to confirm successful compilation of all affected modules (SUB, PA, PB, PW).

---

## 8. Impact Summary

| Metric | Value |
|--------|-------|
| Files modified | 12 |
| Shared function (poll_event.c) | 1 file, single loop with first_pollin |
| Inline conversions | 11 files |
| detect_poll_event callers | 7 (unchanged) |
| Lines net change (estimated) | -50 to -70 (removal of duplicate loops + continue/break) |
| Behavioral change | Multiple POLLIN events now processed per poll cycle |
| Risk | Low (POLLHUP priority preserved per descriptor) |

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-28 | Initial analysis -- 16 verification items, 100% match | Claude |
