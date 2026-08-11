# fifo-struct-refactor 설계 문서

> **요약**: Code-level design for replacing raw byte offsets with struct-based type-safe access in FIFO/SAM file I/O buffer construction and KRX message parsing
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft
> **Plan Reference**: `docs/01-plan/features/fifo-struct-refactor.plan.md`

---

## 1. 개요

### 1.1 설계 목표

Replace three categories of raw byte offset patterns with struct-based access:

1. **`w_data[0..70]`** — Manual BUFF_RW_HEAD construction via numeric offsets → populate `BUFF_RW_HEAD` struct fields, then `memcpy` to `w_data`
2. **`DataBuff[8+6]`** — KRX MsgType access at hardcoded offset 14 → cast `DataBuff` to `KRX_HEADER*` and access `->MsgType`
3. **`ADD_HEADER_SIZE`** — Dead macro (always 0) still referenced → remove entirely

### 1.2 Key Struct Definitions (existing, no changes needed)

```c
/* inc/fep_file.h — 50 bytes, all char arrays, no padding */
typedef struct {
    char Seq[8];            /* offset 0  */
    char If_Seq[8];         /* offset 8  */
    char ApType[8];         /* offset 16 */
    char ResponseCode[4];   /* offset 24 */
    char RecvTime1[10];     /* offset 28 */
    char RecvTime2[12];     /* offset 38 */
    char DataHeader[20];    /* offset 50 */
} BUFF_RW_HEAD;             /* total: 70 bytes (50 header + 20 data header) */

/* inc/fep_tcpip.h — 20 bytes */
typedef struct {
    char Length[4];         /* Data length */
    char DataSeq[8];        /* Data serial number */
    char ResponseCode[4];   /* Response code */
    char LineFlag[3];       /* Outbound line ID */
    char Filler[1];
} FILE_DATA_HEAD;           /* 20 bytes, HEAD_SIZE = sizeof(FILE_DATA_HEAD) */

/* inc/pa_struct.h — 82 bytes */
typedef struct {
    char BeginString[8];    /* offset 0  */
    char BodyLength[6];     /* offset 8  */
    char MsgType[11];       /* offset 14 (= 8+6) */
    char MsgSeqNum[11];     /* offset 25 */
    char SenderCompID[5];   /* offset 36 */
    char DeliverToCompID[10]; /* offset 41 */
    char OnBehalfOfCompID[10]; /* offset 51 */
    char SendingTime[17];   /* offset 61 */
    char DataCnt[3];        /* offset 78 */
    char Encrypt[1];        /* offset 81 */
} KRX_HEADER;               /* 82 bytes, KRX_HEAD_LEN = sizeof(KRX_HEADER) */
```

---

## 2. FR-01: w_data[] Raw BUFF_RW_HEAD Construction → Struct-Based

### 2.1 Problem

Five code blocks across 3 files manually build `BUFF_RW_HEAD` content at literal byte offsets:

```c
/* BEFORE — repeated pattern in pb_7800_tr.c, pa_7800_tr.c, pa_2700_tr.c */
ItoAf (d_seq,        &w_data[0],  8);   // Seq
ItoAf (d_seq,        &w_data[8],  8);   // IF_Seq
memcpy (&w_data[16], ApType,      8);   // ApType
memcpy (&w_data[24], RES_NORMAL, strlen(RES_NORMAL)); // ResponseCode
memcpy (&w_data[28], m_time,      10);  // RecvTime1
memcpy (&w_data[38], &m_time[10], 12);  // RecvTime2
/* ... fill FILE_DATA_HEAD ... */
memcpy (&w_data[50], &File_Data_Head, sizeof(HEAD_SIZE));
memcpy (&w_data[70], data_ptr, data_size); // actual data
```

The offsets 0, 8, 16, 24, 28, 38, 50, 70 duplicate the `BUFF_RW_HEAD` struct layout.

### 2.2 Design

**Step 1**: Add a helper function to `inc/fep_file.h` (FR-06, see section 7).

**Step 2**: Replace each w_data[] block with struct population + memcpy.

