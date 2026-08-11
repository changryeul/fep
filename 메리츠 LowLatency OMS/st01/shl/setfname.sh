# setfname.sh
# change in/out file or fifo name of a process

cd $_P_BIN

if [ $# = 0 ]; then
    px_setfname_mp
elif [ $# = 3 ]; then
    px_setfname_mp $1 $2 $3
elif [ $# = 4 ]; then
    px_setfname_mp $1 $2 $3 $4
else
	echo "=========================================================="
	echo "[change in/out file or fifo name of a process]"
	echo ""
	echo "Usage: 1) setfname.sh <process ID> <file flag (I1 ~ I3)> <file name> <fifo name>"
	echo "          e.g. setfname.sh pa_1201_dd I1 pa_1201_dd pa_1201_dd1"
	echo "       2) setfname.sh <process ID> <file flag (O1 ~ O9)> <file name>"
	echo "          e.g. setfname.sh pa_1251_tr O1 pa_1201_dd"
	echo "=========================================================="
fi
