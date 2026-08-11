# 전략 소스 편입 계획 (채권 LP · 차익거래) — Phase 4

> 시장 재편 A안 §5.2/§5.3 후속. Model B(단일 po_ 슬롯 풀) + SEAM 주문큐 위에 실제 전략을 얹는다.

## 조사 결과 (2026-08-10, fep/채권lp · fep/차익거래)

### 채권 LP (`채권lp/pa_5050_mp.c` + `mat_lpbnd/lib/`)
- **BLP 로직**(`lib/blp/blp*.c`, ~7,900줄)은 **이미 FEP-aware** — `blp.h`가 FEP `shm_memory.h`·`buf_struct.h`를 include. 시장 판단은 내부(`Order_No[0]`=채권).
- **MAT 의존은 지원계층으로 국한**: `mem.h`(메모리풀)·`sem.h`(세마포어)·`mcp.h`(메시지카피). 구현은 `libmats.a`.
- ⚠ `libmats.a`는 **macOS(arm64) 프리빌트 → Linux 재빌드 필요**. `make.lin`이 `LIB_HOME=/fsfxwin/fep/mat` 하드코딩(경로 조정). gcc -m64.
- MAT 전체(`cfg/cmd/dll/etc/log/map/tcp/udp/mcp/mem/sem`)는 프레임워크지만 **blp가 실제 쓰는 건 mem/sem/mcp 서브셋**.
- 주문 emit: `Od_Write_Data`→`Dshm_Add_Count(TS_W1_1)` (D_K 종속) → **SEAM_ORD_W로 교체 대상**(§5.3).
- 인코딩 EUC-KR(주석 mojibake), `DATA_SIZE=2048`.

### 차익거래 (`차익거래/pa_7070_mp.c` + `yarb/`) — assess 정밀화 (2026-08-10)
- **엔진 = libyarb.a**(`yarb/l_*.c` 8파일: l_ord/l_prc/l_set/l_thr/l_def/l_utl/l_sem/l_dbg). pa_7070은 `.c` 임베드 X, **libyarb.a 링크**(arb.h/config.h/def.h/sem.h from yarb/inc + pthread). **LP 매칭엔진(349개)보다 훨씬 가벼움** — 별도 대규모 엔진 시스템 없음.
- **빌드**: Makefile gcc `-no-pie`, `-I/fsfxwin/fep/yarb/inc -I/fsfxwin/fep/st01/inc`(하드코딩) → Linux 재빌드(libblp 방식). config `yarb/conf/config.ini`.
- **부팅 SHM = 자체 IPC_CREAT**(LP와 결정적 차이 — 엔진 선생성 불요): `g_cfg.d_strat_shm_key`(SharedRoot)·`fut_oid_shm_key`(FutOrdNo)를 `shmget(...,IPC_CREAT)`로 자체 생성, `FF_SHM_KEY`(파생 시세)도 IPC_CREAT. → **부팅에 엔진 스텁 불요**(LP는 blp_shm_stub 필요했음).
- **선물 시세 = FEP `FF_SHM`(0x41000003, 전역)** — `g_md_shm_fut=(SHM_FIN_FUT*)shmat(FF)`. pc_ 파생 시세 수신(pc_7100_ur 기존)이 채움.
- **⚠ FX = `shm_fx`(SHM_FX) + FX 주문 = pf_ venue 미구축(P4 관문)**. FX 시세 feed·주문 송신용 신규 시장 letter 필요.
- 주문 emit 동일 `DSHM_W(TS_W1_1)` → SEAM_ORD_W **2-leg**(선물→SEAM_ORDQ_DERIV `Seam_Order_Queue('c')`, FX→SEAM_ORDQ_FX `('f')`). forwarder도 시장별(pc_/pf_).
- 기타: pthread, SLog/g_cfg/SharedRoot/Arb_Init(yarb 자체 타입), tr_seq=2.

