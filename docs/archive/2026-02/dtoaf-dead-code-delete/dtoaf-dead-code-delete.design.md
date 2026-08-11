# dtoaf-dead-code-delete 설계 Document

> **요약**: Delete unused `sub/dtoaf.c` — zero callers, no extern, buffer overflow bugs in dead code
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안
> **Plan Reference**: `docs/01-plan/features/dtoaf-dead-code-delete.plan.md`

---

## 1. Current State 분석

### 1.1 Target 파일

**`sub/dtoaf.c`** (31 lines) — `DtoAf(double, char*, int)` converts double to fixed-width ASCII string.

```c
char *DtoAf (double p_double, char *p_ascii, int p_len)
{
    char    fmt[10], buf[20];
    sprintf (fmt, "%%%03d.0f", p_len);   /* dynamic format string */
    sprintf (buf, fmt, p_double);         /* buf[20] overflow risk */
    memcpy (p_ascii, buf, p_len);
    return (p_ascii);
}
```

### 1.2 Dead Code Evidence

| Check | Command | 결과 |
|-------|---------|--------|
| Source callers | `grep -r "DtoAf" st01/src/` | 0 matches |
| Library callers | `grep -r "DtoAf" st01/sub/` | Only definition (dtoaf.c:16, 26) |
| Header extern | `grep -r "DtoAf" st01/inc/` | 0 matches — not in fep_sub.h |
| Makefile refs | `grep -r "DtoAf\|dtoaf" st01/make/` | 0 matches |
| Backup/legacy | `grep -r "DtoAf" st01/src/PA/JC_OLD/` | 0 matches |

### 1.3 Sibling Functions (All Alive)

| Function | 파일 | Extern in fep_sub.h | Active Callers |
|----------|------|:-------------------:|:--------------:|
| `AtoIf` | `sub/atoif.c` | Yes | 80+ call sites |
| `ItoAf` | `sub/itoaf.c` | Yes | 80+ call sites |
| `AtoDf` | `sub/atodf.c` | Yes | 5 source files |
| `AtoLf` | `sub/atolf.c` | Yes | 2 source files |
| **`DtoAf`** | **`sub/dtoaf.c`** | **No** | **0** |

### 1.4 Build 영향

`Make_Lib_P_c.sh` auto-discovers all `sub/*.c` (except `dbipc.c`). Deleting `dtoaf.c` means:
- `dtoaf.o` will no longer be compiled
- `dtoaf.o` will no longer be archived into `lib/libfepP.a`
- No Makefile changes needed — auto-detected by shell glob
- No linker impact — no binary references `DtoAf`

---

## 2. Implementation 설계

### 2.1 기능 요구사항

| ID | Action | 파일 | 검증 |
|----|--------|------|-------------|
| FR-01 | Delete `sub/dtoaf.c` | `st01/sub/dtoaf.c` | File does not exist after deletion |

### 2.2 Implementation Steps

1. Delete `st01/sub/dtoaf.c`
2. Verify: `grep -r "DtoAf" st01/` returns 0 matches in active code
3. Verify: `ls st01/sub/dtoaf.c` returns "No such file"

### 2.3 Files NOT Modified

- `inc/fep_sub.h` — no extern for DtoAf exists, nothing to remove
- `make/SUB/Make_Lib_P_c.sh` — auto-discovers via glob, no change needed
- `sub/atodf.c`, `sub/atolf.c`, `sub/atoif.c`, `sub/itoaf.c` — siblings, all alive

---

## 3. 위험 평가

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| Hidden caller in unscanned path | 낮음 | Very| 낮음 | Full repo grep confirms 0 callers |
| Future need for double→string | 낮음 || 낮음 | Standard `sprintf(buf, "%*.0f", len, val)` is trivial |

---

## 4. 검증 기준

| Check | Expected| 결과 |
|-------|----------------|
| `ls st01/sub/dtoaf.c` | "No such file or directory" |
| `grep -r "DtoAf" st01/sub/ st01/src/ st01/inc/` | 0 matches |
| Sibling files unchanged | `atodf.c`, `atolf.c`, `atoif.c`, `itoaf.c` untouched |

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
