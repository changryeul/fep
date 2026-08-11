# TR Struct Refactor Design Document

> **요약**: KRX 거래소 TR 전문의 하드코딩된 바이트 오프셋 접근을 구조체 기반 접근으로 전환
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: System Architect
> **Date**: 2026-02-18
> **Status**: Draft

---

## 1. 개요

### 1.1 설계 목표

- KRX TR 전문 파싱/조립 시 하드코딩된 바이트 오프셋(`DataBuff[82]`, `DataBuff[KRX_HEAD_LEN+11]`)을 구조체 필드 접근으로 전환
- KRX 전문규격 변경 시 수정 범위를 `inc/*.h` 1곳으로 한정
- 기존 코드의 동작을 100% 보장하면서 점진적으로 적용

### 1.2 설계 원칙

- **Zero Runtime Overhead**: 구조체 포인터 캐스팅만 사용, 메모리 복사 없음
- **Backward Compatibility**: 기존 `DataBuff[]` 배열과 병행 사용 가능
- **Incremental Migration**: 파일 단위로 순차 적용, 빅뱅 전환 불필요
- **Single Source of Truth**: TR 필드 레이아웃은 `pa_struct.h` / `pb_struct.h` 구조체 정의 1곳에서만 관리

### 1.3 현재 문제 분석

#### 1.3.1 오프셋 하드코딩 패턴 (총 40+ 건)

| 파일 | 패턴 | 빈도 |
|------|------|------|
| `pb_1200_tr.c` | `DataBuff[KRX_HEAD_LEN+11]`, `DataBuff[82]` | 15+ |
| `pa_1200_tr.c` | `DataBuff[KRX_HEAD_LEN+11]`, `DataBuff[KRX_HEAD_LEN+11+11]` | 12+ |
| `pb_7800_tr.c` | `DataBuff[KRX_HEAD_LEN+11]`, `DataBuff[KRX_HEAD_LEN-1]` | 10+ |
| `pa_7800_tr.c` | `DataBuff[KRX_HEAD_LEN+11]`, `DataBuff[KRX_HEAD_LEN+(d_size*i)]` | 8+ |
| `pb_1800_ts.c` | `DataBuff[KRX_HEAD_LEN]` | 5+ |
| `pa_1100_ts.c` | `DataBuff[KRX_HEAD_LEN]` | 5+ |

#### 1.3.2 이미 존재하는 구조체 (미활용)

`pa_struct.h`에 이미 정의되어 있으나 소스 코드에서 활용되지 않는 구조체:

| 구조체 | 용도 | 라인 |
|--------|------|------|
| `KRX_HEADER` (82B) | 공통 헤더 | `pa_struct.h:29-44` |
| `KRX_JUMUN_DATA` (주문 Body) | 주문 요청 | `pa_struct.h:59-454` |
| `KRX_SETTLE_DATA` (체결 Body) | 체결 결과 | `pa_struct.h:488-522` |
| `KRX_SETTLE_RESP_DATA` (체결응답 Body) | 체결 응답 | `pa_struct.h:531-581` |
| `KRX_JUMUN_R_DATA` (주문응답) | 응답/거부 | `pa_struct.h:463-468` |
| `KRX_NOTE_*` 시리즈 | 채권 TR | `pa_struct.h:621-900` |

---

## 2. 아키텍처

### 2.1 레이어 구조

```
┌─────────────────────────────────────────────────────┐
│  Application Layer (pa_*.c, pb_*.c)                 │
│  - Socket_Event_Rtn(), DataRecv_Rtn() 등            │
│  - 구조체 포인터로 필드 접근                         │
├─────────────────────────────────────────────────────┤
│  Protocol Layer (신규 - krx_trcode.h)               │
│  - TR코드 상수 (#define TR_BOND_EXECUTION ...)      │
│  - 응답코드 상수 (#define RESP_SUCCESS "0000")      │
│  - 비교 매크로 (IS_TR, IS_RESP_OK)                  │
├─────────────────────────────────────────────────────┤
│  Structure Layer (pa_struct.h, pb_struct.h)          │
│  - KRX_HEADER (82B, 기존)                           │
│  - KRX_BODY_COMMON (24B, 신규)                      │
│  - KRX_MSG_COMMON (106B, 신규)                      │
│  - TR별 Body 구조체 (기존 + 보완)                    │
├─────────────────────────────────────────────────────┤
│  Buffer Layer (DataBuff[])                           │
│  - 기존 raw byte buffer (변경 없음)                  │
│  - 구조체는 overlay(포인터 캐스팅)로 접근            │
└─────────────────────────────────────────────────────┘
```

