# pa-pb-dedup-phase3 Design Document

> **Summary**: PA/PB module deduplication Phase 3 — Time_Out_Disconnect helper extraction, Fifo_Event_Rtn unification
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft
> **Planning Doc**: [pa-pb-dedup-phase3.plan.md](../../01-plan/features/pa-pb-dedup-phase3.plan.md)

---

## 1. Overview

### 1.1 Design Goals

1. Extract the repeated disconnect/reconnect pattern from Time_Out_Rtn into `Time_Out_Disconnect()` shared helper
2. Unify identical `Fifo_Event_Rtn()` into fep_common.c
3. Remove dead code (#if 0 blocks, commented-out sections) during extraction
4. Maintain zero behavior change and build compatibility

### 1.2 Design Principles

- **Composition over parameterization** — Extract building-block helpers, keep thin wrappers in process files (avoid monolithic mode-flag functions)
- **Actual code citations** — Phase 2 lesson: code-level design → 97% match rate
- **Minimal extern additions** — Reuse existing declarations from fep_common.h

### 1.3 Scope Change: Write_Data DE-SCOPED

Write_Data (FR-03 from Plan) is **de-scoped** from Phase 3 based on Design-phase analysis:
- pa_1200_tr vs pb_1200_tr: only 43% code overlap
- pa_7800_tr vs pb_7800_tr: only 24% code overlap (74 vs 233 lines)
- PB versions add encryption (INL_Decrypt), different sequence extraction (6 vs 7 digits), PROC struct updates
- Parameterization would require unwieldy function signatures or callback patterns
- Recommendation: Defer to Phase 4+ with architectural refactoring approach

---

## 2. Architecture

### 2.1 Component Diagram

```
BEFORE (Phase 2 state):
┌──────────────────┐  ┌──────────────────┐
│ pa_1100_ts.c     │  │ pb_1100_ts.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← identical 56 lines each
│  Fifo_Event_Rtn()│  │  Fifo_Event_Rtn()│  ← identical 4 lines each
├──────────────────┤  ├──────────────────┤
│ pa_1200_tr.c     │  │ pb_1200_tr.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← identical 38 lines each
│  Fifo_Event_Rtn()│  │  Fifo_Event_Rtn()│  ← identical 4 lines each
├──────────────────┤  ├──────────────────┤
│ pa_7800_tr.c     │  │ pb_7800_tr.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← near-identical 43 lines each
│  Fifo_Event_Rtn()│  │  Fifo_Event_Rtn()│  ← identical 4 lines each
└──────────────────┘  └──────────────────┘
         │                     │
         └─────────┬───────────┘
                   ▼
        ┌──────────────────┐
        │ sub/fep_common.c │  (8 functions from Phase 1+2)
        │ sub/fep_encrypt.c│  (2 functions from Phase 2)
        └──────────────────┘

AFTER (Phase 3):
┌──────────────────┐  ┌──────────────────┐
│ pa_1100_ts.c     │  │ pb_1100_ts.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← thin 8 lines each
│                  │  │                  │     (calls Time_Out_Disconnect)
├──────────────────┤  ├──────────────────┤
│ pa_1200_tr.c     │  │ pb_1200_tr.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← thin 10 lines each
│                  │  │                  │     (calls Time_Out_Disconnect)
├──────────────────┤  ├──────────────────┤
│ pa_7800_tr.c     │  │ pb_7800_tr.c     │
│  Time_Out_Rtn()  │  │  Time_Out_Rtn()  │  ← thin 10/7 lines each
│                  │  │                  │     (heartbeat inline)
└──────────────────┘  └──────────────────┘
         │                     │
         └─────────┬───────────┘
                   ▼
        ┌──────────────────────┐
        │ sub/fep_common.c     │  +Time_Out_Disconnect() +Fifo_Event_Rtn()
        │ sub/fep_encrypt.c    │  (unchanged)
        └──────────────────────┘
```

### 2.2 Dependencies

| Component | Depends On | Status |
|-----------|-----------|--------|
| Time_Out_Disconnect | Device_Close, Line_Change, ConnectRetryCnt, TCP2_NET_STA, TCP2_LINE_ST | All already available in fep_common.c scope |
| Fifo_Event_Rtn | START_FD macro (fep_interface.h via fep_fepp.h) | Already available |

---

## 3. Detailed Implementation

### 3.1 FR-01: fep_common.h Changes

**File**: `st01/inc/fep_common.h`
**Current**: 49 lines (Phase 2 state)
**Change**: Add 2 new function prototypes

Add after line 38 (`extern int Device_Open_Logon (int, int);`):

```c
extern void		Time_Out_Disconnect (const char *);
extern void		Fifo_Event_Rtn (void);
```

**Full updated common function prototypes section (lines 27-40 after edit):**

```c
/*------------------------------------------------------------------------
	Common function prototypes
	(implemented in sub/fep_common.c)
------------------------------------------------------------------------*/
extern void		Get_Msec (double *);
extern void		Line_Change (void);
extern int		Device_Read (void);
extern void		Device_Write (void);
extern void		Device_Close_Base (void);
extern void		Err_Msg (void);
extern int		Log_Out_Base (void);
extern int		Device_Open_Logon (int, int);
extern void		Time_Out_Disconnect (const char *);
extern void		Fifo_Event_Rtn (void);
```

**No new extern variable declarations needed.** All globals used by Time_Out_Disconnect (ConnectRetryCnt, etc.) and Fifo_Event_Rtn (START_FD macro) are already accessible.

---

### 3.2 FR-02: Time_Out_Disconnect in fep_common.c

**File**: `st01/sub/fep_common.c`
**Location**: Insert after line 353 (end of Device_Open_Logon), before the "End of Program" comment at line 355.

**New function (actual code):**

```c
/*************************************************************************
	Function		: . Time_Out_Disconnect
	Parameters IN	: . source : connection source label ("KRX" or "FOT")
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . Timeout disconnect/reconnect pattern
					: . Checks TCP2_NET_STA, closes device, triggers
					: . Line_Change after 3 consecutive retries
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Time_Out_Disconnect (const char *source)
/*----------------------------------------------------------------------*/
{
	if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
	{
		TCP2_LINE_ST = END;
		Log (TCP_ERROR, "no data from %s. check status <%d>", source, INT_SEQ);
		Device_Close ();
		ConnectRetryCnt ++;

		if (ConnectRetryCnt == 3)
		{
			Line_Change ();
			ConnectRetryCnt = 0;
		}
	}

	return;
}	/* End of Time_Out_Disconnect ()	*/
```

**Line count**: 17 lines (function body)

**Derivation**: Extracted from the identical block in:
- `pa_1100_ts.c:678-694` (case 2 block, minus the commented #if 0 block)
- `pb_1100_ts.c:737-753` (identical)
- `pa_1200_tr.c:576-592` (case 2/3 block)
- `pb_1200_tr.c:721-737` (identical)

**Key design decision**: The `source` parameter replaces the hardcoded strings:
- 1100_ts files use `"KRX"`
- 1200_tr files use `"FOT"`

---

### 3.3 FR-03: Fifo_Event_Rtn in fep_common.c

**File**: `st01/sub/fep_common.c`
**Location**: Insert after Time_Out_Disconnect, before the "End of Program" comment.

**New function (actual code):**

```c
/*************************************************************************
	Function		: . Fifo_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . FIFO start signal handler (read from daemon pipe)
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char	tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn ()	*/
```

**Line count**: 5 lines (function body)

**Derivation**: Byte-for-byte identical in all 6 target files:
- `pa_1100_ts.c:372-380`
- `pb_1100_ts.c:407-415`
- `pa_1200_tr.c:289-297`
- `pb_1200_tr.c:317-325`
- `pa_7800_tr.c:261-269`
- `pb_7800_tr.c:299-307`

---

### 3.4 Process File Modifications

#### 3.4.1 pa_1100_ts.c

**Remove Fifo_Event_Rtn** (lines 372-380): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 670-725, currently 56 lines including #if 0 dead code):

**BEFORE** (current code, 56 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	2:
			if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
				Log (TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }
#if 0
			if (OpenFlag == ON)
			{
				Log (TCP_ERROR, "no data from KRX. check status <%d>", INT_SEQ);
				Device_Close ();

				ConnectRetryCnt ++;

				if (ConnectRetryCnt >= 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
			}
			else
			{
				Log (USR_OK, "poll timeout <%d>", INT_SEQ);
			}
#endif

			break;
		case	3:
			rt = Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &S_Fmt, SendLen);
			Device_Write ();
			//DeviceSendFlag = ON;
			break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/
```

**AFTER** (new code, 18 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	2:
			Time_Out_Disconnect ("KRX");
			break;
		case	3:
			Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &S_Fmt, SendLen);
			Device_Write ();
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -38 lines. Disconnect block → 1-line call. `#if 0` dead code removed. `int rt` removed (unused). `//DeviceSendFlag = ON` comment removed.

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 106: `void Fifo_Event_Rtn (void);`).

---

#### 3.4.2 pb_1100_ts.c

**Remove Fifo_Event_Rtn** (lines 407-415): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 729-784, currently 56 lines):

**AFTER** (identical to pa_1100_ts.c AFTER, 18 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	2:
			Time_Out_Disconnect ("KRX");
			break;
		case	3:
			Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &S_Fmt, SendLen);
			Device_Write ();
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -38 lines. Identical transformation as pa_1100_ts.c.

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 127: `void Fifo_Event_Rtn (void);`).

