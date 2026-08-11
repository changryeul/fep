#!/bin/sh
#------------------------------------------------------------------------
#   E2E 하니스: PB 주문 인바운드 홉 검증 (F5 order-pipeline-dshm V-04/05/06)
#   File   : run_e2e.sh   (Linux 서버 전용, ~/new_fep 기준)
#
#   파이프라인:
#     mock_oms(:41001, listen) <-connect- pb_1301_tr
#         -[파일큐 or DSHM]-> pb_1101_ts -connect-> mock_krx_server(:37221)
#
#   Usage: sh run_e2e.sh <file|dshm> [count] [interval_ms]
#
#   단계:
#     1) 실행 중 FEP 프로세스 확인 (있으면 중단)
#     2) cfg 백업 후 cfg/back 기반 하니스 설정 생성 (모드별 sed)
#     3) 런타임 초기화 (DAT/PB, LOG/PB, lat)
#     4) pz_memory_mp -> (dshm: pb_1001_mp) -> mock_krx -> pb_1101_ts
#        -> mock_oms -> pb_1301_tr
#     5) 주입/전달 검증 (mock_krx 수신 수, 큐 파일 레코드)
#     6) 전체 종료 + cfg 원복
#------------------------------------------------------------------------

MODE=${1:-file}
COUNT=${2:-100}
INTERVAL=${3:-10}

FEP=$HOME/new_fep
ST01=$FEP/st01
E2E=$ST01/test/e2e
BIN=$ST01/bin
CFG=$ST01/cfg
MOCK_KRX=$ST01/test/integ/bin/mock_krx_server
RESULT=$E2E/result
PIDS=""

log()  { printf '[E2E] %s\n' "$1"; }
fail() { printf '[E2E][FAIL] %s\n' "$1"; cleanup; exit 1; }

cleanup() {
    log "cleanup..."
    for p in $PIDS; do
        kill "$p" 2>/dev/null
    done
    sleep 1
    pkill -f 'pb_1301_tr|pb_1101_ts|pb_1001_mp|pz_memory_mp|mock_krx_server|mock_oms' 2>/dev/null
    if [ -d "$CFG.e2e_backup" ]; then
        rm -rf "$CFG"
        mv "$CFG.e2e_backup" "$CFG"
        log "cfg restored"
    fi
}

trap cleanup INT TERM

[ "$MODE" = file ] || [ "$MODE" = dshm ] || [ "$MODE" = prep ] || [ "$MODE" = restore ] || [ "$MODE" = pc ] || { echo "Usage: sh run_e2e.sh <file|dshm|pc|prep|restore> [count] [interval_ms]"; exit 1; }

# pc 모드: 파생 파일럿 — sender를 pc_1100_ts(파생 294B 코덱)로 교체, mock_oms는 파생 주문 주입.
#   config/큐/포트는 file 모드(PB 'b' 부문)를 그대로 재사용하고, sender 바이너리만 교체하여
#   pc_1100_ts의 KRX-direct 파생 인코딩 경로(TCHODR10001 294B)를 실 파이프라인으로 검증.
if [ "$MODE" = pc ]; then
    export MOCK_OMS_KIND=deriv
fi

# restore: cfg 원복 + 프로세스 정리만
if [ "$MODE" = restore ]; then
    cleanup
    exit 0
fi

# 이중 트리 주의: pkg_env는 사전설정 _FEP_HOME을 존중(:-)하므로, 소싱 전에 이 트리로 고정.
# (안 하면 _P_CFG 등이 기본 ~/fep(구트리)를 가리켜 daemon 설정 오독→pz_memory 크래시)
export _FEP_HOME=$FEP
. $ST01/env/pkg_env.sh >/dev/null 2>&1
ulimit -c unlimited 2>/dev/null   # 크래시 진단용 core 덤프

