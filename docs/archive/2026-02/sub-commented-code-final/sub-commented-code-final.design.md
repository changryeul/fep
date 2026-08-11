# 설계: sub-commented-code-final

## Reference
- Plan: `docs/01-plan/features/sub-commented-code-final.plan.md`

## Implementation Order

1. FR-01 ~ FR-03: file_rw.c (3 items, 27 lines) — bottom-to-top: FR-03 → FR-02 → FR-01
2. FR-04 ~ FR-05: tcpip_connect.c (2 items, 6 lines) — bottom-to-top: FR-05 → FR-04
3. FR-06: queue.c (1 item, 6 lines)

Work bottom-to-top within each file to preserve line numbers for subsequent deletions.

## Change Specification

For each FR item: **delete the `/* */` commented-out code block**. No other changes to any file. All blocks contain old function calls that have been replaced by newer, safer implementations immediately following the block.

---

## 파일 1: sub/file_rw.c (3 items, 27 lines)

Three identical patterns — old `sprintf` directly into `file_rw` struct fields replaced by `sprintf(Tmp,...) + memcpy` pattern for buffer overflow safety.

### FR-01: Commented-out sprintf block in F_W() — 9 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 314-322 | 9 | `/* ... */` block: 7× `sprintf(file_rw.Xxx, ...)` in F_W() loop |

**Context (lines 312-324):**
```c
		memcpy (buff_rw.Seq, p_buf+i, sizeof (BUFF_RW_HEAD));
		// 2025EDIT                             // <-- stays (line 313)
/*                                              // <-- DELETE from here (314)
		sprintf (file_rw.Seq, "[%08d]", cnt);
		sprintf (file_rw.If_Seq, "[%-8.8s]", buff_rw.If_Seq);
		sprintf (file_rw.ApType, "[%-8.8s]", buff_rw.ApType);
		sprintf (file_rw.ResponseCode, "[%-4.4s]", buff_rw.ResponseCode);
		sprintf (file_rw.RecvTime1, "[%-10.10s]", buff_rw.RecvTime1);
		sprintf (file_rw.RecvTime2, "[%-12.12s]", buff_rw.RecvTime2);
		sprintf (file_rw.DataHeader, "[%-20.20s]", buff_rw.DataHeader);
*/                                              // <-- DELETE to here (322)
		/* Seq (10) */                          // <-- stays (line 323)
```

**After cleanup**: `// 2025EDIT` marker at line 313 preserved. Active `sprintf(Tmp,...) + memcpy` code at lines 323-343 is the replacement.

### FR-02: Commented-out sprintf block in F_W_One() — 9 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 645-653 | 9 | `/* ... */` block: 7× `sprintf(file_rw.Xxx, ...)` in F_W_One() |

**Context (lines 643-655):**
```c
	memcpy (buff_rw.Seq, p_buf, sizeof (BUFF_RW_HEAD));
                                                // <-- blank line (644)
/*                                              // <-- DELETE from here (645)
	sprintf (file_rw.Seq, "[%08d]", cnt);
	sprintf (file_rw.If_Seq, "[%-8.8s]", buff_rw.If_Seq);
	sprintf (file_rw.ApType, "[%-8.8s]", buff_rw.ApType);
	sprintf (file_rw.ResponseCode, "[%-4.4s]", buff_rw.ResponseCode);
	sprintf (file_rw.RecvTime1, "[%-10.10s]", buff_rw.RecvTime1);
	sprintf (file_rw.RecvTime2, "[%-12.12s]", buff_rw.RecvTime2);
	sprintf (file_rw.DataHeader, "[%-20.20s]", buff_rw.DataHeader);
*/                                              // <-- DELETE to here (653)
	/* Seq (10) */                              // <-- stays (line 654)
```

**After cleanup**: Active `sprintf(Tmp,...) + memcpy` code at lines 654-674 is the replacement. Identical pattern to FR-01.

### FR-03: Commented-out sprintf block in F_W_Sam() — 9 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 987-995 | 9 | `/* ... */` block: 7× `sprintf(file_rw.Xxx, ...)` in F_W_Sam() loop |

**Context (lines 985-997):**
```c
		memcpy (buff_rw.Seq, p_buf+i, sizeof (BUFF_RW_HEAD));
		// 2025EDIT                             // <-- stays (line 986)
/*                                              // <-- DELETE from here (987)
		sprintf (file_rw.Seq, "[%08d]", cnt);
		sprintf (file_rw.If_Seq, "[%-8.8s]", buff_rw.If_Seq);
		sprintf (file_rw.ApType, "[%-8.8s]", buff_rw.ApType);
		sprintf (file_rw.ResponseCode, "[%-4.4s]", buff_rw.ResponseCode);
		sprintf (file_rw.RecvTime1, "[%-10.10s]", buff_rw.RecvTime1);
		sprintf (file_rw.RecvTime2, "[%-12.12s]", buff_rw.RecvTime2);
		sprintf (file_rw.DataHeader, "[%-20.20s]", buff_rw.DataHeader);
*/                                              // <-- DELETE to here (995)
		/* Seq (10) */                          // <-- stays (line 996)
```

