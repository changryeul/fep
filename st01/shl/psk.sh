##########################################################################
#	Module	: send signal to process
#	File	: psk.sh
##########################################################################

IFS='
'

if [ $# = 0 ] || ( [ $# = 1 ] && [ $1 = "?" ] ); then
	echo "Usage: psk.sh <Pattern> [Signal(9/USR1/USR2)]"
	echo "  e.g. psk.sh pz_ USR2"
	exit
fi

typeset -u SigName=$2
SIG=-15

if [ $# = 2 ] && [ $SigName = "9" ]; then
	echo "Warning: Use of the -9 signal may corrupt any process"
	echo  "Do you want to continue(n) : \c"
	read YN

	if [ "$YN" = "n" ]; then
		echo "Aborting."
		exit
	fi

	SIG=-9
fi

if [ $# = 2 ] && [ $SigName = "USR2" ]; then
	echo "Warning: Use of the -USR2 signal may corrupt any process"
	echo "Do you want to continue(n) : \c"
	read YN

	if [ "$YN" = "n" ]; then
		echo "Aborting."
		exit
	fi

	SIG=-USR2
fi

if [ $# = 2 ] && [ $SigName = "USR1" ]; then
	echo "Warning: Use of the -USR1 signal may corrupt any process"
	echo "Do you want to continue(n) : \c"
	read YN

	if [ "$YN" = "n" ]; then
		echo "Aborting."
		exit
	fi

	SIG=-USR1
fi

echo $1 "thinking ......"

for i in `ps -ef|grep $LOGNAME|grep $1|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep`
do
	echo "$i"
	echo "`echo $i|awk '{print $8}'` ? \c"
	read response

	case $response in
		y*)
			kill $SIG `echo $i|awk '{print $2}'`;;
		q*)
			break
	esac
done

##########################################################################
#	End of File (psk.sh)
##########################################################################
