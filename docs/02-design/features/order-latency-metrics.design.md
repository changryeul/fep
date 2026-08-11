# order-latency-metrics Design Document

> **Summary**: 주문 레이턴시 계측 인프라 — 저오버헤드 트레이스 라이브러리(`sub/lat_trace.c`), 프로세스 경계 계측 지점 4곳, `order_inject` 반복 주입 확장, 히스토그램 리포트 도구(`utl/lat_report.c`).
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-04
> **Status**: Draft
> **Plan**: `docs/01-plan/features/oms-performance-roadmap.plan.md` §3 F1 (마스터 로드맵이 본 기능의 plan 역할)

---

## 1. 설계 원칙

1. **기본 off**: 계측 코드는 `-DLAT_TRACE` 컴파일 스위치로만 활성화. 미정의 시 호출 지점 매크로가 빈 문장으로 전개되어 **기존 바이너리와 동작·성능 동일**.
2. **측정이 측정을 왜곡하지 않게**: 트레이스는 기존 `Log()`(라인당 open/write/close ~4 syscall)를 쓰지 않는다. 전용 stdio 스트림(64KB 전체 버퍼링, 128건마다 flush)에 기록 — 이벤트당 비용은 gettimeofday 1회 + 메모리 버퍼 fprintf.
3. **통일 포맷**: 모든 이벤트는 `<epoch_usec>|<proc>|<point>|<key>` 한 줄. point는 `IN`/`OUT` 두 가지로 표준화(주입 도구 포함). lat_report는 (proc, key)별 IN→OUT 짝을 지어 구간을 계산한다.
4. **C89 준수**: `sub/`, `src/` 변경분은 변수 선언 스코프 최상단, `//` 주석 내 `{` 금지. `utl/` 도구는 기존 order_inject.c 수준(POSIX)의 자유도 허용.

## 2. 컴포넌트

### 2.1 `sub/lat_trace.c` + `inc/lat_trace.h` (신규)

라이브러리 빌드는 `make/SUB/Make_Lib_P_c.sh`의 `for i in *.c` 와일드카드가 자동 포함 — **빌드 스크립트 변경 불필요**. 함수 본체는 항상 컴파일되고, 호출 지점만 `LAT_TRACE` 매크로로 가드된다.

`inc/lat_trace.h`:

```c
/*************************************************************************
    File        : . lat_trace.h
    Comment     : . order latency trace (F1 order-latency-metrics)
                  . call sites are compiled out unless -DLAT_TRACE
*************************************************************************/
#ifndef _LAT_TRACE_H
#define _LAT_TRACE_H

extern  void    Lat_Init(const char *p_proc);
extern  void    Lat_Point(const char *p_point, const char *p_key, int p_klen);
extern  void    Lat_Close(void);

#ifdef LAT_TRACE
#define LAT_INIT(p)             Lat_Init(p)
#define LAT_POINT(pt, k, kl)    Lat_Point(pt, k, kl)
#define LAT_CLOSE()             Lat_Close()
#else
#define LAT_INIT(p)
#define LAT_POINT(pt, k, kl)
#define LAT_CLOSE()
#endif

#endif  /* _LAT_TRACE_H */
```

`sub/lat_trace.c` 동작 규격:
- `Lat_Init(p_proc)`: `$FEP_LAT_DIR`(미설정 시 `.`) 밑에 `<proc기저명>.lat` append 모드 오픈, `setvbuf` 64KB `_IOFBF`. proc명은 경로 포함 argv[0]가 들어와도 basename만 사용. 실패 시 조용히 비활성(핫패스에 영향 금지).
- `Lat_Point(point, key, klen)`: `fprintf(fp, "%ld%06ld|%s|%s|%.*s\n", sec, usec, proc, point, klen, key)`. 128건마다 fflush. fp 없으면 즉시 리턴. key 내 `|`/개행은 신뢰 입력(주문번호/DataSeq)이므로 이스케이프 없음.
- `Lat_Close()`: flush + fclose.
- 스레드 안전 불필요 (FEP 프로세스는 단일 스레드 이벤트 루프).

### 2.2 계측 지점 (4개 프로세스, `-DLAT_TRACE` 가드)

각 소스의 `main()`에서 `Init_Proc(argc, argv)` 직후 `LAT_INIT(argv[0]);`, `Exit_Process()` 직전이 아닌 각 파일의 종료 루틴에는 넣지 않음(프로세스 종료 시 OS가 flush 못한 최대 128건 유실 허용 — 측정 목적상 무시 가능. 단 `Lat_Point` 128건 주기 flush로 상시 관측 가능).

