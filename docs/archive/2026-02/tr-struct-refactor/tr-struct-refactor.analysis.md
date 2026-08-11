# 갭 분석: tr-struct-refactor (반복 1)

> **요약**: KRX TR struct 리팩토링 -- Act-1 수정 후 설계 대비 구현 갭 재분석
>
> **작성자**: Gap Detector Agent
> **날짜**: 2026-02-18
> **설계**: docs/02-design/features/tr-struct-refactor.design.md
> **상태**: 완료 (반복 1 재검증)

---

## 일치율: 89% (이전: 72%)

## 전체 점수

| 카테고리 | 이전 | 현재 | 상태 |
|----------|:--------:|:-------:|:------:|
| 단계 1: 헤더 인프라 | 100% | 100% | PASS |
| 단계 2: 파일럿 (pb_1200_tr.c) | 90% | 95% | PASS |
| 단계 3: 수신 확장 (pa_1200, pb_7800, pa_7800) | 68% | 90% | PASS |
| 단계 4: 전송 과정 (pb_1800, pa_1100) | 40% | 70% | PARTIAL |
| **전체** | **72%** | **89%** | PASS (목표 근처) |

---

## Act-1 수정 검증

### 수정 1: pb_7800_tr.c `ttt` 접두사 제거 (줄 404-405)

**이전**: 줄 404-405는 `ttt{` 및 `tttKRX_MSG_COMMON *msg` — 컴파일 차단 구문 오류.

**현재** (줄 405-406):
```c
			{			/* TR 분기 */
			KRX_MSG_COMMON *msg = (KRX_MSG_COMMON *)DataBuff;
```

**판정**: 수정됨. `ttt` 접두사가 완전히 제거되었습니다. `msg` 선언 및 블록 범위 중괄호가 올바른 구문의 별도 줄에 있습니다.

### 수정 2: pb_7800_tr.c `DataBuff[82]` 제거 (2개 발생)

**이전**: 줄 1076, 1242는 INL_Decrypt 호출에서 `&DataBuff[82]`.

**현재** (줄 1078, 1244):
```c
result = INL_Decrypt(EnCtx,
                     (unsigned char*)&DataBuff[KRX_HEAD_LEN],
```

**판정**: 수정됨. 두 발생 모두 이제 `&DataBuff[KRX_HEAD_LEN]` 사용. Grep은 모든 대상 파일에서 `DataBuff[82]` 없음 확인.

### 수정 3: pb_7800_tr.c TR_DATA case `memcmp` 변환

**이전**: 줄 354, 364는 `memcmp(&DataBuff[14], "TRDESP501xx", 11)` — 하드코딩 오프셋 14 및 리터럴 문자열.

**현재** (줄 355, 365):
```c
if (memcmp(Header_Fmt.MsgType, TR_RDS01_BOND_ITEM, KRX_TRCODE_LEN) == 0)
...
else if (memcmp(Header_Fmt.MsgType, TR_RDS02_KTS_ITEM, KRX_TRCODE_LEN) == 0)
```

**판정**: 수정됨. 변환은 `Header_Fmt.MsgType` (struct 레벨 액세스)과 `TR_RDS01_BOND_ITEM` / `TR_RDS02_KTS_ITEM` 상수 및 `KRX_TRCODE_LEN` 사용. 이전 하드코딩 줄은 주석으로 위에 보존 (줄 354, 364). pb_7800_tr.c에 `memcmp(&DataBuff[14]...)`는 남아있지 않음.

### 수정 4: pb_7800_tr.c `#include` 연결 분할

**이전**: 줄 14는 `#include "krx_trcode.h"#include "ifaddrs.h"`가 한 줄에 있음.

**현재** (줄 14-15):
```c
#include    "krx_trcode.h"
#include    "ifaddrs.h"
```

**판정**: 수정됨. 적절한 별도 줄 서식.

### 수정 5: pb_7800_tr.c 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN`

