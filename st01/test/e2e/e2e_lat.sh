#!/bin/sh
#------------------------------------------------------------------------
#   E2E 엔드투엔드 레이턴시 조인 (주입→KRX송신)
#   mock_oms.lat 의 IN + pb_1101_ts.lat 의 OUT 을 주문번호로 짝지어
#   주입(OMS)→KRX송신(pb_1101 Device_Write) 구간을 측정한다.
#   이 구간에 파일큐 vs DSHM 홉 차이가 포함된다.
#
#   Usage: sh e2e_lat.sh <result_dir> <out_label>
#------------------------------------------------------------------------
RES=$1
OUT=$2
BIN=${_FEP_HOME:-$HOME/common/fep}/st01/bin

# 두 로그의 proc 필드를 공통 토큰으로 치환 → lat_report가 (proc,key)로 짝지음
#   mock_oms IN:  <usec>|e2e|IN|<ordno>   → <usec>|e2e_path|IN|<ordno>
#   pb_1101 OUT:  <usec>|pb_1101_ts|OUT|<ordno> → <usec>|e2e_path|OUT|<ordno>
{
    awk -F'|' '$3=="IN"  {printf "%s|e2e_path|IN|%s\n",  $1,$4}' "$RES/mock_oms.lat"
    awk -F'|' '$3=="OUT" {printf "%s|e2e_path|OUT|%s\n", $1,$4}' "$RES/pb_1101_ts.lat"
} > "/tmp/$OUT.lat"

echo "=== $OUT : 주입->KRX송신 엔드투엔드 (us) ==="
"$BIN/lat_report" "/tmp/$OUT.lat" 2>/dev/null
