# pa-pb-dedup-phase2 Design Document

> **Summary**: PA/PB 모듈 중복 제거 2차 — FmtPtr 패턴 도입, Log_Out/Handshake/Free_All 추출, Device_Open LOGON 추출
>
> **Project**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft
> **Plan Reference**: `docs/01-plan/features/pa-pb-dedup-phase2.plan.md`

---

## 1. Overview

### 1.1 Design Goals

Phase 1에서 de-scoped된 함수들과 추가 발견된 중복 함수를 추출한다. 핵심 기술: **FmtPtr 패턴** — `S_Fmt` vs `KR_Fmt` 차이를 `extern void *FmtPtr` 글로벌로 해결하여 Log_Out, Handshake, Device_Open 모두에서 공통 memcpy 가능.

### 1.2 Design Principles

- **Phase 1 패턴 준수**: extern 글로벌 참조, wrapper function, fep_common.c 확장
- **실제 소스 코드 기반 설계**: Phase 1 Err_Msg 교훈 — pseudocode 대신 실제 코드 직접 인용
- **INISAFE 의존성 분리**: 암호화 함수(Handshake, Free_All)는 별도 `sub/fep_encrypt.c`에 배치

### 1.3 Scope Change from Plan

| Plan FR | Status | Reason |
|---------|--------|--------|
| FR-01 Log_Out | IN SCOPE | FmtPtr로 Fmt 버퍼 차이 해결 |
| FR-02 Time_Out_Rtn | **DE-SCOPED** | 4가지 변형 발견, 추출 가치 대비 복잡도 과다 |
| FR-03 Device_Open LOGON | IN SCOPE | FmtPtr + 2개 boolean 플래그로 추출 가능 |
| FR-04 PB Handshake | IN SCOPE | FmtPtr로 추출 가능, sub/fep_encrypt.c |
| FR-05 PB Free_All | IN SCOPE | byte-for-byte identical, sub/fep_encrypt.c |
| FR-06 Header extension | IN SCOPE | fep_common.h + fep_encrypt.h 신규 |
| NEW: FmtPtr pattern | ADDED | 모든 추출의 핵심 enabler |

### 1.4 Time_Out_Rtn De-scope 근거

6개 파일 실제 코드 분석 결과 4가지 변형 존재:

| 변형 | 파일 | 핵심 차이 |
|------|------|----------|
| A | pa_1100_ts = pb_1100_ts | case 2: reconnect("KRX"), case 3: heartbeat(S_Fmt) |
| B | pa_1200_tr = pb_1200_tr | case 1: FOREVER_TIME log, case 2+3: reconnect("FOT") |
| C | pa_7800_tr | case 1: FOREVER_TIME log, case 2: heartbeat(Reply/KRX_JUMUN_R_FMT) |
| D | pb_7800_tr | case 1: FOREVER_TIME log, case 2: 모든 코드 주석 처리(비활성) |

PA/PB 쌍 내에서는 동일하지만 (A=A, B=B), 3개 쌍 간 로직이 다름. 추출 시 flag 파라미터 4개+ 필요, 순 절감 ~92줄. 향후 Phase 3에서 재검토.

---

## 2. Key Architecture: FmtPtr Pattern

### 2.1 Problem

Log_Out, Handshake, Device_Open 모두 동일한 패턴:
```c
Make_Send_Msg (TR_XXXX);
memset (DataBuff, 0, sizeof (DataBuff));
memcpy (DataBuff, &S_Fmt, SendLen);    /* ← 이 줄만 다름: S_Fmt vs KR_Fmt */
Device_Write ();
```

| 파일 | Fmt 버퍼 | 구조체 타입 |
|------|----------|-----------|
| pa_1100_ts, pb_1100_ts | `&S_Fmt` | `KRX_SESSION_FMT` (82+41=123 bytes) |
| pa_1200_tr, pa_7800_tr, pb_1200_tr, pb_7800_tr | `&KR_Fmt` | `KRX_R_SESSION_FMT` (82+116=198 bytes) |

