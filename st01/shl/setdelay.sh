# setdelay.sh
# change delay check time of a process

cd $_P_BIN

if [ $# = 0 ]; then
    px_setdelay_mp
elif [ $# = 2 ]; then
    px_setdelay_mp $1 $2
else
    echo "=========================================================="
    echo "[change delay check time of a process]"
    echo "(PROC.delay (DELAY_TIME) will be changed)"
    echo ""
    echo "Usage: setdelay.sh <process ID> <delay check time (in millisec)>"
    echo ""
    echo "  e.g. setdelay.sh pa_7001_dd 1000"
    echo "=========================================================="
fi
