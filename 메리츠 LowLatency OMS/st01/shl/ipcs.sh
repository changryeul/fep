#!/bin/sh
##########################################################################
#	Module	: report IPC facilities status
#	File	: ipcs.sh
##########################################################################

date +%Y/%m/%d-%H:%M:%S

echo ""
echo "[7m[Message Queue][0m"
echo "[35mT        ID     KEY        MODE       OWNER    GROUP  CREATOR   CGROUP CBYTES  QNUM QBYTES LSPID LRPID   STIME    RTIME    CTIME[0m"
ipcs -aq|fgrep $LOGNAME|sort +2

echo ""
echo "[7m[Semaphores][0m"
echo "[35mT        ID     KEY        MODE       OWNER    GROUP  CREATOR   CGROUP NSEMS   OTIME    CTIME[0m"
ipcs -as|fgrep $LOGNAME|sort +2

echo "[7m[Shared Memory][0m"
echo "[35mT        ID     KEY        MODE       OWNER    GROUP  CREATOR   CGROUP NATTCH     SEGSZ  CPID  LPID   ATIME    DTIME    CTIME[0m"
ipcs -am|fgrep $LOGNAME|sort +2

##########################################################################
#	End of File (ipcs.sh)
##########################################################################
