# inet-addr-to-pton 설계 Document

> **요약**: Replace 9 deprecated `inet_addr()` calls with `inet_pton()`, fix 3 stale `ul` check bugs + 2 wrong-struct bugs in px_memok.c, clean up 5 dead comment blocks. 7 files, 17 FR items.
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안
> **Planning Doc**: [inet-addr-to-pton.plan.md](../01-plan/features/inet-addr-to-pton.plan.md)

---

## 1. 개요

### 1.1 설계 Goals

- Replace all active `inet_addr()` calls with POSIX `inet_pton(AF_INET, ...)` across 5 files
- Fix 3 stale `ul` variable bugs and 2 wrong-struct target bugs in px_memok.c
- Remove 5 dead `/* 202108 inet_addr ... */` comment blocks from 3 files
- Eliminate the `ul` intermediate variable where possible (write directly via inet_pton 3rd arg)
- Fix potential buffer overwrite from `sizeof(u_long)` on 64-bit platforms (8 bytes into 4-byte `ip_addr`)

### 1.2 설계 Principles

- **Minimal change**: Each transformation follows the same mechanical pattern
- **Direct write**: `inet_pton` writes `struct in_addr` (4 bytes) directly into the destination, eliminating the `ul` intermediate variable and memcpy where possible
- **Read-only correctness**: px_memok.c must NOT write to SHM — use temp `struct in_addr` for comparison
- **Error check upgrade**: `inet_pton() != 1` replaces `ul == -1` / `ul == (unsigned int)-1`

---

## 2. Transformation Patterns

### 2.1 Pattern A — Direct Write (config_db.c, pz_memory_conf.c)

For calls where `ul` is only used as an intermediate for `memcpy(ip_addr, &ul, sizeof(...))`:

```c
/* BEFORE: */
ul = inet_addr (val);
if (ul == (unsigned int)-1)
{
    Log (...);
    return CFG_DB_ERR_TYPE;
}
memcpy (TCP1(p_cnt,n-1).ip_addr, &ul, sizeof(TCP1(p_cnt,n-1).ip_addr));

/* AFTER: */
if (inet_pton (AF_INET, val, TCP1(p_cnt,n-1).ip_addr) != 1)
{
    Log (...);
    return CFG_DB_ERR_TYPE;
}
```

**Rationale**: `inet_pton(AF_INET, ...)` writes exactly `sizeof(struct in_addr)` = 4 bytes, matching `u_char ip_addr[4]`. Eliminates `ul` variable, `memcpy`, and the ambiguous `-1` error check.

### 2.2 Pattern B — Temp Variable (px_sett2ip.c, px_setudp.c)

For calls where the converted IP is used in multiple `memcpy` calls downstream:

```c
/* BEFORE: */
u_long  ul;
...
ul = inet_addr (ip);
if (ul == -1) { ... exit; }
...
memcpy (&TCP2_IP1(dk,pk,0), &ul, sizeof (ul));
memcpy (&TCP2_IP1(dk,pk,1), &ul, sizeof (ul));

/* AFTER: */
struct in_addr  tmp_ia;
...
if (inet_pton (AF_INET, ip, &tmp_ia) != 1) { ... exit; }
...
memcpy (&TCP2_IP1(dk,pk,0), &tmp_ia, sizeof (tmp_ia));
memcpy (&TCP2_IP1(dk,pk,1), &tmp_ia, sizeof (tmp_ia));
```

**Rationale**: `ul` is referenced multiple times after conversion. Replace `u_long ul` with `struct in_addr tmp_ia` (guaranteed 4 bytes). This also fixes a potential buffer overwrite on 64-bit platforms where `sizeof(u_long)` = 8 but `ip_addr` is only 4 bytes.

### 2.3 Pattern C — Direct sockaddr Write (udp_init.c)

For the `sin_addr.s_addr = inet_addr(ip)` pattern:

```c
/* BEFORE: */
svr_addr->sin_addr.s_addr = inet_addr(svr_ip);

/* AFTER: */
if (inet_pton(AF_INET, svr_ip, &svr_addr->sin_addr) != 1)
{
    Log(UDP_FATAL, "inet_pton fail svr_ip[%s] {%d:%s}", svr_ip, SYS_NO, SYS_STR);
    close(sockfd);
    return -1;
}
```

