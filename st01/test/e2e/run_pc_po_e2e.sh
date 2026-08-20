#!/bin/sh
#------------------------------------------------------------------------
#   E2E 하니스: pc_ + po_ 단일 라이브 파이프라인 (KRX→pc_→SEAM→po_→MK_PM)
#   File   : run_pc_po_e2e.sh   (Linux 서버 전용, ~/common/fep 기준)
#
#   그동안 반쪽씩(생산자 run_pc_rx / 소비자 run_po) 검증한 것을 하나의
#   살아있는 파이프라인으로 봉합한다. 'b'(pc_ 수신)와 'o'(po_ 코어)를 동시
#   기동하고 전역 SEAM 큐(0x41000015+TEST)를 공유시킨다:
#
#     mock_krx(:57221) --push--> pc_1200_tr(=pb_1201_tr) --SEAM_W-->
#         SEAM_Q_RESP(TTRODP11301) --> po_1290_mp Make_MiChe_Deriv (mk=1 등록)
#         SEAM_Q_EXEC(TTRTDP21301) --> po_1490_mp Analyze_Che_Deriv (mk=1 감소)
#
#   검증: shm_snap MK_PM에 mk=1 파생 미체결이 등록(ord=0000000002)되고
#         체결로 잔량이 0으로 감소(jan=0) → 전 구간 라이브 동작.
#
#   Usage: sh run_pc_po_e2e.sh
#------------------------------------------------------------------------

FEP=${_FEP_HOME:-$HOME/common/fep}
ST01=$FEP/st01
E2E=$ST01/test/e2e
BIN=$ST01/bin
CFG=$ST01/cfg
MOCK_KRX=$ST01/test/integ/bin/mock_krx_server
RESULT=$E2E/result
PORT=57221
PIDS=""

log()  { printf '[PCPO] %s\n' "$1"; }
fail() { printf '[PCPO][FAIL] %s\n' "$1"; cleanup; exit 1; }

cleanup() {
    log "cleanup..."
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    pkill -9 -x pc_1200_tr 2>/dev/null      # exec -a로 argv0=pb_1201_tr지만 comm=pc_1200_tr
    pkill -9 -x pb_1201_tr 2>/dev/null
    pkill -9 -x po_1290_mp 2>/dev/null
    pkill -9 -x po_1490_mp 2>/dev/null
    pkill -9 -x pz_memory_mp 2>/dev/null
    pkill -9 -x mock_krx_server 2>/dev/null
    sleep 1
    if [ -d "$CFG.e2e_backup" ]; then
        rm -rf "$CFG"; mv "$CFG.e2e_backup" "$CFG"; log "cfg restored"
    fi
}
trap cleanup INT TERM

export _FEP_HOME=$FEP
. $ST01/env/pkg_env.sh >/dev/null 2>&1
ulimit -c unlimited 2>/dev/null

mkdir -p "$RESULT"
rm -f "$RESULT"/pcpo_*.log 2>/dev/null
rm -f /tmp/mock_krx_${PORT}.log 2>/dev/null

if ps -ef | grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)|mock_krx' | grep -v grep >/dev/null; then
    fail "FEP/mock 프로세스가 이미 실행 중. 종료 후 재시도."
fi
ipcs -m 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -s 2>/dev/null

