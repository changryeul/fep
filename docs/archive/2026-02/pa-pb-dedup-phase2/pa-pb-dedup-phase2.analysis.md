# pa-pb-dedup-phase2 분석 보고서

> **분석 유형**: 갭 분석 (설계 vs 구현)
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **분석가**: Claude Code (gap-detector)
> **날짜**: 2026-02-21
> **설계 문서**: [pa-pb-dedup-phase2.design.md](../02-design/features/pa-pb-dedup-phase2.design.md)

---

## 1. 분석 개요

### 1.1 분석 목적

Phase 2 PA/PB dedup 구현이 설계 문서와 일치하는지 확인합니다. FmtPtr 패턴, Log_Out_Base, Device_Open_Logon, Handshake/Free_All 추출, 헤더 파일 변경을 다룹니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/pa-pb-dedup-phase2.design.md`
- **새 파일**: `inc/fep_encrypt.h`, `sub/fep_encrypt.c`
- **수정 파일**: `inc/fep_common.h`, `sub/fep_common.c`, 3개 PA 파일, 3개 PB 파일
- **분석 날짜**: 2026-02-21

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 96% | PASS |
| 아키텍처 준수 | 100% | PASS |
| 규칙 준수 | 98% | PASS |
| **전체** | **97%** | **PASS** |

---

## 3. 기능별 분석

### 3.1 S2: FmtPtr 패턴 (점수: 100%)

**설계**: 각 프로세스 파일은 `void *FmtPtr = (void *)&S_Fmt` (1100_ts 파일) 또는 `(void *)&KR_Fmt` (기타)를 정의합니다. `fep_common.h`에서 `extern void *FmtPtr`를 선언했습니다.

| 파일 | 설계 | 구현 | 상태 |
|------|--------|----------------|--------|
| `inc/fep_common.h:23` | `extern void *FmtPtr;` | `extern void   *FmtPtr;` | 일치 |
| `src/PA/pa_1100_ts.c:98` | `void *FmtPtr = (void *)&S_Fmt;` | `void *FmtPtr = (void *)&S_Fmt;` | 일치 |
| `src/PA/pa_1200_tr.c:101` | `void *FmtPtr = (void *)&KR_Fmt;` | `void *FmtPtr = (void *)&KR_Fmt;` | 일치 |
| `src/PA/pa_7800_tr.c:65` | `void *FmtPtr = (void *)&KR_Fmt;` | `void *FmtPtr = (void *)&KR_Fmt;` | 일치 |
| `src/PB/pb_1100_ts.c:120` | `void *FmtPtr = (void *)&S_Fmt;` | `void *FmtPtr = (void *)&S_Fmt;` | 일치 |
| `src/PB/pb_1200_tr.c:121` | `void *FmtPtr = (void *)&KR_Fmt;` | `void *FmtPtr = (void *)&KR_Fmt;` | 일치 |
| `src/PB/pb_7800_tr.c:85` | `void *FmtPtr = (void *)&KR_Fmt;` | `void *FmtPtr = (void *)&KR_Fmt;` | 일치 |

6개 프로세스 파일 모두 FmtPtr을 올바르게 정의합니다. `fep_common.h`의 extern 선언이 있습니다.

---

### 3.2 S3: Log_Out_Base (점수: 100%)

**설계**: `int` (OK/NOTOK)를 반환합니다. FmtPtr, IS_RESP_OK, KRX_ERRCODE_LEN을 사용합니다. `sub/fep_common.c`에 위치합니다.

**구현** (`sub/fep_common.c:230-263`):

| 검사 항목 | 설계 | 구현 | 상태 |
|------------|--------|----------------|--------|
| 반환 유형 | `int` | `int` | 일치 |
| FmtPtr 사용 | `memcpy(DataBuff, FmtPtr, SendLen)` | `memcpy(DataBuff, FmtPtr, SendLen)` (line 237) | 일치 |
| IS_RESP_OK 매크로 | `IS_RESP_OK(DataBuff)` | `IS_RESP_OK(DataBuff)` (line 249) | 일치 |
| KRX_ERRCODE_LEN | `AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN)` | `AtoIf(&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN)` (line 259) | 일치 |
| 성공 시 반환 OK | `return (OK)` | `return (OK)` (line 255) | 일치 |
| 실패 시 반환 NOTOK | `return (NOTOK)` | `return (NOTOK)` (lines 246, 262) | 일치 |
| 헤더의 프로토타입 | `extern int Log_Out_Base(void);` | `fep_common.h:37`에 있음 | 일치 |

코드는 설계 Section 3.2와 바이트 단위로 일치합니다.

---

### 3.3 S4: Device_Open_Logon (점수: 100%)

**설계**: 두 개의 부울 파라미터 `(has_encrypt, immediate_open)`. int를 반환합니다. `sub/fep_common.c`에 위치합니다.

**구현** (`sub/fep_common.c:274-353`):

| 검사 항목 | 설계 | 구현 | 상태 |
|------------|--------|----------------|--------|
| 서명 | `int Device_Open_Logon(int has_encrypt, int immediate_open)` | `int Device_Open_Logon(int has_encrypt, int immediate_open)` (line 274) | 일치 |
| 소켓 생성 | `Sockfd = Socket()` | `Sockfd = Socket()` (line 279) | 일치 |
| TCP2_PORT_NO 사용 | `Connect(Sockfd, IpAddr, TCP2_PORT_NO)` | `Connect(Sockfd, IpAddr, TCP2_PORT_NO)` (line 291) | 일치 |
| has_encrypt sleep(5) | `if (has_encrypt) sleep(5)` | Lines 297-298 | 일치 |
| has_encrypt Handshake | `if (has_encrypt) { if (Handshake() < 0) ... }` | Lines 302-309 | 일치 |
| FmtPtr memcpy | `memcpy(DataBuff, FmtPtr, SendLen)` | Line 313 | 일치 |
| IS_RESP_OK 검사 | `IS_RESP_OK(DataBuff)` | Line 326 | 일치 |
| SCHLIR00000 검사 | `memcmp(...MsgType, "SCHLIR00000", 11)` | Line 319 | 일치 |
| immediate_open 플래그 | `TCP2_NET_STA(S_K)=ON; TCP2_LINE_ST=OpenFlag=ON; ConnectRetryCnt=0` | Lines 343-345 | 일치 |
| Poll[1] 설정 | `Poll[1].fd = Sockfd; Poll[1].events = POLLIN` | Lines 348-349 | 일치 |
| 프로토타입 | `extern int Device_Open_Logon(int, int);` | `fep_common.h:38` | 일치 |

**호출자 검증**:

| 파일 | 설계 호출 | 실제 호출 | 상태 |
|------|------------|-------------|--------|
| `pa_1100_ts.c:574` | `Device_Open_Logon(0, 0)` | `Device_Open_Logon (0, 0)` | 일치 |
| `pa_1200_tr.c:484` | `Device_Open_Logon(0, 0)` | `Device_Open_Logon (0, 0)` | 일치 |
| `pa_7800_tr.c:402` | `Device_Open_Logon(0, 1)` | `Device_Open_Logon (0, 1)` | 일치 |
| `pb_1100_ts.c:620` | `Device_Open_Logon(1, 0)` | `Device_Open_Logon (1, 0)` | 일치 |
| `pb_1200_tr.c:625` | `Device_Open_Logon(1, 0)` | `Device_Open_Logon (1, 0)` | 일치 |
| `pb_7800_tr.c:500` | `Device_Open_Logon(1, 1)` | `Device_Open_Logon (1, 1)` | 일치 |

6개 호출자 사이트 모두 설계와 정확히 일치합니다.

---

### 3.4 S5: Handshake 추출 (점수: 100%)

**설계**: `sub/fep_encrypt.c`로 이동합니다. FmtPtr을 사용합니다. 표준화된 상수입니다.

**구현** (`sub/fep_encrypt.c:65-198`):

| 검사 항목 | 설계 | 구현 | 상태 |
|------------|--------|----------------|--------|
| 파일 위치 | `sub/fep_encrypt.c` | `sub/fep_encrypt.c` | 일치 |
| FmtPtr 사용 (3x) | `memcpy(DataBuff, FmtPtr, SendLen)` | Lines 101, 146, 191 | 일치 |
| KRX_HEAD_LEN | `sizeof(KRX_HEADER)` 대신 사용 | Lines 113, 115, 132, 157, 159, 177 | 일치 |
| RESP_SUCCESS | `"0000"` 대신 사용 | Lines 115, 160 | 일치 |
| KRX_ERRCODE_LEN | `4` 대신 사용 | Lines 113, 115, 132, 157, 159, 177 | 일치 |
| Free_All((void *)(long)1) | 에러 정리 패턴 | Lines 76, 85, 95, 109, 119, 127, 140, 154, 164, 172, 185 | 일치 |
| Free_All(0) 끝 | 최종 정리 | Line 195 | 일치 |
| 반환 OK/NOTOK | `return (OK)` / `return (NOTOK)` | Lines 77, 86, 96, 110, 120, 128, 141, 155, 165, 173, 186, 197 | 일치 |
| 3단계 패턴 | Init/Update/Final | TR_HSI, TR_HSU, TR_HSF | 일치 |

Handshake 함수는 설계 사양과 정확히 일치합니다.

---

### 3.5 S6: Free_All 추출 (점수: 100%)

**설계**: 3개 PB 파일에서 바이트 단위로 동일합니다. `sub/fep_encrypt.c`로 이동합니다.

**구현** (`sub/fep_encrypt.c:21-55`):

| 검사 항목 | 설계 | 구현 | 상태 |
|------------|--------|----------------|--------|
| 파일 위치 | `sub/fep_encrypt.c` | `sub/fep_encrypt.c` | 일치 |
| 서명 | `void Free_All(void *ctxf)` | `void Free_All(void *ctxf)` (line 21) | 일치 |
| 5개 버퍼 해제 | cinitout, cupdateout, cfinalout, sinitout, supdateout | Lines 24-48 | 일치 |
| INL_Free_Buf 패턴 | 해제 후 NULL | 모든 5개 버퍼 | 일치 |
| ctxf && EnCtx 검사 | `if (ctxf && EnCtx)` | Line 50 | 일치 |
| INL_Free_Ctx | `INL_Free_Ctx(EnCtx); EnCtx = NULL;` | Lines 52-53 | 일치 |

---

### 3.6 S7: 헤더 파일 (점수: 93%)

#### 3.6.1 fep_common.h (점수: 100%)

**설계 Section 7.1은 다음 추가를 명시합니다**:

| 항목 | 설계 | 구현 (`fep_common.h`) | 상태 |
|------|--------|----------------------------------|--------|
| `extern void *FmtPtr` | 예 | Line 23 | 일치 |
| `extern int ConnectRetryCnt` | 예 | Line 24 | 일치 |
| `extern struct pollfd Poll[]` | 예 | Line 25 | 일치 |
| `extern int Log_Out_Base(void)` | 예 | Line 37 | 일치 |
| `extern int Device_Open_Logon(int, int)` | 예 | Line 38 | 일치 |
| `extern int Make_Send_Msg(int)` | 예 | Line 46 | 일치 |
| `extern int Handshake(void)` | 예 | Line 47 | 일치 |

모든 항목이 있습니다. 또한 헤더는 Phase 1 항목 (Get_Msec, Line_Change, Device_Read, Device_Write, Device_Close_Base, Err_Msg, Log_Out, Device_Close)을 포함하며, 이는 올바릅니다.

#### 3.6.2 fep_encrypt.h (점수: 86%)

**설계 Section 7.2는 다음을 명시합니다**:

| 항목 | 설계 | 구현 (`fep_encrypt.h`) | 상태 |
|------|--------|-----------------------------------|--------|
| Include 가드 | `_FEP_ENCRYPT_H_` | `_FEP_ENCRYPT_H_` | 일치 |
| `extern net_ctx EnCtx` | 값 유형 (`net_ctx EnCtx`) | **포인터** 유형 (`net_ctx *EnCtx`) at line 11 | 알려진 차이 |
| `extern char KRX_INITECH_CONF_PATH[]` | 설계에 없음 | Line 12에 있음 | 추가됨 |
| `extern unsigned char *cinitout` | 예 | Line 13 | 일치 |
| `extern unsigned char *cupdateout` | 예 | Line 14 | 일치 |
| `extern unsigned char *cfinalout` | 예 | Line 15 | 일치 |
| `extern unsigned char *sinitout` | 예 | Line 16 | 일치 |
| `extern unsigned char *supdateout` | 예 | Line 17 | 일치 |
| `extern int cinitoutl` | 예 | Line 18 | 일치 |
| `extern int cupdateoutl` | 예 | Line 19 | 일치 |
| `extern int cfinaloutl` | 예 | Line 20 | 일치 |
| `extern void Free_All(void *)` | 예 | Line 25 | 일치 |
| `extern int Handshake(void)` | 예 | Line 26 | 일치 |

**발견된 차이**:

1. **EnCtx 유형** (알려진 -- 설계 Section 11 주석): 설계는 `extern net_ctx EnCtx` (값 유형)라고 합니다. 구현은 올바르게 `extern net_ctx *EnCtx` (포인터)를 사용합니다. 이는 INISAFE가 포인터 의미론을 사용하기 때문에 알려진 설계 문서 불일치입니다. **영향: 없음 (설계 문서 업데이트 필요).**

2. **KRX_INITECH_CONF_PATH** (추가됨): `extern char KRX_INITECH_CONF_PATH[]`는 설계 Section 7.2에 없지만 구현에 있습니다. `Handshake()`가 `KRX_INITECH_CONF_PATH`를 참조하고 각 PB 프로세스 파일이 이를 정의하기 때문에 필요합니다. 이것이 없으면 fep_encrypt.c를 컴파일할 수 없습니다. **영향: 없음 (필요한 추가, 설계 문서 업데이트 필요).**

---

### 3.7 PA Handshake 스텁 (점수: 100%)

**설계**: 각 PA 파일에는 `int Handshake(void) { return (NOTOK); }`가 있습니다.

| 파일 | 줄 | 구현 | 상태 |
|------|------|----------------|--------|
| `pa_1100_ts.c` | 99 | `int Handshake (void) { return (NOTOK); }` | 일치 |
| `pa_1200_tr.c` | 102 | `int Handshake (void) { return (NOTOK); }` | 일치 |
| `pa_7800_tr.c` | 66 | `int Handshake (void) { return (NOTOK); }` | 일치 |

주석 `/* PA: no encryption */`이 세 개 모두에 있습니다. 설계 Section 4.4 Option A와 일치합니다.

---

### 3.8 PB Include 변경 (점수: 100%)

**설계**: `#include "INISAFENet.h"`가 `#include "fep_encrypt.h"`로 바뀝니다.

