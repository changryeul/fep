#!/bin/sh
##########################################################################
#	Module	: make shell - PB - ur (UDP receive)
#	File	: Make_PB_ur.sh
##########################################################################

FEP_SRC_TMP=$_PB_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;	export FEP_OBJ_TMP

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
		#LIB_TMP="-lnsl";    				export LIB_TMP;;
		LIB_TMP="-ltirpc";    				export LIB_TMP;;
esac

for i in 7102 7103
do
	case $i in
		7102|7103)
			SRC_TMP=pb_7100_ur.c;;
	esac
	DEF_TMP=$MDEF" -DB${i}"
	OBJ_TMP=pb_${i}_ur.o
	RUN_TMP=pb_${i}_ur
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_u.mk
done

##########################################################################
#	End of File (Make_PB_ur.sh)
##########################################################################
