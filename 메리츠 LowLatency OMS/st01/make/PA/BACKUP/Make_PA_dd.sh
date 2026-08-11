# !/bin/sh
##########################################################################
#   Module  : make shell - PA (PK System) - dd (divide)
#   File    : Make_PA_dd.sh
##########################################################################

FEP_SRC_TMP=$_PA_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;   export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";     export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lrt"; export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";     export LIB_TMP;;
esac

for i in 6101 6201 6511 6531 6532 6533 6534 6535 6536 6537 6538 6539 6540\
	6541 7001 7101 7102 7201 7202 7203 7901 7902
do
	case $i in
		6101)
			SRC_TMP=pa_6100_dd.c;;
		6201)
			SRC_TMP=pa_6200_dd.c;;
		6511|6531|6532|6533|6534|6535|6536|6537|6538|6539|6540|\
		6541)
			SRC_TMP=pa_6500_dd.c;;
		7101|7102|7201|7202|7203|7901|7902)
			SRC_TMP=pa_7100_dd.c;;
		7001)
			SRC_TMP=pa_7000_dd.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_dd.o
	RUN_TMP=pa_${i}_dd
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_d.mk
done

##########################################################################
#   End of File (Make_PA_dd.sh)
##########################################################################
