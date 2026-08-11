# !/bin/sh
##########################################################################
#	System	: PARK SANG HOON HATS_H System
#	Module	: make shell - PB (선물옵션) - dd (divide)
#	File	: Make_PB_dd.sh
##########################################################################

FEP_SRC_TMP=$_PB_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";		export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lrt";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";		export LIB_TMP;;
esac

for i in 1101
do
	SRC_TMP=pb_1100_dd.c
	DEF_TMP=$MDEF
	OBJ_TMP=pb_${i}_dd.o
	RUN_TMP=pb_${i}_dd
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_d.mk
done

FEP_SRC_TMP=$_PA_SRC;   export FEP_SRC_TMP

for i in 1530 1539\
	7101 7102 7201 7202 7203
do
	case $i in
		1530|1539)
			SRC_TMP=pa_1500_dd.c;;
		7101|7102|7201|7202|7203)
			SRC_TMP=pa_7100_dd.c;;
	esac

	DEF_TMP=$MDEF" -DB${i}"
	OBJ_TMP=pb_${i}_dd.o
	RUN_TMP=pb_${i}_dd
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_d.mk
done

##########################################################################
#	End of File (Make_PB_dd.sh)
##########################################################################
