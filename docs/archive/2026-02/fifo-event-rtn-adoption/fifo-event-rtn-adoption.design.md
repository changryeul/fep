# fifo-event-rtn-adoption 설계 문서

> **요약**: Extend shared Fifo_Event_Rtn to 13 remaining process files via isolated fifo_event.o
>
> **프로젝트**: FEP (Front-End Processor)
> **Author**: Claude Code
> **Date**: 2026-02-22
> **Status**: Draft
> **Planning Doc**: [fifo-event-rtn-adoption.plan.md](../../01-plan/features/fifo-event-rtn-adoption.plan.md)

---

## 1. 개요

### 1.1 설계 목표

1. Move `Fifo_Event_Rtn` from `sub/fep_common.c` to new `sub/fifo_event.c` (isolate into own .o)
2. Remove 13 local copies of `Fifo_Event_Rtn` from process files
3. Keep existing forward declarations in process files as prototypes
4. Zero behavior change, zero linker risk

### 1.2 설계 원칙

- **Object file isolation** — `fifo_event.o` has no variable dependencies beyond `Shm_Mem` (universally available)
- **Minimal change** — only delete local definitions; keep forward declarations untouched
- **Actual code citations** — exact BEFORE/AFTER for every change (Phase 3 lesson: 100% match rate)

---

## 2. 아키텍처

### 2.1 컴포넌트 다이어그램

```
BEFORE:
┌───────────────────┐     ┌───────────────────┐
│ fep_common.o      │     │ 13 process files   │
│  ...9 functions...│     │  Fifo_Event_Rtn() │ ← 13 local copies
│  Fifo_Event_Rtn() │     │  (definition)     │
└───────────────────┘     └───────────────────┘
        ↑ used by 6 KRX files only

AFTER:
┌───────────────────┐     ┌───────────────────┐
│ fep_common.o      │     │ fifo_event.o      │
│  ...9 functions...│     │  Fifo_Event_Rtn() │ ← single shared copy
│  (no Fifo_Event)  │     └───────────────────┘
└───────────────────┘             ↑
        ↑                         │
  6 KRX files              ALL 19 files (6 existing + 13 new)
```

### 2.2 Linker Dependency Comparison

| Object file | Undefined symbols | Safe for all binaries? |
|-------------|-------------------|:----------------------:|
| `fep_common.o` | ConnectRetryCnt, FmtPtr, Sockfd, NoTime[], ... | No (KRX-direct only) |
| `fifo_event.o` | Shm_Mem (via START_FD macro) | **Yes** (all processes) |

---

## 3. Detailed Implementation

### 3.1 FR-01: Create `sub/fifo_event.c` (NEW FILE)

**File**: `st01/sub/fifo_event.c`
**Content** (entire file):

```c
/*------------------------------------------------------------------------
#	Module	: Common FIFO event handler
#	File	: fifo_event.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

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

/*************************************************************************
	End of Program (fifo_event.c)
*************************************************************************/
```

**Line count**: 31 lines

**Note**: Includes `fep_fepp.h` (same as `fep_common.c`). This provides `START_FD` macro via the chain: `fep_fepp.h` → `fep_interface.h` → `START_FD` = `SFIFD(D_K)` = `Shm_Mem[D_K].start_FIFO_fd`.

---

### 3.2 FR-02: Remove Fifo_Event_Rtn from `sub/fep_common.c`

**File**: `st01/sub/fep_common.c`
**Remove**: Lines 385-401 (comment block + function definition)

**BEFORE** (lines 383-406):
```c
}	/* End of Time_Out_Disconnect ()	*/

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

/*************************************************************************
	End of Program (fep_common.c)
*************************************************************************/
```

**AFTER** (lines 383-386):
```c
}	/* End of Time_Out_Disconnect ()	*/

/*************************************************************************
	End of Program (fep_common.c)
*************************************************************************/
```

**Change**: -18 lines. The `fep_common.h` prototype `extern void Fifo_Event_Rtn(void);` at line 40 is **unchanged** — the linker now resolves it from `fifo_event.o` instead of `fep_common.o`.

---

### 3.3 FR-03: Remove local Fifo_Event_Rtn from 13 process files

For each file, remove only the function definition block (comment + function body). Keep the forward declaration unchanged.

#### 3.3.1 pa_2100_ts.c

**Keep**: Line 63 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 410-425 (definition block):
```c
/*************************************************************************
	Function  : . Fifo_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn () */
```

**Delta**: -16 lines

---

#### 3.3.2 pa_3100_ts.c

**Keep**: Line 76 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 323-338 (definition block):
```c
/*************************************************************************
	Function  : . Fifo_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn () */
```

**Delta**: -16 lines

---

#### 3.3.3 pa_5020_mp.c

**Keep**: Line 57 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 884-899 (definition block):
```c
/*************************************************************************
	Function  : . Fifo_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn () */
```

**Delta**: -16 lines

---

#### 3.3.4 pa_7000_tr.c

**Keep**: Line 45 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 248-263 (definition block):
```c
/*************************************************************************
	Function  : . Fifo_Event_Rtn
	Parameters IN : .
	Parameters OUT : .
	Return Code  : . void
	Comment   : . get the management FIFO signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
	char tmp[2];

	read (START_FD, tmp, 1);

	return;
}	/* End of Fifo_Event_Rtn () */
```

**Delta**: -16 lines

---

#### 3.3.5 pa_7100_ts.c

**Keep**: Line 51 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 237-252 (definition block)

**Delta**: -16 lines

---

#### 3.3.6 pa_8100_ts.c

**Keep**: Line 52 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 287-302 (definition block)

**Delta**: -16 lines

---

#### 3.3.7 pa_8200_tr.c