**Rationale**: Currently has NO error check. Adding `inet_pton` with error handling follows existing error pattern in this function (socket open failure returns -1).

### 2.4 Pattern D — Read-Only Comparison (px_memok.c)

For the verification tool that compares config values against SHM without modifying it:

```c
/* BEFORE (buggy 2021 migration): */
/* 202108
            ul = inet_addr (tmp2);
*/
            inet_pton(AF_INET, tmp2, &TCP2(p_cnt,cnt-1).ip_addr);  /* BUG: wrong struct + writes SHM */
            if (ul == -1)                                           /* BUG: stale ul */
                printf ("tcp1...");
            else
            {
                if (memcmp (TCP1(p_cnt,cnt-1).ip_addr, &ul, sizeof (ul)) != 0)  /* BUG: stale ul */
                    printf (...);
            }

/* AFTER (fixed): */
            {
                struct in_addr tmp_ia;
                if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
                    printf ("tcp1...");
                else
                {
                    if (memcmp (TCP1(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia)) != 0)
                        printf (...);
                }
            }
```

**Rationale**: Fixes 3 bugs per call site: (1) writes to wrong struct (TCP2 in TCP1/UDPIP context), (2) writes to SHM (read-only tool), (3) stale `ul` references. Uses block-scoped `struct in_addr` temp variable for clean conversion without SHM modification.

### 2.5 Pattern E — Dead Comment Removal

```c
/* BEFORE: */
/* 202108
    SvrAddr.sin_addr.s_addr    = inet_addr (ip_addr);
*/
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);

/* AFTER: */
    inet_pton(AF_INET, ip_addr, &SvrAddr.sin_addr.s_addr);
```

---

## 3. Detailed Implementation

### 3.1 범주 A: Active Migrations (9 calls, 5 files)

#### FR-01: `sub/config_db.c` line 1350 — TCP1 IP (Pattern A)

**Before** (lines 1350-1360):
```c
				ul = inet_addr (val);
				if (ul == (unsigned int)-1)
				{
					Log (USR_FATAL,
						"cfg_db tcp1(%c,%d):bad IP[%s]",
						g_cnt, n, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
				memcpy (TCP1(p_cnt,n-1).ip_addr, &ul,
					sizeof(TCP1(p_cnt,n-1).ip_addr));
```

**After**:
```c
				if (inet_pton (AF_INET, val, TCP1(p_cnt,n-1).ip_addr) != 1)
				{
					Log (USR_FATAL,
						"cfg_db tcp1(%c,%d):bad IP[%s]",
						g_cnt, n, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
```

**Also**: Remove `unsigned int ul;` declaration at line 1278 (no remaining uses in this function).

#### FR-02: `sub/config_db.c` line 1480 — TCP2 IP (Pattern A)

**Before** (lines 1480-1490):
```c
				ul = inet_addr (val);
				if (ul == (unsigned int)-1)
				{
					Log (USR_FATAL,
						"cfg_db tcp2(%c,%d):bad IP[%s]",
						g_cnt, n, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
				memcpy (TCP2(p_cnt,n-1).ip_addr, &ul,
					sizeof(TCP2(p_cnt,n-1).ip_addr));
```

**After**:
```c
				if (inet_pton (AF_INET, val, TCP2(p_cnt,n-1).ip_addr) != 1)
				{
					Log (USR_FATAL,
						"cfg_db tcp2(%c,%d):bad IP[%s]",
						g_cnt, n, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
```

**Also**: Remove `unsigned int ul;` declaration at line 1408.

#### FR-03: `sub/config_db.c` line 1607 — UDPIP IP (Pattern A)

**Before** (lines 1607-1617):
```c
				ul = inet_addr (val);
				if (ul == (unsigned int)-1)
				{
					Log (USR_FATAL,
						"cfg_db udpip(%c,%d):bad IP_%d[%s]",
						g_cnt, n, m, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
				memcpy (UDPIP(p_cnt,n-1).ip_addr[m-1], &ul,
					sizeof(UDPIP(p_cnt,n-1).ip_addr[m-1]));
```

