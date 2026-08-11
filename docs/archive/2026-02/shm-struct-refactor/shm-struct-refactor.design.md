# shm-struct-refactor 설계 문서

> **요약**: SHM 접근 코드의 type-safety 강화 및 구조체 기반 리팩토링 설계
>
> **프로젝트**: FEP (Front-End Processor) for KRX
> **Author**: Claude Code
> **Date**: 2026-02-21
> **Status**: Draft
> **Plan**: [shm-struct-refactor.plan.md](../../01-plan/features/shm-struct-refactor.plan.md)

---

## 1. Current State Analysis

### 1.1 SHM Memory Layout (shm_memory.h, 1,079줄)

```
Daemon SHM (BASE_SHM_KEY: 0x41000000)
├── ALL_DAEMON_INFO[26]         ← SHM_MAX_SUB=26 (A~Z)
│
Sub-Daemon SHM (per module, e.g. 0x41010000 for PB)
├── SUB_DAEMON_INFO             ← Shm_Mem[i].Daemon
├── PROCESS_INFO[count]         ← Shm_Mem[i].Proc
├── FILE_INFO[count]            ← Shm_Mem[i].File
├── DSHM_INFO[count]            ← Shm_Mem[i].DShm
├── TCP1_INFO[count]            ← Shm_Mem[i].Tcp1
├── TCP2_INFO[count]            ← Shm_Mem[i].Tcp2
├── UDPIP_INFO[count]           ← Shm_Mem[i].Udpip
├── Data Buffer[SHM_DATA_SIZE * data_count]  ← circular buffer
└── SHM Log[SHM_LOG_SIZE * 10000]            ← optional

Market Data SHMs (separate segments):
├── SHM_NOTE[10,000]            ← NOTE_SHM_KEY   0x41000001
├── CO_A001_RDS01[100,000]      ← RDS01_SHM_KEY  0x41000002
├── SHM_FIN_FUT[1,000]          ← FF_SHM_KEY     0x41000003
├── STRRG[20]                   ← STRRG_SHM_KEY  0x41000012
├── MK_PREMATCH[2]              ← MK_PM_SHM_KEY  0x41000013
├── RISK[1]                     ← RISK_SHM_KEY   0x41000014
└── SHM_KEY_ARRY[1]             ← ITEM_SHM_KEY   0x41000030
```

### 1.2 문제 패턴 식별

#### Pattern A: Raw Pointer Offset Chain (shmsub.c:90-107)

```c
/* 현재 코드 — raw char* ptr에 sizeof 누적 덧셈 */
Shm_Mem[i].Daemon = (SUB_DAEMON_INFO *)Shmptr;
Shmptr += sizeof (SUB_DAEMON_INFO);
Shm_Mem[i].Proc = (PROCESS_INFO *)Shmptr;
Shmptr += sizeof (PROCESS_INFO) * INFO(i).process_count;
Shm_Mem[i].File = (FILE_INFO *)Shmptr;
Shmptr += sizeof (FILE_INFO) * INFO(i).file_count;
// ... 7개 섹션 반복
```

**문제점**: Shmptr가 `char*` 타입으로 선언되어 수동 포인터 산술 필요. 순서가 바뀌거나 하나라도 빠지면 전체 오프셋이 어긋남.

#### Pattern B: DSHM Write 3중 복제 (shm_rw.c)

| 함수 | 줄 수 | 차이점 |
|------|-------|--------|
| `DSHM_W` (199-330) | 131줄 | p_out/10, 루프 내 multi-record write, D_K/P_K 사용 |
| `DSHM_W2` (343-460) | 117줄 | p_out 직접 사용, 단일 record write, D_K/P_K 사용 |
| `DSHM_WT` (473-739) | 266줄 | pa_8201_tr 전용 test 함수, 자체 SHM attach, 0/pk 사용 |

공통 로직 (FILE_RW_HEAD 포맷팅, circular buffer offset 계산, SEM lock/unlock, FIFO notify)이 ~80줄씩 3번 복제됨.

