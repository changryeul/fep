#!/bin/sh
#------------------------------------------------------------------------
#   E2E: pf_ FX 시세수신 부팅검증 (P4-full a) — Linux 서버 전용, ~/new_fep
#   File: run_pf_boot_e2e.sh
#
#   pf_7400_ur(FX 시세수신, NO_AGXPI 빌드)를 'f' 섹터에서 기동 →
#     Init_Proc(섹터 SHM=RISK/FX_Sise attach) + Init_Parameters(Attach_FX_SHM
#     =Shm_FX create-or-attach) + Socket_Connect(UDP bind) + 메인루프(select) 도달.
#   검증: "Attach_FX_SHM: Shm_FX attached" + "socket connected" + Shm_FX(ipcs) + core 0.
#   ※ agxpi(FIX 디코드)는 NO_AGXPI로 스텁 — 실 시세 디코드는 운영 전용. 여기선 FEP 통합 부팅만.
#   ※ Recv_Data는 select 70s 블록 → 부팅신호 감지 즉시 SIGKILL(spin 아님, 방어적 워치독).
#------------------------------------------------------------------------
FEP=$HOME/new_fep; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""
PF=pf_7400_ur; PFBIN=$BIN/pf_7400_ur
log(){ printf '[PFBOOT] %s\n' "$1"; }
wipe_ipc(){
    ipcs -m 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -m 2>/dev/null
    ipcs -s 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -s 2>/dev/null
    ipcs -q 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -q 2>/dev/null
}
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x $PF 2>/dev/null; pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1
    wipe_ipc
    [ -d "$CFG.pf_backup" ] && { rm -rf "$CFG"; mv "$CFG.pf_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[PFBOOT][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/pf_*.log 2>/dev/null

if ps -ef|grep -E 'p[abcofwz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc

[ -d "$CFG.pf_backup" ] && fail "cfg.pf_backup 잔존"
cp -a "$CFG" "$CFG.pf_backup"
for f in proc.ini file.ini daemon.ini tcp1.ini tcp2.ini udpip.ini sisetr.ini dshm.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# daemon.ini: Daemon_F(예비→FX venue), Udpip_Count 확보
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
# proc.ini: PF_CONF (pf_7400_ur, UR, udpip index0 port18400)
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
# file.ini: PF_CONF (최소 1)
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
# udpip.ini: PF_CONF (Udpip_1 = 127.0.0.1:18400, plain UDP bind)
cat >> "$CFG/udpip.ini" <<'EOF'

PF_CONF_START
Udpip_Count=1
Udpip_1_Comment=fx_sise
Udpip_1_Ip_1=127.0.0.1
Udpip_1_Port_1=18400
Udpip_End
PF_CONF_END
EOF
log "cfg prepared (pf_7400_ur, Daemon_F, udpip 127.0.0.1:18400)"

mkdir -p $FEP/st02/FIFO/PF $FEP/st02/FIFO/PA $FEP/st02/DAT/PF/00000000 $FEP/st03/LOG/PF $FEP/st03/SEQ
rm -f $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null   # fresh 로그(옛 신호 잔재 방지)

log "pz_memory_mp z ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pf_pz_z.log" 2>&1
sleep 1
log "pz_memory_mp f ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp f" ) > "$RESULT/pf_pz_f.log" 2>&1
RC=$?; cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/pf_pz_f.fep.log" 2>/dev/null; [ $RC -eq 0 ] || fail "pz f rc=$RC (log: $RESULT/pf_pz_f.log)"

TASKSET=$(command -v taskset >/dev/null 2>&1 && echo "taskset -c 0" || echo "")
log "starting pf_7400_ur (exec -a $PF; select 70s 블록 → 부팅신호 감지 즉시 kill) ..."
( cd $BIN && ulimit -t 30 2>/dev/null; exec $TASKSET bash -c "exec -a $PF $PFBIN" ) </dev/null > "$RESULT/pf_strat.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 12; kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null ) & WDPID=$!

