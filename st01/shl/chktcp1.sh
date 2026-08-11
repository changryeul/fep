# chktcp1.sh
# check interface lines (TCP1)

date +%Y/%m/%d-%H:%M:%S
cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_chktcp1_mp $1
elif [ $# = 2 ]; then
	px_chktcp1_mp $1 $2
else
	echo "=========================================================="
	echo "[check interface lines (TCP1)]"
	echo ""
	echo "Usage: chktcp1.sh <sub name|\"all\"> <all>"
	echo ""
	echo "  e.g. 1) chktcp1.sh pa"
	echo "       2) chktcp1.sh pa all"
	echo "       3) chktcp1.sh all"
	echo "       4) chktcp1.sh all all"
	echo "=========================================================="
fi
