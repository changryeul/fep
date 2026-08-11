# select-send-optimize Analysis Report

> **Analysis Type**: Gap Analysis (Design vs Implementation)
>
> **Project**: FEP (Front-End Processor for KRX)
> **Analyst**: Claude (gap-detector)
> **Date**: 2026-02-28
> **Design Doc**: [select-send-optimize.design.md](../02-design/features/select-send-optimize.design.md)

---

## 1. Analysis Overview

### 1.1 Analysis Purpose

Verify that the select-send-optimize feature -- replacing the redundant `select()` syscall in `Select_Send()` with kernel-level `SO_SNDTIMEO` 200ms timeout on socket creation -- was implemented exactly as specified in the design document.

### 1.2 Analysis Scope

- **Design Document**: `docs/02-design/features/select-send-optimize.design.md`
- **Implementation Files**:
  - `st01/sub/tcpip_connect.c` -- FR-01 (Connect), FR-02 (Connect2)
  - `st01/sub/tcpip_accept.c` -- FR-03 (Accept)
  - `st01/sub/select_send.c` -- FR-04 (select removal), FR-05 (Sendn preserved)
  - `st01/inc/fep_sub.h` -- V-07 (signature check)
- **Functional Requirements**: 5 (FR-01 through FR-05)
- **Verification Items**: 11 (V-01 through V-11)

---

## 2. Functional Requirement Verification

### 2.1 FR-01: Connect() SO_SNDTIMEO -- PASS

**Design** (Section 2.1.1): Add `struct timeval snd_timeout` declaration at block top, set `tv_sec=0, tv_usec=200000`, call `setsockopt(p_sfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&snd_timeout, sizeof(snd_timeout))` after TCP_NODELAY, log `TCP_WARN` on failure.

**Implementation** (`st01/sub/tcpip_connect.c` lines 36-49):
```c
	if (rt == 0)
	{
		int flag = 1;
		struct timeval snd_timeout;

		if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);

		snd_timeout.tv_sec = 0;
		snd_timeout.tv_usec = 200000;
		if (setsockopt (p_sfd, SOL_SOCKET, SO_SNDTIMEO,
			(char *)&snd_timeout, sizeof (snd_timeout)) < 0)
			Log (TCP_WARN, "Connect:SO_SNDTIMEO fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**Verdict**: Exact match with design. Timeout 200ms, target fd `p_sfd`, log prefix "Connect:", `(char *)` cast, tab indentation.

### 2.2 FR-02: Connect2() SO_SNDTIMEO -- PASS

**Design** (Section 2.1.2): Identical pattern to Connect() but with space indentation and "Connect2:" log prefix.

**Implementation** (`st01/sub/tcpip_connect.c` lines 89-104):
```c
    {
        int flag = 1;
        struct timeval snd_timeout;

        if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&flag, sizeof (flag)) < 0)
            Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
                SYS_NO, SYS_STR);

        snd_timeout.tv_sec = 0;
        snd_timeout.tv_usec = 200000;
        if (setsockopt (p_sfd, SOL_SOCKET, SO_SNDTIMEO,
            (char *)&snd_timeout, sizeof (snd_timeout)) < 0)
            Log (TCP_WARN, "Connect2:SO_SNDTIMEO fail {%d:%s}",
                SYS_NO, SYS_STR);
    }
```

**Verdict**: Exact match with design. Space indentation preserved, log prefix "Connect2:".

### 2.3 FR-03: Accept() SO_SNDTIMEO on accepted fd -- PASS

**Design** (Section 2.2.1): Set SO_SNDTIMEO on `rt` (accepted fd, not listen fd `p_sfd`), tab indentation, "Accept:" log prefix.

**Implementation** (`st01/sub/tcpip_accept.c` lines 55-70):
```c
	{
		int flag = 1;
		struct timeval snd_timeout;

		if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
			(char *)&flag, sizeof (flag)) < 0)
			Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
				SYS_NO, SYS_STR);

		snd_timeout.tv_sec = 0;
		snd_timeout.tv_usec = 200000;
		if (setsockopt (rt, SOL_SOCKET, SO_SNDTIMEO,
			(char *)&snd_timeout, sizeof (snd_timeout)) < 0)
			Log (TCP_WARN, "Accept:SO_SNDTIMEO fail {%d:%s}",
				SYS_NO, SYS_STR);
	}