```c
/* AFTER */
BUFF_RW_HEAD    bw_head;

memset (&bw_head, 0x20, sizeof(bw_head));
ItoAf (d_seq,        bw_head.Seq,          sizeof(bw_head.Seq));
ItoAf (d_seq,        bw_head.If_Seq,       sizeof(bw_head.If_Seq));
memcpy (bw_head.ApType,        ApType,         sizeof(bw_head.ApType));
memcpy (bw_head.ResponseCode,  RES_NORMAL,     strlen(RES_NORMAL));
memcpy (bw_head.RecvTime1,     m_time,         sizeof(bw_head.RecvTime1));
memcpy (bw_head.RecvTime2,     &m_time[10],    sizeof(bw_head.RecvTime2));

/* FILE_DATA_HEAD population stays the same (already struct-based) */
memcpy (bw_head.DataHeader, &File_Data_Head, sizeof(FILE_DATA_HEAD));

memcpy (w_data, &bw_head, sizeof(BUFF_RW_HEAD));
memcpy (&w_data[sizeof(BUFF_RW_HEAD)], data_ptr, data_size);
```

**Key**: `sizeof(BUFF_RW_HEAD)` = 70 replaces literal `70`. The `DataHeader[20]` field holds the `FILE_DATA_HEAD` content.

### 2.3 Target Files and Locations

| File | Line Range | Seq Source | Data Source |
|------|-----------|------------|-------------|
| `pb_7800_tr.c` | 1309-1331 | `d_seq` | `DataBuff[0]` (82 bytes KRX hdr) + `dec_data[i*d_size]` |
| `pb_7800_tr.c` | 1396-1419 | `d_seq` | `DataBuff[0]` (82 bytes) + `DataBuff[82+i*d_size]` |
| `pb_7800_tr.c` | ~1200 block | `d_seq` | similar pattern (3rd block if exists) |
| `pa_7800_tr.c` | 697-718 | `d_seq` | `DataBuff[KRX_HEAD_LEN+(d_size*i)]` |
| `pa_2700_tr.c` | 411-432 | `INT_SEQ+1` | `RecvPkt[50]` |

### 2.4 Variable Declaration Changes

Each function with w_data[] blocks needs a local variable:

```c
BUFF_RW_HEAD    bw_head;    /* add at function's variable declarations */
```

### 2.5 Validation

`sizeof(BUFF_RW_HEAD)` must equal 70. All fields are `char` arrays with no alignment gaps. Verify on target platform:
```c
/* Can add to a debug log at startup if needed */
/* BUFF_RW_HEAD: Seq(8)+If_Seq(8)+ApType(8)+ResponseCode(4)+RecvTime1(10)+RecvTime2(12)+DataHeader(20) = 70 */
```

---

## 3. FR-02: DataBuff[8+6] → KRX_HEADER Struct Cast

### 3.1 Problem

`DataBuff[8+6]` accesses `KRX_HEADER.MsgType` (offset 14 = `BeginString[8]` + `BodyLength[6]`). This pattern appears ~30 times across 7 files.

### 3.2 Design

Cast `DataBuff` to `KRX_HEADER*` at the point of use (or reuse existing `Header_Fmt` if the file already has one):

```c
/* BEFORE */
if (memcmp(&DataBuff[8+6], "SCHLIQ00102", 11) != 0 ||
    memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4) != 0)
{
    Log (TCP_ERROR, "error[%.11s:%.4s]",
        &DataBuff[8+6], &DataBuff[sizeof(KRX_HEADER)]);
}

/* AFTER — Option A: Local pointer (preferred for functions with multiple accesses) */
KRX_HEADER  *krx_hdr = (KRX_HEADER *)DataBuff;

if (memcmp(krx_hdr->MsgType, "SCHLIQ00102", 11) != 0 ||
    memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4) != 0)
{
    Log (TCP_ERROR, "error[%.11s:%.4s]",
        krx_hdr->MsgType, &DataBuff[sizeof(KRX_HEADER)]);
}
```

**Note**: `&DataBuff[sizeof(KRX_HEADER)]` is already using `sizeof` — keep as-is (no raw number).

### 3.3 Target Files — Conversion Table

| File | Existing Var | Action | Occurrences |
|------|-------------|--------|-------------|
| `pb_1100_ts.c` | `Header_Fmt` (declared) | Check if `Header_Fmt` already holds same data as DataBuff; if not, add `KRX_HEADER *krx_hdr = (KRX_HEADER *)DataBuff;` | ~6 |
| `pb_1200_tr.c` | `Header_Fmt` (declared) | Same approach | ~5 |
| `pb_1800_ts.c` | `Header_Fmt` (declared) | Same approach | ~6 |
| `pb_7800_tr.c` | `Header_Fmt` (declared, used for MsgSeqNum already) | Use `Header_Fmt` (already populated via memcpy) where possible; else use `krx_hdr` | ~6 |
| `pa_1100_ts.c` | Check if exists | Add `KRX_HEADER *krx_hdr = (KRX_HEADER *)DataBuff;` | ~3 |
| `pa_1200_tr.c` | Check if exists | Add pointer | ~2 |
| `pa_7800_tr.c` | Check if exists | Add pointer | ~1 |

