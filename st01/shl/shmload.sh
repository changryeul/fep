#!/bin/sh
##########################################################################
#	Module	: load shared memory for test
#	File	: shmload.sh
##########################################################################

echo ">>> [35mdo NOT excute if fep processes run[0m <<<\n"
echo "[35mare you sure to load shared memory ? (y/N)[0m \c"
read YN

case $YN in
	y*)
		echo "check logs (tlog.sh; tlog.sh err)"
		cd $_P_BIN
		echo ">>> loading ..."
		pz_memory_mp z
		echo "daemon SHM (INFO) created and loaded"
		sleep 3

		for sub in $_FEP_SUBDIR
		do
			typeset -u Sub=$sub
			pz_memory_mp $sub
			echo "P$Sub SHM created and loaded"
		done
		echo ">>> done <<<"
		;;
	*)
		echo ">>> quit <<<"
		exit
esac

##########################################################################
#	End of File (shmload.sh)
##########################################################################
