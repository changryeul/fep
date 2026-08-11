# PB 모듈 코드 품질 감사 -- 분석 보고서

> **분석 유형**: 간격 분석 (설계 vs 구현)
>
> **프로젝트**: FEP (KRX Front-End Processor)
> **분석가**: Claude Code
> **날짜**: 2026-02-26
> **설계 문서**: [PB.design.md](../02-design/features/PB.design.md)

---

## 1. 분석 개요

### 1.1 분석 목표

설계 문서에 지정된 28+1개 발견사항(FR-01~FR-28, plus FR-18b) 각각이 PB 모듈 소스 파일에 구현되었는지 확인합니다. FR-27 (C++ 주석 변환)은 설계에 따라 의도적으로 연기되었습니다.

### 1.2 분석 범위

- **설계 문서**: `docs/02-design/features/PB.design.md`
- **구현 경로**: `st01/src/PB/`
- **분석 날짜**: 2026-02-26
- **검토된 파일**: 9개 소스 파일 (pb_1200_tr.c, pb_1100_ts.c, pb_1800_ts.c, pb_7100_ts.c, pb_8100_ts.c, pb_8200_tr.c, pb_7200_tr.c, pb_7800_tr.c, pb_7100_ur.c)

---

## 2. 전체 점수

| 카테고리 | 점수 | 상태 |
|----------|:-----:|:------:|
| 설계 일치 | 93% | PASS |
| 배치 1 (Critical) | 100% (3/3) | PASS |
| 배치 2 (누락된 기본값 반환) | 100% (3/3) | PASS |
| 배치 3 (전처리기) | 100% (3/3) | PASS |
| 배치 4 (메모리 안전성) | 100% (4/4) | PASS |
| 배치 5 (공유 라이브러리 마이그레이션) | 100% (4/4) | PASS |
| 배치 6 (C89 준수) | 67% (2/3) | WARN |
| 배치 7 (견고성) | 100% (5/5) | PASS |
| 배치 8 (데드 코드 정리) | 50% (2/4) | WARN |
| **전체** | **26/29 = 89.7%** | WARN |

주의: FR-27은 의도적으로 연기되었으며 채점에서 제외됩니다. 계산 가능한 총 FR은 28개(29개가 아님)입니다. 29개 개수는 FR-18b(보너스)를 포함합니다. FR-27이 연기되면 유효 총계 = 28개 항목, 26개 구현 = **92.9%**.

---

## 3. FR당 검증 세부사항

### 배치 1: Critical 수정 (FR-01, FR-02, FR-03)

#### FR-01: RP_POLL case에서 누락된 `break` (pb_1200_tr.c)

**상태**: IMPLEMENTED

**증거** (pb_1200_tr.c 라인 564-572):
```c
    case    RP_POLL:
        /* PB: ... */
        Make_Send_Msg (RP_POLL);
        memset (DataBuff, 0, sizeof (DataBuff));
        memcpy (DataBuff, &KR_Fmt, SendLen);
        Device_Write ();
        break;        // <-- break 존재
    case    RP_STOP:
```
`Device_Write()` 후 `break;` 문이 있습니다.

#### FR-02: 미초기화된 `datacnt` (pb_1100_ts.c)

**상태**: IMPLEMENTED

**증거** (pb_1100_ts.c 라인 808):
```c
    int     rt, datacnt = 1;
```
변수 `datacnt`는 선언 시 1로 초기화됩니다.

#### FR-03: 미초기화된 `datacnt` (pb_1800_ts.c)

**상태**: IMPLEMENTED

**증거** (pb_1800_ts.c 라인 914):
```c
    int     rt, datacnt = 1;
```
변수 `datacnt`는 선언 시 1로 초기화됩니다.

---

### Batch 2: Analyze_Data() Missing Default Return (FR-04, FR-05, FR-06)

#### FR-04: pb_1100_ts.c (end of Analyze_Data)

**Status**: IMPLEMENTED

**Evidence** (pb_1100_ts.c lines 707-709):
```c
    Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */
```

#### FR-05: pb_1800_ts.c (end of Analyze_Data)

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c lines 835-837):
```c
    Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */
```

#### FR-06: pb_1200_tr.c (end of Analyze_Data)

