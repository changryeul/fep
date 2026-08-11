# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

## Project Overview

FEP (Front-End Processor) for KRX (Korea Exchange). A multi-process, daemon-based, real-time trading infrastructure system written in pure C (ANSI/C89). Handles order transmission, response/fill reception, market data, auto-trading strategies, and position management for bonds, derivatives, and equities.

Target platforms: HP-UX, SunOS, AIX, Linux. Production hosts: `podm11` (REAL1), `podm12` (REAL2), others default to TEST.

## Build System

Environment must be set up first by sourcing `st01/env/pkg_env.sh` (normally sourced via `.bash_profile`). This defines `_FEP_HOME`, `_FEP_SYSTEM`, `_FEP_SUBDIR`, and all path variables/aliases used by the build and operational scripts.

### Build Commands (via `st01/shl/mk.sh`)

```sh
mk.sh all           # Full rebuild: clean objects/libs, rebuild library, then all modules (A B W X Z)
mk.sh sub           # Rebuild shared library only (libfepP.a)
mk.sh src           # Rebuild all source modules (PA, PB, PX, PZ) in parallel
mk.sh pb            # Rebuild a specific module (PB)
mk.sh pa ts         # Rebuild specific process type within a module (PA transmit processes)
mk.sh pa us         # Rebuild PA UDP send processes only
```

### Build Chain

1. **Library**: `sub/*.c` → compiled individually → archived into `lib/libfepP.a`
   - Makefile: `make/SUB/Make_Lib_P_c.mk`, driven by `make/SUB/Make_Lib_P_c.sh`
   - Skips `dbipc.c`
2. **Executables**: `src/{PA,PB,PX,PZ}/*.c` → linked with `libfepP.a` → output to `bin/`
   - Each module has `make/{module}/Make_{module}.mk` as entry point
   - Per-type shell scripts (`Make_PB_ts.sh`, `Make_PB_tr.sh`, etc.) iterate over process IDs with `-D` defines
   - OS-specific compiler/linker flags selected via `$osname` (HP-UX, SunOS, AIX, Linux)
   - Linux: uses `-ltirpc -lm -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE`

### Key Compiler Defines

- `-DA{NNNN}` / `-DB{NNNN}` — Process ID number (e.g., `-DA1101` for PA order TX, `-DB1101` for PB bond order TX)
- `-DNO_INISAFE` — Build without INISAFE-Net library (enabled by default in `pkg_env.sh`)
- `-DSAM_USE` — Enable SAM file processing
- `-DHOLIDAY_CHECK` / `-DHOLIDAY_APPLY` — Holiday date handling
- `-D_REENTRANT` — Thread-safe code
- `-D_TR_IMPROVE` — Transaction improvement features

### Build Pitfalls

- **`make` timestamp skipping**: When syncing files to a server (scp/rsync), timestamps may not trigger recompilation. Fix: `rm -f obj/SUB/*.o && mk.sh sub` to force library rebuild.
- **Linux linker**: All modules must use `-ltirpc` (not `-lnsl`). `-lnsl` was removed from modern glibc.
- **`// comment {` bug**: Never place `{` inside a `//` comment — the compiler won't see the brace, breaking `if`/`switch` blocks. Use `if (...) { // comment` instead of `if (...) // comment {`.

### Config-DB Test Build (via `st01/test/Makefile`)

```sh
cd st01/test
make              # Full test: build + import + export + roundtrip verify
make build        # Compile ini2db, db2ini only
make import       # INI → SQLite DB
make export       # DB → INI files
make verify       # Roundtrip comparison (10 INI + 2 IP files)
make dbcheck      # Inspect DB schema, key counts, environments
make dryrun       # Parse-only mode, no DB write
make cfgverify    # Build cfg_verify (server only, requires libfepP)
make install      # Copy ini2db, db2ini to st01/bin/
make clean        # Remove all test artifacts
make ENV_ID=REAL1 # Test with specific environment
```

Remote deployment: `./deploy_test.sh fepp@10.x.x.x` (macOS → server) or `./deploy_test.sh local` (on server).

## Environment Variables

Set by `st01/env/pkg_env.sh`, auto-detected by hostname:

| Variable | Value | Purpose |
|----------|-------|---------|
| `_FEP_DIV` | `REAL1`/`REAL2`/`TEST` | Environment discriminator (podm11→REAL1, podm12→REAL2, else→TEST) |
| `_FEP_HOME` | `$HOME/fep` | Base FEP directory |
| `_FEP_SYSTEM` | `"p"` | System ID (selects library suffix: libfep**P**.a) |
| `_FEP_SUBDIR` | `"A B W X Z"` | Module letters (PA, PB, PW, PX, PZ) |
| `NO_INISAFE` | `1` | Disable INISAFE-Net encryption (default on) |
| `osname` | `uname -s` | OS detection for compiler flags |
| `_P_BIN`, `_P_CFG`, `_P_INC`, `_P_LIB`, `_P_UTL` | Dynamic | Path variables per module directory |

