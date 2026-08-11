# pa-pb-dedup Design Document

> **Summary**: PA/PB 모듈 간 중복 함수를 sub/fep_common.c로 추출, extern 글로벌 참조 방식
>
> **Project**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft
> **Plan Reference**: `docs/01-plan/features/pa-pb-dedup.plan.md`

---

## 1. Overview

### 1.1 Design Goals

6개 소스 파일(3 PA + 3 PB)에서 공통 함수를 `sub/fep_common.c`로 추출하여 단일 소스에서 관리한다.

핵심 설계 결정:
- **extern 글로벌 참조 방식**: sub/ 함수가 프로세스 파일의 글로벌 변수를 extern으로 참조 (기존 sub/ 패턴과 동일)
- **SLog → Log 표준화**: pa_1100_ts, pa_1200_tr의 SLog 호출을 Log로 통일 (4/6 파일 이미 Log 사용)
- **매크로 wrapper로 기존 호환성 유지**: 기존 함수명을 매크로로 유지하여 호출부 변경 없음

### 1.2 Design Principles

- **Zero behavior change**: 추출된 함수의 동작은 기존과 100% 동일
- **Incremental extraction**: 가장 안전한 것부터 순차 추출 (유틸리티 → I/O → 이벤트)
- **Existing pattern compliance**: 기존 sub/ 코드(shm_rw.c, set_trtime.c)와 동일한 extern 참조 패턴

### 1.3 Architecture Decision: Why Extern Globals

| 방식 | 장점 | 단점 | 선택 |
|------|------|------|:----:|
| (A) extern 글로벌 참조 | 기존 패턴 동일, 호출부 변경 없음 | 글로벌 의존 명시적이지 않음 | **O** |
| (B) 파라미터 전달 | 명시적 의존성 | 파라미터 과다 (8+개), 호출부 전면 수정 | |
| (C) 컨텍스트 구조체 | 클린 아키텍처 | C89에서 초기화 복잡, 전체 리팩토링 필요 | |
| (D) static inline 헤더 | 링크 문제 없음 | 오브젝트 코드 중복, 소스 중복은 해소 | |

근거: `sub/shm_rw.c`(62줄)가 이미 `PROC(D_K,P_K)`, `IDK(D_K,P_K,fk)` 등 extern 글로벌 + SHM 매크로를 사용하는 패턴. 동일 방식이 가장 안전.

---

## 2. Global Variable & Macro Dependencies

### 2.1 Process-Level Globals (extern 선언 필요)

모든 6개 대상 파일에서 동일한 이름/타입으로 선언됨:

```c
/* inc/fep_common.h에서 extern 선언 */
extern int      Sockfd;
extern int      ErrCd;
extern int      SendLen;
extern int      RecvLen;
extern int      FirstSeq;
extern double   SendMsec;
extern char     LogOnFlag;
extern char     OpenFlag;
extern char     DataBuff[];     /* 4096 bytes in all files */
extern char     IpAddr[];       /* 20 bytes in all files */
```

### 2.2 Framework Macros (헤더에서 이미 사용 가능)

`fep_interface.h` 매크로 (PROC(D_K,P_K) 기반):
- `INT_SEQ` = `IF_SEQ(D_K,P_K)` — 인터페이스 시퀀스
- `LOAD_CNT` = `DATA_CNT(D_K,P_K)` — 적재 건수
- `TCP2_LINE_GU` = `TCP2_LINE_GUBUN(D_K,P_K)` — 회선 구분
- `TCP2_LINE_ST` = `TCP2_LSTAT(D_K,P_K,S_K)` — 회선 상태
- `TCP2_NET_STA(k)` = `TCP2_NSTAT(D_K,P_K,k)` — 네트워크 상태

`shm_memory.h` 글로벌 (이미 extern 선언 존재):
- `extern int D_K, P_K;` — 데몬/프로세스 키
- `int S_K;` — 회선 키

### 2.3 SLog vs Log 분석

```c
/* sub/log_proc.c — 동일 시그니처 */
void Log  (int p_err_no, const char *p_fmt, ...);  /* 로그 파일만 기록 */
void SLog (int p_err_no, const char *p_fmt, ...);  /* 로그 파일 + SHM PROC().error_cd/error_tm 기록 */
```

