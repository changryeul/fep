# pc_ 파생 시장 분리 파일럿 Implementation Plan

> **For agentic workers:** 실행 시 각 Phase를 순차 진행하고 Phase 경계마다 서버 빌드 + E2E 회귀로 검증한다. 운영 트레이딩 시스템이므로 장 마감 후 단계 배포·롤백 경로 유지.
>
> **Goal:** 파생(IMECO) 시장을 신규 부문 `pc_`로 분리해 A안(letter=시장) 재편의 메커니즘을 실증한다. 가장 고립된 슬라이스(파생 주문송신)부터 시작해 파생 시장 전체로 확장하고, 크로스마켓 계정상태의 `po_` 경계를 확정한다.
>
> **Architecture:** letter=시장(pc=파생, SHM 0x2103) / 계층=배포 / 크로스커팅=공유(po_, 후속). 함수코드 앞자리를 시장에서 해방(2xxx→pc_1xxx). **전 시장 KRX-direct 통일**(2026-08-06): 파생 IMECO 폐기 → pc_는 KRX 코덱(libfepP.a 공유) 사용, pb_(채권) 템플릿과 동형. 상세: `docs/02-design/market-module-reorg.design.md`, 메모리 `market-reorg-direction`.
>
> **Tech Stack:** C89, libfepP.a, proc.ini/dshm.ini/daemon.ini, Make_PC/*.sh, test/e2e 하니스

---

## 0. 현행 파생 footprint (착수 전 확정)

| 프로세스 | 소스 | 성격 | 파일럿 처리 |
|---------|------|------|-----------|
| pa_2101_ts (주문송신) | `pa_2100_ts.c` (전용 IMECO) | 프로토콜 대면 | **Phase 1 → pc_1101_ts** |
| pa_2201_tr (응답/체결수신) | `pa_2200_tr.c` (전용) | 프로토콜 대면 | Phase 2 → pc_1201_tr |
| 파생 IMECO 시세수신 | `pa_2700_tr.c` (전용) | 프로토콜 대면 | Phase 2 → pc_7101_tr |
| pa_2201_mp (분배) | `pa_1200_mp.c` `-DA2201` | 크로스마켓 공유 | **Phase 3 → po_** (미이동) |
| pa_2291_mp (미체결) | `pa_1290_mp.c` `-DA2291` | 크로스마켓 공유 | Phase 3 → po_ |
| pa_2401_mp (체결) | `pa_1400_mp.c` `-DA2401` | 크로스마켓 공유 | Phase 3 → po_ |
| pa_2491/2492_mp (체결원장) | `pa_1490_mp.c` `-DA2491/2492` | 크로스마켓 공유 | Phase 3 → po_ |
| pa_2601_tr/mp (조회) | (확인 필요) | 시장별 조회 | Phase 2 |

**핵심 경계**: 프로토콜 대면부(주문송신/체결수신/시세)가 pc_. 미체결·체결원장·분배는 `[시장][계좌]` SHM(MK_PREMATCH/RISK)에 작동하는 크로스마켓 상태라 pc_가 아니라 **po_(OMS 코어)** 소속 — Phase 3에서 MK_GBN 컴파일상수를 런타임화하며 이동.

**⚠ IMECO 폐기 반영 (2026-08-06)**: 파생을 KRX-direct로 통일하므로, 현 IMECO 소스(`pa_2100_ts.c` 등)를 **그대로 이동하지 않는다**. pc_ 파생 주문송신/체결수신은 **KRX 템플릿(pb_1100_ts/pb_1200_tr)을 기반으로 파생 struct·업무규칙만 얹어 신구축**한다(pb_와 동형). 즉 이 파일럿은 "IMECO 소스 이관"이 아니라 "KRX형 파생 모듈 신설". 기존 IMECO 파생 경로는 전환 완료 후 폐기.

## 함수코드 재인덱스 규약 (2xxx → pc_1xxx)

letter가 파생을 담으므로 앞자리를 순수 기능으로:

| 현행 (PA, 시장=앞자리) | 신규 (pc_, 기능=앞자리) | 기능 |
|----------------------|------------------------|------|
| pa_2101_ts | pc_1101_ts | 주문송신 |
| pa_2201_tr | pc_1201_tr | 응답/체결수신 |
| pa_2601_tr | pc_6101_tr | 조회 |
| 파생시세 | pc_7101_tr / pc_7101_dd | 시세수신/분배 |

프로세스 인스턴스(1/2회선 등)는 argv[0] + proc.ini로 구분(빌드모델 개선은 채권 단계 파일럿이나, pc_ 신설 시 **-D 다중바이너리를 새로 만들지 않는다** — 처음부터 1소스=1바이너리 원칙).

---

## Phase 0 — 헤더 기반 검증 (2026-08-06 완료)

착수 전, 추출 스펙(EXTURE 3.0 v3.24)과 기존 코드 struct 일치를 검증. **핵심 발견: 현/파 주문·체결·헤더 struct가 이미 pa_struct.h에 존재하고 스펙과 정확히 일치**:

| struct | 전문 | 길이 | 검증 |
|--------|------|------|------|
| `KRX_HEADER` | 공통 헤더 | 82B | ✅ (10필드=82) |
| `KRX_MSG_COMMON` | 헤더+공통바디 | 106B | ✅ |
| `KRX_JUMUN_DATA` | TCHODR10001 호가입력(현/파) | 294B | ✅ 44필드 길이순서 0불일치 |
| `KRX_SETTLE_DATA` | TTRTDP21301 체결결과(현/파) | 233B | ✅ 24필드 0불일치 |

- **회귀 가드 신설**: `test/unit/test_krx_struct.c` — sizeof를 스펙 길이에 못박음. 향후 struct 드리프트를 컴파일/런타임에서 차단(S_Fmt 오버플로류 예방).
- **응답 전문 `TTRODP11301`(회원처리호가 정상, 318B, 현/파) 신구축 완료 (2026-08-06)**: 갭이던 24B 부분(레거시 `KRX_JUMUN_R_DATA`=ErrCode+DataSeq)과 별개로, 스펙 48필드 정식 struct `TTRODP11301_DATA`를 **전문별 1개 헤더** `st01/inc/krx_ttrodp11301.h`로 편입(Title_Case_언더스코어 필드명, 규약 첫 적용 사례). 길이 합계 318 = 카탈로그 일치. 회귀 가드 2건 추가(sizeof==318, offsetof(Me_Grp_No)==22) → **서버 167통과/0실패**. 거부(`TTRODP11321`)·자동취소(`TTRODP11303`)는 후속(각각 별도 헤더).
- 결론: 파생 pc_ 주문경로 헤더 완결 — 주문 `KRX_JUMUN_DATA`(294B) + 응답 `TTRODP11301_DATA`(318B) + 체결 `KRX_SETTLE_DATA`(233B) 모두 struct 확보. Task 1.2가 예상보다 단순(신구축이 아니라 기존 struct + pb 이벤트루프 조합).

## Phase 1 — 파일럿: 파생 주문송신 → pc_1101_ts (메커니즘 실증)

가장 고립된 슬라이스로 신규 부문 전 과정(letter·SHM·빌드·설정·E2E)을 검증.

### Task 1.1: 부문 스캐폴딩 (`pc_`/`po_` 등록) — 빌드/디렉토리 완료 (2026-08-06)

**Files:**
- Modify: `st01/env/pkg_env.sh` — `_FEP_SUBDIR="A B W X Z"` → `"A B C O W X Z"` (C=파생, O=OMS코어). ✅ 완료. `subname`(런타임 기동 게이트)은 미변경 → 프로세스 자동기동 없음.
- Create: `st01/make/{PC,PO}/` — `Make_{PC,PO}.mk`+`_all.sh`+`_c.mk`. ✅ 완료. **개선 빌드모델 파일럿 내장**: `src/{PC,PO}/*.c` 1소스=1바이너리(동일 basename), `-DPCxxxx` 다중바이너리 없음, INISAFE는 `NO_INISAFE` 가드, 소스 0개면 no-op.
- Create: `st01/src/{PC,PO}/`(README 규약), `st01/obj/{PC,PO}/`, 런타임 `st02/FIFO/{PC,PO}`·`st02/DAT/{PC,PO}`·`st03/LOG/{PC,PO}`. ✅ 완료.

- [x] **Step 1**: pkg_env.sh SUBDIR에 C·O 추가 → `_PC_*`/`_PO_*` 경로변수·alias(pcsrc/pcobj/pclog…) 자동생성 확인.
- [x] **Step 2**: make/디렉토리 스켈레톤 + 개선 빌드모델. 서버 검증: `mk.sh pc`/`mk.sh po` no-op(소스0), 임시소스 1개 투입 시 `bin/pc_9999_zz` ELF 정상 링크(libfepP.a) 확인 후 정리.
- [x] **Step 3 (비파괴 확인)**: PC/PO는 `_FEP_SUBDIR`에 포함돼 `mk.sh all`/`src`에 편입되나 소스0이라 무영향. `mk.sh <module>` 직접 경로로 온디맨드 빌드 가능.

**SHM 결정(변경)**: pc_ 파생은 **letter별 신규 SHM 키 불필요**. 시세는 기존 `FF_SHM_KEY`(금융파생), 전략/미체결/한도는 이미 전 시장 공유 세그먼트(`STRRG/MK_PM/RISK_SHM_KEY`, base 0x41000000, `[RISK_MK_CNT][acc]` 레이아웃). 따라서 원안의 "0x2103 부문 SHM 신설"은 보류 — 의미별 세그먼트 재사용. daemon.ini `Daemon_C` 블록/`pz_memory_mp c`는 **실제 pc_ 프로세스가 생기는 Task 1.3(proc.ini 배선)과 함께** 처리(런타임 배선을 스캐폴드에서 분리).

**남은 런타임 배선(Task 1.3으로 이관)**: `cfg/daemon.ini` `Daemon_C_*`, `subname`에 `c` 추가, proc.ini PC 블록.

### Task 1.2: KRX형 파생 주문송신 신구축 pc_1100_ts.c (IMECO 아님) — ✅ 완료·빌드검증 (2026-08-06)

**Files:**
- Create: `st01/src/PC/pc_1100_ts.c` — **`pb_1100_ts.c`(KRX 채권 주문송신) 템플릿 기반**, 파생 struct·업무규칙만 차별화. ✅
- 빌드: Task 1.1의 개선모델 `Make_PC_all.sh`(1소스=1바이너리)가 그대로 처리 → **별도 `Make_PC_ts.sh` 불필요**(원안의 `for i in 1101` -D 루프 폐기).

- [x] **Step 1**: `pb_1100_ts.c` → `pc_1100_ts.c` 신구축. 채권 대비 차이: ①전문크기 검출 `TCHODR40001/2/3`(254)·`TCHMOR`(255) → **`TCHODR10001/2/3`=294B(`KRX_JUMUN_DATA`)**, ②주문Q포맷 `KRX_NOTE_ALL_JUMUN_Q_FMT`→`KRX_JUMUN_Q_FMT`, 응답 `KRX_NOTE_JUMUN_R_DATA`→`KRX_JUMUN_R_DATA`, ③`CHK_PROC` `pb_1201_tr`→`pc_1201_tr`, ④`#ifdef Bxxxx` 시장분기 없음(파생 고정), ⑤NO_INISAFE 경로는 `R_Fmt.Data`에서 직접 조립(`KRX_JUMUN_DATA`엔 opaque `.Data` 없음). argv[0]=`pc_1101_ts`에서 Init_Proc가 프로세스ID 파싱.
- [x] **Step 1b**: 짝 프로세스(`pc_1201_tr`, Phase 2 산출물) 미등록 시 FATAL 대신 **self-monitor 폴백**(`Pk=P_K`, WARN) — 점진적 부문 bring-up 수용(오종료 방지).
- [x] **Step 4 (검증)**: 서버 `mk.sh pc` → `bin/pc_1100_ts` 컴파일·링크 **0에러**(libfepP.a 결합, `-DNO_INISAFE`). ELF 확인.

### Task 1.3 / 1.4: E2E 검증 (하니스 확장) — ✅ 완료 (2026-08-06)

**접근(경량 파일럿)**: 새 'c' 부문 데몬을 통째로 세우는 대신 **검증된 PB 하니스('b' 부문)를 재사용**하고 sender 바이너리만 `pc_1100_ts`로 교체. mock_oms가 파생 주문(TCHODR10001 294B) 주입 → 릴레이(`pb_7200_tr`, strlen 가변복사라 294B 무손실 통과) → 큐 → `pc_1100_ts`(argv[0]=`pb_1101_ts`) → mock_krx.

- [x] `mock_oms.c`: `MOCK_OMS_KIND=deriv` → 294B TCHODR10001 파생 주문(`build_deriv_order`, Mkt_Id 없음→Board_Id@24) + 가변길이 프레이밍.
- [x] `run_e2e.sh`: `pc` 모드 — sender `exec -a pb_1101_ts $BIN/pc_1100_ts`, mock_oms 파생 모드. 집계는 `/tmp/mock_krx_<port>.log`도 포함하도록 수정.
- [x] **E2E 통과**: `sh run_e2e.sh pc 5` → mock_krx가 **TCHODR 5건 × 376B(KRX_HEADER 82 + 294)** 수신. `pc 20` → 20/20. 크래시 0. 송신 전문 확인: `KMAPv2.0|0294|TCHODR00000|…TCHODR10001|G1|99999|KR4101K30000|…`.
- [x] **회귀**: 대조군 `run_e2e.sh file 5`(순수 PB 채권) → 5/5 정상.

**🐛 부수 발견·수정(공용 인프라 버그)**: E2E 착수 시 `pz_memory_mp b`가 `Main_Process_Memory()`(pz_memory_proc.c:90)에서 SIGSEGV. **대조군 PB에서도 재현** → pc 무관. 코어분석(-g) 결과 **근본원인 = 이중 트리 `_P_CFG` 불일치**: `_FEP_CFG=getenv("_P_CFG")`가 구트리 `~/fep/st01/cfg`(Daemon_B 미설정)를 가리켜 `Info[1].process_count=0` → `Mem_SHM_Creat`의 `if(!Shmsize)continue`로 `Shm_Mem[1].Daemon` NULL → 라인 90 NULL 역참조. **수정 2건**:
1. `env/pkg_env.sh`: `_FEP_HOME=${_FEP_HOME:-$HOME/fep}` (사전설정 존중, 프로덕션 기본 불변). `test/e2e/run_e2e.sh`: 소싱 전 `export _FEP_HOME=$FEP` → 모든 `_P_*`가 new_fep 일관.
2. `src/PZ/pz_memory_proc.c`: 미설정 데몬(`Shm_Mem[D_K].Daemon==NULL`) 시 segfault 대신 FATAL 종료(형제 루프 line 71 가드와 동형, 원인 `_P_CFG` 로깅). → **pb/pc 전 E2E 언블록.**

**Phase 1 완료 기준 — 충족**: 파생 주문 N건이 client→분배(공유 pb_1301_tr)→pc_1100_ts→거래소(mock)로 크래시 없이 전달(5/5, 20/20). pc_ 부문이 독립 빌드·로그로 KRX-direct 파생 주문 송신 동작 실증.

---

## Phase 2 — pc_ 파생 시장 확장 (프로토콜 대면부 완성)

### Task 2.1: 파생 응답/체결 수신 pc_1200_tr.c — ✅ 신구축·빌드검증 (2026-08-06)
IMECO 폐기 원칙에 따라 `pa_2200_tr.c`(IMECO)가 **아니라 KRX형 `pb_1200_tr.c`(채권 수신) 템플릿**으로 신구축(현/파 공용 전문만 차별화).
- [x] `krx_trcode.h`에 현/파 응답/체결 TR 추가: `TR_ORDER_RESP`(TTRODP11301)/`_REJECT`(11321)/`_CANCEL`(11303)/`_PREFIX`("TTRODP113"), `TR_ORDER_EXECUTION`(TTRTDP21301), `TR_SESSION_TRADE`(수신 세션래퍼 TCHTDP00000).
- [x] `src/PC/pc_1200_tr.c`: `pb_1200_tr.c` 기반, `#if defined(B1201/B1211/B1601)` 분기 제거(응답+체결 고정), 파생 TR 라우팅(체결 TTRTDP21301→Write_Data(2), 회원처리호가 TTRODP113*→Write_Data(1)+15B패딩, IF종료 TCHEDP99000→종료), 개시 SCHOPQ10000/SCHOPR10000, NO_INISAFE. 수신기라 CHK_PROC 의존 없음.
- [x] 서버 `mk.sh pc` → `bin/pc_1200_tr` 컴파일·링크 **0에러**(libfepP.a, `krx_trcode.h` 매크로).
- [x] **수신경로 E2E 통과** (`test/e2e/run_pc_rx_e2e.sh`): mock_krx가 수신측 개시응답 `SCHOPR10000` + 회원처리호가 `TCHTDP00000`+`TTRODP11301`(400B=82+318) push → pc_1200_tr(=argv[0] pb_1201_tr)가 수신·ME그룹seq 검증(meg_no1/seq1) 통과 → `Write_Data(1)` → **OFN_1(pb_1402_ts) 큐 1레코드(485B) 기록**. 크래시 0.
  - mock_krx 확장: `mock/mock_krx_server.c`에 SCHOPQ10000→SCHOPR10000+push 분기, `lib/krx_protocol.c`에 `krx_build_data_push()`(MsgType/MsgSeqNum/ME그룹seq 지정) 추가.
  - 체결(TTRTDP21301, Write_Data(2)→OFN_2)은 Proc_6에 OFN_2 미정의 → 응답(TTRODP11301, Write_Data(1)→OFN_1)으로 수신경로 실증. 체결 OFN 배선은 정식 pc_ 데몬 config에서.

### Task 2.2: 파생 시세 수신 pc_7100_ur.c — ✅ 신구축·빌드검증 (2026-08-06)
**네이밍 정정**: IMECO(TCP) 폐기 → KRX 파생시세는 **UDP 멀티캐스트**(udpip.ini 선물통합 233.38.231.91:10301 등) → `pc_7100_tr`(IMECO-TCP 시절 명)가 아니라 **`pc_7100_ur`**(UDP 수신)가 정확. 채권 시세수신 `pb_7100_ur.c` 템플릿 기반(주 역할=UDP 수신→TCP 중계, 복잡한 SHM 인덱싱 없음).
- [x] `src/PC/pc_7100_ur.c`: `pb_7100_ur.c`에서 파생 차별화 — TR 분류 `A3/G7/B6/M4`(채권) → **`A3/G7/B6/R1`**(파생: A301F 체결·G701F 체결G7·B601F 우선호가·R101F 장운영TS, 접미사 F). DATA_SIZE 700(파생 최대 G701F 431B), HB 기본포트 50020(채권/PA-HA와 미충돌). **개선 빌드모델**: `#if defined B7102/B7103` 다중바이너리·HB offset 제거 → 1소스=1바이너리, HB포트 `_HA_HB_PORT` env. HA(heartbeat/failover) 로직 승계(dev=STANDALONE).
- [x] 서버 `mk.sh pc` → `bin/pc_7100_ur` 컴파일·링크 **0에러**(경고 1: `Connect2` implicit decl — pb_7100_ur.c에서 승계, libfepP.a 존재해 링크 정상).

**남은 시세 E2E**: 주문경로 mock_krx(TCP)와 달리 **UDP 멀티캐스트 송신 mock + TCP 싱크** 필요. EC2 멀티캐스트/NIC(ens6f3) 바인딩 제약 확인 필요. 별도 하니스 유형으로 분리.

- [ ] pa_2601 조회 → pc_6101 (소스 확인 후)
- [ ] 각 이동마다 proc.ini 재배선 + E2E. 이 단계까지 **분배/미체결/체결원장은 PA 공유 유지**(seam은 config로 연결).

---

## Phase 3 — po_ OMS 코어 추출 + MK_GBN 런타임화 (경계 확정)

크로스마켓 계정상태를 po_로 이동하며 `-D` 시장분기 제거.

### Task 3.1: 응답분배 po_1200_mp — ✅ 신구축·빌드검증 (2026-08-06)
`pa_1200_mp.c`(응답분배) 이관. **MK_GBN 런타임화 첫 실증**:
- [x] `src/PO/po_1200_mp.c`: `#if defined A1201/A2201` 시장분기 제거. IMECO 폐기로 전 시장 KRX form `[4 ErrCode][11 seq][전문]`(TrCode@26) 통일 → `imeco_gbn=0` 고정, TR코드 런타임 검출로 채권(TCHODR4000/TCHMOR4000/TCHKOR1000/TTRODP4130/TTRMOP4130/TTRKOP1130) + **현·파(TTRODP113* → `TTRODP11301_DATA.Member_Use_Area`)** 를 **한 바이너리로 분배**. DATA_SIZE=400 고정.
- [x] 서버 `mk.sh po` → `bin/po_1200_mp` 컴파일·링크 **0에러**(`-DSAM_USE` 불요). → `-DA1201/A2201` 2바이너리 → 1바이너리 통합 실증.
- [x] **'o' 런타임 부문화 E2E 통과** (`test/e2e/run_po_e2e.sh`): daemon.ini `Daemon_O`(예비→OMS_CORE, Date_Flag=1) + proc.ini/file.ini `PO_CONF`(P<letter>_CONF→데몬 인덱스) 추가 → `pz_memory_mp o`가 'o' 부문 SHM(0x42150000)·FIFO 6개 생성 → `po_1200_mp`(exec -a)가 SHM attach·config init·stat 파일·메인루프 진입까지 **정상 기동(RUNNING, crash 0)**. **config만으로 신규 부문 독립 기동 실증** → pc_ 독립기동 de-risk.
  - 발견/수정: ①file.ini 각 항목 `File_End` 필수(cnt 증가), ②Date_Flag=9(연속)는 stat을 날짜디렉토리에 두나 Stat_Save는 `00000000` 고정 → 트레이딩 데몬은 Date_Flag=1/2, ③config 주석에 괄호 `()` 금지(파서 FATAL), ④하니스 `pkill -f "po_1200_mp"`는 패턴 담은 호출 셸 자기종료 유발 → `pkill -x`.
- [x] **데이터흐름 E2E 통과** (`run_po_e2e.sh`에 `po_inject`(=po_1209_mp) 추가): 동일 'o' 부문 생산자가 현·파 회원처리호가(TTRODP11301, 회원사용영역 매체=C·전략=0000)를 po 입력큐에 F_W → **po_1200_mp가 read(`RD [po_1200_mp:1]`)·TTRODP113* 검출·매체C→자사 판정·Client 큐(po_out_ts)로 분배(485B=1레코드)**. MK_GBN 통합 분배 로직 실증.
- **아키텍처 발견**: pc_(b부문)→po_(o부문) **크로스부문 파일큐 체인은 불가** — 큐 카운터(w_cnt/r_cnt)가 부문별 SHM에 있어 미동기. seam은 **공유 DSHM 또는 동일부문 co-location** 필요. (그래서 동일 'o' 부문 injector로 실증.)

### Seam 확정 (2026-08-06): STATE-driven
부문 간 큐(파일/DSHM) 불가(부문별 키) → **STATE-driven 채택**: 수신이 전역 SHM(MK_PM/RISK/STRRG)에 직접 기록, po_가 공유상태 소비. 미체결/한도/전략은 이미 전역 SHM이라 **po_ 이관 시 seam 자동 해결**(동일 전역키 attach). 상세 `docs/02-design/market-module-reorg.design.md §5.1`.
**선행 필수**: 수신의 전역SHM write 경로 교체는 미체결/체결 정합성 핵심 → **replay/멱등 검증 하니스 먼저 구축** 후 착수.

#### replay/멱등 하니스 — 기반 도구 완료 (2026-08-06)
- **`utl/shm_snap.c`(→ `bin/shm_snap`)**: 미체결 전역 SHM(`MK_PREMATCH.F_MiChe[시장][계좌]`, MK_PM_SHM_KEY=0x42000013 TEST) 읽기전용 attach → 점유 미체결(잔량≠0/주문번호 존재)을 canonical 텍스트로 덤프. `cmp A B`로 두 스냅샷 diff. 서버 검증: 실 MK_PM attach·덤프(0건)·cmp IDENTICAL(exit0) OK. libfepP 불요(raw shmget/shmat).
- **멱등 검증 절차**(도구 준비됨, 실적용은 po_1290 이관 시): `shm_snap dump A` → 체결/응답 N건 적용 → `dump B` → **동일 N건 재적용(replay=재기동 복구 or 중복수신)** → `dump C` → `cmp B C` **IDENTICAL이어야 멱등**(더블카운트 없음). MICHE 복구는 체결파일 replay 설계([[miche-query-level-design]])이므로 이 절차로 replay 안전성 직접 검증.
- 다음: 한도(RISK) 스냅샷 모드 추가 + po_1290_mp(미체결) 이관 시 실제 체결 적용/재적용으로 멱등 실증.

### Task 3.2: 미체결 po_1290_mp — 🔧 채권경로 무변경 포팅·빌드검증 (2026-08-06)
가장 correctness-sensitive한 미체결(pa_1290_mp.c, 974줄, MK_PREMATCH 갱신+RISK+miche_idx). 마라톤 세션 말미 급쓰기 리스크 회피 위해 **가장 안전한 슬라이스**로 착수:
- [x] `src/PO/po_1290_mp.c`: `pa_1290_mp.c` **채권(A1291, MK_GBN=0) 경로 로직 무변경 복제** + 인소스 `#define A1291`(→ `-DA1291` 제거, 단일 바이너리) + 함수명 PO_1290_MP. 이번 세션 버그픽스(pa_1490 `if(x=0)`류·miche_idx) 포함된 현행 소스 기반이라 **프로덕션 pa_1291_mp의 검증된 정합성 계승**. 서버 `mk.sh po` 0에러.
- [x] STATE-driven seam 정합: 전역 미체결 SHM(`Shm_Mk_PreMatch`, MK_PM_SHM_KEY)을 그대로 attach → 'o' 부문에서도 동일 세그먼트 공유(부문화 메커니즘은 po_1200_mp로 검증됨).
- [x] **실기동 검증(2026-08-06, `run_po_e2e.sh` Proc_3)**: po_1290_mp가 'o' 부문에서 기동(SubDaemon SHM attach·config init·메인루프·`RD` 수신) — crash 0. (단, 전역 MK_PM attach는 아래 blocker로 미완 — 초기 기재 정정.)
- [x] **injector·주입·read 체인 검증**: `po_inject`에 `PO_INJECT_KIND=bond_miche`(TTRODP41301 채권 회원처리호가) 추가, po_1219_mp가 po_1290_mp 입력에 F_W → po_1290_mp `RD [po_1290_mp:1]` 수신 확인. injector 레이아웃 확정(Data[15+11]=TTRODP41301, [15+39]OrderNo, [15+59]ItemCode, [15+71]TradeFlag, [15+72]='1', [15+85]OrderQuantity, MembershipItem@[15+191]: 매체[+30]='C' 시장[+35]='0' item_seq[+37] acc_seq[+42]).
- **🐛 BLOCKER 해소(운영자 결정: 게이트 제거)**: po_1290_mp가 `Make_MiChe`에서 SIGSEGV(`Shm_Mk_PreMatch` NULL)했던 원인 = 프로세스 MK_PM attach 경로 `Sise_SHM()`이 `init_proc.c:77 if(DAEMON(D_K).sisetr_count>0)`로 게이트되는데 sisetr_count가 구조적 항상 0(sise config 파싱 `/* 202201 */` 주석 + `SiseTr_Config_Read` 미정의). → **`init_proc.c`의 게이트 제거, `Sise_SHM()` 무조건 호출**(의미별 SHM은 pz가 전역 고정키로 선행 생성 → attach 안전). libfepP 재빌드 + 전 바이너리 재링크.
- [x] **functional E2E 통과**: 채권 회원처리호가 TTRODP41301 주입 → po_1290_mp가 전역 MK_PM에 **미체결 1건 정확 등록**: `MICHE mk=0 acc=1 idx=0 ord=0000000001 item=KR6000000001 tf=2 jan=100 mkgbn=0`(주입값 일치). shm_snap 포착.
- [x] **replay-safety(멱등) E2E 통과**: po_1290_mp kill→restart → read커서 유지로 재처리 없음 → `shm_snap cmp` **IDENTICAL**(이중등록 없음).
- [x] **회귀 검증**: init_proc(공유 libfepP) 변경 후 pc 재링크→송신 E2E 5/5, pb 재링크→file E2E 5/5. 무해 확인.
- **남은 것**: ②파생 KRX 미체결 경로(A2291 IMECO 제외; pc_1200_tr TTRODP11301 응답 STATE-driven 소비 신규). ③체결원장 pa_1490·체결 pa_1400·한도 pa_9001 → po_ (동일 패턴, shm_snap 멱등 검증 병행).

### Task 3.3: 체결원장 po_1490_mp — ✅ 채권경로 포팅 + functional E2E 통과 (2026-08-06)
- [x] `src/PO/po_1490_mp.c`: `pa_1490_mp.c`(체결원장, 627줄) **채권(A1491/MK_GBN=0, 1st채널) 로직 무변경 복제** + 인소스 `#define A1491`(-D 제거) + PO_1490_MP 개명. 체결(TTRTDP42301) 수신→MICHE OrderNo 검색→`Jan_Cnt -= 체결수량`. 서버 `mk.sh po` 0에러.
- [x] `po_inject`에 `PO_INJECT_KIND=bond_settle`(TTRTDP42301, OrderNo@39·Trading_Volumn@106·MembershipItem@232) 추가.
- [x] **functional E2E 통과**(`run_po_e2e.sh` Proc_5/6): 미체결 등록(jan=100) → 채권 체결(수량100) 주입 → po_1490_mp가 OrderNo 매칭·**Jan_Cnt 100→0 감소**(shm_snap: `jan=0 ocnt=00000100`), MeChe_Cnt--. → **미체결 생명주기(등록→체결감소)가 po_ 전역 MK_PM에서 정확 동작**. po_1290_mp(등록)·po_1490_mp(감소) 두 프로세스가 동일 전역 세그먼트 공유(STATE-driven seam) 실증.
- 파생 체결(A2491 IMECO)은 제외 — KRX TTRTDP21301 별도.

