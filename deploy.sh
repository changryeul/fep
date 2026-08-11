#!/bin/bash
##########################################################################
#   deploy.sh - Mac에서 수정한 소스를 EC2 서버로 배포
#   Usage: ./deploy.sh [파일|디렉토리...]
#          ./deploy.sh                    # 변경된 파일만 전송 (rsync)
#          ./deploy.sh st01/inc/fep_encrypt.h  # 특정 파일만 전송
#          ./deploy.sh tar                # tar로 묶어서 전송
#          ./deploy.sh pull               # 서버 → Mac 전체 소스 받기
#          ./deploy.sh pull st01/sub/     # 서버 → Mac 특정 디렉토리 받기
##########################################################################

# 서버 설정
PEM_KEY="$HOME/.ssh/winway-comp-dev.pem"
REMOTE_USER="ec2-user"
REMOTE_HOST="43.202.38.195"
REMOTE_DIR="~/new_fep"

# 색상
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 기본 경로 확인
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f "$PEM_KEY" ]; then
    echo -e "${RED}ERROR: PEM key not found: $PEM_KEY${NC}"
    exit 1
fi

# pull 모드: 서버 → Mac
if [ "$1" = "pull" ]; then
    shift
    if [ $# -gt 0 ]; then
        echo -e "${YELLOW}=== pull 모드: 서버에서 지정 경로 받기 ===${NC}"
        for f in "$@"; do
            echo "  $f"
            rsync -avz -e "ssh -i $PEM_KEY" \
                ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/${f} \
                ${f}
        done
    else
        echo -e "${YELLOW}=== pull 모드: 서버에서 전체 소스 받기 ===${NC}"
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            --exclude='obj/' \
            --exclude='bin/' \
            --exclude='lib/' \
            --exclude='*.o' \
            --exclude='*.a' \
            --exclude='BACKUP/' \
            --exclude='BACK*/' \
            --exclude='JC_OLD/' \
            --exclude='.claude/' \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/inc/ st01/inc/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            --exclude='BACKUP/' --exclude='BACK*/' --exclude='.claude/' \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/sub/ st01/sub/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            --exclude='BACKUP/' --exclude='BACK*/' --exclude='.claude/' \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/src/ st01/src/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/make/ st01/make/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/cfg/ st01/cfg/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/shl/ st01/shl/
        rsync -avz \
            -e "ssh -i $PEM_KEY" \
            ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/st01/utl/ st01/utl/
    fi
    echo -e "${GREEN}완료!${NC}"

# tar 모드
elif [ "$1" = "tar" ]; then
    echo -e "${YELLOW}=== tar 모드: st01 전체를 압축 후 전송 ===${NC}"
    tar czf /tmp/st01.tar.gz \
        --exclude='st01/obj' \
        --exclude='st01/bin' \
        --exclude='st01/lib' \
        --exclude='st01/src/*/BACKUP' \
        --exclude='st01/src/*/BACK*' \
        --exclude='*.o' \
        --exclude='*.a' \
        st01/

    echo -e "${GREEN}전송 중...${NC}"
    scp -i "$PEM_KEY" /tmp/st01.tar.gz \
        ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/

    echo -e "${GREEN}서버에서 압축 해제 중...${NC}"
    ssh -i "$PEM_KEY" ${REMOTE_USER}@${REMOTE_HOST} \
        "cd ${REMOTE_DIR} && tar xzf st01.tar.gz && rm st01.tar.gz"

    echo -e "${GREEN}완료!${NC}"
    rm -f /tmp/st01.tar.gz

# 특정 파일 모드
elif [ $# -gt 0 ]; then
    echo -e "${YELLOW}=== 지정 파일 전송 ===${NC}"
    for f in "$@"; do
        echo "  $f"
    done
    rsync -avR -e "ssh -i $PEM_KEY" \
        "$@" \
        ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/
    echo -e "${GREEN}완료!${NC}"

# 기본 모드: 변경된 소스만 rsync
else
    echo -e "${YELLOW}=== rsync 모드: 변경된 소스 파일만 전송 ===${NC}"
    rsync -avz --delete \
        -e "ssh -i $PEM_KEY" \
        --exclude='obj/' \
        --exclude='bin/' \
        --exclude='lib/' \
        --exclude='*.o' \
        --exclude='*.a' \
        --exclude='BACKUP/' \
        --exclude='BACK*/' \
        --exclude='JC_OLD/' \
        --exclude='.claude/' \
        --exclude='env/' \
        st01/inc/ st01/sub/ st01/src/ st01/make/ st01/cfg/ st01/shl/ st01/utl/ st01/test/ \
        --relative \
        ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/
    echo -e "${GREEN}완료!${NC}"
fi
