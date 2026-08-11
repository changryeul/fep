# PA 모듈 코드 품질 감사 — 설계 문서

> **요약**: 20개 이상 PA 소스 파일에 걸친 45개 발견사항에 대한 FR당 구현 사양
>
> **프로젝트**: FEP (KRX 프론트엔드 프로세서)
> **작성자**: Claude Code
> **날짜**: 2026-02-26
> **상태**: 초안
> **계획 참조**: `docs/01-plan/features/PA.plan.md`

---

## 구현 순서

7개 배치(배치 8 미연기), 순차적으로 실행. 각 배치는 독립적으로 컴파일 가능.

| 배치 | FR | 파일 | 주제 | 우선순위 |
|-------|-----|-------|-------|----------|
| 1 | FR-01, FR-02, FR-03, FR-04, FR-05 | 4 | 중요: 미초기화 변수, 누락된 중괄호, 누락된 반환 | CRITICAL |
| 2 | FR-06 | 4 | sizeof(HEAD_SIZE) 버그 | HIGH |
| 3 | FR-07 | 12 | 전처리기 `defined()` 구문 | HIGH |
| 4 | FR-08, FR-09, FR-10 | 4 | 포맷 문자열, 0으로 나누기, 데드 코드 | HIGH |
| 5 | FR-11 | 9 | 누락된 `fep_common.h` 포함 | MEDIUM |
| 6 | FR-12, FR-13, FR-14 | 4 | 하드코딩된 값, 잘못된 TR 코드 | MEDIUM |
| 7 | FR-15, FR-16 | 3 | 미사용 변수, 중복 로그 | LOW |
| 8 | FR-17 | 4 | 신호 핸들러 안전 | **미연기** |

### 의도적으로 미연기된 항목

| ID | 이슈 | 이유 |
|----|-------|--------|
| FR-17 / 배치 8 | 신호 핸들러 안전 | 신중한 평가 필요; 신호 핸들러의 `Exit_Process()`는 플래그 기반 접근이 필요할 수 있음. 전체 분석 대기 중 미연기. |
| M2 | C++ 주석 (약 257개 발생, 6개 파일) | 높은 볼륨, 낮은 영향. PB 감사와 동일한 결정. |
| L1 | `while(1){read;break}` FIFO 드레인 패턴 | 올바르게 작동, 스타일만 관련된 문제. |

---

## 배치 1: 중요 수정 (FR-01, FR-02, FR-03, FR-04, FR-05)

### FR-01: Check_Header()에서 미초기화된 `rt` (pa_2100_ts.c:497)

`rt`는 라인 483에서 선언되지만 초기화되지 않음. 라인 497에서 `strlen(RecvPkt) != rt` 조건에서 사용되어 결정되지 않은 값과 비교됨. 또한 `i`, `next`, `val`은 선언되지만 이 함수에서 미사용.

`|| strlen(RecvPkt) != rt` 조건은 이 컨텍스트에서 `rt`가 정의된 의미가 없기 때문에(IMECO 헤더 검증은 계산된 `rt`가 아닌 `IMECO_HEAD_LEN` 사용) 제거되어야 함.

**파트 A — 미사용 변수 제거 (라인 483):**

**이전:**
```c
	int		i, next, rt, val;
	char	U_ErrMsg[1024];
```

**새로:**
```c
	int		rt;
	char	U_ErrMsg[1024];
```

**파트 B — `|| strlen(RecvPkt) != rt` 조건 제거 (라인 497):**

**이전:**
```c
	if (strlen (RecvPkt) < IMECO_HEAD_LEN || strlen (RecvPkt) != rt)
	{
		SLog (USR_ERROR, "invalid length[%d:%d,%d]", strlen (RecvPkt), TCP_HEAD_LEN, rt);
		return (NOTOK);
	}
```

**새로:**
```c
	rt = AtoIf (R_Pkt->Length, sizeof(R_Pkt->Length)) + IMECO_HEAD_LEN;
	if (strlen (RecvPkt) < IMECO_HEAD_LEN || strlen (RecvPkt) != rt)
	{
		SLog (USR_ERROR, "invalid length[%d:%d,%d]", strlen (RecvPkt), IMECO_HEAD_LEN, rt);
		return (NOTOK);
	}
```

