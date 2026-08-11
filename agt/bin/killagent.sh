#!/bin/bash

BINDIR=/fsfxwin/fep/agt/bin
ACTION=$1

if [ -z "ACTION" ];then
	echo "사용법: $0 [oms:bat:rds]"
	exit 1
fi

case "$ACTION" in
    oms)
	    PID=$(pgrep -f "oms_agent")
		if [ -n "$PID" ]; then 
			echo "[oms_agent] 종료중 (PID=$PID)"
			kill -9 $PID

        else
			echo "[oms_agent] 실행중 아님"
        fi
		;;
    bat)
	    PID=$(pgrep -f "bat_agent")
		if [ -n "$PID" ]; then 
			echo "[bat_agent] 종료중 (PID=$PID)"
			kill -9 $PID

        else
			echo "[bat_agent] 실행중 아님"
        fi
        ;;
    rds)
	    PID=$(pgrep -f "rds_agent")
		if [ -n "$PID" ]; then 
			echo "[rds_agent] 종료중 (PID=$PID)"
			kill -9 $PID

        else
			echo "[rds_agent] 실행중 아님"
        fi
		;;
     *) 
	    echo "잘못된 명령:$0 [oms|bat|rds]  "
		;;
esac

