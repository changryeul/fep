# setsimsise.sh
# change the read/write count of a file

cd $_P_BIN

if [ $# = 5 ]; then
	px_setpseq_mp pa_7001_dd $1
	px_setdelay_mp pa_7001_dd $2
	px_settout_mp  pa_7001_dd $3

	px_setfcnt_mp pa_7001_mp W1 $4
	px_setfcnt_mp pa_7001_mp R1 $5

	px_setpstat_mp pa_7001_dd 1

	px_setpseq_mp pa_7002_us $1
	px_setpseq_mp pa_7003_us $1
else
	echo "=========================================================="
	echo "[change the read/write count of a file]"
	echo ""
	echo "Usage: setsimsise.sh <Options Item_Code> <Start_Time(4)> <Sleep_Time(3)>"
	echo ""
	echo "  e.g. 1) setsimsise.sh"
	echo "       2) setsimsise.sh 17 1000 1300 1000000 999999"
	echo "=========================================================="
fi
