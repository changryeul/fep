# tlog.sh
# check log file
# Usage: 1) tlog.sh pa 1111_ts 2) tlog.sh err

clear
date +%Y/%m/%d-%H:%M:%S

if [ $# = 0 ]; then 
	echo "Usage: 1) tlog.sh pa 1111_ts 2) tlog.sh err"
fi

if [ $# = 1 ] || [ $# = 2 ]; then
	typeset -u Sub=$1
	typeset -l sub=$1
	if [ $sub = "pw" ] || [ $sub = "px" ] || [ $sub = "py" ] || [ $sub = "pz" ]
	then
		logfile=$_P_LOG/$Sub/`date +%Y%m%d`/*$2*
	elif [ $sub = "err" ]; then
		logfile=$_P_LOG/PZ/`date +%Y%m%d`/pz_emergency
	else
		logfile=$_P_LOG/$Sub/00000000/*$2*
	fi

	echo $logfile
	echo "[7m##########################################[0m"
	tail -f $logfile
fi
