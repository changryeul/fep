# PX 모듈 코드 품질 감사 — 설계 문서

> **요약**: 35개 PX 소스 파일 중 38개 발견에 대한 FR당 구현 사양
>
> **프로젝트**: FEP (KRX 프론트엔드 프로세서)
> **작성자**: Claude Code
> **날짜**: 2026-02-27
> **상태**: 초안
> **계획 참조**: `docs/01-plan/features/PX.plan.md`

---

## 구현 순서

4개 배치, 순차 실행. 각 배치는 독립적으로 컴파일 가능.

| 배치 | FR | 파일 | 주제 | 우선순위 |
|-------|-----|-------|-------|----------|
| 1 | FR-01부터 FR-07 | 7 | Critical: 잘못된 인덱스, 누락된 리턴, 초기화되지 않은 변수 | CRITICAL |
| 2 | FR-08부터 FR-30 | 23 | 전처리기 `defined()` 구문 | HIGH |
| 3 | FR-31부터 FR-36 | 5 | 초기화되지 않은 플래그, `*buf==NULL`, 하드코딩된 호스트명, 도달할 수 없는 코드, 빈 스텁 | MEDIUM |
| 4 | FR-37, FR-38 | 4 | 사용하지 않는 변수 | LOW |

### 의도적으로 연기됨

| ID | 이슈 | 이유 |
|----|-------|--------|
| D-1 | `px_chkgap.c` 빈 main | 함수 코드가 없는 자리 표시자 파일. 아마도 사용하지 않는 레거시 아티팩트. FR-35로 최소 `return 0;` 수정으로 처리됨. |
| D-2 | `*.back` 파일 버그 | 백업 파일 — 컴파일되지 않음, 배포되지 않음 |

---

## 배치 1: Critical 수정 (FR-01부터 FR-07)

### FR-01: px_showsise.c:115의 잘못된 배열 인덱스 `Shm_FinFut[i]`

`Key_Search()` 후 결과 인덱스는 `rt`에 있지만, 줄 115는 `i` (외부 루프 변수, 관계없음)를 사용. 이것은 잘못된 항목의 데이터를 인쇄합니다.

**이전:**
```c
			rt = Key_Search (DEV_MK_FIF, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_FinFut[i].A0.tr_gbn);
```

**새로 만들기:**
```c
			rt = Key_Search (DEV_MK_FIF, KEY_EXPCODE, (char *)&Key);
			if (rt > 0)
			{
				printf ("A0  rt[%d][%s]\n", rt, Shm_FinFut[rt].A0.tr_gbn);
```

**근거**: `rt`는 `Key_Search()`가 반환한 검색 결과 인덱스입니다. 뒤따르는 `printf`는 찾은 항목의 데이터를 표시해야 하며, 외부 루프의 반복 변수 `i`는 아닙니다.

### FR-02: Change_File_Name()의 누락된 리턴 — px_setfname.c:180-196

함수는 `int`를 반환하지만 for-루프 내에만 반환 경로가 있습니다 (`return (fk + 1)`), 마지막 반복에 `exit(FAIL)`이 있습니다. `DAEMON(Dk).f_count == 0`이면 루프 본문이 실행되지 않아 리턴 없이 끝에 도달합니다.

**이전:**
```c
int		Change_File_Name (void)
{
	int		fk;

	for (fk = 0; fk < DAEMON(Dk).f_count; fk ++)
	{
		if (memcmp (NewFile, FILEM(Dk,fk).file_name, 10) == 0)
			return (fk + 1);

		if (fk == DAEMON(Dk).f_count - 1)
		{
			Log (PRO_FATAL, "%s unregistered", NewFile);
			exit (FAIL);
		}
	}
}	/* End of Change_File_Name ()	*/
```

**새로 만들기:**
```c
int		Change_File_Name (void)
{
	int		fk;

	for (fk = 0; fk < DAEMON(Dk).f_count; fk ++)
	{
		if (memcmp (NewFile, FILEM(Dk,fk).file_name, 10) == 0)
			return (fk + 1);

		if (fk == DAEMON(Dk).f_count - 1)
		{
			Log (PRO_FATAL, "%s unregistered", NewFile);
			exit (FAIL);
		}
	}

	Log (PRO_FATAL, "%s unregistered (f_count=0)", NewFile);
	exit (FAIL);
	return (0);
}	/* End of Change_File_Name ()	*/
```

