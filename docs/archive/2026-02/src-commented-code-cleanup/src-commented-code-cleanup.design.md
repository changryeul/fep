# 설계: src-commented-code-cleanup

## 참조

- Plan: [src-commented-code-cleanup.plan.md](../../01-plan/features/src-commented-code-cleanup.plan.md)

## Implementation 개요

Remove all `//` and `/* */` commented-out code from active `src/` source files (PA, PB, PZ, PW modules). Pure deletion with zero functional change.

- **Implementation order**: PZ (5 files) -> PB (9 files) -> PA (15 files) -> PW (1 file)
- **Deletion order**: Bottom-to-top within each file to preserve line numbers
- **합계**: 30 files, 30 FR items

### Preservation Rules (범위 외)

- Korean descriptive comments (e.g., `// 변수 초기 필요`, `/* 내가낸 주문만 처리한다 */`)
- `// 2025EDIT` markers and edit timestamps
- Function description `/* */` blocks and section banners
- `/* */` inline field-size comments (e.g., `/* Seq (10) */`)
- Numbered step comments (e.g., `// 1. 복호화 호출`)
- `BACK2025/`, `BACKUP/` directory files

---

## Module 1: PZ (5 files, ~56 lines)

### FR-01: pz_memory_conf.c (~33 lines)

All `//sprintf` and `//Log` config loading artifacts from refactored config path logic.

| Lines | Pattern | Count |
|-------|---------|:-----:|
| 162, 169, 174 | `//sprintf (INFO...)` | 3 |
| 187, 194, 195, 209, 216, 217 | `//sprintf (DAEMON/FILEM...)` | 6 |
| 371, 379, 394 | `//sprintf`/`//Log` | 3 |
| 419, 420, 430, 431, 447, 448 | `//sprintf` config loading | 6 |
| 677, 692, 697 | `//sprintf` | 3 |
| 842, 899 | `//sprintf`/`//Log` | 2 |
| 1231, 1244 | `//sprintf` | 2 |
| 2048, 2244, 2416, 2545 | `//sprintf` | 4 |
| 2779, 2865, 2912, 2959 | `//sprintf` | 4 |

**Est. lines**: ~33

### FR-02: pz_daemon_proc.c (12 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 324, 393, 464, 536, 609 | `//sprintf (buf, "%s/%s", ...)` old path | 5 single |
| 680 | `//sigrelse (SIGCHLD);` | 1 single |
| 960, 961, 962 | `//sighold (SIGPOLL);` / `(SIGUSR1)` / `(SIGUSR2)` | 3 single |
| 1005, 1006, 1007 | `//sigrelse (SIGPOLL);` / `(SIGUSR1)` / `(SIGUSR2)` | 3 single |

**Est. lines**: 12

### FR-03: pz_fepp.c (6 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 124 | `//sigrelse (SIGTERM);` | 1 single |
| 311, 480, 610, 774, 937 | `//sprintf (buf, "%s/%s", ...)` old path | 5 single |

**Est. lines**: 6

### FR-04: pz_filechk.c (3 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 107 | `//sprintf (buf, "%s/%s", ...)` old path | 1 single |
| 141, 142 | `//sprintf` old path (2 lines) | 2 single |

**Est. lines**: 3

### FR-05: pz_procchk.c (2 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 922 | `//Used[i] = ... SPARROW` disabled assignment | 1 single |
| 1044 | `//sprintf` path | 1 single |

**Est. lines**: 2

---

## Module 2: PB (9 files, ~160 lines)

### FR-06: pb_1100_ts.c (~18 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 18-21 | `/* #include "fep_fepj.h" / "krx_struct.h" */` | 4-line block |
| 476 | `//back_rd_cnt = RD_CNT;` | 1 single |
| 487-490 | `/* Seq 원복 RD_CNT = back_rd_cnt... */` | 4-line block |
| 584 | `//return;` | 1 single |
| 616-621 | `//mun`, `//INT_SEQ`, `//RD_CNT`, `// 20251115`, `//SHM_DATA`, `//PROC` | 6 single |
| 1082 | `//memcpy(J_Q_Fmt.JumunData...)` | 1 single |
| 1108 | `//return (Size_Len);` | 1 single |

**Est. lines**: 18