### P4 단계 (차익거래, 2026-08-10 계획)
- **P4-0 — ✅ 완료(2026-08-10): libyarb.a 8/8 + FX_Sise 스키마 이식**:
  - **FX스키마 관문 발견·해결**: yarb `l_set.c`가 `RISK.FX_Sise[][]`(FEP RISK에 없던 FX 시세 필드) 사용 → 차익거래는 첫 빌드부터 FEP RISK의 FX 스키마 선결. **결정 (A) 채택** — 유안타 원본으로 이식.
  - **유안타 헤더 확보**: `yanta_pa/inc/`(shm_memory.h·fx.h 등). `shm_memory.h`에 `SHM_MAX_FX=30`, `FX_SISE_FORMAT`(m_exch/m_item_cd/complete_f/sell·buy_1_price/bid·ask_quote_id/auto_use…), `RISK.FX_Sise[1][SHM_MAX_FX]`.
  - **이식 실행**: FEP `st01/inc/shm_memory.h`에 byte-exact 삽입 — SHM_MAX_FX(SHM_MAX_NOTE 뒤) + FX_SISE_FORMAT typedef(STANDARD_SISE_FORMAT 뒤) + `RISK.FX_Sise` (**}RISK 앞 append = 오프셋 보존**). `fx.h → st01/inc/fx.h` 이동(지시).
  - **검증**: libfepP 재빌드 0에러(RISK 변경 정상), l_set.c 재컴파일 성공(FX_Sise 에러 소멸; 단 l_set.c는 shm_memory.h 직접include라 **`-include fep_fepp.h` 프렐류드 강제 필요** — 다른 l_*.c는 plain, l_set만). **libyarb.a 8/8(182KB)**. 전 모듈(po/pc/pb) 재링크 0에러 + 유닛 184/0 무회귀(오프셋 보존 확인).
  - ⚠배포: RISK 크기 증가 → RISK SHM(0x41000014) ipcrm+pz 재생성(장마감 배포).
- **P4-1 — 🔧 진행중 + 유안타 SHM 스키마 divergence 발견(2026-08-10)**:
  - 추가 이식 완료·검증: `SHM_FX`(유안타 shm_memory.h 577-615, CO_B6FX 등 CO_* deps는 FEP krx_mk.h에 이미 존재)+`Shm_FX` 전역 → FEP shm_memory.h(SHM_FIN_FUT 뒤 append). `YSK_DATA_HEAD`(유안타 pa_struct.h 1098-1119, primitive만) → FEP pa_struct.h(SEARCH_HEADER 뒤). `#include "fx.h"`를 shm_memory.h에 추가(SHM_FX의 CO_B6FX·pa_7070의 SMB_ST 해소). **libfepP 재빌드 0에러 + po/pc/pb 재링크 + 유닛 184/0 무회귀**(전부 순수 추가·오프셋 보존).
  - ✅**야간/FX 키 추가 완료(2026-08-10)**: FEP shm_memory.h에 `SHM_MAX_DEV_FIF_N=1000`·`DEV_MK_FIF_N=2`(SHM_MAX_DEV_FIF 뒤) + `FF_N_SHM_KEY=0x41000004L`·`FX_SHM_KEY=0x41000005L`(FF_SHM_KEY 뒤, **FEP 0x41 스킴**; 유안타 0x21 아님). libfepP 재빌드+유닛 184/0 무회귀. → pa_7070의 미정의 키 4개 해소.
  - ✅**(A) auto_use 배열화 완료(2026-08-10)**: FEP `SHM_FIN_FUT.auto_use`(shm_memory.h L540) `int`→`char[MAX_AUTO_PROC]`(유안타 정합, SHM_FIN_FUT만). **영향 0** — po/pc/pb/pa 재링크 0에러(기존 코드가 SHM_FIN_FUT.auto_use 스칼라 접근 없음, 앞선 5파일 우려는 오탐=다른 struct auto_use). ⚠FF 시세 SHM 크기 증가→배포 재생성.
  - ✅**Key_Search_FX·alrt_msg 구현(2026-08-10)**: 유안타 private libfepP에만 있어 미제공 → `sub/fx_util.c` 신설(libfepP 편입). `Key_Search_FX(excode,symb)`=RISK.FX_Sise[0][]를 (m_exch,m_item_cd)로 검색·등록→idx(만석 NOTOK); `alrt_msg`=로그 스텁(pf_ 실배선 P4-full).
  - ✅**pc_7070_mp 링크 성공(185KB)** = 차익거래 어댑터 FEP 컴파일+링크 완결. 전 모듈 재링크+유닛 184/0 무회귀. **P4-1 완료.**
