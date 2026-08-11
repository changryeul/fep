# select-recv-bounds-check Planning Document

> **요약**: select_recv.c 6개 함수에 pkt_len 상한 검증 추가 — 원격 버퍼 오버플로 방지
>
> **프로젝트**: FEP (Front-End Processor)
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안

---

## 1. 개요

### 1.1 목적

`sub/select_recv.c`의 6개 TCP 수신 함수가 네트워크에서 파싱한 `pkt_len` 값에 대해 **상한(upper-bound) 검증 없이** `Recvn()`을 호출한다. 공격자나 손상된 패킷이 거대한 길이 필드를 보내면 호출자의 고정 크기 버퍼(`RecvPkt[5120]`, `DataBuff[4096]` 등)를 넘겨 쓸 수 있다. 이 취약점을 일괄 수정한다.

### 1.2 배경

- 모든 `Select_Receive*` 함수는 `AtoIf()`로 길이 필드를 파싱 → `Recvn(p_sfd, buf+offset, pkt_len)`으로 수신
- `AtoIf()`는 비숫자 입력 시 0을 반환하고, 큰 숫자 입력 시 int 범위까지 반환 가능 (상한 없음)
- 일부 함수는 하한 검사(LOG만 수행)가 있으나 `return NOTOK` 없이 그대로 `Recvn()` 진행 — 검사가 무의미
- `Sise_Select_Receive()`만 유일하게 `recv()` + 정적 버퍼 + cap으로 안전함

### 1.3 관련 문서

- Architecture: `docs/FEP_Architecture_Analysis.md`
- Previous: `docs/archive/2026-02/sizeof-memset-bugfix/` (유사 버퍼 안전성 수정)
- Source: `st01/sub/select_recv.c` (513 lines, 7 functions)

---

## 2. 범위

### 2.1 범위 내

- [x] `Select_Receive` — pkt_len 상한 추가 + 하한 검사에 return NOTOK 추가
- [x] `Select_Receive2` — pkt_len 상한/하한 검증 추가
- [x] `Select_Receive_Krx` — pkt_len 상한 추가
- [x] `Select_Receive_Cli` — pkt_len 상한 추가 + 하한 검사에 return NOTOK 추가
- [x] `Select_Receive_Imeco` — pkt_len 상한 추가 + 하한 검사에 return NOTOK 추가
- [x] `Select_Receive_Imeco_Sise` — pkt_len 상한 추가 + 하한 검사에 return NOTOK 추가

### 2.2 범위 외

- `Sise_Select_Receive()` — 이미 `recv()` + `SZ_FEEDDATA_MAX - sockpos` cap으로 안전
- 호출부(caller) 코드 변경 — 함수 시그니처 유지, 동작 호환
- `AtoIf()` 함수 자체 수정 — 별도 feature로 분리 예정

---

## 3. 요구사항

### 3.1 기능 요구사항

| ID | 요구사항 | 우선순위 | 상태 |
|----|-------------|----------|--------|
| FR-01 | `Select_Receive`: pkt_len > TCP_BUFF_MAX_LEN-5 시 Log + return NOTOK | High | Pending |
| FR-02 | `Select_Receive`: 기존 하한 검사(line 69-70) log-only → return NOTOK 추가 | High | Pending |
| FR-03 | `Select_Receive2`: pkt_len <= 0 또는 pkt_len > TCP_BUFF_MAX_LEN 시 Log + return NOTOK | High | Pending |
| FR-04 | `Select_Receive2`: pkt_len - p_len <= 0 시 Log + return NOTOK | High | Pending |
| FR-05 | `Select_Receive_Krx`: pkt_len > KRX_DATA_BUFF_SIZE - p_len 시 Log + return NOTOK | High | Pending |
| FR-06 | `Select_Receive_Cli`: pkt_len > CLI_BUFF_MAX_LEN-4 시 Log + return NOTOK | High | Pending |
| FR-07 | `Select_Receive_Cli`: 기존 하한 검사(line 282-283) log-only → return NOTOK 추가 | High | Pending |
| FR-08 | `Select_Receive_Imeco`: pkt_len > TCP_BUFF_MAX_LEN-4 시 Log + return NOTOK | High | Pending |
| FR-09 | `Select_Receive_Imeco`: 기존 하한 검사(line 352-353) log-only → return NOTOK 추가 | High | Pending |
| FR-10 | `Select_Receive_Imeco_Sise`: pkt_len > TCP_BUFF_MAX_LEN-10 시 Log + return NOTOK | High | Pending |
| FR-11 | `Select_Receive_Imeco_Sise`: 기존 하한 검사(line 422-423) log-only → return NOTOK 추가 | High | Pending |
| FR-12 | `Select_Receive2`: caller 없음 확인, 그래도 방어적 검증 추가 (미래 caller 대비) | Medium | Pending |

