# select-recv-bounds-check 설계 Document

> **요약**: select_recv.c 6개 함수에 pkt_len 상한/하한 bounds check 추가
>
> **프로젝트**: FEP (Front-End Processor)
> **저자**: Claude
> **날짜**: 2026-02-25
> **상태**: 초안
> **Plan Reference**: `docs/01-plan/features/select-recv-bounds-check.plan.md`

---

## 1. 설계 개요

### 1.1 Target 파일

`st01/sub/select_recv.c` — 1개 파일, 6개 함수 수정

### 1.2 Fix Pattern

모든 함수에 동일 패턴 적용: `AtoIf()` 파싱 후, `Recvn()` 호출 전에 **상한+하한 guard** 삽입.
비정상 pkt_len 탐지 시 `Log(USR_ERROR, ...)` + `return (NOTOK)`.

### 1.3 Buffer Size Constants

| Constant | Value | Source |
|----------|-------|--------|
| `TCP_BUFF_MAX_LEN` | 5120 | `inc/fep_tcpip.h:14` |
| `TCP_HEAD_LEN` | 80 (sizeof TCP_HEAD) | `inc/fep_tcpip.h:17` |
| `KRX_DATA_BUFF_SIZE` | 4096 | `inc/fep_fepp.h:17` |
| `CLI_BUFF_MAX_LEN` | 4096 | `inc/cli_interface.h:13` |
| `CLI_HEAD_LEN` | 50 (sizeof CLI_HEAD) | `inc/cli_interface.h:15` |
| `TCP_MESSAGE_MAX_LEN` | 4300 | `inc/fep_tcpip.h:13` |

---

## 2. Detailed 변경사항

### FR-01, FR-02: Select_Receive (line 67~72)

**Callers**: pa_1600_tr, pw_3010_tr, pw_3030_tr, pw_4000_ts
**Buffer**: `RecvPkt[TCP_BUFF_MAX_LEN]` (5120)
**Protocol**: 1-byte type + 4-byte length + body. Recvn offset=5.
**Max safe body**: TCP_BUFF_MAX_LEN - 5 = 5115

**Before** (line 67~72):
```c
	pkt_len = AtoIf (p_recv+1, 4);

	if (pkt_len < TCP_HEAD_LEN-5)
		Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

	rt = Recvn (p_sfd, p_recv+5, pkt_len);
```

**After**:
```c
	pkt_len = AtoIf (p_recv+1, 4);

	if (pkt_len < TCP_HEAD_LEN - 5 || pkt_len > TCP_BUFF_MAX_LEN - 5)
	{
		Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);
		return (NOTOK);
	}

	rt = Recvn (p_sfd, p_recv+5, pkt_len);
```

**변경사항**:
- 하한 조건 유지 (`< TCP_HEAD_LEN-5` = `< 75`)
- 상한 조건 추가 (`> TCP_BUFF_MAX_LEN-5` = `> 5115`)
- `{ }` 블록화 + `return (NOTOK)` 추가

---

### FR-03, FR-04: Select_Receive2 (line 140~142)

**Callers**: 없음 (미래 방어)
**Buffer**: caller에 따라 다름, TCP_BUFF_MAX_LEN 가정
**Protocol**: p_len-byte length + body. Recvn length = pkt_len - p_len.

**Before** (line 140~142):
```c
	pkt_len = AtoIf (p_recv, p_len);

	rt = Recvn (p_sfd, &p_recv[p_len], pkt_len - p_len);
```

**After**:
```c
	pkt_len = AtoIf (p_recv, p_len);

	if (pkt_len <= p_len || pkt_len > TCP_BUFF_MAX_LEN)
	{
		Log (USR_ERROR, "Select_Receive2:invalid Length[%d]", pkt_len);
		return (NOTOK);
	}

	rt = Recvn (p_sfd, &p_recv[p_len], pkt_len - p_len);
```

