#!/bin/sh
#------------------------------------------------------------------------
#   E2E: 차익거래(arb) 전략 부팅검증 (P4-3) — Linux 서버 전용, ~/common/fep
#   File: run_arb_boot_e2e.sh
#
#   pc_7070_mp(arb, po_ 슬롯) 는 LP와 달리 매칭엔진/엔진스텁이 불필요 —
#   l_arb_init_param 이 config.ini 로드 후 전략/주문번호/시세(FF·FF_N·RISK·FX)
#   SHM 을 전부 자체 IPC_CREAT 하고 브로커 주문 msgq 를 생성한다(yarb-native).
#   따라서 준비물은 (1) FEP 섹터(pz_memory_mp o) (2) yarb 절대경로 자원
#   (/fsfxwin/.../config.ini, /fslog) 뿐. 주문은 SEAM 아님(결정C=yarb 게이트웨이).
#   검증: Init_Proc + l_arb_init_param(SHM 자가생성) + 메인루프 도달(crash 0).
#------------------------------------------------------------------------
FEP=${_FEP_HOME:-$HOME/common/fep}; ST01=$FEP/st01; E2E=$ST01/test/e2e; BIN=$ST01/bin; CFG=$ST01/cfg
RESULT=$E2E/result; PIDS=""
ARB=po_5070_mp                       # exec -a 이름 → argv[0][1]='o' → D_K='o' 슬롯
ARBBIN=$BIN/pc_7070_mp
CFGSRC=$FEP/staging/yarb/conf/config.ini
log(){ printf '[ARBBOOT] %s\n' "$1"; }
wipe_ipc(){
    ipcs -m 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -m 2>/dev/null
    ipcs -s 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -s 2>/dev/null
    ipcs -q 2>/dev/null|awk '/^0x/{print $2}'|xargs -r -n1 ipcrm -q 2>/dev/null
}
cleanup(){
    log "cleanup..."; for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x $ARB 2>/dev/null; pkill -9 -x pc_7070_mp 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null; sleep 1
    wipe_ipc
    [ -d "$CFG.arb_backup" ] && { rm -rf "$CFG"; mv "$CFG.arb_backup" "$CFG"; log "cfg restored"; }
}
fail(){ printf '[ARBBOOT][FAIL] %s\n' "$1"; cleanup; exit 1; }
trap cleanup INT TERM
export _FEP_HOME=$FEP; . $ST01/env/pkg_env.sh >/dev/null 2>&1; ulimit -c unlimited 2>/dev/null
mkdir -p "$RESULT"; rm -f "$RESULT"/arb_*.log 2>/dev/null

if ps -ef|grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)'|grep -v grep >/dev/null; then fail "FEP 실행중"; fi
wipe_ipc

# yarb 절대경로 자원 (운영 /fsfxwin·/fslog) — dev 준비
sudo mkdir -p /fsfxwin/fep/yarb/conf /fslog 2>/dev/null
sudo cp "$CFGSRC" /fsfxwin/fep/yarb/conf/config.ini 2>/dev/null
sudo chmod -R 777 /fsfxwin /fslog 2>/dev/null
[ -f /fsfxwin/fep/yarb/conf/config.ini ] || fail "config.ini 배치 실패"

[ -d "$CFG.arb_backup" ] && fail "cfg.arb_backup 잔존"
cp -a "$CFG" "$CFG.arb_backup"
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
# proc.ini: PO_CONF (arb 1개)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=1
Proc_1_Comment=arb_strategy
Proc_1_ID=po_5070_mp
Proc_1_Status=R
Proc_1_Type=MP
Proc_1_IFN_1=po_5070_mp
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
File_1_Comment=arb_in
File_1_Name=po_5070_mp
File_1_Fifo=1
File_1_Size=2048
File_End
PO_CONF_END
EOF
log "cfg prepared (po_5070_mp arb 1개)"

