# oms-performance-roadmap Planning Document

> **Summary**: 고성능 OMS 달성을 위한 성능/내구성 구조 개편 마스터 계획. 3대 구조적 병목(디스크 파일 큐 IPC, 핫패스 동기 로깅, O(10,000) 미체결 선형조회) 제거와 주문상태 내구성 신설을 8개 기능 PDCA 사이클로 분해.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude
> **Date**: 2026-08-04
> **Status**: Draft
> **선행 분석**: 2026-08-04 고성능 OMS 관점 코드베이스 분석 (핫패스 syscall 인벤토리, SHM 주문상태 구조 조사)

---

## 1. Overview

### 1.1 Purpose

주문 1건의 FEP 내부 처리 레이턴시를 현행 추정 ms 단위에서 100µs 미만(p99)으로 낮추고, 동시에 주문상태(미체결 테이블)의 크래시 복구 경로를 신설한다. 2026-02 성능 최적화 시리즈(§4.9, 10/10 완료)가 코드 레벨 낭비를 제거했다면, 본 계획은 **구조 레벨** 병목을 제거한다.

### 1.2 Background — 3대 구조적 병목 (2026-08-04 분석 결과)

**병목 1: 디스크 파일 큐 IPC** — PB 주문 파이프라인의 프로세스 간 전달이 "디스크 파일 append + fflush + FIFO 1바이트 신호 + 상대편 디스크 파일 read" 구조.

- 주문 1건이 프로세스 1개를 통과할 때 **약 30회 이상의 syscall + 동기 flush**:
  - `F_R` 주문 읽기: open→fcntl 잠금→lseek→read→unlock→close (`sub/file_rw.c:29-118`) ≈ 6 syscall
  - `Seq_Save`: fopen→fcntl→fseek→fwrite→**fflush**→fclose — 매 주문 (`sub/seq_save.c:21-75`) ≈ 6 syscall
  - `Log` ×3회: 매 라인 stat→open→write→close (`sub/log_proc.c:264-342`) ≈ 12 syscall
- 엔드투엔드 주문+체결이 5~6개 프로세스 경유, 매 홉마다 반복:
  `pb_1301_tr → pb_1101_ts → KRX → pb_1201_tr → pb_1402_ts → pb_1211_tr → pb_8111_ts`
- `MAX_CNT=1` (`src/PB/pb_1100_ts.c:37`) — 배칭 없이 1건씩 읽음
- 반면 PA 클라이언트 수신 경로는 이미 DSHM 패턴(SHM 데이터 + FIFO 웨이크업, `sub/shm_rw.c`) 사용 — **전환 패턴이 코드베이스에 이미 존재**

**병목 2: 핫패스 동기 로깅** — `Log()`가 매 라인 파일 open/close (fd 캐싱 없음). SHM 링 기반 `SLog`(`sub/log_proc.c:377`)가 구현되어 있으나 핫패스 미사용.

**병목 3: 미체결 주문 선형 조회** — 체결/정정/취소마다 원주문번호를 최대 10,000슬롯 memcmp 선형 스캔 (`src/PA/pa_1290_mp.c:383-385`). 삭제는 구멍만 남김(`Jan_Cnt<=0`), 삽입도 선형 빈슬롯 탐색(`:288-291`). 인덱스/해시/free-list 없음.

**내구성 격차 (병목 1과 한 몸)**: 미체결 테이블(MICHE)과 주문번호 카운터의 유일한 사본이 SHM. 세그먼트 유실/재생성 시 전체 소실되며 재구성 경로 없음 (`sub/shmsub.c:311-323`은 크기 불일치 시 자동 삭제·재생성). 디스크 파일 큐는 사실상 유일한 디스크 흔적이므로, **이를 제거하려면 저널링이 선행되어야 함**.

### 1.3 Related Documents

