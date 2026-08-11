# ps1.sh

ps=/usr/sysv/bin/ps

echo "   RUSER      PID     PPID   C     STIME     TT  NLWP        TIME COMMAND"
echo "  ====== ======== ========   =  ======== ======  ====    ======== ==============="

$ps -efo ruser,pid,ppid,c,stime,tty,nlwp,time,comm|\
	grep $LOGNAME|grep -v "[0-9] p[aw]_"|sort +8
