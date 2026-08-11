# !/bin/sh
##########################################################################
#   Module  : make shell - PB (HATS System) - dd (divide)
#   File    : Make_PB_dd.sh
##########################################################################

FEP_SRC_TMP=$_PB_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;   export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";     	export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lrt";		export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";     	export LIB_TMP;;
	Linux)
		MDEF=""
		LIB_TMP="-lrt";		export LIB_TMP;;
esac

for i in 7102 7103
do
	case $i in
		7102|7103)
			SRC_TMP=pb_7100_dd.c;;
	esac

	DEF_TMP=$MDEF" -DB${i}"
	OBJ_TMP=pb_${i}_dd.o
	RUN_TMP=pb_${i}_dd
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_d.mk
done


##########################################################################
#   End of File (Make_PB_dd.sh)
##########################################################################