### FR-07: pb_1200_tr.c (~21 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 19-22 | `/* #include "fep_fepj.h" / "krx_struct.h" */` | 4-line block |
| 79-81 | `//extern net_ctx`, `//#define CLIENT_CTX`, `//#define KRX_INITECH_CONF_PATH` | 3 single |
| 111 | `//FILE_BUFF_FORMAT R_Fmt[MAX_CNT]...` | 1 single |
| 113 | `//KRX_JUMUN_Q_FMT J_Q_Fmt...` | 1 single |
| 285 | `//TCP2_PROC_ST = ON;` | 1 single |
| 289 | `//TCP2_LINE_ST = OFF;` | 1 single |
| 299-301 | `// memset/sprintf/Log` INITECH config | 3 single |
| 584 | `//memcmp(... "SCHOPR00000" ...)` disabled comparison | 1 single |
| 760-762 | `/* #if defined B1201 \|\| B1211 */` | 3-line block |
| 777 | `//rt = F_W (p_flag * 10, ...)` | 1 single |
| 863 | `//memcpy (&KR_Fmt.Data[10], LOGON_PW..., 30)` | 1 single |

**Est. lines**: 21

### FR-08: pb_1800_ts.c (~87 lines) -- LARGEST FILE

**`/* */` blocks (52 lines):**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 18-21 | `/* #include "fep_fepj.h" / "krx_struct.h" */` | 4 |
| 207-214 | `/* ConnectRetryCnt++; Line_Change(); */` | 8 |
| 423-426 | `/* memset/memcpy J_R_Fmt → S_Fmt */` | 4 |
| 519-531 | `/* read/break/Log FIFO disabled loop */` | 13 |
| 1045-1059 | `/* if memcmp LogOn/LIOK/HeartBeat → unified */` | 15 |
| 1260-1267 | `/* if MsgLen == -2 Skip(REJC) */` | 8 |

**`//` lines (35 lines):**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 17 | `//#include "INISAFENet.h"` | 1 |
| 75-77 | `//extern net_ctx`, `//#define CLIENT_CTX`, `//#define KRX_INITECH_CONF_PATH` | 3 |
| 365-366 | `//INL_Initialize(...)`, `//INL_New_Ctx(...)` | 2 |
| 449 | `//else if (FirstSeq == INT_SEQ)` | 1 |
| 469-472 | `//Make_Send_Msg/memset/memcpy/Device_Write` | 4 |
| 502 | `//memcpy (DataBuff, &J_Q_Fmt, SendLen)` | 1 |
| 545 | `//void Free_All (ctxf)` old signature | 1 |
| 601,610,620,637,648,657,670,685,696,705,718 | `//Free_All(1);` x11 | 11 |
| 827 | `//return;` | 1 |
| 838 | `//if (IS_RESP_OK(DataBuff))` | 1 |
| 857 | `//RD_CNT = INT_SEQ = FirstSeq;` | 1 |
| 912-913 | `//INL_CtxFree(EnCtx);` / `//Free_All(1);` | 2 |
| 933 | `//rt = Select_Send (Sockfd, ...)` | 1 |
| 946 | `//Log (TCP_OK, "TCP SD ...")` | 1 |
| 1106 | `//memcpy (W_Fmt[i].Seq, ...)` | 1 |
| 1128 | `//memcpy (&W_Fmt[i].Data[...], S_Data, 420)` | 1 |
| 1217 | `//memcpy (&S_Fmt.Data[10], LOGON_PW..., 30)` | 1 |
| 1361 | `//memcpy(J_Q_Fmt.JumunData...)` | 1 |
| 1382 | `//return (Size_Len);` | 1 |

**Est. lines**: 87

### FR-09: pb_7100_ts.c (3 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 100 | `//t = mktime (&tm);` | 1 single |
| 204 | `//Device_Close ();` | 1 single |
| 358 | `//close (Newfd);` | 1 single |

**Est. lines**: 3

### FR-10: pb_7100_ur.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 10 | `//#include <sys/ioctl.h>` | 1 single |

**Est. lines**: 1

### FR-11: pb_7200_tr.c (11 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 85 | `//t = mktime (&tm);` | 1 single |
| 182 | `//Device_Close ();` | 1 single |
| 213 | `//R_CNT (0,1) = W_CNT (0,0);` | 1 single |
| 325 | `//close (Newfd);` | 1 single |
| 408-414 | `/* int rt; Make_Send_Msg; Device_Write */` | 7-line block |

**Est. lines**: 11

### FR-12: pb_7800_tr.c (~30 lines)

**`/* */` blocks:**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 18-21 | `/* #include "fep_fepj.h" / "krx_struct.h" */` | 4 |
| 306-310 | `/* INT_SEQ = 0 initialization */` | 5 |
| 698-701 | `/* if d_seq comparison */` | 4 |
| 1106-1109 | `/* memcpy/ItoAf data block */` | 4 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 50 | `//#define KRX_INITECH_CONF_PATH` | 1 |
| 245 | `//TCP2_PROC_ST = ON;` | 1 |
| 249 | `//TCP2_LINE_ST = OFF;` | 1 |
| 309 | `//memcpy (PROC... curr_tr)` | 1 |
| 312 | `//if (memcmp... "TRDESP50101")` | 1 |
| 322 | `//else if (memcmp... "TRDESP50102")` | 1 |
| 571 | `//FirstSeq = AtoIf (...)` | 1 |
| 721, 771, 883, 972 | `//Log SEQ Add` x4 | 4 |
| 914 | `//rt = F_W (...)` | 1 |
| 999 | `//memcpy (&w_data...)` | 1 |
| 1004 | `//rt = F_W (...)` | 1 |
| 1013-1014 | `//Log` 2-line | 2 |