**After cleanup**: `// 2025EDIT` marker at line 986 preserved. Active `sprintf(Tmp,...) + memcpy` code at lines 996-1016 is the replacement. Identical pattern to FR-01.

---

## 파일 2: sub/tcpip_connect.c (2 items, 6 lines)

Two identical `inet_addr` calls replaced by IPv6-compatible `inet_pton`.

### FR-04: Commented-out inet_addr in Connect() — 3 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 29-31 | 3 | `/* ... */` block: `inet_addr()` call replaced by `inet_pton()` |

**Context (lines 28-33):**
```c
	address.sin_family = AF_INET;
/*                                              // <-- DELETE from here (29)
	address.sin_addr.s_addr = inet_addr (p_ip_addr);
*/                                              // <-- DELETE to here (31)
	inet_pton(AF_INET, p_ip_addr, &address.sin_addr.s_addr);  // <-- stays (32)
	address.sin_port = htons (p_port_no);       // <-- stays (33)
```

**After cleanup**: `inet_pton()` at line 32 is the active replacement for the deprecated `inet_addr()`.

### FR-05: Commented-out inet_addr in Connect2() — 3 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 59-61 | 3 | `/* ... */` block: `inet_addr()` call replaced by `inet_pton()` |

**Context (lines 58-63):**
```c
    address.sin_family = AF_INET;
/*                                              // <-- DELETE from here (59)
    address.sin_addr.s_addr = inet_addr (p_ip_addr);
*/                                              // <-- DELETE to here (61)
	inet_pton(AF_INET, p_ip_addr, &address.sin_addr.s_addr);  // <-- stays (62)
    address.sin_port = htons (p_port_no);       // <-- stays (63)
```

**After cleanup**: Identical pattern to FR-04. Both functions now use only `inet_pton()`.

---

## 파일 3: sub/queue.c (1 item, 6 lines)

### FR-06: Commented-out msgrcv queue flush loop — 6 lines

| Delete Lines | Size | Notes |
|:------------|:----:|-------|
| 126-131 | 6 | `/* ... */` block: disabled queue buffer clearing on creation |

**Context (lines 124-134):**
```c
    if(QID == -1) return FALSE;

    // 선택여부에 따라서 넣든지 빼든지  // <-- stays (line 125, Korean descriptive comment)
/* Q버퍼에 내용을 제거함                // <-- DELETE from here (126)
    msgbuf.mtype = (size_t) 0;
    do {
        rtv = msgrcv(QID,&msgbuf,MAXSIZE,msgbuf.mtype, IPC_NOWAIT);
    } while(rtv > 0);
*/                                              // <-- DELETE to here (131)

    return QID;                                 // <-- stays (line 133)
```

**After cleanup**: Korean descriptive comment at line 125 preserved — it explains the design decision (whether to include/exclude the flush). The queue is intentionally not flushed on creation.

---

## 요약

| 파일 | FR 항목 | Blocks | Lines Removed |
|------|:--------:|:------:|:-------------:|
| sub/file_rw.c | FR-01~03 | 3 | 27 |
| sub/tcpip_connect.c | FR-04~05 | 2 | 6 |
| sub/queue.c | FR-06 | 1 | 6 |
| **Total** | **6** | **6** | **39** |

### Lines Removed Breakdown

| 범주 | file_rw.c | tcpip_connect.c | queue.c | 합계 |
|----------|:---------:|:---------------:|:-------:|:-----:|
| `/* */` commented-out code | 27 | 6 | 6 | 39 |
| **Subtotal** | **27** | **6** | **6** | **39** |

## Preserved Items (범위 외)

| 항목 | 파일 | Line | Reason |
|------|------|:----:|--------|
| `// 2025EDIT` markers | file_rw.c | 313, 344, 640, 675, 986, 1017 + 4 more | Edit timestamps, not dead code |
| `// 선택여부에 따라서...` | queue.c | 125 | Korean descriptive comment |
| Function description `/* */` | hoga_check.c | 18-22, 31-35 | Documentation comments |
| Section banners / end markers | multiple | various | Structural comments |

## 검증 Checklist

1. All 6 `/* */` commented-out code blocks removed across 3 files
2. No functional code lines changed — only comment block deletions
3. All `// 2025EDIT` markers preserved in file_rw.c
4. Korean descriptive comment preserved in queue.c line 125
5. No lines added — pure deletion
6. Build: `mk.sh sub` produces byte-identical `libfepP.a`