**이전**: 줄 590, 638은 `memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4)`.

**현재** (줄 592, 640):
```c
memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
```

**판정**: 수정됨. 두 핸드셰이크 비교 모두 이제 `RESP_SUCCESS` 및 `KRX_ERRCODE_LEN` 사용. 또한 `sizeof(KRX_HEADER)` 대신 `KRX_HEAD_LEN` 사용.

### 수정 6: pb_1800_ts.c `#include` 연결 분할

**이전**: 줄 15는 연결된 `#include` 줄.

**현재** (줄 15-16):
```c
#include    "krx_trcode.h"
#include    "ifaddrs.h"
```

**판정**: 수정됨.

### 수정 7: pb_1800_ts.c 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN`

**이전**: 줄 661, 709는 `memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4)`.

**현재** (줄 662, 710):
```c
memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
```

**판정**: 수정됨. 두 핸드셰이크 비교 변환.

### 수정 8: pb_1200_tr.c 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN`

**이전**: 줄 718, 766은 `memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4)`.

**현재** (줄 718, 766):
```c
memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
```

**판정**: 수정됨. 두 핸드셰이크 비교 변환.

### 수정 9: pa_1100_ts.c `#include` 연결 분할

**이전**: 줄 15는 연결된 `#include` 줄.

**현재** (줄 15-16):
```c
#include    "krx_trcode.h"
#include    "ifaddrs.h"
```

**판정**: 수정됨.

### 수정 10: pa_1100_ts.c `memcmp(DataBuff+14, "TCHODR00000", 11)` 변환

**이전**: 줄 814는 `memcmp(DataBuff+14, "TCHODR00000", 11)` — 하드코딩 오프셋 및 리터럴 문자열.

**현재** (줄 814):
```c
if (memcmp (((KRX_HEADER *)DataBuff)->MsgType, TR_SESSION_DATA, KRX_TRCODE_LEN) == 0)
```

**판정**: 수정됨. 필드 액세스를 위해 `KRX_HEADER` struct 캐스트 및 `TR_SESSION_DATA` 상수와 `KRX_TRCODE_LEN` 사용.

### 수정 11: pa_1100_ts.c `memcmp(S_Fmt.Data, "0000", 4)` 변환

**이전**: 줄 659는 `memcmp(S_Fmt.Data, "0000", 4)`.

**현재** (줄 659):
```c
if (memcmp(S_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN) == 0)
```

**판정**: 수정됨. `RESP_SUCCESS` 상수 및 `KRX_ERRCODE_LEN` 사용.

### 수정 12: pa_1100_ts.c `memcpy "TCHODR00000"` 변환

**이전**: 줄 1175는 `memcpy(J_Q_Fmt.Header.MsgType, "TCHODR00000", 11)`.

**현재** (줄 1175):
```c
memcpy (J_Q_Fmt.Header.MsgType, 	TR_SESSION_DATA, 	KRX_TRCODE_LEN);
```

**판정**: 수정됨. `TR_SESSION_DATA` 상수 및 `KRX_TRCODE_LEN` 사용.

---

## 남은 갭

### 경미 (낮은 영향 -- struct 레벨 코드 또는 데드 코드)

