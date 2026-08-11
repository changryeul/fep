# PB 모듈 코드 품질 감사 — 설계 문서

> **요약**: 9개 PB 소스 파일의 28+1개 발견사항에 대한 FR당 구현 사양
>
> **프로젝트**: FEP (KRX Front-End Processor)
> **작성자**: Claude Code
> **날짜**: 2026-02-26
> **상태**: 초안
> **계획 참조**: `docs/01-plan/features/PB.plan.md`

---

## 구현 순서

8개 배치, 순차적으로 실행. 각 배치는 독립적으로 컴파일 가능합니다.

| 배치 | 기능 요구사항 | 파일 | 테마 | 우선순위 |
|-------|-----|-------|-------|----------|
| 1 | FR-01, FR-02, FR-03 | 3 | Critical: 미초기화 변수 + 누락된 break | CRITICAL |
| 2 | FR-04, FR-05, FR-06 | 3 | Analyze_Data() 누락된 기본값 반환 | HIGH |
| 3 | FR-07, FR-08, FR-09 | 3 | 전처리기 `defined` 키워드 | HIGH |
| 4 | FR-10, FR-11, FR-18, FR-18b | 4 | 메모리 안전성: 미초기화 변수, 범위 초과, sizeof 버그 | HIGH |
| 5 | FR-14, FR-15, FR-16, FR-17 | 1 | pb_1800_ts.c 공유 라이브러리 마이그레이션 | MEDIUM |
| 6 | FR-12, FR-13, FR-27 | 2 | C89 준수: 혼합 선언 + 주석 | MEDIUM |
| 7 | FR-19, FR-20, FR-21, FR-22, FR-23 | 2 | 견고성: 0으로 나누기, 하드코딩된 값 | MEDIUM |
| 8 | FR-24, FR-25, FR-26, FR-28 | 3 | 데드/불필요한 코드 정리 | LOW |

---

## 배치 1: Critical 수정 (FR-01, FR-02, FR-03)

### FR-01: RP_POLL case에서 누락된 `break` (pb_1200_tr.c:564-571)

**old:**
```c
		Device_Write ();
	case	RP_STOP:
```

**new:**
```c
		Device_Write ();
		break;
	case	RP_STOP:
```

### FR-02: 미초기화된 `datacnt` (pb_1100_ts.c:810)

**이전:**
```c
	int		rt, datacnt;
	char	d_time[18];

	rt = 0;
	memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
```

**이후:**
```c
	int		rt, datacnt = 1;
	char	d_time[18];

	rt = 0;
	memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
```

**근거**: `datacnt = 1` 이유: (a) MAX_CNT=1 (단일 주문 처리), (b) 라인 919는 이미 DataBuff[78]에 `"001"`을 하드코딩, 단일 레코드 확인.

### FR-03: 미초기화된 `datacnt` (pb_1800_ts.c:1097)

**이전:**
```c
	int		rt, datacnt;
	char	d_time[18];
```

**이후:**
```c
	int		rt, datacnt = 1;
	char	d_time[18];
```

---

## 배치 2: Analyze_Data() 누락된 기본값 반환 (FR-04, FR-05, FR-06)

세 개의 `Analyze_Data()` 함수 모두 수신한 MsgType이 알려진 패턴과 일치하지 않으면 반환 없이 끝나갑니다. 수정: 에러 로그 + `return (RP_STOP)` 추가하여 우아한 연결 해제를 트리거합니다.

### FR-04: pb_1100_ts.c (Analyze_Data 끝)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**이후:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

### FR-05: pb_1800_ts.c (Analyze_Data 끝)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**이후:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

### FR-06: pb_1200_tr.c (Analyze_Data 끝)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**이후:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

주의: pb_1200_tr.c는 `Header_Fmt` (S_Fmt 아님)을 사용합니다.

---

## 배치 3: 전처리기 `defined` 키워드 (FR-07, FR-08, FR-09)

### FR-07: pb_1200_tr.c:431

**이전:**
```c
#if defined B1201 || B1601
```

**이후:**
```c
#if defined(B1201) || defined(B1601)
```