### Task 3.4: 한도 po_9001_mp — ✅ 포팅 + start/attach E2E 통과 (2026-08-07)
- [x] `src/PO/po_9000_mp.c`: `pa_9000_mp.c`(전략기동/종료 + 계좌한도/RISK 초기화, 495줄) **로직 무변경 복제** + PO_9000_MP 개명. 크로스마켓 본질이라 `#if defined` 분기 없음. `argc==1`이면 `Init_Parameters`가 강제 RISK init(Shm_Risk에 S_Sise·Order_No 범위·ProFit·O_M_Fund 기록). 서버 `mk.sh po` 0에러.
- [x] **start/attach E2E 통과**(`run_po_e2e.sh` Proc_7): `exec -a po_9001_mp $BIN/po_9000_mp`로 기동 → 전역 RISK 세그먼트 전체 attach(로그: `SHM[0x41000001~03/12/13/14/30]`) + `Init_Parameters` 크래시 0(`Init_P OK`) → Poll 대기 **RUNNING**. → 한도(RISK)도 미체결/체결원장과 동일한 전역 SHM STATE-driven seam으로 po_ 코어에 안착. **po_ 코어 상태관리 3종(미체결+체결원장+한도) 완결.**
- **하니스 주의**: 소스명(po_9000_mp)≠argv0(po_9001_mp) → OS comm=po_9000_mp이므로 `pgrep/pkill -x`는 **comm(po_9000_mp)** 기준으로 체크(FEP 내부 등록은 argv0=po_9001_mp).
- **남은 것**: ①`init_proc.c` sise-attach 게이트 제거로 전역 미체결/한도/전략 SHM 무조건 attach(선결 버그 수정, pc 5/5·pb 5/5 회귀 통과). ②파생 KRX 미체결/체결 경로(A2xxx IMECO 제외; pc_1200_tr TTRODP11301/TTRTDP21301 STATE-driven 소비 신규). ③체결 pa_1400_mp → po_ (po_1490과 구분 시). ④pc_ 정식 'c' 데몬 독립기동.