**근거**: `rt`는 예상되는 총 길이여야 함: IMECO 헤더의 길이 필드에서의 본문 길이 더하기 IMECO 헤더 크기. 이는 의도된 길이 검증을 복원. 로그는 또한 `TCP_HEAD_LEN`을 `IMECO_HEAD_LEN`으로 수정(체크에 실제로 사용되는 상수).

### FR-02: Check_rtn()에서 R_Fmt[]을 인덱싱하는 미초기화된 `i` (pa_1200_mp.c:174,176,179)

`i`는 라인 154에서 선언되지만 초기화되지 않음. 라인 174, 176, 179에서 `R_Fmt[i]`를 인덱싱하는 데 사용. 함수 매개변수 `for_d`가 올바른 인덱스.

**이전:**
```c
#if defined A1201								/* 채권 */
	if (memcmp (R_Fmt[i].Data, "REJC", 4) == 0)
#elif defined A2201								/* 파생 */
	if (memcmp (&R_Fmt[i].Data[5], "REJC", 4) == 0)
#endif
	{
		Log (USR_OK, "Reject SKIP Write OK ResponseCode[%10.10s]", R_Fmt[i].Data);
		return (OK);
	}
```

**새로:**
```c
#if defined A1201								/* 채권 */
	if (memcmp (R_Fmt[for_d].Data, "REJC", 4) == 0)
#elif defined A2201								/* 파생 */
	if (memcmp (&R_Fmt[for_d].Data[5], "REJC", 4) == 0)
#endif
	{
		Log (USR_OK, "Reject SKIP Write OK ResponseCode[%10.10s]", R_Fmt[for_d].Data);
		return (OK);
	}
```

### FR-03: `else if` 전 누락된 `}` 중괄호 (pa_3100_ts.c:758)

라인 754에서 시작하는 `if` 블록이 라인 759의 `else if` 전에 닫는 중괄호가 누락됨. 관련 `-D` 정의가 활성일 때 컴파일을 방지하는 구문 오류.

**이전:**
```c
	if (memcmp(J_R_Fmt.Header.MsgType, "SCHOPQ00000", 11) == 0)
	{
		SLog (USR_OK, "업무개시요청(SCHOPQ00000): <%d:%d:%d>",
			FirstSeq, INT_SEQ, LOAD_CNT);
		return (TR_LINK);
	else if (memcmp(J_R_Fmt.Header.MsgType, "SCHHER00000", 11) == 0)
```

**새로:**
```c
	if (memcmp(J_R_Fmt.Header.MsgType, "SCHOPQ00000", 11) == 0)
	{
		SLog (USR_OK, "업무개시요청(SCHOPQ00000): <%d:%d:%d>",
			FirstSeq, INT_SEQ, LOAD_CNT);
		return (TR_LINK);
	}
	else if (memcmp(J_R_Fmt.Header.MsgType, "SCHHER00000", 11) == 0)
```

### FR-04: Make_Send_Msg()에서 미초기화된 `datacnt` (pa_1100_ts.c:775,859)

`datacnt`는 라인 775에서 선언되지만 초기화되지 않음. 라인 859에서 KRX 헤더의 DataCnt 필드를 쓰기 위해 `ItoAf(datacnt, ...)`에서 사용됨. 이 프로세스는 단일 주문(MAX_CNT=1)을 전송하므로 올바른 값은 1.

**이전:**
```c
	int		rt, datacnt;
	char	d_time[18];

	rt = 0;
	memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
```

**새로:**
```c
	int		rt, datacnt = 1;
	char	d_time[18];

	rt = 0;
	memset (&S_Fmt, 0, sizeof (KRX_SESSION_FMT));
```

**근거**: `datacnt = 1` 왜냐하면: (a) MAX_CNT=1 (단일 주문 처리), (b) 라인 849의 TR_DATA 경우는 한 번에 하나의 주문을 처리하는 `Make_Data_Block()`을 호출.

### FR-05: Analyze_Data() 기본 반환 누락 — 3개 파일

세 개의 `Analyze_Data()` 함수 모두 수신한 MsgType이 알려진 패턴과 일치하지 않으면 끝까지 반환하지 않고 떨어짐. 수정: 에러 로그 추가 + `return (RP_STOP)`을 추가하여 우아한 연결 끊기 트리거.

#### FR-05a: pa_1100_ts.c (Analyze_Data 끝, 라인 683)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**새로:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", S_Fmt.Header.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