### FR-08: pb_7100_ts.c:291

**이전:**
```c
#if defined B7612 || B7613
```

**이후:**
```c
#if defined(B7612) || defined(B7613)
```

### FR-09: pb_8100_ts.c — 6개 위치

**위치 1 (라인 364):**

**이전:**
```c
#if defined B8101 || B8111 || B8116
			Add_Count(PS_R_1, 1);
#elif defined B8105 || B8117 || B8118
			Add_Count(PS_R_2, 1);
```

**이후:**
```c
#if defined(B8101) || defined(B8111) || defined(B8116)
			Add_Count(PS_R_1, 1);
#elif defined(B8105) || defined(B8117) || defined(B8118)
			Add_Count(PS_R_2, 1);
```

**위치 2 (라인 632):**

**이전:**
```c
#if defined B8101 || B8111 || B8116
			r_cnt = F_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
#elif defined B8105 || B8117 || B8118
			r_cnt = F_R (PS_R_2, (void *)R_Fmt, MAX_CNT);
```

**이후:**
```c
#if defined(B8101) || defined(B8111) || defined(B8116)
			r_cnt = F_R (PS_R_1, (void *)R_Fmt, MAX_CNT);
#elif defined(B8105) || defined(B8117) || defined(B8118)
			r_cnt = F_R (PS_R_2, (void *)R_Fmt, MAX_CNT);
```

**위치 3 (라인 649):**

**이전:**
```c
#if defined B8101 || B8111 || B8116
#elif defined B8105 || B8117 || B8118
```

**이후:**
```c
#if defined(B8101) || defined(B8111) || defined(B8116)
#elif defined(B8105) || defined(B8117) || defined(B8118)
```

---

## 배치 4: 메모리 안전성 (FR-10, FR-11, FR-18, FR-18b)

### FR-10: 미초기화된 `rt` + 누락된 반환 (pb_8200_tr.c)

**파트 A — 라인 530: `rt` 초기화**

**이전:**
```c
	int		rt, cnt, i, target;
	char	m_time[24];
```

**이후:**
```c
	int		rt = 0, cnt, i, target;
	char	m_time[24];
```

**파트 B — 라인 ~568: else 분기에 반환 추가**

**이전:**
```c
	else
	{
		Log (SAM_FATAL, "Else Case Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
	}
```

**이후:**
```c
	else
	{
		Log (SAM_FATAL, "Else Case Recv TR_Code No Data [%50.50s]", R_Pkt->Data);
		return;
	}
```

### FR-11: `w_gbn` 인덱스 -1 범위 초과 (pb_7200_tr.c:249)

**이전:**
```c
	int		rval, rt, w_gbn = 0;
```

**이후:**
```c
	int		rval, rt, w_gbn = 1;
```

**근거**: `w_gbn-1`은 `out_f[]` 및 `OFN()`으로의 인덱스로 사용됩니다. `w_gbn=1`이면 인덱스는 0 (첫 번째 출력 파일)이 되어, pb_7800_tr.c의 관례와 일치하며 `p_flag=1`은 `p_flag-1=0`을 산출합니다.

### FR-18: `sizeof(HEAD_SIZE)` bug — pb_7800_tr.c (2 locations)

`HEAD_SIZE` is `(sizeof(FILE_DATA_HEAD))` = `size_t` constant. `sizeof(HEAD_SIZE)` = `sizeof(size_t)` = 8 on 64-bit. Intended value: 20.

**pb_7800_tr.c:878:**

**old:**
```c
				memset (&File_Data_Head, ' ',   sizeof (HEAD_SIZE));
				memset (&f_head, 0x20, sizeof(f_head));
```

**new:**
```c
				memset (&File_Data_Head, ' ',   HEAD_SIZE);
				memset (&f_head, 0x20, sizeof(f_head));
```

**pb_7800_tr.c:966:**

**old:**
```c
				memset (&File_Data_Head, ' ',   sizeof (HEAD_SIZE));
				memset (&f_head, 0x20, sizeof(f_head));
```

