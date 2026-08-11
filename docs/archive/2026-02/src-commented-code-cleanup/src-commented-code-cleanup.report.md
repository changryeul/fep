# 보고서: src-commented-code-cleanup

## 요약

| 메트릭 | 값 |
|--------|-------|
| 일치율 | 100% |
| 반복 | 1 |
| 수정된 파일 | 35 |
| FR 항목 (설계) | 30 |
| FR 항목 (실제) | 30 + 5 보너스 |
| 제거된 줄 | ~489 |
| 추가된 줄 | 0 |
| 함수 변화 | 없음 |

## 개요

활성 `src/` 소스 파일 전체에서 PA (18개 파일), PB (9개 파일), PZ (7개 파일), PW (1개 파일) 모듈에서 모든 `//` 및 `/* */` 주석 처리된 데드 코드를 제거했습니다. 이는 FEP 코드베이스 전체를 위한 주석 처리된 코드 정리 시리즈를 완료합니다.

## PDCA 타임라인

| 단계 | 작업 | 비고 |
|-------|--------|-------|
| 계획 | 계획 문서 생성 | ~25-28개 파일, ~210+ 줄 추정 |
| 설계 | 30개 FR 항목으로 설계 생성 | 상세한 라인별 사양, ~391 줄 추정 |
| 실행 (통과 1) | 30개 파일에서 구현 | 초기 통과에서 ~50개 항목 누락 |
| 검증 | 갭 분석 스캔 | 16개 파일에서 ~50개 남은 항목 발견 |
| 대응 (반복 1) | 모든 남은 항목 + 5개 보너스 파일 수정 | 100% 일치율 달성 |
| 검증 (최종) | 포괄적 grep 검증 | 활성 src/ 파일에서 데드 코드 없음 |

## 모듈별 결과

### PZ 모듈 (7개 파일, ~58줄)

| 파일 | 제거된 줄 | 주요 패턴 |
|------|:------------:|--------------|
| pz_memory_conf.c | 33 | `//sprintf` 설정 로딩 아티팩트 |
| pz_daemon_proc.c | 12 | `//sprintf`, `//sigrelse/sighold` |
| pz_fepp.c | 6 | `//sprintf`, `//sigrelse` |
| pz_filechk.c | 3 | `//sprintf` 이전 경로 |
| pz_procchk.c | 2 | `//Used`, `//sprintf` |
| pz_compact.c | 1 | `//char tmp[256]` 데드 변수 |
| pz_memory_proc.c | 1 | `//if (INFO...)` 비활성화된 조건부 |

### PB 모듈 (9개 파일, ~177줄)

| 파일 | 제거된 줄 | 주요 패턴 |
|------|:------------:|--------------|
| pb_1800_ts.c | 87+ | `//Free_All(1)` x11, `//INL_*`, `/* */` 블록 |
| pb_7800_tr.c | 25 | `/* */` 블록, `//rt = F_W`, `//#define` |
| pb_1100_ts.c | 18 | `//back_rd_cnt`, `//mun/INT_SEQ/RD_CNT`, `//return` |
| pb_1200_tr.c | 14 | `//FILE_BUFF_FORMAT`, `//extern net_ctx`, `//TCP2_LINE_ST` |
| pb_8200_tr.c | 14 | `//t = mktime`, `/* business_day */` 블록 |
| pb_7200_tr.c | 12 | `//R_CNT`, `//close (Newfd)`, `/* Time_Out */` |
| pb_7100_ts.c | 4 | `//t = mktime`, `//close (Newfd)` |
| pb_7100_ur.c | 2 | `//#include`, `//setsockopt` |
| pb_8100_ts.c | 1 | `//t = mktime` |

### PA 모듈 (18개 파일, ~251줄)

| 파일 | 제거된 줄 | 주요 패턴 |
|------|:------------:|--------------|
| pa_1100_ts.c | 37 | `/* #include */`, `/* Seq 원복 */` x3, `/* SKIP CHECK */` |
| pa_7100_dd.c | 37 | `//memcpy item_stat`, 33줄 가격 한계 블록 |
| pa_5200_qs.c | 33 | `/* FIFO_fd */` 블록, `/* QueueClear */` 블록 |
| pa_1290_mp.c | 33 | `/* 전략구축시 */`, `/* edt_gum/org_gum */`, `//Shm_Mk_PreMatch` |
| pa_2100_ts.c | 21 | `//t = mktime`, `/* */` 블록, `//Dshm_Add_Count` |
| pa_2700_tr.c | 19 | `/* business_day */`, `/* if rt == NOTOK */` |
| pa_2200_tr.c | 15 | `//IMECO_TCP_HEAD`, `/* business_day */` |
| pa_7000_tr.c | 15 | `//t = mktime`, `/* business_day */`, `//Device_Close` |
| pa_7100_ts.c | 15 | `//t = mktime`, `/* business_day */`, `//Device_Close` |
| pa_8200_tr.c | 2 | `//Device_Close`, `//TCP2_LINE_ST` |
| pa_9000_mp.c | 6 | `//Shm_Risk[0].ProFit` x6 |
| pa_1200_tr.c | 4 | `//TCP2_PROC_ST`, `//TCP2_LINE_ST`, `//FILE_BUFF` |
| pa_1600_tr.c | 3 | `//t = mktime`, `//if`, `//memcpy` |
| pa_5000_qr.c | 3 | `//Msgbuf`, `//ReceiveQueue`, `//Log` |
| pa_7100_ur.c | 3 | `//Set_TR_Time`, `//INT_SEQ`, `//setsockopt` |
| pa_7800_tr.c | 2 | `//TCP2_PROC_ST`, `//TCP2_LINE_ST` |
| pa_1490_mp.c | 1 | `//KRX_SETTLE_DATA` 데드 변수 |
| pa_1600_mp.c | 1 | `//memcmp OUD` 비활성화된 옵션 |
| pa_8100_ts.c | 1 | `//t = mktime` |

### PW 모듈 (1개 파일, ~3줄)

| 파일 | 제거된 줄 | 주요 패턴 |
|------|:------------:|--------------|
| pw_1000_mp.c | 3 | `//if (fifo_fd == -1)`, `//if (fd == -1)`, `//else if` |

## 검증

- 최종 grep 스캔: 활성 `src/` 파일에서 **데드 코드 없음**
- BACKUP/ 및 BACK2025/ 파일: 수정 없음 (범위 외)
- 한글 설명 주석: 모두 보존됨
- `// 2025EDIT` 마커: 모두 보존됨
- 함수 배너 및 섹션 주석: 모두 보존됨
- 빌드 검증: 서버의 `mk.sh src` 대기 중

## 누적 데드 코드 제거 (완전한 시리즈)

| # | 기능 | 줄 | 파일 |
|:-:|---------|:-----:|:-----:|
| 1 | dead-code-cleanup (`#if 0`) | 1,945 | 36 |
| 2 | pa-pb-8100-cleanup | 64 | 2 |
| 3 | commented-code-cleanup-round2 | 213 | 14 |
| 4 | sub-commented-code-final | 39 | 3 |
| 5 | **src-commented-code-cleanup** | **489** | **35** |
| | **총합** | **~2,750** | **~90** |

전체 FEP 코드베이스 (`sub/` + `src/`)는 이제 활성 소스 파일에서 주석 처리된 데드 코드가 없습니다.
