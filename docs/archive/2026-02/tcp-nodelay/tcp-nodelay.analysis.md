# tcp-nodelay Gap Analysis Report

> **Feature**: TCP_NODELAY -- Nagle disable in Connect/Accept
> **Date**: 2026-02-27
> **Design Document**: `docs/02-design/features/tcp-nodelay.design.md`
> **Implementation Files**:
>   - `st01/inc/fep_sub.h` (lines 103-108)
>   - `st01/sub/tcpip_connect.c` (full file)
>   - `st01/sub/tcpip_accept.c` (full file)

---

## Overall Match Rate: 100%

| Category | Score | Status |
|----------|:-----:|:------:|
| FR Match (5/5) | 100% | PASS |
| Verification (8/8, V-09 skipped) | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## FR Match Table

| ID | Requirement | Status | Notes |
|----|-------------|:------:|-------|
| FR-01 | Connect() success -> TCP_NODELAY | PASS | `tcpip_connect.c:34-41` -- `if (rt == 0)` block with setsockopt, exact match to design |
| FR-02 | Accept() success -> TCP_NODELAY | PASS | `tcpip_accept.c:55-61` -- `{}` scoped block after Log lines, setsockopt on `rt` (accepted fd), exact match |
| FR-03 | `<netinet/tcp.h>` include in fep_sub.h | PASS | `fep_sub.h:106` -- placed between `<netinet/in.h>` (105) and `<arpa/inet.h>` (107), exact match |
| FR-04 | Connect2() success -> TCP_NODELAY | PASS | `tcpip_connect.c:80-86` -- `{}` scoped block after `alarm(0)`, before `return (rt)`, exact match |
| FR-05 | setsockopt fail -> Log warning only | PASS | All 3 sites use `TCP_WARN` level; none alter return value; connection proceeds on failure |

---

## Verification Checklist

| ID | Item | Result | Evidence |
|----|------|:------:|----------|
| V-01 | `<netinet/tcp.h>` include added to fep_sub.h | PASS | `fep_sub.h:106`: `#include <netinet/tcp.h>` |
| V-02 | Connect() sets TCP_NODELAY on success | PASS | `tcpip_connect.c:34`: `if (rt == 0)` guard, lines 36-40 setsockopt call |
| V-03 | Connect2() sets TCP_NODELAY on success | PASS | `tcpip_connect.c:80-86`: `{}` block after `alarm(0)`, before `return (rt)` |
| V-04 | Accept() sets TCP_NODELAY on success | PASS | `tcpip_accept.c:55-61`: `{}` block after Log lines, before `return (rt)` |
| V-05 | Accept setsockopt target is `rt` (accepted fd), not `p_sfd` | PASS | `tcpip_accept.c:57`: `setsockopt (rt, IPPROTO_TCP, TCP_NODELAY, ...)` |
| V-06 | setsockopt failure = Log warning only (no connection abort) | PASS | All 3 sites: `if (setsockopt(...) < 0) Log(TCP_WARN, ...)` -- no return/exit |
| V-07 | `(char *)&flag` casting used (HP-UX/AIX compat) | PASS | All 3 sites: `(char *)&flag` -- not `(void *)` |
| V-08 | Existing callers unchanged (0 caller modifications) | PASS | grep confirms 0 TCP_NODELAY references in `st01/src/`; active callers: 10 Connect() in src/ (excl. JC_OLD/.org), 1 Accept() in src/, 1 Connect() in sub/fep_common.c -- all unchanged |
| V-09 | Build success (`mk.sh sub`) | SKIP | Requires server environment -- not tested |

---

## Detailed Code Comparison

### Connect() -- `tcpip_connect.c:34-41`

Design specifies:
```c
if (rt == 0)
{
    int flag = 1;
    if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Implementation (lines 34-41):
```c
if (rt == 0)
{
    int flag = 1;
    if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Connect:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Result: **Exact match** -- guard condition, socket fd, cast, log level, log string, error macros all identical.

### Connect2() -- `tcpip_connect.c:80-86`

Design specifies:
```c
{
    int flag = 1;
    if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Implementation (lines 80-86):
```c
{
    int flag = 1;
    if (setsockopt (p_sfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Connect2:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Result: **Exact match** -- scoped block, alarm(0) precedes, `rt != 0` early return above, log string "Connect2".

### Accept() -- `tcpip_accept.c:55-61`

Design specifies:
```c
{
    int flag = 1;
    if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Implementation (lines 55-61):
```c
{
    int flag = 1;
    if (setsockopt (rt, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag, sizeof (flag)) < 0)
        Log (TCP_WARN, "Accept:TCP_NODELAY fail {%d:%s}",
            SYS_NO, SYS_STR);
}
```

Result: **Exact match** -- target fd is `rt` (accepted socket), scoped block, log string "Accept".

### fep_sub.h -- lines 103-108

Design specifies:
```c
/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<netdb.h>
```

Implementation (lines 103-108):
```c
/* TCP/IP headers	*/
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<arpa/inet.h>
#include	<netdb.h>
```

Result: **Exact match** -- `<netinet/tcp.h>` placed after `<netinet/in.h>`, before `<arpa/inet.h>`.

---

## Caller Impact Verification

Active callers of Connect/Accept in `st01/src/` (excluding JC_OLD, .org, .back):

| Function | File | Modified? |
|----------|------|:---------:|
| Connect | `src/PA/pa_1600_tr.c` | No |
| Connect | `src/PA/pa_2100_ts.c` | No |
| Connect | `src/PA/pa_2200_tr.c` | No |
| Connect | `src/PA/pa_2700_tr.c` | No |
| Connect | `src/PA/pa_3100_ts.c` | No |
| Connect | `src/PA/pa_7100_ts.c` | No |
| Connect | `src/PB/pb_1800_ts.c` | No |
| Connect | `src/PB/pb_7100_ts.c` | No |
| Connect | `src/PB/pb_7100_ur.c` (x2) | No |
| Connect | `src/PB/pb_7200_tr.c` | No |
| Accept  | `src/PA/pa_7000_tr.c` | No |
| Connect | `sub/fep_common.c` | No |

All 12 active call sites (in 12 files) confirmed unchanged. Zero TCP_NODELAY references found in `st01/src/`.

---

## Gaps Found

**None.** All 5 FRs match exactly. All 8 testable verification items pass.

---

## Summary

The tcp-nodelay feature implementation is a **perfect match** to the design document:

1. **3 files modified** as specified: `fep_sub.h` (+1 line), `tcpip_connect.c` (+14 lines in 2 functions), `tcpip_accept.c` (+7 lines in 1 function).
2. **Pattern is consistent** across all 3 setsockopt sites: same `int flag = 1`, same `(char *)&flag` cast, same `TCP_WARN` log level, same `{%d:%s}` format with `SYS_NO, SYS_STR` macros.
3. **Zero caller changes** -- all 12 active call sites in src/ and sub/ are unmodified; TCP_NODELAY is transparently applied.
4. **Safety preserved** -- setsockopt failure logs a warning but never aborts the connection.
5. **Build verification** not tested (requires server environment).
