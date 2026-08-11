# compact.sh
# unlink the expired data and log files

cd $_P_BIN

if [ $# = 0 ]; then
	pz_compact_mp
elif [ $# = 1 ]  && [ $1 != "?" ]; then
	pz_compact_mp $1
elif [ $# = 2 ]; then
	pz_compact_mp $1 $2
else
	echo "=========================================================="
	echo "[unlink the expired data and log files]"
	echo ""
	echo "Usage: compact.sh <expiration days (0 ~)> <sub system name (a ~ z)>"
	echo ""
	echo "  e.g. 1) compact.sh -> 전 부문 기본정리일수 (7일) 이전 삭제"
	echo "       2) compact.sh 3 -> 전 부문 3일 이전 삭제"
	echo "       3) compact.sh 5 a -> PA 부문 5일 이전 삭제"
	echo "=========================================================="
fi