**After**:
```c
				if (inet_pton (AF_INET, val, UDPIP(p_cnt,n-1).ip_addr[m-1]) != 1)
				{
					Log (USR_FATAL,
						"cfg_db udpip(%c,%d):bad IP_%d[%s]",
						g_cnt, n, m, val);
					sqlite3_finalize (stmt);
					return CFG_DB_ERR_TYPE;
				}
```

**Also**: Remove `unsigned int ul;` declaration at line 1534.

#### FR-04: `sub/udp_init.c` line 38 — Server Address (Pattern C)

**Before** (line 38):
```c
	svr_addr->sin_addr.s_addr = inet_addr(svr_ip);
```

**After** (lines 38-43):
```c
	if (inet_pton(AF_INET, svr_ip, &svr_addr->sin_addr) != 1)
	{
		Log(UDP_FATAL, "inet_pton fail svr_ip[%s] {%d:%s}", svr_ip, SYS_NO, SYS_STR);
		close(sockfd);
		return -1;
	}
```

**Note**: This is the only call with NO existing error check. Error pattern follows the socket-open failure at lines 32-35 (same severity, same return value).

#### FR-05: `src/PX/px_sett2ip.c` line 91 — TCP2 IP (Pattern B)

**Before** (lines 91-96, plus downstream at 121-122, 135):
```c
	ul = inet_addr (ip);
	if (ul == -1)
	{
		puts ("malformed IP address");
		exit (FAIL);
	}
	...
				memcpy (&TCP2_IP1(dk,pk,0), &ul, sizeof (ul));
				memcpy (&TCP2_IP1(dk,pk,1), &ul, sizeof (ul));
	...
				memcpy (&TCP2_IP1(dk,pk,l), &ul, sizeof (ul));
```

**After**:
```c
	if (inet_pton (AF_INET, ip, &tmp_ia) != 1)
	{
		puts ("malformed IP address");
		exit (FAIL);
	}
	...
				memcpy (&TCP2_IP1(dk,pk,0), &tmp_ia, sizeof (tmp_ia));
				memcpy (&TCP2_IP1(dk,pk,1), &tmp_ia, sizeof (tmp_ia));
	...
				memcpy (&TCP2_IP1(dk,pk,l), &tmp_ia, sizeof (tmp_ia));
```

**Also**: Replace `u_long ul;` declaration at line 17 with `struct in_addr tmp_ia;`.

**Bonus fix**: On 64-bit platforms, `sizeof(u_long)` = 8 bytes overwriting 4-byte `ip_addr[4]` field. Using `sizeof(tmp_ia)` = 4 bytes fixes this potential buffer overwrite.

#### FR-06: `src/PX/px_setudp.c` line 124 — UDP IP (Pattern B)

**Before** (lines 124-131):
```c
			ul = inet_addr (ip_addr);
			if (ul == -1)
			{
				printf ("malformed IP address[%s]\n", ip_addr);
				exit (0);
			}
			else
				memcpy (&UDP_IP1(dk,pk,sk), &ul, sizeof (ul));
```

**After**:
```c
			if (inet_pton (AF_INET, ip_addr, &tmp_ia) != 1)
			{
				printf ("malformed IP address[%s]\n", ip_addr);
				exit (0);
			}
			else
				memcpy (&UDP_IP1(dk,pk,sk), &tmp_ia, sizeof (tmp_ia));
```

**Also**: Replace `u_long ul;` declaration at line 17 with `struct in_addr tmp_ia;`.

**Bonus fix**: Same 64-bit `sizeof(u_long)` overwrite fix as FR-05.

#### FR-07: `src/PZ/pz_memory_conf.c` line 1709 — TCP1 IP (Pattern A)