mkdir -p "$RESULT"
rm -f "$RESULT"/*.log "$RESULT"/*.lat 2>/dev/null

#--- 1) 안전 확인 --------------------------------------------------------
if ps -ef | grep -E 'p[abwz]_[0-9a-z_]+_(mp|ts|tr|ur)' | grep -v grep >/dev/null; then
    fail "FEP 프로세스가 이미 실행 중입니다. 종료 후 재시도하세요."
fi

#--- 2) cfg 하니스 구성 --------------------------------------------------
log "mode=$MODE count=$COUNT interval=${INTERVAL}ms"
[ -d "$CFG.e2e_backup" ] && fail "cfg.e2e_backup 잔존 - 이전 실행 원복 필요"
cp -a "$CFG" "$CFG.e2e_backup"

# PB 설정을 활성 cfg로 (cfg/back = PB 구성 원본)
for f in proc.ini tcp2.ini tcp1.ini daemon.ini file.ini udpip.ini sisetr.ini; do
    [ -f "$CFG/back/$f" ] && cp "$CFG/back/$f" "$CFG/$f"
done
# dshm.ini는 저장소 본(PB 섹션 포함) 유지

# 하니스 프로세스만 R + 상시 기동시간
sed -i \
    -e 's/^Proc_3_Status=S/Proc_3_Status=R/' \
    -e 's/^Proc_3_Start_Time=.*/Proc_3_Start_Time=0000/' \
    -e 's/^Proc_3_End_Time=.*/Proc_3_End_Time=2359/' \
    -e 's/^Proc_4_Status=S/Proc_4_Status=R/' \
    -e 's/^Proc_4_Start_Time=.*/Proc_4_Start_Time=0000/' \
    -e 's/^Proc_4_End_Time=.*/Proc_4_End_Time=2359/' \
    -e 's/^Proc_17_Status=S/Proc_17_Status=R/' \
    "$CFG/proc.ini"

# dshm.ini의 PB 섹션(1개)과 일치시킴 (file 모드에서는 세그먼트 생성만, 미사용)
sed -i 's/^Daemon_B_Dshm_Count=0/Daemon_B_Dshm_Count=1/' "$CFG/daemon.ini"

if [ "$MODE" = dshm ]; then
    sed -i \
        -e 's/^Proc_3_OFN_1=pb_1101_ts/Proc_3_ODN_1=pb_1101_ts/' \
        -e 's/^Proc_4_IFN_1=pb_1101_ts/Proc_4_IDN_1=pb_1101_ts/' \
        "$CFG/proc.ini"
fi

# 접속 대상 localhost화: pb_1301(41001)->mock_oms, pb_1101(37221)->mock_krx
python3 - "$CFG/tcp2.ini" <<'EOF'
import re, sys
path = sys.argv[1]
txt = open(path, encoding="utf-8", errors="replace").read().split("\n")
for i, line in enumerate(txt):
    m = re.match(r"^Tcp2_(\d+)_Port=(41001|37221)$", line.strip())
    if m:
        n = m.group(1)
        for j in range(len(txt)):
            if re.match(rf"^Tcp2_{n}_Ip=", txt[j].strip()):
                txt[j] = f"Tcp2_{n}_Ip=127.0.0.1"
open(path, "w", encoding="utf-8").write("\n".join(txt))
EOF

log "cfg prepared ($MODE)"
grep -n 'Proc_3_O.N_1\|Proc_4_I.N_1' "$CFG/proc.ini" | head -4