### 3.2 비기능 요구사항

| 범주 | 기준 | Measurement Method |
|----------|----------|-------------------|
| 안전성 | pkt_len이 버퍼 크기를 초과할 수 없음 | 코드 리뷰: 모든 Recvn 호출 전 상한 검사 존재 확인 |
| 호환성 | 함수 시그니처 변경 없음, 정상 패킷 동작 불변 | caller 코드 변경 불필요 |
| 로깅 | 비정상 pkt_len 탐지 시 USR_ERROR 레벨 로그 | Log 호출 확인 |

---

## 4. 분석

### 4.1 Buffer Size Map (Caller → Buffer → Max Safe pkt_len)

| Function | Callers | Buffer Type | Buffer Size | Header Read | Max Safe Recvn |
|----------|---------|-------------|-------------|-------------|----------------|
| `Select_Receive` | pa_1600_tr, pw_3010_tr, pw_3030_tr, pw_4000_ts | `RecvPkt[TCP_BUFF_MAX_LEN]` | 5120 | 5 bytes | 5115 (=5120-5) |
| `Select_Receive2` | (no active callers) | `char *p_recv` (unknown) | unknown | p_len bytes | needs p_buf_size param or TCP_BUFF_MAX_LEN default |
| `Select_Receive_Krx` | pa_3100_ts, pb_1800_ts | `DataBuff[KRX_DATA_BUFF_SIZE]` | 4096 | p_len (82) | 4014 (=4096-82) |
| `Select_Receive_Cli` | pa_8100_ts, pa_8200_tr, pb_7200_tr, pb_8100_ts, pb_8200_tr | `RecvPkt[CLI_BUFF_MAX_LEN]` | 4096 | 4 bytes | 4092 (=4096-4) |
| `Select_Receive_Imeco` | pa_2100_ts, pa_2200_tr | `RecvPkt[TCP_BUFF_MAX_LEN]` | 5120 | 4 bytes | 5116 (=5120-4) |
| `Select_Receive_Imeco_Sise` | pa_2700_tr | `RecvPkt[TCP_BUFF_MAX_LEN]` | 5120 | 10 bytes | 5110 (=5120-10) |

### 4.2 AtoIf() Behavior 분석

```c
int AtoIf (char *p_ascii, int p_len)
```

- 비숫자 문자 → 무시 (skip), 결과값에 영향 없음
- 음수 부호 `-` → `jj * -1` 적용 → **음수 pkt_len 가능**
- 순수 공백/비숫자 → 0 반환
- `"9999"` (4자리) → 9999, `"999999"` (6자리) → 999999 — **상한 없음**
- int overflow 가능 (10자리 이상 숫자열)

### 4.3 Bug Pattern 요약

| Bug | Functions Affected | 위험 |
|-----|--------------------|------|
| pkt_len 상한 검증 없음 → buffer overflow | 전 6개 | 중요 |
| 하한 검사가 Log만, return 없음 | Select_Receive, _Cli, _Imeco, _Imeco_Sise | 높음 |
| 음수 pkt_len → Recvn에 음수 전달 | 전 6개 (AtoIf 특성) | 중간 |
| pkt_len - p_len 음수 가능 | Select_Receive2 | 중간 |

