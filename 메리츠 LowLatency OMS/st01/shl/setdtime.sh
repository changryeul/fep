# setdtime.sh
# change process status of a sub daemon

cd $_P_BIN

if [ $# = 0 ]; then
	px_setdtime_mp
elif [ $# = 4 ]; then
	px_setdtime_mp $1 $2 $3 $4
else
	echo "=========================================================="
	echo "[change start/end time and date flag of a sub daemon]"
	echo "(INFO.start_time, end_time, date_flag will be changed)"
	echo ""
	echo "Usage: setdtime.sh <sub name> <start time> <end time> <date flag>"
	echo "  e.g. setdtime.sh pa 0530 0300 2"
	echo "=========================================================="
fi
