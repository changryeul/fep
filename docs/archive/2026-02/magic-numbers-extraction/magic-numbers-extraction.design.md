# 설계: magic-numbers-extraction

## 참조

- 계획: `docs/01-plan/features/magic-numbers-extraction.plan.md`

## 요약

`inc/fep_fepp.h`에서 `KRX_DATA_BUFF_SIZE` (4096) 및 `UDP_SOCK_RCVBUF_SIZE` (1228800)을 정의한 다음 17개의 소스 파일에서 모든 원시 리터럴을 교체. 또한 `pb_8200_tr.c`에서 1개의 원시 `82`를 기존 `KRX_HEAD_LEN`으로 교체.

## 기능 요구사항

### FR-01: `inc/fep_fepp.h`에서 상수 정의

`End of Program` 블록 앞에 추가:

```c
/*------------------------------------------------------------------------
    FEP Data Buffer Constants
------------------------------------------------------------------------*/
#define KRX_DATA_BUFF_SIZE    4096        /* KRX data buffer size */
#define UDP_SOCK_RCVBUF_SIZE  1228800     /* UDP socket recv buffer (1200KB) */
```

### Batch 1: PA 모듈 — DataBuff 선언 (5개 파일, 5개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-02 | `src/PA/pa_1100_ts.c` | 79 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-03 | `src/PA/pa_1200_tr.c` | 88 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-04 | `src/PA/pa_3100_ts.c` | 56 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-05 | `src/PA/pa_7800_tr.c` | 57 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-06 | `src/PA/pa_8100_ts.c` | 261 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |

### Batch 2: PA 모듈 — Chg_Data + 경계 체크 (3개 파일, 6개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-07 | `src/PA/pa_7000_tr.c` | 242 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |
| FR-08 | `src/PA/pa_7000_tr.c` | 461 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |
| FR-09 | `src/PA/pa_8100_ts.c` | 476 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |
| FR-10 | `src/PA/pa_8200_tr.c` | 262 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |
| FR-11 | `src/PA/pa_8200_tr.c` | 425 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |

주의: FR-06 (pa_8100_ts.c:261)은 Batch 1, FR-09 (pa_8100_ts.c:476)은 이 배치.

### Batch 3: PB 모듈 — DataBuff 선언 (5개 파일, 6개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-12 | `src/PB/pb_1100_ts.c` | 98 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-13 | `src/PB/pb_1200_tr.c` | 100 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-14 | `src/PB/pb_1800_ts.c` | 95 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |
| FR-15 | `src/PB/pb_1800_ts.c` | 96 | `S_Data[4096]` | `S_Data[KRX_DATA_BUFF_SIZE]` |
| FR-16 | `src/PB/pb_7800_tr.c` | 72 | `DataBuff[4096]` | `DataBuff[KRX_DATA_BUFF_SIZE]` |

### Batch 4: PB 모듈 — Chg_Data + 경계 체크 (3개 파일, 6개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-17 | `src/PB/pb_7200_tr.c` | 226 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |
| FR-18 | `src/PB/pb_7200_tr.c` | 353 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |
| FR-19 | `src/PB/pb_8100_ts.c` | 281 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |
| FR-20 | `src/PB/pb_8100_ts.c` | 496 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |
| FR-21 | `src/PB/pb_8200_tr.c` | 238 | `Chg_Data[4096]` | `Chg_Data[KRX_DATA_BUFF_SIZE]` |
| FR-22 | `src/PB/pb_8200_tr.c` | 404 | `rt > 4096` | `rt > KRX_DATA_BUFF_SIZE` |

### Batch 5: PW/PX 모듈 — 시스템 버퍼 (3개 파일, 5개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-23 | `src/PW/pw_1000_mp.c` | 133 | `r_buf[4096], w_buf[4096]` | `r_buf[KRX_DATA_BUFF_SIZE], w_buf[KRX_DATA_BUFF_SIZE]` |
| FR-24 | `src/PW/pw_1000_mp.c` | 152 | `4096 / rec_size` | `KRX_DATA_BUFF_SIZE / rec_size` |
| FR-25 | `src/PX/px_setautostop.c` | 16 | `w_buf[4096]` | `w_buf[KRX_DATA_BUFF_SIZE]` |
| FR-26 | `src/PX/px_chktrcnt.c` | 15 | `buf[4096]` | `buf[KRX_DATA_BUFF_SIZE]` |

### Batch 6: UDP 소켓 버퍼 + KRX 헤더 (3개 파일, 3개 발생)

| FR | 파일 | 라인 | 이전 | 이후 |
|----|------|:----:|--------|-------|
| FR-27 | `src/PA/pa_7100_ur.c` | 298 | `val = 1228800;` | `val = UDP_SOCK_RCVBUF_SIZE;` |
| FR-28 | `src/PB/pb_7100_ur.c` | 338 | `val = 1228800;` | `val = UDP_SOCK_RCVBUF_SIZE;` |
| FR-29 | `src/PB/pb_8200_tr.c` | 563 | `R_Pkt->Data+82, strlen(R_Pkt->Data) -82` | `R_Pkt->Data+KRX_HEAD_LEN, strlen(R_Pkt->Data) -KRX_HEAD_LEN` |

## 구현 순서

1. FR-01 (헤더에서 상수 정의) — 먼저 필수
2. Batch 1 (PA DataBuff) — FR-02부터 FR-06
3. Batch 2 (PA Chg_Data + 경계) — FR-07부터 FR-11
4. Batch 3 (PB DataBuff) — FR-12부터 FR-16
5. Batch 4 (PB Chg_Data + 경계) — FR-17부터 FR-22
6. Batch 5 (PW/PX 버퍼) — FR-23부터 FR-26
7. Batch 6 (UDP + KRX 헤더) — FR-27부터 FR-29

## 검증

- `grep -rn '\b4096\b' st01/src/`는 제외된 파일 (pa_5200_qs.c MAXSIZE, pa_5000_qr.c 데드 코드) 및 BACK2025/JC_OLD만 표시해야 함
- `grep -rn '1228800' st01/src/`는 BACK2025/.org 파일만 표시해야 함
- `grep -n 'Data+82' st01/src/PB/pb_8200_tr.c`는 0개 일치 반환해야 함
- 모든 17개 대상 파일이 `KRX_DATA_BUFF_SIZE` 또는 `UDP_SOCK_RCVBUF_SIZE` 포함해야 함

## 비기능 요구사항

- 제로 기능 변경 — 모든 교체는 값 동일
- C89 호환 상수 이름 및 구문
- BACK2025/, JC_OLD/, .org 파일에 변경 없음
