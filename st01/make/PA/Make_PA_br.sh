##########################################################################
#   Module  : make shell - PA - br (DB receive)
#   File    : Make_PA_br.sh
##########################################################################

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";   export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";    export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";   export LIB_TMP;;
	Linux)
		MDEF=""
		LIB_TMP="-ltirpc";    export LIB_TMP;;
esac

FEP_SRC_TMP=$_PA_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;   export FEP_OBJ_TMP

for i in 9001 9002
do
#	DEF_TMP=$MDEF" -DP${i} -DHOLIDAY_TEST"
	SRC_TARGET=pa_${i}_br
	export SRC_TARGET
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_b.mk
done

##########################################################################
#   End of File (Make_GA_br.sh)
##########################################################################
