# setinfostat.sh
# change process status of a sub daemon

cd $_P_BIN

if [ $# = 0 ]; then
    px_setinfostat_mp
elif [ $# = 2 ]; then
    px_setinfostat_mp $1 $2
else
	echo "=========================================================="
	echo "[change process status of a sub daemon]"
	echo "(INFO.process_status will be changed)"
	echo ""
	echo "Usage: setinfostat.sh <sub name> <process status>"
	echo "       (process status = 0:stop 1:run)"
	echo ""
	echo "  e.g. setinfostat.sh pa 1"
	echo "=========================================================="
fi