**Keep**: Line 50 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 268-283 (definition block)

**Delta**: -16 lines

---

#### 3.3.8 pb_1800_ts.c

**Keep**: Line 118 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 376-391 (definition block)

**Delta**: -16 lines

---

#### 3.3.9 pb_7100_ts.c

**Keep**: Line 62 forward declaration `void    Fifo_Event_Rtn (void);`

**Remove**: Lines 247-262 (definition block, note `*`-prefixed comment style):
```c
/*************************************************************************
 *  Function        : . Fifo_Event_Rtn
 *  Parameters IN   : .
 *  Parameters OUT  : .
 *  Return Code     : . void
 *  Comment         : . 업무 통제 FIFO SIGNAL GET & No Action
**************************************************************************/
/*----------------------------------------------------------------------*/
void    Fifo_Event_Rtn (void)
/*----------------------------------------------------------------------*/
{
    char    tmp[2];
    read (START_FD, tmp, 1);

    return;
}   /* End of Fifo_Event_Rtn () */
```

**Delta**: -16 lines (note: uses 4-space indent instead of tab; `*`-comment style)

---

#### 3.3.10 pb_7200_tr.c

**Keep**: Line 48 forward declaration `void    Fifo_Event_Rtn (void);`

**Remove**: Lines 224-238 (definition block)

**Delta**: -15 lines

---

#### 3.3.11 pb_8100_ts.c

**Keep**: Line 65 forward declaration `void    Fifo_Event_Rtn (void);`

**Remove**: Lines 287-302 (definition block)

**Delta**: -16 lines

---

#### 3.3.12 pb_8200_tr.c

**Keep**: Line 54 forward declaration `void    Fifo_Event_Rtn (void);`

**Remove**: Lines 263-278 (definition block)

**Delta**: -16 lines

---

#### 3.3.13 pw_4000_ts.c

**Keep**: Line 51 forward declaration `void Fifo_Event_Rtn (void);`

**Remove**: Lines 286-302 (definition block):
```c
/*************************************************************************
	Function		: . Fifo_Event_Rtn
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . get the management FIFO signal
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

**Delta**: -17 lines

**Note**: This file includes `fep_fepj.h` (not `fep_fepp.h`), but `START_FD` is defined in `fep_interface.h` which is included transitively by both. The forward declaration resolves via linker from `fifo_event.o`.

---

## 4. Implementation Order

### Phase 1: New shared file
1. Create `st01/sub/fifo_event.c`

### Phase 2: Remove from fep_common.c
2. Edit `st01/sub/fep_common.c` — remove Fifo_Event_Rtn definition

### Phase 3: Remove from 13 process files (parallel-safe)
3. `st01/src/PA/pa_2100_ts.c` — remove definition block
4. `st01/src/PA/pa_3100_ts.c` — remove definition block
5. `st01/src/PA/pa_5020_mp.c` — remove definition block
6. `st01/src/PA/pa_7000_tr.c` — remove definition block
7. `st01/src/PA/pa_7100_ts.c` — remove definition block
8. `st01/src/PA/pa_8100_ts.c` — remove definition block
9. `st01/src/PA/pa_8200_tr.c` — remove definition block
10. `st01/src/PB/pb_1800_ts.c` — remove definition block
11. `st01/src/PB/pb_7100_ts.c` — remove definition block
12. `st01/src/PB/pb_7200_tr.c` — remove definition block
13. `st01/src/PB/pb_8100_ts.c` — remove definition block
14. `st01/src/PB/pb_8200_tr.c` — remove definition block
15. `st01/src/PW/pw_4000_ts.c` — remove definition block

---

## 5. Impact Summary

### 5.1 Line Count Changes

| File | Delta |
|------|-------|
| `sub/fifo_event.c` (new) | **+31** |
| `sub/fep_common.c` | **-18** |
| `src/PA/pa_2100_ts.c` | **-16** |
| `src/PA/pa_3100_ts.c` | **-16** |
| `src/PA/pa_5020_mp.c` | **-16** |
| `src/PA/pa_7000_tr.c` | **-16** |
| `src/PA/pa_7100_ts.c` | **-16** |
| `src/PA/pa_8100_ts.c` | **-16** |
| `src/PA/pa_8200_tr.c` | **-16** |
| `src/PB/pb_1800_ts.c` | **-16** |
| `src/PB/pb_7100_ts.c` | **-16** |
| `src/PB/pb_7200_tr.c` | **-15** |
| `src/PB/pb_8100_ts.c` | **-16** |
| `src/PB/pb_8200_tr.c` | **-16** |
| `src/PW/pw_4000_ts.c` | **-17** |
| **Net** | **-195 lines** |

### 5.2 Files Unchanged

- `inc/fep_common.h` — no change (prototype `extern void Fifo_Event_Rtn(void)` remains at line 40)
- All 6 existing PA/PB KRX files — no change (already use shared version via fep_common.h)

---

## 6. Verification Checklist

- [ ] `sub/fifo_event.c` exists with correct function body
- [ ] `sub/fifo_event.c` includes `fep_fepp.h`
- [ ] `sub/fep_common.c` no longer contains Fifo_Event_Rtn definition
- [ ] `sub/fep_common.c` still ends with "End of Program" comment
- [ ] `inc/fep_common.h` line 40 unchanged: `extern void Fifo_Event_Rtn(void);`
- [ ] All 13 process files: Fifo_Event_Rtn definition removed
- [ ] All 13 process files: forward declaration retained
- [ ] No process file has both definition AND forward declaration for Fifo_Event_Rtn
- [ ] Build: `mk.sh sub` compiles fifo_event.c into libfepP.a
- [ ] Build: `mk.sh src` links all process binaries

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-22 | Initial draft with actual code citations | Claude Code |