### Task 3.5: 부문간 SEAM 스테이징 큐 — ✅ 전역 SHM 링버퍼 신구축 + TDD + E2E (2026-08-07)
- [x] **결정**: 크로스부문(pc_/pb_ 수신 → po_ 소비) 전달을 **전역 스테이징 SHM 링버퍼**로 구현(운영자 확정, design §5.1.1). po_ 미체결/체결 등록 로직 **무변경 보전**, 전달경로만 교체.
- [x] `inc/seam_queue.h` + `sub/seam_ring.c`(순수 링 코어) + `sub/seam_queue.c`(SHM/세마포어 래퍼). 세그먼트 `SEAM_Q_SHM_KEY=0x41000015`(+TEST), 큐2(응답/체결)×링4096×슬롯480B(≥FILE_BUFF_FORMAT 471). 리더별 `r_seq`(multi-reader: 미체결/분배/원장), replay-safe(커서 SHM 상주), 다중writer 세마포어 보호, 오버플로 `dropped` 회계(무음유실 금지).
- [x] **TDD**: `test/unit/test_seam_queue.c`(Unity 6케이스: write/read FIFO·replay·multi-reader·maxrec배치·오버플로회계) — 로컬+서버 6/6 PASS, 전체 스위트 173 PASS 무회귀.
- [x] **배선**: `pc_1200_tr`(수신) `Write_Data` → `SEAM_W`(응답→RESP/체결→EXEC). `po_1290_mp`(미체결)·`po_1490_mp`(체결원장) 읽기 `F_R`→`SEAM_R`(슬롯 480B→R_Fmt 471B 복사로 파싱 바이트 동일, `Add_Count` 제거, Poll 200ms 재스캔). `po_inject`(pc_ 대역) → `SEAM_W`. 서버 `mk.sh sub/po/pc` 0에러.
- [x] **E2E 통과**(`run_po_e2e.sh`): 채권 회원처리호가 주입→SEAM_Q_RESP→po_1290_mp 등록(jan=100) → po_1290_mp kill/restart **replay멱등 IDENTICAL**(SEAM r_seq 영속) → 채권 체결 주입→SEAM_Q_EXEC→po_1490_mp 감소(**jan 100→0**). **크로스부문 seam이 전역 SHM 링버퍼로 실증.**
- **✅ 생산자측 E2E 통과(2026-08-07)**: `run_pc_rx_e2e.sh` — mock_krx가 실제 파생 회원처리호가 `TTRODP11301`(TCHTDP00000 래퍼) push → `pc_1200_tr`(=pb_1201_tr) TCP 수신 → `SEAM_W(SEAM_Q_RESP)`. `utl/seam_peek`(SEAM read-only attach 도구)로 착지 확인: **SEAM_Q_RESP w_seq=1, 레코드 Data에 "TTRODP11301" 확인**, FEP 로그 `SEAM write[q=0:1]`. → **KRX→pc_→SEAM 생산자 반쪽 실증**(소비자 반쪽 SEAM→po_→MK_PM은 위 E2E). 두 반쪽 = 전 구간(단, 아래 파생 등록 갭 참조).
### Task 3.6: 파생 미체결 등록 MK_GBN 런타임화 — ✅ Make_MiChe_Deriv 신설 + E2E (2026-08-07)
- [x] **설계**: 채권 `KRX_NOTE_SETTLE_RESP_DATA`와 파생 `TTRODP11301_DATA`는 필드명·레이아웃이 완전히 달라 C 타입상 한 `dat`로 런타임 선택 불가. **거대 공용본문 정규화 대신 파생 전용 핸들러 `Make_MiChe_Deriv` 신설 + `Analyze_Data`에서 TR코드 디스패치**(TTRODP113→파생, 그 외→채권). **채권 `Make_MiChe` 한 줄도 안 건드림(무회귀)** — 구조체가 실제 다르니 시장별 핸들러가 더 명확. 거부(TTRODP11321)·자동취소(TTRODP11303)는 별도 전문/헤더 미존재라 범위 제외.
- [x] `po_1290_mp.c`: `Make_MiChe_Deriv`(TTRODP11301 신규/정정/취소 미체결 등록 + 한도 ProFit 가감, 채권 접수확인 로직에 충실). `krx_ttrodp11301.h` 포함. `Analyze_Data` 디스패치 추가. `po_inject.c` `deriv_miche` 모드(TTRODP11301_DATA 오프셋 주입) 추가. 서버 `mk.sh po` 0에러.
- [x] **E2E 통과**(`run_po_e2e.sh`): 파생 회원처리호가 TTRODP11301 주입→po_1290_mp `Make_MiChe_Deriv`→**MK_PM에 mk=1 등록(jan=100, item=KR4101SC0009)**. 동시에 채권 3종(functional/replay멱등 IDENTICAL/체결감소 mk=0) 여전히 통과 → **한 바이너리가 채권 mk=0 + 파생 mk=1 미체결 등록**(MK_GBN 런타임화 실증).
- **발견/수정**: 주석 내 `4130*/TTRMOP`의 `*/`가 주석 조기종료(빌드 깨짐) → 문구 수정. injector 수동오프셋 오타(Order_Quantity[10]=82~92 이후 +1 밀림) → 프로덕션 Make_MiChe_Deriv는 struct 멤버접근이라 정확, 테스트 injector만 수정.