#### Pattern C: 매크로 중첩 (shm_memory.h:886)

```c
#define TCP1_PORT(i,j) TCP1(i,PROC(i,j).l.t1.line_gubun-1).port_no
// 전개 순서: TCP1_PORT → TCP1 → Shm_Mem[i].Tcp1[...] → PROC → Shm_Mem[i].Proc[j].l.t1.line_gubun-1
// 총 4단계 치환, 5개 배열 인덱스 참조
```

### 1.3 Size 계산 비교: Create vs Attach

`pz_memory_shm.c:Mem_SHM_Creat()` (생성)과 `shmsub.c:Mem_SHM()` (attach) 비교:

| 항목 | Mem_SHM_Creat (생성) | Mem_SHM (attach) |
|------|---------------------|------------------|
| Size 변수 | `Info[i].Shmsize`, `Info[i].FShmsize` 등 | 직접 계산 |
| Size 추적 | 개별 + 합산 | 합산만 |
| DAEMON copy | `DAEMON(i).Shmsize = Info[i].Shmsize` | 없음 |
| Offset mapping | 동일한 ptr += sizeof() 패턴 | 동일한 ptr += sizeof() 패턴 |

**결론**: 두 함수가 동일 순서/크기로 계산하고 있으므로 레이아웃 일관성은 유지됨. 그러나 둘 다 동일한 코드 중복.

---

## 2. Design Specification

### 2.1 FR-01: shmsub.c struct 기반 매핑

#### Before (현재)

```c
void Mem_SHM (int flag, int dk)
{
    // Shmsize 수동 계산 (62-74줄)
    Shmsize = sizeof(PROCESS_INFO) * INFO(i).process_count +
              sizeof(FILE_INFO) * INFO(i).file_count + ...;
    Shmsize += sizeof(SUB_DAEMON_INFO);

    // ptr 체인 매핑 (90-107줄)
    Shmptr = Shm_Attach(shm_key, &Mem_Shmid[i]);
    Shm_Mem[i].Daemon = (SUB_DAEMON_INFO *)Shmptr;
    Shmptr += sizeof(SUB_DAEMON_INFO);
    Shm_Mem[i].Proc = (PROCESS_INFO *)Shmptr;
    Shmptr += sizeof(PROCESS_INFO) * INFO(i).process_count;
    // ... 7개 반복
}
```

#### After (리팩토링)

```c
/*--------------------------------------------------------------
    Shm_Map_SubDaemon: SHM 세그먼트를 SHM_MEMORY 구조체에 매핑
    - base: SHM 시작 주소
    - info: ALL_DAEMON_INFO (count 정보 포함)
    - mem:  출력 SHM_MEMORY 포인터
    Return: 매핑 후 다음 주소 (Data/Log 영역 시작)
--------------------------------------------------------------*/
static char *Shm_Map_SubDaemon(char *base, ALL_DAEMON_INFO *info,
                               SHM_MEMORY *mem)
{
    char *ptr = base;

    mem->Daemon = (SUB_DAEMON_INFO *)ptr;
    ptr += sizeof(SUB_DAEMON_INFO);

    mem->Proc = (PROCESS_INFO *)ptr;
    ptr += sizeof(PROCESS_INFO) * info->process_count;

    mem->File = (FILE_INFO *)ptr;
    ptr += sizeof(FILE_INFO) * info->file_count;

    mem->DShm = (DSHM_INFO *)ptr;
    ptr += sizeof(DSHM_INFO) * info->dshm_count;

#if defined ISAM_INCL
    mem->Cisam = (CISAM_INFO *)ptr;
    ptr += sizeof(CISAM_INFO) * info->cisam_count;
#endif

    mem->Tcp1 = (TCP1_INFO *)ptr;
    ptr += sizeof(TCP1_INFO) * info->tcp1_count;

    mem->Tcp2 = (TCP2_INFO *)ptr;
    ptr += sizeof(TCP2_INFO) * info->tcp2_count;

    mem->Udpip = (UDPIP_INFO *)ptr;
    ptr += sizeof(UDPIP_INFO) * info->udpip_count;

    return ptr;
}
```