### 2.2 Solution: extern void *FmtPtr

```c
/* inc/fep_common.h — 추가 */
extern void    *FmtPtr;

/* 각 프로세스 파일에서 정의 */
/* pa_1100_ts.c, pb_1100_ts.c: */
void *FmtPtr = (void *)&S_Fmt;

/* pa_1200_tr.c, pa_7800_tr.c, pb_1200_tr.c, pb_7800_tr.c: */
void *FmtPtr = (void *)&KR_Fmt;
```

공유 코드에서:
```c
memcpy (DataBuff, FmtPtr, SendLen);  /* S_Fmt/KR_Fmt 무관하게 동작 */
```

**근거**: `memcpy`는 바이트 단위 복사. `SendLen`이 실제 크기를 결정하므로 구조체 타입 무관. C89에서 `&S_Fmt`는 주소 상수(address constant)로 정적 초기화 유효.

---

## 3. FR-01: Log_Out Extraction

### 3.1 실제 소스 코드 비교

**5/6 파일 공통 코드** (pa_1200_tr 대표, pa_7800_tr/pb_1200_tr/pb_7800_tr 동일):
```c
void    Log_Out (void)
{
    int     rt, rval;

    rt = Make_Send_Msg (TR_LOOU);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, &KR_Fmt, SendLen);       /* ← FmtPtr로 대체 */
    Device_Write ();
    Log (USR_OK, "send LOGOUT request");

    rval = Device_Read ();
    if (rval < 0)
    {
        Log (USR_ERROR, "LOGOUT response recv error");
        close (Sockfd);
        return;
    }

    if (IS_RESP_OK(DataBuff))
    {
        Log (USR_OK, "LOGOUT success");
        Device_Close ();
        TCP2_NET_STA(S_K) = END;
    }
    else
    {
        ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg ();
    }

    return;
}
```

**pa_1100_ts.c** — KR_Fmt 대신 S_Fmt 사용, 나머지 동일.

**pb_1100_ts.c** — 3가지 차이:
1. `memcpy (DataBuff, &S_Fmt, SendLen)` (S_Fmt 사용)
2. `memcmp(&DataBuff[KRX_HEAD_LEN], "0000", 4) == 0` (IS_RESP_OK 대신 직접 비교 — 기능 동일)
3. 성공 분기에 추가 코드:
```c
    /* 2025 추가 */
    PROC(D_K,P_K).start_status = JOB_END;
    PROC(D_K,P_K).process_status = 2;
    write (DTART_FD, "1", 1);
```

### 3.2 추출 설계

**공유 함수**: `Log_Out_Base()` — `int` 반환 (성공=0, 실패=-1)

```c
/* sub/fep_common.c */
int     Log_Out_Base (void)
{
    int     rt, rval;

    rt = Make_Send_Msg (TR_LOOU);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, FmtPtr, SendLen);
    Device_Write ();
    Log (USR_OK, "send LOGOUT request");

    rval = Device_Read ();
    if (rval < 0)
    {
        Log (USR_ERROR, "LOGOUT response recv error");
        close (Sockfd);
        return (NOTOK);
    }

    if (IS_RESP_OK(DataBuff))
    {
        Log (USR_OK, "LOGOUT success");
        Device_Close ();
        TCP2_NET_STA(S_K) = END;
        return (OK);
    }
    else
    {
        ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg ();
        return (NOTOK);
    }
}
```

### 3.3 프로세스 파일 변경

**5/6 파일** (pa_1100_ts, pa_1200_tr, pa_7800_tr, pb_1200_tr, pb_7800_tr):
```c
void    Log_Out (void)
{
    Log_Out_Base ();
}
```

**pb_1100_ts.c**:
```c
void    Log_Out (void)
{
    if (Log_Out_Base () == OK)
    {
        PROC(D_K,P_K).start_status = JOB_END;
        PROC(D_K,P_K).process_status = 2;
        write (DTART_FD, "1", 1);
    }
}
```

