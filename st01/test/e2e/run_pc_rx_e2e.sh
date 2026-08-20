#!/bin/sh
#------------------------------------------------------------------------
#   E2E 하니스: pc_ 파생 응답/체결 수신 검증 (Phase 2, 수신경로)
#   File   : run_pc_rx_e2e.sh   (Linux 서버 전용, ~/common/fep 기준)
#
#   파이프라인:
#     mock_krx_server(:57221, listen+push) <-connect- pc_1200_tr(=argv[0] pb_1201_tr)
#         → 회원처리호가 응답(TTRODP11301, TCHTDP00000) push
#         → pc_1200_tr Write_Data(1) → OFN_1(pb_1402_ts) 파일 기록
#
#   검증: mock가 TTRODP11301 push, pc_1200_tr가 이를 수신해 pb_1402_ts 큐에 1건 기록.
#
#   PB 하니스('b' 부문) 재사용: sender E2E(run_e2e.sh)와 동일 인프라.
#   Usage: sh run_pc_rx_e2e.sh
#------------------------------------------------------------------------

FEP=${_FEP_HOME:-$HOME/common/fep}
ST01=$FEP/st01
E2E=$ST01/test/e2e
BIN=$ST01/bin
CFG=$ST01/cfg
MOCK_KRX=$ST01/test/integ/bin/mock_krx_server
RESULT=$E2E/result
PORT=57221
PIDS=""

log()  { printf '[PCRX] %s\n' "$1"; }
fail() { printf '[PCRX][FAIL] %s\n' "$1"; cleanup; exit 1; }

cleanup() {
    log "cleanup..."
    for p in $PIDS; do kill "$p" 2>/dev/null; done
    sleep 1
    # pkill -x(정확 comm): -f는 이 토큰들을 담은 호출 셸까지 죽여 자기종료 유발.
    # pc_1200_tr은 exec -a pb_1201_tr라 comm=pc_1200_tr.
    pkill -9 -x pc_1200_tr 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null
    pkill -9 -x mock_krx_server 2>/dev/null
    if [ -d "$CFG.e2e_backup" ]; then
        rm -rf "$CFG"; mv "$CFG.e2e_backup" "$CFG"; log "cfg restored"
    fi
}
trap cleanup INT TERM

# 이중 트리: 소싱 전에 _FEP_HOME 고정 (pkg_env가 :- 로 존중)
export _FEP_HOME=$FEP
. $ST01/env/pkg_env.sh >/dev/null 2>&1
ulimit -c unlimited 2>/dev/null

mkdir -p "$RESULT"
rm -f "$RESULT"/pcrx_*.log 2>/dev/null
rm -f /tmp/mock_krx_${PORT}.log 2>/dev/null

#--- 1) 안전 확인 --------------------------------------------------------
if ps -ef | grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)' | grep -v grep >/dev/null; then
    fail "FEP 프로세스가 이미 실행 중입니다. 종료 후 재시도하세요."
fi
ipcs -m 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -s 2>/dev/null

#--- 2) cfg 하니스 구성 --------------------------------------------------
[ -d "$CFG.e2e_backup" ] && fail "cfg.e2e_backup 잔존 - 이전 실행 원복 필요"
cp -a "$CFG" "$CFG.e2e_backup"

for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# pb_1201_tr(Proc_6) 상시 기동
sed -i \
    -e 's/^Proc_6_Status=S/Proc_6_Status=R/' \
    -e 's/^Proc_6_Start_Time=.*/Proc_6_Start_Time=0000/' \
    -e 's/^Proc_6_End_Time=.*/Proc_6_End_Time=2359/' \
    "$CFG/proc.ini"

# dshm.ini의 PB DSHM(1건)과 daemon.ini Dshm_Count 정합 (run_e2e.sh와 동일)
sed -i 's/^Daemon_B_Dshm_Count=0/Daemon_B_Dshm_Count=1/' "$CFG/daemon.ini"