| # | 파일 | 줄 | 이슈 | 영향 | 비고 |
|---|------|------|-------|--------|------|
| 1 | `st01/src/PB/pb_7800_tr.c` | 790 | `memcmp(KR_Fmt.Data, "0000", 4)` TR_LINK 응답 | LOW | 이미 struct 레벨 액세스 (`KR_Fmt.Data`), DataBuff 오프셋 아님. 일관성을 위해 `RESP_SUCCESS` 사용 가능. |
| 2 | `st01/src/PB/pb_7800_tr.c` | 883 | `memcmp(J_Q_Fmt.Header.MsgType, "TCHODR00000", 11)` | NONE | `#if 0` 데드 코드 블록 내부 |
| 3 | `st01/src/PB/pb_7800_tr.c` | 1027-1028 | Analyze_Data()의 `memcmp(Header_Fmt.MsgType, "TRDESP50101/2", 11)` | LOW | 이미 struct 레벨 액세스. 일관성을 위해 TR_RDS01/RDS02 상수 사용 가능. |
| 4 | `st01/src/PB/pb_1800_ts.c` | 478 | `memcmp(S_Fmt.Data, "0000", 4)` | LOW | Struct 레벨 액세스. `RESP_SUCCESS` 사용 가능. |
| 5 | `st01/src/PB/pb_1800_ts.c` | 857 | `memcmp(S_Fmt.Data, "0000", 4)` | LOW | Struct 레벨 액세스. `RESP_SUCCESS` 사용 가능. |
| 6 | `st01/src/PB/pb_1800_ts.c` | 971 | `memcmp(J_Q_Fmt.Header.MsgType, "TCHODR00000", 11)` | NONE | `#if 0` 데드 코드 블록 내부 |
| 7 | `st01/src/PA/pa_1100_ts.c` | 759 | `memcmp(J_Q_Fmt.Header.MsgType, "TCHODR00000", 11)` | NONE | `#if 0` 데드 코드 블록 내부 |
| 8 | `st01/src/PA/pa_1100_ts.c` | 965 | Analyze_Data()의 활성 코드, struct 레벨 액세스 하지만 리터럴 문자열 `memcmp(S_Fmt.Header.MsgType, "TCHODR00000", 11)` | LOW | `TR_SESSION_DATA` 사용 가능. |

### 정보 (범위 외)

| # | 파일 | 줄 | 비고 |
|---|------|------|------|
| I1 | `st01/src/PB/pb_7800_tr.c` | 1474-1552 | `memcpy(KR_Fmt.Data, "0000", 4)` -- 이들은 응답 생성 (쓰기) 작업, 비교 아님. 리터럴 "0000"은 응답 코드로 쓰여지고 있음, 비교 아님. 설계 규칙 4는 비교 (memcmp)를 대상, 할당 (memcpy) 아님. |
| I2 | `st01/src/PB/pb_1800_ts.c` | 1256-1327 | 동일 -- 응답 생성 memcpy 작업. |
| I3 | `st01/src/PA/pa_1100_ts.c` | 1124-1158 | 동일 -- 응답 생성 memcpy 작업. |
| I4 | `st01/src/PA/pa_1200_tr.c` | 1275-1353 | pa_1200_tr.c의 동일 패턴 |
| I5 | `pa_5010_mp.c` (설계 단계 4, 항목 7) | -- | 현재 8개 파일 범위에 없음. |

---

## 상세 분석

### 단계 1: 헤더 인프라 (변경 없음)

| 요구사항 | 상태 | 세부사항 |
|-------------|:------:|--------|
| `krx_trcode.h` 생성 | PASS | 95줄, 모든 상수 및 매크로 존재 |
| `KRX_BODY_COMMON` (24B) pa_struct.h | PASS | 줄 48-52 |
| `KRX_MSG_COMMON` (106B) pa_struct.h | PASS | 줄 55-58 |
| STATIC_ASSERT 검증 | PASS | 줄 1330-1332 |

**판정**: PASS (100%) -- 이전 분석에서 변경 없음.

### 단계 2: 파일럿 -- pb_1200_tr.c

| 요구사항 | 이전 | 현재 | 세부사항 |
|-------------|:--------:|:-------:|--------|
| `#include "krx_trcode.h"` | PASS | PASS | 줄 15, 적절히 서식됨 |
| `KRX_MSG_COMMON *msg` 블록 범위 | PASS | PASS | 줄 390 |
| 모든 IS_TR / IS_TR_PREFIX 변환 | PASS | PASS | 줄 416-535 |
| IS_RESP_OK(DataBuff) | PASS | PASS | 줄 865, 1409 |
| KRX_ERRCODE_LEN와 ErrCd | PASS | PASS | 줄 871, 1418 |
| 핸드셰이크 `"0000"` -> RESP_SUCCESS | **PARTIAL** | **PASS** | 줄 718, 766은 이제 `RESP_SUCCESS, KRX_ERRCODE_LEN` 사용 |

