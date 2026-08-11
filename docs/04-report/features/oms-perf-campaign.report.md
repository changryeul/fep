# PDCA Completion Report: OMS 성능·품질 캠페인 (F1~F6 + 버그픽스 + E2E)

> **Summary**: 고성능 OMS를 목표로 한 성능 최적화 8개 기능(F1~F6, F5 Stage1)과 그 과정에서 발견된 정합성 버그 3건 수정, E2E 하니스 구축을 하나의 캠페인으로 완료. 실측(F1 계측)으로 성능 로드맵의 핵심 전제를 검증·정정.
>
> **Campaign**: oms-perf-campaign
> **Design 산출물**: `02-design/features/{order-latency-metrics, hotpath-log-slog, seq-save-batch, file-rw-fd-pool, miche-index, miche-deadcode-fix, order-pipeline-dshm, rdcnt-underflow-guard}.design.md`
> **Plan**: `01-plan/features/oms-performance-roadmap.plan.md`
> **Match Rate**: 완료 기능 100% 서버 검증 (전 소스 빌드 에러 0, 단위 160 통과·0 실패)
> **Status**: Completed (F1~F6, F5-Stage1, 버그픽스 3, E2E, chk_kor)
> **Date**: 2026-08-06
> **검증 환경**: EC2 dev 서버(43.202.38.195, `~/new_fep`, Linux, `NO_INISAFE`)

---

## 1. Overview

성능 분석(핫패스 syscall 인벤토리 + SHM 주문상태 구조 조사)에서 3대 병목(디스크 파일 큐 IPC, 핫패스 동기 로깅, O(10,000) 미체결 선형조회)을 식별하고, F1(계측)을 먼저 만든 뒤 F2~F6를 실측 기반으로 진행. 모든 변경은 하위호환(opt-in 환경변수/설정감지 또는 폴백)이며 기본 동작 불변.

## 2. 완료 기능 (Deliverables)

| 기능 | 내용 | 활성화 | 실측/효과 |
|------|------|--------|----------|
| **F1** order-latency-metrics | `sub/lat_trace.c`(경계 타임스탬프), `utl/lat_report.c`(히스토그램), `utl/lat_bench.c`(마이크로벤치), order_inject 반복주입 | `-DLAT_TRACE` (기본 off) | 계측 인프라 |
| **F2** hotpath-log-slog | `sub/log_hot.c` — 핫패스 INFO 로그를 SHM 링(SLog)으로 라우팅 | `FEP_HOT_LOG=shm` | 로그 라인당 4.01→1.78µs |
| **F3** seq-save-batch | `sub/seq_throttle.c` + `seq_save.c` fd캐시·주기 flush | `FEP_SEQ_SAVE_INTERVAL` | 5.70→2.55µs(기본), ≈0(interval) |
| **F4** file-rw-fd-pool | `sub/fd_cache.c` — F_R/F_W·FIFO fd 캐시(폴백) | 폴백(항상) | 홉당 왕복 19.6→7.5µs |
| **F6** miche-index | `sub/miche_idx.c` — 미체결 조회 해시캐시(검증+폴백) | 폴백(항상) | 조회 O(10,000)→O(1) |
| **F5 Stage1** order-pipeline-dshm | 주문 인바운드 홉 파일큐↔DSHM 런타임 설정 감지 | proc.ini IDN/ODN | E2E 검증 완료 |

라이브러리 자동 포함(`Make_Lib_P_c.sh` 와일드카드), 단위 테스트 33건 신규(lat_trace 7, lat_report 12, log_hot 7, seq_throttle 7, fd_cache 8, miche_idx 8 — 합계는 파일 기준).

## 3. 버그픽스 (캠페인 중 발견, 별도 Design)

| 버그 | 위치 | 심각도 | 수정 |
|------|------|--------|------|
| **S_Fmt 버퍼 오버플로** | `pb_1100_ts.c`·`pb_1800_ts.c` 4곳 `memcpy(&S_Fmt,DataBuff,RecvLen)` | 높음(원격 크래시) | 구조체 크기 클램프. E2E 하니스가 발견(gdb 백트레이스로 근본원인) |
| **MICHE `=` 오타 + 데드코드** | `pa_1490_mp.c`(매수미체결수량 무조건 0 덮어씀) + 루프 내부 `i>=MAX_MICHE` 7곳(도치재시도·만석대기·못찾음처리 전부 미작동) | 높음(한도 오염·중복재처리 누락) | `==`로 수정 + 데드 블록 루프 밖 복원 |
| **RD_CNT 언더플로 재전송** | `pb_1100_ts.c` KRX 시퀀스 되감기 2곳 | 중간(운영자 판단→fail-safe) | `mun>RD_CNT` 시 ERROR+Exit_Process 가드(운영자 결정) |