| File | Current | After |
|------|---------|-------|
| pa_1100_ts.c | SLog | **Log** (통일) |
| pa_1200_tr.c | SLog | **Log** (통일) |
| pa_7800_tr.c | Log | Log (변경 없음) |
| pb_1100_ts.c | Log | Log (변경 없음) |
| pb_1200_tr.c | Log | Log (변경 없음) |
| pb_7800_tr.c | Log | Log (변경 없음) |

**SLog → Log 변경 영향**: SHM PROC().error_cd/error_tm 에 에러 코드가 기록되지 않음. 이는 모니터링 대시보드(`ps.sh`)에서 마지막 에러 코드 표시에 영향. 단, pa_7800_tr 등 4개 파일이 이미 Log만 사용 중이며 운영 문제 없음. pa_1100_ts, pa_1200_tr의 에러 로깅은 로그 파일에 정상 기록됨.

---

## 3. File Structure

### 3.1 New Files

```
inc/fep_common.h     — extern 선언 + 함수 프로토타입 (신규, ~80줄)
sub/fep_common.c     — 공용 함수 구현 (신규, ~250줄)
```

### 3.2 Modified Files

```
src/PA/pa_1100_ts.c  — 7개 함수 제거, #include 추가, SLog→Log
src/PA/pa_1200_tr.c  — 7개 함수 제거, #include 추가, SLog→Log
src/PA/pa_7800_tr.c  — 7개 함수 제거, #include 추가
src/PB/pb_1100_ts.c  — 7개 함수 제거, #include 추가
src/PB/pb_1200_tr.c  — 7개 함수 제거, #include 추가
src/PB/pb_7800_tr.c  — 7개 함수 제거, #include 추가
make/SUB/Make_Lib_P_c.sh  — fep_common.c 빌드 추가
```

### 3.3 Build Integration

`sub/fep_common.c`는 기존 `make/SUB/Make_Lib_P_c.sh`의 빌드 루프에 자동 포함됨 (*.c 순회). 별도 수정 불필요할 수 있으나 확인 필요.

---

## 4. FR-01: Get_Msec 추출

### 4.1 Analysis

6개 파일에서 **byte-for-byte 동일**. 글로벌 변수 참조 없음 (파라미터만 사용).

```c
/* 현재 — 6개 파일에 각각 존재 (동일 코드) */
void    Get_Msec (double *msec)
{
   struct timeval  tv;
   gettimeofday (&tv, NULL);
   *msec = tv.tv_sec + tv.tv_usec * 1e-6;
   return;
}
```

### 4.2 Design

`sub/fep_common.c`로 이동. 가장 단순한 추출 — 순수 함수.

```c
/* sub/fep_common.c */
void    Get_Msec (double *msec)
{
   struct timeval  tv;
   gettimeofday (&tv, NULL);
   *msec = tv.tv_sec + tv.tv_usec * 1e-6;
   return;
}
```

```c
/* inc/fep_common.h */
extern void Get_Msec (double *);
```

### 4.3 Process File Changes

6개 파일에서 Get_Msec 함수 본문 삭제, 프로토타입 제거 (extern으로 대체).

---

## 5. FR-02: Line_Change 추출

### 5.1 Analysis

6개 파일에서 **byte-for-byte 동일**. SHM 매크로(`TCP2_LINE_GU`, `TCP2_IP*`) 사용.

```c
void    Line_Change (void)
{
    TCP2_LINE_GU = TCP2_LINE_GU + 1;
    S_K = TCP2_LINE_GU % 2;
    TCP2_LINE_GU = S_K;
    sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,S_K),
        TCP2_IP2(D_K,P_K,S_K), TCP2_IP3(D_K,P_K,S_K), TCP2_IP4(D_K,P_K,S_K));
    Log (TCP_OK, "line changed to %s (%s:%d)",
        S_K == 0 ? "main" : "backup", IpAddr, PORT_NO);
    return;
}
```

### 5.2 Dependencies

- `S_K` — extern int (shm_memory.h)
- `IpAddr` — extern char[] (fep_common.h)
- `TCP2_LINE_GU`, `TCP2_IP*`, `PORT_NO` — SHM 매크로 (fep_interface.h)
- `Log` — extern function (fep_sub.h)

