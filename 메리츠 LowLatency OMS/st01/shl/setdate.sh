# setdate.sh
# change work date of processes in a class

cd $_P_BIN

if [ $# = 0 ]; then
	px_setdate_mp
elif [ $# = 2 ]; then
	px_setdate_mp $1 $2
else
	echo "=========================================================="
	echo "[change work date of processes in a class]"
	echo ""
	echo "Usage: setdate.sh <sub name> <work date>"
	echo ""
	echo "  e.g. setdate.sh PA 20071029"
	echo "=========================================================="
fi