**변경사항**:
- 하한: `pkt_len <= p_len` → body size (`pkt_len - p_len`)가 0 이하면 차단
- 상한: `pkt_len > TCP_BUFF_MAX_LEN` → 버퍼 초과 차단
- 새로운 guard 블록 삽입

---

### FR-05: Select_Receive_Krx (line 210~214)

**Callers**: pa_3100_ts (`DataBuff[KRX_DATA_BUFF_SIZE]`, p_len=82), pb_1800_ts (`DataBuff[KRX_DATA_BUFF_SIZE]`, p_len=82)
**Buffer**: `DataBuff[KRX_DATA_BUFF_SIZE]` (4096)
**Protocol**: p_len-byte header (82=KRX_HEAD_LEN) read first, then BodyLength field at offset 8 (6 digits). Recvn length = pkt_len.
**Max safe body**: KRX_DATA_BUFF_SIZE - p_len = 4096 - 82 = 4014

**Before** (line 210~214):
```c
    pkt_len = AtoIf (&p_recv[8], 6);

    if (pkt_len > 0)
    {
        rt = Recvn (p_sfd, &p_recv[p_len], pkt_len);
```

**After**:
```c
    pkt_len = AtoIf (&p_recv[8], 6);

    if (pkt_len > 0)
    {
        if (pkt_len > KRX_DATA_BUFF_SIZE - p_len)
        {
            Log (USR_ERROR, "Select_Receive_Krx:invalid Length[%d]", pkt_len);
            return (NOTOK);
        }

        rt = Recvn (p_sfd, &p_recv[p_len], pkt_len);
```

**변경사항**:
- `pkt_len > 0` 기존 하한 유지
- 상한 `pkt_len > KRX_DATA_BUFF_SIZE - p_len` 추가 (if 블록 내부)

---

### FR-06, FR-07: Select_Receive_Cli (line 281~285)

**Callers**: pa_8100_ts, pa_8200_tr, pb_7200_tr, pb_8100_ts, pb_8200_tr
**Buffer**: `RecvPkt[CLI_BUFF_MAX_LEN]` (4096)
**Protocol**: 4-byte length + body. Recvn offset=4.
**Max safe body**: CLI_BUFF_MAX_LEN - 4 = 4092

**Before** (line 281~285):
```c
	pkt_len = AtoIf (p_recv, 4);
    if (pkt_len < CLI_HEAD_LEN - 4)
        Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

    rt = Recvn (p_sfd, p_recv+4, pkt_len);
```

**After**:
```c
	pkt_len = AtoIf (p_recv, 4);
    if (pkt_len < CLI_HEAD_LEN - 4 || pkt_len > CLI_BUFF_MAX_LEN - 4)
    {
        Log (USR_ERROR, "Select_Receive_Cli:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn (p_sfd, p_recv+4, pkt_len);
```

**변경사항**:
- 하한 조건 유지 (`< CLI_HEAD_LEN - 4` = `< 46`)
- 상한 조건 추가 (`> CLI_BUFF_MAX_LEN - 4` = `> 4092`)
- `{ }` 블록화 + `return (NOTOK)` 추가
- Log 메시지 함수명 수정: `Select_Receive` → `Select_Receive_Cli`

---

### FR-08, FR-09: Select_Receive_Imeco (line 351~355)

**Callers**: pa_2100_ts, pa_2200_tr
**Buffer**: `RecvPkt[TCP_BUFF_MAX_LEN]` (5120)
**Protocol**: 4-byte ASCII length field → AtoIf + 16 = pkt_len. Recvn offset=4.
**Max safe body**: TCP_BUFF_MAX_LEN - 4 = 5116

**Before** (line 351~355):
```c
	pkt_len = AtoIf (p_recv, 4) + 16;
    if (pkt_len < 20 - 4)
        Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

    rt = Recvn (p_sfd, p_recv+4, pkt_len);
```