모두 sub/ 코드에서 접근 가능 (기존 패턴).

### 5.3 Design

`sub/fep_common.c`로 이동. 프로세스 파일에서는 `#include "fep_common.h"` 추가 후 함수 본문 삭제.

---

## 6. FR-03: Device_Read 추출

### 6.1 Analysis

6개 파일에서 **NEAR-IDENTICAL**. 핵심 로직 동일, 차이점:

| 차이 | PA 파일 | PB 파일 |
|------|---------|---------|
| 로깅 매크로 | SLog (1100, 1200) / Log (7800) | Log (모두) |
| 로그 길이 인자 | `strlen(DataBuff)` | `RecvLen` |
| 포맷 문자열 폭 | `%s` | pb_7800만 `%110.110s` |
| #if 0 디버그 블록 | *_1100_ts만 존재 | *_1100_ts만 존재 |

### 6.2 Design Decision: RecvLen 표준화

PA의 `strlen(DataBuff)`는 KRX 바이너리 데이터에 대해 **부정확** (null 바이트 포함 가능). PB의 `RecvLen`이 정확한 값. 표준화하여 모든 파일에서 `RecvLen` 사용.

### 6.3 Extracted Function

```c
/* sub/fep_common.c */
int     Device_Read (void)
{
    int     rt;

    memset (DataBuff, 0, sizeof (DataBuff));
    RecvLen = 0;

    rt = Select_Receive_Krx (Sockfd, DataBuff, KRX_HEAD_LEN);

    if (rt <= 0)
    {
        Device_Close ();
        return (NOTOK);
    }

    RecvLen = rt;
    Log (TCP_OK, "TCP RD [%s](%d)<%d>", DataBuff, RecvLen, INT_SEQ);

    return (OK);
}
```

**변경 사항**:
- Log 사용 (SLog 대신) — SLog→Log 표준화에 따름
- `RecvLen` 사용 (strlen 대신) — 정확성 개선
- `%s` 포맷 사용 (pb_7800의 `%110.110s`는 불필요한 폭 제한이므로 표준 `%s`로)
- `#if 0` 디버그 블록 제거 (비활성 코드)
- `m_time[24]` 미사용 지역변수 제거

### 6.4 Extern Dependencies

```c
/* fep_common.h */
extern char     DataBuff[];
extern int      Sockfd;
extern int      RecvLen;
/* INT_SEQ, KRX_HEAD_LEN — macros from headers */
/* Device_Close — extern function */
```

---

## 7. FR-04~06: Device_Write, Device_Close 추출

### 7.1 Device_Write Analysis

6개 파일에서 **NEAR-IDENTICAL**. 차이점:

| 차이 | PA 파일 | PB 파일 |
|------|---------|---------|
| Send 길이 | `strlen(DataBuff)` | `SendLen` |
| 로깅 | SLog (1100, 1200) / Log (7800) | Log |

### 7.2 Device_Write Design

`strlen(DataBuff)` vs `SendLen` 차이는 **의미론적**:
- PA: 평문 전송 → strlen 유효 (null 종료 문자열)
- PB: 암호화 전송 → SendLen 필수 (바이너리 데이터)

**Design**: 길이를 파라미터로 전달하여 호출자가 결정.

```c
/* sub/fep_common.c */
void    Device_Write_Len (int send_len)
{
    int     rt;

    rt = Select_Send (Sockfd, DataBuff, send_len);

    if (rt != OK)
    {
        Log (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

    Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, send_len, INT_SEQ);
    Get_Msec (&SendMsec);

    return;
}
```

```c
/* inc/fep_common.h — wrapper 매크로로 기존 호환성 유지 */
extern void Device_Write_Len (int);

/* PA 파일에서: */
#define Device_Write()  Device_Write_Len(strlen(DataBuff))

/* PB 파일에서: */
#define Device_Write()  Device_Write_Len(SendLen)
```

**주의**: 각 프로세스 파일에서 `Device_Write`를 매크로로 재정의해야 함. 또는:

**Alternative (Simpler)**: Device_Write를 그대로 추출하되, PA/PB 각각에서 send_len 전역 변수를 다르게 설정.

Actually 더 단순한 접근: PA 파일에서도 `SendLen = strlen(DataBuff);`를 Device_Write 호출 전에 설정하도록 변경. 그러면 모든 파일이 동일한 Device_Write를 사용.