## 4. 핵심 발견 — 실측이 로드맵 전제를 정정 (PDCA "Check")

F1 계측 + E2E로 측정한 결과, **초기 가설(디스크 큐가 주문 레이턴시 병목)이 반증**됨:
- 주문 인바운드 엔드투엔드(주입→KRX): 저부하 ~95µs, 버스트 500건 ~19ms — **file 모드 ≈ DSHM 모드**
- 즉 큐 방식(F5)·로깅(F2)·시퀀스(F3)는 주문 레이턴시 지배 요인이 **아님**. 병목은 **TCP 송신 + 프로세스 스케줄링/홉**.
- **정정**: 측정은 전부 `NO_INISAFE`(암호화 제외) 값. "병목=암호화+TCP"의 암호화 부분은 측정 경로에 없었으므로 오류. INISAFE는 운영 필수·고정비용(최적화 대상 아님).
- **로드맵 재조정**: 성능 레버를 큐 I/O에서 홉 수/스케줄링/처리량으로 이동. F8(홉 축소)은 조사 결과 안전한 대상 없어 운영자 판단으로 종료.

F1~F4의 syscall/디스크 절감은 여전히 유효(CPU·디스크 부하 감소, 버스트 안정성)하나, 단일 주문 왕복 레이턴시 개선은 제한적임을 정직하게 기록.

## 5. 인프라 산출물

- **E2E 하니스** `test/e2e/`(mock_oms, run_e2e.sh, e2e_lat.sh) — 실제 PB 파이프라인(pz→mock_krx→pb_1101→mock_oms→pb_1301) 기동·주입·검증. mock_krx는 실 KRX처럼 주문 소켓 echo 안 함(I-9 수정).
- **test_chk_kor 정리**: EUC-KR 오가정 테스트를 실제 UTF-8 계약으로 재작성 → 서버 전체 160 통과·**0 실패**(세션 내내 있던 3건 소음 제거).

## 6. Verification

- 서버 전 소스 빌드(`mk.sh sub && mk.sh src`) exit 0, 에러 0 (반복 확인)
- 단위 테스트 160 통과, 0 실패
- F5 E2E: 파일/DSHM 양 경로 100건 주입→100건 KRX 도달, 크래시 0
- off 빌드 무영향(V-05): LAT 심볼 0, 기본 동작 바이트 동일
- 서버 상태 운영 빌드로 원복(cfg 파일모드, DSHM 세그먼트 정리)

## 7. 미완 / 후속 (Deferred)

| 항목 | 상태 |
|------|------|
| F5 Stage2 (체결 아웃바운드 DSHM) | 미착수 — 레이턴시 아닌 처리량/내구성 관점 재정의 필요 |
| F7 miche-rebuild (체결파일 replay 복구) | 설계만(저널 신설안 폐기, replay 도구로 축소) |
| I-9 잔여 (버스트 file 재읽기) | 하니스 아티팩트로 규명, 잠재 취약성은 RD_CNT 가드로 완화 |
| 시장별 모듈 재편(A안, pc_/po_) | **KRX 전 시장 전문 규격 대기(blocked)** — 사용자 제공 예정 |
| ~~pb_1200_tr.c:955 `INT_MEG_SEQ(10)` 배열 범위 초과~~ | ✅ 수정 완료(meg-seq-log-fix): 진단 로그의 OOB 읽기+인덱스1 누락+varargs 불일치를 0~9 정확히 10개로 정정. 서버 빌드 검증 |

## 8. Lessons Learned

1. **계측 우선(F1)의 가치**: 가설(디스크 큐 병목)을 측정으로 반증 → 이후 성능 투자 방향을 실증 기반으로 재조정. F1 없이 F5를 밀어붙였다면 잘못된 최적화에 자원 소모.
2. **E2E 하니스가 실제 버그를 잡음**: S_Fmt 오버플로·RD_CNT 폭주는 단위 테스트로는 못 잡는 통합 결함. 실 파이프라인 기동이 결정적.
3. **하위호환 규율**: 전 기능 opt-in/폴백 → 운영 리스크 없이 단계 검증·롤백 가능.
4. **정직한 반증 기록**: "안 되는 것/틀린 가설"을 문서에 남겨야 다음 세션이 같은 함정을 피함(V-06 정정, F8 종료).