### 3.4 Special Case: pa_3100_ts.c DataBuff[80]

**Finding**: `pa_3100_ts.c` is NOT in the active build (`Make_PA_ts.sh` does not include 3100). The `DataBuff[80]` syntax is also a **type error** — `memcmp(DataBuff[80], ...)` passes a `char` value (not a pointer) to `memcmp`. This would crash if compiled and executed.

**Decision**: **De-scope from mandatory changes** since the file is not built. Document as a known issue. If the file is ever reactivated, it needs both:
1. Fix syntax: `DataBuff[80]` → `&DataBuff[KRX_HEAD_LEN]`
2. Fix offset: 80 → 82 (current KRX_HEADER size)

### 3.5 Additional KRX Field Accesses in pa_3100_ts.c (informational)

Line 645: `DataBuff+14` (MsgType) — same as `DataBuff[8+6]` but using pointer arithmetic
Lines 647-654: `DataBuff+68`, `DataBuff+70`, `DataBuff+72`, `DataBuff+74`, `DataBuff+77` — accessing `SendingTime` and `DataCnt` fields

These would also benefit from struct access but are out of scope (file not built).

---

## 4. FR-03: DataBuff[82+4+i*d_size] → Named Constants

### 4.1 Problem

In `pb_7800_tr.c`, multi-item KRX body parsing uses raw offset calculations:

```c
/* BEFORE */
d_seq = AtoIf (&DataBuff[82+4 + i*d_size], 7);
memcpy (&w_data[70+82], &DataBuff[82+i*d_size], d_size);
```

The `82` = KRX_HEADER size, `4` = error code field size at start of body.

### 4.2 Design

```c
/* AFTER */
#define KRX_BODY_ERR_LEN    4    /* Error code field at start of body */

d_seq = AtoIf (&DataBuff[KRX_HEAD_LEN + KRX_BODY_ERR_LEN + i*d_size], 7);
memcpy (&w_data[sizeof(BUFF_RW_HEAD) + KRX_HEAD_LEN],
        &DataBuff[KRX_HEAD_LEN + i*d_size], d_size);
```

Or using the existing pointer approach:

```c
/* Alternative: pointer-based */
char *body_start = &DataBuff[KRX_HEAD_LEN];
char *item_start = &body_start[i * d_size];
char *item_seq   = &body_start[KRX_BODY_ERR_LEN + i * d_size];

d_seq = AtoIf(item_seq, 7);
```

### 4.3 Target Locations in pb_7800_tr.c

| Line | Current | After |
|------|---------|-------|
| 1173 | `DataBuff[82+4 + i*d_size]` | `DataBuff[KRX_HEAD_LEN + KRX_BODY_ERR_LEN + i*d_size]` |
| 1174 | same (in Log) | same replacement |
| 1329 | `&DataBuff[0]` + literal `82` | `DataBuff` + `KRX_HEAD_LEN` |
| 1330 | `dec_data[i*d_size]` | keep (already variable-based) |
| 1372-1373 | `DataBuff[82+4 + i*d_size]` | same as 1173 |
| 1382 | `DataBuff[82+ i*d_size]` | `DataBuff[KRX_HEAD_LEN + i*d_size]` |
| 1417 | `&DataBuff[0]` + literal `82` | same as 1329 |
| 1418 | `DataBuff[82+i*d_size]` | `DataBuff[KRX_HEAD_LEN + i*d_size]` |

### 4.4 Define Location

Add `KRX_BODY_ERR_LEN` either:
- **Option A**: In `pb_7800_tr.c` locally (file-scope `#define`) — simpler, less impact
- **Option B**: In `inc/pa_struct.h` alongside `KRX_HEAD_LEN` — more discoverable

**Selected**: Option A (local define) — this constant is only used in multi-item body parsing, which is specific to `pb_7800_tr.c`.

---

## 5. FR-05: ADD_HEADER_SIZE Dead Macro Cleanup

### 5.1 Problem

```c
/* Defined in 5 files, always 0 */
#define ADD_HEADER_SIZE 0    // 55=>0, 사용하지 않기로했음
```

Used as: `R_Fmt[i].Data[ADD_HEADER_SIZE+11]` → effectively `R_Fmt[i].Data[11]`

### 5.2 Design

1. Delete the `#define ADD_HEADER_SIZE 0` line
2. Replace all `ADD_HEADER_SIZE + N` with just `N`
3. Replace all `ADD_HEADER_SIZE` standalone with `0` (if any exist beyond addition)