**Est. lines**: ~30

### FR-13: pb_8100_ts.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 104 | `//t = mktime (&tm);` | 1 single |

**Est. lines**: 1

### FR-14: pb_8200_tr.c (~15 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 94 | `//t = mktime (&tm);` | 1 single |
| 98-110 | `/* if weekend/business_day check */` | ~13-line block |
| 394 | `//TCP2_LINE_ST = OFF;` | 1 single |

**Est. lines**: ~15

---

## Module 3: PA (15 files, ~130 lines)

### FR-15: pa_1100_ts.c (~30 lines)

**`/* */` blocks (from agent scan, verify in Do phase):**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 18-20 | `/* #include */` | 3 |
| 407-408 | `/* memset/memcpy */` | 2 |
| 448-451 | `/* Seq skip */` | 4 |
| 459-461 | `/* Seq skip */` | 3 |
| 505-512 | `/* Seq skip */` | 8 |
| 752-754 | `/* Seq skip */` | 3 |
| 801-803 | `/* Seq skip */` | 3 |
| 904-910 | `/* Skip check */` | 7 |
| 965-970 | `/* Skip check */` | 6 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 88 | `//KRX_NOTE...` | 1 |
| 323, 327 | `//TCP2_PROC_ST/LINE_ST` | 2 |
| 441 | `//else if` | 1 |
| 569 | `//return;` | 1 |
| 810-811 | `//Dshm_Add_Count` | 2 |

**Est. lines**: ~30+ (verify exact block boundaries in Do phase)

### FR-16: pa_1200_tr.c (~13 lines)

**`/* */` blocks:**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 18-20 | `/* #include */` | 3 |
| 650-654 | `/* if rt != 1 */` | 5 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 91, 93 | `//FILE_BUFF`/`//KRX_JUMUN` variable decls | 2 |
| 261, 265 | `//TCP2_PROC_ST/LINE_ST` | 2 |
| 294 | `//RecvLen` | 1 |

**Est. lines**: ~13

### FR-17: pa_1290_mp.c (~45 lines)

**`/* */` blocks (Korean-annotated dead code):**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 269-274 | `/* 전략구축시... for loop */` disabled loop | 6 |
| 738-740 | `/* 2025 LP... Real_Modify_Cancel_Cnt */` | 3 |
| 772-774 | `/* 2025 LP... Real_Modify_Cancel_Cnt */` | 3 |
| 786-789 | `/* edt_gum = AtoLf... */` disabled calculation | 4 |
| 833-836 | `/* 2025 LP... Real_Modify_Cancel_Cnt */` | 4 |
| 843-846 | `/* org_gum = AtoLf... */` disabled calculation | 4 |
| 986-995 | `/* 2025 LP... Jan_Cnt/MeChe_Cnt */` | 10 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 257 | `//Log (USR_ERROR, "타매체..."` old format | 1 |
| 322 | `//Shm_Mk_PreMatch...Str_No` | 1 |
| 714 | `//Log (USR_ERROR, "타매체..."` old format | 1 |
| 743 | `//Shm_Mk_PreMatch...Mk_gbn` | 1 |
| 750 | `//Shm_Mk_PreMatch...Str_No` | 1 |
| 853 | `//org_cnt01 = AtoLf(...)` | 1 |
| 856 | `//org_cnt02 = AtoLf(...)` | 1 |
| 908 | `//Shm_Risk...item_tot_sugum` | 1 |
| 914 | `//Shm_Risk...item_cha_sugum` | 1 |
| 958 | `//Shm_Risk...item_cha_sugum` | 1 |

**Est. lines**: ~45

### FR-18: pa_1600_tr.c (~20 lines)

**`/* */` blocks (from agent scan):**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 98-108 | `/* business_day check */` | 11 |
| 157-160 | `/* PktType */` | 4 |
| 560-561 | `/* TTRMIP check */` | 2 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 86 | `//t = mktime (&tm);` | 1 |
| 89 | `//if weekend` | 1 |
| 576 | `//memcpy` | 1 |

**Est. lines**: ~20

### FR-19: pa_2100_ts.c (~40 lines)

**`/* */` blocks (from agent scan):**