```

**Verdict**: Exact match with design. Target fd is `rt` (accepted fd), tab indentation, "Accept:" prefix.

### 2.4 FR-04: select() removal from Select_Send -- PASS

**Design** (Section 2.3.1): Remove `fd_set write_set`, `struct timeval timeout`, `FD_ZERO`, `FD_SET`, `select()`, `FD_ISSET` -- 13 lines removed. Keep only `Sendn()` call path.

**Implementation** (`st01/sub/select_send.c` lines 20-34):
```c
int		Select_Send (int p_sfd, char *p_send, int p_length)
{
	int		rt;

	rt = Sendn (p_sfd, p_send, p_length);
	if (rt <= 0)
	{
		Log (TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}",
			rt, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	return (OK);
}
```

**Verdict**: Exact match with design. All select-related code removed. No `fd_set`, no `struct timeval`, no `FD_ZERO`, no `FD_SET`, no `select()`, no `FD_ISSET`.

### 2.5 FR-05: Sendn() error handling preserved -- PASS

**Design**: `Sendn()` call, `rt <= 0` check, `Log(TCP_ERROR, "Select_Send:send failure[%d] {%d:%s}", rt, SYS_NO, SYS_STR)`, `return (NOTOK)`, success `return (OK)`.

**Implementation**: Lines 25-33 -- identical to design. Error message format, return values, and logic all preserved.

---

## 3. Verification Item Detail

| ID | Item | Target File | Result | Evidence |
|----|------|-------------|:------:|----------|
| V-01 | Connect() SO_SNDTIMEO 200ms | tcpip_connect.c:44-49 | PASS | tv_sec=0, tv_usec=200000, setsockopt on p_sfd |
| V-02 | Connect2() SO_SNDTIMEO 200ms | tcpip_connect.c:98-103 | PASS | tv_sec=0, tv_usec=200000, setsockopt on p_sfd |
| V-03 | Accept() SO_SNDTIMEO on rt | tcpip_accept.c:64-69 | PASS | setsockopt target is `rt` (accepted fd) |
| V-04 | Log(TCP_WARN) only, no return | 3 files | PASS | All 3 sites: Log only, no early return |
| V-05 | select/FD_ISSET/fd_set removed | select_send.c | PASS | Zero occurrences of select-related symbols |
| V-06 | Sendn() + error handling preserved | select_send.c:25-31 | PASS | Identical to design: rt<=0 check, TCP_ERROR log |
| V-07 | Select_Send signature unchanged | select_send.c:20 + fep_sub.h:375 | PASS | `int Select_Send(int, char*, int)` both sides |
| V-08 | struct timeval at block top (C89) | tcpip_connect.c:37,91 tcpip_accept.c:57 | PASS | All 3 declarations immediately after `int flag` |
| V-09 | (char *) cast on SO_SNDTIMEO | 3 sites | PASS | `(char *)&snd_timeout` in all 3 setsockopt calls |
| V-10 | Indentation preserved | 3 files | PASS | Connect=tab, Connect2=space, Accept=tab |
| V-11 | Build success (mk.sh sub) | server env | N/T | Requires server environment |

---

## 4. Detailed Cross-Checks

### 4.1 V-04 Detail: Failure Handling Consistency

All three SO_SNDTIMEO setsockopt calls follow the same pattern as the existing TCP_NODELAY calls: log a warning, do not return or abort. This ensures that if SO_SNDTIMEO is unsupported on a platform (unlikely, but possible on older HP-UX), the socket remains functional with default blocking behavior.

| Site | Log Level | Log Prefix | Returns on Fail? |
|------|-----------|------------|:-----------------:|
| Connect() line 48 | TCP_WARN | "Connect:SO_SNDTIMEO" | No |
| Connect2() line 102 | TCP_WARN | "Connect2:SO_SNDTIMEO" | No |
| Accept() line 68 | TCP_WARN | "Accept:SO_SNDTIMEO" | No |

### 4.2 V-07 Detail: Signature Comparison

| Location | Declaration |
|----------|-------------|
| `select_send.c` line 20 | `int Select_Send (int p_sfd, char *p_send, int p_length)` |
| `fep_sub.h` line 375 | `extern int Select_Send (int, char *, int);` |

Parameter types and return type match exactly. The 22 callers of `Select_Send` require no changes.

### 4.3 V-08 Detail: C89 Block-Top Declaration Compliance

C89 requires all variable declarations before any executable statements within a block. Verified:

| File | Block | Declaration Order |
|------|-------|-------------------|
| tcpip_connect.c Connect() | `if (rt == 0) {` block (line 35) | `int flag` (L36), `struct timeval snd_timeout` (L37), then executable (L39) |
| tcpip_connect.c Connect2() | Anonymous block (line 89) | `int flag` (L90), `struct timeval snd_timeout` (L91), then executable (L93) |
| tcpip_accept.c Accept() | Anonymous block (line 55) | `int flag` (L56), `struct timeval snd_timeout` (L57), then executable (L59) |

All three comply with C89 declaration-before-statement rule.

### 4.4 V-10 Detail: Indentation Style Preservation

| Function | Expected Style | Actual Style | Verification Method |
|----------|----------------|--------------|---------------------|
| Connect() | Tab (`\t`) | Tab | Lines 44-49: leading `\t\t` characters |
| Connect2() | Space (4-space) | Space | Lines 98-103: leading spaces, no tabs |
| Accept() | Tab (`\t`) | Tab | Lines 64-69: leading `\t\t` characters |

Each function's pre-existing indentation style was preserved exactly.

---

## 5. Match Rate Summary

```
+---------------------------------------------+
|  Overall Match Rate: 100%                    |
+---------------------------------------------+
|  Verification Items:  11                     |
|  PASS:                10  (91%)              |
|  NOT TESTED:           1  ( 9%)  [V-11]     |
|  FAIL:                 0  ( 0%)              |
+---------------------------------------------+
|  Functional Requirements:  5                 |
|  FR-01 Connect SO_SNDTIMEO:     PASS        |
|  FR-02 Connect2 SO_SNDTIMEO:    PASS        |
|  FR-03 Accept SO_SNDTIMEO:      PASS        |
|  FR-04 select() removal:        PASS        |
|  FR-05 Sendn error preserved:   PASS        |
+---------------------------------------------+
```

**Effective Match Rate**: 100% (10/10 testable items PASS, 1 N/T build-only)

---

## 6. Gap Analysis

### 6.1 Missing Features (Design O, Implementation X)

None.

### 6.2 Added Features (Design X, Implementation O)

None.

### 6.3 Changed Features (Design != Implementation)

None. All implementation code matches the design document exactly -- variable names, timeout values, log messages, indentation, and control flow.

---

## 7. Scores

| Category | Score | Status |
|----------|:-----:|:------:|
| Design Match | 100% | PASS |
| C89 Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 8. Files Changed Summary

| File | Lines Before | Lines After | Delta | Changes |
|------|:-----------:|:-----------:|:-----:|---------|
| `st01/sub/tcpip_connect.c` | ~96 | 111 | +15 | +7 SO_SNDTIMEO in Connect(), +7 in Connect2(), +1 blank line |
| `st01/sub/tcpip_accept.c` | ~66 | 78 | +12 | +6 SO_SNDTIMEO in Accept(), +1 blank line |
| `st01/sub/select_send.c` | ~55 | 39 | -16 | -13 select/FD_ISSET lines, -3 variable declarations |
| `st01/inc/fep_sub.h` | 383 | 383 | 0 | No changes (signature unchanged) |

**Net effect**: +11 lines across the project (-16 in select_send, +15 in connect, +12 in accept).
**Syscall reduction**: Eliminates 1 `select()` syscall per `Select_Send()` invocation (22 call sites).

---

## 9. Recommended Actions

### 9.1 Immediate

None required. All testable verification items pass.

### 9.2 Deferred

| Priority | Item | Notes |
|----------|------|-------|
| Low | V-11 Build verification | Run `mk.sh sub` on server to confirm `libfepP.a` builds cleanly |

---

## 10. Design Document Updates Needed

None. The implementation matches the design exactly.

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-28 | Initial analysis -- 5 FR, 11 V items, 100% match | Claude (gap-detector) |