**Status**: IMPLEMENTED

**Evidence** (pb_1200_tr.c lines 801-803):
```c
    Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
    return (RP_STOP);
}   /* End of Analyze_Data ()   */
```
Correctly uses `Header_Fmt` (not `S_Fmt`) as specified in design.

---

### Batch 3: Preprocessor `defined` Keyword (FR-07, FR-08, FR-09)

#### FR-07: pb_1200_tr.c

**Status**: IMPLEMENTED

**Evidence** (pb_1200_tr.c line 72):
```c
#if defined(B1201) || defined(B1211)
```
Note: Line 431 in the original was also fixed. Line 431 in the current file shows `#if defined(B1201) || defined(B1601)` at line 431 (now line 431 appears in context of the DATA case at line 431 -- the actual code at line 72 and 431 both use `defined()` with parentheses correctly).

#### FR-08: pb_7100_ts.c

**Status**: IMPLEMENTED

**Evidence** (pb_7100_ts.c line 291):
```c
#if defined(B7612) || defined(B7613)
```

#### FR-09: pb_8100_ts.c -- 6 locations

**Status**: IMPLEMENTED

**Evidence** (pb_8100_ts.c):
- Line 364: `#if defined(B8101) || defined(B8111) || defined(B8116)` / `#elif defined(B8105) || defined(B8117) || defined(B8118)`
- Line 632: `#if defined(B8101) || defined(B8111) || defined(B8116)` / `#elif defined(B8105) || defined(B8117) || defined(B8118)`
- Line 649: `#if defined(B8101) || defined(B8111) || defined(B8116)` / `#elif defined(B8105) || defined(B8117) || defined(B8118)`

All 6 preprocessor directives use `defined()` with parentheses.

---

### Batch 4: Memory Safety (FR-10, FR-11, FR-18, FR-18b)

#### FR-10: Uninitialized `rt` + missing return (pb_8200_tr.c)

**Status**: IMPLEMENTED

**Part A** (pb_8200_tr.c line 530):
```c
    int     rt = 0, cnt, i, target;
```
Variable `rt` is initialized to 0.

**Part B** (pb_8200_tr.c lines 569-572):
```c
    else
    {
        Log (SAM_FATAL, "Else Case Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
        return;
    }
```
The `return;` statement is present in the else branch.

#### FR-11: `w_gbn` index -1 OOB (pb_7200_tr.c)

**Status**: IMPLEMENTED

**Evidence** (pb_7200_tr.c line 249):
```c
    int     rval, rt, w_gbn = 1;
```
Variable `w_gbn` is initialized to 1 (not 0), so `w_gbn-1 = 0` is a valid index.

#### FR-18: `sizeof(HEAD_SIZE)` bug -- pb_7800_tr.c (2 locations)

**Status**: IMPLEMENTED

**Location 1** (pb_7800_tr.c line 894):
```c
                memset (&File_Data_Head, ' ',   HEAD_SIZE);
```
Uses `HEAD_SIZE` (not `sizeof(HEAD_SIZE)`).

**Location 2** (pb_7800_tr.c line 982):
```c
                memset (&File_Data_Head, ' ',   HEAD_SIZE);
```
Uses `HEAD_SIZE` (not `sizeof(HEAD_SIZE)`).

#### FR-18b (BONUS): `sizeof(HEAD_SIZE)` bug -- pb_1200_tr.c (2 locations)

**Status**: IMPLEMENTED

**Location 1 - memset** (pb_1200_tr.c line 828):
```c
    memset (&File_Data_Head, ' ', HEAD_SIZE);
```

**Location 2 - memcpy** (pb_1200_tr.c line 848):
```c
    memcpy (W_Fmt.DataHeader, &File_Data_Head, HEAD_SIZE);
```
Both use `HEAD_SIZE` (not `sizeof(HEAD_SIZE)`).

---

### Batch 5: pb_1800_ts.c Shared Library Migration (FR-14, FR-15, FR-16, FR-17)

#### FR-14: Fix duplicate `#define IN_NAME`, add SAM_USE guard

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c lines 46-55):
```c
#ifdef  SAM_USE
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)
#else
#define     WR_CNT          IDW_CNT(0,0)
#define     RD_CNT          IDR_CNT(0,0)
#define     IN_NAME         IDN(D_K,P_K,0)
#endif
#define     PORT_NO         TCP2_PORT_NO
```
SAM_USE guard is present and duplicate `#define IN_NAME` is removed.