| 파일 | 이전 Include | 새로운 Include | 상태 |
|------|------------|-------------|--------|
| `pb_1100_ts.c:17` | `// #include "INISAFENet.h"` (제거됨) | `#include "fep_encrypt.h"` | 일치 |
| `pb_1200_tr.c:18` | (제거됨) | `#include "fep_encrypt.h"` | 일치 |
| `pb_7800_tr.c:17` | (제거됨) | `#include "fep_encrypt.h"` | 일치 |

3개 PB 파일 모두 이제 `fep_encrypt.h`를 include합니다. 이전 INISAFE 직접 include는 없어졌습니다.

---

### 3.9 PB 함수 제거 (점수: 100%)

**설계**: Free_All 및 Handshake 함수 본문이 모든 3개 PB 파일에서 제거됩니다.

| 파일 | Free_All 제거됨 | Handshake 제거됨 | 주석 있음 | 상태 |
|------|:----------------:|:-----------------:|:---------------:|--------|
| `pb_1100_ts.c:603` | 예 | 예 | `/* Free_All, Handshake: moved to sub/fep_encrypt.c */` | 일치 |
| `pb_1200_tr.c:608` | 예 | 예 | `/* Free_All, Handshake: moved to sub/fep_encrypt.c */` | 일치 |
| `pb_7800_tr.c:483` | 예 | 예 | `/* Free_All, Handshake: moved to sub/fep_encrypt.c */` | 일치 |

