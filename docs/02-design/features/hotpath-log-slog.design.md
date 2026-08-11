# hotpath-log-slog Design Document

> **Summary**: 주문 핫패스의 메시지당 INFO 로그를 디스크 직접 기록(`Log`, 라인당 stat+open+write+close ≈ 4 syscall + 디스크 I/O)에서 SHM 링 경유(`SLog`, semop×2 + FIFO write 1바이트 — 디스크 I/O는 LogManager로 이관)로 전환하는 opt-in 라우터 `Log_Hot` 도입.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-05
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F2

---

## 1. 조사 결과 (I-1: SLog 인프라)

- **링 구조**: `SHM_LOG_MAX=10000` 슬롯 × `SHM_LOG_SIZE`, 쓰기 커서 `SLOGW(D_K)`/읽기 커서 `SLOGR(D_K)` (`inc/shm_memory.h:1003-1004`), 세마포어 `ShmLogSemId` 보호 (`sub/log_proc.c:443-449`)
- **통지**: `ShmLogFifoFd`에 1바이트 write (`log_proc.c:455`)
- **소비자**: `src/PW/pw_2000_mp.c` (LogManager, 운영 proc.ini에서 `pa_2001_mp`/`pb_2001_mp`로 기동, Status R) — `Write_SLog()`로 파일 기록. structured 모드(#44)도 지원(`log_proc.c:551,610`)
- **오버플로 정책**: `SLOGW % SHM_LOG_MAX` 랩 — 소비 지연 시 **가장 오래된 미소비 로그를 조용히 덮어씀** (생산자는 절대 블로킹되지 않음). 버스트 허용량 10,000라인. 드롭 카운터 없음 — 본 기능에서는 정책 변경 없이 문서화만 (SLog 코드 수정은 PW 소비자 동작 검증이 필요해 후속 분리)
- **비용 비교 (라인당)**: `Log` = stat+open+write+close (디스크 fs 경로, 저널링 스톨 노출) / `SLog` = vsnprintf+semop×2+memcpy(SHM)+write(FIFO 1B) — 디스크 I/O가 핫 프로세스에서 제거됨

## 2. 설계

### 2.1 `Log_Hot` 라우터 (`sub/log_hot.c` 신규)

**C89 제약**: 가변인자 매크로 불가 → 함수로 구현. `Log`/`SLog`에 va_list 전달 변형이 없으므로 `Log_Hot`이 vsnprintf로 선포맷 후 `"%s"`로 위임한다 (이중 포맷 비용은 문자열 1회 복사 수준).

```c
/* inc/fep_sub.h (Log/SLog 프로토타입 옆, :294 부근) */
extern int      Log_Hot_Mode (void);
extern void     Log_Hot (int, const char *, ...);
```

```c
/* sub/log_hot.c 동작 규격 */
int Log_Hot_Mode(void)
    - 최초 호출 시 getenv("FEP_HOT_LOG") 평가 후 캐시 (is_structured_log 패턴, log_proc.c:22)
    - "shm"이면 1, 그 외/미설정 0
void Log_Hot(int p_err_no, const char *p_fmt, ...)
    - vsnprintf로 버퍼(SHM_LOG_SIZE=대응 크기) 선포맷
    - Log_Hot_Mode()==1 → SLog(p_err_no, "%s", buf)
    - 아니면          → Log(p_err_no, "%s", buf)
```

- **기본 동작 무변경**: `FEP_HOT_LOG` 미설정 시 기존 `Log`와 동일 파일에 동일 내용 기록 (하위호환, NO_INISAFE/structured-logging 패턴)
- 라이브러리 자동 포함: `make/SUB/Make_Lib_P_c.sh`의 `*.c` 와일드카드 — 빌드 스크립트 변경 불필요

### 2.2 전환 대상 (메시지당 반복되는 INFO 로그 10곳)

에러/FATAL 레벨(`p_err_no%10!=0` — PROC SHM에 error_cd 기록)은 **모두 동기 `Log` 유지**. 연결/기동/종료 로그(accepted, bind, listen, 인터페이스 종료 등)도 유지.

| # | 파일:라인 | 로그 | 빈도 |
|---|----------|------|------|
| 1 | `sub/device_rw.c:36` | `TCP RD [...]` | KRX 수신 매건 (PA/PB 공용) |
| 2 | `sub/device_rw.c:61` | `TCP SD [...]` | KRX 송신 매건 (PA/PB 공용) |
| 3 | `src/PB/pb_1100_ts.c:230` | `OK22 W[] R[]` | 메인루프 매회 |
| 4 | `src/PB/pb_1100_ts.c` (Make_Data_Block) | `&R_Fmt[i].Data [...]` | 주문 매건 |
| 5 | `src/PB/pb_1200_tr.c:377` | `OK001` | KRX 메시지 매건 |
| 6 | `src/PB/pb_1200_tr.c:380` | `OK002` | KRX 메시지 매건 |
| 7 | `src/PB/pb_1200_tr.c:837` | `file write[...]` | 체결 매건 |
| 8 | `src/PB/pb_1200_tr.c:839` | `file write[...]` | 체결 매건 |
| 9 | `src/PB/pb_8100_ts.c:491` | `TCP RD [...]` | 클라이언트 수신 매건 |
| 10 | `src/PB/pb_8100_ts.c:509` | `TCP SD [...]` | 클라이언트 송신 매건 |

변환은 기계적 치환: `Log(USR_OK, ...)` → `Log_Hot(USR_OK, ...)` (인자 동일).

### 2.3 Out of Scope (후속 확장 후보)

- `pb_1200_tr.c:423` (시퀀스 체크), `:601-602` (INISAFE 복호화 로그 — `#ifndef NO_INISAFE` 경로), `pa_1290_mp.c` MiChe 조정 로그 — 1차 전환 안정화 후 동일 패턴 적용
- SLog 오버플로 드롭 카운터 — PW 소비자 검증이 필요한 별도 기능
- PA 프로세스들의 대응 지점 — PB 검증 후 수평 전개

## 3. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | `Log_Hot_Mode` env 라우팅/캐싱 | unit test (shm 모드 바이너리) |
| V-02 | 기본(미설정) 모드에서 Log 위임 | unit test (기본 모드 바이너리, 분리 — 캐시가 전역 1회이므로) |
| V-03 | 포맷 인자 전달 정확성 | unit test (stub Log/SLog 캡처) |
| V-04 | 전환 10곳 인자/레벨 불변 | 코드 검토 (치환만) |
| V-05 | off 모드 로그 파일 내용 등가 | TEST 환경 비교 (서버) |
| V-06 | F1 계측으로 주문당 레이턴시 전/후 비교 | 서버 (F1 인프라) |
| V-07 | 서버 전체 빌드 | `mk.sh sub && mk.sh src` (관례상 이연) |
