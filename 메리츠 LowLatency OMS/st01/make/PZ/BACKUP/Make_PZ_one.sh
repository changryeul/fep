##########################################################################
#	Module	: make shell - PZ (°ü¸®) - make only one program
#	File	: Make_PZ_one.sh
##########################################################################

FEP_SRC_TMP=$_PZ_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PZ_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		DEF_TMP="-Ae +DAportable -D_PSTAT64";	export DEF_TMP;;
	SunOS)
		DEF_TMP="-lnsl -lsocket";	export DEF_TMP;;
	AIX)
		DEF_TMP="-O -lxnet -qcpluscmt";	export DEF_TMP;;
esac

cd $_PZ_SRC

if [ $1 = "pz_memory" ]; then
	SRC_TMP=pz_memory.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	SRC_TMP=pz_memory_proc.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	SRC_TMP=pz_memory_shm.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	SRC_TMP=pz_memory_conf.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	RUN_TMP="pz_memory_mp"
	OBJ_TMP="${_PZ_OBJ}/pz_memory.o ${_PZ_OBJ}/pz_memory_proc.o\
		${_PZ_OBJ}/pz_memory_shm.o ${_PZ_OBJ}/pz_memory_conf.o"
	export RUN_TMP OBJ_TMP
	echo $RUN_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk
elif [ $1 = "pz_daemon" ]; then
	SRC_TMP=pz_daemon.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	SRC_TMP=pz_daemon_proc.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	if [ $# = 2 ]; then
		RUN_TMP="p$2_daemon_mp"
		OBJ_TMP="${_PZ_OBJ}/pz_daemon.o ${_PZ_OBJ}/pz_daemon_proc.o\
			${_PZ_OBJ}/pz_memory_conf.o"
		export RUN_TMP OBJ_TMP
		echo $RUN_TMP
		make -f ${_PZ_MAKE}/Make_PZ_m.mk
	else
		for i in $subname
		do
			RUN_TMP="p${i}_daemon_mp"
			OBJ_TMP="${_PZ_OBJ}/pz_daemon.o ${_PZ_OBJ}/pz_daemon_proc.o\
				${_PZ_OBJ}/pz_memory_conf.o"
			export RUN_TMP OBJ_TMP
			echo $RUN_TMP
			make -f ${_PZ_MAKE}/Make_PZ_m.mk
		done
	fi
else
	SRC_TMP=$1.c;	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk

	RUN_TMP=$1_mp;	export RUN_TMP
	echo $RUN_TMP
	make -f ${_PZ_MAKE}/Make_PZ_ms.mk
fi

##########################################################################
#	End of File (Make_PZ_one.sh)
##########################################################################