**호출부 변경 (shmsub.c Mem_SHM, pz_memory_shm.c Mem_SHM_Creat 모두)**:

```c
/* Before: 17줄 ptr 체인 */
/* After: 1줄 호출 */
Shmptr = Shm_Attach(shm_key, &Mem_Shmid[i]);
SHM_Mem[i] = (char *)Shmptr;
ptr = Shm_Map_SubDaemon(Shmptr, &INFO(i), &Shm_Mem[i]);

/* Data buffer / SHM log 매핑은 ptr 이후 그대로 유지 */
if (INFO(i).data_count != 0) { ... }
if (INFO(i).shm_log == 1)   ShmLogPtr = ptr;
```

**배치**: `Shm_Map_SubDaemon()` 함수를 `sub/shmsub.c`에 static으로 추가.
`pz_memory_shm.c`에서도 동일 함수를 사용하려면 `sub/shmsub.c`에 non-static으로 두고 `inc/fep_sub.h`에 선언 추가.

**결정**: `shmsub.c`에 non-static으로 정의, `fep_sub.h`에 prototype 추가.

#### Size 계산 통합 함수

```c
/*--------------------------------------------------------------
    Shm_Calc_SubDaemon_Size: Sub-daemon SHM 전체 크기 계산
--------------------------------------------------------------*/
static size_t Shm_Calc_SubDaemon_Size(ALL_DAEMON_INFO *info)
{
    size_t sz = sizeof(SUB_DAEMON_INFO);

    sz += sizeof(PROCESS_INFO) * info->process_count;
    sz += sizeof(FILE_INFO)    * info->file_count;
    sz += sizeof(DSHM_INFO)    * info->dshm_count;
#if defined ISAM_INCL
    sz += sizeof(CISAM_INFO)   * info->cisam_count;
#endif
    sz += sizeof(TCP1_INFO)    * info->tcp1_count;
    sz += sizeof(TCP2_INFO)    * info->tcp2_count;
    sz += sizeof(UDPIP_INFO)   * info->udpip_count;

    if (info->data_count != 0)
        sz += SHM_DATA_SIZE * info->data_count;

    if (info->shm_log == 1)
        sz += SHM_LOG_SIZE * SHM_LOG_MAX;

    return sz;
}
```

---

### 2.2 FR-02: DSHM Write 함수 통합

#### 3함수 차이점 분석

```
                  DSHM_W              DSHM_W2             DSHM_WT
입력 p_out       p_out / 10          p_out (직접)        p_out / 10
레코드 수        multi (루프)         단일 (1건)          multi (루프)
dk/pk            D_K, P_K            D_K, P_K            0, pk (hardcode)
SHM 초기화       기존 연결 사용       기존 연결 사용       자체 attach (pa_8201_tr)
용도             일반 write           단건 write           테스트용
```

#### 통합 설계: DSHM_W_Core

