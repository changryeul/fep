# 설계: signal-handler-safety

## 참조

- 계획: `docs/01-plan/features/signal-handler-safety.plan.md`

## 요약

11개 signal handler의 async-signal-unsafe 함수 호출 수정. 12개 파일 전체에서. `Log()/SLog()`를 signal 컨텍스트의 `write(STDERR_FILENO, ...)`로 교체. signal handler에서 호출될 때 unsafe 작업을 건너뛰도록 `volatile sig_atomic_t` flag 추가. `exit()`를 `_exit()`로 교체. uncatchable `signal(SIGKILL, ...)`호출 제거.

## 핵심 개념: Async-Signal-Safe

POSIX는 signal handlers가 async-signal-safe 함수만 호출하도록 요구. 현재 사용되는 unsafe 함수:

| Unsafe 함수 | 사용 위치 | Safe 대체 |
|-----------------|----------|------------------|
| `vsnprintf()` | `Log()` | `write()` with static strings |
| `sprintf()` | `Log()`, `Exit_Process()` | `write()` with static strings |
| `strftime()` | `Log()` | signal 컨텍스트에서 omit |
| `fopen()/fwrite()` | `Stat_Save()` | signal 컨텍스트에서 skip |
| `exit()` | `Exit_Process()` | `_exit()` |

올바르게 사용되는 Safe 함수: `write()`, `open()`, `close()`, `_exit()`, `signal()`

## Signal-Safe Write Helper

signal 컨텍스트에서 static 문자열을 쓰기 위한 작은 helper 매크로:

```c
/* Signal handler에서 stderr로 static 문자열 쓰기 (async-signal-safe) */
#define SIG_WRITE_MSG(msg) write(STDERR_FILENO, msg, sizeof(msg) - 1)
```

signal 번호 (정수)의 경우, `sprintf/itoa`가 async-signal-safe가 아니므로 static 문자열의 사전 구축 lookup table 사용.

## 기능 요구사항

### Batch 1: Global flag와 header — `inc/fep_sub.h` + `sub/setsigfatal.c`

| FR | 파일 | 조치 |
|----|------|--------|
| FR-01 | `inc/fep_sub.h` | `End_Routine` extern 후 (줄 346 후) `extern volatile sig_atomic_t _in_signal_handler;` 추가 |
| FR-02 | `sub/setsigfatal.c` | Global 변수 `volatile sig_atomic_t _in_signal_handler = 0;` 추가 |

### Batch 2: `Setsigfatal()` 수정 — `sub/setsigfatal.c`

| FR | 파일 | 줄 | 조치 |
|----|------|:----:|--------|
| FR-03 | `sub/setsigfatal.c` | 23 | `signal(SIGKILL, End_Routine)` 제거 — SIGKILL을 catch 불가능 |

### Batch 3: `End_Routine()` 수정 — `sub/setsigfatal.c`

모든 `Log()` 호출을 async-signal-safe `write(STDERR_FILENO, ...)` with static 문자열로 교체.

| FR | 파일 | 줄 | 조치 |
|----|------|:-----:|--------|
| FR-04 | `sub/setsigfatal.c` | 41-78 | `End_Routine()` 다시 작성: `_in_signal_handler = 1` 설정, 9개의 `Log()` 호출을 `write(STDERR_FILENO, ...)` static 문자열로 교체, `exit(FAIL)`을 `_exit(FAIL)`로 교체 |

새 `End_Routine()`:
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

주: string literal 상수의 `strlen()`은 safe (library state 없음). `write()`와 `_exit()`는 async-signal-safe.

### Batch 4: `Exit_Process()` 수정 — `sub/setsigfatal.c`

Signal 컨텍스트 체크를 추가하여 unsafe 작업 건너뛰기.

| FR | 파일 | 줄 | 조치 |
|----|------|:-----:|--------|
| FR-05 | `sub/setsigfatal.c` | 87-149 | `Exit_Process()` 수정: `_in_signal_handler` 설정 시 `Log()` (줄 126, 146) 건너뛰기, `sprintf()`+`LtoU()` (줄 132-133) 건너뛰기, `Stat_Save()` (줄 143) 건너뛰기, `exit(FAIL)` 대신 `_exit(FAIL)` 사용 (줄 148) |

수정된 `Exit_Process()` 패턴:
```c
void	Exit_Process (void)
{
	char	bumun[4];

	if (D_K != -1)
	{
		/* SHM writes는 safe (메모리 저장만) — 그대로 유지 */
		if (DD_K != -1)
			INFO(DD_K).process_no = 0;

		if (P_K != -1)
		{
			/* ... 기존 TCP2 status reset 로직 변경 없음 ... */

			if (PROC(D_K,P_K).process_status == 1 &&
				PROC(D_K,P_K).start_status != 2)
			{
				if (write(DTART_FD, "1", 1) == -1)
				{
					if (!_in_signal_handler)
						Log (SYS_WARN, "daemon FIFO write failed!!");
				}
			}
		}
	}

	if (!_in_signal_handler)
	{
		sprintf (bumun, "%s", _SubSystem_Name);
		LtoU (bumun, 2);

		if (DD_K == -1 && bumun[1] != 'W' && bumun[1] != 'X' &&
			bumun[1] != 'Y' && bumun[1] != 'Z')
		{
			if (PROC(D_K,P_K).process_status == 4)
				PROC(D_K,P_K).process_status = 2;
			else if (PROC(D_K,P_K).process_status == 8)
				PROC(D_K,P_K).process_status = 9;
			else
				Stat_Save ();
		}

		Log (PRO_OK, "process STOP");
		exit (FAIL);
	}

	_exit (FAIL);
}
```

