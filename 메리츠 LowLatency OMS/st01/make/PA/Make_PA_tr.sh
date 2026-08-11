##########################################################################
#	Module	: make shell - PA - tr (TCP receive)
#	File	: Make_PA_tr.sh
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
		LIB_TMP="-lnsl";					export LIB_TMP;;
esac

for i in 1201 1202 1211 1212 2201 2202 1601 2601
do
	case $i in
		1201|1202|1211|1212|2201|2202)
			SRC_TMP=pa_1200_tr.c;;	# FEP 접속, client, async
		1601|2601)
			SRC_TMP=pa_1600_tr.c;;	# FEP 접속, client, async
	esac

	case $i in
		1150)
			DEF_TMP=$MDEF" -DMEM_USE -DA${i} -DHOLIDAY_CHECK";;
		*)
			DEF_TMP=$MDEF" -DSAM_USE -DA${i} -DHOLIDAY_CHECK";;
	esac

	OBJ_TMP=pa_${i}_tr.o
	RUN_TMP=pa_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

for i in 8201 8202
do
    case $i in
        8201|8202)
            SRC_TMP=pa_8200_tr.c;;     # Recv From Client
    esac

    DEF_TMP=$MDEF" -DA${i} -DHOLIDAY_APPLY"
    OBJ_TMP=pa_${i}_tr.o
    RUN_TMP=pa_${i}_tr
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PA_MAKE}/Make_PA_t.mk
done

##########################################################################
#	End of File (Make_PA_tr.sh)
##########################################################################