#--- cfg 구성: cfg/back 기반 + 'o'(po_) + 'b'(pc_ 수신) ------------------
[ -d "$CFG.e2e_backup" ] && fail "cfg.e2e_backup 잔존 - 원복 필요"
cp -a "$CFG" "$CFG.e2e_backup"
for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# (o-1) daemon.ini: Daemon_O(예비)를 OMS코어 config로
python3 - "$CFG/daemon.ini" <<'EOF'
import sys
p = sys.argv[1]
t = open(p, encoding="utf-8", errors="replace").read().split("\n")
out, i = [], 0
while i < len(t):
    if t[i].strip() == "Daemon_O_Comment=예비":
        out += [
            "Daemon_O_Comment=OMS_CORE","Daemon_O_ID=po_daemon_mp",
            "Daemon_O_Start_Time=0000","Daemon_O_End_Time=2359",
            "Daemon_O_Date_Flag=1","Daemon_O_Compact_Days=4",
            "Daemon_O_Status=1","Daemon_O_FIFO=po_FIFO","Daemon_O_Shm_Log=1",
            "Daemon_O_Proc_Count=10","Daemon_O_File_Count=10",
            "Daemon_O_Dshm_Count=0","Daemon_O_Tcp2_Count=0",
            "Daemon_O_Udpip_Count=0","Daemon_O_Data_Count=0",
        ]
        i += 1
        continue
    out.append(t[i]); i += 1
open(p, "w", encoding="utf-8").write("\n".join(out))
EOF

# (o-2) proc.ini: PO_CONF (po_1290_mp 미체결 + po_1490_mp 체결원장)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=5
Proc_1_Comment=miche
Proc_1_ID=po_1290_mp
Proc_1_Status=R
Proc_1_Type=MP
Proc_1_IFN_1=po_1290_mp
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
Proc_2_Comment=settle
Proc_2_ID=po_1490_mp
Proc_2_Status=R
Proc_2_Type=MP
Proc_2_IFN_1=po_1490_mp
Proc_2_Start_Time=0000
Proc_2_End_Time=2359
Proc_2_Time_Out=0
Proc_End
PO_CONF_END
EOF

# (o-3) file.ini: PO_CONF
cat >> "$CFG/file.ini" <<'EOF'

PO_CONF_START
File_Count=2
File_1_Comment=miche_in
File_1_Name=po_1290_mp
File_1_Fifo=1
File_1_Size=400
File_End
File_2_Comment=settle_in
File_2_Name=po_1490_mp
File_2_Fifo=1
File_2_Size=400
File_End
PO_CONF_END
EOF

# (b-1) proc.ini: pb_1201_tr(Proc_6=파생 수신 pc_1200_tr) 상시 기동
sed -i \
    -e 's/^Proc_6_Status=S/Proc_6_Status=R/' \
    -e 's/^Proc_6_Start_Time=.*/Proc_6_Start_Time=0000/' \
    -e 's/^Proc_6_End_Time=.*/Proc_6_End_Time=2359/' \
    "$CFG/proc.ini"

# (b-2) daemon.ini: PB DSHM 1건 정합
sed -i 's/^Daemon_B_Dshm_Count=0/Daemon_B_Dshm_Count=1/' "$CFG/daemon.ini"

# (b-3) tcp2.ini: 57221 접속대상 localhost화
python3 - "$CFG/tcp2.ini" "$PORT" <<'EOF'
import re, sys
path, port = sys.argv[1], sys.argv[2]
txt = open(path, encoding="utf-8", errors="replace").read().split("\n")
for i, line in enumerate(txt):
    m = re.match(rf"^Tcp2_(\d+)_Port={port}$", line.strip())
    if m:
        n = m.group(1)
        for j in range(len(txt)):
            if re.match(rf"^Tcp2_{n}_Ip=", txt[j].strip()):
                txt[j] = f"Tcp2_{n}_Ip=127.0.0.1"
open(path, "w", encoding="utf-8").write("\n".join(txt))
EOF

log "cfg prepared ('o' po_ + 'b' pc_수신)"

#--- 런타임 디렉토리 ------------------------------------------------------
mkdir -p $FEP/st02/FIFO/PO $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO \
         $FEP/st02/FIFO/PB $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ
rm -f $FEP/st02/DAT/PO/00000000/po_* 2>/dev/null
# pc_(pb_1201_tr) KRX 인터페이스 시퀀스(if_seq/if_meg_seq)는 _stat 파일에 영속되어
# 재실행 시 INT_SEQ가 누적됨 → fresh 세션(LINK seq=0)으로 시작하도록 제거.
rm -f $FEP/st02/DAT/PB/00000000/pb_1201_tr* 2>/dev/null