- **P4-2 — ⚠ arb 주문경로 발견(2026-08-10, LP와 상이)**: LP는 FEP `DSHM_W(TS_W1_1)`→SEAM_ORD_W 스왑(P2)이 됐지만, **arb(pa_7070/yarb)는 FEP DSHM_W 주문 emit이 없음**. `l_arb_fut_ord`/`l_arb_spot_ord`→**`l_add_wqlist_fut/spot`**(yarb 내부 write-queue)→flush→**msgq**(l_utl.c)→**브로커 게이트웨이**(선물=KRX day/ngt, FX=jpm/nh/sh). = yarb-native 다중브로커 주문경로. **P4-2 SEAM 스왑 부적용.**
  - ✅**결정=(C) yarb-native 게이트웨이 유지(2026-08-10)**: arb는 FEP SEAM/송신부 우회, yarb 자체 msgq→게이트웨이(선물=KRX day/ngt, FX=jpm/nh/sh)로 주문. **⇒ arb emit은 코드 변경 없음(yarb-native 유지) — P4-2 SEAM 리다이렉트 arb 미적용.** 함의: arb는 대체로 자립 서브시스템 — FEP는 **시세(FF_SHM 파생 + FX_Sise) 제공 + 프로세스 기동**만 담당. 주문(선물·FX)은 yarb 게이트웨이. **taxonomy 정정**: pf_(FX)는 arb 맥락에선 **FX 시세 수신 전용**(pa_7400_ur 이식→FX_Sise), FX 주문은 yarb jpm/nh/sh; 통화선물 주문도 yarb KRX 게이트웨이(FEP pc_ 아님). SEAM_ORD_W는 LP(pa_5050) 전용으로 유지.
- **P4-3 부팅검증**: 자체 SHM IPC_CREAT + FF_SHM(+shm_fx 스텁) → yarb 부팅(Arb_Init·SEAM_ORD attach). **LP P3a 동형이나 엔진 스텁 불요**.
- **P4-full**: **pf_(FX) venue 신구축**(시세 수신→shm_fx + 주문 송신) — 차익 FX leg 실동작 선결. + pc_ 파생 시세 feed로 실호가.
- 재사용: `Strategy_Letter`(52xx→? — 차익 letter 매핑 추가 필요), SEAM_ORDQ_DERIV/FX(기존), forwarder 패턴(pb_1109).

## 편입 방식 결정 (A vs B) — **운영자 확인 필요**
- **(A) MAT 지원 서브셋 링크**: mem/sem/mcp(+deps)만 Linux로 `libmats.a` 빌드해 pb_5050_mp에 링크. **벤더 semantics 그대로**(faithful), 범위 한정(프레임워크 전체 아님). blp가 이미 FEP SHM 대면이라 표면 좁음. **권장**.
- **(B) FEP 프리미티브로 재호스팅**: mem/sem/mcp를 FEP 네이티브(semipc/shmipc) shim으로 대체. 벤더 lib 불요·이중런타임 없음이나, MAT semantics를 정확히 복제해야(오차 시 정합성 리스크).

## 편입 방식: **A(MAT 서브셋 링크) 확정 (2026-08-10)**

## 단계별 계획
- **P0 — MAT 지원 서브셋 Linux 빌드 — ✅ 완료(2026-08-10)**: 벤더 makefile(/fsfxwin 하드코딩·심링크) 대신 **직접 gcc -m64**로 컴파일. blp 요구 표면 = `Mem_*`/`Sem_*`/`Mcp_OpenServer·Recv·RecvT`. `mem.c·sem.c·semlog.c·mcp.c` 클린 컴파일(`-Iinclude -Imem -Isem -Imcp -Ist01/inc`), `memlog.c`는 선재버그(`sz` 미선언)지만 blp 미사용→제외. 서버 `~/new_fep/staging/mat/libmats.a`(45KB, Mem_*/Sem_*/Mcp_* T심볼 확인). → **근본 빌드 리스크 retire.**
- **P1 — pb_5050_mp 컴파일·링크 편입 — ✅ 완료(2026-08-10, behavior 무변경)**: `pa_5050_mp.c`(blp*.c 임베드) 컴파일 clean(경고 13줄), 링크 시 미해결 = MAT 변환/로깅 유틸 → 지원 서브셋 확장으로 해소. `bin` 227KB `pb_5050_mp` 링크 OK.
  - **아카이브명 `libblp.a`**(매칭엔진 libmats 충돌 회피, 운영자 지시).
  - **libblp.a 오브젝트 셋**(직접 gcc -m64, 벤더 makefile 미사용): `mem/mem.o sem/sem.o sem/semlog.o mcp/mcp.o etc/conv.o etc/etc.o etc/xcube_convert.o log/log.o log/lsm.o`. (`memlog.c`·`util/cfunc/*`는 선재버그/미필요 제외.)
  - **재현 레시피**(서버 `~/new_fep/staging/mat`): 컴파일 `gcc -c -m64 -O -D_GNU_SOURCE -Iinclude -Imem -Isem -Imcp -Ietc -Ilog -I$_FEP_HOME/st01/inc <src>`; 아카이브 `ar rucs libblp.a <objs>`; 프로세스 `gcc -c ... -DNO_INISAFE pa_5050_mp.c` → 링크 `gcc -m64 -O pb_5050.o st01/lib/libfepP.a mat/libblp.a -ltirpc -lm -o bin/pb_5050_mp`.
  - **남은 P1 마무리**: mk.sh(Make_PB) 정식 편입(현재는 staging 직접 gcc) + 소스/벤더트리 st01 배치 결정 + 경고 13줄 점검.