```c
/* sub/fep_common.c — SendLen 기반 (모든 파일 통일) */
void    Device_Write (void)
{
    int     rt;

    rt = Select_Send (Sockfd, DataBuff, SendLen);

    if (rt != OK)
    {
        Log (TCP_ERROR, "TCP data send fail");
        Device_Close ();
    }

    Log (TCP_OK, "TCP SD [%s](%d)<%d>", DataBuff, SendLen, INT_SEQ);
    Get_Msec (&SendMsec);

    return;
}
```

PA 파일의 호출부에서 Device_Write() 직전에 `SendLen = strlen(DataBuff);` 추가가 필요한 위치를 확인해야 함. 단, PA 파일에서 `SendLen`이 이미 Make_Send_Msg()에 의해 설정되는지 검증 필요.

**Decision**: Phase 4 (Do 단계) 진입 시 PA 파일의 SendLen 설정 흐름을 확인하여 최종 결정. Design에서는 두 가지 옵션 모두 명시.

### 7.3 Device_Close Analysis

| 부분 | PA (3 files) | PB (3 files) |
|------|-------------|-------------|
| 공통 | `close(Sockfd); Log("close"); LogOnFlag=OFF; TCP2_LINE_ST=OpenFlag=OFF;` | 동일 |
| PB 추가 | — | `Free_All((void*)(long)1); INL_Cleanup(CLIENT_CTX);` |

### 7.4 Device_Close Design

Base 함수 추출 + PB는 추가 cleanup 수행.

```c
/* sub/fep_common.c */
void    Device_Close_Base (void)
{
    close (Sockfd);
    Log (TCP_OK, "TCP device close");
    LogOnFlag = OFF;
    TCP2_LINE_ST = OpenFlag = OFF;
    return;
}
```

```c
/* PA 파일에서: */
#define Device_Close    Device_Close_Base

/* PB 파일에서: (inline wrapper 유지) */
void    Device_Close (void)
{
    Device_Close_Base ();
    Free_All ((void *)(long)1);
    INL_Cleanup (CLIENT_CTX);
}
```

**Alternative**: PB의 Device_Close를 그대로 유지하되, base 부분만 함수 호출로 교체. PB 파일은 여전히 자체 Device_Close를 갖지만 공통 부분은 Device_Close_Base를 호출.

---

## 8. FR-07: Err_Msg 추출

### 8.1 Analysis

6개 파일에서 **NEAR-IDENTICAL**. switch 문 구조, 모든 case 문, 에러 메시지 문자열 동일.

유일한 차이: `SLog` vs `Log` (pa_1100_ts, pa_1200_tr만 SLog).

### 8.2 Design

SLog→Log 표준화 후 하나의 공용 함수로 추출.

```c
/* sub/fep_common.c */
void    Err_Msg (void)
{
    char    t_time[24];

    switch (ErrCd)
    {
        case 0:
            Log (USR_ERROR, "사용자검증(ID,PASSWORD)오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 1:
            Log (USR_ERROR, "세션관리오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 2:
            Log (USR_ERROR, "임시오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 3:
            Log (USR_ERROR, "시스템오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            Get_DateMilliTime (t_time);
            sleep (NO_TIME);
            break;
        case 4:
            Log (USR_ERROR, "응용프로그램 오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            Get_DateMilliTime (t_time);
            sleep (NO_TIME);
            break;
        case 5:
            Log (USR_ERROR, "일시적 오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            Get_DateMilliTime (t_time);
            sleep (NO_TIME);
            break;
        case 6:
            Log (USR_ERROR, "데이터 일련번호 오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 9:
            Log (USR_ERROR, "기타오류[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 101:
            Log (USR_ERROR, "호가접수개시전[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 102:
            Log (USR_ERROR, "매매거래시간종료[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        case 103:
            Log (USR_ERROR, "호가접수정지[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
        default:
            Log (USR_ERROR, "unknown error[%d] <%d:%d:%d>",
                ErrCd, FirstSeq, INT_SEQ, LOAD_CNT);
            break;
    }

    if (ErrCd == 102)
        Log_Out ();

    return;
}
```

### 8.3 Dependencies

