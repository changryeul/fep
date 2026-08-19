#!/bin/sh
#------------------------------------------------------------------------
#   run_fut_exchange.sh — 선물 mock 거래소 상시 가동  [VX-5]
#   File: st01/test/vexch/run_fut_exchange.sh   (winway@common/fep)
#
#   연속 선물 시세(A306F/A301F) + 거래(주문/체결) + FEP 수신부를 가동/유지.
#     · 시세  vx_fut_pub  → wtg 227.10.20.10:60641(A306F, mci-price-krx/edge-krx)
#                         → FEP 239.1.1.1:17001(A301F, pc_7100_ur=차익거래)
#     · 거래  mock_krx_server(파생 TCHODR10001) 상시 (주문→응답/체결)
#     · FEP   pz z/c + pc_7100_ur (멀티캐스트 수신)
#   종목: 101V6000,105V3000 (KOSPI200 선물).  ⚠wtg 라이브 화면에 mock 시세 표시됨.
#
#   사용:
#     sh run_fut_exchange.sh verify         # 60초 검증(양쪽 수신 확인 후 정리)
#     sh run_fut_exchange.sh 0900 1800      # 당일 09:00~18:00 가동 (기본)
#     nohup sh run_fut_exchange.sh 0900 1800 >~/fut_exch.out 2>&1 &   # 상시 백그라운드
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; VEXCH=$ST01/test/vexch; RESULT=$E2E/result
MODE="${1:-0900}"; ENDHM="${2:-1800}"
GRP_FEP=239.1.1.1; PORT_FEP=17001; GRP_WTG=227.10.20.10; PORT_WTG=60641
ORDPORT=19200; CODES="101V6000,105V3000"
IPC_BASE=/tmp/fut_ipcbase.$$; PIDS=""
log(){ printf '[FUT] %s\n' "$1"; }
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
wipe_ipc(){ [ -d "$IPC_BASE" ] || return 0; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    log "shutting down mock 거래소..."
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x vx_fut_pub 2>/dev/null; pkill -9 -x mock_krx_server 2>/dev/null
    pkill -9 -x pc_7100_ur 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.fut_backup" ] && { rm -rf "$CFG"; mv "$CFG.fut_backup" "$CFG"; log "cfg restored"; }
    rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[FUT][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM EXIT
export _FEP_HOME=$FEP; export _FEP_DIV=TEST; export VX_TEST=1
. $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"
if ps -ef|grep -E 'pz_memory_mp|pc_7100_ur|vx_fut_pub|mock_krx_server'|grep -v grep >/dev/null; then fail "선물 mock 거래소 이미 실행중(중복 방지)"; fi
[ -x "$BIN/pc_7100_ur" ] || fail "pc_7100_ur 없음 (mk.sh pc 필요)"
ipc_snapshot; wipe_ipc

# --- build 시세/거래 도구 ---
log "build vx_fut_pub + mock_krx_server ..."
mkdir -p $INTEG/bin
cc -o $INTEG/bin/vx_fut_pub $INTEG/mock/vx_fut_pub.c 2>&1 | grep -iE error && fail "vx_fut_pub build"
cc -I$ST01/inc -I$VEXCH -o $INTEG/bin/mock_krx_server $INTEG/mock/mock_krx_server.c $INTEG/lib/krx_protocol.c $VEXCH/vexch_catalog.c 2>&1 | grep -iE error && fail "mock_krx_server build"

# --- cfg (Daemon_C + PC_CONF pc_7100_ur + udpip 239.1.1.1:17001) ---
[ -d "$CFG.fut_backup" ] && fail "cfg.fut_backup 잔존 (이전 실행 비정상 종료?)"
cp -a "$CFG" "$CFG.fut_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
python3 - "$CFG/daemon.ini" <<'EOF'
import sys; p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n"); out=[];i=0
while i<len(t):
    if t[i].strip()=="Daemon_C_Comment=예비":
        out+=["Daemon_C_Comment=DERIV","Daemon_C_ID=pc_daemon_mp","Daemon_C_Start_Time=0000",
        "Daemon_C_End_Time=2359","Daemon_C_Date_Flag=1","Daemon_C_Compact_Days=4","Daemon_C_Status=1",
        "Daemon_C_FIFO=pc_FIFO","Daemon_C_Shm_Log=0","Daemon_C_Proc_Count=5","Daemon_C_File_Count=5",
        "Daemon_C_Dshm_Count=0","Daemon_C_Tcp2_Count=0","Daemon_C_Udpip_Count=5","Daemon_C_Data_Count=0"]
        i+=1; continue
    out.append(t[i]); i+=1
open(p,"w",encoding="utf-8").write("\n".join(out))
EOF
cat >> "$CFG/proc.ini" <<'EOF'

PC_CONF_START
Proc_Count=1
Proc_1_Comment=deriv_sise_recv
Proc_1_ID=pc_7100_ur
Proc_1_Status=R
Proc_1_Type=UR
Proc_1_Udp_Port=0,17001
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
PC_CONF_END
EOF
cat >> "$CFG/file.ini" <<'EOF'

PC_CONF_START
File_Count=1
File_1_Comment=pc_in
File_1_Name=pc_7100_ur
File_1_Fifo=1
File_1_Size=2048
File_End
PC_CONF_END
EOF
cat >> "$CFG/udpip.ini" <<'EOF'

PC_CONF_START
Udpip_Count=1
Udpip_1_Comment=deriv_sise
Udpip_1_Ip_1=239.1.1.1
Udpip_1_Port_1=17001
Udpip_End
PC_CONF_END
EOF
mkdir -p $FEP/st02/FIFO/PC $FEP/st02/DAT/PC/00000000 $FEP/st03/LOG/PC $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PC/*/pc_7100_ur* 2>/dev/null
log "cfg prepared (Daemon_C + pc_7100_ur)"

# --- 시간창 대기 (window 모드) ---
if [ "$MODE" != "verify" ]; then
    STARTHM="$MODE"
    while [ "$(date +%H%M)" -lt "$STARTHM" ] 2>/dev/null; do log "대기: 현재 $(date +%H%M) < 시작 $STARTHM"; sleep 30; done
    log "가동 시작 (현재 $(date +%H%M), 종료 예정 $ENDHM)"
fi

# --- 기동: pz → pc_7100_ur → mock_krx_server(거래) → vx_fut_pub(시세) ---
log "pz z / c ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/fut_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp c" ) > "$RESULT/fut_pz_c.log" 2>&1; [ $? -eq 0 ] || fail "pz c"
log "pc_7100_ur (FEP 차익거래 수신부) ..."
( cd $BIN && exec bash -c "exec -a pc_7100_ur $BIN/pc_7100_ur" ) </dev/null > "$RESULT/fut_pc.log" 2>&1 &
PIDS="$PIDS $!"; sleep 3
pgrep -x pc_7100_ur >/dev/null || fail "pc_7100_ur 미기동"
log "mock_krx_server (파생 거래 :$ORDPORT) ..."
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_krx_server $ORDPORT > "$RESULT/fut_krx.log" 2>&1 &
PIDS="$PIDS $!"; sleep 1
WTG_OUT=$HOME/common/wtg/mci-edge-krx.out
WTG_BASE=$(grep -oE '"packets":[0-9]+' "$WTG_OUT" 2>/dev/null | tail -1 | grep -oE '[0-9]+$'); WTG_BASE=${WTG_BASE:-0}
log "vx_fut_pub (연속 시세 → wtg+FEP) ..."
VX_FUT_WTG_GRP=$GRP_WTG VX_FUT_WTG_PORT=$PORT_WTG VX_FUT_FEP_GRP=$GRP_FEP VX_FUT_FEP_PORT=$PORT_FEP \
  VX_FUT_CODES=$CODES $INTEG/bin/vx_fut_pub 500 0 > "$RESULT/fut_pub.log" 2>&1 &
PIDS="$PIDS $!"

# --- 기동 직후 자가검증 (~35s, wtg stats 주기 30s 고려) ---
sleep 35
echo ""; echo "======== VX-5 선물 mock 거래소 기동 검증 ========"
PLOG=$(ls -t $FEP/st03/LOG/PC/*/pc_7100_ur* 2>/dev/null|head -1)
FEP_RECV=$(grep -c "VX_TEST recv" "$PLOG" 2>/dev/null); FEP_RECV=${FEP_RECV:-0}
WTG_NOW=$(grep -oE '"packets":[0-9]+' "$WTG_OUT" 2>/dev/null | tail -1 | grep -oE '[0-9]+$'); WTG_NOW=${WTG_NOW:-0}
WTG_DELTA=$((WTG_NOW - WTG_BASE))
WTG_UNK=$(grep -oE '"unknown":[0-9]+' "$WTG_OUT" 2>/dev/null | tail -1 | grep -oE '[0-9]+$'); WTG_UNK=${WTG_UNK:-0}
KT=$($HOME/common/wtg/bin/krx-tester --url ws://127.0.0.1:8085/v1/subscribe --symbols 101V6000 --count 2 --timeout 10 2>&1)
KTN=$(echo "$KT" | grep -ic 'trade\|101V6000')
echo "  [FEP] pc_7100_ur 수신(A301F)     : $FEP_RECV"
echo "  [wtg] mci-edge-krx packets 증가   : $WTG_DELTA (unknown=$WTG_UNK, 0이어야 정상파싱)"
echo "  [wtg] krx-tester ws(101V6000)     : $KTN msgs (edge→web 최종)"
echo "  거래소(mock_krx_server)          : $(pgrep -x mock_krx_server >/dev/null && echo 상시가동 || echo 미가동) :$ORDPORT"
echo "  판정 : $([ "${FEP_RECV:-0}" -ge 1 ] && [ "${WTG_DELTA:-0}" -ge 1 ] && echo 'PASS (FEP+wtg 양쪽 수신 확인)' || echo 'PENDING')"
echo "================================================"

if [ "$MODE" = "verify" ]; then
    log "verify 모드 — 정리 후 종료"; exit 0        # trap cleanup
fi

# --- 상시 유지: END 시각까지 ---
log "상시 가동 중 (종료 $ENDHM). 프로세스 상태 60초마다 점검."
while [ "$(date +%H%M)" -lt "$ENDHM" ] 2>/dev/null; do
    pgrep -x vx_fut_pub >/dev/null || { log "vx_fut_pub 재기동"; VX_FUT_WTG_GRP=$GRP_WTG VX_FUT_WTG_PORT=$PORT_WTG VX_FUT_FEP_GRP=$GRP_FEP VX_FUT_FEP_PORT=$PORT_FEP VX_FUT_CODES=$CODES $INTEG/bin/vx_fut_pub 500 0 >> "$RESULT/fut_pub.log" 2>&1 & PIDS="$PIDS $!"; }
    pgrep -x mock_krx_server >/dev/null || { log "mock_krx_server 재기동"; VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_krx_server $ORDPORT >> "$RESULT/fut_krx.log" 2>&1 & PIDS="$PIDS $!"; }
    sleep 60
done
log "종료 시각 $ENDHM 도달 — 가동 종료"
# trap cleanup
