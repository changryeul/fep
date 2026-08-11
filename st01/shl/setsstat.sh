# setsstat.sh
# change the start status of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_setsstat_mp
elif [ $# = 2 ]; then
	px_setsstat_mp $1 $2
else
	echo "=========================================================="
	echo "[change the start status of a process]"
	echo "(PROC.start_status will be changed)"
	echo ""
	echo "Usage: setsstat.sh <process ID> <start status (0:init 1:start 2:end 3:stop)>"
	echo ""
	echo "  e.g. 1) setsstat.sh pa_1111_ts 3"
	echo "       2) setsstat.sh pats 1"
	echo "=========================================================="
fi
