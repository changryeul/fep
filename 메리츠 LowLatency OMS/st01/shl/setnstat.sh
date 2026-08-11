# setnstat.sh
# change network status (TCP1, TCP2)

cd $_P_BIN

if [ $# = 0 ]; then
	px_setnstat_mp
elif [ $# = 3 ]; then
	px_setnstat_mp $1 $2 $3
else
	echo "=========================================================="
	echo "[change network status (TCP1, TCP2)]"
	echo ""
	echo "Usage: setnstat.sh <process ID> <line (P|B|A)> <network status (0|1)>"
	echo "       (line = P:primary B:backup A:all"
	echo "       network status = 0:off 1:on 2:end)"
	echo ""
	echo "  e.g. setnstat.sh pa_1111_ts P 1"
	echo "=========================================================="
fi
