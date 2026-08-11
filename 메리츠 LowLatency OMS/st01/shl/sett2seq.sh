# sett2seq.sh
# set interface sequence of TCP client (TCP2) process

cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_sett2seq_mp $1
elif [ $# = 2 ]; then
	px_sett2seq_mp $1 $2
else
	echo "=========================================================="
	echo "[set interface sequence of TCP client (TCP2) process]"
	echo ""
	echo "Usage: sett2seq.sh <sub name> <process ID>\n"
	echo "  e.g. 1) sett2seq.sh pa"
	echo "       2) sett2seq.sh pa pa_1111_ts"
	echo "=========================================================="
fi