- **P2 — 주문 emit SEAM 배선(정합성-핵심) — 🔧 진행중(2026-08-10)**:
  - **레코드 계약 규명(핵심 발견)**: 전략 `pa_5050_mp` DATA_SIZE=2048(W_Fmt≈2119B) ↔ 송신부 `pb_1100_ts` DATA_SIZE=400(레코드 471B). `Od_Write_Data`는 `memcpy(&W_Fmt, Order_St, sizeof(BUFF_RW_HEAD)+ODS)` 후 `DSHM_W(TS_W1_1,&W_Fmt,1)`(mk_gbn/p_flag로 채권일반/채권LP/선물파생 분기, o_gbn 채널). 주문=TCHODR40001 254B. **`SEAM_ORD_W`에 sizeof(전략 W_Fmt) 넘기면 480 슬롯 오버플로** → wire 레코드 471(송신부 기준) 고정 전송. DSHM/SEAM 바이너리 고정레코드라 LineFeed 무관.
  - ✅ **계약 상수·가드**: `SEAM_ORD_WIRE_RECSZ=471`(seam_queue.h) + 컴파일 가드(≤SEAM_Q_RECSZ 480). lib 빌드·유닛 184/0.
  - ✅ **emit rewire 적용·컴파일검증(2026-08-10)**: `pa_5050_mp.c` `Od_Write_Data`의 `DSHM_W(TS_W1_1,&W_Fmt,1)` 3곳 → `SEAM_ORD_W(SEAM_ORDQ_BOND, &W_Fmt, SEAM_ORD_WIRE_RECSZ)`(채권일반·채권LP o_gbn=0) / `SEAM_ORDQ_DERIV`(선물파생 o_gbn=2). `#include seam_queue.h` + main `SEAM_ORD_Init()`. ASCII 앵커(o_gbn 값) perl 치환으로 EUC-KR 주석 무손상. `pb_5050_mp` 재빌드 compile+link OK(232KB, SEAM_ORD_W/Init/Seam_Order_Queue 심볼 확인). 백업 `pa_5050_mp.c.p2bak`.
  - **line 379 `Dshm_Add_Count(TS_W1_1,1)` 미변경(의도)**: File_Event_Rtn 입력루프의 커밋으로 의미 불확실(입력커밋 aliasing 가능성) → 건드리지 않음. DSHM 출력채널은 이제 미사용이라 카운터 drift 무해. **P3 E2E에서 입력 재처리 관찰 시 재검토**.
  - emit 런타임 검증은 전략 실기동(P3). 소비자측은 forwarder로 분리(아래).
  - ✅ **소비자측 forwarder + E2E 완료(2026-08-10)**: `src/PB/pb_1109_mp.c` 신규 — 'b' 부문에서 `SEAM_ORD_R(SEAM_ORDQ_BOND)` 소비 → `F_W(OFN_1)`로 송신부 입력 중계(**pb_1100_ts 무변경**, 수신측 SEAM 소비 대칭; po_1290 루프 골격 + wire 471 복사). Poll_File용 IFN도 등록. compile+link OK(libfepP만, BLP 불요). `utl/seam_ord_inject.c`(raw shmget+seam_ring_write, config 불요 스텁). `run_fwd_e2e.sh`(pb_1109_mp Proc_19 등록). **E2E PASS**: 스텁 TCHODR40001→SEAM_ORDQ_BOND(w_seq=1)→forwarder `forwarder OUT[pb_1109_out] TR[TCHODR40001]`→출력큐 도달(485B). SEAM_ORD 세그먼트 0x41000016(TEST 0x42000016) 생성 확인.
  - **P2 상태**: emit(전략)=코드완료·컴파일검증, 소비자(forwarder)=코드+E2E검증. **완결.** 운영 배선은 forwarder OFN_1=pb_1100_ts 입력으로(현재 검증용 관측 파일큐). 전략 실기동 통합은 P3.
