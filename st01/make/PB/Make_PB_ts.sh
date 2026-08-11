##########################################################################
#	Module	: make shell - PB - ts (TCP send)
#	File	: Make_PB_ts.sh
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
		#LIB_TMP="-ltirpc -lm -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";    			export LIB_TMP;;

        MDEF="$MDEF -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE"
        LIB_TMP="-ltirpc -lm"

		#LIB_TMP="-lnsl -lm";    			export LIB_TMP;;
		#LIB_TMP="-lnsl -lm -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";    			export LIB_TMP;;
		#LIB_TMP="-ltirpc -lm";    			export LIB_TMP;;
		#LIB_TMP="-ltirpc -D_DEFAULT_SOURCE";     export LIB_TMP;;
        #LIB_TMP="-ltirpc -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";      export LIB_TMP;;
esac

# F1 order-latency-metrics: _LAT_TRACE=1 이면 계측 빌드 (기본 off)
if [ "${_LAT_TRACE}" = "1" ]; then
    MDEF="$MDEF -DLAT_TRACE"
fi

for i in 1101 1801
do
    case $i in
        1101)
            SRC_TMP=pb_1100_ts.c;;      # FEP 접속, Async, client사이드
                                        # 1101(현물송신1)
        1801)
            SRC_TMP=pb_1800_ts.c;;      #  일괄 수신 (  지점 정보 송신)
    esac

    DEF_TMP=$MDEF" -DSAM_USE -DB${i} -DHOLIDAY_CHECK"   # 파일처리
    OBJ_TMP=pb_${i}_ts.o
    RUN_TMP=pb_${i}_ts
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PB_MAKE}/Make_PB_t.mk
done

FEP_SRC_TMP=$_PB_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;   export FEP_OBJ_TMP

# KRX접속 송신
#for i in 1401 1402 1403 1411 7612 7613 8111 8116
for i in 1401 1402 1403 1411 8111 8116
do
    case $i in
        8111|8116)
            SRC_TMP=pb_8100_ts.c;;     # Send From Client
		1401|1402|1403|1411)
            SRC_TMP=pb_7100_ts.c;;     # TCP 시세송신(채권만, 시세FEP에서만)
    esac

    DEF_TMP=$MDEF" -DB${i} -DHOLIDAY_APPLY"
    OBJ_TMP=pb_${i}_ts.o
    RUN_TMP=pb_${i}_ts
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PB_MAKE}/Make_PB_t.mk
done

##########################################################################
#	End of File (Make_PB_ts.sh)
##########################################################################
