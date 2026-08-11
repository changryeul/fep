# vlog.sh
# view log file
# Usage: 1) vlog.sh pa 1111_ts 2) vlog.sh err

if [ $# = 0 ]; then
	echo "Usage: 1) vlog.sh pa 1111_ts 2) vlog.sh err"
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

	vi -R $logfile
fi