### Task 3.7: 파생 체결 MK_GBN 런타임화 — ✅ Analyze_Che_Deriv/Che_Rtn_Deriv + E2E (2026-08-07)
- [x] `inc/krx_ttrtdp21301.h` 신구축(현·파 회원체결결과, DATA 233B, KRX 스펙 기반·규약 준수). 회귀가드 `test_krx_struct.c`: sizeof==233 + offsetof(Member_Use_Area)==172/Trading_Volumn==90/Order_Identification==36 (서버 9 tests PASS).
- [x] `po_1490_mp.c`: `Analyze_Che_Deriv`(TTRTDP21301 미체결 OrderNo 검색→잔량 감소, 도치 재시도) + `Che_Rtn_Deriv`(손익/잔고/미체결금액, 비스프레드). `Analyze_Data`에서 TR코드 디스패치(TTRTDP21301→파생). **채권 Analyze_Data/Che_Rtn 무변경(무회귀)**. 채권 Che_Rtn의 `(cast)&p_buf` 버그는 답습 안 함(`(cast)p_buf`). 서버 `mk.sh po` 0에러.
- [x] `po_inject.c` `deriv_settle` 모드(TTRTDP21301_DATA 오프셋). E2E 배선.
- [x] **E2E 통과**(`run_po_e2e.sh`): 파생 체결 TTRTDP21301 주입 → `Analyze_Che_Deriv` → **mk=1 잔량 100→0**. 동시에 채권 3종 + 파생 미체결 등록 통과 → **한 po_1290/1490 바이너리 쌍이 채권(mk=0)+파생(mk=1) 미체결 등록·체결감소 모두 처리 = 파생 등록 경로 미체결+체결 완결**.

