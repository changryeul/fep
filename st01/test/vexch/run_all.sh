#!/bin/sh
#------------------------------------------------------------------------
#   run_all.sh — 가상거래소(vexch) 전체 테스트 확인 (한 번에 실행+요약)
#   File: st01/test/vexch/run_all.sh   (Linux 서버, ~/new_fep)
#
#   사용법: sh run_all.sh
#   각 하니스를 순차 실행하고 판정(PASS/FAIL/PENDING)을 집계한다.
#   개별 상세는 각 하니스를 직접 실행: 아래 [실행] 명령 참조.
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/new_fep}; VEXCH=$FEP/st01/test/vexch; E2E=$FEP/st01/test/e2e
pass=0; other=0; RESULT=$FEP/st01/test/e2e/result; mkdir -p "$RESULT"

run_one() {
    name="$1"; shift
    logf="$RESULT/all_$(echo "$name"|tr ' /→' '___').log"
    "$@" > "$logf" 2>&1
    v=$(grep -aoE "판정 *: *(PASS|FAIL|PENDING)" "$logf" | head -1 | grep -oE "PASS|FAIL|PENDING")
    [ -z "$v" ] && v="NO-VERDICT"
    case "$v" in PASS) pass=$((pass+1));; *) other=$((other+1));; esac
    printf "  %-34s : %-8s (log: %s)\n" "$name" "$v" "$logf"
}

echo "========================================================"
echo "  가상거래소(vexch) 테스트 확인 — $(date '+%Y-%m-%d %H:%M' 2>/dev/null)"
echo "========================================================"
echo "[1] VX 엔진 자체검증 (주문/체결+시세+correlation+order-aware, FEP 무관)"
run_one "VX engine (run_vexch)"          sh "$VEXCH/run_vexch.sh"
echo "[2] KRX 파생 시세 → 실 FEP 수신부 pc_7100_ur (멀티캐스트)"
run_one "VX-2c KRX sise (run_vx_pc)"     sh "$VEXCH/run_vx_pc_e2e.sh"
echo "[3] FX 시세 → 실 FEP 수신부 pf_7400_ur → FX_Sise SHM"
run_one "FX sise (run_pf_sise)"          sh "$E2E/run_pf_sise_e2e.sh"
echo "[4] FX 거래원 주문/체결 (SMB_ST/TCP, 가상 거래원)"
run_one "VX-4a FX venue (run_vx_fx)"     sh "$VEXCH/run_vx_fx.sh"
echo "[5] 실 FEP FX 다거래원 pf_1100_ts ↔ JPM/NH (E2E, excode 라우팅)"
run_one "VX-4c FX multi-venue (run_vx_fx_e2e)" sh "$VEXCH/run_vx_fx_e2e.sh"
echo "--------------------------------------------------------"
echo "  합계: PASS=$pass  기타(FAIL/PENDING/NO-VERDICT)=$other"
echo "========================================================"
echo ""
echo "[개별 상세 실행 방법]"
echo "  VX 엔진 (주문/체결 4전문 + 시세 + 상관 + order-aware):"
echo "    sh $VEXCH/run_vexch.sh"
echo "  이중소켓 교차상관 (주문 A소켓 → 체결 B소켓 echo):"
echo "    VX_CATALOG=$FEP/st01/cfg/vexch.ini \$BIN/mock_krx_server 19995 &"
echo "    \$BIN/vx_probe 19995 send ; \$BIN/vx_probe 19995 recv"
echo "  KRX 시세 → pc_7100_ur (실 FEP 수신):"
echo "    sh $VEXCH/run_vx_pc_e2e.sh"
echo "  FX 시세 → pf_7400_ur → FX_Sise:"
echo "    sh $E2E/run_pf_sise_e2e.sh"
echo "  FX 거래원 주문/체결 (SMB_ST 신규→ack→체결, 상관):"
echo "    sh $VEXCH/run_vx_fx.sh"
echo "  실 FEP FX 주문 프로세스 pf_1100_ts ↔ 가상 거래원 (E2E):"
echo "    sh $VEXCH/run_vx_fx_e2e.sh"
echo "  fill_rule 변형(partial/reject): cfg/vexch.ini fill_rule 변경 후 run_vexch"
echo "  상품 추가 = cfg/vexch.ini 에 [PRODUCT_*] 블록 1개 추가 → 위 테스트 재실행"