---

#### 3.4.3 pa_1200_tr.c

**Remove Fifo_Event_Rtn** (lines 289-297): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 564-601, currently 38 lines):

**BEFORE** (current code, 38 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case    1:
            if (TimeOut == FOREVER_TIME)
                Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
        case    3:
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                Log (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }

            break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/
```

**AFTER** (new code, 19 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	1:
			if (TimeOut == FOREVER_TIME)
				Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
					INT_SEQ, TCP2_NET_STA(S_K));
			break;
		case	2:
		case	3:
			Time_Out_Disconnect ("FOT");
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -19 lines. Disconnect block → 1-line call. `int rt` removed (unused).

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 109: `void Fifo_Event_Rtn (void);`).

---

#### 3.4.4 pb_1200_tr.c

**Remove Fifo_Event_Rtn** (lines 317-325): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 709-746, currently 38 lines):

**AFTER** (identical to pa_1200_tr.c AFTER, 19 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	1:
			if (TimeOut == FOREVER_TIME)
				Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
					INT_SEQ, TCP2_NET_STA(S_K));
			break;
		case	2:
		case	3:
			Time_Out_Disconnect ("FOT");
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -19 lines. Identical transformation as pa_1200_tr.c.

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 128: `void Fifo_Event_Rtn (void);`).

---

#### 3.4.5 pa_7800_tr.c

**Remove Fifo_Event_Rtn** (lines 261-269): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 423-465, currently 43 lines):

**BEFORE** (current code, 43 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case    1:
            if (TimeOut == FOREVER_TIME)
                Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
/*
일괄송신은 KRX가 아닌 증권사에서 POLL을 송신한다. 그래서 이부분을 미사용한다.2025
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                Log (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }
*/
			rt = Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &Reply, sizeof (KRX_JUMUN_R_FMT));
			Device_Write ();

            break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/
```

**AFTER** (new code, 20 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	1:
			if (TimeOut == FOREVER_TIME)
				Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
					INT_SEQ, TCP2_NET_STA(S_K));
			break;
		case	2:
			Make_Send_Msg (TR_POLL);
			memcpy (DataBuff, &Reply, sizeof (KRX_JUMUN_R_FMT));
			Device_Write ();
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -23 lines. Commented-out disconnect block removed. `int rt` removed (unused).

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 73: `void Fifo_Event_Rtn (void);`).