---

## 5. Fix Strategy

### 5.1 Pattern: Upper-bound + Lower-bound Guard

각 함수에서 `Recvn()` 호출 직전에 다음 패턴 적용:

```c
/* 기존 (log-only, no return) */
if (pkt_len < MIN_THRESHOLD)
    Log (USR_ERROR, "...:invalid Length[%d]", pkt_len);

rt = Recvn (p_sfd, p_recv+offset, pkt_len);

/* 수정 후 */
if (pkt_len < MIN_THRESHOLD || pkt_len > MAX_SAFE_LEN)
{
    Log (USR_ERROR, "...:invalid Length[%d] (expected %d~%d)",
        pkt_len, MIN_THRESHOLD, MAX_SAFE_LEN);
    return (NOTOK);
}

rt = Recvn (p_sfd, p_recv+offset, pkt_len);
```

### 5.2 Per-Function Fix Details

| Function | offset | MIN | MAX (= BUF_SIZE - offset) | Notes |
|----------|--------|-----|---------------------------|-------|
| `Select_Receive` | 5 | TCP_HEAD_LEN-5 (=75) | TCP_BUFF_MAX_LEN-5 (=5115) | 기존 하한 조건 재활용, return 추가 |
| `Select_Receive2` | p_len | p_len+1 | TCP_BUFF_MAX_LEN | 시그니처 변경 없이 TCP_BUFF_MAX_LEN 사용 |
| `Select_Receive_Krx` | p_len | 1 (already checked >0) | KRX_DATA_BUFF_SIZE-p_len | 상한만 추가 |
| `Select_Receive_Cli` | 4 | CLI_HEAD_LEN-4 (=46) | CLI_BUFF_MAX_LEN-4 (=4092) | 기존 하한 조건 재활용, return 추가 |
| `Select_Receive_Imeco` | 4 | 16 (=20-4) | TCP_BUFF_MAX_LEN-4 (=5116) | 기존 하한 조건 재활용, return 추가 |
| `Select_Receive_Imeco_Sise` | 10 | 40 (=50-10) | TCP_BUFF_MAX_LEN-10 (=5110) | 기존 하한 조건 재활용, return 추가 |

---

## 6. 성공 기준

### 6.1 완료 정의

- [ ] 6개 함수 모두 Recvn() 호출 전 상한+하한 검증 존재
- [ ] 비정상 pkt_len 시 Log + return NOTOK (Recvn 진입 차단)
- [ ] 함수 시그니처 변경 없음 (caller 호환성 유지)
- [ ] 정상 범위 패킷에 대한 동작 불변
- [ ] `mk.sh sub` 빌드 성공

### 6.2 Quality 기준

- [ ] Zero buffer overflow 가능성 (코드 리뷰)
- [ ] Zero functional change (정상 패킷 경로)
- [ ] `Sise_Select_Receive`는 수정하지 않음 (이미 안전)

---

## 7. 위험 및 완화

| 위험 | 영향 | 확률 | 완화 |
|------|--------|------------|------------|
| 정상 패킷의 pkt_len이 MAX 초과 | 오탐 연결 끊김 | 낮음 | MAX를 버퍼 크기 기반으로 설정, 프로토콜 최대보다 넉넉 |
| Select_Receive2 caller 추가 시 버퍼 크기 불일치 | 오버플로 잔존 | 낮음 | TCP_BUFF_MAX_LEN 기본값 사용, 향후 buf_size 파라미터 추가 고려 |
| Recvn에 0 전달 시 hang | 프로세스 블록 | 낮음 | 하한 검사에서 0 이하 차단 |

---

## 8. 파일 영향

| 파일 | 변경사항 | Lines |
|------|---------|-------|
| `st01/sub/select_recv.c` | 6개 함수에 bounds check 추가 | ~+30 |

---

## 9. 다음 단계

1. [ ] Write design document (`select-recv-bounds-check.design.md`)
2. [ ] Implement bounds checks
3. [ ] Gap analysis

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
