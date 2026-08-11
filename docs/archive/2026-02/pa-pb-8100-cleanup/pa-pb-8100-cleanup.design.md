# 설계: pa-pb-8100-cleanup

## 참조
- 계획: `docs/01-plan/features/pa-pb-8100-cleanup.plan.md`

## 구현 순서

1. FR-01 ~ FR-04: pb_8100_ts.c (4개 항목, 38줄)
2. FR-05 ~ FR-08: pa_8100_ts.c (4개 항목, 24줄)

PB를 먼저 처리 (항목 더 많음), 그 다음 PA.

## 변경 명세

각 FR 항목에 대해: **주석 처리된 코드 블록, 주석 처리된 함수 호출 또는 사용되지 않는 변수 선언 삭제**. 파일에 대한 다른 변경은 없습니다.

---

## 파일 1: pb_8100_ts.c (4개 항목, 38줄)

### FR-01: HOLIDAY_APPLY 주석 처리된 주말 확인 — 12줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 108-119 | 12 | `/* ... */` 블록: 비활성화된 주말/평일 분기 로직 |

**컨텍스트 (105-120줄):**
```c
        tp = localtime (&t);

		break;                          // <-- stays (line 107)
/*                                      // <-- DELETE from here (108)
        if (tp->tm_wday == 0 || tp->tm_wday == 6)       // Sun, Sat
        ...
        }
*/                                      // <-- DELETE to here (119)
    }                                   // <-- stays (line 120)
```

**정리 후**: 107줄의 `break;`은 `tp = localtime(&t);` 이후 while 본체의 유일 문장으로 남습니다. 이는 의도된 것입니다 — PB의 HOLIDAY_APPLY 블록은 즉시 중단합니다.

### FR-02: 주석 처리된 Device_Close 호출 — 1줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 250 | 1 | PB_8100_TS() 끝의 `//Device_Close ();` |

**컨텍스트 (248-253줄):**
```c
    }

    //Device_Close ();                  // <-- DELETE (line 250)

    return;
}   /* End of PB_8100_TS ()    */
```

**정리 후**: `return;` 이전의 빈 줄은 남습니다. 활성 코드 경로는 필요할 때 이미 메인 루프 내에서 `Device_Close()`를 호출합니다.

### FR-03: Make_Send_Msg의 사용되지 않는 변수 — 1줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 601 | 1 | `int arry_cnt, arry_len, arry_dat;` — 선언되었으나 참조 없음 |

**컨텍스트 (599-603줄):**
```c
{
    int     r_cnt, data_length;         // <-- stays (line 600, used)
    int     arry_cnt, arry_len, arry_dat; // <-- DELETE (line 601, unused)
    char    d_time[16];                 // <-- stays (line 602, used)
```

**검증**: Make_Send_Msg 함수 (597-706줄) 전체에서 검색 — `arry_cnt`, `arry_len`, `arry_dat`는 어디에도 나타나지 않습니다.

### FR-04: Make_Send_Msg의 주석 처리된 검증 로직 — 24줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 641-652 | 12 | `#if defined B8101 \|\| B8111 \|\| B8116` 내부의 `/* 2025 ... */` 블록 |
| 654-665 | 12 | `#elif defined B8105 \|\| B8117 \|\| B8118` 내부의 `/* ... */` 블록 |

**컨텍스트 (638-668줄):**
```c
			else
			{
#if defined B8101 || B8111 || B8116     // <-- stays (line 640)
/* 2025 ...                             // <-- DELETE 641-652
				if (memcmp ...
				...
				}
*/
#elif defined B8105 || B8117 || B8118   // <-- stays (line 653)
/*                                      // <-- DELETE 654-665
				if (memcmp ...
				...
				}
*/
#endif                                  // <-- stays (line 666)
			}
			SendFlag = 1;
```

**정리 후**: `#if`/`#elif`/`#endif` 전처리기 지시문 및 `else { }` 블록은 남아 있으나 비어 있습니다. 이는 무해합니다 (컴파일되지 않음). 구조적 변경을 최소화하기 위해 계획 결정에 따라 유지됩니다.

**참고**: `/* 2025 */` 주석은 이 미디어 타입 검증이 2025년에 의도적으로 비활성화됨을 나타냅니다. PA의 Make_Send_Msg에서는 동등한 검증이 활성 코드입니다 (주석 처리되지 않음).

---

## 파일 2: pa_8100_ts.c (4개 항목, 24줄)

### FR-05: HOLIDAY_APPLY 주석 처리된 business_day 확인 — 12줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 103-114 | 12 | `/* ... */` 블록: 비활성화된 business_day 비교 로직 |