### Task 3.8: 단일 라이브 파이프라인 통합 E2E — ✅ KRX→pc_→SEAM→po_→MK_PM (2026-08-07)
- [x] 그동안 반쪽씩(생산자 run_pc_rx / 소비자 run_po) 검증한 것을 하나의 살아있는 파이프라인으로 봉합. `run_pc_po_e2e.sh` 신설 — 'b'(pc_ 수신)+'o'(po_ 코어) 동시 기동, 전역 SEAM 큐 공유.
- [x] `mock_krx_server` 확장: 등록 가능한 완전한 TTRODP11301(294B) + TTRTDP21301(209B) payload push(`build_ttrodp11301/ttrtdp21301_payload`, payload 오프셋 = po_ struct 오프셋−24). SCHOPQ10000 핸들러가 회원처리호가+체결 순차 push.
- [x] **E2E 통과**: mock push TTRODP11301/TTRTDP21301 → pc_1200_tr(=pb_1201_tr) TCP 수신 → SEAM_W(RESP/EXEC) → po_1290_mp 등록(mk=1 jan=100) + po_1490_mp 감소(**jan 100→0**). seam_peek: RESP/EXEC w_seq=1 r_seq=[1]. **전 구간 KRX→pc_→SEAM→po_→MK_PM 라이브 실증.**
- **발견/수정**: ①pc_ KRX 인터페이스 seq(if_seq/if_meg_seq)는 `pb_1201_tr_stat` DAT 파일에 영속 → 재실행 시 INT_SEQ 누적으로 LINK/ME그룹 seq 불일치. 하니스가 stat 제거로 fresh 세션 시작. ②`exec -a pb_1201_tr $BIN/pc_1200_tr` → comm=pc_1200_tr(≠argv0) → cleanup `pkill -x pc_1200_tr` 필요(잔존 프로세스 재실행 차단). mock seq: fresh 세션 LINK=0, DATA Header 1/2, ME그룹 1/2.