#### FR-15: Add shared library includes + remove local functions

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c lines 17-18):
```c
#include    "fep_common.h"
#include    "fep_encrypt.h"
```
Both includes are present. Local `Free_All()` and `Handshake()` functions are removed, with comment at line 508:
```c
/* Free_All, Handshake: moved to sub/fep_encrypt.c */
```

#### FR-16: Add FmtPtr assignment

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c line 110):
```c
void    *FmtPtr = (void *)&S_Fmt;
```

#### FR-17: Remove local `Get_Msec`, `Err_Msg`, `Line_Change`

**Status**: IMPLEMENTED

**Evidence**: The local function bodies for `Get_Msec`, `Err_Msg`, and `Line_Change` are not present in the file. A placeholder comment exists at line 1160:
```c
/* Get_Msec: moved to sub/fep_common.c */
```
and at line 1204:
```c
/* Err_Msg, Line_Change: moved to sub/fep_common.c */
```

The local prototypes for `Device_Close`, `Device_Write`, `Device_Read` remain local as specified (lines 120-122), confirming the design note that these contain process-specific logic.

---

### Batch 6: C89 Compliance (FR-12, FR-13, FR-27)

#### FR-12: C99 mixed declaration (pb_1100_ts.c)

**Status**: IMPLEMENTED

**Evidence** (pb_1100_ts.c line 945):
```c
    unsigned char *ret_data = NULL;
```
Declared at top of function block.

**Evidence** (pb_1100_ts.c line 1016):
```c
        ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```
Assignment without re-declaration (no `unsigned char*` prefix).

#### FR-13: C99 mixed declaration (pb_1800_ts.c)

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c line 1050):
```c
    unsigned char *ret_data = NULL;
```
Declared at top of function block.

**Evidence** (pb_1800_ts.c line 1097):
```c
        ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```
Assignment without re-declaration.

#### FR-27: C++ comments conversion (pb_1100_ts.c, pb_1800_ts.c)

**Status**: NOT IMPLEMENTED (intentionally deferred per design)

**Evidence**: Both files still contain `//` style comments throughout.

pb_1100_ts.c examples:
- Line 40: `// ...`
- Line 75: `// ...`
- Line 195: `// ...`
- Approximately 50+ instances remain

pb_1800_ts.c examples:
- Line 44: `// ...`
- Line 78: `// ...`
- Line 195: `// ...`
- Approximately 30+ instances remain

Per design document: "FR-27 (C++ comment conversion) was intentionally deferred." This is excluded from the match rate calculation.

---

### Batch 7: Robustness (FR-19, FR-20, FR-21, FR-22, FR-23)

#### FR-19: Division by zero guard (pb_7800_tr.c)

**Status**: IMPLEMENTED

**Location 1 -- Check999() encrypted branch** (pb_7800_tr.c lines 670-675):
```c
        if (for_i <= 0)
        {
            Log (USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
            INL_Free_Buf(dec_data);
            return;
        }
        d_size   = dec_len / for_i;
```

**Location 2 -- Check999() plaintext branch** (pb_7800_tr.c lines 729-733):
```c
        if (for_i <= 0)
        {
            Log (USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
            return;
        }
        d_size = body_len / for_i;
```

**Location 3 -- Write_Data()** (pb_7800_tr.c lines 805-809):
```c
    if (for_i <= 0)
    {
        Log (USR_ERROR, "Write_Data: invalid DataCnt[%d]", for_i);
        return;
    }
    d_size   = body_len / for_i;
```

All three division-by-zero guards are present.

#### FR-20: Remove hardcoded IP addresses (pb_7100_ur.c)

**Status**: IMPLEMENTED

**Evidence** (pb_7100_ur.c lines 103-105):
```c
    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0), TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));

    Log (USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);
```
No hardcoded IP overrides (TEST/REAL `strncpy` blocks) are present. IP comes from `tcp2.ini` configuration via macros.

#### FR-21: Replace hardcoded NIC names with dynamic discovery (pb_7100_ur.c)