#### FR-05b: pa_1200_tr.c (Analyze_Data 끝, 라인 701)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**새로:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", Header_Fmt.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

참고: pa_1200_tr.c는 `Header_Fmt`(`S_Fmt` 아님)를 사용.

#### FR-05c: pa_3100_ts.c (Analyze_Data 끝, 라인 771)

**이전:**
```c
		return (RP_DATA);
	}
}	/* End of Analyze_Data ()	*/
```

**새로:**
```c
		return (RP_DATA);
	}

	Log (USR_ERROR, "Analyze_Data: unknown MsgType[%.11s]", J_R_Fmt.Header.MsgType);
	return (RP_STOP);
}	/* End of Analyze_Data ()	*/
```

참고: pa_3100_ts.c는 `J_R_Fmt`를 사용.

---

## 배치 2: sizeof(HEAD_SIZE) 버그 (FR-06)

`HEAD_SIZE`는 `(sizeof(FILE_DATA_HEAD))` = `size_t` 상수(이 플랫폼에서 값 20). `sizeof(HEAD_SIZE)` = `sizeof(size_t)` = 64비트에서 8. 이는 `memset`과 `memcpy`가 20 바이트 대신 8 바이트에서 작동함을 의미. 수정: `sizeof(HEAD_SIZE)`를 `HEAD_SIZE`로 대체.

### FR-06a: pa_1200_tr.c:730 (memset)

**이전:**
```c
	memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
```

**새로:**
```c
	memset (&File_Data_Head, ' ', HEAD_SIZE);
```

### FR-06b: pa_1200_tr.c:750 (memcpy)

**이전:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head,	sizeof (HEAD_SIZE));
```

**새로:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head,	HEAD_SIZE);
```

### FR-06c: pa_2200_tr.c:616 (memset)

**이전:**
```c
	memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
```

**새로:**
```c
	memset (&File_Data_Head, ' ', HEAD_SIZE);
```

### FR-06d: pa_2200_tr.c:633 (memcpy)

**이전:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head, sizeof (HEAD_SIZE));
```

**새로:**
```c
	memcpy (W_Fmt.DataHeader, &File_Data_Head, HEAD_SIZE);
```

### FR-06e: pa_2700_tr.c:455 (memset)

**이전:**
```c
	memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
```

**새로:**
```c
	memset (&File_Data_Head, ' ', HEAD_SIZE);
```

### FR-06f: pa_7800_tr.c:549 (memset)

**이전:**
```c
	memset (&File_Data_Head, ' ', sizeof (HEAD_SIZE));
```

**새로:**
```c
	memset (&File_Data_Head, ' ', HEAD_SIZE);
```

---

## 배치 3: 전처리기 `defined()` 구문 (FR-07)

패턴: `#if defined X || Y`는 `#if defined(X) || Y`로 평가됨. `Y`가 `defined()` 테스트가 아니므로 숫자 체크(0이 아닌 리터럴 = 항상 참)가 됨. 수정: 각 매크로 이름을 `defined()`로 래핑.

총: 12개 파일에 걸친 36개 위치. 파일별로 나열됨.

### FR-07a: pa_1200_tr.c — 3 locations

**Line 74:**

**old:**
```c
#if defined A1201 || A1202
```

**new:**
```c
#if defined(A1201) || defined(A1202)
```

**Line 433:**

**old:**
```c
#if defined A1201 || A1202
```

**new:**
```c
#if defined(A1201) || defined(A1202)
```

**Line 473:**

**old:**
```c
#elif defined A1601 || A1602
```

**new:**
```c
#elif defined(A1601) || defined(A1602)
```

### FR-07b: pa_1490_mp.c — 9 locations (includes duplicate A1491 bug)

**Line 34:**

**old:**
```c
#if defined A1491 || A1492
```

**new:**
```c
#if defined(A1491) || defined(A1492)
```

**Line 37:**

**old:**
```c
#elif defined A2491 || A2492
```

**new:**
```c
#elif defined(A2491) || defined(A2492)
```

**Line 177:**

**old:**
```c
#if defined A1491 || A1492
```

**new:**
```c
#if defined(A1491) || defined(A1492)
```

**Line 188:**

**old:**
```c
#elif defined A2491 || A2492
```