| Line(s) | Content | Lines |
|----------|---------|:-----:|
| 193-198 | `/* MaxCnt */` | 6 |
| 373-376 | `/* */` block | 4 |
| 683-686 | `/* */` block | 4 |
| 732-735 | `/* */` block | 4 |
| 786-791 | `/* */` block | 6 |
| 849-852 | `/* */` block | 4 |
| 962-972 | `/* */` block | 11 |

**`//` lines:**

| Line(s) | Content | Count |
|----------|---------|:-----:|
| 112 | `//t = mktime (&tm);` | 1 |
| 978 | `//Set_TR_Time ()` | 1 |

**Est. lines**: ~40+ (verify exact block boundaries in Do phase)

### FR-20: pa_2200_tr.c (15 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 41 | `//IMECO_TCP_HEAD *R_Pkt...` disabled variable | 1 single |
| 93 | `//t = mktime (&tm);` | 1 single |
| 104-114 | `/* business_day check */` | 11-line block |
| 179 | `//MaxCnt = TCP_DATA_LEN / ...` | 1 single |

**Est. lines**: ~15 (includes 1 line from block boundary check)

### FR-21: pa_2700_tr.c (15 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 74 | `//t = mktime (&tm);` | 1 single |
| 85-95 | `/* business_day check */` | 11-line block |
| 121-126 | `/* if rt == NOTOK/FAIL */` | 6-line block |
| 162 | `//MaxCnt = TCP_DATA_LEN / ...` | 1 single |

**Est. lines**: ~19

### FR-22: pa_5000_qr.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 94 | `//Msgbuf r_buf;` disabled variable | 1 single |

**Est. lines**: 1

### FR-23: pa_5200_qs.c (~33 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 98-118 | `/* FIFO_fd open/FIFO_fd1 open */` disabled FIFO setup | 21-line block |
| 142-151 | `/* QueueClear(...) */` disabled queue clear | 10-line block |
| 161 | `//sleep (5);` | 1 single |
| 186 | `//rt = msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT);` | 1 single |

**Est. lines**: ~33

### FR-24: pa_7000_tr.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 213 | `//Device_Close ();` | 1 single |

**Est. lines**: 1

### FR-25: pa_7100_dd.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 497 | `//idx = AtoIf (&p_buf[0], ...)` | 1 single |

**Est. lines**: 1

### FR-26: pa_7100_ts.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 198 | `//Device_Close ();` | 1 single |

**Est. lines**: 1

### FR-27: pa_7100_ur.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 303 | `//rt = setsockopt (Sockfd, SOL_SOCKET, SO_RCVBUF, ...)` | 1 single |

**Est. lines**: 1

### FR-28: pa_7800_tr.c (2 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 214 | `//TCP2_PROC_ST = ON;` | 1 single |
| 218 | `//TCP2_LINE_ST = OFF;` | 1 single |

**Est. lines**: 2

### FR-29: pa_8200_tr.c (1 line)

| Line(s) | Content | Type |
|----------|---------|------|
| 231 | `//Device_Close ();` | 1 single |

**Est. lines**: 1

---

## Module 4: PW (1 file, ~2 lines)

### FR-30: pw_1000_mp.c (2 lines)

| Line(s) | Content | Type |
|----------|---------|------|
| 164 | `//if (fifo_fd == -1)` replaced by `if (fifo_fd < 0)` | 1 single |
| 182 | `//if (fd == -1)` replaced by `if (fd < 0)` | 1 single |

**Est. lines**: 2

---

## 요약

| Module | Files | FR 항목 | Est. Lines |
|--------|:-----:|:--------:|:----------:|
| PZ | 5 | FR-01~05 | ~56 |
| PB | 9 | FR-06~14 | ~188 |
| PA | 15 | FR-15~29 | ~145 |
| PW | 1 | FR-30 | 2 |
| **Total** | **30** | **30** | **~391** |

Note: Line counts exceed the Plan estimate (~210+) due to additional `/* */` multi-line blocks and `//Free_All(1);` patterns discovered during detailed file inspection. The Plan estimate was based on grep pattern scanning; actual block boundaries are larger.

## Implementation Batches

| 배치 | Module | Files | Notes |
|:-----:|--------|:-----:|-------|
| 1 | PZ | 5 | High volume in pz_memory_conf.c (33 lines) |
| 2 | PB | 9 | pb_1800_ts.c is largest single file (87 lines) |
| 3 | PA | 15 | Most files but many are 1-3 lines each |
| 4 | PW | 1 | Minimal (2 lines) |

Each batch: read file -> verify line numbers -> delete bottom-to-top -> verify no functional code removed.

## 검증

- `mk.sh src` should produce identical binaries (comments have no effect on compilation)
- Zero functional code changes
- All descriptive comments, Korean annotations, and `// 2025EDIT` markers preserved