### 3.4 Dependencies

```c
/* fep_common.h에 추가 */
extern int      Log_Out_Base (void);
extern void    *FmtPtr;

/* fep_common.c에서 필요한 extern (이미 존재하는 것 외) */
extern int      Make_Send_Msg (int);    /* 프로세스 파일에서 구현 */
extern void    *FmtPtr;                 /* 프로세스 파일에서 정의 */
/* TR_LOOU, IS_RESP_OK, KRX_ERRCODE_LEN — fep_interface.h 매크로 */
/* TCP2_NET_STA, END — fep_interface.h/shm_memory.h 매크로 */
```

---

## 4. FR-03: Device_Open LOGON Extraction

### 4.1 실제 소스 코드 비교

**6개 파일의 TR_LOON 부분 공통 구조**:
```
Socket() → Connect() → [PB: Handshake()] → Make_Send_Msg(TR_LOON) →
memcpy(FmtPtr) → Device_Write() → Device_Read() → SCHLIR00000 검증 →
IS_RESP_OK 검증 → LogOnFlag=ON → Poll 설정 → [7800: OpenFlag=ON]
```

**차이점 상세**:

| 차이 | PA 파일 | PB 파일 |
|------|---------|---------|
| Handshake 호출 | 없음 | `Handshake()` 호출 |
| Connect 실패 후 sleep | 없음 | `sleep(5)` |
| LOGON 후 즉시 OpenFlag | 7800만 ON | 7800만 ON |
| Response check | IS_RESP_OK (대부분) | pb_1100만 memcmp "0000" (기능 동일) |

### 4.2 추출 설계

2개 boolean 플래그: `has_encrypt` (PB 암호화), `immediate_open` (7800 모드)

```c
/* sub/fep_common.c */
int     Device_Open_Logon (int has_encrypt, int immediate_open)
{
    int     rt, rval;

    Sockfd = Socket ();

    if (Sockfd < 0)
    {
        Log (TCP_ERROR, "socket fail:Sockfd[%d] {%d:%s}",
            Sockfd, SYS_NO, SYS_STR);
        return (NOTOK);
    }

    Log (USR_OK, "socket created:Sockfd[%d]", Sockfd);
    Log (USR_OK, "connecting to %s:%d", IpAddr, TCP2_PORT_NO);

    rt = Connect (Sockfd, IpAddr, TCP2_PORT_NO);

    if (rt < 0)
    {
        Log (TCP_ERROR, "connect fail {%d:%s}", SYS_NO, SYS_STR);
        close (Sockfd);
        if (has_encrypt)
            sleep (5);
        return (NOTOK);
    }

    if (has_encrypt)
    {
        if (Handshake () < 0)
        {
            close (Sockfd);
            return (NOTOK);
        }
    }

    rt = Make_Send_Msg (TR_LOON);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, FmtPtr, SendLen);
    Device_Write ();
    Log (USR_OK, "send LOGON request");

    rval = Device_Read ();
    if (rval < 0 ||
        memcmp (((KRX_HEADER *)DataBuff)->MsgType, "SCHLIR00000", 11) != 0)
    {
        Log (USR_ERROR, "LOGON response recv error");
        close (Sockfd);
        return (NOTOK);
    }

    if (IS_RESP_OK(DataBuff))
    {
        Log (USR_OK, "LOGON success");
    }
    else
    {
        ErrCd = AtoIf (&DataBuff[KRX_HEAD_LEN], KRX_ERRCODE_LEN);
        Err_Msg ();
        close (Sockfd);
        return (NOTOK);
    }

    LogOnFlag = ON;

    if (immediate_open)
    {
        TCP2_NET_STA(S_K) = ON;
        TCP2_LINE_ST = OpenFlag = ON;
        ConnectRetryCnt = 0;
    }

    Poll[1].fd = Sockfd;
    Poll[1].events = POLLIN;
    Log (TCP_OK, "TCP Connect & LOGON OK");

    return (OK);
}
```

### 4.3 프로세스 파일 변경

