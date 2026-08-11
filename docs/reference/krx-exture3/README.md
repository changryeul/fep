# KRX EXTURE 3.0 전문 — durable 추출본

원본 스펙 `KRX+EXTURE+3.0+시장접속+프로토콜_전문(회원사용)_v3.24_20260730_주식선물옵션정기변경.xlsb`(repo 루트)를 도구 비의존 형식(CSV)으로 고정한 것. **구현 시 이 CSV를 참조**하고, 원본 `.xlsb`는 재추출·검증용으로만 둔다.

- **버전**: v3.24, 2026-07-30 (주식선물옵션 정기변경)
- **추출일**: 2026-08-06

## 산출물

| 파일 | 내용 |
|------|------|
| `interface-list.csv` | 전문 카탈로그 373건. TR-CODE·전문명·길이·시장적용(유가/코스닥/코넥스/파생/파생야간/일반채권/소액채권/KTS/REPO)·송수신·서비스·차세대변경 |
| `fields.csv` | 전 전문 필드 레이아웃 5,392행. TR_CODE·HDR/DATA·순번·항목명(KR/EN)·DataType·길이·필수·정의·차세대변경 |
| `headers/*.h` | **전문별 C 헤더 333개** (전문별 1개, 신규/정정/취소 변형도 각각). 자동생성 |
| `extract_exture.py` | .xlsb → CSV 재추출 (pyxlsb) |
| `gen_headers.py` | CSV → 전문별 헤더 재생성 |

## 전문 헤더 규약 (`headers/*.h`)

바이트 오프셋 수기계산(`DataBuff[82]`)을 없애고 **이름 기반 필드 접근**으로 관리 (CLAUDE.md 컨벤션 정합).

- **전문별 1개** 파일: `<trcode소문자>.h`, struct `<TRCODE>_DATA`
- **필드명 = 기존 코드 형식(Title_Case + 언더스코어)** (항목영문명 → `Word_Word_Word`): 예 `MESSAGE_SEQUENCE_NUMBER`→`Message_Sequence_Number`, `ME_GRP_NO`→`Me_Grp_No`. 기존 struct 스타일(`Transaction_Code`, `Order_Type`, `Board_id`)과 정합
- **고정폭 ASCII 전문**이므로 전 필드 `char name[len];` (오프셋은 struct 순서가 보장)
- **길이 자동 검증**: 각 헤더 주석에 `DATA 길이 합계` 표기 → 카탈로그 `길이(헤더제외)`와 대조. 261/266 정확 일치. 불일치 2건(수동 확인): `TTRTDP42311`(307/318), `TRDESP01301`(2702/2714). 조회계 3건은 카탈로그 길이 공백(가변).
- 차세대 신규/변경 필드는 주석에 `★신규`/`★변경` 표기
- struct는 **DATA부**만. 공통 헤더(KRX_HEADER)는 기존 `pa_struct.h`/코드 공용. 세션 전문(HEADER 전용)은 별도 struct 미생성.
- 재생성: `/tmp/xlsbenv/bin/python gen_headers.py` (특정 TR 미리보기: `gen_headers.py TCHODR10001`)

> 구현(시장 모듈 pc_/pa_/pb_) 착수 시 이 headers/를 `st01/inc/`로 편입·정리. 필드명 형태(대/소문자·접미사)는 규약 확정 시 gen_headers.py 한 줄 수정으로 전체 재생성.

### 재추출 (원본 변경 시)
```sh
python3 -m venv /tmp/xlsbenv && /tmp/xlsbenv/bin/pip install pyxlsb
/tmp/xlsbenv/bin/python docs/reference/krx-exture3/extract_exture.py "<xlsb경로>"
```
(`.xlsb`는 Excel 바이너리라 pyxlsb 필요. 시스템 pip는 PEP668로 막힘 → venv 사용)

### 조회 예
```sh
# 특정 전문의 필드 레이아웃
grep '^매매,TCHODR10001,' fields.csv | cut -d, -f5-11
# 현물+파생 공용 전문
awk -F, '$7=="현/파"' interface-list.csv
```

---

## 재편 핵심 매핑 (letter=시장 A안 직결)

**최대 소견: 현물과 파생은 주문·응답·체결 전문을 공유한다** (TR 앞자리 `1`/`2`). 채권만 별도(`4`). 세션은 전 시장 공통. → pc_(파생)·pa_(현물)은 같은 KRX 코덱, pb_(채권)만 별도 전문.