### 2.2 데이터 흐름

```
[KRX TCP수신] → DataBuff[] → (KRX_MSG_COMMON*)DataBuff
                                  ↓
                          Body.Transaction_Code 확인
                                  ↓
                    ┌─────────────┼──────────────┐
                    ↓             ↓              ↓
            KRX_MSG_EXECUTION  KRX_MSG_ORDER_RESP  KRX_MSG_MARKET_OPR
            (체결결과)          (호가응답)           (장운영)
                    ↓             ↓              ↓
              필드별 접근     필드별 접근      필드별 접근
              (exec->Body.   (resp->Body.    (opr->Body.
               ConsPrice)     OrderNo)        MarketStatus)
```

---

## 3. 데이터 모델 (구조체 설계)

### 3.1 신규 추가: Body 공통 구조체

```c
/* pa_struct.h 또는 별도 krx_msg.h에 추가 */

/* 주문/체결/장운영 Body 공통 앞부분 (24 bytes)
   거의 모든 주문채결 TR이 이 3개 필드로 시작 */
typedef struct {
    char    DataSeq[11];            /* 데이터 일련번호           */
    char    Transaction_Code[11];   /* TR코드 (TTRTDP42301 등)  */
    char    Megrp_no[2];            /* ME그룹번호                */
} KRX_BODY_COMMON;

/* Header + Body 공통부 통합 (106 bytes) */
typedef struct {
    KRX_HEADER      Header;         /* 82 bytes */
    KRX_BODY_COMMON Body;           /* 24 bytes */
} KRX_MSG_COMMON;
```

**검증**: `sizeof(KRX_MSG_COMMON)` == 106 (char[] 멤버만이므로 패딩 없음)

### 3.2 신규 추가: TR코드 상수 헤더

**파일**: `st01/inc/krx_trcode.h` (신규 생성)

```c
#ifndef __KRX_TRCODE_H
#define __KRX_TRCODE_H

/* ================================================================
   KRX TR Code Constants
   - KRX 전문규격서 기준 TR코드를 상수로 정의
   - 매직 문자열 대신 상수명으로 의미 전달
   ================================================================ */

/* TR코드 필드 길이 */
#define KRX_TRCODE_LEN          11
#define KRX_DATASEQ_LEN         11
#define KRX_MEGRPNO_LEN         2
#define KRX_ERRCODE_LEN         4

/* ---- 채권(Bond) TR코드 ---- */
#define TR_BOND_EXECUTION       "TTRTDP42301"   /* 채권 체결결과(일반,조성) 317B */
#define TR_BOND_ORDER_RESP      "TTRODP4130"    /* 채권 일반처리호가 291B (prefix 10자) */
#define TR_BOND_MM_RESP         "TTRMOP4130"    /* 채권 조성처리호가 295B (prefix 10자) */
#define TR_KILL_SWITCH          "TTRKOP1130"    /* Kill Switch 응답 161B (prefix 10자) */

/* ---- 체결 인터페이스 종료 ---- */
#define TR_IF_END_SETTLE        "TCHEDP99000"   /* 체결 인터페이스 종료 153B */
#define TR_IF_END_MARKET        "TCHEDP99001"   /* 장운영 인터페이스 종료 153B */

/* ---- 장운영 ---- */
#define TR_MARKET_OPR_PREFIX    "TTRMIP3"       /* 공개장운영 prefix (7자) */
#define TR_MARKET_OPR_FULL      "TTRMIP31301"   /* 공개장운영 91B */

/* ---- 시세/종목정보 ---- */
#define TR_RDS01_BOND_ITEM      "TRDESP50101"   /* 채권종목정보 (RDS01) */
#define TR_RDS02_KTS_ITEM       "TRDESP50102"   /* KTS종목정보 (RDS02) */

/* ---- 주문 요청 ---- */
#define TR_ORDER_NEW            "TCHODR10001"   /* 신규호가 */
#define TR_ORDER_MODIFY         "TCHODR10002"   /* 정정호가 */
#define TR_ORDER_CANCEL         "TCHODR10003"   /* 취소호가 */

/* ---- 응답코드 ---- */
#define RESP_SUCCESS            "0000"          /* 정상 */
#define RESP_TPS_EXCEEDED       "0020"          /* TPS 한도 초과 */

/* ================================================================
   TR코드 비교 매크로
   - memcmp 직접 호출 대신 의미있는 이름으로 비교
   ================================================================ */

/* 전체 길이(11자) 비교 */
#define IS_TR(msg, code) \
    (memcmp((msg)->Body.Transaction_Code, (code), KRX_TRCODE_LEN) == 0)

/* prefix 비교 (10자 이하 TR코드용) */
#define IS_TR_PREFIX(msg, code) \
    (memcmp((msg)->Body.Transaction_Code, (code), strlen(code)) == 0)

/* 응답코드 비교 (Header 다음 4바이트) */
#define IS_RESP_OK(buf) \
    (memcmp(&(buf)[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) == 0)

/* Encrypt 필드 비교 */
#define IS_ENCRYPTED(hdr) \
    ((hdr)->Encrypt[0] == 'Y')

#endif /* __KRX_TRCODE_H */
```

