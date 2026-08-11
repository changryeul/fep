# Gap Analysis History - Detailed Records

## Config-DB Migration (2026-02-18)
- Design: `docs/02-design/features/config-db-migration.design.md`
- DB layer: `st01/sub/config_db.c` (~2677 lines), `st01/inc/config_db.h`
- Loader: `st01/sub/config_loader.c` (517 lines), `st01/inc/config_loader.h`
- Tools: `st01/utl/ini2db.c`, `st01/utl/db2ini.c`, `st01/utl/cfg_verify.c`
- Integration: `st01/src/PZ/pz_memory_proc.c` (Shm_Conf_Process calls Config_Load_All)
- Known Bug: `config_db.c:2251` queries `ip_addr` but schema column is `ip_address` in fep_ip_whitelist table
- Patterns: Design uses pseudocode; implementation has production-quality C with full SHM post-processing
- sort_order column added to fep_config (not in design) for deterministic ordering
- sisetr disabled since 202201 -- stub functions return CFG_DB_OK
- Daemon config loaded separately from other groups (architectural refinement)
- cfg_verify uses field-by-field comparison, not byte-level memcmp as designed

## TR Struct Refactor (2026-02-18)
- Initial Match Rate: 72% -> After Act-1: 89% (13 fixes applied, all verified)
- All critical/high-priority issues resolved: ttt prefix, DataBuff[82], #include concat, handshake "0000"
- Remaining: struct-level literal strings (KR_Fmt.Data/"0000", Header_Fmt.MsgType/"TRDESP501xx") -- cosmetic
- TR_SESSION_DATA now consumed in pa_1100_ts.c (lines 814, 1175)
- `pa_5010_mp.c` template conversion (design Phase 4 item 7) not yet started
- Pattern: `#if 0` dead code blocks contain unconverted literals -- acceptable, do not count as gaps

## FIFO Struct Refactor (2026-02-21)
- Match Rate: 95% (PASS)
- FR-01 (w_data[] struct): 100% -- all 4 blocks in 3 files converted to BUFF_RW_HEAD f_head
- FR-02 (DataBuff[8+6]): 93% -- 1 residual `DataBuff+14` in pb_1100_ts.c:1078
- FR-03 (DataBuff[82+]): 100% -- all replaced with KRX_HEAD_LEN in pb_7800_tr.c
- FR-04 (DataBuff[80]): de-scoped (pa_3100_ts.c not built)
- FR-05 (ADD_HEADER_SIZE): 100% -- removed from all 4 files
- FR-06 (Helper function): alternative chosen (direct struct access per design section 7.4)
- Residual: DataBuff+68/70/72/74/77 in pa_1100_ts.c/pb_1100_ts.c (SendingTime offsets, not in scope)
- Pattern: `DataBuff+N` pointer arithmetic is same anti-pattern class as `DataBuff[N]` -- check both in future

## PA-PB Dedup Phase 1 (2026-02-21)
- Overall Match Rate: 90% (PASS)
- Design Match: 87%, Architecture: 95%, Convention: 98%
- FR-01 to FR-05 (Get_Msec, Line_Change, Device_Read, Device_Write, Device_Close_Base): all 100%
- FR-06 (Device_Close pattern): 90% -- PA uses wrapper function, not #define macro as designed
- FR-07 (Err_Msg): 65% -- LARGEST GAP. Design pseudocode was inaccurate vs actual production logic
  - Error code-to-message mapping completely different (design had abstract categories, code has KRX-specific)
  - FirstSeq-- rollback in cases 101-103 missing from design (critical production logic)
  - ErrCd=0 before Log_Out in case 102 missing from design
  - sleep(NO_TIME) vs sleep(2) attribution incorrect in design
- FR-08 (header): 88% -- NoTime[] and Device_Close extern added (justified by wrapper approach)
- SLog->Log: 100% complete in pa_1100_ts.c and pa_1200_tr.c
- Include: fep_common.c uses fep_fepp.h (not fep_sub.h) for SHM macro access -- correct adaptation
- Pattern: Design pseudocode for complex switch statements often simplified/idealized; always verify against actual source
- Action: Design document Section 8.2 (Err_Msg) needs updating to match implementation

## PA-PB Dedup Phase 2 (2026-02-21)
- Overall Match Rate: 97% (PASS)
- Design Match: 96%, Architecture: 100%, Convention: 98%
- S2 FmtPtr: 100% -- all 6 files define FmtPtr correctly, extern in fep_common.h
- S3 Log_Out_Base: 100% -- exact match with design, FmtPtr/IS_RESP_OK/KRX_ERRCODE_LEN all correct
- S4 Device_Open_Logon: 100% -- all 6 caller sites match (0,0)/(0,1)/(1,0)/(1,1) pattern
- S5 Handshake extraction: 100% -- fep_encrypt.c matches design, all constants standardized
- S6 Free_All extraction: 100% -- byte-for-byte identical
- S7 Headers: 93% -- 2 minor diffs: EnCtx pointer type (design doc error), KRX_INITECH_CONF_PATH extern added
- PA stubs: 100% -- all 3 PA files have `int Handshake(void) { return (NOTOK); }`
- PB removal: 100% -- Free_All/Handshake removed from all 3 PB files with comment
- pb_1100_ts Log_Out wrapper: 100% -- checks Log_Out_Base()==OK then PROC status
- Phase 2 design quality much better than Phase 1 (97% vs 90%) -- "actual code citation" lesson applied
- Remaining design updates: EnCtx type (Section 6.2), KRX_INITECH_CONF_PATH (Section 7.2)

