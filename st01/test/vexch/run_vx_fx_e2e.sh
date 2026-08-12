#!/bin/sh
#------------------------------------------------------------------------
#   E2E: VX-4b — 실 FEP FX 주문 프로세스(pf_1100_ts) ↔ 가상 FX 거래원
#   File: st01/test/vexch/run_vx_fx_e2e.sh
#
#   mock_fx_venue(19100) 기동 → pf_1100_ts('f' 섹터, 거래원 TCP 접속) →
#   주문요청 FIFO 주입 → pf_1100_ts 가 SMB_ST 'D' 송신 → 거래원이 New ack +
#   체결('8') 반환 → pf_1100_ts 가 수신·기록(VX_TEST exec). ClOrdID 상관 검증.
#   HA 무관(주문 세션). cfg 백업/복원 + 공유서버 IPC 안전(스냅샷 diff).
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/new_fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
INTEG=$ST01/test/integ; VEXCH=$ST01/test/vexch; RESULT=$E2E/result
PIDS=""; VPORT_J=19100; VPORT_N=19101; VPORT_E=19102; ORDFIFO=/tmp/vxfx_ord.fifo
CLORD_J=FEPFXJ0000001; CLORD_N=FEPFXN0000001; CLORD_E=FEPFXE0000001; CXL_E=FEPFXECXL0001
IPC_BASE=/tmp/vxfx_ipcbase.$$
log(){ printf '[VXFX] %s\n' "$1"; }
ipc_snapshot(){ mkdir -p "$IPC_BASE"; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}' > "$IPC_BASE/$t" 2>/dev/null; done; }
wipe_ipc(){ [ -d "$IPC_BASE" ] || return 0; for t in m s q; do ipcs -$t 2>/dev/null|awk '$2 ~ /^[0-9]+$/{print $2}'|grep -vxF -f "$IPC_BASE/$t" 2>/dev/null|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x pf_1100_ts 2>/dev/null; pkill -9 -x mock_fx_venue 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.vxfx_backup" ] && { rm -rf "$CFG"; mv "$CFG.vxfx_backup" "$CFG"; log "cfg restored"; }
    rm -f "$ORDFIFO" 2>/dev/null; rm -rf "$IPC_BASE" 2>/dev/null
}
fail(){ printf '[VXFX][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; export _FEP_DIV=TEST; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/vxfx_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
ipc_snapshot; wipe_ipc

# --- build: pf_1100_ts (신설) + mock_fx_venue ---
log "build pf_1100_ts + mock_fx_venue ..."
mkdir -p $ST01/obj/PF $INTEG/bin
gcc -o $ST01/obj/PF/pf_1100_ts.o -c $ST01/src/PF/pf_1100_ts.c -O -I$ST01/inc \
    -DNO_INISAFE -DNO_AGXPI -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE 2>&1 | grep -iE "error" && fail "pf_1100_ts compile"
gcc -o $BIN/pf_1100_ts $ST01/obj/PF/pf_1100_ts.o $ST01/lib/libfepP.a -ltirpc -lm 2>&1 | grep -iE "undefined|error" && fail "pf_1100_ts link"
cc -I$ST01/inc -I$VEXCH -o $INTEG/bin/mock_fx_venue $INTEG/mock/mock_fx_venue.c $VEXCH/vexch_catalog.c 2>&1 | grep -iE "error" && fail "mock_fx_venue build"
[ -x "$BIN/pf_1100_ts" ] && [ -x "$INTEG/bin/mock_fx_venue" ] || fail "build missing"

# --- cfg (Daemon_F + PF_CONF proc/file, pf_1100_ts) ---
[ -d "$CFG.vxfx_backup" ] && fail "cfg.vxfx_backup 잔존"
cp -a "$CFG" "$CFG.vxfx_backup"
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
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/vxfx_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp f" ) > "$RESULT/vxfx_pz_f.log" 2>&1; [ $? -eq 0 ] || fail "pz f"

# --- 가상 FX 거래원 3개 기동 (JPM 19100 full, NH 19101 full, EBS 19102 ack) ---
log "start mock_fx_venue J:$VPORT_J N:$VPORT_N E:$VPORT_E ..."
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_fx_venue $VPORT_J > "$RESULT/vxfx_venue_J.log" 2>&1 &
MJPID=$!; PIDS="$PIDS $MJPID"
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_fx_venue $VPORT_N > "$RESULT/vxfx_venue_N.log" 2>&1 &
MNPID=$!; PIDS="$PIDS $MNPID"
VX_CATALOG=$CFG/vexch.ini $INTEG/bin/mock_fx_venue $VPORT_E > "$RESULT/vxfx_venue_E.log" 2>&1 &
MEPID=$!; PIDS="$PIDS $MEPID"; sleep 1

# --- pf_1100_ts 기동 (다거래원 접속 + 주문 FIFO 트리거) ---
log "start pf_1100_ts (venues J N E, fifo $ORDFIFO) ..."
( cd $BIN && ulimit -t 30 2>/dev/null;
  VX_FX_VENUES="J:127.0.0.1:$VPORT_J,N:127.0.0.1:$VPORT_N,E:127.0.0.1:$VPORT_E" \
  VX_FX_ORDER_FIFO=$ORDFIFO VX_TEST=1 \
  exec bash -c "exec -a pf_1100_ts $BIN/pf_1100_ts" ) </dev/null > "$RESULT/vxfx_pf.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 20; kill -9 $APID $MJPID $MNPID $MEPID 2>/dev/null; pkill -9 -x pf_1100_ts 2>/dev/null ) & WDPID=$!
sleep 3
pgrep -x pf_1100_ts >/dev/null || fail "pf_1100_ts 미기동(로그 $RESULT/vxfx_pf.log)"

# --- 주문 주입: J 매수(체결) + N 매도(체결) + E 신규(ack만) → E 취소 ---
log "주문 주입: J=$CLORD_J N=$CLORD_N E=$CLORD_E, 그리고 E 취소($CXL_E)"
printf 'J,FXACCT0001,USD/KRW,1,1000000,1385.50,%s\n' "$CLORD_J" > "$ORDFIFO"
printf 'N,FXACCT0001,USD/KRW,2,500000,1385.60,%s\n'  "$CLORD_N" > "$ORDFIFO"
printf 'E,FXACCT0001,USD/KRW,1,2000000,1385.40,%s\n' "$CLORD_E" > "$ORDFIFO"
sleep 1
printf 'CXL,E,%s,%s\n' "$CXL_E" "$CLORD_E" > "$ORDFIFO"     # E 미체결 주문 취소
sleep 3
kill -9 $APID $MJPID $MNPID $MEPID 2>/dev/null; pkill -9 -x pf_1100_ts 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

# --- 검증 (다거래원: J/N 각각 라우팅·왕복·상관) ---
PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_1100_ts* 2>/dev/null|head -1)
if [ -n "$PLOG" ]; then
    CONN=$(grep -c "venue\[.\] connected" "$PLOG")
    SENT=$(grep -c "FX ORDER send" "$PLOG")
    CXLS=$(grep -c "FX CANCEL send" "$PLOG")
    CORR_J=$(grep -c "VX_TEST exec venue=J ExecType=F.*ClOrdID=$CLORD_J" "$PLOG")
    CORR_N=$(grep -c "VX_TEST exec venue=N ExecType=F.*ClOrdID=$CLORD_N" "$PLOG")
    ACK_E=$(grep -c "VX_TEST exec venue=E ExecType=0.*ClOrdID=$CLORD_E" "$PLOG")
    CXL_OK=$(grep -c "VX_TEST exec venue=E ExecType=4.*OrigClOrdID=$CLORD_E" "$PLOG")
