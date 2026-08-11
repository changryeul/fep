# chkproc.sh
# check process info (PROC)

date +%Y/%m/%d-%H:%M:%S
cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_chkproc_mp $1
elif [ $# = 2 ]; then
	px_chkproc_mp $1 $2
else
	echo "=========================================================="
	echo "[check process info (PROC)]"
	echo ""
	echo "Usage: chkproc.sh <sub name> <type>"
	echo ""
	echo "  e.g. 1) chkproc.sh pa"
	echo "       2) chkproc.sh pa ts"
	echo "=========================================================="
fi
