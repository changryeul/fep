#!/bin/sh
##########################################################################
#	Module	: remove shared memory ID
#	File	: ipcrm.sh
##########################################################################

IFS='
'

for i in `ipcs -am|grep $LOGNAME|fgrep -e0x2`
do
	Key=`echo $i|awk '{print $3}'`
	ipcrm -M $Key
	echo "SHM key ($Key) removed"
done

for i in `ipcs -as|grep $LOGNAME|fgrep -e0x2`
do
	Key=`echo $i|awk '{print $3}'`
	ipcrm -S $Key
	echo "semaphore key ($Key) removed"
done

##########################################################################
#	End of File (ipcrm.sh)
##########################################################################
