#!/bin/sh
#------------------------------------------------------------------------
#   run_vx_fx.sh — VX-4a 가상 FX 거래원(SMB_ST/TCP) 스모크 테스트
#   File: st01/test/vexch/run_vx_fx.sh
#
#   mock_fx_venue(카탈로그 order_proto=fx_smb) 기동 → vx_fx_probe 가
#   SMB_ST 신규주문('D') 전송 → 체결통지('8') 수신. ClOrdID echo(상관) +
#   New ack + 체결(fill_rule=full → 2건) 검증. FEP 인프라 불요.
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/new_fep}; ST01=$FEP/st01
INTEG=$ST01/test/integ; VEXCH=$ST01/test/vexch; CFG=$ST01/cfg
PORT=19100; CLORD=FXPROBE00000042
VENUE_LOG=/tmp/mock_fx_$PORT.log
mkdir -p "$INTEG/bin"
cleanup(){ [ -n "$VPID" ] && kill -9 "$VPID" 2>/dev/null; }
trap cleanup INT TERM EXIT

# --- build (vexch_catalog 링크, fx.h 는 st01/inc) ---
cc -I$ST01/inc -I$VEXCH -o $INTEG/bin/mock_fx_venue \
   $INTEG/mock/mock_fx_venue.c $VEXCH/vexch_catalog.c 2>&1 | grep -iE 'error|정의되지' \
   && { echo "판정 : FAIL (mock_fx_venue build)"; exit 1; }
cc -I$ST01/inc -I$VEXCH -o $INTEG/bin/vx_fx_probe \
   $INTEG/mock/vx_fx_probe.c 2>&1 | grep -iE 'error|정의되지' \
   && { echo "판정 : FAIL (vx_fx_probe build)"; exit 1; }

# --- 가상 FX 거래원 기동 ---
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_fx_venue $PORT > /tmp/vxfx_venue.out 2>&1 &
VPID=$!
sleep 1

# --- 프로브: 신규주문 전송 + 체결통지 드레인 ---
$INTEG/bin/vx_fx_probe $PORT $CLORD > /tmp/vxfx_probe.out 2>&1
sleep 1
kill -9 $VPID 2>/dev/null; VPID=

# --- 검증 ---
RES=$(grep 'VXFX RESULT' /tmp/vxfx_probe.out)
EXEC=$(echo "$RES" | grep -oE 'exec=[0-9]+' | cut -d= -f2); EXEC=${EXEC:-0}
CORR=$(echo "$RES" | grep -oE 'correlated=[0-9]+' | cut -d= -f2); CORR=${CORR:-0}
CATLOAD=$(grep -c 'vexch_catalog: loaded' "$VENUE_LOG" 2>/dev/null); CATLOAD=${CATLOAD:-0}
FILLED=$(grep -c 'FX: FILLED' "$VENUE_LOG" 2>/dev/null); FILLED=${FILLED:-0}

echo "========================================"
echo "  VX-4a: 가상 FX 거래원 (SMB_ST/TCP)"
echo "========================================"
echo "  카탈로그 로드           : $CATLOAD"
echo "  수신 체결통지(exec) 수   : $EXEC   (New ack + 체결 = 2 기대)"
echo "  ClOrdID 상관(correlated) : $CORR   (1=주문↔체결 매칭)"
echo "  거래원 FILLED 로그       : $FILLED"
echo "  판정 : $([ "${EXEC:-0}" -ge 2 ] && [ "${CORR:-0}" -eq 1 ] && echo 'PASS (SMB_ST 주문→ack→체결, 상관 확인)' || echo 'FAIL')"
echo "========================================"
echo "--- vx_fx_probe out ---"; cat /tmp/vxfx_probe.out 2>/dev/null
echo "--- mock_fx_venue log (마지막 12줄) ---"; tail -12 "$VENUE_LOG" 2>/dev/null
