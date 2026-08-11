# chkcnt.sh
# report process interface sequence and file read/write count

date +%Y/%m/%d-%H:%M:%S
cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_chkcnt_mp $1
elif [ $# = 2 ]; then
	px_chkcnt_mp $1 $2
else
	echo "=========================================================="
	echo "[report process interface sequence and file R/W count]"
	echo ""
	echo "Usage: chkcnt.sh <\"all\"|sub name> <file name|\"batch\"|\"proc\"|\"file\">"
	echo ""
	echo "  e.g. 1) chkcnt.sh all           - 전 부문"
	echo "       2) chkcnt.sh pa            - PA 부문"
	echo "       3) chkcnt.sh pa pa         - PA 부문 process/file"
	echo "       4) chkcnt.sh pa pa_1111_ts - PA 부문 process, 지정 file"
	echo "       5) chkcnt.sh pa proc       - PA 부문 process"
	echo "       6) chkcnt.sh pa file       - PA 부문 file"
	echo "=========================================================="
fi