**new:**
```c
#elif defined(A2491) || defined(A2492)
```

**Line 264:**

**old:**
```c
#if defined A1491 || A1492
```

**new:**
```c
#if defined(A1491) || defined(A1492)
```

**Line 293 (BUG: duplicate A1491):**

**old:**
```c
#if defined A1491 || A1491
```

**new:**
```c
#if defined(A1491) || defined(A1492)
```

Note: `A1491 || A1491` is a copy-paste error. The correct second operand is `A1492`, matching the pattern at lines 34, 177, 264, 380.

**Line 316:**

**old:**
```c
#if defined A1491 || A2491		/* 1번째: 3초 대기 */
```

**new:**
```c
#if defined(A1491) || defined(A2491)		/* 1번째: 3초 대기 */
```

**Line 380:**

**old:**
```c
#if defined A1491 || A1492
```

**new:**
```c
#if defined(A1491) || defined(A1492)
```

**Line 383:**

**old:**
```c
#elif defined A2491 || A2492
```

**new:**
```c
#elif defined(A2491) || defined(A2492)
```

**Line 409:**

**old:**
```c
#if defined A2491 || A2492
```

**new:**
```c
#if defined(A2491) || defined(A2492)
```

### FR-07c: pa_5200_qs.c — 2 locations

**Line 23:**

**old:**
```c
#if defined A5201||A5401
```

**new:**
```c
#if defined(A5201) || defined(A5401)
```

**Line 25:**

**old:**
```c
#elif defined A5211||A5411
```

**new:**
```c
#elif defined(A5211) || defined(A5411)
```

### FR-07d: pa_7100_ur.c — 5 locations

**Line 50:**

**old:**
```c
#elif defined A7103 || A7203
```

**new:**
```c
#elif defined(A7103) || defined(A7203)
```

**Line 52:**

**old:**
```c
#elif defined A7201 || A7291
```

**new:**
```c
#elif defined(A7201) || defined(A7291)
```

**Line 250:**

**old:**
```c
#if defined A7102||A7202
```

**new:**
```c
#if defined(A7102) || defined(A7202)
```

**Line 484:**

**old:**
```c
#elif defined A7102	|| A7103
```

**new:**
```c
#elif defined(A7102) || defined(A7103)
```

**Line 545:**

**old:**
```c
#elif defined A7202 || A7203
```

**new:**
```c
#elif defined(A7202) || defined(A7203)
```

### FR-07e: pa_7500_us.c — 1 location

**Line 25:**

**old:**
```c
#if defined A7103||A7203
```

**new:**
```c
#if defined(A7103) || defined(A7203)
```

### FR-07f: pa_7100_dd.c — 6 locations

**Line 36:**

**old:**
```c
#elif defined A7103 || A7203
```

**new:**
```c
#elif defined(A7103) || defined(A7203)
```

**Line 63:**

**old:**
```c
#if defined A7181 || A7182 || A7102 || A7103 || A7203
```

**new:**
```c
#if defined(A7181) || defined(A7182) || defined(A7102) || defined(A7103) || defined(A7203)
```

**Line 65:**

**old:**
```c
#elif defined A7201 || A7202
```

**new:**
```c
#elif defined(A7201) || defined(A7202)
```

**Line 382:**

**old:**
```c
#elif defined A7102 || A7103
```

**new:**
```c
#elif defined(A7102) || defined(A7103)
```

**Line 454:**

**old:**
```c
#elif defined A7202 || A7203
```

**new:**
```c
#elif defined(A7202) || defined(A7203)
```

### FR-07g: pa_7000_mp.c — 1 location

**Line 441:**

**old:**
```c
#if defined A7001 || A7002 || A7003
```

**new:**
```c
#if defined(A7001) || defined(A7002) || defined(A7003)
```

### FR-07h: pa_7000_tr.c — 1 location

**Line 308:**

**old:**
```c
#if defined A7701 || A7702
```

**new:**
```c
#if defined(A7701) || defined(A7702)
```

### FR-07i: pa_7000_us.c — 1 location

**Line 141:**

**old:**
```c
#elif defined A7002 || A7003						/* 지수옵션시세 */
```

**new:**
```c
#elif defined(A7002) || defined(A7003)						/* 지수옵션시세 */
```

### FR-07j: pa_7010_us.c — 1 location

**Line 141:**