## PA-PB Dedup Phase 3 (2026-02-21)
- Overall Match Rate: 100% (PASS) -- perfect score
- All 13 verification checklist items: 12 PASS, 1 NOT TESTED (build -- requires server)
- FR-01 (fep_common.h): 100% -- 2 new prototypes added at correct location
- FR-02 (Time_Out_Disconnect): 100% -- byte-for-byte match with design
- FR-03 (Fifo_Event_Rtn): 100% -- byte-for-byte match with design
- All 6 process files: local Fifo_Event_Rtn removed, forward decls removed, Time_Out_Rtn simplified
- Dead code: all `#if 0` blocks and commented-out disconnect code removed from Time_Out_Rtn
- Unused `int rt;` removed from all 6 Time_Out_Rtn functions
- Write_Data de-scope honored (no changes made)
- Cumulative: 12 functions in shared lib, ~1,921 lines removed (22% reduction from 8,843 baseline)
- Pattern: "actual code citation" approach yields 100% match -- confirms Phase 2 lesson
- Trend: Phase 1=90%, Phase 2=97%, Phase 3=100% -- design quality improving each iteration

## Fifo-Event-Rtn Adoption (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- All 16 verification checklist items: 16 PASS, 0 FAIL
- FR-01 (fifo_event.c): 100% -- new file with correct body, includes fep_fepp.h
- FR-02 (fep_common.c): 100% -- Fifo_Event_Rtn definition removed, "End of Program" intact
- FR-03-unchanged (fep_common.h): 100% -- `extern void Fifo_Event_Rtn(void)` at line 40 unchanged
- FR-03-01 to FR-03-13: all 100% -- definitions removed, forward declarations kept at exact line numbers
- Files affected: 7 PA, 4 PB, 1 PW
- Build verification: NOT TESTED (requires server)

## Socket-Linger Extraction (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- All 6 verification checklist items: 6 PASS, 0 FAIL
- LIB-01 (fep_common.h): 100%, LIB-02 (fep_common.c): 100%
- FR-01 to FR-15: Groups A-D all correct
- 12 active call sites, 3 dead code files, ~357 net lines removed

## Dead Code Cleanup (2026-02-22)
- Overall Match Rate: 98% (PASS)
- Design: 31 files, ~60 blocks, ~1,895 lines; Actual: 36 files, ~71 blocks, ~1,945 lines
- All 31 FR items: PASS. 4 bonus files found.
- Zero residual `#if 0` or `#if (0)` in active .c files
- Pattern: When scanning for preprocessor patterns, use regex `#if\s+0|#if\s+\(0\)` not literal grep

## PA-PB 8100 Cleanup (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 8 FR items: all PASS. -62 lines total.

## Commented Code Cleanup Round 2 (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 47 FR items across 14 files: all PASS. Plus 2 BONUS items.

## Sub Commented Code Final (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 6 FR items across 3 files: all PASS. 39 lines removed.

## Duplicate Code Extraction (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 3 new shared functions: format_ip_addr, init_udp_socket, detect_poll_event
- All 3 function bodies match design byte-for-byte. Net reduction: ~154 lines.

## Getenv NULL Check (2026-02-22)
- Overall Match Rate: 98% (PASS)
- 29 FR items: 28 PASS, 1 N/A (FR-07: target line was inside comment block)
- Bug fix: pb_7100_ur.c:88 had "FEP_DIV" (missing underscore)
- Pattern: Always check if getenv targets are inside comment blocks before counting as active

## Header C++ Comment C89 (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 19 FR items across 10 header files: all PASS. 237 `//` comments converted.
- Pattern: `//` inside `/* */` block comments are text, not active C++ comments

## Unsafe strcpy Conversion (2026-02-22)
- Overall Match Rate: 100% (PASS) -- perfect score
- 12 FR items across 11 files: all PASS. 53 active calls converted.

## Signal Handler Safety (2026-02-23)
- Overall Match Rate: 100% (PASS) -- perfect score
- 15 FR items across 12 files: all PASS
- Design: `docs/02-design/features/signal-handler-safety.design.md`
- Infrastructure: volatile sig_atomic_t _in_signal_handler, SIG_WRITE_MSG macro in fep_sub.h
- FR-01/02: Global flag declaration/definition
- FR-03: SIGKILL removed from Setsigfatal
- FR-04: End_Routine rewritten with sig_names[] lookup, write(), _exit()
- FR-05: Exit_Process guarded with !_in_signal_handler for unsafe operations
- FR-06-09: 4 PA Catch_Signal functions (pa_1600_tr, pa_2200_tr, pa_2100_ts, pa_2700_tr)
- FR-10-12: 3 PW Catch_Signal functions (pw_3010_tr, pw_3030_tr, pw_4000_ts)
- FR-13-15: 3 PZ Sig_Handler functions (pz_fepp, pz_daemon_proc, pz_procchk)
- Note: C99 designated initializers in sig_names[] array (acceptable pragmatic deviation from C89)
- Note: pz_daemon_proc.c message says "pa_daemon_proc" (cosmetic, file shared by sub-daemons)
- All signal handlers use only async-signal-safe functions: write, close, _exit, signal