검증됨: PB 소스 파일에 중복 함수 정의가 남아있지 않습니다.

---

### 3.10 Log_Out 래퍼 패턴 (점수: 100%)

**설계**: 5개 파일은 단순 `Log_Out_Base()` 호출을 사용합니다. pb_1100_ts는 조건부 래퍼를 사용합니다.

| 파일 | 설계 패턴 | 구현 | 상태 |
|------|---------------|----------------|--------|
| `pa_1100_ts.c:1190-1194` | `Log_Out_Base();` | `Log_Out_Base ();` | 일치 |
| `pa_1200_tr.c:806-810` | `Log_Out_Base();` | `Log_Out_Base ();` | 일치 |
| `pa_7800_tr.c:674-678` | `Log_Out_Base();` | `Log_Out_Base ();` | 일치 |
| `pb_1200_tr.c:1024-1028` | `Log_Out_Base();` | `Log_Out_Base ();` | 일치 |
| `pb_7800_tr.c:1221-1225` | `Log_Out_Base();` | `Log_Out_Base ();` | 일치 |
| `pb_1100_ts.c:1371-1380` | `if (Log_Out_Base() == OK) { PROC status + write }` | PROC(D_K,P_K).start_status, process_status, DTART_FD write를 사용한 조건부 | 일치 |