```c
/*--------------------------------------------------------------
    DSHM_W_Core: circular buffer write 핵심 로직
    - dk, pk: daemon key, process key
    - fk: file key (0-based)
    - p_buf: 입력 데이터 버퍼
    - p_cnt: 레코드 수
    Return: 기록 건수 또는 NOTOK
--------------------------------------------------------------*/
static int DSHM_W_Core(int dk, int pk, int fk,
                        char *p_buf, int p_cnt)
{
    short   sk;
    int     rt, rec_size, cnt, i, f_size;
    long    offset;
    u_char  fifo_cnt;
    char    *cq, Tmp[128];
    FILE_RW_HEAD    file_rw;
    BUFF_RW_HEAD    buff_rw;

    sk = AtoIf(ODK(dk, pk, fk), 2) - 1;
    fifo_cnt = ODC(dk, pk, fk);
    rec_size = sizeof(BUFF_RW_HEAD) + ODS(dk, pk, fk) + 1;
    f_size = rec_size - sizeof(BUFF_RW_HEAD) + sizeof(FILE_RW_HEAD);

    if (SemId[fk] != -1)
        SEM_Lock(SemId[fk]);

    cnt = ODW(dk, pk, fk, 0) + 1;
    memset(Tmp, 0, sizeof(Tmp));

    for (i = 0; i < rec_size * p_cnt; i += rec_size, cnt++)
    {
        offset = (ODW(dk, pk, fk, 0) % ODM(dk, pk, fk)) * f_size;

        memcpy(buff_rw.Seq, p_buf + i, sizeof(BUFF_RW_HEAD));

        /* FILE_RW_HEAD 포맷팅 */
        sprintf(Tmp, "[%08d]", cnt);
        memcpy(file_rw.Seq, Tmp, 10);
        sprintf(Tmp, "[%-8.8s]", buff_rw.If_Seq);
        memcpy(file_rw.If_Seq, Tmp, 10);
        sprintf(Tmp, "[%-8.8s]", buff_rw.ApType);
        memcpy(file_rw.ApType, Tmp, 10);
        sprintf(Tmp, "[%-4.4s]", buff_rw.ResponseCode);
        memcpy(file_rw.ResponseCode, Tmp, 6);
        sprintf(Tmp, "[%-10.10s]", buff_rw.RecvTime1);
        memcpy(file_rw.RecvTime1, Tmp, 12);
        sprintf(Tmp, "[%-12.12s]", buff_rw.RecvTime2);
        memcpy(file_rw.RecvTime2, Tmp, 14);
        sprintf(Tmp, "[%-20.20s]", buff_rw.DataHeader);
        memcpy(file_rw.DataHeader, Tmp, 22);

        /* circular buffer write */
        cq = DShmPtr[sk] + ODO(dk, pk, fk);
        memcpy(&cq[offset], file_rw.Seq, sizeof(FILE_RW_HEAD));
        memcpy(&cq[offset + sizeof(FILE_RW_HEAD)],
               &p_buf[i + sizeof(BUFF_RW_HEAD)],
               rec_size - sizeof(BUFF_RW_HEAD));

        ODW(dk, pk, fk, 0)++;
    }

    if (SemId[fk] != -1)
        SEM_UnLock(SemId[fk]);

    /* FIFO notify */
    for (i = 0; i <= fifo_cnt; i++)
    {
        if (OD_FIFO_fd[fk][i] <= 0)
        {
            Log(FIF_FATAL, "DSHM_W:cannot open FIFO[%d][%d:%s]",
                OD_FIFO_fd[fk][i], SYS_NO, SYS_STR);
            continue;
        }
        rt = write(OD_FIFO_fd[fk][i], "1", 1);
        if (rt < 0)
            Log(FIF_FATAL, "DSHM_W:cannot write FIFO[%d][%d:%s]",
                OD_FIFO_fd[fk][i], SYS_NO, SYS_STR);
    }

    return p_cnt;
}
```

#### 기존 API 호환 래퍼

```c
/* 기존 DSHM_W: validation + p_out/10 + multi-record */
int DSHM_W(int p_out, char *p_buf, int p_cnt)
{
    int fk;

    if (p_buf == NULL) {
        Log(USR_ERROR, "DSHM_W:write buffer is NULL");
        return (NOTOK);
    }
    if (p_cnt < 1 || 30 < p_cnt) {
        Log(USR_ERROR, "DSHM_W:invalid write count[%d]", p_cnt);
        return (NOTOK);
    }

    fk = (p_out / 10) - 1;
    if (fk < 0 || fk >= 99) {
        Log(USR_ERROR, "DSHM_W:invalid output[%d]", p_out);
        return (NOTOK);
    }

    return DSHM_W_Core(D_K, P_K, fk, p_buf, p_cnt);
}

/* 기존 DSHM_W2: validation + p_out 직접 + 단건 */
int DSHM_W2(int p_out, char *p_buf, int p_cnt)
{
    int fk;

    if (p_buf == NULL) {
        Log(USR_ERROR, "DSHM_W:write buffer is NULL");
        return (NOTOK);
    }

    fk = p_out - 1;
    return DSHM_W_Core(D_K, P_K, fk, p_buf, p_cnt);
}
```

