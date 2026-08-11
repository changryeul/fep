#!/bin/sh
##########################################################################
#	Module	: make shell - PA - ur (UDP receive)
#	File	: Make_PA_ur.sh
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

for i in 7101 7102 7201 7202 7203 7501 7502 7504\
	7520 7521 7522 7523 7524\
	7561 7562 7563 7564 7565 7571 7572 7577 7578 7579
do
	case $i in
		7501|7502|7504|7520|7521|7522|7523|7524)
			SRC_TMP=pa_7500_ur.c;;
		7561|7562|7563|7564|7565|7571|7572|7577|7578|7579)
			SRC_TMP=pa_7600_ur.c;;
		*)
			SRC_TMP=pa_7100_ur.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_ur.o
	RUN_TMP=pa_${i}_ur
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_u.mk
done

##########################################################################
#	End of File (Make_PA_ur.sh)
##########################################################################
