# PZ 모듈 코드 품질 감사 — 계획 문서

> **기능**: PZ (시스템 관리 모듈 코드 품질 감사)
> **프로젝트**: FEP (KRX Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-27
> **상태**: 초안

---

## 목표

모든 10개 PZ (시스템 관리) 모듈 소스 파일의 체계적 코드 품질 감사. 버그, 안전하지 않은 패턴, C89 준수 문제 및 데드 코드를 수정합니다. 완료된 PA (100%), PB (100%), PX (100%) 모듈 감사의 패턴과 교훈을 활용합니다.

## 범위

- **포함**: `st01/src/PZ/`의 모든 10개 `.c` 파일 (활성 파일만)
- **제외**: 백업 파일 (`BACKUP/`), 헤더 (`inc/`), 공유 라이브러리 (`sub/`), 다른 모듈 (PA/PB/PX)
- **이미 수정됨**: 신호 핸들러 비동기 안전성 (signal-handler-safety 기능에서 다룸), 대부분의 strcpy (unsafe-strcpy-conversion 기능에서 다룸)

## 소스 파일

| 파일 | 라인 | 역할 |
|------|-------|------|
| pz_memory_conf.c | ~3400 | 설정 INI 파일 파싱 및 SHM 로딩 |
| pz_daemon_proc.c | ~1030 | 데몬 프로세스 생명주기 (시작/중지/신호) |
| pz_procchk.c | ~1010 | 프로세스 상태 모니터링 및 연결 리스트 관리 |
| pz_memory_proc.c | ~300 | 프로세스 상태 SHM 로딩 |
| pz_fepp.c | ~800 | FEP 데몬 메인 및 제어 FIFO 처리 |
| pz_compact.c | ~260 | 이전 데이터/로그 정리 (rm -rf) |
| pz_memory_shm.c | ~200 | 공유 메모리 생성 |
| pz_memory.c | ~60 | 메모리 프로세스 메인 진입점 |
| pz_daemon.c | ~55 | 데몬 프로세스 메인 진입점 |
| pz_filechk.c | ~55 | 파일 확인 유틸리티 |

## 발견 사항 요약

| 심각도 | 개수 | 테마 |
|----------|-------|-------|
| CRITICAL | 7 | 형식 문자열 버그 (2개), 전처리기 defined() 문법 (5개) |
| HIGH | 6 | 데드 코드, fd 누수, 초기화되지 않은 cmd, 체크되지 않은 write() |
| MEDIUM | 10 | 댕글링 포인터, atexit 오용, r_uid/pid 불일치, C99, fopen 스타일 |
| LOW | 4 | 사용하지 않는 변수, K&R 매개변수, C++ 주석, 하드코딩된 경로 |
| **합계** | **27** | |
| 연기됨 | 2 | 셸 주입 (fork/exec 리팩터 필요), strncpy NUL (2025EDIT 코드) |

---

## 구현 배치

### 배치 1: CRITICAL 수정 (FR-01부터 FR-07) — 7개 발견

| FR | 파일 | 라인 | 이슈 | 수정 |
|----|------|------|-------|-----|
| FR-01 | pz_daemon_proc.c | 760 | 형식 문자열 `[%]` — 잘못된 변환 지시자, 엄격한 컴파일러에서 UB | `[%]`를 `[%s]`로 변경 |
| FR-02 | pz_memory_conf.c | 125 | `strlen(buf)`를 `%s` 형식으로 전달 — 정수를 포인터로 역참조, 보장된 크래시 | `%s`를 `%d`로 변경하고 `(int)strlen(buf)` 캐스팅, 또는 인자 제거 |
| FR-03 | pz_procchk.c | 998 | `#if defined __hpux \|\| sun \|\| _AIX` — `sun`과 `_AIX`가 `defined()`로 래핑되지 않음 | `#if defined(__hpux) \|\| defined(sun) \|\| defined(_AIX)` |
| FR-04 | pz_memory_proc.c | 199 | FR-03과 동일한 전처리기 패턴 | 동일한 수정 |
| FR-05 | pz_memory_proc.c | 225 | FR-03과 동일한 전처리기 패턴 | 동일한 수정 |
| FR-06 | pz_memory_proc.c | 280 | FR-03과 동일한 전처리기 패턴 | 동일한 수정 |
| FR-07 | pz_memory_conf.c | 1468 | FR-03과 동일한 전처리기 패턴 | 동일한 수정 |

> **참고**: PZ는 `#if defined __hpux || sun || _AIX`를 사용합니다 (`__linux` 없음, `#elif` 대신 `#if` 사용). `#elif defined sun || _AIX || __linux`를 사용한 PX와는 다른 패턴입니다.

### 배치 2: HIGH 수정 (FR-08부터 FR-13) — 6개 발견

| FR | 파일 | 라인 | 이슈 | 수정 |
|----|------|------|-------|-----|
| FR-08 | pz_procchk.c | 240-241 | 데드 코드: `continue` 이후 `NR = 0` — NR 카운터가 초기화되지 않아 후속 사이클에서 거짓 "실행 안 함" 보고 발생 | `NR = 0`을 `continue` 이전으로 이동하거나 순서를 `NR = 0; continue;`로 변경 |
| FR-09 | pz_procchk.c | 258-266 | 파일 디스크립터 누수: 루프 내 `fp_b = fopen("/tmp/mrt1")` 호출, 닫히지 않음 | else 분기에서 `fgets` 후 `fclose(fp_b)` 추가 |
| FR-10 | pz_procchk.c | 970-996 | `flag`가 1-3 범위 밖일 때 초기화되지 않은 `cmd`로 `system(cmd)` 호출 | `flag==3` 분기 후 `else { Log(error); return; }` 추가 |
| FR-11 | pz_fepp.c | 405 | 데몬 종료 FIFO에서 체크되지 않은 `write(FIFO_fd, "1", 1)` | 반환값 체크 및 실패 시 Log 추가 |
| FR-12 | pz_fepp.c | 784 | 프로세스 종료 FIFO에서 체크되지 않은 `write(FIFO_fd, "1", 1)` | FR-11과 동일한 수정 |
| FR-13 | pz_daemon_proc.c | 979 | 신호 핸들러에서 체크되지 않은 `write(DTART_FD, "1", 1)` | 반환값 체크 추가 (write-in-signal-safe 패턴 사용) |

### 배치 3: MEDIUM 수정 (FR-14부터 FR-23) — 10개 발견

| FR | 파일 | 라인 | 이슈 | 수정 |
|----|------|------|-------|-----|
| FR-14 | pz_procchk.c | 788-794 | Initialize_Link: Tail NULL 확인 이전에 `Head->next = Tail` 설정; Tail malloc 실패 시 `free(Head)`로 전역 Head 댕글링 | `free(Head)` 후 `Head = NULL;` 추가 |
| FR-15 | pz_procchk.c | 207 | 모니터링 루프 내 `atexit(Release_Link)` 호출 — ~1440회/일 등록, 결국 atexit 오버플로우, 종료 시 Release_Link 여러 번 호출 | 메인 루프 이전으로 `atexit()` 호출 이동 (일회 등록) |
| FR-16 | pz_procchk.c | 705 | `Ordered_Insert(r_uid, r_pname)` — PID 대신 UID 전달; `Ordered_Insert` 매개변수는 `pid_t pid` | PID로 `atoi(dirp->d_name)` 전달 (`r_uid` 대신) |
| FR-17 | pz_daemon_proc.c | 889-891 | `Stop_Process()`의 사용하지 않는 변수 `k`, `proc_x_no`, `cmd` | 사용하지 않는 선언 제거 |
| FR-18 | pz_memory_conf.c | 368,412,428+ | 2025EDIT 블록의 C99 mid-block 선언 (`char temp_buf01[...]`, `size_t max_tmp_len01`) | 함수 최상단으로 선언 이동 (C89 준수) |
| FR-19 | pz_procchk.c | 632,647 | 일시적 `/proc/PID/status` 열기/파싱 실패 시 `continue` 대신 `return` — 전체 프로세스 스캔 중단 | `return`을 `continue`로 변경 (한 프로세스 스킵, 계속 스캔) |
| FR-20 | pz_compact.c | 242 | `memcmp(dir->d_name, ProcDate, strlen(dir->d_name))` — 가변 길이로 비교; 비날짜 디렉토리 이름은 너무 적은 문자로 비교 | 고정 길이 8 사용 (YYYYMMDD 형식) |
| FR-21 | pz_memory.c | 32 | `sprintf(_Exe_Name, "%s", argv[0])` — 전역 버퍼에 경계 확인 없음 | `sizeof(_Exe_Name)`과 함께 `snprintf` 사용 |
| FR-22 | pz_memory_shm.c | 146-147 | 중괄호 블록 내 C99 mid-block 선언 (`int ver`, `int mag`) | 블록 최상단으로 선언 이동 또는 재구성 |
| FR-23 | pz_memory_conf.c | 61,658,931,1286,1493,1592,1877,2061,2271 | `fopen(...) == 0` — FILE*을 정수 0과 비교 (NULL 대신) | `== 0`을 `== NULL`로 변경 (9개 위치) |

### 배치 4: LOW 수정 (FR-24부터 FR-27) — 4개 발견

| FR | 파일 | 라인 | 이슈 | 수정 |
|----|------|------|-------|-----|
| FR-24 | pz_memory_shm.c | 28,57; pz_memory_conf.c:640,1864 | 사용하지 않는 변수 (Sub_SHM_Creat의 `i`, Mem_SHM_Creat의 `j`, File_Config_Read의 `fd`, Tcp2_Config_Read의 `i`) | 사용 제거 또는 확인 |
| FR-25 | pz_daemon_proc.c | 467,527 | K&R 함수 정의 — `SHM_Load_Process ()` 및 `SHM_Backup_Process ()` 빈 괄호 포함 | C89 프로토타입 정확성을 위해 `(void)`로 변경 |
| FR-26 | 5개 활성 파일 | 복수 | C++ `//` 주석 (총 61개: pz_memory_conf.c 41개, pz_daemon_proc.c 9개, pz_procchk.c 6개, pz_memory_shm.c 3개, pz_fepp.c 2개) — non-C89 | `/* */` C89 주석으로 변환 |
| FR-27 | pz_procchk.c | 256 | 하드코딩된 `/tmp/mrt1` 임시 파일 경로 — 다른 프로세스와의 충돌 위험 | `mkstemp()` 또는 프로세스별 임시 경로 사용 |

### 의도적으로 연기됨

| ID | 이슈 | 파일:라인 | 이유 |
|----|-------|-----------|--------|
| D-1 | readdir/SHM 프로세스 이름으로 `system()`을 통한 셸 주입 | pz_procchk.c:256, pz_compact.c:245 | `system()`을 `fork()/exec()`로 바꾸기 필요 — 대규모 리팩터. 내부 데이터 소스만 (SHM, /proc), 외부 입력 없음. |
| D-2 | 2025EDIT 블록의 NUL 종료 없는 `strncpy` | pz_memory_conf.c 복수 | 최근 추가 코드 (2025EDIT). strncpy+manual NUL 패턴이 대부분 적용되어 있음. 나머지 엣지 케이스는 신중한 비즈니스 로직 검토 필요. |

---

## 위험 평가

| 위험 | 영향 | 완화 |
|------|--------|------------|
| FR-01 (형식 문자열) | 엄격한 컴파일러에서 로그 메시지 손상 | 낮은 위험: 로깅만, 데이터 손상 없음 |
| FR-02 (strlen as %s) | **크래시** — 정수를 포인터로 역참조 | 높은 위험: 설정 파싱 오류 경로에서 발생. 수정은 간단함. |
| FR-08 (dead NR=0) | 거짓 "프로세스 실행 중 아님" 보고, 누락된 재시작 | 중간 위험: 모니터링 정확성 영향, 크래시된 프로세스 놓칠 수 있음 |
| FR-10 (초기화되지 않은 cmd) | `system()`이 쓰레기 문자열 실행 | 높은 위험: UB, 잠재적 임의 명령 실행. flag가 실제로 항상 1-3이므로 완화됨. |
| FR-16 (r_uid vs PID) | 프로세스 연결 리스트가 잘못된 식별자 저장 | 중간 위험: 프로세스 존재 확인 정확성 영향 |
| FR-19 (return vs continue) | 단일 /proc 실패가 전체 프로세스 스캔 중단 | 중간 위험: /proc의 일시적 실패는 프로세스 시작/종료 중 정상 |
| FR-26 (C++ 주석) | 엄격한 C89 컴파일러가 `//` 주석 거부 | 낮은 위험: 모든 대상 컴파일러는 실제로 `//` 지원. 준수를 위한 수정. |

## 성공 기준

- 모든 CRITICAL 및 HIGH 발견 사항 수정
- 모든 MEDIUM 발견 사항 수정
- 모든 LOW 발견 사항 수정
- 대상 Linux 서버에서 빌드 검증 통과
- 간격 분석 >= 90% 일치율

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | 10개 PZ 파일에 27개 발견 사항이 있는 초기 계획 | Claude Code |
