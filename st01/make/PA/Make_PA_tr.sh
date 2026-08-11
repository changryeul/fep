##########################################################################
#	Module	: make shell - PA - tr (TCP receive)
#	File	: Make_PA_tr.sh
##########################################################################

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";						export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";		export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";						export LIB_TMP;;
	Linux)
		MDEF=""
		LIB_TMP="-ltirpc -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";	export LIB_TMP;;
		#LIB_TMP="-ltirpc -D_DEFAULT_SOURCE";		export LIB_TMP;;
		#LIB_TMP="-ltirpc -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";		export LIB_TMP;;
esac

for i in 1201 1202 2201 2202 1601 1602 7701 7702 7801
do
	case $i in
		1201|1202|1601|1602)
			SRC_TMP=pa_1200_tr.c;;	# FEP 접속, client, async, 채권(KRX직접접속)
		2201|2202)
			SRC_TMP=pa_2200_tr.c;;	# FEP 접속, client, async, 금융파생(IMECO에서 장운영안줌)
		7701|7702)
			SRC_TMP=pa_2700_tr.c;;	# FEP 접속, client, async, 금융파생시세수신
		7801)
			SRC_TMP=pa_7800_tr.c;;	# FEP 접속, client, sync,  KRX일괄송신(RDS) KRX직접
	esac

#	case $i in
#		1150)
#			DEF_TMP=$MDEF" -DMEM_USE -DA${i} -DHOLIDAY_CHECK";;
#		*)
#			DEF_TMP=$MDEF" -DSAM_USE -DA${i} -DHOLIDAY_CHECK";;
#	esac
	DEF_TMP=$MDEF" -DSAM_USE -DA${i} -DHOLIDAY_CHECK"

	OBJ_TMP=pa_${i}_tr.o
	RUN_TMP=pa_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

for i in 7622 7623 8201 8202
do
    case $i in
        8201|8202)
            SRC_TMP=pa_8200_tr.c;;     # Recv From Client
		*)
            SRC_TMP=pa_7000_tr.c;;     # TCP SISE(Like KRX)
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

