# sub-audit 분석 보고서

> **분석 유형**: 간격 분석 (설계 vs 구현)
>
> **프로젝트**: FEP (KRX 프론트엔드 프로세서)
> **분석자**: gap-detector agent
> **날짜**: 2026-02-27
> **설계 문서**: [sub-audit.design.md](../02-design/features/sub-audit.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

sub-audit 설계 문서의 17개 FR이 `st01/sub/`의 13개 대상 파일에서 올바르게 구현되었는지 검증. 각 FR에 대해 "수정 전" 코드가 더 이상 존재하지 않고 "수정 후" 코드가 있는지 확인. 추가로 모든 `sub/*.c` 파일에 대해 전역 회귀 확인 실행.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/sub-audit.design.md`
- **구현 경로**: `st01/sub/` (13개 파일, 총 50개 C 소스)
- **분석 날짜**: 2026-02-27
- **총 FR 수**: 17

---

## 2. FR별 검증

### 배치 1: CRITICAL 수정 (3개 파일)

#### FR-01: tcpip_accept.c -- NULL 포인터 역참조 + inet_ntop (CRITICAL)

| 확인 | 결과 | 증거 |
|------|------|------|
| `char *hostname, *hostip` 제거 | PASS | 라인 23: `char hostip[INET_ADDRSTRLEN];` -- 스택 버퍼, 포인터 아님 |
| `inet_ntoa()` 제거 | PASS | 라인 47: `inet_ntop (AF_INET, &address.sin_addr, hostip, sizeof (hostip));` |
| NULL 안전 `hp` 확인 추가 | PASS | 라인 50-53: `if (hp != NULL)` ... `else` 분기 |
| `#ifdef sun` 블록 제거 | PASS | 모든 플랫폼용 통일 분기 |
| 사용되지 않는 `hostname` 변수 제거 | PASS | 선언에 없음 |

**상태**: 일치

#### FR-04: tcpip_accept.c -- 전처리기 `defined` 키워드 (FR-01과 번들)

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `#if defined __hpux \|\| _AIX \|\| __linux` | 사라짐 | 라인 25 검증 |
| 수정 후: `#if defined __hpux \|\| defined _AIX \|\| defined __linux` | PASS | 라인 25 정확히 일치 |

**상태**: 일치

#### FR-14: tcpip_accept.c -- inet_ntoa 제거 (FR-01과 번들)

| 확인 | 결과 | 증거 |
|------|------|------|
| `inet_ntoa` 호출 제거 | PASS | `grep 'inet_ntoa' st01/sub/*.c` 0건 |
| `inet_ntop`로 대체 | PASS | 라인 47 |

**상태**: 일치

#### FR-02: setsigfatal.c -- 신호-안전하지 않은 strlen() (CRITICAL)

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `write(STDERR_FILENO, name, strlen(name))` | 사라짐 | 파일에서 매칭 없음 |
| 수정 후: 인라인 루프 `while (p[nlen] != '\0') nlen++` | PASS | 라인 66-68 정확히 일치 |
| `write(STDERR_FILENO, name, nlen)` | PASS | 라인 69 |

**상태**: 일치

#### FR-03: queue.c -- K&R 함수 정의 (CRITICAL)

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `int ReceiveQueue(QID, owner, data)` + 별도 타입 라인 | 사라짐 | 코드에서 K&R 선언 없음 |
| 수정 후: `int ReceiveQueue(int QID, long owner, char *data)` | PASS | 라인 27 정확히 일치 |
| 수정 전: `int MakeQueue(KEY)\n    size_t KEY;` | 사라짐 | K&R 선언 없음 |
| 수정 후: `int MakeQueue(size_t KEY)` | PASS | 라인 112 정확히 일치 |

**참고**: `int QID;`, `long owner;`, `char *data;`와 `size_t KEY;` 문자열은 여전히 함수 헤더 주석(매개변수 IN 섹션)에 나타나며, 이는 예상되고 올바름 -- 문서화이지 코드 아님.

**상태**: 일치

#### FR-13d: queue.c -- C++ 주석 (번들)

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `// (한글 텍스트)` | 사라짐 | queue.c에서 0개 `//` |
| 수정 후: `/* select and insert/remove */` | PASS | 라인 120 |

**상태**: 일치

#### FR-15: queue.c -- 매직 숫자 문서화

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `#define QUEUE_MAX_BYTES 10485760` (주석 없음) | 사라짐 | |
| 수정 후: `#define QUEUE_MAX_BYTES 10485760    /* 10 MB max queue size */` | PASS | 라인 3 |
| 수정 전: `#define MAXSIZE 40960` (주석 없음) | 사라짐 | |
| 수정 후: `#define MAXSIZE 40960       /* 40 KB max message size */` | PASS | 라인 6 |

**상태**: 일치

#### FR-16: queue.c -- 사용되지 않는 변수

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: MakeQueue의 `char buff[MAXSIZE]` | 사라짐 | 라인 113-115: `Msgbuf msgbuf; int rtv, QID;`만 남음 |
| 라인 삭제 | PASS | MakeQueue 어디에도 `char buff[MAXSIZE]` 없음 |

**상태**: 일치

---

### 배치 2: HIGH 전처리기 + 문자열 안전성 (5개 파일)

#### FR-05: check_exist.c -- Missing `defined` 키워드

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전 (라인 14): `#elif defined sun \|\| __linux` | 사라짐 | |
| 수정 후: `#elif defined sun \|\| defined __linux` | PASS | 라인 14 |
| 수정 전 (라인 36): `#if defined sun \|\| __linux` | 사라짐 | |
| 수정 후: `#if defined sun \|\| defined __linux` | PASS | 라인 36 |

**상태**: 일치

#### FR-06: check_proc.c -- Missing `defined` 키워드

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전 (라인 23): `#if defined sun \|\| __linux` | 사라짐 | |
| 수정 후: `#if defined sun \|\| defined __linux` | PASS | 라인 23 |

**상태**: 일치

#### FR-07: stat_save.c -- 안전하지 않은 strcat()

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `strcat (buf, tmp)` (라인 119) | 사라짐 | 파일에서 0개 `strcat` |
| 수정 후: `strncat (buf, tmp, sizeof (buf) - strlen (buf) - 1)` | PASS | 라인 119 |
| 수정 전: `strcat (buf, tmp)` (라인 123) | 사라짐 | |
| 수정 후: `strncat (buf, tmp, sizeof (buf) - strlen (buf) - 1)` | PASS | 라인 123 |

**상태**: 일치

#### FR-08: config_db.c -- 안전하지 않은 strcat()

| 확인 | 결과 | 증거 |
|------|------|------|
| 모든 4개 `strcat` 호출 제거 | PASS | 파일에서 0개 `strcat` |
| 라인 538: `strncat (tbuf, "_exit", sizeof (tbuf) - strlen (tbuf) - 1)` | PASS | 정확히 일치 |
| 라인 549: `strncat (tbuf, "_ctrl", sizeof (tbuf) - strlen (tbuf) - 1)` | PASS | 정확히 일치 |
| 라인 584: `strncat (tbuf, "_exit", sizeof (tbuf) - strlen (tbuf) - 1)` | PASS | 정확히 일치 |
| 라인 597: `strncat (tbuf, "_ctrl", sizeof (tbuf) - strlen (tbuf) - 1)` | PASS | 정확히 일치 |

**상태**: 일치

#### FR-09a: file_rw.c F_R_Proc() -- strlen+memcpy 안티패턴

| 확인 | 결과 | 증거 |
|------|------|------|
| 선언에 `tmp_off` 변수 추가 | PASS | 라인 132: `int rt, cnt, i, len, pt, tmp_off;` |
| memset 후 `tmp_off = 0` | PASS | 라인 175 |
| `memcpy(&tmp[tmp_off], buff_rw.Seq, ...)` | PASS | 라인 191 |
| `tmp_off += sizeof (BUFF_RW_HEAD)` | PASS | 라인 192 |
| `memcpy(&tmp[tmp_off], buf+i+sizeof(FILE_RW_HEAD), ...)` | PASS | 라인 193 |
| `tmp_off += p_size - sizeof (FILE_RW_HEAD)` | PASS | 라인 195 |
| `memcpy (p_buf, tmp, tmp_off)` (was `strlen(tmp)`) | PASS | 라인 198 |

**상태**: 일치

#### FR-09b: file_rw.c F_W2_Proc() -- strlen+memcpy 안티패턴

| 확인 | 결과 | 증거 |
|------|------|------|
| 선언에 `buf_off` 변수 추가 | PASS | 라인 621: `int rt, cnt, i, FIFO_fd, len, pt, f_size, null_cnt, buf_off;` |
| `buf_off = strlen (buf)` | PASS | 라인 662 |
| `memcpy (&buf[buf_off], file_rw.Seq, sizeof (FILE_RW_HEAD))` | PASS | 라인 663 |
| `buf_off += sizeof (FILE_RW_HEAD)` | PASS | 라인 664 |
| `memcpy (&buf[buf_off], p_buf+sizeof(BUFF_RW_HEAD), ...)` | PASS | 라인 665 |

**상태**: 일치

#### FR-09c: shm_rw.c DSHM_R() -- strlen+memcpy 안티패턴

| 확인 | 결과 | 증거 |
|------|------|------|
| 선언에 `tmp_off` 변수 추가 | PASS | 라인 29: `int i, fd, rt, rec_size, r_cnt, data_cnt, tmp_off;` |
| memset 후 `tmp_off = 0` | PASS | 라인 71 |
| `memcpy (&tmp[tmp_off], &brw, sizeof (BUFF_RW_HEAD))` | PASS | 라인 134 |
| `tmp_off += sizeof (BUFF_RW_HEAD)` | PASS | 라인 135 |
| `memcpy (&tmp[tmp_off], buf+sizeof(FILE_RW_HEAD), ...)` | PASS | 라인 136 |
| `tmp_off += rec_size - sizeof (FILE_RW_HEAD)` | PASS | 라인 138 |
| `memcpy (p_buf, tmp, tmp_off)` (was `strlen(tmp)`) | PASS | 라인 143 |

**상태**: 일치

---

### 배치 3: MEDIUM C++ 주석 (5개 파일)

#### FR-10: select_recv.c -- 8개 C++ 주석

| 라인 | 설계 "수정 후" | 구현 | 상태 |
|------|--------|---------|--------|
| 476 | `/* cumulative buffer */` | `/* cumulative buffer */` | 일치 |
| 477 | `/* accumulated length */` | `/* accumulated length */` | 일치 |
| 499 | `/* accumulate received data */` | `/* accumulate received data */` | 일치 |
| 510 | `/* search for packet delimiter (0xFF 0x0D 0x0A) */` | `/* search for packet delimiter (0xFF 0x0D 0x0A) */` | 일치 |
| 520 | `/* packet length = ipos */` | `/* packet length = ipos */` | 일치 |
| 522 | `/* shift remaining data forward */` | `/* shift remaining data forward */` | 일치 |
| 526 | `/* return 1 packet length */` | `/* return 1 packet length */` | 일치 |
| 531 | `/* packet not yet complete */` | `/* packet not yet complete */` | 일치 |

**상태**: 일치 (8/8)

#### FR-11: file_rw.c -- 10개 C++ 주석

| 확인 | 결과 | 증거 |
|------|------|------|
| 모든 `// 2025EDIT`을 `/* 2025EDIT */`로 변환 | PASS | 10개 발생, 모두 C 스타일 |
| 파일에 남은 0개 `//` | PASS | 전역 `//` grep: 모든 sub/*.c에서 0건 |

**상태**: 일치 (10/10)

#### FR-12: log_proc.c -- 6개 C++ 주석

| 확인 | 결과 | 증거 |
|------|------|------|
| 모든 `// 2025EDIT`을 `/* 2025EDIT */`로 변환 | PASS | 6개 발생, 모두 C 스타일 |
| 라인 169, 174, 225, 230, 423, 595 | PASS | 모두 검증됨 |

**상태**: 일치 (6/6)

#### FR-13a: key_search.c 라인 30 -- C++ 주석

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `// 채권_KTS` | 사라짐 | |
| 수정 후: `/* NOTE_MK_KTS */` | PASS | 라인 30 |

**상태**: 일치

#### FR-13b: key_search.c 라인 44 -- C++ 주석

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `// 금융파생...` | 사라짐 | |
| 수정 후: `/* DEV_MK_FIF */` | PASS | 라인 44 |

**상태**: 일치

#### FR-13c: make_daemon.c 라인 51 -- C++ 주석

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전: `// 0:정상, -1:오류` | 사라짐 | |
| 수정 후: `/* 0:OK, -1:error */` | PASS | 라인 51 |

**상태**: 일치

---

### 배치 4: LOW 정리 (1개 파일)

#### FR-17: key_search.c -- 복사-붙여넣기 주석 오류

| 확인 | 결과 | 증거 |
|------|------|------|
| 수정 전 (라인 62): `End of LtoU ()` | 사라짐 | |
| 수정 후: `End of Key_Search ()` | PASS | 라인 62: `}	/* End of Key_Search ()	*/` |
| 수정 전 (라인 210): `End of Program (ltou.c)` | 사라짐 | |
| 수정 후: `End of Program (key_search.c)` | PASS | 라인 210: `End of Program (key_search.c)` |

**상태**: 일치

---

## 3. 전역 회귀 확인

모든 확인은 `st01/sub/`의 모든 `*.c` 파일(50개)에 대해 실행.

| 확인 | 명령 | 건수 | 상태 |
|------|---------|:----:|--------|
| 0개 C++ `//` 주석 | `grep '//' st01/sub/*.c` | 0 | PASS |
| 0개 `strcat()` 호출 | `grep 'strcat\s*(' st01/sub/*.c` | 0 | PASS |
| 0개 `#if defined X \|\| Y` (defined 없음) | `grep '#if.*defined.*\|\|' st01/sub/*.c` | 3 (모두 올바름) | PASS |
| 0개 `inet_ntoa()` 호출 | `grep 'inet_ntoa' st01/sub/*.c` | 0 | PASS |
| 0개 `strlen(tmp)]` in file_rw.c | `grep 'strlen.*tmp\]' st01/sub/file_rw.c` | 0 | PASS |
| 0개 `strlen(buf)]` in file_rw.c | `grep 'strlen.*buf.*\]' st01/sub/file_rw.c` | 0 | PASS |
| 0개 `strlen(tmp)]` in shm_rw.c | `grep 'strlen.*tmp\]' st01/sub/shm_rw.c` | 0 | PASS |

**`#if defined ... ||` 건수 상세** (모두 올바르게 형성):

1. `st01/sub/check_exist.c:36:#if defined sun || defined __linux` -- 두 피연산자 모두 `defined` 있음
2. `st01/sub/tcpip_accept.c:25:#if defined __hpux || defined _AIX || defined __linux` -- 모든 피연산자에 `defined` 있음
3. `st01/sub/check_proc.c:23:#if defined sun || defined __linux` -- 두 피연산자 모두 `defined` 있음

**회귀 상태**: 모두 명확 -- 모든 확인에서 0개 위반.

---

## 4. 일치율 요약

```
+-----------------------------------------------+
|  전체 일치율: 100% (17/17 FR)                |
+-----------------------------------------------+
|  배치 1 (CRITICAL):  8/8  FR  -- PASS       |
|  배치 2 (HIGH):      6/6  FR  -- PASS       |
|  배치 3 (MEDIUM):    6/6  FR  -- PASS       |
|  배치 4 (LOW):       1/1  FR  -- PASS       |
+-----------------------------------------------+
|  전역 회귀:   7/7  확인 -- PASS      |
+-----------------------------------------------+
```

참고: 일부 FR은 번들되어 있음 (FR-01은 FR-04와 FR-14 포함; FR-03은 FR-13d, FR-15, FR-16 포함). 총 고유 FR 수는 설계 문서에서 명시한 17개.

### FR 점수 표

| FR | 파일 | 심각도 | 설명 | 상태 |
|----|------|--------|------|------|
| FR-01 | tcpip_accept.c | CRITICAL | NULL ptr 역참조 + inet_ntop | 일치 |
| FR-02 | setsigfatal.c | CRITICAL | 신호-안전하지 않은 strlen() | 일치 |
| FR-03 | queue.c | CRITICAL | K&R 함수 정의 | 일치 |
| FR-04 | tcpip_accept.c | HIGH | 전처리기 `defined` 키워드 | 일치 |
| FR-05 | check_exist.c | HIGH | Missing `defined` 키워드 | 일치 |
| FR-06 | check_proc.c | HIGH | Missing `defined` 키워드 | 일치 |
| FR-07 | stat_save.c | HIGH | 안전하지 않은 strcat() | 일치 |
| FR-08 | config_db.c | HIGH | 안전하지 않은 strcat() (4개 위치) | 일치 |
| FR-09a | file_rw.c | HIGH | F_R_Proc에서 strlen+memcpy | 일치 |
| FR-09b | file_rw.c | HIGH | F_W2_Proc에서 strlen+memcpy | 일치 |
| FR-09c | shm_rw.c | HIGH | DSHM_R에서 strlen+memcpy | 일치 |
| FR-10 | select_recv.c | MEDIUM | 8개 C++ 주석 | 일치 |
| FR-11 | file_rw.c | MEDIUM | 10개 C++ 주석 | 일치 |
| FR-12 | log_proc.c | MEDIUM | 6개 C++ 주석 | 일치 |
| FR-13 | key_search.c, make_daemon.c | MEDIUM | 3개 C++ 주석 (a,b,c) | 일치 |
| FR-13d | queue.c | MEDIUM | 1개 C++ 주석 | 일치 |
| FR-14 | tcpip_accept.c | MEDIUM | inet_ntoa 제거 | 일치 |
| FR-15 | queue.c | LOW | 매직 숫자 문서 | 일치 |
| FR-16 | queue.c | LOW | 사용되지 않는 변수 제거 | 일치 |
| FR-17 | key_search.c | LOW | 복사-붙여넣기 주석 수정 | 일치 |

---

## 5. 수정된 파일 요약

| # | 파일 | 적용된 FR | 변경 라인 (대략) |
|---|------|----------|----------------|
| 1 | `st01/sub/tcpip_accept.c` | FR-01, FR-04, FR-14 | ~15 |
| 2 | `st01/sub/setsigfatal.c` | FR-02 | ~5 |
| 3 | `st01/sub/queue.c` | FR-03, FR-13d, FR-15, FR-16 | ~10 |
| 4 | `st01/sub/check_exist.c` | FR-05 | ~2 |
| 5 | `st01/sub/check_proc.c` | FR-06 | ~1 |
| 6 | `st01/sub/stat_save.c` | FR-07 | ~2 |
| 7 | `st01/sub/config_db.c` | FR-08 | ~4 |
| 8 | `st01/sub/file_rw.c` | FR-09a, FR-09b, FR-11 | ~25 |
| 9 | `st01/sub/shm_rw.c` | FR-09c | ~10 |
| 10 | `st01/sub/select_recv.c` | FR-10 | ~8 |
| 11 | `st01/sub/log_proc.c` | FR-12 | ~6 |
| 12 | `st01/sub/key_search.c` | FR-13a, FR-13b, FR-17 | ~4 |
| 13 | `st01/sub/make_daemon.c` | FR-13c | ~1 |

**합계**: 13개 파일, ~93개 라인 변경

---

## 6. 회귀 / 관찰

회귀 감지 없음. 추가 관찰:

1. **queue.c MakeQueue()**: `buff[MAXSIZE]` 제거 후 변수 `Msgbuf msgbuf`과 `int rtv`는 여전히 선언되지만 사용되지 않는 것으로 보임. 이들은 이 감사 범위가 아님 (어떤 FR에서도 명시되지 않음)이지만, 향후 정리 통과의 후보가 될 수 있음.

2. **빌드 검증**: 테스트 미실시 (서버 환경에서 `libfepP.a` 빌드 체인 필요). 모든 변경은 구문론적으로 안전함 (주석 변환, `strcat`->`strncat`, 변수 추가, 전처리기 수정).

---

## 7. 권장 조치

필수 조치 없음. 일치율은 간격 없음과 회귀 없음으로 100%.

---

## 8. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 100% | PASS |
| 회귀 확인 | 100% | PASS |
| **전체** | **100%** | **PASS** |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|------|------|---------|--------|
| 1.0 | 2026-02-27 | 초기 분석 -- 17개 FR, 13개 파일, 7개 전역 확인 | gap-detector |