**근거**: 루프 후 추가된 코드는 `f_count == 0`인 엣지 케이스를 처리합니다. `exit(FAIL)` 뒤의 `return (0)`은 도달할 수 없지만 모든 코드 경로에 대해 반환 값을 요구하는 컴파일러를 만족시킵니다. `exit(FAIL)`은 등록된 파일이 없으면 프로세스가 오류로 종료되도록 보장합니다.

### FR-03: Change_Dshm_Name()의 누락된 리턴 — px_setdname.c:180-196

FR-02와 동일한 패턴이지만 DSHM (데이터 공유 메모리) 이름 검색용입니다.

**이전:**
```c
int		Change_Dshm_Name (void)
{
	int		fk;

	for (fk = 0; fk < DAEMON(Dk).d_count; fk ++)
	{
		if (memcmp (NewDshm, DSHM(Dk,fk).data_name, 10) == 0)
			return (fk + 1);

		if (fk == DAEMON(Dk).d_count - 1)
		{
			Log (PRO_FATAL, "%s unregistered", NewDshm);
			exit (FAIL);
		}
	}
}	/* End of Change_Dshm_Name ()	*/
```

**새로 만들기:**
```c
int		Change_Dshm_Name (void)
{
	int		fk;

	for (fk = 0; fk < DAEMON(Dk).d_count; fk ++)
	{
		if (memcmp (NewDshm, DSHM(Dk,fk).data_name, 10) == 0)
			return (fk + 1);

		if (fk == DAEMON(Dk).d_count - 1)
		{
			Log (PRO_FATAL, "%s unregistered", NewDshm);
			exit (FAIL);
		}
	}

	Log (PRO_FATAL, "%s unregistered (d_count=0)", NewDshm);
	exit (FAIL);
	return (0);
}	/* End of Change_Dshm_Name ()	*/
```

### FR-04: px_chkgap_auto1.c:18-19의 초기화되지 않은 변수

`recv_sec`, `send_sec`, `start_tm`, `end_tm`은 초기화 없이 선언됩니다. 데이터 파일이 비어 있으면 (루프 반복 없음), 줄 211과 221에서 초기화되지 않은 상태로 사용됩니다.

**이전:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec, send_sec;
	double	tot_tm, krx_tm, fep_tm, start_tm, end_tm;
```

**새로 만들기:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec = 0, send_sec = 0;
	double	tot_tm, krx_tm, fep_tm, start_tm = 0, end_tm = 0;
```

**근거**: `avg_*`는 줄 39에서 초기화됩니다 (`= 0`), 하지만 `recv_sec`, `send_sec`, `start_tm`, `end_tm`은 파일 읽기 루프 내에서만 할당됩니다. 루프에 반복이 없으면 줄 211 (`end_tm = send_sec`)과 221 (`end_tm - start_tm`)은 결정되지 않은 값을 사용합니다.

### FR-05: px_chkgap_man.c:18-19의 초기화되지 않은 변수

FR-04와 동일한 패턴.

**이전:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec, send_sec;
	double	tot_tm, krx_tm, fep_tm, start_tm, end_tm;
```

**새로 만들기:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec = 0, send_sec = 0;
	double	tot_tm, krx_tm, fep_tm, start_tm = 0, end_tm = 0;
```

### FR-06: px_chkgap_auto3.c:18-19의 초기화되지 않은 변수

FR-04와 동일한 패턴.

**이전:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec, send_sec;
	double	tot_tm, krx_tm, fep_tm, start_tm, end_tm;
```

**새로 만들기:**
```c
	double	avg_tot, avg_krx, avg_fep, recv_sec = 0, send_sec = 0;
	double	tot_tm, krx_tm, fep_tm, start_tm = 0, end_tm = 0;
```

### FR-07: px_runstop.c:18의 초기화되지 않은 `run_flag`/`reload_flag`

`run_flag`와 `reload_flag`는 줄 18에서 `u_char`로 초기화 없이 선언됩니다. `run_flag`는 첫 번째 while 루프 내 줄 81에서 할당됩니다. `reload_flag`는 두 번째 while 루프 내 줄 130에서 할당됩니다. 하지만 `run_flag == 's'` (정지 경로)이면 `query_flag`가 0으로 설정되어 두 번째 while 루프가 완전히 건너뛰어집니다 — `reload_flag`는 줄 174의 로그에서 초기화되지 않은 상태로 사용됩니다.

**이전:**
```c
	u_char	run_flag, reload_flag, query_flag;
```

**새로 만들기:**
```c
	u_char	run_flag = 0, reload_flag = 0, query_flag;
