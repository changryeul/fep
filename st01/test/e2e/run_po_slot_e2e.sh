#!/bin/sh
#------------------------------------------------------------------------
#   E2E: 9000_mp full-loop Inc2 — 500100 클라 → 슬롯할당 → 데몬이 전략 슬롯 exec
#   File: run_po_slot_e2e.sh   (Linux 서버 전용, ~/new_fep)
#
#   po_daemon_mp → po_9000_mp 기동(Inc1) 위에, oms_client_inject(po_9009_mp)가
#   500100(ApType=5050 채권LP)을 po_9000_in 에 F_W → po_9000_mp Start_Client 가
#   빈 슬롯 po_50101mp 찾아 `cp pb_5050_mp po_50101mp` + DTART → 데몬이 po_50101mp
#   execl → 500110 응답. blp_shm_stub 로 BLP SHM 대역(슬롯 전략 부팅용).
#   검증: 9000_mp cp 로그 + 데몬 START[po_50101mp] + po_50101mp alive.
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""; BLP_KEY=0xbb001001
log(){ printf '[POSLOT] %s\n' "$1"; }
wipe_ipc(){ for t in m s q; do ipcs -$t 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    for x in po_daemon_mp po_9000_mp po_9009_mp po_50101mp pb_5050_mp pz_memory_mp; do pkill -9 -x $x 2>/dev/null; done
    sleep 1; wipe_ipc; rm -f $BIN/po_50101mp 2>/dev/null
    [ -d "$CFG.pos_backup" ] && { rm -rf "$CFG"; mv "$CFG.pos_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[POSLOT][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pos_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc; ipcrm -M $BLP_KEY 2>/dev/null; ipcrm -S $BLP_KEY 2>/dev/null

log "build po_9000_mp + oms_client_inject; 확인 pb_5050_mp/po_daemon_mp/blp_shm_stub ..."
( cd $ST01/shl && ./mk.sh po ) >/dev/null 2>&1
gcc -o $BIN/oms_client_inject $ST01/utl/oms_client_inject.c -I$ST01/inc -L$ST01/lib -lfepP -ltirpc -lm 2>&1 | grep -iE "error|undefined" && fail "inject build"
for b in po_9000_mp po_daemon_mp oms_client_inject pb_5050_mp blp_shm_stub; do [ -x "$BIN/$b" ] || fail "$b 없음"; done

[ -d "$CFG.pos_backup" ] && fail "cfg.pos_backup 잔존"
cp -a "$CFG" "$CFG.pos_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
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
# proc.ini: po_9000_mp(R) + 슬롯 po_50101mp(S) + 주입기 po_9009_mp(S)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=3
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
Proc_2_Comment=strategy_slot_1
Proc_2_ID=po_50101mp
Proc_2_Status=S
Proc_2_Type=MP
Proc_2_IFN_1=po_50101_in
Proc_2_Start_Time=0000
Proc_2_End_Time=2359
Proc_2_Time_Out=0
Proc_End
Proc_3_Comment=client_injector
Proc_3_ID=po_9009_mp
Proc_3_Status=S
Proc_3_Type=MP
Proc_3_OFN_1=po_9000_in
Proc_3_Start_Time=0000
Proc_3_End_Time=2359
Proc_3_Time_Out=0
Proc_End
PO_CONF_END
EOF
cat >> "$CFG/file.ini" <<'EOF'

PO_CONF_START
File_Count=3
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
File_3_Comment=slot1_in
File_3_Name=po_50101_in
File_3_Fifo=1
File_3_Size=2048
File_End
PO_CONF_END
EOF
log "cfg prepared (po_9000_mp + slot po_50101mp + injector po_9009_mp)"

mkdir -p $FEP/st02/FIFO/PO $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PO/*/po_9000_mp* $FEP/st03/LOG/PO/*/po_daemon_mp* $FEP/st03/LOG/PO/*/po_50101mp* 2>/dev/null

log "pz z / o ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pos_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/pos_pz_o.log" 2>&1; [ $? -eq 0 ] || fail "pz o"

log "blp_shm_stub (BLP SHM 대역, 슬롯 전략 부팅용) ..."
$BIN/blp_shm_stub 2>&1 | sed 's/^/  /'; [ "$(ipcs -m|grep -c bb001001)" -ge 1 ] || log "warn: BLP SHM 미생성(슬롯 전략이 Blp_Open서 대기할 수 있음)"

log "start po_daemon_mp → po_9000_mp 기동 ..."
( cd $BIN && exec bash -c "exec -a po_daemon_mp $BIN/po_daemon_mp o" ) </dev/null > "$RESULT/pos_daemon.log" 2>&1 &
PIDS="$PIDS $!"; sleep 5
pgrep -x po_9000_mp >/dev/null || fail "po_9000_mp 미기동(Inc1 회귀)"
log "po_9000_mp 기동 확인"

log "inject 500100 (ApType=5050, exec -a po_9009_mp oms_client_inject) ..."
( cd $BIN && OMS_APTYPE=5050 exec bash -c "exec -a po_9009_mp $BIN/oms_client_inject" ) </dev/null > "$RESULT/pos_inject.log" 2>&1
sleep 5   # 9000_mp Start_Client(cp+DTART) + 데몬 relaunch 대기

#--- 검증 ---
MLOG=$(ls -t $FEP/st03/LOG/PO/*/po_9000_mp* 2>/dev/null|head -1)
DLOG=$(ls -t $FEP/st03/LOG/PO/*/po_daemon_mp* 2>/dev/null|head -1)
if [ -n "$MLOG" ]; then CP_LOG=$(grep -c 'cp \[' "$MLOG"); else CP_LOG=0; fi; CP_LOG=${CP_LOG:-0}
SLOT_BIN=$([ -x "$BIN/po_50101mp" ] && echo Y || echo N)
if [ -n "$DLOG" ]; then DAEMON_START=$(grep -c 'START \[po_50101mp' "$DLOG"); else DAEMON_START=0; fi; DAEMON_START=${DAEMON_START:-0}
RESP110=$([ -n "$MLOG" ] && grep -c 'file write\[po_9000_out' "$MLOG" || echo 0); RESP110=${RESP110:-0}
SLOT_ALIVE=$(pgrep -x po_50101mp >/dev/null && echo Y || echo N)
SLOG=$(ls -t $FEP/st03/LOG/PO/*/po_50101mp* 2>/dev/null|head -1)
if [ -n "$SLOG" ]; then
    S_BLP=$(grep -c 'Blp_Open success' "$SLOG"); S_SEAM=$(grep -c 'SEAM_ORD_Init' "$SLOG"); S_INIT=$(grep -c 'Init_Parameters ... END\|Init_Parameters.*END' "$SLOG")
else S_BLP=0; S_SEAM=0; S_INIT=0; fi
S_BLP=${S_BLP:-0}; S_SEAM=${S_SEAM:-0}; S_INIT=${S_INIT:-0}
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null|wc -l)
echo ""
echo "========================================"
echo "  9000_mp full-loop Inc2 (500100→슬롯 기동)"
echo "========================================"
echo "  9000_mp cp 슬롯할당 로그  : $CP_LOG"
echo "  슬롯 바이너리 po_50101mp  : $SLOT_BIN (cp됨)"
echo "  데몬 START[po_50101mp]    : $DAEMON_START"
echo "  500110 응답(po_9000_out)  : $RESP110"
echo "  슬롯 프로세스 alive       : $SLOT_ALIVE"
echo "  [Inc3] 전략초기화 BLP/SEAM/END : $S_BLP / $S_SEAM / $S_INIT (=이벤트 소비준비)"
echo "  core dump                 : $CORES"
echo "  판정 : $([ "${CP_LOG:-0}" -ge 1 ] && [ "$SLOT_BIN" = "Y" ] && [ "${DAEMON_START:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && echo 'PASS (500100→슬롯할당→데몬 execl)' || echo 'FAIL')"
echo "========================================"
echo "--- 9000_mp log ---"; [ -n "$MLOG" ] && grep -E 'cp \[|500110|500120|Start_Client|slot|Search|RD \[' "$MLOG" 2>/dev/null | tail -6
echo "--- daemon log ---"; [ -n "$DLOG" ] && grep -E 'START|DTART|po_50101' "$DLOG" 2>/dev/null | tail -6
echo "--- [진단] FIFO/PO po_9000 ---"; ls -la $FEP/st02/FIFO/PO/ 2>/dev/null | grep -E "po_9000|po_50101"
echo "--- [진단] DAT/PO po_9000 (레코드 기록?) ---"; ls -la $FEP/st02/DAT/PO/*/po_9000* 2>/dev/null; find $FEP/st02/DAT/PO -name "po_9000*" 2>/dev/null
echo "--- [진단] 9000_mp 마지막 3줄 (Poll/RD?) ---"; [ -n "$MLOG" ] && tail -3 "$MLOG" 2>/dev/null
cleanup; log "done"