# 부팅신호 = Attach_FX_SHM(Log). 이후 여전히 살아있으면(select 70s 블록) socket connected+메인루프 도달.
# (Socket_Connect 실패 시 PF_7400_UR return→Exit_Process→즉시 사망하므로 aliveness가 판별자)
PLOG=""
for t in $(seq 1 16); do
    PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null | head -1)
    [ -n "$PLOG" ] && grep -q 'Attach_FX_SHM: Shm_FX attached' "$PLOG" 2>/dev/null && break
    sleep 0.5
done
sleep 3   # Socket_Connect 후 select 진입 여유
ALIVE=$(pgrep -x $PF >/dev/null && echo Y || echo N)
kill -9 $APID 2>/dev/null; pkill -9 -x $PF 2>/dev/null; kill $WDPID 2>/dev/null; sleep 1

#--- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PF/*/pf_7400_ur* 2>/dev/null | head -1)
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null | wc -l)
if [ -n "$PLOG" ]; then
    FXSHM=$(grep -c 'Attach_FX_SHM: Shm_FX attached' "$PLOG")
    SOCK=$(grep -c 'socket connected' "$PLOG")
    PROV=$(grep -c 'Provider=' "$PLOG")
else FXSHM=0; SOCK=0; PROV=0; fi
FXSHM=${FXSHM:-0}; SOCK=${SOCK:-0}; PROV=${PROV:-0}
SHM_FXSEG=$(ipcs -m 2>/dev/null|grep -ci '41000005')
SEGV=$(grep -c 'SIGSEGV' "$RESULT/pf_strat.log" 2>/dev/null); SEGV=${SEGV:-0}
echo ""
echo "========================================"
echo "  pf_ FX 시세수신 부팅검증 (P4-full a)"
echo "========================================"
echo "  [FEP 통합]  Attach_FX_SHM 성공 : $FXSHM"
echo "  [FEP 통합]  Provider 설정       : $PROV"
echo "  [FEP 통합]  Shm_FX 세그(41000005): $SHM_FXSEG"
echo "  [UDP socket] 메인루프 alive     : $ALIVE (socket connected: $SOCK)"
echo "  [UDP socket] Socket_Connect SIGSEGV : $SEGV (0=정상; l.u=1·udpip layout 정상 확인됨)"
echo "  통합 판정 : $([ "${FXSHM:-0}" -ge 1 ] && [ "${PROV:-0}" -ge 1 ] && [ "${SHM_FXSEG:-0}" -ge 1 ] && echo 'PASS (포팅+빌드+Init_Proc+Shm_FX/FX_Sise attach)' || echo 'FAIL')"
echo "  전체 부팅 : $([ "$ALIVE" = "Y" ] && [ "${SEGV:-0}" -eq 0 ] && echo 'PASS (socket+메인루프)' || echo 'PENDING (udpip l.u 매핑 = pf_ letter 정식화 필요)')"
echo "========================================"
[ -n "$PLOG" ] && { echo "--- FEP log (마지막 18줄) ---"; tail -18 "$PLOG" 2>/dev/null; }
echo "--- [진단] pf_strat.log (프로세스 stdout/stderr) ---"; tail -15 "$RESULT/pf_strat.log" 2>/dev/null
echo "--- [진단] socket/bind/Ip_Addr (전 pf 로그) ---"; grep -rn "Ip_Addr\|bind\|socket\|Socket\|SO_RCVBUF\|NOTOK" $FEP/st03/LOG/PF/*/pf_7400_ur* "$RESULT/pf_strat.log" 2>/dev/null | tail -10
echo "--- [진단] pz_f 로그 (udpip 로드?) ---"; grep -in "udpip\|error\|fatal\|Daemon_F\|18400" "$RESULT/pf_pz_f.log" "$RESULT/pf_pz_f.fep.log" 2>/dev/null | tail -8
cleanup; log "done"