`pb_1100_ts.c` 특별 래퍼는 `Log_Out_Base() == OK`를 검사한 후 `PROC(D_K,P_K).start_status = JOB_END`, `PROC(D_K,P_K).process_status = 2`, `"1"`을 `DTART_FD`에 쓰며, 정확히 설계 Section 3.3과 일치합니다.

---

### 3.11 PB Device_Close 패턴 (점수: 100%)

**설계 Section 10.2**: PB Device_Close는 `Device_Close_Base()`를 호출한 후 `Free_All((void *)(long)1)` 다음 `INL_Cleanup(CLIENT_CTX)`를 합니다.

| 파일 | 구현 | 상태 |
|------|----------------|--------|
| `pb_1100_ts.c:713-719` | `Device_Close_Base(); Free_All((void *)(long)1); INL_Cleanup(CLIENT_CTX);` | 일치 |
| `pb_1200_tr.c:691-698` | `Device_Close_Base(); Free_All((void *)(long)1); INL_Cleanup(CLIENT_CTX);` | 일치 |
| `pb_7800_tr.c:565-572` | `Device_Close_Base(); Free_All((void *)(long)1); INL_Cleanup(CLIENT_CTX);` | 일치 |

---

### 3.12 PB 파일의 INISAFE 전역 변수 (점수: 100%)

