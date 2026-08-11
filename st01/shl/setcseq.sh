# setcseq.sh
# change counter sequence of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_setcseq_mp
elif [ $# = 2 ]; then
	px_setcseq_mp $1 $2
else
	echo "=========================================================="
	echo "[change counter sequence of a process]"
	echo ""
	echo "Usage: setcseq.sh <process ID> <counter sequence>"
	echo ""
	echo "  e.g. setcseq.sh pa_1111_ts 10"
	echo "=========================================================="
fi
