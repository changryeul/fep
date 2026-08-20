#!/bin/sh
#------------------------------------------------------------------------
#   E2E: pf_ FX 시세 기록 (P4-full c) — Linux 서버 전용, ~/common/fep
#   File: run_pf_sise_e2e.sh
#
#   fx_sise_inject → UDP CO_B6FX → pf_7400_ur Recv_Data → Conv_Dispatch(NO_AGXPI
#   raw passthrough) → Set_Sise → Shm_Risk[0].FX_Sise[0][idx] + Shm_FX[idx] 기록.
#   검증: pf_ 를 -DTESTLOG 로 빌드 → "Set_Sise STORED idx[..] item[..] buy/sell" 로그로
#   주입 심볼/가격이 FX_Sise/Shm_FX 에 실제 저장됨을 확인. (arb FX leg 가 읽을 데이터)
#   ※ 실 FIX 디코드=agxpi 운영 전용; dev 는 CO_B6FX raw wire 로 대체.
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""; PF=pf_7400_ur; PFBIN=$BIN/pf_7400_ur
log(){ printf '[PFSISE] %s\n' "$1"; }
IPC_BASE=/tmp/pfs_ipcbase.$$
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
# 스냅샷 diff: 테스트 시작 이후 새로 생긴 IPC 만 제거 (공유 서버의 mymq/etcd/postgres 등 기존 IPC 보호)
wipe_ipc(){
    [ -d "$IPC_BASE" ] || return 0
    for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done
}
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x $PF 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.pfs_backup" ] && { rm -rf "$CFG"; mv "$CFG.pfs_backup" "$CFG"; log "cfg restored"; }
    rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[PFSISE][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pfs_*.log 2>/dev/null

if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
ipc_snapshot; wipe_ipc

# --- pf_7400_ur 를 TESTLOG 로 재빌드(STORED 로그) + fx_sise_inject 빌드 ---
log "build pf_7400_ur (-DTESTLOG) + fx_sise_inject ..."
gcc -o $ST01/obj/PF/pf_7400_ur.o -c $ST01/src/PF/pf_7400_ur.c -O -I$ST01/inc \
    -DNO_INISAFE -DNO_AGXPI -DTESTLOG -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE 2>&1 | grep -iE "error" && fail "pf build"
gcc -o $PFBIN $ST01/obj/PF/pf_7400_ur.o $ST01/lib/libfepP.a -ltirpc -lm 2>&1 | grep -iE "undefined|error" && fail "pf link"
gcc -o $BIN/fx_sise_inject $ST01/utl/fx_sise_inject.c -I$ST01/inc 2>&1 | grep -iE "error" && fail "inject build"
[ -x "$PFBIN" ] && [ -x "$BIN/fx_sise_inject" ] || fail "build missing"

# --- cfg (run_pf_boot 와 동일: Daemon_F + PF_CONF proc/file/udpip 127.0.0.1:18400) ---
[ -d "$CFG.pfs_backup" ] && fail "cfg.pfs_backup 잔존"
cp -a "$CFG" "$CFG.pfs_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
python3 - "$CFG/daemon.ini" <<'EOF'
import sys; p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n"); out=[];i=0
while i<len(t):
    if t[i].strip()=="Daemon_F_Comment=예비":
        out+=["Daemon_F_Comment=FX_VENUE","Daemon_F_ID=pf_daemon_mp","Daemon_F_Start_Time=0000",
        "Daemon_F_End_Time=2359","Daemon_F_Date_Flag=1","Daemon_F_Compact_Days=4","Daemon_F_Status=1",
        "Daemon_F_FIFO=pf_FIFO","Daemon_F_Shm_Log=1","Daemon_F_Proc_Count=5","Daemon_F_File_Count=5",
        "Daemon_F_Dshm_Count=0","Daemon_F_Tcp2_Count=0","Daemon_F_Udpip_Count=5","Daemon_F_Data_Count=0"]
        i+=1; continue
    out.append(t[i]); i+=1
open(p,"w",encoding="utf-8").write("\n".join(out))
EOF
cat >> "$CFG/proc.ini" <<'EOF'

PF_CONF_START
Proc_Count=1
Proc_1_Comment=fx_sise_recv
Proc_1_ID=pf_7400_ur
Proc_1_Status=R
Proc_1_Type=UR
Proc_1_Udp_Port=0,18400
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
PF_CONF_END
EOF
cat >> "$CFG/file.ini" <<'EOF'

PF_CONF_START
File_Count=1
File_1_Comment=fx_in
File_1_Name=pf_7400_ur
File_1_Fifo=1
File_1_Size=2048
File_End
PF_CONF_END
EOF
cat >> "$CFG/udpip.ini" <<'EOF'

PF_CONF_START
Udpip_Count=1
Udpip_1_Comment=fx_sise
Udpip_1_Ip_1=127.0.0.1
Udpip_1_Port_1=18400
Udpip_End
PF_CONF_END
EOF
log "cfg prepared"

mkdir -p $FEP/st02/FIFO/PF $FEP/st02/FIFO/PA $FEP/st02/DAT/PF/00000000 $FEP/st03/LOG/PF $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null

log "pz_memory_mp z / f ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pfs_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp f" ) > "$RESULT/pfs_pz_f.log" 2>&1
[ $? -eq 0 ] || fail "pz f"