Shell aliases: `pbin`, `pcfg`, `pinc`, `plib`, `pshl`, `putl`, `psub`, `penv`, `psrc`, `pobj`, `pmake` — quick `cd` to module directories. Per-module aliases like `pbsrc`, `pbobj`, `pblog` navigate to module-specific subdirectories.

## Directory Layout (`st01/`)

| Directory | Purpose |
|-----------|---------|
| `bin/` | ~66 compiled process binaries |
| `cfg/` | INI configuration files (daemon, process, TCP, UDP, file, SHM definitions) |
| `env/` | Environment setup (`pkg_env.sh`, `.bash_profile` template, `.vimrc`) |
| `inc/` | 25 shared header files |
| `lib/` | Static library `libfepP.a` |
| `make/` | Makefiles and build scripts per module (`PA/`, `PB/`, `PX/`, `PZ/`, `SUB/`) |
| `obj/` | Compiled object files, mirroring `src/` structure |
| `shl/` | 53 operational shell scripts (build, monitoring, startup, maintenance) |
| `src/` | C source organized by module (`PA/`, `PB/`, `PX/`, `PZ/`) |
| `sub/` | 50 common subroutine C sources (compiled into `libfepP.a`) |
| `utl/` | Utility/migration tools (`ini2db.c`, `db2ini.c`, `cfg_verify.c`) + test/sample programs |
| `test/` | Test infrastructure (Makefile, deploy_test.sh, roundtrip verification) |

Note: `OB/`, `OW/`, `OX/`, `OZ/` directories exist but are empty/legacy.

### Runtime Directories (`st02/`, `st03/`)

| Directory | Purpose |
|-----------|---------|
| `st02/FIFO/{PA,PB,PX,PZ}` | Named pipes for inter-process communication |
| `st02/DAT/{PA,PB,PX,PZ}` | Runtime data files |
| `st03/LOG/{PA,PB,PX,PZ}` | Process log files |
| `st03/SEQ/` | Sequence files (BATCH, DISC, TOTAL) |

These directories must exist before system startup. Created manually with `mkdir -p`.

## Architecture

### Module Organization

- **PA** — Trading logic: order TX/RX, fills, auto-trading strategies, market data, client comm, limits
- **PB** — Bond-specific KRX FEP: direct exchange connections for bond orders and market data
- **PW** — Daemon and connection management (5 sources: daemon mp, connection TR, TS)
- **PX** — Utility/monitoring processes (config loading, process checking, status display)
- **PZ** — System management: daemon control, memory management, log compaction, process checking

### Process Naming Convention

Format: `p{module}_{NNNN}_{type}`

- Module: `a`=trading, `b`=bonds, `x`=utility, `z`=management
- NNNN: 4-digit function code
  - 1xxx: Bonds/KRX direct (1101=order TX, 1201=response RX, 1401=fill)
  - 2xxx: Derivatives (2101=order TX, 2201=response RX)
  - 5xxx: Auto-trading strategies
  - 6xxx: Inquiry processing
  - 7xxx: Market data reception/distribution
  - 8xxx: Client communication
  - 9xxx: Limit/risk management
- Type: `mp`=manager, `ts`=TCP send, `tr`=TCP receive, `ur`=UDP receive, `dd`=data distribution, `qr`=query request, `qs`=query service, `us`=UDP send

### IPC Mechanisms

- **Shared Memory (SHM)**: Primary data exchange — account info, order state, market data, strategies. Layout defined in `inc/shm_memory.h` (60KB header). Up to 20 accounts, 10,000 unfilled orders, 40 auto-trading processes.
- **FIFO/Named Pipes**: Inter-process message passing (defined in `cfg/file.ini`)
- **Semaphores**: Process synchronization (`sub/semipc.c`)
- **TCP/IP**: Direct connections to KRX and clients (configured in `cfg/tcp1.ini`, `cfg/tcp2.ini`)
- **UDP/IP**: Market data multicast reception (configured in `cfg/udpip.ini`)

### KRX Message Structure

All KRX order/settlement messages share a common header layout defined in `inc/pa_struct.h`:

```
KRX_MSG_COMMON (106 bytes)
├── KRX_HEADER (82 bytes)     — BeginString, BodyLength, MsgType, MsgSeqNum, SenderCompID, SendingTime, ...
└── KRX_BODY_COMMON (24 bytes) — DataSeq[11], Transaction_Code[11], Megrp_no[2]
```

Use `KRX_MSG_COMMON` for struct-based field access instead of hardcoded byte offsets like `DataBuff[82]` or `DataBuff[KRX_HEAD_LEN+11]`. Constants: `KRX_HEAD_LEN = sizeof(KRX_HEADER) = 82`.

### Configuration System

All INI-format files in `cfg/`:

- `daemon.ini` — Daemon lifecycle (start/end times, date flags D/D-1/D+1, compact days, IPC counts)
- `proc.ini` — Process definitions (18+ processes with TCP/UDP ports, start/end times, I/O file mappings)
- `file.ini` — File/FIFO definitions with record sizes
- `tcp1.ini` / `tcp2.ini` — TCP master daemon and direct connection configs
- `udpip.ini` — UDP multicast configs
- `dshm.ini` — Data shared memory definitions
- `sisetr.ini` — Market data TR definitions
- `client.ini` / `pc.ini` — IP whitelist entries

**Config-DB migration**: INI files can be loaded into SQLite DB (`fep_config.db`) via `utl/ini2db.c`. The `sub/config_loader.c` provides unified loading with three modes: `AUTO` (DB with INI fallback), `DB` (DB-only), `INI` (legacy INI-only).

### Shared Library (libfepP.a) — `sub/` Components

52 source files organized by function:

| Category | Files | Purpose |
|----------|-------|---------|
| Config/DB | `config_db.c`, `config_loader.c` | SQLite config loading, unified INI/DB access |
| Common/Dedup | `fep_common.c`, `fep_encrypt.c`, `fifo_event.c`, `device_rw.c`, `sock_linger.c` | Shared PA/PB event-loop functions, INISAFE encryption, FIFO handler, device I/O, socket linger |
| SHM | `shmipc.c`, `shmsub.c`, `shm_rw.c` | Shared memory create/attach/read/write |
| TCP/IP | `tcpip_{accept,bind,connect,listen,recv,select,send,sock}.c`, `udp_init.c` | Network socket layer |
| File I/O | `file_rw.c`, `file_util.c`, `poll_file.c`, `poll_event.c`, `select_{recv,send}.c` | FIFO/file operations |
| Process | `make_daemon.c`, `init_proc.c`, `init_mana.c`, `check_proc.c`, `check_env.c`, `check_exist.c` | Process lifecycle |
| Logging | `log_proc.c`, `stat_save.c`, `seq_save.c` | Audit and status tracking |
| Conversion | `atodf.c`, `atolf.c`, `atoif.c`, `itoaf.c` | Number/string conversions |
| Utility | `key_search.c`, `queue.c`, `gettime.c`, `getfileno.c`, `getenvironment.c`, `set_trtime.c`, `ltou.c`, `utol.c`, `chk_kor.c`, `hoga_check.c`, `display_errmsg.c`, `setsigfatal.c`, `semipc.c`, `ip_format.c` | Misc utilities |

### Critical Header Files

| File | Size | Role |
|------|------|------|
| `inc/krx_mk.h` | 98KB | KRX market data structures (bonds, derivatives, equities) |
| `inc/shm_memory.h` | 60KB | Shared memory layout, account management, order number ranges |
| `inc/pa_struct.h` | 69KB | All PA module data structures, KRX_MSG_COMMON |
| `inc/fep_interface.h` | ~7KB | Protocol constants, handshake codes, status definitions |
| `inc/def_error.h` | 5KB | Error code definitions |
| `inc/strategy.h` | 6KB | Auto-trading strategy definitions |
| `inc/fep_common.h` | ~2KB | Common PA/PB event-loop externs and prototypes |
| `inc/fep_encrypt.h` | ~1KB | INISAFE-Net encryption externs and prototypes (PB only) |

## Operational Scripts (`shl/`)

| Script | Purpose |
|--------|---------|
| `mk.sh` / `mkall.sh` | Build orchestration |
| `master.sh` | Master daemon startup |
| `initA0.sh` | System initialization (reset _dd processes) |
| `ps.sh` | Process status display |
| `chkproc.sh` | Process health check |
| `chktcp1.sh` / `chktcp2.sh` / `chkudp.sh` | Network connectivity checks |
| `chkshm.sh` / `ipcs.sh` | Shared memory/IPC status |
| `compact.sh` | Old data/log cleanup |
| `cfgload.sh` / `cfgback.sh` | Configuration load/backup |
| `ipcrm.sh` | IPC resource cleanup (destructive) |
| `setpstat.sh` / `setfcnt.sh` | Process status and file counter manipulation |