각 파일의 `Device_Open(int tr_code)` 내 `if (tr_code == TR_LOON)` 블록을 한 줄로 교체:

| 파일 | 호출 |
|------|------|
| pa_1100_ts.c | `Device_Open_Logon (0, 0);` |
| pa_1200_tr.c | `Device_Open_Logon (0, 0);` |
| pa_7800_tr.c | `Device_Open_Logon (0, 1);` |
| pb_1100_ts.c | `Device_Open_Logon (1, 0);` |
| pb_1200_tr.c | `Device_Open_Logon (1, 0);` |
| pb_7800_tr.c | `Device_Open_Logon (1, 1);` |

TR_LINK 처리는 각 프로세스 파일에 그대로 유지.

### 4.4 Dependencies

```c
/* fep_common.h에 추가 */
extern int      Device_Open_Logon (int, int);

/* fep_common.c에서 필요한 extern (신규) */
extern int      ConnectRetryCnt;
extern struct   pollfd  Poll[];
extern int      Handshake (void);       /* PB에서만 제공 */
```

**주의**: `Handshake`는 PA 바이너리에서는 호출되지 않지만 (`has_encrypt=0`), 심볼이 fep_common.o에 존재. PA 링크 시 unresolved symbol 방지를 위해:
- **Option A**: PA 파일에 `int Handshake(void) { return -1; }` 스텁 추가
- **Option B**: `has_encrypt` 체크를 함수 포인터로 변경
- **Option C**: Device_Open_Logon을 fep_common.c가 아닌 각 프로세스에 inline

**Decision**: **Option A** (스텁 추가) — 가장 단순. PA 3개 파일에 1줄 스텁 추가.

```c
/* PA 파일 (pa_1100_ts.c, pa_1200_tr.c, pa_7800_tr.c)에 추가 */
int     Handshake (void) { return (NOTOK); }    /* PA: no encryption */
```

---

## 5. FR-04: Handshake Extraction

### 5.1 실제 소스 코드 비교

3개 PB 파일에서 **구조 동일**, 차이:

| 차이 | pb_1100_ts | pb_1200_tr, pb_7800_tr |
|------|-----------|----------------------|
| memcpy 소스 | `&S_Fmt` | `&KR_Fmt` |
| Response check | `memcmp(&DataBuff[sizeof(KRX_HEADER)], "0000", 4)` | `memcmp(&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN)` |

**기능적으로 동일**: `sizeof(KRX_HEADER) == KRX_HEAD_LEN == 82`, `"0000" == RESP_SUCCESS`, `4 == KRX_ERRCODE_LEN`

### 5.2 추출 설계

FmtPtr로 memcpy 차이 해결. Response check는 매크로/상수 버전으로 표준화.

