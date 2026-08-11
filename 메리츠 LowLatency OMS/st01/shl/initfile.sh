# initfile.sh
# change R/W count and clear data file

if [ $# = 1 ] && [ $1 != "?" ]; then
	typeset -u Sub=`echo $1|cut -c1-2 2>/dev/null`
	cd $_P_DAT/$Sub/00000000
	cp $1 $1.bak
	>$1
	echo "$_P_DAT/$Sub/00000000/$1 cleared"
	setfcnt.sh $1 W1 0
	setfcnt.sh $1 R1 0
	setfcnt.sh $1 R2 0
else
	echo "=========================================================="
	echo "[change R/W count and clear data file]"
	echo ""
	echo "Usage: initfile.sh <file name>"
	echo ""
	echo "  e.g. initfile.sh pa_1201_dd"
	echo "=========================================================="
fi
