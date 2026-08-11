#!/bin/sh
##########################################################################
#	Module	: make shell - PA - ur (UDP receive)
#	File	: Make_PA_us.sh
##########################################################################

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";					export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";					export LIB_TMP;;
	Linux)
        MDEF=""
        LIB_TMP="-lnsl";    				export LIB_TMP;;
esac

for i in 7001 7002 7003 7561 7562 7563 7564 7565 7571 7572 9998 9999
do
	case $i in
		7001|7002|7003)
			SRC_TMP=pa_7000_us.c;;
		7561|7562|7563|7564|7565)
			SRC_TMP=pa_7500_us.c;;
		7571|7572)
			SRC_TMP=pa_7500_us.c;;
		9998|9999)
			SRC_TMP=pa_9999_us.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_us.o
	RUN_TMP=pa_${i}_us
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_u.mk
done

##########################################################################
#	End of File (Make_PA_ur.sh)
##########################################################################