```c
/* sub/fep_encrypt.c */
#include "fep_fepp.h"
#include "fep_common.h"
#include "fep_encrypt.h"

int     Handshake (void)
{
    int     result;
    int     hl;

    result = INL_Initialize (CLIENT_CTX, KRX_INITECH_CONF_PATH, NULL);
    if (result != 0)
    {
        Log (TCP_ERROR, "INL_Initialize Failed. [%d:%s]",
            result, INL_Error_String (result));
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    result = INL_New_Ctx (CLIENT_CTX, &EnCtx);
    if (result != 0)
    {
        Log (TCP_ERROR, "INL_CtxNew Failed. [%d:%s]",
            result, INL_Error_String (result));
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    /* InitHandShake */
    result = INL_Handshake_Init (EnCtx, NULL, 0, &cinitout, &cinitoutl);
    if (result != 0)
    {
        Log (TCP_ERROR, "Client Init HandShake Failed. [%d:%s]",
            result, INL_Error_String (result));
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg (TR_HSI);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, FmtPtr, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake init request");

    result = Device_Read ();
    if (result < 0)
    {
        Log (TCP_ERROR, "No Handshake init response from Server[%d]", result);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    hl = RecvLen - KRX_HEAD_LEN - KRX_ERRCODE_LEN;
    if (memcmp (((KRX_HEADER *)DataBuff)->MsgType, "SCHLIQ00102", 11) != 0 ||
        memcmp (&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
    {
        Log (TCP_ERROR, "Handshake init response error[%.11s:%.4s]",
            ((KRX_HEADER *)DataBuff)->MsgType, &DataBuff[KRX_HEAD_LEN]);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    sinitout = (unsigned char *)malloc (hl + 1);
    if (!sinitout)
    {
        Log (TCP_ERROR, "sinitout malloc fail size = [%d]", hl);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    memset (sinitout, '\0', hl + 1);
    memcpy (sinitout, &DataBuff[KRX_HEAD_LEN + KRX_ERRCODE_LEN], hl);

    /* UpdateHandShake */
    result = INL_Handshake_Update (EnCtx, sinitout, hl, &cupdateout, &cupdateoutl);
    if (result != 0)
    {
        Log (TCP_ERROR, "Client Update HandShake Failed. [%d:%s]",
            result, INL_Error_String (result));
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg (TR_HSU);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, FmtPtr, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake update request");

    result = Device_Read ();
    if (result < 0)
    {
        Log (TCP_ERROR, "No Handshake update  response from Server[%d]", result);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    hl = RecvLen - KRX_HEAD_LEN - KRX_ERRCODE_LEN;
    if (memcmp (((KRX_HEADER *)DataBuff)->MsgType, "SCHLIQ00104", 11) != 0 ||
        memcmp (&DataBuff[KRX_HEAD_LEN], RESP_SUCCESS, KRX_ERRCODE_LEN) != 0)
    {
        Log (TCP_ERROR, "Handshake update response error[%.11s:%.4s]",
            ((KRX_HEADER *)DataBuff)->MsgType, &DataBuff[KRX_HEAD_LEN]);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    supdateout = (unsigned char *)malloc (hl + 1);
    if (!supdateout)
    {
        Log (TCP_ERROR, "supdateout malloc fail size = [%d]", hl);
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    memset (supdateout, '\0', hl + 1);
    memcpy (supdateout, &DataBuff[KRX_HEAD_LEN + KRX_ERRCODE_LEN], hl);

    /* FinalHandShake */
    result = INL_Handshake_Final (EnCtx, supdateout, hl, &cfinalout, &cfinaloutl);
    if (result != 0)
    {
        Log (TCP_ERROR, "Client Final HandShake Failed. [%d:%s]",
            result, INL_Error_String (result));
        Free_All ((void *)(long)1);
        return (NOTOK);
    }

    result = Make_Send_Msg (TR_HSF);
    memset (DataBuff, 0, sizeof (DataBuff));
    memcpy (DataBuff, FmtPtr, SendLen);
    Device_Write ();
    Log (USR_OK, "send Handshake final request");

    Free_All (0);

    return (OK);
}
```

### 5.3 표준화 항목

| 원본 | 표준화 |
|------|--------|
| `&S_Fmt` / `&KR_Fmt` | `FmtPtr` |
| `sizeof(KRX_HEADER)` | `KRX_HEAD_LEN` |
| `"0000"` / `RESP_SUCCESS` | `RESP_SUCCESS` |
| `4` / `KRX_ERRCODE_LEN` | `KRX_ERRCODE_LEN` |
| `sizeof(KRX_HEADER) + 4` | `KRX_HEAD_LEN + KRX_ERRCODE_LEN` |
| `//Free_All(1)` 주석 | 제거 (정리) |

---

## 6. FR-05: Free_All Extraction

### 6.1 실제 소스 코드

3개 PB 파일에서 **byte-for-byte identical** (들여쓰기만 다름):

