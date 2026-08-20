#!/bin/sh
#------------------------------------------------------------------------
#   E2E: VX-4f — 실 연동 안전검증: pf_1100_ts(msgq 모드) ↔ 가상 win FEP 브리지
#   File: st01/test/vexch/run_vx_fx_msgq.sh
#
#   실 win 연동과 동일한 전송수단(SysV 메시지큐)·실 큐키(msgqueue.cfg)·SMB_ST
#   포맷으로 FEP 주문 경로를 검증. mock_fx_qbridge(=win mon/fep 대역)가 주문큐
#   msgrcv → 체결큐 msgsnd. 라이브 win 은 미기동/미접촉(현재 msgq 0개, 충돌 없음).
#   생성한 큐는 스냅샷-diff 로 정리(타 시스템 IPC 절대 미삭제).
#
#   실 큐키(msgqueue.cfg): smb_ord 0x90001110 / ebs_ord 0x90001330 / exe 0x90002110
#     mtype: SMB=100, EBS=300 (체결큐 공유 데뮉스)
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; RESULT=$E2E/result; PIDS=""; ORDFIFO=/tmp/vxfxq_ord.fifo
CLORD_S=FEPFXS0000001; CLORD_E=FEPFXE0000001; CXL_E=FEPFXECXL0001
MSGQ_PF="S:0x90001110:0x90002110:100,E:0x90001330:0x90002110:300"
MSGQ_QB="S:0x90001110:0x90002110:100:full,E:0x90001330:0x90002110:300:ack"
IPC_BASE=/tmp/vxfxq_ipcbase.$$
log(){ printf '[VXFXQ] %s\n' "$1"; }
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
wipe_ipc(){ [ -d "$IPC_BASE" ] || return 0; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x pf_1100_ts 2>/dev/null; pkill -9 -x mock_fx_qbridge 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.vxfxq_backup" ] && { rm -rf "$CFG"; mv "$CFG.vxfxq_backup" "$CFG"; log "cfg restored"; }
    rm -f "$ORDFIFO" 2>/dev/null; rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[VXFXQ][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; export _FEP_DIV=TEST; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/vxfxq_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
ipc_snapshot; wipe_ipc

# --- build: pf_1100_ts (msgq 모드 포함) + mock_fx_qbridge ---
log "build pf_1100_ts + mock_fx_qbridge ..."
mkdir -p $ST01/obj/PF $INTEG/bin
gcc -o $ST01/obj/PF/pf_1100_ts.o -c $ST01/src/PF/pf_1100_ts.c -O -I$ST01/inc \
    -DNO_INISAFE -DNO_AGXPI -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE 2>&1 | grep -iE "error" && fail "pf_1100_ts compile"
gcc -o $BIN/pf_1100_ts $ST01/obj/PF/pf_1100_ts.o $ST01/lib/libfepP.a -ltirpc -lm 2>&1 | grep -iE "undefined|error" && fail "pf_1100_ts link"
cc -I$ST01/inc -o $INTEG/bin/mock_fx_qbridge $INTEG/mock/mock_fx_qbridge.c 2>&1 | grep -iE "error" && fail "qbridge build"
[ -x "$BIN/pf_1100_ts" ] && [ -x "$INTEG/bin/mock_fx_qbridge" ] || fail "build missing"

# --- cfg (Daemon_F + PF_CONF pf_1100_ts) ---
[ -d "$CFG.vxfxq_backup" ] && fail "cfg.vxfxq_backup 잔존"
cp -a "$CFG" "$CFG.vxfxq_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
python3 - "$CFG/daemon.ini" <<'EOF'
import sys; p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n"); out=[];i=0
while i<len(t):
    if t[i].strip()=="Daemon_F_Comment=예비":
        out+=["Daemon_F_Comment=FX_ORDER","Daemon_F_ID=pf_daemon_mp","Daemon_F_Start_Time=0000",
        "Daemon_F_End_Time=2359","Daemon_F_Date_Flag=1","Daemon_F_Compact_Days=4","Daemon_F_Status=1",
        "Daemon_F_FIFO=pf_FIFO","Daemon_F_Shm_Log=0","Daemon_F_Proc_Count=5","Daemon_F_File_Count=5",
        "Daemon_F_Dshm_Count=0","Daemon_F_Tcp2_Count=0","Daemon_F_Udpip_Count=5","Daemon_F_Data_Count=0"]
        i+=1; continue
    out.append(t[i]); i+=1
open(p,"w",encoding="utf-8").write("\n".join(out))
EOF
cat >> "$CFG/proc.ini" <<'EOF'

PF_CONF_START
Proc_Count=1
Proc_1_Comment=fx_order_session
Proc_1_ID=pf_1100_ts
Proc_1_Status=R
Proc_1_Type=TS2
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
PF_CONF_END
EOF
cat >> "$CFG/file.ini" <<'EOF'

PF_CONF_START
File_Count=1
File_1_Comment=fx_order_in
File_1_Name=pf_1100_ts
File_1_Fifo=1
File_1_Size=2048
File_End
PF_CONF_END
EOF
log "cfg prepared"

mkdir -p $FEP/st02/FIFO/PF $FEP/st02/DAT/PF/00000000 $FEP/st03/LOG/PF $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PF/*/pf_1100_ts* 2>/dev/null
rm -f "$ORDFIFO" 2>/dev/null; mkfifo "$ORDFIFO" 2>/dev/null

# --- pz z / f ---
log "pz z / f ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/vxfxq_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp f" ) > "$RESULT/vxfxq_pz_f.log" 2>&1; [ $? -eq 0 ] || fail "pz f"

# --- 가상 win FEP 브리지 기동 (실 큐키 생성) ---
log "start mock_fx_qbridge (real keys 0x90001110/0x90001330/0x90002110) ..."
VX_FX_QBRIDGE="$MSGQ_QB" $INTEG/bin/mock_fx_qbridge > "$RESULT/vxfxq_bridge.log" 2>&1 &
QPID=$!; PIDS="$PIDS $QPID"; sleep 1

# --- pf_1100_ts (msgq 모드) 기동 ---
log "start pf_1100_ts (transport=msgq) ..."
( cd $BIN && ulimit -t 30 2>/dev/null;
  VX_FX_TRANSPORT=msgq VX_FX_MSGQ="$MSGQ_PF" VX_FX_ORDER_FIFO=$ORDFIFO VX_TEST=1 \
  exec bash -c "exec -a pf_1100_ts $BIN/pf_1100_ts" ) </dev/null > "$RESULT/vxfxq_pf.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 20; kill -9 $APID $QPID 2>/dev/null; pkill -9 -x pf_1100_ts 2>/dev/null ) & WDPID=$!
sleep 3
pgrep -x pf_1100_ts >/dev/null || fail "pf_1100_ts 미기동(로그 $RESULT/vxfxq_pf.log)"

# --- 주문 주입: S 매수(체결) + E 신규(ack)→취소 (msgq 경유) ---
log "주문 주입(msgq): S=$CLORD_S 체결, E=$CLORD_E ack→취소"
printf 'S,FXACCT0001,USD/KRW,1,1000000,1385.50,%s\n' "$CLORD_S" > "$ORDFIFO"
printf 'E,FXACCT0001,USD/KRW,1,2000000,1385.40,%s\n' "$CLORD_E" > "$ORDFIFO"
sleep 1
printf 'CXL,E,%s,%s\n' "$CXL_E" "$CLORD_E" > "$ORDFIFO"
sleep 3
kill -9 $APID $QPID 2>/dev/null; pkill -9 -x pf_1100_ts 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

# --- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_1100_ts* 2>/dev/null|head -1)
if [ -n "$PLOG" ]; then
    MSGQ=$(grep -c "msgq ready" "$PLOG")
    SENT=$(grep -c "FX ORDER send" "$PLOG")
    FILL_S=$(grep -c "VX_TEST exec venue=S ExecType=F.*ClOrdID=$CLORD_S" "$PLOG")
    ACK_E=$(grep -c "VX_TEST exec venue=E ExecType=0.*ClOrdID=$CLORD_E" "$PLOG")
    CXL_OK=$(grep -c "VX_TEST exec venue=E ExecType=4.*OrigClOrdID=$CLORD_E" "$PLOG")
else MSGQ=0; SENT=0; FILL_S=0; ACK_E=0; CXL_OK=0; fi
MSGQ=${MSGQ:-0}; SENT=${SENT:-0}; FILL_S=${FILL_S:-0}; ACK_E=${ACK_E:-0}; CXL_OK=${CXL_OK:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  VX-4f: 실연동 안전검증 — msgq + 실 큐키 + SMB_ST"
echo "========================================"
echo "  msgq 큐 attach            : $MSGQ  (2 기대: S,E)"
echo "  주문 송신(msgsnd)         : $SENT"
echo "  S 전량체결(msgrcv)        : $FILL_S ($CLORD_S)"
echo "  E 신규 ack                : $ACK_E ($CLORD_E)"
echo "  E 취소확인(Canceled)      : $CXL_OK (OrigClOrdID=$CLORD_E)"
echo "  core dump                 : $CORES"
echo "  판정 : $([ "${MSGQ:-0}" -ge 2 ] && [ "${SENT:-0}" -ge 2 ] && [ "${FILL_S:-0}" -ge 1 ] && [ "${ACK_E:-0}" -ge 1 ] && [ "${CXL_OK:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (실 전송수단 msgq+실큐키+SMB_ST 왕복 검증)' || echo 'FAIL/PENDING')"
echo "========================================"
echo "--- pf_1100_ts log (마지막 16줄) ---"; [ -n "$PLOG" ] && tail -16 "$PLOG" 2>/dev/null
echo "--- mock_fx_qbridge log (마지막 10줄) ---"; tail -10 "$RESULT/vxfxq_bridge.log" 2>/dev/null
cleanup; log "done"
