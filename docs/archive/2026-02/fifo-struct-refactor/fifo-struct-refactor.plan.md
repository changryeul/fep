# fifo-struct-refactor 계획 문서

> **요약**: Refactor FIFO/SAM file I/O buffer construction and KRX message parsing from raw byte offsets to struct-based type-safe access
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft

---

## 1. 개요

### 1.1 목표

Replace hardcoded byte offset patterns (`w_data[0]`, `w_data[8]`, `w_data[50]`, `DataBuff[8+6]`, `DataBuff[82+4]`, `DataBuff[80]`) with struct-based field access when constructing FIFO/SAM file records and parsing KRX messages. This is the natural continuation of `tr-struct-refactor` (KRX protocol structs) and `shm-struct-refactor` (SHM access).

### 1.2 배경

The FEP file I/O layer uses a signal+data architecture:
- **FIFO**: Carries only 1-byte wakeup signals (`"1"`) between processes
- **SAM files**: Store actual record data in `FILE_BUFF_FORMAT` (header + data + LF)

The FIFO/signal mechanism itself is clean. However, the code that **constructs write buffers** and **parses read buffers** for SAM file records still uses raw byte offsets in multiple source files:

1. `w_data[0..70]` — manually builds `BUFF_RW_HEAD` (50 bytes) + `FILE_DATA_HEAD` (20 bytes) using literal numeric offsets instead of populating struct fields
2. `DataBuff[8+6]` — accesses `KRX_HEADER.MsgType` via raw offset (8=BeginString, 6=BodyLength) instead of struct cast
3. `DataBuff[82+4+i*d_size]` — parses multi-item KRX body at literal offset 82 (header) + 4 (error code)
4. `DataBuff[80]` — uses **wrong** legacy header size (80 vs current 82) in `pa_3100_ts.c`
5. `ADD_HEADER_SIZE` — dead macro (always 0) still used in 5 files

### 1.3 관련 문서

- Predecessor: `docs/archive/2026-02/tr-struct-refactor/` (KRX message struct, 92%)
- Predecessor: `docs/archive/2026-02/shm-struct-refactor/` (SHM access, 95%)
- Headers: `inc/fep_file.h` (BUFF_RW_HEAD, FILE_RW_HEAD), `inc/buf_struct.h` (FILE_BUFF_FORMAT)
- Headers: `inc/pa_struct.h` (KRX_MSG_COMMON, KRX_HEADER), `inc/fep_tcpip.h` (FILE_DATA_HEAD)

---

## 2. 범위

### 2.1 포함 범위

- [ ] FR-01: `w_data[]` raw BUFF_RW_HEAD construction → struct-based population (3 files, 5 blocks)
- [ ] FR-02: `DataBuff[8+6]` KRX MsgType access → KRX_HEADER struct cast (7 files, ~30 occurrences)
- [ ] FR-03: `DataBuff[82+...]` raw body parsing → KRX_HEAD_LEN + struct offset (1 file, pb_7800_tr.c)
- [ ] FR-04: `DataBuff[80]` legacy header size fix → KRX_HEAD_LEN (1 file, pa_3100_ts.c — potential bug)
- [ ] FR-05: `ADD_HEADER_SIZE` dead macro cleanup (5 files)
- [ ] FR-06: Helper macro/function for BUFF_RW_HEAD population in `sub/` or `inc/`

### 2.2 제외 범위

- FIFO signal mechanism itself (1-byte wakeup — already clean)
- `file_rw.c` library internals (FILE_RW_HEAD ↔ BUFF_RW_HEAD conversion — already struct-based)
- `poll_file.c` / `select_recv.c` / `select_send.c` (no byte offset issues)
- `BACK2025/` and `JC_OLD/` backup files
- `R_Fmt[i].Data[...]` business data parsing (varies per TR type, uses typed struct casts already in some places — separate effort)

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | Replace `w_data[0/8/16/24/28/38/50/70]` raw BUFF_RW_HEAD construction with struct field assignment in `pb_7800_tr.c` (3 blocks), `pa_7800_tr.c` (1 block), `pa_2700_tr.c` (1 block) | High | Pending |
| FR-02 | Replace `DataBuff[8+6]` (=MsgType at offset 14) with `KRX_HEADER*` struct cast in `pb_1800_ts.c`, `pb_1100_ts.c`, `pb_1200_tr.c`, `pb_7800_tr.c`, `pa_1100_ts.c`, `pa_1200_tr.c`, `pa_7800_tr.c` | High | Pending |
| FR-03 | Replace `DataBuff[82+4+i*d_size]` multi-item body parsing with `KRX_HEAD_LEN` constant + named struct offset in `pb_7800_tr.c` | High | Pending |
| FR-04 | Fix `DataBuff[80]` → `DataBuff[KRX_HEAD_LEN]` in `pa_3100_ts.c` (lines 525, 531, 1221, 1230) — verify if this is a bug or intentional for a different protocol | High | Pending |
| FR-05 | Remove `ADD_HEADER_SIZE` macro (defined as 0) and simplify all usages in `pa_1100_ts.c`, `pb_1100_ts.c`, `pa_5000_qr.c`, `pa_1200_mp.c`, `pa_1290_mp.c` | Medium | Pending |
| FR-06 | Create `Buff_RW_Head_Set()` helper function or macro in `inc/fep_file.h` to populate BUFF_RW_HEAD fields from (seq, if_seq, aptype, resp_code, time) params | Medium | Pending |

### 3.2 비기능 요구사항

