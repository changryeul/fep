# setfcnt.sh
# change the read/write count of a file

cd $_P_BIN

if [ $# = 0 ]; then
	px_setfcnt_mp
elif [ $# = 3 ]; then
	px_setfcnt_mp $1 $2 $3
else
	echo "=========================================================="
	echo "[change the read/write count of a file]"
	echo ""
	echo "Usage: setfcnt.sh <file name> <R/W flag (R1 ~ R9, W1, W2)> <count>"
	echo ""
	echo "  e.g. 1) setfcnt.sh"
	echo "       2) setfcnt.sh pa_1101_dd R1 0"
	echo "=========================================================="
fi