- **P3 — E2E — ⚠ 재프레이밍(2026-08-10, 매칭엔진 의존 발견)**:
  - **발견**: 전략 `pa_5050_mp`는 **매칭엔진의 얇은 FEP측 어댑터**. `Init_Parameters`의 `Blp = Blp_Open(BLP_KEY,1)` → `Mem_Open(key)`(기존 SHM **attach만**, 생성 X) + `Sem_Open`. NULL이면 `Exit_Process` → **BLP SHM 없이는 부팅 불가**. BLP SHM 생성·채움은 **별도 매칭엔진**(`mat_lpbnd/{proc5,otp7,mds4,mrg10,match81}` — match/만 81개 .c). 전략은 LP호가를 스스로 계산하지 않고 매칭엔진이 BLP SHM에 넣은 것을 `Blp_ProcessExecute`로 읽어 `Od_Write_Data`→(P2)`SEAM_ORD_W`로 emit. (BLP_KEY≈0xbb001001, 정의 위치 미확인.)
  - **P3 옵션**:
    - **P3a(부팅검증) — ✅ PASS(2026-08-10)**: `utl/blp_shm_stub.c`(매칭엔진 대역: `Mem_Create(BLP_KEY=0xbb001001, sizeof(BLP_MAP)=249048)`+`Sem_Create`+`map->stat.key/service=1`, blp_all.c Blp_Create 미러; FEP 프렐류드 fep_fepp.h 선행 필수). `run_blp_boot_e2e.sh`: pz z+o, 스텁 BLP SHM 생성, `exec -a po_5050_mp $BIN/pb_5050_mp` 기동. **검증: Blp_Open success + SEAM_ORD_Init OK + Init_Parameters ...END + alive**. → 전략이 FEP 프로세스로 부팅(Init_Proc)·매칭엔진 SHM attach·SEAM 주문큐 attach·메인루프 도달. ⚠타이밍: PA_5050_MP는 Init_Parameters 전 `sleep(5)`(클라 파라미터 대기) → 하니스 8초 대기. **FEP측 통합 완결.** 실호가 emit은 매칭엔진 필요(P3-full).
    - **P3-emit스텁(b) — ⚠ 범위확대 규명(2026-08-10, 실현성 조사)**: `Od_Write_Data` 발화 2경로 모두 전략 활성화 프로토콜 선행. ①Auto_Logic(case 3): 판단근거는 **전역 FEP SHM**(Shm_Note 시세 0x41000001 + Shm_Strrg 전략 0x41000012, 스텁 가능)이나 **case 3 poll채널은 Set_Flag>0(500200 Set_In_Param 활성화 후)에만 활성** + 조건(bond_accum_exec_vol>0·HogaLastGbn=0·volume>50·up-tick·item_seq·OD_SEQ 정합) 정밀세팅(krx_mk.h 98KB) 필요. ②Blp_ProcessExecute: 매칭엔진 BLP SHM 상태 필요(=P3-full). → (b)는 "medium 스텁"이 아니라 **매칭엔진 없는 전략 운영환경 재현**(500200+시세채널+전역SHM+OD_SEQ). **ROI 낮음**: emit코드=P2 컴파일검증, forwarder=E2E검증, 부팅+SEAM attach=P3a검증 → (b) 한계증명가치(런타임 SEAM_ORDQ_BOND 착지)는 좁은 갭·저위험인데 시뮬 비용 큼. **보류 권장** — 실제 가치는 P3-full(매칭엔진).
    - **P3-full(대규모)**: 매칭엔진(mat_lpbnd match/ 등) Linux 빌드·기동→실제 LP호가. 별도 프로젝트(자체 config main.cfg/process.cfg·데몬).
  - 9000 cp-into-slot 기동경로(500100→Strategy_Letter='b'→po_슬롯)는 어느 옵션에서도 동일.
- **P4 — 차익(7070)**: 동형 + `pf_`(FX) venue 신규(별도 트랙). `SEAM_ORDQ_DERIV`+`_FX` 2 leg.

## 리스크 · 주의
- MAT 재빌드 경로/누락 심볼(P0에서 조기 확인). EUC-KR 빌드 경고. MAT 자체 SHM/IPC가 FEP와 중복 시 격리 필요.
- 주문 emit 변경 = 정합성-핵심 → seam_peek/shm_snap + E2E 필수.
- 슬롯 런타임 identity D_K='o'(Model B): blp가 시장 내부인지라 무해하나 주문경로만 SEAM.

## 선행 완료(재사용 자산)
- 걸림돌① `Strategy_Letter`(5050→b), 걸림돌② Model B, §5.3 SEAM 주문큐(`SEAM_ORD_*`, `Seam_Order_Queue`) — 모두 완료·단위검증. 전략 배선만 남음.
