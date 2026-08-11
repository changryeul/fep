if [ $# != 1 ];then
	echo "Usage : $0 [SMBS/CMBS...]"
	exit
fi

while [ 1 ];
do
	clear
	viewmst $1
	sleep 1
done