---

#### 3.4.6 pb_7800_tr.c

**Remove Fifo_Event_Rtn** (lines 299-307): Delete entire function definition.

**Replace Time_Out_Rtn** (lines 582-624, currently 43 lines):

**BEFORE** (current code, 43 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	int		rt;

	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case    1:
            if (TimeOut == FOREVER_TIME)
                Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
                    INT_SEQ, TCP2_NET_STA(S_K));

            break;
        case    2:
/*
일괄송신은 KRX가 아닌 증권사에서 POLL을 송신한다. 그래서 이부분을 미사용한다.2025
            if (TCP2_NET_STA(S_K) == OFF || TCP2_NET_STA(S_K) == ON)
            {
                TCP2_LINE_ST = END;
                Log (TCP_ERROR, "no data from FOT. check status <%d>", INT_SEQ);
                Device_Close ();
                ConnectRetryCnt ++;

                if (ConnectRetryCnt == 3)
                {
                    Line_Change ();
                    ConnectRetryCnt = 0;
                }
            }
*/

            break;
		default:
			break;
	}

	return;
}	/* End of Time_Out_Rtn ()	*/
```

**AFTER** (new code, 14 lines):
```c
void	Time_Out_Rtn (void)
/*----------------------------------------------------------------------*/
{
	memset (DataBuff, 0, sizeof (DataBuff));

	switch (PollCnt)
	{
		case	1:
			if (TimeOut == FOREVER_TIME)
				Log (USR_OK, "poll timeout <%d>:NSTAT[%d]",
					INT_SEQ, TCP2_NET_STA(S_K));
			break;
		default:
			break;
	}
}	/* End of Time_Out_Rtn ()	*/
```

**Changes**: -29 lines. Commented-out disconnect block removed. Empty `case 2` removed (was no-op). `int rt` removed (unused).

**Also remove** the forward declaration of `Fifo_Event_Rtn` (line 92: `void Fifo_Event_Rtn (void);`).

---

## 4. Implementation Order

### 4.1 Phase 1: Header Update
1. Edit `st01/inc/fep_common.h` — add 2 new prototypes

### 4.2 Phase 2: Shared Library Extension
2. Edit `st01/sub/fep_common.c` — add `Time_Out_Disconnect()` and `Fifo_Event_Rtn()`

### 4.3 Phase 3: Process File Updates (6 files, parallel-safe)
3. Edit `st01/src/PA/pa_1100_ts.c` — remove Fifo_Event_Rtn + forward decl, replace Time_Out_Rtn
4. Edit `st01/src/PB/pb_1100_ts.c` — same
5. Edit `st01/src/PA/pa_1200_tr.c` — same
6. Edit `st01/src/PB/pb_1200_tr.c` — same
7. Edit `st01/src/PA/pa_7800_tr.c` — same
8. Edit `st01/src/PB/pb_7800_tr.c` — same

---

## 5. Impact Summary

### 5.1 Line Count Changes

| File | Before | After | Delta |
|------|--------|-------|-------|
| `inc/fep_common.h` | 49 | 51 | +2 |
| `sub/fep_common.c` | 358 | 393 | +35 |
| `src/PA/pa_1100_ts.c` | ~1,279 | ~1,232 | **-47** |
| `src/PB/pb_1100_ts.c` | ~1,667 | ~1,620 | **-47** |
| `src/PA/pa_1200_tr.c` | ~895 | ~867 | **-28** |
| `src/PB/pb_1200_tr.c` | ~1,315 | ~1,287 | **-28** |
| `src/PA/pa_7800_tr.c` | ~767 | ~735 | **-32** |
| `src/PB/pb_7800_tr.c` | ~1,519 | ~1,481 | **-38** |
| **Total** | | | **-183** |

### 5.2 Cumulative Impact (Phase 1 + 2 + 3)

| Metric | Phase 1 | Phase 2 | Phase 3 | Cumulative |
|--------|---------|---------|---------|------------|
| Functions added to shared lib | 6 | 4 | 2 | **12** |
| Net lines removed | 1,133 | 605 | 183 | **1,921** |
| Reduction % | 13% | 7% | 2% | **~22%** |
| Original baseline | 8,843 | 7,710 | 7,105 | 8,843 → ~6,922 |

### 5.3 Dead Code Removed

| File | Dead Code Removed |
|------|-------------------|
| pa_1100_ts.c | 17-line `#if 0` block + `//DeviceSendFlag = ON` comment |
| pb_1100_ts.c | 17-line `#if 0` block + `//DeviceSendFlag = ON` comment |
| pa_7800_tr.c | 13-line commented-out disconnect block |
| pb_7800_tr.c | 13-line commented-out disconnect block + empty case 2 |

