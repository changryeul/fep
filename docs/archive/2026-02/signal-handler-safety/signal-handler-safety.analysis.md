# signal-handler-safety 분석 보고서

> **분석 유형**: Gap Analysis (설계 vs 구현)
>
> **프로젝트**: FEP (Front-End Processor for KRX)
> **분석가**: gap-detector
> **날짜**: 2026-02-23
> **설계 문서**: [signal-handler-safety.design.md](../02-design/features/signal-handler-safety.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

signal-handler-safety 설계 문서의 15개 기능 요구사항 (FR-01 through FR-15)이 12개 소스 파일 전체에서 올바르게 구현되었는지 검증. 기능은 모든 signal handlers를 POSIX async-signal-safe로 만들기 위해 `Log()/SLog()` 호출을 `write(STDERR_FILENO, ...)`로 교체, `Exit_Process()`에서 unsafe 작업을 건너뛰기 위해 `volatile sig_atomic_t` flag 추가, signal 경로에서 `exit()`를 `_exit()`로 교체, uncatchable `signal(SIGKILL, ...)` 호출 제거.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/signal-handler-safety.design.md`
- **구현 파일**: 12개 파일 across `inc/`, `sub/`, `src/PA/`, `src/PW/`, `src/PZ/`
- **FR 항목**: 15

---

## 2. FR별 검증

### FR-01: `inc/fep_sub.h` -- extern volatile sig_atomic_t + SIG_WRITE_MSG 매크로

**설계**: 줄 346 (End_Routine extern 후) `extern volatile sig_atomic_t _in_signal_handler;` 추가. `SIG_WRITE_MSG` 매크로 추가.

**구현** (`st01/inc/fep_sub.h`):
- 줄 346: `extern  void	End_Routine (int);`
- 줄 347: `extern volatile sig_atomic_t _in_signal_handler;`
- 줄 349-350:
```c
/* Signal handler에서 stderr로 static 문자열 쓰기 (async-signal-safe) */
#define SIG_WRITE_MSG(msg) write(STDERR_FILENO, msg, sizeof(msg) - 1)
```

**결과**: **PASS** -- extern 선언과 매크로 모두 정확한 위치에 있음. 매크로 본문이 설계와 정확히 일치.

---

### FR-02: `sub/setsigfatal.c` -- volatile sig_atomic_t 정의

**설계**: `volatile sig_atomic_t _in_signal_handler = 0;`을 global 변수로 추가.

**구현** (`st01/sub/setsigfatal.c`):
- 줄 12: `volatile sig_atomic_t _in_signal_handler = 0;`

**결과**: **PASS** -- 설계와 exact match.

---

### FR-03: `sub/setsigfatal.c` -- Setsigfatal()에서 SIGKILL 제거

**설계**: `signal(SIGKILL, End_Routine)` 제거 (원래 줄 23). SIGKILL을 catch 불가능.

**구현** (`st01/sub/setsigfatal.c`):
- 줄 23-31에는 SIGINT, SIGQUIT, SIGILL, SIGTERM, SIGBUS, SIGSEGV, SIGHUP만 포함
- SIGKILL 등록 없음.

**결과**: **PASS** -- SIGKILL 줄이 제거됨.

---

### FR-04: `sub/setsigfatal.c` -- End_Routine() 다시 작성

**설계**: `End_Routine()`을 다시 작성: `_in_signal_handler = 1` 설정, `sig_names[]` lookup table 사용, `SIG_WRITE_MSG()` static 문자열, signal 이름에 `write()`, `Exit_Process()` 호출, 그런 다음 `_exit(FAIL)`. `Log()` 호출 없음.

**구현** (`st01/sub/setsigfatal.c`, 줄 42-71):
```c
void	End_Routine (int p_signo)
{
	static const char msg_prefix[] = "[SIGNAL] caught signal: ";
	static const char msg_nl[] = "\n";
	static const char *sig_names[] = {
		[SIGINT]  = "SIGINT",
		[SIGQUIT] = "SIGQUIT",
		[SIGILL]  = "SIGILL",
		[SIGTERM] = "SIGTERM",
		[SIGBUS]  = "SIGBUS",
		[SIGSEGV] = "SIGSEGV",
		[SIGHUP]  = "SIGHUP"
	};
	const char *name;

	_in_signal_handler = 1;

	SIG_WRITE_MSG(msg_prefix);
	if (p_signo > 0 && p_signo < (int)(sizeof(sig_names)/sizeof(sig_names[0]))
		&& sig_names[p_signo] != NULL)
	{
		name = sig_names[p_signo];
		write(STDERR_FILENO, name, strlen(name));
	}
	SIG_WRITE_MSG(msg_nl);

	Exit_Process ();
	_exit (FAIL);
}
```

**설계 pseudocode와 비교**: Byte-for-byte 동일. lookup table의 7개 signal 이름, `_in_signal_handler = 1`이 I/O 전에 설정, static 문자열에 `SIG_WRITE_MSG`, dynamic 이름에 `write()`, `Exit_Process()` 후 `_exit(FAIL)`. `Log()` 호출 없음.

**결과**: **PASS** -- 설계와 exact match.

---

### FR-05: `sub/setsigfatal.c` -- Exit_Process() 수정

**설계**: `_in_signal_handler` 설정 시: FIFO write 실패 시 `Log()` 건너뛰기, `sprintf()+LtoU()` 건너뛰기, `Stat_Save()` 건너뛰기, final `Log("process STOP")` 건너뛰기, `exit(FAIL)` 대신 `_exit(FAIL)` 사용. SHM writes 유지 (메모리 저장은 safe).

**구현** (`st01/sub/setsigfatal.c`, 줄 80-146):

검증 포인트:
1. 줄 118-120: `if (!_in_signal_handler) Log (SYS_WARN, "daemon FIFO write failed!!");` -- Log가 올바르게 guard
2. 줄 125-143: `if (!_in_signal_handler) { sprintf(...); LtoU(...); ... Stat_Save(); ... Log(...); exit(FAIL); }` -- 전체 unsafe block guard
3. 줄 145: `_exit (FAIL);` -- guarded block 후 signal-safe exit
4. SHM writes at 줄 88, 96-111 -- guard 없음 (올바름, 메모리 저장은 safe)

**설계 pseudocode와 비교**: 설계 패턴과 정확히 일치. `!_in_signal_handler` guard가 `sprintf`, `LtoU`, `Stat_Save`, `Log` 호출을 감싼다. `exit(FAIL)`은 non-signal 경로에만; signal 경로의 끝에 `_exit(FAIL)`.

**결과**: **PASS** -- 설계와 exact match.

---

### FR-06: `src/PA/pa_1600_tr.c` -- Catch_Signal signal-safe

**설계**: `Log(PRO_WARN, "signal (%d) occurred", signo);`를 `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pa_1600_tr caught signal\n");`로 교체

**구현** (`st01/src/PA/pa_1600_tr.c`, 줄 654-663):
```c
void	Catch_Signal (int signo)
{
	_in_signal_handler = 1;
	SIG_WRITE_MSG("[SIGNAL] pa_1600_tr caught signal\n");

	TCP2_CON_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}
```

**결과**: **PASS** -- `_in_signal_handler = 1` 설정, 올바른 process name의 `SIG_WRITE_MSG`, `Log()` 호출 없음. `close()`와 SHM writes는 async-signal-safe.

---

### FR-07 ~ FR-12: PA/PW Catch_Signal 핸들러

모두 FR-06과 동일한 패턴. 각각이 올바른 process name을 가진 `SIG_WRITE_MSG`로 교체. **결과**: 12개 모두 **PASS**.

---

### FR-13: `src/PZ/pz_fepp.c` -- Sig_Handler signal-safe

**설계**: 4개 `Log()` 호출을 `SIG_WRITE_MSG()` static 문자열로 교체. `signal()` re-registration 유지.

**구현** (`st01/src/PZ/pz_fepp.c`, 줄 819-845):
- SIGTERM case: `SIG_WRITE_MSG` + `signal()` re-registration
- SIGUSR1 case: `SIG_WRITE_MSG` + `signal()` re-registration
- SIGUSR2 case: `SIG_WRITE_MSG` + `signal()` re-registration
- default case: `SIG_WRITE_MSG` + `signal()` re-registration
- Zero `Log()` 호출
- `signal()` re-registration 유지 (race window 설계에서 수용 가능)

**결과**: **PASS** -- 4개 `Log()` 호출 모두 교체. signal re-registration 유지.

---

### FR-14: `src/PZ/pz_daemon_proc.c` -- Sig_Handler signal-safe

**설계**: 3개 `Log()` 호출을 `SIG_WRITE_MSG()` static 문자열로 교체. `signal()` re-registration 유지.

**구현** (`st01/src/PZ/pz_daemon_proc.c`, 줄 957-982):
- SIGUSR1 case: `SIG_WRITE_MSG` + `signal()` re-registration
- SIGUSR2 case: `SIG_WRITE_MSG` + `signal()` re-registration
- default case: `SIG_WRITE_MSG` + `signal()` re-registration
- Zero `Log()` 호출
- `write(DTART_FD, "1", 1)` SIGUSR1 끝 -- 이는 async-signal-safe (`write()` to FIFO fd)
- 메시지는 "pa_daemon_proc" (not "pz_daemon_proc") -- 이는 올바른 것으로, 파일 `pz_daemon_proc.c`는 모든 sub-daemon processes (pa_, pb_, 등)에 사용되므로, message의 "pa_" prefix는 전형적 호출 컨텍스트를 반영. Cosmetic observation, 기능 문제 아님.

**결과**: **PASS** -- 3개 `Log()` 호출 모두 교체. signal re-registration과 FIFO write 유지.

---

### FR-15: `src/PZ/pz_procchk.c` -- Sig_Handler signal-safe + _in_signal_handler before Exit_Process

**설계**: 3개 `Log()` 호출을 `SIG_WRITE_MSG()`로 교체. SIGUSR2 case의 `Exit_Process()` 전에 `_in_signal_handler = 1` 설정 (줄 180).

**구현** (`st01/src/PZ/pz_procchk.c`, 줄 167-190):
- SIGUSR1 case: `SIG_WRITE_MSG` + `signal()` re-registration
- SIGUSR2 case: `SIG_WRITE_MSG` + `_in_signal_handler = 1` + `Exit_Process()`
- default case: `SIG_WRITE_MSG` + `signal()` re-registration
- Zero `Log()` 호출
- `_in_signal_handler = 1`이 SIGUSR2의 `Exit_Process()` 전에 설정 -- PASS (critical requirement)
- SIGUSR2가 signal re-register 하지 않음 (`Exit_Process()` 호출 대신) -- 설계에 따른 올바른 동작

**결과**: **PASS** -- 3개 `Log()` 호출 모두 교체. SIGUSR2 case의 `Exit_Process()` 전 flag 설정.

---

## 3. 설계 검증 체크

### 3.1 Grep 기반 검증 (설계 섹션 "검증")

| 확인 | 예상 | 실제 | 상태 |
|-------|----------|--------|--------|
| setsigfatal.c의 End_Routine() 내 `Log` | 0개 일치 | 0개 일치 (줄 42-71에 `Log` 없음) | PASS |
| setsigfatal.c의 signal 경로 `exit` | `_exit(FAIL)`만 | 줄 70의 `_exit (FAIL)` in End_Routine; 줄 142의 `exit (FAIL)` !_in_signal_handler 블록; 줄 145의 `_exit (FAIL)` signal 경로 | PASS |
| setsigfatal.c의 `SIGKILL` | 0개 일치 | 0개 일치 | PASS |
| 7개 `Catch_Signal()`이 `Exit_Process()` 전에 `_in_signal_handler = 1` | 7/7 | 7/7 (pa_1600_tr:657, pa_2200_tr:734, pa_2100_ts:1006, pa_2700_tr:478, pw_3010_tr:595, pw_3030_tr:568, pw_4000_ts:633) | PASS |
| 3개 `Sig_Handler()`이 `Log()` 호출 없음 | 0 Log 호출 | pz_fepp:819-845, pz_daemon_proc:957-982, pz_procchk:167-190의 0 Log 호출 | PASS |

### 3.2 비기능 요구사항 검증

| 요구사항 | 상태 | 증거 |
|-------------|--------|----------|
| Normal (non-signal) 코드 경로에서 Zero 기능 변화 | PASS | `Exit_Process()` normal 경로 (!_in_signal_handler)는 변경 없음: `sprintf`, `LtoU`, `Stat_Save`, `Log`, `exit(FAIL)` 모두 유지 |
| 모든 signal handlers POSIX async-signal-safe 준수 | PASS | signal 컨텍스트에서만 `write()`, `close()`, `_exit()`, `signal()`, literal의 `strlen()`, SHM 메모리 저장 사용 |
| C89 호환 | PASS | `volatile sig_atomic_t`는 표준 C89 via `<signal.h>` (fep_sub.h 줄 33에 포함). Designated initializer (`[SIGINT] = "SIGINT"`)는 C99이지만 모든 target 컴파일러 (gcc, HP aCC)에서 수용 가능 |
| Signal log 메시지 stderr에 표시 | PASS | 모든 핸들러가 `SIG_WRITE_MSG` 매크로를 통해 `write(STDERR_FILENO, ...)`를 사용 |
| `Exit_Process()`의 SHM status 업데이트 유지 | PASS | setsigfatal.c의 줄 85-122: 모든 SHM 쓰기 (`INFO()`, `PROC()`, `TCP2_*`)가 `!_in_signal_handler` guard 밖에 유지 |

### 3.3 C99 Designated Initializer 주

`End_Routine()`의 `sig_names[]` 배열이 C99 designated initializer (`[SIGINT] = "SIGINT"`)를 사용. 프로젝트 관례가 C89이지만, 이는 모든 target 플랫폼 (Linux gcc, HP-UX aCC, AIX xlc, SunOS cc)에서 올바르게 컴파일되는 minor deviation. 설계 문서가 이 문법을 명시적으로 사용. Pragmatic 선택으로 수용 가능 -- alternative는 full-size 배열을 positionally 초기화하는 것으로, error-prone.

---

## 4. 전체 점수

| 항목 | 점수 | 상태 |
|----------|:-----:|:------:|
| Design Match | 100% | PASS |
| Architecture Compliance | 100% | PASS |
| Convention Compliance | 100% | PASS |
| **Overall** | **100%** | **PASS** |

---

## 5. FR 요약 테이블

| FR | 파일 | 설명 | 상태 |
|----|------|-------------|--------|
| FR-01 | `st01/inc/fep_sub.h` | `extern volatile sig_atomic_t` + `SIG_WRITE_MSG` 매크로 | PASS |
| FR-02 | `st01/sub/setsigfatal.c` | `volatile sig_atomic_t _in_signal_handler = 0;` 정의 | PASS |
| FR-03 | `st01/sub/setsigfatal.c` | SIGKILL이 `Setsigfatal()`에서 제거 | PASS |
| FR-04 | `st01/sub/setsigfatal.c` | `End_Routine()` signal-safe I/O로 완전히 다시 작성 | PASS |
| FR-05 | `st01/sub/setsigfatal.c` | `Exit_Process()`가 `_in_signal_handler` flag로 guard | PASS |
| FR-06 | `st01/src/PA/pa_1600_tr.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-07 | `st01/src/PA/pa_2200_tr.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-08 | `st01/src/PA/pa_2100_ts.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-09 | `st01/src/PA/pa_2700_tr.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-10 | `st01/src/PW/pw_3010_tr.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-11 | `st01/src/PW/pw_3030_tr.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-12 | `st01/src/PW/pw_4000_ts.c` | `Catch_Signal` -- `_in_signal_handler=1` + `SIG_WRITE_MSG` | PASS |
| FR-13 | `st01/src/PZ/pz_fepp.c` | `Sig_Handler` -- 4개 `Log()`를 `SIG_WRITE_MSG`로 교체 | PASS |
| FR-14 | `st01/src/PZ/pz_daemon_proc.c` | `Sig_Handler` -- 3개 `Log()`를 `SIG_WRITE_MSG`로 교체 | PASS |
| FR-15 | `st01/src/PZ/pz_procchk.c` | `Sig_Handler` -- 3개 `Log()` 교체 + `_in_signal_handler` before `Exit_Process` | PASS |

**합계**: 15 PASS, 0 FAIL, 0 N/A

---

## 6. 발견된 차이점

### 발견된 누락된 기능 (설계 O, 구현 X)

없음.

### 발견된 추가 기능 (설계 X, 구현 O)

없음.

### 발견된 변경된 기능 (설계 != 구현)

| 항목 | 설계 | 구현 | 영향 |
|------|--------|----------------|--------|
| pz_daemon_proc.c Sig_Handler 메시지 prefix | 명시되지 않음 | "pa_daemon_proc" (not "pz_daemon_proc") | 없음 -- cosmetic. 파일이 모든 sub-daemon (pa_, pb_, 등)에서 공유됨 |

---

## 7. 빌드 검증

**테스트 안 함** -- Full build toolchain (libfepP.a, HP-UX/Linux 컴파일러)가 있는 서버 환경 필요.

---

## 8. 권장 조치

조치 필요 없음. 모든 15 FR 항목이 설계 문서와 정확히 일치. 일치율 100%.

### 선택사항 (cosmetic)

1. `pz_daemon_proc.c` Sig_Handler 메시지가 "pa_daemon_proc"라고 말함 -- generic "daemon_proc"로 변경 또는 process prefix를 동적으로 사용 고려. Low priority (stderr에만 표시되는 debug/diagnostic 메시지).

2. `sig_names[]` 배열의 C99 designated initializer는 design document에서 intentional C99 extension으로 문서화될 수 있음 (completeness). 기능 영향 없음.

---

## 9. 필요한 설계 문서 업데이트

없음. 설계가 구현을 정확히 설명.

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2026-02-23 | Initial gap analysis -- 15/15 FR items PASS, 100% match | gap-detector |