### Task 3.9: 파생 거부(11321)·자동취소(11303) 처리 — ✅ Make_MiChe_Deriv 확장 + E2E (2026-08-07)
- [x] `docs/reference/krx-exture3` diff로 **11301/11321/11303 레이아웃 완전 동일** 확인(채권 41301/2/3이 한 struct 공유한 것과 동형) → `TTRODP11301_DATA` 재사용(주석 명시). 별도 헤더는 동일 레이아웃이라 생략(채권 선례; 필요 시 trivial 분리).
- [x] `Make_MiChe_Deriv` 가드를 11301/11321/11303 수용으로 확장. 공통 추출(mk/item/acc) 후 **거부/자동취소는 한도(ProFit) 되돌림 + 자동취소는 미체결 슬롯 잔량 감소** 조기 종료(채권 거부/자취 로직 이식, 매체체크 앞). 정상(11301)은 기존 등록. 디스패치는 `TTRODP113` 접두사라 3변형 자동 라우팅.
- [x] `po_inject.c` `deriv_autocxl` 모드(TTRODP11303, Real_Modify_Or_Cancel_Order_Quantity@271=100).
- [x] **E2E 통과**: 재등록(deriv_miche, 잔량 감소 슬롯 재사용 jan=100) → 자동취소(deriv_autocxl) → **jan 100→0**. 채권 3종 + 파생 미체결/체결 동시 통과(회귀 없음). 거부(11321)는 MK_PM 비가시(한도-only)라 빌드검증 + autocxl 한도차감과 동일 코드경로로 커버.

