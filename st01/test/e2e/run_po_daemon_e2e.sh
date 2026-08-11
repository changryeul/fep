#!/bin/sh
#------------------------------------------------------------------------
#   E2E: 9000_mp full-loop Inc1 — po_daemon_mp가 po_9000_mp 기동 (데몬 경로)
#   File: run_po_daemon_e2e.sh   (Linux 서버 전용, ~/new_fep)
#
#   기존 E2E는 exec -a 로 프로세스 직접기동(데몬 우회)했으나, 9000_mp 슬롯할당은
#   데몬(DTART_FD 신호→execl)이 필수. 본 하니스는 'o' 섹터 데몬(po_daemon_mp)이
#   proc.ini(status R, 시간창)를 읽어 **po_9000_mp 를 execl 기동**하는지 검증(Inc1).
#   (Inc2=500100→슬롯 cp+DTART→슬롯 exec, Inc3=full-loop 는 후속)
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""
log(){ printf '[PODMN] %s\n' "$1"; }
wipe_ipc(){ for t in m s q; do ipcs -$t 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x po_daemon_mp 2>/dev/null; pkill -9 -x po_9000_mp 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.pod_backup" ] && { rm -rf "$CFG"; mv "$CFG.pod_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[PODMN][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pod_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc

log "build po_9000_mp(mk.sh po) ... (po_daemon_mp 는 기빌드)"
( cd $ST01/shl && ./mk.sh po ) >/dev/null 2>&1
[ -x "$BIN/po_9000_mp" ] || fail "po_9000_mp 없음"
[ -x "$BIN/po_daemon_mp" ] || fail "po_daemon_mp 없음(Make_PZ_one.sh pz_daemon o 필요)"

[ -d "$CFG.pod_backup" ] && fail "cfg.pod_backup 잔존"
cp -a "$CFG" "$CFG.pod_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
# daemon.ini: Daemon_O (ID=po_daemon_mp)
python3 - "$CFG/daemon.ini" <<'EOF'
import sys; p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n"); out=[];i=0
while i<len(t):
    if t[i].strip()=="Daemon_O_Comment=예비":
        out+=["Daemon_O_Comment=OMS_CORE","Daemon_O_ID=po_daemon_mp","Daemon_O_Start_Time=0000",
        "Daemon_O_End_Time=2359","Daemon_O_Date_Flag=1","Daemon_O_Compact_Days=4","Daemon_O_Status=1",
        "Daemon_O_FIFO=po_FIFO","Daemon_O_Shm_Log=1","Daemon_O_Proc_Count=5","Daemon_O_File_Count=5",
        "Daemon_O_Dshm_Count=0","Daemon_O_Tcp2_Count=0","Daemon_O_Udpip_Count=0","Daemon_O_Data_Count=0"]
        i+=1; continue
    out.append(t[i]); i+=1
open(p,"w",encoding="utf-8").write("\n".join(out))
EOF
# proc.ini: PO_CONF (po_9000_mp, MP, status R, IFN/OFN, 시간창 0000-2359)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=1
Proc_1_Comment=strategy_manager
Proc_1_ID=po_9000_mp
Proc_1_Status=R
Proc_1_Type=MP
Proc_1_IFN_1=po_9000_in
Proc_1_OFN_1=po_9000_out
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
PO_CONF_END
EOF
# file.ini: PO_CONF (input/output FIFO)
cat >> "$CFG/file.ini" <<'EOF'

PO_CONF_START
File_Count=2
File_1_Comment=mgr_in
File_1_Name=po_9000_in
File_1_Fifo=1
File_1_Size=2048
File_End
File_2_Comment=mgr_out
File_2_Name=po_9000_out
File_2_Fifo=1
File_2_Size=2048
File_End
PO_CONF_END
EOF
log "cfg prepared (Daemon_O + po_9000_mp)"

mkdir -p $FEP/st02/FIFO/PO $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PO/*/po_9000_mp* $FEP/st03/LOG/PO/*/po_daemon_mp* 2>/dev/null

log "pz_memory_mp z / o ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pod_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/pod_pz_o.log" 2>&1; [ $? -eq 0 ] || fail "pz o"

log "start po_daemon_mp (데몬이 proc.ini status R → po_9000_mp execl 기동) ..."
( cd $BIN && exec bash -c "exec -a po_daemon_mp $BIN/po_daemon_mp o" ) </dev/null > "$RESULT/pod_daemon.log" 2>&1 &
DPID=$!; PIDS="$PIDS $DPID"
sleep 6

#--- 검증 ---
DLOG=$(ls -t $FEP/st03/LOG/PO/*/po_daemon_mp* 2>/dev/null | head -1)
MLOG=$(ls -t $FEP/st03/LOG/PO/*/po_9000_mp* 2>/dev/null | head -1)
DAEMON_ALIVE=$(pgrep -x po_daemon_mp >/dev/null && echo Y || echo N)
MGR_ALIVE=$(pgrep -x po_9000_mp >/dev/null && echo Y || echo N)
MGR_INIT=$([ -n "$MLOG" ] && grep -c 'process initialized\|Init_Parameters' "$MLOG" 2>/dev/null || echo 0)
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null | wc -l)
echo ""
echo "========================================"
echo "  9000_mp full-loop Inc1 (데몬→po_9000_mp)"
echo "========================================"
echo "  po_daemon_mp alive     : $DAEMON_ALIVE"
echo "  po_9000_mp alive(데몬기동): $MGR_ALIVE  ← 핵심(데몬 execl)"
echo "  po_9000_mp init 로그   : $MGR_INIT"
echo "  core dump              : $CORES"
echo "  판정 : $([ "$MGR_ALIVE" = "Y" ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (데몬이 po_9000_mp execl 기동)' || echo 'FAIL')"
echo "========================================"
echo "--- daemon log ---"; [ -n "$DLOG" ] && tail -8 "$DLOG" 2>/dev/null
echo "--- po_9000_mp log ---"; [ -n "$MLOG" ] && tail -6 "$MLOG" 2>/dev/null
cleanup; log "done"