### 3.3 기존 구조체 활용 매핑

현재 `pa_struct.h`에 이미 정의된 구조체와 소스코드 매핑:

| 수신 TR | 기존 구조체 | 소스 파일 | 현재 사용 여부 |
|---------|------------|----------|--------------|
| 체결결과 (TTRTDP42301) | `KRX_SETTLE_DATA` (pa_struct.h:488) | pb_1200_tr.c, pa_1200_tr.c | **미사용** (오프셋 접근) |
| 호가응답 (TTRODP4130x) | `KRX_SETTLE_RESP_DATA` (pa_struct.h:531) | pb_1200_tr.c, pa_1200_tr.c | **미사용** (오프셋 접근) |
| 주문응답 (S_Fmt) | `KRX_JUMUN_R_FMT` (pa_struct.h:470) | pa_1100_ts.c, pb_1800_ts.c | **부분 사용** (Header만) |
| 장운영 (TTRMIP3xxxx) | 없음 | pb_1200_tr.c, pa_1200_tr.c | N/A |
| 종목정보 (TRDESP50101) | 없음 | pb_7800_tr.c, pa_7800_tr.c | N/A |

**채권(PB) 전용 구조체** (`pa_struct.h:621-900`):

| 구조체 | 용도 |
|--------|------|
| `KRX_NOTE_JUMUN_DATA` | 채권 주문 Body |
| `KRX_NOTE_JUMUN_R_DATA` | 채권 주문응답 Body |
| `KRX_NOTE_SETTLE_DATA` | 채권 체결 Body |
| `KRX_NOTE_SETTLE_RESP_DATA` | 채권 체결응답 Body |

---

## 4. 변환 규칙 (Transformation Rules)

### 4.1 수신 전문 파싱 변환

#### Rule 1: 공통부 접근 (DataSeq, TrCode, Megrp_no)

```c
/* Before */
r_meg_no  = AtoIf(&DataBuff[KRX_HEAD_LEN+11+11], 2);
r_meg_seq = AtoIf(&DataBuff[KRX_HEAD_LEN], 11);
if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRTDP42301", 11) == 0)

/* After */
KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
r_meg_no  = AtoIf(msg->Body.Megrp_no, KRX_MEGRPNO_LEN);
r_meg_seq = AtoIf(msg->Body.DataSeq, KRX_DATASEQ_LEN);
if (IS_TR(msg, TR_BOND_EXECUTION))
```

#### Rule 2: 헤더 Encrypt 필드 접근

```c
/* Before */
if (memcmp(&DataBuff[KRX_HEAD_LEN-1], "Y", 1) == 0)

/* After */
KRX_HEADER *hdr = (KRX_HEADER *)DataBuff;
if (IS_ENCRYPTED(hdr))
```

#### Rule 3: DecryptBody 호출

```c
/* Before */
DecryptBody(&DataBuff[82], body_len);

/* After */
DecryptBody(&DataBuff[KRX_HEAD_LEN], body_len);
```

