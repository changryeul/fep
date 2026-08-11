# rdcnt-underflow-guard Design Document

> **Summary**: KRX 시퀀스 재동기화 되감기(`RD_CNT -= mun`)가 읽기 커서를 음수로 만들면 드레인 루프(`WR_CNT > RD_CNT`)가 주문을 무한 재전송하는 잠재 취약성을, 되감기 전 언더플로 검사 + fail-safe 중단으로 경화. 운영자(도메인) 결정 반영.
>
> **Project**: FEP (Front-End Processor)
> **Author**: Claude (운영자 결정 협의)
> **Date**: 2026-08-06
> **Status**: Implemented, 서버 검증 완료
> **발견 경위**: E2E 하니스(I-9) — 버스트 시 주문 중복전송 추적 중 RD_CNT 음수화 경로 확인

---

## 1. 취약성

`src/PB/pb_1100_ts.c`의 KRX 세션 시퀀스 재동기화는 KRX가 보고한 `FirstSeq`(거래소가 인지한 마지막 시퀀스)에 맞춰 읽기 커서 `RD_CNT`(=`IFR(D_K,P_K,0,0)`)와 `INT_SEQ`를 되감아, KRX가 못 받은 주문을 재전송한다. 라이브 되감기 지점은 2곳 (grep 확증, 라인 469는 주석 처리된 죽은 코드):

| 지점 | 상황 | 되감기 |
|------|------|--------|
| LINK 응답 (SCHOPR, `:575` 부근) | `FirstSeq < INT_SEQ` (우리가 KRX보다 앞섬) | `mun = INT_SEQ - FirstSeq; RD_CNT -= mun; INT_SEQ -= mun;` |
| RP_DATA (주문오류 응답, `:449` 부근) | 주문 소켓에 DATA 수신(=오류) | `mun = (INT_SEQ - FirstSeq) - 1; RD_CNT -= mun; INT_SEQ -= mun;` |

**문제**: `mun > RD_CNT`이면 `RD_CNT`가 음수가 된다(=KRX가 우리 큐에 남은 것보다 더 이전 주문부터 재전송하라고 요청). 그러면 드레인 루프 `if (WR_CNT > RD_CNT) { Data_Event_Rtn(); continue; }`가 음수 커서를 "보낼 게 매우 많다"로 오인해 주문을 폭주 재전송 → 거래소에 중복 주문. E2E에서 RD_CNT=-27 관측, 100건 주입에 KRX 8202건 수신 재현(하니스 조건).

기존 코드는 반대 방향(`FirstSeq > INT_SEQ`, KRX가 앞섬)만 fail-safe 처리(ERROR + Exit_Process)했고, 되감기 방향의 언더플로는 무방비였다.

## 2. 운영자 결정 (2026-08-06)

1. **음수 되감기 시 동작**: **중단+알림(fail-safe)** — 기존 `FirstSeq>INT_SEQ` 케이스와 동일하게 ERROR 로그 + `Exit_Process`. 잘못된 재전송을 절대 하지 않음(중복주문 방지 우선). 대신 해당 회선 송신이 멈춰 운영자 수동 개입 필요.
2. **절대 상한**: **두지 않음(음수 가드만)** — 정상 범위 되감기는 신뢰. mun 임계 기반 방어는 추가하지 않음.

## 3. 구현

두 지점 모두 되감기 적용 **전** 언더플로 검사 추가:

```c
if (mun > RD_CNT) {
    /* (LINK 지점만) TCP2_LINE_ST = OpenFlag = OFF; */
    Log(USR_ERROR, "...Seq rewind underflow mun[%d]>RD_CNT[%d] "
            "FirstSeq[%d] INT_SEQ[%d] - 중복재전송 위험, 중단", ...);
    sleep(3);
    Exit_Process();
}
RD_CNT  -= mun;
INT_SEQ -= mun;
```

- LINK 지점(`:575`)은 기존 `FirstSeq>INT_SEQ` 블록과 동일하게 `TCP2_LINE_ST=OpenFlag=OFF`도 설정.
- 정상 흐름(`mun <= RD_CNT`)은 동작 불변.

## 4. 검증 (서버, EC2)

- **정적 확증**: `RD_CNT -=` 라이브 지점이 정확히 2곳이며 둘 다 가드됨(grep). 라인 469는 `/* */` 내부. → 이 fix 이후 프로덕션에서 RD_CNT는 음수가 될 수 없음.
- **클린 E2E**: SHM/`_seq` 초기화 후 file 모드 100건 버스트(간격 0) → **100건 주입 → 100건 송신 → mock KRX 100건 수신, 중복 0**. RD_CNT 1→100 정상 상승(음수 없음), 가드 미발동(정상 흐름).
- **회귀**: 전 소스 빌드(`mk.sh src`) 에러 0, 단위 테스트 156 통과(실패 3건은 무관한 기존 `test_chk_kor`).
- 단위 테스트 불가(KRX 세션/SHM 전역 결합) — deadcode-fix와 동일 선례.

## 5. 잔여/후속

- 이미 오염된 영속 상태(음수 `_seq`)는 이 fix가 치유하지 않음(가드는 "생성"만 차단). 운영 배포 시 미체결 0 상태(장 마감 후) + 정상 `_seq`에서 시작 권장.
- E2E 하니스의 mock_krx는 실 KRX처럼 주문 송신 소켓에 echo하지 않도록 유지(I-9).
