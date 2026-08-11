# sett2ip.sh
# change server IP address of TCP2 process

cd $_P_BIN

if [ $# = 0 ]; then
	px_sett2ip_mp
elif [ $# = 3 ]; then
	px_sett2ip_mp $1 $2 $3
else
	echo "=========================================================="
	echo "[change server IP address of TCP2 process]"
	echo ""
	echo "Usage: sett2ip.sh <process ID> <line (P|B|A)> <IP address>"
	echo "       (line = P:primary B:backup A:all)"
	echo ""
	echo "  e.g. sett2ip.sh pa_1111_ts P 172.30.220.168"
	echo "=========================================================="
fi