| 구분 | 현물+파생 (pa_/pc_) | 채권 (pb_) | 비고 |
|------|--------------------|-----------|------|
| **주문(호가)** | `TCHODR10001/2/3` (294B) | `TCHODR40001/2/3` (254B), 조성 `TCHMOR4xxxx` (255B) | 신규/정정/취소 |
| 경쟁대량주문 | `TCHAOR10001/3` (294B, 현물만) | — | |
| **응답(처리호가)** | `TTRODP11301`정상/`11321`거부/`11303`자동취소 (318B) | `TTRODP41301/2/3` (291B), 조성 `TTRMOP41301/2/3` (295B) | |
| **체결결과** | `TTRTDP21301` (233B) | `TTRTDP42301` (317B) | |
| **세션(공용)** | SCHLIQ/SCHLIR(로그온), SCHOPQ/SCHOPR(업무개시), **SCHOPQ10000/SCHOPR10000**(체결·장운영·DropCopy 전용 116B), SCHHEQ/SCHHER(회선), SCHLOQ/SCHLOR(로그아웃) | | 코드에 이미 구현 |

현 코드 대응: `pb_1100_ts`=`TCHODR40001`(채권주문), `pb_1200_tr`(B1201)=`TTRODP41301`(채권처리호가). 파생 pc_는 `TCHODR10001`(294B) 신구축 필요.

### TCHODR10001 (호가입력, 현/파, 294B) 주요 필드 — 예시 렌더
DATA부 앞부분 (전체는 `fields.csv` 참조). 채권 254B 대비 **차세대 추가필드**(★): 최소체결수량·시장조성자호가구분·자전거래방지·거래자ID·호가그룹번호·알고리즘전략구분 등.

| 순번 | 항목 | EN | Type | 길이 | 비고 |
|------|------|----|------|------|------|
| 1 | 메세지일련번호 | Message_Sequence_Number | Long | 11 | |
| 2 | 트랜잭션코드 | Transaction_Code | String | 11 | TCHODR10001 등 |
| 3 | ME그룹번호 | ME_GRP_NO | String | 2 | 회원사 '00' 전달 |
| 4 | 보드ID | BOARD_ID | String | 2 | 별첨-보드ID 매핑 |
| 5 | 회원번호 | MEMBER_NUMBER | String | 5 | |
| 6 | 지점번호 | BRANCH_NUMBER | String | 5 | |
| 7 | 주문ID | ORDER_IDENTIFICATION | String | 10 | 종목+회원+지점+주문ID = Unique |
| 8 | 원주문ID | ORIGINAL_ORDER_IDENTIFICATION | String | 10 | 신규는 SPACE |
| 9 | 종목코드 | ISSUE_CODE | String | 12 | ISIN (현/파/채/REPO 통합) |
| 10 | 매도매수구분 | ASK_BID_TYPE_CODE | String | 1 | 1매도 2매수 |
| 11 | 정정취소구분 | MODIFY_OR_CANCEL_TYPE_CODE | String | 1 | 1신규 2정정 3취소 |
| 12 | 계좌번호 | ACCOUNT_NUMBER | String | 12 | |
| 13 | 호가수량 | ORDER_QUANTITY | Long | 10 | |
| 14 | 호가가격 | ORDER_PRICE | Float | 11 | 가격없는 유형 0 |
| 15 | 호가유형코드 | ORDER_TYPE_CODE | String | 1 | 1시장가2지정가… **파생은 1,X 불가(T,W 대체)** |
| 16 | 호가조건코드 | ORDER_CONDITION_CODE | String | 1 | 0일반 3FAK(IOC) 4FOK |
| 17★ | 최소체결수량 | MIN_TRDVOL | Long | 10 | 회원사 '0' 전달 |
| 18★ | 시장조성자호가구분 | MM_ORD_TP_CD | String | 1 | 현물 0/1/2, 파생 0/2 |

> 시장별 업무규칙이 필드 정의(정의 컬럼)에 다수 명시됨(예: 호가유형 "파생시장 1,X 불가", 자사주 "파생은 0 해당없음"). 시장 모듈 구현 시 정의 컬럼 정독 필요.

## 별첨(코드 매핑) — 필요 시 원본 시트 참조
보드ID(8), 지수ID(9), 세션ID(11), 장운영상품그룹ID(16), 차세대 상품ID체계(18), 파생 계좌구분/증거금(27), 회원별접속포트(29) 등 30종. 구현 착수 시 해당 별첨을 개별 CSV로 추가 추출 권장.