### 5.3 Target Files

| File | #define Line | Usage Lines | Conversion |
|------|-------------|-------------|------------|
| `pa_1100_ts.c` | line 28 | 1288-1290, 1295-1297, 1302, 1318 | `Data[ADD_HEADER_SIZE+11]` → `Data[11]`, `ADD_HEADER_SIZE + sizeof(...)` → `sizeof(...)` |
| `pb_1100_ts.c` | line 28 | 1602-1604, 1609-1611, 1616, 1633 | same pattern |
| `pa_5000_qr.c` | line 30 | 175-180, 193-198, 209, 212 | `r_buf[ADD_HEADER_SIZE+11]` → `r_buf[11]`, `ADD_HEADER_SIZE + sizeof(...)` → `sizeof(...)` |
| `pa_1200_mp.c` | line 25 | 179, 181, 185, 187, 191, 193, 197, 199, 203, 205, 209, 211 | `p_buf[4+11 +ADD_HEADER_SIZE+ 11]` → `p_buf[4+11+11]` i.e. `p_buf[26]`, `p_buf[4+11 +ADD_HEADER_SIZE]` → `p_buf[15]` |

### 5.4 pa_1200_mp.c Special Case

The `pa_1200_mp.c` uses a compound offset: `p_buf[4+11 +ADD_HEADER_SIZE+ 11]` which equals `p_buf[26]`. After cleanup:

```c
/* BEFORE */
if (memcmp (&p_buf[4+11 +ADD_HEADER_SIZE+ 11], "TCHODR4000", 10) == 0)
    KRX_NOTE_JUMUN_DATA *dat = (KRX_NOTE_JUMUN_DATA *)&p_buf[4+11 +ADD_HEADER_SIZE];

/* AFTER */
if (memcmp (&p_buf[26], "TCHODR4000", 10) == 0)      /* 4(ResponseCode)+11(DataSeq)+11(TRCode) */
    KRX_NOTE_JUMUN_DATA *dat = (KRX_NOTE_JUMUN_DATA *)&p_buf[15];   /* 4(ResponseCode)+11(DataSeq) */
```

Better yet, document what 4+11+11 means with a comment:

```c
/* AFTER (with comments) */
/* p_buf layout: ResponseCode[4] + DataSeq[11] + Transaction_Code[11] + data... */
if (memcmp (&p_buf[4 + 11 + 11], "TCHODR4000", 10) == 0)     /* TR code at offset 26 */
    KRX_NOTE_JUMUN_DATA *dat = (KRX_NOTE_JUMUN_DATA *)&p_buf[4 + 11];  /* data at offset 15 */
```

---

## 6. FR-04: pa_3100_ts.c DataBuff[80] (De-scoped)

### 6.1 Finding

`pa_3100_ts.c` is **not in the active build** (`Make_PA_ts.sh` does not list 3100).

The `memcmp(DataBuff[80], "0000", 4)` syntax is a **type error**: `DataBuff[80]` is `char`, not `char*`. This would crash if compiled and executed.

### 6.2 Decision

**De-scoped**: No changes in this iteration. Document as known issue:
- Lines 525, 531, 1221, 1230: `DataBuff[80]` → should be `&DataBuff[KRX_HEAD_LEN]`
- Both the `&` prefix and the offset (80→82) are wrong

If the file is reactivated in the future, apply both fixes.

---

## 7. FR-06: Buff_RW_Head_Set() Helper

### 7.1 Design

Add to `inc/fep_file.h` before the `#endif`:

```c
/*------------------------------------------------------------------------
    Helper: populate BUFF_RW_HEAD fields
    Note: caller must memset bw_head to 0x20 before calling
------------------------------------------------------------------------*/
static void Buff_RW_Head_Set(
    BUFF_RW_HEAD    *bw,
    int             seq,
    int             if_seq,
    const char      *ap_type,
    const char      *resp_code,
    const char      *time_buf)      /* 22+ bytes: time1(10) + time2(12) */
{
    ItoAf(seq,      bw->Seq,            sizeof(bw->Seq));
    ItoAf(if_seq,   bw->If_Seq,         sizeof(bw->If_Seq));
    memcpy(bw->ApType,        ap_type,   sizeof(bw->ApType));
    memcpy(bw->ResponseCode,  resp_code, sizeof(bw->ResponseCode));
    memcpy(bw->RecvTime1,     time_buf,       sizeof(bw->RecvTime1));
    memcpy(bw->RecvTime2,     &time_buf[10],  sizeof(bw->RecvTime2));
}
```

