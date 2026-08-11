#!/bin/bash
##########################################################################
#	deploy_test.sh - 서버에 test 파일 배포 및 테스트 실행
#
#	Usage:
#	  1) macOS에서: ./deploy_test.sh fepp@10.x.x.x
#	  2) 서버에서 직접: ./deploy_test.sh local
##########################################################################

set -e

TARGET=$1

if [ -z "$TARGET" ]; then
	echo "======================================"
	echo "Usage:"
	echo "  ./deploy_test.sh fepp@10.x.x.x   # macOS -> 서버 배포+테스트"
	echo "  ./deploy_test.sh local            # 서버에서 직접 실행"
	echo "======================================"
	exit 1
fi

#-- 서버 경로 --
REMOTE_HOME='$HOME/fep/st01'
REMOTE_TEST='$HOME/fep/st01/test'

if [ "$TARGET" = "local" ]; then
	#----------------------------------------------------------------------
	#  서버에서 직접 실행하는 경우
	#----------------------------------------------------------------------
	echo "========================================"
	echo "  Running test locally on $(hostname)"
	echo "========================================"

	# 스크립트가 위치한 디렉토리로 이동 (macOS/서버 경로 무관)
	SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
	cd "$SCRIPT_DIR"

	# sqlite3 확인
	if ! command -v sqlite3 >/dev/null 2>&1; then
		echo "ERROR: sqlite3 not found in PATH"
		echo "  Install: yum install sqlite-devel (RHEL/CentOS)"
		echo "           apt install libsqlite3-dev sqlite3 (Debian/Ubuntu)"
		exit 1
	fi

	echo "[CHECK] sqlite3 OK: $(sqlite3 --version)"
	echo "[CHECK] OS: $(uname -s)"
	echo "[CHECK] CC: $(cc --version 2>&1 | head -1 || echo 'cc available')"
	echo ""

	make clean
	make
	make dbcheck

	echo ""
	echo "========================================"
	echo "  Server test complete!"
	echo "========================================"
	echo ""
	echo "  Optional: cfg_verify (SHM이 실행 중일 때)"
	echo "    make cfgverify"
	echo "    ./cfg_verify -e ${_FEP_DIV:-TEST} -d fep_config.db -v"
	echo ""

else
	#----------------------------------------------------------------------
	#  macOS에서 서버로 원격 배포하는 경우
	#----------------------------------------------------------------------
	echo "========================================"
	echo "  Deploying to $TARGET"
	echo "========================================"

	# 1) 서버에 test 디렉토리 생성
	echo "[1/4] Creating test directory..."
	ssh $TARGET "mkdir -p ~/fep/st01/test/export"

	# 2) 파일 전송 (Makefile + 소스)
	echo "[2/4] Uploading files..."
	scp -q Makefile $TARGET:~/fep/st01/test/
	scp -q deploy_test.sh $TARGET:~/fep/st01/test/
	ssh $TARGET "chmod +x ~/fep/st01/test/deploy_test.sh"

	# 3) 원격 테스트 실행
	echo "[3/4] Running test on server..."
	echo ""
	ssh $TARGET "cd ~/fep/st01/test && ./deploy_test.sh local"

	# 4) 결과
	echo ""
	echo "[4/4] Done. DB file on server: ~/fep/st01/test/fep_config.db"
fi