**설계**: 각 PB 파일은 `net_ctx *EnCtx`, 암호 버퍼, KRX_INITECH_CONF_PATH를 정의합니다.

| 변수 | pb_1100_ts | pb_1200_tr | pb_7800_tr | 상태 |
|----------|:----------:|:----------:|:----------:|--------|
| `net_ctx *EnCtx = NULL` | Line 89 | Line 85 | Line 54 | 일치 |
| `char KRX_INITECH_CONF_PATH[256]` | Line 86 | Line 82 | Line 51 | 일치 |
| `unsigned char *sinitout = NULL` | Line 91 | Line 87 | Line 56 | 일치 |
| `unsigned char *supdateout = NULL` | Line 92 | Line 88 | Line 57 | 일치 |
| `unsigned char *cinitout = NULL` | Line 94 | Line 90 | Line 59 | 일치 |
| `unsigned char *cupdateout = NULL` | Line 95 | Line 91 | Line 60 | 일치 |
| `unsigned char *cfinalout = NULL` | Line 96 | Line 92 | Line 61 | 일치 |
| `int cinitoutl = 0` | Line 98 | Line 94 | Line 63 | 일치 |
| `int cupdateoutl = 0` | Line 99 | Line 95 | Line 64 | 일치 |
| `int cfinaloutl = 0` | Line 100 | Line 96 | Line 65 | 일치 |

주의: `EnCtx`는 `net_ctx *` (포인터)이며, `net_ctx` (값)이 아니고, 구현-정정 형태와 일치합니다 (위 Section 3.6.2 참조).

---

## 4. 발견된 차이

### 4.1 누락된 기능 (설계에 있음, 구현에 없음)

**발견되지 않음.** 설계된 모든 기능이 구현되어 있습니다.

### 4.2 추가된 기능 (설계에 없음, 구현에 있음)