#--- SHM 생성: z(데몬) + b(파생수신) + o(OMS코어) ----------------------
log "pz_memory_mp z ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pcpo_pz_z.log" 2>&1
[ "$?" -eq 0 ] || fail "pz_memory_mp z 실패"
sleep 1
log "pz_memory_mp b ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/pcpo_pz_b.log" 2>&1
[ "$?" -eq 0 ] || fail "pz_memory_mp b 실패"
sleep 1
log "pz_memory_mp o ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/pcpo_pz_o.log" 2>&1
RC=$?
cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/pcpo_pz_o.fep.log" 2>/dev/null
[ "$RC" -eq 0 ] || fail "pz_memory_mp o 실패 rc=$RC"
log "  → 'b'/'o' 부문 생성 성공"

#--- 소비자(po_) 먼저 기동 (SEAM 대기) ---------------------------------
log "starting po_1290_mp (미체결, SEAM_Q_RESP 소비) ..."
( cd $BIN && exec bash -c "exec -a po_1290_mp $BIN/po_1290_mp" ) </dev/null > "$RESULT/pcpo_miche.log" 2>&1 &
PIDS="$PIDS $!"
log "starting po_1490_mp (체결원장, SEAM_Q_EXEC 소비) ..."
( cd $BIN && exec bash -c "exec -a po_1490_mp $BIN/po_1490_mp" ) </dev/null > "$RESULT/pcpo_settle.log" 2>&1 &
PIDS="$PIDS $!"
sleep 2

#--- 생산자: mock_krx + pc_1200_tr ------------------------------------
log "starting mock_krx_server :$PORT (push TTRODP11301 + TTRTDP21301)..."
"$MOCK_KRX" $PORT > "$RESULT/pcpo_mock_krx.log" 2>&1 &
PIDS="$PIDS $!"
sleep 1
log "starting pc_1200_tr (as pb_1201_tr, 파생 수신 → SEAM_W)..."
( cd $BIN && exec bash -c "exec -a pb_1201_tr $BIN/pc_1200_tr" ) > "$RESULT/pcpo_pc.log" 2>&1 &
PIDS="$PIDS $!"

#--- 대기 + 검증 --------------------------------------------------------
log "waiting 15s (handshake + 2 push + po_ poll/도치 소비)..."
sleep 15

PUSHED_R=$(grep -c 'Pushed DATA TTRODP11301' /tmp/mock_krx_${PORT}.log 2>/dev/null)
PUSHED_E=$(grep -c 'Pushed DATA TTRTDP21301' /tmp/mock_krx_${PORT}.log 2>/dev/null)
SEAM_OUT=$($BIN/seam_peek 2>&1)
$BIN/shm_snap dump "$RESULT/pcpo_miche.snap" 2>>"$RESULT/pcpo_snap.log"
DERIV=$(grep '^MICHE' "$RESULT/pcpo_miche.snap" 2>/dev/null | grep 'mk=1' | head -1)
DERIV_JAN=$(echo "$DERIV" | grep -oE 'jan=-?[0-9]+')

echo ""
echo "========================================"
echo "  PC→PO 단일 라이브 파이프라인 Result"
echo "========================================"
echo "  mock push TTRODP11301/TTRTDP21301 : $PUSHED_R / $PUSHED_E"
echo "  --- seam_peek ---"
printf '%s\n' "$SEAM_OUT" | sed 's/^/  /'
echo "  파생 미체결(mk=1)  : ${DERIV:-<없음>}"
echo "  전 구간 판정       : $([ -n "$DERIV" ] && [ "$DERIV_JAN" = "jan=0" ] && echo 'PASS (KRX→pc_→SEAM→po_ 등록 후 체결감소 100→0)' || echo "미달(entry='${DERIV}')")"
echo "========================================"

cleanup
log "done"
