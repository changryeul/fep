#!/bin/sh
#------------------------------------------------------------------------
#   E2E 하니스: po_ OMS코어 런타임 부문화 + po_1200_mp 응답분배 검증
#   File   : run_po_e2e.sh   (Linux 서버 전용, ~/new_fep 기준)
#
#   목적:
#     1) 'o' 부문(D_K=14)을 config만으로 세운다 (Daemon_O + PO_CONF proc/file).
#        → pz_memory_mp o 가 'o' SHM/FIFO를 생성 (독립기동 config의 첫 조각).
#     2) po_1200_mp(응답분배)를 기동하여 입력 FIFO에서 회원처리호가를 읽어
#        Client FIFO(OFN)로 분배하는지 검증.
#
#   주입: pc_1200_tr E2E와 달리 생산자가 없으므로, 하니스가 만든 mini 생산자
#         (po_inject: F_W로 크래프트 레코드 1건 기록)로 입력 FIFO를 채운다.
#
#   Usage: sh run_po_e2e.sh
#------------------------------------------------------------------------

FEP=$HOME/new_fep
ST01=$FEP/st01
E2E=$ST01/test/e2e
BIN=$ST01/bin
CFG=$ST01/cfg
RESULT=$E2E/result
PIDS=""

log()  { printf '[POE2E] %s\n' "$1"; }
fail() { printf '[POE2E][FAIL] %s\n' "$1"; cleanup; exit 1; }