| 분류 | 기준 | 측정 방법 |
|----------|----------|-------------------|
| Binary compatibility | All ~72 binaries build successfully on Linux | `mk.sh all` clean build |
| Functional equivalence | No behavioral change — byte-identical output buffers | Manual comparison of struct sizeof vs hardcoded sizes |
| Zero regression | Existing FIFO signal/data flow unchanged | Code review of F_R/F_W call sites |

---

## 4. 성공 기준

### 4.1 완료 정의

- [ ] All FR-01..FR-06 implemented
- [ ] `mk.sh all` builds without errors
- [ ] No raw numeric offsets (0, 8, 16, 24, 28, 38, 50, 70) for BUFF_RW_HEAD fields in target files
- [ ] No `DataBuff[8+6]` pattern remaining in target files
- [ ] No `ADD_HEADER_SIZE` macro usage remaining
- [ ] `DataBuff[80]` corrected or documented as intentional

### 4.2 품질 기준

- [ ] PDCA gap analysis match rate >= 90%
- [ ] No new compiler warnings introduced
- [ ] Struct field access uses `sizeof()` for size calculations where applicable

---

## 5. 위험 및 완화

| 위험 | 영향 | 가능성 | 완화 |
|------|--------|------------|------------|
| `DataBuff[80]` in pa_3100_ts.c may be intentional for IMECO protocol (different header size) | High | Medium | Read pa_3100_ts.c fully in Design phase to determine protocol context before changing |
| `w_data[]` struct replacement may have alignment/padding issues | High | Low | Verify `sizeof(BUFF_RW_HEAD)` == 50 (no padding) with static assert; all fields are char arrays |
| `ADD_HEADER_SIZE` removal may break if any variant still uses non-zero value | Medium | Low | Grep all defines — confirmed all are `#define ADD_HEADER_SIZE 0` |
| Multi-item body parsing in pb_7800_tr.c (encrypted vs non-encrypted paths) is complex | Medium | Medium | Carefully map both code paths in Design; change offsets only, not logic |
| Changes span PA and PB modules — different build targets | Low | Low | Test with `mk.sh pa` and `mk.sh pb` separately before `mk.sh all` |

---

## 6. 아키텍처 고려사항

### 6.1 프로젝트 레벨

This is a **production trading system** — pure C89, multi-platform, no framework. Architecture level is **Enterprise** (real-time exchange connectivity).

### 6.2 핵심 아키텍처 결정

| 결정 | 옵션 | 선택 | 근거 |
|----------|---------|----------|-----------|
| BUFF_RW_HEAD population | (a) Direct struct assignment (b) Helper function (c) Helper macro | (b) Helper function | Consistent with shm-struct-refactor pattern (Shm_Map_SubDaemon); function allows type checking |
| KRX header access | (a) Cast DataBuff to KRX_HEADER* (b) Overlay union (c) memcpy to local struct | (a) Cast | Already used in tr-struct-refactor; zero-copy, no behavior change |
| ADD_HEADER_SIZE cleanup | (a) Remove macro, use 0 literal (b) Remove macro, remove +0 entirely | (b) Remove entirely | Cleaner code; `Data[0+11]` → `Data[11]` |

### 6.3 Target Files Summary

| File | Module | FR | Change Type |
|------|--------|-----|-------------|
| `pb_7800_tr.c` | PB | FR-01, FR-02, FR-03 | w_data[] + DataBuff[8+6] + DataBuff[82+...] |
| `pa_7800_tr.c` | PA | FR-01, FR-02 | w_data[] + DataBuff[8+6] |
| `pa_2700_tr.c` | PA | FR-01 | w_data[] |
| `pb_1800_ts.c` | PB | FR-02 | DataBuff[8+6] |
| `pb_1100_ts.c` | PB | FR-02, FR-05 | DataBuff[8+6] + ADD_HEADER_SIZE |
| `pb_1200_tr.c` | PB | FR-02 | DataBuff[8+6] |
| `pa_1100_ts.c` | PA | FR-02, FR-05 | DataBuff[8+6] + ADD_HEADER_SIZE |
| `pa_1200_tr.c` | PA | FR-02 | DataBuff[8+6] |
| `pa_3100_ts.c` | PA | FR-04 | DataBuff[80] → KRX_HEAD_LEN |
| `pa_5000_qr.c` | PA | FR-05 | ADD_HEADER_SIZE cleanup |
| `pa_1200_mp.c` | PA | FR-05 | ADD_HEADER_SIZE cleanup |
| `inc/fep_file.h` | INC | FR-06 | Add Buff_RW_Head_Set() helper |

**Total**: 11 source files + 1 header = 12 files

---

## 7. 규칙 사전 조건

### 7.1 Existing Conventions (from CLAUDE.md)

- [x] Pure C89/ANSI C — no C++ features
- [x] Struct naming: `*_FMT`, `*_S` suffixes
- [x] KRX message access: Use `KRX_MSG_COMMON` struct, not raw byte offsets
- [x] Tab width: 4 spaces
- [x] Comments: Mix of Korean and English

### 7.2 Pattern Consistency

Follow established patterns from previous refactors:
- **tr-struct-refactor**: `KRX_HEADER *hdr = (KRX_HEADER *)DataBuff;` then `hdr->MsgType`
- **shm-struct-refactor**: `Shm_Map_SubDaemon()` helper function pattern

---

## 8. Next Steps

1. [ ] Write design document (`fifo-struct-refactor.design.md`)
2. [ ] Investigate `pa_3100_ts.c` `DataBuff[80]` context (IMECO vs KRX protocol)
3. [ ] Verify `sizeof(BUFF_RW_HEAD)` == 50 (no padding)
4. [ ] Start implementation

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