**Before** (lines 1709-1722):
```c
				ul = inet_addr (tmp2);
				if (ul == -1)
				{
					Log (USR_FATAL, "tcp1(%c,%d):malformed IP address:%s[%s]",
						g_cnt, cnt, tmp1, tmp2);
					sleep (3);
					exit (FAIL);
				}
				else
				{
					memcpy (TCP1(p_cnt,cnt-1).ip_addr, &ul, sizeof (TCP1(p_cnt,cnt-1).ip_addr));
					Log (USR_OK, "tcp1(%c,%d):malformed IP address:%s[%s], ul:[%lu] [%c]",
						g_cnt, cnt, tmp1, tmp2, ul, TCP1(p_cnt,cnt-1).ip_addr);
				}
```

**After**:
```c
				if (inet_pton (AF_INET, tmp2, TCP1(p_cnt,cnt-1).ip_addr) != 1)
				{
					Log (USR_FATAL, "tcp1(%c,%d):malformed IP address:%s[%s]",
						g_cnt, cnt, tmp1, tmp2);
					sleep (3);
					exit (FAIL);
				}
				else
				{
					Log (USR_OK, "tcp1(%c,%d):IP address:%s[%s]",
						g_cnt, cnt, tmp1, tmp2);
				}
```

**Also**: Remove `unsigned int ul;` declaration at line 1585 (no remaining uses).

**Log change**: Remove `ul:[%lu]` from success-path Log (variable eliminated). The `[%c]` format with `ip_addr` pointer was producing garbage anyway (pointer cast to char).

#### FR-08: `src/PZ/pz_memory_conf.c` line 1996 — TCP2 IP (Pattern A)

**Before** (lines 1996-2009):
```c
				ul = inet_addr (tmp2);
				if (ul == -1)
				{
					Log (USR_FATAL, "tcp2(%c,%d):malformed IP address:%s tmp2[%s][%d]",
						g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
					sleep (3);
					exit (FAIL);
				}
				else
				{
					memcpy (TCP2(p_cnt,cnt-1).ip_addr, &ul, sizeof (TCP2(p_cnt,cnt-1).ip_addr));
					Log (USR_OK, "tcp2(%c,%d):malformed IP address:%s, ul:[%u] [%c] tmp2[%s][%d]",
						g_cnt, cnt, tmp1, ul, TCP2(p_cnt,cnt-1).ip_addr, tmp2, strlen(tmp2));
				}
```

**After**:
```c
				if (inet_pton (AF_INET, tmp2, TCP2(p_cnt,cnt-1).ip_addr) != 1)
				{
					Log (USR_FATAL, "tcp2(%c,%d):malformed IP address:%s tmp2[%s][%d]",
						g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
					sleep (3);
					exit (FAIL);
				}
				else
				{
					Log (USR_OK, "tcp2(%c,%d):IP address:%s tmp2[%s][%d]",
						g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
				}
```

**Also**: Remove `unsigned int ul;` declaration at line 1873.

#### FR-09: `src/PZ/pz_memory_conf.c` line 2187 — UDPIP IP (Pattern A)

**Before** (lines 2187-2202):
```c
					ul = inet_addr (tmp2);

					if (ul == -1)
					{
						Log (USR_FATAL,
							"udpip(%c,%d):malformed IP address:%s tmp2[%s][%d]",
							g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
						sleep (3);
						exit (FAIL);
					}
					else
					{
						memcpy (UDPIP(p_cnt,cnt-1).ip_addr[j], &ul, sizeof (UDPIP(p_cnt,cnt-1).ip_addr[j]));
						Log (USR_OK, "udpip(%c,%d):malformed IP address:%s, ul:[%u] [%c] tmp2[%s][%d]",
							g_cnt, cnt, tmp1, ul, UDPIP(p_cnt,cnt-1).ip_addr[j], tmp2, strlen(tmp2));
					}
```

**After**:
```c
					if (inet_pton (AF_INET, tmp2, UDPIP(p_cnt,cnt-1).ip_addr[j]) != 1)
					{
						Log (USR_FATAL,
							"udpip(%c,%d):malformed IP address:%s tmp2[%s][%d]",
							g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
						sleep (3);
						exit (FAIL);
					}
					else
					{
						Log (USR_OK, "udpip(%c,%d):IP address:%s tmp2[%s][%d]",
							g_cnt, cnt, tmp1, tmp2, strlen(tmp2));
					}
```