```c
/* sub/fep_encrypt.c */
void    Free_All (void *ctxf)
{
    if (cinitout)
    {
        INL_Free_Buf (cinitout);
        cinitout = NULL;
    }
    if (cupdateout)
    {
        INL_Free_Buf (cupdateout);
        cupdateout = NULL;
    }
    if (cfinalout)
    {
        INL_Free_Buf (cfinalout);
        cfinalout = NULL;
    }
    if (sinitout)
    {
        INL_Free_Buf (sinitout);
        sinitout = NULL;
    }
    if (supdateout)
    {
        INL_Free_Buf (supdateout);
        supdateout = NULL;
    }

    if (ctxf && EnCtx)
    {
        INL_Free_Ctx (EnCtx);
        EnCtx = NULL;
    }
}
```

### 6.2 INISAFE 글로벌 변수 (extern 선언 필요)

```c
/* inc/fep_encrypt.h */
extern net_ctx          EnCtx;
extern unsigned char   *cinitout;
extern unsigned char   *cupdateout;
extern unsigned char   *cfinalout;
extern unsigned char   *sinitout;
extern unsigned char   *supdateout;
extern int              cinitoutl;
extern int              cupdateoutl;
extern int              cfinaloutl;
```

이 변수들은 각 PB 프로세스 파일에서 정의됨 (기존 그대로).

---

## 7. FR-06: Header Files

### 7.1 fep_common.h 확장

```c
/* inc/fep_common.h — 추가 항목 */

/* Process-level globals (new) */
extern void    *FmtPtr;
extern int      ConnectRetryCnt;
extern struct pollfd    Poll[];

/* Common function prototypes (new) */
extern int      Log_Out_Base (void);
extern int      Device_Open_Logon (int, int);

/* Process-specific function prototypes (new) */
extern int      Make_Send_Msg (int);
extern int      Handshake (void);
```

### 7.2 fep_encrypt.h 신규

```c
/*------------------------------------------------------------------------
#   Module  : INISAFE-Net encryption functions for PB processes
#   File    : fep_encrypt.h
------------------------------------------------------------------------*/
#ifndef _FEP_ENCRYPT_H_
#define _FEP_ENCRYPT_H_

/*------------------------------------------------------------------------
    INISAFE-Net encryption globals (defined in each PB process file)
------------------------------------------------------------------------*/
extern net_ctx          EnCtx;
extern unsigned char   *cinitout;
extern unsigned char   *cupdateout;
extern unsigned char   *cfinalout;
extern unsigned char   *sinitout;
extern unsigned char   *supdateout;
extern int              cinitoutl;
extern int              cupdateoutl;
extern int              cfinaloutl;

/*------------------------------------------------------------------------
    Encryption function prototypes (implemented in sub/fep_encrypt.c)
------------------------------------------------------------------------*/
extern void     Free_All (void *);
extern int      Handshake (void);

#endif /* _FEP_ENCRYPT_H_ */
```

### 7.3 Build Integration

- `sub/fep_encrypt.c` → `Make_Lib_P_c.sh`의 `for i in *.c` 루프에 자동 포함
- `fep_encrypt.o`는 `libfepP.a`에 포함되지만, PA 바이너리 링크 시 참조되지 않아 pull 안됨
- PB 바이너리만 Free_All/Handshake 참조 → fep_encrypt.o pull → INISAFE 라이브러리 링크 필요 (기존 PB Makefile에 이미 설정됨)

---

## 8. File Structure

### 8.1 New Files

```
inc/fep_encrypt.h     — INISAFE extern 선언 + 프로토타입 (~30줄)
sub/fep_encrypt.c     — Free_All + Handshake (~200줄)
```

### 8.2 Modified Files

```
inc/fep_common.h      — FmtPtr, ConnectRetryCnt, Poll, Log_Out_Base, Device_Open_Logon, Make_Send_Msg, Handshake 추가
sub/fep_common.c      — Log_Out_Base, Device_Open_Logon 추가
src/PA/pa_1100_ts.c   — Log_Out wrapper, Device_Open LOGON 교체, FmtPtr 정의, Handshake 스텁
src/PA/pa_1200_tr.c   — 동일
src/PA/pa_7800_tr.c   — 동일 (immediate_open=1)
src/PB/pb_1100_ts.c   — Log_Out wrapper(+PROC), Device_Open LOGON 교체, FmtPtr 정의, Handshake/Free_All 제거, include 추가
src/PB/pb_1200_tr.c   — 동일
src/PB/pb_7800_tr.c   — 동일 (immediate_open=1)
```

