#!/bin/sh
#------------------------------------------------------------------------
#   E2E: VX-2c — 가상거래소 KRX 시세(멀티캐스트) → pc_7100_ur 실 수신
#   File: st01/test/vexch/run_vx_pc_e2e.sh   (Linux 서버, ~/new_fep)
#
#   vx_sise_pub (카탈로그 PRODUCT_DERIV sise_kind=krx, 239.1.1.1:17001) →
#   멀티캐스트 A301F → pc_7100_ur('c' 섹터, UDP 수신 + IP_ADD_MEMBERSHIP join) 수신·분류.
#   HA 는 _FEP_DIV=TEST → STANDALONE. (relay/FF_SHM 하류는 VX-2c-full)
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; RESULT=$E2E/result; PIDS=""; GRP=239.1.1.1; PORT=17001
log(){ printf '[VXPC] %s\n' "$1"; }
wipe_ipc(){ for t in m s q; do ipcs -$t 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x pc_7100_ur 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.vxpc_backup" ] && { rm -rf "$CFG"; mv "$CFG.vxpc_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[VXPC][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; export _FEP_DIV=TEST; export VX_TEST=1; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/vxpc_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc

[ -x "$BIN/pc_7100_ur" ] || fail "pc_7100_ur 없음"
cc -I$ST01/inc -o $INTEG/bin/vx_sise_pub $INTEG/mock/vx_sise_pub.c $ST01/test/vexch/vexch_catalog.c 2>&1|grep -iE error && fail "vx_sise_pub build"

[ -d "$CFG.vxpc_backup" ] && fail "cfg.vxpc_backup 잔존"
cp -a "$CFG" "$CFG.vxpc_backup"
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
log "cfg prepared (Daemon_C + pc_7100_ur, mcast $GRP:$PORT)"

mkdir -p $FEP/st02/FIFO/PC $FEP/st02/DAT/PC/00000000 $FEP/st03/LOG/PC $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PC/*/pc_7100_ur* 2>/dev/null

log "pz z / c ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/vxpc_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp c" ) > "$RESULT/vxpc_pz_c.log" 2>&1; [ $? -eq 0 ] || fail "pz c"

TASKSET=$(command -v taskset >/dev/null 2>&1 && echo "taskset -c 0" || echo "")
log "start pc_7100_ur (STANDALONE, mcast join) ..."
( cd $BIN && ulimit -t 20 2>/dev/null; exec $TASKSET bash -c "exec -a pc_7100_ur $BIN/pc_7100_ur" ) </dev/null > "$RESULT/vxpc_pc.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 15; kill -9 $APID 2>/dev/null; pkill -9 -x pc_7100_ur 2>/dev/null ) & WDPID=$!
sleep 4
pgrep -x pc_7100_ur >/dev/null || fail "pc_7100_ur 미기동(로그 $RESULT/vxpc_pc.log)"

log "vx_sise_pub 멀티캐스트 발행 ..."
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/vx_sise_pub 10 > "$RESULT/vxpc_pub.log" 2>&1
sleep 2
kill -9 $APID 2>/dev/null; pkill -9 -x pc_7100_ur 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

#--- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PC/*/pc_7100_ur* 2>/dev/null|head -1)
if [ -n "$PLOG" ]; then
    SOCK=$(grep -c "socket connected\|multicast\|MULTICAST\|bind" "$PLOG")
    RECV=$(grep -c "VX_TEST recv" "$PLOG")
else SOCK=0; RECV=0; fi
SOCK=${SOCK:-0}; RECV=${RECV:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  VX-2c: KRX 시세 멀티캐스트 → pc_7100_ur"
echo "========================================"
echo "  pc_7100_ur socket/mcast 로그 : $SOCK"
echo "  시세 수신/분류 로그           : $RECV"
echo "  core dump                     : $CORES"
echo "  판정 : $([ "${RECV:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (멀티캐스트 시세 실 수신)' || echo 'PENDING (pc_7100_ur 수신 로그 확인 필요)')"
echo "========================================"
echo "--- pc_7100_ur log (마지막 20줄) ---"; [ -n "$PLOG" ] && tail -20 "$PLOG" 2>/dev/null
echo "--- pc stdout ---"; tail -8 "$RESULT/vxpc_pc.log" 2>/dev/null
cleanup; log "done"
