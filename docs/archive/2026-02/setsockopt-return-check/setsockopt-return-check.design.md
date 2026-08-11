# setsockopt-return-check 설계 Document

> **요약**: Add return value checks to 6 unchecked setsockopt() calls — log-only on failure (UDP_WARN), no abort
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안
> **Plan Reference**: `docs/01-plan/features/setsockopt-return-check.plan.md`

---

## 1. Current State 분석

### 1.1 Unchecked Calls (6 total, 2 files)

**파일 1: `sub/udp_init.c`** (shared library, created in Feature #12 duplicate-code-extraction)

```c
/* Line 50 */ setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen);
/* Line 56 */ setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *)&optval, optlen);
/* Line 62 */ setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&optval, optlen);
```

Variable `rt` already declared on line 27: `int sockfd, rt;`

**파일 2: `src/PA/pa_7000_mp.c`** (market data manager process)

```c
/* Line 381 */ setsockopt (Sockfd[i][j],SOL_SOCKET,SO_SNDBUF, (void *)&bufflen, oplen);
/* Line 382 */ setsockopt (Sockfd[i][j],SOL_SOCKET,SO_RCVBUF, (void *)&bufflen, oplen);
/* Line 384 */ setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
```

Variable `rt` already declared — used on line 385: `rt = bind(...)`.

### 1.2 Reference Pattern (existing checked calls)

From `sub/fep_common.c:403-405` (Set_Socket_Linger):
```c
rt = setsockopt (fd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof (ling));
if (rt < 0)
    Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);
```

For UDP options, use `UDP_WARN` severity (consistent with `pa_7100_ur.c:304`, `pb_7100_ur.c:346`).

---

## 2. Implementation 설계

### 2.1 기능 요구사항

| ID | 파일 | Line | Option | Before | After |
|----|------|------|--------|--------|-------|
| FR-01 | `sub/udp_init.c` | 50 | SO_SNDBUF | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |
| FR-02 | `sub/udp_init.c` | 56 | SO_RCVBUF | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |
| FR-03 | `sub/udp_init.c` | 62 | SO_BROADCAST | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |
| FR-04 | `src/PA/pa_7000_mp.c` | 381 | SO_SNDBUF | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |
| FR-05 | `src/PA/pa_7000_mp.c` | 382 | SO_RCVBUF | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |
| FR-06 | `src/PA/pa_7000_mp.c` | 384 | SO_BROADCAST | `setsockopt(...)` | `rt = setsockopt(...); if (rt < 0) Log(UDP_WARN, ...)` |

### 2.2 Exact Transformations

**Batch 1: `sub/udp_init.c` (FR-01~03)**

```c
/* BEFORE (lines 49-50): */
		optval = sndbuf_size;
		setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen);

/* AFTER: */
		optval = sndbuf_size;
		rt = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen);
		if (rt < 0)
			Log(UDP_WARN, "setsockopt SO_SNDBUF fail {%d:%s}", SYS_NO, SYS_STR);
```

Same pattern for SO_RCVBUF (line 56) and SO_BROADCAST (line 62).

**Batch 2: `src/PA/pa_7000_mp.c` (FR-04~06)**

```c
/* BEFORE (lines 381-384): */
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_SNDBUF, (void *)&bufflen, oplen);
			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_RCVBUF, (void *)&bufflen, oplen);

			setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);

/* AFTER: */
			rt = setsockopt (Sockfd[i][j],SOL_SOCKET,SO_SNDBUF, (void *)&bufflen, oplen);
			if (rt < 0)
				Log (UDP_WARN, "setsockopt SO_SNDBUF fail {%d:%s}", SYS_NO, SYS_STR);
			rt = setsockopt (Sockfd[i][j],SOL_SOCKET,SO_RCVBUF, (void *)&bufflen, oplen);
			if (rt < 0)
				Log (UDP_WARN, "setsockopt SO_RCVBUF fail {%d:%s}", SYS_NO, SYS_STR);

			rt = setsockopt (Sockfd[i][j],SOL_SOCKET,SO_BROADCAST, (void *)&bufflen, oplen);
			if (rt < 0)
				Log (UDP_WARN, "setsockopt SO_BROADCAST fail {%d:%s}", SYS_NO, SYS_STR);
```

### 2.3 Implementation Order

1. Batch 1: `sub/udp_init.c` — FR-01, FR-02, FR-03
2. Batch 2: `src/PA/pa_7000_mp.c` — FR-04, FR-05, FR-06

### 2.4 설계 Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Severity level | `UDP_WARN` | Non-critical options; socket works with kernel defaults |
| Abort on failure? | No | Buffer sizes are optimizations, not requirements |
| Log format | `"setsockopt SO_xxx fail {%d:%s}"` | Consistent with fep_common.c pattern |
| Variable reuse | Existing `rt` | Both files already declare `int rt` |

---

## 3. 검증 기준

| Check | Expected| 결과 |
|-------|----------------|
| `grep -c "rt = setsockopt" st01/sub/udp_init.c` | 3 |
| `grep -c "rt = setsockopt" st01/src/PA/pa_7000_mp.c` | 3 |
| `grep -c "UDP_WARN.*setsockopt" st01/sub/udp_init.c` | 3 |
| `grep -c "UDP_WARN.*setsockopt" st01/src/PA/pa_7000_mp.c` | 3 |
| Unchecked setsockopt in active sub/ + src/ | 0 |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
