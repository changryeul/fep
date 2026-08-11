# chkshm.sh
# report the shared memory information

cd $_P_BIN

TIME=`date +%H.%M`
DATE=`date +%Y%m%d`

if [[ ! -d $_PX_LOG/$DATE ]]; then
	mkdir $_PX_LOG/$DATE
fi

px_chkshm_mp > $_PX_LOG/$DATE/chkshm_$TIME
vi -R $_PX_LOG/$DATE/chkshm_$TIME
