# setfsize.sh
# change file record size

cd $_P_BIN

if [ $# = 0 ]; then
	px_setfsize_mp
elif [ $# = 2 ]; then
	px_setfsize_mp $1 $2
else
	echo "=========================================================="
	echo "[change the record size of a file]"
	echo ""
	echo "Usage: setfsize.sh <file name> <size>"
	echo ""
	echo "  e.g. setfsize.sh pa_1201_dd 120"
	echo "=========================================================="
fi