### Task 3.10: po_1200_mp 분배 SEAM 전환 — ✅ 응답 큐 multi-reader 완성 (2026-08-07)
- [x] `po_1200_mp.c` 읽기 `F_R(PS_R_1)` → `SEAM_R(SEAM_Q_RESP, SEAM_R_DIST)`(슬롯 480B→R_Fmt 복사, Add_Count 제거, Poll 200ms). 미체결(po_1290, SEAM_R_MICHE)와 **동일 응답 큐를 독립 커서로 소비**하는 multi-reader 완성.
- [x] **E2E 통과**: po_1200_mp alive + `SEAM_R_DIST` 5건 소비(bond/deriv 회원처리호가). 미체결(po_1290)·분배(po_1200)가 같은 SEAM_Q_RESP를 각자 커서로 독립 소비 실증. 채권 3종 + 파생 미체결/체결/자동취소 동시 통과(무회귀).
- → **SEAM 응답 큐가 설계대로 multi-reader**(미체결+분배) 완전 동작. 부문간 seam이 응답 전 소비자에 대해 완결.

- **남은 것**:
  - ①**파생 스프레드 체결(근/원월물) — 보류(딜러 규약 확인 선행, 2026-08-07)**: 미체결 잔량 감소는 이미 동작(Analyze_Che_Deriv 공통 경로, 스프레드 무관). 남은 건 근/원월물 P&L(Shm_Risk.ProFit)뿐. **레거시 참조 버그 발견**: 채권/IMECO `Che_Rtn` 스프레드 경로가 양 레그 모두 원월물 seq(+16) 사용(Analyze_Data가 item_seq=+16 전달 + 내부 item_seqn=+16), 근월물 seq(+4)는 P&L 미사용 → 근월물 가격(Nearby)이 원월물 종목에 적용되는 불일치. IMECO 전용(`#if A2491`)이라 실행된 적 없음. **올바른 참조가 없어 포팅 불가**. 딜러 확인 필요: (a)근/원월물 종목 seq 오프셋(+4/+16 배정), (b)매수/매도 스프레드의 근/원월물 부호, (c)증거금 산정. + ProFit 스냅샷 도구 선행(shm_snap은 MK_PM만이라 P&L 검증 불가).
  - ②pc_ 독립기동('c' 데몬 정식 config). ③doorbell FIFO 저지연. ④at-most-once→peek/commit.

### Task 3.11: 채권 Che_Rtn `&p_buf` 버그 수정 + ProFit 스냅샷 도구 — ✅ (2026-08-07)
- [x] `po_1490_mp.c` `Che_Rtn`: `(KRX_NOTE_SETTLE_DATA *)&p_buf`(char**=파라미터 주소, 스택 garbage 읽기) → `(cast)p_buf` 정정. 기존엔 손익/잔고(ProFit) 계산이 garbage를 읽어 프로덕션은 item_code 불일치 early-return으로 **채권 체결 잔고/손익 accounting이 사실상 dead**였음. ※운영 롤아웃 전 검토 필요(활성화되는 거동).
- [x] `utl/shm_snap.c` `risk` 서브커맨드 신설: RISK(0x41000014) attach → ProFit[mk][item][acc] getcnt/avg/su_miche/do_miche 덤프(점유분). **스프레드 P&L 검증에도 재사용 가능**.
- [x] **E2E 검증**: 채권 체결 후 `shm_snap risk` → **ProFit mk=0 item=1 acc=1 getcnt=100**(수정 전 dead였던 잔고 accounting 활성화). 파생도 getcnt=100 avg=10000(Che_Rtn_Deriv 정상). `su_miche` 음수는 테스트가 주문송신 한도 pre-alloc 생략한 아티팩트(운영은 +/− 상쇄). 전 항목 무회귀.