**Also**: Remove `unsigned int ul;` declaration at line 2060.

---

### 3.2 범주 B: Bug Fixes — px_memok.c (3 call sites, Pattern D)

All 3 call sites share the same bugs from the incomplete 2021 migration:
1. **Wrong struct target**: `inet_pton` writes to `TCP2(...)` when context is TCP1 (line 874) or UDPIP (line 1206)
2. **SHM write in read-only tool**: `inet_pton` writes directly to SHM struct (all 3 sites)
3. **Stale `ul` reference**: `if (ul == -1)` and `memcmp(..., &ul, sizeof(ul))` reference `ul` which is never set (commented out)

#### FR-10: `src/PX/px_memok.c` lines 871-886 — TCP1 Section

**Before** (lines 871-886):
```c
/* 202108
			ul = inet_addr (tmp2);
*/
			inet_pton(AF_INET, tmp2, &TCP2(p_cnt,cnt-1).ip_addr);
			if (ul == -1)
				printf ("tcp1(%c,%d):malformed IP address:%s[%s]\n",
					g_cnt, cnt, tmp1, tmp2);
			else
			{
				if (memcmp (TCP1(p_cnt,cnt-1).ip_addr, &ul, sizeof (ul)) != 0)
					printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
						TCP1(p_cnt,cnt-1).ip_addr[0],
						TCP1(p_cnt,cnt-1).ip_addr[1],
						TCP1(p_cnt,cnt-1).ip_addr[2],
						TCP1(p_cnt,cnt-1).ip_addr[3], tmp2);
			}
```

**After**:
```c
			{
				struct in_addr tmp_ia;
				if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
					printf ("tcp1(%c,%d):malformed IP address:%s[%s]\n",
						g_cnt, cnt, tmp1, tmp2);
				else
				{
					if (memcmp (TCP1(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia)) != 0)
						printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
							TCP1(p_cnt,cnt-1).ip_addr[0],
							TCP1(p_cnt,cnt-1).ip_addr[1],
							TCP1(p_cnt,cnt-1).ip_addr[2],
							TCP1(p_cnt,cnt-1).ip_addr[3], tmp2);
				}
			}
```

**Also**: Remove `u_long ul;` declaration at line 774 (no remaining uses in this function).

#### FR-11: `src/PX/px_memok.c` lines 1038-1053 — TCP2 Section

**Before** (lines 1038-1053):
```c
/* 202108
			ul = inet_addr (tmp2);
*/
			inet_pton(AF_INET, tmp2, &TCP2(p_cnt,cnt-1).ip_addr);
			if (ul == -1)
				printf ("tcp2(%c,%d):malformed IP address:%s[%s]\n",
					g_cnt, cnt, tmp1, tmp2);
			else
			{
				if (memcmp (TCP2(p_cnt,cnt-1).ip_addr, &ul, sizeof (ul)) != 0)
					printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
						TCP2(p_cnt,cnt-1).ip_addr[0],
						TCP2(p_cnt,cnt-1).ip_addr[1],
						TCP2(p_cnt,cnt-1).ip_addr[2],
						TCP2(p_cnt,cnt-1).ip_addr[3], tmp2);
			}
```

**After**:
```c
			{
				struct in_addr tmp_ia;
				if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
					printf ("tcp2(%c,%d):malformed IP address:%s[%s]\n",
						g_cnt, cnt, tmp1, tmp2);
				else
				{
					if (memcmp (TCP2(p_cnt,cnt-1).ip_addr, &tmp_ia, sizeof (tmp_ia)) != 0)
						printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
							TCP2(p_cnt,cnt-1).ip_addr[0],
							TCP2(p_cnt,cnt-1).ip_addr[1],
							TCP2(p_cnt,cnt-1).ip_addr[2],
							TCP2(p_cnt,cnt-1).ip_addr[3], tmp2);
				}
			}
```

**Also**: Remove `u_long ul;` declaration at line 941.

#### FR-12: `src/PX/px_memok.c` lines 1203-1220 — UDPIP Section

