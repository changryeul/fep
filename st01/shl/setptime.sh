# setptime.sh
# change start/end time of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_setptime_mp
elif [ $# = 3 ]; then
	px_setptime_mp $1 $2 $3
else
	echo "=========================================================="
	echo "[change start/end time of a process]"
	echo "(PROC.start_time and/or PROC.end_time will be changed)"
	echo ""
	echo "Usage: setptime.sh <process ID> <s/e> <time>"
	echo "       (s:start time, e:end time)"
	echo ""
	echo "  e.g. setptime.sh pa_1111_ts s 0630"
	echo "=========================================================="
fi