#### DSHM_WT 처리

`DSHM_WT`는 자체 SHM attach 로직 (~100줄)을 포함하는 **테스트 전용** 함수. 이 자체 초기화 로직은 유지하되, 핵심 write 부분만 `DSHM_W_Core(0, pk, fk, ...)` 호출로 교체.

**예상 삭제량**: ~160줄 (DSHM_W2 전체 + DSHM_W/WT의 중복 write 로직)

---

### 2.3 FR-03: SHM_VERSION 추가

#### shm_memory.h 추가

```c
/* shm_memory.h 상단, Constants 섹션에 추가 */
#define SHM_VERSION         20260221    /* YYYYMMDD format */
#define SHM_MAGIC           0x46455053  /* "FEPS" in ASCII */
```

#### SUB_DAEMON_INFO 확장 방안

**제약 조건**: 기존 바이너리와 100% 호환 필요.

**방안 A (선택)**: `process_info[40]` 필드의 미사용 영역 활용
- `SUB_DAEMON_INFO.process_info` (40바이트)는 현재 데몬 정보 문자열
- 마지막 8바이트를 version marker로 사용 (process_info[32..39])
- 장점: struct 크기 변경 없음, 기존 바이너리 호환
- 단점: 기존 info 문자열 32자 제한

**방안 B**: 새 필드 추가 (struct 끝에)
- `int shm_version;` / `int shm_magic;` 추가
- sizeof(SUB_DAEMON_INFO) 변경 → **모든 바이너리 재빌드 필요**

**결정**: 방안 A 채택. process_info 마지막 8바이트 활용.

```c
/* pz_memory_shm.c Mem_SHM_Creat()에서 SHM 생성 직후 */
{
    int ver = SHM_VERSION;
    int mag = SHM_MAGIC;
    memcpy(&DAEMON(i).process_info[32], &mag, 4);
    memcpy(&DAEMON(i).process_info[36], &ver, 4);
}
```

**검증 함수**:

```c
/* shmsub.c에 추가 */
int Shm_Check_Version(int dk)
{
    int mag, ver;
    memcpy(&mag, &DAEMON(dk).process_info[32], 4);
    memcpy(&ver, &DAEMON(dk).process_info[36], 4);

    if (mag != SHM_MAGIC) {
        Log(USR_WARN, "SHM[%d] no version marker (legacy)", dk);
        return 0;  /* legacy SHM, 정상 진행 */
    }

    Log(USR_OK, "SHM[%d] version=%d", dk, ver);
    return ver;
}
```

---

### 2.4 FR-04: shmipc.c attach 시 size 검증

#### SHM_Attach에 size 검증 추가

```c
/* shmipc.c — 기존 SHM_Attach 확장 */
char *SHM_Attach_Verify(int p_shmid, size_t expected_size)
{
    char            *rt;
    struct shmid_ds shm_stat;

    rt = shmat(p_shmid, (char *)0, 0);

    if (rt == (char *)-1)
        return rt;

    /* size 검증 */
    if (expected_size > 0)
    {
        if (shmctl(p_shmid, IPC_STAT, &shm_stat) == 0)
        {
            if (shm_stat.shm_segsz < expected_size)
            {
                Log(SAM_WARN,
                    "SHM size mismatch: actual=%d expected=%d",
                    (int)shm_stat.shm_segsz, (int)expected_size);
            }
        }
    }

    return rt;
}
```