- `docs/FEP_Architecture_Analysis.md` §4 (성능), §3.5 (자동 재기동 미완)
- `docs/archive/2026-02/{tcp-nodelay,stat-save-optimize,file-rw-optimize,select-send-optimize,final-perf-optimize}/` — 선행 최적화 시리즈, 특히 stat-save-optimize의 시간 스로틀 패턴 재사용
- `docs/01-plan/features/INI-server-config.plan.md` — 운영 토폴로지 (AP서버 PA 75프로세스 / 시세서버 PB 4프로세스, 전략 프로세스 40개 전부 비활성)
- `st01/test/integ/` — Mock KRX 서버 (검증 인프라로 재사용)
- `st01/utl/order_inject.c` — 주문 주입 도구 (계측 도구의 기반)

---

## 2. 전체 구조 — 8개 기능, 4개 Phase

```
Phase A (계측)          F1 order-latency-metrics ──────────┐ 모든 기능의 효과 측정 기준선
                                                           │
Phase B (즉효, 독립)    F2 hotpath-log-slog     ←──────────┤ 각각 독립 PDCA,
                        F3 seq-save-batch       ←──────────┤ 병렬 진행 가능
                        F4 file-rw-fd-pool      ←──────────┘
                                                           
Phase C (구조 개편)     F5 order-pipeline-dshm (전제: 원문 파일 write-behind 보존)
                        F6 miche-index    (독립 — 가치: 체결 폭주 시 처리량/데이터 신선도)
                        F7 miche-rebuild  (운영 도구 — 저널 신설안은 2026-08-06 폐기)
                        F8 hop-reduction  (F5 이후 권장)
                                                           
Phase D (안정성/확장)   자동 재기동, 용량 상수 설정화 — 본 계획 범위 밖 (별도 PDCA)
```

**의존 관계** (2026-08-06 갱신):
- F1은 모든 기능의 선행 (효과를 수치로 검증할 수단) — 완료
- ~~F7(저널)은 F5의 필수 선행~~ → F5의 전제는 "주문/체결 원문 파일의 write-behind 보존"으로 완화. F7은 독립 운영 도구
- F2/F3/F4/F6은 상호 독립 — 어느 순서든 가능 (F2/F3/F4 완료)
- F8은 F5 완료 후 라우팅이 단순해진 상태에서 진행 권장

**예상 효과 요약** (F1 계측으로 확정할 가설):

| 기능 | 제거 대상 | 주문당 절감 (가설) |
|------|----------|------------------|
| F2 | 로그 syscall ~12회 | 수백 µs |
| F3 | Seq_Save fflush ~6회 | 수백 µs ~ ms (디스크 의존) |
| F4 | F_R/F_W open/close ~4회/홉 | 수십~수백 µs |
| F5 | 홉당 디스크 왕복 전체 | **ms 단위 → 수십 µs** (최대 효과) |
| F6 | 체결당 O(10,000) 스캔 | 체결 폭주 시 CPU 병목 해소 |

---

## 3. Feature 상세

### F1. order-latency-metrics — 주문 레이턴시 계측 인프라

**목표**: 주문 1건의 프로세스 경계별 소요 시간을 측정하는 저침습 계측 체계와 왕복 레이턴시 리포트 도구.