### 7.2 Consideration: static inline vs sub/ function

| Option | Pros | Cons |
|--------|------|------|
| `static` in header | Zero overhead, no lib rebuild | Each .c gets a copy; `ItoAf` must be visible |
| Function in `sub/` | Single copy, standard pattern | Requires lib rebuild, extra call overhead |

**Selected**: `static` in `inc/fep_file.h` — matches the pattern that `BUFF_RW_HEAD` is already defined there. The function is small (6 lines) and called in hot paths where avoiding function call overhead is beneficial.

**Prerequisite**: The header must be included after `fep_sub.h` (which declares `ItoAf`). Check include order in each target file.

### 7.3 Usage Example (FR-01 simplified)

```c
/* AFTER with helper */
BUFF_RW_HEAD    bw_head;
memset(&bw_head, 0x20, sizeof(bw_head));
Buff_RW_Head_Set(&bw_head, d_seq, d_seq, ApType, RES_NORMAL, m_time);
memcpy(bw_head.DataHeader, &File_Data_Head, sizeof(FILE_DATA_HEAD));
memcpy(w_data, &bw_head, sizeof(BUFF_RW_HEAD));
```

### 7.4 Alternative: No Helper, Direct Struct Access

If the helper function adds complexity (include order issues, ItoAf dependency), each file can directly populate the struct fields without a helper. The key improvement is using struct field names instead of numeric offsets — the helper is a convenience, not a requirement.

---

## 8. Implementation Order

| Step | FR | Files | Dependency |
|------|-----|-------|------------|
| 1 | FR-06 | `inc/fep_file.h` | None — add helper first |
| 2 | FR-01 | `pa_2700_tr.c` | FR-06 — simplest w_data block (1 block) |
| 3 | FR-01 | `pa_7800_tr.c` | FR-06 — 1 block |
| 4 | FR-01 | `pb_7800_tr.c` | FR-06 — 3 blocks (most complex) |
| 5 | FR-03 | `pb_7800_tr.c` | After FR-01 for same file — DataBuff[82+...] |
| 6 | FR-02 | `pa_1200_tr.c` | None — fewest occurrences (~2) |
| 7 | FR-02 | `pa_7800_tr.c` | None — ~1 occurrence |
| 8 | FR-02 | `pa_1100_ts.c` | None — ~3 occurrences |
| 9 | FR-02 | `pb_1200_tr.c` | None — ~5 occurrences |
| 10 | FR-02 | `pb_1100_ts.c` | None — ~6 occurrences |
| 11 | FR-02 | `pb_1800_ts.c` | None — ~6 occurrences |
| 12 | FR-02 | `pb_7800_tr.c` | After FR-03 — ~6 occurrences |
| 13 | FR-05 | `pa_1100_ts.c` | After FR-02 for same file |
| 14 | FR-05 | `pb_1100_ts.c` | After FR-02 for same file |
| 15 | FR-05 | `pa_5000_qr.c` | None |
| 16 | FR-05 | `pa_1200_mp.c` | None |
| 17 | Build | `mk.sh sub && mk.sh pa && mk.sh pb` | All changes complete |

---

## 9. Build Verification

```sh
# Step 1: Rebuild library (if fep_file.h changed, all objects need rebuild)
mk.sh sub

# Step 2: Rebuild PA module
mk.sh pa

# Step 3: Rebuild PB module
mk.sh pb

# Full rebuild (alternative)
mk.sh all
```

Expected: Zero new warnings, zero errors. All ~72 binaries produced.

---

## 10. Risk Mitigations

| Risk | Mitigation |
|------|------------|
| `sizeof(BUFF_RW_HEAD)` != 70 | All fields are `char[]` → no alignment padding on any platform. But verify after first build. |
| Include order for `static Buff_RW_Head_Set()` | `ItoAf()` is declared in `fep_sub.h`, included via `fep_fepp.h` which all sources include first. `fep_file.h` is included separately. Ensure helper function is only used after `fep_sub.h` is included. |
| `Header_Fmt` vs `DataBuff` pointer cast | Some files already have `Header_Fmt` populated via `memcpy(Header_Fmt, DataBuff, sizeof(KRX_HEADER))`. In these cases, use `Header_Fmt.MsgType` directly instead of casting DataBuff. |
| `strlen(RES_NORMAL)` in helper | `RES_NORMAL` is `"0000"` (4 bytes). `sizeof(bw->ResponseCode)` = 4. Direct `memcpy` of 4 bytes is safe. |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