TASKSET=$(command -v taskset >/dev/null 2>&1 && echo "taskset -c 0" || echo "")
log "start pf_7400_ur ..."
( cd $BIN && ulimit -t 30 2>/dev/null; exec $TASKSET bash -c "exec -a $PF $PFBIN" ) </dev/null > "$RESULT/pfs_strat.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 20; kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null ) & WDPID=$!

# socket bind(메인루프 select) 도달 대기
for t in $(seq 1 12); do
    pgrep -x $PF >/dev/null || break
    PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null | head -1)
    [ -n "$PLOG" ] && grep -q 'Attach_FX_SHM: Shm_FX attached' "$PLOG" 2>/dev/null && break
    sleep 0.5
done
sleep 2
ALIVE=$(pgrep -x $PF >/dev/null && echo Y || echo N)
[ "$ALIVE" = "Y" ] || fail "pf_ not alive before inject (socket bind 실패?)"

log "inject 3 CO_B6FX (EURUSD/USDJPY/GBPUSD) → UDP 18400 ..."
$BIN/fx_sise_inject 3 127.0.0.1 18400 > "$RESULT/pfs_inject.log" 2>&1
sleep 2
kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

#--- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null | head -1)
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null | wc -l)
if [ -n "$PLOG" ]; then
    STORED=$(grep -c 'Set_Sise STORED' "$PLOG")
    EUR=$(grep -c 'Set_Sise STORED.*EURUSD' "$PLOG")
    JPY=$(grep -c 'Set_Sise STORED.*USDJPY' "$PLOG")
    GBP=$(grep -c 'Set_Sise STORED.*GBPUSD' "$PLOG")
else STORED=0; EUR=0; JPY=0; GBP=0; fi
STORED=${STORED:-0}; EUR=${EUR:-0}; JPY=${JPY:-0}; GBP=${GBP:-0}
echo ""
echo "========================================"
echo "  pf_ FX 시세 기록 검증 (P4-full c)"
echo "========================================"
echo "  주입: $(grep -c 'sent #' "$RESULT/pfs_inject.log" 2>/dev/null) CO_B6FX"
echo "  Set_Sise STORED 총건 : $STORED"
echo "  EURUSD/USDJPY/GBPUSD : $EUR / $JPY / $GBP"
echo "  core dump            : $CORES"
echo "  판정 : $([ "${STORED:-0}" -ge 3 ] && [ "${EUR:-0}" -ge 1 ] && [ "${JPY:-0}" -ge 1 ] && [ "${GBP:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (CO_B6FX→Set_Sise→FX_Sise/Shm_FX 기록)' || echo 'FAIL')"
echo "========================================"
echo "--- Set_Sise STORED 로그 ---"; grep 'Set_Sise STORED' "$PLOG" 2>/dev/null | tail -6
cleanup; log "done"