**기존 API 호환**: `SHM_Attach`는 변경 없이 유지. 새 함수 `SHM_Attach_Verify`를 추가하여 점진적 도입.

---

### 2.5 FR-05: 매크로 문서화

shm_memory.h의 95개 접근 매크로에 대해 접근 경로 주석 추가:

```c
/* TCP1_PORT(i,j) 전개 경로:
   → TCP1(i, PROC(i,j).l.t1.line_gubun - 1).port_no
   → Shm_Mem[i].Tcp1[Shm_Mem[i].Proc[j].l.t1.line_gubun - 1].port_no
   참조: TCP1_INFO.port_no (u_short)
   용례: TCP1_PORT(D_K, P_K) — 현재 프로세스의 TCP1 마스터 포트
*/
#define TCP1_PORT(i,j)  TCP1(i,PROC(i,j).l.t1.line_gubun-1).port_no
```

주요 매크로 그룹별 문서화:
- 기본 접근자: `INFO`, `DAEMON`, `PROC`, `FILEM`, `DSHM`, `TCP1`, `TCP2`, `UDPIP` (8개)
- TCP1 체인: `TCP1_PORT`, `TCP1_IP`, `TCP1_P_ST` 등 (7개)
- TCP2 체인: `TCP2_PORT`, `TCP2_IP*`, `TCP2_*STAT` 등 (10개)
- UDP 체인: `UDP_PORT`, `UDP_IP*` 등 (8개)
- File I/O: `IFN`, `IFW`, `OFN`, `OFW` 등 (12개)
- DSHM I/O: `IDN`, `IDK`, `ODN`, `ODK` 등 (18개)

---

### 2.6 FR-06: Create ↔ Attach 정합성

`Shm_Calc_SubDaemon_Size()`와 `Shm_Map_SubDaemon()`을 **양쪽 함수에서 공유**하면 자동으로 정합성 보장.

```
pz_memory_shm.c:Mem_SHM_Creat()
    size = Shm_Calc_SubDaemon_Size(&INFO(i))
    SHM_Creat_Attach(key, size, &id)
    Shm_Map_SubDaemon(ptr, &INFO(i), &Shm_Mem[i])

shmsub.c:Mem_SHM()
    size = Shm_Calc_SubDaemon_Size(&INFO(i))   ← 검증용
    Shm_Attach(key, &id)
    Shm_Map_SubDaemon(ptr, &INFO(i), &Shm_Mem[i])
```

---

## 3. Implementation Order

| Phase | 파일 | FR | 작업 내용 | 의존성 |
|-------|------|----|----------|--------|
| 1 | `sub/shmsub.c` | FR-01 | `Shm_Map_SubDaemon()`, `Shm_Calc_SubDaemon_Size()` 추가, `Mem_SHM()` 리팩토링 | None |
| 2 | `src/PZ/pz_memory_shm.c` | FR-06 | `Mem_SHM_Creat()`에서 동일 함수 사용 | Phase 1 |
| 3 | `sub/shm_rw.c` | FR-02 | `DSHM_W_Core()` 추출, DSHM_W/W2/WT 리팩토링 | None |
| 4 | `inc/shm_memory.h` | FR-03,05 | `SHM_VERSION`/`SHM_MAGIC` 추가, 매크로 문서화 | None |
| 5 | `sub/shmipc.c` | FR-04 | `SHM_Attach_Verify()` 추가 | None |
| 6 | `inc/fep_sub.h` | — | 새 함수 prototype 추가 | Phase 1,3,5 |
| 7 | Build | — | `mk.sh sub` → `mk.sh all` 빌드 검증 | All |

Phase 1-5는 서로 독립적이므로 병렬 진행 가능. Phase 6-7은 순차.

---

## 4. File Change Summary

