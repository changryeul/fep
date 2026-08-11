##########################################################################
#	Module	: make shell - PA - ts (TCP send)
#	File	: Make_PA_ts.sh
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
		LIB_TMP="-lnsl -lm";    			export LIB_TMP;;
esac

for i in 1101 1102 1111 1112 2101 2102
do
	case $i in
		1101|1102|1111|1112|2101|2102)
			SRC_TMP=pa_1100_ts.c	# FEP 접속, Async, client사이드
									# 1101/1102(현물송신1/2), 2101/2102(파생송신1/2)
			DEF_TMP=$MDEF" -DA${i} -DHOLIDAY_CHECK";;
	esac

	OBJ_TMP=pa_${i}_ts.o
	RUN_TMP=pa_${i}_ts
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

for i in 8101 8102
do
    case $i in
        8101|8102)
            SRC_TMP=pa_8100_ts.c;;     # Client Send
    esac

    DEF_TMP=$MDEF" -DA${i} -DHOLIDAY_APPLY"
    OBJ_TMP=pa_${i}_ts.o
    RUN_TMP=pa_${i}_ts
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PA_MAKE}/Make_PA_t.mk
done

##########################################################################
#	End of File (Make_PA_ts.sh)
##########################################################################
