##########################################################################
#	Module	: stop process
#	File	: jstop.sh
##########################################################################

if [ $# -eq 0 ]; then
	echo "Usage: jstop.sh <process pattern>"
	exit 1
fi

# change process status (1:run, 9:stop)
setpstat.sh $1 9

IFS='
'

for i in `ps -ef|grep "$1"|grep -v jstop.sh|grep -v "ps -ef"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|grep $LOGNAME`
do
	procid=`echo $i|awk '{print $8}'|cut -c1-10 2>/dev/null`
	proctype=`echo $procid|cut -c9-10 2>/dev/null`

	echo "$procid killed"
	kill `echo $i|awk '{print $2}'`
done

##########################################################################
#	End of File (jstop.sh)
##########################################################################