(참고: `DataBuff[82]`는 이미 `KRX_HEAD_LEN`으로 대체 가능하나, 일부 파일에서 리터럴 82를 직접 사용 중)

#### Rule 4: 응답코드 비교

```c
/* Before */
if (memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0)
ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], 4);

/* After */
if (IS_RESP_OK(DataBuff))
ErrCd = AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
```

#### Rule 5: TR별 Body 구조체 캐스팅

```c
/* Before */
if (memcmp(&DataBuff[KRX_HEAD_LEN+11], "TTRTDP42301", 11) == 0) {
    // DataBuff[KRX_HEAD_LEN+11+11+2+2+5+5+10+10+12] 식으로 접근
}

/* After */
KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
if (IS_TR(msg, TR_BOND_EXECUTION)) {
    KRX_SETTLE_DATA *settle = (KRX_SETTLE_DATA *)&DataBuff[KRX_HEAD_LEN];
    // settle->ItemCode, settle->ConsPrice 등 필드명으로 접근
}
```

### 4.2 송신 전문 조립 변환

#### Rule 6: 헤더 템플릿 → 구조체 초기화

```c
/* Before (pa_5010_mp.c) */
char Order_Ohd[83] = {"KMAPv1.0000300TCHODR00000           00021   ..."};
memcpy(DataBuff, Order_Ohd, 83);

/* After */
KRX_HEADER hdr;
memset(&hdr, ' ', sizeof(hdr));
memcpy(hdr.BeginString,     "KMAPv1.0",    8);
memcpy(hdr.BodyLength,      "000300",       6);
memcpy(hdr.MsgType,         "TCHODR00000", 11);
memcpy(hdr.SenderCompID,    "00021",        5);
memcpy(hdr.DataCnt,         "001",          3);
hdr.Encrypt[0] = ' ';
memcpy(DataBuff, &hdr, KRX_HEAD_LEN);
```

#### Rule 7: Body 템플릿 → 구조체 초기화

```c
/* Before (pa_5010_mp.c) */
char O_Order_Dat[301] = {"00000000000TCHODR1000110002100999..."};
memcpy(&DataBuff[82], &O_Order_Dat[11], 290);
memcpy(&DataBuff[82+22+2+5+5+10+10], item_code, 12);  // 종목코드

/* After */
KRX_JUMUN_DATA order;
memset(&order, ' ', sizeof(order));
memcpy(order.Transaction_Code, TR_ORDER_NEW, KRX_TRCODE_LEN);
memcpy(order.Megrp_no,        "00",         KRX_MEGRPNO_LEN);
memcpy(order.Board_Id,        "21",         2);
memcpy(order.ItemCode,        item_code,    12);
/* ... 나머지 필드도 이름으로 접근 ... */
memcpy(&DataBuff[KRX_HEAD_LEN], &order, sizeof(order));
```

---

## 5. 영향받는 파일 목록

### 5.1 헤더 파일 (수정/신규)

| 파일 | 작업 | 내용 |
|------|------|------|
| `inc/krx_trcode.h` | **신규** | TR코드 상수, 응답코드 상수, 비교 매크로 |
| `inc/pa_struct.h` | **수정** | `KRX_BODY_COMMON`, `KRX_MSG_COMMON` 추가 |

### 5.2 소스 파일 (단계별 적용)

| 우선순위 | 파일 | 변환 대상 건수 | 비고 |
|---------|------|--------------|------|
| **1 (시범)** | `src/PB/pb_1200_tr.c` | ~15건 | 채권 체결/응답 수신 |
| **2** | `src/PA/pa_1200_tr.c` | ~12건 | 선물옵션 체결/응답 수신 |
| **3** | `src/PB/pb_7800_tr.c` | ~10건 | 채권 종목정보/세션 |
| **4** | `src/PA/pa_7800_tr.c` | ~8건 | 선물옵션 종목정보/세션 |
| **5** | `src/PB/pb_1800_ts.c` | ~5건 | 채권 주문 송신 |
| **6** | `src/PA/pa_1100_ts.c` | ~5건 | 선물옵션 주문 송신 |
| **7** | `src/PA/pa_5010_mp.c` | 템플릿 전체 | 자동매매 주문 조립 |

