# !/bin/sh
##########################################################################
#   Module  : make shell - PA (PK System) - dd (divide)
#   File    : Make_PA_qs.sh
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
	Linux)
		MDEF=""
		LIB_TMP="-lrt";     export LIB_TMP;;
esac

for i in 5201 5211 5401 5411
do
	SRC_TMP=pa_5200_qs.c
	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_qs.o
	RUN_TMP=pa_${i}_qs
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_q.mk
done

##########################################################################
#   End of File (Make_PA_qs.sh)
##########################################################################
