##########################################################################
#	Module	: make shell - PA - mp (manager)
#	File	: Make_PA_mp.sh
##########################################################################

FEP_SRC_TMP=$_P_SRC/PW;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

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

# F1 order-latency-metrics: _LAT_TRACE=1 이면 계측 빌드 (기본 off)
if [ "${_LAT_TRACE}" = "1" ]; then
	MDEF="$MDEF -DLAT_TRACE"
fi

for i in 1001
do
	SRC_TMP=pw_1000_mp.c
	DEF_TMP=$MDEF
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
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
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP=""; export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP=""; export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP=""; export LIB_TMP;;
	Linux)
        MDEF=""
        LIB_TMP=""; export LIB_TMP;;
esac

#for i in 1201 1291 1401 1491 2201 2291 2401 2491 9001
#do
#	case $i in
#		1201)
#			SRC_TMP=pa_1200_mp.c;;
#		1291)
#			SRC_TMP=pa_1290_mp.c;;
#		1401)
#			SRC_TMP=pa_1400_mp.c;;
#		1491)
#			SRC_TMP=pa_1490_mp.c;;
#		2201)
#			SRC_TMP=pa_1200_mp.c;;
#		2291)
#			SRC_TMP=pa_1290_mp.c;;
#		2401)
#			SRC_TMP=pa_1400_mp.c;;
#		2491)
#			SRC_TMP=pa_1490_mp.c;;
#		9001)
#			SRC_TMP=pa_9000_mp.c;;
#	esac
#
#	DEF_TMP=$MDEF" -DA${i}"
#	OBJ_TMP=pa_${i}_mp.o
#	RUN_TMP=pa_${i}_mp
#	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
#	echo $RUN_TMP
#	make -f ${_PA_MAKE}/Make_PA_m.mk
#done
for i in 1201 1291 2201 2291 1401 2401 1491 1492 2491 2492 9001
do
	case $i in
		1201|2201)
			SRC_TMP=pa_1200_mp.c;;
		1401|2401)
			SRC_TMP=pa_1400_mp.c;;
		1291|2291)
			SRC_TMP=pa_1290_mp.c;;
		1491|1492|2491|2492)
			SRC_TMP=pa_1490_mp.c;;
		9001)
			SRC_TMP=pa_9000_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

for i in 1601 2601
do
    SRC_TMP=pa_1600_mp.c
    DEF_TMP=$MDEF" -DA${i}"
    OBJ_TMP=pa_${i}_mp.o
    RUN_TMP=pa_${i}_mp
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PA_MAKE}/Make_PA_m.mk
done

#for i in 5020 5030
for i in 5020
do
	case $i in
		5020)
			SRC_TMP=pa_5020_mp.c;;
#		5030)
#			SRC_TMP=pa_5030_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

#for i in 6001 
#do
#    SRC_TMP=pa_6000_mp.c
#    DEF_TMP=$MDEF" -DA${i}"
#    OBJ_TMP=pa_${i}_mp.o
#    RUN_TMP=pa_${i}_mp
#    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
#    echo $RUN_TMP
#    make -f ${_PA_MAKE}/Make_PA_m.mk
#done

##########################################################################
#	End of File (Make_PA_mp.sh)
##########################################################################