---

## 6. Safety Validation

### 6.1 컴파일 타임 검증

각 구조체 정의 후 `sizeof()` 검증을 추가하여 KRX 규격서와 바이트 단위로 일치하는지 확인:

```c
/* pa_struct.h 또는 krx_trcode.h 하단에 추가 */

/* Compile-time size validation */
#define STATIC_ASSERT(cond, msg) \
    typedef char static_assert_##msg[(cond) ? 1 : -1]

STATIC_ASSERT(sizeof(KRX_HEADER) == 82,        KRX_HEADER_size_mismatch);
STATIC_ASSERT(sizeof(KRX_BODY_COMMON) == 24,   KRX_BODY_COMMON_size_mismatch);
STATIC_ASSERT(sizeof(KRX_MSG_COMMON) == 106,   KRX_MSG_COMMON_size_mismatch);
/* 각 TR별 Body도 동일하게 검증 */
```

### 6.2 런타임 영향 없음 확인

- 구조체 포인터 캐스팅은 어셈블리 수준에서 동일한 코드 생성
- 모든 멤버가 `char[]`이므로 alignment padding 없음
- `memcmp`, `AtoIf` 등 기존 함수의 인자가 동일한 메모리 주소를 가리킴

### 6.3 회귀 테스트 방법

```
1. 변환 전: pb_1200_tr.c가 처리하는 체결/응답 전문 로그 캡처
2. 변환 후: 동일 전문 입력 시 동일한 로그 출력 확인
3. 비교 기준:
   - Log() 함수 출력 내용 동일
   - SHM에 기록되는 데이터 동일
   - FIFO로 전달되는 데이터 동일
```

---

## 7. Implementation Order

### 7.1 Phase 1: 인프라 (헤더 파일 추가)

- [ ] `inc/krx_trcode.h` 신규 생성 (TR코드 상수 + 매크로)
- [ ] `inc/pa_struct.h`에 `KRX_BODY_COMMON`, `KRX_MSG_COMMON` 추가
- [ ] `STATIC_ASSERT` 사이즈 검증 추가
- [ ] 전체 빌드 확인 (기존 코드 변경 없이 컴파일 통과)

### 7.2 Phase 2: 시범 적용 (pb_1200_tr.c)

- [ ] `#include "krx_trcode.h"` 추가
- [ ] `DataBuff[82]` → `DataBuff[KRX_HEAD_LEN]` 전환
- [ ] `memcmp(&DataBuff[KRX_HEAD_LEN+11], ...)` → `IS_TR(msg, TR_*)` 전환
- [ ] `AtoIf(&DataBuff[KRX_HEAD_LEN+11+11], 2)` → `AtoIf(msg->Body.Megrp_no, ...)` 전환
- [ ] 빌드 및 테스트

### 7.3 Phase 3: 확대 적용 (나머지 수신 프로세스)

- [ ] `pa_1200_tr.c` 적용
- [ ] `pb_7800_tr.c` 적용
- [ ] `pa_7800_tr.c` 적용

### 7.4 Phase 4: 송신 프로세스 적용

- [ ] `pb_1800_ts.c` 적용
- [ ] `pa_1100_ts.c` 적용
- [ ] `pa_5010_mp.c` 템플릿 문자열 → 구조체 초기화 전환

---

## 8. Naming Convention

기존 코드베이스의 명명 규칙을 따름:

| 대상 | 규칙 | 예시 |
|------|------|------|
| 구조체 타입 | UPPER_CASE | `KRX_BODY_COMMON`, `KRX_MSG_COMMON` |
| TR코드 상수 | `TR_` prefix | `TR_BOND_EXECUTION`, `TR_KILL_SWITCH` |
| 응답코드 상수 | `RESP_` prefix | `RESP_SUCCESS`, `RESP_TPS_EXCEEDED` |
| 매크로 | UPPER_CASE | `IS_TR()`, `IS_RESP_OK()`, `IS_ENCRYPTED()` |
| 길이 상수 | `KRX_*_LEN` | `KRX_TRCODE_LEN`, `KRX_ERRCODE_LEN` |
| 로컬 포인터 변수 | 소문자 | `msg`, `exec`, `resp`, `hdr` |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-18 | Initial draft | System Architect |
