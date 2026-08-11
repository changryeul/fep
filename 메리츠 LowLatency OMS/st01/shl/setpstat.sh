# setpstat.sh
# change the process status of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_setpstat_mp
elif [ $# = 2 ]; then
	px_setpstat_mp $1 $2
else
	echo "=========================================================="
	echo "[change the process status of a process]"
	echo "(PROC.process_status will be changed)"
	echo "Usage: setpstat.sh <process ID> <process status (1:run 2:stop 9:not run)>"
	echo ""
	echo "  e.g. 1) setpstat.sh pa_1111_ts 1"
	echo "       2) setpstat.sh patr 1"
	echo "=========================================================="
fi