**Before** (lines 1203-1220):
```c
/* 202108
					ul = inet_addr (tmp2);
*/
					inet_pton(AF_INET, tmp2, &TCP2(p_cnt,cnt-1).ip_addr);

					if (ul == -1)
						printf ("udpip(%c,%d):malformed IP address:%s[%s]\n",
							g_cnt, cnt, tmp1, tmp2);
					else
					{
						if (memcmp (UDPIP(p_cnt,cnt-1).ip_addr[j], &ul,
							sizeof (ul)) != 0)
							printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
								UDPIP(p_cnt,cnt-1).ip_addr[j][0],
								UDPIP(p_cnt,cnt-1).ip_addr[j][1],
								UDPIP(p_cnt,cnt-1).ip_addr[j][2],
								UDPIP(p_cnt,cnt-1).ip_addr[j][3], tmp2);
					}
```

**After**:
```c
					{
						struct in_addr tmp_ia;
						if (inet_pton(AF_INET, tmp2, &tmp_ia) != 1)
							printf ("udpip(%c,%d):malformed IP address:%s[%s]\n",
								g_cnt, cnt, tmp1, tmp2);
						else
						{
							if (memcmp (UDPIP(p_cnt,cnt-1).ip_addr[j], &tmp_ia,
								sizeof (tmp_ia)) != 0)
								printf ("%s [%d.%d.%d.%d:%s]\n", tmp3,
									UDPIP(p_cnt,cnt-1).ip_addr[j][0],
									UDPIP(p_cnt,cnt-1).ip_addr[j][1],
									UDPIP(p_cnt,cnt-1).ip_addr[j][2],
									UDPIP(p_cnt,cnt-1).ip_addr[j][3], tmp2);
						}
					}
```

**Also**: Remove `u_long ul;` declaration at line 1100.

---

### 3.3 범주 C: Dead Comment Cleanup (5 blocks, 3 files)

#### FR-13: `src/PA/pa_7100_ur.c` lines 278-280

**Delete**:
```c
/* 202108
    SvrAddr.sin_addr.s_addr    = inet_addr (ip_addr);
*/
```

#### FR-14: `src/PA/pa_7000_mp.c` lines 359-361

**Delete**:
```c
/*
            Svr_Addr[i][j].sin_addr.s_addr    = inet_addr (&Svr_IP[20*i]);
*/
```

#### FR-15: `src/PX/px_memok.c` lines 871-873 (TCP1)

Deleted as part of FR-10 transformation (the `/* 202108 ... */` block is replaced entirely).

#### FR-16: `src/PX/px_memok.c` lines 1038-1040 (TCP2)

Deleted as part of FR-11 transformation.

#### FR-17: `src/PX/px_memok.c` lines 1203-1205 (UDPIP)

Deleted as part of FR-12 transformation.

---

## 4. Variable Cleanup 요약

After migration, the `ul` variable has zero remaining uses in each function scope.

| 파일 | Declaration Line | Type | Action |
|------|:----------------:|------|--------|
| `sub/config_db.c` | 1278 | `unsigned int ul` | Delete |
| `sub/config_db.c` | 1408 | `unsigned int ul` | Delete |
| `sub/config_db.c` | 1534 | `unsigned int ul` | Delete |
| `src/PZ/pz_memory_conf.c` | 1585 | `unsigned int ul` | Delete |
| `src/PZ/pz_memory_conf.c` | 1873 | `unsigned int ul` | Delete |
| `src/PZ/pz_memory_conf.c` | 2060 | `unsigned int ul` | Delete |
| `src/PX/px_memok.c` | 774 | `u_long ul` | Delete |
| `src/PX/px_memok.c` | 941 | `u_long ul` | Delete |
| `src/PX/px_memok.c` | 1100 | `u_long ul` | Delete |
| `src/PX/px_sett2ip.c` | 17 | `u_long ul` | Replace with `struct in_addr tmp_ia` |
| `src/PX/px_setudp.c` | 17 | `u_long ul` | Replace with `struct in_addr tmp_ia` |

---

## 5. Implementation Order

### Batch 1: `sub/config_db.c` (FR-01, FR-02, FR-03)