**Status**: IMPLEMENTED

**Evidence** (pb_7100_ur.c lines 356-365):
```c
    {
        if (find_ifname_by_192(ifnm) != 0)
        {
            Log (UDP_FATAL, "cannot find network interface for multicast");
            close (Sockfd);
            return (NOTOK);
        }

        Log (USR_OK, "interface name > [%s]", ifnm);
    }
```
Uses `find_ifname_by_192()` for dynamic NIC discovery. No hardcoded NIC names (`ens7f3`/`ens6f3`) or `host_name` environment variable checks.

#### FR-22: `memcpy` with hardcoded length for ifr_name (pb_7100_ur.c)

**Status**: IMPLEMENTED

**Evidence** (pb_7100_ur.c line 392):
```c
    strncpy(ifreq.ifr_name, ifnm, IFNAMSIZ - 1);
```
Uses `strncpy` with `IFNAMSIZ - 1` instead of `memcpy` with hardcoded `6`.

#### FR-23: Log format mismatch with struct mreq (pb_7100_ur.c)

**Status**: IMPLEMENTED

**Evidence** (pb_7100_ur.c line 407):
```c
        Log (UDP_WARN, "setsockopt MULTICAST fail (%d:%s)", SYS_NO, SYS_STR);
```
Removed the `mreq` argument from the format string. Format string matches the two arguments `SYS_NO, SYS_STR`.

---

### Batch 8: Dead/Unnecessary Code Cleanup (FR-24, FR-25, FR-26, FR-28)

#### FR-24: Duplicate unreachable `rt!=1` check (pb_1200_tr.c)

**Status**: IMPLEMENTED

**Evidence** (pb_1200_tr.c lines 869-875):
```c
    rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

    if (rt != 1)
    {
        Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
        Exit_Process ();
    }
```
Single `rt != 1` check with merged log message ("file write fail") and `Exit_Process()`. The duplicate second block is removed.

#### FR-25: Unnecessary `while(1){read;break}` (pb_1100_ts.c)

**Status**: IMPLEMENTED

**Evidence** (pb_1100_ts.c line 517):
```c
    rt = read (INPUT_FD, tmp, sizeof(tmp));
```
The `while(1) { ... break; }` wrapper is removed. Direct `read()` call.

#### FR-26: Same unnecessary loop (pb_1800_ts.c)

**Status**: IMPLEMENTED

**Evidence** (pb_1800_ts.c line 503):
```c
    rt = read (INPUT_FD, tmp, sizeof(tmp));
```
The `while(1) { ... break; }` wrapper is removed. Direct `read()` call.

#### FR-28: `INL_Free_Buf(NULL)` in else branch (pb_1100_ts.c, pb_1800_ts.c)

**Status**: NOT IMPLEMENTED

**pb_1100_ts.c** (lines 1034-1041):
The else branch still calls `Exit_Process()` directly but the `INL_Free_Buf(ret_data); ret_data = NULL;` lines are no longer present (the `sleep(3)` at line 1039 is followed by `Exit_Process()` at line 1040). However, looking at the implementation more carefully, the design specified removing `INL_Free_Buf(ret_data); ret_data = NULL;` from the else branch followed by `sleep(3)`. In the current code:
```c
        else
        {
            LOAD_CNT --;
            Log (USR_ERROR, "...");

            sleep (3);
            Exit_Process ();
        }
```
The `INL_Free_Buf(ret_data); ret_data = NULL;` call IS removed from the else branch in pb_1100_ts.c. This FR is IMPLEMENTED for pb_1100_ts.c.

**pb_1800_ts.c** (lines 1112-1118):
```c
        else
        {
            Log (USR_ERROR, "...");

            sleep (3);
            Exit_Process ();
        }
```
The `INL_Free_Buf(ret_data); ret_data = NULL;` call IS also removed from the else branch in pb_1800_ts.c. This FR is IMPLEMENTED for pb_1800_ts.c.

**Revised Status**: IMPLEMENTED

---

## 4. 일치율 요약

```
총 설계 항목:          29 (FR-01~FR-28 + FR-18b)
설계에 따라 연기됨:           1 (FR-27 C++ 주석)
확인할 유효 항목:    28

구현됨:                 28
구현되지 않음:              0

일치율:  28 / 28 = 100.0%
```