## Documentation

Reference documents in `docs/`:

| File | Purpose |
|------|---------|
| `FEP_Architecture_Analysis.md` | System architecture deep-dive (4 modules, IPC, security) |
| `FEP_Macro_Reference.md` | Complete macro reference (SHM, error codes, TCP flags, KRX messages) |
| `04-report/changelog.md` | Project deliverables history with PDCA metrics |
| `archive/2026-02/_INDEX.md` | Archived feature index (42 completed features with PDCA docs) |
| `archive/2026-03/_INDEX.md` | Archived feature index (March 2026) |

PDCA documents follow the structure: `docs/01-plan/`, `02-design/`, `03-analysis/`, `04-report/`. Completed features are archived to `docs/archive/YYYY-MM/{feature}/`.

### PA/PB Deduplication Architecture

PA (trading) and PB (bonds) modules share nearly identical event-loop patterns. Common functions have been extracted to the shared library:

- **`fep_common.c`** — Core event-loop functions shared by PA and PB: `Get_Msec`, `Line_Change`, `Device_Close_Base`, `Err_Msg`, `Log_Out_Base`, `Device_Open_Logon`, `Time_Out_Disconnect`
- **`device_rw.c`** — `Device_Read`, `Device_Write` (separated from `fep_common.c` to avoid multiple-definition conflicts with processes that define their own, e.g., `pb_8100_ts`, `pb_1800_ts`)
- **`sock_linger.c`** — `Set_Socket_Linger` (separated from `fep_common.c` for the same reason as `device_rw.c`)
- **`fep_encrypt.c`** — PB-only INISAFE encryption: `Free_All`, `Handshake` (PB uses encrypted KRX connections; PA uses stub `Handshake`)
- **`fifo_event.c`** — Isolated `Fifo_Event_Rtn` (separate .o to avoid pulling in `fep_common.o` dependencies for processes that only need the FIFO handler)

**Key pattern**: `extern void *FmtPtr` resolves PA's `S_Fmt` vs PB's `KR_Fmt` struct differences — each process assigns its own format struct to `FmtPtr`, and common functions use it via cast. Process-specific behavior is handled through thin wrappers (e.g., `Device_Close` calls `Device_Close_Base` plus PB-specific cleanup).

## System Startup Order

Startup order is critical — processes depend on shared resources created by earlier processes:

1. **`pz_memory_mp`** (first) — Creates shared memory segments and FIFO named pipes via `Create_FIFO()`
2. **`pa_daemon_mp`** / **`pb_daemon_mp`** — Opens FIFOs and spawns child processes via `execl()`
3. Child processes (`*_ts`, `*_tr`, `*_ur`, `*_us`, etc.) — Started by their parent daemon

If `pa_daemon_mp` fails with "cannot open FIFO", `pz_memory_mp` hasn't run yet (or runtime directories don't exist).

## Security

Uses INISAFE-Net library (v7.2.47, 64-bit) for KRX encrypted connections. Library located at `~/fep/krx/INISAFE_Net_for_C_v7.2.47_64/`. Include path and `LD_LIBRARY_PATH` set in `pkg_env.sh`. `NO_INISAFE=1` is exported by default in `pkg_env.sh` for environments without the library; PB sources have `#ifndef NO_INISAFE` guards around all encryption calls.

## Conventions

- **Language**: Pure C89/ANSI C — no C++ features
- **Tab width**: 4 spaces (configured in `.vimrc`)
- **Struct naming**: `*_FMT`, `*_S` suffixes
- **SHM variables**: `SHM_*` prefix for types, `Shm_*` for C variables
- **Comments**: Mix of Korean and English
- **Source backups**: `BACK2025/`, `JC_OLD/` directories alongside active sources; `.org` and `.back` suffixes for old versions
- **One source → multiple binaries**: A single `.c` file may produce multiple binaries via `-D` defines (e.g., `pb_1100_ts.c` builds `pb_1101_ts` with `-DB1101`)
- **KRX message access**: Use `KRX_MSG_COMMON` struct, not raw byte offsets (`DataBuff[82]`)
- **KRX vs IMECO structs**: PA 2xxx processes (derivatives) use IMECO protocol with different struct layouts (e.g., no `DataSeq` field, `char ErrCd[8]` instead of `int ErrCd`). Shared headers like `fep_common.h` must not declare types that conflict with IMECO processes — process-variant externs go in `fep_common.c` directly.
- **Structured logging**: Set `FEP_LOG_FORMAT=structured` env var for pipe-delimited log output (disabled by default)