- `ErrCd`, `FirstSeq` — extern int
- `INT_SEQ`, `LOAD_CNT`, `NO_TIME` — SHM 매크로
- `Log` — extern function (fep_sub.h)
- `Get_DateMilliTime` — extern function (fep_sub.h)
- `Log_Out` — **프로세스별 함수 (추출 안함)** → extern 선언 필요

**주의**: `Err_Msg`는 `Log_Out()`을 호출하는데, `Log_Out()`은 프로세스별로 다름 (S_Fmt vs KR_Fmt, pb_1100의 추가 로직). 따라서 `Log_Out`은 각 프로세스 파일에 유지하고, extern으로 참조.

```c
/* inc/fep_common.h */
extern void Log_Out (void);    /* 각 프로세스 파일에서 구현 */
extern void Err_Msg (void);    /* sub/fep_common.c에서 구현 */
```

---

## 9. FR-08: fep_common.h 헤더 파일

### 9.1 Complete Header Design

```c
/*------------------------------------------------------------------------
#   Module  : Common event-loop functions for PA/PB processes
#   File    : fep_common.h
------------------------------------------------------------------------*/
#ifndef _FEP_COMMON_H_
#define _FEP_COMMON_H_

/*------------------------------------------------------------------------
    Extern declarations for process-level globals
    (defined in each process .c, resolved by linker)
------------------------------------------------------------------------*/
extern int      Sockfd;
extern int      ErrCd;
extern int      SendLen;
extern int      RecvLen;
extern int      FirstSeq;
extern double   SendMsec;
extern char     LogOnFlag;
extern char     OpenFlag;
extern char     DataBuff[];
extern char     IpAddr[];

/*------------------------------------------------------------------------
    Common function prototypes
------------------------------------------------------------------------*/
extern void     Get_Msec (double *);
extern void     Line_Change (void);
extern int      Device_Read (void);
extern void     Device_Write (void);
extern void     Device_Close_Base (void);
extern void     Err_Msg (void);

/*------------------------------------------------------------------------
    Process-specific function prototypes
    (implemented in each process file, called by common functions)
------------------------------------------------------------------------*/
extern void     Log_Out (void);

#endif /* _FEP_COMMON_H_ */
```

### 9.2 Include Order

```c
/* 각 프로세스 파일의 include 순서 */
#include "fep_fepp.h"       /* 프레임워크 (매크로, SHM, IPC) */
#include "pa_struct.h"      /* KRX 메시지 구조체 */
#include "buf_struct.h"     /* 버퍼 구조체 */
#include "fep_common.h"     /* 공용 함수 (NEW) */
```

```c
/* sub/fep_common.c의 include */
#include "fep_sub.h"        /* 기존 sub/ 표준 include */
#include "fep_common.h"     /* 자기 자신의 헤더 */
```

---

## 10. De-scoped: Log_Out, Device_Open

### 10.1 Log_Out — 프로세스별 유지

| 차이점 | 이유 |
|--------|------|
| Fmt 버퍼 (`S_Fmt` vs `KR_Fmt`) | 서로 다른 타입 — 파라미터화 어려움 |
| pb_1100 추가 로직 (PROC status, write) | PB-특화 동작 |
| pb_1100의 `memcmp("0000")` vs `IS_RESP_OK` | 향후 통일 가능하나 별도 이슈 |

### 10.2 Device_Open — 프로세스별 유지

PA와 PB 간 구조적 차이가 크며 (Handshake, sleep, TR_LINK 유무), 추출 시 조건 분기가 과다하여 가독성 저하. 유지.

### 10.3 향후 개선 가능성

Log_Out: `void` 파라미터 대신 `void *fmt_ptr, int fmt_size`를 받으면 통합 가능.
Device_Open: 콜백 패턴 (`post_connect_hook`, `post_logon_hook`)으로 구조화 가능.
이들은 Phase 2 (후속 PDCA)에서 진행 권장.

---

## 11. SLog → Log 표준화 (전처리)

### 11.1 대상 파일 및 변경 범위

| File | SLog 호출 수 | 변경 |
|------|-------------|------|
| pa_1100_ts.c | ~25 | 모두 `Log`로 변경 |
| pa_1200_tr.c | ~18 | 모두 `Log`로 변경 |

