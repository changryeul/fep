# chktrcnt.sh
# check sise TR count

clear
cd $_P_BIN

if [ $# = 1 ]; then
    px_chktrcnt_mp $1
else
    px_chktrcnt_mp
fi
