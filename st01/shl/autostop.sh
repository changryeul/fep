# autostop.sh

cd $_P_BIN

if [ $# = 1 ]; then
	px_setautostop_mp $1
else
	px_setautostop_mp
fi
