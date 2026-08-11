# settout.sh
# change timeout of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_settout_mp
elif [ $# = 2 ]; then
	px_settout_mp $1 $2
else
	echo "=========================================================="
	echo "[change timeout of a process]"
	echo "(PROC.timeout (TIME_VALUE) will be changed)"
	echo ""
	echo "Usage: settout.sh <process ID> <timeout (in sec)>"
	echo ""
	echo "  e.g. settout.sh pa_1111_ts 10"
	echo "=========================================================="
fi