```

**근거**: 둘 다 `0`으로 초기화합니다. `query_flag`는 항상 줄 34에서 할당됩니다 (`query_flag = 1`) 사용 전. `run_flag`는 항상 줄 81에서 확인 전에 할당되지만, `0`으로 초기화하는 것이 방어적입니다. `reload_flag`는 실제 버그가 있습니다 — 정지 경로에서 초기화되지 않음 — 따라서 초기화가 필요합니다.

---

## 배치 2: 전처리기 `defined()` 구문 (FR-08부터 FR-30)

### 패턴 설명

모든 23개 파일이 동일한 패턴을 공유합니다:

```c
// 현재 (버그):
#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
```

`#elif` 줄은 `defined(sun) || _AIX || __linux`로 평가됩니다. `_AIX`와 `__linux`는 `defined()`로 감싸지지 않았으므로, 전처리기는 이들을 숫자 표현식으로 평가합니다. Linux에서 `__linux`는 `1`로 정의되므로 조건이 우연히 참입니다. 이들 중 어느 것도 정의되지 않은 가상 플랫폼에서는 전체 `#elif` 블록이 건너뛰어져 `fp`가 초기화되지 않습니다.

**모든 23개 파일에 대한 수정** — `#elif` 줄을 교체합니다:

**이전:**
```c
#elif defined sun || _AIX || __linux
```

**새로 만들기:**
```c
#elif defined(sun) || defined(_AIX) || defined(__linux)
```

### 파일 목록 (23개 파일)

| FR | 파일 | 줄 |
|----|------|------|
| FR-08 | px_setudp.c | 151 |
| FR-09 | px_runstop.c | 167 |
| FR-10 | px_setcseq.c | 93 |
| FR-11 | px_setdate.c | 80 |
| FR-12 | px_setdelay.c | 95 |
| FR-13 | px_setdname.c | 163 |
| FR-14 | px_setdsize.c | 94 |
| FR-15 | px_setdtime.c | 105 |
| FR-16 | px_setfcnt.c | 184 |
| FR-17 | px_setfname.c | 163 |
| FR-18 | px_setfsize.c | 94 |
| FR-19 | px_sethandsk.c | 100 |
| FR-20 | px_setinfostat.c | 82 |
| FR-21 | px_setlstat.c | 144 |
| FR-22 | px_setnstat.c | 170 |
| FR-23 | px_setpseq.c | 93 |
| FR-24 | px_setpstat.c | 123 |
| FR-25 | px_setptime.c | 125 |
| FR-26 | px_setsisereco.c | 131 |
| FR-27 | px_setsstat.c | 108 |
| FR-28 | px_sett2ip.c | 153 |
| FR-29 | px_sett2seq.c | 109 |
| FR-30 | px_settout.c | 94 |

**구현**: `replace_all: true`를 Edit 도구와 함께 사용합니다. 모든 파일이 동일한 패턴의 정확히 하나의 인스턴스를 가지므로. 단일 `old_string` / `new_string` 쌍이 모든 23개 파일에 적용됩니다.

---

## 배치 3: Medium 수정 (FR-31부터 FR-36)

### FR-31: px_memok.c:768의 초기화되지 않은 `continue_flag`

`continue_flag`는 초기화 없이 선언됩니다. 서비스 포트 키가 일치하는 경우 중첩된 for-루프 내에서 줄 907에서 `1`로 설정됩니다. 줄 912에서 확인됩니다 (`if (continue_flag == 1) continue;`). 버그: 한 번 `1`로 설정되면 `continue_flag`는 다시 `0`으로 재설정되지 않아 `continue`를 통해 모든 다음 설정 줄을 자동으로 건너뜁니다.

**이전 (줄 768):**
```c
	int 	i, j, cnt, itemcnt, continue_flag;
```

**새로 만들기 (줄 768):**
```c
	int 	i, j, cnt, itemcnt, continue_flag = 0;
```

또한 내부 for-루프 전에 재설정을 추가합니다 (줄 898 전):

**이전 (줄 898):**
```c
		for (j = 0; j < 9; j ++)
```

**새로 만들기 (줄 898):**
```c
		continue_flag = 0;
		for (j = 0; j < 9; j ++)
```

**근거**: 두 가지 수정이 필요합니다: (1) 첫 번째 반복을 위해 선언 시 초기화, (2) 각 내부 루프 확인 전에 재설정하여 이전 반복에서 지속되지 않도록. 재설정이 없으면 어떤 `TCP1_n_SPORTnn` 키가 일치하면 이후 모든 줄 (관련 없는 설정 키 포함)이 자동으로 건너뛰어집니다.