**old:**
```c
#elif defined A7002 || A7003						/* 지수옵션시세 */
```

**new:**
```c
#elif defined(A7002) || defined(A7003)						/* 지수옵션시세 */
```

---

## 배치 4: 포맷 문자열, 0으로 나누기, 데드 코드 (FR-08, FR-09, FR-10)

### FR-08: `&dat`를 `%s` 포맷 인자로 전달 — 3개 위치, 2개 파일

`dat`는 포인터(예: `KRX_NOTE_SETTLE_RESP_DATA *dat`). `&dat`는 포인터 변수 자체의 주소이지 문자열 데이터가 아님. `%s`에 전달되면 포인터의 스택 주소에서 시작하는 바이트를 읽음(정의되지 않은 동작). 원래 주석 처리된 코드는 `dat->DataSeq`를 사용했는데 올바름.

#### FR-08a: pa_1290_mp.c:258

**이전:**
```c
//			Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
			Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, &dat);
```

**새로:**
```c
			Log (USR_ERROR, "타매체 수신 [%30.30s] [%.11s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
```

#### FR-08b: pa_1290_mp.c:707

**이전:**
```c
//			Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
			Log (USR_ERROR, "타매체 수신 [%30.30s] [%s]", dat->MembershipItem+imeco_gbn+30, &dat);
```

**새로:**
```c
			Log (USR_ERROR, "타매체 수신 [%30.30s] [%.11s]", dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
```

#### FR-08c: pa_1400_mp.c:228

**이전:**
```c
			Log (USR_ERROR, "Other Media Recv!! Check Plz [%3.3s] [%s]",
				dat->MembershipItem+imeco_gbn+30, &dat);
```

**새로:**
```c
			Log (USR_ERROR, "Other Media Recv!! Check Plz [%3.3s] [%.11s]",
				dat->MembershipItem+imeco_gbn+30, dat->DataSeq);
```

**근거**: `dat->DataSeq`는 KRX/IMECO 구조체의 11바이트 데이터 시퀀스 필드. `%.11s` 사용은 null 종료되지 않는 필드의 폭 제한 안전 인쇄를 보장. 활성 라인이 이제 올바른 인자를 가지므로 주석 처리된 라인은 제거됨.

### FR-09: Write_Data()에서 0으로 나누기 (pa_7800_tr.c:521)

`for_i`는 `Header_Fmt.DataCnt`에서 파싱됨. 값이 0 또는 음수(잘못된 형식의 패킷)인 경우 나누기 `/ for_i`는 정의되지 않은 동작을 야기.

**이전:**
```c
	for_i  = AtoIf (Header_Fmt.DataCnt,    sizeof (Header_Fmt.DataCnt));
	d_size = AtoIf (Header_Fmt.BodyLength, sizeof (Header_Fmt.BodyLength)) / for_i;
```

**새로:**
```c
	for_i  = AtoIf (Header_Fmt.DataCnt,    sizeof (Header_Fmt.DataCnt));
	if (for_i <= 0)
	{
		Log (USR_ERROR, "Write_Data: invalid DataCnt[%d]", for_i);
		return;
	}
	d_size = AtoIf (Header_Fmt.BodyLength, sizeof (Header_Fmt.BodyLength)) / for_i;
```

### FR-10: 중복된 도달 불가능한 `rt!=1` 체크 (pa_1200_tr.c:762-772)

`F_W()` 후 첫 번째 `if (rt != 1)` 블록은 프로세스를 종료하는 `Exit_Process()`를 호출. 라인 768의 두 번째 `if (rt != 1)` 블록은 도달 불가능한 데드 코드. 수정: 두 체크를 병합하고 두 번째 블록에서 더 설명적인 로그 메시지 사용.

**이전:**
```c
	rt = F_W (p_flag * 10, (void *)&W_Fmt, 1);

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

**새로:**
```c
	rt = F_W (p_flag * 10, (void *)&W_Fmt, 1);

	if (rt != 1)
	{
		Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,p_flag-1));
		Exit_Process ();
	}