### Batch 5: PA `Catch_Signal()` 수정 — 4개 파일

`Log()/SLog()`를 signal-safe write로 교체, flag를 `Exit_Process()` 전에 설정.

| FR | 파일 | 줄 | Before | After |
|----|------|:----:|--------|-------|
| FR-06 | `src/PA/pa_1600_tr.c` | 657 | `Log(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pa_1600_tr caught signal\n");` |
| FR-07 | `src/PA/pa_2200_tr.c` | 734 | `Log(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pa_2200_tr caught signal\n");` |
| FR-08 | `src/PA/pa_2100_ts.c` | 1006 | `SLog(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pa_2100_ts caught signal\n");` |
| FR-09 | `src/PA/pa_2700_tr.c` | 478 | `Log(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pa_2700_tr caught signal\n");` |

### Batch 6: PW `Catch_Signal()` 수정 — 3개 파일

| FR | 파일 | 줄 | Before | After |
|----|------|:----:|--------|-------|
| FR-10 | `src/PW/pw_3010_tr.c` | 595 | `Log(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pw_3010_tr caught signal\n");` |
| FR-11 | `src/PW/pw_3030_tr.c` | 568 | `Log(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pw_3030_tr caught signal\n");` |
| FR-12 | `src/PW/pw_4000_ts.c` | 633 | `SLog(PRO_WARN, "signal (%d) occurred", signo);` | `_in_signal_handler = 1; SIG_WRITE_MSG("[SIGNAL] pw_4000_ts caught signal\n");` |

### Batch 7: PZ `Sig_Handler()` 수정 — 3개 파일

Non-fatal handlers (SIGUSR1/SIGUSR2). `Log()`를 signal-safe write로 교체. `signal()` re-registration 유지 (경쟁 window는 이들 제어된 signal에 대해 수용 가능).

주: `pz_procchk.c` SIGUSR2 case는 `Exit_Process()` 호출 — flag 설정 필요.

| FR | 파일 | 줄 | 조치 |
|----|------|:-----:|--------|
| FR-13 | `src/PZ/pz_fepp.c` | 819-846 | 4개의 `Log()` 호출을 `SIG_WRITE_MSG()` static 문자열로 교체 |
| FR-14 | `src/PZ/pz_daemon_proc.c` | 957-982 | 3개의 `Log()` 호출을 `SIG_WRITE_MSG()` static 문자열로 교체 |
| FR-15 | `src/PZ/pz_procchk.c` | 167-189 | 3개의 `Log()` 호출을 `SIG_WRITE_MSG()`로 교체. SIGUSR2 case에서 `Exit_Process()` 전에 `_in_signal_handler = 1` 설정 (줄 180) |

## 구현 순서

1. Batch 1 (FR-01, FR-02) — Global flag 선언
2. Batch 2 (FR-03) — Uncatchable SIGKILL 제거
3. Batch 3 (FR-04) — `End_Routine()` 다시 작성
4. Batch 4 (FR-05) — `Exit_Process()` 수정
5. Batch 5 (FR-06 ~ FR-09) — PA `Catch_Signal()` 수정
6. Batch 6 (FR-10 ~ FR-12) — PW `Catch_Signal()` 수정
7. Batch 7 (FR-13 ~ FR-15) — PZ `Sig_Handler()` 수정

## 검증

- `grep -rn 'Log\s*(' st01/sub/setsigfatal.c`는 `End_Routine()` 내부에서 0개 일치 반환 (`Exit_Process()`에서만, `!_in_signal_handler`로 guard)
- `grep -rn 'exit\s*(' st01/sub/setsigfatal.c`는 signal 경로의 `_exit(FAIL)`, non-signal 블록에만 `exit(FAIL)` 표시
- `grep 'SIGKILL' st01/sub/setsigfatal.c`는 0개 일치 반환
- 모든 7개 `Catch_Signal()` 함수가 `Exit_Process()` 전에 `_in_signal_handler = 1` 설정
- 모든 3개 `Sig_Handler()` 함수가 `Log()` 호출 없음

## 개수 요약

| 항목 | FR 항목 | 파일 | 조치 |
|----------|:--------:|:-----:|--------|
| Global flag | 2 | 2 | flag 변수와 extern 추가 |
| SIGKILL 제거 | 1 | 1 | 줄 삭제 |
| End_Routine 다시 작성 | 1 | 1 | 9 Log() + exit() 교체 |
| Exit_Process 수정 | 1 | 1 | flag guard 추가 |
| PA Catch_Signal | 4 | 4 | Log/SLog + flag 설정 교체 |
| PW Catch_Signal | 3 | 3 | Log/SLog + flag 설정 교체 |
| PZ Sig_Handler | 3 | 3 | Log() 호출 교체 |
| **합계** | **15** | **12** | |

주: `sub/setsigfatal.c`와 `inc/fep_sub.h`는 여러 batch에 나타나지만 파일 합계에서는 한 번만 계산.

## 비기능 요구사항

- Zero 기능 변화 in normal (non-signal) code path
- 모든 signal handlers POSIX async-signal-safe 준수
- C89 호환 (`volatile sig_atomic_t`는 표준 C89 via `<signal.h>`)
- Signal log 메시지는 여전히 stderr에 표시 (timestamp 없는 단순화 형식)
- `Exit_Process()`의 SHM status 업데이트 유지 (메모리 쓰기는 safe)
