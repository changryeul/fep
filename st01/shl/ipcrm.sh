#!/bin/sh
##########################################################################
#	Module	: remove shared memory ID
#	File	: ipcrm.sh
#	Note	: HP-UX ipcs: key is field 3 (type shmid key ...)
#	          Linux ipcs: key is field 1 (key shmid owner ...)
#	          Use awk to find the 0x field regardless of position.
##########################################################################

IFS='
'

for i in `ipcs -am|grep $LOGNAME|fgrep -e0x4`
do
	Key=`echo $i|awk '{for(j=1;j<=NF;j++) if($j ~ /^0x/) {print $j; exit}}'`
	if [ -n "$Key" ]; then
		ipcrm -M $Key
		echo "SHM key ($Key) removed"
	fi
done

for i in `ipcs -as|grep $LOGNAME|fgrep -e0x4`
do
	Key=`echo $i|awk '{for(j=1;j<=NF;j++) if($j ~ /^0x/) {print $j; exit}}'`
	if [ -n "$Key" ]; then
		ipcrm -S $Key
		echo "semaphore key ($Key) removed"
	fi
done

##########################################################################
#	End of File (ipcrm.sh)
##########################################################################