**After**:
```c
	pkt_len = AtoIf (p_recv, 4) + 16;
    if (pkt_len < 20 - 4 || pkt_len > TCP_BUFF_MAX_LEN - 4)
    {
        Log (USR_ERROR, "Select_Receive_Imeco:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn (p_sfd, p_recv+4, pkt_len);
```

**변경사항**:
- 하한 조건 유지 (`< 16`)
- 상한 조건 추가 (`> TCP_BUFF_MAX_LEN - 4` = `> 5116`)
- `{ }` 블록화 + `return (NOTOK)` 추가
- Log 메시지 함수명 수정: `Select_Receive` → `Select_Receive_Imeco`

---

### FR-10, FR-11: Select_Receive_Imeco_Sise (line 421~425)

**Callers**: pa_2700_tr
**Buffer**: `RecvPkt[TCP_BUFF_MAX_LEN]` (5120)
**Protocol**: 10-byte ASCII length field → AtoIf + 40 = pkt_len. Recvn offset=10.
**Max safe body**: TCP_BUFF_MAX_LEN - 10 = 5110

**Before** (line 421~425):
```c
	pkt_len = AtoIf (p_recv, 10) + 40;		// 헤더 50
    if (pkt_len < 50 - 10)
        Log (USR_ERROR, "Select_Receive:invalid Length[%d]", pkt_len);

    rt = Recvn (p_sfd, p_recv+10, pkt_len);
```

**After**:
```c
	pkt_len = AtoIf (p_recv, 10) + 40;
    if (pkt_len < 50 - 10 || pkt_len > TCP_BUFF_MAX_LEN - 10)
    {
        Log (USR_ERROR, "Select_Receive_Imeco_Sise:invalid Length[%d]", pkt_len);
        return (NOTOK);
    }

    rt = Recvn (p_sfd, p_recv+10, pkt_len);
```

**변경사항**:
- 하한 조건 유지 (`< 40`)
- 상한 조건 추가 (`> TCP_BUFF_MAX_LEN - 10` = `> 5110`)
- `{ }` 블록화 + `return (NOTOK)` 추가
- Log 메시지 함수명 수정: `Select_Receive` → `Select_Receive_Imeco_Sise`
- `// 헤더 50` 인라인 주석 제거 (C89 호환, C++ comment)

---

## 3. 범위 외 (확인)

### Sise_Select_Receive (line 445~508) — 수정 불요

```c
rt = recv(p_sfd, sockbuff + sockpos, SZ_FEEDDATA_MAX - sockpos, 0);
```

이미 `SZ_FEEDDATA_MAX - sockpos`로 cap되어 있음. 안전.

---

## 4. Implementation Order

단일 파일이므로 순서대로 수정:

| Step | Function | Lines | FR |
|------|----------|-------|----|
| 1 | `Select_Receive` | 67~72 | FR-01, FR-02 |
| 2 | `Select_Receive2` | 140~142 | FR-03, FR-04 |
| 3 | `Select_Receive_Krx` | 210~214 | FR-05 |
| 4 | `Select_Receive_Cli` | 281~285 | FR-06, FR-07 |
| 5 | `Select_Receive_Imeco` | 351~355 | FR-08, FR-09 |
| 6 | `Select_Receive_Imeco_Sise` | 421~425 | FR-10, FR-11 |

---

## 5. 검증 Checklist

- [ ] 6개 함수 모두 Recvn 호출 전 상한+하한 guard 존재
- [ ] 모든 guard에서 `return (NOTOK)` 수행 (log-only 아님)
- [ ] 함수 시그니처 변경 없음
- [ ] `Sise_Select_Receive`는 미수정
- [ ] `mk.sh sub` 빌드 성공
- [ ] Log 메시지의 함수명이 실제 함수명과 일치

---

## 버전 기록

| 버전 | 날짜 | 변경사항 | 저자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-25 | Initial draft | Claude |
