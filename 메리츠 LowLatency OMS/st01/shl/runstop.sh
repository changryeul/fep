# runstop.sh
# start or stop the sub daemon

cd $_P_BIN

if [ $# = 0 ]; then
	px_runstop_mp
else
	echo "=========================================================="
	echo "[start or stop the sub daemon]"
	echo ""
	echo "Usage: runstop.sh"
	echo "=========================================================="
fi