**설계 방안**:
1. **경계 타임스탬프 로그**: 각 핫패스 프로세스(주문 수신/송신/응답/체결)의 처리 시작·종료 지점에 `Get_Msec()`(기존 함수, `sub/fep_common.c`) 기반 구조화 로그 1줄 추가. 형식: `LAT|<proc명>|<주문번호10자리>|<in_usec>|<out_usec>`. `FEP_LOG_FORMAT=structured` 모드(#44 기존 인프라) 전제.
   - 계측 자체가 병목이 되지 않도록 `-DLAT_TRACE` 컴파일 스위치로 온오프 (기본 off, TEST 환경에서만 on)
2. **부하 주입 확장**: `utl/order_inject.c`에 반복 옵션(`-n <count>`, `-i <interval_usec>`)과 주문번호별 송신 시각 기록 추가. 응답 수신 시각과 대조해 클라이언트 관점 RTT 산출.
3. **리포트 도구**: `utl/lat_report.c` 신설 — LAT 로그와 order_inject 기록을 병합해 구간별/전체 p50·p90·p99 히스토그램 출력.

**대상 파일**:
- Modify: `st01/utl/order_inject.c`, `src/PB/pb_1100_ts.c`, `src/PB/pb_1200_tr.c`, `src/PB/pb_8100_ts.c`, `src/PA/pa_1290_mp.c`
- Create: `st01/utl/lat_report.c`

**검증**: TEST 환경 + `test/integ` Mock KRX 서버로 주문 1,000건 주입 → 구간별 히스토그램 산출 성공. `-DLAT_TRACE` off 빌드에서 성능·동작 무영향 확인 (unit 테스트 통과).

**리스크**: 낮음 (기본 off 스위치, 기존 구조화 로깅 인프라 재사용).

---

### F2. hotpath-log-slog — 핫패스 로깅 SHM 경유 전환

**목표**: 주문 경로의 INFO 레벨 `Log()` 호출을 SHM 링 기반 `SLog` 경로로 전환해 주문당 ~12 syscall 제거.

**설계 방안**:
1. `Log_Hot()` 래퍼 매크로 신설 (`inc/fep_sub.h`): 환경 변수 `FEP_HOT_LOG=shm`이면 `SLog`(SHM 링 + FIFO 1바이트, `sub/log_proc.c:377`) 경유, 미설정 시 기존 `Log` 그대로 — **기본 동작 무변경, 하위호환**.
2. 전환 대상은 핫패스 INFO 로그만: `pb_1100_ts.c:230` ("OK22"), `:968` ("&R_Fmt"), `sub/device_rw.c:61` ("TCP SD") 및 pb_1200_tr/pb_8100_ts의 대응 지점. **에러/FATAL 레벨은 동기 유지** (장애 분석 시 유실 방지).
3. SLog 소비자(LogManager `pb_2001_mp`, 운영 중 Status R)의 링 오버플로 정책 확인: 오버플로 시 드롭 카운터 기록 (조사 항목 I-1).

**대상 파일**:
- Modify: `st01/inc/fep_sub.h`, `sub/device_rw.c`, `src/PB/pb_1100_ts.c`, `src/PB/pb_1200_tr.c`, `src/PB/pb_8100_ts.c`

**검증**: F1 계측으로 전환 전/후 주문당 레이턴시 비교. 로그 내용 등가성 확인 (SLog 출력과 기존 Log 출력의 필드 일치). `FEP_HOT_LOG` 미설정 시 기존과 바이트 단위 동일 로그 확인.

**리스크**: 중간 — 프로세스 크래시 직전 로그가 링에 남아 유실될 수 있음 → 에러 레벨 동기 유지로 완화.

---

### F3. seq-save-batch — 시퀀스 저장 flush 주기화

**목표**: 매 주문 실행되는 `Seq_Save`의 동기 디스크 flush(~6 syscall)를 주기화.

**전제 조사 (I-2, 설계 확정 전 필수)**: `Seq_Save`가 지키는 복구 시맨틱 명확화. 시퀀스 파일은 재기동 시 파일 큐 읽기 커서 복원용. fflush를 제거해도 fwrite는 커널 버퍼에 도달하므로 **프로세스 크래시에는 안전**하고 OS 크래시에만 취약 — OS 크래시 시엔 SHM도 함께 소실되므로 KRX 재접속 시 MsgSeqNum 협상으로 복구되는지 확인 후 flush 정책 결정.

**설계 방안** (stat-save-optimize #36의 시간 스로틀 패턴 동일 적용):
1. `sub/seq_save.c`에 fd 상시 오픈 캐시 도입 (매건 fopen/fclose 제거)
2. `fflush`를 매건 → `SEQ_FLUSH_INTERVAL_SEC`(기본 1초) 주기 또는 N건(기본 64건) 단위로 변경. 인터벌 상수는 `inc/fep_sub.h`에 정의.
3. 프로세스 정상 종료 경로(`Device_Close` 등)에서 최종 flush 보장.

**대상 파일**:
- Modify: `sub/seq_save.c`, `inc/fep_sub.h`

**검증**: 프로세스 kill -9 후 재기동 시 커서 복원이 최대 1초/64건 이내 오차로 동작하고 중복 처리분이 시퀀스 검사로 걸러지는지 integ 테스트로 확인. F1 계측으로 주문당 절감 측정.

**리스크**: 중간 — 복구 시맨틱 변화. I-2 조사 결과에 따라 flush 주기 보수적 설정.

---

### F4. file-rw-fd-pool — 파일 I/O fd 풀링 + 읽기 배칭

**목표**: `F_R`/`F_W` 계열의 매건 open/close 제거, 주문 읽기 배칭.

**배경**: file-rw-optimize(#38)는 잠금 범위만 레코드 단위로 좁혔고 open/close 반복은 그대로 남음 (`sub/file_rw.c:66,115`).

**설계 방안** (2026-08-05 조사 후 확정 — 상세: `02-design/features/file-rw-fd-pool.design.md`):
1. 범용 fd 캐시 `sub/fd_cache.c` 신설 (경로 키, 실패/오버플로 시 호출측이 기존 open/close로 폴백 — 동작 보존).
2. 전환 범위: `F_R`/`F_W` 파일 fd + `F_W_Proc`/`F_W2_Proc`/`F_WB`의 FIFO 통지 fd (핫패스 한정). 비핫패스 변형(F_R2/R3/W2/W3/WB/SF_W의 파일 fd)은 후속.
3. ~~일자 롤오버 처리~~ → 불필요 확인: 모든 경로가 `00000000` 고정 디렉토리.
4. ~~MAX_CNT 1→8 배칭~~ → **F4에서 제외**: Make_Data_Block이 전 레코드에 동일 DataSeq(INT_SEQ+1) 부여, Add_Count(PS_R_1, **1**) 하드코딩, 다건 KRX 패킷/암호화 단위 재설계 필요 — 주문 정확성에 닿으므로 F5에서 큐 대체와 함께 설계.

**대상 파일**:
- Create: `sub/fd_cache.c`, `test/unit/test_fd_cache.c`
- Modify: `sub/file_rw.c`, `inc/fep_sub.h`

**검증**: unit 테스트(캐시 히트/오버플로 폴백/Close_All), 서버 빌드 + 전체 테스트, lat_bench 재측정.

**리스크**: 낮음~중간 — 폴백 패턴으로 캐시 실패 시 기존 동작 보존. **참고**: F5 완료 시 주문 경로에서는 본 기능의 효과가 흡수되나, 파일 큐를 유지하는 비핫패스 경로에는 계속 유효.

---

### F5. order-pipeline-dshm — 주문 파이프라인 디스크 큐 → DSHM 전환 (본체)

**목표**: 주문/체결 홉의 프로세스 간 전달을 디스크 파일 큐에서 DSHM(SHM 링 + FIFO 웨이크업)으로 전환. **최대 레이턴시 개선 항목**.

**선행 조건**: F7(order-journal) 완료 — 디스크 큐가 갖던 내구성 역할 대체.

**설계 방안**:
1. **패턴 재사용**: PA 클라이언트 경로가 이미 사용 중인 `DSHM_W`/`DSHM_R`(`sub/shm_rw.c`, 세마포어 보호 SHM 링 + FIFO 1바이트 웨이크업) 적용. 신규 발명 없음.
2. **단계적 전환** (홉 단위 독립 릴리스):
   - 1단계: 주문 인바운드 `pb_1301_tr → pb_1101_ts` (가장 레이턴시 민감)
   - 2단계: 체결 아웃바운드 `pb_1201_tr → pb_1402_ts → pb_1211_tr → pb_8111_ts`
3. `cfg/dshm.ini`에 신규 세그먼트 정의 추가 (기존 42개 세그먼트 형식 준수: Key/Fifo/Size/Max). 주문 레코드 Size=400, Max=10000 — 기존 `Dshm_1`(pa_1101_ts) 정의와 동일 스펙.
4. **감사 흔적 보존**: 전환 후에도 주문 원문의 디스크 기록이 필요하면 write-behind 아카이버(비핫패스 프로세스가 DSHM 링을 후행 소비해 파일 append)로 대체. F7 저널이 이 역할을 겸할 수 있는지 설계 단계에서 결정 (조사 항목 I-3).
5. 롤백 경로: proc.ini/빌드 스위치로 홉별 파일 큐 복귀 가능하게 이중 경로 유지 후, 안정화 확인 시 제거.

**대상 파일** (바이너리→소스 매핑은 `make/PB/Make_PB_tr.sh:27-38`, `Make_PB_ts.sh:56-62` 기준):
- Modify: `src/PB/pb_7200_tr.c`(pb_1301_tr, Recv From OMS), `src/PB/pb_1100_ts.c`(pb_1101_ts), `src/PB/pb_1200_tr.c`(pb_1201_tr), `src/PB/pb_7100_ts.c`(pb_1402_ts 계열), `src/PB/pb_8100_ts.c`(pb_8111_ts), `cfg/dshm.ini`, `cfg/proc.ini`
- 참조: `sub/shm_rw.c` (변경 없음 — 기존 API 사용)

**검증**: integ Phase 3(`run_pb_integ.sh`, 실제 바이너리 E2E) 확장 — 주문→응답→체결 왕복 무결성, 프로세스 재기동 시 미소비 레코드 처리. F1 계측으로 홉당 레이턴시 전/후 비교 (**성공 기준: 홉당 수십 µs**).

**리스크**: 높음 — 주문 경로 핵심 변경. 홉 단위 단계 릴리스 + 이중 경로 롤백 + Mock KRX E2E로 완화. 장 마감 후 배포 필수.

---

### F6. miche-index — 미체결 테이블 해시 인덱스 + free-list

**목표**: 미체결 조회 O(10,000) → O(1), 빈슬롯 탐색 O(10,000) → O(1).

**설계 방안**:
1. **해시 인덱스**: (시장×계좌)별 open-addressing 해시 테이블을 SHM에 추가 — `int Hash_Slot[RISK_MK_CNT][ACC_NO_CNT][MICHE_HASH_SIZE]` (MICHE_HASH_SIZE=16384, 2^14 ≥ 10000×1.6 로드팩터). 키: OrderNo 10바이트의 정수 변환. 값: F_MiChe 슬롯 인덱스+1 (0=빈칸). 삭제는 tombstone(-1).
   - 추가 메모리: 2×20×16384×4B ≈ 2.6MB (기존 세그먼트 ~96MB 대비 미미)
2. **free-list**: `int Free_Head[RISK_MK_CNT][ACC_NO_CNT]` + `int Next_Free[..][..][MAX_MICHE]` — 삽입 시 O(1) pop, 삭제 시 O(1) push.
3. **동시성 계약 유지**: 계좌당 단일 소유 프로세스 전제(현행과 동일)이므로 잠금 불필요. 이 전제를 `inc/shm_memory.h` 주석에 **명문화**.
4. **재구성 함수**: `Miche_Index_Rebuild()` — 기동 시 F_MiChe 전체를 1회 스캔해 해시/free-list 재구성. 인덱스는 파생 데이터이므로 저널(F7) 대상에서 제외 가능.

**대상 파일**:
- Modify: `inc/shm_memory.h`(MK_PREMATCH 확장), `src/PA/pa_1290_mp.c`(:288-291 삽입, :383-385 조회, :405-409 삭제), `src/PZ/pz_memory_shm.c`(세그먼트 크기)
- Create: `sub/miche_index.c`(해시/free-list 연산 + rebuild), `inc/` 프로토타입은 `fep_sub.h`에 추가
- Test: `st01/test/unit/test_miche_index.c`

**검증**: unit 테스트 — 삽입/조회/삭제/충돌/tombstone/rebuild, 10,000건 채운 뒤 전 슬롯 조회 일치. 기존 선형 스캔 결과와의 등가성 비교 테스트.

**리스크**: **SHM 레이아웃 변경 = 세그먼트 크기 변경 → `shmsub.c:311-323`이 기동 시 세그먼트를 자동 삭제·재생성하여 기존 미체결 상태가 소실됨.** 반드시 장 마감 후(미체결 0 상태) 배포하는 운영 절차를 릴리스 노트에 명시. F7 완료 후라면 저널 replay로 복원 가능.

---

### F7. miche-rebuild — 미체결 재구성 도구 (2026-08-06 재정의)

**재정의 배경**: MICHE 갱신은 클라이언트 응답 경로와 병렬(조회/한도/손익 소비용 — "조회 수준")이며, **당일 체결 파일 자체가 이미 append-only 저널**이다. 별도 저널 신설은 같은 데이터의 이중 기록 — 원안(order-journal) 폐기.

**목표**: SHM 유실 시 당일 체결 파일 replay로 MICHE를 재구성하는 **운영 도구** 신설. 핫패스 추가 비용 0.

**설계 방안**:
1. 재구성 유틸(px 계열 또는 utl): 대상 체결 파일을 오프셋 0부터 순회하며 MICHE 갱신 로직 재적용. `Make_MiChe`가 PROFIT/RISK 등 다른 SHM도 갱신하므로 **재적용 범위(미체결만 vs 손익 포함)와 멱등성**을 설계 단계에서 확정 (조사 항목 I-7).
2. 주문번호 카운터(`ORDER_NO.USED_JMNO`) 복구는 `_stat`/`_seq` 기존 메커니즘 + 재구성 시 관측된 최대 주문번호로 보정.
3. F6 적용 후에는 재구성 마지막에 인덱스 rebuild 호출.

**F5 전제 완화**: F5는 "저널 선행"이 아니라 **주문/체결 원문 파일 기록을 write-behind 아카이버로 보존**(감사추적 + 본 도구의 원천 데이터 유지)을 전제로 한다.

**대상 파일** (설계 시 확정): Create: `utl/miche_rebuild.c` 또는 `src/PX/px_michereco.c`

**검증**: integ — 주문 N건 처리 상태에서 SHM 삭제(ipcrm) 후 도구 실행 → 미체결 테이블 등가 복원.

**리스크**: 낮음 (운영 도구, 핫패스 무접촉). Make_MiChe 로직 재사용 방식에 따라 이중 계상 위험 → I-7에서 확정.

---

### F8. hop-reduction — 체결 경로 포워더 홉 축소 (선택)

**목표**: 체결 아웃바운드의 포워더 홉(`pb_1402_ts`, `pb_1211_tr`)을 축소해 5~6홉 → 3~4홉.

**전제 조사 (I-5) — 완료 (2026-08-06), 토폴로지 정정**: 로드맵이 가정한 선형 4홉 체인(`pb_1201_tr→pb_1402_ts→pb_1211_tr→pb_8111_ts`)은 **틀림**. 실제 proc.ini(cfg/back) 확인 결과 PB 서버 체결 경로는 **2개 독립 경로**:
- **응답체결(주경로)**: KRX → `pb_1201_tr`(TR2 수신 57221, B1201 변환=15B 패딩) → 큐 → `pb_1402_ts`(TS2 송신 41003, 순수 중계+4B 길이헤더) → **AP서버 pa_1402_tr**
- **DropCopy(DR재해복구)**: KRX → `pb_1211_tr`(TR2 수신 61726, LogonID D001) → 큐 → `pb_8111_ts`(TS2 송신 32009) — **독립 DR 경로, 제거 불가**

→ **PB 서버 내 안전한 drop-in 홉 축소 대상 없음**. 주경로의 유일한 여지는 `pb_1201_tr → pb_1402_ts` 큐 홉인데, KRX 수신(TR2)과 AP 송신(TS2)의 **역할이 다른 두 프로세스의 구조적 병합**이라 단순 OFN 재라우팅 불가. AP 연결 장애 시 KRX 수신 블로킹 등 실패모드 영향.

**운영자 결정 (2026-08-06): 현 구조 유지, F8 종료.** 근거: (1) V-06 실측상 홈 이득이 ~20-40µs로 작음, (2) 수신/송신 분리(장애 격리, 독립 재접속/시퀀스)를 포기하는 리스크가 이득 대비 큼. 체결 전달은 트레이딩 핵심 경로라 안정성 우선. AP 서버 측 경로 축소도 현 시점 미착수(수요 시 별도 범위).

**대상 파일**: `cfg/proc.ini` (+ 조사 결과에 따라 `src/PB/pb_7100_ts.c`(pb_1402_ts) 로직 이관)

**검증**: integ Phase 3 E2E 체결 왕복 무결성. F1 계측으로 홉 절감 확인.

**리스크**: 조사 결과에 따라 낮음~중간. F5 완료 후 진행 (DSHM 전환으로 라우팅이 단순해진 뒤가 안전).

---

## 4. 조사 항목 (설계 단계 확정 필요)

| ID | 항목 | 관련 기능 |
|----|------|----------|
| I-1 | SLog 링 오버플로 정책과 LogManager(pb_2001_mp) 소비 속도 한계 | F2 |
| I-2 | Seq_Save 복구 시맨틱 — KRX 재접속 시 MsgSeqNum 협상으로 커서 오차 흡수 가능 여부 | F3 |
| I-3 | 주문 원문 디스크 감사 흔적의 법규/운영 요건 — F7 저널로 대체 가능한지 | F5 |
| I-4 | ~~스냅샷 담당~~ (F7 재정의로 폐기) | — |
| I-7 | Make_MiChe 재적용 범위(미체결만 vs 손익 포함)와 replay 멱등성 | F7 |
| I-8 | ✅ 해결: pz DSHM 세그먼트는 key_info 섹션 'E'/'O'에서만 생성 — PB 단일큐는 `,O` 필요 (dshm.ini). 코드 아닌 설정 문제 | F5 |
| I-9 | ✅ 조사완료: 버스트 중복전송은 **하니스 아티팩트**(mock echo). 현실적 mock으로 100→100 확인 | — |
| RD_CNT 경화 | ✅ 완료 (2026-08-06): KRX 시퀀스 되감기 언더플로 → 재전송 폭주 취약성을 fail-safe 가드로 경화(운영자 결정). 두 되감기 지점 모두 `mun>RD_CNT` 시 ERROR+Exit_Process. 클린 E2E 100→100 검증. 상세: `02-design/features/rdcnt-underflow-guard.design.md` | 완료 |

**F5 실측 결론 (2026-08-06, 08-06 정정)**: DSHM 전환은 주문 인바운드 레이턴시를 유의미하게 개선하지 못함 (저부하 file≈DSHM ~95µs, 버스트 둘 다 ~19ms 포화). **측정은 모두 `NO_INISAFE`=암호화 제외 값**(dev 필수 조건). 병목은 큐 방식도 암호화도 아니라 **TCP 송신 + 프로세스 스케줄링/홉**(pb_1101 내부 ~40µs + 상류 ~50µs). 운영은 여기에 INISAFE 주문당 암호화가 고정 추가되나 KRX 필수라 최적화 대상 아님.

→ **성능 로드맵 재조정**: 큐 I/O(F5)·로깅(F2)·시퀀스(F3)는 dev 측정상 주문 레이턴시 지배 요인이 아님이 실증됨. 남은 레버는 (a) 홉 수 축소(F8), (b) 프로세스 스케줄링/웨이크업 지연, (c) 처리량(버스트 드레인). 상세: `02-design/features/order-pipeline-dshm.design.md` §3.1, 메모리 `inisafe-dev-excluded`.
| I-5 | pb_1402_ts/pb_1211_tr 포워더의 변환 로직 유무 | F8 |
| I-6 | 현행 운영 디스크(SSD/HDD) 확인 — fflush 비용 가설 검증 | F1, F3 |

## 5. 성공 지표

### 5.0 원시 연산 실측 (2026-08-05, 개발 서버 EC2/EBS, `utl/lat_bench` n=10,000)

무부하 + 웜 페이지캐시 조건의 **최선치(하한)**. 장중 부하에서 p99 꼬리는 디스크 경합으로 이보다 훨씬 커질 수 있음.

| 패턴 | 회당 비용 | 의미 |
|------|----------|------|
| log_legacy (stat+open+write+close) | 4.01 µs | 기존 Log 1줄 — 주문당 3줄 ≈ 12 µs |
| log_fdcache (write만) | 0.70 µs | fd 캐시 시 |
| slog_producer (semop×2+memcpy+FIFO 1B) | 1.78 µs | F2 SLog 생산자 — 라인당 2.3배 + 디스크 꼬리 제거 |
| seq_legacy (fopen+fcntl+fwrite+fflush+fclose) | 5.70 µs | 기존 Seq_Save |
| seq_fdcache (F3 기본 모드) | 2.55 µs | fd 캐시만으로 55% 절감, interval 모드 시 ≈0 |
| fw_legacy (파일 append+잠금+FIFO 통지) | 8.77~12.73 µs | 홉당 쓰기 측 (부하 편차) |
| fr_legacy (open+잠금+read+close) | 4.74~6.85 µs | 홉당 읽기 측 |
| fw_fdcache (F4: 캐시 FILE*+캐시 FIFO fd) | 4.01 µs | 쓰기 측 **~68% 절감** |
| fr_fdcache (F4: 캐시 fd) | 3.45 µs | 읽기 측 **~50% 절감** — 홉당 왕복 19.6→7.5 µs |

**시사점**: 파일 큐 왕복(fw+fr)은 홉당 ≈13.5 µs × 5~6홉 ≈ 70~80 µs (syscall 비용만). 원시 비용이 µs대로 측정되므로, E2E ms급 지연의 지배 요인은 syscall 자체보다 **홉마다의 프로세스 웨이크업/스케줄링 지연과 부하 시 디스크 꼬리**로 추정 — E2E 기준선 측정(풀 하니스)으로 확인 필요.

| 지표 | 현행 (가설) | Phase B 후 | Phase C 후 |
|------|------------|-----------|-----------|
| 주문 인바운드 내부 처리 p99 (client 수신→KRX 송신) | ms 단위 | < 1ms | **< 100µs** |
| 체결 아웃바운드 내부 처리 p99 (KRX 수신→client 송신) | 수 ms | < 2ms | < 200µs |
| 체결 1건당 미체결 조회 비교 횟수 | 최대 10,000 | — | ≤ 3 (해시 프로브) |
| SHM 유실 시 미체결 복구 | 불가능 | — | 스냅샷+저널로 완전 복원 |

※ "현행" 수치는 F1 완료 시 실측으로 대체하고 본 표를 갱신한다.

## 6. 공통 원칙 및 제약

- **C89 준수**: 변수 선언 스코프 최상단, `//` 주석 내 `{` 금지 (CLAUDE.md 컨벤션)
- **하위호환 우선**: 모든 신규 동작은 환경 변수/컴파일 스위치로 opt-in, 기본값은 기존 동작 (structured-logging #44, NO_INISAFE 패턴 동일)
- **호출자 무변경 지향**: sub/ 라이브러리 내부 최적화 우선 (select-send-optimize #41의 교훈)
- **장 마감 후 배포**: SHM 레이아웃 변경(F6, F7)은 미체결 0 상태에서만
- **기능별 PDCA**: 각 기능은 착수 시 개별 `*.plan.md` → `*.design.md`(정확한 before/after 코드) → 구현 → `*.report.md` 사이클 준수. 본 문서는 마스터 로드맵으로서 기능 간 순서·의존성·범위를 고정한다.

## 7. Out of Scope

- 프로세스 자동 재기동 (§3.5) — 별도 안정성 PDCA
- 용량 상수(ACC_NO_CNT 등) 설정화 — Phase D, 수요 발생 시
- PA 파생(2xxx, IMECO) 경로 — 본 계획은 채권(PB) 파이프라인과 PA 미체결 관리에 집중. 파생 경로는 F5 안정화 후 동일 패턴 수평 전개
- epoll/io_uring 등 Linux 전용 I/O — HP-UX/AIX 이식성 제약으로 배제 (poll 유지)
