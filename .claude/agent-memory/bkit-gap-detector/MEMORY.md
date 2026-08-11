# Gap Detector Agent Memory

## Project: FEP (Front-End Processor for KRX)

### Architecture
- Pure C (ANSI C89) system for Korean stock exchange
- SHM-based config with macros: INFO(), DAEMON(), PROC(), TCP1(), TCP2(), UDPIP(), FILEM(), DSHM()
- SQLite DB migration layer added (config_db.c / config_loader.c)
- Key env vars: _FEP_DB_MODE (AUTO/DB/INI), _FEP_DB, _FEP_CFG

### Key Files
- Design docs: `docs/02-design/features/{feature}.design.md`
- Analysis output: `docs/03-analysis/{feature}.analysis.md`
- Shared lib: `st01/sub/` -> `st01/lib/libfepP.a`
- Headers: `st01/inc/`
- Modules: `st01/src/{PA,PB,PW,PX,PZ}/`

### Detailed Records
- See `analysis-history.md` for per-feature detailed analysis notes

### Analysis Summary Table

| # | Feature | Date | Match | FR | Notes |
|---|---------|------|-------|------|-------|
| 1 | Config-DB Migration | 02-18 | ~85% | N/A | Known bug: ip_addr vs ip_address |
| 2 | TR Struct Refactor | 02-18 | 72->89% | N/A | After Act-1 iteration |
| 3 | FIFO Struct Refactor | 02-21 | 95% | 6 | 1 residual DataBuff+14 |
| 4 | PA-PB Dedup Phase 1 | 02-21 | 90% | 8 | Err_Msg gap (65%) |
| 5 | PA-PB Dedup Phase 2 | 02-21 | 97% | 7 | "actual code citation" approach |
| 6 | PA-PB Dedup Phase 3 | 02-21 | 100% | 3 | Perfect score |
| 7 | Fifo-Event-Rtn Adoption | 02-22 | 100% | 16 | 7 PA + 4 PB + 1 PW |
| 8 | Socket-Linger Extraction | 02-22 | 100% | 15 | 12 active call sites |
| 9 | Dead Code Cleanup | 02-22 | 98% | 31 | +4 bonus files found |
| 10 | PA-PB 8100 Cleanup | 02-22 | 100% | 8 | -62 lines |
| 11 | Comment Cleanup R2 | 02-22 | 100% | 47 | 14 files |
| 12 | Sub Comment Final | 02-22 | 100% | 6 | 3 files, 39 lines |
| 13 | Dup Code Extraction | 02-22 | 100% | varies | 3 new shared functions |
| 14 | Getenv NULL Check | 02-22 | 98% | 29 | 1 N/A (in comment block) |
| 15 | Header C++ Comment C89 | 02-22 | 100% | 19 | 237 // comments converted |
| 16 | Unsafe strcpy Conversion | 02-22 | 100% | 12 | 53 active calls converted |
| 17 | Signal Handler Safety | 02-23 | 100% | 15 | async-signal-safe, 12 files |
| 18 | Unchecked mkfifo Fix | 02-25 | 100% | 22 | 30 call sites, 3 files + new file_util.c |
| 19 | DtoAf Dead Code Delete | 02-25 | 100% | 1 | 1 file deleted, 31 lines, zero callers |
| 20 | setsockopt Return Check | 02-25 | 100% | 6 | 2 files, +12 lines, 0 unchecked remaining |
| 21 | PA Code Quality Audit | 02-26 | 97% | 16 | 69 locations, 1 gap: pa_1490_mp.c:293 A1491 dup |
| 22 | PX Code Quality Audit | 02-27 | 100% | 37 | 28 files, 4 batches, 1 minor deviation (FR-36 exit(OK)) |
| 23 | PZ Code Quality Audit | 02-27 | 96% | 26 | 8 files, 4 batches, 1 gap: FR-24b j removed but used |
| 24 | Sub (libfepP) Audit | 02-27 | 100% | 17 | 13 files, 4 batches, 7 global checks all clear |
| 25 | Stat Save Optimize | 02-27 | 100% | 5 | 3 files, 53 callers unchanged, throttle + Force |
| 26 | TCP_NODELAY | 02-27 | 100% | 5 | 3 files, 12 callers unchanged, Nagle disable |
| 27 | File RW Optimize | 02-28 | 100% | 9 | 1 file, 7 functions, fcntl lock scope narrowed |
| 28 | atoif-optimize | 02-28 | 100% | 3 | 3 files, inner loop -> range check, 59 callers unchanged |
| 29 | poll-traversal | 02-28 | 100% | 4 | 12 files, 16 verification items, 15/15 PASS + 1 N/T |
| 30 | select-send-optimize | 02-28 | 100% | 5 | 3 files, 11 V items, 10/10 PASS + 1 N/T, -1 syscall/send |
| 31 | final-perf-optimize | 02-28 | 100% | 6 | 2 files, 11 V items, 10/10 PASS + 1 N/T, memset/strlen/sleep |
| 32 | structured-logging | 03-01 | 100% | 6 | 1 file (log_proc.c), 4 change points, 24 V items all PASS |

### Learned Patterns
- Design pseudocode for complex switch statements often simplified/idealized; always verify against actual source
- "Actual code citation" in design docs yields higher match rates (Phase1=90%, Phase2=97%, Phase3+=100%)
- When scanning for preprocessor patterns, use regex `#if\s+0|#if\s+\(0\)` not literal grep
- `DataBuff+N` pointer arithmetic is same anti-pattern class as `DataBuff[N]` -- check both
- Always check if targets are inside comment blocks before counting as active code
- `//` inside `/* */` block comments are text, not active C++ comments -- do not count as violations
- `#if 0` dead code blocks contain unconverted literals -- acceptable, do not count as gaps
- BACK2025/ and BACKUP/ directories contain old code (expected, not counted as gaps)
- C99 designated initializers (e.g., sig_names[] array) acceptable pragmatic deviation from C89 convention
- Build verification always NOT TESTED (requires server environment)
- When design specifies `defined()` syntax fix AND a copy-paste bug fix on the same line, verify BOTH were applied -- partial fix is common
- Design may specify `return -1` for void functions; check function signature before flagging as gap
- Batch preprocessor fixes (23 identical patterns) can be verified efficiently with grep for new pattern + reverse grep for old pattern
- When design says "remove line" but implementation changes value instead, count as match if functionally equivalent (dead code path)
- "Unused variable" claims in design docs MUST be verified by searching entire function body -- long functions (100+ lines) may use variables deep in nested loops that are easy to miss during audit
- When design says "remove variable X", always grep for `\bX\b` in the FULL function scope before implementing

### Cumulative Project Impact
- 32 features analyzed, 30 at 100% or near-100% match
- Shared lib (libfepP.a) grew: fep_common.c, fep_encrypt.c, fifo_event.c, ip_format.c, udp_init.c, poll_event.c, file_util.c
- Total lines removed: ~4,530+ across all features
- Design quality trend: improving each iteration due to "actual code citation" approach