**new:**
```c
				memset (&File_Data_Head, ' ',   HEAD_SIZE);
				memset (&f_head, 0x20, sizeof(f_head));
```

### FR-18b (BONUS): `sizeof(HEAD_SIZE)` bug — pb_1200_tr.c (2 locations)

**pb_1200_tr.c:823 (memset):**

**old:**
```c
	memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
```

**new:**
```c
	memset (&File_Data_Head, ' ', HEAD_SIZE);
```

**pb_1200_tr.c:843 (memcpy):**

**old:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head,	sizeof (HEAD_SIZE));
```

**new:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head,	HEAD_SIZE);
```

---

## 배치 5: pb_1800_ts.c 공유 라이브러리 마이그레이션 (FR-14, FR-15, FR-16, FR-17)

이 배치는 `pb_1800_ts.c`를 `fep_common.c` 및 `fep_encrypt.c`의 공유 라이브러리 함수를 사용하도록 마이그레이션하여, `pb_1100_ts.c` 및 `pb_1200_tr.c`에 이미 확립된 패턴과 일치시킵니다.

### FR-14: 중복된 `#define IN_NAME` 수정, SAM_USE 가드 추가

**이전:**
```c
#define     WR_CNT          W_CNT(0,0)
#define     RD_CNT          R_CNT(0,0)
#define     IN_NAME         IFN(D_K,P_K,0)

#define     IN_NAME         IFN(D_K,P_K,0)
#define     PORT_NO         TCP2_PORT_NO
```

**이후:**
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

`pb_1100_ts.c:42-50`에서 복사한 패턴.

### FR-15: 공유 라이브러리 포함 추가

**이전:**
```c
#include    "ifaddrs.h"

```

**이후:**
```c
#include    "ifaddrs.h"
#include    "fep_common.h"
#include    "fep_encrypt.h"

```

그런 다음 로컬 `Free_All()` 함수(라인 ~519-553) 및 `Handshake()` 함수(라인 ~563-694)를 삭제합니다. 주석으로 대체:

```c
/* Free_All, Handshake: moved to sub/fep_encrypt.c */
```

### FR-16: FmtPtr 할당 추가

**이전:**
```c
struct pollfd		Poll[3];
```

**이후:**
```c
struct pollfd		Poll[3];
void	*FmtPtr = (void *)&S_Fmt;
```

### FR-17: 로컬 `Get_Msec`, `Err_Msg`, `Line_Change` 제거

`fep_common.h` 포함 추가 (FR-15) 후, 다음을 제거합니다:
- 로컬 프로토타입 `void Line_Change (void);` (라인 ~117)
- 로컬 프로토타입 `void Get_Msec (double *);` (라인 ~123)
- 로컬 프로토타입 `void Err_Msg (void);` (라인 ~125)
- 로컬 프로토타입 `void Fifo_Event_Rtn (void);` (라인 ~110) — 이미 fep_common.h에 있음
- 함수 본문 `Get_Msec` (라인 ~1345-1361)
- 함수 본문 `Err_Msg` (라인 ~1405-1494)
- 함수 본문 `Line_Change` (라인 ~1496-1516)

**주의**: `Device_Close`, `Device_Write`, `Device_Read`는 로컬로 남아 있습니다 — 이들은 프로세스 특정 로직(암호화 호출)을 포함합니다.

---

## 배치 6: C89 준수 (FR-12, FR-13, FR-27)

### FR-12: C99 혼합 선언 (pb_1100_ts.c)

`unsigned char* ret_data`를 함수 블록의 상단으로 이동합니다.

**라인 ~946에 추가 (기존 선언과 함께):**

**이전:**
```c
	int		enc_len = 0;
	char	err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];
```

**이후:**
```c
	int		enc_len = 0;
	unsigned char *ret_data = NULL;
	char	err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];
```

**라인 ~1017에서 변경:**

**이전:**
```c
		unsigned char* ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```

**이후:**
```c
		ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```

### FR-13: C99 혼합 선언 (pb_1800_ts.c)

