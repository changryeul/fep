##########################################################################
#	Module	: make shell - PZ (관리)
#	File	: Make_PZ_mp.sh
##########################################################################

FEP_SRC_TMP=$_PZ_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PZ_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		DEF_TMP="-Ae +DAportable -D_PSTAT64";	export DEF_TMP;;
	SunOS)
		DEF_TMP="-lsocket -lnsl -lxnet";		export DEF_TMP;;
	AIX)
		DEF_TMP="-O -lxnet -qcpluscmt";			export DEF_TMP;;
	Linux)
		#DEF_TMP="-lc -lnsl";					export DEF_TMP;;
		#DEF_TMP="-D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -lc -lnsl"   export DEF_TMP;;
		DEF_TMP="-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -lc"   export DEF_TMP;;
		#DEF_TMP="-D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -lc -ltirpc"   export DEF_TMP;;
esac

cd $_PZ_SRC
for i in *.c
do
	SRC_TMP=${i};	export SRC_TMP
	echo $SRC_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk
done

RUN_TMP="pz_memory_mp"
OBJ_TMP="${_PZ_OBJ}/pz_memory.o ${_PZ_OBJ}/pz_memory_proc.o\
	${_PZ_OBJ}/pz_memory_shm.o ${_PZ_OBJ}/pz_memory_conf.o" 
export RUN_TMP OBJ_TMP
echo $RUN_TMP
make -f ${_PZ_MAKE}/Make_PZ_m.mk

for i in $subname
do
	RUN_TMP="p${i}_daemon_mp"
	OBJ_TMP="${_PZ_OBJ}/pz_daemon.o ${_PZ_OBJ}/pz_daemon_proc.o\
		${_PZ_OBJ}/pz_memory_conf.o"
	export RUN_TMP OBJ_TMP
	echo $RUN_TMP
	make -f ${_PZ_MAKE}/Make_PZ_m.mk
done

for i in pz_compact_mp pz_filechk_mp pz_procchk_mp pz_fepp_mp
do
	RUN_TMP=${i};	export RUN_TMP
	echo $RUN_TMP
	make -f ${_PZ_MAKE}/Make_PZ_ms.mk
done

##########################################################################
#	End of File (Make_PZ_mp.sh)
##########################################################################