Shared library file. Three independent inet_addr→inet_pton conversions plus 3 `ul` declaration removals. Pattern A (direct write).

### Batch 2: `sub/udp_init.c` (FR-04)

Shared library file. Single call with new error handling added. Pattern C (sockaddr write).

### Batch 3: `src/PZ/pz_memory_conf.c` (FR-07, FR-08, FR-09)

Daemon process. Three conversions plus 3 `ul` removals plus 3 Log message updates. Pattern A.

### Batch 4: `src/PX/px_memok.c` (FR-10, FR-11, FR-12, FR-15, FR-16, FR-17)

검증 tool. Three bug fixes (stale ul + wrong struct + SHM write) plus 3 dead comment removals plus 3 `ul` removals. Pattern D (read-only comparison). Most complex batch.

### Batch 5: `src/PX/px_sett2ip.c` (FR-05)

Utility tool. Single conversion with downstream memcpy updates. Pattern B (temp variable).

### Batch 6: `src/PX/px_setudp.c` (FR-06)

Utility tool. Single conversion with downstream memcpy update. Pattern B.

### Batch 7: Dead comment cleanup (FR-13, FR-14)

`pa_7100_ur.c` and `pa_7000_mp.c`. Simple 3-line comment block deletions. Pattern E.

---

## 6. 검증 기준

### 6.1 Zero Active inet_addr

```sh
grep -rn "inet_addr" st01/sub/*.c st01/src/PA/*.c st01/src/PB/*.c st01/src/PX/*.c st01/src/PZ/*.c \
  --include="*.c" | grep -v "\.org:" | grep -v "\.back:" | grep -v "BACKUP/" | grep -v "utl/"
```

**Expected**: 0 matches (no active `inet_addr` calls or comments remaining).

### 6.2 inet_pton Count

```sh
grep -c "inet_pton" st01/sub/config_db.c
grep -c "inet_pton" st01/sub/udp_init.c
grep -c "inet_pton" st01/src/PZ/pz_memory_conf.c
grep -c "inet_pton" st01/src/PX/px_memok.c
grep -c "inet_pton" st01/src/PX/px_sett2ip.c
grep -c "inet_pton" st01/src/PX/px_setudp.c
```

| 파일 | Expected Count |
|------|:--------------:|
| `sub/config_db.c` | 3 |
| `sub/udp_init.c` | 1 |
| `src/PZ/pz_memory_conf.c` | 3 |
| `src/PX/px_memok.c` | 3 |
| `src/PX/px_sett2ip.c` | 1 |
| `src/PX/px_setudp.c` | 1 |

### 6.3 Stale Variable Check

```sh
grep -n "\bul\b" st01/src/PX/px_memok.c
```

**Expected**: 0 matches (all `ul` references removed).

### 6.4 No SHM Write in px_memok.c

```sh
grep -n "inet_pton.*TCP1\|inet_pton.*TCP2\|inet_pton.*UDPIP" st01/src/PX/px_memok.c
```

**Expected**: 0 matches (all inet_pton calls write to local `tmp_ia`, not SHM structs).

### 6.5 Dead Comment Check

```sh
grep -n "inet_addr" st01/src/PA/pa_7100_ur.c st01/src/PA/pa_7000_mp.c
```

**Expected**: 0 matches.

---

## 7. 설계 Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Target function | `inet_pton(AF_INET, ...)` | POSIX.1-2001 standard replacement |
| Error check | `!= 1` | Returns 1 on success, 0=invalid format, -1=AF error |
| `ul` elimination | Remove where possible | Reduces code, eliminates memcpy intermediate |
| Temp variable type | `struct in_addr` (not `u_long`) | Guaranteed 4 bytes on all platforms |
| px_memok.c scope | Block-scoped `struct in_addr` | Prevents SHM writes, clean variable lifetime |
| udp_init.c error severity | `UDP_FATAL` | Matches existing socket-open error at line 33 |
| pz_memory_conf.c Log update | Remove `ul` from format string | Variable eliminated; `%c` with pointer arg was garbage anyway |
| Header includes | No changes needed | `<arpa/inet.h>` already included via `fep_sub.h` |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