#--- 3) 런타임 초기화 ----------------------------------------------------
mkdir -p $FEP/st02/FIFO/PB $FEP/st02/DAT/PB/00000000 $FEP/st03/LOG/PB $FEP/st03/SEQ
rm -f $FEP/st02/DAT/PB/00000000/pb_1101_ts* $FEP/st02/DAT/PB/00000000/pb_1401_ts* 2>/dev/null
rm -f $FEP/st03/LOG/PB/*.lat 2>/dev/null
export FEP_LAT_DIR=$RESULT
export FEP_LAT_FLUSH=1   # 측정 하니스: 프로세스 kill 시 버퍼 유실 방지

#--- 4) 기동 -------------------------------------------------------------
# FEP 프로세스는 argv[0]에서 서브시스템명을 파생하므로 (데몬의 execl과
# 동일하게) exec -a 로 순수 프로세스명을 argv[0]에 넣어 기동한다.
start_fep() {
    _name=$1; _log=$2; _args=$3
    ( cd $BIN && exec bash -c "exec -a $_name $BIN/$_name $_args" ) > "$_log" 2>&1 &
    _pid=$!
    PIDS="$PIDS $_pid"
    echo "$_pid"
}

# SHM 생성+설정 로드 (shmload.sh 규약: 'z' = 데몬 SHM, 'b' = PB 부문)
log "loading SHM: pz_memory_mp z ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp z" ) > "$RESULT/pz_z.log" 2>&1
RC=$?
[ "$RC" -eq 0 ] || { cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/" 2>/dev/null; fail "pz_memory_mp z exit=$RC (result/pz_memory_mp* 로그 확인)"; }
sleep 1

log "loading SHM: pz_memory_mp b ..."
( cd $BIN && exec bash -c "exec -a pz_memory_mp $BIN/pz_memory_mp b" ) > "$RESULT/pz_b.log" 2>&1
RC=$?
cp $FEP/st03/LOG/PZ/$(date +%Y%m%d)/pz_memory_mp* "$RESULT/pz_memory_mp.fep.log" 2>/dev/null
[ "$RC" -eq 0 ] || fail "pz_memory_mp b exit=$RC (result/pz_memory_mp.fep.log 확인)"
sleep 1

if [ "$MODE" = dshm ]; then
    log "starting SyncManager pb_1001_mp..."
    start_fep pb_1001_mp "$RESULT/sync.log" >/dev/null
    sleep 1
fi

log "starting mock_krx_server :37221..."
"$MOCK_KRX" 37221 > "$RESULT/mock_krx.log" 2>&1 &
PIDS="$PIDS $!"
sleep 1

if [ -n "$E2E_DEBUG_1101" ] || [ "$MODE" = prep ]; then
    log "E2E_DEBUG_1101: pb_1101_ts 기동 생략 - 별도 셸에서 직접/gdb로 기동하세요:"
    log "  cd $BIN && PATH=\$PWD:\$PATH gdb -batch -ex run -ex bt pb_1101_ts"
elif [ "$MODE" = pc ]; then
    log "starting pc_1100_ts (as pb_1101_ts, 파생 KRX-direct 코덱)..."
    ( cd $BIN && exec bash -c "exec -a pb_1101_ts $BIN/pc_1100_ts" ) > "$RESULT/pb_1101.log" 2>&1 &
    PIDS="$PIDS $!"
else
    log "starting pb_1101_ts..."
    start_fep pb_1101_ts "$RESULT/pb_1101.log" >/dev/null
fi
sleep 2

log "starting mock_oms :41001 ($COUNT orders)..."
cd "$RESULT" && "$E2E/mock_oms" 41001 "$COUNT" "$INTERVAL" > "$RESULT/mock_oms.log" 2>&1 &
PIDS="$PIDS $!"
cd $BIN
sleep 1

log "starting pb_1301_tr..."
start_fep pb_1301_tr "$RESULT/pb_1301.log" >/dev/null

#--- 5) 대기 + 검증 ------------------------------------------------------
# prep: 큐 채움(주입)까지 완료 후 mock_krx/pb_1301은 살려둔 채 종료
#       (cfg 백업 유지 - 'restore'로 원복. pb_1101을 수동/gdb로 기동해 재현)
if [ "$MODE" = prep ]; then
    sleep $(( COUNT * INTERVAL / 1000 + 5 ))
    log "prep done: queue filled, mock_krx up. gdb 예:"
    log "  cd $BIN && gdb -batch -ex 'set follow-fork-mode child' -ex 'set follow-exec-mode new' -ex 'set exec-wrapper /bin/bash -c \"exec -a pb_1101_ts \\\$0\"' -ex run -ex bt ./pb_1101_ts"
    trap - INT TERM
    exit 0
fi

WAIT=$(( COUNT * INTERVAL / 1000 + COUNT / 4 + 15 ))   # 주문당 응답 왕복 페이스 고려
[ -n "$E2E_DEBUG_1101" ] && WAIT=300
log "waiting ${WAIT}s for pipeline..."
sleep "$WAIT"

SENT=$(grep -c 'IN|' "$RESULT/mock_oms.lat" 2>/dev/null || echo 0)
# mock_krx는 상세 수신로그를 /tmp/mock_krx_<port>.log에 기록(stdout=$RESULT/mock_krx.log엔 요약만)
# → 두 파일 모두에서 TCHODR 수신 라인 집계
KRX_RECV=$(cat "$RESULT/mock_krx.log" /tmp/mock_krx_37221.log 2>/dev/null | grep -c 'Received.*TCHODR')
QUEUE_FILE=$FEP/st02/DAT/PB/00000000/pb_1101_ts
QFILE_SZ=$(stat -c %s "$QUEUE_FILE" 2>/dev/null || echo 0)

echo ""
echo "========================================"
echo "  E2E Result (mode=$MODE)"
echo "========================================"
echo "  injected (mock_oms) : $SENT"
echo "  mock_krx recv lines : $KRX_RECV"
echo "  queue/backing file  : $QFILE_SZ bytes"
echo "  lat files           : $(ls "$RESULT"/*.lat 2>/dev/null | tr '\n' ' ')"
echo "========================================"
echo "  logs: $RESULT/"
echo ""
tail -3 "$RESULT/pb_1301.log" 2>/dev/null
tail -3 "$RESULT/pb_1101.log" 2>/dev/null
tail -5 "$RESULT/mock_krx.log" 2>/dev/null

#--- 6) 정리 -------------------------------------------------------------
cleanup
log "done"
