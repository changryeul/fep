# 계획: sub-audit

> sub/ 모듈 (libfepP.a 공유 라이브러리) 포괄적 코드 품질 감사

## 1. 개요

| 항목 | 세부사항 |
|------|--------|
| 기능 | sub-audit |
| 유형 | 코드 품질 감사 |
| 범위 | `st01/sub/` 내 50개 C 소스 파일 |
| 우선순위 | HIGH |
| 예상 FR 수 | ~35 |

### 배경

`sub/` 모듈은 공유 서브루틴 라이브러리(`libfepP.a`)로, FEP 시스템의 모든 프로세스 바이너리에 링크됩니다. 여기의 버그는 시스템 전체에 영향을 미칩니다. 이전 횡단 PDCA 사이클들이 많은 문제들을 수정했지만(getenv-null-check, signal-handler-safety, unsafe-strcpy-conversion, setsockopt-return-check, select-recv-bounds-check, inet-addr-to-pton 등), 전용 모듈 수준의 감사는 아직 수행되지 않았습니다. 이 감사는 횡단 통과에서 놓친 남은 문제들을 목표로 합니다.

### 목표

1. 남은 모든 C89 준수 위반 사항 수정 (C++ 주석, K&R 선언)
2. CRITICAL/HIGH 버그 수정 (NULL 포인터 역참조, strcat 오버플로, 신호 안전성, 전처리기 오류)
3. 견고성 개선 (미확인 시스템 호출 반환, sprintf 오버플로 위험)
4. 데드 코드 정리 및 코딩 표준 위반 해결

## 2. 범위

### 범위 내 (50개 파일)

`st01/sub/`의 모든 활성 `.c` 파일:

| 카테고리 | 파일 | 개수 |
|----------|------|------|
| 설정/DB | config_db.c, config_loader.c | 2 |
| 공통/중복제거 | fep_common.c, fep_encrypt.c, fifo_event.c | 3 |
| SHM | shmipc.c, shmsub.c, shm_rw.c | 3 |
| TCP/IP | tcpip_{accept,bind,connect,listen,recv,select,send,sock}.c | 8 |
| 파일 I/O | file_rw.c, poll_file.c, select_{recv,send}.c, file_util.c | 5 |
| 프로세스 | make_daemon.c, init_proc.c, init_mana.c, check_proc.c, check_env.c, check_exist.c | 6 |
| 로깅 | log_proc.c, stat_save.c, seq_save.c | 3 |
| 변환 | atodf.c, atolf.c, atoif.c, itoaf.c | 4 |
| 유틸리티 | key_search.c, queue.c, gettime.c, getfileno.c, getenvironment.c, set_trtime.c, ltou.c, utol.c, chk_kor.c, hoga_check.c, display_errmsg.c, setsigfatal.c, semipc.c | 13 |
| 새 공유 | ip_format.c, poll_event.c, udp_init.c | 3 |

### 범위 외

- `inc/` 헤더 파일 (header-cpp-comment-c89 사이클에서 감사됨)
- 삭제된 파일 (dtoaf.c는 dtoaf-dead-code-delete 사이클에서 제거됨)
- 빌드 시스템 / Makefile
- config_db.c 내부 SQL 로직 (DB 유틸리티 코드, 런타임 중요도 낮음)

## 3. 발견 사항 요약

### CRITICAL (3개 발견사항)

| ID | 파일 | 이슈 | 설명 |
|----|------|------|------|
| FR-01 | tcpip_accept.c:53 | NULL 포인터 역참조 | `gethostbyaddr()` 이후 NULL 확인 없이 `hp->h_name` 사용. DNS 해석 실패 시 `hp`는 NULL이 되어 실패한 모든 호스트명 조회에서 SIGSEGV 크래시 발생. |
| FR-02 | setsigfatal.c:65 | 신호-안전하지 않은 strlen() | 신호 핸들러 `End_Routine()` 내에서 `strlen(name)` 호출. `strlen()`은 POSIX async-signal-safe로 보장되지 않음. `sig_names[]`에서 사전 계산된 문자열 길이 사용 필요. |
| FR-03 | queue.c:27,115 | K&R 구식 함수 정의 | `ReceiveQueue(QID, owner, data)`와 `MakeQueue(KEY)`는 ANSI 이전 K&R 매개변수 선언 사용. 엄격한 C89 프로토타입과 호환되지 않음. 암시적 타입 승격 버그 발생. |

### HIGH (6개 발견사항)

| ID | 파일 | 이슈 | 설명 |
|----|------|------|------|
| FR-04 | tcpip_accept.c:25 | 전처리기 missing `defined` | `#if defined __hpux \|\| _AIX \|\| __linux` — `_AIX`와 `__linux`는 정의-확인이 아닌 표현식(0)으로 평가. AIX/Linux에서 잘못된 타입 사용. |
| FR-05 | check_exist.c:14,36 | 전처리기 missing `defined` | `#elif defined sun \|\| __linux` — `__linux`는 표현식으로 평가. Linux는 포함 및 전역에 대해 잘못된 코드 경로 이동. |
| FR-06 | check_proc.c:23 | 전처리기 missing `defined` | `#if defined sun \|\| __linux` — check_exist.c와 동일한 패턴. |
| FR-07 | stat_save.c:119,123 | 안전하지 않은 strcat() | `strcat(buf, tmp)`는 경계 확인 없이 1024바이트 `buf`에 추가. 현재 데이터는 작지만 TCP2 라인 수 증가 시 오버플로 가능. |
| FR-08 | config_db.c:538,549,584,597 | 안전하지 않은 strcat() | `strcat(tbuf, "_exit")`과 `strcat(tbuf, "_ctrl")`을 20바이트 `tbuf`에 적용. 선행 strncpy(mlen=14)로 인해 현재는 안전하지만 취약 — 한 상수 변경이 문제 발생. |
| FR-09 | file_rw.c:190-191,659-660; shm_rw.c:133-134 | strlen+memcpy 안티패턴 | `memcpy(&tmp[strlen(tmp)], ...)`는 이진 데이터 버퍼를 문자열로 처리. `tmp`에 내장 NUL 바이트 포함 시(KRX 이진 메시지에서 흔함), strlen은 잘못된 오프셋 반환으로 데이터 손상 발생. |