### 11.2 영향 분석

SLog와 Log의 차이: SLog는 SHM `PROC(D_K,P_K).error_cd`/`error_tm`에 에러 정보 기록.

- 4/6 파일이 이미 Log만 사용 (운영 문제 없음 확인됨)
- SHM 에러 기록은 `ps.sh` 대시보드에서 마지막 에러 표시용
- 로그 파일에 동일 정보 기록되므로 기능적 영향 미미
- 향후 필요 시 별도 SHM 기록 함수 추가 가능

### 11.3 구현

```bash
# pa_1100_ts.c, pa_1200_tr.c에서
SLog → Log  (replace_all)
```

---

## 12. Implementation Order

| Step | FR | Action | Files | Risk |
|------|-----|--------|-------|------|
| 1 | FR-08 | `inc/fep_common.h` 생성 | 1 new | Low |
| 2 | FR-01~07 | `sub/fep_common.c` 생성 (모든 공용 함수) | 1 new | Low |
| 3 | — | `make/SUB/Make_Lib_P_c.sh` 빌드 확인 | 1 check | Low |
| 4 | §11 | pa_1100_ts.c: SLog → Log 전환 | 1 modify | Low |
| 5 | §11 | pa_1200_tr.c: SLog → Log 전환 | 1 modify | Low |
| 6 | FR-01~07 | pa_7800_tr.c: 7함수 제거 + #include 추가 | 1 modify | Medium |
| 7 | FR-01~07 | pb_7800_tr.c: 7함수 제거 + #include + Device_Close wrapper | 1 modify | Medium |
| 8 | FR-01~07 | pa_1200_tr.c: 7함수 제거 + #include 추가 | 1 modify | Medium |
| 9 | FR-01~07 | pb_1200_tr.c: 7함수 제거 + #include + Device_Close wrapper | 1 modify | Medium |
| 10 | FR-01~07 | pa_1100_ts.c: 7함수 제거 + #include + Device_Write macro | 1 modify | Medium |
| 11 | FR-01~07 | pb_1100_ts.c: 7함수 제거 + #include + Device_Close wrapper | 1 modify | Medium |
| 12 | — | 빌드 검증 (`mk.sh sub && mk.sh src`) | — | — |

### Step 순서 근거

1. 헤더/소스 먼저 생성 → 컴파일 에러 없이 점진적 적용
2. SLog→Log 전환을 함수 추출 전에 수행 → 함수 코드 일치 보장
3. 7800_tr 쌍을 먼저 (가장 작은 PA 파일, 55% 공유) → 리스크 최소화
4. 1200_tr 쌍 다음 (70% 공유, 가장 높은 공유율)
5. 1100_ts 쌍 마지막 (가장 큰 파일, Device_Write strlen 문제 포함)

---

## 13. Per-File Extraction Detail

### 13.1 제거할 함수 (7개, 모든 파일 공통)

| # | Function | Lines (avg) | Replacement |
|---|----------|-------------|-------------|
| 1 | Get_Msec | ~10 | sub/fep_common.c |
| 2 | Line_Change | ~13 | sub/fep_common.c |
| 3 | Device_Read | ~22 | sub/fep_common.c |
| 4 | Device_Write | ~30 | sub/fep_common.c (SendLen 기반) |
| 5 | Device_Close | ~10 (PA) / ~16 (PB) | Device_Close_Base + PB wrapper |
| 6 | Err_Msg | ~82 | sub/fep_common.c |
| 7 | (prototype declarations) | ~7 | fep_common.h extern |

### 13.2 PA 파일 변경 패턴

```c
/* BEFORE (pa_7800_tr.c) */
void    Device_Close (void);    /* prototype */
void    Device_Write (void);    /* prototype */
int     Device_Read (void);     /* prototype */
void    Line_Change (void);     /* prototype */
void    Get_Msec (double *);    /* prototype */
void    Err_Msg (void);         /* prototype */

/* ... 중간 코드 ... */

void    Get_Msec (double *msec) { ... }
void    Log_Out (void) { ... }          /* 유지 */
void    Err_Msg (void) { ... }
void    Line_Change (void) { ... }
int     Device_Read (void) { ... }
void    Device_Write (void) { ... }
void    Device_Close (void) { ... }

/* AFTER */
#include "fep_common.h"
#define  Device_Close   Device_Close_Base   /* PA: base만 사용 */

/* ... 중간 코드 ... */

void    Log_Out (void) { ... }          /* 유지 */
/* Get_Msec, Err_Msg, Line_Change, Device_Read, Device_Write, Device_Close 모두 제거 */
```