### FR-32: px_cfgback.c:460의 `*buf == NULL`

`*buf`는 `char` (역참조된 포인터). 비교 `char` 값을 `NULL` (포인터 상수, 보통 `(void *)0`)과 비교하는 것은 타입 불일치입니다. 의도는 널 종결자 문자 `'\0'`을 확인하는 것입니다.

**이전:**
```c
		else if (*buf == NULL || *buf == '\t' || *buf == ' ')
```

**새로 만들기:**
```c
		else if (*buf == '\0' || *buf == '\t' || *buf == ' ')
```

**근거**: `NULL`은 포인터 상수입니다. `*buf == NULL`은 컴파일되지만 (암묵적 정수 비교 0), 의미론적으로는 잘못되었고 엄격한 컴파일러로 경고가 생성됩니다. `'\0'`은 문자열 끝 확인을 위한 올바른 널 문자 리터럴입니다.

### FR-33: (FR-07과 통합됨)

`px_runstop.c`의 `reload_flag` 초기화 문제는 배치 1의 FR-07로 처리됩니다. 추가 작업이 필요하지 않습니다.

### FR-34: px_chkgap_auto1.c:111의 하드코딩된 호스트명 "ap67"

호스트명 `"ap67"`은 PB (채권) 및 JC (선물) 로그 경로 중 선택하도록 하드코딩됩니다. 이것은 다른 명명 규칙을 가진 배포 호스트에서 손상됩니다.

**이전:**
```c
		if (memcmp (env_hostname, "ap67", 4) == 0)
			sprintf (path, "%s/PB/%s/pb_1252_ts", env_plog, day);
		else
			sprintf (path, "%s/JC/%s/jc_1251_ts", env_jlog, day);
```

**새로 만들기:**
```c
		if (memcmp (env_hostname, "ap67", 4) == 0 ||
			memcmp (env_hostname, "podm11", 6) == 0 ||
			memcmp (env_hostname, "podm12", 6) == 0)
			sprintf (path, "%s/PB/%s/pb_1252_ts", env_plog, day);
		else
			sprintf (path, "%s/JC/%s/jc_1251_ts", env_jlog, day);
```

**근거**: 이것은 진단 도구 (갭 확인자)입니다. 원래 호스트명 `"ap67"`은 레거시 호스트입니다. 프로덕션 호스트는 `pkg_env.sh`에 따라 `podm11` (REAL1)과 `podm12` (REAL2)입니다. 이들을 추가하면 도구가 현재 프로덕션 서버에서 작동합니다. 폴백 (`else`)은 테스트 환경의 기본값으로 JC 경로로 유지됩니다.

> **위험**: 낮음. 이것은 독립 실행형 진단 유틸리티, 데몬 프로세스 아님. 호스트명 목록은 새 프로덕션 호스트가 배포될 경우 추가 업데이트가 필요할 수 있습니다. 향후 개선을 위해 환경 변수 (예: `_FEP_DIV`)를 사용하여 더 강력한 확인을 고려합니다.

### FR-35: px_chkgap.c의 빈 main() 스텁

파일에 값을 반환하지 않는 빈 `main()`이 있습니다. `main()`이 `int`를 반환하므로 C89/C99에 따라 이것은 정의되지 않은 동작입니다.

**이전:**
```c
int		main (int argc, char *argv[])
{
}	/* End of main ()	*/
```

**새로 만들기:**
```c
int		main (int argc, char *argv[])
{
	return (0);
}	/* End of main ()	*/
```

**근거**: 최소 수정. 파일은 자리 표시자 스텁입니다 (실제 갭 확인 로직은 `px_chkgap_auto1.c`, `px_chkgap_man.c`, `px_chkgap_auto3.c`에 있음). `return (0)`을 추가하면 경고 없이 깔끔하게 컴파일됩니다.

### FR-36: px_chktrcnt.c:97의 도달할 수 없는 `exit(1)`

줄 58-95의 if/else 체인 후 모든 분기가 `exit()`을 호출합니다. 줄 97의 `exit(1)`은 도달할 수 없는 데드 코드입니다.

**이전:**
```c
    else
    {
        printf ("input parameter error [para 2 - %s] ...!\n", argv[1]);
        exit (1);
    }

    exit (1);
} /* end of main */
```