---

## 6. Verification Checklist

- [ ] `fep_common.h` has 2 new prototypes (Time_Out_Disconnect, Fifo_Event_Rtn)
- [ ] `fep_common.c` has Time_Out_Disconnect with `const char *source` parameter
- [ ] `fep_common.c` has Fifo_Event_Rtn reading START_FD
- [ ] All 6 process files: Fifo_Event_Rtn definition removed
- [ ] All 6 process files: Fifo_Event_Rtn forward declaration removed
- [ ] pa_1100_ts.c, pb_1100_ts.c: Time_Out_Rtn calls `Time_Out_Disconnect("KRX")` in case 2
- [ ] pa_1200_tr.c, pb_1200_tr.c: Time_Out_Rtn calls `Time_Out_Disconnect("FOT")` in case 2/3
- [ ] pa_7800_tr.c: Time_Out_Rtn case 2 retains heartbeat (Make_Send_Msg+Reply+Device_Write)
- [ ] pb_7800_tr.c: Time_Out_Rtn has only case 1 (FOREVER_TIME log) + default
- [ ] All `#if 0` dead code blocks removed
- [ ] All commented-out disconnect code blocks removed
- [ ] No `int rt;` in any Time_Out_Rtn (was unused variable in all 6 files)
- [ ] Build: `mk.sh sub && mk.sh src` passes

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft with actual code citations | Claude Code |