동일한 패턴입니다. `Make_Data_Block()` 상단에 `unsigned char *ret_data = NULL;` 추가, 할당 라인에서 `unsigned char*` 제거합니다.

**라인 ~1232에 추가:**

**이전:**
```c
	int		enc_len = 0;
	char	err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];
```

**이후:**
```c
	int		enc_len = 0;
	unsigned char *ret_data = NULL;
	char	err_cd[10], tmp[128], w_buf[FILE_BUF_LEN];
```

**라인 ~1279에서 변경:**

**이전:**
```c
		unsigned char* ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```

**이후:**
```c
		ret_data = EncryptAndMakeSendPacket(&R_Fmt[i].Data,
```

### FR-27: PB 소스 파일의 C++ 주석 (pb_1100_ts.c, pb_1800_ts.c)

`pb_1100_ts.c`(~50개 인스턴스) 및 `pb_1800_ts.c`(~30개 인스턴스)의 모든 `//` 주석을 `/* */`로 변환합니다. 완료된 `header-cpp-comment-c89` 기능과 동일한 패턴을 사용합니다.

**패턴**: `// text` → `/* text */`

**범위**: pb_1100_ts.c 및 pb_1800_ts.c만 (다른 PB 파일은 최소 또는 `//` 주석이 없음).

---

## 배치 7: 견고성 (FR-19, FR-20, FR-21, FR-22, FR-23)

### FR-19: 0으로 나누기 보호 (pb_7800_tr.c)

모든 나누기 작업 전에 `for_i <= 0` 보호를 추가합니다.

**위치 1 — Check999() 암호화 분기 (~라인 670):**

**이전:**
```c
		d_size	 = dec_len / for_i;
```

**이후:**
```c
		if (for_i <= 0)
		{
			Log (USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
			INL_Free_Buf(dec_data);
			return;
		}
		d_size	 = dec_len / for_i;
```

**위치 2 — Check999() 평문 분기 (~라인 723):**

**이전:**
```c
		d_size = body_len / for_i;
```

**이후:**
```c
		if (for_i <= 0)
		{
			Log (USR_ERROR, "Check999: invalid DataCnt[%d]", for_i);
			return;
		}
		d_size = body_len / for_i;
```

**위치 3 — Write_Data() (~라인 794):**

**이전:**
```c
	d_size	 = AtoIf (Header_Fmt.BodyLength,	sizeof (Header_Fmt.BodyLength)) / for_i;
```

**이후:**
```c
	if (for_i <= 0)
	{
		Log (USR_ERROR, "Write_Data: invalid DataCnt[%d]", for_i);
		return;
	}
	d_size	 = AtoIf (Header_Fmt.BodyLength,	sizeof (Header_Fmt.BodyLength)) / for_i;
```

**위치 4 — Write_Data() 암호화 분기 (~라인 834):**

위치 3 보호로 이미 포함됨 (동일한 `for_i`).

### FR-20: 하드코딩된 IP 주소 제거 (pb_7100_ur.c)

**이전:**
```c
	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0), TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));

	/* 환경별 IP/포트 설정 (TEST vs REAL) */
	if (memcmp(_FEP_DIV, "TEST", 4) == 0)
	{
		strncpy(IpAddr, "10.38.111.79", sizeof(IpAddr) - 1);
		IpAddr[sizeof(IpAddr) - 1] = '\0';
		TCP2_PORT_NO = 42001;
	}
	else
	{
		strncpy(IpAddr, "10.37.11.61", sizeof(IpAddr) - 1);
		IpAddr[sizeof(IpAddr) - 1] = '\0';
		TCP2_PORT_NO = 42001;
	}
```

**이후:**
```c
	sprintf (IpAddr, "%d.%d.%d.%d", TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0), TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));
```

IP 및 포트는 하드코딩된 재정의가 아니라 `tcp2.ini` 구성에서 와야 합니다.

### FR-21: 하드코딩된 NIC 이름을 동적 검색으로 교체 (pb_7100_ur.c)

