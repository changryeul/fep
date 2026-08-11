##########################################################################
#	Module	: make programs and check errors
#	File	: mkall.sh
#	Usage	: 1) mkall.sh sub  2) mkall.sh pa  3) mkall.sh all
##########################################################################

mklog=/tmp/mk_$LOGNAME

if [ $# = 0 ] || ( [ $# = 1 ] && [ $1 = "?" ] ); then
	echo "Usage: 1) mkall.sh sub  2) mkall.sh pa  3) mkall.sh all"
else
	clear
	date +%Y/%m/%d-%H:%M:%S
	typeset -u sub=$1
	mk.sh $sub > $mklog 2>&1

	while true
	do
		clear
		cat $mklog|grep -i warning
		cat $mklog|grep -i error
		cat $mklog|grep 옜
		cat $mklog|grep FEPp|grep End|grep -v echo|sort
		sleep 5
	done
fi

##########################################################################
#	End of File (mkall.sh)
##########################################################################