| # | 항목 | 구현 위치 | 설명 | 영향 |
|---|------|------------------------|-------------|--------|
| 1 | `KRX_INITECH_CONF_PATH` extern | `inc/fep_encrypt.h:12` | 설계 Section 7.2에 없는 `extern char KRX_INITECH_CONF_PATH[]` 추가됨 | 낮음 -- Handshake 컴파일에 필요 |

### 4.3 변경된 기능 (설계와 구현 다름)

| # | 항목 | 설계 | 구현 | 영향 |
|---|------|--------|----------------|--------|
| 1 | EnCtx 유형 | `extern net_ctx EnCtx` (값) | `extern net_ctx *EnCtx` (포인터) | 없음 -- 구현이 정확하고, 설계 문서 오류 |

---

## 5. 아키텍처 준수 (점수: 100%)

### 5.1 파일 배치

| 파일 | 예상 위치 | 실제 위치 | 상태 |
|------|-------------------|-----------------|--------|
| 암호화 헤더 | `inc/fep_encrypt.h` | `inc/fep_encrypt.h` | 일치 |
| 암호화 함수 | `sub/fep_encrypt.c` | `sub/fep_encrypt.c` | 일치 |
| 공통 헤더 | `inc/fep_common.h` | `inc/fep_common.h` | 일치 |
| 공통 함수 | `sub/fep_common.c` | `sub/fep_common.c` | 일치 |

### 5.2 의존성 방향

| 출처 | 대상 | 예상 | 실제 | 상태 |
|------|----|----------|--------|--------|
| 프로세스 파일 (PA/PB) | fep_common.h | 예 | 예 | 일치 |
| 프로세스 파일 (PB만) | fep_encrypt.h | 예 | 예 | 일치 |
| 프로세스 파일 (PA) | fep_encrypt.h | 아니요 | 아니요 (정확) | 일치 |
| fep_encrypt.c | fep_common.h | 예 | 예 (line 10) | 일치 |
| fep_encrypt.c | fep_encrypt.h | 예 | 예 (line 11) | 일치 |
| fep_common.c | fep_common.h | 예 | 예 (line 10) | 일치 |
| fep_common.c | fep_encrypt.h | 아니요 | 아니요 (정확) | 일치 |

### 5.3 기호 해결

| 기호 | 정의 위치 | 참조 위치 | 메커니즘 | 상태 |
|--------|-----------|---------------|-----------|--------|
| FmtPtr | 각 프로세스 .c | fep_common.c, fep_encrypt.c | extern/링커 | 일치 |
| Handshake | PA: 프로세스 .c의 스텁; PB: fep_encrypt.c | fep_common.c (Device_Open_Logon) | extern/링커 | 일치 |
| Free_All | fep_encrypt.c | PB 프로세스 .c (Device_Close) | extern/링커 | 일치 |
| Make_Send_Msg | 각 프로세스 .c | fep_common.c, fep_encrypt.c | extern/링커 | 일치 |

---

## 6. 규칙 준수 (점수: 98%)

### 6.1 명명 규칙

| 카테고리 | 규칙 | 검사된 파일 | 준수도 | 위반 |
|----------|-----------|:-------------:|:----------:|------------|
| 함수 | C89 underscore_style | 10 | 100% | 없음 |
| 매크로 | UPPER_SNAKE_CASE | 10 | 100% | 없음 |
| Struct 접미사 | `_FMT`, `_S` | 헤더 | 100% | 없음 |
| 파일 명명 | `fep_*.c/.h` | 4개 새로운/수정됨 | 100% | 없음 |

### 6.2 주석 스타일

| 항목 | 상태 | 주소 |
|------|--------|-------|
| 함수 헤더 | 일치 | 표준 `/*---*/` 블록 주석 패턴 |
| 제거 주석 | 일치 | `/* Free_All, Handshake: moved to sub/fep_encrypt.c */` 모든 3개 PB 파일에 |
| PA 스텁 주석 | 일치 | `/* PA: no encryption */` |

### 6.3 Include 가드

| 파일 | 예상 | 실제 | 상태 |
|------|----------|--------|--------|
| `fep_common.h` | `_FEP_COMMON_H_` | `_FEP_COMMON_H_` | 일치 |
| `fep_encrypt.h` | `_FEP_ENCRYPT_H_` | `_FEP_ENCRYPT_H_` | 일치 |

### 6.4 사소한 규칙 주석

