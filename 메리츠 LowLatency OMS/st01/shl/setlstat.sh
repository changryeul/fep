# setlstat.sh
# change line status (TCP2)

cd $_P_BIN

if [ $# = 0 ]; then
	px_setlstat_mp
elif [ $# = 3 ]; then
	px_setlstat_mp $1 $2 $3
else
	echo "=========================================================="
	echo "[change line status (TCP2)]"
	echo ""
	echo "Usage: setlstat.sh <process ID> <line (P|B|A)> <line status (0|1)>"
	echo "       (line = P:primary B:backup A:all"
	echo "        line status = 0:off 1:on)"
	echo ""
	echo "  e.g. setlstat.sh pa_1111_ts P 1"
	echo "=========================================================="
fi
