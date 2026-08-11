#!/bin/sh
#------------------------------------------------------------------------
#   E2E: 채권 LP 전략 부팅검증 (P3a) — pb_5050_mp가 매칭엔진 SHM에 attach·부팅
#   File: run_blp_boot_e2e.sh  (Linux 서버 전용, ~/new_fep)
#
#   blp_shm_stub(매칭엔진 대역) ─Mem_Create(BLP_KEY)─▶ [BLP SHM 0xbb001001]
#       └─ pb_5050_mp(전략, po_ 슬롯) Blp_Open→Mem_Open attach → 부팅
#   검증: Init_Parameters(Blp_Open success) + SEAM_ORD_Init OK + 메인루프 도달(crash 0).
#   (실제 LP호가 emit은 매칭엔진 필요 — P3-full. 여기선 FEP측 통합 부팅만.)
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""; BLP_KEY=0xbb001001
log(){ printf '[BLPBOOT] %s\n' "$1"; }
fail(){ printf '[BLPBOOT][FAIL] %s\n' "$1"; cleanup; exit 1; }
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x po_5050_mp 2>/dev/null; pkill -9 -x pb_5050_mp 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1
    ipcrm -M $BLP_KEY 2>/dev/null; ipcrm -S $BLP_KEY 2>/dev/null
    [ -d "$CFG.blp_backup" ] && { rm -rf "$CFG"; mv "$CFG.blp_backup" "$CFG"; log "cfg restored"; }
}
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/blp_*.log 2>/dev/null

if ps -ef|grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
ipcs -m 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -s 2>/dev/null
ipcrm -M $BLP_KEY 2>/dev/null; ipcrm -S $BLP_KEY 2>/dev/null

[ -d "$CFG.blp_backup" ] && fail "cfg.blp_backup 잔존"
cp -a "$CFG" "$CFG.blp_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# daemon.ini: Daemon_O(예비→OMS코어)
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
# proc.ini: PO_CONF (전략 1개)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=1
Proc_1_Comment=bond_lp_strategy
Proc_1_ID=po_5050_mp
Proc_1_Status=R
Proc_1_Type=MP
Proc_1_IFN_1=po_5050_mp
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
PO_CONF_END
EOF
# file.ini: PO_CONF
cat >> "$CFG/file.ini" <<'EOF'

PO_CONF_START
File_Count=1
File_1_Comment=lp_in
File_1_Name=po_5050_mp
File_1_Fifo=1
File_1_Size=2048
File_End
PO_CONF_END
EOF
log "cfg prepared (po_5050_mp 전략 1개)"

mkdir -p $FEP/st02/FIFO/PO $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO $FEP/st03/SEQ
log "pz_memory_mp z ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/blp_pz_z.log" 2>&1; [ $? -eq 0 ] || fail "pz z"
sleep 1
log "pz_memory_mp o ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/blp_pz_o.log" 2>&1
RC=$?; cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/blp_pz_o.fep.log" 2>/dev/null; [ $RC -eq 0 ] || fail "pz o rc=$RC"

log "blp_shm_stub (매칭엔진 대역: BLP SHM 생성) ..."
$BIN/blp_shm_stub 2>&1 | sed 's/^/  /'
[ "$(ipcs -m|grep -c bb001001)" -ge 1 ] || fail "BLP SHM 미생성"

log "starting po_5050_mp (전략, exec -a) ..."
( cd $BIN && exec bash -c "exec -a po_5050_mp $BIN/pb_5050_mp" ) </dev/null > "$RESULT/blp_strat.log" 2>&1 &
PIDS="$PIDS $!"
sleep 8   # 전략은 Init_Parameters 전 sleep(5)(클라 파라미터 대기) → 8초 후 확인

#--- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PO/*/po_5050_mp* 2>/dev/null | head -1)
ALIVE=$(pgrep -x pb_5050_mp >/dev/null && echo Y || echo N)
BLP_OK=$([ -n "$PLOG" ] && grep -c 'Blp_Open success' "$PLOG" 2>/dev/null || echo 0)
SEAM_OK=$([ -n "$PLOG" ] && grep -c 'SEAM_ORD_Init OK\|SEAM_ORD_Init: zero-init' "$PLOG" 2>/dev/null || echo 0)
INITEND=$([ -n "$PLOG" ] && grep -c 'Init_Parameters ... END' "$PLOG" 2>/dev/null || echo 0)
echo ""
echo "========================================"
echo "  채권 LP 전략 부팅검증 (P3a)"
echo "========================================"
echo "  Blp_Open success   : $BLP_OK"
echo "  SEAM_ORD_Init       : $SEAM_OK"
echo "  Init_Parameters END : $INITEND"
echo "  프로세스 alive      : $ALIVE (comm=pb_5050_mp)"
echo "  판정                : $([ "${BLP_OK:-0}" -ge 1 ] && [ "$ALIVE" = "Y" ] && echo 'PASS (매칭엔진 SHM attach + 부팅 + 메인루프)' || echo 'FAIL')"
echo "========================================"
[ -n "$PLOG" ] && tail -12 "$PLOG" 2>/dev/null
cleanup; log "done"