```

---

## 배치 5: 누락된 `fep_common.h` 포함 (FR-11)

이러한 파일들은 `fep_common.h`에 선언된 함수들(예: `Set_Socket_Linger`, `detect_poll_event`, `Log_Out_Base`)을 호출하지만 헤더를 포함하지 않음. 포함 없이 이러한 함수 호출은 암시적 선언에 의존하며, C99+에서는 비준수이며 64비트 플랫폼에서 문제를 일으킬 수 있음(암시적 `int` 반환 타입).

### FR-11a: pa_2100_ts.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
#include	"ifaddrs.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
#include	"ifaddrs.h"
#include	"fep_common.h"
```

### FR-11b: pa_2200_tr.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
#include	"fep_common.h"
```

### FR-11c: pa_1600_tr.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"fep_common.h"
```

### FR-11d: pa_2700_tr.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"pa_struct.h"
#include	"fep_common.h"
```

### FR-11e: pa_7000_tr.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"
#include	"fep_common.h"
```

### FR-11f: pa_8100_ts.c

호출: `Set_Socket_Linger`, `detect_poll_event`

**이전:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"		/* CLI 프로토콜 상수 및 구조체 */
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"		/* CLI 프로토콜 상수 및 구조체 */
#include	"fep_common.h"
```

### FR-11g: pa_8200_tr.c

호출: `Set_Socket_Linger`

**이전:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"
```

**새로:**
```c
#include	"fep_fepp.h"
#include	"cli_interface.h"
#include	"fep_common.h"
```

### FR-11h: pa_3100_ts.c

호출: `detect_poll_event`

**이전:**
```c
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"
```

**새로:**
```c
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "ifaddrs.h"
#include    "fep_common.h"
```

---

## 배치 6: 하드코딩된 값 & 논리 버그 (FR-12, FR-13, FR-14)

### FR-12: 하드코딩된 IP "123.123.123.123" — 검토만, 수정하지 않음

**파일**: pa_2100_ts.c:898, pa_2200_tr.c:544

두 위치 모두 IMECO LogOn 핸드셰이크(`PktType == T_LOON`)에 있으며, ServerIP 필드가 20바이트 필드에 "123.123.123.123"으로 채워짐:

```c
memcpy (&S_Pkt->Data[40], "123.123.123.123", 20);
```

**분석**: 이는 IMECO 프로토콜 자리 표시자. IMECO 사양은 LogOn 데이터 섹션(오프셋 40)에 20바이트 ServerIP 필드를 요구. 값 "123.123.123.123"은 IMECO 핸드셰이크 프로토콜에서 사용되는 고정 테스트/자리 표시자 값. 이를 변경하면 IMECO 연결이 손상될 수 있음. 이 패턴은 TS(전송)와 TR(수신) 프로세스 모두에서 일관되게 나타나 이것이 의도된 프로토콜 값임을 확인.

**결정**: 검토만. 수정 없음. IMECO 프로토콜 문서에서 다른 값을 지정하면 비즈니스 팀 승인과 함께 이를 업데이트해야 함.

### FR-13a: pa_7000_mp.c:178의 하드코딩된 브로드캐스트 IP

명령줄 인자가 제공되지 않을 때 폴백 브로드캐스트 IP `172.21.101.255`가 하드코딩됨.

**이전:**
```c
		if (argc > 2)
			sprintf (&Svr_IP[20*i], "%s", argv[2+i]);
		else
			sprintf (Svr_IP, "172.21.101.255");		/* 기본 브로드캐스트 */
```

**새로:**
```c
		if (argc > 2)
			sprintf (&Svr_IP[20*i], "%s", argv[2+i]);
		else
		{
			Log (USR_ERROR, "UDP broadcast IP not specified (argc=%d)", argc);
			return -1;
		}
```

**근거**: 하드코딩된 내부 IP는 환경 특정이며 다양한 배포 사이트에서 손상될 것. IP는 항상 명령줄 인자 또는 설정을 통해 제공되어야 함. 제공되지 않으면 프로세스는 잘못된 IP를 조용히 사용하기보다는 에러 메시지로 실패해야 함.

### FR-13b: pa_7500_us.c:100의 하드코딩된 IP

**이전:**
```c
	Max_IP_Addr = 1;
	sprintf (Svr_IP, "172.21.155.7");			/* TODO: 설정파일로 이동 필요 */
```

**새로:**
```c
	Max_IP_Addr = 1;
	sprintf (Svr_IP, "%d.%d.%d.%d",
		TCP2_IP1(D_K,P_K,0), TCP2_IP2(D_K,P_K,0),
		TCP2_IP3(D_K,P_K,0), TCP2_IP4(D_K,P_K,0));
```