### MEDIUM (5개 발견사항)

| ID | 파일 | 이슈 | 설명 |
|----|------|------|------|
| FR-10 | select_recv.c:476-531 | C++ 주석 (8개) | 한글 인라인 주석이 `//` 스타일 사용. |
| FR-11 | file_rw.c:305-990 | C++ 주석 (10개) | `// 2025EDIT` 마커. |
| FR-12 | log_proc.c:169-595 | C++ 주석 (6개) | `// 2025EDIT` 마커. |
| FR-13 | key_search.c:30,44; make_daemon.c:51; queue.c:125 | C++ 주석 (4개) | 한글 인라인 주석. |
| FR-14 | tcpip_accept.c:47 | 사용 중단된 inet_ntoa() | `inet_ntoa()`는 스레드 안전하지 않으며 사용 중단됨. inet-addr-to-pton 감사와의 일관성을 위해 `inet_ntop()` 사용 필요. |

### LOW (3개 발견사항)

| ID | 파일 | 이슈 | 설명 |
|----|------|------|------|
| FR-15 | queue.c:3,6 | 매직 숫자 | `QUEUE_MAX_BYTES=10485760`, `MAXSIZE=40960` — 문서화되지 않은 크기. |
| FR-16 | queue.c:118 | 사용되지 않는 변수 | `char buff[MAXSIZE]`는 선언되었지만 `MakeQueue()`에서 사용되지 않음. |
| FR-17 | key_search.c:62,210 | 주석의 잘못된 함수명 | `End of LtoU()`과 `End of Program (ltou.c)` — ltou.c에서 복사-붙여넣기. `Key_Search` / `key_search.c`로 변경 필요. |

## 4. 구현 전략

### 배치 구성

**배치 1: CRITICAL 수정 (FR-01 ~ FR-03)** — 3개 파일
- tcpip_accept.c: gethostbyaddr NULL 확인 + inet_ntop 마이그레이션 (FR-01, FR-04, FR-14)
- setsigfatal.c: 신호 핸들러용 사전 계산 strlen (FR-02)
- queue.c: K&R → ANSI 프로토타입 + 데드 변수 + C++ 주석 + 매직 숫자 (FR-03, FR-13d, FR-15, FR-16)

**배치 2: HIGH 전처리기 + 문자열 안전성 (FR-04 ~ FR-09)** — 5개 파일
- check_exist.c: missing `defined` 키워드 추가 (FR-05)
- check_proc.c: missing `defined` 키워드 추가 (FR-06)
- stat_save.c: strcat → snprintf (FR-07)
- config_db.c: strcat → snprintf (FR-08)
- file_rw.c + shm_rw.c: strlen+memcpy → 오프셋 추적 (FR-09)

**배치 3: MEDIUM C++ 주석 (FR-10 ~ FR-13)** — 5개 파일
- select_recv.c: 8개 C++ 주석 → C89 (FR-10)
- file_rw.c: 10개 C++ 주석 → C89 (FR-11)
- log_proc.c: 6개 C++ 주석 → C89 (FR-12)
- key_search.c, make_daemon.c: 3개 C++ 주석 → C89 (FR-13)

**배치 4: LOW 정리 (FR-15 ~ FR-17)** — 2개 파일
- queue.c: 매직 숫자 문서화 (FR-15)
- key_search.c: 복사-붙여넣기 주석 오류 수정 (FR-17)

### 위험 평가

| 위험 | 영향 | 완화 |
|------|------|------|
| strlen+memcpy 변경으로 이진 프로토콜 손상 | HIGH | 모든 호출자를 추적하여 오프셋 계산 논리 검증 |
| K&R→ANSI 프로토타입 변경으로 호출자 영향 | LOW | queue.c 함수는 고정된 호출 시그니처 보유 |
| gethostbyaddr NULL 확인으로 로그 출력 변경 | LOW | 정상 저하 — 호스트명 미사용 시 IP만 로깅 |

## 5. 성공 기준

- 모든 17개 FR 구현
- sub/ 내 남은 C++ `//` 주석 없음
- `#if defined X || Y` 형태에 `defined` 키워드 없음
- 남은 strcat() 호출 없음
- K&R 구식 함수 정의 없음
- 신호 핸들러는 async-signal-safe 함수만 사용
- 간격 분석 일치율 >= 90%

## 6. 참조

- 이전 횡단 감사: `docs/archive/2026-02/_INDEX.md` 참조
- PX 감사: `docs/archive/2026-02/PX/`
- PZ 감사: `docs/archive/2026-02/PZ/`