### Task 3.12: SEAM 컴파일 가드 + 전체 회귀 스윕 + peek/commit 인프라 (2026-08-07)
- [x] **컴파일타임 불변식**(`sub/seam_queue.c`): `SEAM_Q_RECSZ >= sizeof(FILE_BUFF_FORMAT)`(DATA_SIZE=400) 정적 assert(음수 배열 트릭). 480 vs 471 부류 버그 빌드차단. 통과 확인.
- [x] **전체 회귀 스윕 GREEN**: 유닛 175 tests 0 fail. E2E — run_po_e2e(미체결/replay/체결원장/파생 미체결·체결·자동취소/분배SEAM/Che_Rtn잔고/한도 전부 PASS), run_pc_rx_e2e(pc→SEAM), run_pc_po_e2e(전 구간). Task 3.4~3.11 무결성 확인. 부수: run_pc_rx cleanup `pkill -f`→`-x`(comm) 자기종료 위험 제거.
- [x] **peek/commit 인프라**(at-least-once 기반): `seam_ring_peek/commit`(r_seq 불변 조회 + 별도 커밋) + `SEAM_Peek/SEAM_Commit`(락 래퍼). 유닛 9/9(peek 불전진·commit 전진·미커밋 재조회). **소비자 미배선**(거동 무변경).
- **⚠ at-least-once 배선 전제 = 멱등 계층(발견)**: peek/commit만으로는 불충분 — 미체결 정정/취소/자동취소·체결감소·분배가 모두 **비멱등 mutation**이라 크래시 재처리 시 이중차감/중복(손실보다 나쁨). 안전 배선엔 **처리-seq 추적 멱등**(register OrderNo dedup + settle/cancel 처리seq 스킵)이 선행 필수 = 별도 correctness 서브시스템. 현 at-most-once는 중복무위험(미세창 유실만)이라 그때까지 유지.

**남은 것(우선순위)**: ①at-least-once 멱등 계층(처리-seq 추적) → peek/commit 배선. ②pc_ 독립기동('c' 데몬 deep config). ③스프레드 P&L(딜러 규약, ProFit 스냅샷 검증). ④doorbell FIFO 저지연.

### Task 3.4~ (예정, 트레이딩 정합성 핵심 — 신중)
- [ ] pa_1400_mp(체결)·파생 KRX 경로(A2xxx 대체) → po_. **MK_GBN 컴파일상수 → 런타임**(SHM은 이미 `[RISK_MK_CNT][acc]` → 한 프로세스가 채권+파생 처리). pa_1291/2291 등 `-D` 쌍 통합.
- [ ] 자동전략(5xxx)·접속서버(8xxx)·미니원장 → po_
- [ ] `po_` 런타임 부문화(daemon.ini Daemon_O + proc.ini PO + subname o) + 분배→pc_/pb_ 큐 seam 재정렬
- [ ] SHM 레이아웃 변경 → 장 마감(미체결 0) 배포

**주의**: Task 3.2~는 미체결·체결·한도 = 트레이딩 정합성 핵심. 이번 세션 버그들(RD_CNT 언더플로·MICHE `=` 오타·데드코드)이 이 코드에 있으므로, 이동 전 수정 반영 확인 + **replay/멱등 검증 필수**. 별도 focused 착수 권장.

---

## Phase 4 — 채권 pb_ 정리 + 빌드모델 개선 파일럿 (별도 plan)

현 1xxx(채권)를 pb_ 규약으로 정렬하며 `-D` 다중바이너리 제거·DATA_SIZE 런타임화(file.ini/dshm.ini record_size)를 처음 전면 적용. 별도 `pb-cleanup.plan.md`로 분리.

---

## 리스크 · 롤백

- **운영 트레이딩**: 전 Phase 장 마감 후 배포. proc.ini 부문 Status로 pc_ 부문만 선택 기동/중지 → 문제 시 파생을 기존 PA 경로로 롤백(설정 원복).
- **SHM 키 신설(0x2103 pc, po)**: 세그먼트 재생성 필요 → 미체결 0 상태 배포.
- **~~IMECO 프로토콜~~**: 폐기 확정 — 파생도 KRX-direct라 기존 KRX mock 사용, 별도 확장 불필요. 대신 **KRX 파생 전문 규격**(주문/체결 struct) 확정이 선행 필요.
- **seam(공유 생산자 ↔ pc_ 소비자)**: Phase 1~2 동안 분배가 PA에 남아 큐로 연결. 이름/카운터 정합을 proc.ini에서 정확히.
- **함수코드 재인덱스(2xxx→pc_1xxx)**: 운영 모니터링/스크립트가 프로세스 ID를 참조 → 재편 시 운영 절차서(매뉴얼) 동시 갱신.

## 착수 전 확인 (실행 세션 시작 시)

1. ✅ **KRX 파생 전문 규격 확보** (2026-08-06): `KRX EXTURE 3.0 전문 v3.24`(repo 루트 .xlsb, 메모리 `krx-exture3-spec`). 핵심: **파생 주문 = `TCHODR10001/2/3`, 294바이트, 현물과 공용**. 채권(현 pb_)은 `TCHODR4xxxx` 254B. 294B = 채권 254B + 차세대필드(ME그룹번호·최소체결수량·알고리즘전략·거래자ID·호가그룹·자전거래). → pc_ 주문송신은 `pb_1100_ts` 확장으로 구현.
   - **Task 1.2 갱신**: pc_1100_ts.c는 `TCHODR10001`(294B) 핸들러. 종목코드=ISIN 12, 보드ID 별첨 매핑 참조.
2. pa_2601(조회) 소스·역할 확정
3. 파생 주문 입력 큐의 현 생산자 정확 식별 (분배 pa_2201_mp OFN 경로)
4. **전문 사전 산출물 검토**: 실행 착수 시 EXTURE3.0 스펙에서 파생 주문/체결/시세 전문 필드 레이아웃을 repo 내 durable 형식(헤더/CSV)으로 추출해 두면 `.xlsb` 도구 의존 없이 참조 가능 (권장 선행).