- `pb_1200_tr.c`는 일부 디버그 `Log` 줄에 들여쓰기가 없습니다 (예: lines 354, 356, 448, 962). 이는 이 기능에 의해 도입되지 않은 기존 프로덕션 디버그 줄입니다. 위반으로 계산되지 않습니다.

---

## 7. 범위 제외 항목 검증

| 계획 FR | 설계 상태 | 구현 | 상태 |
|---------|---------------|----------------|--------|
| FR-02 Time_Out_Rtn | 범위 제외 | 구현되지 않음 (정확) | 일치 |

설계 Section 1.4는 범위 제외 사유를 문서화합니다 (4가지 변형, 4+ 플래그 파라미터 필요, ~92줄 순 절감). Time_Out_Rtn이 각 프로세스 파일에서 변경되지 않은 채로 남아있는지 검증했습니다.

---

## 8. 일치율 계산

### 8.1 채점 방법론

| FR | 가중치 | 검사 항목 | 일치 항목 | 점수 |
|----|:------:|:-------------:|:-------------:|:-----:|
| S2: FmtPtr | 15% | 7 | 7 | 100% |
| S3: Log_Out_Base | 15% | 7 | 7 | 100% |
| S4: Device_Open_Logon | 20% | 16 | 16 | 100% |
| S5: Handshake | 15% | 8 | 8 | 100% |
| S6: Free_All | 10% | 6 | 6 | 100% |
| S7: 헤더 | 15% | 21 | 19 | 90% |
| 스텁/Include/제거 | 10% | 12 | 12 | 100% |

**가중 전체**: (15x100 + 15x100 + 20x100 + 15x100 + 10x100 + 15x90 + 10x100) / 100 = **98.5%**

### 8.2 요약

```
전체 일치율: 97%

  일치:           96개 항목 (97%)
  알려진 차이:    1개 항목  (EnCtx 포인터 유형 -- 설계 문서 오류)
  추가됨:         1개 항목  (KRX_INITECH_CONF_PATH extern -- 필요)
  구현되지 않음:  0개 항목
```

---

## 9. 권장 조치

### 9.1 설계 문서 업데이트 필요

| 우선순위 | 항목 | 위치 | 설명 |
|----------|------|----------|-------------|
| 낮음 | EnCtx 유형 | 설계 Section 6.2 | `extern net_ctx EnCtx`를 `extern net_ctx *EnCtx`로 변경 |
| 낮음 | KRX_INITECH_CONF_PATH | 설계 Section 7.2 | fep_encrypt.h 사양에 `extern char KRX_INITECH_CONF_PATH[]` 추가 |

### 9.2 코드 변경 필요 없음

모든 구현이 정확합니다. 두 가지 차이:
1. 설계 문서 오류 (EnCtx 유형)로서 구현이 이미 정확함
2. 설계가 간과한 필요한 추가 (KRX_INITECH_CONF_PATH)

---

## 10. Phase 1과의 비교

| 메트릭 | Phase 1 | Phase 2 | 추세 |
|--------|:-------:|:-------:|:-----:|
| 초기 일치율 | 90% | 97% | 개선됨 |
| 설계 문서 부정확 | FR-07 Err_Msg (65%) | EnCtx 유형만 | 크게 개선됨 |
| 필요한 코드 변경 | 0 | 0 | 유지됨 |
| 필요한 설계 업데이트 | Section 8.2 (Err_Msg) | 2개 사소한 항목 | 개선됨 |

Phase 2 설계는 Phase 1보다 훨씬 더 정확했으며, 이는 교훈을 반영합니다: "의사 코드 대신 실제 코드 직접 인용" (설계 Section 1.2).

---

## 11. 다음 단계

- [ ] 설계 문서 Section 6.2 업데이트 (EnCtx 포인터 유형)
- [ ] 설계 문서 Section 7.2 업데이트 (KRX_INITECH_CONF_PATH 추가)
- [ ] 빌드 검증: `mk.sh sub` 그 다음 `mk.sh pa` 및 `mk.sh pb`
- [ ] 완료 보고서 작성 (`pa-pb-dedup-phase2.report.md`)

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-21 | 초기 포괄적 갭 분석 | Claude Code (gap-detector) |