**판정**: PASS (95% -> 100%). 핸드셰이크 코드는 Act-1에서 수정됨.

### 단계 3: 확장 파일

#### 3.1 pa_1200_tr.c (변경 없음)

**판정**: PASS (95%) -- 이전에서 변경 없음. 세션 `memcmp(KR_Fmt.Data, "0000", 4)`는 struct 레벨 액세스.

#### 3.2 pb_7800_tr.c (주요 개선)

| 요구사항 | 이전 | 현재 | 세부사항 |
|-------------|:--------:|:-------:|--------|
| `#include` 서식 | **FAIL** | **PASS** | 줄 14-15, 적절히 분할 |
| `KRX_MSG_COMMON *msg` 구문 오류 없음 | **FAIL** | **PASS** | 줄 405-406, `ttt` 제거됨 |
| TR_DATA case: struct 기반 TR 체크 | **FAIL** | **PASS** | 줄 355, 365: `Header_Fmt.MsgType, TR_RDS01/RDS02, KRX_TRCODE_LEN` |
| RP_DATA case: IS_TR 매크로 | PASS | PASS | 줄 407, 415 |
| 남은 `DataBuff[82]` 없음 | **FAIL** | **PASS** | 줄 1078, 1244는 이제 `KRX_HEAD_LEN` 사용 |
| 핸드셰이크 `"0000"` -> RESP_SUCCESS | **PARTIAL** | **PASS** | 줄 592, 640은 이제 `RESP_SUCCESS, KRX_ERRCODE_LEN` 사용 |
| IS_RESP_OK 사용 | PASS | PASS | 줄 451, 741, 1609 |

**남은 경미함**: 줄 790 `memcmp(KR_Fmt.Data, "0000", 4)` (struct 레벨), 줄 1027-1028 Analyze_Data()의 `memcmp(Header_Fmt.MsgType, "TRDESP501xx", 11)` (struct 레벨, 상수 사용 가능).

**판정**: FAIL (55%) -> **PASS (88%)**. 모든 중요 및 높은 우선순위 수정 적용. 남은 항목은 일관성을 위해 선택적으로 상수를 사용할 수 있는 struct 레벨 코드.

#### 3.3 pa_7800_tr.c (변경 없음)

**판정**: PASS (95%) -- 이전에서 변경 없음.

### 단계 4: 전송 과정 파일

#### 4.1 pb_1800_ts.c

| 요구사항 | 이전 | 현재 | 세부사항 |
|-------------|:--------:|:-------:|--------|
| `#include` 서식 | **FAIL** | **PASS** | 줄 15-16, 적절히 분할 |
| IS_RESP_OK(DataBuff) | PASS | PASS | 줄 809, 1553 |
| KRX_ERRCODE_LEN와 ErrCd | PASS | PASS | 줄 815, 1562 |
| 핸드셰이크 `"0000"` -> RESP_SUCCESS | **FAIL** | **PASS** | 줄 662, 710은 이제 `RESP_SUCCESS, KRX_ERRCODE_LEN` 사용 |
| 남은 `S_Fmt.Data, "0000"` 비교 | N/A | PARTIAL | 줄 478, 857 -- struct 레벨, RESP_SUCCESS 사용 가능 |

**판정**: PARTIAL (50%) -> **PASS (80%)**. 모든 DataBuff 레벨 이슈 수정. 남은 `S_Fmt.Data, "0000"`은 struct 레벨 액세스.

#### 4.2 pa_1100_ts.c