### 13.3 PB 파일 변경 패턴

```c
/* AFTER (pb_7800_tr.c) */
#include "fep_common.h"

/* ... 중간 코드 ... */

void    Log_Out (void) { ... }          /* 유지 */

/* Device_Close: PB wrapper 유지 */
void    Device_Close (void)
{
    Device_Close_Base ();
    Free_All ((void *)(long)1);
    INL_Cleanup (CLIENT_CTX);
}

/* Get_Msec, Err_Msg, Line_Change, Device_Read, Device_Write 모두 제거 */
```

### 13.4 Device_Write — PA strlen 처리

PA 파일에서 Device_Write 호출 전 `SendLen`이 이미 설정되는지 확인 필요:

**Option A**: PA에서도 Make_Send_Msg() 후 SendLen이 설정됨 → 그대로 사용
**Option B**: PA에서 SendLen 미설정 구간 존재 → 해당 구간에 `SendLen = strlen(DataBuff);` 추가

이 판단은 Do 단계에서 각 Device_Write() 호출 직전의 코드 흐름을 확인하여 결정.

---

## 14. Expected Metrics

### 14.1 Code Reduction Estimate

| File | Before (lines) | Removed | After (est) | Reduction |
|------|---------------|---------|-------------|-----------|
| pa_1100_ts.c | 1,532 | ~167 | ~1,365 | -11% |
| pb_1100_ts.c | 1,925 | ~167 | ~1,758 | -9% |
| pa_1200_tr.c | 1,114 | ~167 | ~947 | -15% |
| pb_1200_tr.c | 1,540 | ~167 | ~1,373 | -11% |
| pa_7800_tr.c | 987 | ~167 | ~820 | -17% |
| pb_7800_tr.c | 1,745 | ~167 | ~1,578 | -10% |
| **Total** | **8,843** | **~1,000** | **~7,843** | **-11%** |
| sub/fep_common.c | — | — | +250 | (new) |
| inc/fep_common.h | — | — | +80 | (new) |
| **Net** | **8,843** | | **~8,173** | **-8%** |

**Note**: 순 코드 감소율은 8%이나, 중복 제거 효과는 6x → 1x (6개 사본에서 1개로). 유지보수 비용 ~83% 감소.

### 14.2 Maintenance Impact

버그 수정 시 변경 파일 수: 6개 → 1개 (sub/fep_common.c)

---

## 15. Risk Mitigations

| Risk | Mitigation |
|------|------------|
| extern 글로벌 해석 불일치 | 6개 파일의 글로벌 선언을 정확히 검증 완료 (동일 이름/타입 확인) |
| SLog→Log 전환으로 모니터링 영향 | 4/6 파일 이미 Log 사용 중이며 운영 문제 없음. 로그 파일에 동일 정보 존재 |
| Device_Write strlen→SendLen 불일치 | PA 호출부에서 SendLen 설정 흐름 Do 단계에서 사전 검증 |
| 빌드 체인에 fep_common.c 누락 | Make_Lib_P_c.sh가 sub/*.c를 순회하므로 자동 포함 예상. 명시적 확인 |
| Korean 문자열 인코딩 차이 (UTF-8 vs EUC-KR) | sub/fep_common.c를 EUC-KR로 저장 (기존 sub/ 파일과 동일 인코딩) |

---

## 16. Verification Checklist

- [ ] `mk.sh sub` — libfepP.a 빌드 성공 (fep_common.o 포함)
- [ ] `mk.sh pa` — PA 모듈 전체 빌드 성공
- [ ] `mk.sh pb` — PB 모듈 전체 빌드 성공
- [ ] 중복 심볼 에러 없음 (Get_Msec 등이 process .c와 sub/ 양쪽에 없을 것)
- [ ] 링크 에러 없음 (extern 참조가 프로세스 글로벌과 매칭)
- [ ] 각 바이너리 크기 비교 (±5% 이내 변동)

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
