##########################################################################
#	Module	: make shell - PB - mp (manager)
#	File	: Make_PB_mp.sh
##########################################################################

FEP_SRC_TMP=$_P_SRC/PW;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lpthread";		export LIB_TMP;;
	SunOS)
		MDEF="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
		LIB_TMP="-lrt -lpthread";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt -D_THREAD_SAFE_ERRNO"
		LIB_TMP="-lpthreads";		export LIB_TMP;;
	Linux)
		MDEF="-D_THREAD_SAFE_ERRNO -D_REENTRANT"
		LIB_TMP="-lpthread";    	export LIB_TMP;;
esac

for i in 1001
do
	SRC_TMP=pw_1000_mp.c
	DEF_TMP=$MDEF
	OBJ_TMP=pb_${i}_mp.o
	RUN_TMP=pb_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_m.mk
done

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";	export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";	export LIB_TMP;;
	Linux)
        MDEF=""
        LIB_TMP=""; export LIB_TMP;;
esac

for i in 2001
do
	SRC_TMP=pw_2000_mp.c
	DEF_TMP=$MDEF
	OBJ_TMP=pb_${i}_mp.o
	RUN_TMP=pb_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_m.mk
done

##########################################################################
#	End of File (Make_PB_mp.sh)
##########################################################################
