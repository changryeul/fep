#!/bin/sh
##########################################################################
#	Module	: change UDP IP address and/or port of a process
#	File	: setudp.sh
##########################################################################

cd $_P_BIN

if [ $# = 0 ]; then
	px_setudp_mp
elif [ $# = 4 ]; then
	px_setudp_mp $1 $2 $3 $4
else
	echo "=========================================================="
	echo "[change UDP IP address and/or port of a process]"
	echo ""
	echo "Usage: setudp.sh <process name> <ID> <IP address> <port>"
	echo ""
	echo "  e.g. 1) setudp.sh"
	echo "       2) setudp.sh pa_7101_ur 1 0.0.0.0 5571"
	echo "=========================================================="
fi

##########################################################################
#	End of File (setudp.sh)
##########################################################################
