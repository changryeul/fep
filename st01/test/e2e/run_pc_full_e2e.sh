#!/bin/sh
#------------------------------------------------------------------------
#   E2E: VX-6b — 파생 주문 완전 통합 왕복 (실 송신부 + 실 수신부 + 1 mock)
#   File: st01/test/e2e/run_pc_full_e2e.sh   (winway@common/fep)
#
#   mock_oms(:41001) → pb_1301_tr → 큐 → pc_1100_ts(=pb_1101_ts) ─TCHODR10001→
#     mock_krx_server(:37221) ─(주문 북킹 + 교차 push 응답/체결)→
#       pc_1200_tr(=pb_1201_tr, 수신세션 SCHOPQ) → SEAM q0(응답)+q1(체결)
#   두 실 FEP 프로세스가 하나의 mock 포트(37221)에 접속(주문소켓+수신소켓) →
#   주문↔체결 이중소켓 왕복이 실 송신·수신부를 모두 통과. PB 'b' 섹터 재사용.
#   Usage: sh run_pc_full_e2e.sh [주문수(기본1)]
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; MOCK_KRX=$INTEG/bin/mock_krx_server; RESULT=$E2E/result
PORT=37221; OMS=41001; COUNT=${1:-1}; PIDS=""; IPC_BASE=/tmp/pcfull_ipcbase.$$
log(){ printf '[PCFULL] %s\n' "$1"; }
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2~/^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
wipe_ipc(){ [ -d "$IPC_BASE" ] || return 0; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2~/^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    for p in $PIDS; do kill "$p" 2>/dev/null; done; sleep 1
    for x in pb_1101_ts pc_1100_ts pb_1201_tr pc_1200_tr pb_1301_tr mock_oms mock_krx_server pz_memory_mp; do pkill -9 -x $x 2>/dev/null; done
    sleep 1; wipe_ipc
    [ -d "$CFG.pcfull_backup" ] && { rm -rf "$CFG"; mv "$CFG.pcfull_backup" "$CFG"; log "cfg restored"; }
    rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[PCFULL][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pcfull_*.log 2>/dev/null; rm -f /tmp/mock_krx_${PORT}.log 2>/dev/null
if ps -ef|grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
ipcs -m 2>/dev/null|awk '$1~/^0x4[12]/{print $2}'|xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null|awk '$1~/^0x4[12]/{print $2}'|xargs -r -n1 ipcrm -s 2>/dev/null
ipc_snapshot; wipe_ipc

# --- build mock_krx(교차push) + mock_oms + pc 바이너리 확인 ---
log "build mock_krx_server + mock_oms ..."
mkdir -p $INTEG/bin
cc -I$ST01/inc -I$INTEG/lib -I$ST01/test/vexch -o $MOCK_KRX \
   $INTEG/mock/mock_krx_server.c $INTEG/lib/krx_protocol.c $ST01/test/vexch/vexch_catalog.c 2>&1 | grep -iE error && fail "mock_krx build"
[ -f "$E2E/mock_oms.c" ] && { cc -I$ST01/inc -I$INTEG/lib -o $E2E/mock_oms $E2E/mock_oms.c $INTEG/lib/krx_protocol.c 2>&1 | grep -iE error && fail "mock_oms build"; }
[ -x "$E2E/mock_oms" ] || fail "mock_oms 없음"
[ -x "$BIN/pc_1100_ts" ] && [ -x "$BIN/pc_1200_tr" ] || { (cd $ST01/shl && sh mk.sh pc >/dev/null 2>&1); }
[ -x "$BIN/pc_1100_ts" ] && [ -x "$BIN/pc_1200_tr" ] || fail "pc_1100_ts/pc_1200_tr 없음 (mk.sh pc)"

# --- cfg (Proc_3 pc송신 + Proc_4 oms수신 + Proc_6 pc수신 + Proc_17) ---
[ -d "$CFG.pcfull_backup" ] && fail "cfg.pcfull_backup 잔존"
cp -a "$CFG" "$CFG.pcfull_backup"
for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"; done
# dshm.ini 는 저장소 본(PB DSHM 1건) 유지 → daemon.ini Daemon_B_Dshm_Count 를 1 로 정합(불일치시 pz b FATAL)
sed -i 's/^Daemon_B_Dshm_Count=0/Daemon_B_Dshm_Count=1/' "$CFG/daemon.ini"
sed -i -e 's/^Proc_3_Status=S/Proc_3_Status=R/' -e 's/^Proc_3_Start_Time=.*/Proc_3_Start_Time=0000/' -e 's/^Proc_3_End_Time=.*/Proc_3_End_Time=2359/' \
       -e 's/^Proc_4_Status=S/Proc_4_Status=R/' -e 's/^Proc_4_Start_Time=.*/Proc_4_Start_Time=0000/' -e 's/^Proc_4_End_Time=.*/Proc_4_End_Time=2359/' \
       -e 's/^Proc_6_Status=S/Proc_6_Status=R/' -e 's/^Proc_6_Start_Time=.*/Proc_6_Start_Time=0000/' -e 's/^Proc_6_End_Time=.*/Proc_6_End_Time=2359/' \
       -e 's/^Proc_17_Status=S/Proc_17_Status=R/' "$CFG/proc.ini"
# tcp2 IP 만 localhost 화(포트 유지): oms 41001 / order krx 37221 / recv krx 57221
# → mock 이 37221·57221 둘 다 listen(단일 프로세스)이라 tcp2 포트 원본 유지(proc↔tcp2 링크 보존)
python3 - "$CFG/tcp2.ini" <<'EOF'
import re, sys
path = sys.argv[1]; txt = open(path, encoding="utf-8", errors="replace").read().split("\n")
for i, line in enumerate(txt):
    m = re.match(r"^Tcp2_(\d+)_Port=(41001|37221|57221)$", line.strip())
    if m:
        n = m.group(1)
        for j in range(len(txt)):
            if re.match(rf"^Tcp2_{n}_Ip=", txt[j].strip()): txt[j] = f"Tcp2_{n}_Ip=127.0.0.1"
open(path, "w", encoding="utf-8").write("\n".join(txt))
EOF
log "cfg prepared (Proc_3/4/6/17, 41001/37221/57221 IP→localhost)"

for s in PA PB PC PF PO PW PX PY PZ; do mkdir -p $FEP/st02/FIFO/$s; done   # 전 섹터 FIFO 디렉토리(mkfifo FATAL 방지)
mkdir -p $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ/BATCH $FEP/st03/SEQ/DISC $FEP/st03/SEQ/TOTAL
rm -f $FEP/st03/LOG/PB/*/pb_1101_ts* $FEP/st03/LOG/PB/*/pb_1201_tr* 2>/dev/null
rm -f $FEP/st02/DAT/PB/00000000/* $FEP/st03/SEQ/BATCH/* $FEP/st03/SEQ/DISC/* $FEP/st03/SEQ/TOTAL/* 2>/dev/null

# --- 기동 ---
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pcfull_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/pcfull_pz_b.log" 2>&1; [ $? -eq 0 ] || fail "pz b"
log "mock_krx_server :$PORT+57221 (주문+수신 이중포트, 주문 트리거 push만) ..."
VX_NO_SCHOPQ_PUSH=1 VX_CATALOG=$CFG/vexch.ini "$MOCK_KRX" $PORT 57221 > "$RESULT/pcfull_mock.log" 2>&1 & PIDS="$PIDS $!"; sleep 1
log "pc_1200_tr (수신세션 SCHOPQ→g_push_fd) — 주문 전에 먼저 ..."
( cd $BIN && exec bash -c "exec -a pb_1201_tr $BIN/pc_1200_tr" ) > "$RESULT/pcfull_pc1201.log" 2>&1 & PIDS="$PIDS $!"; sleep 3
log "pc_1100_ts (주문 송신) ..."
( cd $BIN && exec bash -c "exec -a pb_1101_ts $BIN/pc_1100_ts" ) > "$RESULT/pcfull_pc1101.log" 2>&1 & PIDS="$PIDS $!"; sleep 2
log "mock_oms :$OMS ($COUNT 파생주문 TCHODR10001) + pb_1301_tr ..."
( cd "$RESULT" && MOCK_OMS_KIND=deriv exec "$E2E/mock_oms" $OMS "$COUNT" 300 ) > "$RESULT/pcfull_oms.log" 2>&1 & PIDS="$PIDS $!"; sleep 1
( cd $BIN && exec bash -c "exec -a pb_1301_tr $BIN/pb_1301_tr" ) > "$RESULT/pcfull_pc1301.log" 2>&1 & PIDS="$PIDS $!"
sleep 8

# --- 검증 ---
KRXSENT=$(grep -c 'ORDER recv.*booked' /tmp/mock_krx_${PORT}.log 2>/dev/null); KRXSENT=${KRXSENT:-0}
XPUSH=$(grep -c 'push EXEC.*CORRELATED' /tmp/mock_krx_${PORT}.log 2>/dev/null); XPUSH=${XPUSH:-0}
PS=$(ls -t $FEP/st03/LOG/PB/*/pb_1101_ts* 2>/dev/null|head -1); TXSND=$(grep -c 'TCP SD' "$PS" 2>/dev/null); TXSND=${TXSND:-0}
PR=$(ls -t $FEP/st03/LOG/PB/*/pb_1201_tr* 2>/dev/null|head -1); WROTE=$(grep -c 'SEAM write\[' "$PR" 2>/dev/null); WROTE=${WROTE:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  VX-6b: 파생 주문 완전 통합 왕복"
echo "========================================"
echo "  pc_1100_ts TCP 송신(TCP SD) : $TXSND"
echo "  mock 주문 수신(ORDER recv)   : $KRXSENT"
echo "  mock 교차push(EXEC CORREL)   : $XPUSH"
echo "  pc_1200_tr SEAM write(응답+체결): $WROTE"
echo "  core dump                    : $CORES"
echo "  판정 : $([ "${TXSND:-0}" -ge 1 ] && [ "${KRXSENT:-0}" -ge 1 ] && [ "${WROTE:-0}" -ge 2 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (실 송신부→거래소→실 수신부 왕복)' || echo 'FAIL/PENDING')"
echo "========================================"
echo "--- mock_krx (마지막 8줄) ---"; tail -8 /tmp/mock_krx_${PORT}.log 2>/dev/null
echo "--- pc_1200_tr (마지막 5줄) ---"; tail -5 "$PR" 2>/dev/null
echo "--- pc_1100_ts (마지막 4줄) ---"; tail -4 "$PS" 2>/dev/null
cleanup; log "done"
