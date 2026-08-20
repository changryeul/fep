#!/bin/sh
#------------------------------------------------------------------------
#   E2E: VX-6a — 파생 주문 왕복 (실 주문 → mock 북킹 → 교차 push → 실 pc_1200_tr)
#   File: st01/test/e2e/run_pc_order_e2e.sh   (winway@common/fep)
#
#   pc_1200_tr(=argv0 pb_1201_tr) 가 mock_krx(:57221) 에 접속·SCHOPQ 로 수신세션
#   등록(g_push_fd) → vx_probe 가 실 TCHODR10001(파생) 주문 송신 → mock 이 주문
#   북킹 후 응답/체결(TTRODP11301/TTRTDP21301)을 수신세션(pc_1200_tr)으로 교차 push
#   → pc_1200_tr 수신·SEAM 큐 기록. (주문↔체결 왕복이 실 FEP 수신부까지)
#   PB 'b' 섹터 재사용. Usage: sh run_pc_order_e2e.sh
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; MOCK_KRX=$INTEG/bin/mock_krx_server; RESULT=$E2E/result; PORT=57221; PIDS=""
IPC_BASE=/tmp/pcord_ipcbase.$$
log(){ printf '[PCORD] %s\n' "$1"; }
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2~/^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
wipe_ipc(){ [ -d "$IPC_BASE" ] || return 0; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2~/^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    for p in $PIDS; do kill "$p" 2>/dev/null; done; sleep 1
    pkill -9 -x pc_1200_tr 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; pkill -9 -x mock_krx_server 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.pcord_backup" ] && { rm -rf "$CFG"; mv "$CFG.pcord_backup" "$CFG"; log "cfg restored"; }
    rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[PCORD][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pcord_*.log 2>/dev/null; rm -f /tmp/mock_krx_${PORT}.log 2>/dev/null
if ps -ef|grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
# FEP 전용 SHM(0x41*/0x42*, pz 생성분) 잔여 제거 — 재실행 stale meg_seq/LINK seq 방지. mymq(0x00000000)/wtg 무관.
ipcs -m 2>/dev/null | awk '$1 ~ /^0x4[12]/{print $2}' | xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null | awk '$1 ~ /^0x4[12]/{print $2}' | xargs -r -n1 ipcrm -s 2>/dev/null
ipc_snapshot; wipe_ipc

# --- build mock_krx_server(교차push 포함) + vx_probe ---
log "build mock_krx_server + vx_probe ..."
mkdir -p $INTEG/bin
cc -I$ST01/inc -I$INTEG/lib -I$ST01/test/vexch -o $MOCK_KRX \
   $INTEG/mock/mock_krx_server.c $INTEG/lib/krx_protocol.c $ST01/test/vexch/vexch_catalog.c 2>&1 | grep -iE error && fail "mock build"
cc -I$ST01/inc -I$INTEG/lib -o $INTEG/bin/vx_probe $INTEG/mock/vx_probe.c $INTEG/lib/krx_protocol.c 2>&1 | grep -iE error && fail "vx_probe build"

# --- cfg (PB 'b' 섹터 재사용: Proc_6 pb_1201_tr 상시 + tcp2 57221→localhost) ---
[ -d "$CFG.pcord_backup" ] && fail "cfg.pcord_backup 잔존"
cp -a "$CFG" "$CFG.pcord_backup"
for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"; done
sed -i -e 's/^Proc_6_Status=S/Proc_6_Status=R/' -e 's/^Proc_6_Start_Time=.*/Proc_6_Start_Time=0000/' -e 's/^Proc_6_End_Time=.*/Proc_6_End_Time=2359/' "$CFG/proc.ini"
sed -i 's/^Daemon_B_Dshm_Count=0/Daemon_B_Dshm_Count=1/' "$CFG/daemon.ini"
python3 - "$CFG/tcp2.ini" "$PORT" <<'EOF'
import re, sys
path, port = sys.argv[1], sys.argv[2]
txt = open(path, encoding="utf-8", errors="replace").read().split("\n")
for i, line in enumerate(txt):
    m = re.match(rf"^Tcp2_(\d+)_Port={port}$", line.strip())
    if m:
        n = m.group(1)
        for j in range(len(txt)):
            if re.match(rf"^Tcp2_{n}_Ip=", txt[j].strip()): txt[j] = f"Tcp2_{n}_Ip=127.0.0.1"
open(path, "w", encoding="utf-8").write("\n".join(txt))
EOF
log "cfg prepared (Proc_6 pb_1201_tr, $PORT→localhost)"
mkdir -p $FEP/st02/FIFO/PB $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PB/*/pb_1201_tr* 2>/dev/null   # pc 로그 초기화(WROTE 메트릭 정확)
# 영속 시퀀스(seq_save) 리셋 — 재실행 시 INT_MEG_SEQ 잔존→LINK seq 불일치 방지
rm -f $FEP/st02/DAT/PB/00000000/* $FEP/st03/SEQ/BATCH/* $FEP/st03/SEQ/DISC/* $FEP/st03/SEQ/TOTAL/* 2>/dev/null

# --- 기동: pz z/b → mock_krx → pc_1200_tr(수신세션 SCHOPQ) ---
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pcord_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/pcord_pz_b.log" 2>&1; [ $? -eq 0 ] || fail "pz b"
log "mock_krx_server :$PORT (주문 트리거 push만) ..."; VX_NO_SCHOPQ_PUSH=1 VX_CATALOG=$CFG/vexch.ini "$MOCK_KRX" $PORT > "$RESULT/pcord_mock.log" 2>&1 & PIDS="$PIDS $!"; sleep 1
log "pc_1200_tr (수신세션, SCHOPQ→g_push_fd) ..."; ( cd $BIN && exec bash -c "exec -a pb_1201_tr $BIN/pc_1200_tr" ) > "$RESULT/pcord_pc1201.log" 2>&1 & PIDS="$PIDS $!"
sleep 10

# --- 기준선(SCHOPQ push 후) 캡처 → 실 주문 송신 → 왕복 push 확인 ---
RESP_BASE=$($BIN/seam_peek 2>/dev/null | awk -F'w_seq=' '/q=0\(RESP\)/{print $2+0}'); RESP_BASE=${RESP_BASE:-0}
log "실 파생주문 송신 (vx_probe → TCHODR10001) ..."
$INTEG/bin/vx_probe $PORT send > "$RESULT/pcord_probe.log" 2>&1
sleep 4
RESP_NOW=$($BIN/seam_peek 2>/dev/null | awk -F'w_seq=' '/q=0\(RESP\)/{print $2+0}'); RESP_NOW=${RESP_NOW:-0}
RTRIP=$((RESP_NOW - RESP_BASE))

# --- 검증 ---
BOOK=$(grep -c 'ORDER recv.*booked' /tmp/mock_krx_${PORT}.log 2>/dev/null); BOOK=${BOOK:-0}
XPUSH=$(grep -c 'push EXEC.*CORRELATED' /tmp/mock_krx_${PORT}.log 2>/dev/null); XPUSH=${XPUSH:-0}
PLOG=$(ls -t $FEP/st03/LOG/PB/*/pb_1201_tr* 2>/dev/null|head -1)
WROTE=$(grep -c 'SEAM write\[' "$PLOG" 2>/dev/null); WROTE=${WROTE:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  VX-6a: 파생 주문 왕복 → 실 pc_1200_tr 수신"
echo "========================================"
echo "  mock 주문 북킹(booked)      : $BOOK"
echo "  mock 교차push(주문→체결 왕복): $XPUSH"
echo "  pc SEAM write(응답+체결)     : $WROTE (>=2 이면 주문→응답+체결 착지)"
echo "  seam_peek RESP 증가(참고)    : $RTRIP (기준 $RESP_BASE → $RESP_NOW)"
echo "  core dump                    : $CORES"
echo "  판정 : $([ "${BOOK:-0}" -ge 1 ] && [ "${WROTE:-0}" -ge 2 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (주문→응답+체결 왕복이 실 FEP 수신부 SEAM 착지)' || echo 'FAIL/PENDING')"
echo "========================================"
echo "--- mock_krx (마지막 8줄) ---"; tail -8 /tmp/mock_krx_${PORT}.log 2>/dev/null
echo "--- pc_1200_tr (마지막 6줄) ---"; tail -6 "$PLOG" 2>/dev/null
cleanup; log "done"