| # | 소스 (바이너리) | IN | OUT | key |
|---|----------------|----|----|-----|
| 1 | `src/PB/pb_1100_ts.c` (pb_1101_ts) | `Make_Data_Block()`: `F_R(PS_R_1)` 성공 후 레코드 루프 진입 시 레코드별 | `Data_Event_Rtn()`: `Device_Write()` 직후 | `&R_Fmt[i].Data[39]`, 10B (주문전문 OrderNo: DataSeq 11+TrCode 11+Megrp 2+MktId 3+Board 2+Member 5+Branch 5 = offset 39) |
| 2 | `src/PB/pb_1200_tr.c` (pb_1201_tr) | `Socket_Event_Rtn()`: `Device_Read()` 성공(`rval >= 0`) 직후 | 동일 함수: `F_W(TS_W1_1)` 성공 직후 | `&DataBuff[KRX_HEAD_LEN]`, 11B (Body DataSeq) |
| 3 | `src/PB/pb_8100_ts.c` (pb_8111_ts) | `Make_Send_Msg()` TR_DATA 분기: `F_R` 후 `r_cnt > 0` 확정 지점 | `Data_Event_Rtn()`: `Device_Write()` 직후 (SendFlag=1일 때) | `R_Fmt[0].Data`, 11B (전달 데이터 선두 DataSeq) |
| 4 | `src/PA/pa_1290_mp.c` (pa_1291_mp) | `Analyze_Data()`의 `Make_MiChe()` 호출 직전 | 호출 직후 | 단조증가 시퀀스 10자리 (F6 미체결 스캔 비용 측정용) |

**#4 구현 결정**: `Make_MiChe()`는 리턴 지점이 13곳이고 A1291/A2291 전문별로 OrderNo 오프셋이 달라 함수 내부 계측은 침습적 — 호출부(단일 지점) 래핑으로 변경. 단일 프로세스 순차 처리이므로 구간 짝짓기에는 시퀀스 키로 충분.

**호출 형태 (공통 패턴, 각 지점에 동일 적용)**:

```c
#ifdef LAT_TRACE
    LAT_POINT("IN", &R_Fmt[i].Data[39], 10);
#endif
```

(매크로 자체가 빈 전개되므로 `#ifdef` 이중 가드는 불필요하나, 프로젝트의 `#ifndef NO_INISAFE` 관례에 맞춰 가시성을 위해 `#ifdef LAT_TRACE` 블록으로 감싼다. 헤더 include는 가드 없이 `#include "lat_trace.h"` 1줄 — 매크로만 쓰이므로 off 빌드에도 무해.)

**빌드 활성화**: `make/PB/Make_PB_ts.sh`, `make/PB/Make_PB_tr.sh`, `make/PA/Make_PA_mp.sh`의 `DEF_TMP` 조립 직후에 아래 1줄 추가 — 환경 변수 opt-in (`NO_INISAFE` 패턴):

```sh
if [ "${_LAT_TRACE}" = "1" ]; then DEF_TMP="$DEF_TMP -DLAT_TRACE"; fi
```

### 2.3 `utl/order_inject.c` 확장

기존 위치 인자 뒤에 2개 추가 — **하위호환 유지** (기존 6인자 호출 동작 불변):

```
order_inject <IP> <PORT> <종목코드> [매수매도] [수량] [가격] [건수] [간격ms]
```

- `건수`(argv[7], 기본 1): LINK 1회 후 DATA를 N회 반복. 주문별로 `OrderNo = 시작값(1)+k`를 10자리 zero-pad로 부여(기존 고정 "0000000001" 대체), `h->SeqNo`도 2+k로 증가.
- `간격ms`(argv[8], 기본 0): 주문 간 `usleep(간격ms * 1000)`.
- **RTT 기록**: 주문별 DATA 송신 직전 `IN`, DAOK 수신 직후 `OUT` 이벤트를 `order_inject.lat` 파일(§2.1과 동일 포맷, proc=`order_inject`, key=OrderNo 10자리)에 기록. gettimeofday 직접 사용(라이브러리 비의존 — utl 도구는 libfepP 없이 단독 빌드 유지).
- 완료 시 요약 출력: 전송 건수, 실패 건수, RTT min/avg/max (µs).
- DAOK 아닌 응답은 실패 카운트 후 계속 진행 (기존은 즉시 종료 — 반복 모드에서만 계속, 1건 모드는 기존과 동일하게 에러 종료).