else CONN=0; SENT=0; CXLS=0; CORR_J=0; CORR_N=0; ACK_E=0; CXL_OK=0; fi
CONN=${CONN:-0}; SENT=${SENT:-0}; CXLS=${CXLS:-0}; CORR_J=${CORR_J:-0}; CORR_N=${CORR_N:-0}; ACK_E=${ACK_E:-0}; CXL_OK=${CXL_OK:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  VX-4d: FEP FX 다거래원 + 취소 라이프사이클"
echo "========================================"
echo "  거래원 접속(connected)   : $CONN   (3 기대: J,N,E)"
echo "  주문 송신 / 취소 송신    : $SENT / $CXLS"
echo "  J 체결(FILLED)           : $CORR_J ($CLORD_J → venue=J)"
echo "  N 체결(FILLED)           : $CORR_N ($CLORD_N → venue=N)"
echo "  E 신규 ack(미체결)       : $ACK_E ($CLORD_E → venue=E)"
echo "  E 취소확인(Canceled)     : $CXL_OK (OrigClOrdID=$CLORD_E)"
echo "  core dump                : $CORES"
echo "  판정 : $([ "${CONN:-0}" -ge 3 ] && [ "${CORR_J:-0}" -ge 1 ] && [ "${CORR_N:-0}" -ge 1 ] && [ "${ACK_E:-0}" -ge 1 ] && [ "${CXL_OK:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (다거래원 라우팅+체결+취소 라이프사이클 확인)' || echo 'FAIL/PENDING')"
echo "========================================"
echo "--- pf_1100_ts log (마지막 22줄) ---"; [ -n "$PLOG" ] && tail -22 "$PLOG" 2>/dev/null
echo "--- venue E log (ack+취소) ---"; tail -8 "$RESULT/vxfx_venue_E.log" 2>/dev/null
cleanup; log "done"
