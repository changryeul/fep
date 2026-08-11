#!/bin/sh
#------------------------------------------------------------------------
#   E2E: pf_ 도어벨 fanout (P4-full d, 생산자측) — Linux 서버 전용, ~/new_fep
#   File: run_pf_dbell_e2e.sh
#
#   pf_7400_ur 이 Set_Sise 후 auto_use[slot]!=0 인 슬롯의 도어벨(pf_dbell_<slot>)을
#   ring 하는지 검증. 흐름: pf_ 기동 → 심볼 주입(auto_use=0, 무링) → fx_autouse 로
#   auto_use[0]=1 세팅 → 재주입 → pf_ 가 pf_dbell_00 에 "1" write → read 로 확인.
#   (소비자측=arb Poll[3]=pf_dbell_<OD_SEQ> 는 slot-keyed 동일 이름 → 별도 arb-slot 부팅으로 확인)
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""; PF=pf_7400_ur; PFBIN=$BIN/pf_7400_ur
DBELL=$FEP/st02/FIFO/PO/pf_dbell_00
log(){ printf '[PFDBELL] %s\n' "$1"; }
wipe_ipc(){ for t in m s q; do ipcs -$t 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -$t 2>/dev/null; done; }
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x $PF 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1; wipe_ipc
    [ -d "$CFG.pfd_backup" ] && { rm -rf "$CFG"; mv "$CFG.pfd_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[PFDBELL][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pfd_*.log 2>/dev/null
if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc

log "build pf_(mk.sh pf) + fx_sise_inject + fx_autouse ..."
( cd $ST01/shl && ./mk.sh pf ) >/dev/null 2>&1
gcc -o $BIN/fx_sise_inject $ST01/utl/fx_sise_inject.c -I$ST01/inc 2>&1 | grep -iE "error" && fail "inject build"
gcc -o $BIN/fx_autouse $ST01/utl/fx_autouse.c -I$ST01/inc 2>&1 | grep -iE "error" && fail "autouse build"
[ -x "$PFBIN" ] && [ -x "$BIN/fx_sise_inject" ] && [ -x "$BIN/fx_autouse" ] || fail "build missing"

[ -d "$CFG.pfd_backup" ] && fail "cfg.pfd_backup 잔존"
cp -a "$CFG" "$CFG.pfd_backup"
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

mkdir -p $FEP/st02/FIFO/PF $FEP/st02/FIFO/PO $FEP/st02/DAT/PF/00000000 $FEP/st03/LOG/PF $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null
# 도어벨 FIFO (슬롯 0~39) 생성 — pf_ 가 open(O_RDWR), 소비자(arb/이 하니스)가 read
s=0; while [ $s -lt 40 ]; do ff=$(printf "$FEP/st02/FIFO/PO/pf_dbell_%02d" $s); [ -p "$ff" ] || mkfifo "$ff" 2>/dev/null; s=$((s+1)); done
log "pf_dbell_00..39 준비"

log "pz z / f ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pfd_pz_z.log" 2>&1; sleep 1
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp f" ) > "$RESULT/pfd_pz_f.log" 2>&1; [ $? -eq 0 ] || fail "pz f"

TASKSET=$(command -v taskset >/dev/null 2>&1 && echo "taskset -c 0" || echo "")
log "start pf_7400_ur ..."
( cd $BIN && ulimit -t 30 2>/dev/null; exec $TASKSET bash -c "exec -a $PF $PFBIN" ) </dev/null > "$RESULT/pfd_strat.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 25; kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null ) & WDPID=$!
for t in $(seq 1 12); do
    pgrep -x $PF >/dev/null || break
    PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null|head -1)
    [ -n "$PLOG" ] && grep -q 'Attach_FX_SHM: Shm_FX attached' "$PLOG" 2>/dev/null && break
    sleep 0.5
done
sleep 2
pgrep -x $PF >/dev/null || fail "pf_ not alive"

read_dbell(){ dd if="$DBELL" bs=1 count=1 iflag=nonblock 2>/dev/null; }

log "1) 심볼 주입 (auto_use=0, 무링 기대) ..."
$BIN/fx_sise_inject 1 127.0.0.1 18400 > "$RESULT/pfd_inj1.log" 2>&1; sleep 1
PRE=$(read_dbell | tr -d '\0'); PRE_LEN=$(printf '%s' "$PRE" | wc -c)

log "2) fx_autouse 0 0 1 (Shm_FX[0].auto_use[0]=1 구독) ..."
$BIN/fx_autouse 0 0 1 > "$RESULT/pfd_autouse.log" 2>&1; cat "$RESULT/pfd_autouse.log"

log "3) 심볼 재주입 (auto_use=1 → pf_dbell_00 ring 기대) ..."
$BIN/fx_sise_inject 1 127.0.0.1 18400 > "$RESULT/pfd_inj2.log" 2>&1; sleep 1
POST=$(read_dbell | tr -d '\0'); POST_LEN=$(printf '%s' "$POST" | wc -c)

kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

echo ""
echo "========================================"
echo "  pf_ 도어벨 fanout 검증 (P4-full d, 생산자)"
echo "========================================"
echo "  구독 전 read pf_dbell_00 : len=$PRE_LEN [$PRE] (0 기대=무링)"
echo "  auto_use[0] 세팅         : $(grep -c 'auto_use\[0\]=1 set' "$RESULT/pfd_autouse.log")"
echo "  구독 후 read pf_dbell_00 : len=$POST_LEN [$POST] (1 기대=ring '1')"
echo "  판정 : $([ "${PRE_LEN:-9}" -eq 0 ] && [ "${POST_LEN:-0}" -ge 1 ] && echo 'PASS (auto_use gate + slot-keyed pf_dbell ring)' || echo 'FAIL')"
echo "========================================"
cleanup; log "done"