# 접속 대상 localhost화: pb_1201_tr(57221)->mock_krx
python3 - "$CFG/tcp2.ini" "$PORT" <<'EOF'
import re, sys
path, port = sys.argv[1], sys.argv[2]
txt = open(path, encoding="utf-8", errors="replace").read().split("\n")
for i, line in enumerate(txt):
    m = re.match(rf"^Tcp2_(\d+)_Port={port}$", line.strip())
    if m:
        n = m.group(1)
        for j in range(len(txt)):
            if re.match(rf"^Tcp2_{n}_Ip=", txt[j].strip()):
                txt[j] = f"Tcp2_{n}_Ip=127.0.0.1"
open(path, "w", encoding="utf-8").write("\n".join(txt))
EOF
log "cfg prepared (port $PORT -> localhost)"

#--- 3) 런타임 초기화 ----------------------------------------------------
mkdir -p $FEP/st02/FIFO/PB $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ
rm -f $FEP/st02/DAT/PB/00000000/pb_1402_ts* 2>/dev/null

#--- 4) 기동 -------------------------------------------------------------
log "loading SHM: pz_memory_mp z ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pcrx_pz_z.log" 2>&1
[ "$?" -eq 0 ] || fail "pz_memory_mp z 실패"
sleep 1
log "loading SHM: pz_memory_mp b ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/pcrx_pz_b.log" 2>&1
[ "$?" -eq 0 ] || fail "pz_memory_mp b 실패"
sleep 1

log "starting mock_krx_server :$PORT (push mode)..."
"$MOCK_KRX" $PORT > "$RESULT/pcrx_mock_krx.log" 2>&1 &
PIDS="$PIDS $!"
sleep 1

log "starting pc_1200_tr (as pb_1201_tr, 파생 응답/체결 수신)..."
( cd $BIN && exec bash -c "exec -a pb_1201_tr $BIN/pc_1200_tr" ) > "$RESULT/pcrx_pc_1201.log" 2>&1 &
PIDS="$PIDS $!"

#--- 5) 대기 + 검증 ------------------------------------------------------
log "waiting 12s for handshake + push..."
sleep 12

PUSHED=$(cat /tmp/mock_krx_${PORT}.log 2>/dev/null | grep -c 'Pushed DATA TTRODP11301')
# pc_1200_tr는 이제 파일큐(F_W) 대신 전역 SEAM 큐(SEAM_W)에 기록 → seam_peek로 착지 확인
SEAM_OUT=$($BIN/seam_peek 2>&1)
RESP_W=$(printf '%s\n' "$SEAM_OUT" | awk -F'w_seq=' '/q=0\(RESP\)/{print $2+0}')
[ -z "$RESP_W" ] && RESP_W=0
# FEP 로그(구조화)에서 SEAM write 확인
PLOG=$(ls -t $FEP/st03/LOG/PB/*/pb_1201_tr* 2>/dev/null | head -1)
[ -n "$PLOG" ] && WROTE=$(grep -c 'SEAM write\[' "$PLOG" 2>/dev/null) || WROTE=0

echo ""
echo "========================================"
echo "  PC-RX E2E Result (수신 → SEAM 생산자측)"
echo "========================================"
echo "  mock pushed TTRODP11301   : $PUSHED"
echo "  pc → SEAM_Q_RESP w_seq    : $RESP_W  (>=1 이면 KRX수신→SEAM 기록 OK)"
echo "  pc SEAM-write log lines   : $WROTE (feplog)"
echo "  --- seam_peek ---"
printf '%s\n' "$SEAM_OUT" | sed 's/^/  /'
echo "========================================"
tail -6 "$PLOG" 2>/dev/null
echo "--- mock_krx ---"
tail -6 /tmp/mock_krx_${PORT}.log 2>/dev/null

#--- 6) 정리 -------------------------------------------------------------
cleanup
log "done"
