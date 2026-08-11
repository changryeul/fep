# 계획: socket-linger-extraction

## 개요

15개 소스 파일에서 중복된 `Set_Socket_Linger()` 함수를 매개변수화된 fd 인수를 가진 공유 라이브러리 `sub/fep_common.c`로 추출합니다.

## 문제

`Set_Socket_Linger()`는 15개 활성 파일(PA: 8, PB: 4, PW: 3)에서 복사-붙여넣기됨. 함수는 모든 경우에 동일 — 소켓 fd에서 `SO_LINGER`을 `l_onoff=1, l_linger=0`으로 설정 — 두 가지 변형만 있음:

1. **fd 변수**: 10개 파일은 `Sockfd` 사용, 5개 파일은 `Newfd` 사용
2. **Log 함수**: 14개 파일은 `Log()` 사용, 1개 파일 `pw_4000_ts.c`는 `SLog()` 사용

추가로, 15개 파일 중 3개는 함수를 정의하지만 **호출하지 않음** — 완전한 데드 코드.

## 범위

### 활성 호출 사이트가 있는 파일 (12개 파일)

**그룹 A — Sockfd, Log (9개 파일):**

| # | 파일 | 정의 라인 | 호출 라인 |
|---|------|----------------|-----------|
| 1 | `src/PA/pa_2100_ts.c` | 1046 | 261 |
| 2 | `src/PA/pa_2200_tr.c` | 773 | 241 |
| 3 | `src/PA/pa_2700_tr.c` | 523 | 224 |
| 4 | `src/PA/pa_7000_tr.c` | 555 | 145 |
| 5 | `src/PA/pa_1600_tr.c` | 703 | 277 |
| 6 | `src/PA/pa_8200_tr.c` | 679 | 155 |
| 7 | `src/PB/pb_8200_tr.c` | 697 | 150 |
| 8 | `src/PW/pw_3010_tr.c` | 612 | 74 |
| 9 | `src/PW/pw_3030_tr.c` | 587 | 74 |

**그룹 B — Sockfd, SLog (1개 파일):**

| # | 파일 | 정의 라인 | 호출 라인 |
|---|------|----------------|-----------|
| 10 | `src/PW/pw_4000_ts.c` | 650 | 93 |

**그룹 C — Newfd, Log (2개 파일):**

| # | 파일 | 정의 라인 | 호출 라인 |
|---|------|----------------|-----------|
| 11 | `src/PA/pa_8100_ts.c` | 724 | 154 |
| 12 | `src/PB/pb_8100_ts.c` | 718 | 158 |

### 데드 코드가 있는 파일 — 정의했지만 호출하지 않음 (3개 파일)

| # | 파일 | 정의 라인 | 비고 |
|---|------|----------------|-------|
| 13 | `src/PB/pb_7200_tr.c` | 429 | Newfd 사용, 호출하지 않음 |
| 14 | `src/PB/pb_7100_ts.c` | 505 | Newfd 사용, 호출하지 않음 |
| 15 | `src/PA/pa_7100_ts.c` | 487 | Newfd 사용, 호출하지 않음 |

### 라이브러리 파일 (2개 파일 수정)

| 파일 | 변경 |
|------|--------|
| `sub/fep_common.c` | `Set_Socket_Linger(int fd)` 정의 추가 |
| `inc/fep_common.h` | `extern void Set_Socket_Linger(int fd);` 선언 추가 |

## 접근 방식

### 추출 설계

```c
// sub/fep_common.c의 새로운 공유 함수
void Set_Socket_Linger(int fd)
{
    int    rt;
    struct linger ling;

    ling.l_onoff  = 1;
    ling.l_linger = 0;

    rt = setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
    if (rt < 0)
        Log(TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);
}
```

### 파일별 변경사항

**활성 호출 사이트가 있는 12개 파일:**
1. 전방 선언 제거: `void Set_Socket_Linger(void);`
2. 함수 정의 블록 제거 (~15줄 포함 주석 배너)
3. 호출 사이트 업데이트: `Set_Socket_Linger()` → `Set_Socket_Linger(Sockfd)` 또는 `Set_Socket_Linger(Newfd)`

**데드 코드가 있는 3개 파일:**
1. 전방 선언 제거: `void Set_Socket_Linger(void);`
2. 함수 정의 블록 제거 (~15줄 포함 주석 배너)
3. 호출 사이트 변경 없음 (호출이 없음)

### SLog → Log 참고

`pw_4000_ts.c`는 현재 로컬 복사본에서 `SLog` 사용. 추출된 공유 버전은 `Log` 사용. 이는 pa-pb-dedup 단계 1에서 수행된 SLog→Log 마이그레이션과 일치. `Log` 매크로는 표준 헤더를 통해 모든 PW 파일에서 사용 가능.

## 영향

| 메트릭 | 값 |
|--------|-------|
| 수정된 파일 | 17 (15 소스 + 2 라이브러리) |
| 제거된 함수 | 15개 로컬 복사본 |
| 추가된 함수 | 1개 공유 |
| 제거된 데드 코드 | 3개 미사용 정의 |
| 추정 제거된 줄 | ~240 (15 정의 x ~16 줄 각) |
| 추정 추가된 줄 | ~12 (1 정의 + 1 선언) |
| 순 변화 | ~-228 줄 |
| 유지보수 비용 감소 | 15개 복사본 → 1개 (-93%) |

## 위험 평가

- **LOW**: 순수 기계적 추출 — 동일 로직, fd만 매개변수화됨
- **서명 변경**: `void` → `int fd`는 유일한 동작 차이; 호출자는 로컬 버전이 사용한 동일 fd 전달
- **빌드 검증**: `mk.sh sub` (라이브러리 재빌드) 그 후 `mk.sh src` (모든 모듈 재빌드)
- **동작 변화 없음**: Linger 동작 (close 시 즉시 RST)은 동일

## 범위 외

- linger 값 변경 (l_onoff, l_linger)
- 기존 Log 이상의 오류 처리 추가
- 이들 파일의 다른 함수 수정
- BACK2025/ 백업 파일 (활성 코드 아님)

## 성공 기준

1. 15개 로컬 정의 모두 제거됨
2. 올바른 fd 인수로 모든 12개 활성 호출 사이트 업데이트됨
3. `sub/fep_common.c`의 공유 함수가 `libfepP.a`로 컴파일됨
4. `inc/fep_common.h`의 선언
5. `src/`에 `Set_Socket_Linger` 정의는 없고 호출만 있음