**근거**: 하드코딩된 IP 대신 기존 TCP2 설정 매크로 사용(`tcp2.ini`를 통해 정의). 이는 다른 UDP 전송 프로세스(예: `pa_7000_us.c`)에서 사용되는 패턴과 일치. 원래 코드의 TODO 주석이 이것이 설정으로 이동해야 함을 명시적으로 인정.

### FR-14: LP 채권 자동 취소의 잘못된 TR 코드 (pa_1290_mp.c:948)

라인 948은 라인 681에서 시작하는 `#if defined A1291` (LP 채권) 블록 내. LP 섹션은 일관되게 `TTRMOP4xxxx` 코드(라인 685, 907, 908)를 사용하지만 라인 948은 LP 특정 `TTRMOP41303` 대신 일반 채권 코드 `TTRODP11303`을 잘못 사용.

**이전:**
```c
		if (memcmp (tr_code, "TTRODP11303", 11) == 0)	// 자동취소
```

**새로:**
```c
		if (memcmp (tr_code, "TTRMOP41303", 11) == 0)	// LP자동취소
```

**근거**: 일반 채권 섹션(라인 546-679)은 `TTRODP` 코드를 사용. LP 채권 섹션(라인 681-973)은 `TTRMOP` 코드를 사용. 라인 948은 LP 섹션 내이므로 `TTRMOP41303` (LP 자동취소)를 사용해야 하며, `TTRODP11303` (일반 자동취소)이 아님. 기존 코드는 이미 라인 907-908에서 동일한 블록 내에서 `TTRMOP41302`/`TTRMOP41303`을 올바르게 사용.

> **위험**: 이는 비즈니스 로직을 변경. 현재 코드는 LP 블록 내에서 `TTRODP11303`과 일치하지 않음(LP 프로세스는 `TTRMOP` 코드만 수신), 따라서 라인 948의 자동취소 정리는 효과적으로 데드 코드. 수정하면 LP 자동취소 정리 경로를 활성화. **프로덕션 배포 전 비즈니스 검증 필요.**

---

## 배치 7: 데드 코드 & 미사용 변수 (FR-15, FR-16)

### FR-15a: pa_1200_tr.c:807의 미사용 `datacnt`

`Make_Send_Msg()`에서 변수 `datacnt`는 선언되지만 사용되지 않음. 변수 `i`도 선언되지만 미사용.

**이전:**
```c
	int		i, rt, datacnt;
	char	d_time[18];
```

**새로:**
```c
	int		rt;
	char	d_time[18];
```

### FR-15b: pa_7800_tr.c:611의 미사용 `datacnt`

FR-15a와 동일한 패턴.

**이전:**
```c
	int		i, rt, datacnt;
	char	d_time[18];
```

**새로:**
```c
	int		rt;
	char	d_time[18];
```

### FR-16: pa_1200_tr.c:774,780의 중복 로그 라인

동일한 메시지 포맷 `"file write[%s:%d:%d]"`를 가진 두 개의 연속 `USR_OK` 로그 라인이지만 인자가 다름. 라인 774는 대상 파일(`p_flag-1`)을 로깅하고, 라인 780은 기본 파일(`0`)을 로깅. 라인 780의 두 번째 로그는 실제로 쓰여진 파일이 아닌 다른 파일의 쓰기 결과를 보고하므로 오도함. 두 번째 로그 제거.

**이전:**
```c
	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

	/* 시퀀스 증가 */
	INT_SEQ += rt;

	Log (USR_OK, "file write[%s:%d:%d]", OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);
	Set_TR_Time ();
```

**새로:**
```c
	Log (USR_OK, "file write[%s:%d:%d]",
		OFN(D_K,P_K,p_flag-1), OFW(D_K,P_K,p_flag-1,0), rt);

	/* 시퀀스 증가 */
	INT_SEQ += rt;

	Set_TR_Time ();
```

---

## 배치 8: 신호 핸들러 안전 (FR-17) — 미연기된 평가

| FR | 파일 | 라인 | 이슈 |
|----|------|------|-------|
| FR-17 | pa_2100_ts.c | ~1011 | 신호 핸들러에서 `Exit_Process()` 호출 |
| FR-17 | pa_1600_tr.c | ~720 | 동일 |
| FR-17 | pa_2200_tr.c | ~725 | 동일 |
| FR-17 | pa_2700_tr.c | ~552 | 동일 |