| 파일 | 변경 유형 | 추가 줄 | 삭제 줄 | 수정 줄 |
|------|----------|---------|---------|---------|
| `sub/shmsub.c` | 함수 추가 + 리팩토링 | +50 | -17 | ~20 |
| `sub/shm_rw.c` | 함수 추출 + 래퍼 | +60 | -160 | ~30 |
| `sub/shmipc.c` | 함수 추가 | +25 | 0 | 0 |
| `inc/shm_memory.h` | 매크로/주석 추가 | +30 | 0 | 0 |
| `inc/fep_sub.h` | prototype 추가 | +5 | 0 | 0 |
| `src/PZ/pz_memory_shm.c` | 함수 호출 변경 | +5 | -17 | ~5 |
| **합계** | | **+175** | **-194** | **~55** |

순 코드 감소: **-19줄** (중복 제거 효과)

---

## 5. Binary Compatibility Verification

### 5.1 Struct Size 불변 검증

리팩토링 전후 아래 sizeof가 동일해야 함:

```c
/* 검증 대상 (컴파일 타임) */
sizeof(SUB_DAEMON_INFO)  /* 변경 없음 */
sizeof(PROCESS_INFO)     /* 변경 없음 */
sizeof(FILE_INFO)        /* 변경 없음 */
sizeof(DSHM_INFO)        /* 변경 없음 */
sizeof(TCP1_INFO)        /* 변경 없음 */
sizeof(TCP2_INFO)        /* 변경 없음 */
sizeof(UDPIP_INFO)       /* 변경 없음 */
sizeof(SHM_MEMORY)       /* 변경 없음 */
sizeof(ALL_DAEMON_INFO)  /* 변경 없음 */
```

### 5.2 검증 방법

```c
/* mk.sh all 후 실행하여 모든 바이너리가 동일 sizeof 출력하는지 확인 */
#include "fep_sub.h"
int main() {
    printf("SUB_DAEMON_INFO: %zu\n", sizeof(SUB_DAEMON_INFO));
    printf("PROCESS_INFO:    %zu\n", sizeof(PROCESS_INFO));
    printf("FILE_INFO:       %zu\n", sizeof(FILE_INFO));
    printf("DSHM_INFO:       %zu\n", sizeof(DSHM_INFO));
    printf("SHM_MEMORY:      %zu\n", sizeof(SHM_MEMORY));
    return 0;
}
```

---

## 6. Conventions

| 항목 | 규칙 |
|------|------|
| 새 함수 위치 | `sub/shmsub.c` (매핑/계산), `sub/shm_rw.c` (write core) |
| 함수 네이밍 | `Shm_` prefix: `Shm_Map_SubDaemon`, `Shm_Calc_SubDaemon_Size`, `Shm_Check_Version` |
| 기존 API | DSHM_W, DSHM_W2, DSHM_WT 함수명/시그니처 유지 (내부만 변경) |
| static/non-static | 단일 파일 사용 → static, 다중 파일 사용 → non-static + fep_sub.h 선언 |
| C89 준수 | 변수 선언은 블록 시작, inline 미사용, // 주석은 기존 스타일 유지 |
| Version marker | `process_info[32..39]`에 MAGIC(4) + VERSION(4) 저장 |

---

## 7. Risk Mitigation

| Risk | 대응 |
|------|------|
| struct size 변경 | 어떤 구조체도 필드 추가/제거하지 않음. `process_info[40]` 기존 영역 재활용 |
| DSHM_WT 자체 init 로직 | Core 함수 호출만 교체, SHM attach/FIFO open 로직은 그대로 유지 |
| 컴파일러 차이 | HP-UX `-Ae`, Linux `-Wall` 모두 경고 0 목표. `static` 함수로 unused warning 방지 |
| 운영 영향 | 메모리 레이아웃 변경 없음 → 기존/신규 바이너리 혼재 가능 |

---

## 버전 이력

| 버전 | 날짜 | 변경사항 | 작성자 |
|---------|------|---------|--------|
| 0.1 | 2026-02-21 | Initial draft | Claude Code |