**이전:**
```c
	{
		char *env_hostname = getenv ("host_name");
		if (env_hostname == NULL) env_hostname = "";

		if (memcmp (env_hostname, "sbhond", 6) == 0)
			sprintf(ifnm, "ens7f3");
		else
			sprintf (ifnm, "ens6f3");

		Log (USR_OK, "interface name > [%s] host_name > [%s]", ifnm, env_hostname);
	}
```

**이후:**
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

같은 파일에 이미 정의된 기존 `find_ifname_by_192()` 함수(라인 549)를 사용합니다.

### FR-22: ifr_name에 대한 하드코딩된 길이가 있는 `memcpy` (pb_7100_ur.c:407)

**이전:**
```c
	memcpy(ifreq.ifr_name, ifnm, 6);
```

**이후:**
```c
	strncpy(ifreq.ifr_name, ifnm, IFNAMSIZ - 1);
```

### FR-23: struct mreq와의 로그 포맷 불일치 (pb_7100_ur.c:422)

**이전:**
```c
		Log (UDP_WARN, "setsockopt MULTICAST(%d) fail (%d:%s)", mreq, SYS_NO, SYS_STR);
```

**이후:**
```c
		Log (UDP_WARN, "setsockopt MULTICAST fail (%d:%s)", SYS_NO, SYS_STR);
```

---

## Batch 8: Dead/Unnecessary Code Cleanup (FR-24, FR-25, FR-26, FR-28)

### FR-24: Duplicate unreachable `rt!=1` check (pb_1200_tr.c:866-876)

**old:**
```c
	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write[%s]", OFN(D_K,P_K,0));
		Exit_Process ();
	}

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
		return;
	}
```

**new:**
```c
	rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
		Exit_Process ();
	}
```

Merged: use the more descriptive log message from the second block, keep `Exit_Process()` from the first.

### FR-25: Unnecessary `while(1){read;break}` (pb_1100_ts.c:517-522)

**old:**
```c
	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		break;
	}
```

**new:**
```c
	rt = read (INPUT_FD, tmp, sizeof(tmp));
```

### FR-26: Same unnecessary loop (pb_1800_ts.c:500-505)

**old:**
```c
	while (1)
	{
		rt = read (INPUT_FD, tmp, sizeof(tmp));

		break;
	}
```

**new:**
```c
	rt = read (INPUT_FD, tmp, sizeof(tmp));
```

### FR-28: `INL_Free_Buf(NULL)` in else branch (pb_1100_ts.c, pb_1800_ts.c)

**pb_1100_ts.c (~line 1040):**

**old:**
```c
			// 암호화 버퍼 해제(송신후 해야함)
			INL_Free_Buf(ret_data);
			ret_data = NULL;
			sleep (3);
```

**new:**
```c
			sleep (3);
```

**pb_1800_ts.c (same pattern):**

Remove `INL_Free_Buf(ret_data); ret_data = NULL;` from the else branch where `ret_data` is guaranteed NULL.

---

## 검증

### 빌드 검증

```sh
cd st01/shl
./mk.sh pb
```

모든 PB 바이너리는 오류 없이 컴파일되어야 합니다: pb_1101_ts, pb_1201_tr, pb_1401_tr, pb_1601_tr, pb_1801_ts, pb_7100_ts, pb_7100_ur, pb_7200_tr, pb_7612_ts, pb_7613_ts, pb_7800_tr, pb_8100_ts 시리즈, pb_8200_tr.

### 변경 요약

| 메트릭 | 개수 |
|--------|-------|
| 수정된 파일 | 9 |
| CRITICAL 수정 | 3 (미초기화 변수, 누락된 break) |
| HIGH 수정 | 8+1 (누락된 반환, 전처리기, 범위 초과, sizeof) |
| MEDIUM 수정 | 10 (중복 제거 마이그레이션, C89, 0으로 나누기, 하드코딩) |
| LOW 수정 | 7 (데드 코드, 루프, 주석) |
| 순 예상 라인 | ~-200 (중복 제거 마이그레이션은 ~180 제거, 기타 수정은 순 ~-20) |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-26 | 28+1 FR 사양이 포함된 초기 설계 | Claude Code |
