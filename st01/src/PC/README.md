# PC — 파생(선물/옵션) 시장 모듈 (KRX-direct)

시장 재편 A안: **letter = 시장**. PC = 파생. 전 시장 KRX-direct 통일(IMECO 코덱 없음).

## 위치
- 소스: `st01/src/PC/pc_*.c`
- 오브젝트: `st01/obj/PC/`
- 빌드: `mk.sh pc` (개별) — 소스가 생기기 전까지 no-op. `_FEP_SUBDIR`에 `C` 포함되어 `mk.sh all`/`src`에도 편입되나 소스 0개면 무영향.
- 런타임: FIFO `st02/FIFO/PC`, DAT `st02/DAT/PC`, LOG `st03/LOG/PC`

## 빌드모델 (설계 §6, 여기서 파일럿)
- **1 소스 = 1 바이너리**. `-DPCxxxx` 다중바이너리 없음.
- 프로세스 인스턴스(ID/포트/기능코드/전문크기)는 `proc.ini`·`file.ini`에서 런타임 로드.
- 데몬이 `exec -a pc_{NNNN}_{type}`로 argv[0] 부여 → `Init_Proc`가 파싱.
- 남는 `-D`는 플랫폼/기능 플래그(NO_INISAFE, HOLIDAY_*, osname)뿐.

## 전문(KRX EXTURE 3.0, 현/파 공용)
- 주문(호가): `TCHODR10001/2/3` (294B) — `KRX_JUMUN_DATA` (pa_struct.h) 재사용
- 응답(회원처리호가): `TTRODP11301` (318B) — `krx_ttrodp11301.h`, 거부 `11321`/자동취소 `11303`
- 체결결과: `TTRTDP21301` (233B) — `KRX_SETTLE_DATA`
- 세션: SCHLIQ/OPQ/HEQ/LOQ (전 시장 공통, 코드 기존)

## Phase 1 진행: pc_ 파생 주문송신 (`pc_1100_ts.c`)
`pb_1100_ts.c` 이벤트루프 템플릿 + `TCHODR10001`(294B, `KRX_JUMUN_DATA`) 코덱으로 신구축 완료.
- **소스 1개 = 바이너리 1개**: `pc_1100_ts.c` → `bin/pc_1100_ts` → 프로세스 `pc_1101_ts`(argv[0], `exec -a`).
- 함수코드 재인덱스: 2xxx(파생, 시장=앞자리) → **1xxx(주문/체결, 기능=앞자리)**. 시장은 letter(c)가 담당.
- 채권 대비 차이: 전문크기 검출 254/255 → TCHODR10001/2/3=294B, `KRX_NOTE_ALL_JUMUN_Q_FMT`→`KRX_JUMUN_Q_FMT`, `CHK_PROC` `pb_1201_tr`→`pc_1201_tr`(Phase 2 산출물).
- 서버 빌드검증: `mk.sh pc` → `bin/pc_1100_ts` 링크 0에러.

## Phase 2 진행: 파생 응답/체결 수신 (`pc_1200_tr.c`)
`pb_1200_tr.c`(채권 수신) 템플릿 기반, 현/파 공용 전문만 차별화(IMECO `pa_2200_tr.c` 아님). → `bin/pc_1200_tr` → 프로세스 `pc_1201_tr`.
- TR 라우팅: 체결 `TTRTDP21301`(233B)→Write_Data(2), 회원처리호가 `TTRODP113*`(정상 11301/거부 11321/자동취소 11303, 318B)→Write_Data(1), IF종료 `TCHEDP99000`→종료. 수신 세션래퍼 Header=`TCHTDP00000`.
- 개시 `SCHOPQ10000`/`SCHOPR10000`(체결·장운영·DropCopy 전용 회선). `#if defined(B120x)` 분기 없음(단일). NO_INISAFE.
- TR 코드는 `inc/krx_trcode.h`에 현/파용 추가(`TR_ORDER_RESP`/`_EXECUTION`/`_SESSION_TRADE` 등).
- 서버 `mk.sh pc` → `bin/pc_1200_tr` 링크 0에러. (수신경로 E2E는 mock_krx 체결 push 확장 후 — plan Task 2.1 참조)

## Phase 2 진행: 파생 시세수신 (`pc_7100_ur.c`)
KRX 파생시세는 **UDP 멀티캐스트**(IMECO-TCP 폐기) → `pc_7100_tr` 아니라 **`pc_7100_ur`**. `pb_7100_ur.c`(채권 시세수신, UDP→TCP 중계+HA) 템플릿 기반.
- TR 분류 파생화: `A3/G7/B6/R1` = A301F(체결)·G701F(체결G7)·B601F(우선호가)·R101F(장운영TS) (접미사 F, 채권은 K/M4).
- 개선 빌드모델: `#if defined B7102/B7103` 다중바이너리·HB offset 제거 → 1소스=1바이너리, HB포트 `_HA_HB_PORT` env(기본 50020). HA(heartbeat/failover) 승계, dev=STANDALONE.
- 서버 `mk.sh pc` → `bin/pc_7100_ur` 링크 0에러. (시세 E2E는 UDP 멀티캐스트 송신 mock 필요 — plan Task 2.2)

상세·E2E: `docs/01-plan/features/pc-derivatives-pilot.plan.md`