---

## 9. Implementation Order

| Step | FR | Action | Files | Risk |
|------|-----|--------|-------|------|
| 1 | FR-06 | `inc/fep_encrypt.h` 생성 | 1 new | Low |
| 2 | FR-06 | `inc/fep_common.h` 확장 (FmtPtr, prototypes) | 1 modify | Low |
| 3 | FR-05 | `sub/fep_encrypt.c` 생성 — Free_All | 1 new | Low |
| 4 | FR-04 | `sub/fep_encrypt.c` — Handshake 추가 | 1 modify | Low |
| 5 | FR-01 | `sub/fep_common.c` — Log_Out_Base 추가 | 1 modify | Medium |
| 6 | FR-03 | `sub/fep_common.c` — Device_Open_Logon 추가 | 1 modify | Medium |
| 7 | — | 6개 프로세스 파일: FmtPtr 정의 추가 | 6 modify | Low |
| 8 | FR-05,04 | 3개 PB 파일: Free_All, Handshake 제거 + fep_encrypt.h include | 3 modify | Medium |
| 9 | FR-01 | 6개 파일: Log_Out → wrapper 변경 | 6 modify | Medium |
| 10 | FR-03 | 6개 파일: Device_Open TR_LOON → Device_Open_Logon 호출 + PA Handshake 스텁 | 6 modify | Medium |

### Step 순서 근거

1-2: 헤더 먼저 생성 → 컴파일 에러 없이 점진적 적용
3-4: PB 전용 암호화 함수 먼저 (가장 단순, byte-for-byte 동일)
5-6: 공용 함수 추가 (FmtPtr 사용)
7: FmtPtr 정의는 모든 파일에 선행 필요
8-10: 프로세스 파일 순차 수정

---

## 10. Per-File Change Detail

### 10.1 PA 파일 변경 패턴

```c
/* BEFORE (pa_1200_tr.c 대표) */
void    Log_Out (void)
{
    /* ... 30줄 ... */
}

void    Device_Open (int tr_code)
{
    if (tr_code == TR_LOON)
    {
        /* ... 40줄 LOGON ... */
    }
    else if (tr_code == TR_LINK)
    {
        /* ... 50줄 LINK (유지) ... */
    }
}

/* AFTER */
#include "fep_common.h"     /* 이미 존재 */

void *FmtPtr = (void *)&KR_Fmt;              /* NEW */
int   Handshake (void) { return (NOTOK); }   /* NEW: 스텁 */

void    Log_Out (void)
{
    Log_Out_Base ();
}

void    Device_Open (int tr_code)
{
    if (tr_code == TR_LOON)
    {
        Device_Open_Logon (0, 0);   /* PA, no encrypt, no immediate open */
    }
    else if (tr_code == TR_LINK)
    {
        /* ... 50줄 LINK (유지, 변경 없음) ... */
    }
}
```

### 10.2 PB 파일 변경 패턴

```c
/* BEFORE (pb_1200_tr.c 대표) */
void    Free_All (void *ctxf) { /* ... 35줄 ... */ }
int     Handshake () { /* ... 140줄 ... */ }
void    Log_Out (void) { /* ... 33줄 ... */ }
void    Device_Open (int tr_code) { /* TR_LOON 40줄 + TR_LINK 50줄 */ }

/* AFTER */
#include "fep_common.h"     /* 이미 존재 */
#include "fep_encrypt.h"    /* NEW */

void *FmtPtr = (void *)&KR_Fmt;              /* NEW */

/* Free_All, Handshake 제거 → sub/fep_encrypt.c로 이동 */

void    Log_Out (void)
{
    Log_Out_Base ();
}

void    Device_Open (int tr_code)
{
    if (tr_code == TR_LOON)
    {
        Device_Open_Logon (1, 0);   /* PB, encrypt, no immediate open */
    }
    else if (tr_code == TR_LINK)
    {
        /* ... 50줄 LINK (유지, 변경 없음) ... */
    }
}

/* Device_Close wrapper 유지 (Phase 1) */
void    Device_Close (void)
{
    Device_Close_Base ();
    Free_All ((void *)(long)1);
    INL_Cleanup (CLIENT_CTX);
}
```