mkdir -p $FEP/st02/FIFO/PO $FEP/st02/FIFO/PA $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO $FEP/st03/SEQ
# ⚠ 로그 초기화 필수 — po_5070_mp 로그는 날짜디렉토리에 append라, 옛 실행의
#    "OD_SEQ=" 잔재가 BOOT_OK 오탐+검증 stale 판정을 유발(거짓 PASS). fresh 시작.
rm -f $FEP/st03/LOG/PO/*/po_5070_mp* 2>/dev/null
log "pz_memory_mp z ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/arb_pz_z.log" 2>&1
sleep 1
log "pz_memory_mp o ..."; ( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/arb_pz_o.log" 2>&1
RC=$?; cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/arb_pz_o.fep.log" 2>/dev/null; [ $RC -eq 0 ] || fail "pz o rc=$RC"

# P4-3 graceful 수정: arb는 유안타 하드코딩 입력 FIFO(pa_7070_mp.c:500/520
# /PA/pa_7570_mp1·m11 = 저지연 도어벨) 부재를 비치명 처리(100ms poll 폴백) →
# 가짜 FIFO 생성 불필요. 이 하니스는 "pipe 없는 부팅"을 검증한다.
# 데몬 ctrl FIFO만 보강(init_proc 데몬 연결용).
# pa_7570 입력 FIFO는 반드시 제거 — 잔존 시 graceful 폴백 경로가 검증 안 됨(open 성공해버림).
rm -f $FEP/st02/FIFO/PA/pa_7570_mp1 $FEP/st02/FIFO/PA/pa_7570_m11 2>/dev/null
for ff in po_FIFO_ctrl po_FIFO; do
    [ -p "$FEP/st02/FIFO/PO/$ff" ] || mkfifo "$FEP/st02/FIFO/PO/$ff" 2>/dev/null
    [ -p "$FEP/st02/FIFO/PA/$ff" ] || mkfifo "$FEP/st02/FIFO/PA/$ff" 2>/dev/null
done
log "pa_7570 입력 FIFO 제거됨 (graceful 폴백 = pipe 없는 부팅 검증)"

# ⚠ arb는 부팅 후 빈 O_RDWR FIFO에서 busy-spin(2스레드) → 전 코어 CPU 포화 위험.
# 방지: (1) taskset -c 0 로 CPU0에만 핀(sshd는 타 코어) (2) ulimit -t 로 CPU시간 상한
# (3) 하드 워치독 SIGKILL (4) 부팅성공 신호 "OD_SEQ="(l_arb_init_param 완주) 감지 즉시 종료.
TASKSET=$(command -v taskset >/dev/null 2>&1 && echo "taskset -c 0" || echo "")
log "starting arb (CPU0 핀 + 워치독; 부팅 신호 감지 즉시 종료) ..."
( cd $BIN && ulimit -t 6 2>/dev/null; exec $TASKSET bash -c "exec -a $ARB $ARBBIN" ) </dev/null > "$RESULT/arb_strat.log" 2>&1 &
APID=$!; PIDS="$PIDS $APID"
( sleep 6; kill -9 $APID 2>/dev/null; pkill -9 -x $ARB 2>/dev/null; pkill -9 -x pc_7070_mp 2>/dev/null ) & WDPID=$!

BOOT_OK=0; PLOG=""
for t in $(seq 1 10); do
    PLOG=$(ls -t $FEP/st03/LOG/PO/*/po_5070_mp* 2>/dev/null | head -1)
    if [ -n "$PLOG" ] && grep -q 'OD_SEQ=' "$PLOG" 2>/dev/null; then BOOT_OK=1; break; fi
    sleep 0.5
done
# 부팅 신호 확인(또는 타임아웃) 즉시 종료 — spin 최소화
kill -9 $APID 2>/dev/null; pkill -9 -x $ARB 2>/dev/null; pkill -9 -x pc_7070_mp 2>/dev/null
kill $WDPID 2>/dev/null; sleep 1

#--- 검증 ---
PLOG=$(ls -t $FEP/st03/LOG/PO/*/po_5070_mp* 2>/dev/null | head -1)
CORES=$(ls $E2E/core.* $BIN/core.* 2>/dev/null | wc -l)
# grep -c 는 0매칭 시 stdout에 0을 찍고 exit 1 → $()에선 exit 무시되어 값만 캡처. `|| echo 0` 금지(이중값).
if [ -n "$PLOG" ]; then
    FIFOABS=$(grep -c 'fd\[-1\]' "$PLOG"); INITEND=$(grep -c 'OD_SEQ=' "$PLOG"); ARBLOG=$(grep -c 'l_arb_init_param...ARB' "$PLOG")
else FIFOABS=0; INITEND=0; ARBLOG=0; fi
FIFOABS=${FIFOABS:-0}; INITEND=${INITEND:-0}; ARBLOG=${ARBLOG:-0}
echo ""
echo "========================================"
echo "  차익거래(arb) 전략 부팅검증 (P4-3)"
echo "========================================"
echo "  l_arb_init_param 진입      : $ARBLOG"
echo "  입력 pipe 부재 확인(fd[-1]) : $FIFOABS (기대 ≥2 = Poll2/3 graceful)"
echo "  l_arb_init_param 완주(OD_SEQ) : $INITEND"
echo "  core dump                  : $CORES"
echo "  판정 : $([ "${BOOT_OK:-0}" -ge 1 ] && [ "${CORES:-0}" -eq 0 ] && [ "${FIFOABS:-0}" -ge 2 ] && [ "${INITEND:-0}" -ge 1 ] && echo 'PASS (pipe 없이 graceful 폴백 부팅 + l_arb_init_param 완주 + 메인루프 진입)' || echo 'FAIL')"
echo "========================================"
[ -n "$PLOG" ] && { echo "--- FEP log (마지막 18줄) ---"; tail -18 "$PLOG" 2>/dev/null; }
cleanup; log "done"
