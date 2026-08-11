#!/bin/sh
#------------------------------------------------------------------------
#   run_vexch.sh — 가상거래소(vexch) 통합 빌드 + 자체 스모크 (VX-3)
#   File: st01/test/vexch/run_vexch.sh   (Linux 서버, ~/new_fep)
#
#   공유 카탈로그(vexch_catalog)로 구동되는 VX 도구를 한 번에 빌드/검증:
#     - mock_krx_server : 거래소 엔진(주문/체결 TCP, 카탈로그 구동 push)
#     - vx_probe        : 수신측 프로브(SCHOPQ10000 → push 드레인 → TrCode 스캔)
#     - vx_sise_pub     : 시세 발행기(카탈로그 sise_kind별 UDP)
#   자체 스모크: 거래소 기동 → probe로 카탈로그 상품 응답/체결 push 확인
#              + vx_sise_pub 카탈로그 로드/발행 확인. (FEP 무관 = VX 단위 검증)
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/new_fep}; ST01=$FEP/st01; INTEG=$ST01/test/integ; VEXCH=$ST01/test/vexch
CAT=$ST01/cfg/vexch.ini; PORT=${1:-19997}; PIDS=""
log(){ printf '[VEXCH] %s\n' "$1"; }
cleanup(){ for p in $PIDS; do kill -9 "$p" 2>/dev/null; done; }
trap cleanup INT TERM EXIT
mkdir -p $INTEG/bin

log "build (공유 vexch_catalog 링크) ..."
cc -I$ST01/inc -I$INTEG/lib -I$INTEG/mock -o $INTEG/bin/mock_krx_server \
   $INTEG/mock/mock_krx_server.c $INTEG/lib/krx_protocol.c $VEXCH/vexch_catalog.c 2>&1 | grep -iE "error" && { log "FAIL mock_krx_server build"; exit 1; }
cc -I$ST01/inc -I$INTEG/lib -o $INTEG/bin/vx_probe \
   $INTEG/mock/vx_probe.c $INTEG/lib/krx_protocol.c 2>&1 | grep -iE "error" && { log "FAIL vx_probe build"; exit 1; }
cc -I$ST01/inc -o $INTEG/bin/vx_sise_pub \
   $INTEG/mock/vx_sise_pub.c $VEXCH/vexch_catalog.c 2>&1 | grep -iE "error" && { log "FAIL vx_sise_pub build"; exit 1; }
log "build OK (mock_krx_server / vx_probe / vx_sise_pub)"

#--- 스모크 1: 주문/체결 (거래소 + probe) ---
log "smoke1 주문/체결: 거래소 기동(port $PORT) + probe ..."
VX_CATALOG=$CAT $INTEG/bin/mock_krx_server $PORT > /tmp/vexch_srv.log 2>&1 &
PIDS="$PIDS $!"; sleep 1
$INTEG/bin/vx_probe $PORT > /tmp/vexch_probe.log 2>&1
kill -9 $PIDS 2>/dev/null; PIDS=""; sleep 1

CATLOAD=$(grep -c "vexch_catalog: loaded" /tmp/vexch_srv.log)
# 서버 stdout은 full-buffered→kill -9시 유실. 서버 자체 로그파일(fflush됨)에서 push 집계.
PUSHCNT=$(grep -c "VX: push" /tmp/mock_krx_$PORT.log 2>/dev/null); PUSHCNT=${PUSHCNT:-0}
# probe 수신 TrCode = 권위 지표(실제 소켓 수신)
PROBE_TR=$(grep -cE "TTRODP11301 : 1|TTRODP41301 : 1|TTRTDP21301 : 1|TTRTDP42301 : 1" /tmp/vexch_probe.log)
# VX-1b: 거래소 order-aware(주문→카탈로그 상품 식별)
ORDERAWARE=$(grep -c "VX: ORDER recv TCHODR10001 \[PRODUCT_DERIV\]" /tmp/mock_krx_$PORT.log 2>/dev/null); ORDERAWARE=${ORDERAWARE:-0}
# VX-1b-full: 주문 회원영역이 응답/체결에 echo(상관) — probe가 수신에서 마커 확인
CORR=$(grep -cE "CORRELATION member echo \[VXCLORD0000042\] : [1-9]" /tmp/vexch_probe.log 2>/dev/null); CORR=${CORR:-0}

#--- 스모크 2: 시세 (vx_sise_pub 카탈로그 발행) ---
log "smoke2 시세: vx_sise_pub 카탈로그 발행 ..."
VX_CATALOG=$CAT $INTEG/bin/vx_sise_pub 2 > /tmp/vexch_sise.log 2>&1
SISE_LOAD=$(grep -c "loaded" /tmp/vexch_sise.log)
SISE_PUB=$(grep -c "CO_B6FX\|krx" /tmp/vexch_sise.log)

echo ""
echo "========================================"
echo "  가상거래소(vexch) 통합 스모크 (VX-3)"
echo "========================================"
echo "  [주문/체결] 거래소 카탈로그 로드 : $CATLOAD"
echo "  [주문/체결] 카탈로그 push 건수    : $PUSHCNT (2상품 응답+체결=4 기대)"
echo "  [주문/체결] 거래소 order-aware    : $ORDERAWARE (주문→상품 식별, 1 기대)"
echo "  [주문/체결] 주문↔체결 상관(echo)  : $CORR (회원영역 마커 echo, 1 기대)"
echo "  [주문/체결] probe TrCode 수신     : $PROBE_TR (4 기대)"
echo "  [시세]      발행기 카탈로그 로드  : $SISE_LOAD"
echo "  [시세]      발행 틱                : $SISE_PUB"
echo "  판정 : $([ "${CATLOAD:-0}" -ge 1 ] && [ "${PROBE_TR:-0}" -ge 4 ] && [ "${SISE_PUB:-0}" -ge 1 ] && echo 'PASS (공유 카탈로그가 주문/체결+시세 구동)' || echo 'FAIL')"
echo "========================================"
echo "--- 거래소 push 로그 ---"; grep "VX: push" /tmp/mock_krx_$PORT.log 2>/dev/null
echo "--- probe ---"; grep -E "received|: [0-9]" /tmp/vexch_probe.log
echo "--- 시세 발행 ---"; grep -E "loaded|CO_B6FX|krx" /tmp/vexch_sise.log
