# magic-numbers-extraction 분석 보고서

> **분석 유형**: 갭 분석 (설계 vs 구현)
>
> **프로젝트**: FEP (KRX용 프론트엔드 프로세서)
> **분석자**: Claude Code (gap-detector)
> **날짜**: 2026-02-22
> **설계 문서**: [magic-numbers-extraction.design.md](../02-design/features/magic-numbers-extraction.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

매직 넘버 추출(raw `4096`, `1228800`, 82 리터럴을 명명된 상수로 대체)의 구현이 모든 29개 FR 항목에 걸쳐 설계 문서와 일치하는지 검증합니다. 이것은 "magic-numbers-extraction" 기능의 PDCA 사이클 검증 단계입니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/magic-numbers-extraction.design.md`
- **헤더**: `st01/inc/fep_fepp.h` (상수 정의)
- **대상 파일**: PA, PB, PW, PX 모듈에 걸친 17개 소스 파일 + 2개 UDP 파일 + 1개 KRX 헤더 수정
- **제외 파일**: `pa_5000_qr.c` (데드 코드), `pa_5200_qs.c` (큐 MAXSIZE), BACK2025/, JC_OLD/
- **분석 날짜**: 2026-02-22

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 100% | PASS |
| 아키텍처 준수 | 100% | PASS |
| 규약 준수 | 100% | PASS |
| **전체** | **100%** | **PASS** |

---

## 3. FR-01: `inc/fep_fepp.h`에서의 상수 정의

| 검사 | 설계 | 구현 | 상태 |
|-------|--------|----------------|--------|
| 파일 | `inc/fep_fepp.h` | `st01/inc/fep_fepp.h` (23줄) | PASS |
| 코멘트 블록 | `FEP Data Buffer Constants` | 줄 14-16: 정확히 일치 | PASS |
| KRX_DATA_BUFF_SIZE | `#define KRX_DATA_BUFF_SIZE 4096` | 줄 17: `#define KRX_DATA_BUFF_SIZE    4096        /* KRX data buffer size */` | PASS |
| UDP_SOCK_RCVBUF_SIZE | `#define UDP_SOCK_RCVBUF_SIZE 1228800` | 줄 18: `#define UDP_SOCK_RCVBUF_SIZE  1228800     /* UDP socket recv buffer (1200KB) */` | PASS |
| 배치 | `End of Program` 이전 | 줄 20의 `End of Program` 이전 줄 17-18 | PASS |

**점수: 100%** (5/5 검사 통과)

---

## 4. 배치 1: PA 모듈 -- DataBuff 선언 (FR-02 to FR-06)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-02 | `src/PA/pa_1100_ts.c` | 79 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-03 | `src/PA/pa_1200_tr.c` | 88 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-04 | `src/PA/pa_3100_ts.c` | 56 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-05 | `src/PA/pa_7800_tr.c` | 57 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-06 | `src/PA/pa_8100_ts.c` | 261 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |

**점수: 100%** (5/5 항목 통과)

---

## 5. 배치 2: PA 모듈 -- Chg_Data + 경계 검사 (FR-07 to FR-11)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-07 | `src/PA/pa_7000_tr.c` | 242 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-08 | `src/PA/pa_7000_tr.c` | 461 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |
| FR-09 | `src/PA/pa_8100_ts.c` | 476 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |
| FR-10 | `src/PA/pa_8200_tr.c` | 262 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-11 | `src/PA/pa_8200_tr.c` | 425 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |

**점수: 100%** (5/5 항목 통과)

---

## 6. 배치 3: PB 모듈 -- DataBuff 선언 (FR-12 to FR-16)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-12 | `src/PB/pb_1100_ts.c` | 98 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-13 | `src/PB/pb_1200_tr.c` | 100 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-14 | `src/PB/pb_1800_ts.c` | 95 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-15 | `src/PB/pb_1800_ts.c` | 96 | `S_Data[KRX_DATA_BUFF_SIZE]` | `S_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-16 | `src/PB/pb_7800_tr.c` | 72 | `DataBuff[KRX_DATA_BUFF_SIZE]` | `DataBuff[KRX_DATA_BUFF_SIZE]` | PASS |

**점수: 100%** (5/5 항목 통과)

---

## 7. 배치 4: PB 모듈 -- Chg_Data + 경계 검사 (FR-17 to FR-22)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-17 | `src/PB/pb_7200_tr.c` | 226 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-18 | `src/PB/pb_7200_tr.c` | 353 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |
| FR-19 | `src/PB/pb_8100_ts.c` | 281 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-20 | `src/PB/pb_8100_ts.c` | 496 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |
| FR-21 | `src/PB/pb_8200_tr.c` | 238 | `Chg_Data[KRX_DATA_BUFF_SIZE]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-22 | `src/PB/pb_8200_tr.c` | 404 | `rt > KRX_DATA_BUFF_SIZE` | `rt > KRX_DATA_BUFF_SIZE` | PASS |

**점수: 100%** (6/6 항목 통과)

---

## 8. 배치 5: PW/PX 모듈 -- 시스템 버퍼 (FR-23 to FR-26)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-23 | `src/PW/pw_1000_mp.c` | 133 | `r_buf[KRX_DATA_BUFF_SIZE], w_buf[KRX_DATA_BUFF_SIZE]` | `r_buf[KRX_DATA_BUFF_SIZE], w_buf[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-24 | `src/PW/pw_1000_mp.c` | 152 | `KRX_DATA_BUFF_SIZE / rec_size` | `KRX_DATA_BUFF_SIZE / rec_size` | PASS |
| FR-25 | `src/PX/px_setautostop.c` | 16 | `w_buf[KRX_DATA_BUFF_SIZE]` | `w_buf[KRX_DATA_BUFF_SIZE]` | PASS |
| FR-26 | `src/PX/px_chktrcnt.c` | 15 | `buf[KRX_DATA_BUFF_SIZE]` | `buf[KRX_DATA_BUFF_SIZE]` | PASS |

**점수: 100%** (4/4 항목 통과)

---

## 9. 배치 6: UDP 소켓 버퍼 + KRX 헤더 (FR-27 to FR-29)

| FR | 파일 | 줄 | 예상 | 실제 | 상태 |
|----|------|:----:|----------|--------|:------:|
| FR-27 | `src/PA/pa_7100_ur.c` | 298 | `val = UDP_SOCK_RCVBUF_SIZE;` | `val = UDP_SOCK_RCVBUF_SIZE;` | PASS |
| FR-28 | `src/PB/pb_7100_ur.c` | 338 | `val = UDP_SOCK_RCVBUF_SIZE;` | `val = UDP_SOCK_RCVBUF_SIZE;` | PASS |
| FR-29 | `src/PB/pb_8200_tr.c` | 563 | `R_Pkt->Data+KRX_HEAD_LEN, strlen(R_Pkt->Data) -KRX_HEAD_LEN` | `R_Pkt->Data+KRX_HEAD_LEN, strlen(R_Pkt->Data) -KRX_HEAD_LEN` | PASS |

**점수: 100%** (3/3 항목 통과)

---

## 10. 잔여 스캔 -- 검증 검사

### 10.1 `st01/src/`에서의 Raw `4096`

| 파일 | 줄 | 문맥 | 분류 |
|------|:----:|---------|:--------------:|
| `PA/pa_5200_qs.c` | 34 | `#define MAXSIZE 4096` (큐 상수) | EXCLUDED -- 다른 목적 |
| `PA/pa_5000_qr.c` | 36 | `#define MAXSIZE 4096` (큐 상수) | EXCLUDED -- 다른 목적 |
| `PA/JC_OLD/jc_1400_tr.c.krx` | 42 | `DataBuff[4096]` | EXCLUDED -- JC_OLD 보관 |
| `PA/JC_OLD/jc_1600_tr.c.krx` | 37 | `DataBuff[4096]` | EXCLUDED -- JC_OLD 보관 |
| `PA/JC_OLD/jc_1900_ts.c.krx.080729` | 46 | `DataBuff[4096]` | EXCLUDED -- JC_OLD 보관 |
| `PA/JC_OLD/jc_1900_ts.c.krx` | 46 | `DataBuff[4096]` | EXCLUDED -- JC_OLD 보관 |
| `PA/JC_OLD/jc_1100_ts.c.krx` | 57 | `DataBuff[4096]` | EXCLUDED -- JC_OLD 보관 |
| `PA/BACK2025/pa_8100_ts.c` | 316,541 | `Chg_Data[4096]`, `rt > 4096` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_8200_tr.c` | 297,461 | `Chg_Data[4096]`, `rt > 4096` | EXCLUDED -- BACK2025 백업 |

**결과**: 활성 소스 파일에서 raw 4096이 0개 (큐 MAXSIZE 및 보관/백업 파일만 남음). **PASS**

### 10.2 `st01/src/`에서의 Raw `1228800`

| 파일 | 문맥 | 분류 |
|------|---------|:--------------:|
| `PA/BACK2025/pa_7500_ur.c` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_7111_ur.c` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_7100_ur.c` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_7500_ur.20220221` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_7100_ur.org` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_7100_ur.20220221` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_6100_ur.c` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PA/BACK2025/pa_6100_ur.20220221` | `val = 1228800;` | EXCLUDED -- BACK2025 백업 |
| `PB/pb_7100_ur.org` | `val = 1228800;` | EXCLUDED -- .org 백업 |

**결과**: 활성 소스 파일에서 raw 1228800이 0개 (BACK2025 및 .org 백업만 남음). **PASS**

### 10.3 `src/PB/pb_8200_tr.c`에서의 Raw `Data+82`

**결과**: `pb_8200_tr.c`에서 `Data+82`에 대한 0 일치. **PASS**

### 10.4 대상 파일 범위

모든 17개 파일에 `KRX_DATA_BUFF_SIZE` 포함:

| # | 파일 | 발생 |
|---|------|:-----------:|
| 1 | `PA/pa_1100_ts.c` | 1 |
| 2 | `PA/pa_1200_tr.c` | 1 |
| 3 | `PA/pa_3100_ts.c` | 1 |
| 4 | `PA/pa_7000_tr.c` | 2 |
| 5 | `PA/pa_7800_tr.c` | 1 |
| 6 | `PA/pa_8100_ts.c` | 2 |
| 7 | `PA/pa_8200_tr.c` | 2 |
| 8 | `PB/pb_1100_ts.c` | 1 |
| 9 | `PB/pb_1200_tr.c` | 1 |
| 10 | `PB/pb_1800_ts.c` | 2 |
| 11 | `PB/pb_7200_tr.c` | 2 |
| 12 | `PB/pb_7800_tr.c` | 1 |
| 13 | `PB/pb_8100_ts.c` | 2 |
| 14 | `PB/pb_8200_tr.c` | 2 |
| 15 | `PW/pw_1000_mp.c` | 2 |
| 16 | `PX/px_chktrcnt.c` | 1 |
| 17 | `PX/px_setautostop.c` | 1 |

모든 2개 파일에 `UDP_SOCK_RCVBUF_SIZE` 포함:

| # | 파일 | 발생 |
|---|------|:-----------:|
| 1 | `PA/pa_7100_ur.c` | 1 |
| 2 | `PB/pb_7100_ur.c` | 1 |

**합계**: 17 + 2 = 28개의 상수 사용 파일로 모든 28개 대체 FR 항목 범위. **PASS**

---

## 11. 비함수 요구사항 검증

| 검사 | 예상 | 실제 | 상태 |
|-------|----------|--------|:------:|
| 0 함수 변경 | 모든 대체는 값이 동일 | `KRX_DATA_BUFF_SIZE=4096`, `UDP_SOCK_RCVBUF_SIZE=1228800`, `KRX_HEAD_LEN=82` | PASS |
| C89 호환성 | 정수 리터럴이 있는 `#define` | 모든 상수는 `#define` 정수 리터럴 사용 | PASS |
| BACK2025 미수정 | 수정 없음 | BACK2025 파일은 여전히 raw 리터럴 포함 | PASS |
| JC_OLD 미수정 | 수정 없음 | JC_OLD 파일은 여전히 raw 리터럴 포함 | PASS |
| .org 파일 미수정 | 수정 없음 | `pb_7100_ur.org`은 여전히 raw `1228800` 포함 | PASS |
| 헤더 포함 | 모든 대상 파일이 `fep_fepp.h` 포함 | 검증됨: `pa_1100_ts.c:13`, `pw_1000_mp.c:11`, `px_chktrcnt.c:10`, `px_setautostop.c:10` | PASS |

**점수: 100%** (6/6 검사 통과)

---

## 12. FR 요약

| FR | 설명 | 상태 |
|----|-------------|:------:|
| FR-01 | fep_fepp.h에서 KRX_DATA_BUFF_SIZE 및 UDP_SOCK_RCVBUF_SIZE 정의 | PASS |
| FR-02 | pa_1100_ts.c:79 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-03 | pa_1200_tr.c:88 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-04 | pa_3100_ts.c:56 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-05 | pa_7800_tr.c:57 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-06 | pa_8100_ts.c:261 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-07 | pa_7000_tr.c:242 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-08 | pa_7000_tr.c:461 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-09 | pa_8100_ts.c:476 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-10 | pa_8200_tr.c:262 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-11 | pa_8200_tr.c:425 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-12 | pb_1100_ts.c:98 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-13 | pb_1200_tr.c:100 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-14 | pb_1800_ts.c:95 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-15 | pb_1800_ts.c:96 S_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-16 | pb_7800_tr.c:72 DataBuff[KRX_DATA_BUFF_SIZE] | PASS |
| FR-17 | pb_7200_tr.c:226 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-18 | pb_7200_tr.c:353 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-19 | pb_8100_ts.c:281 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-20 | pb_8100_ts.c:496 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-21 | pb_8200_tr.c:238 Chg_Data[KRX_DATA_BUFF_SIZE] | PASS |
| FR-22 | pb_8200_tr.c:404 rt > KRX_DATA_BUFF_SIZE | PASS |
| FR-23 | pw_1000_mp.c:133 r_buf[KRX_DATA_BUFF_SIZE], w_buf[KRX_DATA_BUFF_SIZE] | PASS |
| FR-24 | pw_1000_mp.c:152 KRX_DATA_BUFF_SIZE / rec_size | PASS |
| FR-25 | px_setautostop.c:16 w_buf[KRX_DATA_BUFF_SIZE] | PASS |
| FR-26 | px_chktrcnt.c:15 buf[KRX_DATA_BUFF_SIZE] | PASS |
| FR-27 | pa_7100_ur.c:298 val = UDP_SOCK_RCVBUF_SIZE | PASS |
| FR-28 | pb_7100_ur.c:338 val = UDP_SOCK_RCVBUF_SIZE | PASS |
| FR-29 | pb_8200_tr.c:563 R_Pkt->Data+KRX_HEAD_LEN | PASS |

**29/29 FR 항목: PASS (100%)**

---

## 13. 발견된 차이점

### 설계에 없고 구현에 있음 (설계 X, 구현 O)

없음.

### 구현에 없고 설계에 있음 (설계 O, 구현 X)

없음.

### 변경된 기능 (설계 != 구현)

없음.

---

## 14. 설계 문서 업데이트 필요

없음. 설계 문서가 모든 29개 FR 항목을 정확하게 설명하며 모든 검증 검사가 통과됩니다.

---

## 15. 권장 조치

조치 필요 없음. 일치율이 100%.

- 빌드 검증: NOT TESTED (`mk.sh` 빌드 환경이 있는 서버 필요)

---

## 16. 다음 단계

- [ ] 서버에서 빌드 검증 (`mk.sh all`)
- [ ] 완료 보고서 작성 (`magic-numbers-extraction.report.md`)
- [ ] PDCA 문서 보관

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-22 | 초기 갭 분석 -- 29/29 PASS, 100% 일치 | Claude Code (gap-detector) |