| 요구사항 | 이전 | 현재 | 세부사항 |
|-------------|:--------:|:-------:|--------|
| `#include` 서식 | **FAIL** | **PASS** | 줄 15-16, 적절히 분할 |
| IS_RESP_OK(DataBuff) | PASS | PASS | 줄 615, 1404 |
| KRX_ERRCODE_LEN와 ErrCd | PASS | PASS | 줄 621, 1413 |
| `DataBuff+14` -> struct 액세스 | **FAIL** | **PASS** | 줄 814: `((KRX_HEADER *)DataBuff)->MsgType, TR_SESSION_DATA, KRX_TRCODE_LEN` |
| `S_Fmt.Data, "0000"` -> RESP_SUCCESS | **PARTIAL** | **PASS** | 줄 659: `RESP_SUCCESS, KRX_ERRCODE_LEN` |
| `memcpy MsgType, "TCHODR00000"` -> TR_SESSION_DATA | **PARTIAL** | **PASS** | 줄 1175: `TR_SESSION_DATA, KRX_TRCODE_LEN` |
| 활성 코드의 남은 `"TCHODR00000"` | N/A | PARTIAL | 줄 965: `memcmp(S_Fmt.Header.MsgType, "TCHODR00000", 11)` -- struct 레벨이지만 리터럴 문자열 |

**판정**: PARTIAL (40%) -> **PASS (85%)**. 주요 개선. 줄 965의 한 개 남은 활성 코드 리터럴 (struct 레벨).

---

## 변환 규칙 요약 (업데이트)

| 규칙 | 설명 | pb_1200 | pa_1200 | pb_7800 | pa_7800 | pb_1800 | pa_1100 |
|------|-------------|:-------:|:-------:|:-------:|:-------:|:-------:|:-------:|
| R1 | DataBuff[82] -> DataBuff[KRX_HEAD_LEN] | PASS | PASS | **PASS** | PASS | PASS | PASS |
| R2 | memcmp TR 코드 -> IS_TR / IS_TR_PREFIX / const | PASS | PASS | **PASS** | PASS | N/A | **PASS** |
| R3 | memcmp RESP -> IS_RESP_OK / RESP_SUCCESS | **PASS** | PASS | **PASS** | PASS | **PASS** | **PASS** |
| R4 | AtoIf 본문 필드 -> msg->Body.field | PASS | PASS | N/A | N/A | N/A | N/A |
| R5 | 암호화 확인 -> IS_ENCRYPTED | N/A | N/A | PASS | N/A | N/A | N/A |
| R6 | 하드코딩 4 -> KRX_ERRCODE_LEN | PASS | PASS | PASS | PASS | PASS | PASS |
| R7 | KRX_MSG_COMMON *msg 블록 범위 | PASS | PASS | **PASS** | PASS | N/A | N/A |

(굵게 = 이전 분석에서 변경됨)

---

## 점수 분해

### 상세 요구사항 점수 (18개 요구사항, 이전과 동일)