---

## 11. Expected Metrics

### 11.1 Code Reduction

| File | Before | Removed | Added | After (est) |
|------|--------|---------|-------|-------------|
| pa_1100_ts.c | 1,279 | ~75 (Log_Out+Device_Open LOGON) | +3 (FmtPtr, stub, wrapper) | ~1,207 |
| pa_1200_tr.c | 895 | ~70 | +3 | ~828 |
| pa_7800_tr.c | 767 | ~65 | +3 | ~705 |
| pb_1100_ts.c | 1,667 | ~245 (Log_Out+Device_Open+Handshake+Free_All) | +10 (FmtPtr, include, wrappers) | ~1,432 |
| pb_1200_tr.c | 1,315 | ~240 | +5 | ~1,080 |
| pb_7800_tr.c | 1,519 | ~240 | +5 | ~1,284 |
| sub/fep_common.c | 225 | 0 | +90 (Log_Out_Base + Device_Open_Logon) | ~315 |
| sub/fep_encrypt.c | 0 | 0 | +200 (Free_All + Handshake) | ~200 |
| inc/fep_common.h | 43 | 0 | +10 | ~53 |
| inc/fep_encrypt.h | 0 | 0 | +30 | ~30 |

**6-file total**: 7,442 → ~6,536 = **-906 lines (-12%)**
**New shared code**: +330 lines
**Net**: -576 lines

### 11.2 Cumulative (Phase 1 + Phase 2)

| Metric | Phase 1 | Phase 2 (est) | Cumulative |
|--------|---------|--------------|------------|
| Functions in sub/ | 6 | +4 (Log_Out_Base, Device_Open_Logon, Free_All, Handshake) | 10 |
| Net lines removed | 1,133 | ~576 | ~1,709 |
| Reduction from original | 13% | +7% | ~19% |

---

## 12. Risk Mitigations

| Risk | Mitigation |
|------|------------|
| FmtPtr 정적 초기화 C89 호환 | `&S_Fmt`는 address constant — C89 유효. ANSI C 표준 문서 확인 |
| PA Handshake 스텁 + 링커 | 스텁 함수로 심볼 제공. has_encrypt=0이므로 실제 호출 안됨 |
| fep_encrypt.o INISAFE 링크 | static archive에서 참조 없는 .o는 pull 안됨. PA 빌드 영향 없음 |
| IS_RESP_OK vs memcmp "0000" 기능 동치 | IS_RESP_OK 매크로 정의 확인 필요 (Do 단계) |
| pb_1100_ts Log_Out 추가 로직 | Log_Out_Base 반환값으로 분기, wrapper에서 처리 |
| Device_Open TR_LINK 미터치 | TR_LINK는 프로세스 파일에 그대로 유지, 스코프 명확 |

---

## 13. Verification Checklist

- [ ] `mk.sh sub` — libfepP.a 빌드 (fep_common.o + fep_encrypt.o 포함)
- [ ] `mk.sh pa` — PA 모듈 빌드 (INISAFE 미링크 확인)
- [ ] `mk.sh pb` — PB 모듈 빌드 (INISAFE 정상 링크)
- [ ] 중복 심볼 없음 (Free_All, Handshake가 PB 프로세스와 sub/ 양쪽에 없음)
- [ ] grep 검증: Free_All 정의가 PB 소스에서 제거됨
- [ ] grep 검증: Handshake 정의가 PB 소스에서 제거됨
- [ ] grep 검증: FmtPtr이 6개 파일 모두에 정의됨

---

## Version History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