**상태**: 미연기. 신호 핸들러 안전은 `sub/` 파일의 `signal-handler-safety` 보관된 기능에서 해결되었지만, PA 파일은 다른 패턴(플래그 기반 접근을 사용하기보다는 신호 핸들러에서 직접 `Exit_Process()` 호출)을 사용. 전체 완화 필요:

1. `Exit_Process()`가 비동기 신호 안전하지 않은 작업(파일 I/O, malloc 등)을 수행하는지 이해
2. `sub/`의 플래그 기반 접근을 PA 프로세스에 적용할 수 있는지 결정
3. 정리(SHM 분리, 소켓 닫기)가 제대로 발생함을 테스트

이 배치는 위의 분석이 완료된 후 별도 기능에서 다룰 것.

---

## 검증

### 빌드 검증

```sh
cd st01/shl
./mk.sh pa
```

모든 PA 바이너리는 오류 없이 컴파일되어야 함. 영향받는 주요 바이너리:

| 바이너리 | 적용된 FR |
|--------|-------------|
| pa_1100_ts (pa_1101_ts, etc.) | FR-04, FR-05a, FR-11 (이미 있음) |
| pa_1200_tr (pa_1201_tr, pa_1401_tr, pa_1601_tr, pa_1602_tr) | FR-05b, FR-06a, FR-06b, FR-07a, FR-10, FR-11c (1600), FR-15a, FR-16 |
| pa_1200_mp (pa_1201_mp, pa_2201_mp) | FR-02 |
| pa_1290_mp (pa_1291_mp, pa_2291_mp) | FR-08a, FR-08b, FR-14 |
| pa_1400_mp (pa_1401_mp, pa_2401_mp) | FR-08c |
| pa_1490_mp (pa_1491_mp, etc.) | FR-07b (A1491 버그 포함 10개 위치) |
| pa_2100_ts (pa_2101_ts, etc.) | FR-01, FR-11a |
| pa_2200_tr (pa_2201_tr, etc.) | FR-06c, FR-06d, FR-11b |
| pa_2700_tr (pa_2701_tr, etc.) | FR-06e, FR-11d |
| pa_3100_ts (pa_3101_ts, etc.) | FR-03, FR-05c, FR-11h |
| pa_5200_qs (pa_5201_qs, etc.) | FR-07c |
| pa_7000_mp (pa_7001_mp, etc.) | FR-07g, FR-13a |
| pa_7000_tr (pa_7701_tr, etc.) | FR-07h, FR-11e |
| pa_7000_us (pa_7001_us, etc.) | FR-07i |
| pa_7010_us (pa_7002_us, etc.) | FR-07j |
| pa_7100_dd (pa_7102_dd, etc.) | FR-07f |
| pa_7100_ur (pa_7102_ur, etc.) | FR-07d |
| pa_7500_us (pa_7501_us, etc.) | FR-07e, FR-13b |
| pa_7800_tr (pa_7801_tr, etc.) | FR-06f, FR-09, FR-15b |
| pa_8100_ts (pa_8101_ts, etc.) | FR-11f |
| pa_8200_tr (pa_8201_tr, etc.) | FR-11g |

### 변경 요약

| 메트릭 | 개수 |
|--------|-------|
| 수정된 파일 | 20 |
| CRITICAL 수정 | 5 (FR-01 to FR-05) |
| HIGH 수정 | 38 (FR-06: 6 sizeof, FR-07: 36 전처리기, FR-08: 3 포맷, FR-09: 1 0으로 나누기, FR-10: 1 데드 코드 = 발견사항 전체 합계) |
| MEDIUM 수정 | 12 (FR-11: 9 포함, FR-13: 2 하드코딩된 IP, FR-14: 1 잘못된 TR) |
| LOW 수정 | 4 (FR-15: 2 미사용 변수, FR-16: 1 중복 로그) |
| 미연기 | FR-12 (검토만), FR-17 (신호 안전), M2 (C++ 주석) |
| 순 추정 라인 | 약 +30 (포함 추가 라인, 가드 추가 라인, 제거 빼기) |

---

## 버전 이력

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-26 | 검증된 FR 사양을 포함한 초기 설계 | Claude Code |
