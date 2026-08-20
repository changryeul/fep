#!/bin/sh
#------------------------------------------------------------------------
#   E2E: 채권 주문 forwarder (SEAM_ORDQ_BOND → 송신부 입력) — Phase 4 P2
#   File: run_fwd_e2e.sh  (Linux 서버 전용, ~/common/fep)
#
#   스텁(seam_ord_inject, raw) ─SEAM_ORD_W─▶ [SEAM_ORDQ_BOND]
#       └─SEAM_ORD_R─▶ pb_1109_mp(forwarder) ─F_W(OFN)─▶ pb_1109_out(관측 파일큐)
#
#   검증: 주입한 채권주문(TCHODR40001)이 forwarder를 거쳐 출력 큐에 도달.
#   (운영은 OFN_1=pb_1101_ts 입력 → 송신부 무변경. 여기선 관측 파일로 대체.)
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}
ST01=$FEP/st01
E2E=$ST01/test/e2e
BIN=$ST01/bin
CFG=$ST01/cfg
RESULT=$E2E/result
PIDS=""

log()  { printf '[FWD] %s\n' "$1"; }
fail() { printf '[FWD][FAIL] %s\n' "$1"; cleanup; exit 1; }
cleanup() {
    log "cleanup..."
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x pb_1109_mp 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null
    sleep 1
    if [ -d "$CFG.fwd_backup" ]; then rm -rf "$CFG"; mv "$CFG.fwd_backup" "$CFG"; log "cfg restored"; fi
}
trap cleanup INT TERM

export _FEP_HOME=$FEP
. $ST01/env/pkg_env.sh >/dev/null 2>&1
ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/fwd_*.log 2>/dev/null

if ps -ef | grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)' | grep -v grep >/dev/null; then
    fail "FEP 프로세스 실행중. 종료 후 재시도."
fi
ipcs -m 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -s 2>/dev/null

[ -d "$CFG.fwd_backup" ] && fail "cfg.fwd_backup 잔존"
cp -a "$CFG" "$CFG.fwd_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# (1) proc.ini: Proc_Count 18→19, Proc_19=pb_1109_mp (PB_CONF_END 앞 삽입)
python3 - "$CFG/proc.ini" <<'EOF'
import sys
p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n")
out=[]
blk=["Proc_19_Comment=order_forwarder","Proc_19_ID=pb_1109_mp","Proc_19_Status=R",
     "Proc_19_Type=MP","Proc_19_IFN_1=pb_1109_mp","Proc_19_OFN_1=pb_1109_out",
     "Proc_19_Start_Time=0000","Proc_19_End_Time=2359","Proc_19_Time_Out=0","Proc_End"]
for line in t:
    if line.strip()=="Proc_Count=18": out.append("Proc_Count=19"); continue
    if line.strip()=="PB_CONF_END": out+=blk+[line]; continue
    out.append(line)
open(p,"w",encoding="utf-8").write("\n".join(out))
print("proc.ini: pb_1109_mp(Proc_19) added")
EOF

# (2) file.ini: File_Count 8→10, File_9=pb_1109_mp(입력FIFO), File_10=pb_1109_out(출력)
python3 - "$CFG/file.ini" <<'EOF'
import sys
p=sys.argv[1]; t=open(p,encoding="utf-8",errors="replace").read().split("\n")
out=[]; blk=["","File_9_Comment=fwd_in","File_9_Name=pb_1109_mp","File_9_Fifo=1","File_9_Size=400","File_End",
             "File_10_Comment=fwd_out","File_10_Name=pb_1109_out","File_10_Fifo=1","File_10_Size=400","File_End"]
ins=False
for line in t:
    if line.strip()=="File_Count=8": out.append("File_Count=10"); continue
    if line.strip()=="PB_CONF_END" and not ins: out+=blk+[line]; ins=True; continue
    out.append(line)
if not ins: out+=blk
open(p,"w",encoding="utf-8").write("\n".join(out))
print("file.ini: pb_1109_mp/out added")
EOF

# (3) daemon.ini: File_Count 8→10 (Proc_Count 20 이미 ≥19)
sed -i 's/^Daemon_B_File_Count=8/Daemon_B_File_Count=10/' "$CFG/daemon.ini"
log "cfg prepared (pb_1109_mp forwarder 등록)"

mkdir -p $FEP/st02/FIFO/PB $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ
rm -f $FEP/st02/DAT/PB/00000000/pb_1109_out* 2>/dev/null

log "pz_memory_mp z ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/fwd_pz_z.log" 2>&1
[ "$?" -eq 0 ] || fail "pz z 실패"
sleep 1
log "pz_memory_mp b ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/fwd_pz_b.log" 2>&1
RC=$?; cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/fwd_pz_b.fep.log" 2>/dev/null
[ "$RC" -eq 0 ] || fail "pz b 실패 rc=$RC (fwd_pz_b.fep.log)"

log "starting pb_1109_mp (forwarder) ..."
( cd $BIN && exec bash -c "exec -a pb_1109_mp $BIN/pb_1109_mp" ) </dev/null > "$RESULT/fwd_fwd.log" 2>&1 &
PIDS="$PIDS $!"
sleep 2
pgrep -x pb_1109_mp >/dev/null || fail "forwarder 조기종료 (fwd_fwd.log)"

log "injecting 1 TCHODR40001 → SEAM_ORDQ_BOND (raw stub) ..."
$BIN/seam_ord_inject 2>&1 | sed 's/^/  /'
sleep 2

#--- 검증 ---
OUTF=$FEP/st02/DAT/PB/00000000/pb_1109_out
PLOG=$(ls -t $FEP/st03/LOG/PB/*/pb_1109_mp* 2>/dev/null | head -1)   # forwarder Log()는 FEP 로그로 감
FWD_OUT=$([ -n "$PLOG" ] && grep -c 'forwarder OUT' "$PLOG" 2>/dev/null || echo 0)
OUT_HIT=$(grep -c 'TCHODR40001' "$OUTF" 2>/dev/null)
echo ""
echo "========================================"
echo "  Forwarder E2E Result (SEAM_ORDQ_BOND → 송신부 입력)"
echo "========================================"
echo "  forwarder OUT 로그 : $FWD_OUT 건"
echo "  출력큐 TCHODR40001 : $OUT_HIT 건 ($OUTF)"
echo "  판정               : $([ "${FWD_OUT:-0}" -ge 1 ] && [ "${OUT_HIT:-0}" -ge 1 ] && echo 'PASS (스텁→SEAM→forwarder→출력큐 도달)' || echo 'FAIL')"
echo "========================================"
tail -6 "$RESULT/fwd_fwd.log" 2>/dev/null
cleanup
log "done"