**컨텍스트 (100-116줄):**
```c
        else
        {
			Log (USR_OK, "it's not weekend. not sleeping...[%d]", tp->tm_wday);
			break;                      // <-- stays (line 102)
/*                                      // <-- DELETE from here (103)
            Log (USR_OK, "it's weekday. ok...[%d]", tp->tm_wday);
			if (memcmp (Shm_Risk[0].business_day, DAEMON(D_K).date, 8) == 0)
                break;
            ...
*/                                      // <-- DELETE to here (114)
        }                               // <-- stays (line 115)
    }
```

**정리 후**: `else` 절은 `Log` + `break;` (101-102줄)를 보존합니다. `/* ... */` 블록은 `business_day` vs `DAEMON date`를 비교한 후 시작을 허용하기 전에 비교하는 포기된 향상이었습니다.

### FR-06: 주석 처리된 Device_Close 호출 — 1줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 250 | 1 | PA_8100_TS() 끝의 `//Device_Close ();` |

**컨텍스트 (248-253줄):**
```c
    }

    //Device_Close ();                  // <-- DELETE (line 250)

    return;
}   /* End of PA_8100_TS ()    */
```

PB (FR-02)와 동일한 패턴.

### FR-07: Socket_Event_Rtn의 주석 처리된 seq 리셋 블록 — 10줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 358-367 | 10 | `/* 20211218 */` 날짜 마커 + `/* ... */` 코드 블록 |

**컨텍스트 (355-368줄):**
```c
        if (r_seq != INT_SEQ && r_seq <= WRITE_CNT)
        {
            Log (USR_WARN, "TR_LIOK recv:check HUB SEQ <%d> < <%d>", r_seq, INT_SEQ);
/* 20211218 */                          // <-- DELETE from here (358)
/* LIOK로 ... comment */
/*
#if defined A8101
            R_CNT(0,0) = INT_SEQ = r_seq;
#elif defined A8102
            R_CNT(0,1) = INT_SEQ = r_seq;
#endif
*/
/* 20211218 */                          // <-- DELETE to here (367)
        }                               // <-- stays (line 368)
```

**정리 후**: `if` 블록은 357줄의 `Log` 호출만 보존합니다. 주석 처리된 seq 리셋 (2021-12-18에 비활성화)은 클라이언트가 LIOK 응답을 통해 서버의 seq을 수신하는 현재 접근 방식으로 대체되었습니다.

**참고**: PB의 동등한 `Socket_Event_Rtn`은 이 블록을 가지지 않습니다 (PB 355-359줄에는 단순히 Log + 닫기 괄호가 있습니다).

### FR-08: Make_Send_Msg의 사용되지 않는 변수 — 1줄

| 삭제 줄 | 크기 | 참고 |
|:------------|:----:|-------|
| 611 | 1 | `int arry_cnt, arry_len, arry_dat;` — 선언되었으나 참조 없음 |

**컨텍스트 (609-613줄):**
```c
{
    int     r_cnt, data_length;         // <-- stays (line 610, used)
    int     arry_cnt, arry_len, arry_dat; // <-- DELETE (line 611, unused)
    char    d_time[16];                 // <-- stays (line 612, used)
```

PB (FR-03)와 동일한 패턴. 검증됨: 3개 변수는 함수 어디에서도 참조되지 않습니다.

---

## 요약

| 파일 | FR 항목 | 블록 | 제거된 줄 |
|------|:--------:|:------:|:-------------:|
| pb_8100_ts.c | FR-01~04 | 4 | 38 |
| pa_8100_ts.c | FR-05~08 | 5 | 24 |
| **합계** | **8** | **9** | **62** |

### 제거된 줄 분석

| 카테고리 | PB | PA | 합계 |
|----------|:--:|:--:|:-----:|
| `/* */` 주석 처리된 코드 | 36 | 22 | 58 |
| `//` 주석 처리된 코드 | 1 | 1 | 2 |
| 사용되지 않는 변수 선언 | 1 | 1 | 2 |
| **소계** | **38** | **24** | **62** |

### 정리 후 파일 크기

| 파일 | 이전 | 이후 | 변경 |
|------|:------:|:-----:|:------:|
| pb_8100_ts.c | 711줄 | 673줄 | -38 |
| pa_8100_ts.c | 717줄 | 693줄 | -24 |

## 검증 체크리스트

1. 두 파일 모두에 남은 `/* ... */` 주석 처리된 코드 블록 없음 (함수 배너 및 인라인 `/* comment */`처럼 정당한 설명 주석 제외)
2. 남은 `//` 주석 처리된 함수 호출 없음
3. Make_Send_Msg에 남은 사용되지 않는 변수 선언 없음
4. 블록 삭제 이외의 변경된 줄 없음
5. 모든 전처리기 지시문 (`#if defined`, `#elif`, `#endif`) 보존됨
6. 빌드: `mk.sh src`는 동일 바이너리 생성