### 2.4 `utl/lat_report.c` (신규)

```
Usage: lat_report <file.lat> [file2.lat ...]
```

- 입력: §2.1 포맷 라인들. 파싱 실패 라인은 건너뛰고 stderr에 카운트만 보고.
- 짝짓기: (proc, key)별로 시각 오름차순 정렬 후 `IN` 다음에 오는 첫 `OUT`을 짝으로 구간(µs) 산출. 짝 없는 이벤트는 unmatched 카운트.
- 출력: proc별 표 — `count / min / p50 / p90 / p99 / max` (µs). 백분위는 정렬 배열의 `ceil(p*n)-1` 인덱스(nearest-rank).
- 구현: 단독 빌드(외부 의존 없음). 테스트를 위해 코어 로직을 함수로 분리하고 `#ifndef LAT_REPORT_NO_MAIN` 가드로 main 제외 컴파일 지원:
  - `int lat_parse_line(const char *line, LAT_EV *ev)` — 1줄 파싱 (0=OK, -1=skip)
  - `int lat_pair(LAT_EV *evs, int n, long *out_us)` — 정렬된 이벤트 배열에서 구간 추출, 구간 수 리턴
  - `long lat_pctl(long *sorted, int n, double p)` — nearest-rank 백분위
- `utl/Makefile`에 `lat_report` 타깃 추가 (order_inject와 동일 패턴), `all` 타깃에 포함.

### 2.5 단위 테스트 (신규 2건, `test/unit/Makefile` TESTS에 추가)

- `test/unit/test_lat_trace.c`: `FEP_LAT_DIR`을 임시 디렉토리로 설정 → `Lat_Init`/`Lat_Point`×N/`Lat_Close` → 파일 재독 후 라인 수·포맷(`usec|proc|point|key`)·basename 처리·미초기화 시 no-op 검증. 링크: `obj/lat_trace.o` (Tier 1A 패턴).
- `test/unit/test_lat_report.c`: `-DLAT_REPORT_NO_MAIN -I../../utl`로 lat_report.c 포함 컴파일 → 파싱 정상/불량 라인, IN/OUT 짝짓기(정상·unmatched·역순), nearest-rank 백분위(n=1, n=100) 검증.

## 3. 측정 시나리오 (TEST 환경)

```sh
export _LAT_TRACE=1 && mk.sh sub && mk.sh pb && mk.sh pa mp   # 계측 빌드
export FEP_LAT_DIR=$HOME/fep/lat && mkdir -p $FEP_LAT_DIR
# (integ Mock KRX 기동, 대상 프로세스 기동)
order_inject 127.0.0.1 18200 KR7005930003 2 100 98500 1000 10   # 1,000건, 10ms 간격
lat_report $FEP_LAT_DIR/*.lat order_inject.lat
```

## 4. 검증 항목 (V-items)

| ID | 항목 | 방법 |
|----|------|------|
| V-01 | lat_trace 포맷/basename/no-op | unit test_lat_trace (macOS+서버) |
| V-02 | lat_report 파싱/짝짓기/백분위 | unit test_lat_report (macOS+서버) |
| V-03 | order_inject 기존 6인자 호출 동작 불변 | 코드 검토 + TEST 환경 1건 주입 |
| V-04 | order_inject N건 주입 시 OrderNo/SeqNo 고유 증가 | order_inject.lat 검사 |
| V-05 | LAT_TRACE off 빌드에서 오브젝트 코드에 계측 흔적 없음 | off 빌드 후 `nm`/`strings`로 Lat_Point 심볼 참조 부재 확인 |
| V-06 | on 빌드 4개 프로세스 lat 파일 생성·짝 매칭 | integ Mock KRX 시나리오 (서버) |
| V-07 | 서버 전체 빌드 통과 (`mk.sh sub && mk.sh src`) | 서버 환경 (관례상 이연) |

## 5. Out of Scope

- PA 파생(2xxx)/시세 경로 계측 — F1은 채권 주문 파이프라인 한정
- 트레이스 파일 로테이션/보존 정책 — 측정 세션 단위 수동 관리
- 엔드투엔드 단일 주문 추적(프로세스 간 키가 다른 구간의 자동 조인) — 구간별 통계로 충분, 필요 시 후속
