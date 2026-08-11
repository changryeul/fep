# setpseq.sh
# change interface sequence of a process

cd $_P_BIN

if [ $# = 0 ]; then
	px_setpseq_mp
elif [ $# = 2 ]; then
	px_setpseq_mp $1 $2
else
	echo "=========================================================="
	echo "[change interface sequence of a process]"
	echo ""
	echo "Usage: setpseq.sh <process ID> <interface sequence>"
	echo ""
	echo "  e.g. setpseq.sh pa_1251_tr 10"
	echo "=========================================================="
fi