cleanup() {
    log "cleanup..."
    for p in $PIDS; do kill -9 "$p" 2>/dev/null; done
    # pkill -x(정확 comm 매칭): -f는 패턴 문자열을 담은 호출 셸까지 죽여 자기종료 유발
    pkill -9 -x po_1200_mp 2>/dev/null
    pkill -9 -x po_1290_mp 2>/dev/null
    pkill -9 -x po_1490_mp 2>/dev/null
    pkill -9 -x po_9001_mp 2>/dev/null
    pkill -9 -x po_9000_mp 2>/dev/null   # 한도: exec -a로 argv0=po_9001_mp지만 comm=po_9000_mp
    pkill -9 -x pz_memory_mp 2>/dev/null
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
rm -f "$RESULT"/poe2e_*.log 2>/dev/null

if ps -ef | grep -E 'p[abcowz]_[0-9a-z_]+_(mp|ts|tr|ur)' | grep -v grep >/dev/null; then
    fail "FEP 프로세스가 이미 실행 중. 종료 후 재시도."
fi
ipcs -m 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -m 2>/dev/null
ipcs -s 2>/dev/null | awk '/^0x/{print $2}' | xargs -r -n1 ipcrm -s 2>/dev/null

#--- cfg 구성: cfg/back 기반 + 'o' 부문 추가 ----------------------------
[ -d "$CFG.e2e_backup" ] && fail "cfg.e2e_backup 잔존 - 원복 필요"
cp -a "$CFG" "$CFG.e2e_backup"
for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done

# (1) daemon.ini: Daemon_O(예비) 블록을 OMS코어 config로 채움
python3 - "$CFG/daemon.ini" <<'EOF'
import sys, re
p = sys.argv[1]
t = open(p, encoding="utf-8", errors="replace").read().split("\n")
out, i = [], 0
while i < len(t):
    line = t[i]
    if line.strip() == "Daemon_O_Comment=예비":
        out += [
            "Daemon_O_Comment=OMS_CORE",
            "Daemon_O_ID=po_daemon_mp",
            "Daemon_O_Start_Time=0000",
            "Daemon_O_End_Time=2359",
            "Daemon_O_Date_Flag=1",
            "Daemon_O_Compact_Days=4",
            "Daemon_O_Status=1",
            "Daemon_O_FIFO=po_FIFO",
            "Daemon_O_Shm_Log=1",
            "Daemon_O_Proc_Count=10",
            "Daemon_O_File_Count=10",
            "Daemon_O_Dshm_Count=0",
            "Daemon_O_Tcp2_Count=0",
            "Daemon_O_Udpip_Count=0",
            "Daemon_O_Data_Count=0",
        ]
        # 다음 Daemon_End 는 그대로 두고, 현재 Comment 라인만 대체
        i += 1
        continue
    out.append(line)
    i += 1
open(p, "w", encoding="utf-8").write("\n".join(out))
print("daemon.ini Daemon_O filled")
EOF

# (2) proc.ini: PO_CONF 추가 (DAEMON_CONF_END 뒤 아무데나 — reader가 PO_CONF_START 스캔)
cat >> "$CFG/proc.ini" <<'EOF'

PO_CONF_START
Proc_Count=7
Proc_1_Comment=resp_distribute
Proc_1_ID=po_1200_mp
Proc_1_Status=R
Proc_1_Type=MP
Proc_1_IFN_1=po_1200_mp
Proc_1_OFN_1=po_out_ts
Proc_1_Start_Time=0000
Proc_1_End_Time=2359
Proc_1_Time_Out=0
Proc_End
Proc_2_Comment=inject
Proc_2_ID=po_1209_mp
Proc_2_Status=R
Proc_2_Type=MP
Proc_2_OFN_1=po_1200_mp
Proc_2_Start_Time=0000
Proc_2_End_Time=2359
Proc_2_Time_Out=0
Proc_End
Proc_3_Comment=miche
Proc_3_ID=po_1290_mp
Proc_3_Status=R
Proc_3_Type=MP
Proc_3_IFN_1=po_1290_mp
Proc_3_Start_Time=0000
Proc_3_End_Time=2359
Proc_3_Time_Out=0
Proc_End
Proc_4_Comment=miche_inject
Proc_4_ID=po_1219_mp
Proc_4_Status=R
Proc_4_Type=MP
Proc_4_OFN_1=po_1290_mp
Proc_4_Start_Time=0000
Proc_4_End_Time=2359
Proc_4_Time_Out=0
Proc_End
Proc_5_Comment=settle
Proc_5_ID=po_1490_mp
Proc_5_Status=R
Proc_5_Type=MP
Proc_5_IFN_1=po_1490_mp
Proc_5_Start_Time=0000
Proc_5_End_Time=2359
Proc_5_Time_Out=0
Proc_End
Proc_6_Comment=settle_inject
Proc_6_ID=po_1229_mp
Proc_6_Status=R
Proc_6_Type=MP
Proc_6_OFN_1=po_1490_mp
Proc_6_Start_Time=0000
Proc_6_End_Time=2359
Proc_6_Time_Out=0
Proc_End
Proc_7_Comment=risk_limit
Proc_7_ID=po_9001_mp
Proc_7_Status=R
Proc_7_Type=MP
Proc_7_IFN_1=po_9001_mp
Proc_7_Start_Time=0000
Proc_7_End_Time=2359
Proc_7_Time_Out=0
Proc_End
PO_CONF_END
EOF

# (3) file.ini: PO_CONF 추가
cat >> "$CFG/file.ini" <<'EOF'

PO_CONF_START
File_Count=5
File_1_Comment=resp_dist_in
File_1_Name=po_1200_mp
File_1_Fifo=1
File_1_Size=400
File_End
File_2_Comment=resp_dist_out
File_2_Name=po_out_ts
File_2_Fifo=1
File_2_Size=400
File_End
File_3_Comment=miche_in
File_3_Name=po_1290_mp
File_3_Fifo=1
File_3_Size=400
File_End
File_4_Comment=settle_in
File_4_Name=po_1490_mp
File_4_Fifo=1
File_4_Size=400
File_End
File_5_Comment=risk_in
File_5_Name=po_9001_mp
File_5_Fifo=1
File_5_Size=2048
File_End
PO_CONF_END
EOF

# (sisetr PO_CONF 불필요: Sise_SHM 게이트는 init_proc에서 제거됨 — 항상 attach)

log "cfg prepared ('o' 부문 추가)"

#--- 런타임 디렉토리 ------------------------------------------------------
mkdir -p $FEP/st02/FIFO/PO $FEP/st02/DAT/PO/00000000 $FEP/st03/LOG/PO $FEP/st03/SEQ
rm -f $FEP/st02/DAT/PO/00000000/po_* 2>/dev/null

#--- SHM 생성: z(데몬) + o(OMS코어 부문) -------------------------------
log "pz_memory_mp z ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/poe2e_pz_z.log" 2>&1
[ "$?" -eq 0 ] || fail "pz_memory_mp z 실패 (poe2e_pz_z.log)"
sleep 1
log "pz_memory_mp o ... (‘o’ 부문 SHM/FIFO 생성 = 부문화 검증)"
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp o" ) > "$RESULT/poe2e_pz_o.log" 2>&1
RC=$?
cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/poe2e_pz_o.fep.log" 2>/dev/null
[ "$RC" -eq 0 ] || fail "pz_memory_mp o 실패 rc=$RC (poe2e_pz_o.fep.log 확인)"
log "  → 'o' 부문 생성 성공"
echo "  FIFO/PO: $(ls $FEP/st02/FIFO/PO 2>/dev/null | tr '\n' ' ')"
echo "  SHM(0x420e*): $(ipcs -m | grep -c 0x420e)"

#--- po_1200_mp 기동 (부문화 검증: 정상기동+FIFO 대기) ------------------
log "starting po_1200_mp ..."
( cd $BIN && exec bash -c "exec -a po_1200_mp $BIN/po_1200_mp" ) </dev/null > "$RESULT/poe2e_po.log" 2>&1 &
PIDS="$PIDS $!"
log "starting po_1290_mp (미체결, 전역 MK_PM attach) ..."
( cd $BIN && exec bash -c "exec -a po_1290_mp $BIN/po_1290_mp" ) </dev/null > "$RESULT/poe2e_miche.log" 2>&1 &
PIDS="$PIDS $!"
log "starting po_1490_mp (체결원장, 미체결 감소) ..."
( cd $BIN && exec bash -c "exec -a po_1490_mp $BIN/po_1490_mp" ) </dev/null > "$RESULT/poe2e_settle.log" 2>&1 &
PIDS="$PIDS $!"
log "starting po_9001_mp (한도/RISK 초기화, argc=1→강제init) ..."
( cd $BIN && exec bash -c "exec -a po_9001_mp $BIN/po_9000_mp" ) </dev/null > "$RESULT/poe2e_risk.log" 2>&1 &
PIDS="$PIDS $!"
sleep 3

if pgrep -f 'po_1200_mp' >/dev/null 2>&1; then
    log "  → po_1200_mp 정상 기동·대기 중 (crash 0)"
    PO_ALIVE=1
else
    log "  → po_1200_mp 조기 종료 (poe2e_po.log 확인)"
    PO_ALIVE=0
fi

#--- 데이터흐름: po_inject로 회원처리호가 1건 주입 → po_1200_mp 분배 ---
log "injecting 1 TTRODP11301(매체=C) via po_1209_mp(=po_inject) ..."
( cd $BIN && exec bash -c "exec -a po_1209_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_inject.log" 2>&1
sleep 2

#--- 미체결 데이터흐름: 채권 회원처리호가 주입 → po_1290_mp 등록 → shm_snap ---
log "injecting 1 TTRODP41301(채권 회원처리호가) via po_1219_mp ..."
( cd $BIN && exec bash -c "export PO_INJECT_KIND=bond_miche; exec -a po_1219_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_miche_inject.log" 2>&1
sleep 2
$BIN/shm_snap dump "$RESULT/miche_A.snap" 2>>"$RESULT/poe2e_snap.log"
MICHE_N=$(grep -c '^MICHE' "$RESULT/miche_A.snap" 2>/dev/null)
log "  → 미체결 등록 ${MICHE_N}건 (functional): $(grep '^MICHE' "$RESULT/miche_A.snap" 2>/dev/null | head -1)"

#--- replay-safety: po_1290_mp kill→restart → 재처리(이중등록) 없어야 함 ---
log "replay: po_1290_mp kill→restart (read커서 유지 → 재처리 없어야) ..."
pkill -9 -x po_1290_mp 2>/dev/null; sleep 1
( cd $BIN && exec bash -c "exec -a po_1290_mp $BIN/po_1290_mp" ) </dev/null > "$RESULT/poe2e_miche2.log" 2>&1 &
PIDS="$PIDS $!"
sleep 3
$BIN/shm_snap dump "$RESULT/miche_B.snap" 2>>"$RESULT/poe2e_snap.log"
$BIN/shm_snap cmp "$RESULT/miche_A.snap" "$RESULT/miche_B.snap" >"$RESULT/poe2e_cmp.log" 2>&1
REPLAY_RC=$?

#--- 체결원장: 채권 체결(전량 100) 주입 → po_1490_mp가 미체결 잔량 감소(100→0) ---
log "injecting 1 TTRTDP42301(채권 체결 수량100) via po_1229_mp → po_1490_mp ..."
( cd $BIN && exec bash -c "export PO_INJECT_KIND=bond_settle; exec -a po_1229_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_settle_inject.log" 2>&1
sleep 2
$BIN/shm_snap dump "$RESULT/miche_C.snap" 2>>"$RESULT/poe2e_snap.log"
SETTLE_LINE=$(grep '^MICHE' "$RESULT/miche_C.snap" 2>/dev/null | grep 'mk=0' | head -1)
SETTLE_JAN=$(echo "$SETTLE_LINE" | grep -oE 'jan=-?[0-9]+')

#--- 파생 미체결(MK_GBN 런타임): 파생 회원처리호가 TTRODP11301 주입 → po_1290_mp Make_MiChe_Deriv 등록(mk=1) ---
log "injecting 1 TTRODP11301(파생 회원처리호가) via po_1219_mp → po_1290_mp Make_MiChe_Deriv ..."
( cd $BIN && exec bash -c "export PO_INJECT_KIND=deriv_miche; exec -a po_1219_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_deriv_inject.log" 2>&1
sleep 2
$BIN/shm_snap dump "$RESULT/miche_D.snap" 2>>"$RESULT/poe2e_snap.log"
DERIV_LINE=$(grep '^MICHE' "$RESULT/miche_D.snap" 2>/dev/null | grep 'mk=1' | head -1)

#--- 파생 체결원장(MK_GBN 런타임): 파생 체결 TTRTDP21301 주입 → po_1490_mp Analyze_Che_Deriv → mk=1 잔량 감소 ---
log "injecting 1 TTRTDP21301(파생 체결 수량100) via po_1229_mp → po_1490_mp Analyze_Che_Deriv ..."
( cd $BIN && exec bash -c "export PO_INJECT_KIND=deriv_settle; exec -a po_1229_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_deriv_settle.log" 2>&1
sleep 2
$BIN/shm_snap dump "$RESULT/miche_E.snap" 2>>"$RESULT/poe2e_snap.log"
DERIV_SETTLE_LINE=$(grep '^MICHE' "$RESULT/miche_E.snap" 2>/dev/null | grep 'mk=1' | head -1)
DERIV_SETTLE_JAN=$(echo "$DERIV_SETTLE_LINE" | grep -oE 'jan=-?[0-9]+')

#--- 파생 자동취소(TTRODP11303): 재등록(잔량 감소된 슬롯 재사용) → 자동취소 → 잔량 100→0 ---
log "injecting deriv_miche(재등록) then deriv_autocxl(TTRODP11303) via po_1219_mp ..."
( cd $BIN && exec bash -c "export PO_INJECT_KIND=deriv_miche; exec -a po_1219_mp $BIN/po_inject" ) </dev/null >> "$RESULT/poe2e_deriv_inject.log" 2>&1
sleep 2
( cd $BIN && exec bash -c "export PO_INJECT_KIND=deriv_autocxl; exec -a po_1219_mp $BIN/po_inject" ) </dev/null > "$RESULT/poe2e_deriv_autocxl.log" 2>&1
sleep 2
$BIN/shm_snap dump "$RESULT/miche_F.snap" 2>>"$RESULT/poe2e_snap.log"
DERIV_ACXL_LINE=$(grep '^MICHE' "$RESULT/miche_F.snap" 2>/dev/null | grep 'mk=1' | head -1)
DERIV_ACXL_JAN=$(echo "$DERIV_ACXL_LINE" | grep -oE 'jan=-?[0-9]+')

#--- Che_Rtn &p_buf 버그수정 검증: 채권 체결 후 ProFit 잔고(item_getcnt) 활성화 ---
$BIN/shm_snap risk "$RESULT/risk.snap" 2>>"$RESULT/poe2e_snap.log"
RISK_BOND=$(grep '^PROFIT mk=0 ' "$RESULT/risk.snap" 2>/dev/null | head -1)
RISK_DERIV=$(grep '^PROFIT mk=1 ' "$RESULT/risk.snap" 2>/dev/null | head -1)

#--- 분배기 SEAM 전환 검증: po_1200_mp가 응답 큐를 SEAM_R_DIST 커서로 소비했는지 ---
PODIST_LOG=$(ls -t $FEP/st03/LOG/PO/*/po_1200_mp* 2>/dev/null | head -1)
PODIST_RD=$([ -n "$PODIST_LOG" ] && grep -c 'SEAM RD \[q=0 r=1\]' "$PODIST_LOG" 2>/dev/null || echo 0)
PODIST_ALIVE=$(pgrep -x po_1200_mp >/dev/null && echo Y || echo N)

#--- 결과 요약 ----------------------------------------------------------
echo ""
echo "========================================"
echo "  PO 부문화 E2E Result"
echo "========================================"
PLOG=$(ls -t $FEP/st03/LOG/PO/*/po_1200_mp* 2>/dev/null | head -1)
OUTFILE=$FEP/st02/DAT/PO/00000000/po_out_ts
OUT_SZ=$(stat -c %s "$OUTFILE" 2>/dev/null || echo 0)
DIST=$( [ -n "$PLOG" ] && grep -c 'file write\[' "$PLOG" 2>/dev/null || echo 0)
echo "  'o' 부문 SHM/FIFO : $(ls $FEP/st02/FIFO/PO 2>/dev/null | wc -l) FIFO, stat=$( [ -f $FEP/st02/DAT/PO/00000000/po_1200_mp_stat ] && echo OK || echo MISSING )"
echo "  po_1200_mp 상태   : $( [ "$PO_ALIVE" = 1 ] && echo 'RUNNING (부문화 OK)' || echo DEAD )"
echo "  po_1290_mp 상태   : $(pgrep -x po_1290_mp >/dev/null && echo 'RUNNING (미체결 전역MK_PM attach OK)' || echo DEAD)"
echo "  미체결 functional : ${MICHE_N}건 등록 (채권 회원처리호가 TTRODP41301 주입→분배)"
echo "  미체결 replay멱등 : $( [ "$REPLAY_RC" = 0 ] && echo 'IDENTICAL (재기동 재처리 없음 OK)' || echo "DIFFER(rc=$REPLAY_RC)" ) → $(cat "$RESULT/poe2e_cmp.log" 2>/dev/null | tail -1)"
echo "  등록 미체결 상세  : $(grep '^MICHE' "$RESULT/miche_A.snap" 2>/dev/null | head -1)"
echo "  체결원장 감소     : 체결(수량100) 후 $SETTLE_JAN (100→0 이면 전량체결 반영 OK)"
echo "  체결 후 상세      : $SETTLE_LINE"
echo "  파생 미체결(mk=1) : ${DERIV_LINE:-<없음 - Make_MiChe_Deriv 미등록>}"
echo "    → 등록 MK_GBN: $([ -n "$DERIV_LINE" ] && echo 'PASS (한 바이너리가 채권 mk=0 + 파생 mk=1 등록)' || echo 'FAIL')"
echo "  파생 체결감소(mk=1): ${DERIV_SETTLE_LINE:-<없음>}"
echo "    → 체결 MK_GBN: $([ "$DERIV_SETTLE_JAN" = "jan=0" ] && echo 'PASS (파생 체결 100→0, Analyze_Che_Deriv)' || echo "미달($DERIV_SETTLE_JAN)")"
echo "  파생 자동취소(mk=1): ${DERIV_ACXL_LINE:-<없음>}"
echo "    → 자동취소(11303): $([ "$DERIV_ACXL_JAN" = "jan=0" ] && echo 'PASS (재등록 100 → 자동취소 100→0, Make_MiChe_Deriv)' || echo "미달($DERIV_ACXL_JAN)")"
echo "  분배기 po_1200_mp  : alive=$PODIST_ALIVE, SEAM_R_DIST 소비=${PODIST_RD}건"
echo "    → 분배 SEAM 전환: $([ "$PODIST_ALIVE" = "Y" ] && [ "${PODIST_RD:-0}" -gt 0 ] && echo 'PASS (응답 큐 multi-reader: 미체결+분배 독립 소비)' || echo "미달(alive=$PODIST_ALIVE rd=$PODIST_RD)")"
echo "  Che_Rtn &p_buf수정 : 채권 ProFit[$RISK_BOND]"
echo "                       파생 ProFit[$RISK_DERIV]"
echo "    → 잔고 활성화: $(echo "$RISK_BOND" | grep -q 'getcnt=100' && echo 'PASS (채권 체결→getcnt=100, Che_Rtn P&L 정상 동작)' || echo "미달($RISK_BOND)")"
echo "  po_9001_mp 한도   : $(pgrep -x po_9000_mp >/dev/null && echo 'RUNNING (RISK attach·Init_Parameters 초기화 crash 0)' || echo 'DEAD (poe2e_risk.log 확인)')"
echo "                      (comm=po_9000_mp/argv0=po_9001_mp : 소스명≠실행명이라 comm으로 체크)"
ILOG=$(ls -t $FEP/st03/LOG/PO/*/po_1209_mp* 2>/dev/null | head -1)
INJ=$( [ -n "$ILOG" ] && grep -c 'wrote 1' "$ILOG" 2>/dev/null || echo 0)
echo "  주입→분배         : inject(po_1209_mp wrote)=$INJ, po Client출력(po_out_ts)=$OUT_SZ bytes, po_1200_mp 분배log=$DIST"
echo "  po FEP log tail:"
[ -n "$PLOG" ] && tail -8 "$PLOG" 2>/dev/null
echo "========================================"

cleanup
log "done"