| # | 요구사항 | 이전 | 현재 | 가중치 | 변경 |
|---|-------------|:--------:|:-------:|:------:|:------:|
| 1 | krx_trcode.h 생성 모든 TR 코드 상수 (13+) | PASS | PASS | 1 | -- |
| 2 | 비교 매크로: IS_TR, IS_TR_PREFIX, IS_RESP_OK, IS_ENCRYPTED | PASS | PASS | 1 | -- |
| 3 | 길이 상수: KRX_TRCODE_LEN, KRX_DATASEQ_LEN, KRX_MEGRPNO_LEN, KRX_ERRCODE_LEN | PASS | PASS | 1 | -- |
| 4 | STATIC_ASSERT 매크로 정의 | PASS | PASS | 1 | -- |
| 5 | KRX_BODY_COMMON (24B) pa_struct.h | PASS | PASS | 1 | -- |
| 6 | KRX_MSG_COMMON (106B) pa_struct.h | PASS | PASS | 1 | -- |
| 7 | pa_struct.h STATIC_ASSERT 검증 | PASS | PASS | 1 | -- |
| 8 | pb_1200_tr.c: 모든 규칙 적용 (~17 변환) | PASS | PASS | 1 | -- |
| 9 | pa_1200_tr.c: 모든 규칙 적용 (~14 변환) | PASS | PASS | 1 | -- |
| 10 | pb_7800_tr.c: 모든 규칙 적용 (~10 변환) | **FAIL** | **PASS** | 1 | 수정됨 |
| 11 | pa_7800_tr.c: 모든 규칙 적용 (~5 변환) | PASS | PASS | 1 | -- |
| 12 | pb_1800_ts.c: 모든 규칙 적용 (~5 변환) | PARTIAL | **PASS** | 1 | 개선됨 |
| 13 | pa_1100_ts.c: 모든 규칙 적용 (~5 변환) | PARTIAL | **PASS** | 1 | 개선됨 |
| 14 | 대상 파일의 남은 DataBuff[82] 없음 | **FAIL** | **PASS** | 1 | 수정됨 |
| 15 | 대상 파일의 남은 하드코딩 TR memcmp 없음 | **FAIL** | **PARTIAL** | 1 | 개선됨 (struct 레벨 "TCHODR00000" pa_1100_ts.c:965, pb_7800_tr.c:1027-1028에서 남음) |
| 16 | C89를 위한 블록 범위 KRX_MSG_COMMON *msg | PARTIAL | **PASS** | 1 | 수정됨 (pb_7800_tr.c 구문 수정) |
| 17 | 변환 코드의 구문 오류 없음 | **FAIL** | **PASS** | 1 | 수정됨 (ttt 제거됨) |
| 18 | #include 서식 올바름 | PARTIAL | **PASS** | 1 | 수정됨 (모든 3개 파일) |

### 계산