**새로 만들기:**
```c
    else
    {
        printf ("input parameter error [para 2 - %s] ...!\n", argv[1]);
        exit (1);
    }
} /* end of main */
```

**근거**: `if (memcmp(argv[1], "pa", 2) == 0)` 블록이 줄 89에서 종료됩니다. `else` 블록이 줄 94에서 종료됩니다. 줄 97의 `exit(1)`은 절대 도달할 수 없습니다. 이것을 제거하면 데드 코드가 제거되고 잠재적 컴파일러 경고가 방지됩니다.

---

## 배치 4: Low 수정 (FR-37, FR-38)

### FR-37: px_sethandsk.c:17의 사용하지 않는 `fifo_name[128]`

`fifo_name`이 선언되지만 파일의 어디에서도 읽거나 쓰지 않습니다.

**이전:**
```c
	char	proc_id[12], pstat[4], sub[4], buf[128], fifo_name[128];
```

**새로 만들기:**
```c
	char	proc_id[12], pstat[4], sub[4], buf[128];
```

### FR-38: 3개 chkgap 파일의 사용하지 않는 `sub[4]`

`sub[4]`는 줄 42에서 선언되고 memset되지만 세 개 갭 확인자 파일에서 읽히지 않습니다.

#### FR-38a: px_chkgap_auto1.c:20

**이전:**
```c
	char	sub[4], yn[4], chk_from[12], chk_to[12], buf[5120];
```

**새로 만들기:**
```c
	char	yn[4], chk_from[12], chk_to[12], buf[5120];
```

또한 해당 memset을 제거합니다 (줄 42):

**이전:**
```c
	memset (sub, 0, sizeof (sub));
	memset (yn, 0, sizeof (yn));
```

**새로 만들기:**
```c
	memset (yn, 0, sizeof (yn));
```

#### FR-38b: px_chkgap_man.c:20

FR-38a와 동일한 변경. 선언 (줄 20)에서 `sub[4]` 제거 및 그 `memset` (줄 42).

#### FR-38c: px_chkgap_auto3.c:20

FR-38a와 동일한 변경. 선언 (줄 20)에서 `sub[4]` 제거 및 그 `memset` (줄 42).

---

## 검증

### 빌드 검증

```sh
cd st01/shl
./mk.sh px
```

모든 PX 바이너리는 수정과 관련된 오류 또는 경고 없이 컴파일되어야 합니다.

### 변경 요약

| 메트릭 | 개수 |
|--------|-------|
| 수정된 파일 | 31 (배치1 7 + 배치2 23 + 배치3 5 + 배치4 4, 겹침 포함: px_runstop.c 배치1+2, px_setfname.c 배치1+2, px_setdname.c 배치1+2, px_sethandsk.c 배치2+4, px_chkgap_auto1.c 배치1+3+4) |
| 고유 수정 파일 | 28 |
| CRITICAL 수정 | 7 (FR-01부터 FR-07) |
| HIGH 수정 | 23 (FR-08부터 FR-30: 전처리기 `defined()`) |
| MEDIUM 수정 | 5개 고유 (FR-31, FR-32, FR-34, FR-35, FR-36; FR-33 FR-07과 통합) |
| LOW 수정 | 2개 (FR-37, FR-38 4개 파일 중) |
| 순 예상 줄 | ~+10 (배치1은 ~8줄 추가, 배치2는 제자리 교체, 배치3은 ~3줄 추가, 배치4는 ~6줄 제거) |

### 영향을 받은 주요 바이너리

| 바이너리 | 적용된 FR |
|--------|-------------|
| px_showsise | FR-01 |
| px_setfname | FR-02, FR-17 (전처리기) |
| px_setdname | FR-03, FR-13 (전처리기) |
| px_chkgap_auto1 | FR-04, FR-34, FR-38a |
| px_chkgap_man | FR-05, FR-38b |
| px_chkgap_auto3 | FR-06, FR-38c |
| px_runstop | FR-07, FR-09 (전처리기) |
| px_memok | FR-31 |
| px_cfgback | FR-32 |
| px_chkgap | FR-35 |
| px_chktrcnt | FR-36 |
| px_sethandsk | FR-19 (전처리기), FR-37 |
| 22개 다른 px_set* 유틸리티 | FR-08부터 FR-30 (전처리기) |

---

## 버전 역사

| 버전 | 날짜 | 변경 사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-27 | 모든 38개 발견에 대한 검증된 FR 사양을 가진 초기 설계 | Claude Code |
