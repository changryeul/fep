# 계획: signal-handler-safety

## 개요

신호 핸들러에서 async-signal-unsafe 함수 호출을 FEP 코드베이스 전체에서 제거합니다. POSIX는 신호 핸들러가 "async-signal-safe" 함수만 호출해야 합니다. 현재 코드는 `Log()` (vsnprintf, sprintf, strftime 사용), `Stat_Save()` (sprintf, fopen, fwrite 사용), `exit()` 등 모두 async-signal-unsafe인 함수들을 호출합니다. 이는 신호 전달 중 교착 상태, 데이터 손상, 정의되지 않은 동작을 유발할 수 있습니다.

## 배경

POSIX는 신호 핸들러 내에서 안전하게 호출할 수 있는 async-signal-safe 함수 세트를 정의합니다. 현재 핸들러에서 사용하는 주요 unsafe 함수들:

- `vsnprintf()`, `sprintf()`, `strftime()` — NOT async-signal-safe
- `fopen()`, `fwrite()`, `fclose()` — NOT async-signal-safe
- `exit()` — NOT async-signal-safe (atexit 핸들러 실행, stdio 플러시)
- `malloc()`, `free()` — NOT async-signal-safe

주요 안전 대체함수: `write()`, `open()`, `close()`, `_exit()`, `stat()`, `mkdir()`, `signal()`

### FEP의 신호 핸들러

**A. `sub/setsigfatal.c`의 `End_Routine()` — 모든 프로세스에서 공유**

다음을 통해 등록됨: `Setsigfatal()`는 다음에서 호출됨:
- `sub/init_proc.c` (모든 일반 프로세스)
- `sub/init_mana.c` (모든 매니저 프로세스)
- `src/PZ/pz_daemon.c`, `pz_fepp.c`, `pz_filechk.c`, `pz_procchk.c`

8개 신호 처리: SIGINT, SIGKILL(무용지물), SIGQUIT, SIGILL, SIGTERM, SIGBUS, SIGSEGV, SIGHUP.

Unsafe 호출:
- 9 × `Log()` (vsnprintf + sprintf + strftime + 파일 I/O)
- 1 × `Exit_Process()` 자체 호출: `Log()`, `sprintf()`, `Stat_Save()`, `exit()`

**B. 7개 PA/PW 파일의 프로세스별 `Catch_Signal()`**

SIGPIPE 및 SIGTERM에 대해 `sigaction()`으로 등록. 각각 `Log()/SLog()` 그 후 `Exit_Process()` 호출.

파일: `pa_1600_tr.c`, `pa_2100_ts.c`, `pa_2200_tr.c`, `pa_2700_tr.c`, `pw_3010_tr.c`, `pw_3030_tr.c`, `pw_4000_ts.c`

**C. 3개 파일의 PZ `Sig_Handler()` (비치명적, 로그-반환)**

SIGUSR1/SIGUSR2 (데몬에서의 제어된 신호) 처리. `Log()` + `signal()` 재등록 호출. 제어된 신호이고 핸들러가 반환하므로 위험도 낮음.

파일: `pz_fepp.c`, `pz_daemon_proc.c`, `pz_procchk.c`

## 범위

### 범위 내

1. **`sub/setsigfatal.c`** — `End_Routine()` 및 `Exit_Process()` 수정:
   - `End_Routine()`의 `Log()`를 async-signal-safe `write(STDERR_FILENO, ...)` 정적 문자열로 변경
   - `volatile sig_atomic_t _in_signal_handler` 플래그 추가
   - `Exit_Process()`에서: 플래그 설정 시 `Log()`, `sprintf()`, `Stat_Save()` 건너뛰기
   - 신호 경로에서 `exit(FAIL)` 대신 `_exit(FAIL)` 사용
   - `signal(SIGKILL, End_Routine)` 제거 — SIGKILL은 캐치할 수 없음

2. **7개 PA/PW `Catch_Signal()` 파일** — `Exit_Process()` 호출 전 신호 플래그 설정:
   - `Log()/SLog()`을 신호-안전 `write()`로 변경
   - `Exit_Process()` 호출 전 `_in_signal_handler = 1` 설정

3. **3개 PZ `Sig_Handler()` 파일** — `Log()`를 신호-안전 로깅으로 변경:
   - `write(STDERR_FILENO, ...)` 대신 `Log()` 사용
   - `signal()` 재등록 대신 `sigaction()` 사용 (경합 창 제거)

4. **`inc/fep_sub.h`** — `extern volatile sig_atomic_t _in_signal_handler` 추가

### 범위 외

- `Setsigfatal()` 자체의 `signal()` → `sigaction()` 마이그레이션 (별도 기능)
- 일반 (비신호) 코드 경로에서 `sprintf()` → `snprintf()` (별도 기능)
- Log() 내부 리팩토링

## 위험 평가

- **위험**: MEDIUM — 신호 핸들러는 중요 코드 경로이지만, 변경은 가산식 (플래그 기반)이고 구조 변경 아님
- **테스트**: 프로세스 시작/종료 + 신호 전달 (`kill -TERM`, `kill -USR1`)
- **롤백**: 단순 되돌리기
- **호환성**: `volatile sig_atomic_t`는 표준 C89, `_exit()`는 POSIX, `write()`는 POSIX

## 예상 영향

- 11개 파일 수정 (1 sub + 7 PA/PW + 3 PZ)
- 1개 헤더 업데이트 (fep_sub.h)
- 순 변화: ~+30 줄 (신호-안전 쓰기 문자열 + 플래그 확인)
- 함수 변화: 최소 — 신호 컨텍스트의 로그 메시지가 더 간단해짐 (타임스탬프/서식 없음) 하지만 여전히 stderr에서 보임