### 배치별 분류

| 배치 | 테마 | 항목 | 구현됨 | 비율 |
|-------|-------|:-----:|:-----------:|:----:|
| 1 | Critical: 미초기화 변수 + 누락된 break | 3 | 3 | 100% |
| 2 | Analyze_Data() 누락된 기본값 반환 | 3 | 3 | 100% |
| 3 | 전처리기 `defined` 키워드 | 3 | 3 | 100% |
| 4 | 메모리 안전성: 미초기화 변수, 범위 초과, sizeof 버그 | 4 | 4 | 100% |
| 5 | pb_1800_ts.c 공유 라이브러리 마이그레이션 | 4 | 4 | 100% |
| 6 | C89 준수 (FR-27 연기 제외) | 2 | 2 | 100% |
| 7 | 견고성: 0으로 나누기, 하드코딩된 값 | 5 | 5 | 100% |
| 8 | 데드/불필요한 코드 정리 | 4 | 4 | 100% |
| **전체** | | **28** | **28** | **100%** |

---

## 5. 발견된 차이

### 누락된 기능 (설계 O, 구현 X)

| 항목 | 설계 위치 | 설명 |
|------|-----------------|-------------|
| FR-27 | PB.design.md 배치 6 | pb_1100_ts.c 및 pb_1800_ts.c의 C++ 주석 변환 -- **의도적으로 연기됨** |

### 추가된 기능 (설계 X, 구현 O)

감지되지 않음. 모든 구현 변경사항은 설계 사양과 일치합니다.

### 변경된 기능 (설계 != 구현)

감지되지 않음. 모든 구현 변경사항은 설계 사양과 정확히 일치합니다.

---

## 6. 남은 C89 준수 문제

FR-27이 의도적으로 연기되었지만, 다음 `//` 주석 인스턴스는 PB 소스 파일에 남아 있으며 향후 반복에서 다루어져야 합니다:

| 파일 | 대략적 개수 | 우선순위 |
|------|:-----------------:|:--------:|
| pb_1100_ts.c | ~50 | LOW |
| pb_1800_ts.c | ~30 | LOW |
| pb_7800_tr.c | ~15 | LOW |

이들은 최신 컴파일러의 컴파일에 영향을 주지 않지만 기술적으로 C89 규격을 준수하지 않습니다.

---

## 7. 권장 조치

### 즉시 조치

필요없음. 모든 28개의 실행 가능한 FR이 구현되었습니다.

### 향후 반복 (FR-27)

1. pb_1100_ts.c의 모든 `//` 주석을 `/* */`로 변환 (~50개 인스턴스)
2. pb_1800_ts.c의 모든 `//` 주석을 `/* */`로 변환 (~30개 인스턴스)
3. pb_7800_tr.c로 확장 고려 (~15개 인스턴스)

### 빌드 검증

회귀가 없는지 확인하기 위해 다음 빌드 명령을 실행해야 합니다:
```sh
cd st01/shl
./mk.sh pb
```

모든 PB 바이너리는 오류 없이 컴파일되어야 합니다: pb_1101_ts, pb_1201_tr, pb_1401_tr, pb_1601_tr, pb_1801_ts, pb_7100_ts, pb_7100_ur, pb_7200_tr, pb_7612_ts, pb_7613_ts, pb_7800_tr, pb_8100_ts 시리즈, pb_8200_tr.

---

## 8. 전체 평가

```
일치율:  28 / 28 = 100.0%  (FR-27 연기 제외)
             28 / 29 = 96.6%   (FR-27을 미완료로 포함)

판정: PASS (>= 90% 기준 충족)
```

모든 CRITICAL 수정사항(배치 1)이 구현되었습니다. 모든 HIGH 우선순위 수정사항(배치 2-4)이 구현되었습니다. 모든 MEDIUM 우선순위 수정사항(배치 5-7)이 구현되었습니다. 모든 LOW 우선순위 수정사항(배치 8)이 구현되었습니다. 유일하게 남은 항목은 의도적으로 연기된 FR-27(C++ 주석 변환)입니다.

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 1.0 | 2026-02-26 | 28+1 FR에 대한 초기 간격 분석 | Claude Code |
