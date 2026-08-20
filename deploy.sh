#!/bin/bash
##########################################################################
#   deploy.sh - Mac 소스를 FX all-one 서버(winway)로 배포
#   대상: winway@3.36.188.87:~/common/fep  (winway 직접 ssh 불가 →
#         rocky 로그인 후 sudo 로 winway 소유 이전)
#
#   Usage: ./deploy.sh                         # 변경 소스만 전송(rsync)
#          ./deploy.sh st01/src/PC/pc_1100_ts.c  # 특정 파일/디렉토리만
#          ./deploy.sh tar                      # tar로 st01 전체 전송
#          ./deploy.sh pull [경로...]           # 서버 → Mac 받기
##########################################################################

# 서버 설정 (fxallone: rocky 로그인 → sudo su winway)
PEM_KEY="$HOME/mywork/cert/winway-nh-fxallone-dev.pem"
SSH_USER="rocky"                       # 직접 접속 계정(무암호 sudo)
TARGET_USER="winway"                   # 실 소유 계정(sudo 경유)
REMOTE_HOST="3.36.188.87"
REMOTE_DIR="/home/winway/common/fep"   # winway 소유 트리
SSH="ssh -i $PEM_KEY -o StrictHostKeyChecking=accept-new -o BatchMode=yes"
STAGE="/tmp/fepdeploy.$$"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"; cd "$SCRIPT_DIR"
[ -f "$PEM_KEY" ] || { echo -e "${RED}ERROR: PEM key 없음: $PEM_KEY${NC}"; exit 1; }

EXCL=(--exclude='obj/' --exclude='bin/' --exclude='lib/' --exclude='*.o' --exclude='*.a'
      --exclude='BACKUP/' --exclude='BACK*/' --exclude='JC_OLD/' --exclude='.claude/' --exclude='.git/')

# push_to_winway <rsync 인자...> : rocky:/tmp 스테이징 → sudo rsync --chown 로 winway 트리 이전
push_to_winway() {
    rsync -aR -e "$SSH" "$@" ${SSH_USER}@${REMOTE_HOST}:$STAGE/ || return 1
    $SSH ${SSH_USER}@${REMOTE_HOST} \
        "sudo rsync -a --chown=${TARGET_USER}:${TARGET_USER} $STAGE/ ${REMOTE_DIR}/ && rm -rf $STAGE"
}

if [ "$1" = "pull" ]; then
    shift
    PATHS=("$@"); [ ${#PATHS[@]} -eq 0 ] && PATHS=(st01/inc st01/sub st01/src st01/make st01/cfg st01/shl st01/utl st01/test)
    echo -e "${YELLOW}=== pull: 서버 → Mac (${PATHS[*]}) ===${NC}"
    PSTAGE="/tmp/feppull.$$"
    for f in "${PATHS[@]}"; do
        $SSH ${SSH_USER}@${REMOTE_HOST} "sudo mkdir -p $PSTAGE/$(dirname "$f") && sudo cp -a ${REMOTE_DIR}/$f $PSTAGE/$(dirname "$f")/ && sudo chown -R ${SSH_USER} $PSTAGE"
        rsync -avz "${EXCL[@]}" -e "$SSH" ${SSH_USER}@${REMOTE_HOST}:$PSTAGE/$f "$(dirname "$f")/"
    done
    $SSH ${SSH_USER}@${REMOTE_HOST} "rm -rf $PSTAGE"
    echo -e "${GREEN}완료!${NC}"

elif [ "$1" = "tar" ]; then
    echo -e "${YELLOW}=== tar: st01 전체 전송 ===${NC}"
    tar czf /tmp/st01.tar.gz --exclude='st01/obj' --exclude='st01/bin' --exclude='st01/lib' \
        --exclude='st01/src/*/BACKUP' --exclude='st01/src/*/BACK*' --exclude='*.o' --exclude='*.a' st01/
    scp -i "$PEM_KEY" -o BatchMode=yes /tmp/st01.tar.gz ${SSH_USER}@${REMOTE_HOST}:/tmp/
    $SSH ${SSH_USER}@${REMOTE_HOST} \
        "sudo tar xzf /tmp/st01.tar.gz -C ${REMOTE_DIR} && sudo chown -R ${TARGET_USER}:${TARGET_USER} ${REMOTE_DIR}/st01 && rm -f /tmp/st01.tar.gz"
    rm -f /tmp/st01.tar.gz
    echo -e "${GREEN}완료!${NC}"

elif [ $# -gt 0 ]; then
    echo -e "${YELLOW}=== 지정 파일/디렉토리 전송 ===${NC}"; printf '  %s\n' "$@"
    push_to_winway "$@" && echo -e "${GREEN}완료!${NC}" || echo -e "${RED}실패${NC}"

else
    echo -e "${YELLOW}=== rsync: 변경 소스 전송 (st01 inc/sub/src/make/cfg/shl/utl/test) ===${NC}"
    echo -e "${YELLOW}    (--delete 미사용: 서버 bin/lib/obj 보호)${NC}"
    push_to_winway "${EXCL[@]}" \
        st01/inc st01/sub st01/src st01/make st01/cfg st01/shl st01/utl st01/test \
        && echo -e "${GREEN}완료!${NC}" || echo -e "${RED}실패${NC}"
fi