- PASS: 16개 항목 = 16.0
- PARTIAL: 1개 항목 (#15) = 0.5
- FAIL: 0개 항목 = 0.0

**일치율 = (16.0 + 0.5) / 18 = 16.5 / 18 = 91.7%**

남은 struct 레벨 리터럴 문자열 (남은 갭에 나열된 항목)의 설명을 고려하여 보수적으로 반올림:

**최종 일치율: 89%**

보수적 반올림 근거: 공식 18 요구사항 점수는 91.7%을 산출하지만, struct 레벨 액세스 (DataBuff 오프셋 위반 아님)인 3개 활성 코드 위치에서 리터럴 문자열을 사용하고 있습니다 (`pb_7800_tr.c:790,1027-1028`, `pb_1800_ts.c:478,857`, `pa_1100_ts.c:965`). 따라서 중요하지 않지만 완전하지 않은 일관성을 나타냅니다. 89% 수치는 이러한 실질적 평가를 반영합니다.

---

## 비교: 이전 vs 현재

| 메트릭 | 이전 (v1.0) | 현재 (v1.1) | 델타 |
|--------|:----------------:|:---------------:|:-----:|
| 일치율 | 72% | 89% | +17% |
| PASS 항목 | 10 | 16 | +6 |
| PARTIAL 항목 | 3 | 1 | -2 |
| FAIL 항목 | 5 | 0 | -5 |
| 중요 버그 | 1 (ttt 구문) | 0 | -1 |
| 남은 DataBuff[82] | 2 | 0 | -2 |
| 하드코딩 memcmp (DataBuff 오프셋) | 3 | 0 | -3 |
| #include 서식 이슈 | 3 | 0 | -3 |
| 남은 핸드셰이크 "0000" | 6 | 0 | -6 |

### Act-1 수정 요약

| 수정 # | 설명 | 파일 | 검증됨 |
|-------|-------------|------|:--------:|
| 1 | `ttt` 접두사 제거 | pb_7800_tr.c:405-406 | PASS |
| 2 | msg 선언 if에서 분할 | pb_7800_tr.c:405-406 | PASS |
| 3 | `DataBuff[82]` -> `DataBuff[KRX_HEAD_LEN]` (2x) | pb_7800_tr.c:1078,1244 | PASS |
| 4 | TR_DATA memcmp -> `Header_Fmt.MsgType, TR_RDS01/RDS02, KRX_TRCODE_LEN` | pb_7800_tr.c:355,365 | PASS |
| 5 | `#include` 분할 | pb_7800_tr.c:14-15 | PASS |
| 6 | 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN` (2x) | pb_7800_tr.c:592,640 | PASS |
| 7 | `#include` 분할 | pb_1800_ts.c:15-16 | PASS |
| 8 | 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN` (2x) | pb_1800_ts.c:662,710 | PASS |
| 9 | 핸드셰이크 `"0000"` -> `RESP_SUCCESS, KRX_ERRCODE_LEN` (2x) | pb_1200_tr.c:718,766 | PASS |
| 10 | `#include` 분할 | pa_1100_ts.c:15-16 | PASS |
| 11 | `memcmp(DataBuff+14, "TCHODR00000", 11)` -> `((KRX_HEADER*)DataBuff)->MsgType, TR_SESSION_DATA, KRX_TRCODE_LEN` | pa_1100_ts.c:814 | PASS |
| 12 | `memcmp(S_Fmt.Data, "0000", 4)` -> `RESP_SUCCESS, KRX_ERRCODE_LEN` | pa_1100_ts.c:659 | PASS |
| 13 | `memcpy "TCHODR00000"` -> `TR_SESSION_DATA, KRX_TRCODE_LEN` | pa_1100_ts.c:1175 | PASS |

**모든 13개 Act-1 수정이 올바르게 적용된 것으로 검증됨.**

---

## 권장사항

### 90%+ 달성을 위해 (선택적 일관성 수정)

이들은 모두 struct 레벨 액세스 (DataBuff 오프셋 위반 아님)이므로 안전하고 기능적으로 그대로입니다. 변환은 일관성만 개선할 것입니다.

1. **pb_7800_tr.c 줄 790**: `memcmp(KR_Fmt.Data, "0000", 4)` -> `memcmp(KR_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN)`

2. **pb_7800_tr.c 줄 1027-1028**: `memcmp(Header_Fmt.MsgType, "TRDESP50101/2", 11)` -> Analyze_Data()에서 `memcmp(Header_Fmt.MsgType, TR_RDS01_BOND_ITEM/TR_RDS02_KTS_ITEM, KRX_TRCODE_LEN)`

3. **pb_1800_ts.c 줄 478, 857**: `memcmp(S_Fmt.Data, "0000", 4)` -> `memcmp(S_Fmt.Data, RESP_SUCCESS, KRX_ERRCODE_LEN)`

4. **pa_1100_ts.c 줄 965**: `memcmp(S_Fmt.Header.MsgType, "TCHODR00000", 11)` -> `memcmp(S_Fmt.Header.MsgType, TR_SESSION_DATA, KRX_TRCODE_LEN)`

### 향후 단계 (설계 단계 4, 항목 7)

5. **pa_5010_mp.c**: 템플릿 문자열 -> struct 초기화 변환 (규칙 6/7). 현재 8개 파일 범위에 없음.

---

## 동기화 결정

**89%**의 일치율은 90% 목표 임계값 근처입니다. 두 옵션:

> **옵션 A**: 89%에서 수락 -- 모든 DataBuff 오프셋 위반 및 구문 오류가 해결됨. 남은 항목은 struct 레벨 코드의 미용 일관성 개선. 기능적으로 완료.

> **옵션 B**: 위의 4개 경미 일관성 수정 적용하여 공식적으로 92%+에 도달하고 검증 단계 종료.

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-18 | 초기 갭 분석 (72%) | Gap Detector Agent |
| 1.1 | 2026-02-18 | Act-1 재검증 (89%) -- 13개 수정 검증됨 | Gap Detector Agent |
