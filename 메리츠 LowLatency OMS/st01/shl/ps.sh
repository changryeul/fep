##########################################################################
#	Module	: show current status of processes
#	File	: ps.sh
##########################################################################

date +%Y/%m/%d-%H:%M:%S

if [ $osname = "HP-UX" ]; then
	if [ $# = 1 ] && [ $1 = "-h" ]; then
		echo "=========================================================="
		echo "[show current status of processes]"
		echo ""
		echo "Usage: ps.sh <sub name>"
		echo ""
		echo "  e.g. 1) ps.sh"
		echo "       2) ps.sh pa"
		echo "=========================================================="
	else
		echo "[35m     UID   PID  PPID  C    STIME TTY       TIME COMMAND[0m"
		if [ $# = 0 ]; then
			ps -ef|grep "[0-9] p[a-z]_"|grep $LOGNAME|grep -v ps.sh|\
				grep -v "ps -e"|grep -v more|grep -v cat|grep -v grep|\
				grep -v tail|grep -v vi|sort +7
		else
			ps -ef|grep "[0-9] $1_"|grep $LOGNAME|grep -v ps.sh|\
				grep -v "ps -e"|grep -v more|grep -v cat|grep -v grep|\
				grep -v tail|grep -v vi|sort +7
		fi
	fi
else
	case $osname in
		SunOS)
			pscmd='ps -elafo';;
		AIX)
			pscmd='/usr/sysv/bin/ps -efo';;
	esac

	if ( [ $# = 1 ] && [ $1 = "all" ] ) || ( [ $# = 2 ] && [ $2 = "all" ] )
	then
		psargs=ruser,pid,ppid,c,stime,tty,nlwp,lwp,s,time,etime,args
	else
		psargs=ruser,pid,ppid,c,stime,tty,nlwp,time,args
	fi

	echo "[35m`$pscmd $psargs|grep RUSER|grep -v grep`[0m"

	if [ $# -ge 3 ] || ( [ $# = 1 ] && [ $1 = "-h" ] ); then
		echo "=========================================================="
		echo "[show current status of processes (user '$LOGNAME' only)]"
		echo ""
		echo "Usage: ps.sh <sub name|\"all\"> <\"all\">"
		echo ""
		echo "  e.g. 1) ps.sh"
		echo "       2) ps.sh all"
		echo "       3) ps.sh pa"
		echo "       4) ps.sh pa all"
		echo "=========================================================="
	else
		if [ $# = 0 ] || ( [ $# = 1 ] && [ $1 = "all" ] ); then
			$pscmd $psargs|grep "[0-9] p[a-z]_"|grep $LOGNAME|grep -v ps.sh|\
				grep -v "ps -e"|grep -v more|grep -v cat|grep -v grep|\
				grep -v tail|grep -v vi|sort +8
		else
			$pscmd $psargs|grep "[0-9] $1_"|grep $LOGNAME|grep -v ps.sh|\
				grep -v "ps -e"|grep -v more|grep -v cat|grep -v grep|\
				grep -v tail|grep -v vi|sort +8
		fi
	fi
fi

##########################################################################
#	End of File (ps.sh)
##########################################################################
